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
#include <osl/process.h>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/salobj.h>
#include <unx/gtk/gtkgdi.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkobject.hxx>
#include <unx/gtk/atkbridge.hxx>
#include <unx/gtk/gtkprn.hxx>
#include <unx/gtk/gtksalmenu.hxx>
#include <headless/svpvd.hxx>
#include <headless/svpbmp.hxx>
#include <salimestatus.hxx>
#include <vcl/inputtypes.hxx>
#include <unx/genpspgraphics.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <rtl/uri.hxx>

#include <vcl/settings.hxx>

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#include <unx/gtk/gtkprintwrapper.hxx>

extern "C"
{
    #define GET_YIELD_MUTEX() static_cast<GtkYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())
    static void GdkThreadsEnter()
    {
        GtkYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsEnter();
    }
    static void GdkThreadsLeave()
    {
        GtkYieldMutex *pYieldMutex = GET_YIELD_MUTEX();
        pYieldMutex->ThreadsLeave();
    }

    VCLPLUG_GTK_PUBLIC SalInstance* create_SalInstance()
    {
        SAL_INFO(
            "vcl.gtk",
            "create vcl plugin instance with gtk version " << gtk_major_version
                << " " << gtk_minor_version << " " << gtk_micro_version);

#if !GTK_CHECK_VERSION(3,0,0)
        if( gtk_major_version < 2 || // very unlikely sanity check
            ( gtk_major_version == 2 && gtk_minor_version < 4 ) )
        {
            g_warning("require a newer gtk than %d.%d for gdk_threads_set_lock_functions", static_cast<int>(gtk_major_version), gtk_minor_version);
            return nullptr;
        }
#else
        if (gtk_major_version == 3 && gtk_minor_version < 18)
        {
            g_warning("require gtk >= 3.18 for theme expectations");
            return nullptr;
        }
#endif

        // for gtk2 it is always built with X support, so this is always called
        // for gtk3 it is normally built with X and Wayland support, if
        // X is supported GDK_WINDOWING_X11 is defined and this is always
        // called, regardless of if we're running under X or Wayland.
        // We can't use (DLSYM_GDK_IS_X11_DISPLAY(pDisplay)) to only do it under
        // X, because we need to do it earlier than we have a display
#if !GTK_CHECK_VERSION(3,0,0) || defined(GDK_WINDOWING_X11)
        /* #i92121# workaround deadlocks in the X11 implementation
        */
        static const char* pNoXInitThreads = getenv( "SAL_NO_XINITTHREADS" );
        /* #i90094#
           from now on we know that an X connection will be
           established, so protect X against itself
        */
        if( ! ( pNoXInitThreads && *pNoXInitThreads ) )
            XInitThreads();
#endif

        // init gdk thread protection
        bool const sup = g_thread_supported();
            // extracted from the 'if' to avoid Clang -Wunreachable-code
        if ( !sup )
            g_thread_init( nullptr );

        gdk_threads_set_lock_functions (GdkThreadsEnter, GdkThreadsLeave);
        SAL_INFO("vcl.gtk", "Hooked gdk threads locks");

        auto pYieldMutex = std::make_unique<GtkYieldMutex>();

        gdk_threads_init();

        GtkInstance* pInstance = new GtkInstance( std::move(pYieldMutex) );
        SAL_INFO("vcl.gtk", "creating GtkInstance " << pInstance);

        // Create SalData, this does not leak
        new GtkSalData( pInstance );

        return pInstance;
    }
}

#if GTK_CHECK_VERSION(3,0,0)
static VclInputFlags categorizeEvent(const GdkEvent *pEvent)
{
    VclInputFlags nType = VclInputFlags::NONE;
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
        nType = VclInputFlags::MOUSE;
        break;
    case GDK_KEY_PRESS:
    // case GDK_KEY_RELEASE: //similar to the X11SalInstance one
        nType = VclInputFlags::KEYBOARD;
        break;
    case GDK_EXPOSE:
        nType = VclInputFlags::PAINT;
        break;
    default:
        nType = VclInputFlags::OTHER;
        break;
    }
    return nType;
}
#endif

GtkInstance::GtkInstance( std::unique_ptr<SalYieldMutex> pMutex )
#if GTK_CHECK_VERSION(3,0,0)
    : SvpSalInstance( std::move(pMutex) )
#else
    : X11SalInstance( std::move(pMutex) )
#endif
    , m_pTimer(nullptr)
    , bNeedsInit(true)
    , m_pLastCairoFontOptions(nullptr)
{
}

//We want to defer initializing gtk until we are after uno has been
//bootstrapped so we can ask the config what the UI language is so that we can
//force that in as $LANGUAGE to get gtk to render widgets RTL if we have a RTL
//UI in a LTR locale
void GtkInstance::AfterAppInit()
{
    EnsureInit();
}

void GtkInstance::EnsureInit()
{
    if (!bNeedsInit)
        return;
    // initialize SalData
    GtkSalData *pSalData = GetGtkSalData();
    pSalData->Init();
    GtkSalData::initNWF();

    InitAtkBridge();

    ImplSVData* pSVData = ImplGetSVData();
#ifdef GTK_TOOLKIT_NAME
    pSVData->maAppData.mxToolkitName = OUString(GTK_TOOLKIT_NAME);
#elif GTK_CHECK_VERSION(3,0,0)
    pSVData->maAppData.mxToolkitName = OUString("gtk3");
#else
    pSVData->maAppData.mxToolkitName = OUString("gtk2");
#endif

    bNeedsInit = false;
}

GtkInstance::~GtkInstance()
{
    assert( nullptr == m_pTimer );
    DeInitAtkBridge();
    ResetLastSeenCairoFontOptions(nullptr);
}

SalFrame* GtkInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    EnsureInit();
    return new GtkSalFrame( pParent, nStyle );
}

SalFrame* GtkInstance::CreateChildFrame( SystemParentData* pParentData, SalFrameStyleFlags )
{
    EnsureInit();
    return new GtkSalFrame( pParentData );
}

SalObject* GtkInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow )
{
    EnsureInit();
#if !GTK_CHECK_VERSION(3,0,0)
    // there is no method to set a visual for a GtkWidget
    // so we need the X11SalObject in that case
    if( pWindowData )
        return X11SalObject::CreateObject( pParent, pWindowData, bShow );
#else
    (void)pWindowData;
    //FIXME: Missing CreateObject functionality ...
#endif

    return new GtkSalObject( static_cast<GtkSalFrame*>(pParent), bShow );
}

#if !GTK_CHECK_VERSION(3,0,0)
std::unique_ptr<SalI18NImeStatus> GtkInstance::CreateI18NImeStatus()
{
    //we want the default SalInstance::CreateI18NImeStatus returns the no-op
    //stub here, not the X11Instance::CreateI18NImeStatus which the gtk2
    //one would use otherwise
    return SalInstance::CreateI18NImeStatus();
}
#endif

extern "C"
{
    typedef void*(* getDefaultFnc)();
    typedef void(* addItemFnc)(void *, const char *);
}

void GtkInstance::AddToRecentDocumentList(const OUString& rFileUrl, const OUString&, const OUString&)
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
        gchar *g_uri = g_filename_to_uri(sSystemUrl.getStr(), nullptr, nullptr);
        sGtkURL = OString(g_uri);
        g_free(g_uri);
    }
    GtkRecentManager *manager = gtk_recent_manager_get_default ();
    gtk_recent_manager_add_item (manager, sGtkURL.getStr());
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

std::unique_ptr<SalPrinter> GtkInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    EnsureInit();
#if defined ENABLE_GTK_PRINT || GTK_CHECK_VERSION(3,0,0)
    mbPrinterInit = true;
    return std::unique_ptr<SalPrinter>(new GtkSalPrinter( pInfoPrinter ));
#else
    return Superclass_t::CreatePrinter( pInfoPrinter );
#endif
}

/*
 * These methods always occur in pairs
 * A ThreadsEnter is followed by a ThreadsLeave
 * We need to queue up the recursive lock count
 * for each pair, so we can accurately restore
 * it later.
 */
thread_local std::stack<sal_uInt32> GtkYieldMutex::yieldCounts;

void GtkYieldMutex::ThreadsEnter()
{
    acquire();
    if (!yieldCounts.empty()) {
        auto n = yieldCounts.top();
        yieldCounts.pop();
        for (; n != 0; --n) {
            acquire();
        }
    }
}

void GtkYieldMutex::ThreadsLeave()
{
    assert(m_nCount != 0);
    auto n = m_nCount - 1;
    yieldCounts.push(n);
    for (sal_uIntPtr i = 0; i != n + 1; ++i) {
        release();
    }
}

std::unique_ptr<SalVirtualDevice> GtkInstance::CreateVirtualDevice( SalGraphics *pG,
                                                    long &nDX, long &nDY,
                                                    DeviceFormat eFormat,
                                                    const SystemGraphicsData *pGd )
{
    EnsureInit();
#if GTK_CHECK_VERSION(3,0,0)
    (void) pGd;
    SvpSalGraphics *pSvpSalGraphics = dynamic_cast<SvpSalGraphics*>(pG);
    assert(pSvpSalGraphics);
    std::unique_ptr<SalVirtualDevice> pNew(new SvpSalVirtualDevice(eFormat, pSvpSalGraphics->getSurface()));
    pNew->SetSize( nDX, nDY );
    return pNew;
#else
    GtkSalGraphics *pGtkSalGraphics = dynamic_cast<GtkSalGraphics*>(pG);
    assert(pGtkSalGraphics);
    return CreateX11VirtualDevice(pG, nDX, nDY, eFormat, pGd,
            std::make_unique<GtkSalGraphics>(pGtkSalGraphics->GetGtkFrame(),
                               pGtkSalGraphics->GetGtkWidget(),
                               pGtkSalGraphics->GetScreenNumber()));
#endif
}

std::shared_ptr<SalBitmap> GtkInstance::CreateSalBitmap()
{
    EnsureInit();
#if GTK_CHECK_VERSION(3,0,0)
    return SvpSalInstance::CreateSalBitmap();//new SvpSalBitmap();
#else
    return X11SalInstance::CreateSalBitmap();
#endif
}

#ifdef ENABLE_GMENU_INTEGRATION

std::unique_ptr<SalMenu> GtkInstance::CreateMenu( bool bMenuBar, Menu* pVCLMenu )
{
    EnsureInit();
    GtkSalMenu* pSalMenu = new GtkSalMenu( bMenuBar );
    pSalMenu->SetMenu( pVCLMenu );
    return std::unique_ptr<SalMenu>(pSalMenu);
}

std::unique_ptr<SalMenuItem> GtkInstance::CreateMenuItem( const SalItemParams & rItemData )
{
    EnsureInit();
    return std::unique_ptr<SalMenuItem>(new GtkSalMenuItem( &rItemData ));
}

#else // not ENABLE_GMENU_INTEGRATION

std::unique_ptr<SalMenu>     GtkInstance::CreateMenu( bool, Menu* )          { return nullptr; }
std::unique_ptr<SalMenuItem> GtkInstance::CreateMenuItem( const SalItemParams & ) { return nullptr; }

#endif

SalTimer* GtkInstance::CreateSalTimer()
{
    EnsureInit();
    assert( nullptr == m_pTimer );
    if ( nullptr == m_pTimer )
        m_pTimer = new GtkSalTimer();
    return m_pTimer;
}

void GtkInstance::RemoveTimer ()
{
    EnsureInit();
    m_pTimer = nullptr;
}

bool GtkInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    EnsureInit();
    return GetGtkSalData()->Yield( bWait, bHandleAllCurrentEvents );
}

bool GtkInstance::IsTimerExpired()
{
    EnsureInit();
    return (m_pTimer && m_pTimer->Expired());
}

bool GtkInstance::AnyInput( VclInputFlags nType )
{
    EnsureInit();
    if( (nType & VclInputFlags::TIMER) && IsTimerExpired() )
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
    GdkEvent *pEvent = nullptr;
    while ((pEvent = gdk_event_get()))
    {
        aEvents.push(pEvent);
        VclInputFlags nEventType = categorizeEvent(pEvent);
        if ( (nEventType & nType) || ( nEventType == VclInputFlags::NONE && (nType & VclInputFlags::OTHER) ) )
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

std::shared_ptr<vcl::unx::GtkPrintWrapper> const &
GtkInstance::getPrintWrapper() const
{
    if (!m_xPrintWrapper)
        m_xPrintWrapper.reset(new vcl::unx::GtkPrintWrapper);
    return m_xPrintWrapper;
}

const cairo_font_options_t* GtkInstance::GetCairoFontOptions()
{
    const cairo_font_options_t* pCairoFontOptions = gdk_screen_get_font_options(gdk_screen_get_default());
    if (!m_pLastCairoFontOptions && pCairoFontOptions)
        m_pLastCairoFontOptions = cairo_font_options_copy(pCairoFontOptions);
    return pCairoFontOptions;
}

const cairo_font_options_t* GtkInstance::GetLastSeenCairoFontOptions()
{
    return m_pLastCairoFontOptions;
}

void GtkInstance::ResetLastSeenCairoFontOptions(const cairo_font_options_t* pCairoFontOptions)
{
    if (m_pLastCairoFontOptions)
        cairo_font_options_destroy(m_pLastCairoFontOptions);
    if (pCairoFontOptions)
        m_pLastCairoFontOptions = cairo_font_options_copy(pCairoFontOptions);
    else
        m_pLastCairoFontOptions = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
