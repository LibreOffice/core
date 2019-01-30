/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <cppuhelper/compbase.hxx>
#include "Qt5Clipboard.hxx"

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

class Qt5Frame;
class QMimeData;

class Qt5DnDTransferable : public Qt5Transferable
{
public:
    Qt5DnDTransferable(const QMimeData* pMimeData);
    virtual css::uno::Any SAL_CALL
    getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;

    virtual std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector() override;

private:
    const QMimeData* m_pMimeData;
};

class Qt5DragSource
    : public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDragSource,
                                           css::lang::XInitialization, css::lang::XServiceInfo>
{
    osl::Mutex m_aMutex;
    Qt5Frame* m_pFrame;
    css::uno::Reference<css::datatransfer::dnd::XDragSourceListener> m_xListener;
    css::uno::Reference<css::datatransfer::XTransferable> m_xTrans;

public:
    Qt5DragSource()
        : WeakComponentImplHelper(m_aMutex)
        , m_pFrame(nullptr)
    {
    }

    virtual ~Qt5DragSource() override;

    // XDragSource
    virtual sal_Bool SAL_CALL isDragImageSupported() override;
    virtual sal_Int32 SAL_CALL getDefaultCursor(sal_Int8 dragAction) override;
    virtual void SAL_CALL startDrag(
        const css::datatransfer::dnd::DragGestureEvent& trigger, sal_Int8 sourceActions,
        sal_Int32 cursor, sal_Int32 image,
        const css::uno::Reference<css::datatransfer::XTransferable>& transferable,
        const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener>& listener) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;
    void deinitialize();

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void dragFailed();
    void fire_dragEnd(sal_Int8 nAction);

    static Qt5DragSource* m_ActiveDragSource;

    css::uno::Reference<css::datatransfer::XTransferable> const& GetTransferable() const
    {
        return m_xTrans;
    }
};

class Qt5DropTarget
    : public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDropTarget,
                                           css::datatransfer::dnd::XDropTargetDragContext,
                                           css::datatransfer::dnd::XDropTargetDropContext,
                                           css::lang::XInitialization, css::lang::XServiceInfo>
{
    osl::Mutex m_aMutex;
    Qt5Frame* m_pFrame;
    sal_Int8 mnDragAction;
    sal_Int8 mnDropAction;
    bool m_bActive;
    sal_Int8 m_nDefaultActions;
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> m_aListeners;

public:
    Qt5DropTarget();
    virtual ~Qt5DropTarget() override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArgs) override;
    void deinitialize();

    // XDropTarget
    virtual void SAL_CALL addDropTargetListener(
        const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override;
    virtual void SAL_CALL removeDropTargetListener(
        const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override;
    virtual sal_Bool SAL_CALL isActive() override;
    virtual void SAL_CALL setActive(sal_Bool active) override;
    virtual sal_Int8 SAL_CALL getDefaultActions() override;
    virtual void SAL_CALL setDefaultActions(sal_Int8 actions) override;

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

    void fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde);
    void fire_dragOver(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde);
    void fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde);

    sal_Int8 proposedDragAction() const { return mnDragAction; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
