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

#include <vcl/svapp.hxx>
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
#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext { namespace presenter {

namespace {
    class BorderSize
    {
    public:
        BorderSize();
        sal_Int32 mnLeft;
        sal_Int32 mnTop;
        sal_Int32 mnRight;
        sal_Int32 mnBottom;
    };

    class RendererPaneStyle
    {
    public:
        RendererPaneStyle (
            const std::shared_ptr<PresenterTheme>& rpTheme,
            const OUString& rsStyleName);

        awt::Rectangle AddBorder (
            const awt::Rectangle& rBox,
            drawing::framework::BorderType eBorderType) const;
        awt::Rectangle RemoveBorder (
            const awt::Rectangle& rBox,
            drawing::framework::BorderType eBorderType) const;
        Reference<rendering::XCanvasFont> GetFont (
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
        SharedBitmapDescriptor const mpEmpty;
        PresenterTheme::SharedFontDescriptor mpFont;
        sal_Int32 mnFontXOffset;
        sal_Int32 mnFontYOffset;
        enum class Anchor { Left, Right, Center };
        Anchor meFontAnchor;
        BorderSize maInnerBorderSize;
        BorderSize maOuterBorderSize;
        BorderSize maTotalBorderSize;
    private:
        void UpdateBorderSizes();
        SharedBitmapDescriptor GetBitmap(
            const std::shared_ptr<PresenterTheme>& rpTheme,
            const OUString& rsStyleName,
            const OUString& rsBitmapName);
    };
}

class  PresenterPaneBorderPainter::Renderer
{
public:
    Renderer (
        const Reference<XComponentContext>& rxContext,
        const std::shared_ptr<PresenterTheme>& rpTheme);

    void SetCanvas (const Reference<rendering::XCanvas>& rxCanvas);
    void PaintBorder (
        const OUString& rsTitle,
        const awt::Rectangle& rBBox,
        const awt::Rectangle& rUpdateBox,
        const OUString& rsPaneURL);
    void PaintTitle (
        const OUString& rsTitle,
        const std::shared_ptr<RendererPaneStyle>& rpStyle,
        const awt::Rectangle& rUpdateBox,
        const awt::Rectangle& rOuterBox,
        const awt::Rectangle& rInnerBox);
    void SetupClipping (
        const awt::Rectangle& rUpdateBox,
        const awt::Rectangle& rOuterBox,
        const OUString& rsPaneStyleName);
    std::shared_ptr<RendererPaneStyle> GetRendererPaneStyle (const OUString& rsResourceURL);
    void SetCalloutAnchor (
        const awt::Point& rCalloutAnchor);

private:
    std::shared_ptr<PresenterTheme> mpTheme;
    typedef ::std::map<OUString, std::shared_ptr<RendererPaneStyle> > RendererPaneStyleContainer;
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
        const SharedBitmapDescriptor& rpBitmap);
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

PresenterPaneBorderPainter::~PresenterPaneBorderPainter()
{
}

//----- XPaneBorderPainter ----------------------------------------------------

awt::Rectangle SAL_CALL PresenterPaneBorderPainter::addBorder (
    const OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    drawing::framework::BorderType eBorderType)
{
    ThrowIfDisposed();

    ProvideTheme();

    return AddBorder(rsPaneBorderStyleName, rRectangle, eBorderType);
}

awt::Rectangle SAL_CALL PresenterPaneBorderPainter::removeBorder (
    const OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    drawing::framework::BorderType eBorderType)
{
    ThrowIfDisposed();

    ProvideTheme();

    return RemoveBorder(rsPaneBorderStyleName, rRectangle, eBorderType);
}

void SAL_CALL PresenterPaneBorderPainter::paintBorder (
    const OUString& rsPaneBorderStyleName,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rOuterBorderRectangle,
    const css::awt::Rectangle& rRepaintArea,
    const OUString& rsTitle)
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

    if (mpRenderer == nullptr)
        return;

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

void SAL_CALL PresenterPaneBorderPainter::paintBorderWithCallout (
    const OUString& rsPaneBorderStyleName,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rOuterBorderRectangle,
    const css::awt::Rectangle& rRepaintArea,
    const OUString& rsTitle,
    const css::awt::Point& rCalloutAnchor)
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

    if (mpRenderer == nullptr)
        return;

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

awt::Point SAL_CALL PresenterPaneBorderPainter::getCalloutOffset (
    const OUString& rsPaneBorderStyleName)
{
    ThrowIfDisposed();
    ProvideTheme();
    if (mpRenderer != nullptr)
    {
        const std::shared_ptr<RendererPaneStyle> pRendererPaneStyle(
            mpRenderer->GetRendererPaneStyle(rsPaneBorderStyleName));
        if (pRendererPaneStyle != nullptr && pRendererPaneStyle->mpBottomCallout.get() != nullptr)
        {
            return awt::Point (
                0,
                pRendererPaneStyle->mpBottomCallout->mnHeight
                    - pRendererPaneStyle->mpBottomCallout->mnYHotSpot);
        }
    }

    return awt::Point(0,0);
}


bool PresenterPaneBorderPainter::ProvideTheme (const Reference<rendering::XCanvas>& rxCanvas)
{
    bool bModified (false);

    if ( ! mxContext.is())
        return false;

    if (mpTheme != nullptr)
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
        mpTheme.reset(new PresenterTheme(mxContext, rxCanvas));
        bModified = true;
    }

    if (bModified)
    {
        if (mpRenderer == nullptr)
            mpRenderer.reset(new Renderer(mxContext, mpTheme));
        else
            mpRenderer->SetCanvas(rxCanvas);
    }

    return bModified;
}

void PresenterPaneBorderPainter::ProvideTheme()
{
    if (mpTheme == nullptr)
    {
        // Create a theme without bitmaps (no canvas => no bitmaps).
        ProvideTheme(nullptr);
    }
        // When there already is a theme then without a canvas we can not
        // add anything new.
}

void PresenterPaneBorderPainter::SetTheme (const std::shared_ptr<PresenterTheme>& rpTheme)
{
    mpTheme = rpTheme;
    if (mpRenderer == nullptr)
        mpRenderer.reset(new Renderer(mxContext, mpTheme));
}

awt::Rectangle PresenterPaneBorderPainter::AddBorder (
    const OUString& rsPaneURL,
    const awt::Rectangle& rInnerBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    if (mpRenderer != nullptr)
    {
        const std::shared_ptr<RendererPaneStyle> pRendererPaneStyle(mpRenderer->GetRendererPaneStyle(rsPaneURL));
        if (pRendererPaneStyle != nullptr)
            return pRendererPaneStyle->AddBorder(rInnerBox, eBorderType);
    }
    return rInnerBox;
}

awt::Rectangle PresenterPaneBorderPainter::RemoveBorder (
    const OUString& rsPaneURL,
    const css::awt::Rectangle& rOuterBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    if (mpRenderer != nullptr)
    {
        const std::shared_ptr<RendererPaneStyle> pRendererPaneStyle(mpRenderer->GetRendererPaneStyle(rsPaneURL));
        if (pRendererPaneStyle != nullptr)
            return pRendererPaneStyle->RemoveBorder(rOuterBox, eBorderType);
    }
    return rOuterBox;
}

void PresenterPaneBorderPainter::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterPaneBorderPainter object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

//===== PresenterPaneBorderPainter::Renderer =====================================

PresenterPaneBorderPainter::Renderer::Renderer (
    const Reference<XComponentContext>& rxContext,
    const std::shared_ptr<PresenterTheme>& rpTheme)
    : mpTheme(rpTheme),
      maRendererPaneStyles(),
      mxCanvas(),
      mxPresenterHelper(),
      maViewState(geometry::AffineMatrix2D(1,0,0, 0,1,0), nullptr),
      mxViewStateClip(),
      mbHasCallout(false),
      maCalloutAnchor()
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPresenterHelper.set(
            xFactory->createInstanceWithContext(
                "com.sun.star.comp.Draw.PresenterHelper",
                rxContext),
            UNO_QUERY_THROW);
    }
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
    std::shared_ptr<RendererPaneStyle> pStyle (GetRendererPaneStyle(rsPaneURL));
    if (pStyle == nullptr)
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

    // Paint the sides.
    PaintBitmap(aCenterBox, rUpdateBox, 0,-1,
        pTopLeft->mnXOffset, pTopRight->mnXOffset, true, pTop);
    PaintBitmap(aCenterBox, rUpdateBox, -1,0,
        pTopLeft->mnYOffset, pBottomLeft->mnYOffset, true, pLeft);
    PaintBitmap(aCenterBox, rUpdateBox, +1,0,
        pTopRight->mnYOffset, pBottomRight->mnYOffset, true, pRight);
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
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1, nCalloutX,0, false, pStyle->mpBottomCallout);
        // Paint regular bottom bitmap left and right.
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1,
            pBottomLeft->mnXOffset, nCalloutX-aCenterBox.Width, true, pBottom);
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1,
            nCalloutX+nCalloutWidth, pBottomRight->mnXOffset, true, pBottom);
    }
    else
    {
        // Stretch the bottom bitmap over the full width.
        PaintBitmap(aCenterBox, rUpdateBox, 0,+1,
            pBottomLeft->mnXOffset, pBottomRight->mnXOffset, true, pBottom);
    }

    // Paint the corners.
    PaintBitmap(aCenterBox, rUpdateBox, -1,-1, 0,0, false, pTopLeft);
    PaintBitmap(aCenterBox, rUpdateBox, +1,-1, 0,0, false, pTopRight);
    PaintBitmap(aCenterBox, rUpdateBox, -1,+1, 0,0, false, pBottomLeft);
    PaintBitmap(aCenterBox, rUpdateBox, +1,+1, 0,0, false, pBottomRight);

    // Paint the title.
    PaintTitle(rsTitle, pStyle, rUpdateBox, aOuterBox, aInnerBox);

    // In a double buffering environment request to make the changes visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(false);
}

void PresenterPaneBorderPainter::Renderer::PaintTitle (
    const OUString& rsTitle,
    const std::shared_ptr<RendererPaneStyle>& rpStyle,
    const awt::Rectangle& rUpdateBox,
    const awt::Rectangle& rOuterBox,
    const awt::Rectangle& rInnerBox)
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

    /// this is responsible of the texts above the slide windows
    geometry::RealRectangle2D aBox (xLayout->queryTextBounds());
    const double nTextHeight = aBox.Y2 - aBox.Y1;
    const double nTextWidth = aBox.X1 + aBox.X2;
    const sal_Int32 nTitleBarHeight = rInnerBox.Y - rOuterBox.Y - 1;
    double nY = rOuterBox.Y + (nTitleBarHeight - nTextHeight) / 2 - aBox.Y1;
    if (nY >= rInnerBox.Y)
        nY = rInnerBox.Y - 1;
    double nX;
    switch (rpStyle->meFontAnchor)
    {
    case RendererPaneStyle::Anchor::Left:
        nX = rInnerBox.X;
        break;
    case RendererPaneStyle::Anchor::Right:
        nX = rInnerBox.X + rInnerBox.Width - nTextWidth;
        break;
    default: // RendererPaneStyle::Anchor::Center
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
        nullptr,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    PresenterCanvasHelper::SetDeviceColor(
            aRenderState,
            rpStyle->mpFont->mnColor);

    mxCanvas->drawTextLayout (
            xLayout,
            maViewState,
            aRenderState);
}

std::shared_ptr<RendererPaneStyle>
    PresenterPaneBorderPainter::Renderer::GetRendererPaneStyle (const OUString& rsResourceURL)
{
    OSL_ASSERT(mpTheme != nullptr);

    RendererPaneStyleContainer::const_iterator iStyle (maRendererPaneStyles.find(rsResourceURL));
    if (iStyle == maRendererPaneStyles.end())
    {
        OUString sPaneStyleName ("DefaultRendererPaneStyle");

        // Get pane layout name for resource URL.
        const OUString sStyleName (mpTheme->GetStyleName(rsResourceURL));
        if (!sStyleName.isEmpty())
            sPaneStyleName = sStyleName;

        // Create a new pane style object and initialize it with bitmaps.
        std::shared_ptr<RendererPaneStyle> pStyle (
            new RendererPaneStyle(mpTheme,sPaneStyleName));
        iStyle = maRendererPaneStyles.emplace(rsResourceURL, pStyle).first;
    }
    if (iStyle != maRendererPaneStyles.end())
        return iStyle->second;
    else
        return std::shared_ptr<RendererPaneStyle>();
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
    const SharedBitmapDescriptor& rpBitmap)
{
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
        nullptr,
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
    mxViewStateClip = nullptr;
    maViewState.Clip = nullptr;

    if ( ! mxCanvas.is())
        return;

    std::shared_ptr<RendererPaneStyle> pStyle (GetRendererPaneStyle(rsPaneStyleName));
    if (pStyle == nullptr)
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

BorderSize::BorderSize()
    : mnLeft(0),
      mnTop(0),
      mnRight(0),
      mnBottom(0)
{
}

//===== RendererPaneStyle  ============================================================

RendererPaneStyle::RendererPaneStyle (
    const std::shared_ptr<PresenterTheme>& rpTheme,
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
      mpEmpty(new PresenterBitmapDescriptor()),
      mpFont(),
      mnFontXOffset(0),
      mnFontYOffset(0),
      meFontAnchor(Anchor::Center),
      maInnerBorderSize(),
      maOuterBorderSize(),
      maTotalBorderSize()
{
    if (rpTheme == nullptr)
        return;

    mpTopLeft = GetBitmap(rpTheme, rsStyleName, "TopLeft");
    mpTop = GetBitmap(rpTheme, rsStyleName, "Top");
    mpTopRight = GetBitmap(rpTheme, rsStyleName, "TopRight");
    mpLeft = GetBitmap(rpTheme, rsStyleName,"Left");
    mpRight = GetBitmap(rpTheme, rsStyleName, "Right");
    mpBottomLeft = GetBitmap(rpTheme, rsStyleName, "BottomLeft");
    mpBottom = GetBitmap(rpTheme, rsStyleName, "Bottom");
    mpBottomRight = GetBitmap(rpTheme, rsStyleName, "BottomRight");
    mpBottomCallout = GetBitmap(rpTheme, rsStyleName, "BottomCallout");

    // Get font description.
    mpFont = rpTheme->GetFont(rsStyleName);

    OUString sAnchor ("Left");
    if (mpFont.get() != nullptr)
    {
        sAnchor = mpFont->msAnchor;
        mnFontXOffset = mpFont->mnXOffset;
        mnFontYOffset = mpFont->mnYOffset;
    }

    if ( sAnchor == "Left" )
        meFontAnchor = Anchor::Left;
    else if ( sAnchor == "Right" )
        meFontAnchor = Anchor::Right;
    else
        meFontAnchor = Anchor::Center;

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

awt::Rectangle RendererPaneStyle::AddBorder (
    const awt::Rectangle& rBox,
    const drawing::framework::BorderType eBorderType) const
{
    const BorderSize* pBorderSize = nullptr;
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
    const BorderSize* pBorderSize = nullptr;
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

Reference<rendering::XCanvasFont> RendererPaneStyle::GetFont (
    const Reference<rendering::XCanvas>& rxCanvas) const
{
    if (mpFont)
    {
        mpFont->PrepareFont(rxCanvas);
        return mpFont->mxFont;
    }
    return Reference<rendering::XCanvasFont>();
}

void RendererPaneStyle::UpdateBorderSizes()
{
    maTotalBorderSize.mnLeft = maInnerBorderSize.mnLeft + maOuterBorderSize.mnLeft;
    maTotalBorderSize.mnTop = maInnerBorderSize.mnTop + maOuterBorderSize.mnTop;
    maTotalBorderSize.mnRight = maInnerBorderSize.mnRight + maOuterBorderSize.mnRight;
    maTotalBorderSize.mnBottom = maInnerBorderSize.mnBottom + maOuterBorderSize.mnBottom;
}

SharedBitmapDescriptor RendererPaneStyle::GetBitmap(
    const std::shared_ptr<PresenterTheme>& rpTheme,
    const OUString& rsStyleName,
    const OUString& rsBitmapName)
{
    SharedBitmapDescriptor pDescriptor (rpTheme->GetBitmap(rsStyleName, rsBitmapName));
    if (pDescriptor.get() != nullptr)
        return pDescriptor;
    else
        return mpEmpty;
}

} // end of anonymous namespace

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
