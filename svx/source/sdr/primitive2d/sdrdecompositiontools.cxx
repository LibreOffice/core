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

#include <sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHatchPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGraphicPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <sdr/attribute/sdrtextattribute.hxx>
#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <svx/svdotext.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <drawinglayer/attribute/sdrglowattribute.hxx>
#include <osl/diagnose.h>

// for SlideBackgroundFillPrimitive2D
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <set>

using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
namespace
{
// See also: SdrTextObj::AdjustRectToTextDistance
basegfx::B2DRange getTextAnchorRange(const attribute::SdrTextAttribute& rText,
                                     const basegfx::B2DRange& rSnapRange)
{
    // Take vertical text orientation into account when deciding
    // which dimension is its width, and which is its height
    const OutlinerParaObject& rOutlinerParaObj = rText.getOutlinerParaObject();
    const bool bVerticalWriting(rOutlinerParaObj.IsEffectivelyVertical());
    const double fWidthForText = bVerticalWriting ? rSnapRange.getHeight() : rSnapRange.getWidth();
    // create a range describing the wanted text position and size (aTextAnchorRange). This
    // means to use the text distance values here
    // If the margin is larger than the entire width of the text area, then limit the
    // margin.
    const double fTextLeftDistance
        = std::min(static_cast<double>(rText.getTextLeftDistance()), fWidthForText);
    const double nTextRightDistance
        = std::min(static_cast<double>(rText.getTextRightDistance()), fWidthForText);
    double fDistanceForTextL, fDistanceForTextT, fDistanceForTextR, fDistanceForTextB;
    if (!bVerticalWriting)
    {
        fDistanceForTextL = fTextLeftDistance;
        fDistanceForTextT = rText.getTextUpperDistance();
        fDistanceForTextR = nTextRightDistance;
        fDistanceForTextB = rText.getTextLowerDistance();
    }
    else if (rOutlinerParaObj.IsTopToBottom())
    {
        fDistanceForTextL = rText.getTextLowerDistance();
        fDistanceForTextT = fTextLeftDistance;
        fDistanceForTextR = rText.getTextUpperDistance();
        fDistanceForTextB = nTextRightDistance;
    }
    else
    {
        fDistanceForTextL = rText.getTextUpperDistance();
        fDistanceForTextT = nTextRightDistance;
        fDistanceForTextR = rText.getTextLowerDistance();
        fDistanceForTextB = fTextLeftDistance;
    }
    const basegfx::B2DPoint aTopLeft(rSnapRange.getMinX() + fDistanceForTextL,
                                     rSnapRange.getMinY() + fDistanceForTextT);
    const basegfx::B2DPoint aBottomRight(rSnapRange.getMaxX() - fDistanceForTextR,
                                         rSnapRange.getMaxY() - fDistanceForTextB);
    basegfx::B2DRange aAnchorRange;
    aAnchorRange.expand(aTopLeft);
    aAnchorRange.expand(aBottomRight);

    // If the shape has no width, then don't attempt to break the text into multiple
    // lines, not a single character would satisfy a zero width requirement.
    // SdrTextObj::impDecomposeBlockTextPrimitive() uses the same constant to
    // effectively set no limits.
    if (!bVerticalWriting && aAnchorRange.getWidth() == 0)
    {
        aAnchorRange.expand(basegfx::B2DPoint(aTopLeft.getX() - 1000000, aTopLeft.getY()));
        aAnchorRange.expand(basegfx::B2DPoint(aBottomRight.getX() + 1000000, aBottomRight.getY()));
    }
    else if (bVerticalWriting && aAnchorRange.getHeight() == 0)
    {
        aAnchorRange.expand(basegfx::B2DPoint(aTopLeft.getX(), aTopLeft.getY() - 1000000));
        aAnchorRange.expand(basegfx::B2DPoint(aBottomRight.getX(), aBottomRight.getY() + 1000000));
    }
    return aAnchorRange;
}

sdr::contact::ViewContact* getMasterPageViewContact(
    const geometry::ViewInformation2D& rViewInformation,
    basegfx::B2DVector& rPageSize)
{
    // get SdrPage
    const SdrPage* pVisualizedPage(GetSdrPageFromXDrawPage(rViewInformation.getVisualizedPage()));
    if(nullptr == pVisualizedPage)
        return nullptr;

    // do not use in MasterPage mode, so initial SdrPage shall *not* be a
    // MasterPage
    if(pVisualizedPage->IsMasterPage())
        return nullptr;

    // we need that SdrPage to have a MasterPage
    if(!pVisualizedPage->TRG_HasMasterPage())
        return nullptr;

    // copy needed values for processing
    rPageSize.setX(pVisualizedPage->GetWidth());
    rPageSize.setY(pVisualizedPage->GetHeight());

    // return it's ViewContact
    return &pVisualizedPage->TRG_GetMasterPageDescriptorViewContact();
}

// provide a Primitive2D for the SlideBackgroundFill-mode. It is capable
// of expressing that state of fill and it's decomposition implements all
// needed preparation of the geometry in an isolated and controllable
// space and way.
// It is currently simple buffered (due to being derived from
// BufferedDecompositionPrimitive2D) and detects if MasterPage changes
class SlideBackgroundFillPrimitive2D final : public BufferedDecompositionPrimitive2D
{
private:
    /// the basegfx::B2DPolyPolygon geometry
    basegfx::B2DPolyPolygon maPolyPolygon;

    /// the polygon fill color - to allow simple fallback if needed
    basegfx::BColor maBColor;

    /// the last VC the geometry was created for
    sdr::contact::ViewContact* mpLastVC;

protected:
    // create decomposition data
    virtual void create2DDecomposition(
        Primitive2DContainer& rContainer,
        const geometry::ViewInformation2D& rViewInformation) const override;

public:
    /// constructor
    SlideBackgroundFillPrimitive2D(
        const basegfx::B2DPolyPolygon& rPolyPolygon,
        const basegfx::BColor& rBColor);

    /// check existing decomposition data, call parent
    virtual void get2DDecomposition(
        Primitive2DDecompositionVisitor& rVisitor,
        const geometry::ViewInformation2D& rViewInformation) const override;

    /// data read access
    const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
    const basegfx::BColor& getBColor() const { return maBColor; }

    /// compare operator
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    /// get range
    virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    /// provide unique ID
    virtual sal_uInt32 getPrimitive2DID() const override;
};

SlideBackgroundFillPrimitive2D::SlideBackgroundFillPrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon,
    const basegfx::BColor& rBColor)
: BufferedDecompositionPrimitive2D()
, maPolyPolygon(rPolyPolygon)
, maBColor(rBColor)
, mpLastVC(nullptr)
{
}

void SlideBackgroundFillPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer,
    const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DVector aPageSize;
    sdr::contact::ViewContact* pViewContact(getMasterPageViewContact(rViewInformation, aPageSize));

    // Check that we have a referenced SdrPage that is no MasterPage itself and has a MasterPage,
    // we got it's ViewContact in pViewContact
    if(nullptr != pViewContact)
    {
        // Get PolygonRange of own local geometry
        const basegfx::B2DRange aPolygonRange(getB2DPolyPolygon().getB2DRange());

        // if local geometry is empty, nothing will be shown, we are done
        if(aPolygonRange.isEmpty())
            return;

        // Get PageRange
        const basegfx::B2DRange aPageRange(0.0, 0.0, aPageSize.getX(), aPageSize.getY());

        // if local geometry does not overlap with PageRange, nothing will be shown, we are done
        if(!aPageRange.overlaps(aPolygonRange))
            return;

        // Get the geometry

        // Add the MasterPage BG fill (if used, e.g. Picture/gradient, ...)
        pViewContact->getViewIndependentPrimitive2DContainer(rContainer);

        // To create the MasterPage's ObjectHierarchy we need an ObjectContact (AKA View-side of things).
        // We do not have one, but can use a temporary one anytime for temporary work. That whole VC/VOC/OC
        // is designed to work with on-demand temporary objects if needed
        sdr::contact::ObjectContact aObjectContact;

        // get the VOC from it (gets created)
        sdr::contact::ViewObjectContact& rViewObjectContact(pViewContact->GetViewObjectContact(aObjectContact));
        sdr::contact::DisplayInfo aDisplayInfo;

        // we need this DisplayInfo-flag here - exceptionally - to get the same output as
        // if the MasterPage is used as sub-content when creating geometry content for
        // the non-MasterPage-view
        aDisplayInfo.SetSubContentActive(true);

        // get the full MasterPage ObjectHierarchy
        rViewObjectContact.getPrimitive2DSequenceSubHierarchy(aDisplayInfo, rContainer);

        if(!rContainer.empty())
        {
            // We got the geometry, but it may overlap the PageBounds of the
            // Page/MasterPage, thus showing more beyond the PageBorders than
            // the regular PageView does and is intended.
            // This is independent from the geometry we collected in rContainer
            // since the defining geometry is the getB2DPolyPolygon() one.
            // We have already checked above that it's no empty and overlaps
            // somehow.
            // It also might be completely inside the PageRange. If not, we
            // additionally would need to mask the content against PageBounds,
            // so using potentially two different MaskPrimitive2D's.
            // Since in this case we have a PolyPolygon and a B2DRange it is cheaper
            // to geometrically clip that PolyPolygon geometry and use it
            basegfx::B2DPolyPolygon aPolyPolygon(getB2DPolyPolygon());

            if(!aPageRange.isInside(aPolygonRange))
            {
                // we need to clip local geometry against PageBounds
                aPolyPolygon = basegfx::utils::clipPolyPolygonOnRange(
                    aPolyPolygon,
                    aPageRange,
                    true /* bInside, use inside geometry */,
                    false /* bStroke, handle as filled PolyPolygon */);
            }

            // create MaskPrimitive2D to limit display to PolygonGeometry
            const Primitive2DReference aMasked(
                new MaskPrimitive2D(
                    aPolyPolygon,
                    std::move(rContainer)));

            rContainer = Primitive2DContainer { aMasked };
            return;
        }
    }

    // fallback: create as if drawing::FillStyle_SOLID was used
    rContainer.push_back(
        new PolyPolygonColorPrimitive2D(
            getB2DPolyPolygon(),
            getBColor()));
}

void SlideBackgroundFillPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DVector aPageSize;
    sdr::contact::ViewContact* pViewContact(getMasterPageViewContact(rViewInformation, aPageSize));

    if(!getBuffered2DDecomposition().empty())
    {
        if(nullptr != pViewContact && pViewContact != mpLastVC)
        {
            // conditions of last local decomposition have changed, delete
            const_cast< SlideBackgroundFillPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
        }
    }

    if(getBuffered2DDecomposition().empty())
    {
        // remember last MasterPageViewContact
        const_cast< SlideBackgroundFillPrimitive2D* >(this)->mpLastVC = pViewContact;
    }

    // tdf#149650 allow remember/detect of potential recursion for content creation.
    // use a std::set association - instead of a single bool or adress - due to the
    // possibility of multiple SlideBackgroundFillPrimitive2D's being used at the same
    // refresh. Also possible would be a local member (bool), but that just makes the
    // class more complicated. Working wth the adress is not a problem here since below
    // it reliably gets added/removed while being incarnated only.
    static std::set<const SlideBackgroundFillPrimitive2D*> potentiallyActiveRecursion;

    if(potentiallyActiveRecursion.end() != potentiallyActiveRecursion.find(this))
    {
        // The method getPrimitive2DSequenceSubHierarchy used in create2DDecomposition
        // above has the potential to create a recursion, e.g. when the content of a page
        // contains a SdrPageObj that again displays the page content (and potentially so
        // on).
        // This is valid, but works like a fractal, showing page content
        // smaller and smaller inside a page. This needs to be controlled here to avoid
        // the recursion. In this case just allow one single step since
        // we are mainly interested in the page's BG fill anyways
        return;
    }

    // remember that we enter a potential recursion
    potentiallyActiveRecursion.insert(this);

    // use parent implementation
    BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);

    // forget about potential recursion
    potentiallyActiveRecursion.extract(this);
}

bool SlideBackgroundFillPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const SlideBackgroundFillPrimitive2D& rCompare
            = static_cast<const SlideBackgroundFillPrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
            && getBColor() == rCompare.getBColor());
    }

    return false;
}

basegfx::B2DRange SlideBackgroundFillPrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    // return range
    return basegfx::utils::getRange(getB2DPolyPolygon());
}

// provide unique ID
sal_uInt32 SlideBackgroundFillPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SLIDEBACKGROUNDFILLPRIMITIVE2D;
}

}; // end of anonymous namespace

        class TransparencePrimitive2D;

        Primitive2DReference createPolyPolygonFillPrimitive(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient)
        {
            // when we have no given definition range, use the range of the given geometry
            // also for definition (simplest case)
            const basegfx::B2DRange aRange(basegfx::utils::getRange(rPolyPolygon));

            return createPolyPolygonFillPrimitive(
                rPolyPolygon,
                aRange,
                rFill,
                rFillGradient);
        }

        Primitive2DReference createPolyPolygonFillPrimitive(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::B2DRange& rDefinitionRange,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient)
        {
            if(basegfx::fTools::moreOrEqual(rFill.getTransparence(), 1.0))
            {
                return Primitive2DReference();
            }

            // prepare fully scaled polygon
            rtl::Reference<BasePrimitive2D> pNewFillPrimitive;

            if(!rFill.getGradient().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonGradientPrimitive2D(
                    rPolyPolygon,
                    rDefinitionRange,
                    rFill.getGradient());
            }
            else if(!rFill.getHatch().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonHatchPrimitive2D(
                    rPolyPolygon,
                    rDefinitionRange,
                    rFill.getColor(),
                    rFill.getHatch());
            }
            else if(!rFill.getFillGraphic().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonGraphicPrimitive2D(
                    rPolyPolygon,
                    rDefinitionRange,
                    rFill.getFillGraphic().createFillGraphicAttribute(rDefinitionRange));
            }
            else if(rFill.isSlideBackgroundFill())
            {
                // create needed Primitive2D representation for
                // SlideBackgroundFill-mode
                pNewFillPrimitive = new SlideBackgroundFillPrimitive2D(
                    rPolyPolygon,
                    rFill.getColor());
            }
            else
            {
                pNewFillPrimitive = new PolyPolygonColorPrimitive2D(
                    rPolyPolygon,
                    rFill.getColor());
            }

            if(0.0 != rFill.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitive
                Primitive2DContainer aContent { pNewFillPrimitive };
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(std::move(aContent), rFill.getTransparence()));
            }
            else if(!rFillGradient.isDefault())
            {
                // create sequence with created fill primitive
                Primitive2DContainer aContent { pNewFillPrimitive };

                // create FillGradientPrimitive2D for transparence and add to new sequence
                // fillGradientPrimitive is enough here (compared to PolyPolygonGradientPrimitive2D) since float transparence will be masked anyways
                const basegfx::B2DRange aRange(basegfx::utils::getRange(rPolyPolygon));
                Primitive2DReference xRefB(
                    new FillGradientPrimitive2D(
                        aRange,
                        rDefinitionRange,
                        rFillGradient));
                Primitive2DContainer aAlpha { xRefB };

                // create TransparencePrimitive2D using alpha and content
                return Primitive2DReference(new TransparencePrimitive2D(std::move(aContent), std::move(aAlpha)));
            }
            else
            {
                // add to decomposition
                return pNewFillPrimitive;
            }
        }

        Primitive2DReference createPolygonLinePrimitive(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::SdrLineAttribute& rLine,
            const attribute::SdrLineStartEndAttribute& rStroke)
        {
            // create line and stroke attribute
            const attribute::LineAttribute aLineAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin(), rLine.getCap());
            attribute::StrokeAttribute aStrokeAttribute(std::vector(rLine.getDotDashArray()), rLine.getFullDotDashLen());
            rtl::Reference<BasePrimitive2D> pNewLinePrimitive;

            if(!rPolygon.isClosed() && !rStroke.isDefault())
            {
                attribute::LineStartEndAttribute aStart(rStroke.getStartWidth(), rStroke.getStartPolyPolygon(), rStroke.isStartCentered());
                attribute::LineStartEndAttribute aEnd(rStroke.getEndWidth(), rStroke.getEndPolyPolygon(), rStroke.isEndCentered());

                // create data
                pNewLinePrimitive = new PolygonStrokeArrowPrimitive2D(rPolygon, aLineAttribute, aStrokeAttribute, aStart, aEnd);
            }
            else
            {
                // create data
                pNewLinePrimitive = new PolygonStrokePrimitive2D(rPolygon, aLineAttribute, std::move(aStrokeAttribute));
            }

            if(0.0 != rLine.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitive
                Primitive2DContainer aContent { pNewLinePrimitive };
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(std::move(aContent), rLine.getTransparence()));
            }
            else
            {
                // add to decomposition
                return pNewLinePrimitive;
            }
        }

        Primitive2DReference createTextPrimitive(
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrTextAttribute& rText,
            const attribute::SdrLineAttribute& rStroke,
            bool bCellText,
            bool bWordWrap)
        {
            basegfx::B2DHomMatrix aAnchorTransform(rObjectTransform);
            rtl::Reference<SdrTextPrimitive2D> pNew;

            if(rText.isContour())
            {
                // contour text
                if(!rStroke.isDefault() && 0.0 != rStroke.getWidth())
                {
                    // take line width into account and shrink contour polygon accordingly
                    // decompose to get scale
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    rObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                    // scale outline to object's size to allow growing with value relative to that size
                    // and also to keep aspect ratio
                    basegfx::B2DPolyPolygon aScaledUnitPolyPolygon(rUnitPolyPolygon);
                    aScaledUnitPolyPolygon.transform(basegfx::utils::createScaleB2DHomMatrix(
                        fabs(aScale.getX()), fabs(aScale.getY())));

                    // grow the polygon. To shrink, use negative value (half width)
                    aScaledUnitPolyPolygon = basegfx::utils::growInNormalDirection(aScaledUnitPolyPolygon, -(rStroke.getWidth() * 0.5));

                    // scale back to unit polygon
                    aScaledUnitPolyPolygon.transform(basegfx::utils::createScaleB2DHomMatrix(
                        0.0 != aScale.getX() ? 1.0 / aScale.getX() : 1.0,
                        0.0 != aScale.getY() ? 1.0 / aScale.getY() : 1.0));

                    // create with unit polygon
                    pNew = new SdrContourTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aScaledUnitPolyPolygon,
                        rObjectTransform);
                }
                else
                {
                    // create with unit polygon
                    pNew = new SdrContourTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        rUnitPolyPolygon,
                        rObjectTransform);
                }
            }
            else if(!rText.getSdrFormTextAttribute().isDefault())
            {
                // text on path, use scaled polygon
                basegfx::B2DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
                aScaledPolyPolygon.transform(rObjectTransform);
                pNew = new SdrPathTextPrimitive2D(
                    &rText.getSdrText(),
                    rText.getOutlinerParaObject(),
                    aScaledPolyPolygon,
                    rText.getSdrFormTextAttribute());
            }
            else
            {
                // rObjectTransform is the whole SdrObject transformation from unit rectangle
                // to its size and position. Decompose to allow working with single values.
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                rObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                // extract mirroring
                const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
                const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));
                aScale = basegfx::absolute(aScale);

                // Get the real size, since polygon outline and scale
                // from the object transformation may vary (e.g. ellipse segments)
                basegfx::B2DHomMatrix aJustScaleTransform;
                aJustScaleTransform.set(0, 0, aScale.getX());
                aJustScaleTransform.set(1, 1, aScale.getY());
                basegfx::B2DPolyPolygon aScaledUnitPolyPolygon(rUnitPolyPolygon);
                aScaledUnitPolyPolygon.transform(aJustScaleTransform);
                const basegfx::B2DRange aTextAnchorRange
                    = getTextAnchorRange(rText, basegfx::utils::getRange(aScaledUnitPolyPolygon));

                // now create a transformation from this basic range (aTextAnchorRange)
                // #i121494# if we have no scale use at least 1.0 to have a carrier e.g. for
                // mirror values, else these will get lost
                aAnchorTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(
                    basegfx::fTools::equalZero(aTextAnchorRange.getWidth()) ? 1.0 : aTextAnchorRange.getWidth(),
                    basegfx::fTools::equalZero(aTextAnchorRange.getHeight()) ? 1.0 : aTextAnchorRange.getHeight(),
                    aTextAnchorRange.getMinX(), aTextAnchorRange.getMinY());

                // apply mirroring
                aAnchorTransform.scale(bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0);

                // apply object's other transforms
                aAnchorTransform = basegfx::utils::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                    * aAnchorTransform;

                if(rText.isFitToSize())
                {
                    // stretched text in range
                    pNew = new SdrStretchTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aAnchorTransform,
                        rText.isFixedCellHeight());
                }
                else if(rText.isAutoFit())
                {
                    // isotropically scaled text in range
                    pNew = new SdrAutoFitTextPrimitive2D(
                                    &rText.getSdrText(),
                                    rText.getOutlinerParaObject(),
                                    aAnchorTransform,
                                    bWordWrap);
                }
                else if( rText.isChainable() && !rText.isInEditMode() )
                {
                    pNew = new SdrChainedTextPrimitive2D(
                                    &rText.getSdrText(),
                                    rText.getOutlinerParaObject(),
                                    aAnchorTransform );
                }
                else // text in range
                {
                    // build new primitive
                    pNew = new SdrBlockTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aAnchorTransform,
                        rText.getSdrTextHorzAdjust(),
                        rText.getSdrTextVertAdjust(),
                        rText.isFixedCellHeight(),
                        rText.isScroll(),
                        bCellText,
                        bWordWrap);
                }
            }

            OSL_ENSURE(pNew != nullptr, "createTextPrimitive: no text primitive created (!)");

            if(rText.isBlink())
            {
                // prepare animation and primitive list
                drawinglayer::animation::AnimationEntryList aAnimationList;
                rText.getBlinkTextTiming(aAnimationList);

                if(0.0 != aAnimationList.getDuration())
                {
                    // create content sequence
                    Primitive2DReference xRefA(pNew);
                    Primitive2DContainer aContent { xRefA };

                    // create and add animated switch primitive
                    return Primitive2DReference(new AnimatedBlinkPrimitive2D(aAnimationList, std::move(aContent)));
                }
                else
                {
                    // add to decomposition
                    return Primitive2DReference(pNew);
                }
            }

            if(rText.isScroll())
            {
                // suppress scroll when FontWork
                if(rText.getSdrFormTextAttribute().isDefault())
                {
                    // get scroll direction
                    const SdrTextAniDirection eDirection(rText.getSdrText().GetObject().GetTextAniDirection());
                    const bool bHorizontal(SdrTextAniDirection::Left == eDirection || SdrTextAniDirection::Right == eDirection);

                    // decompose to get separated values for the scroll box
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    aAnchorTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                    // build transform from scaled only to full AnchorTransform and inverse
                    const basegfx::B2DHomMatrix aSRT(basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
                        fShearX, fRotate, aTranslate));
                    basegfx::B2DHomMatrix aISRT(aSRT);
                    aISRT.invert();

                    // bring the primitive back to scaled only and get scaled range, create new clone for this
                    rtl::Reference<SdrTextPrimitive2D> pNew2 = pNew->createTransformedClone(aISRT);
                    OSL_ENSURE(pNew2, "createTextPrimitive: Could not create transformed clone of text primitive (!)");
                    pNew = pNew2.get();

                    // create neutral geometry::ViewInformation2D for local range and decompose calls. This is okay
                    // since the decompose is view-independent
                    geometry::ViewInformation2D aViewInformation2D;

                    // get range
                    const basegfx::B2DRange aScaledRange(pNew->getB2DRange(aViewInformation2D));

                    // create left outside and right outside transformations. Also take care
                    // of the clip rectangle
                    basegfx::B2DHomMatrix aLeft, aRight;
                    basegfx::B2DPoint aClipTopLeft(0.0, 0.0);
                    basegfx::B2DPoint aClipBottomRight(aScale.getX(), aScale.getY());

                    if(bHorizontal)
                    {
                        aClipTopLeft.setY(aScaledRange.getMinY());
                        aClipBottomRight.setY(aScaledRange.getMaxY());
                        aLeft.translate(-aScaledRange.getMaxX(), 0.0);
                        aRight.translate(aScale.getX() - aScaledRange.getMinX(), 0.0);
                    }
                    else
                    {
                        aClipTopLeft.setX(aScaledRange.getMinX());
                        aClipBottomRight.setX(aScaledRange.getMaxX());
                        aLeft.translate(0.0, -aScaledRange.getMaxY());
                        aRight.translate(0.0, aScale.getY() - aScaledRange.getMinY());
                    }

                    aLeft *= aSRT;
                    aRight *= aSRT;

                    // prepare animation list
                    drawinglayer::animation::AnimationEntryList aAnimationList;

                    if(bHorizontal)
                    {
                        rText.getScrollTextTiming(aAnimationList, aScale.getX(), aScaledRange.getWidth());
                    }
                    else
                    {
                        rText.getScrollTextTiming(aAnimationList, aScale.getY(), aScaledRange.getHeight());
                    }

                    if(0.0 != aAnimationList.getDuration())
                    {
                        // create a new Primitive2DContainer containing the animated text in its scaled only state.
                        // use the decomposition to force to simple text primitives, those will no longer
                        // need the outliner for formatting (alternatively it is also possible to just add
                        // pNew to aNewPrimitiveSequence)
                        Primitive2DContainer aAnimSequence;
                        pNew->get2DDecomposition(aAnimSequence, aViewInformation2D);
                        pNew.clear();

                        // create a new animatedInterpolatePrimitive and add it
                        Primitive2DReference xRefA(new AnimatedInterpolatePrimitive2D({ aLeft, aRight }, aAnimationList, std::move(aAnimSequence)));
                        Primitive2DContainer aContent { xRefA };

                        // scrolling needs an encapsulating clipping primitive
                        const basegfx::B2DRange aClipRange(aClipTopLeft, aClipBottomRight);
                        basegfx::B2DPolygon aClipPolygon(basegfx::utils::createPolygonFromRect(aClipRange));
                        aClipPolygon.transform(aSRT);
                        return Primitive2DReference(new MaskPrimitive2D(basegfx::B2DPolyPolygon(aClipPolygon), std::move(aContent)));
                    }
                    else
                    {
                        // add to decomposition
                        return Primitive2DReference(pNew);
                    }
                }
            }

            if(rText.isInEditMode())
            {
                // #i97628#
                // encapsulate with TextHierarchyEditPrimitive2D to allow renderers
                // to suppress actively edited content if needed
                Primitive2DReference xRefA(pNew);
                Primitive2DContainer aContent { xRefA };

                // create and add TextHierarchyEditPrimitive2D primitive
                return Primitive2DReference(new TextHierarchyEditPrimitive2D(std::move(aContent)));
            }
            else
            {
                // add to decomposition
                return pNew;
            }
        }

        Primitive2DContainer createEmbeddedShadowPrimitive(
            Primitive2DContainer&& rContent,
            const attribute::SdrShadowAttribute& rShadow,
            const basegfx::B2DHomMatrix& rObjectMatrix,
            const Primitive2DContainer* pContentForShadow)
        {
            if(rContent.empty())
                return std::move(rContent);

            basegfx::B2DHomMatrix aShadowOffset;

            if(rShadow.getSize().getX() != 100000)
            {
                basegfx::B2DTuple aScale;
                basegfx::B2DTuple aTranslate;
                double fRotate = 0;
                double fShearX = 0;
                rObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);
                // Scale the shadow
                double nTranslateX = aTranslate.getX();
                double nTranslateY = aTranslate.getY();

                // The origin for scaling is the top left corner by default. A negative
                // shadow offset changes the origin.
                if (rShadow.getOffset().getX() < 0)
                    nTranslateX += aScale.getX();
                if (rShadow.getOffset().getY() < 0)
                    nTranslateY += aScale.getY();

                aShadowOffset.translate(-nTranslateX, -nTranslateY);
                aShadowOffset.scale(rShadow.getSize().getX() * 0.00001, rShadow.getSize().getY() * 0.00001);
                aShadowOffset.translate(nTranslateX, nTranslateY);
            }

            aShadowOffset.translate(rShadow.getOffset().getX(), rShadow.getOffset().getY());

            // create shadow primitive and add content
            const Primitive2DContainer& rContentForShadow
                    = pContentForShadow ? *pContentForShadow : rContent;
            int nContentWithTransparence = std::count_if(
                rContentForShadow.begin(), rContentForShadow.end(),
                [](const Primitive2DReference& xChild) {
                    auto pChild = dynamic_cast<BufferedDecompositionPrimitive2D*>(xChild.get());
                    return pChild && pChild->getTransparenceForShadow() != 0;
                });
            if (nContentWithTransparence == 0)
            {
                Primitive2DContainer aRetval(2);
                aRetval[0] = Primitive2DReference(
                    new ShadowPrimitive2D(
                        aShadowOffset,
                        rShadow.getColor(),
                        rShadow.getBlur(),
                        Primitive2DContainer(pContentForShadow ? *pContentForShadow : rContent)));

                if (0.0 != rShadow.getTransparence())
                {
                    // create SimpleTransparencePrimitive2D
                    Primitive2DContainer aTempContent{ aRetval[0] };

                    aRetval[0] = Primitive2DReference(
                        new UnifiedTransparencePrimitive2D(
                            std::move(aTempContent),
                            rShadow.getTransparence()));
                }

                aRetval[1] = Primitive2DReference(new GroupPrimitive2D(std::move(rContent)));
                return aRetval;
            }

            Primitive2DContainer aRetval;
            for (const auto& xChild : rContentForShadow)
            {
                double fChildTransparence = 0.0;
                auto pChild = dynamic_cast<BufferedDecompositionPrimitive2D*>(xChild.get());
                if (pChild)
                {
                        fChildTransparence = pChild->getTransparenceForShadow();
                        fChildTransparence /= 100;
                }
                aRetval.push_back(Primitive2DReference(
                    new ShadowPrimitive2D(aShadowOffset, rShadow.getColor(), rShadow.getBlur(),
                                            Primitive2DContainer({ xChild }))));
                if (rShadow.getTransparence() != 0.0 || fChildTransparence != 0.0)
                {
                    Primitive2DContainer aTempContent{ aRetval.back() };

                    double fChildAlpha = 1.0 - fChildTransparence;
                    double fShadowAlpha = 1.0 - rShadow.getTransparence();
                    double fTransparence = 1.0 - fChildAlpha * fShadowAlpha;
                    aRetval.back() = Primitive2DReference(new UnifiedTransparencePrimitive2D(
                            std::move(aTempContent), fTransparence));
                }
            }

            aRetval.push_back(
                    Primitive2DReference(new GroupPrimitive2D(std::move(rContent))));
            return aRetval;
        }

        Primitive2DContainer createEmbeddedGlowPrimitive(
            Primitive2DContainer&& rContent,
            const attribute::SdrGlowAttribute& rGlow)
        {
            if(rContent.empty())
                return std::move(rContent);
            Primitive2DContainer aRetval(2);
            aRetval[0] = Primitive2DReference(
                new GlowPrimitive2D(rGlow.getColor(), rGlow.getRadius(), Primitive2DContainer(rContent)));
            aRetval[1] = Primitive2DReference(new GroupPrimitive2D(Primitive2DContainer(rContent)));
            return aRetval;
        }

        Primitive2DContainer createEmbeddedSoftEdgePrimitive(Primitive2DContainer&& aContent,
                                                             sal_Int32 nRadius)
        {
            if (aContent.empty() || !nRadius)
                return std::move(aContent);
            Primitive2DContainer aRetval(1);
            aRetval[0] = Primitive2DReference(new SoftEdgePrimitive2D(nRadius, std::move(aContent)));
            return aRetval;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
