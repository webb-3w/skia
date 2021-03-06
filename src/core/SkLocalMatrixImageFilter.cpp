/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformer.h"
#include "SkImageFilterPriv.h"
#include "SkLocalMatrixImageFilter.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkString.h"

sk_sp<SkImageFilter> SkLocalMatrixImageFilter::Make(const SkMatrix& localM,
                                                    sk_sp<SkImageFilter> input) {
    if (!input) {
        return nullptr;
    }
    if (localM.getType() & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
        return nullptr;
    }
    if (localM.isIdentity()) {
        return input;
    }
    return sk_sp<SkImageFilter>(new SkLocalMatrixImageFilter(localM, input));
}

SkLocalMatrixImageFilter::SkLocalMatrixImageFilter(const SkMatrix& localM,
                                                   sk_sp<SkImageFilter> input)
    : INHERITED(&input, 1, nullptr)
    , fLocalM(localM) {
}

sk_sp<SkFlattenable> SkLocalMatrixImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix lm;
    buffer.readMatrix(&lm);
    return SkLocalMatrixImageFilter::Make(lm, common.getInput(0));
}

void SkLocalMatrixImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fLocalM);
}

sk_sp<SkSpecialImage> SkLocalMatrixImageFilter::onFilterImage(SkSpecialImage* source,
                                                              const Context& ctx,
                                                              SkIPoint* offset) const {
    Context localCtx(SkMatrix::Concat(ctx.ctm(), fLocalM), ctx.clipBounds(), ctx.cache(),
                     ctx.outputProperties());
    return this->filterInput(0, source, localCtx, offset);
}

SkIRect SkLocalMatrixImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                                 MapDirection dir, const SkIRect* inputRect) const {
    return this->getInput(0)->filterBounds(src, SkMatrix::Concat(ctm, fLocalM), dir, inputRect);
}

sk_sp<SkImageFilter> SkLocalMatrixImageFilter::onMakeColorSpace(SkColorSpaceXformer* xformer)
const {
    SkASSERT(1 == this->countInputs() && this->getInput(0));

    auto input = xformer->apply(this->getInput(0));
    if (input.get() != this->getInput(0)) {
        return SkLocalMatrixImageFilter::Make(fLocalM, std::move(input));
    }
    return this->refMe();
}
