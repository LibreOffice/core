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
#include <Qt5Transferable.hxx>
#include <Qt5Widget.hxx>

#include <QtGui/QDrag>

using namespace com::sun::star;

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

    if (m_pFrame)
    {
        QDrag* drag = new QDrag(m_pFrame->GetQWidget());
        drag->setMimeData(new Qt5MimeData(rTrans));
        // just a reminder that exec starts a nested event loop, so everything after
        // this call is just executed, after D'n'D has finished!
        drag->exec(toQtDropActions(sourceActions), getPreferredDropAction(sourceActions));
    }

    // the drop will eventually call fire_dragEnd, which will clear the listener.
    // if D'n'D ends without success, we just get a leave event without any indicator,
    // but the event loop will be terminated, so we have to try to inform the source of
    // a failure in any way.
    fire_dragEnd(datatransfer::dnd::DNDConstants::ACTION_NONE, false);
}

void Qt5DragSource::fire_dragEnd(sal_Int8 nAction, bool bDropSuccessful)
{
    if (!m_xListener.is())
        return;

    datatransfer::dnd::DragSourceDropEvent aEv;
    aEv.DropAction = nAction;
    aEv.DropSuccess = bDropSuccessful;

    auto xListener = m_xListener;
    m_xListener.clear();
    xListener->dragDropEnd(aEv);
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
    return { "com.sun.star.datatransfer.dnd.Qt5DragSource" };
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
    return { "com.sun.star.datatransfer.dnd.Qt5DropTarget" };
}

Qt5DropTarget::~Qt5DropTarget() {}

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

    m_nDropAction = datatransfer::dnd::DNDConstants::ACTION_NONE;

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
        listener->dragOver(dtde);
}

void Qt5DropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    m_bDropSuccessful = true;

    osl::ClearableGuard<osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
        listener->drop(dtde);
}

void Qt5DropTarget::fire_dragExit(const css::datatransfer::dnd::DropTargetEvent& dte)
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
        listener->dragExit(dte);
}

void Qt5DropTarget::acceptDrag(sal_Int8 dragOperation) { m_nDropAction = dragOperation; }

void Qt5DropTarget::rejectDrag() { m_nDropAction = 0; }

void Qt5DropTarget::acceptDrop(sal_Int8 dropOperation) { m_nDropAction = dropOperation; }

void Qt5DropTarget::rejectDrop() { m_nDropAction = 0; }

void Qt5DropTarget::dropComplete(sal_Bool success)
{
    m_bDropSuccessful = (m_bDropSuccessful && success);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
