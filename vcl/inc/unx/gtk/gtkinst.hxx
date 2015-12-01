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
    virtual void AfterAppInit() override;

    virtual SalFrame*           CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) override;
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
                                                     const SystemGraphicsData* ) override;
    virtual SalBitmap*          CreateSalBitmap() override;

    virtual SalYieldResult      DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) override;
    virtual bool                AnyInput( VclInputFlags nType ) override;

    virtual GenPspGraphics     *CreatePrintGraphics() override;

    virtual bool hasNativeFileSelection() const override { return true; }

    virtual css::uno::Reference< css::ui::dialogs::XFilePicker2 >
        createFilePicker( const css::uno::Reference< css::uno::XComponentContext >& ) override;
    virtual css::uno::Reference< css::ui::dialogs::XFolderPicker2 >
        createFolderPicker( const css::uno::Reference< css::uno::XComponentContext >& ) override;

#if GTK_CHECK_VERSION(3,0,0)
    virtual css::uno::Reference< css::uno::XInterface > CreateClipboard( const css::uno::Sequence< css::uno::Any >& i_rArguments ) override;
#endif

    virtual const cairo_font_options_t* GetCairoFontOptions() override;

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
