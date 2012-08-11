/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "PresenterSprite.hxx"

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/ViewState.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;

namespace sdext { namespace presenter {

PresenterSprite::PresenterSprite (void)
    : mxSpriteFactory(),
      mxSprite(),
      maSize(0,0),
      maLocation(0,0),
      mbIsVisible(false),
      mnPriority(0),
      mnAlpha(1.0)
{
}

PresenterSprite::~PresenterSprite (void)
{
    if (mxSprite.is())
    {
        mxSprite->hide();
        Reference<lang::XComponent> xComponent (mxSprite, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
        mxSprite = NULL;
    }
}

void PresenterSprite::SetFactory (
    const ::css::uno::Reference<css::rendering::XSpriteCanvas>& rxSpriteFactory)
{
    if (mxSpriteFactory != rxSpriteFactory)
    {
        DisposeSprite();
        mxSpriteFactory = rxSpriteFactory;
        if (mbIsVisible)
            ProvideSprite();
    }
}

::css::uno::Reference<css::rendering::XCanvas> PresenterSprite::GetCanvas (void)
{
    ProvideSprite();
    if (mxSprite.is())
        return mxSprite->getContentCanvas();
    else
        return NULL;
}

void PresenterSprite::Show (void)
{
    mbIsVisible = true;
    if (mxSprite.is())
        mxSprite->show();
    else
        ProvideSprite();
}

void PresenterSprite::Hide (void)
{
    mbIsVisible = false;
    if (mxSprite.is())
        mxSprite->hide();
}

void PresenterSprite::Resize (const css::geometry::RealSize2D& rSize)
{
    maSize = rSize;
    if (mxSprite.is())
        DisposeSprite();
    if (mbIsVisible)
        ProvideSprite();
}

void PresenterSprite::MoveTo (const css::geometry::RealPoint2D& rLocation)
{
    maLocation = rLocation;
    if (mxSprite.is())
        mxSprite->move(
            maLocation,
            rendering::ViewState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL),
            rendering::RenderState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL,
                uno::Sequence<double>(4),
                rendering::CompositeOperation::SOURCE)
            );
}

void PresenterSprite::Update (void)
{
    if (mxSpriteFactory.is())
        mxSpriteFactory->updateScreen(sal_False);
}

void PresenterSprite::ProvideSprite (void)
{
    if ( ! mxSprite.is()
        && mxSpriteFactory.is()
        && maSize.Width>0
        && maSize.Height>0)
    {
        mxSprite = mxSpriteFactory->createCustomSprite(maSize);
        if (mxSprite.is())
        {
            mxSprite->move(maLocation,
                rendering::ViewState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL),
            rendering::RenderState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL,
                uno::Sequence<double>(4),
                rendering::CompositeOperation::SOURCE)
                );
            mxSprite->setAlpha(mnAlpha);
            mxSprite->setPriority(mnPriority);
            if (mbIsVisible)
                mxSprite->show();
        }
    }
}

void PresenterSprite::DisposeSprite (void)
{
    if (mxSprite.is())
    {
        mxSprite->hide();
        Reference<lang::XComponent> xComponent (mxSprite, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
        mxSprite = NULL;
    }
}

} } //end of namespace sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
