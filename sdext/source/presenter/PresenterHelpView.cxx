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

#include "PresenterHelpView.hxx"
#include "PresenterButton.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterHelper.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/util/Color.hpp>
#include <algorithm>
#include <vector>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;
using ::std::vector;

#define A2S(pString) (::rtl::OUString(pString))

namespace sdext { namespace presenter {

namespace {
    const static sal_Int32 gnHorizontalGap (20);
    const static sal_Int32 gnVerticalBorder (30);
    const static sal_Int32 gnVerticalButtonPadding (12);

    class LineDescriptor
    {
    public:
        LineDescriptor(void);
        void AddPart (
            const OUString& rsLine,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont);
        bool IsEmpty (void) const;

        OUString msLine;
        geometry::RealSize2D maSize;
        double mnVerticalOffset;

        void CalculateSize (const css::uno::Reference<css::rendering::XCanvasFont>& rxFont);
    };

    class LineDescriptorList
    {
    public:
        LineDescriptorList (
            const OUString& rsText,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);

        void Update (
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);

        double Paint(
            const Reference<rendering::XCanvas>& rxCanvas,
            const geometry::RealRectangle2D& rBBox,
            const bool bFlushLeft,
            const rendering::ViewState& rViewState,
            rendering::RenderState& rRenderState,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont) const;
        double GetHeight (void) const;

    private:
        const OUString msText;
        ::boost::shared_ptr<vector<LineDescriptor> > mpLineDescriptors;

        void SplitText (const ::rtl::OUString& rsText, vector<rtl::OUString>& rTextParts);
        void FormatText (
            const vector<rtl::OUString>& rTextParts,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);
    };

    class Block
    {
    public:
        Block (const Block& rBlock);
        Block (
            const OUString& rsLeftText,
            const OUString& rsRightText,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);
        void Update (
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);

        LineDescriptorList maLeft;
        LineDescriptorList maRight;
    };
} // end of anonymous namespace

class PresenterHelpView::TextContainer : public vector<boost::shared_ptr<Block> >
{
};

PresenterHelpView::PresenterHelpView (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterHelpViewInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxViewId(rxViewId),
      mxPane(),
      mxWindow(),
      mxCanvas(),
      mpPresenterController(rpPresenterController),
      mpFont(),
      mpTextContainer(),
      mpCloseButton(),
      mnSeparatorY(0),
      mnMaximalWidth(0)
{
    try
    {
        // Get the content window via the pane anchor.
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC (
            xCM->getConfigurationController(), UNO_QUERY_THROW);
        mxPane = Reference<XPane>(xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);

        mxWindow = mxPane->getWindow();
        ProvideCanvas();

        mxWindow->addWindowListener(this);
        mxWindow->addPaintListener(this);
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->setBackground(util::Color(0xff000000));
        mxWindow->setVisible(sal_True);

        if (mpPresenterController.is())
        {
            mpFont = mpPresenterController->GetViewFont(mxViewId->getResourceURL());
            if (mpFont.get() != NULL)
            {
                mpFont->PrepareFont(mxCanvas);
            }
        }

        // Create the close button.
        mpCloseButton = PresenterButton::Create(
            mxComponentContext,
            mpPresenterController,
            mpPresenterController->GetTheme(),
            mxWindow,
            mxCanvas,
            A2S("HelpViewCloser"));

        ReadHelpStrings();
        Resize();
    }
    catch (RuntimeException&)
    {
        mxViewId = NULL;
        mxWindow = NULL;
        throw;
    }
}

PresenterHelpView::~PresenterHelpView (void)
{
}

void SAL_CALL PresenterHelpView::disposing (void)
{
    mxViewId = NULL;

    if (mpCloseButton.is())
    {
        Reference<lang::XComponent> xComponent (
            static_cast<XWeak*>(mpCloseButton.get()), UNO_QUERY);
        mpCloseButton = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
    }
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterHelpView::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxCanvas)
    {
        mxCanvas = NULL;
    }
    else if (rEventObject.Source == mxWindow)
    {
        mxWindow = NULL;
        dispose();
    }
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterHelpView::windowResized (const awt::WindowEvent& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    Resize();
}

void SAL_CALL PresenterHelpView::windowMoved (const awt::WindowEvent& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

void SAL_CALL PresenterHelpView::windowShown (const lang::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    Resize();
}

void SAL_CALL PresenterHelpView::windowHidden (const lang::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterHelpView::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    Paint(rEvent.UpdateRect);
}

void PresenterHelpView::Paint (const awt::Rectangle& rUpdateBox)
{
    ProvideCanvas();
    if ( ! mxCanvas.is())
        return;

    // Clear background.
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    mpPresenterController->GetCanvasHelper()->Paint(
        mpPresenterController->GetViewBackground(mxViewId->getResourceURL()),
        Reference<rendering::XCanvas>(mxCanvas, UNO_QUERY),
        rUpdateBox,
        awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height),
        awt::Rectangle());

    // Paint vertical divider.

    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, mxCanvas->getDevice()));

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, mpFont->mnColor);

    mxCanvas->drawLine(
        geometry::RealPoint2D(aWindowBox.Width/2, gnVerticalBorder),
        geometry::RealPoint2D(aWindowBox.Width/2, mnSeparatorY - gnVerticalBorder),
        aViewState,
        aRenderState);

    // Paint the horizontal separator.
    mxCanvas->drawLine(
        geometry::RealPoint2D(0, mnSeparatorY),
        geometry::RealPoint2D(aWindowBox.Width, mnSeparatorY),
        aViewState,
        aRenderState);

    // Paint text.
    double nY (gnVerticalBorder);
    TextContainer::const_iterator iBlock (mpTextContainer->begin());
    TextContainer::const_iterator iBlockEnd (mpTextContainer->end());
    for ( ; iBlock!=iBlockEnd; ++iBlock)
    {
        const double nLeftHeight (
            (*iBlock)->maLeft.Paint(mxCanvas,
                geometry::RealRectangle2D(
                    gnHorizontalGap,
                    nY,
                    aWindowBox.Width/2 - gnHorizontalGap,
                    aWindowBox.Height - gnVerticalBorder),
                false,
                aViewState,
                aRenderState,
                mpFont->mxFont));
        const double nRightHeight (
            (*iBlock)->maRight.Paint(mxCanvas,
                geometry::RealRectangle2D(
                    aWindowBox.Width/2 + gnHorizontalGap,
                    nY,
                    aWindowBox.Width - gnHorizontalGap,
                    aWindowBox.Height - gnVerticalBorder),
                true,
                aViewState,
                aRenderState,
                mpFont->mxFont));
        nY += ::std::max(nLeftHeight,nRightHeight);
    }

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}

void PresenterHelpView::ReadHelpStrings (void)
{
    mpTextContainer.reset(new TextContainer());
    PresenterConfigurationAccess aConfiguration (
        mxComponentContext,
        OUString("/org.openoffice.Office.extension.PresenterScreen/"),
        PresenterConfigurationAccess::READ_ONLY);
    Reference<container::XNameAccess> xStrings (
        aConfiguration.GetConfigurationNode(A2S("PresenterScreenSettings/HelpView/HelpStrings")),
        UNO_QUERY);
    PresenterConfigurationAccess::ForAll(
        xStrings,
        ::boost::bind(&PresenterHelpView::ProcessString, this, _2));
}

void PresenterHelpView::ProcessString (
    const Reference<beans::XPropertySet>& rsProperties)
{
    if ( ! rsProperties.is())
        return;

    OUString sLeftText;
    PresenterConfigurationAccess::GetProperty(rsProperties, A2S("Left")) >>= sLeftText;
    OUString sRightText;
    PresenterConfigurationAccess::GetProperty(rsProperties, A2S("Right")) >>= sRightText;
    mpTextContainer->push_back(
        ::boost::shared_ptr<Block>(
            new Block(sLeftText, sRightText, mpFont->mxFont, mnMaximalWidth)));
}

void PresenterHelpView::CheckFontSize (void)
{
    if (mpFont.get() == NULL)
        return;

    sal_Int32 nBestSize (6);

    // Scaling down and then reformatting can cause the text to be too large
    // still.  So do this again and again until the text size is
    // small enough.  Restrict the number of loops.
    for (int nLoopCount=0; nLoopCount<5; ++nLoopCount)
    {
        double nY (0.0);
        TextContainer::iterator iBlock (mpTextContainer->begin());
        TextContainer::const_iterator iBlockEnd (mpTextContainer->end());
        for ( ; iBlock!=iBlockEnd; ++iBlock)
            nY += ::std::max(
                (*iBlock)->maLeft.GetHeight(),
                (*iBlock)->maRight.GetHeight());

        const double nHeightDifference (nY - (mnSeparatorY-gnVerticalBorder));
        if (nHeightDifference <= 0 && nHeightDifference > -50)
        {
            // We have found a good font size that is large and leaves not
            // too much space below the help text.
            return;
        }

        // Use a simple linear transformation to calculate initial guess of
        // a size that lets all help text be shown inside the window.
        const double nScale (double(mnSeparatorY-gnVerticalBorder) / nY);
        if (nScale > 1.0 && nScale < 1.05)
            break;

        sal_Int32 nFontSizeGuess (sal_Int32(mpFont->mnSize * nScale));
        if (nHeightDifference<=0 && mpFont->mnSize>nBestSize)
            nBestSize = mpFont->mnSize;
        mpFont->mnSize = nFontSizeGuess;
        mpFont->mxFont = NULL;
        mpFont->PrepareFont(mxCanvas);

        // Reformat blocks.
        for (iBlock=mpTextContainer->begin(); iBlock!=iBlockEnd; ++iBlock)
            (*iBlock)->Update(mpFont->mxFont, mnMaximalWidth);
    }

    if (nBestSize != mpFont->mnSize)
    {
        mpFont->mnSize = nBestSize;
        mpFont->mxFont = NULL;
        mpFont->PrepareFont(mxCanvas);

        // Reformat blocks.
        for (TextContainer::iterator
                 iBlock (mpTextContainer->begin()),
                 iEnd (mpTextContainer->end());
             iBlock!=iEnd;
             ++iBlock)
        {
            (*iBlock)->Update(mpFont->mxFont, mnMaximalWidth);
        }
    }
}

//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterHelpView::getResourceId (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxViewId;
}

sal_Bool SAL_CALL PresenterHelpView::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}

//-----------------------------------------------------------------------------

void PresenterHelpView::ProvideCanvas (void)
{
    if ( ! mxCanvas.is() && mxPane.is())
    {
        mxCanvas = mxPane->getCanvas();
        if ( ! mxCanvas.is())
            return;
        Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(static_cast<awt::XPaintListener*>(this));

        if (mpCloseButton.is())
            mpCloseButton->SetCanvas(mxCanvas, mxWindow);
    }
}

void PresenterHelpView::Resize (void)
{
    if (mpCloseButton.get() != NULL && mxWindow.is())
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        mnMaximalWidth = (mxWindow->getPosSize().Width - 4*gnHorizontalGap) / 2;

        // Place vertical separator.
        mnSeparatorY = aWindowBox.Height
            - mpCloseButton->GetSize().Height - gnVerticalButtonPadding;

        mpCloseButton->SetCenter(geometry::RealPoint2D(
            aWindowBox.Width/2,
            aWindowBox.Height - mpCloseButton->GetSize().Height/2));

        CheckFontSize();
    }
}

void PresenterHelpView::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString( "PresenterHelpView has been already disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

//===== LineDescritor =========================================================

namespace {

LineDescriptor::LineDescriptor (void)
    : msLine(),
      maSize(0,0),
      mnVerticalOffset(0)
{
}

void LineDescriptor::AddPart (
    const OUString& rsLine,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont)
{
    msLine += rsLine;

    CalculateSize(rxFont);
}

bool LineDescriptor::IsEmpty (void) const
{
    return msLine.isEmpty();
}

void LineDescriptor::CalculateSize (
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont)
{
    OSL_ASSERT(rxFont.is());

    rendering::StringContext aContext (msLine, 0, msLine.getLength());
    Reference<rendering::XTextLayout> xLayout (
        rxFont->createTextLayout(aContext, rendering::TextDirection::WEAK_LEFT_TO_RIGHT, 0));
    const geometry::RealRectangle2D aTextBBox (xLayout->queryTextBounds());
    maSize = css::geometry::RealSize2D(aTextBBox.X2 - aTextBBox.X1, aTextBBox.Y2 - aTextBBox.Y1);
    mnVerticalOffset = aTextBBox.Y2;
}

} // end of anonymous namespace

//===== LineDescriptorList ====================================================

namespace {

LineDescriptorList::LineDescriptorList (
    const OUString& rsText,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const sal_Int32 nMaximalWidth)
    : msText(rsText)
{
    Update(rxFont, nMaximalWidth);
}

double LineDescriptorList::Paint(
    const Reference<rendering::XCanvas>& rxCanvas,
    const geometry::RealRectangle2D& rBBox,
    const bool bFlushLeft,
    const rendering::ViewState& rViewState,
    rendering::RenderState& rRenderState,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont) const
{
    if ( ! rxCanvas.is())
        return 0;

    double nY (rBBox.Y1);
    vector<LineDescriptor>::const_iterator iLine (mpLineDescriptors->begin());
    vector<LineDescriptor>::const_iterator iEnd (mpLineDescriptors->end());
    for ( ; iLine!=iEnd; ++iLine)
    {
        double nX (rBBox.X1);
        if ( ! bFlushLeft)
            nX = rBBox.X2 - iLine->maSize.Width;
        rRenderState.AffineTransform.m02 = nX;
        rRenderState.AffineTransform.m12 = nY + iLine->maSize.Height - iLine->mnVerticalOffset;

        const rendering::StringContext aContext (iLine->msLine, 0, iLine->msLine.getLength());

        rxCanvas->drawText (
            aContext,
            rxFont,
            rViewState,
            rRenderState,
            rendering::TextDirection::WEAK_LEFT_TO_RIGHT);

        nY += iLine->maSize.Height * 1.2;
    }

    return nY - rBBox.Y1;
}

double LineDescriptorList::GetHeight (void) const
{
    double nHeight (0);
    vector<LineDescriptor>::const_iterator iLine (mpLineDescriptors->begin());
    vector<LineDescriptor>::const_iterator iEnd (mpLineDescriptors->end());
    for ( ; iLine!=iEnd; ++iLine)
        nHeight += iLine->maSize.Height * 1.2;

    return nHeight;
}

void LineDescriptorList::Update (
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const sal_Int32 nMaximalWidth)
{
    vector<OUString> aTextParts;
    SplitText(msText, aTextParts);
    FormatText(aTextParts, rxFont, nMaximalWidth);
}

void LineDescriptorList::SplitText (
    const OUString& rsText,
    vector<OUString>& rTextParts)
{
    const sal_Char cQuote ('\'');
    const sal_Char cSeparator (',');

    sal_Int32 nIndex (0);
    sal_Int32 nStart (0);
    sal_Int32 nLength (rsText.getLength());
    bool bIsQuoted (false);
    while (nIndex < nLength)
    {
        const sal_Int32 nQuoteIndex (rsText.indexOf(cQuote, nIndex));
        const sal_Int32 nSeparatorIndex (rsText.indexOf(cSeparator, nIndex));
        if (nQuoteIndex>=0 && (nSeparatorIndex==-1 || nQuoteIndex<nSeparatorIndex))
        {
            bIsQuoted = !bIsQuoted;
            nIndex = nQuoteIndex+1;
            continue;
        }

        const sal_Int32 nNextIndex = nSeparatorIndex;
        if (nNextIndex < 0)
        {
            break;
        }
        else if ( ! bIsQuoted)
        {
            rTextParts.push_back(rsText.copy(nStart, nNextIndex-nStart));
            nStart = nNextIndex + 1;
        }
        nIndex = nNextIndex+1;
    }
    if (nStart < nLength)
        rTextParts.push_back(rsText.copy(nStart, nLength-nStart));
}

void LineDescriptorList::FormatText (
    const vector<OUString>& rTextParts,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const sal_Int32 nMaximalWidth)
{
    LineDescriptor aLineDescriptor;

    mpLineDescriptors.reset(new vector<LineDescriptor>());

    vector<OUString>::const_iterator iPart (rTextParts.begin());
    vector<OUString>::const_iterator iEnd (rTextParts.end());
    while (iPart!=iEnd)
    {
        if (aLineDescriptor.IsEmpty())
        {
            // Avoid empty lines.
            if (PresenterCanvasHelper::GetTextSize(
                rxFont, *iPart).Width > nMaximalWidth)
            {
                const sal_Char cSpace (' ');

                sal_Int32 nIndex (0);
                sal_Int32 nStart (0);
                sal_Int32 nLength (iPart->getLength());
                while (nIndex < nLength)
                {
                    sal_Int32  nSpaceIndex (iPart->indexOf(cSpace, nIndex));
                    while (nSpaceIndex >= 0 && PresenterCanvasHelper::GetTextSize(
                        rxFont, iPart->copy(nStart, nSpaceIndex-nStart)).Width <= nMaximalWidth)
                    {
                        nIndex = nSpaceIndex;
                        nSpaceIndex = iPart->indexOf(cSpace, nIndex+1);
                    }

                    if (nSpaceIndex < 0 && PresenterCanvasHelper::GetTextSize(
                        rxFont, iPart->copy(nStart, nLength-nStart)).Width <= nMaximalWidth)
                    {
                        nIndex = nLength;
                    }

                    if (nIndex == nStart)
                    {
                        nIndex = nLength;
                    }

                    aLineDescriptor.AddPart(iPart->copy(nStart, nIndex-nStart), rxFont);
                    if (nIndex != nLength)
                    {
                        mpLineDescriptors->push_back(aLineDescriptor);
                        aLineDescriptor = LineDescriptor();
                    }
                    nStart = nIndex;
                }
            }
            else
            {
                aLineDescriptor.AddPart(*iPart, rxFont);
            }
        }
        else if (PresenterCanvasHelper::GetTextSize(
            rxFont, aLineDescriptor.msLine+A2S(", ")+*iPart).Width > nMaximalWidth)
        {
            aLineDescriptor.AddPart(A2S(","), rxFont);
            mpLineDescriptors->push_back(aLineDescriptor);
            aLineDescriptor = LineDescriptor();
            continue;
        }
        else
        {
            aLineDescriptor.AddPart(A2S(", ")+*iPart, rxFont);
        }
        ++iPart;
    }
    if ( ! aLineDescriptor.IsEmpty())
    {
        mpLineDescriptors->push_back(aLineDescriptor);
    }
}

} // end of anonymous namespace

//===== Block =================================================================

namespace {

Block::Block (
    const OUString& rsLeftText,
    const OUString& rsRightText,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const sal_Int32 nMaximalWidth)
    : maLeft(rsLeftText, rxFont, nMaximalWidth),
      maRight(rsRightText, rxFont, nMaximalWidth)
{
}

void Block::Update (
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const sal_Int32 nMaximalWidth)
{
    maLeft.Update(rxFont, nMaximalWidth);
    maRight.Update(rxFont, nMaximalWidth);
}

} // end of anonymous namespace

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
