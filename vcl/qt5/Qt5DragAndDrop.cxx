/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <Qt5DragAndDrop.hxx>
#include <Qt5Frame.hxx>
#include <Qt5Widget.hxx>

using namespace com::sun::star;

bool Qt5DragSource::m_bDropSuccessSet = false;
bool Qt5DragSource::m_bDropSuccess = false;
Qt5DragSource* Qt5DragSource::m_ActiveDragSource = nullptr;

Qt5DragSource::~Qt5DragSource() {}

void Qt5DragSource::deinitialize() { m_pFrame = nullptr; }

sal_Bool Qt5DragSource::isDragImageSupported() { return true; }

sal_Int32 Qt5DragSource::getDefaultCursor(sal_Int8) { return 0; }

void Qt5DragSource::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    if (rArguments.getLength() < 2)
    {
        throw uno::RuntimeException("DragSource::initialize: Cannot install window event handler",
                                    static_cast<OWeakObject*>(this));
    }

    sal_IntPtr nFrame = 0;
    rArguments.getConstArray()[1] >>= nFrame;

    if (!nFrame)
    {
        throw uno::RuntimeException("DragSource::initialize: missing SalFrame",
                                    static_cast<OWeakObject*>(this));
    }

    m_pFrame = reinterpret_cast<Qt5Frame*>(nFrame);
    m_pFrame->registerDragSource(this);
}

void Qt5DragSource::startDrag(
    const datatransfer::dnd::DragGestureEvent& /*rEvent*/, sal_Int8 sourceActions,
    sal_Int32 /*cursor*/, sal_Int32 /*image*/,
    const css::uno::Reference<css::datatransfer::XTransferable>& rTrans,
    const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener>& rListener)
{
    m_xListener = rListener;
    m_xTrans = rTrans;

    if (m_pFrame)
    {
        Qt5Widget* qw = static_cast<Qt5Widget*>(m_pFrame->GetQWidget());
        m_ActiveDragSource = this;
        m_bDropSuccessSet = false;
        m_bDropSuccess = false;
        qw->startDrag(sourceActions);
    }
    else
        dragFailed();
}

void Qt5DragSource::dragFailed()
{
    if (m_xListener.is())
    {
        datatransfer::dnd::DragSourceDropEvent aEv;
        aEv.DropAction = datatransfer::dnd::DNDConstants::ACTION_NONE;
        aEv.DropSuccess = false;
        auto xListener = m_xListener;
        m_xListener.clear();
        xListener->dragDropEnd(aEv);
    }
}

void Qt5DragSource::fire_dragEnd(sal_Int8 nAction)
{
    if (m_xListener.is())
    {
        datatransfer::dnd::DragSourceDropEvent aEv;
        aEv.DropAction = nAction;

        // internal DnD can accept the drop
        // but still fail in Qt5DropTarget::dropComplete
        if (m_bDropSuccessSet)
            aEv.DropSuccess = m_bDropSuccess;
        else
            aEv.DropSuccess = true;

        auto xListener = m_xListener;
        m_xListener.clear();
        xListener->dragDropEnd(aEv);
    }
    m_ActiveDragSource = nullptr;
}

OUString SAL_CALL Qt5DragSource::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclQt5DragSource";
}

sal_Bool SAL_CALL Qt5DragSource::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL Qt5DragSource::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet{ "com.sun.star.datatransfer.dnd.Qt5DragSource" };
    return aRet;
}

Qt5DropTarget::Qt5DropTarget()
    : WeakComponentImplHelper(m_aMutex)
    , m_pFrame(nullptr)
    , m_bActive(false)
    , m_nDefaultActions(0)
{
}

OUString SAL_CALL Qt5DropTarget::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclQt5DropTarget";
}

sal_Bool SAL_CALL Qt5DropTarget::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL Qt5DropTarget::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet{ "com.sun.star.datatransfer.dnd.Qt5DropTarget" };
    return aRet;
}

Qt5DropTarget::~Qt5DropTarget()
{
    //if (m_pFrame)
    //m_pFrame->deregisterDropTarget(this);
}

void Qt5DropTarget::deinitialize()
{
    m_pFrame = nullptr;
    m_bActive = false;
}

void Qt5DropTarget::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    if (rArguments.getLength() < 2)
    {
        throw uno::RuntimeException("DropTarget::initialize: Cannot install window event handler",
                                    static_cast<OWeakObject*>(this));
    }

    sal_IntPtr nFrame = 0;
    rArguments.getConstArray()[1] >>= nFrame;

    if (!nFrame)
    {
        throw uno::RuntimeException("DropTarget::initialize: missing SalFrame",
                                    static_cast<OWeakObject*>(this));
    }

    mnDragAction = datatransfer::dnd::DNDConstants::ACTION_NONE;
    mnDropAction = datatransfer::dnd::DNDConstants::ACTION_NONE;

    m_pFrame = reinterpret_cast<Qt5Frame*>(nFrame);
    m_pFrame->registerDropTarget(this);
    m_bActive = true;
}

void Qt5DropTarget::addDropTargetListener(
    const uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_aListeners.push_back(xListener);
}

void Qt5DropTarget::removeDropTargetListener(
    const uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), xListener),
                       m_aListeners.end());
}

sal_Bool Qt5DropTarget::isActive() { return m_bActive; }

void Qt5DropTarget::setActive(sal_Bool bActive) { m_bActive = bActive; }

sal_Int8 Qt5DropTarget::getDefaultActions() { return m_nDefaultActions; }

void Qt5DropTarget::setDefaultActions(sal_Int8 nDefaultActions)
{
    m_nDefaultActions = nDefaultActions;
}

void Qt5DropTarget::fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragEnter(dtde);
    }
}

void Qt5DropTarget::fire_dragOver(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragOver(dtde);
    }
}

void Qt5DropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    osl::ClearableGuard<osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->drop(dtde);
    }
}

void Qt5DropTarget::acceptDrag(sal_Int8 dragOperation)
{
    mnDragAction = dragOperation;
    return;
}

void Qt5DropTarget::rejectDrag()
{
    mnDragAction = 0;
    return;
}

void Qt5DropTarget::acceptDrop(sal_Int8 dropOperation)
{
    mnDropAction = dropOperation;
    return;
}

void Qt5DropTarget::rejectDrop()
{
    mnDropAction = 0;
    return;
}

void Qt5DropTarget::dropComplete(sal_Bool success)
{
    // internal DnD
    if (Qt5DragSource::m_ActiveDragSource)
    {
        Qt5DragSource::m_bDropSuccessSet = true;
        Qt5DragSource::m_bDropSuccess = success;
    }

    return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
