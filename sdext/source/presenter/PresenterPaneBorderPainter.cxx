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

#include "PresenterPaneBorderPainter.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterTheme.hxx"
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#include <com/sun/star/awt/XFont.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define A2S(s) (::rtl::OUString(s))

namespace sdext { namespace presenter {

namespace {
    class BorderSize
    {
    public:
        BorderSize (void);
        BorderSize (const BorderSize& rBorderSize);
        BorderSize& operator= (const BorderSize& rBoderSize);
        sal_Int32 mnLeft;
        sal_Int32 mnTop;
        sal_Int32 mnRight;
        sal_Int32 mnBottom;
    };

    class RendererPaneStyle
    {
    public:
        RendererPaneStyle (
            const ::boost::shared_ptr<PresenterTheme>& rpTheme,
            const OUString& rsStyleName);

        awt::Rectangle AddBorder (
            const awt::Rectangle& rBox,
            drawing::framework::BorderType eBorderType) const;
        awt::Rectangle RemoveBorder (
            const awt::Rectangle& rBox,
            drawing::framework::BorderType eBorderType) const;
        const Reference<rendering::XCanvasFont> GetFont (
            const Reference<rendering::XCanvas>& rxCanvas) const;

        SharedBitmapDescriptor mpTopLeft;
        SharedBitmapDescriptor mpTop;
        SharedBitmapDescriptor mpTopRight;
        SharedBitmapDescriptor mpLeft;
        SharedBitmapDescriptor mpRight;
        SharedBitmapDescriptor mpBottomLeft;
        SharedBitmapDescriptor mpBottom;
        SharedBitmapDescriptor mpBottomRight;
        SharedBitmapDescriptor mpBottomCallout;
        SharedBitmapDescriptor mpBackground;
        SharedBitmapDescriptor mpEmpty;
        PresenterTheme::SharedFontDescriptor mpFont;
        sal_Int32 mnFontXOffset;
        sal_Int32 mnFontYOffset;
        enum Anchor { AnchorLeft, AnchorRight, AnchorCenter } meFontAnchor;
        BorderSize maInnerBorderSize;
        BorderSize maOuterBorderSize;
        BorderSize maTotalBorderSize;
        enum Side { Left, Top, Right, Bottom };
    private:
        void UpdateBorderSizes (void);
        SharedBitmapDescriptor GetBitmap(
            const ::boost::shared_ptr<PresenterTheme>& rpTheme,
            const OUString& rsStyleName,
            const OUString& rsBitmapName);
    };
}

class  PresenterPaneBorderPainter::Renderer
{
public:
    Renderer (
        const Reference<XComponentContext>& rxContext,
        const ::boost::shared_ptr<PresenterTheme>& rpTheme);
    ~Renderer (void);

    void SetCanvas (const Reference<rendering::XCanvas>& rxCanvas);
    void PaintBorder (
        const OUString& rsTitle,
        const awt::Rectangle& rBBox,
        const awt::Rectangle& rUpdateBox,
        const OUString& rsPaneURL);
    void PaintTitle (
        const OUString& rsTitle,
        const ::boost::shared_ptr<RendererPaneStyle>& rpStyle,
        const awt::Rectangle& rUpdateBox,
        const awt::Rectangle& rOuterBox,
        const awt::Rectangle& rInnerBox,
        const bool bPaintBackground);
    void SetupClipping (
        const awt::Rectangle& rUpdateBox,
        const awt::Rectangle& rOuterBox,
        const OUString& rsPaneStyleName);
    ::boost::shared_ptr<RendererPaneStyle> GetRendererPaneStyle (const OUString& rsResourceURL);
    void SetCalloutAnchor (
        const awt::Point& rCalloutAnchor);

private:
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    typedef ::std::map<OUString, ::boost::shared_ptr<RendererPaneStyle> > RendererPaneStyleContainer;
    RendererPaneStyleContainer maRendererPaneStyles;
    Reference<rendering::XCanvas> mxCanvas;
    Reference<drawing::XPresenterHelper> mxPresenterHelper;
    css::rendering::ViewState maViewState;
    Reference<rendering::XPolyPolygon2D> mxViewStateClip;
    bool mbHasCallout;
    awt::Point maCalloutAnchor;

    void PaintBitmap(
        const awt::Rectangle& rBox,
        const awt::Rectangle& rUpdateBox,
        const sal_Int32 nXPosition,
        const sal_Int32 nYPosition,
        const sal_Int32 nStartOffset,
        const sal_Int32 nEndOffset,
        const bool bExpand,
        const SharedBitmapDescriptor& rpBitmap,
        const SharedBitmapDescriptor& rpBackgroundBitmap);
};

// ===== PresenterPaneBorderPainter ===========================================

PresenterPaneBorderPainter::PresenterPaneBorderPainter (
    const Reference<XComponentContext>& rxContext)
    : PresenterPaneBorderPainterInterfaceBase(m_aMutex),
      mxContext(rxContext),
      mpTheme(),
      mpRenderer()
{
}

PresenterPaneBorderPainter::~PresenterPaneBorderPainter (void)
{
}

//----- XPaneBorderPainter ----------------------------------------------------

awt::Rectangle SAL_CALL PresenterPaneBorderPainter::addBorder (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    drawing::framework::BorderType eBorderType)
    throw(css::uno::RuntimeException)
{
    ThrowIfDisposed();

    ProvideTheme();

    return AddBorder(rsPaneBorderStyleName, rRectangle, eBorderType);
}

awt::Rectangle SAL_CALL PresenterPaneBorderPainter::removeBorder (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    drawing::framework::BorderType eBorderType)
    throw(css::uno::RuntimeException)
{
    ThrowIfDisposed();

    ProvideTheme();

    return RemoveBorder(rsPaneBorderStyleName, rRectangle, eBorderType);
}

void SAL_CALL PresenterPaneBorderPainter::paintBorder (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rOuterBorderRectangle,
    const css::awt::Rectangle& rRepaintArea,
    const rtl::OUString& rsTitle)
    throw(css::uno::RuntimeException)
{
    ThrowIfDisposed();

    // Early reject paints completely outside the repaint area.
    if (rRepaintArea.X >= rOuterBorderRectangle.X+rOuterBorderRectangle.Width
        || rRepaintArea.Y >= rOuterBorderRectangle.Y+rOuterBorderRectangle.Height
        || rRepaintArea.X+rRepaintArea.Width <= rOuterBorderRectangle.X
        || rRepaintArea.Y+rRepaintArea.Height <= rOuterBorderRectangle.Y)
    {
        return;
    }
    ProvideTheme(rxCanvas);

    if (mpRenderer.get() != NULL)
    {
        mpRenderer->SetCanvas(rxCanvas);
        mpRenderer->SetupClipping(
            rRepaintArea,
            rOuterBorderRectangle,
            rsPaneBorderStyleName);
        mpRenderer->PaintBorder(
            rsTitle,
            rOuterBorderRectangle,
            rRepaintArea,
            rsPaneBorderStyleName);
    }
}

void SAL_CALL PresenterPaneBorderPainter::paintBorderWithCallout (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rOuterBorderRectangle,
    const css::awt::Rectangle& rRepaintArea,
    const rtl::OUString& rsTitle,
    const css::awt::Point& rCalloutAnchor)
    throw(css::uno::RuntimeException)
{
    ThrowIfDisposed();

    // Early reject paints completely outside the repaint area.
    if (rRepaintArea.X >= rOuterBorderRectangle.X+rOuterBorderRectangle.Width
        || rRepaintArea.Y >= rOuterBorderRectangle.Y+rOuterBorderRectangle.Height
        || rRepaintArea.X+rRepaintArea.Width <= rOuterBorderRectangle.X
        || rRepaintArea.Y+rRepaintArea.Height <= rOuterBorderRectangle.Y)
    {
        return;
    }
    ProvideTheme(rxCanvas);

    if (mpRenderer.get() != NULL)
    {
        mpRenderer->SetCanvas(rxCanvas);
        mpRenderer->SetupClipping(
            rRepaintArea,
            rOuterBorderRectangle,
            rsPaneBorderStyleName);
        mpRenderer->SetCalloutAnchor(rCalloutAnchor);
        mpRenderer->PaintBorder(
            rsTitle,
            rOuterBorderRectangle,
            rRepaintArea,
            rsPaneBorderStyleName);
    }
}

awt::Point SAL_CALL PresenterPaneBorderPainter::getCalloutOffset (
    const rtl::OUString& rsPaneBorderStyleName)
    throw(css::uno::RuntimeException)
{
    ThrowIfDisposed();
    ProvideTheme();
    if (mpRenderer.get() != NULL)
    {
        const ::boost::shared_ptr<RendererPaneStyle> pRendererPaneStyle(
            mpRenderer->GetRendererPaneStyle(rsPaneBorderStyleName));
        if (pRendererPaneStyle.get() != NULL
            && pRendererPaneStyle->mpBottomCallout.get() != NULL)
        {
            return awt::Point (
                0,
                pRendererPaneStyle->mpBottomCallout->mnHeight
                    - pRendererPaneStyle->mpBottomCallout->mnYHotSpot);
        }
    }

    return awt::Point(0,0);
}

//-----------------------------------------------------------------------------

bool PresenterPaneBorderPainter::ProvideTheme (const Reference<rendering::XCanvas>& rxCanvas)
{
    bool bModified (false);

    if ( ! mxContext.is())
        return false;

    if (mpTheme.get() != NULL)
    {
        // Check if the theme already has a canvas.
        if ( ! mpTheme->HasCanvas())
        {
            mpTheme->ProvideCanvas(rxCanvas);
            bModified = true;
        }
    }
    else
    {
        mpTheme.reset(new PresenterTheme(mxContext, OUString(), rxCanvas));
        bModified = true;
    }

    if (mpTheme.get() != NULL && bModified)
    {
        if (mpRenderer.get() == NULL)
            mpRenderer.reset(new Renderer(mxContext, mpTheme));
        else
            mpRenderer->SetCanvas(rxCanvas);
    }

    return bModified;
}

bool PresenterPaneBorderPainter::ProvideTheme (void)
{
    if (mpTheme.get() == NULL)
    {
        // Create a theme without bitmaps (no canvas => no bitmaps).
        return ProvideTheme(NULL);
    }
    else
    {
        // When there already is a theme then without a canvas we can not
        // add anything new.
        return false;
    }
}

void PresenterPaneBorderPainter::SetTheme (const ::boost::shared_ptr<PresenterTheme>& rpTheme)
{
    mpTheme = rpTheme;
    if (mpRenderer.get() == NULL)
        mpRenderer.reset(new Renderer(mxContext, mpTheme));
}

awt::Rectangle PresenterPaneBorderPainter::AddBorder (
    const ::rtl::OUString& rsPaneURL,
    const awt::Rectangle& rInnerBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    if (mpRenderer.get() != NULL)
    {
        const ::boost::shared_ptr<RendererPaneStyle> pRendererPaneStyle(mpRenderer->GetRendererPaneStyle(rsPaneURL));
        if (pRendererPaneStyle.get() != NULL)
            return pRendererPaneStyle->AddBorder(rInnerBox, eBorderType);
    }
    return rInnerBox;
}

awt::Rectangle PresenterPaneBorderPainter::RemoveBorder (
    const ::rtl::OUString& rsPaneURL,
    const css::awt::Rectangle& rOuterBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    if (mpRenderer.get() != NULL)
    {
        const ::boost::shared_ptr<RendererPaneStyle> pRendererPaneStyle(mpRenderer->GetRendererPaneStyle(rsPaneURL));
        if (pRendererPaneStyle.get() != NULL)
            return pRendererPaneStyle->RemoveBorder(rOuterBox, eBorderType);
    }
    return rOuterBox;
}

void PresenterPaneBorderPainter::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(
                "PresenterPaneBorderPainter object has already been disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

//===== PresenterPaneBorderPainter::Renderer =====================================

PresenterPaneBorderPainter::Renderer::Renderer (
    const Reference<XComponentContext>& rxContext,
    const ::boost::shared_ptr<PresenterTheme>& rpTheme)
    : mpTheme(rpTheme),
      maRendererPaneStyles(),
      mxCanvas(),
      mxPresenterHelper(),
      maViewState(geometry::AffineMatrix2D(1,0,0, 0,1,0), NULL),
      mxViewStateClip(),
      mbHasCallout(false),
      maCalloutAnchor()
{
    (void)rxContext;

    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString("com.sun.star.comp.Draw.PresenterHelper"),
                rxContext),
            UNO_QUERY_THROW);
    }
}

PresenterPaneBorderPainter::Renderer::~Renderer (void)
{
}

void PresenterPaneBorderPainter::Renderer::SetCanvas (const Reference<rendering::XCanvas>& rxCanvas)
{
    if (mxCanvas != rxCanvas)
    {
        mxCanvas = rxCanvas;
    }
}

void PresenterPaneBorderPainter::Renderer::PaintBorder (
    const OUString& rsTitle,
    const awt::Rectangle& rBBox,
    const awt::Rectangle& rUpdateBox,
    const OUString& rsPaneURL)
{
    if ( ! mxCanvas.is())
        return;

    // Create the outer and inner border of the, ahm, border.
    ::boost::shared_ptr<RendererPaneStyle> pStyle (GetRendererPaneStyle(rsPaneURL));
    if (pStyle.get() == NULL)
        return;

    awt::Rectangle aOuterBox (rBBox);
    awt::Rectangle aCenterBox (
        pStyle->RemoveBorder(aOuterBox, drawing::framework::BorderType_OUTER_BORDER));
    awt::Rectangle aInnerBox (
        pStyle->RemoveBorder(aOuterBox, drawing::framework::BorderType_TOTAL_BORDER));

    // Prepare references for all used bitmaps.
    SharedBitmapDescriptor pTop (pStyle->mpTop);
    SharedBitmapDescriptor pTopLeft (pStyle->mpTopLeft);
    SharedBitmapDescriptor pTopRight (pStyle->mpTopRight);
    SharedBitmapDescriptor pLeft (pStyle->mpLeft);
    SharedBitmapDescriptor pRight (pStyle->mpRight);
    SharedBitmapDescriptor pBottomLeft (pStyle->mpBottomLeft);
    SharedBitmapDescriptor pBottomRight (pStyle->mpBottomRight);
    SharedBitmapDescriptor pBottom (pStyle->mpBottom);
    SharedBitmapDescriptor pBackground (pStyle->mpBackground);

    // Paint the sides.
    PaintBitmap(aCenterBox, rUpdateBox, 0,-1,
        pTopLeft->mnXOffset, pTopRight->mnXOffset, true, pTop, pBackground);
    PaintBitmap(aCenterBox, rUpdateBox, -1,0,
        pTopLeft->mnYOffset, pBottomLeft->mnYOffset, true, pLeft, pBackground);
    PaintBitmap(aCenterBox, rUpdateBox, +1,0,
        pTopRight->mnYOffset, pBottomRight->mnYOffset, true, pRight, pBackground);
    if (mbHasCallout && pStyle->mpBottomCallout->GetNormalBitmap().is())
    {
        const sal_Int32 nCalloutWidth (pStyle->mpBottomCallout->mnWidth);
        sal_Int32 nCalloutX (maCalloutAnchor.X - pStyle->mpBottomCallout->mnXHotSpot
            - (aCenterBox.X - aOuterBox.X));
        if (nCalloutX < pBottomLeft->mnXOffset + aCenterBox.X)
            nCalloutX = pBottomLeft->mnXOffset + aCenterBox.X;
        if (nCalloutX > pBottomRight->mnXOffset + aCenterBox.X + aCenterBox.Width)
            nCalloutX = pBottomRight->mnXOffset + aCenterBox.X + aCenterBox.Width;
        // Paint bottom callout.
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1, nCalloutX,0, false, pStyle->mpBottomCallout, pBackground);
        // Paint regular bottom bitmap left and right.
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1,
            pBottomLeft->mnXOffset, nCalloutX-aCenterBox.Width, true, pBottom, pBackground);
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1,
            nCalloutX+nCalloutWidth, pBottomRight->mnXOffset, true, pBottom, pBackground);
    }
    else
    {
        // Stretch the bottom bitmap over the full width.
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1,
            pBottomLeft->mnXOffset, pBottomRight->mnXOffset, true, pBottom, pBackground);
    }

    // Paint the corners.
    PaintBitmap(aCenterBox, rUpdateBox, -1,-1, 0,0, false, pTopLeft, pBackground);
    PaintBitmap(aCenterBox, rUpdateBox, +1,-1, 0,0, false, pTopRight, pBackground);
    PaintBitmap(aCenterBox, rUpdateBox, -1,+1, 0,0, false, pBottomLeft, pBackground);
    PaintBitmap(aCenterBox, rUpdateBox, +1,+1, 0,0, false, pBottomRight, pBackground);

    // Paint the title.
    PaintTitle(rsTitle, pStyle, rUpdateBox, aOuterBox, aInnerBox, false);

    // In a double buffering environment request to make the changes visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}

void PresenterPaneBorderPainter::Renderer::PaintTitle (
    const OUString& rsTitle,
    const ::boost::shared_ptr<RendererPaneStyle>& rpStyle,
    const awt::Rectangle& rUpdateBox,
    const awt::Rectangle& rOuterBox,
    const awt::Rectangle& rInnerBox,
    bool bPaintBackground)
{
    if ( ! mxCanvas.is())
        return;

    if (rsTitle.isEmpty())
        return;

    Reference<rendering::XCanvasFont> xFont (rpStyle->GetFont(mxCanvas));
    if ( ! xFont.is())
        return;

    rendering::StringContext aContext (
        rsTitle,
        0,
        rsTitle.getLength());
    Reference<rendering::XTextLayout> xLayout (xFont->createTextLayout(
        aContext,
        rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
        0));
    if ( ! xLayout.is())
        return;

    geometry::RealRectangle2D aBox (xLayout->queryTextBounds());
    const double nTextHeight = aBox.Y2 - aBox.Y1;
    const double nTextWidth = aBox.X2 - aBox.X1;
    double nX = rInnerBox.X + (rInnerBox.Width - nTextWidth)/2;
    const sal_Int32 nTitleBarHeight = rInnerBox.Y - rOuterBox.Y - 1;
    double nY = rOuterBox.Y + (nTitleBarHeight - nTextHeight) / 2 - aBox.Y1;
    if (nY >= rInnerBox.Y)
        nY = rInnerBox.Y - 1;
    switch (rpStyle->meFontAnchor)
    {
        default:
        case RendererPaneStyle::AnchorLeft:
            nX = rInnerBox.X;
            break;
        case RendererPaneStyle::AnchorRight:
            nX = rInnerBox.X + rInnerBox.Width - nTextWidth;
            break;
        case RendererPaneStyle::AnchorCenter:
            nX = rInnerBox.X + (rInnerBox.Width - nTextWidth)/2;
            break;
    }
    nX += rpStyle->mnFontXOffset;
    nY += rpStyle->mnFontYOffset;

    if (rUpdateBox.X >= nX+nTextWidth
        || rUpdateBox.Y >= nY+nTextHeight
        || rUpdateBox.X+rUpdateBox.Width <= nX
        || rUpdateBox.Y+rUpdateBox.Height <= nY)
    {
        return;
    }

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,nX, 0,1,nY),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    if (bPaintBackground)
    {
        PresenterCanvasHelper::SetDeviceColor(aRenderState, util::Color(0x00ffffff));
        Sequence<Sequence<geometry::RealPoint2D> > aPolygons(1);
        aPolygons[0] = Sequence<geometry::RealPoint2D>(4);
        aPolygons[0][0] = geometry::RealPoint2D(0, -nTextHeight);
        aPolygons[0][1] = geometry::RealPoint2D(0, 0);
        aPolygons[0][2] = geometry::RealPoint2D(nTextWidth, 0);
        aPolygons[0][3] = geometry::RealPoint2D(nTextWidth, -nTextHeight);
        Reference<rendering::XPolyPolygon2D> xPolygon (
            mxCanvas->getDevice()->createCompatibleLinePolyPolygon(aPolygons), UNO_QUERY);
        if (xPolygon.is())
            xPolygon->setClosed(0, sal_True);
        mxCanvas->fillPolyPolygon(
            xPolygon,
            maViewState,
            aRenderState);
    }
    else
    {
        PresenterCanvasHelper::SetDeviceColor(
            aRenderState,
            rpStyle->mpFont->mnColor);

        mxCanvas->drawText(
            aContext,
            xFont,
            maViewState,
            aRenderState,
            rendering::TextDirection::WEAK_LEFT_TO_RIGHT);
    }
}

::boost::shared_ptr<RendererPaneStyle>
    PresenterPaneBorderPainter::Renderer::GetRendererPaneStyle (const OUString& rsResourceURL)
{
    OSL_ASSERT(mpTheme.get()!=NULL);

    RendererPaneStyleContainer::const_iterator iStyle (maRendererPaneStyles.find(rsResourceURL));
    if (iStyle == maRendererPaneStyles.end())
    {
        OUString sPaneStyleName ("DefaultRendererPaneStyle");

        // Get pane layout name for resource URL.
        const OUString sStyleName (mpTheme->GetStyleName(rsResourceURL));
        if (!sStyleName.isEmpty())
            sPaneStyleName = sStyleName;

        // Create a new pane style object and initialize it with bitmaps.
        ::boost::shared_ptr<RendererPaneStyle> pStyle (
            new RendererPaneStyle(mpTheme,sPaneStyleName));
        iStyle = maRendererPaneStyles.insert(
            RendererPaneStyleContainer::value_type(rsResourceURL, pStyle)).first;
    }
    if (iStyle != maRendererPaneStyles.end())
        return iStyle->second;
    else
        return ::boost::shared_ptr<RendererPaneStyle>();
}

void PresenterPaneBorderPainter::Renderer::SetCalloutAnchor (
    const awt::Point& rCalloutAnchor)
{
    mbHasCallout = true;
    maCalloutAnchor = rCalloutAnchor;
}

void PresenterPaneBorderPainter::Renderer::PaintBitmap(
    const awt::Rectangle& rBox,
    const awt::Rectangle& rUpdateBox,
    const sal_Int32 nXPosition,
    const sal_Int32 nYPosition,
    const sal_Int32 nStartOffset,
    const sal_Int32 nEndOffset,
    const bool bExpand,
    const SharedBitmapDescriptor& rpBitmap,
    const SharedBitmapDescriptor& rpBackgroundBitmap)
{
    (void)rpBackgroundBitmap;

    bool bUseCanvas (mxCanvas.is());
    if ( ! bUseCanvas)
        return;

    if (rpBitmap->mnWidth<=0 || rpBitmap->mnHeight<=0)
        return;

    Reference<rendering::XBitmap> xBitmap (rpBitmap->GetNormalBitmap(), UNO_QUERY);
    if ( ! xBitmap.is())
        return;

    // Calculate position, and for side bitmaps, the size.
    sal_Int32 nX = 0;
    sal_Int32 nY = 0;
    sal_Int32 nW = rpBitmap->mnWidth;
    sal_Int32 nH = rpBitmap->mnHeight;
    if (nXPosition < 0)
    {
        nX = rBox.X - rpBitmap->mnWidth + rpBitmap->mnXOffset;
    }
    else if (nXPosition > 0)
    {
        nX = rBox.X + rBox.Width + rpBitmap->mnXOffset;
    }
    else
    {
        nX = rBox.X + nStartOffset;
        if (bExpand)
            nW = rBox.Width - nStartOffset + nEndOffset;
    }

    if (nYPosition < 0)
    {
        nY = rBox.Y - rpBitmap->mnHeight + rpBitmap->mnYOffset;
    }
    else if (nYPosition > 0)
    {
        nY = rBox.Y + rBox.Height + rpBitmap->mnYOffset;
    }
    else
    {
        nY = rBox.Y + nStartOffset;
        if (bExpand)
            nH = rBox.Height - nStartOffset + nEndOffset;
    }

    // Do not paint when bitmap area does not intersect with update box.
    if (nX >= rUpdateBox.X + rUpdateBox.Width
        || nX+nW <= rUpdateBox.X
        || nY >= rUpdateBox.Y + rUpdateBox.Height
        || nY+nH <= rUpdateBox.Y)
    {
        return;
    }

    /*
    Reference<rendering::XBitmap> xMaskedBitmap (
        PresenterBitmapHelper::FillMaskedWithColor (
            mxCanvas,
            Reference<rendering::XIntegerBitmap>(xBitmap, UNO_QUERY),
            rBitmap.mxMaskBitmap,
            0x00ff0000,
            rBackgroundBitmap.maReplacementColor));
    if (xMaskedBitmap.is())
        xBitmap = xMaskedBitmap;
    else if (rBitmap.mxMaskBitmap.is() && mxPresenterHelper.is())
    {
        const static sal_Int32 nOutsideMaskColor (0x00ff0000);
        Reference<rendering::XIntegerBitmap> xMask (
            mxPresenterHelper->createMask(
                mxCanvas,
                rBitmap.mxMaskBitmap,
                nOutsideMaskColor,
                false));
        xBitmap = mxPresenterHelper->applyBitmapMaskWithColor(
            mxCanvas,
            Reference<rendering::XIntegerBitmap>(xBitmap, UNO_QUERY),
            xMask,
            rBackgroundBitmap.maReplacementColor);
    }
    */
    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(
            double(nW)/rpBitmap->mnWidth, 0, nX,
            0, double(nH)/rpBitmap->mnHeight, nY),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::OVER);

    if (xBitmap.is())
        mxCanvas->drawBitmap(
            xBitmap,
            maViewState,
            aRenderState);
}

void PresenterPaneBorderPainter::Renderer::SetupClipping (
    const awt::Rectangle& rUpdateBox,
    const awt::Rectangle& rOuterBox,
    const OUString& rsPaneStyleName)
{
    mxViewStateClip = NULL;
    maViewState.Clip = NULL;

    if ( ! mxCanvas.is())
        return;

    ::boost::shared_ptr<RendererPaneStyle> pStyle (GetRendererPaneStyle(rsPaneStyleName));
    if (pStyle.get() == NULL)
    {
        mxViewStateClip = PresenterGeometryHelper::CreatePolygon(
            rUpdateBox,
            mxCanvas->getDevice());
    }
    else
    {
        awt::Rectangle aInnerBox (
            pStyle->RemoveBorder(rOuterBox, drawing::framework::BorderType_TOTAL_BORDER));
        ::std::vector<awt::Rectangle> aRectangles;
        aRectangles.push_back(PresenterGeometryHelper::Intersection(rUpdateBox, rOuterBox));
        aRectangles.push_back(PresenterGeometryHelper::Intersection(rUpdateBox, aInnerBox));
        mxViewStateClip = PresenterGeometryHelper::CreatePolygon(
            aRectangles,
            mxCanvas->getDevice());
        if (mxViewStateClip.is())
            mxViewStateClip->setFillRule(rendering::FillRule_EVEN_ODD);
    }
    maViewState.Clip = mxViewStateClip;
}

namespace {

//===== BorderSize ============================================================

BorderSize::BorderSize (void)
    : mnLeft(0),
      mnTop(0),
      mnRight(0),
      mnBottom(0)
{
}

BorderSize::BorderSize (const BorderSize& rBorderSize)
    : mnLeft(rBorderSize.mnLeft),
      mnTop(rBorderSize.mnTop),
      mnRight(rBorderSize.mnRight),
      mnBottom(rBorderSize.mnBottom)
{
}

BorderSize& BorderSize::operator= (const BorderSize& rBorderSize)
{
    if (&rBorderSize != this)
    {
        mnLeft = rBorderSize.mnLeft;
        mnTop = rBorderSize.mnTop;
        mnRight = rBorderSize.mnRight;
        mnBottom = rBorderSize.mnBottom;
    }
    return *this;
}

//===== RendererPaneStyle  ============================================================

RendererPaneStyle::RendererPaneStyle (
    const ::boost::shared_ptr<PresenterTheme>& rpTheme,
    const OUString& rsStyleName)
    : mpTopLeft(),
      mpTop(),
      mpTopRight(),
      mpLeft(),
      mpRight(),
      mpBottomLeft(),
      mpBottom(),
      mpBottomRight(),
      mpBottomCallout(),
      mpBackground(),
      mpEmpty(new PresenterBitmapDescriptor()),
      mpFont(),
      mnFontXOffset(0),
      mnFontYOffset(0),
      meFontAnchor(AnchorCenter),
      maInnerBorderSize(),
      maOuterBorderSize(),
      maTotalBorderSize()
{
    if (rpTheme.get() != NULL)
    {
        mpTopLeft = GetBitmap(rpTheme, rsStyleName, A2S("TopLeft"));
        mpTop = GetBitmap(rpTheme, rsStyleName,  A2S("Top"));
        mpTopRight = GetBitmap(rpTheme, rsStyleName,  A2S("TopRight"));
        mpLeft = GetBitmap(rpTheme, rsStyleName, A2S("Left"));
        mpRight = GetBitmap(rpTheme, rsStyleName,  A2S("Right"));
        mpBottomLeft = GetBitmap(rpTheme, rsStyleName, A2S("BottomLeft"));
        mpBottom = GetBitmap(rpTheme, rsStyleName,  A2S("Bottom"));
        mpBottomRight = GetBitmap(rpTheme, rsStyleName,  A2S("BottomRight"));
        mpBottomCallout = GetBitmap(rpTheme, rsStyleName,  A2S("BottomCallout"));
        mpBackground = GetBitmap(rpTheme, OUString(), A2S("Background"));

        // Get font description.
        mpFont = rpTheme->GetFont(rsStyleName);

        OUString sAnchor ("Left");
        if (mpFont.get() != NULL)
        {
            sAnchor = mpFont->msAnchor;
            mnFontXOffset = mpFont->mnXOffset;
            mnFontYOffset = mpFont->mnYOffset;
        }

        if ( sAnchor == "Left" )
            meFontAnchor = AnchorLeft;
        else if ( sAnchor == "Right" )
            meFontAnchor = AnchorRight;
        else
            meFontAnchor = AnchorCenter;

        // Get border sizes.
        try
        {
            ::std::vector<sal_Int32> aInnerBorder (rpTheme->GetBorderSize(rsStyleName, false));
            OSL_ASSERT(aInnerBorder.size()==4);
            maInnerBorderSize.mnLeft = aInnerBorder[0];
            maInnerBorderSize.mnTop = aInnerBorder[1];
            maInnerBorderSize.mnRight = aInnerBorder[2];
            maInnerBorderSize.mnBottom = aInnerBorder[3];

            ::std::vector<sal_Int32> aOuterBorder (rpTheme->GetBorderSize(rsStyleName, true));
            OSL_ASSERT(aOuterBorder.size()==4);
            maOuterBorderSize.mnLeft = aOuterBorder[0];
            maOuterBorderSize.mnTop = aOuterBorder[1];
            maOuterBorderSize.mnRight = aOuterBorder[2];
            maOuterBorderSize.mnBottom = aOuterBorder[3];
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        UpdateBorderSizes();
    }
}

awt::Rectangle RendererPaneStyle::AddBorder (
    const awt::Rectangle& rBox,
    const drawing::framework::BorderType eBorderType) const
{
    const BorderSize* pBorderSize = NULL;
    switch (eBorderType)
    {
        case drawing::framework::BorderType_INNER_BORDER:
            pBorderSize = &maInnerBorderSize;
            break;
        case drawing::framework::BorderType_OUTER_BORDER:
            pBorderSize = &maOuterBorderSize;
            break;
        case drawing::framework::BorderType_TOTAL_BORDER:
            pBorderSize = &maTotalBorderSize;
            break;
        default:
            return rBox;
    }
    return awt::Rectangle (
        rBox.X - pBorderSize->mnLeft,
        rBox.Y - pBorderSize->mnTop,
        rBox.Width + pBorderSize->mnLeft + pBorderSize->mnRight,
        rBox.Height + pBorderSize->mnTop + pBorderSize->mnBottom);
}

awt::Rectangle RendererPaneStyle::RemoveBorder (
    const awt::Rectangle& rBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    const BorderSize* pBorderSize = NULL;
    switch (eBorderType)
    {
        case drawing::framework::BorderType_INNER_BORDER:
            pBorderSize = &maInnerBorderSize;
            break;
        case drawing::framework::BorderType_OUTER_BORDER:
            pBorderSize = &maOuterBorderSize;
            break;
        case drawing::framework::BorderType_TOTAL_BORDER:
            pBorderSize = &maTotalBorderSize;
            break;
        default:
            return rBox;
    }
    return awt::Rectangle (
        rBox.X + pBorderSize->mnLeft,
        rBox.Y + pBorderSize->mnTop,
        rBox.Width - pBorderSize->mnLeft - pBorderSize->mnRight,
        rBox.Height - pBorderSize->mnTop - pBorderSize->mnBottom);
}

const Reference<rendering::XCanvasFont> RendererPaneStyle::GetFont (
    const Reference<rendering::XCanvas>& rxCanvas) const
{
    if (mpFont.get() != NULL)
        mpFont->PrepareFont(rxCanvas);
    return mpFont->mxFont;
}

void RendererPaneStyle::UpdateBorderSizes (void)
{
    maTotalBorderSize.mnLeft = maInnerBorderSize.mnLeft + maOuterBorderSize.mnLeft;
    maTotalBorderSize.mnTop = maInnerBorderSize.mnTop + maOuterBorderSize.mnTop;
    maTotalBorderSize.mnRight = maInnerBorderSize.mnRight + maOuterBorderSize.mnRight;
    maTotalBorderSize.mnBottom = maInnerBorderSize.mnBottom + maOuterBorderSize.mnBottom;
}

SharedBitmapDescriptor RendererPaneStyle::GetBitmap(
    const ::boost::shared_ptr<PresenterTheme>& rpTheme,
    const OUString& rsStyleName,
    const OUString& rsBitmapName)
{
    SharedBitmapDescriptor pDescriptor (rpTheme->GetBitmap(rsStyleName, rsBitmapName));
    if (pDescriptor.get() != NULL)
        return pDescriptor;
    else
        return mpEmpty;
}

} // end of anonymous namespace

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
