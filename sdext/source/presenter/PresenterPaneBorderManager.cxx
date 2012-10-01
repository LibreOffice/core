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

// The body of this file is only used when PresenterWindowManager defines
// the preprocessor symbol ENABLE_PANE_RESIZING, which by default is not the
// case.
#ifdef ENABLE_PANE_RESIZING

#include "PresenterPaneBorderManager.hxx"
#include "PresenterController.hxx"
#include "PresenterPaintManager.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <cppuhelper/compbase1.hxx>
#include <osl/mutex.hxx>
#include <boost/weak_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== Service ===============================================================

OUString PresenterPaneBorderManager::getImplementationName_static (void)
{
    return OUString("com.sun.star.comp.Draw.PresenterPaneBorderManager");
}

Sequence<OUString> PresenterPaneBorderManager::getSupportedServiceNames_static (void)
{
    static const ::rtl::OUString sServiceName(
        "com.sun.star.drawing.PresenterPaneBorderManager");
    return Sequence<rtl::OUString>(&sServiceName, 1);
}

Reference<XInterface> PresenterPaneBorderManager::Create (const Reference<uno::XComponentContext>& rxContext)
    SAL_THROW((css::uno::Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(
        new PresenterPaneBorderManager(rxContext, NULL)));
}

//===== PresenterPaneBorderManager ============================================

PresenterPaneBorderManager::PresenterPaneBorderManager (
    const Reference<XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterPaneBorderManagerInterfaceBase(m_aMutex),
      mpPresenterController(rpPresenterController),
      mxComponentContext(rxContext),
      mxPresenterHelper(),
      maWindowList(),
      mnPointerType(),
      maDragAnchor(),
      meDragType(Outside),
      mxOuterDragWindow(),
      mxInnerDragWindow(),
      mxPointer()
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPointer = Reference<awt::XPointer>(
            xFactory->createInstanceWithContext(
                OUString("com.sun.star.awt.Pointer"),
                rxContext),
            UNO_QUERY_THROW);

        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString("com.sun.star.comp.Draw.PresenterHelper"),
                rxContext),
            UNO_QUERY_THROW);
    }
}

PresenterPaneBorderManager::~PresenterPaneBorderManager (void)
{
}

void PresenterPaneBorderManager::disposing (void)
{
    WindowList::const_iterator iDescriptor;
    for (iDescriptor=maWindowList.begin(); iDescriptor!=maWindowList.end(); ++iDescriptor)
    {
        iDescriptor->first->removeMouseListener(this);
        iDescriptor->first->removeMouseMotionListener(this);
    }
    maWindowList.clear();
}

namespace {
const static sal_Int32 mnOutside = 0;
const static sal_Int32 mnLeft = 0x01;
const static sal_Int32 mnHorizontalCenter = 0x02;
const static sal_Int32 mnRight = 0x04;
const static sal_Int32 mnTop = 0x10;
const static sal_Int32 mnVerticalCenter = 0x20;
const static sal_Int32 mnBottom = 0x40;
}

PresenterPaneBorderManager::BorderElement
    PresenterPaneBorderManager::ClassifyBorderElementUnderMouse (
        const Reference<awt::XWindow>& rxOuterWindow,
        const Reference<awt::XWindow>& rxInnerWindow,
        const awt::Point aPosition) const
{
    OSL_ASSERT(rxOuterWindow.is());
    OSL_ASSERT(rxInnerWindow.is());

    awt::Rectangle aOuterBox (rxOuterWindow->getPosSize());
    const awt::Rectangle aInnerBox (rxInnerWindow->getPosSize());

    // Coordinates of the pointer position are given in the window
    // coordinate system.  Therefore the upper left corner of the outer box
    // is the origin.
    aOuterBox.X = 0;
    aOuterBox.Y = 0;

    sal_Int32 nCode = 0;

    // Add horizontal classification to nCode.
    if (aPosition.X < aInnerBox.X)
        if (aPosition.X < aOuterBox.X)
            nCode = mnOutside;
        else
            nCode = mnLeft;
    else if (aPosition.X >= aInnerBox.X+aInnerBox.Width)
        if (aPosition.X >= aOuterBox.X+aOuterBox.Width)
            nCode = mnOutside;
        else
            nCode = mnRight;
    else
        nCode = mnHorizontalCenter;

    // Add vertical classification to nCode.
    if (aPosition.Y < aInnerBox.Y)
        if (aPosition.Y < aOuterBox.Y)
            nCode |= mnOutside;
        else
            nCode |= mnTop;
    else if (aPosition.Y >= aInnerBox.Y+aInnerBox.Height)
        if (aPosition.Y >= aOuterBox.Y+aOuterBox.Height)
            nCode |= mnOutside;
        else
            nCode |= mnBottom;
    else
        nCode |= mnVerticalCenter;

    // Translate bits in nCode into BorderElement value.
    switch (nCode)
    {
        case mnOutside | mnOutside:
        case mnOutside | mnLeft:
        case mnOutside | mnRight:
        case mnOutside | mnHorizontalCenter:
        case mnTop | mnOutside:
        case mnBottom | mnOutside:
        case mnVerticalCenter | mnOutside:
        default:
            return Outside;

        case mnVerticalCenter | mnHorizontalCenter:
            return Content;

        case mnTop | mnLeft:
            return TopLeft;

        case mnTop | mnRight:
            return TopRight;

        case mnTop | mnHorizontalCenter:
            return Top;

        case mnBottom | mnLeft:
            return BottomLeft;

        case mnBottom | mnRight:
            return BottomRight;

        case mnBottom | mnHorizontalCenter:
            return Bottom;

        case mnVerticalCenter | mnLeft:
            return Left;

        case mnVerticalCenter | mnRight:
            return Right;
    }
}

//----- XInitialization -------------------------------------------------------

void SAL_CALL PresenterPaneBorderManager::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();

    if (rArguments.getLength()%2 == 1 && mxComponentContext.is())
    {
        try
        {
            mxParentWindow = Reference<awt::XWindow>(rArguments[0], UNO_QUERY_THROW);

            // Get the outer and inner windows from the argument list and
            // build a window list of it.
            for (sal_Int32 nIndex=1; nIndex<rArguments.getLength(); nIndex+=2)
            {
                Reference<awt::XWindow> xOuterWindow (rArguments[nIndex], UNO_QUERY_THROW);
                Reference<awt::XWindow> xInnerWindow (rArguments[nIndex+1], UNO_QUERY_THROW);

                maWindowList.push_back(WindowDescriptor(xOuterWindow,xInnerWindow));

                xOuterWindow->addMouseListener(this);
                xOuterWindow->addMouseMotionListener(this);
            }
        }
        catch (RuntimeException&)
        {
            PresenterPaneBorderManager::disposing();
            throw;
        }
    }
    else
    {
        throw RuntimeException(
            OUString("PresenterPane: invalid number of arguments"),
                static_cast<XWeak*>(this));
    }
}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterPaneBorderManager::mousePressed (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    // Find window descriptor of the window that has been clicked.
    WindowList::const_iterator iDescriptor;
    for (iDescriptor=maWindowList.begin(); iDescriptor!=maWindowList.end(); ++iDescriptor)
        if (iDescriptor->first == rEvent.Source)
            break;

    if (iDescriptor != maWindowList.end())
    {
        // Prepare dragging.
        mxOuterDragWindow = iDescriptor->first;
        mxInnerDragWindow = iDescriptor->second;
        OSL_ASSERT(mxOuterDragWindow.is() && mxInnerDragWindow.is());
        const awt::Rectangle aOuterBox (mxOuterDragWindow->getPosSize());
        maDragAnchor.X = rEvent.X + aOuterBox.X;
        maDragAnchor.Y = rEvent.Y + aOuterBox.Y;
        meDragType = ClassifyBorderElementUnderMouse(
            mxOuterDragWindow,
            mxInnerDragWindow,
            awt::Point(rEvent.X, rEvent.Y));
    }
}

void SAL_CALL PresenterPaneBorderManager::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    ReleaseMouse(mxOuterDragWindow);
    meDragType = PresenterPaneBorderManager::Outside;
    mxOuterDragWindow = NULL;
    mxInnerDragWindow = NULL;
}

void SAL_CALL PresenterPaneBorderManager::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterPaneBorderManager::mouseExited (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    ReleaseMouse(mxOuterDragWindow);
    meDragType = PresenterPaneBorderManager::Outside;
    mxOuterDragWindow = NULL;
    mxInnerDragWindow = NULL;
}

//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterPaneBorderManager::mouseMoved (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    WindowList::const_iterator iDescriptor;
    for (iDescriptor=maWindowList.begin(); iDescriptor!=maWindowList.end(); ++iDescriptor)
        if (iDescriptor->first == rEvent.Source)
            break;
    if (iDescriptor != maWindowList.end())
    {
        // Choose pointer shape according to position in the window border.
        switch (ClassifyBorderElementUnderMouse(
            iDescriptor->first,
            iDescriptor->second,
            awt::Point(rEvent.X,rEvent.Y)))
        {
            case PresenterPaneBorderManager::Top:
                mnPointerType = awt::SystemPointer::MOVE;
                break;
            case PresenterPaneBorderManager::TopLeft:
                mnPointerType = awt::SystemPointer::WINDOW_NWSIZE;
                break;
            case PresenterPaneBorderManager::TopRight:
                mnPointerType = awt::SystemPointer::WINDOW_NESIZE;
                break;
            case PresenterPaneBorderManager::Left:
                mnPointerType = awt::SystemPointer::WINDOW_WSIZE;
                break;
            case PresenterPaneBorderManager::Right:
                mnPointerType = awt::SystemPointer::WINDOW_ESIZE;
                break;
            case PresenterPaneBorderManager::BottomLeft:
                mnPointerType = awt::SystemPointer::WINDOW_SWSIZE;
                break;
            case PresenterPaneBorderManager::BottomRight:
                mnPointerType = awt::SystemPointer::WINDOW_SESIZE;
                break;
            case PresenterPaneBorderManager::Bottom:
                mnPointerType = awt::SystemPointer::WINDOW_SSIZE;
                break;

            case PresenterPaneBorderManager::Content:
            case PresenterPaneBorderManager::Outside:
            default:
                mnPointerType = awt::SystemPointer::ARROW;
                break;
        }

        // Make the pointer shape visible.
        Reference<awt::XWindowPeer> xPeer (iDescriptor->first, UNO_QUERY);
        if (xPeer.is())
        {
            if (mxPointer.is())
                mxPointer->setType(mnPointerType);
            xPeer->setPointer(mxPointer);
        }
    }
}

void SAL_CALL PresenterPaneBorderManager::mouseDragged (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    if ( ! mxOuterDragWindow.is())
        return;

    CaptureMouse(mxOuterDragWindow);

    const awt::Rectangle aOldBox (mxOuterDragWindow->getPosSize());
    const sal_Int32 nX = rEvent.X + aOldBox.X;
    const sal_Int32 nY = rEvent.Y + aOldBox.Y;
    const sal_Int32 nDiffX = nX - maDragAnchor.X;
    const sal_Int32 nDiffY = nY - maDragAnchor.Y;
    maDragAnchor.X = nX;
    maDragAnchor.Y = nY;

    const sal_Int32 nOldRight = aOldBox.X + aOldBox.Width;
    const sal_Int32 nOldBottom = aOldBox.Y + aOldBox.Height;

    awt::Rectangle aBox (aOldBox);
    sal_Int32 nRight = aBox.X + aBox.Width;
    sal_Int32 nBottom = aBox.Y + aBox.Height;

    // Update position and/or size according to initial pointer position
    // inside the window border.
    switch (meDragType)
    {
        case PresenterPaneBorderManager::Top:
            aBox.X += nDiffX; aBox.Y += nDiffY;
            nRight += nDiffX; nBottom += nDiffY;
            break;
        case PresenterPaneBorderManager::TopLeft:
            aBox.X += nDiffX; aBox.Y += nDiffY;
            break;
        case PresenterPaneBorderManager::TopRight:
            nRight += nDiffX; aBox.Y += nDiffY;
            break;
        case PresenterPaneBorderManager::Left:
            aBox.X += nDiffX;
            break;
        case PresenterPaneBorderManager::Right:
            nRight += nDiffX;
            break;
        case PresenterPaneBorderManager::BottomLeft:
            aBox.X += nDiffX; nBottom += nDiffY;
            break;
        case PresenterPaneBorderManager::BottomRight:
            nRight += nDiffX; nBottom += nDiffY;
            break;
        case PresenterPaneBorderManager::Bottom:
            nBottom += nDiffY;
            break;
        default: break;
    }

    aBox.Width = nRight - aBox.X;
    aBox.Height = nBottom - aBox.Y;
    if (aBox.Width > 20
        && aBox.Height > 20)
    {
        // Set position and/or size of the border window to the new values.
        sal_Int16 nFlags (0);
        if (aBox.X != aOldBox.X)
            nFlags |= awt::PosSize::X;
        if (aBox.Y != aOldBox.Y)
            nFlags |= awt::PosSize::Y;
        if (aBox.Width != aOldBox.Width)
            nFlags |= awt::PosSize::WIDTH;
        if (aBox.Height != aOldBox.Height)
            nFlags |= awt::PosSize::HEIGHT;
        mxOuterDragWindow->setPosSize(aBox.X, aBox.Y, aBox.Width, aBox.Height, nFlags);

        // Invalidate that is or was covered by the border window before and
        // after the move/resize.
        if (mpPresenterController.get() != NULL)
        {
            const sal_Int32 nLeft = ::std::min(aOldBox.X,aBox.X);
            const sal_Int32 nTop = ::std::min(aOldBox.Y,aBox.Y);
            const sal_Int32 nWidth = ::std::max(nOldRight,nRight) - nLeft;
            const sal_Int32 nHeight = ::std::max(nOldBottom,nBottom) - nTop;

            OSL_ASSERT(mpPresenterController->GetPaintManager().get()!=NULL);
            mpPresenterController->GetPaintManager()->Invalidate(
                mxParentWindow,
                ::awt::Rectangle(nLeft,nTop,nWidth-1,nHeight-1));
        }
    }
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterPaneBorderManager::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    WindowList::iterator iDescriptor;
    for (iDescriptor=maWindowList.begin(); iDescriptor!=maWindowList.end(); ++iDescriptor)
        if (iDescriptor->first == rEvent.Source)
        {
            maWindowList.erase(iDescriptor);
            break;
        }
}

//-----------------------------------------------------------------------------

void PresenterPaneBorderManager::CaptureMouse (const Reference<awt::XWindow>& rxWindow)
{
    if (mxPresenterHelper.is())
        mxPresenterHelper->captureMouse(rxWindow);
}

void PresenterPaneBorderManager::ReleaseMouse (const Reference<awt::XWindow>& rxWindow)
{
    if (mxPresenterHelper.is())
        mxPresenterHelper->releaseMouse(rxWindow);
}

void PresenterPaneBorderManager::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString( "PresenterPaneBorderManager object has already been disposed"),
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::presenter

#endif // ENABLE_PANE_RESIZING

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
