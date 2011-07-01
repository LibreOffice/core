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

#include <unx/salinst.h>
#include <unx/salsys.h>
#include <unx/headless/svpinst.hxx>
#include <gtk/gtk.h>

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

#if GTK_CHECK_VERSION(3,0,0) && !defined GTK3_X11_RENDER
class GtkInstance : public SvpSalInstance
{
    SalYieldMutex *mpSalYieldMutex;
public:
    GtkInstance( SalYieldMutex* pMutex )
        : SvpSalInstance(), mpSalYieldMutex( pMutex )
#else
class GtkInstance : public X11SalInstance
{
public:
    GtkInstance( SalYieldMutex* pMutex )
            : X11SalInstance( pMutex )
#endif
    {}
    virtual ~GtkInstance();

    virtual SalFrame*           CreateFrame( SalFrame* pParent, sal_uLong nStyle );
    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle );
    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );
    virtual SalSystem*          CreateSalSystem();
    virtual void                AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);
    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics*,
                                                     long nDX, long nDY,
                                                     sal_uInt16 nBitCount,
                                                     const SystemGraphicsData* );
    virtual SalBitmap*			CreateSalBitmap();

    virtual osl::SolarMutex*    GetYieldMutex();
    virtual sal_uIntPtr			ReleaseYieldMutex();
    virtual void				AcquireYieldMutex( sal_uIntPtr nCount );
    virtual bool                CheckYieldMutex();
    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool				AnyInput( sal_uInt16 nType );
};

#if GTK_CHECK_VERSION(3,0,0) && !defined GTK3_X11_RENDER
class GtkSalSystem : public SalSystem
{
public:
    GtkSalSystem() : SalSystem() {}
#else
class GtkSalSystem : public X11SalSystem
{
public:
    GtkSalSystem() : X11SalSystem() {}
#endif
    virtual ~GtkSalSystem();

#if GTK_CHECK_VERSION(3,0,0) && !defined GTK3_X11_RENDER
    virtual unsigned int GetDisplayScreenCount();
    virtual bool IsMultiDisplay();
    virtual unsigned int GetDefaultDisplayNumber();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle GetDisplayWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetScreenName( unsigned int nScreen );
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
#endif

    virtual int ShowNativeDialog( const String& rTitle,
                                  const String& rMessage,
                                  const std::list< String >& rButtons,
                                  int nDefButton );
};

#endif // _VCL_GTKINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
