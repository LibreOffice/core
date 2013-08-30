/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <AnchorOverlayObject.hxx>
#include <SidebarWindowsConsts.hxx>

#include <swrect.hxx>
#include <view.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>

namespace sw { namespace sidebarwindows {

// helper class: Primitive for discrete visualisation
class AnchorPrimitive : public drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D
{
private:
    basegfx::B2DPolygon             maTriangle;
    basegfx::B2DPolygon             maLine;
    basegfx::B2DPolygon             maLineTop;
    const AnchorState               maAnchorState;
    basegfx::BColor                 maColor;

    // discrete line width
    double                          mfDiscreteLineWidth;

    // bitfield
    bool                            mbShadow : 1;
    bool                            mbLineSolid : 1;

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    AnchorPrimitive( const basegfx::B2DPolygon& rTriangle,
                     const basegfx::B2DPolygon& rLine,
                     const basegfx::B2DPolygon& rLineTop,
                     AnchorState aAnchorState,
                     const basegfx::BColor& rColor,
                     double fDiscreteLineWidth,
                     bool bShadow,
                     bool bLineSolid )
    :   drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D(),
        maTriangle(rTriangle),
        maLine(rLine),
        maLineTop(rLineTop),
        maAnchorState(aAnchorState),
        maColor(rColor),
        mfDiscreteLineWidth(fDiscreteLineWidth),
        mbShadow(bShadow),
        mbLineSolid(bLineSolid)
    {}

    // data access
    const basegfx::B2DPolygon& getTriangle() const { return maTriangle; }
    const basegfx::B2DPolygon& getLine() const { return maLine; }
    const basegfx::B2DPolygon& getLineTop() const { return maLineTop; }
    AnchorState getAnchorState() const { return maAnchorState; }
    const basegfx::BColor& getColor() const { return maColor; }
    double getDiscreteLineWidth() const { return mfDiscreteLineWidth; }
    bool getShadow() const { return mbShadow; }
    bool getLineSolid() const { return mbLineSolid; }

    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;

    DeclPrimitive2DIDBlock()
};

drawinglayer::primitive2d::Primitive2DSequence AnchorPrimitive::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/) const
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;

    if ( AS_TRI == maAnchorState ||
         AS_ALL == maAnchorState ||
         AS_START == maAnchorState )
    {
        // create triangle
        const drawinglayer::primitive2d::Primitive2DReference aTriangle(
            new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                basegfx::B2DPolyPolygon(getTriangle()),
                getColor()));

        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aTriangle);
    }

    // prepare view-independent LineWidth and color
    const drawinglayer::attribute::LineAttribute aLineAttribute(
        getColor(),
        getDiscreteLineWidth() * getDiscreteUnit());

    if ( AS_ALL == maAnchorState ||
         AS_START == maAnchorState )
    {
        // create line start
        if(getLineSolid())
        {
            const drawinglayer::primitive2d::Primitive2DReference aSolidLine(
                new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                    getLine(),
                    aLineAttribute));

            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aSolidLine);
        }
        else
        {
            ::std::vector< double > aDotDashArray;
            const double fDistance(3.0 * 15.0);
            const double fDashLen(5.0 * 15.0);

            aDotDashArray.push_back(fDashLen);
            aDotDashArray.push_back(fDistance);

            const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(
                aDotDashArray,
                fDistance + fDashLen);

            const drawinglayer::primitive2d::Primitive2DReference aStrokedLine(
                new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                    getLine(),
                    aLineAttribute,
                    aStrokeAttribute));

            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aStrokedLine);
        }
    }

    if(aRetval.hasElements() && getShadow())
    {
        // shadow is only for triangle and line start, and in upper left
        // and lower right direction, in different colors
        const double fColorChange(20.0 / 255.0);
        const basegfx::B3DTuple aColorChange(fColorChange, fColorChange, fColorChange);
        basegfx::BColor aLighterColor(getColor() + aColorChange);
        basegfx::BColor aDarkerColor(getColor() - aColorChange);

        aLighterColor.clamp();
        aDarkerColor.clamp();

        // create shadow sequence
        drawinglayer::primitive2d::Primitive2DSequence aShadows(2);
        basegfx::B2DHomMatrix aTransform;

        aTransform.set(0, 2, -getDiscreteUnit());
        aTransform.set(1, 2, -getDiscreteUnit());

        aShadows[0] = drawinglayer::primitive2d::Primitive2DReference(
            new drawinglayer::primitive2d::ShadowPrimitive2D(
                aTransform,
                aLighterColor,
                aRetval));

        aTransform.set(0, 2, getDiscreteUnit());
        aTransform.set(1, 2, getDiscreteUnit());

        aShadows[1] = drawinglayer::primitive2d::Primitive2DReference(
            new drawinglayer::primitive2d::ShadowPrimitive2D(
                aTransform,
                aDarkerColor,
                aRetval));

        // add shadow before geometry to make it be proccessed first
        const drawinglayer::primitive2d::Primitive2DSequence aTemporary(aRetval);

        aRetval = aShadows;
        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, aTemporary);
    }

    if ( AS_ALL == maAnchorState ||
         AS_END == maAnchorState )
    {
        // LineTop has to be created, too, but uses no shadow, so add after
        // the other parts are created
        const drawinglayer::primitive2d::Primitive2DReference aLineTop(
            new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                getLineTop(),
                aLineAttribute));

        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aLineTop);
    }

    return aRetval;
}

bool AnchorPrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(drawinglayer::primitive2d::DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
    {
        const AnchorPrimitive& rCompare = static_cast< const AnchorPrimitive& >(rPrimitive);

        return (getTriangle() == rCompare.getTriangle()
            && getLine() == rCompare.getLine()
            && getLineTop() == rCompare.getLineTop()
            && getAnchorState() == rCompare.getAnchorState()
            && getColor() == rCompare.getColor()
            && getDiscreteLineWidth() == rCompare.getDiscreteLineWidth()
            && getShadow() == rCompare.getShadow()
            && getLineSolid() == rCompare.getLineSolid());
    }

    return false;
}

ImplPrimitive2DIDBlock(AnchorPrimitive, PRIMITIVE2D_ID_SWSIDEBARANCHORPRIMITIVE)

/*static*/ AnchorOverlayObject* AnchorOverlayObject::CreateAnchorOverlayObject(
                                                       SwView& rDocView,
                                                       const SwRect& aAnchorRect,
                                                       const long& aPageBorder,
                                                       const Point& aLineStart,
                                                       const Point& aLineEnd,
                                                       const Color& aColorAnchor )
{
    AnchorOverlayObject* pAnchorOverlayObject( 0 );
    if ( rDocView.GetDrawView() )
    {
        SdrPaintWindow* pPaintWindow = rDocView.GetDrawView()->GetPaintWindow(0);
        if( pPaintWindow )
        {
            rtl::Reference< ::sdr::overlay::OverlayManager > xOverlayManager = pPaintWindow->GetOverlayManager();

            if ( xOverlayManager.is() )
            {
                pAnchorOverlayObject = new AnchorOverlayObject(
                    basegfx::B2DPoint( aAnchorRect.Left() , aAnchorRect.Bottom()-5*15),
                    basegfx::B2DPoint( aAnchorRect.Left()-5*15 , aAnchorRect.Bottom()+5*15),
                    basegfx::B2DPoint( aAnchorRect.Left()+5*15 , aAnchorRect.Bottom()+5*15),
                    basegfx::B2DPoint( aAnchorRect.Left(), aAnchorRect.Bottom()+2*15),
                    basegfx::B2DPoint( aPageBorder ,aAnchorRect.Bottom()+2*15),
                    basegfx::B2DPoint( aLineStart.X(),aLineStart.Y()),
                    basegfx::B2DPoint( aLineEnd.X(),aLineEnd.Y()) ,
                    aColorAnchor,
                    false,
                    false);
                xOverlayManager->add(*pAnchorOverlayObject);
            }
        }
    }

    return pAnchorOverlayObject;
}

/*static*/ void AnchorOverlayObject::DestroyAnchorOverlayObject( AnchorOverlayObject* pAnchor )
{
    if ( pAnchor )
    {
        if ( pAnchor->getOverlayManager() )
        {
            // remove this object from the chain
            pAnchor->getOverlayManager()->remove(*pAnchor);
        }
        delete pAnchor;
    }
}

AnchorOverlayObject::AnchorOverlayObject( const basegfx::B2DPoint& rBasePos,
                                          const basegfx::B2DPoint& rSecondPos,
                                          const basegfx::B2DPoint& rThirdPos,
                                          const basegfx::B2DPoint& rFourthPos,
                                          const basegfx::B2DPoint& rFifthPos,
                                          const basegfx::B2DPoint& rSixthPos,
                                          const basegfx::B2DPoint& rSeventhPos,
                                          const Color aBaseColor,
                                          const bool bShadowedEffect,
                                          const bool bLineSolid)
    : OverlayObjectWithBasePosition( rBasePos, aBaseColor )
    , maSecondPosition(rSecondPos)
    , maThirdPosition(rThirdPos)
    , maFourthPosition(rFourthPos)
    , maFifthPosition(rFifthPos)
    , maSixthPosition(rSixthPos)
    , maSeventhPosition(rSeventhPos)
    , maTriangle()
    , maLine()
    , maLineTop()
    , mHeight(0)
    , mAnchorState(AS_ALL)
    , mbShadowedEffect(bShadowedEffect)
    , mbLineSolid(bLineSolid)
{
}

AnchorOverlayObject::~AnchorOverlayObject()
{
}

void AnchorOverlayObject::implEnsureGeometry()
{
    if(!maTriangle.count())
    {
        maTriangle.append(getBasePosition());
        maTriangle.append(GetSecondPosition());
        maTriangle.append(GetThirdPosition());
        maTriangle.setClosed(true);
    }

    if(!maLine.count())
    {
        maLine.append(GetFourthPosition());
        maLine.append(GetFifthPosition());
        maLine.append(GetSixthPosition());
    }

  if(!maLineTop.count())
    {
        maLineTop.append(GetSixthPosition());
        maLineTop.append(GetSeventhPosition());
    }
}

void AnchorOverlayObject::implResetGeometry()
{
    maTriangle.clear();
    maLine.clear();
    maLineTop.clear();
}

drawinglayer::primitive2d::Primitive2DSequence AnchorOverlayObject::createOverlayObjectPrimitive2DSequence()
{
    implEnsureGeometry();

    static double aDiscreteLineWidth(1.6);
    const drawinglayer::primitive2d::Primitive2DReference aReference(
        new AnchorPrimitive( maTriangle,
                             maLine,
                             maLineTop,
                             GetAnchorState(),
                             getBaseColor().getBColor(),
                             ANCHORLINE_WIDTH * aDiscreteLineWidth,
                             getShadowedEffect(),
                             getLineSolid()) );

    return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
}

void AnchorOverlayObject::SetAllPosition( const basegfx::B2DPoint& rPoint1,
                                          const basegfx::B2DPoint& rPoint2,
                                          const basegfx::B2DPoint& rPoint3,
                                          const basegfx::B2DPoint& rPoint4,
                                          const basegfx::B2DPoint& rPoint5,
                                          const basegfx::B2DPoint& rPoint6,
                                          const basegfx::B2DPoint& rPoint7)
{
    if ( rPoint1 != getBasePosition() ||
         rPoint2 != GetSecondPosition() ||
         rPoint3 != GetThirdPosition() ||
         rPoint4 != GetFourthPosition() ||
         rPoint5 != GetFifthPosition() ||
         rPoint6 != GetSixthPosition() ||
         rPoint7 != GetSeventhPosition() )
    {
        maBasePosition = rPoint1;
        maSecondPosition = rPoint2;
        maThirdPosition = rPoint3;
        maFourthPosition = rPoint4;
        maFifthPosition = rPoint5;
        maSixthPosition = rPoint6;
        maSeventhPosition = rPoint7;

        implResetGeometry();
        objectChange();
    }
}

void AnchorOverlayObject::SetSixthPosition(const basegfx::B2DPoint& rNew)
{
  if(rNew != maSixthPosition)
  {
      maSixthPosition = rNew;
        implResetGeometry();
      objectChange();
  }
}

void AnchorOverlayObject::SetSeventhPosition(const basegfx::B2DPoint& rNew)
{
  if(rNew != maSeventhPosition)
  {
      maSeventhPosition = rNew;
        implResetGeometry();
      objectChange();
  }
}

void AnchorOverlayObject::SetTriPosition(const basegfx::B2DPoint& rPoint1,const basegfx::B2DPoint& rPoint2,const basegfx::B2DPoint& rPoint3,
                                  const basegfx::B2DPoint& rPoint4,const basegfx::B2DPoint& rPoint5)
{
    if(rPoint1 != getBasePosition()
        || rPoint2 != GetSecondPosition()
        || rPoint3 != GetThirdPosition()
        || rPoint4 != GetFourthPosition()
        || rPoint5 != GetFifthPosition())
    {
      maBasePosition = rPoint1;
      maSecondPosition = rPoint2;
      maThirdPosition = rPoint3;
      maFourthPosition = rPoint4;
      maFifthPosition = rPoint5;

      implResetGeometry();
      objectChange();
    }
}

void AnchorOverlayObject::setLineSolid( const bool bNew )
{
  if ( bNew != getLineSolid() )
  {
      mbLineSolid = bNew;
      objectChange();
  }
}

void AnchorOverlayObject::SetAnchorState( const AnchorState aState)
{
  if ( mAnchorState != aState)
  {
      mAnchorState = aState;
      objectChange();
  }
}

} } // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
