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

#include "precompiled_sd.hxx"

#include "view/SlsPageObjectViewObjectContact.hxx"

#include "controller/SlsProperties.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectViewContact.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsFontProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "res_bmp.hrc"
#include "tools/IconCache.hxx"
#include "PreviewRenderer.hxx"

#include "sdpage.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "drawdoc.hxx"
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svdoutl.hxx>
#include <svx/sdrpagewindow.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/lineinfo.hxx>
#include <tools/color.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

using namespace ::sdr::contact;
using namespace ::sd::slidesorter::model;

using drawinglayer::primitive2d::Primitive2DReference;
using drawinglayer::primitive2d::Primitive2DSequence;

namespace sd { namespace slidesorter { namespace view {


const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorOffset = 2;
const sal_Int32 PageObjectViewObjectContact::mnSelectionIndicatorThickness = 3;
const sal_Int32 PageObjectViewObjectContact::mnFocusIndicatorOffset = 3;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorOffset = 9;
const sal_Int32 PageObjectViewObjectContact::mnFadeEffectIndicatorSize = 14;
const sal_Int32 PageObjectViewObjectContact::mnPageNumberOffset = 9;
const sal_Int32 PageObjectViewObjectContact::mnMouseOverEffectOffset = 3;
const sal_Int32 PageObjectViewObjectContact::mnMouseOverEffectThickness = 1;

PageObjectViewObjectContact::PageObjectViewObjectContact (
    ObjectContact& rObjectContact,
    ViewContact& rViewContact,
    const ::boost::shared_ptr<cache::PageCache>& rpCache,
    const ::boost::shared_ptr<controller::Properties>& rpProperties)
    : ViewObjectContactOfPageObj(rObjectContact, rViewContact),
      mbInDestructor(false),
      mxCurrentPageContents(),
      mpCache(rpCache),
      mpProperties(rpProperties)
{
    SharedPageDescriptor pDescriptor (GetPageDescriptor());
    OSL_ASSERT(pDescriptor.get()!=NULL);
    if (pDescriptor.get() != NULL)
        pDescriptor->SetViewObjectContact(this);
}




PageObjectViewObjectContact::~PageObjectViewObjectContact (void)
{
    mbInDestructor = true;

    GetPageDescriptor()->SetViewObjectContact(NULL);

    if (mpCache.get() != NULL)
    {
        const SdrPage* pPage = GetPage();

        if(pPage)
        {
            mpCache->ReleasePreviewBitmap(GetPage());
        }
    }
}




void PageObjectViewObjectContact::SetCache (const ::boost::shared_ptr<cache::PageCache>& rpCache)
{
    mpCache = rpCache;
}




Rectangle PageObjectViewObjectContact::GetBoundingBox (
    OutputDevice& rDevice,
    BoundingBoxType eType,
    CoordinateSystem eCoordinateSystem) const
{
    // Most of the bounding boxes are based on the bounding box of the preview.
    // SdrPageObj is a SdrObject, so use SdrObject::aOutRect as model data
    const PageObjectViewContact& rPaObVOC(static_cast<PageObjectViewContact&>(GetViewContact()));
    Rectangle aBoundingBox(rPaObVOC.GetPageObject().GetLastBoundRect());

    CoordinateSystem eCurrentCoordinateSystem (ModelCoordinateSystem);
    switch(eType)
    {
        case PageObjectBoundingBox:
        {
            const SvBorder aPageDescriptorBorder(GetPageDescriptor()->GetModelBorder());
            aBoundingBox.Left() -= aPageDescriptorBorder.Left();
            aBoundingBox.Top() -= aPageDescriptorBorder.Top();
            aBoundingBox.Right() += aPageDescriptorBorder.Right();
            aBoundingBox.Bottom() += aPageDescriptorBorder.Bottom();
            break;
        }
        case PreviewBoundingBox:
        {
            // The aBoundingBox already has the right value.
            break;
        }
        case MouseOverIndicatorBoundingBox:
        {
            const sal_Int32 nBorderWidth (mnMouseOverEffectOffset+mnMouseOverEffectThickness);
            const Size aBorderSize (rDevice.PixelToLogic(Size(nBorderWidth,nBorderWidth)));
            aBoundingBox.Left() -= aBorderSize.Width();
            aBoundingBox.Top() -= aBorderSize.Height();
            aBoundingBox.Right() += aBorderSize.Width();
            aBoundingBox.Bottom() += aBorderSize.Height();
            break;
        }
        case FocusIndicatorBoundingBox:
        {
            const sal_Int32 nBorderWidth (mnFocusIndicatorOffset+1);
            const Size aBorderSize (rDevice.PixelToLogic(Size(nBorderWidth,nBorderWidth)));
            aBoundingBox.Left() -= aBorderSize.Width();
            aBoundingBox.Top() -= aBorderSize.Height();
            aBoundingBox.Right() += aBorderSize.Width();
            aBoundingBox.Bottom() += aBorderSize.Height();
            break;
        }
        case SelectionIndicatorBoundingBox:
        {
            const sal_Int32 nBorderWidth(mnSelectionIndicatorOffset+mnSelectionIndicatorThickness);
            const Size aBorderSize (rDevice.PixelToLogic(Size(nBorderWidth,nBorderWidth)));
            aBoundingBox.Left() -= aBorderSize.Width();
            aBoundingBox.Top() -= aBorderSize.Height();
            aBoundingBox.Right() += aBorderSize.Width();
            aBoundingBox.Bottom() += aBorderSize.Height();
            break;
        }
        case PageNumberBoundingBox:
        {
            Size aModelOffset = rDevice.PixelToLogic(Size(mnPageNumberOffset,mnPageNumberOffset));
            Size aNumberSize (GetPageDescriptor()->GetPageNumberAreaModelSize());
            aBoundingBox = Rectangle (
                Point (
                    aBoundingBox.Left() - aModelOffset.Width() - aNumberSize.Width(),
                    aBoundingBox.Top()),
                aNumberSize);
            break;
        }

        case NameBoundingBox:
            break;

        case FadeEffectIndicatorBoundingBox:
            Size aModelOffset = rDevice.PixelToLogic(Size (0, mnFadeEffectIndicatorOffset));
            // Flush left just outside the selection rectangle.
            aBoundingBox = Rectangle (
                Point (
                    aBoundingBox.Left(),
                    aBoundingBox.Bottom() + aModelOffset.Height()
                    ),
                rDevice.PixelToLogic (
                    IconCache::Instance().GetIcon(BMP_FADE_EFFECT_INDICATOR).GetSizePixel())
                );
            break;
    }

    // Make sure the bounding box uses the requested coordinate system.
    if (eCurrentCoordinateSystem != eCoordinateSystem)
    {
        if (eCoordinateSystem == ModelCoordinateSystem)
            aBoundingBox = Rectangle(
                rDevice.PixelToLogic(aBoundingBox.TopLeft()),
                rDevice.PixelToLogic(aBoundingBox.GetSize()));
        else
            aBoundingBox = Rectangle(
                rDevice.LogicToPixel(aBoundingBox.TopLeft()),
                rDevice.LogicToPixel(aBoundingBox.GetSize()));
    }
    return aBoundingBox;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// example implementation for primitive usage for PageObjectViewObjectContact

} } } // end of namespace ::sd::slidesorter::view

#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <sd_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/sdr/contact/objectcontact.hxx>

namespace sd { namespace slidesorter { namespace view {

///////////////////////////////////////////////////////////////////////////////////////////////
// All primitives for SdrPageObject visualisation are based on one range which describes
// the size of the inner rectangle for PagePreview visualisation. Use a common implementation
// class for all derived SdPageObjectPrimitives. The SdPageObjectBasePrimitive itself
// is pure virtual

class SdPageObjectBasePrimitive : public drawinglayer::primitive2d::BufferedDecompositionPrimitive2D
{
private:
    // the inner range of the SdPageObject visualisation
    basegfx::B2DRange                   maRange;

public:
    // constructor and destructor
    SdPageObjectBasePrimitive(const basegfx::B2DRange& rRange);
    virtual ~SdPageObjectBasePrimitive();

    // data access
    const basegfx::B2DRange& getPageObjectRange() const { return maRange; }

    // compare operator
    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;
};

SdPageObjectBasePrimitive::SdPageObjectBasePrimitive(const basegfx::B2DRange& rRange)
:   drawinglayer::primitive2d::BufferedDecompositionPrimitive2D(),
    maRange(rRange)
{
}

SdPageObjectBasePrimitive::~SdPageObjectBasePrimitive()
{
}

bool SdPageObjectBasePrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(drawinglayer::primitive2d::BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const SdPageObjectBasePrimitive& rCompare = static_cast< const SdPageObjectBasePrimitive& >(rPrimitive);
        return (getPageObjectRange() == rCompare.getPageObjectRange());
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// SdPageObjectPrimitive for selected visualisation

class SdPageObjectPageBitmapPrimitive : public SdPageObjectBasePrimitive
{
private:
    // the bitmap containing the PagePreview
    BitmapEx                            maBitmapEx;

protected:
    // method which is to be used to implement the local decomposition of a 2D primitive.
    virtual Primitive2DSequence create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    // constructor and destructor
    SdPageObjectPageBitmapPrimitive(
        const basegfx::B2DRange& rRange,
        const BitmapEx& rBitmapEx);
    ~SdPageObjectPageBitmapPrimitive();

    // data access
    const BitmapEx& getBitmapEx() const { return maBitmapEx; }

    // compare operator
    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;

    // provide unique ID
    DeclPrimitrive2DIDBlock()
};

Primitive2DSequence SdPageObjectPageBitmapPrimitive::create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const
{
    // add bitmap primitive
    // to avoid scaling, use the Bitmap pixel size as primitive size
    basegfx::B2DHomMatrix aBitmapTransform;
    const Size aBitmapSize(getBitmapEx().GetSizePixel());
    const basegfx::B2DVector aBitmapSizeLogic(rViewInformation.getInverseObjectToViewTransformation() *
        basegfx::B2DVector(aBitmapSize.getWidth() - 1, aBitmapSize.getHeight() - 1));

    // short form for scale and translate transformation
    aBitmapTransform.set(0L, 0L, aBitmapSizeLogic.getX());
    aBitmapTransform.set(1L, 1L, aBitmapSizeLogic.getY());
    aBitmapTransform.set(0L, 2L, getPageObjectRange().getMinX());
    aBitmapTransform.set(1L, 2L, getPageObjectRange().getMinY());

    // add a BitmapPrimitive2D to the result
    const Primitive2DReference xReference(
        new drawinglayer::primitive2d::BitmapPrimitive2D(getBitmapEx(), aBitmapTransform));
    return Primitive2DSequence(&xReference, 1);
}

SdPageObjectPageBitmapPrimitive::SdPageObjectPageBitmapPrimitive(
    const basegfx::B2DRange& rRange,
    const BitmapEx& rBitmapEx)
:   SdPageObjectBasePrimitive(rRange),
    maBitmapEx(rBitmapEx)
{
}

SdPageObjectPageBitmapPrimitive::~SdPageObjectPageBitmapPrimitive()
{
}

bool SdPageObjectPageBitmapPrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(SdPageObjectBasePrimitive::operator==(rPrimitive))
    {
        const SdPageObjectPageBitmapPrimitive& rCompare = static_cast< const SdPageObjectPageBitmapPrimitive& >(rPrimitive);
        return (getBitmapEx() == rCompare.getBitmapEx());
    }

    return false;
}

ImplPrimitrive2DIDBlock(SdPageObjectPageBitmapPrimitive, PRIMITIVE2D_ID_SDPAGEOBJECTPAGEBITMAPPRIMITIVE)

///////////////////////////////////////////////////////////////////////////////////////////////
// SdPageObjectPrimitive for selected visualisation

class SdPageObjectSelectPrimitive : public SdPageObjectBasePrimitive
{
private:
    /// Gap between border of page object and inside of selection rectangle.
    static const sal_Int32 mnSelectionIndicatorOffset;

    /// Thickness of the selection rectangle.
    static const sal_Int32 mnSelectionIndicatorThickness;

protected:
    // method which is to be used to implement the local decomposition of a 2D primitive.
    virtual Primitive2DSequence create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    // constructor and destructor
    SdPageObjectSelectPrimitive(const basegfx::B2DRange& rRange);
    ~SdPageObjectSelectPrimitive();

    // provide unique ID
    DeclPrimitrive2DIDBlock()
};

const sal_Int32 SdPageObjectSelectPrimitive::mnSelectionIndicatorOffset(1);
const sal_Int32 SdPageObjectSelectPrimitive::mnSelectionIndicatorThickness(3);

Primitive2DSequence SdPageObjectSelectPrimitive::create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const
{
    Primitive2DSequence xRetval(2);

    // since old Width/Height calculations always added a single pixel value,
    // it is necessary to create a inner range which is one display unit less
    // at the bottom right.
    const basegfx::B2DVector aDiscretePixel(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
    const basegfx::B2DRange aAdaptedInnerRange(
        getPageObjectRange().getMinX(), getPageObjectRange().getMinY(),
        getPageObjectRange().getMaxX() - aDiscretePixel.getX(), getPageObjectRange().getMaxY() - aDiscretePixel.getY());

    // PaintSelectionIndicator replacement. Grow by offset first
    basegfx::B2DRange aDiscreteOuterRange(aAdaptedInnerRange);
    aDiscreteOuterRange.grow(mnSelectionIndicatorOffset * aDiscretePixel.getX());

    // Remember inner border. Make it one bigger in top left since polygons
    // do not paint their lower-right corners. Since this is the inner polygon,
    // the top-left corders are the ones to grow here
    const basegfx::B2DRange aDiscreteInnerRange(
        aDiscreteOuterRange.getMinimum() + aDiscretePixel,
        aDiscreteOuterRange.getMaximum() - aDiscretePixel);

    // grow by line width
    aDiscreteOuterRange.grow((mnSelectionIndicatorThickness - 1) * aDiscretePixel.getX());

    // create a PolyPolygon from those ranges. For the outer polygon, round edges by
    // giving a relative radius to the polygon creator (use mnSelectionIndicatorThickness here, too)
    const double fPixelFactor(aDiscretePixel.getX() * (mnSelectionIndicatorThickness + 2.5));
    const double fRelativeRadiusX(fPixelFactor / ::std::max(aDiscreteOuterRange.getWidth(), 1.0));
    const double fRelativeRadiusY(fPixelFactor / ::std::max(aDiscreteOuterRange.getHeight(), 1.0));
    basegfx::B2DPolyPolygon aFramePolyPolygon;
    const basegfx::B2DPolygon aRoundedOuterPolygon(basegfx::tools::createPolygonFromRect(aDiscreteOuterRange, fRelativeRadiusX, fRelativeRadiusY));

    aFramePolyPolygon.append(aRoundedOuterPolygon);
    aFramePolyPolygon.append(basegfx::tools::createPolygonFromRect(aDiscreteInnerRange));

    // add colored PolyPolygon
    const svtools::ColorConfig aColorConfig;
    const basegfx::BColor aFrameColor(Application::GetSettings().GetStyleSettings().GetMenuHighlightColor().getBColor());

    xRetval[0] = Primitive2DReference(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(aFramePolyPolygon, aFrameColor));

    // add aRoundedOuterPolygon again as non-filled line polygon to get the roundungs
    // painted correctly
    xRetval[1] = Primitive2DReference(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aRoundedOuterPolygon, aFrameColor));

    return xRetval;
}

SdPageObjectSelectPrimitive::SdPageObjectSelectPrimitive(const basegfx::B2DRange& rRange)
:   SdPageObjectBasePrimitive(rRange)
{
}

SdPageObjectSelectPrimitive::~SdPageObjectSelectPrimitive()
{
}

ImplPrimitrive2DIDBlock(SdPageObjectSelectPrimitive, PRIMITIVE2D_ID_SDPAGEOBJECTSELECTPRIMITIVE)

///////////////////////////////////////////////////////////////////////////////////////////////
// SdPageObjectPrimitive for border around bitmap visualisation

class SdPageObjectBorderPrimitive : public SdPageObjectBasePrimitive
{
protected:
    // method which is to be used to implement the local decomposition of a 2D primitive.
    virtual Primitive2DSequence create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    // constructor and destructor
    SdPageObjectBorderPrimitive(const basegfx::B2DRange& rRange);
    ~SdPageObjectBorderPrimitive();

    // provide unique ID
    DeclPrimitrive2DIDBlock()
};

Primitive2DSequence SdPageObjectBorderPrimitive::create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const
{
    // since old Width/Height calculations always added a single pixel value,
    // it is necessary to create a inner range which is one display unit less
    // at the bottom right.
    const basegfx::B2DVector aDiscretePixel(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
    const basegfx::B2DRange aAdaptedInnerRange(
        getPageObjectRange().getMinX(), getPageObjectRange().getMinY(),
        getPageObjectRange().getMaxX() - aDiscretePixel.getX(), getPageObjectRange().getMaxY() - aDiscretePixel.getY());

    // Paint_Border replacement. (use aBorderColor)
    const svtools::ColorConfig aColorConfig;
    const basegfx::BColor aBorderColor(Color(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor).getBColor());

    const Primitive2DReference xReference(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(basegfx::tools::createPolygonFromRect(aAdaptedInnerRange), aBorderColor));
    return Primitive2DSequence(&xReference, 1);
}

SdPageObjectBorderPrimitive::SdPageObjectBorderPrimitive(const basegfx::B2DRange& rRange)
:   SdPageObjectBasePrimitive(rRange)
{
}

SdPageObjectBorderPrimitive::~SdPageObjectBorderPrimitive()
{
}

ImplPrimitrive2DIDBlock(SdPageObjectBorderPrimitive, PRIMITIVE2D_ID_SDPAGEOBJECTBORDERPRIMITIVE)

///////////////////////////////////////////////////////////////////////////////////////////////
// SdPageObjectPrimitive for focus visualisation

class SdPageObjectFocusPrimitive : public SdPageObjectBasePrimitive
{
private:
    /// Gap between border of page object and inside of focus rectangle.
    static const sal_Int32 mnFocusIndicatorOffset;
    const bool mbContrastToSelected;

protected:
    // method which is to be used to implement the local decomposition of a 2D primitive.
    virtual Primitive2DSequence create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    // constructor and destructor
    SdPageObjectFocusPrimitive(const basegfx::B2DRange& rRange, const bool bContrast);
    ~SdPageObjectFocusPrimitive();

    // provide unique ID
    DeclPrimitrive2DIDBlock()
};

const sal_Int32 SdPageObjectFocusPrimitive::mnFocusIndicatorOffset(2);

Primitive2DSequence SdPageObjectFocusPrimitive::create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const
{
    Primitive2DSequence xRetval(2);

    // since old Width/Height calculations always added a single pixel value,
    // it is necessary to create a inner range which is one display unit less
    // at the bottom right.
    const basegfx::B2DVector aDiscretePixel(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
    const basegfx::B2DRange aAdaptedInnerRange(
        getPageObjectRange().getMinX(), getPageObjectRange().getMinY(),
        getPageObjectRange().getMaxX() - aDiscretePixel.getX(), getPageObjectRange().getMaxY() - aDiscretePixel.getY());

    // Paint_FocusIndicator replacement. (black and white).
    // imitate Paint_DottedRectangle: First paint a white rectangle and above it a black dotted one
    basegfx::B2DRange aFocusIndicatorRange(aAdaptedInnerRange);
    aFocusIndicatorRange.grow(mnFocusIndicatorOffset * aDiscretePixel.getX());

    // create polygon
    const basegfx::B2DPolygon aIndicatorPolygon(basegfx::tools::createPolygonFromRect(aFocusIndicatorRange));

    const StyleSettings& rStyleSettings(Application::GetSettings().GetStyleSettings());

    // "background" rectangle
    const Color aBackgroundColor(mbContrastToSelected ? rStyleSettings.GetMenuHighlightColor() : rStyleSettings.GetWindowColor());
    xRetval[0] = Primitive2DReference(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aIndicatorPolygon, Color(COL_WHITE).getBColor()));

    // dotted black rectangle with same geometry
    ::std::vector< double > aDotDashArray;

    const sal_Int32 nFocusIndicatorWidth (3);
    aDotDashArray.push_back(nFocusIndicatorWidth *aDiscretePixel.getX());
    aDotDashArray.push_back(nFocusIndicatorWidth * aDiscretePixel.getX());

    // prepare line and stroke attributes
    const Color aLineColor(mbContrastToSelected ? rStyleSettings.GetMenuHighlightTextColor() : rStyleSettings.GetWindowTextColor());
    const drawinglayer::attribute::LineAttribute aLineAttribute(aLineColor.getBColor());
    const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(
        aDotDashArray, 2.0 * nFocusIndicatorWidth * aDiscretePixel.getX());


    xRetval[1] = Primitive2DReference(
        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(aIndicatorPolygon, aLineAttribute, aStrokeAttribute));

    return xRetval;
}

SdPageObjectFocusPrimitive::SdPageObjectFocusPrimitive(const basegfx::B2DRange& rRange, const bool bContrast)
    :   SdPageObjectBasePrimitive(rRange),
        mbContrastToSelected(bContrast)
{
}

SdPageObjectFocusPrimitive::~SdPageObjectFocusPrimitive()
{
}

ImplPrimitrive2DIDBlock(SdPageObjectFocusPrimitive, PRIMITIVE2D_ID_SDPAGEOBJECTFOCUSPRIMITIVE)

///////////////////////////////////////////////////////////////////////////////////////////////
// SdPageObjectPrimitive for fade effect visualisation

class SdPageObjectFadeNameNumberPrimitive : public SdPageObjectBasePrimitive
{
private:
    /// Size of width and height of the fade effect indicator in pixels.
    static const sal_Int32              mnFadeEffectIndicatorOffset;

    /// Size of width and height of the comments indicator in pixels.
    static const sal_Int32              mnCommentsIndicatorOffset;

    /// Gap between border of page object and number rectangle.
    static const sal_Int32              mnPageNumberOffset;

    /// the indicator bitmaps. Static since it is usable outside this primitive
    /// for size comparisons
    static BitmapEx*                    mpFadeEffectIconBitmap;
    static BitmapEx*                    mpCommentsIconBitmap;

    /// page name, number and needed infos
    String                              maPageName;
    sal_uInt32                          mnPageNumber;
    Font                                maPageNameFont;
    Size                                maPageNumberAreaModelSize;

    // bitfield
    bool mbShowFadeEffectIcon : 1;
    bool mbShowCommentsIcon : 1;
    bool mbExcluded : 1;

    // private helpers
    const BitmapEx& getFadeEffectIconBitmap() const;

protected:
    // method which is to be used to implement the local decomposition of a 2D primitive.
    virtual Primitive2DSequence create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    // constructor and destructor
    SdPageObjectFadeNameNumberPrimitive(
        const basegfx::B2DRange& rRange,
        const String& rPageName,
        sal_uInt32 nPageNumber,
        const Font& rPageNameFont,
        const Size& rPageNumberAreaModelSize,
        bool bShowFadeEffectIcon,
        bool bShowCommentsIcon,
        bool bExcluded);
    ~SdPageObjectFadeNameNumberPrimitive();

    // data access
    const String& getPageName() const { return maPageName; }
    sal_uInt32 getPageNumber() const { return mnPageNumber; }
    const Font& getPageNameFont() const { return maPageNameFont; }
    const Size& getPageNumberAreaModelSize() const { return maPageNumberAreaModelSize; }
    bool getShowFadeEffectIcon() const { return mbShowFadeEffectIcon; }
    bool getShowCommentsIcon() const { return mbShowCommentsIcon; }
    bool getExcluded() const { return mbExcluded; }

    // compare operator
    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;

    // provide unique ID
    DeclPrimitrive2DIDBlock()
};

const sal_Int32 SdPageObjectFadeNameNumberPrimitive::mnFadeEffectIndicatorOffset(9);
const sal_Int32 SdPageObjectFadeNameNumberPrimitive::mnPageNumberOffset(9);
BitmapEx* SdPageObjectFadeNameNumberPrimitive::mpFadeEffectIconBitmap = 0;

const BitmapEx& SdPageObjectFadeNameNumberPrimitive::getFadeEffectIconBitmap() const
{
    if(mpFadeEffectIconBitmap == NULL)
    {
        // prepare FadeEffectIconBitmap on demand
        const sal_uInt16 nIconId(BMP_FADE_EFFECT_INDICATOR);
        const BitmapEx aFadeEffectIconBitmap(IconCache::Instance().GetIcon(nIconId).GetBitmapEx());
        const_cast< SdPageObjectFadeNameNumberPrimitive* >(this)->mpFadeEffectIconBitmap = new BitmapEx(aFadeEffectIconBitmap);
    }

    return *mpFadeEffectIconBitmap;
}


const sal_Int32 SdPageObjectFadeNameNumberPrimitive::mnCommentsIndicatorOffset(9);
BitmapEx* SdPageObjectFadeNameNumberPrimitive::mpCommentsIconBitmap = 0;

Primitive2DSequence SdPageObjectFadeNameNumberPrimitive::create2DDecomposition(const drawinglayer::geometry::ViewInformation2D& rViewInformation) const
{
    const xub_StrLen nTextLength(getPageName().Len());
    const sal_uInt32 nCount(
        (getShowFadeEffectIcon() ? 1 : 0) +     // FadeEffect icon
        (nTextLength ? 1 : 0) +                 // PageName
        1 +                                     // PageNumber (always)
        (getExcluded() ? 2 : 0)                 // PageNumber crossed out
        );
    sal_uInt32 nInsert(0);
    Primitive2DSequence xRetval(nCount);

    // since old Width/Height calculations always added a single pixel value,
    // it is necessary to create a inner range which is one display unit less
    // at the bottom right.
    const basegfx::B2DVector aDiscretePixel(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
    const basegfx::B2DRange aAdaptedInnerRange(
        getPageObjectRange().getMinX(), getPageObjectRange().getMinY(),
        getPageObjectRange().getMaxX() - aDiscretePixel.getX(), getPageObjectRange().getMaxY() - aDiscretePixel.getY());

    // preapre TextLayouter
    drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
    aTextLayouter.setFont(getPageNameFont());

    // get font attributes
    basegfx::B2DVector aTextSizeAttribute;
    const drawinglayer::attribute::FontAttribute aFontAttribute(
        drawinglayer::primitive2d::getFontAttributeFromVclFont(
            aTextSizeAttribute,
            getPageNameFont(),
            false,
            false));

    // prepare locale; this may need some more information in the future
    const ::com::sun::star::lang::Locale aLocale;

    // prepare font color from System
    const basegfx::BColor aFontColor(Application::GetSettings().GetStyleSettings().GetFontColor().getBColor());

    if(getShowFadeEffectIcon())
    {
        // prepare fFadeEffect Sizes
        const basegfx::B2DVector aFadeEffectBitmapSizeLogic(rViewInformation.getInverseObjectToViewTransformation() *
            basegfx::B2DVector(
                getFadeEffectIconBitmap().GetSizePixel().getWidth() - 1,
                getFadeEffectIconBitmap().GetSizePixel().getHeight() - 1));

        // Paint_FadeEffectIndicator replacement.
        // create transformation. To avoid bitmap scaling, use bitmap size as size
        basegfx::B2DHomMatrix aBitmapTransform;

        // short form for scale and translate transformation
        aBitmapTransform.set(0L, 0L, aFadeEffectBitmapSizeLogic.getX());
        aBitmapTransform.set(1L, 1L, aFadeEffectBitmapSizeLogic.getY());
        aBitmapTransform.set(0L, 2L, aAdaptedInnerRange.getMinX());
        aBitmapTransform.set(1L, 2L, aAdaptedInnerRange.getMaxY() + ((mnFadeEffectIndicatorOffset + 1) * aDiscretePixel.getX()));

        xRetval[nInsert++] = Primitive2DReference(
            new drawinglayer::primitive2d::BitmapPrimitive2D(getFadeEffectIconBitmap(), aBitmapTransform));
    }

    if(nTextLength)
    {
        // prepare fFadeEffect Sizes since it consumes from text size
        const basegfx::B2DVector aFadeEffectBitmapSizeLogic(rViewInformation.getInverseObjectToViewTransformation() *
            basegfx::B2DVector(
                getFadeEffectIconBitmap().GetSizePixel().getWidth() - 1,
                getFadeEffectIconBitmap().GetSizePixel().getHeight() - 1));

        // Paint_PageName replacement. Get text size
        const double fTextWidth(aTextLayouter.getTextWidth(getPageName(), 0, nTextLength));
        const double fTextHeight(getPageNameFont().GetHeight());
        const double fFadeEffectWidth(aFadeEffectBitmapSizeLogic.getX() * 2.0);
        const double fFadeEffectTextGap(((mnFadeEffectIndicatorOffset + 2) * aDiscretePixel.getX()));
        String aPageName(getPageName());

        // calculate text start position
        double fStartX(
            aAdaptedInnerRange.getMaxX()
            - fTextWidth
            + (aDiscretePixel.getX() * 3.0));
        const double fStartY(
            aAdaptedInnerRange.getMaxY()
            + fTextHeight
            + fFadeEffectTextGap);
        const bool bNeedClipping(fStartX < aAdaptedInnerRange.getMinX() + fFadeEffectWidth);

        // if text is too big, clip it
        if(bNeedClipping)
        {
            // new left start
            fStartX = aAdaptedInnerRange.getMinX() + fFadeEffectWidth;

            // find out how many characters to use
            const double fAvailableLength(aAdaptedInnerRange.getWidth() - fFadeEffectWidth);
            static const String aThreePoints(String::CreateFromAscii("..."));
            const double fWidthThreePoints(aTextLayouter.getTextWidth(aThreePoints, 0, aThreePoints.Len()));
            xub_StrLen a(1);

            for(; a < (xub_StrLen)nTextLength; a++)
            {
                const double fSnippetLength(aTextLayouter.getTextWidth(aPageName, 0, a));

                if(fSnippetLength + fWidthThreePoints > fAvailableLength)
                {
                    break;
                }
            }

            // build new string
            aPageName = String(aPageName, 0, a - 1);
            aPageName += aThreePoints;
        }

        // fill text matrix
        basegfx::B2DHomMatrix aTextMatrix;

        aTextMatrix.set(0, 0, aTextSizeAttribute.getX());
        aTextMatrix.set(1, 1, aTextSizeAttribute.getY());
        aTextMatrix.set(0, 2, fStartX);
        aTextMatrix.set(1, 2, fStartY);

        // prepare DXTextArray (can be empty one)
        const ::std::vector< double > aDXArray;

        // create Text primitive and add to target
        xRetval[nInsert++] = Primitive2DReference(
            new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                aTextMatrix,
                aPageName,
                0,
                aPageName.Len(),
                aDXArray,
                aFontAttribute,
                aLocale,
                aFontColor));
    }

    {
        // Paint_PageNumber replacement. Get the range where it shall be centered and prepare the string
        const double fLeft(aAdaptedInnerRange.getMinX() - (mnPageNumberOffset * aDiscretePixel.getX()) - getPageNumberAreaModelSize().Width());
        const double fTop(aAdaptedInnerRange.getMinY());
        const basegfx::B2DRange aNumberRange(fLeft, fTop,
            fLeft + getPageNumberAreaModelSize().Width(), fTop + getPageNumberAreaModelSize().Height());
        const String aPageNumber(String::CreateFromInt32(getPageNumber()));
        const xub_StrLen nNumberLen(aPageNumber.Len());

        // Get text size
        const double fTextWidth(aTextLayouter.getTextWidth(aPageNumber, 0, nNumberLen));
        const double fTextHeight(getPageNameFont().GetHeight());

        // get text start postion
        const double fStartX(aNumberRange.getCenterX() - (fTextWidth / 2.0));
        const double fStartY(aNumberRange.getMinY() + fTextHeight + aDiscretePixel.getX());

        // fill text matrix
        basegfx::B2DHomMatrix aTextMatrix;

        aTextMatrix.set(0, 0, aTextSizeAttribute.getX());
        aTextMatrix.set(1, 1, aTextSizeAttribute.getY());
        aTextMatrix.set(0, 2, fStartX);
        aTextMatrix.set(1, 2, fStartY);

        // prepare DXTextArray (can be empty one)
        const ::std::vector< double > aDXArray;

        // create Text primitive
        xRetval[nInsert++] = Primitive2DReference(
            new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                aTextMatrix,
                aPageNumber,
                0,
                nNumberLen,
                aDXArray,
                aFontAttribute,
                aLocale,
                aFontColor));

        if(getExcluded())
        {
            // create a box with strikethrough from top left to bottom right
            const basegfx::BColor aActiveColor(Application::GetSettings().GetStyleSettings().GetActiveColor().getBColor());
            basegfx::B2DPolygon aStrikethrough;

            aStrikethrough.append(aNumberRange.getMinimum());
            aStrikethrough.append(aNumberRange.getMaximum());

            xRetval[nInsert++] = Primitive2DReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                basegfx::tools::createPolygonFromRect(aNumberRange), aActiveColor));

            xRetval[nInsert++] = Primitive2DReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                aStrikethrough, aActiveColor));
        }
    }

    return xRetval;
}

SdPageObjectFadeNameNumberPrimitive::SdPageObjectFadeNameNumberPrimitive(
    const basegfx::B2DRange& rRange,
    const String& rPageName,
    sal_uInt32 nPageNumber,
    const Font& rPageNameFont,
    const Size& rPageNumberAreaModelSize,
    bool bShowFadeEffectIcon,
    bool bShowCommentsIcon,
    bool bExcluded)
:   SdPageObjectBasePrimitive(rRange),
    maPageName(rPageName),
    mnPageNumber(nPageNumber),
    maPageNameFont(rPageNameFont),
    maPageNumberAreaModelSize(rPageNumberAreaModelSize),
    mbShowFadeEffectIcon(bShowFadeEffectIcon),
    mbShowCommentsIcon(bShowCommentsIcon),
    mbExcluded(bExcluded)
{
}

SdPageObjectFadeNameNumberPrimitive::~SdPageObjectFadeNameNumberPrimitive()
{
}

bool SdPageObjectFadeNameNumberPrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(SdPageObjectBasePrimitive::operator==(rPrimitive))
    {
        const SdPageObjectFadeNameNumberPrimitive& rCompare = static_cast< const SdPageObjectFadeNameNumberPrimitive& >(rPrimitive);

        return (getPageName() == rCompare.getPageName()
            && getPageNumber() == rCompare.getPageNumber()
            && getPageNameFont() == rCompare.getPageNameFont()
            && getPageNumberAreaModelSize() == rCompare.getPageNumberAreaModelSize()
            && getShowFadeEffectIcon() == rCompare.getShowFadeEffectIcon()
            && getExcluded() == rCompare.getExcluded());
    }

    return false;
}

ImplPrimitrive2DIDBlock(SdPageObjectFadeNameNumberPrimitive, PRIMITIVE2D_ID_SDPAGEOBJECTFADENAMENUMBERPRIMITIVE)

///////////////////////////////////////////////////////////////////////////////////////////////
// createPrimitive2DSequence
//
// This method will replace the whole painting mechanism. Task is no longer to paint stuff to an OutDev,
// but to provide the necessary geometrical information using primitives.

Primitive2DSequence PageObjectViewObjectContact::createPrimitive2DSequence(const sdr::contact::DisplayInfo& rDisplayInfo) const
{
    // OutputDevice* pDevice = rDisplayInfo.GetDIOutputDevice();
    OutputDevice* pDevice = GetObjectContact().TryToGetOutputDevice();

    // get primitive vector from parent class. Do remember the contents for later use; this
    // is done to create the page content renderer (see PagePrimitiveExtractor in svx) at the
    // original object and to setup the draw hierarchy there so that changes to VCs of displayed
    // objects will lead to InvalidatePartOfView-calls which will be forwarded from the helper-OC
    // to this VOC in calling a ActionChanged().
    //
    // This already produces the displayable page content as a primitive sequence, complete with
    // embedding in the page visualizer, clipping if needed and object and aspect ratio
    // preparations. It would thus be the base for creating the cached visualisation, too,
    // by just painting extactly this primitive sequence.
    //
    // Currently, this slows down PagePane display heavily. Reason is that the current mechanism
    // to react on a SdrObject change in an edit view is to react on the ModelChange and to completely
    // reset the PagePane (delete SdrPageObjs, re-create and layout them). This works, but kicks
    // the complete sequence of primitive creation at VOCs and VCs and their buffering out of
    // memory each time. So there are two choices:
    //
    // 1, disable getting the sequence of primtives
    //    -> invalidate uses ModelChange
    //    -> cache repaint uses complete view creation and repainting
    //
    // 2, create and use the sequence of primitives
    //    -> invalidate would not need ModelChange, no destroy/recreate of SdrObjects, no rearrange,
    //       the invalidate and the following repaint would exactly update the SdrPages involved and
    //       use the DrawingLayer provided ActionChanged() invalidations over the VOCs and VCs
    //    -> cache repaint could use the here offered sequence of primitives to re-create the bitmap
    //       (just hand over the local member to the cache)
    //
    // For the moment i will use (1) and disable primitive creation for SdrPageObj contents here

    // const_cast< PageObjectViewObjectContact* >(this)->mxCurrentPageContents = ViewObjectContactOfPageObj::createPrimitive2DSequence(rDisplayInfo);

    // assert when this call is issued indirectly from the destructor of
    // this instance. This is not allowed and needs to be looked at
#ifdef DBG_UTIL
    if(mbInDestructor)
    {
        OSL_ENSURE(false, "Higher call inside PageObjectViewObjectContact in destructor (!)");
    }
#endif

    // Check if buffering can and shall be done.
    if (pDevice != NULL
        && !GetObjectContact().isOutputToPrinter()
        && !GetObjectContact().isOutputToRecordingMetaFile()
        && !mbInDestructor)
    {
        // get inner and outer logic rectangles. Use model data directly for creation. Do NOT use getBoundRect()/
        // getSnapRect() functionality; these will use the sequence of primitives in the long run itself. SdrPageObj
        // is a SdrObject, so use SdrObject::aOutRect as model data. Access using GetLastBoundRect() to not execute anything
        PageObjectViewContact& rPaObVOC(static_cast< PageObjectViewContact& >(GetViewContact()));
        const Rectangle aInnerLogic(rPaObVOC.GetPageObject().GetLastBoundRect());

        // get BitmapEx from cache. Do exactly the same as Paint_Preview() to avoid a repaint loop
        // caused by slightly different pixel sizes of what the cache sees as pixel size and what is
        // calculated here in discrete coordinates. This includes to not use LogicToPiyel on the Rectangle,
        // but to do the same as the GetBoundingBox() implementation
        const Rectangle aInnerPixel(Rectangle(pDevice->LogicToPixel(aInnerLogic.TopLeft()), pDevice->LogicToPixel(aInnerLogic.GetSize())));
        BitmapEx aBitmapEx(const_cast< PageObjectViewObjectContact* >(this)->GetPreview(rDisplayInfo, aInnerPixel));

        // prepare inner range
        const basegfx::B2DRange aInnerRange(aInnerLogic.Left(), aInnerLogic.Top(), aInnerLogic.Right(), aInnerLogic.Bottom());

        // provide default parameters
        String aPageName;
        Font aPageNameFont;
        sal_uInt32 nPageNumber(0);
        Size aPageNumberAreaModelSize;
        bool bShowFadeEffectIcon(false);
        bool bShowCommentsIcon(false);
        bool bExcluded(false);

        if(GetPage())
        {
            const SdPage* pPage = static_cast<const SdPage*>(GetPage());

            // decide if fade effect indicator will be painted
            if(pPage->getTransitionType() > 0)
            {
                bShowFadeEffectIcon = true;
            }

            bShowCommentsIcon = !pPage->getAnnotations().empty();

            // prepare PageName, PageNumber, font and AreaModelSize
            aPageName = pPage->GetName();
            aPageNameFont = *FontProvider::Instance().GetFont(*pDevice);
            nPageNumber = ((pPage->GetPageNum() - 1) / 2) + 1;
            aPageNumberAreaModelSize = GetPageDescriptor()->GetPageNumberAreaModelSize();

            if(!aPageName.Len())
            {
                aPageName = String(SdResId(STR_PAGE));
                aPageName += String::CreateFromInt32(nPageNumber);
            }

            // decide if page is excluded
            bExcluded = pPage->IsExcluded();
        }

        // create specialized primitives for focus, select and PagePreview itself
        const bool bCreateBitmap(!aBitmapEx.IsEmpty());
        const bool bCreateFocused(GetPageDescriptor()->IsFocused());
        const bool bCreateSelected(GetPageDescriptor()->IsSelected());

        const sal_uInt32 nCount(
            (bCreateBitmap ? 1 : 0) +       // bitmap itself
            1 +                             // border around bitmap (always)
            1 +                             // FadeEffect, PageName and PageNumber visualisation (always)
            (bCreateFocused ? 1 : 0) +      // create focused
            (bCreateSelected ? 1 : 0)       // create selected
            );
        sal_uInt32 nInsert(0);
        Primitive2DSequence xRetval(nCount);

        if(bCreateBitmap)
        {
            // add selection indicator if used
            xRetval[nInsert++] = Primitive2DReference(new SdPageObjectPageBitmapPrimitive(aInnerRange, aBitmapEx));
        }

        if(true)
        {
            // add border (always)
            xRetval[nInsert++] = Primitive2DReference(new SdPageObjectBorderPrimitive(aInnerRange));
        }

        if(true)
        {
            // add fade effext, page name and number if used
            xRetval[nInsert++] = Primitive2DReference(new SdPageObjectFadeNameNumberPrimitive(
                aInnerRange,
                aPageName,
                nPageNumber,
                aPageNameFont,
                aPageNumberAreaModelSize,
                bShowFadeEffectIcon,
                bShowCommentsIcon,
                bExcluded));
        }

        if(bCreateSelected)
        {
            // add selection indicator if used
            xRetval[nInsert++] = Primitive2DReference(new SdPageObjectSelectPrimitive(aInnerRange));
        }

        if(bCreateFocused)
        {
            // add focus indicator if used
            xRetval[nInsert++] = Primitive2DReference(new SdPageObjectFocusPrimitive(aInnerRange, bCreateSelected));
        }

        return xRetval;
    }
    else
    {
        // Call parent. Output to printer or metafile will use vector data, not cached bitmaps
        return ViewObjectContactOfPageObj::createPrimitive2DSequence(rDisplayInfo);
    }
}

BitmapEx PageObjectViewObjectContact::CreatePreview (const DisplayInfo& /*rDisplayInfo*/)
{
    const SdPage* pPage = static_cast<const SdPage*>(GetPage());
    OutputDevice* pDevice = GetObjectContact().TryToGetOutputDevice();

    if(pDevice)
    {
        Rectangle aPreviewPixelBox (GetBoundingBox(*pDevice,PreviewBoundingBox,PixelCoordinateSystem));

        PreviewRenderer aRenderer (pDevice);
        Image aPreview (aRenderer.RenderPage(
            pPage,
            aPreviewPixelBox.GetSize(),
            String()));

        return aPreview.GetBitmapEx();
    }
    else
    {
        return BitmapEx();
    }
}




BitmapEx PageObjectViewObjectContact::GetPreview (
    const DisplayInfo& rDisplayInfo,
    const Rectangle& rNewSizePixel)
{
    BitmapEx aBitmap;

    try
    {
        // assert when this call is issued indirectly from the destructor of
        // this instance. This is not allowed and needs to be looked at
        OSL_ENSURE(!mbInDestructor, "Higher call inside PageObjectViewObjectContact in destructor (!)");

        if (!mbInDestructor)
        {
            if (mpCache != NULL)
            {
                aBitmap = mpCache->GetPreviewBitmap(
                    GetPage(),
                    rNewSizePixel.GetSize());
                mpCache->SetPreciousFlag(GetPage(), true);
            }
            else
                aBitmap = CreatePreview(rDisplayInfo);
        }
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aBitmap;
}




const SdrPage* PageObjectViewObjectContact::GetPage (void) const
{
    return static_cast<PageObjectViewContact&>(GetViewContact()).GetPage();
}




void PageObjectViewObjectContact::ActionChanged (void)
{
    // Even when we are called from destructor we still have to invalide
    // the preview bitmap in the cache.
    const SdrPage* pPage = GetPage();

    if(pPage)
    {
        SdDrawDocument* pDocument = dynamic_cast<SdDrawDocument*>(pPage->GetModel());
        if (mpCache!=NULL && pPage!=NULL && pDocument!=NULL)
        {
            cache::PageCacheManager::Instance()->InvalidatePreviewBitmap(
                pDocument->getUnoModel(),
                pPage);
        }
    }

    // call parent
    ViewObjectContactOfPageObj::ActionChanged();
}

//////////////////////////////////////////////////////////////////////////////
// helper MouseOverEffectPrimitive
//
// Used to allow view-dependent primitive definition. For that purpose, the
// initially created primitive (here: in createMouseOverEffectPrimitive2DSequence())
// always has to be view-independent, but the decomposition is made view-dependent.
// Very simple primitive which just remembers the discrete data and applies
// it at decomposition time.

class MouseOverEffectPrimitive : public drawinglayer::primitive2d::BufferedDecompositionPrimitive2D
{
private:
    basegfx::B2DRange           maLogicRange;
    sal_uInt32                  mnDiscreteOffset;
    sal_uInt32                  mnDiscreteWidth;
    basegfx::BColor             maRGBColor;

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence create2DDecomposition(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation) const;

public:
    MouseOverEffectPrimitive(
        const basegfx::B2DRange& rLogicRange,
        sal_uInt32 nDiscreteOffset,
        sal_uInt32 nDiscreteWidth,
        const basegfx::BColor& rRGBColor)
    :   drawinglayer::primitive2d::BufferedDecompositionPrimitive2D(),
        maLogicRange(rLogicRange),
        mnDiscreteOffset(nDiscreteOffset),
        mnDiscreteWidth(nDiscreteWidth),
        maRGBColor(rRGBColor)
    {}

    // data access
    const basegfx::B2DRange& getLogicRange() const { return maLogicRange; }
    sal_uInt32 getDiscreteOffset() const { return mnDiscreteOffset; }
    sal_uInt32 getDiscreteWidth() const { return mnDiscreteWidth; }
    const basegfx::BColor& getRGBColor() const { return maRGBColor; }

    virtual bool operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const;

    DeclPrimitrive2DIDBlock()
};

drawinglayer::primitive2d::Primitive2DSequence MouseOverEffectPrimitive::create2DDecomposition(
    const drawinglayer::geometry::ViewInformation2D& rViewInformation) const
{
    // get logic sizes in object coordinate system
    const double fDiscreteWidth((rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0)).getLength());
    const double fOffset(fDiscreteWidth * getDiscreteOffset());
    const double fWidth(fDiscreteWidth * getDiscreteWidth());

    // create range (one pixel less to get a good fitting)
    basegfx::B2DRange aRange(
        getLogicRange().getMinimum(),
        getLogicRange().getMaximum() - basegfx::B2DTuple(fDiscreteWidth, fDiscreteWidth));

    // grow range
    aRange.grow(fOffset - (fWidth * 0.5));

    // create fat line with parameters. The formerly hand-painted edge
    // roundings will now be done using rounded edges of this fat line
    const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aRange));
    const drawinglayer::attribute::LineAttribute aLineAttribute(getRGBColor(), fWidth);
    const drawinglayer::primitive2d::Primitive2DReference xReference(
        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
            aPolygon,
            aLineAttribute));

    return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
}

bool MouseOverEffectPrimitive::operator==( const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive ) const
{
    if(drawinglayer::primitive2d::BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const MouseOverEffectPrimitive& rCompare = static_cast< const MouseOverEffectPrimitive& >(rPrimitive);

        return (getLogicRange() == rCompare.getLogicRange()
            && getDiscreteOffset() == rCompare.getDiscreteOffset()
            && getDiscreteWidth() == rCompare.getDiscreteWidth()
            && getRGBColor() == rCompare.getRGBColor());
    }

    return false;
}

ImplPrimitrive2DIDBlock(MouseOverEffectPrimitive, PRIMITIVE2D_ID_SDMOUSEOVEREFFECTPRIMITIVE)

//////////////////////////////////////////////////////////////////////////////

drawinglayer::primitive2d::Primitive2DSequence PageObjectViewObjectContact::createMouseOverEffectPrimitive2DSequence()
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;

    if(GetPageDescriptor()->IsSelected() && mpProperties.get() && mpProperties->IsShowSelection())
    {
        // When the selection frame is visualized the mouse over frame is not
        // visible and does not have to be created.
    }
    else
    {
        const PageObjectViewContact& rPaObVOC(static_cast<PageObjectViewContact&>(GetViewContact()));
        const Rectangle aBoundingBox(rPaObVOC.GetPageObject().GetLastBoundRect());
        const basegfx::B2DRange aLogicRange(aBoundingBox.Left(), aBoundingBox.Top(), aBoundingBox.Right(), aBoundingBox.Bottom());
        const basegfx::BColor aSelectionColor(mpProperties->GetSelectionColor().getBColor());
        const drawinglayer::primitive2d::Primitive2DReference aReference(
            new MouseOverEffectPrimitive(
                aLogicRange,
                mnMouseOverEffectOffset,
                mnMouseOverEffectThickness,
                aSelectionColor));

        aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
    }

    return aRetval;
}




SvBorder PageObjectViewObjectContact::CalculatePageModelBorder (
    OutputDevice* pDevice,
    int nPageCount)
{
    SvBorder aModelBorder;

    if (pDevice != NULL)
    {
        // 1. Initialize the border with the values that do not depend on
        // the device.
        Size aTopLeftBorders (pDevice->PixelToLogic (Size (
            mnPageNumberOffset+1,
            mnSelectionIndicatorOffset + mnSelectionIndicatorThickness)));
        Size aBottomRightBorders (pDevice->PixelToLogic (Size (
            mnSelectionIndicatorOffset + mnSelectionIndicatorThickness,
            mnFadeEffectIndicatorOffset)));
        aModelBorder = SvBorder (
            aTopLeftBorders.Width(),
            aTopLeftBorders.Height(),
            aBottomRightBorders.Width(),
            aBottomRightBorders.Height());


        // 2. Add the device dependent values.

        // Calculate the area of the page number.
        Size aPageNumberModelSize (
            CalculatePageNumberAreaModelSize (pDevice, nPageCount));

        // Update the border.
        aModelBorder.Left() += aPageNumberModelSize.Width();
        // The height of the page number area is the same as the height of
        // the page name area.
        aModelBorder.Bottom() += aPageNumberModelSize.Height();
    }

    return aModelBorder;
}




Size PageObjectViewObjectContact::CalculatePageNumberAreaModelSize (
    OutputDevice* pDevice,
    int nPageCount)
{
    // Set the correct font.
    Font aOriginalFont (pDevice->GetFont());
    pDevice->SetFont(*FontProvider::Instance().GetFont(*pDevice));

    String sPageNumberTemplate;
    if (nPageCount < 10)
        sPageNumberTemplate = String::CreateFromAscii("9");
    else if (nPageCount < 100)
        sPageNumberTemplate = String::CreateFromAscii("99");
    else if (nPageCount < 200)
        // Just for the case that 1 is narrower than 9.
        sPageNumberTemplate = String::CreateFromAscii("199");
    else if (nPageCount < 1000)
        sPageNumberTemplate = String::CreateFromAscii("999");
    else
        sPageNumberTemplate = String::CreateFromAscii("9999");
    // More then 9999 pages are not handled.

    Size aSize (
        pDevice->GetTextWidth (sPageNumberTemplate),
        pDevice->GetTextHeight ());

    pDevice->SetFont (aOriginalFont);

    return aSize;
}




model::SharedPageDescriptor
    PageObjectViewObjectContact::GetPageDescriptor (void) const
{
    PageObjectViewContact& rViewContact (
        static_cast<PageObjectViewContact&>(GetViewContact()));
    PageObject& rPageObject (
        static_cast<PageObject&>(rViewContact.GetPageObject()));
    return rPageObject.GetDescriptor();
}



} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
