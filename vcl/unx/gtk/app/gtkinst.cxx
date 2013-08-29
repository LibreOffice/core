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


#include <stack>
#include <string.h>
#include <osl/module.h>
#include <osl/process.h>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/salobj.h>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkobject.hxx>
#include <unx/gtk/atkbridge.hxx>
#include <unx/gtk/gtkprn.hxx>
#include <unx/gtk/gtksalmenu.hxx>
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

extern "C"
{
    #define GET_YIELD_MUTEX() static_cast<GtkYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())
    static void GdkThreadsEnter( void )
    {
        GtkYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsEnter();
    }
    static void GdkThreadsLeave( void )
    {
        GtkYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsLeave();
    }

    VCLPLUG_GTK_PUBLIC SalInstance* create_SalInstance( oslModule )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "create vcl plugin instance with gtk version %d %d %d\n",
                 (int) gtk_major_version, (int) gtk_minor_version,
                 (int) gtk_micro_version );
#endif
        if( gtk_major_version < 2 || // very unlikely sanity check
            ( gtk_major_version == 2 && gtk_minor_version < 4 ) )
        {
            g_warning("require a newer gtk than %d.%d for gdk_threads_set_lock_functions", (int) gtk_major_version, gtk_minor_version);
            return NULL;
        }

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

#if !GTK_CHECK_VERSION(2,4,0)
#error "Requires gtk 2.4.0+ for lock hooking"
#endif
        gdk_threads_set_lock_functions (GdkThreadsEnter, GdkThreadsLeave);

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Hooked gdk threads locks\n" );
#endif

        pYieldMutex = new GtkYieldMutex();

        gdk_threads_init();

        GtkInstance* pInstance = new GtkInstance( pYieldMutex );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "creating GtkSalInstance 0x%p\n", pInstance );
#endif

        //Create SalData, this does not leak
        /*GtkData *pSalData =*/ new GtkData( pInstance );

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
    , bNeedsInit(true)
{
}

//We want to defer initializing gtk until we are after uno has been
//bootstrapped so we can ask the config what the UI language is so that we can
//force that in as $LANGUAGE to get gtk to render widgets RTL if we have a RTL
//UI in a LTR locale
void GtkInstance::AfterAppInit()
{
    OUString aLocaleString(Application::GetSettings().GetUILanguageTag().getGlibcLocaleString(".UTF-8"));
    if (!aLocaleString.isEmpty())
    {
        OUString envVar("LANGUAGE");
        osl_setEnvironment(envVar.pData, aLocaleString.pData);
    }
    EnsureInit();
}

void GtkInstance::EnsureInit()
{
    if (!bNeedsInit)
        return;
    // initialize SalData
    GtkData *pSalData = GetGtkSalData();
    pSalData->Init();
    pSalData->initNWF();

    InitAtkBridge();

    bNeedsInit = false;
}

GtkInstance::~GtkInstance()
{
    while( !m_aTimers.empty() )
        delete *m_aTimers.begin();
    DeInitAtkBridge();
}

SalFrame* GtkInstance::CreateFrame( SalFrame* pParent, sal_uLong nStyle )
{
    EnsureInit();
    return new GtkSalFrame( pParent, nStyle );
}

SalFrame* GtkInstance::CreateChildFrame( SystemParentData* pParentData, sal_uLong )
{
    EnsureInit();
    return new GtkSalFrame( pParentData );
}

SalObject* GtkInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow )
{
    EnsureInit();
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

void GtkInstance::AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService)
{
    EnsureInit();
    OString sGtkURL;
    rtl_TextEncoding aSystemEnc = osl_getThreadTextEncoding();
    if ((aSystemEnc == RTL_TEXTENCODING_UTF8) || !rFileUrl.startsWith( "file://" ))
        sGtkURL = OUStringToOString(rFileUrl, RTL_TEXTENCODING_UTF8);
    else
    {
        //Non-utf8 locales are a bad idea if trying to work with non-ascii filenames
        //Decode %XX components
        OUString sDecodedUri = rtl::Uri::decode(rFileUrl.copy(7), rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
        //Convert back to system locale encoding
        OString sSystemUrl = OUStringToOString(sDecodedUri, aSystemEnc);
        //Encode to an escaped ASCII-encoded URI
        gchar *g_uri = g_filename_to_uri(sSystemUrl.getStr(), NULL, NULL);
        sGtkURL = OString(g_uri);
        g_free(g_uri);
    }
#if GTK_CHECK_VERSION(2,10,0)
    GtkRecentManager *manager = gtk_recent_manager_get_default ();
    gtk_recent_manager_add_item (manager, sGtkURL.getStr());
    (void)rMimeType;
    (void)rDocumentService;
#else
    static getDefaultFnc sym_gtk_recent_manager_get_default =
        (getDefaultFnc)osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gtk_recent_manager_get_default" );

    static addItemFnc sym_gtk_recent_manager_add_item =
        (addItemFnc)osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gtk_recent_manager_add_item");
    if (sym_gtk_recent_manager_get_default && sym_gtk_recent_manager_add_item)
        sym_gtk_recent_manager_add_item(sym_gtk_recent_manager_get_default(), sGtkURL.getStr());
    else
        X11SalInstance::AddToRecentDocumentList(rFileUrl, rMimeType, rDocumentService);
#endif
}

SalInfoPrinter* GtkInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
    ImplJobSetup* pSetupData )
{
    EnsureInit();
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
    EnsureInit();
#if defined ENABLE_GTK_PRINT || GTK_CHECK_VERSION(3,0,0)
    mbPrinterInit = true;
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
    SalYieldMutex::acquire();
}

void GtkYieldMutex::release()
{
    SalYieldMutex::release();
}

/*
 * These methods always occur in pairs
 * A ThreadsEnter is followed by a ThreadsLeave
 * We need to queue up the recursive lock count
 * for each pair, so we can accurately restore
 * it later.
 */
void GtkYieldMutex::ThreadsEnter()
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

void GtkYieldMutex::ThreadsLeave()
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

SalVirtualDevice* GtkInstance::CreateVirtualDevice( SalGraphics *pG,
                                                    long nDX, long nDY,
                                                    sal_uInt16 nBitCount,
                                                    const SystemGraphicsData *pGd )
{
    EnsureInit();
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
    EnsureInit();
#if GTK_CHECK_VERSION(3,0,0)
    return new SvpSalBitmap();
#else
    return X11SalInstance::CreateSalBitmap();
#endif
}

#ifdef ENABLE_GMENU_INTEGRATION

SalMenu* GtkInstance::CreateMenu( sal_Bool bMenuBar, Menu* pVCLMenu )
{
    EnsureInit();
    GtkSalMenu* pSalMenu = new GtkSalMenu( bMenuBar );
    pSalMenu->SetMenu( pVCLMenu );
    return pSalMenu;
}

void GtkInstance::DestroyMenu( SalMenu* pMenu )
{
    EnsureInit();
    delete pMenu;
}

SalMenuItem* GtkInstance::CreateMenuItem( const SalItemParams* pItemData )
{
    EnsureInit();
    return new GtkSalMenuItem( pItemData );
}

void GtkInstance::DestroyMenuItem( SalMenuItem* pItem )
{
    EnsureInit();
    delete pItem;
}

#else // not ENABLE_GMENU_INTEGRATION

SalMenu*     GtkInstance::CreateMenu( sal_Bool, Menu* )          { return NULL; }
void         GtkInstance::DestroyMenu( SalMenu* )                {}
SalMenuItem* GtkInstance::CreateMenuItem( const SalItemParams* ) { return NULL; }
void         GtkInstance::DestroyMenuItem( SalMenuItem* )        {}

#endif

SalTimer* GtkInstance::CreateSalTimer()
{
    EnsureInit();
    GtkSalTimer *pTimer = new GtkSalTimer();
    m_aTimers.push_back( pTimer );
    return pTimer;
}

void GtkInstance::RemoveTimer (SalTimer *pTimer)
{
    EnsureInit();
    std::vector<GtkSalTimer *>::iterator it;
    it = std::find( m_aTimers.begin(), m_aTimers.end(), pTimer );
    if( it != m_aTimers.end() )
        m_aTimers.erase( it );
}

void GtkInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    EnsureInit();
    GetGtkSalData()->Yield( bWait, bHandleAllCurrentEvents );
}

bool GtkInstance::IsTimerExpired()
{
    EnsureInit();
    for( std::vector<GtkSalTimer *>::iterator it = m_aTimers.begin();
         it != m_aTimers.end(); ++it )
        if( (*it)->Expired() )
            return true;

    return false;
}

bool GtkInstance::AnyInput( sal_uInt16 nType )
{
    EnsureInit();
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
    EnsureInit();
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
