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

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKINST_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKINST_HXX

#include <sal/config.h>

#include <stack>

#include <unx/salinst.h>
#include <unx/gensys.h>
#include <headless/svpinst.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>
#include <gtk/gtk.h>

namespace vcl
{
namespace unx
{
class GtkPrintWrapper;
}
}

class GenPspGraphics;
class GtkYieldMutex : public SalYieldMutex
{
    thread_local static std::stack<sal_uIntPtr> yieldCounts;

public:
         GtkYieldMutex() {}
    void ThreadsEnter();
    void ThreadsLeave();
};

#if GTK_CHECK_VERSION(3,0,0)
class GtkSalFrame;

struct VclToGtkHelper
{
    std::vector<css::datatransfer::DataFlavor> aInfoToFlavor;
    std::vector<GtkTargetEntry> FormatsToGtk(const css::uno::Sequence<css::datatransfer::DataFlavor> &rFormats);
    void setSelectionData(const css::uno::Reference<css::datatransfer::XTransferable> &rTrans,
                          GtkSelectionData *selection_data, guint info);
private:
    GtkTargetEntry makeGtkTargetEntry(const css::datatransfer::DataFlavor& rFlavor);
};

class GtkTransferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
protected:
    std::map<OUString, GdkAtom> m_aMimeTypeToAtom;

    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector(GdkAtom *targets, gint n_targets);
public:

    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override = 0;

    virtual std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector() = 0;

    virtual css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

class GtkDropTarget : public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDropTarget,
                                                           css::lang::XInitialization,
                                                           css::lang::XServiceInfo>
{
    osl::Mutex m_aMutex;
    GtkSalFrame* m_pFrame;
    bool m_bActive;
    sal_Int8 m_nDefaultActions;
    std::list<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> m_aListeners;
public:
    GtkDropTarget();
    virtual ~GtkDropTarget() override;

    // XInitialization
    virtual void        SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArgs) override;
            void        deinitialize();

    // XDropTarget
    virtual void        SAL_CALL addDropTargetListener(const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override;
    virtual void        SAL_CALL removeDropTargetListener(const css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>&) override;
    virtual sal_Bool    SAL_CALL isActive() override;
    virtual void        SAL_CALL setActive(sal_Bool active) override;
    virtual sal_Int8    SAL_CALL getDefaultActions() override;
    virtual void        SAL_CALL setDefaultActions(sal_Int8 actions) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee);
    void fire_dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde);
    void fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde);
    void fire_dragExit(const css::datatransfer::dnd::DropTargetEvent& dte);
};

class GtkDragSource : public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDragSource,
                                                           css::lang::XInitialization,
                                                           css::lang::XServiceInfo>
{
    osl::Mutex m_aMutex;
    GtkSalFrame* m_pFrame;
    css::uno::Reference<css::datatransfer::dnd::XDragSourceListener> m_xListener;
    css::uno::Reference<css::datatransfer::XTransferable> m_xTrans;
    VclToGtkHelper m_aConversionHelper;
public:
    GtkDragSource()
        : WeakComponentImplHelper(m_aMutex)
        , m_pFrame(nullptr)
    {
    }

    virtual ~GtkDragSource() override;

    // XDragSource
    virtual sal_Bool    SAL_CALL isDragImageSupported() override;
    virtual sal_Int32   SAL_CALL getDefaultCursor(sal_Int8 dragAction) override;
    virtual void        SAL_CALL startDrag(
        const css::datatransfer::dnd::DragGestureEvent& trigger, sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
        const css::uno::Reference< css::datatransfer::XTransferable >& transferable,
        const css::uno::Reference< css::datatransfer::dnd::XDragSourceListener >& listener) override;

    // XInitialization
    virtual void        SAL_CALL initialize(const css::uno::Sequence<css::uno::Any >& rArguments) override;
            void        deinitialize();

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void dragFailed();
    void dragDelete();
    void dragEnd(GdkDragContext* context);
    void dragDataGet(GtkSelectionData *data, guint info);

    // For LibreOffice internal D&D we provide the Transferable without Gtk
    // intermediaries as a shortcut, see tdf#100097 for how dbaccess depends on this
    static GtkDragSource* g_ActiveDragSource;
    css::uno::Reference<css::datatransfer::XTransferable> const & GetTransferrable() const { return m_xTrans; }
};

#endif

class GtkSalTimer;
#if GTK_CHECK_VERSION(3,0,0)
class GtkInstance : public SvpSalInstance
#else
class GtkInstance : public X11SalInstance
#endif
{
#if GTK_CHECK_VERSION(3,0,0)
    typedef SvpSalInstance Superclass_t;
#else
    typedef X11SalInstance Superclass_t;
#endif
public:
            GtkInstance( SalYieldMutex* pMutex );
    virtual ~GtkInstance() override;
    void    EnsureInit();
    virtual void AfterAppInit() override;

    virtual SalFrame*           CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow ) override;
#if !GTK_CHECK_VERSION(3,0,0)
    virtual SalI18NImeStatus*   CreateI18NImeStatus() override;
#endif
    virtual SalSystem*          CreateSalSystem() override;
    virtual SalInfoPrinter*     CreateInfoPrinter(SalPrinterQueueInfo* pPrinterQueueInfo, ImplJobSetup* pJobSetup) override;
    virtual SalPrinter*         CreatePrinter( SalInfoPrinter* pInfoPrinter ) override;
    virtual SalMenu*            CreateMenu( bool, Menu* ) override;
    virtual void                DestroyMenu( SalMenu* pMenu ) override;
    virtual SalMenuItem*        CreateMenuItem( const SalItemParams* ) override;
    virtual void                DestroyMenuItem( SalMenuItem* pItem ) override;
    virtual SalTimer*           CreateSalTimer() override;
    virtual void                AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) override;
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics*,
                                                     long &nDX, long &nDY,
                                                     DeviceFormat eFormat,
                                                     const SystemGraphicsData* = nullptr ) override;
    virtual SalBitmap*          CreateSalBitmap() override;

    virtual bool                DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) override;
    virtual bool                AnyInput( VclInputFlags nType ) override;
    // impossible to handle correctly, as "main thread" depends on the dispatch mutex
    virtual bool                IsMainThread() const override { return false; }

    virtual GenPspGraphics     *CreatePrintGraphics() override;

    virtual bool hasNativeFileSelection() const override { return true; }

    virtual css::uno::Reference< css::ui::dialogs::XFilePicker2 >
        createFilePicker( const css::uno::Reference< css::uno::XComponentContext >& ) override;
    virtual css::uno::Reference< css::ui::dialogs::XFolderPicker2 >
        createFolderPicker( const css::uno::Reference< css::uno::XComponentContext >& ) override;

#if GTK_CHECK_VERSION(3,0,0)
    virtual css::uno::Reference< css::uno::XInterface > CreateClipboard( const css::uno::Sequence< css::uno::Any >& i_rArguments ) override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDragSource() override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDropTarget() override;
    virtual OpenGLContext* CreateOpenGLContext() override;
#endif

    virtual const cairo_font_options_t* GetCairoFontOptions() override;
            const cairo_font_options_t* GetLastSeenCairoFontOptions();
                                   void ResetLastSeenCairoFontOptions();

    void                        RemoveTimer();

    std::shared_ptr<vcl::unx::GtkPrintWrapper> const & getPrintWrapper() const;

private:
    GtkSalTimer*                m_pTimer;
#if GTK_CHECK_VERSION(3,0,0)
    std::unordered_map< GdkAtom, css::uno::Reference<css::uno::XInterface> > m_aClipboards;
#endif
    bool                        IsTimerExpired();
    bool                        bNeedsInit;
    cairo_font_options_t*       m_pLastCairoFontOptions;

    mutable std::shared_ptr<vcl::unx::GtkPrintWrapper> m_xPrintWrapper;
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
