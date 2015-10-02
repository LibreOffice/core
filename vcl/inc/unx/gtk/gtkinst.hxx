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

#include <unx/salinst.h>
#include <generic/gensys.h>
#include <headless/svpinst.hxx>
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
    std::list<sal_uLong> aYieldStack;

public:
         GtkYieldMutex() {}
    void ThreadsEnter();
    void ThreadsLeave();
};

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
    virtual ~GtkInstance();
    void    EnsureInit();
    virtual void AfterAppInit() SAL_OVERRIDE;

    virtual SalFrame*           CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) SAL_OVERRIDE;
    virtual SalSystem*          CreateSalSystem() SAL_OVERRIDE;
    virtual SalInfoPrinter*     CreateInfoPrinter(SalPrinterQueueInfo* pPrinterQueueInfo, ImplJobSetup* pJobSetup) SAL_OVERRIDE;
    virtual SalPrinter*         CreatePrinter( SalInfoPrinter* pInfoPrinter ) SAL_OVERRIDE;
    virtual SalMenu*            CreateMenu( bool, Menu* ) SAL_OVERRIDE;
    virtual void                DestroyMenu( SalMenu* pMenu ) SAL_OVERRIDE;
    virtual SalMenuItem*        CreateMenuItem( const SalItemParams* ) SAL_OVERRIDE;
    virtual void                DestroyMenuItem( SalMenuItem* pItem ) SAL_OVERRIDE;
    virtual SalTimer*           CreateSalTimer() SAL_OVERRIDE;
    virtual void                AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) SAL_OVERRIDE;
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics*,
                                                     long &nDX, long &nDY,
                                                     sal_uInt16 nBitCount,
                                                     const SystemGraphicsData* ) SAL_OVERRIDE;
    virtual SalBitmap*          CreateSalBitmap() SAL_OVERRIDE;

    virtual void                DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) SAL_OVERRIDE;
    virtual bool                AnyInput( VclInputFlags nType ) SAL_OVERRIDE;

    virtual GenPspGraphics     *CreatePrintGraphics() SAL_OVERRIDE;

    virtual bool hasNativeFileSelection() const SAL_OVERRIDE { return true; }

    virtual com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFilePicker2 >
        createFilePicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFolderPicker2 >
        createFolderPicker( const com::sun::star::uno::Reference<
                                com::sun::star::uno::XComponentContext >& ) SAL_OVERRIDE;

#if GTK_CHECK_VERSION(3,0,0)
    virtual css::uno::Reference< css::uno::XInterface > CreateClipboard( const css::uno::Sequence< css::uno::Any >& i_rArguments ) SAL_OVERRIDE;
#endif

    void                        RemoveTimer (SalTimer *pTimer);

    std::shared_ptr<vcl::unx::GtkPrintWrapper> getPrintWrapper() const;

private:
    std::vector<GtkSalTimer *>  m_aTimers;
    bool                        IsTimerExpired();
    bool                        bNeedsInit;

    mutable std::shared_ptr<vcl::unx::GtkPrintWrapper> m_xPrintWrapper;
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
