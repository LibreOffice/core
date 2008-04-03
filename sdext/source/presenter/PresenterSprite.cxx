/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterSprite.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:05:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
      maTransform(1,0,0, 0,1,0),
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
            PresenterSprite();
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




bool PresenterSprite::IsVisible (void) const
{
    return mbIsVisible;
}




void PresenterSprite::SetPriority (const double nPriority)
{
    mnPriority = nPriority;
    if (mxSprite.is())
        mxSprite->setPriority(mnPriority);
}




double PresenterSprite::GetPriority (void) const
{
    return mnPriority;
}




void PresenterSprite::Resize (const css::geometry::RealSize2D& rSize)
{
    maSize = rSize;
    if (mxSprite.is())
        DisposeSprite();
    if (mbIsVisible)
        ProvideSprite();
}




css::geometry::RealSize2D PresenterSprite::GetSize (void) const
{
    return maSize;
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
                uno::Sequence<double>(3),
                rendering::CompositeOperation::SOURCE)
            );
}




css::geometry::RealPoint2D PresenterSprite::GetLocation (void) const
{
    return maLocation;
}




void PresenterSprite::Transform (const css::geometry::AffineMatrix2D& rTransform)
{
    maTransform = rTransform;
    if (mxSprite.is())
        mxSprite->transform(maTransform);
}




css::geometry::AffineMatrix2D PresenterSprite::GetTransform (void) const
{
    return maTransform;
}




void PresenterSprite::SetAlpha (const double nAlpha)
{
    mnAlpha = nAlpha;
    if (mxSprite.is())
        mxSprite->setAlpha(mnAlpha);
}




double PresenterSprite::GetAlpha (void) const
{
    return mnAlpha;
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
            mxSprite->transform(maTransform);
            mxSprite->move(maLocation,
                rendering::ViewState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL),
            rendering::RenderState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL,
                uno::Sequence<double>(3),
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

} }
