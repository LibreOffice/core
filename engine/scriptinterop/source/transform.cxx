/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cpo/uno/Reference.hxx>
#include <cpo/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <sal/config.h>
#include <scriptinterop/XAffineTransform.hpp>
#include <scriptinterop/XAffineTransformBuilder.hpp>

#include "transform.hxx"

namespace
{
// The six coefficients of a two-dimensional affine transform.  The defaults describe the identity
// transform.
struct Coefficients
{
    double scaleX = 1.0;
    double scaleY = 1.0;
    double shearX = 0.0;
    double shearY = 0.0;
    double translateX = 0.0;
    double translateY = 0.0;
};

class AffineTransformImpl : public cppu::WeakImplHelper<scriptinterop::XAffineTransform>
{
public:
    explicit AffineTransformImpl(Coefficients const& coefficients)
        : coefficients_(coefficients)
    {
    }

    // A transform is a plain value with no drawing layer object behind it.
    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return nullptr; }

    double getScaleX() override { return coefficients_.scaleX; }

    double getScaleY() override { return coefficients_.scaleY; }

    double getShearX() override { return coefficients_.shearX; }

    double getShearY() override { return coefficients_.shearY; }

    double getTranslateX() override { return coefficients_.translateX; }

    double getTranslateY() override { return coefficients_.translateY; }

    cpo::uno::Reference<scriptinterop::XAffineTransformBuilder> toBuilder() override;

private:
    Coefficients coefficients_;
};

class AffineTransformBuilderImpl
    : public cppu::WeakImplHelper<scriptinterop::XAffineTransformBuilder>
{
public:
    AffineTransformBuilderImpl() = default;

    explicit AffineTransformBuilderImpl(Coefficients const& coefficients)
        : coefficients_(coefficients)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return nullptr; }

    cpo::uno::Reference<scriptinterop::XAffineTransform> build() override
    {
        return new AffineTransformImpl(coefficients_);
    }

    cpo::uno::Reference<scriptinterop::XAffineTransformBuilder>
    setScaleX(double scaleX) override
    {
        coefficients_.scaleX = scaleX;
        return this;
    }

    cpo::uno::Reference<scriptinterop::XAffineTransformBuilder>
    setScaleY(double scaleY) override
    {
        coefficients_.scaleY = scaleY;
        return this;
    }

    cpo::uno::Reference<scriptinterop::XAffineTransformBuilder>
    setShearX(double shearX) override
    {
        coefficients_.shearX = shearX;
        return this;
    }

    cpo::uno::Reference<scriptinterop::XAffineTransformBuilder>
    setShearY(double shearY) override
    {
        coefficients_.shearY = shearY;
        return this;
    }

    cpo::uno::Reference<scriptinterop::XAffineTransformBuilder>
    setTranslateX(double translateX) override
    {
        coefficients_.translateX = translateX;
        return this;
    }

    cpo::uno::Reference<scriptinterop::XAffineTransformBuilder>
    setTranslateY(double translateY) override
    {
        coefficients_.translateY = translateY;
        return this;
    }

private:
    Coefficients coefficients_;
};

cpo::uno::Reference<scriptinterop::XAffineTransformBuilder>
AffineTransformImpl::toBuilder()
{
    return new AffineTransformBuilderImpl(coefficients_);
}
}

namespace scriptinterop::detail
{
cpo::uno::Reference<XAffineTransformBuilder> createAffineTransformBuilder()
{
    return new AffineTransformBuilderImpl;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
