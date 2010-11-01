/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/************************************************************************* *
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postit.cxx,v $
 * $Revision: 1.8.42.11 $
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


#include "precompiled_sw.hxx"

#include <ShadowOverlayObject.hxx>

#include <view.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>

namespace sw { namespace sidebarwindows {

//////////////////////////////////////////////////////////////////////////////
// helper SwPostItShadowPrimitive
//
// Used to allow view-dependent primitive definition. For that purpose, the
// initially created primitive (this one) always has to be view-independent,
// but the decomposition is made view-dependent. Very simple primitive which
// just remembers the discrete data and applies it at decomposition time.

class ShadowPrimitive : public drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D
{
private:
    basegfx::B2DPoint           maBasePosition;
    basegfx::B2DPoint           maSecondPosition;
    ShadowState                 maShadowState;

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    ShadowPrimitive(
        const basegfx::B2DPoint& rBasePosition,
        const basegfx::B2DPoint& rSecondPosition,
        ShadowState aShadowState)
    :   drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D(),
        maBasePosition(rBasePosition),
        maSecondPosition(rSecondPosition),
        maShadowState(aShadowState)
    {}

    // data access
    const basegfx::B2DPoint& getBasePosition() const { return maBasePosition; }
    const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }
    ShadowState getShadowState() const { return maShadowState; }

    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;

    DeclPrimitrive2DIDBlock()
};

drawinglayer::primitive2d::Primitive2DSequence ShadowPrimitive::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // get logic sizes in object coordinate system
    drawinglayer::primitive2d::Primitive2DSequence xRetval;
    basegfx::B2DRange aRange(getBasePosition());

    switch(maShadowState)
    {
        case SS_NORMAL:
        {
            aRange.expand(basegfx::B2DTuple(getSecondPosition().getX(), getSecondPosition().getY() + (2.0 * getDiscreteUnit())));
            const ::drawinglayer::attribute::FillGradientAttribute aFillGradientAttribute(
                drawinglayer::attribute::GRADIENTSTYLE_LINEAR,
                0.0,
                0.5,
                0.5,
                1800.0 * F_PI1800,
                basegfx::BColor(230.0/255.0,230.0/255.0,230.0/255.0),
                basegfx::BColor(180.0/255.0,180.0/255.0,180.0/255.0),
                2);

            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::FillGradientPrimitive2D(
                    aRange,
                    aFillGradientAttribute));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            break;
        }
        case SS_VIEW:
        {
            aRange.expand(basegfx::B2DTuple(getSecondPosition().getX(), getSecondPosition().getY() + (4.0 * getDiscreteUnit())));
            const drawinglayer::attribute::FillGradientAttribute aFillGradientAttribute(
                drawinglayer::attribute::GRADIENTSTYLE_LINEAR,
                0.0,
                0.5,
                0.5,
                1800.0 * F_PI1800,
                basegfx::BColor(230.0/255.0,230.0/255.0,230.0/255.0),
                basegfx::BColor(180.0/255.0,180.0/255.0,180.0/255.0),
                4);

            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::FillGradientPrimitive2D(
                    aRange,
                    aFillGradientAttribute));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            break;
        }
        case SS_EDIT:
        {
            aRange.expand(basegfx::B2DTuple(getSecondPosition().getX(), getSecondPosition().getY() + (4.0 * getDiscreteUnit())));
            const drawinglayer::attribute::FillGradientAttribute aFillGradientAttribute(
                drawinglayer::attribute::GRADIENTSTYLE_LINEAR,
                0.0,
                0.5,
                0.5,
                1800.0 * F_PI1800,
                basegfx::BColor(230.0/255.0,230.0/255.0,230.0/255.0),
                basegfx::BColor(83.0/255.0,83.0/255.0,83.0/255.0),
                4);

            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::FillGradientPrimitive2D(
                    aRange,
                    aFillGradientAttribute));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            break;
        }
        default:
        {
            break;
        }
    }

    return xRetval;
}

bool ShadowPrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
    {
        const ShadowPrimitive& rCompare = static_cast< const ShadowPrimitive& >(rPrimitive);

        return (getBasePosition() == rCompare.getBasePosition()
            && getSecondPosition() == rCompare.getSecondPosition()
            && getShadowState() == rCompare.getShadowState());
    }

    return false;
}

ImplPrimitrive2DIDBlock(ShadowPrimitive, PRIMITIVE2D_ID_SWSIDEBARSHADOWPRIMITIVE)

/****** ShadowOverlayObject  **************************************************/
/* static */ ShadowOverlayObject* ShadowOverlayObject::CreateShadowOverlayObject( SwView& rDocView )
{
    ShadowOverlayObject* pShadowOverlayObject( 0 );

    if ( rDocView.GetDrawView() )
    {
        SdrPaintWindow* pPaintWindow = rDocView.GetDrawView()->GetPaintWindow(0);
        if( pPaintWindow )
        {
            sdr::overlay::OverlayManager* pOverlayManager = pPaintWindow->GetOverlayManager();

            if ( pOverlayManager )
            {
                pShadowOverlayObject = new ShadowOverlayObject( basegfx::B2DPoint(0,0),
                                                                basegfx::B2DPoint(0,0),
                                                                Color(0,0,0),
                                                                SS_NORMAL );
                pOverlayManager->add(*pShadowOverlayObject);
            }
        }
    }

    return pShadowOverlayObject;
}

/* static */ void ShadowOverlayObject::DestroyShadowOverlayObject( ShadowOverlayObject* pShadow )
{
    if ( pShadow )
    {
        if ( pShadow->getOverlayManager() )
        {
            pShadow->getOverlayManager()->remove(*pShadow);
        }
        delete pShadow;
    }
}

ShadowOverlayObject::ShadowOverlayObject( const basegfx::B2DPoint& rBasePos,
                                          const basegfx::B2DPoint& rSecondPosition,
                                          Color aBaseColor,
                                          ShadowState aState )
    : OverlayObjectWithBasePosition(rBasePos, aBaseColor)
    , maSecondPosition(rSecondPosition)
    , mShadowState(aState)
{
}

ShadowOverlayObject::~ShadowOverlayObject()
{
}

drawinglayer::primitive2d::Primitive2DSequence ShadowOverlayObject::createOverlayObjectPrimitive2DSequence()
{
    const drawinglayer::primitive2d::Primitive2DReference aReference(
        new ShadowPrimitive( getBasePosition(),
                             GetSecondPosition(),
                             GetShadowState() ) );
    return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
}

void ShadowOverlayObject::SetShadowState(ShadowState aState)
{
    if (mShadowState != aState)
    {
        mShadowState = aState;

        objectChange();
    }
}

void ShadowOverlayObject::SetPosition( const basegfx::B2DPoint& rPoint1,
                                       const basegfx::B2DPoint& rPoint2)
{
    if(!rPoint1.equal(getBasePosition()) || !rPoint2.equal(GetSecondPosition()))
    {
        maBasePosition = rPoint1;
        maSecondPosition = rPoint2;

        objectChange();
    }
}

} } // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
