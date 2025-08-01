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

#include <utility>
#include <vcl/settings.hxx>
#include "PresenterHelpView.hxx"
#include "PresenterButton.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include <DrawController.hxx>
#include <framework/ConfigurationController.hxx>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/util/Color.hpp>
#include <algorithm>
#include <numeric>
#include <string_view>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::std::vector;

namespace sdext::presenter {

namespace {
    const sal_Int32 gnHorizontalGap (20);
    const sal_Int32 gnVerticalBorder (30);
    const sal_Int32 gnVerticalButtonPadding (12);

    class LineDescriptor
    {
    public:
        LineDescriptor();
        void AddPart (
            std::u16string_view rsLine,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont);
        bool IsEmpty() const;

        OUString msLine;
        geometry::RealSize2D maSize;
        double mnVerticalOffset;

        void CalculateSize (const css::uno::Reference<css::rendering::XCanvasFont>& rxFont);
    };

    class LineDescriptorList
    {
    public:
        LineDescriptorList (
            OUString sText,
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
        double GetHeight() const;

    private:
        const OUString msText;
        std::shared_ptr<vector<LineDescriptor> > mpLineDescriptors;

        static void SplitText (std::u16string_view rsText, vector<OUString>& rTextParts);
        void FormatText (
            const vector<OUString>& rTextParts,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);
    };

    class Block
    {
    public:
        Block (
            const OUString& rsLeftText,
            const OUString& rsRightText,
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);
        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;
        void Update (
            const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
            const sal_Int32 nMaximalWidth);

        LineDescriptorList maLeft;
        LineDescriptorList maRight;
    };
} // end of anonymous namespace

class PresenterHelpView::TextContainer : public vector<std::shared_ptr<Block> >
{
};

PresenterHelpView::PresenterHelpView (
    const Reference<uno::XComponentContext>& rxContext,
    const rtl::Reference<sd::framework::ResourceId>& rxViewId,
    const rtl::Reference<::sd::DrawController>& rxController,
    ::rtl::Reference<PresenterController> xPresenterController)
    : mxComponentContext(rxContext),
      mxViewId(rxViewId),
      mpPresenterController(std::move(xPresenterController)),
      mnSeparatorY(0),
      mnMaximalWidth(0)
{
    try
    {
        // Get the content window via the pane anchor.
        rtl::Reference<sd::framework::ConfigurationController> xCC (
            rxController->getConfigurationController());
        mxPane = dynamic_cast<sd::framework::AbstractPane*>(xCC->getResource(rxViewId->getAnchor()).get());

        mxWindow = mxPane->getWindow();
        ProvideCanvas();

        mxWindow->addWindowListener(this);
        mxWindow->addPaintListener(this);
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->setBackground(util::Color(0xff000000));
        mxWindow->setVisible(true);

        if (mpPresenterController.is())
        {
            mpFont = mpPresenterController->GetViewFont(mxViewId->getResourceURL());
            if (mpFont)
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
            u"HelpViewCloser"_ustr);

        ReadHelpStrings();
        Resize();
    }
    catch (RuntimeException&)
    {
        mxViewId = nullptr;
        mxWindow = nullptr;
        throw;
    }
}

PresenterHelpView::~PresenterHelpView()
{
}

void PresenterHelpView::disposing(std::unique_lock<std::mutex>&)
{
    mxViewId = nullptr;

    if (mpCloseButton.is())
    {
        rtl::Reference<PresenterButton> xComponent = std::move(mpCloseButton);
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
{
    if (rEventObject.Source == mxCanvas)
    {
        mxCanvas = nullptr;
    }
    else if (rEventObject.Source == mxWindow)
    {
        mxWindow = nullptr;
        dispose();
    }
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterHelpView::windowResized (const awt::WindowEvent&)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }
    Resize();
}

void SAL_CALL PresenterHelpView::windowMoved (const awt::WindowEvent&)
{
    std::unique_lock l(m_aMutex);
    throwIfDisposed(l);
}

void SAL_CALL PresenterHelpView::windowShown (const lang::EventObject&)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }
    Resize();
}

void SAL_CALL PresenterHelpView::windowHidden (const lang::EventObject&)
{
    std::unique_lock l(m_aMutex);
    throwIfDisposed(l);
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterHelpView::windowPaint (const css::awt::PaintEvent& rEvent)
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
        mxCanvas,
        rUpdateBox,
        awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height),
        awt::Rectangle());

    // Paint vertical divider.

    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, mxCanvas->getDevice()));

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        nullptr,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, mpFont->mnColor);

    mxCanvas->drawLine(
            geometry::RealPoint2D((aWindowBox.Width/2.0), gnVerticalBorder),
            geometry::RealPoint2D((aWindowBox.Width/2.0), mnSeparatorY - gnVerticalBorder),
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
    for (const auto& rxBlock : *mpTextContainer)
    {
        sal_Int32 LeftX1 = gnHorizontalGap;
        sal_Int32 LeftX2 = aWindowBox.Width/2 - gnHorizontalGap;
        sal_Int32 RightX1 = aWindowBox.Width/2 + gnHorizontalGap;
        sal_Int32 RightX2 = aWindowBox.Width - gnHorizontalGap;
        /* check whether RTL interface or not
           then replace the windowbox position */
        if(AllSettings::GetLayoutRTL())
        {
            LeftX1 = aWindowBox.Width/2 + gnHorizontalGap;
            LeftX2 = aWindowBox.Width - gnHorizontalGap;
            RightX1 = gnHorizontalGap;
            RightX2 = aWindowBox.Width/2 - gnHorizontalGap;
        }
        const double nLeftHeight (
            rxBlock->maLeft.Paint(mxCanvas,
                geometry::RealRectangle2D(
                        LeftX1,
                        nY,
                        LeftX2,
                        aWindowBox.Height - gnVerticalBorder),
                false,
                aViewState,
                aRenderState,
                mpFont->mxFont));
        const double nRightHeight (
            rxBlock->maRight.Paint(mxCanvas,
                geometry::RealRectangle2D(
                        RightX1,
                        nY,
                        RightX2,
                        aWindowBox.Height - gnVerticalBorder),
                true,
                aViewState,
                aRenderState,
                mpFont->mxFont));

        nY += ::std::max(nLeftHeight,nRightHeight);
    }

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(false);
}

void PresenterHelpView::ReadHelpStrings()
{
    mpTextContainer.reset(new TextContainer);
    PresenterConfigurationAccess aConfiguration (
        mxComponentContext,
        u"/org.openoffice.Office.PresenterScreen/"_ustr,
        PresenterConfigurationAccess::READ_ONLY);
    Reference<container::XNameAccess> xStrings (
        aConfiguration.GetConfigurationNode(u"PresenterScreenSettings/HelpView/HelpStrings"_ustr),
        UNO_QUERY);
    PresenterConfigurationAccess::ForAll(
        xStrings,
        [this](OUString const&, uno::Reference<beans::XPropertySet> const& xProps)
        {
            return this->ProcessString(xProps);
        });
}

void PresenterHelpView::ProcessString (
    const Reference<beans::XPropertySet>& rsProperties)
{
    if ( ! rsProperties.is())
        return;

    OUString sLeftText;
    PresenterConfigurationAccess::GetProperty(rsProperties, u"Left"_ustr) >>= sLeftText;
    OUString sRightText;
    PresenterConfigurationAccess::GetProperty(rsProperties, u"Right"_ustr) >>= sRightText;
    mpTextContainer->push_back(
        std::make_shared<Block>(
            sLeftText, sRightText, mpFont->mxFont, mnMaximalWidth));
}

void PresenterHelpView::CheckFontSize()
{
    if (!mpFont)
        return;

    sal_Int32 nBestSize (6);

    // Scaling down and then reformatting can cause the text to be too large
    // still.  So do this again and again until the text size is
    // small enough.  Restrict the number of loops.
    for (int nLoopCount=0; nLoopCount<5; ++nLoopCount)
    {
        double nY = std::accumulate(mpTextContainer->begin(), mpTextContainer->end(), double(0),
            [](const double& sum, const std::shared_ptr<Block>& rxBlock) {
                return sum + std::max(
                    rxBlock->maLeft.GetHeight(),
                    rxBlock->maRight.GetHeight());
            });

        const double nHeightDifference (nY - (mnSeparatorY-gnVerticalBorder));
        if (nHeightDifference <= 0 && nHeightDifference > -50)
        {
            // We have found a good font size that is large and leaves not
            // too much space below the help text.
            return;
        }

        // Use a simple linear transformation to calculate initial guess of
        // a size that lets all help text to be shown inside the window.
        const double nScale (double(mnSeparatorY-gnVerticalBorder) / nY);
        if (nScale > 1.0 && nScale < 1.05)
            break;

        sal_Int32 nFontSizeGuess (sal_Int32(mpFont->mnSize * nScale));
        if (nHeightDifference<=0 && mpFont->mnSize>nBestSize)
            nBestSize = mpFont->mnSize;
        mpFont->mnSize = nFontSizeGuess;
        mpFont->mxFont = nullptr;
        mpFont->PrepareFont(mxCanvas);

        // Reformat blocks.
        for (auto& rxBlock : *mpTextContainer)
            rxBlock->Update(mpFont->mxFont, mnMaximalWidth);
    }

    if (nBestSize != mpFont->mnSize)
    {
        mpFont->mnSize = nBestSize;
        mpFont->mxFont = nullptr;
        mpFont->PrepareFont(mxCanvas);

        // Reformat blocks.
        for (auto& rxBlock : *mpTextContainer)
        {
            rxBlock->Update(mpFont->mxFont, mnMaximalWidth);
        }
    }
}

//----- AbstractResource -----------------------------------------------------------

rtl::Reference<sd::framework::ResourceId> PresenterHelpView::getResourceId()
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }
    return mxViewId;
}

bool PresenterHelpView::isAnchorOnly()
{
    return false;
}


void PresenterHelpView::ProvideCanvas()
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

void PresenterHelpView::Resize()
{
    if (!(mpCloseButton && mxWindow.is()))
        return;

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    mnMaximalWidth = (mxWindow->getPosSize().Width - 4*gnHorizontalGap) / 2;

    // Place vertical separator.
    mnSeparatorY = aWindowBox.Height
        - mpCloseButton->GetSize().Height - gnVerticalButtonPadding;

    mpCloseButton->SetCenter(geometry::RealPoint2D(
        aWindowBox.Width/2.0,
        aWindowBox.Height - mpCloseButton->GetSize().Height/2.0));

    CheckFontSize();
}

//===== LineDescriptor =========================================================

namespace {

LineDescriptor::LineDescriptor()
    : maSize(0,0),
      mnVerticalOffset(0)
{
}

void LineDescriptor::AddPart (
    std::u16string_view rsLine,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont)
{
    msLine += rsLine;

    CalculateSize(rxFont);
}

bool LineDescriptor::IsEmpty() const
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
    OUString sText,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const sal_Int32 nMaximalWidth)
    : msText(std::move(sText))
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
    for (const auto& rLine : *mpLineDescriptors)
    {
        double nX;
        /// check whether RTL interface or not
        if(!AllSettings::GetLayoutRTL())
        {
            nX = rBBox.X1;
            if ( ! bFlushLeft)
                nX = rBBox.X2 - rLine.maSize.Width;
        }
        else
        {
            nX=rBBox.X2 - rLine.maSize.Width;
            if ( ! bFlushLeft)
                nX = rBBox.X1;
        }
        rRenderState.AffineTransform.m02 = nX;
        rRenderState.AffineTransform.m12 = nY + rLine.maSize.Height - rLine.mnVerticalOffset;

        const rendering::StringContext aContext (rLine.msLine, 0, rLine.msLine.getLength());
        Reference<rendering::XTextLayout> xLayout (
        rxFont->createTextLayout(aContext, rendering::TextDirection::WEAK_LEFT_TO_RIGHT, 0));
        rxCanvas->drawTextLayout (
            xLayout,
            rViewState,
            rRenderState);

        nY += rLine.maSize.Height * 1.2;
    }

    return nY - rBBox.Y1;
}

double LineDescriptorList::GetHeight() const
{
    return std::accumulate(mpLineDescriptors->begin(), mpLineDescriptors->end(), double(0),
        [](const double& nHeight, const LineDescriptor& rLine) {
            return nHeight + rLine.maSize.Height * 1.2;
        });
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
    std::u16string_view rsText,
    vector<OUString>& rTextParts)
{
    const char cQuote ('\'');
    const char cSeparator (',');

    size_t nIndex (0);
    size_t nStart (0);
    size_t nLength (rsText.size());
    bool bIsQuoted (false);
    while (nIndex < nLength)
    {
        const size_t nQuoteIndex (rsText.find(cQuote, nIndex));
        const size_t nSeparatorIndex (rsText.find(cSeparator, nIndex));
        if (nQuoteIndex != std::u16string_view::npos &&
            (nSeparatorIndex == std::u16string_view::npos || nQuoteIndex<nSeparatorIndex))
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
            rTextParts.push_back(OUString(rsText.substr(nStart, nNextIndex-nStart)));
            nStart = nNextIndex + 1;
        }
        nIndex = nNextIndex+1;
    }
    if (nStart < nLength)
        rTextParts.push_back(OUString(rsText.substr(nStart, nLength-nStart)));
}

void LineDescriptorList::FormatText (
    const vector<OUString>& rTextParts,
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const sal_Int32 nMaximalWidth)
{
    LineDescriptor aLineDescriptor;

    mpLineDescriptors = std::make_shared<vector<LineDescriptor>>();

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
                const char cSpace (' ');

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

                    aLineDescriptor.AddPart(iPart->subView(nStart, nIndex-nStart), rxFont);
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
            rxFont, aLineDescriptor.msLine+", "+*iPart).Width > nMaximalWidth)
        {
            aLineDescriptor.AddPart(u",", rxFont);
            mpLineDescriptors->push_back(aLineDescriptor);
            aLineDescriptor = LineDescriptor();
            continue;
        }
        else
        {
            aLineDescriptor.AddPart(Concat2View(", "+*iPart), rxFont);
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

} // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
