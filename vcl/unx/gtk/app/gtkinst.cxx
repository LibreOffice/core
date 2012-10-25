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


#include <stack>
#include <string.h>
#include <osl/module.h>
#define GLIB_DISABLE_DEPRECATION_WARNINGS
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/salobj.h>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkobject.hxx>
#include <unx/gtk/atkbridge.hxx>
#include <unx/gtk/gtkprn.hxx>
#include <headless/svpvd.hxx>
#include <headless/svpbmp.hxx>
#include <vcl/apptypes.hxx>
#include <generic/genpspgraphics.h>
#include <rtl/strbuf.hxx>

#include <rtl/uri.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#include "gtkprintwrapper.hxx"

GtkHookedYieldMutex::GtkHookedYieldMutex()
{
}

/*
 * These methods always occur in pairs
 * A ThreadsEnter is followed by a ThreadsLeave
 * We need to queue up the recursive lock count
 * for each pair, so we can accurately restore
 * it later.
 */
void GtkHookedYieldMutex::ThreadsEnter()
{
    acquire();
    if( !aYieldStack.empty() )
    { /* Previously called ThreadsLeave() */
        sal_uLong nCount = aYieldStack.front();
        aYieldStack.pop_front();
        while( nCount-- > 1 )
            acquire();
    }
}

void GtkHookedYieldMutex::ThreadsLeave()
{
    aYieldStack.push_front( mnCount );

#if OSL_DEBUG_LEVEL > 1
    if( mnThreadId &&
        mnThreadId != osl::Thread::getCurrentIdentifier())
        fprintf( stderr, "\n\n--- A different thread owns the mutex ...---\n\n\n");
#endif

    while( mnCount > 1 )
        release();
    release();
}

void GtkHookedYieldMutex::acquire()
{
    SalYieldMutex::acquire();
}

void GtkHookedYieldMutex::release()
{
    SalYieldMutex::release();
}

extern "C"
{
    #define GET_YIELD_MUTEX() static_cast<GtkHookedYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())
    static void GdkThreadsEnter( void )
    {
        GtkHookedYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsEnter();
    }
    static void GdkThreadsLeave( void )
    {
        GtkHookedYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsLeave();
    }
    static bool hookLocks( oslModule pModule )
    {
        typedef void (*GdkLockFn) (GCallback enter_fn, GCallback leave_fn);

        GdkLockFn gdk_threads_set_lock_functions =
                (GdkLockFn) osl_getAsciiFunctionSymbol( pModule, "gdk_threads_set_lock_functions" );
        if ( !gdk_threads_set_lock_functions )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Failed to hook gdk threads locks\n" );
#endif
            return false;
        }

        gdk_threads_set_lock_functions (GdkThreadsEnter, GdkThreadsLeave);
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Hooked gdk threads locks\n" );
#endif
        return true;
    }

    VCLPLUG_GTK_PUBLIC SalInstance* create_SalInstance( oslModule pModule )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "create vcl plugin instance with gtk version %d %d %d\n",
                 (int) gtk_major_version, (int) gtk_minor_version,
                 (int) gtk_micro_version );
#endif
        /* #i92121# workaround deadlocks in the X11 implementation
        */
        static const char* pNoXInitThreads = getenv( "SAL_NO_XINITTHREADS" );
        /* #i90094#
           from now on we know that an X connection will be
           established, so protect X against itself
        */
        if( ! ( pNoXInitThreads && *pNoXInitThreads ) )
            XInitThreads();

#if GTK_CHECK_VERSION(3,0,0)
        const gchar* pVersion = gtk_check_version( 3, 2, 0 );
#else
        const gchar* pVersion = gtk_check_version( 2, 2, 0 );
#endif
        if( pVersion )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "gtk version conflict: %s\n", pVersion );
#endif
            return NULL;
        }

        GtkYieldMutex *pYieldMutex;

        // init gdk thread protection
        if ( !g_thread_supported() )
            g_thread_init( NULL );

        if ( hookLocks( pModule ) )
            pYieldMutex = new GtkHookedYieldMutex();
        else
#if GTK_CHECK_VERSION(3,0,0)
            g_error ("impossible case for gtk3");
#else
            pYieldMutex = new GtkYieldMutex();
#endif

        gdk_threads_init();

        GtkInstance* pInstance = new GtkInstance( pYieldMutex );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "creating GtkSalInstance 0x%p\n", pInstance );
#endif

        // initialize SalData
        GtkData *pSalData = new GtkData( pInstance );
        pSalData->Init();
        pSalData->initNWF();

        pInstance->Init();

        InitAtkBridge();

        return pInstance;
    }
}

#if GTK_CHECK_VERSION(3,0,0)
static sal_uInt16 categorizeEvent(const GdkEvent *pEvent)
{
    sal_uInt16 nType = 0;
    switch( pEvent->type )
    {
    case GDK_MOTION_NOTIFY:
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
    case GDK_SCROLL:
        nType = VCL_INPUT_MOUSE;
        break;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
        nType = VCL_INPUT_KEYBOARD;
        break;
    case GDK_EXPOSE:
        nType = VCL_INPUT_PAINT;
        break;
    default:
        nType = VCL_INPUT_OTHER;
        break;
    }
    return nType;
}
#endif

GtkInstance::GtkInstance( SalYieldMutex* pMutex )
#if GTK_CHECK_VERSION(3,0,0)
    : SvpSalInstance( pMutex )
#else
    : X11SalInstance( pMutex )
#endif
{
}

// This has to happen after gtk_init has been called by saldata.cxx's
// Init or our handlers just get clobbered.
void GtkInstance::Init()
{
}

GtkInstance::~GtkInstance()
{
    while( !m_aTimers.empty() )
        delete *m_aTimers.begin();
    DeInitAtkBridge();
}

SalFrame* GtkInstance::CreateFrame( SalFrame* pParent, sal_uLong nStyle )
{
    return new GtkSalFrame( pParent, nStyle );
}

SalFrame* GtkInstance::CreateChildFrame( SystemParentData* pParentData, sal_uLong )
{
    return new GtkSalFrame( pParentData );
}

SalObject* GtkInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow )
{
#if !GTK_CHECK_VERSION(3,0,0)
    // there is no method to set a visual for a GtkWidget
    // so we need the X11SalObject in that case
    if( pWindowData )
        return X11SalObject::CreateObject( pParent, pWindowData, bShow );
#else
    (void)pWindowData;
#warning FIXME: Missing CreateObject functionality ...
#endif

    return new GtkSalObject( static_cast<GtkSalFrame*>(pParent), bShow );
}

extern "C"
{
    typedef void*(* getDefaultFnc)();
    typedef void(* addItemFnc)(void *, const char *);
}

void GtkInstance::AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType)
{
    rtl::OString sGtkURL;
    rtl_TextEncoding aSystemEnc = osl_getThreadTextEncoding();
    if ((aSystemEnc == RTL_TEXTENCODING_UTF8) || (rFileUrl.compareToAscii( "file://", 7 ) !=  0))
        sGtkURL = rtl::OUStringToOString(rFileUrl, RTL_TEXTENCODING_UTF8);
    else
    {
        //Non-utf8 locales are a bad idea if trying to work with non-ascii filenames
        //Decode %XX components
        rtl::OUString sDecodedUri = rtl::Uri::decode(rFileUrl.copy(7), rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
        //Convert back to system locale encoding
        rtl::OString sSystemUrl = rtl::OUStringToOString(sDecodedUri, aSystemEnc);
        //Encode to an escaped ASCII-encoded URI
        gchar *g_uri = g_filename_to_uri(sSystemUrl.getStr(), NULL, NULL);
        sGtkURL = rtl::OString(g_uri);
        g_free(g_uri);
    }
#if GTK_CHECK_VERSION(2,10,0)
    GtkRecentManager *manager = gtk_recent_manager_get_default ();
    gtk_recent_manager_add_item (manager, sGtkURL.getStr());
    (void)rMimeType;
#else
    static getDefaultFnc sym_gtk_recent_manager_get_default =
        (getDefaultFnc)osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gtk_recent_manager_get_default" );

    static addItemFnc sym_gtk_recent_manager_add_item =
        (addItemFnc)osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gtk_recent_manager_add_item");
    if (sym_gtk_recent_manager_get_default && sym_gtk_recent_manager_add_item)
        sym_gtk_recent_manager_add_item(sym_gtk_recent_manager_get_default(), sGtkURL.getStr());
    else
        X11SalInstance::AddToRecentDocumentList(rFileUrl, rMimeType);
#endif
}

/*
 * Obsolete, non-working, and crufty code from the
 * beginning of time. When we update our base platform
 * we should kill this with extreme prejudice.
 */
#if !GTK_CHECK_VERSION(3,0,0)
#  define HORRIBLE_OBSOLETE_YIELDMUTEX_IMPL
#endif

SalInfoPrinter* GtkInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
    ImplJobSetup* pSetupData )
{
#if defined ENABLE_GTK_PRINT || GTK_CHECK_VERSION(3,0,0)
    mbPrinterInit = true;
    // create and initialize SalInfoPrinter
    PspSalInfoPrinter* pPrinter = new GtkSalInfoPrinter;
    configurePspInfoPrinter(pPrinter, pQueueInfo, pSetupData);
    return pPrinter;
#else
    return Superclass_t::CreateInfoPrinter( pQueueInfo, pSetupData );
#endif
}

SalPrinter* GtkInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
#if defined ENABLE_GTK_PRINT || GTK_CHECK_VERSION(3,0,0)
    mbPrinterInit = true;
    fprintf(stderr, "gtk printer\n");
    return new GtkSalPrinter( pInfoPrinter );
#else
    return Superclass_t::CreatePrinter( pInfoPrinter );
#endif
}


GtkYieldMutex::GtkYieldMutex()
{
}

void GtkYieldMutex::acquire()
{
#ifdef HORRIBLE_OBSOLETE_YIELDMUTEX_IMPL
    oslThreadIdentifier aCurrentThread = osl::Thread::getCurrentIdentifier();
    // protect member manipulation
    SolarMutexObject::acquire();
    if( mnCount > 0 && mnThreadId == aCurrentThread )
    {
        mnCount++;
        SolarMutexObject::release();
        return;
    }
    SolarMutexObject::release();

    // obtain gdk mutex
    gdk_threads_enter();

    // obtained gdk mutex, now lock count is one by definition
    SolarMutexObject::acquire();
    mnCount = 1;
    mnThreadId = aCurrentThread;
    SolarMutexObject::release();
#else
    g_error ("never called");
#endif
}

void GtkYieldMutex::release()
{
#ifdef HORRIBLE_OBSOLETE_YIELDMUTEX_IMPL
    oslThreadIdentifier aCurrentThread = osl::Thread::getCurrentIdentifier();
    // protect member manipulation
    SolarMutexObject::acquire();
    // strange things happen, do nothing if we don't own the mutex
    if( mnThreadId == aCurrentThread )
    {
        mnCount--;
        if( mnCount == 0 )
        {
            gdk_threads_leave();
            mnThreadId = 0;
        }
    }
    SolarMutexObject::release();
#else
    g_error ("never called");
#endif
}

sal_Bool GtkYieldMutex::tryToAcquire()
{
#ifdef HORRIBLE_OBSOLETE_YIELDMUTEX_IMPL
    oslThreadIdentifier aCurrentThread = osl::Thread::getCurrentIdentifier();
    // protect member manipulation
    SolarMutexObject::acquire();
    if( mnCount > 0 )
    {
        if( mnThreadId == aCurrentThread )
        {
            mnCount++;
            SolarMutexObject::release();
            return sal_True;
        }
        else
        {
            SolarMutexObject::release();
            return sal_False;
        }
    }
    SolarMutexObject::release();

    // HACK: gdk_threads_mutex is private, we shouldn't use it.
    // how to we do a try_lock without having a gdk_threads_try_enter ?
    if( ! g_mutex_trylock( gdk_threads_mutex ) )
        return sal_False;

    // obtained gdk mutex, now lock count is one by definition
    SolarMutexObject::acquire();
    mnCount = 1;
    mnThreadId = aCurrentThread;
    SolarMutexObject::release();

#else
    g_error ("never called");
#endif
    return sal_True;
}

int GtkYieldMutex::Grab()
{
#ifdef HORRIBLE_OBSOLETE_YIELDMUTEX_IMPL
    // this MUST only be called by gdk/gtk callbacks:
    // they are entered with gdk mutex locked; the mutex
    // was unlocked by GtkYieldMutex befor yielding which
    // is now locked again by gtk implicitly

    // obtained gdk mutex, now lock count is one by definition
    SolarMutexObject::acquire();
    int nRet = mnCount;
    if( mnCount == 0 ) // recursive else
        mnThreadId = osl::Thread::getCurrentIdentifier();
#if OSL_DEBUG_LEVEL > 1
    else if( mnThreadId != osl::Thread::getCurrentIdentifier() )
    {
        fprintf( stderr, "Yield mutex grabbed in different thread !\n" );
        abort();
    }
#endif
    mnCount = 1;
    SolarMutexObject::release();
    return nRet;
#else
    g_error ("never called");
    return sal_True;
#endif
}

void GtkYieldMutex::Ungrab( int nGrabs )
{
#ifdef HORRIBLE_OBSOLETE_YIELDMUTEX_IMPL
    // this MUST only be called when leaving the callback
    // that locked the mutex with Grab()
    SolarMutexObject::acquire();
    mnCount = nGrabs;
    if( mnCount == 0 )
        mnThreadId = 0;
    SolarMutexObject::release();
#else
    (void)nGrabs;
    g_error ("never called");
#endif
}

SalVirtualDevice* GtkInstance::CreateVirtualDevice( SalGraphics *pG,
                                                    long nDX, long nDY,
                                                    sal_uInt16 nBitCount,
                                                    const SystemGraphicsData *pGd )
{
#if GTK_CHECK_VERSION(3,0,0)
    (void)pG; (void) pGd;
    SvpSalVirtualDevice* pNew = new SvpSalVirtualDevice( nBitCount );
    pNew->SetSize( nDX, nDY );
    return pNew;
#else
    return X11SalInstance::CreateVirtualDevice( pG, nDX, nDY, nBitCount, pGd );
#endif
}

SalBitmap* GtkInstance::CreateSalBitmap()
{
#if GTK_CHECK_VERSION(3,0,0)
    return new SvpSalBitmap();
#else
    return X11SalInstance::CreateSalBitmap();
#endif
}

SalTimer* GtkInstance::CreateSalTimer()
{
    GtkSalTimer *pTimer = new GtkSalTimer();
    m_aTimers.push_back( pTimer );
    return pTimer;
}

void GtkInstance::RemoveTimer (SalTimer *pTimer)
{
    std::vector<GtkSalTimer *>::iterator it;
    it = std::find( m_aTimers.begin(), m_aTimers.end(), pTimer );
    if( it != m_aTimers.end() )
        m_aTimers.erase( it );
}

void GtkInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    GetGtkSalData()->Yield( bWait, bHandleAllCurrentEvents );
}

bool GtkInstance::IsTimerExpired()
{
    for( std::vector<GtkSalTimer *>::iterator it = m_aTimers.begin();
         it != m_aTimers.end(); ++it )
        if( (*it)->Expired() )
            return true;

    return false;
}

bool GtkInstance::AnyInput( sal_uInt16 nType )
{
    if( (nType & VCL_INPUT_TIMER) && IsTimerExpired() )
        return true;
#if !GTK_CHECK_VERSION(3,0,0)
    bool bRet = X11SalInstance::AnyInput(nType);
#else
    if (!gdk_events_pending())
        return false;

    if (nType == VCL_INPUT_ANY)
        return true;

    bool bRet = false;
    std::stack<GdkEvent*> aEvents;
    GdkEvent *pEvent = NULL;
    while ((pEvent = gdk_event_get()))
    {
        aEvents.push(pEvent);
        sal_uInt16 nEventType = categorizeEvent(pEvent);
        if ( (nEventType & nType) || ( ! nEventType && (nType & VCL_INPUT_OTHER) ) )
        {
            bRet = true;
            break;
        }
    }

    while (!aEvents.empty())
    {
        pEvent = aEvents.top();
        gdk_event_put(pEvent);
        gdk_event_free(pEvent);
        aEvents.pop();
    }
#endif
    return bRet;
}

GenPspGraphics *GtkInstance::CreatePrintGraphics()
{
    return new GenPspGraphics();
}

boost::shared_ptr<vcl::unx::GtkPrintWrapper>
GtkInstance::getPrintWrapper() const
{
    if (!m_pPrintWrapper)
        m_pPrintWrapper.reset(new vcl::unx::GtkPrintWrapper);
    return m_pPrintWrapper;
}

#if GTK_CHECK_VERSION(3,0,0)
#include "../../../headless/svpinst.cxx"
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
