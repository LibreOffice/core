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

#ifndef _VCL_GTKINST_HXX
#define _VCL_GTKINST_HXX

#include <boost/shared_ptr.hpp>

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
                        GtkYieldMutex();
    virtual void        acquire();
    virtual void        release();

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
    void    Init();

    virtual SalFrame*           CreateFrame( SalFrame* pParent, sal_uLong nStyle );
    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle );
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );
    virtual SalSystem*          CreateSalSystem();
    virtual SalInfoPrinter*     CreateInfoPrinter(SalPrinterQueueInfo* pPrinterQueueInfo, ImplJobSetup* pJobSetup);
    virtual SalPrinter*         CreatePrinter( SalInfoPrinter* pInfoPrinter );
    virtual SalMenu*            CreateMenu( sal_Bool, Menu* );
    virtual void                DestroyMenu( SalMenu* pMenu );
    virtual SalMenuItem*        CreateMenuItem( const SalItemParams* );
    virtual void                DestroyMenuItem( SalMenuItem* pItem );
    virtual SalTimer*           CreateSalTimer();
    virtual void                AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService);
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics*,
                                                     long nDX, long nDY,
                                                     sal_uInt16 nBitCount,
                                                     const SystemGraphicsData* );
    virtual SalBitmap*          CreateSalBitmap();

    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool                AnyInput( sal_uInt16 nType );

    virtual GenPspGraphics     *CreatePrintGraphics();

    virtual bool hasNativeFileSelection() const { return true; }

    virtual com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFilePicker2 >
        createFilePicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& );
    virtual com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFolderPicker2 >
        createFolderPicker( const com::sun::star::uno::Reference<
                                com::sun::star::uno::XComponentContext >& );

    void                        RemoveTimer (SalTimer *pTimer);

    // for managing a mirror of the in-flight un-dispatched gdk event queue
    void                        addEvent( sal_uInt16 nMask );
    void                        subtractEvent( sal_uInt16 nMask );

    boost::shared_ptr<vcl::unx::GtkPrintWrapper> getPrintWrapper() const;
  private:
    std::vector<GtkSalTimer *>  m_aTimers;
    bool                        IsTimerExpired();

    mutable boost::shared_ptr<vcl::unx::GtkPrintWrapper> m_pPrintWrapper;
};

#endif // _VCL_GTKINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
