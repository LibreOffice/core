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

QtDragSource::~QtDragSource() {}

void QtDragSource::deinitialize() { m_pFrame = nullptr; }

sal_Bool QtDragSource::isDragImageSupported() { return true; }

sal_Int32 QtDragSource::getDefaultCursor(sal_Int8) { return 0; }

void QtDragSource::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
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

    m_pFrame = reinterpret_cast<QtFrame*>(nFrame);
    m_pFrame->registerDragSource(this);
}

void QtDragSource::startDrag(
    const datatransfer::dnd::DragGestureEvent& /*rEvent*/, sal_Int8 sourceActions,
    sal_Int32 /*cursor*/, sal_Int32 /*image*/,
    const css::uno::Reference<css::datatransfer::XTransferable>& rTrans,
    const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener>& rListener)
{
    m_xListener = rListener;

    if (m_pFrame)
    {
        QDrag* drag = new QDrag(m_pFrame->GetQWidget());
        drag->setMimeData(new QtMimeData(rTrans));
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

void QtDragSource::fire_dragEnd(sal_Int8 nAction, bool bDropSuccessful)
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

OUString SAL_CALL QtDragSource::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclQtDragSource";
}

sal_Bool SAL_CALL QtDragSource::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL QtDragSource::getSupportedServiceNames()
{
    return { "com.sun.star.datatransfer.dnd.QtDragSource" };
}

QtDropTarget::QtDropTarget()
    : WeakComponentImplHelper(m_aMutex)
    , m_pFrame(nullptr)
    , m_bActive(false)
    , m_nDefaultActions(0)
{
}

OUString SAL_CALL QtDropTarget::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclQtDropTarget";
}

sal_Bool SAL_CALL QtDropTarget::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL QtDropTarget::getSupportedServiceNames()
{
    return { "com.sun.star.datatransfer.dnd.QtDropTarget" };
}

QtDropTarget::~QtDropTarget() {}

void QtDropTarget::deinitialize()
{
    m_pFrame = nullptr;
    m_bActive = false;
}

void QtDropTarget::initialize(const uno::Sequence<uno::Any>& rArguments)
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

    m_pFrame = reinterpret_cast<QtFrame*>(nFrame);
    m_pFrame->registerDropTarget(this);
    m_bActive = true;
}

void QtDropTarget::addDropTargetListener(
    const uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_aListeners.push_back(xListener);
}

void QtDropTarget::removeDropTargetListener(
    const uno::Reference<css::datatransfer::dnd::XDropTargetListener>& xListener)
{
    ::osl::Guard<::osl::Mutex> aGuard(m_aMutex);

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), xListener),
                       m_aListeners.end());
}

sal_Bool QtDropTarget::isActive() { return m_bActive; }

void QtDropTarget::setActive(sal_Bool bActive) { m_bActive = bActive; }

sal_Int8 QtDropTarget::getDefaultActions() { return m_nDefaultActions; }

void QtDropTarget::setDefaultActions(sal_Int8 nDefaultActions)
{
    m_nDefaultActions = nDefaultActions;
}

void QtDropTarget::fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
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

void QtDropTarget::fire_dragOver(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
        listener->dragOver(dtde);
}

void QtDropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    m_bDropSuccessful = true;

    osl::ClearableGuard<osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
        listener->drop(dtde);
}

void QtDropTarget::fire_dragExit(const css::datatransfer::dnd::DropTargetEvent& dte)
{
    osl::ClearableGuard<::osl::Mutex> aGuard(m_aMutex);
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(
        m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
        listener->dragExit(dte);
}

void QtDropTarget::acceptDrag(sal_Int8 dragOperation) { m_nDropAction = dragOperation; }

void QtDropTarget::rejectDrag() { m_nDropAction = 0; }

void QtDropTarget::acceptDrop(sal_Int8 dropOperation) { m_nDropAction = dropOperation; }

void QtDropTarget::rejectDrop() { m_nDropAction = 0; }

void QtDropTarget::dropComplete(sal_Bool success)
{
    m_bDropSuccessful = (m_bDropSuccessful && success);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
