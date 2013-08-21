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

#include "vcl/svapp.hxx"
#include "PresenterToolBar.hxx"

#include "PresenterBitmapContainer.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterTimer.hxx"
#include "PresenterWindowManager.hxx"

#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <rtl/ustrbuf.hxx>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext { namespace presenter {

static const sal_Int32 gnGapSize (20);
static const sal_Int32 gnMinimalSeparatorSize (20);
static const sal_Int32 gnSeparatorInset (0);

namespace {

    class Text
    {
    public:
        Text (void);
        Text (const Text& rText);
        Text (
            const OUString& rsText,
            const PresenterTheme::SharedFontDescriptor& rpFont);

        void SetText (const OUString& rsText);
        OUString GetText (void) const;
        PresenterTheme::SharedFontDescriptor GetFont (void) const;

        void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState,
            const awt::Rectangle& rBoundingBox,
            const awt::Point& rOffset);

        geometry::RealRectangle2D GetBoundingBox (
            const Reference<rendering::XCanvas>& rxCanvas);

    private:
        OUString msText;
        PresenterTheme::SharedFontDescriptor mpFont;
    };

    class ElementMode
        : private ::boost::noncopyable
    {
    public:
        ElementMode (void);

        SharedBitmapDescriptor mpIcon;
        OUString msAction;
        Text maText;

        void ReadElementMode (
            const Reference<beans::XPropertySet>& rxProperties,
            const OUString& rsModeName,
            ::boost::shared_ptr<ElementMode>& rpDefaultMode,
            ::sdext::presenter::PresenterToolBar::Context& rContext);
    };
    typedef ::boost::shared_ptr<ElementMode> SharedElementMode;

}  // end of anonymous namespace

class PresenterToolBar::Context
    : private ::boost::noncopyable
{
public:
    Reference<drawing::XPresenterHelper> mxPresenterHelper;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
};

//===== PresenterToolBar::Element =============================================

namespace {
    typedef cppu::WeakComponentImplHelper2<
        css::document::XEventListener,
        css::frame::XStatusListener
        > ElementInterfaceBase;

    class Element
        : private ::cppu::BaseMutex,
          private ::boost::noncopyable,
          public ElementInterfaceBase
    {
    public:
        Element (const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        virtual ~Element (void);

        virtual void SAL_CALL disposing (void);

        virtual void SetModes (
            const SharedElementMode& rpNormalMode,
            const SharedElementMode& rpMouseOverMode,
            const SharedElementMode& rpSelectedMode,
            const SharedElementMode& rpDisabledMode);
        virtual void CurrentSlideHasChanged (void);
        virtual void SetLocation (const awt::Point& rLocation);
        virtual void SetSize (const geometry::RealSize2D& rSize);
        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState) = 0;
        awt::Size GetBoundingSize (
            const Reference<rendering::XCanvas>& rxCanvas);
        awt::Rectangle GetBoundingBox (void) const;
        virtual bool SetState (const bool bIsOver, const bool bIsPressed);
        virtual void Invalidate (const bool bSynchronous = true);
        virtual bool IsOutside (const awt::Rectangle& rBox);
        virtual bool IsFilling (void) const;
        void UpdateState (void);

        OUString GetAction (void) const;

        // lang::XEventListener

        virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
            throw(css::uno::RuntimeException);

        // document::XEventListener

        virtual void SAL_CALL notifyEvent (const css::document::EventObject& rEvent)
            throw(css::uno::RuntimeException);

        // frame::XStatusListener

        virtual void SAL_CALL statusChanged (const css::frame::FeatureStateEvent& rEvent)
            throw(css::uno::RuntimeException);

    protected:
        ::rtl::Reference<PresenterToolBar> mpToolBar;
        awt::Point maLocation;
        awt::Size maSize;
        SharedElementMode mpNormal;
        SharedElementMode mpMouseOver;
        SharedElementMode mpSelected;
        SharedElementMode mpDisabled;
        SharedElementMode mpMode;
        bool mbIsOver;
        bool mbIsPressed;
        bool mbIsSelected;

        virtual awt::Size CreateBoundingSize (
            const Reference<rendering::XCanvas>& rxCanvas) = 0;

        bool IsEnabled (void) const;
    private:
        bool mbIsEnabled;
    };

} // end of anonymous namespace

class PresenterToolBar::ElementContainerPart
    : public ::std::vector<rtl::Reference<Element> >
{
};

//===== Button ================================================================

namespace {

    class Button : public Element
    {
    public:
        static ::rtl::Reference<Element> Create (
            const ::rtl::Reference<PresenterToolBar>& rpToolBar);

        virtual ~Button (void);
        virtual void SAL_CALL disposing (void);

        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState);

        // lang::XEventListener

        virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
            throw(css::uno::RuntimeException);

    protected:
        virtual awt::Size CreateBoundingSize (
            const Reference<rendering::XCanvas>& rxCanvas);

    private:
        bool mbIsListenerRegistered;

        Button (const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        void Initialize (void);
        void PaintIcon (
            const Reference<rendering::XCanvas>& rxCanvas,
            const sal_Int32 nTextHeight,
            const rendering::ViewState& rViewState);
        PresenterBitmapDescriptor::Mode GetMode (void) const;
    };

//===== Label =================================================================

    class Label : public Element
    {
    public:
        Label (const ::rtl::Reference<PresenterToolBar>& rpToolBar);

        void SetText (const OUString& rsText);
        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState);
        virtual bool SetState (const bool bIsOver, const bool bIsPressed);

    protected:
        virtual awt::Size CreateBoundingSize (
            const Reference<rendering::XCanvas>& rxCanvas);
    };

// Some specialized controls.

    class TimeFormatter
    {
    public:
        TimeFormatter (void);
        OUString FormatTime (const oslDateTime& rTime);
    private:
        bool mbIs24HourFormat;
        bool mbIsAmPmFormat;
        bool mbIsShowSeconds;
    };

    class TimeLabel : public Label
    {
    public:
        void ConnectToTimer (void);
        virtual void TimeHasChanged (const oslDateTime& rCurrentTime) = 0;
    protected:
        TimeLabel(const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        using Element::disposing;
        virtual void SAL_CALL disposing (void);
    private:
        class Listener : public PresenterClockTimer::Listener
        {
        public:
            Listener (const ::rtl::Reference<TimeLabel>& rxLabel)
                : mxLabel(rxLabel) {}
            virtual ~Listener (void) {}
            virtual void TimeHasChanged (const oslDateTime& rCurrentTime)
            { if (mxLabel.is()) mxLabel->TimeHasChanged(rCurrentTime); }
        private:
            ::rtl::Reference<TimeLabel> mxLabel;
        };
        ::boost::shared_ptr<PresenterClockTimer::Listener> mpListener;
    };

    class CurrentTimeLabel : public TimeLabel
    {
    public:
        static ::rtl::Reference<Element> Create (
            const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        virtual void SetModes (
            const SharedElementMode& rpNormalMode,
            const SharedElementMode& rpMouseOverMode,
            const SharedElementMode& rpSelectedMode,
            const SharedElementMode& rpDisabledMode);
    private:
        TimeFormatter maTimeFormatter;
        CurrentTimeLabel (const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        virtual ~CurrentTimeLabel (void);
        virtual void TimeHasChanged (const oslDateTime& rCurrentTime);
    };

    class PresentationTimeLabel : public TimeLabel
    {
    public:
        static ::rtl::Reference<Element> Create (
            const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        virtual void SetModes (
            const SharedElementMode& rpNormalMode,
            const SharedElementMode& rpMouseOverMode,
            const SharedElementMode& rpSelectedMode,
            const SharedElementMode& rpDisabledMode);
    private:
        TimeFormatter maTimeFormatter;
        TimeValue maStartTimeValue;
        PresentationTimeLabel (const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        virtual ~PresentationTimeLabel (void);
        virtual void TimeHasChanged (const oslDateTime& rCurrentTime);
    };

    class VerticalSeparator : public Element
    {
    public:
        explicit VerticalSeparator (const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState);
        virtual bool IsFilling (void) const;

    protected:
        virtual awt::Size CreateBoundingSize (
            const Reference<rendering::XCanvas>& rxCanvas);
    };

    class HorizontalSeparator : public Element
    {
    public:
        explicit HorizontalSeparator (const ::rtl::Reference<PresenterToolBar>& rpToolBar);
        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState);
        virtual bool IsFilling (void) const;

    protected:
        virtual awt::Size CreateBoundingSize (
            const Reference<rendering::XCanvas>& rxCanvas);
    };
} // end of anonymous namespace

//===== PresenterToolBar ======================================================

PresenterToolBar::PresenterToolBar (
    const Reference<XComponentContext>& rxContext,
    const css::uno::Reference<css::awt::XWindow>& rxWindow,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const Anchor eAnchor)
    : PresenterToolBarInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      maElementContainer(),
      mpCurrentContainerPart(),
      mxWindow(rxWindow),
      mxCanvas(rxCanvas),
      mxSlideShowController(),
      mxCurrentSlide(),
      mpPresenterController(rpPresenterController),
      mbIsLayoutPending(false),
      meAnchor(eAnchor),
      maBoundingBox(),
      maMinimalSize()
{
}

void PresenterToolBar::Initialize (
    const OUString& rsConfigurationPath)
{
    try
    {
        CreateControls(rsConfigurationPath);

        if (mxWindow.is())
        {
            mxWindow->addWindowListener(this);
            mxWindow->addPaintListener(this);
            mxWindow->addMouseListener(this);
            mxWindow->addMouseMotionListener(this);

            Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
            if (xPeer.is())
                xPeer->setBackground(util::Color(0xff000000));

            mxWindow->setVisible(sal_True);
        }

        mxSlideShowController = mpPresenterController->GetSlideShowController();
        UpdateSlideNumber();
        mbIsLayoutPending = true;
    }
    catch (RuntimeException&)
    {
        mpCurrentContainerPart.reset();
        maElementContainer.clear();
        throw;
    }
}

PresenterToolBar::~PresenterToolBar (void)
{
}

void SAL_CALL PresenterToolBar::disposing (void)
{
    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);
        mxWindow = NULL;
    }

    // Dispose tool bar elements.
    ElementContainer::iterator iPart (maElementContainer.begin());
    ElementContainer::const_iterator iEnd (maElementContainer.end());
    for ( ; iPart!=iEnd; ++iPart)
    {
        OSL_ASSERT(iPart->get()!=NULL);
        ElementContainerPart::iterator iElement ((*iPart)->begin());
        ElementContainerPart::const_iterator iPartEnd ((*iPart)->end());
        for ( ; iElement!=iPartEnd; ++iElement)
        {
            if (iElement->get() != NULL)
            {
                ::rtl::Reference<Element> pElement (*iElement);
                Reference<lang::XComponent> xComponent (
                    static_cast<XWeak*>(pElement.get()), UNO_QUERY);
                if (xComponent.is())
                    xComponent->dispose();
            }
        }
    }

    mpCurrentContainerPart.reset();
    maElementContainer.clear();
}

void PresenterToolBar::InvalidateArea (
    const awt::Rectangle& rRepaintBox,
    const bool bSynchronous)
{
    mpPresenterController->GetPaintManager()->Invalidate(
        mxWindow,
        rRepaintBox,
        bSynchronous);
}

void PresenterToolBar::RequestLayout (void)
{
    mbIsLayoutPending = true;

    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
}

geometry::RealSize2D PresenterToolBar::GetMinimalSize (void)
{
    if (mbIsLayoutPending)
        Layout(mxCanvas);
    return maMinimalSize;
}

::rtl::Reference<PresenterController> PresenterToolBar::GetPresenterController (void) const
{
    return mpPresenterController;
}

Reference<XComponentContext> PresenterToolBar::GetComponentContext (void) const
{
    return mxComponentContext;
}

//-----  lang::XEventListener -------------------------------------------------

void SAL_CALL PresenterToolBar::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxWindow)
        mxWindow = NULL;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterToolBar::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    mbIsLayoutPending = true;
}

void SAL_CALL PresenterToolBar::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterToolBar::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    mbIsLayoutPending = true;
}

void SAL_CALL PresenterToolBar::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}

//----- XPaintListener --------------------------------------------------------
void SAL_CALL PresenterToolBar::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    if ( ! mxCanvas.is())
        return;

    if ( ! mbIsPresenterViewActive)
        return;

    const rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rEvent.UpdateRect, mxCanvas->getDevice()));

    if (mbIsLayoutPending)
        Layout(mxCanvas);

    Paint(rEvent.UpdateRect, aViewState);

    // Make the back buffer visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}

//----- XMouseListener --------------------------------------------------------
void SAL_CALL PresenterToolBar::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
        ThrowIfDisposed();
        CheckMouseOver(rEvent, true, true);
}

void SAL_CALL PresenterToolBar::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
        ThrowIfDisposed();
        CheckMouseOver(rEvent, true);
}

void SAL_CALL PresenterToolBar::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
        ThrowIfDisposed();
        CheckMouseOver(rEvent, true);
}

void SAL_CALL PresenterToolBar::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
        ThrowIfDisposed();
        CheckMouseOver(rEvent, false);
 }

//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterToolBar::mouseMoved (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
        ThrowIfDisposed();
        CheckMouseOver(rEvent, true);
 }

void SAL_CALL PresenterToolBar::mouseDragged (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    (void)rEvent;
}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterToolBar::setCurrentPage (const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException)
{
    if (rxSlide != mxCurrentSlide)
    {
        mxCurrentSlide = rxSlide;
        UpdateSlideNumber();
    }
}

Reference<drawing::XDrawPage> SAL_CALL PresenterToolBar::getCurrentPage (void)
    throw (RuntimeException)
{
    return mxCurrentSlide;
}

//-----------------------------------------------------------------------------

void PresenterToolBar::CreateControls (
    const OUString& rsConfigurationPath)
{
    if ( ! mxWindow.is())
        return;

    // Expand the macro in the bitmap file names.
    PresenterConfigurationAccess aConfiguration (
        mxComponentContext,
        OUString("/org.openoffice.Office.PresenterScreen/"),
        PresenterConfigurationAccess::READ_ONLY);

    mpCurrentContainerPart.reset(new ElementContainerPart());
    maElementContainer.clear();
    maElementContainer.push_back(mpCurrentContainerPart);

    Reference<container::XHierarchicalNameAccess> xToolBarNode (
        aConfiguration.GetConfigurationNode(rsConfigurationPath),
        UNO_QUERY);
    if (xToolBarNode.is())
    {
        Reference<container::XNameAccess> xEntries (
            PresenterConfigurationAccess::GetConfigurationNode(xToolBarNode, "Entries"),
            UNO_QUERY);
        Context aContext;
        aContext.mxPresenterHelper = mpPresenterController->GetPresenterHelper();
        aContext.mxCanvas = mxCanvas;
        if (xEntries.is()
            && aContext.mxPresenterHelper.is()
            && aContext.mxCanvas.is())
        {
            PresenterConfigurationAccess::ForAll(
                xEntries,
                ::boost::bind(&PresenterToolBar::ProcessEntry, this, _2, ::boost::ref(aContext)));
        }
    }
}

void PresenterToolBar::ProcessEntry (
    const Reference<beans::XPropertySet>& rxProperties,
    Context& rContext)
{
    if ( ! rxProperties.is())
        return;

    // Type has to be present.
    OUString sType;
    if ( ! (PresenterConfigurationAccess::GetProperty(rxProperties, "Type") >>= sType))
        return;

    OUString sName;
    PresenterConfigurationAccess::GetProperty(rxProperties, "Name") >>= sName;

    // Read mode specific values.
    SharedElementMode pNormalMode (new ElementMode());
    SharedElementMode pMouseOverMode (new ElementMode());
    SharedElementMode pSelectedMode (new ElementMode());
    SharedElementMode pDisabledMode (new ElementMode());
    pNormalMode->ReadElementMode(rxProperties, "Normal", pNormalMode, rContext);
    pMouseOverMode->ReadElementMode(rxProperties, "MouseOver", pNormalMode, rContext);
    pSelectedMode->ReadElementMode(rxProperties, "Selected", pNormalMode, rContext);
    pDisabledMode->ReadElementMode(rxProperties, "Disabled", pNormalMode, rContext);

    // Create new element.
    ::rtl::Reference<Element> pElement;
    if ( sType == "Button" )
        pElement = Button::Create(this);
    else if ( sType == "CurrentTimeLabel" )
        pElement = CurrentTimeLabel::Create(this);
    else if ( sType == "PresentationTimeLabel" )
        pElement = PresentationTimeLabel::Create(this);
    else if ( sType == "VerticalSeparator" )
        pElement = ::rtl::Reference<Element>(new VerticalSeparator(this));
    else if ( sType == "HorizontalSeparator" )
        pElement = ::rtl::Reference<Element>(new HorizontalSeparator(this));
    else if ( sType == "Label" )
        pElement = ::rtl::Reference<Element>(new Label(this));
    else if ( sType == "ChangeOrientation" )
    {
        mpCurrentContainerPart.reset(new ElementContainerPart());
        maElementContainer.push_back(mpCurrentContainerPart);
        return;
    }
    if (pElement.is())
    {
        pElement->SetModes( pNormalMode, pMouseOverMode, pSelectedMode, pDisabledMode);
        pElement->UpdateState();
        if (mpCurrentContainerPart.get() != NULL)
            mpCurrentContainerPart->push_back(pElement);
    }
}

void PresenterToolBar::Layout (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    if (maElementContainer.empty())
        return;

    mbIsLayoutPending = false;

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    ElementContainer::iterator iPart;
    ElementContainer::iterator iEnd (maElementContainer.end());
    ElementContainer::iterator iBegin (maElementContainer.begin());
    ::std::vector<geometry::RealSize2D> aPartSizes (maElementContainer.size());
    geometry::RealSize2D aTotalSize (0,0);
    bool bIsHorizontal (true);
    sal_Int32 nIndex;
    double nTotalHorizontalGap (0);
    sal_Int32 nGapCount (0);
    for (iPart=maElementContainer.begin(),nIndex=0; iPart!=iEnd; ++iPart,++nIndex)
    {
        geometry::RealSize2D aSize (CalculatePartSize(rxCanvas, *iPart, bIsHorizontal));

        // Remember the size of each part for later.
        aPartSizes[nIndex] = aSize;

        // Add gaps between elements.
        if ((*iPart)->size()>1 && bIsHorizontal)
        {
            nTotalHorizontalGap += ((*iPart)->size() - 1) * gnGapSize;
            nGapCount += (*iPart)->size()-1;
        }

        // Orientation changes for each part.
        bIsHorizontal = !bIsHorizontal;
        // Width is accumulated.
        aTotalSize.Width += aSize.Width;
        // Height is the maximum height of all parts.
        aTotalSize.Height = ::std::max(aTotalSize.Height, aSize.Height);
    }
    // Add gaps between parts.
    if (maElementContainer.size() > 1)
    {
        nTotalHorizontalGap += (maElementContainer.size() - 1) * gnGapSize;
        nGapCount += maElementContainer.size()-1;
    }

    // Calculate the minimal size so that the window size of the tool bar
    // can be adapted accordingly.
    maMinimalSize = aTotalSize;
    maMinimalSize.Width += nTotalHorizontalGap;

    // Calculate the gaps between elements.
    double nGapWidth (0);
    if (nGapCount > 0)
    {
        if (aTotalSize.Width + nTotalHorizontalGap > aWindowBox.Width)
            nTotalHorizontalGap = aWindowBox.Width - aTotalSize.Width;
        nGapWidth = nTotalHorizontalGap / nGapCount;
    }

    // Determine the location of the left edge.
    double nX (0);
    switch (meAnchor)
    {
        case Left : nX = 0; break;
        case Center: nX = (aWindowBox.Width - aTotalSize.Width - nTotalHorizontalGap) / 2; break;
        case Right: nX = aWindowBox.Width - aTotalSize.Width - nTotalHorizontalGap; break;
    }

    // Place the parts.
    double nY ((aWindowBox.Height - aTotalSize.Height) / 2);
    bIsHorizontal = true;

    maBoundingBox.X1 = nX;
    maBoundingBox.Y1 = nY;
    maBoundingBox.X2 = nX + aTotalSize.Width + nTotalHorizontalGap;
    maBoundingBox.Y2 = nY + aTotalSize.Height;

    /* push front or back ? ... */
    /// check whether RTL interface or not
    if(!Application::GetSettings().GetLayoutRTL()){
        for (iPart=maElementContainer.begin(), nIndex=0; iPart!=iEnd; ++iPart,++nIndex)
        {
            geometry::RealRectangle2D aBoundingBox(
                nX, nY,
                nX+aPartSizes[nIndex].Width, nY+aTotalSize.Height);

            // Add space for gaps between elements.
            if ((*iPart)->size() > 1)
                if (bIsHorizontal)
                    aBoundingBox.X2 += ((*iPart)->size()-1) * nGapWidth;

            LayoutPart(rxCanvas, *iPart, aBoundingBox, aPartSizes[nIndex], bIsHorizontal);
            bIsHorizontal = !bIsHorizontal;
            nX += aBoundingBox.X2 - aBoundingBox.X1 + nGapWidth;
        }
    }
    else {
        for (iPart=maElementContainer.end()-1, nIndex=2; iPart!=iBegin-1; --iPart, --nIndex)
        {
            geometry::RealRectangle2D aBoundingBox(
                nX, nY,
                nX+aPartSizes[nIndex].Width, nY+aTotalSize.Height);

            // Add space for gaps between elements.
            if ((*iPart)->size() > 1)
                if (bIsHorizontal)
                    aBoundingBox.X2 += ((*iPart)->size()-1) * nGapWidth;

            LayoutPart(rxCanvas, *iPart, aBoundingBox, aPartSizes[nIndex], bIsHorizontal);
            bIsHorizontal = !bIsHorizontal;
            nX += aBoundingBox.X2 - aBoundingBox.X1 + nGapWidth;
        }
    }

    // The whole window has to be repainted.
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
}

geometry::RealSize2D PresenterToolBar::CalculatePartSize (
    const Reference<rendering::XCanvas>& rxCanvas,
    const SharedElementContainerPart& rpPart,
    const bool bIsHorizontal)
{
    geometry::RealSize2D aTotalSize (0,0);

    if (mxWindow.is())
    {
        // Calculate the summed width of all elements.
        ElementContainerPart::const_iterator iElement;
        for (iElement=rpPart->begin(); iElement!=rpPart->end(); ++iElement)
        {
            if (iElement->get() == NULL)
                continue;

            const awt::Size aBSize ((*iElement)->GetBoundingSize(rxCanvas));
            if (bIsHorizontal)
            {
                aTotalSize.Width += aBSize.Width;
                if (aBSize.Height > aTotalSize.Height)
                    aTotalSize.Height = aBSize.Height;
            }
            else
            {
                aTotalSize.Height += aBSize.Height;
                if (aBSize.Width > aTotalSize.Width)
                    aTotalSize.Width = aBSize.Width;
            }
        }
    }
    return aTotalSize;
}

void PresenterToolBar::LayoutPart (
    const Reference<rendering::XCanvas>& rxCanvas,
    const SharedElementContainerPart& rpPart,
    const geometry::RealRectangle2D& rBoundingBox,
    const geometry::RealSize2D& rPartSize,
    const bool bIsHorizontal)
{
    double nGap (0);
    if (rpPart->size() > 1)
    {
        if (bIsHorizontal)
            nGap = (rBoundingBox.X2 - rBoundingBox.X1 - rPartSize.Width) / (rpPart->size()-1);
        else
            nGap = (rBoundingBox.Y2 - rBoundingBox.Y1 - rPartSize.Height) / (rpPart->size()-1);
    }

    // Place the elements.
    double nX (rBoundingBox.X1);
    double nY (rBoundingBox.Y1);

    ElementContainerPart::const_iterator iElement;
    ElementContainerPart::const_iterator iEnd (rpPart->end());
    ElementContainerPart::const_iterator iBegin (rpPart->begin());

    /// check whether RTL interface or not
    if(!Application::GetSettings().GetLayoutRTL()){
        for (iElement=rpPart->begin(); iElement!=iEnd; ++iElement)
        {
            if (iElement->get() == NULL)
                continue;

            const awt::Size aElementSize ((*iElement)->GetBoundingSize(rxCanvas));
            if (bIsHorizontal)
            {
                if ((*iElement)->IsFilling())
                {
                    nY = rBoundingBox.Y1;
                    (*iElement)->SetSize(geometry::RealSize2D(aElementSize.Width, rBoundingBox.Y2 - rBoundingBox.Y1));
                }
                else
                    nY = rBoundingBox.Y1 + (rBoundingBox.Y2-rBoundingBox.Y1 - aElementSize.Height) / 2;
                (*iElement)->SetLocation(awt::Point(sal_Int32(0.5 + nX), sal_Int32(0.5 + nY)));
                nX += aElementSize.Width + nGap;
            }
            else
            {
                if ((*iElement)->IsFilling())
                {
                    nX = rBoundingBox.X1;
                    (*iElement)->SetSize(geometry::RealSize2D(rBoundingBox.X2 - rBoundingBox.X1, aElementSize.Height));
                }
                else
                    nX = rBoundingBox.X1 + (rBoundingBox.X2-rBoundingBox.X1 - aElementSize.Width) / 2;
                (*iElement)->SetLocation(awt::Point(sal_Int32(0.5 + nX), sal_Int32(0.5 + nY)));
                nY += aElementSize.Height + nGap;
            }
        }
    }
    else {
        for (iElement=rpPart->end()-1; iElement!=iBegin-1; --iElement)
        {
            if (iElement->get() == NULL)
                continue;

            const awt::Size aElementSize ((*iElement)->GetBoundingSize(rxCanvas));
            if (bIsHorizontal)
            {
                if ((*iElement)->IsFilling())
                {
                    nY = rBoundingBox.Y1;
                    (*iElement)->SetSize(geometry::RealSize2D(aElementSize.Width, rBoundingBox.Y2 - rBoundingBox.Y1));
                }
                else
                    nY = rBoundingBox.Y1 + (rBoundingBox.Y2-rBoundingBox.Y1 - aElementSize.Height) / 2;
                (*iElement)->SetLocation(awt::Point(sal_Int32(0.5 + nX), sal_Int32(0.5 + nY)));
                nX += aElementSize.Width + nGap;
            }
            else
            {
                // reverse presentation time with current time
                if (iElement==iBegin){
                    iElement=iBegin+2;
                }
                else if (iElement==iBegin+2){
                    iElement=iBegin;
                }
                const awt::Size aNewElementSize ((*iElement)->GetBoundingSize(rxCanvas));
                if ((*iElement)->IsFilling())
                {
                    nX = rBoundingBox.X1;
                    (*iElement)->SetSize(geometry::RealSize2D(rBoundingBox.X2 - rBoundingBox.X1, aNewElementSize.Height));
                }
                else
                    nX = rBoundingBox.X1 + (rBoundingBox.X2-rBoundingBox.X1 - aNewElementSize.Width) / 2;
                (*iElement)->SetLocation(awt::Point(sal_Int32(0.5 + nX), sal_Int32(0.5 + nY)));
                nY += aNewElementSize.Height + nGap;

                // return the index as it was before the reversing
                if (iElement==iBegin)
                    iElement=iBegin+2;
                else if (iElement==iBegin+2)
                    iElement=iBegin;
            }
        }
    }

}

void PresenterToolBar::Paint (
    const awt::Rectangle& rUpdateBox,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(mxCanvas.is());

    ElementContainer::iterator iPart;
    ElementContainer::const_iterator iEnd (maElementContainer.end());
    for (iPart=maElementContainer.begin(); iPart!=iEnd; ++iPart)
    {
        ElementContainerPart::iterator iElement;
        ElementContainerPart::const_iterator iPartEnd ((*iPart)->end());
        for (iElement=(*iPart)->begin(); iElement!=iPartEnd; ++iElement)
        {
            if (iElement->get() != NULL)
            {
                if ( ! (*iElement)->IsOutside(rUpdateBox))
                    (*iElement)->Paint(mxCanvas, rViewState);
            }
        }
    }
}

void PresenterToolBar::UpdateSlideNumber (void)
{
    if( mxSlideShowController.is() )
    {
        ElementContainer::iterator iPart;
        ElementContainer::const_iterator iEnd (maElementContainer.end());
        for (iPart=maElementContainer.begin(); iPart!=iEnd; ++iPart)
        {
            ElementContainerPart::iterator iElement;
            ElementContainerPart::const_iterator iPartEnd ((*iPart)->end());
            for (iElement=(*iPart)->begin(); iElement!=iPartEnd; ++iElement)
            {
                if (iElement->get() != NULL)
                    (*iElement)->CurrentSlideHasChanged();
            }
        }
    }
}

void PresenterToolBar::CheckMouseOver (
    const css::awt::MouseEvent& rEvent,
    const bool bOverWindow,
    const bool bMouseDown)
{
    css::awt::MouseEvent rTemp =rEvent;
    if(Application::GetSettings().GetLayoutRTL()){
        awt::Rectangle aWindowBox = mxWindow->getPosSize();
        rTemp.X=aWindowBox.Width-rTemp.X;
    }
    ElementContainer::iterator iPart;
    ElementContainer::const_iterator iEnd (maElementContainer.end());
    for (iPart=maElementContainer.begin(); iPart!=iEnd; ++iPart)
    {
        ElementContainerPart::iterator iElement;
        ElementContainerPart::const_iterator iPartEnd ((*iPart)->end());
        for (iElement=(*iPart)->begin(); iElement!=iPartEnd; ++iElement)
        {
            if (iElement->get() == NULL)
                continue;

            awt::Rectangle aBox ((*iElement)->GetBoundingBox());
            const bool bIsOver = bOverWindow
                && aBox.X <= rTemp.X
                && aBox.Width+aBox.X-1 >= rTemp.X
                && aBox.Y <= rTemp.Y
                && aBox.Height+aBox.Y-1 >= rTemp.Y;
            (*iElement)->SetState(
                bIsOver,
                bIsOver && rTemp.Buttons!=0 && bMouseDown && rTemp.ClickCount>0);
        }
    }
}

void PresenterToolBar::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString( "PresenterToolBar has already been disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

//===== PresenterToolBarView ==================================================

PresenterToolBarView::PresenterToolBarView (
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterToolBarViewInterfaceBase(m_aMutex),
      mxPane(),
      mxViewId(rxViewId),
      mxWindow(),
      mxCanvas(),
      mpPresenterController(rpPresenterController),
      mxSlideShowController(rpPresenterController->GetSlideShowController()),
      mpToolBar()
{
    try
    {
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC(xCM->getConfigurationController(),UNO_QUERY_THROW);
        mxPane = Reference<XPane>(xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);

        mxWindow = mxPane->getWindow();
        mxCanvas = mxPane->getCanvas();

        mpToolBar = new PresenterToolBar(
            rxContext,
            mxWindow,
            mxCanvas,
            rpPresenterController,
            PresenterToolBar::Center);
        mpToolBar->Initialize("PresenterScreenSettings/ToolBars/ToolBar");

        if (mxWindow.is())
        {
            mxWindow->addPaintListener(this);

            Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
            if (xPeer.is())
                xPeer->setBackground(util::Color(0xff000000));

            mxWindow->setVisible(sal_True);
        }
    }
    catch (RuntimeException&)
    {
        mxViewId = NULL;
        throw;
    }
}

PresenterToolBarView::~PresenterToolBarView (void)
{
}

void SAL_CALL PresenterToolBarView::disposing (void)
{
    Reference<lang::XComponent> xComponent (static_cast<XWeak*>(mpToolBar.get()), UNO_QUERY);
    mpToolBar = NULL;
    if (xComponent.is())
        xComponent->dispose();

    if (mxWindow.is())
    {
        mxWindow->removePaintListener(this);
        mxWindow = NULL;
    }
    mxCanvas = NULL;
    mxViewId = NULL;
    mxPane = NULL;
    mpPresenterController = NULL;
    mxSlideShowController = NULL;

}

::rtl::Reference<PresenterToolBar> PresenterToolBarView::GetPresenterToolBar (void) const
{
    return mpToolBar;
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterToolBarView::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    awt::Rectangle aWindowBox (mxWindow->getPosSize());
    mpPresenterController->GetCanvasHelper()->Paint(
        mpPresenterController->GetViewBackground(mxViewId->getResourceURL()),
        mxCanvas,
        rEvent.UpdateRect,
        awt::Rectangle(0,0,aWindowBox.Width, aWindowBox.Height),
        awt::Rectangle());
}

//-----  lang::XEventListener -------------------------------------------------

void SAL_CALL PresenterToolBarView::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxWindow)
        mxWindow = NULL;
}

//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterToolBarView::getResourceId (void)
    throw (RuntimeException)
{
    return mxViewId;
}

sal_Bool SAL_CALL PresenterToolBarView::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterToolBarView::setCurrentPage (const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException)
{
    Reference<drawing::XDrawView> xToolBar (static_cast<XWeak*>(mpToolBar.get()), UNO_QUERY);
    if (xToolBar.is())
        xToolBar->setCurrentPage(rxSlide);
}

Reference<drawing::XDrawPage> SAL_CALL PresenterToolBarView::getCurrentPage (void)
    throw (RuntimeException)
{
    return NULL;
}

//===== PresenterToolBar::Element =============================================

namespace {

Element::Element (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : ElementInterfaceBase(m_aMutex),
      mpToolBar(rpToolBar),
      maLocation(),
      maSize(),
      mpNormal(),
      mpMouseOver(),
      mpSelected(),
      mpDisabled(),
      mpMode(),
      mbIsOver(false),
      mbIsPressed(false),
      mbIsSelected(false),
      mbIsEnabled(true)
{
    if (mpToolBar.get() != NULL)
    {
        OSL_ASSERT(mpToolBar->GetPresenterController().is());
        OSL_ASSERT(mpToolBar->GetPresenterController()->GetWindowManager().is());
    }
}

Element::~Element (void)
{
}

void Element::SetModes (
    const SharedElementMode& rpNormalMode,
    const SharedElementMode& rpMouseOverMode,
    const SharedElementMode& rpSelectedMode,
    const SharedElementMode& rpDisabledMode)
{
    mpNormal = rpNormalMode;
    mpMouseOver = rpMouseOverMode;
    mpSelected = rpSelectedMode;
    mpDisabled = rpDisabledMode;
    mpMode = rpNormalMode;
}

void Element::disposing (void)
{
}

awt::Size Element::GetBoundingSize (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    maSize = CreateBoundingSize(rxCanvas);
    return maSize;
}

awt::Rectangle Element::GetBoundingBox (void) const
{
    return awt::Rectangle(maLocation.X,maLocation.Y, maSize.Width, maSize.Height);
}

void Element::CurrentSlideHasChanged (void)
{
    UpdateState();
}

void Element::SetLocation (const awt::Point& rLocation)
{
    maLocation = rLocation;
}

void Element::SetSize (const geometry::RealSize2D& rSize)
{
    maSize = awt::Size(sal_Int32(0.5+rSize.Width), sal_Int32(0.5+rSize.Height));
}

bool Element::SetState (
    const bool bIsOver,
    const bool bIsPressed)
{
    bool bModified (mbIsOver != bIsOver || mbIsPressed != bIsPressed);
    bool bClicked (mbIsPressed && bIsOver && ! bIsPressed);

    mbIsOver = bIsOver;
    mbIsPressed = bIsPressed;

    // When the element is disabled then ignore mouse over or selection.
    // When the element is selected then ignore mouse over.
    if ( ! mbIsEnabled)
        mpMode = mpDisabled;
    else if (mbIsSelected)
        mpMode = mpSelected;
    else if (mbIsOver)
        mpMode = mpMouseOver;
    else
        mpMode = mpNormal;

    if (bClicked && mbIsEnabled)
    {
        if (mpMode.get() != NULL)
        {
            do
            {
                if (mpMode->msAction.isEmpty())
                    break;

                if (mpToolBar.get() == NULL)
                    break;

                if (mpToolBar->GetPresenterController().get() == NULL)
                    break;

                mpToolBar->GetPresenterController()->DispatchUnoCommand(mpMode->msAction);
                mpToolBar->RequestLayout();
            }
            while (false);
        }

    }
    else if (bModified)
    {
        Invalidate();
    }

    return bModified;
}

void Element::Invalidate (const bool bSynchronous)
{
    OSL_ASSERT(mpToolBar.is());
    mpToolBar->InvalidateArea(GetBoundingBox(), bSynchronous);
}

bool Element::IsOutside (const awt::Rectangle& rBox)
{
    if (rBox.X >= maLocation.X+maSize.Width)
        return true;
    else if (rBox.Y >= maLocation.Y+maSize.Height)
        return true;
    else if (maLocation.X >= rBox.X+rBox.Width)
        return true;
    else if (maLocation.Y >= rBox.Y+rBox.Height)
        return true;
    else
        return false;
}

bool Element::IsEnabled (void) const
{
    return mbIsEnabled;
}

bool Element::IsFilling (void) const
{
    return false;
}

void Element::UpdateState (void)
{
    OSL_ASSERT(mpToolBar.get() != NULL);
    OSL_ASSERT(mpToolBar->GetPresenterController().get() != NULL);

    if (mpMode.get() == NULL)
        return;

    util::URL aURL (mpToolBar->GetPresenterController()->CreateURLFromString(mpMode->msAction));
    Reference<frame::XDispatch> xDispatch (mpToolBar->GetPresenterController()->GetDispatch(aURL));
    if (xDispatch.is())
    {
        xDispatch->addStatusListener(this, aURL);
        xDispatch->removeStatusListener(this, aURL);
    }
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL Element::disposing (const css::lang::EventObject& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}

//----- document::XEventListener ----------------------------------------------

void SAL_CALL Element::notifyEvent (const css::document::EventObject& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    UpdateState();
}

//----- frame::XStatusListener ------------------------------------------------

void SAL_CALL Element::statusChanged (const css::frame::FeatureStateEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    bool bIsSelected (mbIsSelected);
    bool bIsEnabled (rEvent.IsEnabled);
    rEvent.State >>= bIsSelected;

    if (bIsSelected != mbIsSelected || bIsEnabled != mbIsEnabled)
    {
        mbIsEnabled = bIsEnabled;
        mbIsSelected = bIsSelected;
        SetState(mbIsOver, mbIsPressed);
        mpToolBar->RequestLayout();
    }
}

} // end of anonymous namespace

//===== ElementMode ===========================================================

namespace {

ElementMode::ElementMode (void)
    : mpIcon(),
      msAction(),
      maText()
{
}

void ElementMode::ReadElementMode (
    const Reference<beans::XPropertySet>& rxElementProperties,
    const OUString& rsModeName,
    ::boost::shared_ptr<ElementMode>& rpDefaultMode,
    ::sdext::presenter::PresenterToolBar::Context& rContext)
{
    try
    {
    Reference<container::XHierarchicalNameAccess> xNode (
        PresenterConfigurationAccess::GetProperty(rxElementProperties, rsModeName),
        UNO_QUERY);
    Reference<beans::XPropertySet> xProperties (
        PresenterConfigurationAccess::GetNodeProperties(xNode, OUString()));
    if ( ! xProperties.is() && rpDefaultMode.get()!=NULL)
    {
        // The mode is not specified.  Use the given, possibly empty,
        // default mode instead.
        mpIcon = rpDefaultMode->mpIcon;
        msAction = rpDefaultMode->msAction;
        maText = rpDefaultMode->maText;
    }

    // Read action.
    if ( ! (PresenterConfigurationAccess::GetProperty(xProperties, "Action") >>= msAction))
        if (rpDefaultMode.get()!=NULL)
            msAction = rpDefaultMode->msAction;

    // Read text and font
    OUString sText (rpDefaultMode.get()!=NULL ? rpDefaultMode->maText.GetText() : OUString());
    PresenterConfigurationAccess::GetProperty(xProperties, "Text") >>= sText;
    Reference<container::XHierarchicalNameAccess> xFontNode (
        PresenterConfigurationAccess::GetProperty(xProperties, "Font"), UNO_QUERY);
    PresenterTheme::SharedFontDescriptor pFont (PresenterTheme::ReadFont(
        xFontNode,
        "",
        rpDefaultMode.get()!=NULL
            ? rpDefaultMode->maText.GetFont()
            : PresenterTheme::SharedFontDescriptor()));
    maText = Text(sText,pFont);

    // Read bitmaps to display as icons.
    Reference<container::XHierarchicalNameAccess> xIconNode (
        PresenterConfigurationAccess::GetProperty(xProperties, "Icon"), UNO_QUERY);
    mpIcon = PresenterBitmapContainer::LoadBitmap(
        xIconNode,
        "",
        rContext.mxPresenterHelper,
        rContext.mxCanvas,
        rpDefaultMode.get()!=NULL ? rpDefaultMode->mpIcon : SharedBitmapDescriptor());
    }
    catch(Exception&)
    {
        OSL_ASSERT(false);
    }
}

} // end of anonymous namespace

//===== Button ================================================================

namespace {

::rtl::Reference<Element> Button::Create (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
{
    ::rtl::Reference<Button> pElement (new Button(rpToolBar));
    pElement->Initialize();
    return ::rtl::Reference<Element>(pElement.get());
}

Button::Button (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : Element(rpToolBar),
      mbIsListenerRegistered(false)
{
    OSL_ASSERT(mpToolBar.get() != NULL);
    OSL_ASSERT(mpToolBar->GetPresenterController().is());
    OSL_ASSERT(mpToolBar->GetPresenterController()->GetWindowManager().is());
}

Button::~Button (void)
{
}

void Button::Initialize (void)
{
    mpToolBar->GetPresenterController()->GetWindowManager()->AddLayoutListener(this);
    mbIsListenerRegistered = true;
}

void Button::disposing (void)
{
    OSL_ASSERT(mpToolBar.get() != NULL);
    if (mpToolBar.get() != NULL
        && mbIsListenerRegistered)
    {
        OSL_ASSERT(mpToolBar->GetPresenterController().is());
        OSL_ASSERT(mpToolBar->GetPresenterController()->GetWindowManager().is());

        mbIsListenerRegistered = false;
        mpToolBar->GetPresenterController()->GetWindowManager()->RemoveLayoutListener(this);
    }
    Element::disposing();
}

void Button::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(rxCanvas.is());

    if (mpMode.get() == NULL)
        return;

    if (mpMode->mpIcon.get() == NULL)
        return;

    geometry::RealRectangle2D aTextBBox (mpMode->maText.GetBoundingBox(rxCanvas));
    sal_Int32 nTextHeight (sal::static_int_cast<sal_Int32>(0.5 + aTextBBox.Y2 - aTextBBox.Y1));

    PaintIcon(rxCanvas, nTextHeight, rViewState);
    awt::Point aOffset(0,0);
    if ( ! IsEnabled())
        if (mpMode->mpIcon.get() != NULL)
        {
            Reference<rendering::XBitmap> xBitmap (mpMode->mpIcon->GetNormalBitmap());
            if (xBitmap.is())
                aOffset.Y = xBitmap->getSize().Height;
        }
    mpMode->maText.Paint(rxCanvas, rViewState, GetBoundingBox(), aOffset);
}

awt::Size Button::CreateBoundingSize (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    if (mpMode.get() == NULL)
        return awt::Size();

    geometry::RealRectangle2D aTextBBox (mpMode->maText.GetBoundingBox(rxCanvas));
    const sal_Int32 nGap (5);
    sal_Int32 nTextHeight (sal::static_int_cast<sal_Int32>(0.5 + aTextBBox.Y2 - aTextBBox.Y1));
    sal_Int32 nTextWidth (sal::static_int_cast<sal_Int32>(0.5 + aTextBBox.X2 - aTextBBox.X1));
    Reference<rendering::XBitmap> xBitmap;
    if (mpMode->mpIcon.get() != NULL)
        xBitmap = mpMode->mpIcon->GetNormalBitmap();
    if (xBitmap.is())
    {
        geometry::IntegerSize2D aSize (xBitmap->getSize());
        return awt::Size(
            ::std::max(aSize.Width, sal_Int32(0.5 + aTextBBox.X2 - aTextBBox.X1)),
            aSize.Height+ nGap + nTextHeight);
    }
    else
        return awt::Size(nTextWidth,nTextHeight);
}

void Button::PaintIcon (
    const Reference<rendering::XCanvas>& rxCanvas,
    const sal_Int32 nTextHeight,
    const rendering::ViewState& rViewState)
{
    if (mpMode.get() == NULL)
        return;

    Reference<rendering::XBitmap> xBitmap (mpMode->mpIcon->GetBitmap(GetMode()));
    if (xBitmap.is())
    {
        /// check whether RTL interface or not
        if(!Application::GetSettings().GetLayoutRTL()){
            const sal_Int32 nX (maLocation.X
                + (maSize.Width-xBitmap->getSize().Width) / 2);
            const sal_Int32 nY (maLocation.Y
                + (maSize.Height - nTextHeight - xBitmap->getSize().Height) / 2);
            const rendering::RenderState aRenderState(
                geometry::AffineMatrix2D(1,0,nX, 0,1,nY),
                NULL,
                Sequence<double>(4),
                rendering::CompositeOperation::OVER);
            rxCanvas->drawBitmap(xBitmap, rViewState, aRenderState);
        }
        else {
            const sal_Int32 nX (maLocation.X
                + (maSize.Width+xBitmap->getSize().Width) / 2);
            const sal_Int32 nY (maLocation.Y
                + (maSize.Height - nTextHeight - xBitmap->getSize().Height) / 2);
            const rendering::RenderState aRenderState(
                geometry::AffineMatrix2D(-1,0,nX, 0,1,nY),
                NULL,
                Sequence<double>(4),
                rendering::CompositeOperation::OVER);
            rxCanvas->drawBitmap(xBitmap, rViewState, aRenderState);
        }
    }
}

PresenterBitmapDescriptor::Mode Button::GetMode (void) const
{
    if ( ! IsEnabled())
        return PresenterBitmapDescriptor::Disabled;
    else if (mbIsPressed)
        return PresenterBitmapDescriptor::ButtonDown;
    else if (mbIsOver)
        return PresenterBitmapDescriptor::MouseOver;
    else
        return PresenterBitmapDescriptor::Normal;
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL Button::disposing (const css::lang::EventObject& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    mbIsListenerRegistered = false;
    Element::disposing(rEvent);
}

} // end of anonymous namespace

//===== PresenterToolBar::Label ===============================================

namespace {

Label::Label (const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : Element(rpToolBar)
{
}

awt::Size Label::CreateBoundingSize (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    if (mpMode.get() == NULL)
        return awt::Size(0,0);

    geometry::RealRectangle2D aTextBBox (mpMode->maText.GetBoundingBox(rxCanvas));
    return awt::Size(
        sal::static_int_cast<sal_Int32>(0.5 + aTextBBox.X2 - aTextBBox.X1),
        sal::static_int_cast<sal_Int32>(0.5 + aTextBBox.Y2 - aTextBBox.Y1));
}

void Label::SetText (const OUString& rsText)
{
    OSL_ASSERT(mpToolBar.get() != NULL);
    if (mpMode.get() == NULL)
        return;

    const bool bRequestLayout (mpMode->maText.GetText().getLength() != rsText.getLength());

    mpMode->maText.SetText(rsText);
    // Just use the character count for determing whether a layout is
    // necessary.  This is an optimization to avoid layouts every time a new
    // time value is set on some labels.
    if (bRequestLayout)
        mpToolBar->RequestLayout();
    else
        Invalidate(false);
}

void Label::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(rxCanvas.is());
    if (mpMode.get() == NULL)
        return;

    mpMode->maText.Paint(rxCanvas, rViewState, GetBoundingBox(), awt::Point(0,0));
}

bool Label::SetState (const bool bIsOver, const bool bIsPressed)
{
    // For labels there is no mouse over effect.
    (void)bIsOver;
    (void)bIsPressed;
    return Element::SetState(false, false);
}

} // end of anonymous namespace

//===== Text ==================================================================

namespace {

Text::Text (void)
    : msText(),
      mpFont()
{
}

Text::Text (const Text& rText)
    : msText(rText.msText),
      mpFont(rText.mpFont)
{
}

Text::Text (
    const OUString& rsText,
    const PresenterTheme::SharedFontDescriptor& rpFont)
    : msText(rsText),
      mpFont(rpFont)
{
}

void Text::SetText (const OUString& rsText)
{
    msText = rsText;
}

OUString Text::GetText (void) const
{
    return msText;
}

PresenterTheme::SharedFontDescriptor Text::GetFont (void) const
{
    return mpFont;
}

void Text::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState,
    const awt::Rectangle& rBoundingBox,
    const awt::Point& rOffset)
{
    (void)rOffset;
    OSL_ASSERT(rxCanvas.is());

    if (msText.isEmpty())
        return;
    if (mpFont.get() == NULL)
        return;

    if ( ! mpFont->mxFont.is())
        mpFont->PrepareFont(rxCanvas);
    if ( ! mpFont->mxFont.is())
        return;

    rendering::StringContext aContext (msText, 0, msText.getLength());

    Reference<rendering::XTextLayout> xLayout (
        mpFont->mxFont->createTextLayout(
            aContext,
            rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
            0));
    geometry::RealRectangle2D aBox (xLayout->queryTextBounds());
    const double nTextWidth = aBox.X2 - aBox.X1;
    const double nY = rBoundingBox.Y + rBoundingBox.Height - aBox.Y2;
    const double nX = rBoundingBox.X + (rBoundingBox.Width - nTextWidth)/2;

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,nX, 0,1,nY),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, mpFont->mnColor);
    rxCanvas->drawTextLayout(
        xLayout,
        rViewState,
        aRenderState);
}

geometry::RealRectangle2D Text::GetBoundingBox (const Reference<rendering::XCanvas>& rxCanvas)
{
    if (mpFont.get() != NULL && !msText.isEmpty())
    {
        if ( ! mpFont->mxFont.is())
            mpFont->PrepareFont(rxCanvas);
        if (mpFont->mxFont.is())
        {
            rendering::StringContext aContext (msText, 0, msText.getLength());
                Reference<rendering::XTextLayout> xLayout (
                mpFont->mxFont->createTextLayout(
                    aContext,
                    rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                    0));
            return xLayout->queryTextBounds();
        }
    }
    return geometry::RealRectangle2D(0,0,0,0);
}

//===== TimeFormatter =========================================================

TimeFormatter::TimeFormatter (void)
    : mbIs24HourFormat(true),
      mbIsAmPmFormat(false),
      mbIsShowSeconds(true)
{
}

OUString TimeFormatter::FormatTime (const oslDateTime& rTime)
{
    OUStringBuffer sText;

    const sal_Int32 nHours (sal::static_int_cast<sal_Int32>(rTime.Hours));
    const sal_Int32 nMinutes (sal::static_int_cast<sal_Int32>(rTime.Minutes));
    const sal_Int32 nSeconds(sal::static_int_cast<sal_Int32>(rTime.Seconds));
    // Hours
    if (mbIs24HourFormat)
        sText.append(OUString::number(nHours));
    else
        sText.append(OUString::number(
            sal::static_int_cast<sal_Int32>(nHours>12 ? nHours-12 : nHours)));

    sText.append(":");

    // Minutes
    const OUString sMinutes (OUString::number(nMinutes));
    if (sMinutes.getLength() == 1)
        sText.append("0");
    sText.append(sMinutes);

    // Seconds
    if (mbIsShowSeconds)
    {
        sText.append(":");
        const OUString sSeconds (OUString::number(nSeconds));
        if (sSeconds.getLength() == 1)
            sText.append("0");
        sText.append(sSeconds);
    }
    if (mbIsAmPmFormat)
    {
        if (rTime.Hours < 12)
            sText.append("am");
        else
            sText.append("pm");
    }
    return sText.makeStringAndClear();
}

//===== TimeLabel =============================================================

TimeLabel::TimeLabel (const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : Label(rpToolBar),
      mpListener()
{
}

void SAL_CALL TimeLabel::disposing (void)
{
    PresenterClockTimer::Instance(mpToolBar->GetComponentContext())->RemoveListener(mpListener);
    mpListener.reset();
}

void TimeLabel::ConnectToTimer (void)
{
    mpListener.reset(new Listener(this));
    PresenterClockTimer::Instance(mpToolBar->GetComponentContext())->AddListener(mpListener);
}

//===== CurrentTimeLabel ======================================================

::rtl::Reference<Element> CurrentTimeLabel::Create (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
{
    ::rtl::Reference<TimeLabel> pElement(new CurrentTimeLabel(rpToolBar));
    pElement->ConnectToTimer();
    return ::rtl::Reference<Element>(pElement.get());
}

CurrentTimeLabel::~CurrentTimeLabel (void)
{
}

CurrentTimeLabel::CurrentTimeLabel (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : TimeLabel(rpToolBar),
      maTimeFormatter()
{
}

void CurrentTimeLabel::TimeHasChanged (const oslDateTime& rCurrentTime)
{
    SetText(maTimeFormatter.FormatTime(rCurrentTime));
    Invalidate(false);
}

void CurrentTimeLabel::SetModes (
    const SharedElementMode& rpNormalMode,
    const SharedElementMode& rpMouseOverMode,
    const SharedElementMode& rpSelectedMode,
    const SharedElementMode& rpDisabledMode)
{
    TimeLabel::SetModes(rpNormalMode, rpMouseOverMode, rpSelectedMode, rpDisabledMode);
    SetText(maTimeFormatter.FormatTime(PresenterClockTimer::GetCurrentTime()));
}

//===== PresentationTimeLabel =================================================

::rtl::Reference<Element> PresentationTimeLabel::Create (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
{
    ::rtl::Reference<TimeLabel> pElement(new PresentationTimeLabel(rpToolBar));
    pElement->ConnectToTimer();
    return ::rtl::Reference<Element>(pElement.get());
}

PresentationTimeLabel::~PresentationTimeLabel (void)
{
}

PresentationTimeLabel::PresentationTimeLabel (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : TimeLabel(rpToolBar),
      maTimeFormatter(),
      maStartTimeValue()
{
    maStartTimeValue.Seconds = 0;
    maStartTimeValue.Nanosec = 0;
}

void PresentationTimeLabel::TimeHasChanged (const oslDateTime& rCurrentTime)
{
    TimeValue aCurrentTimeValue;
    if (osl_getTimeValueFromDateTime(&rCurrentTime, &aCurrentTimeValue))
    {
        if (maStartTimeValue.Seconds==0 && maStartTimeValue.Nanosec==0)
        {
            // This method is called for the first time.  Initialize the
            // start time.  The start time is rounded to nearest second to
            // keep the time updates synchronized with the current time label.
            maStartTimeValue = aCurrentTimeValue;
            if (maStartTimeValue.Nanosec >= 500000000)
                maStartTimeValue.Seconds += 1;
            maStartTimeValue.Nanosec = 0;
        }

        TimeValue aElapsedTimeValue;
        aElapsedTimeValue.Seconds = aCurrentTimeValue.Seconds - maStartTimeValue.Seconds;
        aElapsedTimeValue.Nanosec = aCurrentTimeValue.Nanosec - maStartTimeValue.Nanosec;

        oslDateTime aElapsedDateTime;
        if (osl_getDateTimeFromTimeValue(&aElapsedTimeValue, &aElapsedDateTime))
        {
            SetText(maTimeFormatter.FormatTime(aElapsedDateTime));
            Invalidate(false);
        }
    }
}

void PresentationTimeLabel::SetModes (
    const SharedElementMode& rpNormalMode,
    const SharedElementMode& rpMouseOverMode,
    const SharedElementMode& rpSelectedMode,
    const SharedElementMode& rpDisabledMode)
{
    TimeLabel::SetModes(rpNormalMode, rpMouseOverMode, rpSelectedMode, rpDisabledMode);

    oslDateTime aStartDateTime;
    if (osl_getDateTimeFromTimeValue(&maStartTimeValue, &aStartDateTime))
    {
        SetText(maTimeFormatter.FormatTime(aStartDateTime));
    }
}

//===== VerticalSeparator =====================================================

VerticalSeparator::VerticalSeparator (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : Element(rpToolBar)
{
}

void VerticalSeparator::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(rxCanvas.is());

    awt::Rectangle aBBox (GetBoundingBox());

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::OVER);
    if (mpMode.get() != NULL)
    {
        PresenterTheme::SharedFontDescriptor pFont (mpMode->maText.GetFont());
        if (pFont.get() != NULL)
            PresenterCanvasHelper::SetDeviceColor(aRenderState, pFont->mnColor);
    }

    if (aBBox.Height >= gnMinimalSeparatorSize + 2*gnSeparatorInset)
    {
        aBBox.Height -= 2*gnSeparatorInset;
        aBBox.Y += gnSeparatorInset;
    }
    rxCanvas->fillPolyPolygon(
        PresenterGeometryHelper::CreatePolygon(aBBox, rxCanvas->getDevice()),
        rViewState,
        aRenderState);
}

awt::Size VerticalSeparator::CreateBoundingSize (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    (void)rxCanvas;
    return awt::Size(1,20);
}

bool VerticalSeparator::IsFilling (void) const
{
    return true;
}

//===== HorizontalSeparator ===================================================

HorizontalSeparator::HorizontalSeparator (
    const ::rtl::Reference<PresenterToolBar>& rpToolBar)
    : Element(rpToolBar)
{
}

void HorizontalSeparator::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(rxCanvas.is());

    awt::Rectangle aBBox (GetBoundingBox());

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::OVER);
    if (mpMode.get() != NULL)
    {
        PresenterTheme::SharedFontDescriptor pFont (mpMode->maText.GetFont());
        if (pFont.get() != NULL)
            PresenterCanvasHelper::SetDeviceColor(aRenderState, pFont->mnColor);
    }

    if (aBBox.Width >= gnMinimalSeparatorSize+2*gnSeparatorInset)
    {
        aBBox.Width -= 2*gnSeparatorInset;
        aBBox.X += gnSeparatorInset;
    }
    rxCanvas->fillPolyPolygon(
        PresenterGeometryHelper::CreatePolygon(aBBox, rxCanvas->getDevice()),
        rViewState,
        aRenderState);
}

awt::Size HorizontalSeparator::CreateBoundingSize (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    (void)rxCanvas;
    return awt::Size(20,1);
}

bool HorizontalSeparator::IsFilling (void) const
{
    return true;
}

} // end of anonymous namespace

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
