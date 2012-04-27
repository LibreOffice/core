/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
public:
                                                GtkYieldMutex();
    virtual void                                acquire();
    virtual void                                release();
    virtual sal_Bool                            tryToAcquire();

    virtual int Grab();
    virtual void Ungrab( int );

    class GtkYieldGuard
    {
        GtkYieldMutex*  m_pMutex;
        int             m_nGrab;
    public:
        GtkYieldGuard( GtkYieldMutex* pMutex )
                : m_pMutex( pMutex )
        {
            m_nGrab = m_pMutex->Grab();
        }
        ~GtkYieldGuard()
        {
            m_pMutex->Ungrab( m_nGrab );
        }
    };
};

class GtkHookedYieldMutex : public GtkYieldMutex
{
    virtual int      Grab()             { return 0; };
    virtual void     Ungrab(int )       {};
    std::list<sal_uLong> aYieldStack;
public:
    GtkHookedYieldMutex();
    virtual void      acquire();
    virtual void      release();
    virtual sal_Bool  tryToAcquire() { return SalYieldMutex::tryToAcquire(); }
    void ThreadsEnter();
    void ThreadsLeave();
};


#define GTK_YIELD_GRAB() GtkYieldMutex::GtkYieldGuard aLocalGtkYieldGuard( static_cast<GtkYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex()) )

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
    virtual SalTimer*           CreateSalTimer();
    virtual void                AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics*,
                                                     long nDX, long nDY,
                                                     sal_uInt16 nBitCount,
                                                     const SystemGraphicsData* );
    virtual SalBitmap*			CreateSalBitmap();

    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool				AnyInput( sal_uInt16 nType );

    virtual GenPspGraphics     *CreatePrintGraphics();

    virtual bool hasNativeFileSelection() const { return true; }

    virtual com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFilePicker2 >
        createFilePicker( const com::sun::star::uno::Reference<
                              com::sun::star::uno::XComponentContext >& );
    virtual com::sun::star::uno::Reference< com::sun::star::ui::dialogs::XFolderPicker >
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
