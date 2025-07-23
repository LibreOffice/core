/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <DropTarget.hxx>

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>

#include <QtCore/QObject>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>

class QtFrame;

class QtDragSource final : public QObject,
                           public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDragSource,
                                                                css::lang::XServiceInfo>
{
    Q_OBJECT

    osl::Mutex m_aMutex;
    QtFrame* m_pFrame;
    css::uno::Reference<css::datatransfer::dnd::XDragSourceListener> m_xListener;

public:
    QtDragSource(QtFrame* pFrame);

    virtual ~QtDragSource() override;

    // XDragSource
    virtual sal_Bool SAL_CALL isDragImageSupported() override;
    virtual sal_Int32 SAL_CALL getDefaultCursor(sal_Int8 dragAction) override;
    virtual void SAL_CALL startDrag(
        const css::datatransfer::dnd::DragGestureEvent& trigger, sal_Int8 sourceActions,
        sal_Int32 cursor, sal_Int32 image,
        const css::uno::Reference<css::datatransfer::XTransferable>& transferable,
        const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener>& listener) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void fire_dragEnd(sal_Int8 nAction, bool bSuccessful);
};

class QtDropTarget final
    : public QObject,
      public cppu::ImplInheritanceHelper<DropTarget, css::datatransfer::dnd::XDropTargetDragContext,
                                         css::datatransfer::dnd::XDropTargetDropContext,
                                         css::lang::XServiceInfo>
{
    Q_OBJECT

    sal_Int8 m_nDropAction;
    bool m_bDropSuccessful;

public:
    QtDropTarget();
    virtual ~QtDropTarget() override;

    // XDropTargetDragContext
    virtual void SAL_CALL acceptDrag(sal_Int8 dragOperation) override;
    virtual void SAL_CALL rejectDrag() override;

    // XDropTargetDropContext
    virtual void SAL_CALL acceptDrop(sal_Int8 dropOperation) override;
    virtual void SAL_CALL rejectDrop() override;
    virtual void SAL_CALL dropComplete(sal_Bool success) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void handleDragEnterEvent(QDragEnterEvent& rEvent, qreal fScaleFactor = 1.0);
    void handleDragMoveEvent(QDragMoveEvent& rEvent, qreal fScaleFactor = 1.0);
    void handleDropEvent(QDropEvent& rEvent, qreal fScaleFactor = 1.0);

    sal_Int8 proposedDropAction() const { return m_nDropAction; }
    bool dropSuccessful() const { return m_bDropSuccessful; }

private:
    css::datatransfer::dnd::DropTargetDragEnterEvent
    createDropTargetDragEnterEvent(const QDragMoveEvent& rEvent, bool bSetDataFlavors,
                                   qreal fScaleFactor);
    css::datatransfer::dnd::DropTargetDropEvent createDropTargetDropEvent(const QDropEvent& rEvent,
                                                                          qreal fScaleFactor);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
