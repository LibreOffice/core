/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <deque>
#include <stack>
#include <string.h>
#include <string_view>

#include <osl/process.h>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/genprn.h>
#include <unx/salobj.h>
#include <unx/gtk/gtkgdi.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkobject.hxx>
#include <unx/gtk/atkbridge.hxx>
#include <unx/gtk/gtksalmenu.hxx>
#include <headless/svpvd.hxx>
#include <headless/svpbmp.hxx>
#include <vcl/builder.hxx>
#include <vcl/inputtypes.hxx>
#include <vcl/specialchars.hxx>
#include <vcl/transfer.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <unx/genpspgraphics.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <rtl/uri.hxx>

#include <vcl/settings.hxx>

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#if !GTK_CHECK_VERSION(4, 0, 0)
#include "a11y/atkwrapper.hxx"
#endif
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/bootstrap.hxx>
#include <o3tl/unreachable.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <tools/helpers.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <unotools/resmgr.hxx>
#include <unx/gstsink.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/event.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/quickselectionengine.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/syswin.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <vcl/wrkwin.hxx>
#include "customcellrenderer.hxx"
#include <strings.hrc>
#include <window.h>
#include <numeric>

#include <boost/property_tree/ptree.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

extern "C"
{
    #define GET_YIELD_MUTEX() static_cast<GtkYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())
#if !GTK_CHECK_VERSION(4, 0, 0)
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
#endif

    VCLPLUG_GTK_PUBLIC SalInstance* create_SalInstance()
    {
        SAL_INFO(
            "vcl.gtk",
            "create vcl plugin instance with gtk version " << gtk_get_major_version()
                << " " << gtk_get_minor_version() << " " << gtk_get_micro_version());

        if (gtk_get_major_version() == 3 && gtk_get_minor_version() < 18)
        {
            g_warning("require gtk >= 3.18 for theme expectations");
            return nullptr;
        }

        // for gtk2 it is always built with X support, so this is always called
        // for gtk3 it is normally built with X and Wayland support, if
        // X is supported GDK_WINDOWING_X11 is defined and this is always
        // called, regardless of if we're running under X or Wayland.
        // We can't use (DLSYM_GDK_IS_X11_DISPLAY(pDisplay)) to only do it under
        // X, because we need to do it earlier than we have a display
#if defined(GDK_WINDOWING_X11)
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

#if !GTK_CHECK_VERSION(4, 0, 0)
        // init gdk thread protection
        bool const sup = g_thread_supported();
            // extracted from the 'if' to avoid Clang -Wunreachable-code
        if ( !sup )
            g_thread_init( nullptr );

        gdk_threads_set_lock_functions (GdkThreadsEnter, GdkThreadsLeave);
        SAL_INFO("vcl.gtk", "Hooked gdk threads locks");
#endif

        auto pYieldMutex = std::make_unique<GtkYieldMutex>();

#if !GTK_CHECK_VERSION(4, 0, 0)
        gdk_threads_init();
#endif

        GtkInstance* pInstance = new GtkInstance( std::move(pYieldMutex) );
        SAL_INFO("vcl.gtk", "creating GtkInstance " << pInstance);

        // Create SalData, this does not leak
        new GtkSalData( pInstance );

        return pInstance;
    }
}

#if !GTK_CHECK_VERSION(4, 0, 0)
static VclInputFlags categorizeEvent(const GdkEvent *pEvent)
{
    VclInputFlags nType = VclInputFlags::NONE;
    switch (gdk_event_get_event_type(pEvent))
    {
    case GDK_MOTION_NOTIFY:
    case GDK_BUTTON_PRESS:
#if !GTK_CHECK_VERSION(4, 0, 0)
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
#endif
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
#if !GTK_CHECK_VERSION(4, 0, 0)
    case GDK_EXPOSE:
        nType = VclInputFlags::PAINT;
        break;
#endif
    default:
        nType = VclInputFlags::OTHER;
        break;
    }
    return nType;
}
#endif

GtkInstance::GtkInstance( std::unique_ptr<SalYieldMutex> pMutex )
    : SvpSalInstance( std::move(pMutex) )
    , m_pTimer(nullptr)
    , bNeedsInit(true)
    , m_pLastCairoFontOptions(nullptr)
{
    m_bSupportsOpenGL = true;
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

#if !GTK_CHECK_VERSION(4, 0, 0)
    InitAtkBridge();
#endif

    ImplSVData* pSVData = ImplGetSVData();
#ifdef GTK_TOOLKIT_NAME
    pSVData->maAppData.mxToolkitName = OUString(GTK_TOOLKIT_NAME);
#else
    pSVData->maAppData.mxToolkitName = OUString("gtk3");
#endif

    bNeedsInit = false;
}

GtkInstance::~GtkInstance()
{
    assert( nullptr == m_pTimer );
#if !GTK_CHECK_VERSION(4, 0, 0)
    DeInitAtkBridge();
#endif
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
    //FIXME: Missing CreateObject functionality ...
    if (pWindowData && pWindowData->bClipUsingNativeWidget)
        return new GtkSalObjectWidgetClip(static_cast<GtkSalFrame*>(pParent), bShow);
    return new GtkSalObject(static_cast<GtkSalFrame*>(pParent), bShow);
}

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
    mbPrinterInit = true;
    // create and initialize SalInfoPrinter
    PspSalInfoPrinter* pPrinter = new PspSalInfoPrinter;
    configurePspInfoPrinter(pPrinter, pQueueInfo, pSetupData);
    return pPrinter;
}

std::unique_ptr<SalPrinter> GtkInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    EnsureInit();
    mbPrinterInit = true;
    return std::unique_ptr<SalPrinter>(new PspSalPrinter(pInfoPrinter));
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
    if (yieldCounts.empty())
        return;
    auto n = yieldCounts.top();
    yieldCounts.pop();

    const bool bUndoingLeaveWithoutEnter = n == 0;
    // if the ThreadsLeave bLeaveWithoutEnter of true condition occurred to
    // create this entry then return early undoing the initial acquire of the
    // function
    if G_UNLIKELY(bUndoingLeaveWithoutEnter)
    {
        release();
        return;
    }

    assert(n > 0);
    n--;
    if (n > 0)
        acquire(n);
}

void GtkYieldMutex::ThreadsLeave()
{
    const bool bLeaveWithoutEnter = m_nCount == 0;
    SAL_WARN_IF(bLeaveWithoutEnter, "vcl.gtk", "gdk_threads_leave without matching gdk_threads_enter");
    yieldCounts.push(m_nCount);
    if G_UNLIKELY(bLeaveWithoutEnter) // this ideally shouldn't happen, but can due to the gtk3 file dialog
        return;
    release(true);
}

std::unique_ptr<SalVirtualDevice> GtkInstance::CreateVirtualDevice( SalGraphics &rG,
                                                    tools::Long &nDX, tools::Long &nDY,
                                                    DeviceFormat /*eFormat*/,
                                                    const SystemGraphicsData* pGd )
{
    EnsureInit();
    SvpSalGraphics *pSvpSalGraphics = dynamic_cast<SvpSalGraphics*>(&rG);
    assert(pSvpSalGraphics);
    // tdf#127529 see SvpSalInstance::CreateVirtualDevice for the rare case of a non-null pPreExistingTarget
    cairo_surface_t* pPreExistingTarget = pGd ? static_cast<cairo_surface_t*>(pGd->pSurface) : nullptr;
    std::unique_ptr<SalVirtualDevice> pNew(new SvpSalVirtualDevice(pSvpSalGraphics->getSurface(), pPreExistingTarget));
    pNew->SetSize( nDX, nDY );
    return pNew;
}

std::shared_ptr<SalBitmap> GtkInstance::CreateSalBitmap()
{
    EnsureInit();
    return SvpSalInstance::CreateSalBitmap();
}

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

namespace
{
    bool DisplayHasAnyInput()
    {
        GdkDisplay* pDisplay = gdk_display_get_default();
#if defined(GDK_WINDOWING_WAYLAND)
        if (DLSYM_GDK_IS_WAYLAND_DISPLAY(pDisplay))
        {
            bool bRet = false;
            wl_display* pWLDisplay = gdk_wayland_display_get_wl_display(pDisplay);
            static auto wayland_display_get_fd = reinterpret_cast<int (*) (wl_display*)>(dlsym(nullptr, "wl_display_get_fd"));
            if (wayland_display_get_fd)
            {
                GPollFD aPollFD;
                aPollFD.fd = wayland_display_get_fd(pWLDisplay);
                aPollFD.events = G_IO_IN | G_IO_ERR | G_IO_HUP;
                bRet = g_poll(&aPollFD, 1, 0) > 0;
            }
            return bRet;
        }
#endif
#if defined(GDK_WINDOWING_X11)
        if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
        {
            GPollFD aPollFD;
            aPollFD.fd = ConnectionNumber(gdk_x11_display_get_xdisplay(pDisplay));
            aPollFD.events = G_IO_IN;
            return g_poll(&aPollFD, 1, 0) > 0;
        }
#endif
        return false;
    }
}

bool GtkInstance::AnyInput( VclInputFlags nType )
{
    EnsureInit();
    if( (nType & VclInputFlags::TIMER) && IsTimerExpired() )
        return true;

    // strip timer bits now
    nType = nType & ~VclInputFlags::TIMER;

    static constexpr VclInputFlags ANY_INPUT_EXCLUDING_TIMER = VCL_INPUT_ANY & ~VclInputFlags::TIMER;

    const bool bCheckForAnyInput = nType == ANY_INPUT_EXCLUDING_TIMER;

    bool bRet = false;

    if (bCheckForAnyInput)
        bRet = DisplayHasAnyInput();

#if !GTK_CHECK_VERSION(4, 0, 0)
    GdkDisplay* pDisplay = gdk_display_get_default();
    if (!gdk_display_has_pending(pDisplay))
        return bRet;

    if (bCheckForAnyInput)
        return true;

    std::deque<GdkEvent*> aEvents;
    GdkEvent *pEvent = nullptr;
    while ((pEvent = gdk_display_get_event(pDisplay)))
    {
        aEvents.push_back(pEvent);
        VclInputFlags nEventType = categorizeEvent(pEvent);
        if ( (nEventType & nType) || ( nEventType == VclInputFlags::NONE && (nType & VclInputFlags::OTHER) ) )
        {
            bRet = true;
        }
    }

    while (!aEvents.empty())
    {
        pEvent = aEvents.front();
        gdk_display_put_event(pDisplay, pEvent);
        gdk_event_free(pEvent);
        aEvents.pop_front();
    }
#endif

    return bRet;
}

std::unique_ptr<GenPspGraphics> GtkInstance::CreatePrintGraphics()
{
    EnsureInit();
    return std::make_unique<GenPspGraphics>();
}

const cairo_font_options_t* GtkInstance::GetCairoFontOptions()
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    const cairo_font_options_t* pCairoFontOptions = gdk_screen_get_font_options(gdk_screen_get_default());
#else
    auto pDefaultWin = ImplGetDefaultWindow();
    assert(pDefaultWin);
    SalFrame* pDefaultFrame = pDefaultWin->ImplGetFrame();
    GtkSalFrame* pGtkFrame = dynamic_cast<GtkSalFrame*>(pDefaultFrame);
    assert(pGtkFrame);
    const cairo_font_options_t* pCairoFontOptions = pGtkFrame->get_font_options();
#endif
    if (!m_pLastCairoFontOptions && pCairoFontOptions)
        m_pLastCairoFontOptions = cairo_font_options_copy(pCairoFontOptions);
    return pCairoFontOptions;
}

const cairo_font_options_t* GtkInstance::GetLastSeenCairoFontOptions() const
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


namespace
{
    struct TypeEntry
    {
        const char*     pNativeType;        // string corresponding to nAtom for the case of nAtom being uninitialized
        const char*     pType;              // Mime encoding on our side
    };

    const TypeEntry aConversionTab[] =
    {
        { "ISO10646-1", "text/plain;charset=utf-16" },
        { "UTF8_STRING", "text/plain;charset=utf-8" },
        { "UTF-8", "text/plain;charset=utf-8" },
        { "text/plain;charset=UTF-8", "text/plain;charset=utf-8" },
        // ISO encodings
        { "ISO8859-2", "text/plain;charset=iso8859-2" },
        { "ISO8859-3", "text/plain;charset=iso8859-3" },
        { "ISO8859-4", "text/plain;charset=iso8859-4" },
        { "ISO8859-5", "text/plain;charset=iso8859-5" },
        { "ISO8859-6", "text/plain;charset=iso8859-6" },
        { "ISO8859-7", "text/plain;charset=iso8859-7" },
        { "ISO8859-8", "text/plain;charset=iso8859-8" },
        { "ISO8859-9", "text/plain;charset=iso8859-9" },
        { "ISO8859-10", "text/plain;charset=iso8859-10" },
        { "ISO8859-13", "text/plain;charset=iso8859-13" },
        { "ISO8859-14", "text/plain;charset=iso8859-14" },
        { "ISO8859-15", "text/plain;charset=iso8859-15" },
        // asian encodings
        { "JISX0201.1976-0", "text/plain;charset=jisx0201.1976-0" },
        { "JISX0208.1983-0", "text/plain;charset=jisx0208.1983-0" },
        { "JISX0208.1990-0", "text/plain;charset=jisx0208.1990-0" },
        { "JISX0212.1990-0", "text/plain;charset=jisx0212.1990-0" },
        { "GB2312.1980-0", "text/plain;charset=gb2312.1980-0" },
        { "KSC5601.1992-0", "text/plain;charset=ksc5601.1992-0" },
        // eastern european encodings
        { "KOI8-R", "text/plain;charset=koi8-r" },
        { "KOI8-U", "text/plain;charset=koi8-u" },
        // String (== iso8859-1)
        { "STRING", "text/plain;charset=iso8859-1" },
        // special for compound text
        { "COMPOUND_TEXT", "text/plain;charset=compound_text" },

        // PIXMAP
        { "PIXMAP", "image/bmp" }
    };

    class DataFlavorEq
    {
    private:
        const css::datatransfer::DataFlavor& m_rData;
    public:
        explicit DataFlavorEq(const css::datatransfer::DataFlavor& rData) : m_rData(rData) {}
        bool operator() (const css::datatransfer::DataFlavor& rData) const
        {
            return rData.MimeType == m_rData.MimeType &&
                   rData.DataType  == m_rData.DataType;
        }
    };
}

#if GTK_CHECK_VERSION(4, 0, 0)
std::vector<css::datatransfer::DataFlavor> GtkTransferable::getTransferDataFlavorsAsVector(const char * const *targets, gint n_targets)
#else
std::vector<css::datatransfer::DataFlavor> GtkTransferable::getTransferDataFlavorsAsVector(GdkAtom *targets, gint n_targets)
#endif
{
    std::vector<css::datatransfer::DataFlavor> aVector;

    bool bHaveText = false, bHaveUTF16 = false;

    for (gint i = 0; i < n_targets; ++i)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        const gchar* pName = targets[i];
#else
        gchar* pName = gdk_atom_name(targets[i]);
#endif
        const char* pFinalName = pName;
        css::datatransfer::DataFlavor aFlavor;

        // omit text/plain;charset=unicode since it is not well defined
        if (rtl_str_compare(pName, "text/plain;charset=unicode") == 0)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            g_free(pName);
#endif
            continue;
        }

        for (size_t j = 0; j < SAL_N_ELEMENTS(aConversionTab); ++j)
        {
            if (rtl_str_compare(pName, aConversionTab[j].pNativeType) == 0)
            {
                pFinalName = aConversionTab[j].pType;
                break;
            }
        }

        // There are more non-MIME-types reported that are not translated by
        // aConversionTab, like "SAVE_TARGETS", "INTEGER", "ATOM"; just filter
        // them out for now before they confuse this code's clients:
        if (rtl_str_indexOfChar(pFinalName, '/') == -1)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            g_free(pName);
#endif
            continue;
        }

        aFlavor.MimeType = OUString(pFinalName,
                                    strlen(pFinalName),
                                    RTL_TEXTENCODING_UTF8);

        m_aMimeTypeToGtkType[aFlavor.MimeType] = targets[i];

        aFlavor.DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

        sal_Int32 nIndex(0);
        if (aFlavor.MimeType.getToken(0, ';', nIndex) == "text/plain")
        {
            bHaveText = true;
            OUString aToken(aFlavor.MimeType.getToken(0, ';', nIndex));
            if (aToken == "charset=utf-16")
            {
                bHaveUTF16 = true;
                aFlavor.DataType = cppu::UnoType<OUString>::get();
            }
        }
        aVector.push_back(aFlavor);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_free(pName);
#endif
    }

    //If we have text, but no UTF-16 format which is basically the only
    //text-format LibreOffice supports for cnp then claim we do and we
    //will convert on demand
    if (bHaveText && !bHaveUTF16)
    {
        css::datatransfer::DataFlavor aFlavor;
        aFlavor.MimeType = "text/plain;charset=utf-16";
        aFlavor.DataType = cppu::UnoType<OUString>::get();
        aVector.push_back(aFlavor);
    }

    return aVector;
}

css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL GtkTransferable::getTransferDataFlavors()
{
    return comphelper::containerToSequence(getTransferDataFlavorsAsVector());
}

sal_Bool SAL_CALL GtkTransferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    const std::vector<css::datatransfer::DataFlavor> aAll =
        getTransferDataFlavorsAsVector();

    return std::any_of(aAll.begin(), aAll.end(), DataFlavorEq(rFlavor));
}

#if GTK_CHECK_VERSION(4, 0, 0)
void read_transfer_result::read_block_async_completed(GObject* source, GAsyncResult* res, gpointer user_data)
{
    GInputStream* stream = G_INPUT_STREAM(source);
    read_transfer_result* pRes = static_cast<read_transfer_result*>(user_data);

    gsize bytes_read = g_input_stream_read_finish(stream, res, nullptr);

    bool bFinished = bytes_read == 0;

    if (bFinished)
    {
        g_object_unref(stream);
        pRes->aVector.resize(pRes->nRead);
        pRes->bDone = true;
        g_main_context_wakeup(nullptr);
        return;
    }

    pRes->nRead += bytes_read;

    pRes->aVector.resize(pRes->nRead + read_transfer_result::BlockSize);

    g_input_stream_read_async(stream,
                              pRes->aVector.data() + pRes->nRead,
                              read_transfer_result::BlockSize,
                              G_PRIORITY_DEFAULT,
                              nullptr,
                              read_block_async_completed,
                              user_data);
}

OUString read_transfer_result::get_as_string() const
{
    const char* pStr = reinterpret_cast<const char*>(aVector.data());
    return OUString(pStr, aVector.size(), RTL_TEXTENCODING_UTF8).replaceAll("\r\n", "\n");
}

css::uno::Sequence<sal_Int8> read_transfer_result::get_as_sequence() const
{
    return css::uno::Sequence<sal_Int8>(aVector.data(), aVector.size());
}
#endif

namespace {

GdkClipboard* clipboard_get(SelectionType eSelection)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    if (eSelection == SELECTION_CLIPBOARD)
        return gdk_display_get_clipboard(gdk_display_get_default());
    return gdk_display_get_primary_clipboard(gdk_display_get_default());
#else
    return gtk_clipboard_get(eSelection == SELECTION_CLIPBOARD ? GDK_SELECTION_CLIPBOARD : GDK_SELECTION_PRIMARY);
#endif
}

#if GTK_CHECK_VERSION(4, 0, 0)

void read_clipboard_async_completed(GObject* source, GAsyncResult* res, gpointer user_data)
{
    GdkClipboard* clipboard = GDK_CLIPBOARD(source);
    read_transfer_result* pRes = static_cast<read_transfer_result*>(user_data);

    GInputStream* pResult = gdk_clipboard_read_finish(clipboard, res, nullptr, nullptr);

    if (!pResult)
    {
        pRes->bDone = true;
        g_main_context_wakeup(nullptr);
        return;
    }

    pRes->aVector.resize(read_transfer_result::BlockSize);

    g_input_stream_read_async(pResult,
                              pRes->aVector.data(),
                              pRes->aVector.size(),
                              G_PRIORITY_DEFAULT,
                              nullptr,
                              read_transfer_result::read_block_async_completed,
                              user_data);
}

#endif

class GtkClipboardTransferable : public GtkTransferable
{
private:
    SelectionType m_eSelection;

public:

    explicit GtkClipboardTransferable(SelectionType eSelection)
        : m_eSelection(eSelection)
    {
    }

    /*
     * XTransferable
     */

    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override
    {
        css::datatransfer::DataFlavor aFlavor(rFlavor);
        if (aFlavor.MimeType == "text/plain;charset=utf-16")
            aFlavor.MimeType = "text/plain;charset=utf-8";

        auto it = m_aMimeTypeToGtkType.find(aFlavor.MimeType);
        if (it == m_aMimeTypeToGtkType.end())
            return css::uno::Any();

        css::uno::Any aRet;

        GdkClipboard* clipboard = clipboard_get(m_eSelection);

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (aFlavor.MimeType == "text/plain;charset=utf-8")
        {
            gchar *pText = gtk_clipboard_wait_for_text(clipboard);
            OUString aStr(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pText);
            aRet <<= aStr.replaceAll("\r\n", "\n");
            return aRet;
        }
        else
        {
            GtkSelectionData* data = gtk_clipboard_wait_for_contents(clipboard,
                                                                     it->second);
            if (!data)
            {
                return css::uno::Any();
            }
            gint length;
            const guchar *rawdata = gtk_selection_data_get_data_with_length(data,
                                                                            &length);
            Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(rawdata), length);
            gtk_selection_data_free(data);
            aRet <<= aSeq;
        }
#else
        SalInstance* pInstance = GetSalData()->m_pInstance;
        read_transfer_result aRes;
        const char *mime_types[] = { it->second.getStr(), nullptr };

        gdk_clipboard_read_async(clipboard,
                                 mime_types,
                                 G_PRIORITY_DEFAULT,
                                 nullptr,
                                 read_clipboard_async_completed,
                                 &aRes);

        while (!aRes.bDone)
            pInstance->DoYield(true, false);

        if (aFlavor.MimeType == "text/plain;charset=utf-8")
            aRet <<= aRes.get_as_string();
        else
            aRet <<= aRes.get_as_sequence();
#endif
        return aRet;
    }

    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector()
        override
    {
        std::vector<css::datatransfer::DataFlavor> aVector;

        GdkClipboard* clipboard = clipboard_get(m_eSelection);

#if GTK_CHECK_VERSION(4, 0, 0)
        GdkContentFormats* pFormats = gdk_clipboard_get_formats(clipboard);
        gsize n_targets;
        const char * const *targets = gdk_content_formats_get_mime_types(pFormats, &n_targets);
        aVector = GtkTransferable::getTransferDataFlavorsAsVector(targets, n_targets);
#else
        GdkAtom *targets;
        gint n_targets;
        if (gtk_clipboard_wait_for_targets(clipboard, &targets, &n_targets))
        {
            aVector = GtkTransferable::getTransferDataFlavorsAsVector(targets, n_targets);
            g_free(targets);
        }
#endif

        return aVector;
    }
};

class VclGtkClipboard :
        public cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        datatransfer::clipboard::XFlushableClipboard,
        XServiceInfo>
{
    SelectionType                                            m_eSelection;
    osl::Mutex                                               m_aMutex;
    gulong                                                   m_nOwnerChangedSignalId;
    ImplSVEvent*                                             m_pSetClipboardEvent;
    Reference<css::datatransfer::XTransferable>              m_aContents;
    Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector< Reference<css::datatransfer::clipboard::XClipboardListener> > m_aListeners;
#if GTK_CHECK_VERSION(4, 0, 0)
    std::vector<OString> m_aGtkTargets;
    TransferableContent* m_pClipboardContent;
#else
    std::vector<GtkTargetEntry> m_aGtkTargets;
#endif
    VclToGtkHelper m_aConversionHelper;

    DECL_LINK(AsyncSetGtkClipboard, void*, void);

#if GTK_CHECK_VERSION(4, 0, 0)
    DECL_LINK(DetachClipboard, void*, void);
#endif

public:

    explicit VclGtkClipboard(SelectionType eSelection);
    virtual ~VclGtkClipboard() override;

    /*
     * XServiceInfo
     */

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /*
     * XClipboard
     */

    virtual Reference< css::datatransfer::XTransferable > SAL_CALL getContents() override;

    virtual void SAL_CALL setContents(
        const Reference< css::datatransfer::XTransferable >& xTrans,
        const Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) override;

    virtual OUString SAL_CALL getName() override;

    /*
     * XClipboardEx
     */

    virtual sal_Int8 SAL_CALL getRenderingCapabilities() override;

    /*
     * XFlushableClipboard
     */
    virtual void SAL_CALL flushClipboard() override;

    /*
     * XClipboardNotifier
     */
    virtual void SAL_CALL addClipboardListener(
        const Reference< css::datatransfer::clipboard::XClipboardListener >& listener ) override;

    virtual void SAL_CALL removeClipboardListener(
        const Reference< css::datatransfer::clipboard::XClipboardListener >& listener ) override;

#if !GTK_CHECK_VERSION(4, 0, 0)
    void ClipboardGet(GtkSelectionData *selection_data, guint info);
#endif
    void OwnerPossiblyChanged(GdkClipboard *clipboard);
    void ClipboardClear();
    void SetGtkClipboard();
    void SyncGtkClipboard();
};

}

OUString VclGtkClipboard::getImplementationName()
{
    return "com.sun.star.datatransfer.VclGtkClipboard";
}

Sequence< OUString > VclGtkClipboard::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
    return aRet;
}

sal_Bool VclGtkClipboard::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Reference< css::datatransfer::XTransferable > VclGtkClipboard::getContents()
{
    if (!m_aContents.is())
    {
        //tdf#93887 This is the system clipboard/selection. We fetch it when we are not
        //the owner of the clipboard and have not already fetched it.
        m_aContents = new GtkClipboardTransferable(m_eSelection);
#if GTK_CHECK_VERSION(4, 0, 0)
        if (m_pClipboardContent)
            transerable_content_set_transferable(m_pClipboardContent, m_aContents.get());
#endif
    }
    return m_aContents;
}

#if !GTK_CHECK_VERSION(4, 0, 0)
void VclGtkClipboard::ClipboardGet(GtkSelectionData *selection_data, guint info)
{
    if (!m_aContents.is())
        return;
    // tdf#129809 take a reference in case m_aContents is replaced during this
    // call
    Reference<datatransfer::XTransferable> xCurrentContents(m_aContents);
    m_aConversionHelper.setSelectionData(xCurrentContents, selection_data, info);
}

namespace
{
    const OString& getPID()
    {
        static OString sPID;
        if (!sPID.getLength())
        {
            oslProcessIdentifier aProcessId = 0;
            oslProcessInfo info;
            info.Size = sizeof (oslProcessInfo);
            if (osl_getProcessInfo(nullptr, osl_Process_IDENTIFIER, &info) == osl_Process_E_None)
                aProcessId = info.Ident;
            sPID = OString::number(aProcessId);
        }
        return sPID;
    }

    void ClipboardGetFunc(GdkClipboard* /*clipboard*/, GtkSelectionData *selection_data,
                          guint info,
                          gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardGet(selection_data, info);
    }

    void ClipboardClearFunc(GdkClipboard* /*clipboard*/, gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardClear();
    }
}
#endif

namespace
{
#if GTK_CHECK_VERSION(4, 0, 0)
    void handle_owner_change(GdkClipboard *clipboard, gpointer user_data)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data);
        pThis->OwnerPossiblyChanged(clipboard);
    }
#else
    void handle_owner_change(GdkClipboard *clipboard, GdkEvent* /*event*/, gpointer user_data)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data);
        pThis->OwnerPossiblyChanged(clipboard);
    }
#endif
}

void VclGtkClipboard::OwnerPossiblyChanged(GdkClipboard* clipboard)
{
    SyncGtkClipboard(); // tdf#138183 do any pending SetGtkClipboard calls
    if (!m_aContents.is())
        return;

#if GTK_CHECK_VERSION(4, 0, 0)
    bool bSelf = gdk_clipboard_is_local(clipboard);
#else
    //if gdk_display_supports_selection_notification is not supported, e.g. like
    //right now under wayland, then you only get owner-changed notifications at
    //opportune times when the selection might have changed. So here
    //we see if the selection supports a dummy selection type identifying
    //our pid, in which case it's us.
    bool bSelf = false;

    //disconnect and reconnect after gtk_clipboard_wait_for_targets to
    //avoid possible recursion
    g_signal_handler_disconnect(clipboard, m_nOwnerChangedSignalId);

    OString sTunnel = "application/x-libreoffice-internal-id-" + getPID();
    GdkAtom *targets;
    gint n_targets;
    if (gtk_clipboard_wait_for_targets(clipboard, &targets, &n_targets))
    {
        for (gint i = 0; i < n_targets && !bSelf; ++i)
        {
            gchar* pName = gdk_atom_name(targets[i]);
            if (strcmp(pName, sTunnel.getStr()) == 0)
            {
                bSelf = true;
            }
            g_free(pName);
        }

        g_free(targets);
    }

    m_nOwnerChangedSignalId = g_signal_connect(clipboard, "owner-change",
                                               G_CALLBACK(handle_owner_change), this);
#endif

    if (!bSelf)
    {
        //null out m_aContents to return control to the system-one which
        //will be retrieved if getContents is called again
        setContents(Reference<css::datatransfer::XTransferable>(),
                    Reference<css::datatransfer::clipboard::XClipboardOwner>());
    }
}

void VclGtkClipboard::ClipboardClear()
{
    if (m_pSetClipboardEvent)
    {
        Application::RemoveUserEvent(m_pSetClipboardEvent);
        m_pSetClipboardEvent = nullptr;
    }
#if !GTK_CHECK_VERSION(4, 0, 0)
    for (auto &a : m_aGtkTargets)
        g_free(a.target);
#endif
    m_aGtkTargets.clear();
}

#if GTK_CHECK_VERSION(4, 0, 0)
IMPL_LINK_NOARG(VclGtkClipboard, DetachClipboard, void*, void)
{
    ClipboardClear();
}

OString VclToGtkHelper::makeGtkTargetEntry(const css::datatransfer::DataFlavor& rFlavor)
{
    OString aEntry = OUStringToOString(rFlavor.MimeType, RTL_TEXTENCODING_UTF8);
    auto it = std::find_if(aInfoToFlavor.begin(), aInfoToFlavor.end(),
                           DataFlavorEq(rFlavor));
    if (it == aInfoToFlavor.end())
        aInfoToFlavor.push_back(rFlavor);
    return aEntry;
}
#else
GtkTargetEntry VclToGtkHelper::makeGtkTargetEntry(const css::datatransfer::DataFlavor& rFlavor)
{
    GtkTargetEntry aEntry;
    aEntry.target =
        g_strdup(OUStringToOString(rFlavor.MimeType, RTL_TEXTENCODING_UTF8).getStr());
    aEntry.flags = 0;
    auto it = std::find_if(aInfoToFlavor.begin(), aInfoToFlavor.end(),
                        DataFlavorEq(rFlavor));
    if (it != aInfoToFlavor.end())
        aEntry.info = std::distance(aInfoToFlavor.begin(), it);
    else
    {
        aEntry.info = aInfoToFlavor.size();
        aInfoToFlavor.push_back(rFlavor);
    }
    return aEntry;
}
#endif

#if GTK_CHECK_VERSION(4, 0, 0)

namespace
{
    void write_mime_type_done(GObject* pStream, GAsyncResult* pResult, gpointer pTaskPtr)
    {
        GTask* pTask = static_cast<GTask*>(pTaskPtr);

        GError* pError = nullptr;
        if (!g_output_stream_write_all_finish(G_OUTPUT_STREAM(pStream),
                                              pResult, nullptr, &pError))
        {
            g_task_return_error(pTask, pError);
        }
        else
        {
            g_task_return_boolean(pTask, true);
        }

        g_object_unref(pTask);
    }

    class MimeTypeEq
    {
    private:
        const OUString& m_rMimeType;
    public:
        explicit MimeTypeEq(const OUString& rMimeType) : m_rMimeType(rMimeType) {}
        bool operator() (const css::datatransfer::DataFlavor& rData) const
        {
            return rData.MimeType == m_rMimeType;
        }
    };
}

void VclToGtkHelper::setSelectionData(const Reference<css::datatransfer::XTransferable> &rTrans,
                                      GdkContentProvider* provider,
                                      const char* mime_type,
                                      GOutputStream* stream,
                                      int io_priority,
                                      GCancellable* cancellable,
                                      GAsyncReadyCallback callback,
                                      gpointer user_data)
{
    GTask *task = g_task_new(provider, cancellable, callback, user_data);
    g_task_set_priority(task, io_priority);

    OUString sMimeType(mime_type, strlen(mime_type), RTL_TEXTENCODING_UTF8);

    auto it = std::find_if(aInfoToFlavor.begin(), aInfoToFlavor.end(),
                           MimeTypeEq(sMimeType));
    if (it == aInfoToFlavor.end())
    {
        SAL_WARN( "vcl.gtk", "unknown mime-type request from clipboard");
        g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
            "unknown mime-type “%s” request from clipboard", mime_type);
        g_object_unref(task);
        return;
    }

    css::datatransfer::DataFlavor aFlavor(*it);
    if (aFlavor.MimeType == "UTF8_STRING" || aFlavor.MimeType == "STRING")
        aFlavor.MimeType = "text/plain;charset=utf-8";

    Sequence<sal_Int8> aData;
    Any aValue;

    try
    {
        aValue = rTrans->getTransferData(aFlavor);
    }
    catch (...)
    {
    }

    if (aValue.getValueTypeClass() == TypeClass_STRING)
    {
        OUString aString;
        aValue >>= aString;
        aData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8 const *>(aString.getStr()), aString.getLength() * sizeof( sal_Unicode ) );
    }
    else if (aValue.getValueType() == cppu::UnoType<Sequence< sal_Int8 >>::get())
    {
        aValue >>= aData;
    }
    else if (aFlavor.MimeType == "text/plain;charset=utf-8")
    {
        //didn't have utf-8, try utf-16 and convert
        aFlavor.MimeType = "text/plain;charset=utf-16";
        aFlavor.DataType = cppu::UnoType<OUString>::get();
        try
        {
            aValue = rTrans->getTransferData(aFlavor);
        }
        catch (...)
        {
        }
        OUString aString;
        aValue >>= aString;
        OString aUTF8String(OUStringToOString(aString, RTL_TEXTENCODING_UTF8));

        g_output_stream_write_all_async(stream, aUTF8String.getStr(), aUTF8String.getLength(),
                                        io_priority, cancellable, write_mime_type_done, task);
        return;
    }

    g_output_stream_write_all_async(stream, aData.getArray(), aData.getLength(),
                                    io_priority, cancellable, write_mime_type_done, task);
}
#else
void VclToGtkHelper::setSelectionData(const Reference<css::datatransfer::XTransferable> &rTrans,
                                      GtkSelectionData *selection_data, guint info)
{
    GdkAtom type(gdk_atom_intern(OUStringToOString(aInfoToFlavor[info].MimeType,
                                                   RTL_TEXTENCODING_UTF8).getStr(),
                                 false));

    css::datatransfer::DataFlavor aFlavor(aInfoToFlavor[info]);
    if (aFlavor.MimeType == "UTF8_STRING" || aFlavor.MimeType == "STRING")
        aFlavor.MimeType = "text/plain;charset=utf-8";

    Sequence<sal_Int8> aData;
    Any aValue;

    try
    {
        aValue = rTrans->getTransferData(aFlavor);
    }
    catch (...)
    {
    }

    if (aValue.getValueTypeClass() == TypeClass_STRING)
    {
        OUString aString;
        aValue >>= aString;
        aData = Sequence< sal_Int8 >( reinterpret_cast<sal_Int8 const *>(aString.getStr()), aString.getLength() * sizeof( sal_Unicode ) );
    }
    else if (aValue.getValueType() == cppu::UnoType<Sequence< sal_Int8 >>::get())
    {
        aValue >>= aData;
    }
    else if (aFlavor.MimeType == "text/plain;charset=utf-8")
    {
        //didn't have utf-8, try utf-16 and convert
        aFlavor.MimeType = "text/plain;charset=utf-16";
        aFlavor.DataType = cppu::UnoType<OUString>::get();
        try
        {
            aValue = rTrans->getTransferData(aFlavor);
        }
        catch (...)
        {
        }
        OUString aString;
        aValue >>= aString;
        OString aUTF8String(OUStringToOString(aString, RTL_TEXTENCODING_UTF8));
        gtk_selection_data_set(selection_data, type, 8,
                               reinterpret_cast<const guchar *>(aUTF8String.getStr()),
                               aUTF8String.getLength());
        return;
    }

    gtk_selection_data_set(selection_data, type, 8,
                           reinterpret_cast<const guchar *>(aData.getArray()),
                           aData.getLength());
}
#endif

VclGtkClipboard::VclGtkClipboard(SelectionType eSelection)
    : cppu::WeakComponentImplHelper<datatransfer::clipboard::XSystemClipboard,
                                    datatransfer::clipboard::XFlushableClipboard, XServiceInfo>
        (m_aMutex)
    , m_eSelection(eSelection)
    , m_pSetClipboardEvent(nullptr)
#if GTK_CHECK_VERSION(4, 0, 0)
    , m_pClipboardContent(nullptr)
#endif
{
    GdkClipboard* clipboard = clipboard_get(m_eSelection);
#if GTK_CHECK_VERSION(4, 0, 0)
    m_nOwnerChangedSignalId = g_signal_connect(clipboard, "changed",
                                               G_CALLBACK(handle_owner_change), this);
#else
    m_nOwnerChangedSignalId = g_signal_connect(clipboard, "owner-change",
                                               G_CALLBACK(handle_owner_change), this);
#endif
}

void VclGtkClipboard::flushClipboard()
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    SolarMutexGuard aGuard;

    if (m_eSelection != SELECTION_CLIPBOARD)
        return;

    GdkClipboard* clipboard = clipboard_get(m_eSelection);
    gtk_clipboard_store(clipboard);
#endif
}

VclGtkClipboard::~VclGtkClipboard()
{
    GdkClipboard* clipboard = clipboard_get(m_eSelection);
    g_signal_handler_disconnect(clipboard, m_nOwnerChangedSignalId);
    if (!m_aGtkTargets.empty())
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gdk_clipboard_set_content(clipboard, nullptr);
        m_pClipboardContent = nullptr;
#else
        gtk_clipboard_clear(clipboard);
#endif
        ClipboardClear();
    }
    assert(!m_pSetClipboardEvent);
    assert(m_aGtkTargets.empty());
}

#if GTK_CHECK_VERSION(4, 0, 0)
std::vector<OString> VclToGtkHelper::FormatsToGtk(const css::uno::Sequence<css::datatransfer::DataFlavor> &rFormats)
#else
std::vector<GtkTargetEntry> VclToGtkHelper::FormatsToGtk(const css::uno::Sequence<css::datatransfer::DataFlavor> &rFormats)
#endif
{
#if GTK_CHECK_VERSION(4, 0, 0)
    std::vector<OString> aGtkTargets;
#else
    std::vector<GtkTargetEntry> aGtkTargets;
#endif

    bool bHaveText(false), bHaveUTF8(false);
    for (const css::datatransfer::DataFlavor& rFlavor : rFormats)
    {
        sal_Int32 nIndex(0);
        if (rFlavor.MimeType.getToken(0, ';', nIndex) == "text/plain")
        {
            bHaveText = true;
            OUString aToken(rFlavor.MimeType.getToken(0, ';', nIndex));
            if (aToken == "charset=utf-8")
            {
                bHaveUTF8 = true;
            }
        }
        aGtkTargets.push_back(makeGtkTargetEntry(rFlavor));
    }

    if (bHaveText)
    {
        css::datatransfer::DataFlavor aFlavor;
        aFlavor.DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();
        if (!bHaveUTF8)
        {
            aFlavor.MimeType = "text/plain;charset=utf-8";
            aGtkTargets.push_back(makeGtkTargetEntry(aFlavor));
        }
        aFlavor.MimeType = "UTF8_STRING";
        aGtkTargets.push_back(makeGtkTargetEntry(aFlavor));
        aFlavor.MimeType = "STRING";
        aGtkTargets.push_back(makeGtkTargetEntry(aFlavor));
    }

    return aGtkTargets;
}

IMPL_LINK_NOARG(VclGtkClipboard, AsyncSetGtkClipboard, void*, void)
{
    osl::ClearableMutexGuard aGuard( m_aMutex );
    m_pSetClipboardEvent = nullptr;
    SetGtkClipboard();
}

void VclGtkClipboard::SyncGtkClipboard()
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    if (m_pSetClipboardEvent)
    {
        Application::RemoveUserEvent(m_pSetClipboardEvent);
        m_pSetClipboardEvent = nullptr;
        SetGtkClipboard();
    }
}

void VclGtkClipboard::SetGtkClipboard()
{
    GdkClipboard* clipboard = clipboard_get(m_eSelection);
#if GTK_CHECK_VERSION(4, 0, 0)
    m_pClipboardContent = TRANSFERABLE_CONTENT(transerable_content_new(&m_aConversionHelper, m_aContents.get()));
    transerable_content_set_detach_clipboard_link(m_pClipboardContent, LINK(this, VclGtkClipboard, DetachClipboard));
    gdk_clipboard_set_content(clipboard, GDK_CONTENT_PROVIDER(m_pClipboardContent));
#else
    gtk_clipboard_set_with_data(clipboard, m_aGtkTargets.data(), m_aGtkTargets.size(),
                                ClipboardGetFunc, ClipboardClearFunc, this);
    gtk_clipboard_set_can_store(clipboard, m_aGtkTargets.data(), m_aGtkTargets.size());
#endif
}

void VclGtkClipboard::setContents(
        const Reference< css::datatransfer::XTransferable >& xTrans,
        const Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
{
    css::uno::Sequence<css::datatransfer::DataFlavor> aFormats;
    if (xTrans.is())
    {
        aFormats = xTrans->getTransferDataFlavors();
    }

    osl::ClearableMutexGuard aGuard( m_aMutex );
    Reference< datatransfer::clipboard::XClipboardOwner > xOldOwner( m_aOwner );
    Reference< datatransfer::XTransferable > xOldContents( m_aContents );
    m_aContents = xTrans;
#if GTK_CHECK_VERSION(4, 0, 0)
    if (m_pClipboardContent)
        transerable_content_set_transferable(m_pClipboardContent, m_aContents.get());
#endif
    m_aOwner = xClipboardOwner;

    std::vector< Reference< datatransfer::clipboard::XClipboardListener > > aListeners( m_aListeners );
    datatransfer::clipboard::ClipboardEvent aEv;

    GdkClipboard* clipboard = clipboard_get(m_eSelection);
    if (!m_aGtkTargets.empty())
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gdk_clipboard_set_content(clipboard, nullptr);
        m_pClipboardContent = nullptr;
#else
        gtk_clipboard_clear(clipboard);
#endif
        ClipboardClear();
    }
    assert(m_aGtkTargets.empty());
    if (m_aContents.is())
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        std::vector<OString> aGtkTargets(m_aConversionHelper.FormatsToGtk(aFormats));
#else
        std::vector<GtkTargetEntry> aGtkTargets(m_aConversionHelper.FormatsToGtk(aFormats));
#endif
        if (!aGtkTargets.empty())
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            GtkTargetEntry aEntry;
            OString sTunnel = "application/x-libreoffice-internal-id-" + getPID();
            aEntry.target = g_strdup(sTunnel.getStr());
            aEntry.flags = 0;
            aEntry.info = 0;
            aGtkTargets.push_back(aEntry);
#endif
            m_aGtkTargets = aGtkTargets;

            if (!m_pSetClipboardEvent)
                m_pSetClipboardEvent = Application::PostUserEvent(LINK(this, VclGtkClipboard, AsyncSetGtkClipboard));
        }
    }

    aEv.Contents = getContents();

    aGuard.clear();

    if (xOldOwner.is() && xOldOwner != xClipboardOwner)
        xOldOwner->lostOwnership( this, xOldContents );
    for (auto const& listener : aListeners)
    {
        listener->changedContents( aEv );
    }
}

OUString VclGtkClipboard::getName()
{
    return (m_eSelection == SELECTION_CLIPBOARD) ? OUString("CLIPBOARD") : OUString("PRIMARY");
}

sal_Int8 VclGtkClipboard::getRenderingCapabilities()
{
    return 0;
}

void VclGtkClipboard::addClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    m_aListeners.push_back( listener );
}

void VclGtkClipboard::removeClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), listener), m_aListeners.end());
}

// We run unit tests in parallel, which is a problem when touching a shared resource
// the system clipboard, so rather use the dummy GenericClipboard.
// Note, cannot make this a global variable, because it might be initialised BEFORE the putenv() call in cppunittester.
static bool IsRunningUnitTest() { return getenv("LO_TESTNAME") != nullptr; }

Reference< XInterface > GtkInstance::CreateClipboard(const Sequence< Any >& arguments)
{
    if ( IsRunningUnitTest() )
        return SalInstance::CreateClipboard( arguments );

    OUString sel;
    if (!arguments.hasElements()) {
        sel = "CLIPBOARD";
    } else if (arguments.getLength() != 1 || !(arguments[0] >>= sel)) {
        throw css::lang::IllegalArgumentException(
            "bad GtkInstance::CreateClipboard arguments",
            css::uno::Reference<css::uno::XInterface>(), -1);
    }

    SelectionType eSelection = (sel == "CLIPBOARD") ? SELECTION_CLIPBOARD : SELECTION_PRIMARY;

    if (m_aClipboards[eSelection].is())
        return m_aClipboards[eSelection];

    Reference<XInterface> xClipboard(static_cast<cppu::OWeakObject *>(new VclGtkClipboard(eSelection)));
    m_aClipboards[eSelection] = xClipboard;
    return xClipboard;
}

GtkInstDropTarget::GtkInstDropTarget()
    : WeakComponentImplHelper(m_aMutex)
    , m_pFrame(nullptr)
    , m_pFormatConversionRequest(nullptr)
    , m_bActive(false)
#if !GTK_CHECK_VERSION(4, 0, 0)
    , m_bInDrag(false)
#endif
    , m_nDefaultActions(0)
{
}

OUString SAL_CALL GtkInstDropTarget::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclGtkDropTarget";
}

sal_Bool SAL_CALL GtkInstDropTarget::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL GtkInstDropTarget::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.datatransfer.dnd.GtkDropTarget" };
    return aRet;
}

GtkInstDropTarget::~GtkInstDropTarget()
{
    if (m_pFrame)
        m_pFrame->deregisterDropTarget(this);
}

void GtkInstDropTarget::deinitialize()
{
    m_pFrame = nullptr;
    m_bActive = false;
}

void GtkInstDropTarget::initialize(const Sequence<Any>& rArguments)
{
    if (rArguments.getLength() < 2)
    {
        throw RuntimeException("DropTarget::initialize: Cannot install window event handler",
                               static_cast<OWeakObject*>(this));
    }

    sal_IntPtr nFrame = 0;
    rArguments.getConstArray()[1] >>= nFrame;

    if (!nFrame)
    {
        throw RuntimeException("DropTarget::initialize: missing SalFrame",
                               static_cast<OWeakObject*>(this));
    }

    m_pFrame = reinterpret_cast<GtkSalFrame*>(nFrame);
    m_pFrame->registerDropTarget(this);
    m_bActive = true;
}

void GtkInstDropTarget::addDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& xListener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.push_back( xListener );
}

void GtkInstDropTarget::removeDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& xListener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), xListener), m_aListeners.end());
}

void GtkInstDropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    osl::ClearableGuard<osl::Mutex> aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->drop( dtde );
    }
}

void GtkInstDropTarget::fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragEnter( dtde );
    }
}

void GtkInstDropTarget::fire_dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragOver( dtde );
    }
}

void GtkInstDropTarget::fire_dragExit(const css::datatransfer::dnd::DropTargetEvent& dte)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragExit( dte );
    }
}

sal_Bool GtkInstDropTarget::isActive()
{
    return m_bActive;
}

void GtkInstDropTarget::setActive(sal_Bool bActive)
{
    m_bActive = bActive;
}

sal_Int8 GtkInstDropTarget::getDefaultActions()
{
    return m_nDefaultActions;
}

void GtkInstDropTarget::setDefaultActions(sal_Int8 nDefaultActions)
{
    m_nDefaultActions = nDefaultActions;
}

Reference< XInterface > GtkInstance::CreateDropTarget()
{
    if ( IsRunningUnitTest() )
        return SalInstance::CreateDropTarget();

    return Reference<XInterface>(static_cast<cppu::OWeakObject*>(new GtkInstDropTarget));
}

GtkInstDragSource::~GtkInstDragSource()
{
    if (m_pFrame)
        m_pFrame->deregisterDragSource(this);

    if (GtkInstDragSource::g_ActiveDragSource == this)
    {
        SAL_WARN( "vcl.gtk", "dragEnd should have been called on GtkInstDragSource before dtor");
        GtkInstDragSource::g_ActiveDragSource = nullptr;
    }
}

void GtkInstDragSource::deinitialize()
{
    m_pFrame = nullptr;
}

sal_Bool GtkInstDragSource::isDragImageSupported()
{
    return true;
}

sal_Int32 GtkInstDragSource::getDefaultCursor( sal_Int8 )
{
    return 0;
}

void GtkInstDragSource::initialize(const css::uno::Sequence<css::uno::Any >& rArguments)
{
    if (rArguments.getLength() < 2)
    {
        throw RuntimeException("DragSource::initialize: Cannot install window event handler",
                               static_cast<OWeakObject*>(this));
    }

    sal_IntPtr nFrame = 0;
    rArguments.getConstArray()[1] >>= nFrame;

    if (!nFrame)
    {
        throw RuntimeException("DragSource::initialize: missing SalFrame",
                               static_cast<OWeakObject*>(this));
    }

    m_pFrame = reinterpret_cast<GtkSalFrame*>(nFrame);
    m_pFrame->registerDragSource(this);
}

OUString SAL_CALL GtkInstDragSource::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclGtkDragSource";
}

sal_Bool SAL_CALL GtkInstDragSource::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL GtkInstDragSource::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.datatransfer.dnd.GtkDragSource" };
    return aRet;
}

Reference< XInterface > GtkInstance::CreateDragSource()
{
    if ( IsRunningUnitTest() )
        return SalInstance::CreateDragSource();

    return Reference< XInterface >( static_cast<cppu::OWeakObject *>(new GtkInstDragSource()) );
}

namespace {

class GtkOpenGLContext : public OpenGLContext
{
    GLWindow m_aGLWin;
    GtkWidget *m_pGLArea;
    GdkGLContext *m_pContext;
    gulong m_nDestroySignalId;
    gulong m_nRenderSignalId;
    guint m_nAreaFrameBuffer;
    guint m_nFrameBuffer;
    guint m_nRenderBuffer;
    guint m_nDepthBuffer;
    guint m_nFrameScratchBuffer;
    guint m_nRenderScratchBuffer;
    guint m_nDepthScratchBuffer;

public:
    GtkOpenGLContext()
        : m_pGLArea(nullptr)
        , m_pContext(nullptr)
        , m_nDestroySignalId(0)
        , m_nRenderSignalId(0)
        , m_nAreaFrameBuffer(0)
        , m_nFrameBuffer(0)
        , m_nRenderBuffer(0)
        , m_nDepthBuffer(0)
        , m_nFrameScratchBuffer(0)
        , m_nRenderScratchBuffer(0)
        , m_nDepthScratchBuffer(0)
    {
    }

    virtual void initWindow() override
    {
        if( !m_pChildWindow )
        {
            SystemWindowData winData = generateWinData(mpWindow, mbRequestLegacyContext);
            m_pChildWindow = VclPtr<SystemChildWindow>::Create(mpWindow, 0, &winData, false);
        }

        if (m_pChildWindow)
        {
            InitChildWindow(m_pChildWindow.get());
        }
    }

private:
    virtual const GLWindow& getOpenGLWindow() const override { return m_aGLWin; }
    virtual GLWindow& getModifiableOpenGLWindow() override { return m_aGLWin; }

    static void signalDestroy(GtkWidget*, gpointer context)
    {
        GtkOpenGLContext* pThis = static_cast<GtkOpenGLContext*>(context);
        pThis->m_pGLArea = nullptr;
        pThis->m_nDestroySignalId = 0;
        pThis->m_nRenderSignalId = 0;
    }

    static gboolean signalRender(GtkGLArea*, GdkGLContext*, gpointer window)
    {
        GtkOpenGLContext* pThis = static_cast<GtkOpenGLContext*>(window);

        int scale = gtk_widget_get_scale_factor(pThis->m_pGLArea);
        int width = pThis->m_aGLWin.Width * scale;
        int height = pThis->m_aGLWin.Height * scale;

        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, pThis->m_nAreaFrameBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        gdk_gl_context_make_current(pThis->m_pContext);
        return true;
    }

    virtual void adjustToNewSize() override
    {
        if (!m_pGLArea)
            return;

        int scale = gtk_widget_get_scale_factor(m_pGLArea);
        int width = m_aGLWin.Width * scale;
        int height = m_aGLWin.Height * scale;

        // seen in tdf#124729 width/height of 0 leading to GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
        int allocwidth = std::max(width, 1);
        int allocheight = std::max(height, 1);

        gtk_gl_area_make_current(GTK_GL_AREA(m_pGLArea));
        if (GError *pError = gtk_gl_area_get_error(GTK_GL_AREA(m_pGLArea)))
        {
            SAL_WARN("vcl.gtk", "gtk gl area error: " << pError->message);
            return;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, m_nRenderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, allocwidth, allocheight);
        glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, allocwidth, allocheight);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_nAreaFrameBuffer);

        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                     GL_RENDERBUFFER_EXT, m_nRenderBuffer);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT, m_nDepthBuffer);

        gdk_gl_context_make_current(m_pContext);
        glBindRenderbuffer(GL_RENDERBUFFER, m_nRenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthBuffer);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_nFrameBuffer);

        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                     GL_RENDERBUFFER_EXT, m_nRenderBuffer);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT, m_nDepthBuffer);
        glViewport(0, 0, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, m_nRenderScratchBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, allocwidth, allocheight);
        glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthScratchBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, allocwidth, allocheight);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_nFrameScratchBuffer);

        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                     GL_RENDERBUFFER_EXT, m_nRenderScratchBuffer);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT, m_nDepthScratchBuffer);

        glViewport(0, 0, width, height);
    }

    virtual bool ImplInit() override
    {
        const SystemEnvData* pEnvData = m_pChildWindow->GetSystemData();
        GtkWidget *pParent = static_cast<GtkWidget*>(pEnvData->pWidget);
        m_pGLArea = gtk_gl_area_new();
        m_nDestroySignalId = g_signal_connect(G_OBJECT(m_pGLArea), "destroy", G_CALLBACK(signalDestroy), this);
        m_nRenderSignalId = g_signal_connect(G_OBJECT(m_pGLArea), "render", G_CALLBACK(signalRender), this);
        gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(m_pGLArea), true);
        gtk_gl_area_set_auto_render(GTK_GL_AREA(m_pGLArea), false);
        gtk_widget_set_hexpand(m_pGLArea, true);
        gtk_widget_set_vexpand(m_pGLArea, true);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_add(GTK_CONTAINER(pParent), m_pGLArea);
        gtk_widget_show_all(pParent);
#else
        gtk_grid_attach(GTK_GRID(pParent), m_pGLArea, 0, 0, 1, 1);
        gtk_widget_show(pParent);
        gtk_widget_show(m_pGLArea);
#endif

        gtk_gl_area_make_current(GTK_GL_AREA(m_pGLArea));
        if (GError *pError = gtk_gl_area_get_error(GTK_GL_AREA(m_pGLArea)))
        {
            SAL_WARN("vcl.gtk", "gtk gl area error: " << pError->message);
            return false;
        }

        gtk_gl_area_attach_buffers(GTK_GL_AREA(m_pGLArea));
        glGenFramebuffersEXT(1, &m_nAreaFrameBuffer);

        GdkSurface* pWindow = widget_get_surface(pParent);
        m_pContext = surface_create_gl_context(pWindow);
        if (!m_pContext)
            return false;

        if (!gdk_gl_context_realize(m_pContext, nullptr))
            return false;

        gdk_gl_context_make_current(m_pContext);
        glGenFramebuffersEXT(1, &m_nFrameBuffer);
        glGenRenderbuffersEXT(1, &m_nRenderBuffer);
        glGenRenderbuffersEXT(1, &m_nDepthBuffer);
        glGenFramebuffersEXT(1, &m_nFrameScratchBuffer);
        glGenRenderbuffersEXT(1, &m_nRenderScratchBuffer);
        glGenRenderbuffersEXT(1, &m_nDepthScratchBuffer);

        bool bRet = InitGL();
        InitGLDebugging();
        return bRet;
    }

    virtual void restoreDefaultFramebuffer() override
    {
        OpenGLContext::restoreDefaultFramebuffer();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_nFrameScratchBuffer);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                     GL_RENDERBUFFER_EXT, m_nRenderScratchBuffer);
    }

    virtual void makeCurrent() override
    {
        if (isCurrent())
            return;

        clearCurrent();

        if (m_pGLArea)
        {
            int scale = gtk_widget_get_scale_factor(m_pGLArea);
            int width = m_aGLWin.Width * scale;
            int height = m_aGLWin.Height * scale;

            gdk_gl_context_make_current(m_pContext);

            glBindRenderbuffer(GL_RENDERBUFFER, m_nRenderScratchBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthScratchBuffer);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_nFrameScratchBuffer);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                         GL_RENDERBUFFER_EXT, m_nRenderScratchBuffer);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                         GL_RENDERBUFFER_EXT, m_nDepthScratchBuffer);
            glViewport(0, 0, width, height);
        }

        registerAsCurrent();
    }

    virtual void destroyCurrentContext() override
    {
        gdk_gl_context_clear_current();
    }

    virtual bool isCurrent() override
    {
        return m_pGLArea && gdk_gl_context_get_current() == m_pContext;
    }

    virtual void sync() override
    {
    }

    virtual void resetCurrent() override
    {
        clearCurrent();
        gdk_gl_context_clear_current();
    }

    virtual void swapBuffers() override
    {
        int scale = gtk_widget_get_scale_factor(m_pGLArea);
        int width = m_aGLWin.Width * scale;
        int height = m_aGLWin.Height * scale;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_nFrameBuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_nFrameScratchBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_nFrameScratchBuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        gtk_gl_area_queue_render(GTK_GL_AREA(m_pGLArea));
        BuffersSwapped();
    }

    virtual ~GtkOpenGLContext() override
    {
        if (m_nDestroySignalId)
            g_signal_handler_disconnect(m_pGLArea, m_nDestroySignalId);
        if (m_nRenderSignalId)
            g_signal_handler_disconnect(m_pGLArea, m_nRenderSignalId);
        if (m_pContext)
            g_clear_object(&m_pContext);
    }
};

}

OpenGLContext* GtkInstance::CreateOpenGLContext()
{
    return new GtkOpenGLContext;
}

// tdf#123800 avoid requiring wayland at runtime just because it existed at buildtime
bool DLSYM_GDK_IS_WAYLAND_DISPLAY(GdkDisplay* pDisplay)
{
    static auto get_type = reinterpret_cast<GType (*) (void)>(dlsym(nullptr, "gdk_wayland_display_get_type"));
    if (!get_type)
        return false;
    static bool bResult = G_TYPE_CHECK_INSTANCE_TYPE(pDisplay, get_type());
    return bResult;
}

bool DLSYM_GDK_IS_X11_DISPLAY(GdkDisplay* pDisplay)
{
    static auto get_type = reinterpret_cast<GType (*) (void)>(dlsym(nullptr, "gdk_x11_display_get_type"));
    if (!get_type)
        return false;
    static bool bResult = G_TYPE_CHECK_INSTANCE_TYPE(pDisplay, get_type());
    return bResult;
}

namespace
{

class GtkInstanceBuilder;

    void set_help_id(const GtkWidget *pWidget, const OString& rHelpId)
    {
        gchar *helpid = g_strdup(rHelpId.getStr());
        g_object_set_data_full(G_OBJECT(pWidget), "g-lo-helpid", helpid, g_free);
    }

    OString get_help_id(const GtkWidget *pWidget)
    {
        void* pData = g_object_get_data(G_OBJECT(pWidget), "g-lo-helpid");
        const gchar* pStr = static_cast<const gchar*>(pData);
        return OString(pStr, pStr ? strlen(pStr) : 0);
    }

    KeyEvent CreateKeyEvent(guint keyval, guint16 hardware_keycode, guint state, guint8 group)
    {
        sal_uInt16 nKeyCode = GtkSalFrame::GetKeyCode(keyval);
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (nKeyCode == 0)
        {
            guint updated_keyval = GtkSalFrame::GetKeyValFor(gdk_keymap_get_default(), hardware_keycode, group);
            nKeyCode = GtkSalFrame::GetKeyCode(updated_keyval);
        }
#else
        (void)hardware_keycode;
        (void)group;
#endif
        nKeyCode |= GtkSalFrame::GetKeyModCode(state);
        return KeyEvent(gdk_keyval_to_unicode(keyval), nKeyCode, 0);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    KeyEvent GtkToVcl(const GdkEventKey& rEvent)
    {
        return CreateKeyEvent(rEvent.keyval, rEvent.hardware_keycode, rEvent.state, rEvent.group);
    }
#endif
}

static MouseEventModifiers ImplGetMouseButtonMode(sal_uInt16 nButton, sal_uInt16 nCode)
{
    MouseEventModifiers nMode = MouseEventModifiers::NONE;
    if ( nButton == MOUSE_LEFT )
        nMode |= MouseEventModifiers::SIMPLECLICK;
    if ( (nButton == MOUSE_LEFT) && !(nCode & (MOUSE_MIDDLE | MOUSE_RIGHT)) )
        nMode |= MouseEventModifiers::SELECT;
    if ( (nButton == MOUSE_LEFT) && (nCode & KEY_MOD1) &&
         !(nCode & (MOUSE_MIDDLE | MOUSE_RIGHT | KEY_SHIFT)) )
        nMode |= MouseEventModifiers::MULTISELECT;
    if ( (nButton == MOUSE_LEFT) && (nCode & KEY_SHIFT) &&
         !(nCode & (MOUSE_MIDDLE | MOUSE_RIGHT | KEY_MOD1)) )
        nMode |= MouseEventModifiers::RANGESELECT;
    return nMode;
}

static MouseEventModifiers ImplGetMouseMoveMode(sal_uInt16 nCode)
{
    MouseEventModifiers nMode = MouseEventModifiers::NONE;
    if ( !nCode )
        nMode |= MouseEventModifiers::SIMPLEMOVE;
    if ( (nCode & MOUSE_LEFT) && !(nCode & KEY_MOD1) )
        nMode |= MouseEventModifiers::DRAGMOVE;
    if ( (nCode & MOUSE_LEFT) && (nCode & KEY_MOD1) )
        nMode |= MouseEventModifiers::DRAGCOPY;
    return nMode;
}

namespace
{
    bool SwapForRTL(GtkWidget* pWidget)
    {
        GtkTextDirection eDir = gtk_widget_get_direction(pWidget);
        if (eDir == GTK_TEXT_DIR_RTL)
            return true;
        if (eDir == GTK_TEXT_DIR_LTR)
            return false;
        return AllSettings::GetLayoutRTL();
    }

    GtkWidget* getPopupRect(GtkWidget* pWidget, const tools::Rectangle& rInRect, GdkRectangle& rOutRect)
    {
        if (GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(pWidget))
        {
            // this is the relatively unusual case where pParent is the toplevel GtkSalFrame and not a stock GtkWidget
            // so use the same style of logic as GtkSalMenu::ShowNativePopupMenu to get the right position
            tools::Rectangle aFloatRect = FloatingWindow::ImplConvertToAbsPos(pFrame->GetWindow(), rInRect);
            aFloatRect.Move(-pFrame->maGeometry.nX, -pFrame->maGeometry.nY);

            rOutRect = GdkRectangle{static_cast<int>(aFloatRect.Left()), static_cast<int>(aFloatRect.Top()),
                                    static_cast<int>(aFloatRect.GetWidth()), static_cast<int>(aFloatRect.GetHeight())};

            pWidget = pFrame->getMouseEventWidget();
        }
        else
        {
            rOutRect = GdkRectangle{static_cast<int>(rInRect.Left()), static_cast<int>(rInRect.Top()),
                                 static_cast<int>(rInRect.GetWidth()), static_cast<int>(rInRect.GetHeight())};
            if (SwapForRTL(pWidget))
                rOutRect.x = gtk_widget_get_allocated_width(pWidget) - rOutRect.width - 1 - rOutRect.x;
        }
        return pWidget;
    }

    void replaceWidget(GtkWidget* pWidget, GtkWidget* pReplacement)
    {
        // remove the widget and replace it with pReplacement
        GtkWidget* pParent = gtk_widget_get_parent(pWidget);

        // if pWidget was un-parented then don't bother
        if (!pParent)
            return;

        g_object_ref(pWidget);

        gint nTopAttach(0), nLeftAttach(0), nHeight(1), nWidth(1);
        if (GTK_IS_GRID(pParent))
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            gtk_container_child_get(GTK_CONTAINER(pParent), pWidget,
                    "left-attach", &nLeftAttach,
                    "top-attach", &nTopAttach,
                    "width", &nWidth,
                    "height", &nHeight,
                    nullptr);
#else
            gtk_grid_query_child(GTK_GRID(pParent), pWidget,
                                 &nLeftAttach, &nTopAttach,
                                 &nWidth, &nHeight);
#endif
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        gboolean bExpand(false), bFill(false);
        GtkPackType ePackType(GTK_PACK_START);
        guint nPadding(0);
        gint nPosition(0);
        if (GTK_IS_BOX(pParent))
        {
            gtk_container_child_get(GTK_CONTAINER(pParent), pWidget,
                    "expand", &bExpand,
                    "fill", &bFill,
                    "pack-type", &ePackType,
                    "padding", &nPadding,
                    "position", &nPosition,
                    nullptr);
        }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
        // for gtk3 remove before replacement inserted, or there are warnings
        // from GTK_BIN about having two children
        container_remove(pParent, pWidget);
#endif

        gtk_widget_set_visible(pReplacement, gtk_widget_get_visible(pWidget));
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_set_no_show_all(pReplacement, gtk_widget_get_no_show_all(pWidget));
#endif

        int nReqWidth, nReqHeight;
        gtk_widget_get_size_request(pWidget, &nReqWidth, &nReqHeight);
        gtk_widget_set_size_request(pReplacement, nReqWidth, nReqHeight);

        static GQuark quark_size_groups = g_quark_from_static_string("gtk-widget-size-groups");
        GSList* pSizeGroups = static_cast<GSList*>(g_object_get_qdata(G_OBJECT(pWidget), quark_size_groups));
        while (pSizeGroups)
        {
            GtkSizeGroup *pSizeGroup = static_cast<GtkSizeGroup*>(pSizeGroups->data);
            pSizeGroups = pSizeGroups->next;
            gtk_size_group_remove_widget(pSizeGroup, pWidget);
            gtk_size_group_add_widget(pSizeGroup, pReplacement);
        }

        // tdf#135368 change the mnemonic to point to our replacement
        GList* pLabels = gtk_widget_list_mnemonic_labels(pWidget);
        for (GList* pLabel = g_list_first(pLabels); pLabel; pLabel = g_list_next(pLabel))
        {
            GtkWidget* pLabelWidget = static_cast<GtkWidget*>(pLabel->data);
            if (!GTK_IS_LABEL(pLabelWidget))
                continue;
            gtk_label_set_mnemonic_widget(GTK_LABEL(pLabelWidget), pReplacement);
        }
        g_list_free(pLabels);


        if (GTK_IS_GRID(pParent))
        {
            gtk_grid_attach(GTK_GRID(pParent), pReplacement, nLeftAttach, nTopAttach, nWidth, nHeight);
        }
        else if (GTK_IS_BOX(pParent))
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            gtk_box_pack_start(GTK_BOX(pParent), pReplacement, bExpand, bFill, nPadding);
            gtk_container_child_set(GTK_CONTAINER(pParent), pReplacement,
                    "pack-type", ePackType,
                    "position", nPosition,
                    nullptr);
#else
            gtk_box_insert_child_after(GTK_BOX(pParent), pReplacement, pWidget);
#endif
        }
#if !GTK_CHECK_VERSION(4, 0, 0)
        else
            gtk_container_add(GTK_CONTAINER(pParent), pReplacement);
#endif

        if (gtk_widget_get_hexpand_set(pWidget))
            gtk_widget_set_hexpand(pReplacement, gtk_widget_get_hexpand(pWidget));

        if (gtk_widget_get_vexpand_set(pWidget))
            gtk_widget_set_vexpand(pReplacement, gtk_widget_get_vexpand(pWidget));

        gtk_widget_set_halign(pReplacement, gtk_widget_get_halign(pWidget));
        gtk_widget_set_valign(pReplacement, gtk_widget_get_valign(pWidget));

#if GTK_CHECK_VERSION(4, 0, 0)
        // for gtk4 remove after replacement inserted so we could use gtk_box_insert_child_after
        container_remove(pParent, pWidget);
#endif

        // coverity[freed_arg : FALSE] - this does not free pWidget, it is reffed by pReplacement
        g_object_unref(pWidget);
    }

    void insertAsParent(GtkWidget* pWidget, GtkWidget* pReplacement)
    {
        g_object_ref(pWidget);

        replaceWidget(pWidget, pReplacement);

        // coverity[pass_freed_arg : FALSE] - pWidget is not freed here due to initial g_object_ref
        container_add(pReplacement, pWidget);

        // coverity[freed_arg : FALSE] - this does not free pWidget, it is reffed by pReplacement
        g_object_unref(pWidget);
    }

    GtkWidget* ensureEventWidget(GtkWidget* pWidget)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return pWidget;
#else

        if (!pWidget)
            return nullptr;

        GtkWidget* pMouseEventBox;
        // not every widget has a GdkWindow and can get any event, so if we
        // want an event it doesn't have, insert a GtkEventBox so we can get
        // those
        if (gtk_widget_get_has_window(pWidget))
            pMouseEventBox = pWidget;
        else
        {
            // remove the widget and replace it with an eventbox and put the old
            // widget into it
            pMouseEventBox = gtk_event_box_new();
            gtk_event_box_set_above_child(GTK_EVENT_BOX(pMouseEventBox), false);
            gtk_event_box_set_visible_window(GTK_EVENT_BOX(pMouseEventBox), false);
            insertAsParent(pWidget, pMouseEventBox);
        }

        return pMouseEventBox;
#endif
    }
}

namespace {

#if !GTK_CHECK_VERSION(4, 0, 0)
GdkDragAction VclToGdk(sal_Int8 dragOperation)
{
    GdkDragAction eRet(static_cast<GdkDragAction>(0));
    if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_COPY)
        eRet = static_cast<GdkDragAction>(eRet | GDK_ACTION_COPY);
    if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
        eRet = static_cast<GdkDragAction>(eRet | GDK_ACTION_MOVE);
    if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_LINK)
        eRet = static_cast<GdkDragAction>(eRet | GDK_ACTION_LINK);
    return eRet;
}
#endif

GtkWindow* get_active_window()
{
    GtkWindow* pFocus = nullptr;

    GList* pList = gtk_window_list_toplevels();

    for (GList* pEntry = pList; pEntry; pEntry = pEntry->next)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        if (gtk_window_is_active(GTK_WINDOW(pEntry->data)))
#else
        if (gtk_window_has_toplevel_focus(GTK_WINDOW(pEntry->data)))
#endif
        {
            pFocus = GTK_WINDOW(pEntry->data);
            break;
        }
    }

    g_list_free(pList);

    return pFocus;
}

void LocalizeDecimalSeparator(guint& keyval)
{
    // #i1820# use locale specific decimal separator
    if (keyval == GDK_KEY_KP_Decimal && Application::GetSettings().GetMiscSettings().GetEnableLocalizedDecimalSep())
    {
        GtkWindow* pFocusWin = get_active_window();
        GtkWidget* pFocus = pFocusWin ? gtk_window_get_focus(pFocusWin) : nullptr;
        // tdf#138932 except if the target is a GtkEntry used for passwords
        // GTK4: TODO is it a GtkEntry or a child GtkText that has the focus in this situation?
        if (!pFocus || !GTK_IS_ENTRY(pFocus) || gtk_entry_get_visibility(GTK_ENTRY(pFocus)))
        {
            OUString aSep(Application::GetSettings().GetLocaleDataWrapper().getNumDecimalSep());
            keyval = aSep[0];
        }
    }
}

void set_cursor(GtkWidget* pWidget, const char *pName)
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    if (!gtk_widget_get_realized(pWidget))
        gtk_widget_realize(pWidget);
    GdkDisplay *pDisplay = gtk_widget_get_display(pWidget);
    GdkCursor *pCursor = pName ? gdk_cursor_new_from_name(pDisplay, pName) : nullptr;
    widget_set_cursor(pWidget, pCursor);
    gdk_display_flush(pDisplay);
    if (pCursor)
        g_object_unref(pCursor);
#else
    (void)pWidget;
    (void)pName;
#endif
}

vcl::Font get_font(GtkWidget* pWidget)
{
    PangoContext* pContext = gtk_widget_get_pango_context(pWidget);
    return pango_to_vcl(pango_context_get_font_description(pContext),
                        Application::GetSettings().GetUILanguageTag().getLocale());
}

}

OString get_buildable_id(GtkBuildable* pWidget)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    const gchar* pStr = gtk_buildable_get_buildable_id(pWidget);
#else
    const gchar* pStr = gtk_buildable_get_name(pWidget);
#endif
    return OString(pStr, pStr ? strlen(pStr) : 0);
}

void set_buildable_id(GtkBuildable* pWidget, const OString& rId)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkBuildableIface *iface = GTK_BUILDABLE_GET_IFACE(pWidget);
    (*iface->set_id)(pWidget, rId.getStr());
#else
    gtk_buildable_set_name(pWidget, rId.getStr());
#endif
}

namespace {

class GtkInstanceWidget : public virtual weld::Widget
{
protected:
    GtkWidget* m_pWidget;
    GtkWidget* m_pMouseEventBox;
    GtkInstanceBuilder* m_pBuilder;

#if !GTK_CHECK_VERSION(4, 0, 0)
    DECL_LINK(async_drag_cancel, void*, void);
#endif

    bool IsFirstFreeze() const { return m_nFreezeCount == 0; }
    bool IsLastThaw() const { return m_nFreezeCount == 1; }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalFocusIn(GtkEventControllerFocus*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_in();
    }
#else
    static gboolean signalFocusIn(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_in();
        return false;
    }
#endif

    void signal_focus_in()
    {
        GtkWidget* pTopLevel = widget_get_toplevel(m_pWidget);
        // see commentary in GtkSalObjectWidgetClip::Show
        if (pTopLevel && g_object_get_data(G_OBJECT(pTopLevel), "g-lo-BlockFocusChange"))
            return;

        m_aFocusInHdl.Call(*this);
    }

    static gboolean signalMnemonicActivate(GtkWidget*, gboolean, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_mnemonic_activate();
    }

    bool signal_mnemonic_activate()
    {
        return m_aMnemonicActivateHdl.Call(*this);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalFocusOut(GtkEventControllerFocus*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_in();
    }
#else
    static gboolean signalFocusOut(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_out();
        return false;
    }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    void launch_drag_cancel(GdkDragContext* context)
    {
        // post our drag cancel to happen at the next available event cycle
        if (m_pDragCancelEvent)
            return;
        g_object_ref(context);
        m_pDragCancelEvent = Application::PostUserEvent(LINK(this, GtkInstanceWidget, async_drag_cancel), context);
    }
#endif

    void signal_focus_out()
    {
        GtkWidget* pTopLevel = widget_get_toplevel(m_pWidget);
        // see commentary in GtkSalObjectWidgetClip::Show
        if (pTopLevel && g_object_get_data(G_OBJECT(pTopLevel), "g-lo-BlockFocusChange"))
            return;

        m_aFocusOutHdl.Call(*this);
    }

    virtual void ensureMouseEventWidget()
    {
        if (!m_pMouseEventBox)
            m_pMouseEventBox = ::ensureEventWidget(m_pWidget);
    }

    void ensureButtonPressSignal()
    {
        if (!m_nButtonPressSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            GtkEventController* pClickController = get_click_controller();
            m_nButtonPressSignalId = g_signal_connect(pClickController, "pressed", G_CALLBACK(signalButtonPress), this);
#else
            ensureMouseEventWidget();
            m_nButtonPressSignalId = g_signal_connect(m_pMouseEventBox, "button-press-event", G_CALLBACK(signalButtonPress), this);
#endif
        }
    }

    void ensureButtonReleaseSignal()
    {
        if (!m_nButtonReleaseSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            GtkEventController* pClickController = get_click_controller();
            m_nButtonReleaseSignalId = g_signal_connect(pClickController, "released", G_CALLBACK(signalButtonRelease), this);
#else
            ensureMouseEventWidget();
            m_nButtonReleaseSignalId = g_signal_connect(m_pMouseEventBox, "button-release-event", G_CALLBACK(signalButtonRelease), this);
#endif
        }
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalPopupMenu(GtkWidget* pWidget, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        //center it when we don't know where else to use
        Point aPos(gtk_widget_get_allocated_width(pWidget) / 2,
                   gtk_widget_get_allocated_height(pWidget) / 2);
        CommandEvent aCEvt(aPos, CommandEventId::ContextMenu, false);
        return pThis->signal_popup_menu(aCEvt);
    }
#endif

    bool SwapForRTL() const
    {
        return ::SwapForRTL(m_pWidget);
    }

    void do_enable_drag_source(const rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants)
    {
        ensure_drag_source();

#if !GTK_CHECK_VERSION(4, 0, 0)
        auto aFormats = rHelper->getTransferDataFlavors();
        std::vector<GtkTargetEntry> aGtkTargets(m_xDragSource->FormatsToGtk(aFormats));

        m_eDragAction = VclToGdk(eDNDConstants);
        drag_source_set(aGtkTargets, m_eDragAction);

        for (auto &a : aGtkTargets)
            g_free(a.target);

        m_xDragSource->set_datatransfer(rHelper, rHelper);
#else
        (void)rHelper;
        (void)eDNDConstants;
#endif
    }

    void localizeDecimalSeparator()
    {
        // tdf#128867 if localize decimal separator is active we will always
        // need to be able to change the output of the decimal key press
        if (!m_nKeyPressSignalId && Application::GetSettings().GetMiscSettings().GetEnableLocalizedDecimalSep())
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            m_nKeyPressSignalId = g_signal_connect(get_key_controller(), "key-pressed", G_CALLBACK(signalKeyPressed), this);
#else
            m_nKeyPressSignalId = g_signal_connect(m_pWidget, "key-press-event", G_CALLBACK(signalKey), this);
#endif
        }
    }

    void ensure_drag_begin_end()
    {
        if (!m_nDragBeginSignalId)
        {
            // using "after" due to https://gitlab.gnome.org/GNOME/pygobject/issues/251
#if GTK_CHECK_VERSION(4, 0, 0)
            m_nDragBeginSignalId = g_signal_connect_after(get_drag_controller(), "drag-begin", G_CALLBACK(signalDragBegin), this);
#else
            m_nDragBeginSignalId = g_signal_connect_after(m_pWidget, "drag-begin", G_CALLBACK(signalDragBegin), this);
#endif
        }
        if (!m_nDragEndSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            m_nDragEndSignalId = g_signal_connect(get_drag_controller(), "drag-end", G_CALLBACK(signalDragEnd), this);
#else
            m_nDragEndSignalId = g_signal_connect(m_pWidget, "drag-end", G_CALLBACK(signalDragEnd), this);
#endif
        }
    }

    void DisconnectMouseEvents()
    {
        if (m_nButtonPressSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_click_controller(), m_nButtonPressSignalId);
#else
            g_signal_handler_disconnect(m_pMouseEventBox, m_nButtonPressSignalId);
#endif
            m_nButtonPressSignalId = 0;
        }
        if (m_nMotionSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_motion_controller(), m_nMotionSignalId);
#else
            g_signal_handler_disconnect(m_pMouseEventBox, m_nMotionSignalId);
#endif
            m_nMotionSignalId = 0;
        }
        if (m_nLeaveSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_motion_controller(), m_nLeaveSignalId);
#else
            g_signal_handler_disconnect(m_pMouseEventBox, m_nLeaveSignalId);
#endif
            m_nLeaveSignalId = 0;
        }
        if (m_nEnterSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_motion_controller(), m_nEnterSignalId);
#else
            g_signal_handler_disconnect(m_pMouseEventBox, m_nEnterSignalId);
#endif
            m_nEnterSignalId = 0;
        }
        if (m_nButtonReleaseSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_click_controller(), m_nButtonReleaseSignalId);
#else
            g_signal_handler_disconnect(m_pMouseEventBox, m_nButtonReleaseSignalId);
#endif
            m_nButtonReleaseSignalId = 0;
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!m_pMouseEventBox || m_pMouseEventBox == m_pWidget)
            return;

        // GtkWindow replacement for GtkPopover case
        if (!GTK_IS_EVENT_BOX(m_pMouseEventBox))
        {
            m_pMouseEventBox = nullptr;
            return;
        }

        // put things back they way we found them
        GtkWidget* pParent = gtk_widget_get_parent(m_pMouseEventBox);

        g_object_ref(m_pWidget);
        gtk_container_remove(GTK_CONTAINER(m_pMouseEventBox), m_pWidget);

        gtk_widget_destroy(m_pMouseEventBox);

        gtk_container_add(GTK_CONTAINER(pParent), m_pWidget);
        // coverity[freed_arg : FALSE] - this does not free m_pWidget, it is reffed by pParent
        g_object_unref(m_pWidget);

        m_pMouseEventBox = m_pWidget;
#endif
    }

private:
    bool m_bTakeOwnership;
#if !GTK_CHECK_VERSION(4, 0, 0)
    bool m_bDraggedOver;
#endif
    int m_nWaitCount;
    int m_nFreezeCount;
    sal_uInt16 m_nLastMouseButton;
#if !GTK_CHECK_VERSION(4, 0, 0)
    sal_uInt16 m_nLastMouseClicks;
#endif
    int m_nPressedButton;
#if !GTK_CHECK_VERSION(4, 0, 0)
    int m_nPressStartX;
    int m_nPressStartY;
#endif
    ImplSVEvent* m_pDragCancelEvent;
    GtkCssProvider* m_pBgCssProvider;
#if !GTK_CHECK_VERSION(4, 0, 0)
    GdkDragAction m_eDragAction;
#endif
    gulong m_nFocusInSignalId;
    gulong m_nMnemonicActivateSignalId;
    gulong m_nFocusOutSignalId;
    gulong m_nKeyPressSignalId;
    gulong m_nKeyReleaseSignalId;
protected:
    gulong m_nSizeAllocateSignalId;
private:
    gulong m_nButtonPressSignalId;
    gulong m_nMotionSignalId;
    gulong m_nLeaveSignalId;
    gulong m_nEnterSignalId;
    gulong m_nButtonReleaseSignalId;
    gulong m_nDragMotionSignalId;
    gulong m_nDragDropSignalId;
    gulong m_nDragDropReceivedSignalId;
    gulong m_nDragLeaveSignalId;
    gulong m_nDragBeginSignalId;
    gulong m_nDragEndSignalId;
    gulong m_nDragFailedSignalId;
    gulong m_nDragDataDeleteignalId;
    gulong m_nDragGetSignalId;

#if GTK_CHECK_VERSION(4, 0, 0)
    int m_nGrabCount;
    GtkEventController* m_pFocusController;
    GtkEventController* m_pClickController;
    GtkEventController* m_pMotionController;
    GtkEventController* m_pDragController;
    GtkEventController* m_pKeyController;
#endif

    rtl::Reference<GtkInstDropTarget> m_xDropTarget;
    rtl::Reference<GtkInstDragSource> m_xDragSource;

    static void signalSizeAllocate(GtkWidget*, GdkRectangle* allocation, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_size_allocate(allocation->width, allocation->height);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalKeyPressed(GtkEventControllerKey*, guint keyval, guint keycode, GdkModifierType state, gpointer widget)
    {
        LocalizeDecimalSeparator(keyval);
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        return pThis->signal_key_press(keyval, keycode, state);
    }

    static gboolean signalKeyReleased(GtkEventControllerKey*, guint keyval, guint keycode, GdkModifierType state, gpointer widget)
    {
        LocalizeDecimalSeparator(keyval);
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        return pThis->signal_key_release(keyval, keycode, state);
    }
#else
    static gboolean signalKey(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        LocalizeDecimalSeparator(pEvent->keyval);
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        if (pEvent->type == GDK_KEY_PRESS)
            return pThis->signal_key_press(pEvent);
        return pThis->signal_key_release(pEvent);
    }
#endif

    virtual bool signal_popup_menu(const CommandEvent&)
    {
        return false;
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalButtonPress(GtkGestureClick* pGesture, int n_press, gdouble x, gdouble y, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_button(pGesture, SalEvent::MouseButtonDown, n_press, x, y);
    }

    static void signalButtonRelease(GtkGestureClick* pGesture, int n_press, gdouble x, gdouble y, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_button(pGesture, SalEvent::MouseButtonUp, n_press, x, y);
    }

    void signal_button(GtkGestureClick* pGesture, SalEvent nEventType, int n_press, gdouble x, gdouble y)
    {
        m_nPressedButton = -1;

        Point aPos(x, y);
        if (SwapForRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());

        if (n_press == 1)
        {
            GdkEventSequence* pSequence = gtk_gesture_single_get_current_sequence(GTK_GESTURE_SINGLE(pGesture));
            GdkEvent* pEvent = gtk_gesture_get_last_event(GTK_GESTURE(pGesture), pSequence);
            if (gdk_event_triggers_context_menu(pEvent))
            {
                //if handled for context menu, stop processing
                CommandEvent aCEvt(aPos, CommandEventId::ContextMenu, true);
                if (signal_popup_menu(aCEvt))
                {
                    gtk_gesture_set_state(GTK_GESTURE(pGesture), GTK_EVENT_SEQUENCE_CLAIMED);
                    return;
                }
            }
        }

        GdkModifierType eType = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(pGesture));
        int nButton = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(pGesture));

        switch (nButton)
        {
            case 1:
                m_nLastMouseButton = MOUSE_LEFT;
                break;
            case 2:
                m_nLastMouseButton = MOUSE_MIDDLE;
                break;
            case 3:
                m_nLastMouseButton = MOUSE_RIGHT;
                break;
            default:
                return;
        }

        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(eType);
        // strip out which buttons are involved from the nModCode and replace with m_nLastMouseButton
        sal_uInt16 nCode = m_nLastMouseButton | (nModCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2));
        MouseEvent aMEvt(aPos, n_press, ImplGetMouseButtonMode(m_nLastMouseButton, nModCode), nCode, nCode);

        if (nEventType == SalEvent::MouseButtonDown && m_aMousePressHdl.Call(aMEvt))
            gtk_gesture_set_state(GTK_GESTURE(pGesture), GTK_EVENT_SEQUENCE_CLAIMED);

        if (nEventType == SalEvent::MouseButtonUp && m_aMouseReleaseHdl.Call(aMEvt))
            gtk_gesture_set_state(GTK_GESTURE(pGesture), GTK_EVENT_SEQUENCE_CLAIMED);
    }

#else

    static gboolean signalButtonPress(GtkWidget*, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_button(pEvent);
    }

    static gboolean signalButtonRelease(GtkWidget*, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_button(pEvent);
    }

    bool signal_button(GdkEventButton* pEvent)
    {
        m_nPressedButton = -1;

        Point aPos(pEvent->x, pEvent->y);
        if (SwapForRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());

        if (gdk_event_triggers_context_menu(reinterpret_cast<GdkEvent*>(pEvent)) && pEvent->type == GDK_BUTTON_PRESS)
        {
            //if handled for context menu, stop processing
            CommandEvent aCEvt(aPos, CommandEventId::ContextMenu, true);
            if (signal_popup_menu(aCEvt))
                return true;
        }

        if (!m_aMousePressHdl.IsSet() && !m_aMouseReleaseHdl.IsSet())
            return false;

        SalEvent nEventType = SalEvent::NONE;
        switch (pEvent->type)
        {
            case GDK_BUTTON_PRESS:
                if (GdkEvent* pPeekEvent = gdk_event_peek())
                {
                    bool bSkip = pPeekEvent->type == GDK_2BUTTON_PRESS ||
                                 pPeekEvent->type == GDK_3BUTTON_PRESS;
                    gdk_event_free(pPeekEvent);
                    if (bSkip)
                    {
                        return false;
                    }
                }
                nEventType = SalEvent::MouseButtonDown;
                m_nLastMouseClicks = 1;
                break;
            case GDK_2BUTTON_PRESS:
                m_nLastMouseClicks = 2;
                nEventType = SalEvent::MouseButtonDown;
                break;
            case GDK_3BUTTON_PRESS:
                m_nLastMouseClicks = 3;
                nEventType = SalEvent::MouseButtonDown;
                break;
            case GDK_BUTTON_RELEASE:
                nEventType = SalEvent::MouseButtonUp;
                break;
            default:
                return false;
        }

        switch (pEvent->button)
        {
            case 1:
                m_nLastMouseButton = MOUSE_LEFT;
                break;
            case 2:
                m_nLastMouseButton = MOUSE_MIDDLE;
                break;
            case 3:
                m_nLastMouseButton = MOUSE_RIGHT;
                break;
            default:
                return false;
        }

        /* Save press to possibly begin a drag */
        if (pEvent->type != GDK_BUTTON_RELEASE)
        {
            m_nPressedButton = pEvent->button;
            m_nPressStartX = pEvent->x;
            m_nPressStartY = pEvent->y;
        }

        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(pEvent->state);
        // strip out which buttons are involved from the nModCode and replace with m_nLastMouseButton
        sal_uInt16 nCode = m_nLastMouseButton | (nModCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2));
        MouseEvent aMEvt(aPos, m_nLastMouseClicks, ImplGetMouseButtonMode(m_nLastMouseButton, nModCode), nCode, nCode);

        if (nEventType == SalEvent::MouseButtonDown)
        {
            if (!m_aMousePressHdl.IsSet())
                return false;
            return m_aMousePressHdl.Call(aMEvt);
        }

        if (!m_aMouseReleaseHdl.IsSet())
            return false;
        return m_aMouseReleaseHdl.Call(aMEvt);
    }
#endif

    bool simple_signal_motion(double x, double y, guint nState)
    {
        if (!m_aMouseMotionHdl.IsSet())
            return false;

        Point aPos(x, y);
        if (SwapForRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());
        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(nState);
        MouseEvent aMEvt(aPos, 0, ImplGetMouseMoveMode(nModCode), nModCode, nModCode);

        return m_aMouseMotionHdl.Call(aMEvt);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalMotion(GtkEventControllerMotion *pController, double x, double y, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        GdkModifierType eType = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(pController));

        SolarMutexGuard aGuard;
        pThis->simple_signal_motion(x, y, eType);
    }

#else
    static gboolean signalMotion(GtkWidget*, GdkEventMotion* pEvent, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_motion(pEvent);
    }

    bool signal_motion(const GdkEventMotion* pEvent)
    {
        GtkTargetList* pDragData = (m_eDragAction != 0 && m_nPressedButton != -1 && m_xDragSource.is()) ? gtk_drag_source_get_target_list(m_pWidget) : nullptr;
        bool bUnsetDragIcon(false);
        if (pDragData && gtk_drag_check_threshold(m_pWidget, m_nPressStartX, m_nPressStartY, pEvent->x, pEvent->y) && !do_signal_drag_begin(bUnsetDragIcon))
        {
            GdkDragContext* pContext = gtk_drag_begin_with_coordinates(m_pWidget,
                                                                       pDragData,
                                                                       m_eDragAction,
                                                                       m_nPressedButton,
                                                                       const_cast<GdkEvent*>(reinterpret_cast<const GdkEvent*>(pEvent)),
                                                                       m_nPressStartX, m_nPressStartY);

            if (pContext && bUnsetDragIcon)
            {
                cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
                gtk_drag_set_icon_surface(pContext, surface);
            }

            m_nPressedButton = -1;
            return false;
        }

        return simple_signal_motion(pEvent->x, pEvent->y, pEvent->state);
    }
#endif

    bool signal_crossing(double x, double y, guint nState, MouseEventModifiers eMouseEventModifiers)
    {
        if (!m_aMouseMotionHdl.IsSet())
            return false;

        Point aPos(x, y);
        if (SwapForRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());
        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(nState);
        MouseEventModifiers eModifiers = ImplGetMouseMoveMode(nModCode);
        eModifiers = eModifiers | eMouseEventModifiers;
        MouseEvent aMEvt(aPos, 0, eModifiers, nModCode, nModCode);

        m_aMouseMotionHdl.Call(aMEvt);
        return false;
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalEnter(GtkEventControllerMotion *pController, double x, double y, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        GdkModifierType eType = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(pController));
        SolarMutexGuard aGuard;
        pThis->signal_crossing(x, y, eType, MouseEventModifiers::ENTERWINDOW);
    }

    static void signalLeave(GtkEventControllerMotion *pController, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        GdkModifierType eType = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(pController));
        SolarMutexGuard aGuard;
        pThis->signal_crossing(-1, -1, eType, MouseEventModifiers::LEAVEWINDOW);
    }
#else
    static gboolean signalCrossing(GtkWidget*, GdkEventCrossing* pEvent, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        MouseEventModifiers eMouseEventModifiers = pEvent->type == GDK_ENTER_NOTIFY ? MouseEventModifiers::ENTERWINDOW : MouseEventModifiers::LEAVEWINDOW;
        SolarMutexGuard aGuard;
        return pThis->signal_crossing(pEvent->x, pEvent->y, pEvent->state, eMouseEventModifiers);
    }
#endif

    virtual void drag_started()
    {
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalDragMotion(GtkWidget *pWidget, GdkDragContext *context, gint x, gint y, guint time, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        if (!pThis->m_bDraggedOver)
        {
            pThis->m_bDraggedOver = true;
            pThis->drag_started();
        }
        return pThis->m_xDropTarget->signalDragMotion(pWidget, context, x, y, time);
    }

    static gboolean signalDragDrop(GtkWidget* pWidget, GdkDragContext* context, gint x, gint y, guint time, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        return pThis->m_xDropTarget->signalDragDrop(pWidget, context, x, y, time);
    }

    static void signalDragDropReceived(GtkWidget* pWidget, GdkDragContext* context, gint x, gint y, GtkSelectionData* data, guint ttype, guint time, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->m_xDropTarget->signalDragDropReceived(pWidget, context, x, y, data, ttype, time);
    }
#endif

    virtual void drag_ended()
    {
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void signalDragLeave(GtkWidget* pWidget, GdkDragContext*, guint /*time*/, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->m_xDropTarget->signalDragLeave(pWidget);
        if (pThis->m_bDraggedOver)
        {
            pThis->m_bDraggedOver = false;
            pThis->drag_ended();
        }
    }
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalDragBegin(GtkDragSource* context, GdkDrag*, gpointer widget)
#else
    static void signalDragBegin(GtkWidget*, GdkDragContext* context, gpointer widget)
#endif
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->signal_drag_begin(context);
    }

    void ensure_drag_source()
    {
        if (!m_xDragSource)
        {
            m_xDragSource.set(new GtkInstDragSource);

#if !GTK_CHECK_VERSION(4, 0, 0)
            m_nDragFailedSignalId = g_signal_connect(m_pWidget, "drag-failed", G_CALLBACK(signalDragFailed), this);
            m_nDragDataDeleteignalId = g_signal_connect(m_pWidget, "drag-data-delete", G_CALLBACK(signalDragDelete), this);
            m_nDragGetSignalId = g_signal_connect(m_pWidget, "drag-data-get", G_CALLBACK(signalDragDataGet), this);
#endif

            ensure_drag_begin_end();
        }
    }

    virtual bool do_signal_drag_begin(bool& rUnsetDragIcon)
    {
        rUnsetDragIcon = false;
        return false;
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    void signal_drag_begin(GtkDragSource* context)
#else
    void signal_drag_begin(GdkDragContext* context)
#endif
    {
        bool bUnsetDragIcon(false);
        if (do_signal_drag_begin(bUnsetDragIcon))
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            launch_drag_cancel(context);
#else
            (void)context;
#endif
            return;
        }
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (bUnsetDragIcon)
        {
            cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
            gtk_drag_set_icon_surface(context, surface);
        }
#endif
        if (!m_xDragSource)
            return;
        m_xDragSource->setActiveDragSource();
    }

    virtual void do_signal_drag_end()
    {
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalDragEnd(GtkGestureDrag* /*gesture*/, double /*offset_x*/, double /*offset_y*/, gpointer widget)
#else
    static void signalDragEnd(GtkWidget* /*widget*/, GdkDragContext* context, gpointer widget)
#endif
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->do_signal_drag_end();
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (pThis->m_xDragSource.is())
            pThis->m_xDragSource->dragEnd(context);
#endif
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalDragFailed(GtkWidget* /*widget*/, GdkDragContext* /*context*/, GtkDragResult /*result*/, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->m_xDragSource->dragFailed();
        return false;
    }

    static void signalDragDelete(GtkWidget* /*widget*/, GdkDragContext* /*context*/, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->m_xDragSource->dragDelete();
    }

    static void signalDragDataGet(GtkWidget* /*widget*/, GdkDragContext* /*context*/, GtkSelectionData *data, guint info,
                                  guint /*time*/, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->m_xDragSource->dragDataGet(data, info);
    }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    virtual void drag_source_set(const std::vector<GtkTargetEntry>& rGtkTargets, GdkDragAction eDragAction)
    {
        if (rGtkTargets.empty() && !eDragAction)
            gtk_drag_source_unset(m_pWidget);
        else
            gtk_drag_source_set(m_pWidget, GDK_BUTTON1_MASK, rGtkTargets.data(), rGtkTargets.size(), eDragAction);
    }
#endif

    void do_set_background(const Color& rColor)
    {
        const bool bRemoveColor = rColor == COL_AUTO;
        if (bRemoveColor && !m_pBgCssProvider)
            return;
        GtkStyleContext *pWidgetContext = gtk_widget_get_style_context(GTK_WIDGET(m_pWidget));
        if (m_pBgCssProvider)
        {
            gtk_style_context_remove_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pBgCssProvider));
            m_pBgCssProvider = nullptr;
        }
        if (bRemoveColor)
            return;
        OUString sColor = rColor.AsRGBHexString();
        m_pBgCssProvider = gtk_css_provider_new();
        OUString aBuffer = "* { background-color: #" + sColor + "; }";
        OString aResult = OUStringToOString(aBuffer, RTL_TEXTENCODING_UTF8);
        css_provider_load_from_data(m_pBgCssProvider, aResult.getStr(), aResult.getLength());
        gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pBgCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void update_style(GtkWidget* pWidget, gpointer pData)
    {
        if (GTK_IS_CONTAINER(pWidget))
            gtk_container_foreach(GTK_CONTAINER(pWidget), update_style, pData);
        GtkWidgetClass* pWidgetClass = GTK_WIDGET_GET_CLASS(pWidget);
        pWidgetClass->style_updated(pWidget);
    }
#endif

public:
    GtkInstanceWidget(GtkWidget* pWidget, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : m_pWidget(pWidget)
        , m_pMouseEventBox(nullptr)
        , m_pBuilder(pBuilder)
        , m_bTakeOwnership(bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_bDraggedOver(false)
#endif
        , m_nWaitCount(0)
        , m_nFreezeCount(0)
        , m_nLastMouseButton(0)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nLastMouseClicks(0)
#endif
        , m_nPressedButton(-1)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nPressStartX(-1)
        , m_nPressStartY(-1)
#endif
        , m_pDragCancelEvent(nullptr)
        , m_pBgCssProvider(nullptr)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_eDragAction(GdkDragAction(0))
#endif
        , m_nFocusInSignalId(0)
        , m_nMnemonicActivateSignalId(0)
        , m_nFocusOutSignalId(0)
        , m_nKeyPressSignalId(0)
        , m_nKeyReleaseSignalId(0)
        , m_nSizeAllocateSignalId(0)
        , m_nButtonPressSignalId(0)
        , m_nMotionSignalId(0)
        , m_nLeaveSignalId(0)
        , m_nEnterSignalId(0)
        , m_nButtonReleaseSignalId(0)
        , m_nDragMotionSignalId(0)
        , m_nDragDropSignalId(0)
        , m_nDragDropReceivedSignalId(0)
        , m_nDragLeaveSignalId(0)
        , m_nDragBeginSignalId(0)
        , m_nDragEndSignalId(0)
        , m_nDragFailedSignalId(0)
        , m_nDragDataDeleteignalId(0)
        , m_nDragGetSignalId(0)
#if GTK_CHECK_VERSION(4, 0, 0)
        , m_nGrabCount(0)
        , m_pFocusController(nullptr)
        , m_pClickController(nullptr)
        , m_pMotionController(nullptr)
        , m_pDragController(nullptr)
        , m_pKeyController(nullptr)
#endif
    {
        if (!bTakeOwnership)
            g_object_ref(m_pWidget);

        localizeDecimalSeparator();
    }

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override
    {
        if (!m_nKeyPressSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            m_nKeyPressSignalId = g_signal_connect(get_key_controller(), "key-pressed", G_CALLBACK(signalKeyPressed), this);
#else
            m_nKeyPressSignalId = g_signal_connect(m_pWidget, "key-press-event", G_CALLBACK(signalKey), this);
#endif
        }
        weld::Widget::connect_key_press(rLink);
    }

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override
    {
        if (!m_nKeyReleaseSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            m_nKeyReleaseSignalId = g_signal_connect(get_key_controller(), "key-released", G_CALLBACK(signalKeyReleased), this);
#else
            m_nKeyReleaseSignalId = g_signal_connect(m_pWidget, "key-release-event", G_CALLBACK(signalKey), this);
#endif
        }
        weld::Widget::connect_key_release(rLink);
    }

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensureButtonPressSignal();
        weld::Widget::connect_mouse_press(rLink);
    }

    virtual void connect_mouse_move(const Link<const MouseEvent&, bool>& rLink) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkEventController* pMotionController = get_motion_controller();
        if (!m_nMotionSignalId)
            m_nMotionSignalId = g_signal_connect(pMotionController, "motion", G_CALLBACK(signalMotion), this);
        if (!m_nLeaveSignalId)
            m_nLeaveSignalId = g_signal_connect(pMotionController, "leave", G_CALLBACK(signalEnter), this);
        if (!m_nEnterSignalId)
            m_nEnterSignalId = g_signal_connect(pMotionController, "enter", G_CALLBACK(signalLeave), this);
#else
        ensureMouseEventWidget();
        if (!m_nMotionSignalId)
            m_nMotionSignalId = g_signal_connect(m_pMouseEventBox, "motion-notify-event", G_CALLBACK(signalMotion), this);
        if (!m_nLeaveSignalId)
            m_nLeaveSignalId = g_signal_connect(m_pMouseEventBox, "leave-notify-event", G_CALLBACK(signalCrossing), this);
        if (!m_nEnterSignalId)
            m_nEnterSignalId = g_signal_connect(m_pMouseEventBox, "enter-notify-event", G_CALLBACK(signalCrossing), this);
#endif
        weld::Widget::connect_mouse_move(rLink);
    }

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensureButtonReleaseSignal();
        weld::Widget::connect_mouse_release(rLink);
    }

    virtual void set_sensitive(bool sensitive) override
    {
        gtk_widget_set_sensitive(m_pWidget, sensitive);
    }

    virtual bool get_sensitive() const override
    {
        return gtk_widget_get_sensitive(m_pWidget);
    }

    virtual bool get_visible() const override
    {
        return gtk_widget_get_visible(m_pWidget);
    }

    virtual bool is_visible() const override
    {
        return gtk_widget_is_visible(m_pWidget);
    }

    virtual void set_can_focus(bool bCanFocus) override
    {
        gtk_widget_set_can_focus(m_pWidget, bCanFocus);
    }

    virtual void grab_focus() override
    {
        if (has_focus())
            return;
        gtk_widget_grab_focus(m_pWidget);
    }

    virtual bool has_focus() const override
    {
        return gtk_widget_has_focus(m_pWidget);
    }

    virtual bool is_active() const override
    {
        GtkWindow* pTopLevel = GTK_WINDOW(widget_get_toplevel(m_pWidget));
        return pTopLevel && gtk_window_is_active(pTopLevel) && has_focus();
    }

    // is the focus in a child of this widget, where a transient popup attached
    // to a widget is considered a child of that widget
    virtual bool has_child_focus() const override
    {
        GtkWindow* pFocusWin = get_active_window();
        if (!pFocusWin)
            return false;
        GtkWidget* pFocus = gtk_window_get_focus(pFocusWin);
        if (pFocus && gtk_widget_is_ancestor(pFocus, m_pWidget))
            return true;
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pAttachedTo = gtk_window_get_attached_to(pFocusWin);
        if (!pAttachedTo)
            return false;
        if (pAttachedTo == m_pWidget || gtk_widget_is_ancestor(pAttachedTo, m_pWidget))
            return true;
#endif
        return false;
    }

    virtual void set_has_default(bool has_default) override
    {
        g_object_set(G_OBJECT(m_pWidget), "has-default", has_default, nullptr);
    }

    virtual bool get_has_default() const override
    {
        gboolean has_default(false);
        g_object_get(G_OBJECT(m_pWidget), "has-default", &has_default, nullptr);
        return has_default;
    }

    virtual void show() override
    {
        gtk_widget_show(m_pWidget);
    }

    virtual void hide() override
    {
        gtk_widget_hide(m_pWidget);
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_VIEWPORT(pParent))
            pParent = gtk_widget_get_parent(pParent);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
        {
            gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(pParent), nWidth);
            gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(pParent), nHeight);
        }
        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual Size get_size_request() const override
    {
        int nWidth, nHeight;
        gtk_widget_get_size_request(m_pWidget, &nWidth, &nHeight);
        return Size(nWidth, nHeight);
    }

    virtual Size get_preferred_size() const override
    {
        GtkRequisition size;
        gtk_widget_get_preferred_size(m_pWidget, nullptr, &size);
        return Size(size.width, size.height);
    }

    virtual float get_approximate_digit_width() const override
    {
        PangoContext* pContext = gtk_widget_get_pango_context(m_pWidget);
        PangoFontMetrics* pMetrics = pango_context_get_metrics(pContext,
                                         pango_context_get_font_description(pContext),
                                         pango_context_get_language(pContext));
        float nDigitWidth = pango_font_metrics_get_approximate_digit_width(pMetrics);
        pango_font_metrics_unref(pMetrics);

        return nDigitWidth / PANGO_SCALE;
    }

    virtual int get_text_height() const override
    {
        PangoContext* pContext = gtk_widget_get_pango_context(m_pWidget);
        PangoFontMetrics* pMetrics = pango_context_get_metrics(pContext,
                                         pango_context_get_font_description(pContext),
                                         pango_context_get_language(pContext));
        int nLineHeight = pango_font_metrics_get_ascent(pMetrics) + pango_font_metrics_get_descent(pMetrics);
        pango_font_metrics_unref(pMetrics);
        return nLineHeight / PANGO_SCALE;
    }

    virtual Size get_pixel_size(const OUString& rText) const override
    {
        OString aStr(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        PangoLayout* pLayout = gtk_widget_create_pango_layout(m_pWidget, aStr.getStr());
        gint nWidth, nHeight;
        pango_layout_get_pixel_size(pLayout, &nWidth, &nHeight);
        g_object_unref(pLayout);
        return Size(nWidth, nHeight);
    }

    virtual vcl::Font get_font() override
    {
        return ::get_font(m_pWidget);
    }

    virtual void set_grid_left_attach(int nAttach) override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
        int row, width, height;
        gtk_grid_query_child(GTK_GRID(pParent), m_pWidget, nullptr, &row, &width, &height);
        g_object_ref(m_pWidget);
        gtk_grid_remove(GTK_GRID(pParent), m_pWidget);
        gtk_grid_attach(GTK_GRID(pParent), m_pWidget, nAttach, row, width, height);
        g_object_unref(m_pWidget);
#else
        gtk_container_child_set(GTK_CONTAINER(pParent), m_pWidget, "left-attach", nAttach, nullptr);
#endif
    }

    virtual int get_grid_left_attach() const override
    {
        gint nAttach(0);
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_grid_query_child(GTK_GRID(pParent), m_pWidget, &nAttach, nullptr, nullptr, nullptr);
#else
        gtk_container_child_get(GTK_CONTAINER(pParent), m_pWidget, "left-attach", &nAttach, nullptr);
#endif
        return nAttach;
    }

    virtual void set_grid_width(int nCols) override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
        int col, row, height;
        gtk_grid_query_child(GTK_GRID(pParent), m_pWidget, &col, &row, nullptr, &height);
        g_object_ref(m_pWidget);
        gtk_grid_remove(GTK_GRID(pParent), m_pWidget);
        gtk_grid_attach(GTK_GRID(pParent), m_pWidget, col, row, nCols, height);
        g_object_unref(m_pWidget);
#else
        gtk_container_child_set(GTK_CONTAINER(pParent), m_pWidget, "width", nCols, nullptr);
#endif
    }

    virtual void set_grid_top_attach(int nAttach) override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
        int col, width, height;
        gtk_grid_query_child(GTK_GRID(pParent), m_pWidget, &col, nullptr, &width, &height);
        g_object_ref(m_pWidget);
        gtk_grid_remove(GTK_GRID(pParent), m_pWidget);
        gtk_grid_attach(GTK_GRID(pParent), m_pWidget, col, nAttach, width, height);
        g_object_unref(m_pWidget);
#else
        gtk_container_child_set(GTK_CONTAINER(pParent), m_pWidget, "top-attach", nAttach, nullptr);
#endif
    }

    virtual int get_grid_top_attach() const override
    {
        gint nAttach(0);
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_grid_query_child(GTK_GRID(pParent), m_pWidget, nullptr, &nAttach, nullptr, nullptr);
#else
        gtk_container_child_get(GTK_CONTAINER(pParent), m_pWidget, "top-attach", &nAttach, nullptr);
#endif
        return nAttach;
    }

    virtual void set_hexpand(bool bExpand) override
    {
        gtk_widget_set_hexpand(m_pWidget, bExpand);
    }

    virtual bool get_hexpand() const override
    {
        return gtk_widget_get_hexpand(m_pWidget);
    }

    virtual void set_vexpand(bool bExpand) override
    {
        gtk_widget_set_vexpand(m_pWidget, bExpand);
    }

    virtual bool get_vexpand() const override
    {
        return gtk_widget_get_vexpand(m_pWidget);
    }

    virtual void set_margin_top(int nMargin) override
    {
        gtk_widget_set_margin_top(m_pWidget, nMargin);
    }

    virtual void set_margin_bottom(int nMargin) override
    {
        gtk_widget_set_margin_bottom(m_pWidget, nMargin);
    }

    virtual void set_margin_start(int nMargin) override
    {
        gtk_widget_set_margin_start(m_pWidget, nMargin);
    }

    virtual void set_margin_end(int nMargin) override
    {
        gtk_widget_set_margin_end(m_pWidget, nMargin);
    }

    virtual int get_margin_top() const override
    {
        return gtk_widget_get_margin_top(m_pWidget);
    }

    virtual int get_margin_bottom() const override
    {
        return gtk_widget_get_margin_bottom(m_pWidget);
    }

    virtual int get_margin_start() const override
    {
        return gtk_widget_get_margin_start(m_pWidget);
    }

    virtual int get_margin_end() const override
    {
        return gtk_widget_get_margin_end(m_pWidget);
    }

    virtual void set_accessible_name(const OUString& rName) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        atk_object_set_name(pAtkObject, OUStringToOString(rName, RTL_TEXTENCODING_UTF8).getStr());
#else
        (void)rName;
#endif
    }

    virtual void set_accessible_description(const OUString& rDescription) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        atk_object_set_description(pAtkObject, OUStringToOString(rDescription, RTL_TEXTENCODING_UTF8).getStr());
#else
        (void)rDescription;
#endif
    }

    virtual OUString get_accessible_name() const override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_name(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
#else
        return OUString();
#endif
    }

    virtual OUString get_accessible_description() const override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
#else
        return OUString();
#endif
    }

    virtual void set_accessible_relation_labeled_by(weld::Widget* pLabel) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        AtkObject *pAtkLabel = pLabel ? gtk_widget_get_accessible(dynamic_cast<GtkInstanceWidget&>(*pLabel).getWidget()) : nullptr;
        AtkRelationSet *pRelationSet = atk_object_ref_relation_set(pAtkObject);
        AtkRelation *pRelation = atk_relation_set_get_relation_by_type(pRelationSet, ATK_RELATION_LABELLED_BY);
        if (pRelation)
            atk_relation_set_remove(pRelationSet, pRelation);
        if (pAtkLabel)
        {
            AtkObject *obj_array[1];
            obj_array[0] = pAtkLabel;
            pRelation = atk_relation_new(obj_array, 1, ATK_RELATION_LABELLED_BY);
            atk_relation_set_add(pRelationSet, pRelation);
        }
        g_object_unref(pRelationSet);
#else
        (void)pLabel;
#endif
    }

    virtual void set_accessible_relation_label_for(weld::Widget* pLabeled) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        AtkObject *pAtkLabeled = pLabeled ? gtk_widget_get_accessible(dynamic_cast<GtkInstanceWidget&>(*pLabeled).getWidget()) : nullptr;
        AtkRelationSet *pRelationSet = atk_object_ref_relation_set(pAtkObject);
        AtkRelation *pRelation = atk_relation_set_get_relation_by_type(pRelationSet, ATK_RELATION_LABEL_FOR);
        if (pRelation)
            atk_relation_set_remove(pRelationSet, pRelation);
        if (pAtkLabeled)
        {
            AtkObject *obj_array[1];
            obj_array[0] = pAtkLabeled;
            pRelation = atk_relation_new(obj_array, 1, ATK_RELATION_LABEL_FOR);
            atk_relation_set_add(pRelationSet, pRelation);
        }
        g_object_unref(pRelationSet);
#else
        (void)pLabeled;
#endif
    }

    virtual bool get_extents_relative_to(const weld::Widget& rRelative, int& x, int &y, int& width, int &height) const override
    {
        //for toplevel windows this is sadly futile under wayland, so we can't tell where a dialog is in order to allow
        //the document underneath to auto-scroll to place content in a visible location
        gtk_coord fX(0.0), fY(0.0);
        bool ret = gtk_widget_translate_coordinates(m_pWidget,
                                                    dynamic_cast<const GtkInstanceWidget&>(rRelative).getWidget(),
                                                    0, 0, &fX, &fY);
        x = fX;
        y = fY;
        width = gtk_widget_get_allocated_width(m_pWidget);
        height = gtk_widget_get_allocated_height(m_pWidget);
        return ret;
    }

    virtual void set_tooltip_text(const OUString& rTip) override
    {
        gtk_widget_set_tooltip_text(m_pWidget, OUStringToOString(rTip, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_tooltip_text() const override
    {
        const gchar* pStr = gtk_widget_get_tooltip_text(m_pWidget);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual std::unique_ptr<weld::Container> weld_parent() const override;

    virtual OString get_buildable_name() const override
    {
        return ::get_buildable_id(GTK_BUILDABLE(m_pWidget));
    }

    virtual void set_buildable_name(const OString& rId) override
    {
        ::set_buildable_id(GTK_BUILDABLE(m_pWidget), rId);
    }

    virtual void set_help_id(const OString& rHelpId) override
    {
        ::set_help_id(m_pWidget, rHelpId);
    }

    virtual OString get_help_id() const override
    {
        OString sRet = ::get_help_id(m_pWidget);
        if (sRet.isEmpty())
            sRet = OString("null");
        return sRet;
    }

    GtkWidget* getWidget() const
    {
        return m_pWidget;
    }

    GtkWindow* getWindow()
    {
        return GTK_WINDOW(widget_get_toplevel(m_pWidget));
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    GtkEventController* get_focus_controller()
    {
        if (!m_pFocusController)
        {
            gtk_widget_set_focusable(m_pWidget, true);
            m_pFocusController = gtk_event_controller_focus_new();
            gtk_widget_add_controller(m_pWidget, m_pFocusController);
        }
        return m_pFocusController;
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    GtkEventController* get_click_controller()
    {
        if (!m_pClickController)
        {
            GtkGesture *pClick = gtk_gesture_click_new();
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(pClick), 0);
            m_pClickController = GTK_EVENT_CONTROLLER(pClick);
            gtk_widget_add_controller(m_pWidget, m_pClickController);
        }
        return m_pClickController;
    }

    GtkEventController* get_motion_controller()
    {
        if (!m_pMotionController)
        {
            m_pMotionController = gtk_event_controller_motion_new();
            gtk_widget_add_controller(m_pWidget, m_pMotionController);
        }
        return m_pMotionController;
    }

    GtkEventController* get_drag_controller()
    {
        if (!m_pDragController)
        {
            GtkDragSource* pDrag = gtk_drag_source_new();
            m_pDragController = GTK_EVENT_CONTROLLER(pDrag);
            gtk_widget_add_controller(m_pWidget, m_pDragController);
        }
        return m_pDragController;
    }

    GtkEventController* get_key_controller()
    {
        if (!m_pKeyController)
        {
            m_pKeyController = gtk_event_controller_key_new();
            gtk_widget_add_controller(m_pWidget, m_pKeyController);
        }
        return m_pKeyController;
    }

#endif


#endif

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        if (!m_nFocusInSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            m_nFocusInSignalId = g_signal_connect(get_focus_controller(), "enter", G_CALLBACK(signalFocusIn), this);
#else
            m_nFocusInSignalId = g_signal_connect(m_pWidget, "focus-in-event", G_CALLBACK(signalFocusIn), this);
#endif
        }

        weld::Widget::connect_focus_in(rLink);
    }

    virtual void connect_mnemonic_activate(const Link<Widget&, bool>& rLink) override
    {
        if (!m_nMnemonicActivateSignalId)
            m_nMnemonicActivateSignalId = g_signal_connect(m_pWidget, "mnemonic-activate", G_CALLBACK(signalMnemonicActivate), this);
        weld::Widget::connect_mnemonic_activate(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
        if (!m_nFocusOutSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            m_nFocusOutSignalId = g_signal_connect(get_focus_controller(), "leave", G_CALLBACK(signalFocusOut), this);
#else
            m_nFocusOutSignalId = g_signal_connect(m_pWidget, "focus-out-event", G_CALLBACK(signalFocusOut), this);
#endif
        }
        weld::Widget::connect_focus_out(rLink);
    }

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override
    {
        m_nSizeAllocateSignalId = g_signal_connect(m_pWidget, "size-allocate", G_CALLBACK(signalSizeAllocate), this);
        weld::Widget::connect_size_allocate(rLink);
    }

    virtual void signal_size_allocate(guint nWidth, guint nHeight)
    {
        m_aSizeAllocateHdl.Call(Size(nWidth, nHeight));
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    bool signal_key_press(guint keyval, guint keycode, GdkModifierType state)
    {
        if (m_aKeyPressHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            return m_aKeyPressHdl.Call(CreateKeyEvent(keyval, keycode, state, 0));
        }
        return false;
    }

    bool signal_key_release(guint keyval, guint keycode, GdkModifierType state)
    {
        if (m_aKeyReleaseHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            return m_aKeyReleaseHdl.Call(CreateKeyEvent(keyval, keycode, state, 0));
        }
        return false;
    }
#else
    bool signal_key_press(const GdkEventKey* pEvent)
    {
        if (m_aKeyPressHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            return m_aKeyPressHdl.Call(GtkToVcl(*pEvent));
        }
        return false;
    }

    bool signal_key_release(const GdkEventKey* pEvent)
    {
        if (m_aKeyReleaseHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            return m_aKeyReleaseHdl.Call(GtkToVcl(*pEvent));
        }
        return false;
    }
#endif

    virtual void grab_add() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        ++m_nGrabCount;
#else
        gtk_grab_add(m_pWidget);
#endif
    }

    virtual bool has_grab() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return m_nGrabCount != 0;
#else
        return gtk_widget_has_grab(m_pWidget);
#endif
    }

    virtual void grab_remove() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        --m_nGrabCount;
#else
        gtk_grab_remove(m_pWidget);
#endif
    }

    virtual bool get_direction() const override
    {
        return gtk_widget_get_direction(m_pWidget) == GTK_TEXT_DIR_RTL;
    }

    virtual void set_direction(bool bRTL) override
    {
        gtk_widget_set_direction(m_pWidget, bRTL ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR);
    }

    virtual void freeze() override
    {
        ++m_nFreezeCount;
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_freeze_child_notify(m_pWidget);
#endif
        g_object_freeze_notify(G_OBJECT(m_pWidget));
    }

    virtual void thaw() override
    {
        --m_nFreezeCount;
        g_object_thaw_notify(G_OBJECT(m_pWidget));
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_thaw_child_notify(m_pWidget);
#endif
    }

    virtual void set_busy_cursor(bool bBusy) override
    {
        if (bBusy)
            ++m_nWaitCount;
        else
            --m_nWaitCount;
        if (m_nWaitCount == 1)
            set_cursor(m_pWidget, "progress");
        else if (m_nWaitCount == 0)
            set_cursor(m_pWidget, nullptr);
        assert (m_nWaitCount >= 0);
    }

    virtual void queue_resize() override
    {
        gtk_widget_queue_resize(m_pWidget);
    }

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override
    {
        if (!m_xDropTarget)
        {
            m_xDropTarget.set(new GtkInstDropTarget);
#if !GTK_CHECK_VERSION(4, 0, 0)
            if (!gtk_drag_dest_get_track_motion(m_pWidget))
            {
                gtk_drag_dest_set(m_pWidget, GtkDestDefaults(0), nullptr, 0, GdkDragAction(0));
                gtk_drag_dest_set_track_motion(m_pWidget, true);
            }
            m_nDragMotionSignalId = g_signal_connect(m_pWidget, "drag-motion", G_CALLBACK(signalDragMotion), this);
            m_nDragDropSignalId = g_signal_connect(m_pWidget, "drag-drop", G_CALLBACK(signalDragDrop), this);
            m_nDragDropReceivedSignalId = g_signal_connect(m_pWidget, "drag-data-received", G_CALLBACK(signalDragDropReceived), this);
            m_nDragLeaveSignalId = g_signal_connect(m_pWidget, "drag-leave", G_CALLBACK(signalDragLeave), this);
#endif
        }
        return m_xDropTarget;
    }

    virtual css::uno::Reference<css::datatransfer::clipboard::XClipboard> get_clipboard() const override
    {
        // the gen backend can have per-frame clipboards which is (presumably) useful for LibreOffice Online
        // but normal usage is the shared system clipboard
        return GetSystemClipboard();
    }

    virtual void connect_get_property_tree(const Link<tools::JsonWriter&, void>& /*rLink*/) override
    {
        //not implemented for the gtk variant
    }

    virtual void get_property_tree(tools::JsonWriter& /*rJsonWriter*/) override
    {
        //not implemented for the gtk variant
    }

    virtual void set_stack_background() override
    {
        do_set_background(Application::GetSettings().GetStyleSettings().GetWindowColor());
    }

    virtual void set_title_background() override
    {
        do_set_background(Application::GetSettings().GetStyleSettings().GetShadowColor());
    }

    virtual void set_highlight_background() override
    {
        do_set_background(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    }

    virtual void set_background(const Color& rColor) override
    {
        do_set_background(rColor);
    }

    virtual void set_toolbar_background() override
    {
        // no-op
    }

    virtual ~GtkInstanceWidget() override
    {
        if (m_pDragCancelEvent)
            Application::RemoveUserEvent(m_pDragCancelEvent);
        if (m_nDragMotionSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragMotionSignalId);
        if (m_nDragDropSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragDropSignalId);
        if (m_nDragDropReceivedSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragDropReceivedSignalId);
        if (m_nDragLeaveSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragLeaveSignalId);
        if (m_nDragEndSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_drag_controller(), m_nDragEndSignalId);
#else
            g_signal_handler_disconnect(m_pWidget, m_nDragEndSignalId);
#endif
        }
        if (m_nDragBeginSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_drag_controller(), m_nDragBeginSignalId);
#else
            g_signal_handler_disconnect(m_pWidget, m_nDragBeginSignalId);
#endif
        }
        if (m_nDragFailedSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragFailedSignalId);
        if (m_nDragDataDeleteignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragDataDeleteignalId);
        if (m_nDragGetSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragGetSignalId);
        if (m_nKeyPressSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_key_controller(), m_nKeyPressSignalId);
#else
            g_signal_handler_disconnect(m_pWidget, m_nKeyPressSignalId);
#endif
        }
        if (m_nKeyReleaseSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_key_controller(), m_nKeyReleaseSignalId);
#else
            g_signal_handler_disconnect(m_pWidget, m_nKeyReleaseSignalId);
#endif
        }

        if (m_nFocusInSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_focus_controller(), m_nFocusInSignalId);
#else
            g_signal_handler_disconnect(m_pWidget, m_nFocusInSignalId);
#endif
        }
        if (m_nMnemonicActivateSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nMnemonicActivateSignalId);
        if (m_nFocusOutSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_disconnect(get_focus_controller(), m_nFocusOutSignalId);
#else
            g_signal_handler_disconnect(m_pWidget, m_nFocusOutSignalId);
#endif
        }
        if (m_nSizeAllocateSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nSizeAllocateSignalId);

        do_set_background(COL_AUTO);

        DisconnectMouseEvents();

        if (m_bTakeOwnership)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            gtk_widget_destroy(m_pWidget);
#else
            gtk_window_destroy(GTK_WINDOW(m_pWidget));
#endif
        }
        else
            g_object_unref(m_pWidget);
    }

    virtual void disable_notify_events()
    {
        if (m_nFocusInSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_block(get_focus_controller(), m_nFocusInSignalId);
#else
            g_signal_handler_block(m_pWidget, m_nFocusInSignalId);
#endif
        }
        if (m_nMnemonicActivateSignalId)
            g_signal_handler_block(m_pWidget, m_nMnemonicActivateSignalId);
        if (m_nFocusOutSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_block(get_focus_controller(), m_nFocusOutSignalId);
#else
            g_signal_handler_block(m_pWidget, m_nFocusOutSignalId);
#endif
        }
        if (m_nSizeAllocateSignalId)
            g_signal_handler_block(m_pWidget, m_nSizeAllocateSignalId);
    }

    virtual void enable_notify_events()
    {
        if (m_nSizeAllocateSignalId)
            g_signal_handler_unblock(m_pWidget, m_nSizeAllocateSignalId);
        if (m_nFocusOutSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_unblock(get_focus_controller(), m_nFocusOutSignalId);
#else
            g_signal_handler_unblock(m_pWidget, m_nFocusOutSignalId);
#endif
        }
        if (m_nMnemonicActivateSignalId)
            g_signal_handler_unblock(m_pWidget, m_nMnemonicActivateSignalId);

        if (m_nFocusInSignalId)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            g_signal_handler_unblock(get_focus_controller(), m_nFocusInSignalId);
#else
            g_signal_handler_unblock(m_pWidget, m_nFocusInSignalId);
#endif
        }
    }

    virtual void help_hierarchy_foreach(const std::function<bool(const OString&)>& func) override;

    virtual OUString strip_mnemonic(const OUString &rLabel) const override
    {
        return rLabel.replaceFirst("_", "");
    }

    virtual VclPtr<VirtualDevice> create_virtual_device() const override
    {
        // create with no separate alpha layer like everything sane does
        auto xRet = VclPtr<VirtualDevice>::Create();
        xRet->SetBackground(COL_TRANSPARENT);
        return xRet;
    }

    virtual void draw(OutputDevice& rOutput, const Point& rPos, const Size& rPixelSize) override
    {
        // detect if we have to manually setup its size
        bool bAlreadyRealized = gtk_widget_get_realized(m_pWidget);
        // has to be visible for draw to work
        bool bAlreadyVisible = gtk_widget_get_visible(m_pWidget);
        // has to be mapped for draw to work
        bool bAlreadyMapped = gtk_widget_get_mapped(m_pWidget);

        if (!bAlreadyRealized)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            /*
               tdf#141633 The "sample db" example (Mockup.odb) has multiline
               entries used in its "Journal Entry" column. Those are painted by
               taking snapshots of a never-really-shown textview widget.
               Without this style_updated then the textview is always drawn
               using its original default font size and changing the page zoom
               has no effect on the size of text in the "Journal Entry" column.
            */
            update_style(m_pWidget, nullptr);
#endif
            gtk_widget_realize(m_pWidget);
        }
        if (!bAlreadyVisible)
            gtk_widget_show(m_pWidget);
        if (!bAlreadyMapped)
            gtk_widget_map(m_pWidget);

        assert(gtk_widget_is_drawable(m_pWidget)); // all that should result in this holding

        // turn off animations, otherwise we get a frame of an animation sequence
        gboolean bAnimations;
        GtkSettings* pSettings = gtk_widget_get_settings(m_pWidget);
        g_object_get(pSettings, "gtk-enable-animations", &bAnimations, nullptr);
        if (bAnimations)
            g_object_set(pSettings, "gtk-enable-animations", false, nullptr);

        Size aSize(rPixelSize);

        GtkAllocation aOrigAllocation;
        gtk_widget_get_allocation(m_pWidget, &aOrigAllocation);

        GtkAllocation aNewAllocation {aOrigAllocation.x,
                                      aOrigAllocation.y,
                                      static_cast<int>(aSize.Width()),
                                      static_cast<int>(aSize.Height()) };
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_size_allocate(m_pWidget, &aNewAllocation);
#else
        gtk_widget_size_allocate(m_pWidget, &aNewAllocation, 0);
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_CONTAINER(m_pWidget))
            gtk_container_resize_children(GTK_CONTAINER(m_pWidget));
#endif

        VclPtr<VirtualDevice> xOutput(VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT));
        xOutput->SetOutputSizePixel(aSize);

        switch (rOutput.GetOutDevType())
        {
            case OUTDEV_WINDOW:
            case OUTDEV_VIRDEV:
                xOutput->DrawOutDev(Point(), aSize, rPos, aSize, rOutput);
                break;
            case OUTDEV_PRINTER:
            case OUTDEV_PDF:
                xOutput->SetBackground(rOutput.GetBackground());
                xOutput->Erase();
                break;
        }

        cairo_surface_t* pSurface = get_underlying_cairo_surface(*xOutput);
        cairo_t* cr = cairo_create(pSurface);

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_draw(m_pWidget, cr);
#else
        GtkSnapshot* pSnapshot = gtk_snapshot_new();
        GtkWidgetClass* pWidgetClass = GTK_WIDGET_GET_CLASS(m_pWidget);
        pWidgetClass->snapshot(m_pWidget, pSnapshot);
        GskRenderNode* pNode = gtk_snapshot_free_to_node(pSnapshot);
        gsk_render_node_draw(pNode, cr);
        gsk_render_node_unref(pNode);
#endif

        cairo_destroy(cr);

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_set_allocation(m_pWidget, &aOrigAllocation);
        gtk_widget_size_allocate(m_pWidget, &aOrigAllocation);
#else
        gtk_widget_size_allocate(m_pWidget, &aOrigAllocation, 0);
#endif

        switch (rOutput.GetOutDevType())
        {
            case OUTDEV_WINDOW:
            case OUTDEV_VIRDEV:
                rOutput.DrawOutDev(rPos, aSize, Point(), aSize, *xOutput);
                break;
            case OUTDEV_PRINTER:
            case OUTDEV_PDF:
                rOutput.DrawBitmapEx(rPos, xOutput->GetBitmapEx(Point(), aSize));
                break;
        }

        if (bAnimations)
            g_object_set(pSettings, "gtk-enable-animations", true, nullptr);

        if (!bAlreadyMapped)
            gtk_widget_unmap(m_pWidget);
        if (!bAlreadyVisible)
            gtk_widget_hide(m_pWidget);
        if (!bAlreadyRealized)
            gtk_widget_unrealize(m_pWidget);
    }
};

}

#if !GTK_CHECK_VERSION(4, 0, 0)
IMPL_LINK(GtkInstanceWidget, async_drag_cancel, void*, arg, void)
{
    m_pDragCancelEvent = nullptr;
    GdkDragContext* context = static_cast<GdkDragContext*>(arg);

    // tdf#132477 simply calling gtk_drag_cancel on the treeview dnd under X
    // doesn't seem to work as hoped for (though under wayland all is well).
    // Under X the next (allowed) drag effort doesn't work to drop anything,
    // but a then repeated attempt does.
    // emitting cancel to get gtk to cancel the drag for us does work as hoped for.
    g_signal_emit_by_name(context, "cancel", 0, GDK_DRAG_CANCEL_USER_CANCELLED);

    g_object_unref(context);
}
#endif

namespace
{
    OString MapToGtkAccelerator(const OUString &rStr)
    {
        return OUStringToOString(rStr.replaceFirst("~", "_"), RTL_TEXTENCODING_UTF8);
    }

    OUString get_label(GtkLabel* pLabel)
    {
        const gchar* pStr = gtk_label_get_label(pLabel);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_label(GtkLabel* pLabel, const OUString& rText)
    {
        gtk_label_set_label(pLabel, MapToGtkAccelerator(rText).getStr());
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    GtkWidget* find_label_widget(GtkWidget* pContainer)
    {
        GtkWidget* pLabel = nullptr;
        for (GtkWidget* pChild = gtk_widget_get_first_child(pContainer);
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            if (GTK_IS_LABEL(pChild))
            {
                pLabel = pChild;
                break;
            }
            else
            {
                pLabel = find_label_widget(pChild);
                if (pLabel)
                    break;
            }
        }
        return pLabel;
    }

    GtkWidget* find_image_widget(GtkWidget* pContainer)
    {
        GtkWidget* pImage = nullptr;
        for (GtkWidget* pChild = gtk_widget_get_first_child(pContainer);
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            if (GTK_IS_IMAGE(pChild))
            {
                pImage = pChild;
                break;
            }
            else
            {
                pImage = find_image_widget(pChild);
                if (pImage)
                    break;
            }
        }
        return pImage;
    }
#else
    GtkWidget* find_label_widget(GtkContainer* pContainer)
    {
        GList* pChildren = gtk_container_get_children(pContainer);

        GtkWidget* pChild = nullptr;
        for (GList* pCandidate = pChildren; pCandidate; pCandidate = pCandidate->next)
        {
            if (GTK_IS_LABEL(pCandidate->data))
            {
                pChild = GTK_WIDGET(pCandidate->data);
                break;
            }
            else if (GTK_IS_CONTAINER(pCandidate->data))
            {
                pChild = find_label_widget(GTK_CONTAINER(pCandidate->data));
                if (pChild)
                    break;
            }
        }
        g_list_free(pChildren);

        return pChild;
    }

    GtkWidget* find_image_widget(GtkContainer* pContainer)
    {
        GList* pChildren = gtk_container_get_children(pContainer);

        GtkWidget* pChild = nullptr;
        for (GList* pCandidate = pChildren; pCandidate; pCandidate = pCandidate->next)
        {
            if (GTK_IS_IMAGE(pCandidate->data))
            {
                pChild = GTK_WIDGET(pCandidate->data);
                break;
            }
            else if (GTK_IS_CONTAINER(pCandidate->data))
            {
                pChild = find_image_widget(GTK_CONTAINER(pCandidate->data));
                if (pChild)
                    break;
            }
        }
        g_list_free(pChildren);

        return pChild;
    }
#endif

    GtkLabel* get_label_widget(GtkWidget* pButton)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(pButton));

        if (GTK_IS_CONTAINER(pChild))
            pChild = find_label_widget(GTK_CONTAINER(pChild));
        else if (!GTK_IS_LABEL(pChild))
            pChild = nullptr;

        return GTK_LABEL(pChild);
#else
        return GTK_LABEL(find_label_widget(pButton));
#endif
    }

    GtkImage* get_image_widget(GtkWidget *pButton)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(pButton));

        if (GTK_IS_CONTAINER(pChild))
            pChild = find_image_widget(GTK_CONTAINER(pChild));
        else if (!GTK_IS_IMAGE(pChild))
            pChild = nullptr;

        return GTK_IMAGE(pChild);
#else
        return GTK_IMAGE(find_image_widget(pButton));
#endif
    }

    OUString button_get_label(GtkButton* pButton)
    {
        if (GtkLabel* pLabel = get_label_widget(GTK_WIDGET(pButton)))
            return ::get_label(pLabel);
        const gchar* pStr = gtk_button_get_label(pButton);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    void button_set_label(GtkButton* pButton, const OUString& rText)
    {
        if (GtkLabel* pLabel = get_label_widget(GTK_WIDGET(pButton)))
        {
            ::set_label(pLabel, rText);
            gtk_widget_set_visible(GTK_WIDGET(pLabel), true);
            return;
        }
        gtk_button_set_label(pButton, MapToGtkAccelerator(rText).getStr());
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    OUString get_label(GtkCheckButton* pButton)
    {
        const gchar* pStr = gtk_check_button_get_label(pButton);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_label(GtkCheckButton* pButton, const OUString& rText)
    {
        gtk_check_button_set_label(pButton, MapToGtkAccelerator(rText).getStr());
    }
#endif

    OUString get_title(GtkWindow* pWindow)
    {
        const gchar* pStr = gtk_window_get_title(pWindow);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_title(GtkWindow* pWindow, std::u16string_view rTitle)
    {
        gtk_window_set_title(pWindow, OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8).getStr());
    }

    OUString get_primary_text(GtkMessageDialog* pMessageDialog)
    {
        gchar* pText = nullptr;
        g_object_get(G_OBJECT(pMessageDialog), "text", &pText, nullptr);
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_primary_text(GtkMessageDialog* pMessageDialog, std::u16string_view rText)
    {
        g_object_set(G_OBJECT(pMessageDialog), "text",
            OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
            nullptr);
    }

    void set_secondary_text(GtkMessageDialog* pMessageDialog, std::u16string_view rText)
    {
        g_object_set(G_OBJECT(pMessageDialog), "secondary-text",
                OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
                nullptr);
    }

    OUString get_secondary_text(GtkMessageDialog* pMessageDialog)
    {
        gchar* pText = nullptr;
        g_object_get(G_OBJECT(pMessageDialog), "secondary-text", &pText, nullptr);
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }
}

namespace
{
    GdkPixbuf* load_icon_from_stream(SvMemoryStream& rStream)
    {
        auto nLength = rStream.TellEnd();
        if (!nLength)
            return nullptr;
        const guchar* pData = static_cast<const guchar*>(rStream.GetData());
        assert((*pData == 137 || *pData == '<') && "if we want to support more than png or svg this function must change");
        // if we know the image type, it's a little faster to hand the type over and skip the type detection.
        GdkPixbufLoader *pixbuf_loader = gdk_pixbuf_loader_new_with_type(*pData == 137 ? "png" : "svg", nullptr);
        gdk_pixbuf_loader_write(pixbuf_loader, pData, nLength, nullptr);
        gdk_pixbuf_loader_close(pixbuf_loader, nullptr);
        GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);
        if (pixbuf)
            g_object_ref(pixbuf);
        g_object_unref(pixbuf_loader);
        return pixbuf;
    }

    GdkPixbuf* load_icon_by_name_theme_lang(const OUString& rIconName, const OUString& rIconTheme, const OUString& rUILang)
    {
        auto xMemStm = ImageTree::get().getImageStream(rIconName, rIconTheme, rUILang);
        if (!xMemStm)
            return nullptr;
        return load_icon_from_stream(*xMemStm);
    }
}

GdkPixbuf* load_icon_by_name(const OUString& rIconName)
{
    OUString sIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    OUString sUILang = Application::GetSettings().GetUILanguageTag().getBcp47();
    return load_icon_by_name_theme_lang(rIconName, sIconTheme, sUILang);
}

namespace
{
    GdkPixbuf* getPixbuf(const css::uno::Reference<css::graphic::XGraphic>& rImage)
    {
        Image aImage(rImage);

        OUString sStock(aImage.GetStock());
        if (!sStock.isEmpty())
            return load_icon_by_name(sStock);

        SvMemoryStream aMemStm;

        // We "know" that this gets passed to zlib's deflateInit2_(). 1 means best speed.
        css::uno::Sequence<css::beans::PropertyValue> aFilterData{ comphelper::makePropertyValue(
            "Compression", sal_Int32(1)) };

        vcl::PNGWriter aWriter(aImage.GetBitmapEx(), &aFilterData);
        aWriter.Write(aMemStm);

        return load_icon_from_stream(aMemStm);
    }

    GdkPixbuf* getPixbuf(const VirtualDevice& rDevice)
    {
        Size aSize(rDevice.GetOutputSizePixel());
        cairo_surface_t* orig_surface = get_underlying_cairo_surface(rDevice);
        double m_fXScale, m_fYScale;
        dl_cairo_surface_get_device_scale(orig_surface, &m_fXScale, &m_fYScale);

        cairo_surface_t* surface;
        if (m_fXScale != 1.0 || m_fYScale != -1)
        {
            surface = cairo_surface_create_similar_image(orig_surface,
                                                         CAIRO_FORMAT_ARGB32,
                                                         aSize.Width(),
                                                         aSize.Height());
            cairo_t* cr = cairo_create(surface);
            cairo_set_source_surface(cr, orig_surface, 0, 0);
            cairo_paint(cr);
            cairo_destroy(cr);
        }
        else
            surface = orig_surface;

        GdkPixbuf* pRet = gdk_pixbuf_get_from_surface(surface, 0, 0, aSize.Width(), aSize.Height());

        if (surface != orig_surface)
            cairo_surface_destroy(surface);

        return pRet;
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    cairo_surface_t* render_paintable_to_surface(GdkPaintable *paintable, int nWidth, int nHeight)
    {
        cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);

        GtkSnapshot* snapshot = gtk_snapshot_new();
        gdk_paintable_snapshot(paintable, snapshot, nWidth, nHeight);
        GskRenderNode* node = gtk_snapshot_free_to_node(snapshot);

        cairo_t* cr = cairo_create(surface);
        gsk_render_node_draw(node, cr);
        cairo_destroy(cr);

        gsk_render_node_unref(node);

        return surface;
    }
#endif

    GdkPixbuf* getPixbuf(const OUString& rIconName)
    {
        if (rIconName.isEmpty())
            return nullptr;

        GdkPixbuf* pixbuf = nullptr;
        if (rIconName.lastIndexOf('.') != rIconName.getLength() - 4)
        {
            assert((rIconName== "dialog-warning" || rIconName== "dialog-error" || rIconName== "dialog-information") &&
                   "unknown stock image");

#if GTK_CHECK_VERSION(4, 0, 0)
            GtkIconTheme *icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
            GtkIconPaintable *icon = gtk_icon_theme_lookup_icon(icon_theme,
                                     OUStringToOString(rIconName, RTL_TEXTENCODING_UTF8).getStr(),
                                     nullptr,
                                     16,
                                     1,
                                     AllSettings::GetLayoutRTL() ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR,
                                     static_cast<GtkIconLookupFlags>(0));
            GdkPaintable* paintable = GDK_PAINTABLE(icon);
            int nWidth = gdk_paintable_get_intrinsic_width(paintable);
            int nHeight = gdk_paintable_get_intrinsic_height(paintable);
            cairo_surface_t* surface = render_paintable_to_surface(paintable, nWidth, nHeight);
            pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, nWidth, nHeight);
            cairo_surface_destroy(surface);
#else
            GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
            GError *error = nullptr;
            pixbuf = gtk_icon_theme_load_icon(icon_theme, OUStringToOString(rIconName, RTL_TEXTENCODING_UTF8).getStr(),
                                              16, GTK_ICON_LOOKUP_USE_BUILTIN, &error);
#endif
        }
        else
        {
            const AllSettings& rSettings = Application::GetSettings();
            pixbuf = load_icon_by_name_theme_lang(rIconName,
                                       rSettings.GetStyleSettings().DetermineIconTheme(),
                                       rSettings.GetUILanguageTag().getBcp47());
        }
        return pixbuf;
    }
}

namespace
{
#if GTK_CHECK_VERSION(4, 0, 0)
    SurfacePaintable* paintable_new_from_virtual_device(const VirtualDevice& rImageSurface)
    {
        cairo_surface_t* surface = get_underlying_cairo_surface(rImageSurface);

        Size aSize(rImageSurface.GetOutputSizePixel());
        cairo_surface_t* target = cairo_surface_create_similar(surface,
                                                               cairo_surface_get_content(surface),
                                                               aSize.Width(),
                                                               aSize.Height());
        cairo_t* cr = cairo_create(target);
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);

        SurfacePaintable* pPaintable = SURFACE_PAINTABLE(g_object_new(surface_paintable_get_type(), nullptr));
        surface_paintable_set_source(pPaintable, target, aSize.Width(), aSize.Height());
        return pPaintable;
    }

    GtkWidget* image_new_from_virtual_device(const VirtualDevice& rImageSurface)
    {
        SurfacePaintable* paintable = paintable_new_from_virtual_device(rImageSurface);
        return gtk_image_new_from_paintable(GDK_PAINTABLE(paintable));
    }

    GtkWidget* picture_new_from_virtual_device(const VirtualDevice& rImageSurface)
    {
        SurfacePaintable* paintable = paintable_new_from_virtual_device(rImageSurface);
        return gtk_picture_new_for_paintable(GDK_PAINTABLE(paintable));
    }

#else
    GtkWidget* image_new_from_virtual_device(const VirtualDevice& rImageSurface)
    {
        GtkWidget* pImage = nullptr;
        cairo_surface_t* surface = get_underlying_cairo_surface(rImageSurface);

        Size aSize(rImageSurface.GetOutputSizePixel());
        cairo_surface_t* target = cairo_surface_create_similar(surface,
                                                               cairo_surface_get_content(surface),
                                                               aSize.Width(),
                                                               aSize.Height());
        cairo_t* cr = cairo_create(target);
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);

        pImage = gtk_image_new_from_surface(target);
        cairo_surface_destroy(target);
        return pImage;
    }
#endif

    void image_set_from_icon_name(GtkImage* pImage, const OUString& rIconName)
    {
        GdkPixbuf* pixbuf = load_icon_by_name(rIconName);
        gtk_image_set_from_pixbuf(pImage, pixbuf);
        if (!pixbuf)
            return;
        g_object_unref(pixbuf);
    }

    void image_set_from_virtual_device(GtkImage* pImage, const VirtualDevice* pDevice)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_image_set_from_paintable(pImage, pDevice ? GDK_PAINTABLE(paintable_new_from_virtual_device(*pDevice)) : nullptr);
#else
        gtk_image_set_from_surface(pImage, pDevice ? get_underlying_cairo_surface(*pDevice) : nullptr);
#endif
    }

    void image_set_from_xgraphic(GtkImage* pImage, const css::uno::Reference<css::graphic::XGraphic>& rImage)
    {
        GdkPixbuf* pixbuf = getPixbuf(rImage);
        gtk_image_set_from_pixbuf(pImage, pixbuf);
        if (pixbuf)
            g_object_unref(pixbuf);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    void picture_set_from_icon_name(GtkPicture* pPicture, const OUString& rIconName)
    {
        GdkPixbuf* pixbuf = load_icon_by_name(rIconName);
        gtk_picture_set_pixbuf(pPicture, pixbuf);
        if (pixbuf)
            g_object_unref(pixbuf);
    }

    void picture_set_from_virtual_device(GtkPicture* pPicture, const VirtualDevice* pDevice)
    {
        if (!pDevice)
            gtk_picture_set_paintable(pPicture, nullptr);
        else
            gtk_picture_set_paintable(pPicture, GDK_PAINTABLE(paintable_new_from_virtual_device(*pDevice)));
    }

    void picture_set_from_xgraphic(GtkPicture* pPicture, const css::uno::Reference<css::graphic::XGraphic>& rPicture)
    {
        GdkPixbuf* pixbuf = getPixbuf(rPicture);
        gtk_picture_set_pixbuf(pPicture, pixbuf);
        if (pixbuf)
            g_object_unref(pixbuf);
    }
#endif

    void button_set_from_icon_name(GtkButton* pButton, const OUString& rIconName)
    {
        if (GtkImage* pImage = get_image_widget(GTK_WIDGET(pButton)))
        {
            ::image_set_from_icon_name(pImage, rIconName);
            gtk_widget_set_visible(GTK_WIDGET(pImage), true);
            return;
        }

        GdkPixbuf* pixbuf = load_icon_by_name(rIconName);
        GtkWidget* pImage;
        if (!pixbuf)
            pImage = nullptr;
        else
        {
            pImage = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_button_set_child(pButton, pImage);
#else
        gtk_button_set_image(pButton, pImage);
#endif
    }

    void button_set_image(GtkButton* pButton, const VirtualDevice* pDevice)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_button_set_always_show_image(pButton, true);
        gtk_button_set_image_position(pButton, GTK_POS_LEFT);
#endif
        GtkWidget* pImage = pDevice ? image_new_from_virtual_device(*pDevice) : nullptr;
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_button_set_child(pButton, pImage);
#else
        gtk_button_set_image(pButton, pImage);
#endif
    }

    void button_set_image(GtkButton* pButton, const css::uno::Reference<css::graphic::XGraphic>& rImage)
    {
        if (GtkImage* pImage = get_image_widget(GTK_WIDGET(pButton)))
        {
            ::image_set_from_xgraphic(pImage, rImage);
            gtk_widget_set_visible(GTK_WIDGET(pImage), true);
            return;
        }

        GdkPixbuf* pixbuf = getPixbuf(rImage);
        GtkWidget* pImage;
        if (!pixbuf)
            pImage = nullptr;
        else
        {
            pImage = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_button_set_child(pButton, pImage);
#else
        gtk_button_set_image(pButton, pImage);
#endif
    }


class MenuHelper
{
protected:
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkMenu* m_pMenu;

    std::map<OString, GtkMenuItem*> m_aMap;
#else
    GtkPopoverMenu* m_pMenu;

    o3tl::sorted_vector<OString> m_aInsertedActions; // must outlive m_aActionEntries
    std::map<OString, OString> m_aIdToAction;
    std::set<OString> m_aHiddenIds;
    std::vector<GActionEntry> m_aActionEntries;
    GActionGroup* m_pActionGroup;
    // move 'invisible' entries to m_pHiddenActionGroup
    GActionGroup* m_pHiddenActionGroup;
#endif
    bool m_bTakeOwnership;
private:

    virtual void signal_item_activate(const OString& rIdent) = 0;

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void collect(GtkWidget* pItem, gpointer widget)
    {
        GtkMenuItem* pMenuItem = GTK_MENU_ITEM(pItem);
        if (GtkWidget* pSubMenu = gtk_menu_item_get_submenu(pMenuItem))
            gtk_container_foreach(GTK_CONTAINER(pSubMenu), collect, widget);
        MenuHelper* pThis = static_cast<MenuHelper*>(widget);
        pThis->add_to_map(pMenuItem);
    }

    static void signalActivate(GtkMenuItem* pItem, gpointer widget)
    {
        MenuHelper* pThis = static_cast<MenuHelper*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_item_activate(::get_buildable_id(GTK_BUILDABLE(pItem)));
    }
#else
    static std::pair<GMenuModel*, int> find_id(GMenuModel* pMenuModel, const OString& rId)
    {
        for (int i = 0, nCount = g_menu_model_get_n_items(pMenuModel); i < nCount; ++i)
        {
            OString sTarget;
            char *id;
            if (g_menu_model_get_item_attribute(pMenuModel, i, "target", "s", &id))
            {
                sTarget = OString(id);
                g_free(id);
            }

            if (sTarget == rId)
                return std::make_pair(pMenuModel, i);

            if (GMenuModel* pSectionModel = g_menu_model_get_item_link(pMenuModel, i, G_MENU_LINK_SECTION))
            {
                std::pair<GMenuModel*, int> aRet = find_id(pSectionModel, rId);
                if (aRet.first)
                    return aRet;
            }
            if (GMenuModel* pSubMenuModel = g_menu_model_get_item_link(pMenuModel, i, G_MENU_LINK_SUBMENU))
            {
                std::pair<GMenuModel*, int> aRet = find_id(pSubMenuModel, rId);
                if (aRet.first)
                    return aRet;
            }
        }

        return std::make_pair(nullptr, -1);
    }

    void clear_actions()
    {
        for (const auto& rAction : m_aActionEntries)
        {
            g_action_map_remove_action(G_ACTION_MAP(m_pActionGroup), rAction.name);
            g_action_map_remove_action(G_ACTION_MAP(m_pHiddenActionGroup), rAction.name);
        }
        m_aActionEntries.clear();
        m_aInsertedActions.clear();
        m_aIdToAction.clear();
    }

    static void action_activated(GSimpleAction*, GVariant* pParameter, gpointer widget)
    {
        gsize nLength(0);
        const gchar* pStr = g_variant_get_string(pParameter, &nLength);
        OString aStr(pStr, nLength);
        MenuHelper* pThis = static_cast<MenuHelper*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_item_activate(aStr);
    }
#endif

public:
#if !GTK_CHECK_VERSION(4, 0, 0)
    MenuHelper(GtkMenu* pMenu, bool bTakeOwnership)
#else
    MenuHelper(GtkPopoverMenu* pMenu, bool bTakeOwnership)
#endif
        : m_pMenu(pMenu)
        , m_bTakeOwnership(bTakeOwnership)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!m_pMenu)
            return;
        gtk_container_foreach(GTK_CONTAINER(m_pMenu), collect, this);
#else
        m_pActionGroup = G_ACTION_GROUP(g_simple_action_group_new());
        m_pHiddenActionGroup = G_ACTION_GROUP(g_simple_action_group_new());
#endif
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    void add_to_map(GtkMenuItem* pMenuItem)
    {
        OString id = ::get_buildable_id(GTK_BUILDABLE(pMenuItem));
        m_aMap[id] = pMenuItem;
        g_signal_connect(pMenuItem, "activate", G_CALLBACK(signalActivate), this);
    }

    void remove_from_map(GtkMenuItem* pMenuItem)
    {
        OString id = ::get_buildable_id(GTK_BUILDABLE(pMenuItem));
        auto iter = m_aMap.find(id);
        g_signal_handlers_disconnect_by_data(pMenuItem, this);
        m_aMap.erase(iter);
    }

    void disable_item_notify_events()
    {
        for (auto& a : m_aMap)
            g_signal_handlers_block_by_func(a.second, reinterpret_cast<void*>(signalActivate), this);
    }

    void enable_item_notify_events()
    {
        for (auto& a : m_aMap)
            g_signal_handlers_unblock_by_func(a.second, reinterpret_cast<void*>(signalActivate), this);
    }
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
    /* LibreOffice likes to think of separators between menu entries, while gtk likes
       to think of sections of menus with separators drawn between sections. We always
       arrange to have a section in a menu so toplevel menumodels comprise of
       sections and we move entries between sections on pretending to insert separators */
    static std::pair<GMenuModel*, int> get_section_and_pos_for(GMenuModel* pMenuModel, int pos)
    {
        int nSectionCount = g_menu_model_get_n_items(pMenuModel);
        assert(nSectionCount);

        GMenuModel* pSectionModel = nullptr;
        int nIndexWithinSection = 0;

        int nExternalPos = 0;
        for (int nSection = 0; nSection < nSectionCount; ++nSection)
        {
            pSectionModel = g_menu_model_get_item_link(pMenuModel, nSection, G_MENU_LINK_SECTION);
            assert(pSectionModel);
            int nCount = g_menu_model_get_n_items(pSectionModel);
            for (nIndexWithinSection = 0; nIndexWithinSection < nCount; ++nIndexWithinSection)
            {
                if (pos == nExternalPos)
                    break;
                ++nExternalPos;
            }
            ++nExternalPos;
        }

        return std::make_pair(pSectionModel, nIndexWithinSection);
    }

    static int count_immediate_children(GMenuModel* pMenuModel)
    {
        int nSectionCount = g_menu_model_get_n_items(pMenuModel);
        assert(nSectionCount);

        int nExternalPos = 0;
        for (int nSection = 0; nSection < nSectionCount; ++nSection)
        {
            GMenuModel* pSectionModel = g_menu_model_get_item_link(pMenuModel, nSection, G_MENU_LINK_SECTION);
            assert(pSectionModel);
            int nCount = g_menu_model_get_n_items(pSectionModel);
            for (int nIndexWithinSection = 0; nIndexWithinSection < nCount; ++nIndexWithinSection)
            {
                ++nExternalPos;
            }
            ++nExternalPos;
        }

        return nExternalPos - 1;
    }
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
    void process_menu_model(GMenuModel* pMenuModel)
    {
        for (int i = 0, nCount = g_menu_model_get_n_items(pMenuModel); i < nCount; ++i)
        {
            OString sAction, sTarget;
            char *id;
            if (g_menu_model_get_item_attribute(pMenuModel, i, "action", "s", &id))
            {
                assert(OString(id).startsWith("menu."));

                sAction = OString(id + 5);

                auto res = m_aInsertedActions.insert(sAction);
                if (res.second)
                {
                    // the const char* arg isn't copied by anything so it must continue to exist for the life time of
                    // the action group
                    if (sAction.startsWith("radio."))
                        m_aActionEntries.push_back({res.first->getStr(), action_activated, "s", "'none'", nullptr, {}});
                    else
                        m_aActionEntries.push_back({res.first->getStr(), action_activated, "s", nullptr, nullptr, {}});
                }

                g_free(id);
            }

            if (g_menu_model_get_item_attribute(pMenuModel, i, "target", "s", &id))
            {
                sTarget = OString(id);
                g_free(id);
            }

            m_aIdToAction[sTarget] = sAction;

            if (GMenuModel* pSectionModel = g_menu_model_get_item_link(pMenuModel, i, G_MENU_LINK_SECTION))
                process_menu_model(pSectionModel);
            if (GMenuModel* pSubMenuModel = g_menu_model_get_item_link(pMenuModel, i, G_MENU_LINK_SUBMENU))
                process_menu_model(pSubMenuModel);
        }
    }

    // build an action group for the menu, "action" is the normal menu entry case
    // the others are radiogroups
    void update_action_group_from_popover_model()
    {
        clear_actions();

        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            process_menu_model(pMenuModel);
        }

        // move hidden entries to m_pHiddenActionGroup
        g_action_map_add_action_entries(G_ACTION_MAP(m_pActionGroup), m_aActionEntries.data(), m_aActionEntries.size(), this);
        for (const auto& id : m_aHiddenIds)
        {
            GAction* pAction = g_action_map_lookup_action(G_ACTION_MAP(m_pActionGroup), m_aIdToAction[id].getStr());
            g_action_map_add_action(G_ACTION_MAP(m_pHiddenActionGroup), pAction);
            g_action_map_remove_action(G_ACTION_MAP(m_pActionGroup), m_aIdToAction[id].getStr());
        }
    }
#endif

    void insert_item(int pos, const OUString& rId, const OUString& rStr,
                     const OUString* pIconName, const VirtualDevice* pImageSurface,
                     TriState eCheckRadioFalse)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pImage = nullptr;
        if (pIconName && !pIconName->isEmpty())
        {
            GdkPixbuf* pixbuf = load_icon_by_name(*pIconName);
            if (!pixbuf)
            {
                pImage = gtk_image_new_from_pixbuf(pixbuf);
                g_object_unref(pixbuf);
            }
        }
        else if (pImageSurface)
            pImage = image_new_from_virtual_device(*pImageSurface);

        GtkWidget *pItem;
        if (pImage)
        {
            GtkBox *pBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
            GtkWidget *pLabel = gtk_label_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new() : gtk_menu_item_new();
            gtk_box_pack_start(pBox, pImage, true, true, 0);
            gtk_box_pack_start(pBox, pLabel, true, true, 0);
            gtk_container_add(GTK_CONTAINER(pItem), GTK_WIDGET(pBox));
            gtk_widget_show_all(pItem);
        }
        else
        {
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr())
                                                       : gtk_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
        }

        if (eCheckRadioFalse == TRISTATE_FALSE)
            gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(pItem), true);

        ::set_buildable_id(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8));
        gtk_menu_shell_append(GTK_MENU_SHELL(m_pMenu), pItem);
        gtk_widget_show(pItem);
        add_to_map(GTK_MENU_ITEM(pItem));
        if (pos != -1)
            gtk_menu_reorder_child(m_pMenu, pItem, pos);
#else
        (void)pIconName; (void)pImageSurface;

        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            auto aSectionAndPos = get_section_and_pos_for(pMenuModel, pos);
            GMenu* pMenu = G_MENU(aSectionAndPos.first);
            // action with a target value ... the action name and target value are separated by a double
            // colon ... For example: "app.action::target"
            OUString sActionAndTarget;
            if (eCheckRadioFalse == TRISTATE_INDET)
                sActionAndTarget = "menu.normal." + rId + "::" + rId;
            else
                sActionAndTarget = "menu.radio." + rId + "::" + rId;
            g_menu_insert(pMenu, aSectionAndPos.second, MapToGtkAccelerator(rStr).getStr(), sActionAndTarget.toUtf8().getStr());

            assert(eCheckRadioFalse == TRISTATE_INDET); // come back to this later

            // TODO not redo entire group
            update_action_group_from_popover_model();
        }
#endif
    }

    void insert_separator(int pos, const OUString& rId)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pItem = gtk_separator_menu_item_new();
        ::set_buildable_id(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8));
        gtk_menu_shell_append(GTK_MENU_SHELL(m_pMenu), pItem);
        gtk_widget_show(pItem);
        add_to_map(GTK_MENU_ITEM(pItem));
        if (pos != -1)
            gtk_menu_reorder_child(m_pMenu, pItem, pos);
#else
        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            auto aSectionAndPos = get_section_and_pos_for(pMenuModel, pos);

            for (int nSection = 0, nSectionCount = g_menu_model_get_n_items(pMenuModel); nSection < nSectionCount; ++nSection)
            {
                GMenuModel* pSectionModel = g_menu_model_get_item_link(pMenuModel, nSection, G_MENU_LINK_SECTION);
                assert(pSectionModel);
                if (aSectionAndPos.first == pSectionModel)
                {
                    GMenu* pNewSection = g_menu_new();
                    GMenuItem* pSectionItem = g_menu_item_new_section(nullptr, G_MENU_MODEL(pNewSection));
                    OUString sActionAndTarget = "menu.separator." + rId + "::" + rId;
                    g_menu_item_set_detailed_action(pSectionItem, sActionAndTarget.toUtf8().getStr());
                    g_menu_insert_item(G_MENU(pMenuModel), nSection + 1, pSectionItem);
                    int nOldSectionCount = g_menu_model_get_n_items(pSectionModel);
                    for (int i = nOldSectionCount - 1; i >= aSectionAndPos.second; --i)
                    {
                        GMenuItem* pMenuItem = g_menu_item_new_from_model(pSectionModel, i);
                        g_menu_prepend_item(pNewSection, pMenuItem);
                        g_menu_remove(G_MENU(pSectionModel), i);
                        g_object_unref(pMenuItem);
                    }
                    g_object_unref(pSectionItem);
                    g_object_unref(pNewSection);
                }
            }
        }

#endif
    }

    void remove_item(const OString& rIdent)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkMenuItem* pMenuItem = m_aMap[rIdent];
        remove_from_map(pMenuItem);
        gtk_widget_destroy(GTK_WIDGET(pMenuItem));
#else
        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            std::pair<GMenuModel*, int> aRes = find_id(pMenuModel, rIdent);
            if (!aRes.first)
                return;
            g_menu_remove(G_MENU(aRes.first), aRes.second);
        }
#endif
    }

    void set_item_sensitive(const OString& rIdent, bool bSensitive)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GActionGroup* pActionGroup = m_aHiddenIds.find(rIdent) == m_aHiddenIds.end() ? m_pActionGroup : m_pHiddenActionGroup;
        GAction* pAction = g_action_map_lookup_action(G_ACTION_MAP(pActionGroup), m_aIdToAction[rIdent].getStr());
        g_simple_action_set_enabled(G_SIMPLE_ACTION(pAction), bSensitive);
#else
        gtk_widget_set_sensitive(GTK_WIDGET(m_aMap[rIdent]), bSensitive);
#endif
    }

    bool get_item_sensitive(const OString& rIdent) const
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GActionGroup* pActionGroup = m_aHiddenIds.find(rIdent) == m_aHiddenIds.end() ? m_pActionGroup : m_pHiddenActionGroup;
        GAction* pAction = g_action_map_lookup_action(G_ACTION_MAP(pActionGroup), m_aIdToAction.find(rIdent)->second.getStr());
        return g_action_get_enabled(pAction);
#else
        return gtk_widget_get_sensitive(GTK_WIDGET(m_aMap.find(rIdent)->second));
#endif
    }

    void set_item_active(const OString& rIdent, bool bActive)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        disable_item_notify_events();
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(m_aMap[rIdent]), bActive);
        enable_item_notify_events();
#else
        GActionGroup* pActionGroup = m_aHiddenIds.find(rIdent) == m_aHiddenIds.end() ? m_pActionGroup : m_pHiddenActionGroup;
        g_action_group_change_action_state(pActionGroup, m_aIdToAction[rIdent].getStr(),
                                           g_variant_new_string(bActive ? rIdent.getStr() : "'none'"));
#endif
    }

    bool get_item_active(const OString& rIdent) const
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(m_aMap.find(rIdent)->second));
#else
        GActionGroup* pActionGroup = m_aHiddenIds.find(rIdent) == m_aHiddenIds.end() ? m_pActionGroup : m_pHiddenActionGroup;
        GVariant* pState = g_action_group_get_action_state(pActionGroup, m_aIdToAction.find(rIdent)->second.getStr());
        if (!pState)
            return false;
        const char *pStateString = g_variant_get_string(pState, nullptr);
        bool bInactive = g_strcmp0(pStateString, "'none'") == 0;
        g_variant_unref(pState);
        return bInactive;
#endif
    }

    void set_item_label(const OString& rIdent, const OUString& rText)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_menu_item_set_label(m_aMap[rIdent], MapToGtkAccelerator(rText).getStr());
#else
        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            std::pair<GMenuModel*, int> aRes = find_id(pMenuModel, rIdent);
            if (!aRes.first)
                return;
            // clone the original item, remove the original, insert the replacement at
            // the original location
            GMenuItem* pMenuItem = g_menu_item_new_from_model(aRes.first, aRes.second);
            g_menu_remove(G_MENU(aRes.first), aRes.second);
            g_menu_item_set_label(pMenuItem, MapToGtkAccelerator(rText).getStr());
            g_menu_insert_item(G_MENU(aRes.first), aRes.second, pMenuItem);
            g_object_unref(pMenuItem);
        }
#endif
    }

    OUString get_item_label(const OString& rIdent) const
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        const gchar* pText = gtk_menu_item_get_label(m_aMap.find(rIdent)->second);
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
#else
        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            std::pair<GMenuModel*, int> aRes = find_id(pMenuModel, rIdent);
            if (!aRes.first)
                return OUString();

            // clone the original item to query its label
            GMenuItem* pMenuItem = g_menu_item_new_from_model(aRes.first, aRes.second);
            char *pLabel = nullptr;
            g_menu_item_get_attribute(pMenuItem, G_MENU_ATTRIBUTE_LABEL, "&s", &pLabel);
            OUString aRet(pLabel, pLabel ? strlen(pLabel) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pLabel);
            g_object_unref(pMenuItem);
            return aRet;
        }
        return OUString();
#endif
    }

    void set_item_visible(const OString& rIdent, bool bShow)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pWidget = GTK_WIDGET(m_aMap[rIdent]);
        if (bShow)
            gtk_widget_show(pWidget);
        else
            gtk_widget_hide(pWidget);
#else
        bool bOldVisible = m_aHiddenIds.find(rIdent) == m_aHiddenIds.end();
        if (bShow == bOldVisible)
            return;

        if (!bShow)
        {
            GAction* pAction = g_action_map_lookup_action(G_ACTION_MAP(m_pActionGroup), m_aIdToAction[rIdent].getStr());
            g_action_map_add_action(G_ACTION_MAP(m_pHiddenActionGroup), pAction);
            g_action_map_remove_action(G_ACTION_MAP(m_pActionGroup), m_aIdToAction[rIdent].getStr());
            m_aHiddenIds.insert(rIdent);
        }
        else
        {
            GAction* pAction = g_action_map_lookup_action(G_ACTION_MAP(m_pHiddenActionGroup), m_aIdToAction[rIdent].getStr());
            g_action_map_add_action(G_ACTION_MAP(m_pActionGroup), pAction);
            g_action_map_remove_action(G_ACTION_MAP(m_pHiddenActionGroup), m_aIdToAction[rIdent].getStr());
            m_aHiddenIds.erase(rIdent);
        }
#endif
    }

    OString get_item_id(int pos) const
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pMenu));
        gpointer pMenuItem = g_list_nth_data(pChildren, pos);
        OString id = ::get_buildable_id(GTK_BUILDABLE(pMenuItem));
        g_list_free(pChildren);
        return id;
#else
        OString sTarget;
        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            auto aSectionAndPos = get_section_and_pos_for(pMenuModel, pos);
            char *id;
            if (g_menu_model_get_item_attribute(aSectionAndPos.first, aSectionAndPos.second, "target", "s", &id))
            {
                sTarget = OString(id);
                g_free(id);
            }
        }
        return sTarget;
#endif
    }

    int get_n_children() const
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pMenu));
        int nLen = g_list_length(pChildren);
        g_list_free(pChildren);
        return nLen;
#else
        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
            return count_immediate_children(pMenuModel);
        return 0;
#endif
    }

    void clear_items()
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        for (const auto& a : m_aMap)
        {
            GtkMenuItem* pMenuItem = a.second;
            g_signal_handlers_disconnect_by_data(pMenuItem, this);
            gtk_widget_destroy(GTK_WIDGET(pMenuItem));
        }
        m_aMap.clear();
#else
        if (GMenuModel* pMenuModel = m_pMenu ? gtk_popover_menu_get_menu_model(m_pMenu) : nullptr)
        {
            GMenu* pMenu = G_MENU(pMenuModel);
            g_menu_remove_all(pMenu);
            g_menu_insert_section(pMenu, 0, nullptr, G_MENU_MODEL(g_menu_new()));
            m_aHiddenIds.clear();
            update_action_group_from_popover_model();
        }
#endif
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkMenu* getMenu() const
#else
    GtkPopoverMenu* getMenu() const
#endif
    {
        return m_pMenu;
    }

    virtual ~MenuHelper()
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        for (auto& a : m_aMap)
            g_signal_handlers_disconnect_by_data(a.second, this);
        if (m_bTakeOwnership)
            gtk_widget_destroy(GTK_WIDGET(m_pMenu));
#else
        g_object_unref(m_pActionGroup);
        g_object_unref(m_pHiddenActionGroup);
#endif
    }
};

class GtkInstanceSizeGroup : public weld::SizeGroup
{
private:
    GtkSizeGroup* m_pGroup;
public:
    GtkInstanceSizeGroup()
        : m_pGroup(gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL))
    {
    }
    virtual void add_widget(weld::Widget* pWidget) override
    {
        GtkInstanceWidget* pVclWidget = dynamic_cast<GtkInstanceWidget*>(pWidget);
        assert(pVclWidget);
        gtk_size_group_add_widget(m_pGroup, pVclWidget->getWidget());
    }
    virtual void set_mode(VclSizeGroupMode eVclMode) override
    {
        GtkSizeGroupMode eGtkMode(GTK_SIZE_GROUP_NONE);
        switch (eVclMode)
        {
            case VclSizeGroupMode::NONE:
                eGtkMode = GTK_SIZE_GROUP_NONE;
                break;
            case VclSizeGroupMode::Horizontal:
                eGtkMode = GTK_SIZE_GROUP_HORIZONTAL;
                break;
            case VclSizeGroupMode::Vertical:
                eGtkMode = GTK_SIZE_GROUP_VERTICAL;
                break;
            case VclSizeGroupMode::Both:
                eGtkMode = GTK_SIZE_GROUP_BOTH;
                break;
        }
        gtk_size_group_set_mode(m_pGroup, eGtkMode);
    }
    virtual ~GtkInstanceSizeGroup() override
    {
        g_object_unref(m_pGroup);
    }
};

class ChildFrame : public WorkWindow
{
private:
    Idle  maLayoutIdle;

    DECL_LINK(ImplHandleLayoutTimerHdl, Timer*, void);
public:
    ChildFrame(vcl::Window* pParent, WinBits nStyle)
        : WorkWindow(pParent, nStyle)
        , maLayoutIdle( "ChildFrame maLayoutIdle" )
    {
        maLayoutIdle.SetPriority(TaskPriority::RESIZE);
        maLayoutIdle.SetInvokeHandler( LINK( this, ChildFrame, ImplHandleLayoutTimerHdl ) );
    }

    virtual void dispose() override
    {
        maLayoutIdle.Stop();
        WorkWindow::dispose();
    }

    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override
    {
        WorkWindow::queue_resize(eReason);
        if (maLayoutIdle.IsActive())
            return;
        maLayoutIdle.Start();
    }

    void Layout()
    {
        if (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild))
            pChild->SetPosSizePixel(Point(0, 0), GetSizePixel());
    }

    virtual void Resize() override
    {
        maLayoutIdle.Stop();
        Layout();
        WorkWindow::Resize();
    }
};

IMPL_LINK_NOARG(ChildFrame, ImplHandleLayoutTimerHdl, Timer*, void)
{
    Layout();
}

class GtkInstanceContainer : public GtkInstanceWidget, public virtual weld::Container
{
private:
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkContainer* m_pContainer;
#else
    GtkWidget* m_pContainer;
#endif
    gulong m_nSetFocusChildSignalId;
    bool m_bChildHasFocus;

    static void implResetDefault(GtkWidget *pWidget, gpointer user_data)
    {
        if (GTK_IS_BUTTON(pWidget))
            g_object_set(G_OBJECT(pWidget), "has-default", false, nullptr);
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_CONTAINER(pWidget))
            gtk_container_forall(GTK_CONTAINER(pWidget), implResetDefault, user_data);
#else
        (void)user_data;
#endif
    }

    void signal_set_focus_child(bool bChildHasFocus)
    {
        if (m_bChildHasFocus != bChildHasFocus)
        {
            m_bChildHasFocus = bChildHasFocus;
            signal_container_focus_changed();
        }
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void signalSetFocusChild(GtkContainer*, GtkWidget* pChild, gpointer widget)
    {
        GtkInstanceContainer* pThis = static_cast<GtkInstanceContainer*>(widget);
        pThis->signal_set_focus_child(pChild != nullptr);
    }
#endif

public:
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkInstanceContainer(GtkContainer* pContainer, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pContainer), pBuilder, bTakeOwnership)
#else
    GtkInstanceContainer(GtkWidget* pContainer, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(pContainer, pBuilder, bTakeOwnership)
#endif
        , m_pContainer(pContainer)
        , m_nSetFocusChildSignalId(0)
        , m_bChildHasFocus(false)
    {
    }

    virtual void connect_container_focus_changed(const Link<Container&, void>& rLink) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!m_nSetFocusChildSignalId)
            m_nSetFocusChildSignalId = g_signal_connect(G_OBJECT(m_pContainer), "set-focus-child", G_CALLBACK(signalSetFocusChild), this);
#endif
        weld::Container::connect_container_focus_changed(rLink);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    GtkWidget* getContainer() { return m_pContainer; }
#else
    GtkContainer* getContainer() { return m_pContainer; }
#endif

    virtual void child_grab_focus() override
    {
        gtk_widget_grab_focus(m_pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
        bool bHasFocusChild = gtk_widget_get_focus_child(GTK_WIDGET(m_pContainer));
#else
        bool bHasFocusChild = gtk_container_get_focus_child(m_pContainer);
#endif
        if (!bHasFocusChild)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            if (GtkWidget* pChild = gtk_widget_get_first_child(m_pContainer))
            {
                gtk_widget_set_focus_child(m_pContainer, pChild);
                bHasFocusChild = true;
            }
#else
            GList* pChildren = gtk_container_get_children(m_pContainer);
            if (GList* pChild = g_list_first(pChildren))
            {
                gtk_container_set_focus_child(m_pContainer, static_cast<GtkWidget*>(pChild->data));
                bHasFocusChild = true;
            }
            g_list_free(pChildren);
#endif
        }

        if (bHasFocusChild)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            gtk_widget_child_focus(gtk_widget_get_focus_child(m_pWidget), GTK_DIR_TAB_FORWARD);
#else
            gtk_widget_child_focus(gtk_container_get_focus_child(GTK_CONTAINER(m_pWidget)), GTK_DIR_TAB_FORWARD);
#endif
        }

    }

    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override
    {
        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pWidget);
        assert(pGtkWidget);
        GtkWidget* pChild = pGtkWidget->getWidget();
        g_object_ref(pChild);
        auto pOldContainer = getContainer();
        container_remove(GTK_WIDGET(pOldContainer), pChild);

        GtkInstanceContainer* pNewGtkParent = dynamic_cast<GtkInstanceContainer*>(pNewParent);
        assert(!pNewParent || pNewGtkParent);
        if (pNewGtkParent)
        {
            auto pNewContainer = pNewGtkParent->getContainer();
            container_add(GTK_WIDGET(pNewContainer), pChild);
        }
        g_object_unref(pChild);
    }

    virtual void recursively_unset_default_buttons() override
    {
        implResetDefault(GTK_WIDGET(m_pContainer), nullptr);
    }

    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() override
    {
        // This will cause a GtkSalFrame to be created. With WB_SYSTEMCHILDWINDOW set it
        // will create a toplevel GtkEventBox window
        auto xEmbedWindow = VclPtr<ChildFrame>::Create(ImplGetDefaultWindow(), WB_SYSTEMCHILDWINDOW | WB_DIALOGCONTROL | WB_CHILDDLGCTRL);
        SalFrame* pFrame = xEmbedWindow->ImplGetFrame();
        GtkSalFrame* pGtkFrame = dynamic_cast<GtkSalFrame*>(pFrame);
        assert(pGtkFrame);

        // relocate that toplevel GtkEventBox into this widget
        GtkWidget* pWindow = pGtkFrame->getWindow();

        GtkWidget* pParent = gtk_widget_get_parent(pWindow);

        g_object_ref(pWindow);
        container_remove(pParent, pWindow);
        container_add(GTK_WIDGET(m_pContainer), pWindow);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_child_set(m_pContainer, pWindow, "expand", true, "fill", true, nullptr);
#endif
        gtk_widget_set_hexpand(pWindow, true);
        gtk_widget_set_vexpand(pWindow, true);
        gtk_widget_realize(pWindow);
        gtk_widget_set_can_focus(pWindow, true);
        g_object_unref(pWindow);

        // NoActivate otherwise Show grab focus to this widget
        xEmbedWindow->Show(true, ShowFlags::NoActivate);
        css::uno::Reference<css::awt::XWindow> xWindow(xEmbedWindow->GetComponentInterface(), css::uno::UNO_QUERY);
        return xWindow;
    }

    virtual ~GtkInstanceContainer() override
    {
        if (m_nSetFocusChildSignalId)
            g_signal_handler_disconnect(m_pContainer, m_nSetFocusChildSignalId);
    }
};

}

std::unique_ptr<weld::Container> GtkInstanceWidget::weld_parent() const
{
    GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
    if (!pParent)
        return nullptr;
#if !GTK_CHECK_VERSION(4, 0, 0)
    return std::make_unique<GtkInstanceContainer>(GTK_CONTAINER(pParent), m_pBuilder, false);
#else
    return std::make_unique<GtkInstanceContainer>(pParent, m_pBuilder, false);
#endif
}

namespace {

bool sortButtons(const GtkWidget* pA, const GtkWidget* pB)
{
    //order within groups according to platform rules
    return getButtonPriority(get_buildable_id(GTK_BUILDABLE(pA))) <
           getButtonPriority(get_buildable_id(GTK_BUILDABLE(pB)));
}

void sort_native_button_order(GtkBox* pContainer)
{
    std::vector<GtkWidget*> aChildren;
#if GTK_CHECK_VERSION(4, 0, 0)
    for (GtkWidget* pChild = gtk_widget_get_first_child(GTK_WIDGET(pContainer));
         pChild; pChild = gtk_widget_get_next_sibling(pChild))
    {
        aChildren.push_back(pChild);
    }
#else
    GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pContainer));
    for (GList* pChild = g_list_first(pChildren); pChild; pChild = g_list_next(pChild))
        aChildren.push_back(static_cast<GtkWidget*>(pChild->data));
    g_list_free(pChildren);
#endif

    //sort child order within parent so that we match the platform button order
    std::stable_sort(aChildren.begin(), aChildren.end(), sortButtons);

#if GTK_CHECK_VERSION(4, 0, 0)
    for (size_t pos = 0; pos < aChildren.size(); ++pos)
        gtk_box_reorder_child_after(pContainer, aChildren[pos], pos ? aChildren[pos - 1] : nullptr);
#else
    for (size_t pos = 0; pos < aChildren.size(); ++pos)
        gtk_box_reorder_child(pContainer, aChildren[pos], pos);
#endif
}

class GtkInstanceBox : public GtkInstanceContainer, public virtual weld::Box
{
private:
    GtkBox* m_pBox;

public:
    GtkInstanceBox(GtkBox* pBox, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        : GtkInstanceContainer(GTK_CONTAINER(pBox), pBuilder, bTakeOwnership)
#else
        : GtkInstanceContainer(GTK_WIDGET(pBox), pBuilder, bTakeOwnership)
#endif
        , m_pBox(pBox)
    {
    }

    virtual void reorder_child(weld::Widget* pWidget, int nNewPosition) override
    {
        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pWidget);
        assert(pGtkWidget);
        GtkWidget* pChild = pGtkWidget->getWidget();

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_box_reorder_child(m_pBox, pChild, nNewPosition);
#else
        if (nNewPosition == 0)
            gtk_box_reorder_child_after(m_pBox, pChild, nullptr);
        else
        {
            int nNewSiblingPos = nNewPosition - 1;
            int nChildPosition = 0;
            for (GtkWidget* pNewSibling = gtk_widget_get_first_child(GTK_WIDGET(m_pBox));
                 pNewSibling; pNewSibling = gtk_widget_get_next_sibling(pNewSibling))
            {
                if (nChildPosition == nNewSiblingPos)
                {
                    gtk_box_reorder_child_after(m_pBox, pChild, pNewSibling);
                    break;
                }
                ++nChildPosition;
            }
        }
#endif
    }

    virtual void sort_native_button_order() override
    {
        ::sort_native_button_order(m_pBox);
    }
};

}

namespace
{
    Point get_csd_offset(GtkWidget* pTopLevel)
    {
        // try and omit drawing CSD under wayland
        GtkWidget* pChild = widget_get_first_child(pTopLevel);

        gtk_coord x, y;
        gtk_widget_translate_coordinates(pChild, pTopLevel, 0, 0, &x, &y);

#if !GTK_CHECK_VERSION(4, 0, 0)
        int innerborder = gtk_container_get_border_width(GTK_CONTAINER(pChild));
        int outerborder = gtk_container_get_border_width(GTK_CONTAINER(pTopLevel));
        int totalborder = outerborder + innerborder;
        x -= totalborder;
        y -= totalborder;
#endif

        return Point(x, y);
    }

    void do_collect_screenshot_data(GtkWidget* pItem, gpointer data)
    {
        GtkWidget* pTopLevel = widget_get_toplevel(pItem);

        gtk_coord x, y;
        gtk_widget_translate_coordinates(pItem, pTopLevel, 0, 0, &x, &y);

        Point aOffset = get_csd_offset(pTopLevel);

        GtkAllocation alloc;
        gtk_widget_get_allocation(pItem, &alloc);

        const basegfx::B2IPoint aCurrentTopLeft(x - aOffset.X(), y - aOffset.Y());
        const basegfx::B2IRange aCurrentRange(aCurrentTopLeft, aCurrentTopLeft + basegfx::B2IPoint(alloc.width, alloc.height));

        if (!aCurrentRange.isEmpty())
        {
            weld::ScreenShotCollection* pCollection = static_cast<weld::ScreenShotCollection*>(data);
            pCollection->emplace_back(::get_help_id(pItem), aCurrentRange);
        }

#if GTK_CHECK_VERSION(4, 0, 0)
        for (GtkWidget* pChild = gtk_widget_get_first_child(pItem);
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            do_collect_screenshot_data(pChild, data);
        }
#else
        if (GTK_IS_CONTAINER(pItem))
            gtk_container_forall(GTK_CONTAINER(pItem), do_collect_screenshot_data, data);
#endif
    }

    tools::Rectangle get_monitor_workarea(GtkWidget* pWindow)
    {
        GdkRectangle aRect;
#if !GTK_CHECK_VERSION(4, 0, 0)
        GdkScreen* pScreen = gtk_widget_get_screen(pWindow);
        gint nMonitor = gdk_screen_get_monitor_at_window(pScreen, widget_get_surface(pWindow));
        gdk_screen_get_monitor_workarea(pScreen, nMonitor, &aRect);
#else
        GdkDisplay* pDisplay = gtk_widget_get_display(pWindow);
        GdkSurface* gdkWindow = widget_get_surface(pWindow);
        GdkMonitor* pMonitor = gdk_display_get_monitor_at_surface(pDisplay, gdkWindow);
        gdk_monitor_get_geometry(pMonitor, &aRect);
#endif
        return tools::Rectangle(aRect.x, aRect.y, aRect.x + aRect.width, aRect.y + aRect.height);
    }


class GtkInstanceWindow : public GtkInstanceContainer, public virtual weld::Window
{
private:
    GtkWindow* m_pWindow;
    rtl::Reference<SalGtkXWindow> m_xWindow; //uno api
    gulong m_nToplevelFocusChangedSignalId;

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean help_pressed(GtkAccelGroup*, GObject*, guint, GdkModifierType, gpointer widget)
    {
        GtkInstanceWindow* pThis = static_cast<GtkInstanceWindow*>(widget);
        pThis->help();
        return true;
    }
#endif

    static void signalToplevelFocusChanged(GtkWindow*, GParamSpec*, gpointer widget)
    {
        GtkInstanceWindow* pThis = static_cast<GtkInstanceWindow*>(widget);
        pThis->signal_container_focus_changed();
    }

    bool isPositioningAllowed() const
    {
        // no X/Y positioning under Wayland
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        return !DLSYM_GDK_IS_WAYLAND_DISPLAY(pDisplay);
    }

protected:
    void help();
public:
    GtkInstanceWindow(GtkWindow* pWindow, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        : GtkInstanceContainer(GTK_CONTAINER(pWindow), pBuilder, bTakeOwnership)
#else
        : GtkInstanceContainer(GTK_WIDGET(pWindow), pBuilder, bTakeOwnership)
#endif
        , m_pWindow(pWindow)
        , m_nToplevelFocusChangedSignalId(0)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        const bool bIsFrameWeld = pBuilder == nullptr;
        if (!bIsFrameWeld)
        {
            //hook up F1 to show help
            GtkAccelGroup *pGroup = gtk_accel_group_new();
            GClosure* closure = g_cclosure_new(G_CALLBACK(help_pressed), this, nullptr);
            gtk_accel_group_connect(pGroup, GDK_KEY_F1, static_cast<GdkModifierType>(0), GTK_ACCEL_LOCKED, closure);
            gtk_window_add_accel_group(pWindow, pGroup);
        }
#endif
    }

    virtual void set_title(const OUString& rTitle) override
    {
        ::set_title(m_pWindow, rTitle);
    }

    virtual OUString get_title() const override
    {
        return ::get_title(m_pWindow);
    }

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() override
    {
        if (!m_xWindow.is())
            m_xWindow.set(new SalGtkXWindow(this, m_pWidget));
        return m_xWindow;
    }

    virtual void set_modal(bool bModal) override
    {
        gtk_window_set_modal(m_pWindow, bModal);
    }

    virtual bool get_modal() const override
    {
        return gtk_window_get_modal(m_pWindow);
    }

    virtual void resize_to_request() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_window_set_default_size(m_pWindow, 1, 1);
#else
        gtk_window_resize(m_pWindow, 1, 1);
#endif
    }

    virtual void window_move(int x, int y) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_window_move(m_pWindow, x, y);
#else
        (void)x;
        (void)y;
#endif
    }

    virtual SystemEnvData get_system_data() const override
    {
        GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(GTK_WIDGET(m_pWindow));
        assert(pFrame && "nothing should call this impl, yet anyway, if ever, except on result of GetFrameWeld()");
        const SystemEnvData* pEnvData = pFrame->GetSystemData();
        assert(pEnvData);
        return *pEnvData;
    }

    virtual Size get_size() const override
    {
        int current_width, current_height;
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_window_get_size(m_pWindow, &current_width, &current_height);
#else
        gtk_window_get_default_size(m_pWindow, &current_width, &current_height);
#endif
        return Size(current_width, current_height);
    }

    virtual Point get_position() const override
    {
        int current_x(0), current_y(0);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_window_get_position(m_pWindow, &current_x, &current_y);
#endif
        return Point(current_x, current_y);
    }

    virtual tools::Rectangle get_monitor_workarea() const override
    {
        return ::get_monitor_workarea(GTK_WIDGET(m_pWindow));
    }

    virtual void set_centered_on_parent(bool bTrackGeometryRequests) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (bTrackGeometryRequests)
            gtk_window_set_position(m_pWindow, GTK_WIN_POS_CENTER_ALWAYS);
        else
            gtk_window_set_position(m_pWindow, GTK_WIN_POS_CENTER_ON_PARENT);
#else
        (void)bTrackGeometryRequests;
#endif
    }

    virtual bool get_resizable() const override
    {
        return gtk_window_get_resizable(m_pWindow);
    }

    virtual bool has_toplevel_focus() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return gtk_window_is_active(m_pWindow);
#else
        return gtk_window_has_toplevel_focus(m_pWindow);
#endif
    }

    virtual void present() override
    {
        gtk_window_present(m_pWindow);
    }

    virtual void set_window_state(const OString& rStr) override
    {
        WindowStateData aData;
        ImplWindowStateFromStr( aData, rStr );

        auto nMask = aData.GetMask();
        auto nState = aData.GetState() & WindowStateState::SystemMask;

        if (nMask & WindowStateMask::Width && nMask & WindowStateMask::Height)
        {
            gtk_window_set_default_size(m_pWindow, aData.GetWidth(), aData.GetHeight());
        }
        if (nMask & WindowStateMask::State)
        {
            if (nState & WindowStateState::Maximized)
                gtk_window_maximize(m_pWindow);
            else
                gtk_window_unmaximize(m_pWindow);
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (isPositioningAllowed() && (nMask & WindowStateMask::X && nMask & WindowStateMask::Y))
        {
            gtk_window_move(m_pWindow, aData.GetX(), aData.GetY());
        }
#endif
    }

    virtual OString get_window_state(WindowStateMask nMask) const override
    {
        bool bPositioningAllowed = isPositioningAllowed();

        WindowStateData aData;
        WindowStateMask nAvailable = WindowStateMask::State |
                                     WindowStateMask::Width | WindowStateMask::Height;
        if (bPositioningAllowed)
            nAvailable |= WindowStateMask::X | WindowStateMask::Y;
        aData.SetMask(nMask & nAvailable);

        if (nMask & WindowStateMask::State)
        {
            WindowStateState nState = WindowStateState::Normal;
            if (gtk_window_is_maximized(m_pWindow))
                nState |= WindowStateState::Maximized;
            aData.SetState(nState);
        }

        if (bPositioningAllowed && (nMask & (WindowStateMask::X | WindowStateMask::Y)))
        {
            auto aPos = get_position();
            aData.SetX(aPos.X());
            aData.SetY(aPos.Y());
        }

        if (nMask & (WindowStateMask::Width | WindowStateMask::Height))
        {
            auto aSize = get_size();
            aData.SetWidth(aSize.Width());
            aData.SetHeight(aSize.Height());
        }

        return aData.ToStr();
    }

    virtual void connect_container_focus_changed(const Link<Container&, void>& rLink) override
    {
        if (!m_nToplevelFocusChangedSignalId)
            m_nToplevelFocusChangedSignalId = g_signal_connect(m_pWindow, "notify::has-toplevel-focus", G_CALLBACK(signalToplevelFocusChanged), this);
        GtkInstanceContainer::connect_container_focus_changed(rLink);
    }

    virtual void disable_notify_events() override
    {
        if (m_nToplevelFocusChangedSignalId)
            g_signal_handler_block(m_pWidget, m_nToplevelFocusChangedSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        if (m_nToplevelFocusChangedSignalId)
            g_signal_handler_unblock(m_pWidget, m_nToplevelFocusChangedSignalId);
    }

    virtual VclPtr<VirtualDevice> screenshot() override
    {
        // detect if we have to manually setup its size
        bool bAlreadyRealized = gtk_widget_get_realized(GTK_WIDGET(m_pWindow));
        // has to be visible for draw to work
        bool bAlreadyVisible = gtk_widget_get_visible(GTK_WIDGET(m_pWindow));
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!bAlreadyVisible)
        {
            if (GTK_IS_DIALOG(m_pWindow))
                sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(m_pWindow))));
            gtk_widget_show(GTK_WIDGET(m_pWindow));
        }
#endif

        if (!bAlreadyRealized)
        {
            GtkAllocation allocation;
            gtk_widget_realize(GTK_WIDGET(m_pWindow));
            gtk_widget_get_allocation(GTK_WIDGET(m_pWindow), &allocation);
#if !GTK_CHECK_VERSION(4, 0, 0)
            gtk_widget_size_allocate(GTK_WIDGET(m_pWindow), &allocation);
#else
            gtk_widget_size_allocate(GTK_WIDGET(m_pWindow), &allocation, 0);
#endif
        }

        VclPtr<VirtualDevice> xOutput(VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT));
        xOutput->SetOutputSizePixel(get_size());
        cairo_surface_t* pSurface = get_underlying_cairo_surface(*xOutput);
        cairo_t* cr = cairo_create(pSurface);

        Point aOffset = get_csd_offset(GTK_WIDGET(m_pWindow));

        cairo_translate(cr, -aOffset.X(), -aOffset.Y());

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_draw(GTK_WIDGET(m_pWindow), cr);
#else
        GtkSnapshot* pSnapshot = gtk_snapshot_new();
        GtkWidgetClass* pWidgetClass = GTK_WIDGET_GET_CLASS(GTK_WIDGET(m_pWindow));
        pWidgetClass->snapshot(GTK_WIDGET(m_pWindow), pSnapshot);
        GskRenderNode* pNode = gtk_snapshot_free_to_node(pSnapshot);
        gsk_render_node_draw(pNode, cr);
        gsk_render_node_unref(pNode);
#endif

        cairo_destroy(cr);

        if (!bAlreadyVisible)
            gtk_widget_hide(GTK_WIDGET(m_pWindow));
        if (!bAlreadyRealized)
            gtk_widget_unrealize(GTK_WIDGET(m_pWindow));

        return xOutput;
    }

    virtual weld::ScreenShotCollection collect_screenshot_data() override
    {
        weld::ScreenShotCollection aRet;

#if GTK_CHECK_VERSION(4, 0, 0)
        for (GtkWidget* pChild = gtk_widget_get_first_child(GTK_WIDGET(m_pWindow));
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            do_collect_screenshot_data(pChild, &aRet);
        }
#else
        gtk_container_foreach(GTK_CONTAINER(m_pWindow), do_collect_screenshot_data, &aRet);
#endif

        return aRet;
    }

    virtual ~GtkInstanceWindow() override
    {
        if (m_nToplevelFocusChangedSignalId)
            g_signal_handler_disconnect(m_pWindow, m_nToplevelFocusChangedSignalId);
        if (m_xWindow.is())
            m_xWindow->clear();
    }
};

class GtkInstanceDialog;

struct DialogRunner
{
    GtkWindow* m_pDialog;
    GtkInstanceDialog *m_pInstance;
    gint m_nResponseId;
    GMainLoop *m_pLoop;
    VclPtr<vcl::Window> m_xFrameWindow;
    int m_nModalDepth;

    DialogRunner(GtkWindow* pDialog, GtkInstanceDialog* pInstance)
       : m_pDialog(pDialog)
       , m_pInstance(pInstance)
       , m_nResponseId(GTK_RESPONSE_NONE)
       , m_pLoop(nullptr)
       , m_nModalDepth(0)
    {
        GtkWindow* pParent = gtk_window_get_transient_for(m_pDialog);
        GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(GTK_WIDGET(pParent)) : nullptr;
        m_xFrameWindow = pFrame ? pFrame->GetWindow() : nullptr;
    }

    bool loop_is_running() const
    {
        return m_pLoop && g_main_loop_is_running(m_pLoop);
    }

    void loop_quit()
    {
        if (g_main_loop_is_running(m_pLoop))
            g_main_loop_quit(m_pLoop);
    }

    static void signal_response(GtkDialog*, gint nResponseId, gpointer data);
    static void signal_cancel(GtkAssistant*, gpointer data);

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signal_delete(GtkDialog* pDialog, GdkEventAny*, gpointer data)
    {
        DialogRunner* pThis = static_cast<DialogRunner*>(data);
        if (GTK_IS_ASSISTANT(pThis->m_pDialog))
        {
            // An assistant isn't a dialog, but we want to treat it like one
            signal_response(pDialog, GTK_RESPONSE_DELETE_EVENT, data);
        }
        else
            pThis->loop_quit();
        return true; /* Do not destroy */
    }
#endif

    static void signal_destroy(GtkDialog*, gpointer data)
    {
        DialogRunner* pThis = static_cast<DialogRunner*>(data);
        pThis->loop_quit();
    }

    void inc_modal_count()
    {
        if (m_xFrameWindow)
        {
            m_xFrameWindow->IncModalCount();
            if (m_nModalDepth == 0)
                m_xFrameWindow->ImplGetFrame()->NotifyModalHierarchy(true);
            ++m_nModalDepth;
        }
    }

    void dec_modal_count()
    {
        if (m_xFrameWindow)
        {
            m_xFrameWindow->DecModalCount();
            --m_nModalDepth;
            if (m_nModalDepth == 0)
                m_xFrameWindow->ImplGetFrame()->NotifyModalHierarchy(false);
        }
    }

    // same as gtk_dialog_run except that unmap doesn't auto-respond
    // so we can hide the dialog and restore it without a response getting
    // triggered
    gint run()
    {
        g_object_ref(m_pDialog);

        inc_modal_count();

        bool bWasModal = gtk_window_get_modal(m_pDialog);
        if (!bWasModal)
            gtk_window_set_modal(m_pDialog, true);

        if (!gtk_widget_get_visible(GTK_WIDGET(m_pDialog)))
            gtk_widget_show(GTK_WIDGET(m_pDialog));

        gulong nSignalResponseId = GTK_IS_DIALOG(m_pDialog) ? g_signal_connect(m_pDialog, "response", G_CALLBACK(signal_response), this) : 0;
        gulong nSignalCancelId = GTK_IS_ASSISTANT(m_pDialog) ? g_signal_connect(m_pDialog, "cancel", G_CALLBACK(signal_cancel), this) : 0;
#if !GTK_CHECK_VERSION(4, 0, 0)
        gulong nSignalDeleteId = g_signal_connect(m_pDialog, "delete-event", G_CALLBACK(signal_delete), this);
#endif
        gulong nSignalDestroyId = g_signal_connect(m_pDialog, "destroy", G_CALLBACK(signal_destroy), this);

        m_pLoop = g_main_loop_new(nullptr, false);
        m_nResponseId = GTK_RESPONSE_NONE;

        main_loop_run(m_pLoop);

        g_main_loop_unref(m_pLoop);

        m_pLoop = nullptr;

        if (!bWasModal)
            gtk_window_set_modal(m_pDialog, false);

        if (nSignalResponseId)
            g_signal_handler_disconnect(m_pDialog, nSignalResponseId);
        if (nSignalCancelId)
            g_signal_handler_disconnect(m_pDialog, nSignalCancelId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pDialog, nSignalDeleteId);
#endif
        g_signal_handler_disconnect(m_pDialog, nSignalDestroyId);

        dec_modal_count();

        g_object_unref(m_pDialog);

        return m_nResponseId;
    }

    ~DialogRunner()
    {
        if (m_xFrameWindow && m_nModalDepth)
        {
            // if, like the calc validation dialog does, the modality was
            // toggled off during execution ensure that on cleanup the parent
            // is left in the state it was found
            while (m_nModalDepth++ < 0)
                m_xFrameWindow->IncModalCount();
        }
    }
};

}

typedef std::set<GtkWidget*> winset;

namespace
{
#if GTK_CHECK_VERSION(4, 0, 0)
    void collectVisibleChildren(GtkWidget* pTop, winset& rVisibleWidgets)
    {
        for (GtkWidget* pChild = gtk_widget_get_first_child(pTop);
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            if (!gtk_widget_get_visible(pChild))
                continue;
            rVisibleWidgets.insert(pChild);
            collectVisibleChildren(pChild, rVisibleWidgets);
        }
    }
#endif

    void hideUnless(GtkWidget* pTop, const winset& rVisibleWidgets,
        std::vector<GtkWidget*> &rWasVisibleWidgets)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        for (GtkWidget* pChild = gtk_widget_get_first_child(pTop);
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            if (!gtk_widget_get_visible(pChild))
                continue;
            if (rVisibleWidgets.find(pChild) == rVisibleWidgets.end())
            {
                g_object_ref(pChild);
                rWasVisibleWidgets.emplace_back(pChild);
                gtk_widget_hide(pChild);
            }
            else
            {
                hideUnless(pChild, rVisibleWidgets, rWasVisibleWidgets);
            }
        }
#else
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pTop));
        for (GList* pEntry = g_list_first(pChildren); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkWidget* pChild = static_cast<GtkWidget*>(pEntry->data);
            if (!gtk_widget_get_visible(pChild))
                continue;
            if (rVisibleWidgets.find(pChild) == rVisibleWidgets.end())
            {
                g_object_ref(pChild);
                rWasVisibleWidgets.emplace_back(pChild);
                gtk_widget_hide(pChild);
            }
            else if (GTK_IS_CONTAINER(pChild))
            {
                hideUnless(pChild, rVisibleWidgets, rWasVisibleWidgets);
            }
        }
        g_list_free(pChildren);
#endif
    }

class GtkInstanceButton;

class GtkInstanceDialog : public GtkInstanceWindow, public virtual weld::Dialog
{
private:
    GtkWindow* m_pDialog;
    DialogRunner m_aDialogRun;
    std::shared_ptr<weld::DialogController> m_xDialogController;
    // Used to keep ourself alive during a runAsync(when doing runAsync without a DialogController)
    std::shared_ptr<weld::Dialog> m_xRunAsyncSelf;
    std::function<void(sal_Int32)> m_aFunc;
    gulong m_nCloseSignalId;
    gulong m_nResponseSignalId;
    gulong m_nCancelSignalId;
    gulong m_nSignalDeleteId;

    // for calc ref dialog that shrink to range selection widgets and resize back
    GtkWidget* m_pRefEdit;
    std::vector<GtkWidget*> m_aHiddenWidgets;    // vector of hidden Controls
    int m_nOldEditWidth;  // Original width of the input field
    int m_nOldEditWidthReq; // Original width request of the input field
#if !GTK_CHECK_VERSION(4, 0, 0)
    int m_nOldBorderWidth; // border width for expanded dialog
#endif

    void signal_close()
    {
        close(true);
    }

    static void signalClose(GtkWidget*, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        pThis->signal_close();
    }

    static void signalAsyncResponse(GtkWidget*, gint ret, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        pThis->asyncresponse(ret);
    }

    static void signalAsyncCancel(GtkAssistant*, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        // make esc in an assistant act as if cancel button was pressed
        pThis->close(false);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalAsyncDelete(GtkWidget* pDialog, GdkEventAny*, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        if (GTK_IS_ASSISTANT(pThis->m_pDialog))
        {
            // An assistant isn't a dialog, but we want to treat it like one
            signalAsyncResponse(pDialog, GTK_RESPONSE_DELETE_EVENT, widget);
        }
        return true; /* Do not destroy */
    }
#endif

    static int GtkToVcl(int ret)
    {
        if (ret == GTK_RESPONSE_OK)
            ret = RET_OK;
        else if (ret == GTK_RESPONSE_CANCEL)
            ret = RET_CANCEL;
        else if (ret == GTK_RESPONSE_DELETE_EVENT)
            ret = RET_CANCEL;
        else if (ret == GTK_RESPONSE_CLOSE)
            ret = RET_CLOSE;
        else if (ret == GTK_RESPONSE_YES)
            ret = RET_YES;
        else if (ret == GTK_RESPONSE_NO)
            ret = RET_NO;
        else if (ret == GTK_RESPONSE_HELP)
            ret = RET_HELP;
        return ret;
    }

    static int VclToGtk(int nResponse)
    {
        if (nResponse == RET_OK)
            return GTK_RESPONSE_OK;
        else if (nResponse == RET_CANCEL)
            return GTK_RESPONSE_CANCEL;
        else if (nResponse == RET_CLOSE)
            return GTK_RESPONSE_CLOSE;
        else if (nResponse == RET_YES)
            return GTK_RESPONSE_YES;
        else if (nResponse == RET_NO)
            return GTK_RESPONSE_NO;
        else if (nResponse == RET_HELP)
            return GTK_RESPONSE_HELP;
        return nResponse;
    }

    void asyncresponse(gint ret);

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void signalActivate(GtkMenuItem*, gpointer data)
    {
        bool* pActivate = static_cast<bool*>(data);
        *pActivate = true;
    }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    bool signal_screenshot_popup_menu(const GdkEventButton* pEvent)
    {
        GtkWidget *pMenu = gtk_menu_new();

        GtkWidget* pMenuItem = gtk_menu_item_new_with_mnemonic(MapToGtkAccelerator(VclResId(SV_BUTTONTEXT_SCREENSHOT)).getStr());
        gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
        bool bActivate(false);
        g_signal_connect(pMenuItem, "activate", G_CALLBACK(signalActivate), &bActivate);
        gtk_widget_show(pMenuItem);

        int button, event_time;
        if (pEvent)
        {
            button = pEvent->button;
            event_time = pEvent->time;
        }
        else
        {
            button = 0;
            event_time = gtk_get_current_event_time();
        }

        gtk_menu_attach_to_widget(GTK_MENU(pMenu), GTK_WIDGET(m_pDialog), nullptr);

        GMainLoop* pLoop = g_main_loop_new(nullptr, true);
        gulong nSignalId = g_signal_connect_swapped(G_OBJECT(pMenu), "deactivate", G_CALLBACK(g_main_loop_quit), pLoop);

        gtk_menu_popup(GTK_MENU(pMenu), nullptr, nullptr, nullptr, nullptr, button, event_time);

        if (g_main_loop_is_running(pLoop))
            main_loop_run(pLoop);

        g_main_loop_unref(pLoop);
        g_signal_handler_disconnect(pMenu, nSignalId);
        gtk_menu_detach(GTK_MENU(pMenu));

        if (bActivate)
        {
            // open screenshot annotation dialog
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            VclPtr<AbstractScreenshotAnnotationDlg> xTmp = pFact->CreateScreenshotAnnotationDlg(*this);
            ScopedVclPtr<AbstractScreenshotAnnotationDlg> xDialog(xTmp);
            xDialog->Execute();
        }

        return false;
    }
#endif

    static gboolean signalScreenshotPopupMenu(GtkWidget*, gpointer widget)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        return pThis->signal_screenshot_popup_menu(nullptr);
#else
        (void)widget;
        return false;
#endif
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalScreenshotButton(GtkWidget*, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_screenshot_button(pEvent);
    }

    bool signal_screenshot_button(GdkEventButton* pEvent)
    {
        if (gdk_event_triggers_context_menu(reinterpret_cast<GdkEvent*>(pEvent)) && pEvent->type == GDK_BUTTON_PRESS)
        {
            //if handled for context menu, stop processing
            return signal_screenshot_popup_menu(pEvent);
        }
        return false;
    }
#endif

public:
    GtkInstanceDialog(GtkWindow* pDialog, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWindow(pDialog, pBuilder, bTakeOwnership)
        , m_pDialog(pDialog)
        , m_aDialogRun(pDialog, this)
        , m_nResponseSignalId(0)
        , m_nCancelSignalId(0)
        , m_nSignalDeleteId(0)
        , m_pRefEdit(nullptr)
        , m_nOldEditWidth(0)
        , m_nOldEditWidthReq(0)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nOldBorderWidth(0)
#endif
    {
        if (GTK_IS_DIALOG(m_pDialog) || GTK_IS_ASSISTANT(m_pDialog))
            m_nCloseSignalId = g_signal_connect(m_pDialog, "close", G_CALLBACK(signalClose), this);
        else
            m_nCloseSignalId = 0;
        const bool bScreenshotMode(officecfg::Office::Common::Misc::ScreenshotMode::get());
        if (bScreenshotMode)
        {
            g_signal_connect(m_pDialog, "popup-menu", G_CALLBACK(signalScreenshotPopupMenu), this);
#if !GTK_CHECK_VERSION(4, 0, 0)
            g_signal_connect(m_pDialog, "button-press-event", G_CALLBACK(signalScreenshotButton), this);
#endif
        }
    }

    virtual bool runAsync(std::shared_ptr<weld::DialogController> rDialogController, const std::function<void(sal_Int32)>& func) override
    {
        assert(!m_nResponseSignalId && !m_nCancelSignalId && !m_nSignalDeleteId);

        m_xDialogController = rDialogController;
        m_aFunc = func;

        if (get_modal())
            m_aDialogRun.inc_modal_count();
        show();

        m_nResponseSignalId = GTK_IS_DIALOG(m_pDialog) ? g_signal_connect(m_pDialog, "response", G_CALLBACK(signalAsyncResponse), this) : 0;
        m_nCancelSignalId = GTK_IS_ASSISTANT(m_pDialog) ? g_signal_connect(m_pDialog, "cancel", G_CALLBACK(signalAsyncCancel), this) : 0;
#if !GTK_CHECK_VERSION(4, 0, 0)
        m_nSignalDeleteId = g_signal_connect(m_pDialog, "delete-event", G_CALLBACK(signalAsyncDelete), this);
#endif

        return true;
    }

    virtual bool runAsync(std::shared_ptr<Dialog> const & rxSelf, const std::function<void(sal_Int32)>& func) override
    {
        assert( rxSelf.get() == this );
        assert(!m_nResponseSignalId && !m_nCancelSignalId && !m_nSignalDeleteId);

        // In order to store a shared_ptr to ourself, we have to have been constructed by make_shared,
        // which is that rxSelf enforces.
        m_xRunAsyncSelf = rxSelf;
        m_aFunc = func;

        if (get_modal())
            m_aDialogRun.inc_modal_count();
        show();

        m_nResponseSignalId = GTK_IS_DIALOG(m_pDialog) ? g_signal_connect(m_pDialog, "response", G_CALLBACK(signalAsyncResponse), this) : 0;
        m_nCancelSignalId = GTK_IS_ASSISTANT(m_pDialog) ? g_signal_connect(m_pDialog, "cancel", G_CALLBACK(signalAsyncCancel), this) : 0;
#if !GTK_CHECK_VERSION(4, 0, 0)
        m_nSignalDeleteId = g_signal_connect(m_pDialog, "delete-event", G_CALLBACK(signalAsyncDelete), this);
#endif

        return true;
    }

    GtkInstanceButton* has_click_handler(int nResponse);

    virtual int run() override;

    virtual void show() override
    {
        if (gtk_widget_get_visible(m_pWidget))
            return;
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_DIALOG(m_pDialog))
            sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog))));
#endif
        GtkInstanceWindow::show();
    }

    virtual void set_modal(bool bModal) override
    {
        if (get_modal() == bModal)
            return;
        GtkInstanceWindow::set_modal(bModal);
        /* if change the dialog modality while its running, then also change the parent LibreOffice window
           modal count, we typically expect the dialog modality to be restored to its original state

           This change modality while running case is for...

           a) the calc/chart dialogs which put up an extra range chooser
           dialog, hides the original, the user can select a range of cells and
           on completion the original dialog is restored

           b) the validity dialog in calc
        */
        // tdf#135567 we know we are running in the sync case if loop_is_running is true
        // but for the async case we instead check for m_xDialogController which is set in
        // runAsync and cleared in asyncresponse
        if (m_aDialogRun.loop_is_running() || m_xDialogController)
        {
            if (bModal)
                m_aDialogRun.inc_modal_count();
            else
                m_aDialogRun.dec_modal_count();
        }
    }

    virtual void response(int nResponse) override;

    virtual void add_button(const OUString& rText, int nResponse, const OString& rHelpId) override
    {
        GtkWidget* pWidget = gtk_dialog_add_button(GTK_DIALOG(m_pDialog), MapToGtkAccelerator(rText).getStr(), VclToGtk(nResponse));
        if (!rHelpId.isEmpty())
            ::set_help_id(pWidget, rHelpId);
    }

    virtual void set_default_response(int nResponse) override
    {
        gtk_dialog_set_default_response(GTK_DIALOG(m_pDialog), VclToGtk(nResponse));
    }

    virtual GtkButton* get_widget_for_response(int nGtkResponse)
    {
        return GTK_BUTTON(gtk_dialog_get_widget_for_response(GTK_DIALOG(m_pDialog), nGtkResponse));
    }

    virtual weld::Button* weld_widget_for_response(int nVclResponse) override;

    virtual Container* weld_content_area() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        return new GtkInstanceContainer(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(m_pDialog))), m_pBuilder, false);
#else
        return new GtkInstanceContainer(gtk_dialog_get_content_area(GTK_DIALOG(m_pDialog)), m_pBuilder, false);
#endif
    }

    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override
    {
        GtkInstanceWidget* pVclEdit = dynamic_cast<GtkInstanceWidget*>(pEdit);
        assert(pVclEdit);
        GtkInstanceWidget* pVclButton = dynamic_cast<GtkInstanceWidget*>(pButton);

        GtkWidget* pRefEdit = pVclEdit->getWidget();
        GtkWidget* pRefBtn = pVclButton ? pVclButton->getWidget() : nullptr;

        m_nOldEditWidth = gtk_widget_get_allocated_width(pRefEdit);

        gtk_widget_get_size_request(pRefEdit, &m_nOldEditWidthReq, nullptr);

        //We want just pRefBtn and pRefEdit to be shown
        //mark widgets we want to be visible, starting with pRefEdit
        //and all its direct parents.
        winset aVisibleWidgets;
        GtkWidget *pContentArea = gtk_dialog_get_content_area(GTK_DIALOG(m_pDialog));
        for (GtkWidget *pCandidate = pRefEdit;
            pCandidate && pCandidate != pContentArea && gtk_widget_get_visible(pCandidate);
            pCandidate = gtk_widget_get_parent(pCandidate))
        {
            aVisibleWidgets.insert(pCandidate);
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        collectVisibleChildren(pRefEdit, aVisibleWidgets);
#endif
        if (pRefBtn)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            collectVisibleChildren(pRefBtn, aVisibleWidgets);
#endif
            //same again with pRefBtn, except stop if there's a
            //shared parent in the existing widgets
            for (GtkWidget *pCandidate = pRefBtn;
                pCandidate && pCandidate != pContentArea && gtk_widget_get_visible(pCandidate);
                pCandidate = gtk_widget_get_parent(pCandidate))
            {
                if (aVisibleWidgets.insert(pCandidate).second)
                    break;
            }
        }

        //hide everything except the aVisibleWidgets
        hideUnless(pContentArea, aVisibleWidgets, m_aHiddenWidgets);
        gtk_widget_set_size_request(pRefEdit, m_nOldEditWidth, -1);
#if !GTK_CHECK_VERSION(4, 0, 0)
        m_nOldBorderWidth = gtk_container_get_border_width(GTK_CONTAINER(m_pDialog));
        gtk_container_set_border_width(GTK_CONTAINER(m_pDialog), 0);
        if (GtkWidget* pActionArea = gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog)))
            gtk_widget_hide(pActionArea);
#else
        if (GtkWidget* pActionArea = gtk_dialog_get_header_bar(GTK_DIALOG(m_pDialog)))
            gtk_widget_hide(pActionArea);
#endif

        // calc's insert->function is springing back to its original size if the ref-button
        // is used to shrink the dialog down and then the user clicks in the calc area to do
        // the selection
        bool bWorkaroundSizeSpringingBack = DLSYM_GDK_IS_WAYLAND_DISPLAY(gtk_widget_get_display(m_pWidget));
        if (bWorkaroundSizeSpringingBack)
            gtk_widget_unmap(GTK_WIDGET(m_pDialog));

        resize_to_request();

        if (bWorkaroundSizeSpringingBack)
            gtk_widget_map(GTK_WIDGET(m_pDialog));

        m_pRefEdit = pRefEdit;
    }

    virtual void undo_collapse() override
    {
        // All others: Show();
        for (GtkWidget* pWindow : m_aHiddenWidgets)
        {
            gtk_widget_show(pWindow);
            g_object_unref(pWindow);
        }
        m_aHiddenWidgets.clear();

        gtk_widget_set_size_request(m_pRefEdit, m_nOldEditWidthReq, -1);
        m_pRefEdit = nullptr;
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_set_border_width(GTK_CONTAINER(m_pDialog), m_nOldBorderWidth);
        if (GtkWidget* pActionArea = gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog)))
            gtk_widget_show(pActionArea);
#else
        if (GtkWidget* pActionArea = gtk_dialog_get_header_bar(GTK_DIALOG(m_pDialog)))
            gtk_widget_show(pActionArea);
#endif
        resize_to_request();
        present();
    }

    void close(bool bCloseSignal);

    virtual void SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>&) override
    {
        //not implemented for the gtk variant
    }

    virtual ~GtkInstanceDialog() override
    {
        if (!m_aHiddenWidgets.empty())
        {
            for (GtkWidget* pWindow : m_aHiddenWidgets)
                g_object_unref(pWindow);
            m_aHiddenWidgets.clear();
        }

        if (m_nCloseSignalId)
            g_signal_handler_disconnect(m_pDialog, m_nCloseSignalId);
        assert(!m_nResponseSignalId && !m_nCancelSignalId && !m_nSignalDeleteId);
    }
};

}

void DialogRunner::signal_response(GtkDialog*, gint nResponseId, gpointer data)
{
    DialogRunner* pThis = static_cast<DialogRunner*>(data);

    // make GTK_RESPONSE_DELETE_EVENT act as if cancel button was pressed
    if (nResponseId == GTK_RESPONSE_DELETE_EVENT)
    {
        pThis->m_pInstance->close(false);
        return;
    }

    pThis->m_nResponseId = nResponseId;
    pThis->loop_quit();
}

void DialogRunner::signal_cancel(GtkAssistant*, gpointer data)
{
    DialogRunner* pThis = static_cast<DialogRunner*>(data);

    // make esc in an assistant act as if cancel button was pressed
    pThis->m_pInstance->close(false);
}

namespace {

class GtkInstanceMessageDialog : public GtkInstanceDialog, public virtual weld::MessageDialog
{
private:
    GtkMessageDialog* m_pMessageDialog;
public:
    GtkInstanceMessageDialog(GtkMessageDialog* pMessageDialog, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceDialog(GTK_WINDOW(pMessageDialog), pBuilder, bTakeOwnership)
        , m_pMessageDialog(pMessageDialog)
    {
    }

    virtual void set_primary_text(const OUString& rText) override
    {
        ::set_primary_text(m_pMessageDialog, rText);
    }

    virtual OUString get_primary_text() const override
    {
        return ::get_primary_text(m_pMessageDialog);
    }

    virtual void set_secondary_text(const OUString& rText) override
    {
        ::set_secondary_text(m_pMessageDialog, rText);
    }

    virtual OUString get_secondary_text() const override
    {
        return ::get_secondary_text(m_pMessageDialog);
    }

    virtual Container* weld_message_area() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        return new GtkInstanceContainer(GTK_CONTAINER(gtk_message_dialog_get_message_area(m_pMessageDialog)), m_pBuilder, false);
#else
        return new GtkInstanceContainer(gtk_message_dialog_get_message_area(m_pMessageDialog), m_pBuilder, false);
#endif
    }
};

void set_label_wrap(GtkLabel* pLabel, bool bWrap)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_label_set_wrap(pLabel, bWrap);
#else
    gtk_label_set_line_wrap(pLabel, bWrap);
#endif
}

class GtkInstanceAssistant : public GtkInstanceDialog, public virtual weld::Assistant
{
private:
    GtkAssistant* m_pAssistant;
    GtkWidget* m_pSidebar;
    GtkWidget* m_pSidebarEventBox;
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkButtonBox* m_pButtonBox;
#else
    GtkBox* m_pButtonBox;
#endif
    GtkButton* m_pHelp;
    GtkButton* m_pBack;
    GtkButton* m_pNext;
    GtkButton* m_pFinish;
    GtkButton* m_pCancel;
    gulong m_nButtonPressSignalId;
    std::vector<std::unique_ptr<GtkInstanceContainer>> m_aPages;
    std::map<OString, bool> m_aNotClickable;

    int find_page(std::string_view ident) const
    {
        int nPages = gtk_assistant_get_n_pages(m_pAssistant);
        for (int i = 0; i < nPages; ++i)
        {
            GtkWidget* pPage = gtk_assistant_get_nth_page(m_pAssistant, i);
            OString sBuildableName = ::get_buildable_id(GTK_BUILDABLE(pPage));
            if (sBuildableName == ident)
                return i;
        }
        return -1;
    }

    static void wrap_sidebar_label(GtkWidget *pWidget, gpointer /*user_data*/)
    {
        if (GTK_IS_LABEL(pWidget))
        {
            ::set_label_wrap(GTK_LABEL(pWidget), true);
            gtk_label_set_width_chars(GTK_LABEL(pWidget), 22);
            gtk_label_set_max_width_chars(GTK_LABEL(pWidget), 22);
        }
    }

    static void find_sidebar(GtkWidget *pWidget, gpointer user_data)
    {
        OString sBuildableName = ::get_buildable_id(GTK_BUILDABLE(pWidget));
        if (sBuildableName == "sidebar")
        {
            GtkWidget **ppSidebar = static_cast<GtkWidget**>(user_data);
            *ppSidebar = pWidget;
        }
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_CONTAINER(pWidget))
            gtk_container_forall(GTK_CONTAINER(pWidget), find_sidebar, user_data);
#endif
    }

    static void signalHelpClicked(GtkButton*, gpointer widget)
    {
        GtkInstanceAssistant* pThis = static_cast<GtkInstanceAssistant*>(widget);
        pThis->signal_help_clicked();
    }

    void signal_help_clicked()
    {
        help();
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalButton(GtkWidget*, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceAssistant* pThis = static_cast<GtkInstanceAssistant*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_button(pEvent);
    }

    bool signal_button(const GdkEventButton* pEvent)
    {
        int nNewCurrentPage = -1;

        GtkAllocation allocation;

        int nPageIndex = 0;

#if GTK_CHECK_VERSION(4, 0, 0)
        for (GtkWidget* pWidget = gtk_widget_get_first_child(m_pSidebar);
             pWidget; pWidget = gtk_widget_get_next_sibling(pChild))
        {
#else
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pSidebar));
        for (GList* pChild = g_list_first(pChildren); pChild; pChild = g_list_next(pChild))
        {
            GtkWidget* pWidget = static_cast<GtkWidget*>(pChild->data);
#endif
            if (!gtk_widget_get_visible(pWidget))
                continue;

            gtk_widget_get_allocation(pWidget, &allocation);

            gtk_coord dest_x1, dest_y1;
            gtk_widget_translate_coordinates(pWidget,
                                             m_pSidebarEventBox,
                                             0,
                                             0,
                                             &dest_x1,
                                             &dest_y1);

            gtk_coord dest_x2, dest_y2;
            gtk_widget_translate_coordinates(pWidget,
                                             m_pSidebarEventBox,
                                             allocation.width,
                                             allocation.height,
                                             &dest_x2,
                                             &dest_y2);


            if (pEvent->x >= dest_x1 && pEvent->x <= dest_x2 && pEvent->y >= dest_y1 && pEvent->y <= dest_y2)
            {
                nNewCurrentPage = nPageIndex;
                break;
            }

            ++nPageIndex;
        }
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_list_free(pChildren);
#endif

        if (nNewCurrentPage != -1 && nNewCurrentPage != get_current_page())
        {
            OString sIdent = get_page_ident(nNewCurrentPage);
            if (!m_aNotClickable[sIdent] && !signal_jump_page(sIdent))
                set_current_page(nNewCurrentPage);
        }

        return false;
    }
#endif

public:
    GtkInstanceAssistant(GtkAssistant* pAssistant, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceDialog(GTK_WINDOW(pAssistant), pBuilder, bTakeOwnership)
        , m_pAssistant(pAssistant)
        , m_pSidebar(nullptr)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        m_pButtonBox = GTK_BUTTON_BOX(gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL));
        gtk_button_box_set_layout(m_pButtonBox, GTK_BUTTONBOX_END);
        gtk_box_set_spacing(GTK_BOX(m_pButtonBox), 6);
#else
        m_pButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
#endif

        m_pBack = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Back)).getStr()));
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_set_can_default(GTK_WIDGET(m_pBack), true);
        ::set_buildable_id(GTK_BUILDABLE(m_pBack), "previous");
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pBack), false, false, 0);

        m_pNext = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Next)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pNext), true);
        ::set_buildable_id(GTK_BUILDABLE(m_pNext), "next");
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pNext), false, false, 0);

        m_pCancel = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Cancel)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pCancel), true);
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pCancel), false, false, 0);

        m_pFinish = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Finish)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pFinish), true);
        ::set_buildable_id(GTK_BUILDABLE(m_pFinish), "finish");
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pFinish), false, false, 0);

        m_pHelp = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Help)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pHelp), true);
        g_signal_connect(m_pHelp, "clicked", G_CALLBACK(signalHelpClicked), this);
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pHelp), false, false, 0);

        gtk_assistant_add_action_widget(pAssistant, GTK_WIDGET(m_pButtonBox));
        gtk_button_box_set_child_secondary(m_pButtonBox, GTK_WIDGET(m_pHelp), true);
        gtk_widget_set_hexpand(GTK_WIDGET(m_pButtonBox), true);

        GtkWidget* pParent = gtk_widget_get_parent(GTK_WIDGET(m_pButtonBox));
        gtk_container_child_set(GTK_CONTAINER(pParent), GTK_WIDGET(m_pButtonBox), "expand", true, "fill", true, nullptr);
        gtk_widget_set_halign(pParent, GTK_ALIGN_FILL);

        // Hide the built-in ones early so we get a nice optimal size for the width without
        // including the unused contents
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pParent));
        for (GList* pChild = g_list_first(pChildren); pChild; pChild = g_list_next(pChild))
        {
            GtkWidget* pWidget = static_cast<GtkWidget*>(pChild->data);
            gtk_widget_hide(pWidget);
        }
        g_list_free(pChildren);

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_show_all(GTK_WIDGET(m_pButtonBox));
#else
        gtk_widget_show(GTK_WIDGET(m_pButtonBox));
#endif

        find_sidebar(GTK_WIDGET(m_pAssistant), &m_pSidebar);

        m_pSidebarEventBox = ::ensureEventWidget(m_pSidebar);
        m_nButtonPressSignalId = m_pSidebarEventBox ? g_signal_connect(m_pSidebarEventBox, "button-press-event", G_CALLBACK(signalButton), this) : 0;
#endif
    }

    virtual int get_current_page() const override
    {
        return gtk_assistant_get_current_page(m_pAssistant);
    }

    virtual int get_n_pages() const override
    {
        return gtk_assistant_get_n_pages(m_pAssistant);
    }

    virtual OString get_page_ident(int nPage) const override
    {
        const GtkWidget* pWidget = gtk_assistant_get_nth_page(m_pAssistant, nPage);
        return ::get_buildable_id(GTK_BUILDABLE(pWidget));
    }

    virtual OString get_current_page_ident() const override
    {
        return get_page_ident(get_current_page());
    }

    virtual void set_current_page(int nPage) override
    {
        OString sDialogTitle(gtk_window_get_title(GTK_WINDOW(m_pAssistant)));

        gtk_assistant_set_current_page(m_pAssistant, nPage);

        // if the page doesn't have a title, then the dialog will now have no
        // title, so restore the original title as a fallback
        GtkWidget* pPage = gtk_assistant_get_nth_page(m_pAssistant, nPage);
        if (!gtk_assistant_get_page_title(m_pAssistant, pPage))
            gtk_window_set_title(GTK_WINDOW(m_pAssistant), sDialogTitle.getStr());
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        int nPage = find_page(rIdent);
        if (nPage == -1)
            return;
        set_current_page(nPage);
    }

    virtual void set_page_title(const OString& rIdent, const OUString& rTitle) override
    {
        int nIndex = find_page(rIdent);
        if (nIndex == -1)
            return;
        GtkWidget* pPage = gtk_assistant_get_nth_page(m_pAssistant, nIndex);
        gtk_assistant_set_page_title(m_pAssistant, pPage,
                                     OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8).getStr());
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_forall(GTK_CONTAINER(m_pSidebar), wrap_sidebar_label, nullptr);
#endif
    }

    virtual OUString get_page_title(const OString& rIdent) const override
    {
        int nIndex = find_page(rIdent);
        if (nIndex == -1)
            return OUString();
        GtkWidget* pPage = gtk_assistant_get_nth_page(m_pAssistant, nIndex);
        const gchar* pStr = gtk_assistant_get_page_title(m_pAssistant, pPage);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void set_page_sensitive(const OString& rIdent, bool bSensitive) override
    {
        m_aNotClickable[rIdent] = !bSensitive;
    }

    virtual void set_page_index(const OString& rIdent, int nNewIndex) override
    {
        int nOldIndex = find_page(rIdent);
        if (nOldIndex == -1)
            return;

        if (nOldIndex == nNewIndex)
            return;

        GtkWidget* pPage = gtk_assistant_get_nth_page(m_pAssistant, nOldIndex);

        g_object_ref(pPage);
        OString sTitle(gtk_assistant_get_page_title(m_pAssistant, pPage));
        gtk_assistant_remove_page(m_pAssistant, nOldIndex);
        gtk_assistant_insert_page(m_pAssistant, pPage, nNewIndex);
        gtk_assistant_set_page_type(m_pAssistant, pPage, GTK_ASSISTANT_PAGE_CUSTOM);
        gtk_assistant_set_page_title(m_pAssistant, pPage, sTitle.getStr());
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_forall(GTK_CONTAINER(m_pSidebar), wrap_sidebar_label, nullptr);
#endif
        g_object_unref(pPage);
    }

    virtual weld::Container* append_page(const OString& rIdent) override
    {
        disable_notify_events();

        GtkWidget *pChild = gtk_grid_new();
        ::set_buildable_id(GTK_BUILDABLE(pChild), rIdent);
        gtk_assistant_append_page(m_pAssistant, pChild);
        gtk_assistant_set_page_type(m_pAssistant, pChild, GTK_ASSISTANT_PAGE_CUSTOM);
        gtk_widget_show(pChild);

        enable_notify_events();

#if !GTK_CHECK_VERSION(4, 0, 0)
        m_aPages.emplace_back(new GtkInstanceContainer(GTK_CONTAINER(pChild), m_pBuilder, false));
#else
        m_aPages.emplace_back(new GtkInstanceContainer(pChild, m_pBuilder, false));
#endif

        return m_aPages.back().get();
    }

    virtual void set_page_side_help_id(const OString& rHelpId) override
    {
        if (!m_pSidebar)
            return;
        ::set_help_id(m_pSidebar, rHelpId);
    }

    virtual GtkButton* get_widget_for_response(int nGtkResponse) override
    {
        GtkButton* pButton = nullptr;
        if (nGtkResponse == GTK_RESPONSE_YES)
            pButton = m_pNext;
        else if (nGtkResponse == GTK_RESPONSE_NO)
            pButton = m_pBack;
        else if (nGtkResponse == GTK_RESPONSE_OK)
            pButton = m_pFinish;
        else if (nGtkResponse == GTK_RESPONSE_CANCEL)
            pButton = m_pCancel;
        else if (nGtkResponse == GTK_RESPONSE_HELP)
            pButton = m_pHelp;
        return pButton;
    }

    virtual ~GtkInstanceAssistant() override
    {
        if (m_nButtonPressSignalId)
            g_signal_handler_disconnect(m_pSidebarEventBox, m_nButtonPressSignalId);
    }
};

class GtkInstanceFrame : public GtkInstanceContainer, public virtual weld::Frame
{
private:
    GtkFrame* m_pFrame;
public:
    GtkInstanceFrame(GtkFrame* pFrame, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        : GtkInstanceContainer(GTK_CONTAINER(pFrame), pBuilder, bTakeOwnership)
#else
        : GtkInstanceContainer(GTK_WIDGET(pFrame), pBuilder, bTakeOwnership)
#endif
        , m_pFrame(pFrame)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        gtk_label_set_label(GTK_LABEL(gtk_frame_get_label_widget(m_pFrame)), rText.replaceFirst("~", "").toUtf8().getStr());
    }

    virtual OUString get_label() const override
    {
        const gchar* pStr = gtk_frame_get_label(m_pFrame);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual std::unique_ptr<weld::Label> weld_label_widget() const override;
};

class GtkInstancePaned : public GtkInstanceContainer, public virtual weld::Paned
{
private:
    GtkPaned* m_pPaned;
public:
    GtkInstancePaned(GtkPaned* pPaned, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        : GtkInstanceContainer(GTK_CONTAINER(pPaned), pBuilder, bTakeOwnership)
#else
        : GtkInstanceContainer(GTK_WIDGET(pPaned), pBuilder, bTakeOwnership)
#endif
        , m_pPaned(pPaned)
    {
    }

    virtual void set_position(int nPos) override
    {
        gtk_paned_set_position(m_pPaned, nPos);
    }

    virtual int get_position() const override
    {
        return gtk_paned_get_position(m_pPaned);
    }
};

}

static GType immobilized_viewport_get_type();
static gpointer immobilized_viewport_parent_class;

#ifndef NDEBUG
#   define IMMOBILIZED_TYPE_VIEWPORT         (immobilized_viewport_get_type())
#   define IMMOBILIZED_IS_VIEWPORT(obj)      (G_TYPE_CHECK_INSTANCE_TYPE((obj), IMMOBILIZED_TYPE_VIEWPORT))
#endif

namespace {

struct ImmobilizedViewportPrivate
{
    GtkAdjustment  *hadjustment;
    GtkAdjustment  *vadjustment;
};

}

#define IMMOBILIZED_VIEWPORT_PRIVATE_DATA "ImmobilizedViewportPrivateData"

enum
{
    PROP_0,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY,
    PROP_SHADOW_TYPE
};

static void viewport_set_adjustment(GtkViewport *viewport,
                                    GtkOrientation  orientation,
                                    GtkAdjustment  *adjustment)
{
    ImmobilizedViewportPrivate* priv =
        static_cast<ImmobilizedViewportPrivate*>(g_object_get_data(G_OBJECT(viewport),
                                                                   IMMOBILIZED_VIEWPORT_PRIVATE_DATA));

    if (!adjustment)
        adjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        if (priv->hadjustment)
            g_object_unref(priv->hadjustment);
        priv->hadjustment = adjustment;
    }
    else
    {
        if (priv->vadjustment)
            g_object_unref(priv->vadjustment);
        priv->vadjustment = adjustment;
    }

    g_object_ref_sink(adjustment);
}

static void
immobilized_viewport_set_property(GObject* object,
                                  guint prop_id,
                                  const GValue* value,
                                  GParamSpec* /*pspec*/)
{
    GtkViewport *viewport = GTK_VIEWPORT(object);

    switch (prop_id)
    {
        case PROP_HADJUSTMENT:
            viewport_set_adjustment(viewport, GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(g_value_get_object(value)));
            break;
        case PROP_VADJUSTMENT:
            viewport_set_adjustment(viewport, GTK_ORIENTATION_VERTICAL, GTK_ADJUSTMENT(g_value_get_object(value)));
            break;
        case PROP_HSCROLL_POLICY:
        case PROP_VSCROLL_POLICY:
            break;
        default:
            SAL_WARN( "vcl.gtk", "unknown property\n");
            break;
    }
}

static void
immobilized_viewport_get_property(GObject* object,
                                  guint prop_id,
                                  GValue* value,
                                  GParamSpec* /*pspec*/)
{
    ImmobilizedViewportPrivate* priv =
        static_cast<ImmobilizedViewportPrivate*>(g_object_get_data(object,
                                                                   IMMOBILIZED_VIEWPORT_PRIVATE_DATA));

    switch (prop_id)
    {
        case PROP_HADJUSTMENT:
            g_value_set_object(value, priv->hadjustment);
            break;
        case PROP_VADJUSTMENT:
            g_value_set_object(value, priv->vadjustment);
            break;
        case PROP_HSCROLL_POLICY:
            g_value_set_enum(value, GTK_SCROLL_MINIMUM);
            break;
        case PROP_VSCROLL_POLICY:
            g_value_set_enum(value, GTK_SCROLL_MINIMUM);
            break;
        default:
            SAL_WARN( "vcl.gtk", "unknown property\n");
            break;
    }
}

static ImmobilizedViewportPrivate*
immobilized_viewport_new_private_data()
{
    ImmobilizedViewportPrivate* priv = g_slice_new0(ImmobilizedViewportPrivate);
    priv->hadjustment = nullptr;
    priv->vadjustment = nullptr;
    return priv;
}

static void
immobilized_viewport_instance_init(GTypeInstance *instance, gpointer /*klass*/)
{
    GObject* object = G_OBJECT(instance);
    g_object_set_data(object, IMMOBILIZED_VIEWPORT_PRIVATE_DATA,
                      immobilized_viewport_new_private_data());
}

static void
immobilized_viewport_finalize(GObject* object)
{
    void* priv = g_object_get_data(object, IMMOBILIZED_VIEWPORT_PRIVATE_DATA);
    if (priv)
    {
        g_slice_free(ImmobilizedViewportPrivate, priv);
        g_object_set_data(object, IMMOBILIZED_VIEWPORT_PRIVATE_DATA, nullptr);
    }
    G_OBJECT_CLASS(immobilized_viewport_parent_class)->finalize(object);
}

static void immobilized_viewport_class_init(GtkWidgetClass* klass)
{
    immobilized_viewport_parent_class = g_type_class_peek_parent(klass);

    GObjectClass* o_class = G_OBJECT_CLASS(klass);

    /* GObject signals */
    o_class->finalize = immobilized_viewport_finalize;
    o_class->set_property = immobilized_viewport_set_property;
    o_class->get_property = immobilized_viewport_get_property;

    /* Properties */
    g_object_class_override_property(o_class, PROP_HADJUSTMENT,    "hadjustment");
    g_object_class_override_property(o_class, PROP_VADJUSTMENT,    "vadjustment");
    g_object_class_override_property(o_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property(o_class, PROP_VSCROLL_POLICY, "vscroll-policy");
}

GType immobilized_viewport_get_type()
{
    static GType type = 0;

    if (!type)
    {
        GTypeQuery query;
        g_type_query(gtk_viewport_get_type(), &query);

        static const GTypeInfo tinfo =
        {
            static_cast<guint16>(query.class_size),
            nullptr,  /* base init */
            nullptr,  /* base finalize */
            reinterpret_cast<GClassInitFunc>(immobilized_viewport_class_init), /* class init */
            nullptr,  /* class finalize */
            nullptr,  /* class data */
            static_cast<guint16>(query.instance_size), /* instance size */
            0,        /* nb preallocs */
            immobilized_viewport_instance_init,  /* instance init */
            nullptr   /* value table */
        };

        type = g_type_register_static(GTK_TYPE_VIEWPORT, "ImmobilizedViewport",
                                      &tinfo, GTypeFlags(0));
    }

    return type;
}

static VclPolicyType GtkToVcl(GtkPolicyType eType)
{
    VclPolicyType eRet(VclPolicyType::NEVER);
    switch (eType)
    {
        case GTK_POLICY_ALWAYS:
            eRet = VclPolicyType::ALWAYS;
            break;
        case GTK_POLICY_AUTOMATIC:
            eRet = VclPolicyType::AUTOMATIC;
            break;
        case GTK_POLICY_EXTERNAL:
        case GTK_POLICY_NEVER:
            eRet = VclPolicyType::NEVER;
            break;
    }
    return eRet;
}

static GtkPolicyType VclToGtk(VclPolicyType eType)
{
    GtkPolicyType eRet(GTK_POLICY_ALWAYS);
    switch (eType)
    {
        case VclPolicyType::ALWAYS:
            eRet = GTK_POLICY_ALWAYS;
            break;
        case VclPolicyType::AUTOMATIC:
            eRet = GTK_POLICY_AUTOMATIC;
            break;
        case VclPolicyType::NEVER:
            eRet = GTK_POLICY_NEVER;
            break;
    }
    return eRet;
}

static GtkMessageType VclToGtk(VclMessageType eType)
{
    GtkMessageType eRet(GTK_MESSAGE_INFO);
    switch (eType)
    {
        case VclMessageType::Info:
            eRet = GTK_MESSAGE_INFO;
            break;
        case VclMessageType::Warning:
            eRet = GTK_MESSAGE_WARNING;
            break;
        case VclMessageType::Question:
            eRet = GTK_MESSAGE_QUESTION;
            break;
        case VclMessageType::Error:
            eRet = GTK_MESSAGE_ERROR;
            break;
        case VclMessageType::Other:
            eRet = GTK_MESSAGE_OTHER;
            break;
    }
    return eRet;
}

static GtkButtonsType VclToGtk(VclButtonsType eType)
{
    GtkButtonsType eRet(GTK_BUTTONS_NONE);
    switch (eType)
    {
        case VclButtonsType::NONE:
            eRet = GTK_BUTTONS_NONE;
            break;
        case VclButtonsType::Ok:
            eRet = GTK_BUTTONS_OK;
            break;
        case VclButtonsType::Close:
            eRet = GTK_BUTTONS_CLOSE;
            break;
        case VclButtonsType::Cancel:
            eRet = GTK_BUTTONS_CANCEL;
            break;
        case VclButtonsType::YesNo:
            eRet = GTK_BUTTONS_YES_NO;
            break;
        case VclButtonsType::OkCancel:
            eRet = GTK_BUTTONS_OK_CANCEL;
            break;
    }
    return eRet;
}

static GtkSelectionMode VclToGtk(SelectionMode eType)
{
    GtkSelectionMode eRet(GTK_SELECTION_NONE);
    switch (eType)
    {
        case SelectionMode::NONE:
            eRet = GTK_SELECTION_NONE;
            break;
        case SelectionMode::Single:
            eRet = GTK_SELECTION_SINGLE;
            break;
        case SelectionMode::Range:
            eRet = GTK_SELECTION_BROWSE;
            break;
        case SelectionMode::Multiple:
            eRet = GTK_SELECTION_MULTIPLE;
            break;
    }
    return eRet;
}

namespace {

class GtkInstanceScrolledWindow final : public GtkInstanceContainer, public virtual weld::ScrolledWindow
{
private:
    GtkScrolledWindow* m_pScrolledWindow;
    GtkWidget *m_pOrigViewport;
    GtkCssProvider* m_pScrollBarCssProvider;
    GtkAdjustment* m_pVAdjustment;
    GtkAdjustment* m_pHAdjustment;
    gulong m_nVAdjustChangedSignalId;
    gulong m_nHAdjustChangedSignalId;

    static void signalVAdjustValueChanged(GtkAdjustment*, gpointer widget)
    {
        GtkInstanceScrolledWindow* pThis = static_cast<GtkInstanceScrolledWindow*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_vadjustment_changed();
    }

    static void signalHAdjustValueChanged(GtkAdjustment*, gpointer widget)
    {
        GtkInstanceScrolledWindow* pThis = static_cast<GtkInstanceScrolledWindow*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_hadjustment_changed();
    }

public:
    GtkInstanceScrolledWindow(GtkScrolledWindow* pScrolledWindow, GtkInstanceBuilder* pBuilder, bool bTakeOwnership, bool bUserManagedScrolling)
#if !GTK_CHECK_VERSION(4, 0, 0)
        : GtkInstanceContainer(GTK_CONTAINER(pScrolledWindow), pBuilder, bTakeOwnership)
#else
        : GtkInstanceContainer(GTK_WIDGET(pScrolledWindow), pBuilder, bTakeOwnership)
#endif
        , m_pScrolledWindow(pScrolledWindow)
        , m_pOrigViewport(nullptr)
        , m_pScrollBarCssProvider(nullptr)
        , m_pVAdjustment(gtk_scrolled_window_get_vadjustment(m_pScrolledWindow))
        , m_pHAdjustment(gtk_scrolled_window_get_hadjustment(m_pScrolledWindow))
        , m_nVAdjustChangedSignalId(g_signal_connect(m_pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustValueChanged), this))
        , m_nHAdjustChangedSignalId(g_signal_connect(m_pHAdjustment, "value-changed", G_CALLBACK(signalHAdjustValueChanged), this))
    {
        if (bUserManagedScrolling)
            set_user_managed_scrolling();
    }

    void set_user_managed_scrolling()
    {
        disable_notify_events();
        //remove the original viewport and replace it with our bodged one which
        //doesn't do any scrolling and expects its child to figure it out somehow
        assert(!m_pOrigViewport);
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget *pViewport = gtk_scrolled_window_get_child(m_pScrolledWindow);
#else
        GtkWidget *pViewport = gtk_bin_get_child(GTK_BIN(m_pScrolledWindow));
#endif
        assert(GTK_IS_VIEWPORT(pViewport));
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget *pChild= gtk_viewport_get_child(GTK_VIEWPORT(pViewport));
#else
        GtkWidget *pChild = gtk_bin_get_child(GTK_BIN(pViewport));
#endif
        g_object_ref(pChild);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_viewport_set_child(GTK_VIEWPORT(pViewport), nullptr);
#else
        gtk_container_remove(GTK_CONTAINER(pViewport), pChild);
#endif
        g_object_ref(pViewport);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_scrolled_window_set_child(m_pScrolledWindow, nullptr);
#else
        gtk_container_remove(GTK_CONTAINER(m_pScrolledWindow), pViewport);
#endif
        GtkWidget* pNewViewport = GTK_WIDGET(g_object_new(immobilized_viewport_get_type(), nullptr));
        gtk_widget_show(pNewViewport);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_scrolled_window_set_child(m_pScrolledWindow, pNewViewport);
        gtk_viewport_set_child(GTK_VIEWPORT(pNewViewport), pChild);
#else
        gtk_container_add(GTK_CONTAINER(m_pScrolledWindow), pNewViewport);
        gtk_container_add(GTK_CONTAINER(pNewViewport), pChild);
#endif
        g_object_unref(pChild);
        m_pOrigViewport = pViewport;
        enable_notify_events();
    }

    virtual void hadjustment_configure(int value, int lower, int upper,
                                       int step_increment, int page_increment,
                                       int page_size) override
    {
        disable_notify_events();
        if (SwapForRTL())
            value = upper - (value - lower + page_size);
        gtk_adjustment_configure(m_pHAdjustment, value, lower, upper, step_increment, page_increment, page_size);
        enable_notify_events();
    }

    virtual int hadjustment_get_value() const override
    {
        int value = gtk_adjustment_get_value(m_pHAdjustment);

        if (SwapForRTL())
        {
            int upper = gtk_adjustment_get_upper(m_pHAdjustment);
            int lower = gtk_adjustment_get_lower(m_pHAdjustment);
            int page_size = gtk_adjustment_get_page_size(m_pHAdjustment);
            value = lower + (upper - value - page_size);
        }

        return value;
    }

    virtual void hadjustment_set_value(int value) override
    {
        disable_notify_events();

        if (SwapForRTL())
        {
            int upper = gtk_adjustment_get_upper(m_pHAdjustment);
            int lower = gtk_adjustment_get_lower(m_pHAdjustment);
            int page_size = gtk_adjustment_get_page_size(m_pHAdjustment);
            value = upper - (value - lower + page_size);
        }

        gtk_adjustment_set_value(m_pHAdjustment, value);
        enable_notify_events();
    }

    virtual int hadjustment_get_upper() const override
    {
         return gtk_adjustment_get_upper(m_pHAdjustment);
    }

    virtual void hadjustment_set_upper(int upper) override
    {
        disable_notify_events();
        gtk_adjustment_set_upper(m_pHAdjustment, upper);
        enable_notify_events();
    }

    virtual int hadjustment_get_page_size() const override
    {
        return gtk_adjustment_get_page_size(m_pHAdjustment);
    }

    virtual void hadjustment_set_page_size(int size) override
    {
        gtk_adjustment_set_page_size(m_pHAdjustment, size);
    }

    virtual void hadjustment_set_page_increment(int size) override
    {
        gtk_adjustment_set_page_increment(m_pHAdjustment, size);
    }

    virtual void hadjustment_set_step_increment(int size) override
    {
        gtk_adjustment_set_step_increment(m_pHAdjustment, size);
    }

    virtual void set_hpolicy(VclPolicyType eHPolicy) override
    {
        GtkPolicyType eGtkVPolicy;
        gtk_scrolled_window_get_policy(m_pScrolledWindow, nullptr, &eGtkVPolicy);
        gtk_scrolled_window_set_policy(m_pScrolledWindow, VclToGtk(eHPolicy), eGtkVPolicy);
    }

    virtual VclPolicyType get_hpolicy() const override
    {
        GtkPolicyType eGtkHPolicy;
        gtk_scrolled_window_get_policy(m_pScrolledWindow, &eGtkHPolicy, nullptr);
        return GtkToVcl(eGtkHPolicy);
    }

    virtual void vadjustment_configure(int value, int lower, int upper,
                                       int step_increment, int page_increment,
                                       int page_size) override
    {
        disable_notify_events();
        gtk_adjustment_configure(m_pVAdjustment, value, lower, upper, step_increment, page_increment, page_size);
        enable_notify_events();
    }

    virtual int vadjustment_get_value() const override
    {
        return gtk_adjustment_get_value(m_pVAdjustment);
    }

    virtual void vadjustment_set_value(int value) override
    {
        disable_notify_events();
        gtk_adjustment_set_value(m_pVAdjustment, value);
        enable_notify_events();
    }

    virtual int vadjustment_get_upper() const override
    {
         return gtk_adjustment_get_upper(m_pVAdjustment);
    }

    virtual void vadjustment_set_upper(int upper) override
    {
        disable_notify_events();
        gtk_adjustment_set_upper(m_pVAdjustment, upper);
        enable_notify_events();
    }

    virtual int vadjustment_get_lower() const override
    {
         return gtk_adjustment_get_lower(m_pVAdjustment);
    }

    virtual void vadjustment_set_lower(int lower) override
    {
        disable_notify_events();
        gtk_adjustment_set_lower(m_pVAdjustment, lower);
        enable_notify_events();
    }

    virtual int vadjustment_get_page_size() const override
    {
        return gtk_adjustment_get_page_size(m_pVAdjustment);
    }

    virtual void vadjustment_set_page_size(int size) override
    {
        gtk_adjustment_set_page_size(m_pVAdjustment, size);
    }

    virtual void vadjustment_set_page_increment(int size) override
    {
        gtk_adjustment_set_page_increment(m_pVAdjustment, size);
    }

    virtual void vadjustment_set_step_increment(int size) override
    {
        gtk_adjustment_set_step_increment(m_pVAdjustment, size);
    }

    virtual void set_vpolicy(VclPolicyType eVPolicy) override
    {
        GtkPolicyType eGtkHPolicy;
        gtk_scrolled_window_get_policy(m_pScrolledWindow, &eGtkHPolicy, nullptr);
        gtk_scrolled_window_set_policy(m_pScrolledWindow, eGtkHPolicy, VclToGtk(eVPolicy));
    }

    virtual VclPolicyType get_vpolicy() const override
    {
        GtkPolicyType eGtkVPolicy;
        gtk_scrolled_window_get_policy(m_pScrolledWindow, nullptr, &eGtkVPolicy);
        return GtkToVcl(eGtkVPolicy);
    }

    virtual int get_scroll_thickness() const override
    {
        if (gtk_scrolled_window_get_overlay_scrolling(m_pScrolledWindow))
            return 0;
        return gtk_widget_get_allocated_width(gtk_scrolled_window_get_vscrollbar(m_pScrolledWindow));
    }

    virtual void set_scroll_thickness(int nThickness) override
    {
        GtkWidget *pHorzBar = gtk_scrolled_window_get_hscrollbar(m_pScrolledWindow);
        GtkWidget *pVertBar = gtk_scrolled_window_get_vscrollbar(m_pScrolledWindow);
        gtk_widget_set_size_request(pHorzBar, -1, nThickness);
        gtk_widget_set_size_request(pVertBar, nThickness, -1);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_block(m_pHAdjustment, m_nHAdjustChangedSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_unblock(m_pHAdjustment, m_nHAdjustChangedSignalId);
    }

    virtual void customize_scrollbars(const Color& rBackgroundColor,
                                      const Color& rShadowColor,
                                      const Color& rFaceColor) override
    {
        GtkWidget *pHorzBar = gtk_scrolled_window_get_hscrollbar(m_pScrolledWindow);
        GtkWidget *pVertBar = gtk_scrolled_window_get_vscrollbar(m_pScrolledWindow);
        GtkStyleContext *pHorzContext = gtk_widget_get_style_context(pHorzBar);
        GtkStyleContext *pVertContext = gtk_widget_get_style_context(pVertBar);
        if (m_pScrollBarCssProvider)
        {
            gtk_style_context_remove_provider(pHorzContext, GTK_STYLE_PROVIDER(m_pScrollBarCssProvider));
            gtk_style_context_remove_provider(pVertContext, GTK_STYLE_PROVIDER(m_pScrollBarCssProvider));
        }

        m_pScrollBarCssProvider = gtk_css_provider_new();
        // intentionally 'trough' a long, narrow open container.
        OUString aBuffer = "scrollbar contents trough { background-color: #" + rBackgroundColor.AsRGBHexString() + "; } "
                           "scrollbar contents trough slider { background-color: #" + rShadowColor.AsRGBHexString() + "; } "
                           "scrollbar contents button { background-color: #" + rFaceColor.AsRGBHexString() + "; } "
                           "scrollbar contents button { color: #000000; } "
                           "scrollbar contents button:disabled { color: #7f7f7f; }";
        OString aResult = OUStringToOString(aBuffer, RTL_TEXTENCODING_UTF8);
        css_provider_load_from_data(m_pScrollBarCssProvider, aResult.getStr(), aResult.getLength());

        gtk_style_context_add_provider(pHorzContext, GTK_STYLE_PROVIDER(m_pScrollBarCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_style_context_add_provider(pVertContext, GTK_STYLE_PROVIDER(m_pScrollBarCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    virtual ~GtkInstanceScrolledWindow() override
    {
        // we use GtkInstanceContainer::[disable|enable]_notify_events later on
        // to avoid touching these removed handlers
        g_signal_handler_disconnect(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_disconnect(m_pHAdjustment, m_nHAdjustChangedSignalId);

        if (m_pScrollBarCssProvider)
        {
            GtkStyleContext *pHorzContext = gtk_widget_get_style_context(gtk_scrolled_window_get_hscrollbar(m_pScrolledWindow));
            GtkStyleContext *pVertContext = gtk_widget_get_style_context(gtk_scrolled_window_get_vscrollbar(m_pScrolledWindow));
            gtk_style_context_remove_provider(pHorzContext, GTK_STYLE_PROVIDER(m_pScrollBarCssProvider));
            gtk_style_context_remove_provider(pVertContext, GTK_STYLE_PROVIDER(m_pScrollBarCssProvider));
            m_pScrollBarCssProvider = nullptr;
        }

        //put it back the way it was
        if (!m_pOrigViewport)
            return;

        GtkInstanceContainer::disable_notify_events();

        // force in new adjustment to drop the built-in handlers on value-changed
        // which are getting called eventually by the gtk_container_add call
        // and which access the scrolled window indicators which, in the case
        // of user-managed scrolling windows in toolbar popups during popdown
        // are nullptr causing crashes when the scrolling windows is not at its
        // initial 0,0 position
        GtkAdjustment *pVAdjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        gtk_scrolled_window_set_vadjustment(m_pScrolledWindow, pVAdjustment);
        GtkAdjustment *pHAdjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        gtk_scrolled_window_set_hadjustment(m_pScrolledWindow, pHAdjustment);

#if GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget *pViewport = gtk_scrolled_window_get_child(m_pScrolledWindow);
#else
        GtkWidget *pViewport = gtk_bin_get_child(GTK_BIN(m_pScrolledWindow));
#endif
        assert(IMMOBILIZED_IS_VIEWPORT(pViewport));
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget *pChild= gtk_viewport_get_child(GTK_VIEWPORT(pViewport));
#else
        GtkWidget *pChild = gtk_bin_get_child(GTK_BIN(pViewport));
#endif
        g_object_ref(pChild);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_viewport_set_child(GTK_VIEWPORT(pViewport), nullptr);
#else
        gtk_container_remove(GTK_CONTAINER(pViewport), pChild);
#endif
        g_object_ref(pViewport);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_scrolled_window_set_child(m_pScrolledWindow, nullptr);
#else
        gtk_container_remove(GTK_CONTAINER(m_pScrolledWindow), pViewport);
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_scrolled_window_set_child(m_pScrolledWindow, m_pOrigViewport);
#else
        gtk_container_add(GTK_CONTAINER(m_pScrolledWindow), m_pOrigViewport);
#endif
        // coverity[freed_arg : FALSE] - this does not free m_pOrigViewport, it is reffed by m_pScrolledWindow
        g_object_unref(m_pOrigViewport);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_viewport_set_child(GTK_VIEWPORT(m_pOrigViewport), pChild);
#else
        gtk_container_add(GTK_CONTAINER(m_pOrigViewport), pChild);
#endif
        g_object_unref(pChild);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_destroy(pViewport);
#endif
        g_object_unref(pViewport);
        m_pOrigViewport = nullptr;
        GtkInstanceContainer::enable_notify_events();
    }
};

}

namespace {

class GtkInstanceNotebook : public GtkInstanceWidget, public virtual weld::Notebook
{
private:
    GtkNotebook* m_pNotebook;
    GtkBox* m_pOverFlowBox;
    GtkNotebook* m_pOverFlowNotebook;
    gulong m_nSwitchPageSignalId;
    gulong m_nOverFlowSwitchPageSignalId;
#if GTK_CHECK_VERSION(4, 0, 0)
    NotifyingLayout* m_pLayout;
#else
    gulong m_nNotebookSizeAllocateSignalId;
    gulong m_nFocusSignalId;
#endif
    gulong m_nChangeCurrentPageId;
    guint m_nLaunchSplitTimeoutId;
    bool m_bOverFlowBoxActive;
    bool m_bOverFlowBoxIsStart;
    bool m_bInternalPageChange;
    int m_nStartTabCount;
    int m_nEndTabCount;
    mutable std::vector<std::unique_ptr<GtkInstanceContainer>> m_aPages;

    static void signalSwitchPage(GtkNotebook*, GtkWidget*, guint nNewPage, gpointer widget)
    {
        GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_switch_page(nNewPage);
    }

    static gboolean launch_overflow_switch_page(GtkInstanceNotebook* pThis)
    {
        SolarMutexGuard aGuard;
        pThis->signal_overflow_switch_page();
        return false;
    }

    static void signalOverFlowSwitchPage(GtkNotebook*, GtkWidget*, guint, gpointer widget)
    {
        g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, reinterpret_cast<GSourceFunc>(launch_overflow_switch_page), widget, nullptr);
    }

    void signal_switch_page(int nNewPage)
    {
        if (m_bOverFlowBoxIsStart)
        {
            auto nOverFlowLen = m_bOverFlowBoxActive ? gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1 : 0;
            // add count of overflow pages, minus the extra tab
            nNewPage += nOverFlowLen;
        }

        bool bAllow = m_bInternalPageChange || !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
        if (!bAllow)
        {
            g_signal_stop_emission_by_name(m_pNotebook, "switch-page");
            return;
        }
        if (m_bOverFlowBoxActive)
            gtk_notebook_set_current_page(m_pOverFlowNotebook, gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1);
        OString sNewIdent(get_page_ident(nNewPage));
        if (!m_bInternalPageChange)
            m_aEnterPageHdl.Call(sNewIdent);
    }

    void unsplit_notebooks()
    {
        int nOverFlowPages = gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1;
        int nMainPages = gtk_notebook_get_n_pages(m_pNotebook);
        int nPageIndex = 0;
        if (!m_bOverFlowBoxIsStart)
            nPageIndex += nMainPages;

        // take the overflow pages, and put them back at the end of the normal one
        int i = nMainPages;
        while (nOverFlowPages)
        {
            OString sIdent(get_page_ident(m_pOverFlowNotebook, 0));
            OUString sLabel(get_tab_label_text(m_pOverFlowNotebook, 0));
            remove_page(m_pOverFlowNotebook, sIdent);

            GtkWidget* pPage = m_aPages[nPageIndex]->getWidget();
            insert_page(m_pNotebook, sIdent, sLabel, pPage, -1);

            GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pNotebook,
                                                               gtk_notebook_get_nth_page(m_pNotebook, i));
            gtk_widget_set_hexpand(pTabWidget, true);
            --nOverFlowPages;
            ++i;
            ++nPageIndex;
        }

        // remove the dangling placeholder tab page
        remove_page(m_pOverFlowNotebook, "useless");
    }

    // a tab has been selected on the overflow notebook
    void signal_overflow_switch_page()
    {
        int nNewPage = gtk_notebook_get_current_page(m_pOverFlowNotebook);
        int nOverFlowPages = gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1;
        if (nNewPage == nOverFlowPages)
        {
            // the useless tab which is there because there has to be an active tab
            return;
        }

        // check if we are allowed leave before attempting to resplit the notebooks
        bool bAllow = !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
        if (!bAllow)
            return;

        disable_notify_events();

        // take the overflow pages, and put them back at the end of the normal one
        unsplit_notebooks();

        // now redo the split, the pages will be split the other way around this time
        std::swap(m_nStartTabCount, m_nEndTabCount);
        split_notebooks();

        // coverity[pass_freed_arg : FALSE] - m_pNotebook is not freed here
        gtk_notebook_set_current_page(m_pNotebook, nNewPage);

        enable_notify_events();

        // trigger main notebook switch-page callback
        OString sNewIdent(get_page_ident(m_pNotebook, nNewPage));
        m_aEnterPageHdl.Call(sNewIdent);
    }

    static OString get_page_ident(GtkNotebook *pNotebook, guint nPage)
    {
        const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(pNotebook, gtk_notebook_get_nth_page(pNotebook, nPage));
        return ::get_buildable_id(GTK_BUILDABLE(pTabWidget));
    }

    static gint get_page_number(GtkNotebook *pNotebook, std::string_view ident)
    {
        gint nPages = gtk_notebook_get_n_pages(pNotebook);
        for (gint i = 0; i < nPages; ++i)
        {
            const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(pNotebook, gtk_notebook_get_nth_page(pNotebook, i));
            OString sBuildableName = ::get_buildable_id(GTK_BUILDABLE(pTabWidget));
            if (sBuildableName == ident)
                return i;
        }
        return -1;
    }

    int remove_page(GtkNotebook *pNotebook, std::string_view ident)
    {
        disable_notify_events();
        int nPageNumber = get_page_number(pNotebook, ident);
        gtk_notebook_remove_page(pNotebook, nPageNumber);
        enable_notify_events();
        return nPageNumber;
    }

    static OUString get_tab_label_text(GtkNotebook *pNotebook, guint nPage)
    {
        const gchar* pStr = gtk_notebook_get_tab_label_text(pNotebook, gtk_notebook_get_nth_page(pNotebook, nPage));
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    static void set_tab_label_text(GtkNotebook *pNotebook, guint nPage, const OUString& rText)
    {
        OString sUtf8(rText.toUtf8());

        GtkWidget* pPage = gtk_notebook_get_nth_page(pNotebook, nPage);

        // tdf#128241 if there's already a label here, reuse it so the buildable
        // name remains the same, gtk_notebook_set_tab_label_text will replace
        // the label widget with a new one
        GtkWidget* pTabWidget = gtk_notebook_get_tab_label(pNotebook, pPage);
        if (pTabWidget && GTK_IS_LABEL(pTabWidget))
        {
            gtk_label_set_label(GTK_LABEL(pTabWidget), sUtf8.getStr());
            return;
        }

        gtk_notebook_set_tab_label_text(pNotebook, pPage, sUtf8.getStr());
    }

    void append_useless_page(GtkNotebook *pNotebook)
    {
        disable_notify_events();

        GtkWidget *pTabWidget = gtk_fixed_new();
        ::set_buildable_id(GTK_BUILDABLE(pTabWidget), "useless");

        GtkWidget *pChild = gtk_grid_new();
        gtk_notebook_append_page(pNotebook, pChild, pTabWidget);
        gtk_widget_show(pChild);
        gtk_widget_show(pTabWidget);

        enable_notify_events();
    }

    void insert_page(GtkNotebook *pNotebook, const OString& rIdent, const OUString& rLabel, GtkWidget *pChild, int nPos)
    {
        disable_notify_events();

        GtkWidget *pTabWidget = gtk_label_new_with_mnemonic(MapToGtkAccelerator(rLabel).getStr());
        ::set_buildable_id(GTK_BUILDABLE(pTabWidget), rIdent);
        gtk_notebook_insert_page(pNotebook, pChild, pTabWidget, nPos);
        gtk_widget_show(pChild);
        gtk_widget_show(pTabWidget);

        if (nPos != -1)
        {
            unsigned int nPageIndex = static_cast<unsigned int>(nPos);
            if (nPageIndex < m_aPages.size())
                m_aPages.insert(m_aPages.begin() + nPageIndex, nullptr);
        }

        enable_notify_events();
    }

    void make_overflow_boxes()
    {
        m_pOverFlowBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        GtkWidget* pParent = gtk_widget_get_parent(GTK_WIDGET(m_pNotebook));
        container_add(pParent, GTK_WIDGET(m_pOverFlowBox));
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_box_append(m_pOverFlowBox, GTK_WIDGET(m_pOverFlowNotebook));
#else
        gtk_box_pack_start(m_pOverFlowBox, GTK_WIDGET(m_pOverFlowNotebook), false, false, 0);
#endif
        g_object_ref(m_pNotebook);
        container_remove(pParent, GTK_WIDGET(m_pNotebook));
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_box_append(m_pOverFlowBox, GTK_WIDGET(m_pNotebook));
#else
        gtk_box_pack_start(m_pOverFlowBox, GTK_WIDGET(m_pNotebook), true, true, 0);
#endif
        // coverity[freed_arg : FALSE] - this does not free m_pNotebook , it is reffed by pParent
        g_object_unref(m_pNotebook);
        gtk_widget_show(GTK_WIDGET(m_pOverFlowBox));
    }

    void split_notebooks()
    {
        // get the original preferred size for the notebook, the sane width
        // expected here depends on the notebooks all initially having
        // scrollable tabs enabled
        GtkAllocation alloc;
        gtk_widget_get_allocation(GTK_WIDGET(m_pNotebook), &alloc);

        // toggle the direction of the split since the last time
        m_bOverFlowBoxIsStart = !m_bOverFlowBoxIsStart;
        if (!m_pOverFlowBox)
             make_overflow_boxes();

        // don't scroll the tabs anymore
        // coverity[pass_freed_arg : FALSE] - m_pNotebook is not freed here
        gtk_notebook_set_scrollable(m_pNotebook, false);

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pNotebook));
        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pOverFlowNotebook));
#else
        g_object_freeze_notify(G_OBJECT(m_pNotebook));
        g_object_freeze_notify(G_OBJECT(m_pOverFlowNotebook));
#endif

        gtk_widget_show(GTK_WIDGET(m_pOverFlowNotebook));

        gint nPages;

        GtkRequisition size1, size2;

        if (!m_nStartTabCount && !m_nEndTabCount)
        {
            nPages = gtk_notebook_get_n_pages(m_pNotebook);

            std::vector<int> aLabelWidths;
            //move tabs to the overflow notebook
            for (int i = 0; i < nPages; ++i)
            {
                OUString sLabel(get_tab_label_text(m_pNotebook, i));
                aLabelWidths.push_back(get_pixel_size(sLabel).Width());
            }
            int row_width = std::accumulate(aLabelWidths.begin(), aLabelWidths.end(), 0) / 2;
            int count = 0;
            for (int i = 0; i < nPages; ++i)
            {
                count += aLabelWidths[i];
                if (count >= row_width)
                {
                    m_nStartTabCount = i;
                    break;
                }
            }

            m_nEndTabCount = nPages - m_nStartTabCount;
        }

        //move the tabs to the overflow notebook
        int i = 0;
        int nOverFlowPages = m_nStartTabCount;
        while (nOverFlowPages)
        {
            OString sIdent(get_page_ident(m_pNotebook, 0));
            OUString sLabel(get_tab_label_text(m_pNotebook, 0));
            remove_page(m_pNotebook, sIdent);
            insert_page(m_pOverFlowNotebook, sIdent, sLabel, gtk_grid_new(), -1);
            GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pOverFlowNotebook,
                                                               gtk_notebook_get_nth_page(m_pOverFlowNotebook, i));
            gtk_widget_set_hexpand(pTabWidget, true);

            --nOverFlowPages;
            ++i;
        }

        for (i = 0; i < m_nEndTabCount; ++i)
        {
            GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pNotebook,
                                                               gtk_notebook_get_nth_page(m_pNotebook, i));
            gtk_widget_set_hexpand(pTabWidget, true);
        }

        // have to have some tab as the active tab of the overflow notebook
        append_useless_page(m_pOverFlowNotebook);
        gtk_notebook_set_current_page(m_pOverFlowNotebook, gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1);
        if (gtk_widget_has_focus(GTK_WIDGET(m_pOverFlowNotebook)))
            gtk_widget_grab_focus(GTK_WIDGET(m_pNotebook));

        // add this temporarily to the normal notebook to measure how wide
        // the row would be if switched to the other notebook
        append_useless_page(m_pNotebook);

        gtk_widget_get_preferred_size(GTK_WIDGET(m_pNotebook), nullptr, &size1);
        gtk_widget_get_preferred_size(GTK_WIDGET(m_pOverFlowNotebook), nullptr, &size2);

        auto nWidth = std::max(size1.width, size2.width);
        gtk_widget_set_size_request(GTK_WIDGET(m_pNotebook), nWidth, alloc.height);
        gtk_widget_set_size_request(GTK_WIDGET(m_pOverFlowNotebook), nWidth, -1);

        // remove it once we've measured it
        remove_page(m_pNotebook, "useless");

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pOverFlowNotebook));
        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pNotebook));
#else
        g_object_thaw_notify(G_OBJECT(m_pOverFlowNotebook));
        g_object_thaw_notify(G_OBJECT(m_pNotebook));
#endif

        m_bOverFlowBoxActive = true;
    }

    static gboolean launch_split_notebooks(GtkInstanceNotebook* pThis)
    {
        int nCurrentPage = pThis->get_current_page();
        pThis->split_notebooks();
        pThis->set_current_page(nCurrentPage);
        pThis->m_nLaunchSplitTimeoutId = 0;
        return false;
    }

    // tdf#120371
    // https://developer.gnome.org/hig-book/unstable/controls-notebooks.html.en#controls-too-many-tabs
    // if no of tabs > 6, but only if the notebook would auto-scroll, then split the tabs over
    // two notebooks. Checking for the auto-scroll allows themes like Ambience under Ubuntu 16.04 to keep
    // tabs in a single row when they would fit
    void signal_notebook_size_allocate()
    {
        if (m_bOverFlowBoxActive || m_nLaunchSplitTimeoutId)
            return;
        disable_notify_events();
        gint nPages = gtk_notebook_get_n_pages(m_pNotebook);
        if (nPages > 6 && gtk_notebook_get_tab_pos(m_pNotebook) == GTK_POS_TOP)
        {
            for (gint i = 0; i < nPages; ++i)
            {
                GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pNotebook, gtk_notebook_get_nth_page(m_pNotebook, i));
#if GTK_CHECK_VERSION(4, 0, 0)
                bool bTabVisible = gtk_widget_get_child_visible(gtk_widget_get_parent(pTabWidget));
#else
                bool bTabVisible = gtk_widget_get_child_visible(pTabWidget);
#endif
                if (!bTabVisible)
                {
                    m_nLaunchSplitTimeoutId = g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, reinterpret_cast<GSourceFunc>(launch_split_notebooks), this, nullptr);
                    break;
                }
            }
        }
        enable_notify_events();
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    DECL_LINK(SizeAllocateHdl, void*, void);
#else
    static void signalSizeAllocate(GtkWidget*, GdkRectangle*, gpointer widget)
    {
        GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
        pThis->signal_notebook_size_allocate();
    }
#endif

    bool signal_focus(GtkDirectionType direction)
    {
        if (!m_bOverFlowBoxActive)
            return false;

        int nPage = gtk_notebook_get_current_page(m_pNotebook);
        if (direction == GTK_DIR_LEFT && nPage == 0)
        {
            auto nOverFlowLen = gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1;
            gtk_notebook_set_current_page(m_pOverFlowNotebook, nOverFlowLen - 1);
            return true;
        }
        else if (direction == GTK_DIR_RIGHT && nPage == gtk_notebook_get_n_pages(m_pNotebook) - 1)
        {
            gtk_notebook_set_current_page(m_pOverFlowNotebook, 0);
            return true;
        }

        return false;
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalFocus(GtkNotebook* notebook, GtkDirectionType direction, gpointer widget)
    {
        // if the notebook widget itself has focus
        if (gtk_widget_is_focus(GTK_WIDGET(notebook)))
        {
            GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
            return pThis->signal_focus(direction);
        }
        return false;
    }
#endif

    // ctrl + page_up/ page_down
    bool signal_change_current_page(gint arg1)
    {
        bool bHandled = signal_focus(arg1 < 0 ? GTK_DIR_LEFT : GTK_DIR_RIGHT);
        if (bHandled)
            g_signal_stop_emission_by_name(m_pNotebook, "change-current-page");
        return false;
    }

    static gboolean signalChangeCurrentPage(GtkNotebook*, gint arg1, gpointer widget)
    {
        if (arg1 == 0)
            return true;
        GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
        return pThis->signal_change_current_page(arg1);
    }

public:
    GtkInstanceNotebook(GtkNotebook* pNotebook, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pNotebook), pBuilder, bTakeOwnership)
        , m_pNotebook(pNotebook)
        , m_pOverFlowBox(nullptr)
        , m_pOverFlowNotebook(GTK_NOTEBOOK(gtk_notebook_new()))
        , m_nSwitchPageSignalId(g_signal_connect(pNotebook, "switch-page", G_CALLBACK(signalSwitchPage), this))
        , m_nOverFlowSwitchPageSignalId(g_signal_connect(m_pOverFlowNotebook, "switch-page", G_CALLBACK(signalOverFlowSwitchPage), this))
#if GTK_CHECK_VERSION(4, 0, 0)
        , m_pLayout(nullptr)
#else
        , m_nNotebookSizeAllocateSignalId(0)
        , m_nFocusSignalId(g_signal_connect(pNotebook, "focus", G_CALLBACK(signalFocus), this))
#endif
        , m_nChangeCurrentPageId(g_signal_connect(pNotebook, "change-current-page", G_CALLBACK(signalChangeCurrentPage), this))
        , m_nLaunchSplitTimeoutId(0)
        , m_bOverFlowBoxActive(false)
        , m_bOverFlowBoxIsStart(false)
        , m_bInternalPageChange(false)
        , m_nStartTabCount(0)
        , m_nEndTabCount(0)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_add_events(GTK_WIDGET(pNotebook), GDK_SCROLL_MASK);
#endif
        gint nPages = gtk_notebook_get_n_pages(m_pNotebook);
        if (nPages > 6)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            m_nNotebookSizeAllocateSignalId = g_signal_connect_after(pNotebook, "size-allocate", G_CALLBACK(signalSizeAllocate), this);
#else
            m_pLayout = NOTIFYING_LAYOUT(g_object_new(notifying_layout_get_type(), nullptr));
            notifying_layout_start_watch(m_pLayout, GTK_WIDGET(pNotebook), LINK(this, GtkInstanceNotebook, SizeAllocateHdl));
#endif
        }
        gtk_notebook_set_show_border(m_pOverFlowNotebook, false);

        // tdf#122623 it's nigh impossible to have a GtkNotebook without an active (checked) tab, so try and theme
        // the unwanted tab into invisibility via the 'overflow' class themed by global CreateStyleProvider
        GtkStyleContext *pNotebookContext = gtk_widget_get_style_context(GTK_WIDGET(m_pOverFlowNotebook));
        gtk_style_context_add_class(pNotebookContext, "overflow");
    }

    virtual int get_current_page() const override
    {
        int nPage = gtk_notebook_get_current_page(m_pNotebook);
        if (nPage == -1)
            return nPage;
        if (m_bOverFlowBoxIsStart)
        {
            auto nOverFlowLen = m_bOverFlowBoxActive ? gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1 : 0;
            // add count of overflow pages, minus the extra tab
            nPage += nOverFlowLen;
        }
        return nPage;
    }

    virtual OString get_page_ident(int nPage) const override
    {
        auto nMainLen = gtk_notebook_get_n_pages(m_pNotebook);
        auto nOverFlowLen = m_bOverFlowBoxActive ? gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1 : 0;
        if (m_bOverFlowBoxIsStart)
        {
            if (nPage < nOverFlowLen)
                return get_page_ident(m_pOverFlowNotebook, nPage);
            nPage -= nOverFlowLen;
            return get_page_ident(m_pNotebook, nPage);
        }
        else
        {
            if (nPage < nMainLen)
                return get_page_ident(m_pNotebook, nPage);
            nPage -= nMainLen;
            return get_page_ident(m_pOverFlowNotebook, nPage);
        }
    }

    virtual OString get_current_page_ident() const override
    {
        const int nPage = get_current_page();
        return nPage != -1 ? get_page_ident(nPage) : OString();
    }

    virtual int get_page_index(const OString& rIdent) const override
    {
        auto nMainIndex = get_page_number(m_pNotebook, rIdent);
        auto nOverFlowIndex = get_page_number(m_pOverFlowNotebook, rIdent);

        if (nMainIndex == -1 && nOverFlowIndex == -1)
            return -1;

        if (m_bOverFlowBoxIsStart)
        {
            if (nOverFlowIndex != -1)
                return nOverFlowIndex;
            else
            {
                auto nOverFlowLen = m_bOverFlowBoxActive ? gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1 : 0;
                return nMainIndex + nOverFlowLen;
            }
        }
        else
        {
            if (nMainIndex != -1)
                return nMainIndex;
            else
            {
                auto nMainLen = gtk_notebook_get_n_pages(m_pNotebook);
                return nOverFlowIndex + nMainLen;
            }
        }
    }

    virtual weld::Container* get_page(const OString& rIdent) const override
    {
        int nPage = get_page_index(rIdent);
        if (nPage < 0)
            return nullptr;

        GtkWidget* pChild;
        if (m_bOverFlowBoxIsStart)
        {
            auto nOverFlowLen = m_bOverFlowBoxActive ? gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1 : 0;
            if (nPage < nOverFlowLen)
                pChild = gtk_notebook_get_nth_page(m_pOverFlowNotebook, nPage);
            else
            {
                nPage -= nOverFlowLen;
                pChild = gtk_notebook_get_nth_page(m_pNotebook, nPage);
            }
        }
        else
        {
            auto nMainLen = gtk_notebook_get_n_pages(m_pNotebook);
            if (nPage < nMainLen)
                pChild = gtk_notebook_get_nth_page(m_pNotebook, nPage);
            else
            {
                nPage -= nMainLen;
                pChild = gtk_notebook_get_nth_page(m_pOverFlowNotebook, nPage);
            }
        }

        unsigned int nPageIndex = static_cast<unsigned int>(nPage);
        if (m_aPages.size() < nPageIndex + 1)
            m_aPages.resize(nPageIndex + 1);
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new GtkInstanceContainer(GTK_CONTAINER(pChild), m_pBuilder, false));
#else
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new GtkInstanceContainer(pChild, m_pBuilder, false));
#endif
        return m_aPages[nPageIndex].get();
    }

    virtual void set_current_page(int nPage) override
    {
        // normally we'd call disable_notify_events/enable_notify_events here,
        // but the notebook is complicated by the need to support the
        // double-decker hackery so for simplicity just flag that the page
        // change is not a directly user-triggered one
        bool bInternalPageChange = m_bInternalPageChange;
        m_bInternalPageChange = true;

        if (m_bOverFlowBoxIsStart)
        {
            auto nOverFlowLen = m_bOverFlowBoxActive ? gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1 : 0;
            if (nPage < nOverFlowLen)
                gtk_notebook_set_current_page(m_pOverFlowNotebook, nPage);
            else
            {
                nPage -= nOverFlowLen;
                gtk_notebook_set_current_page(m_pNotebook, nPage);
            }
        }
        else
        {
            auto nMainLen = gtk_notebook_get_n_pages(m_pNotebook);
            if (nPage < nMainLen)
                gtk_notebook_set_current_page(m_pNotebook, nPage);
            else
            {
                nPage -= nMainLen;
                gtk_notebook_set_current_page(m_pOverFlowNotebook, nPage);
            }
        }

        m_bInternalPageChange = bInternalPageChange;
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        gint nPage = get_page_index(rIdent);
        set_current_page(nPage);
    }

    virtual int get_n_pages() const override
    {
        int nLen = gtk_notebook_get_n_pages(m_pNotebook);
        if (m_bOverFlowBoxActive)
            nLen += gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1;
        return nLen;
    }

    virtual OUString get_tab_label_text(const OString& rIdent) const override
    {
        gint nPageNum = get_page_number(m_pNotebook, rIdent);
        if (nPageNum != -1)
            return get_tab_label_text(m_pNotebook, nPageNum);
        nPageNum = get_page_number(m_pOverFlowNotebook, rIdent);
        if (nPageNum != -1)
            return get_tab_label_text(m_pOverFlowNotebook, nPageNum);
        return OUString();
    }

    virtual void set_tab_label_text(const OString& rIdent, const OUString& rText) override
    {
        gint nPageNum = get_page_number(m_pNotebook, rIdent);
        if (nPageNum != -1)
        {
            set_tab_label_text(m_pNotebook, nPageNum, rText);
            return;
        }
        nPageNum = get_page_number(m_pOverFlowNotebook, rIdent);
        if (nPageNum != -1)
        {
            set_tab_label_text(m_pOverFlowNotebook, nPageNum, rText);
        }
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pNotebook, m_nSwitchPageSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_block(m_pNotebook, m_nFocusSignalId);
#endif
        g_signal_handler_block(m_pNotebook, m_nChangeCurrentPageId);
        g_signal_handler_block(m_pOverFlowNotebook, m_nOverFlowSwitchPageSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pOverFlowNotebook));
#endif
        g_object_freeze_notify(G_OBJECT(m_pOverFlowNotebook));
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_object_thaw_notify(G_OBJECT(m_pOverFlowNotebook));
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pOverFlowNotebook));
#endif
        g_signal_handler_unblock(m_pOverFlowNotebook, m_nOverFlowSwitchPageSignalId);
        g_signal_handler_unblock(m_pNotebook, m_nSwitchPageSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_unblock(m_pNotebook, m_nFocusSignalId);
#endif
        g_signal_handler_unblock(m_pNotebook, m_nChangeCurrentPageId);
    }

    void reset_split_data()
    {
        // reset overflow and allow it to be recalculated if necessary
        gtk_widget_hide(GTK_WIDGET(m_pOverFlowNotebook));
        m_bOverFlowBoxActive = false;
        m_nStartTabCount = 0;
        m_nEndTabCount = 0;
    }

    virtual void remove_page(const OString& rIdent) override
    {
        if (m_bOverFlowBoxActive)
        {
            unsplit_notebooks();
            reset_split_data();
        }

        unsigned int nPageIndex = remove_page(m_pNotebook, rIdent);
        if (nPageIndex < m_aPages.size())
            m_aPages.erase(m_aPages.begin() + nPageIndex);
    }

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override
    {
        if (m_bOverFlowBoxActive)
        {
            unsplit_notebooks();
            reset_split_data();
        }

        // reset overflow and allow it to be recalculated if necessary
        gtk_widget_hide(GTK_WIDGET(m_pOverFlowNotebook));
        m_bOverFlowBoxActive = false;

        insert_page(m_pNotebook, rIdent, rLabel, gtk_grid_new(), nPos);
    }

    virtual ~GtkInstanceNotebook() override
    {
        if (m_nLaunchSplitTimeoutId)
            g_source_remove(m_nLaunchSplitTimeoutId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (m_nNotebookSizeAllocateSignalId)
            g_signal_handler_disconnect(m_pNotebook, m_nNotebookSizeAllocateSignalId);
#else
        if (m_pLayout)
        {
            // put it back how we found it initially
            notifying_layout_stop_watch(m_pLayout);
        }
#endif
        g_signal_handler_disconnect(m_pNotebook, m_nSwitchPageSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pNotebook, m_nFocusSignalId);
#endif
        g_signal_handler_disconnect(m_pNotebook, m_nChangeCurrentPageId);
        g_signal_handler_disconnect(m_pOverFlowNotebook, m_nOverFlowSwitchPageSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_destroy(GTK_WIDGET(m_pOverFlowNotebook));
#else
        GtkWidget* pOverFlowWidget = GTK_WIDGET(m_pOverFlowNotebook);
        g_clear_pointer(&pOverFlowWidget, gtk_widget_unparent);
#endif
        if (!m_pOverFlowBox)
            return;

        // put it back to how we found it initially
        GtkWidget* pParent = gtk_widget_get_parent(GTK_WIDGET(m_pOverFlowBox));
        g_object_ref(m_pNotebook);
        container_remove(GTK_WIDGET(m_pOverFlowBox), GTK_WIDGET(m_pNotebook));
        container_add(GTK_WIDGET(pParent), GTK_WIDGET(m_pNotebook));
        g_object_unref(m_pNotebook);

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_destroy(GTK_WIDGET(m_pOverFlowBox));
#else
        GtkWidget* pOverFlowBox = GTK_WIDGET(m_pOverFlowBox);
        g_clear_pointer(&pOverFlowBox, gtk_widget_unparent);
#endif
    }
};

#if GTK_CHECK_VERSION(4, 0, 0)
IMPL_LINK_NOARG(GtkInstanceNotebook, SizeAllocateHdl, void*, void)
{
    signal_notebook_size_allocate();
}
#endif


OUString vcl_font_to_css(const vcl::Font& rFont)
{
    OUStringBuffer sCSS;
    sCSS.append("font-family: \"" + rFont.GetFamilyName() + "\"; ");
    sCSS.append("font-size: " + OUString::number(rFont.GetFontSize().Height()) + "pt; ");
    switch (rFont.GetItalic())
    {
        case ITALIC_NONE:
            sCSS.append("font-style: normal; ");
            break;
        case ITALIC_NORMAL:
            sCSS.append("font-style: italic; ");
            break;
        case ITALIC_OBLIQUE:
            sCSS.append("font-style: oblique; ");
            break;
        default:
            break;
    }
    switch (rFont.GetWeight())
    {
        case WEIGHT_ULTRALIGHT:
            sCSS.append("font-weight: 200; ");
            break;
        case WEIGHT_LIGHT:
            sCSS.append("font-weight: 300; ");
            break;
        case WEIGHT_NORMAL:
            sCSS.append("font-weight: 400; ");
            break;
        case WEIGHT_BOLD:
            sCSS.append("font-weight: 700; ");
            break;
        case WEIGHT_ULTRABOLD:
            sCSS.append("font-weight: 800; ");
            break;
        default:
            break;
    }
    switch (rFont.GetWidthType())
    {
        case WIDTH_ULTRA_CONDENSED:
            sCSS.append("font-stretch: ultra-condensed; ");
            break;
        case WIDTH_EXTRA_CONDENSED:
            sCSS.append("font-stretch: extra-condensed; ");
            break;
        case WIDTH_CONDENSED:
            sCSS.append("font-stretch: condensed; ");
            break;
        case WIDTH_SEMI_CONDENSED:
            sCSS.append("font-stretch: semi-condensed; ");
            break;
        case WIDTH_NORMAL:
            sCSS.append("font-stretch: normal; ");
            break;
        case WIDTH_SEMI_EXPANDED:
            sCSS.append("font-stretch: semi-expanded; ");
            break;
        case WIDTH_EXPANDED:
            sCSS.append("font-stretch: expanded; ");
            break;
        case WIDTH_EXTRA_EXPANDED:
            sCSS.append("font-stretch: extra-expanded; ");
            break;
        case WIDTH_ULTRA_EXPANDED:
            sCSS.append("font-stretch: ultra-expanded; ");
            break;
        default:
            break;
    }
    return sCSS.toString();
}

void update_attr_list(PangoAttrList* pAttrList, const vcl::Font& rFont)
{
    pango_attr_list_change(pAttrList, pango_attr_family_new(OUStringToOString(rFont.GetFamilyName(), RTL_TEXTENCODING_UTF8).getStr()));
    pango_attr_list_change(pAttrList, pango_attr_size_new(rFont.GetFontSize().Height() * PANGO_SCALE));

    switch (rFont.GetItalic())
    {
        case ITALIC_NONE:
            pango_attr_list_change(pAttrList, pango_attr_style_new(PANGO_STYLE_NORMAL));
            break;
        case ITALIC_NORMAL:
            pango_attr_list_change(pAttrList, pango_attr_style_new(PANGO_STYLE_ITALIC));
            break;
        case ITALIC_OBLIQUE:
            pango_attr_list_change(pAttrList, pango_attr_style_new(PANGO_STYLE_OBLIQUE));
            break;
        default:
            break;
    }
    switch (rFont.GetWeight())
    {
        case WEIGHT_ULTRALIGHT:
            pango_attr_list_change(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_ULTRALIGHT));
            break;
        case WEIGHT_LIGHT:
            pango_attr_list_change(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_LIGHT));
            break;
        case WEIGHT_NORMAL:
            pango_attr_list_change(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_NORMAL));
            break;
        case WEIGHT_BOLD:
            pango_attr_list_change(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
            break;
        case WEIGHT_ULTRABOLD:
            pango_attr_list_change(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_ULTRABOLD));
            break;
        default:
            break;
    }
    switch (rFont.GetWidthType())
    {
        case WIDTH_ULTRA_CONDENSED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_ULTRA_CONDENSED));
            break;
        case WIDTH_EXTRA_CONDENSED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_EXTRA_CONDENSED));
            break;
        case WIDTH_CONDENSED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_CONDENSED));
            break;
        case WIDTH_SEMI_CONDENSED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_SEMI_CONDENSED));
            break;
        case WIDTH_NORMAL:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_NORMAL));
            break;
        case WIDTH_SEMI_EXPANDED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_SEMI_EXPANDED));
            break;
        case WIDTH_EXPANDED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_EXPANDED));
            break;
        case WIDTH_EXTRA_EXPANDED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_EXTRA_EXPANDED));
            break;
        case WIDTH_ULTRA_EXPANDED:
            pango_attr_list_change(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_ULTRA_EXPANDED));
            break;
        default:
            break;
    }
}

gboolean filter_pango_attrs(PangoAttribute *attr, gpointer data)
{
    PangoAttrType* pFilterAttrs = static_cast<PangoAttrType*>(data);
    while (*pFilterAttrs)
    {
        if (attr->klass->type == *pFilterAttrs)
            return true;
        ++pFilterAttrs;
    }
    return false;
}

void set_font(GtkLabel* pLabel, const vcl::Font& rFont)
{
    PangoAttrList* pOrigList = gtk_label_get_attributes(pLabel);
    PangoAttrList* pAttrList = pOrigList ? pango_attr_list_copy(pOrigList) : pango_attr_list_new();

    if (pOrigList)
    {
        // tdf#143443 remove both PANGO_ATTR_ABSOLUTE_SIZE and PANGO_ATTR_SIZE
        // because pango_attr_list_change(..., pango_attr_size_new...) isn't
        // sufficient on its own to ensure a new size sticks.
        PangoAttrType aFilterAttrs[] = {PANGO_ATTR_ABSOLUTE_SIZE, PANGO_ATTR_SIZE, PANGO_ATTR_INVALID};
        PangoAttrList* pRemovedAttrs = pango_attr_list_filter(pAttrList, filter_pango_attrs, &aFilterAttrs);
        pango_attr_list_unref(pRemovedAttrs);
    }

    update_attr_list(pAttrList, rFont);
    gtk_label_set_attributes(pLabel, pAttrList);
    pango_attr_list_unref(pAttrList);
}

}

namespace {

class WidgetBackground
{
private:
    GtkWidget* m_pWidget;
    GtkCssProvider* m_pCustomCssProvider;
    std::unique_ptr<utl::TempFile> m_xCustomImage;

public:
    // See: https://developer.gnome.org/Buttons/
    void use_custom_content(const VirtualDevice* pDevice)
    {
        GtkStyleContext *pWidgetContext = gtk_widget_get_style_context(m_pWidget);

        if (m_pCustomCssProvider)
        {
            gtk_style_context_remove_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pCustomCssProvider));
            m_pCustomCssProvider = nullptr;
        }

        m_xCustomImage.reset();

        if (!pDevice)
            return;

        m_xCustomImage.reset(new utl::TempFile);
        m_xCustomImage->EnableKillingFile(true);

        cairo_surface_t* surface = get_underlying_cairo_surface(*pDevice);
        Size aSize = pDevice->GetOutputSizePixel();
        cairo_surface_write_to_png(surface, OUStringToOString(m_xCustomImage->GetFileName(), osl_getThreadTextEncoding()).getStr());

        m_pCustomCssProvider = gtk_css_provider_new();
        OUString aBuffer = "* { background-image: url(\"" + m_xCustomImage->GetURL() + "\"); "
                           "background-size: " + OUString::number(aSize.Width()) + "px " + OUString::number(aSize.Height()) + "px; "
                           "border-radius: 0; border-width: 0; }";
        OString aResult = OUStringToOString(aBuffer, RTL_TEXTENCODING_UTF8);
        css_provider_load_from_data(m_pCustomCssProvider, aResult.getStr(), aResult.getLength());
        gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pCustomCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

public:
    WidgetBackground(GtkWidget* pWidget)
        : m_pWidget(pWidget)
        , m_pCustomCssProvider(nullptr)
    {
    }

    ~WidgetBackground()
    {
        if (m_pCustomCssProvider)
            use_custom_content(nullptr);
        assert(!m_pCustomCssProvider);
    }
};

class WidgetFont
{
private:
    GtkWidget* m_pWidget;
    GtkCssProvider* m_pFontCssProvider;
    std::unique_ptr<vcl::Font> m_xFont;
public:
    WidgetFont(GtkWidget* pWidget)
        : m_pWidget(pWidget)
        , m_pFontCssProvider(nullptr)
    {
    }

    void use_custom_font(const vcl::Font* pFont, std::u16string_view rCSSSelector)
    {
        GtkStyleContext *pWidgetContext = gtk_widget_get_style_context(m_pWidget);
        if (m_pFontCssProvider)
        {
            gtk_style_context_remove_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pFontCssProvider));
            m_pFontCssProvider = nullptr;
        }

        m_xFont.reset();

        if (!pFont)
            return;

        m_xFont.reset(new vcl::Font(*pFont));
        m_pFontCssProvider = gtk_css_provider_new();
        OUString aBuffer = rCSSSelector + OUString::Concat(" { ") + vcl_font_to_css(*pFont) + OUString::Concat(" }");
        OString aResult = OUStringToOString(aBuffer, RTL_TEXTENCODING_UTF8);
        css_provider_load_from_data(m_pFontCssProvider, aResult.getStr(), aResult.getLength());
        gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pFontCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    const vcl::Font* get_custom_font() const
    {
        return m_xFont.get();
    }

    ~WidgetFont()
    {
        if (m_pFontCssProvider)
            use_custom_font(nullptr, u"");
        assert(!m_pFontCssProvider);
    }
};

class GtkInstanceButton : public GtkInstanceWidget, public virtual weld::Button
{
private:
    GtkButton* m_pButton;
    gulong m_nSignalId;
    std::optional<vcl::Font> m_xFont;
    WidgetBackground m_aCustomBackground;

    static void signalClicked(GtkButton*, gpointer widget)
    {
        GtkInstanceButton* pThis = static_cast<GtkInstanceButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_clicked();
    }

    virtual void ensureMouseEventWidget() override
    {
        // The GtkButton is sufficient to get mouse events without an intermediate GtkEventBox
        if (!m_pMouseEventBox)
            m_pMouseEventBox = m_pWidget;
    }

public:
    GtkInstanceButton(GtkButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_nSignalId(g_signal_connect(pButton, "clicked", G_CALLBACK(signalClicked), this))
        , m_aCustomBackground(GTK_WIDGET(pButton))
    {
        g_object_set_data(G_OBJECT(m_pButton), "g-lo-GtkInstanceButton", this);
    }

    virtual void set_label(const OUString& rText) override
    {
        ::button_set_label(m_pButton, rText);
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        ::button_set_image(m_pButton, pDevice);
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        ::button_set_from_icon_name(m_pButton, rIconName);
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        ::button_set_image(m_pButton, rImage);
    }

    virtual void set_custom_button(VirtualDevice* pDevice) override
    {
        m_aCustomBackground.use_custom_content(pDevice);
    }

    virtual OUString get_label() const override
    {
        return ::button_get_label(m_pButton);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_xFont = rFont;
        GtkLabel* pChild = ::get_label_widget(GTK_WIDGET(m_pButton));
        ::set_font(pChild, rFont);
    }

    virtual vcl::Font get_font() override
    {
        if (m_xFont)
            return *m_xFont;
        return GtkInstanceWidget::get_font();
    }

    // allow us to block buttons with click handlers making dialogs return a response
    bool has_click_handler() const
    {
        return m_aClickHdl.IsSet();
    }

    void clear_click_handler()
    {
        m_aClickHdl = Link<Button&, void>();
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pButton, m_nSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pButton, m_nSignalId);
    }

    virtual ~GtkInstanceButton() override
    {
        g_object_steal_data(G_OBJECT(m_pButton), "g-lo-GtkInstanceButton");
        g_signal_handler_disconnect(m_pButton, m_nSignalId);
    }
};

}

void GtkInstanceDialog::asyncresponse(gint ret)
{
    SolarMutexGuard aGuard;

    if (ret == GTK_RESPONSE_HELP)
    {
        help();
        return;
    }

    GtkInstanceButton* pClickHandler = has_click_handler(ret);
    if (pClickHandler)
    {
        // make GTK_RESPONSE_DELETE_EVENT act as if cancel button was pressed
        if (ret == GTK_RESPONSE_DELETE_EVENT)
            close(false);
        return;
    }

    if (get_modal())
        m_aDialogRun.dec_modal_count();
    hide();

    // move the self pointer, otherwise it might be de-allocated by time we try to reset it
    auto xRunAsyncSelf = std::move(m_xRunAsyncSelf);
    auto xDialogController = std::move(m_xDialogController);
    auto aFunc = std::move(m_aFunc);

    auto nResponseSignalId = m_nResponseSignalId;
    auto nCancelSignalId = m_nCancelSignalId;
    auto nSignalDeleteId = m_nSignalDeleteId;
    m_nResponseSignalId = 0;
    m_nCancelSignalId = 0;
    m_nSignalDeleteId = 0;

    if (aFunc)
        aFunc(GtkToVcl(ret));

    if (nResponseSignalId)
        g_signal_handler_disconnect(m_pDialog, nResponseSignalId);
    if (nCancelSignalId)
        g_signal_handler_disconnect(m_pDialog, nCancelSignalId);
    if (nSignalDeleteId)
        g_signal_handler_disconnect(m_pDialog, nSignalDeleteId);

    xDialogController.reset();
    xRunAsyncSelf.reset();
}

int GtkInstanceDialog::run()
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    if (GTK_IS_DIALOG(m_pDialog))
        sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog))));
#endif
    int ret;
    while (true)
    {
        ret = m_aDialogRun.run();
        if (ret == GTK_RESPONSE_HELP)
        {
            help();
            continue;
        }
        else if (has_click_handler(ret))
            continue;
        break;
    }
    hide();
    return GtkToVcl(ret);
}

weld::Button* GtkInstanceDialog::weld_widget_for_response(int nVclResponse)
{
    GtkButton* pButton = get_widget_for_response(VclToGtk(nVclResponse));
    if (!pButton)
        return nullptr;
    return new GtkInstanceButton(pButton, m_pBuilder, false);
}

void GtkInstanceDialog::response(int nResponse)
{
    int nGtkResponse = VclToGtk(nResponse);
    //unblock this response now when activated through code
    if (GtkButton* pWidget = get_widget_for_response(nGtkResponse))
    {
        void* pData = g_object_get_data(G_OBJECT(pWidget), "g-lo-GtkInstanceButton");
        GtkInstanceButton* pButton = static_cast<GtkInstanceButton*>(pData);
        if (pButton)
            pButton->clear_click_handler();
    }
    if (GTK_IS_DIALOG(m_pDialog))
        gtk_dialog_response(GTK_DIALOG(m_pDialog), nGtkResponse);
    else if (GTK_IS_ASSISTANT(m_pDialog))
    {
        if (!m_aDialogRun.loop_is_running())
            asyncresponse(nGtkResponse);
        else
        {
            m_aDialogRun.m_nResponseId = nGtkResponse;
            m_aDialogRun.loop_quit();
        }
    }
}

void GtkInstanceDialog::close(bool bCloseSignal)
{
    GtkInstanceButton* pClickHandler = has_click_handler(GTK_RESPONSE_CANCEL);
    if (pClickHandler)
    {
        if (bCloseSignal)
            g_signal_stop_emission_by_name(m_pDialog, "close");
        // make esc (bCloseSignal == true) or window-delete (bCloseSignal == false)
        // act as if cancel button was pressed
        pClickHandler->clicked();
        return;
    }
    response(RET_CANCEL);
}

GtkInstanceButton* GtkInstanceDialog::has_click_handler(int nResponse)
{
    GtkInstanceButton* pButton = nullptr;
    // e.g. map GTK_RESPONSE_DELETE_EVENT to GTK_RESPONSE_CANCEL
    nResponse = VclToGtk(GtkToVcl(nResponse));
    if (GtkButton* pWidget = get_widget_for_response(nResponse))
    {
        void* pData = g_object_get_data(G_OBJECT(pWidget), "g-lo-GtkInstanceButton");
        pButton = static_cast<GtkInstanceButton*>(pData);
        if (pButton && !pButton->has_click_handler())
            pButton = nullptr;
    }
    return pButton;
}

namespace {

class GtkInstanceToggleButton : public GtkInstanceButton, public virtual weld::ToggleButton
{
protected:
    GtkToggleButton* m_pToggleButton;
    gulong m_nToggledSignalId;
private:
    static void signalToggled(GtkToggleButton*, gpointer widget)
    {
        GtkInstanceToggleButton* pThis = static_cast<GtkInstanceToggleButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_toggled();
    }
public:
    GtkInstanceToggleButton(GtkToggleButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceButton(GTK_BUTTON(pButton), pBuilder, bTakeOwnership)
        , m_pToggleButton(pButton)
        , m_nToggledSignalId(g_signal_connect(m_pToggleButton, "toggled", G_CALLBACK(signalToggled), this))
    {
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
        set_inconsistent(false);
        gtk_toggle_button_set_active(m_pToggleButton, active);
        enable_notify_events();
    }

    virtual bool get_active() const override
    {
        return gtk_toggle_button_get_active(m_pToggleButton);
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        if (inconsistent)
            gtk_widget_set_state_flags(GTK_WIDGET(m_pToggleButton), GTK_STATE_FLAG_INCONSISTENT, false);
        else
            gtk_widget_unset_state_flags(GTK_WIDGET(m_pToggleButton), GTK_STATE_FLAG_INCONSISTENT);
#else
        gtk_toggle_button_set_inconsistent(m_pToggleButton, inconsistent);
#endif
    }

    virtual bool get_inconsistent() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return gtk_widget_get_state_flags(GTK_WIDGET(m_pToggleButton)) & GTK_STATE_FLAG_INCONSISTENT;
#else
        return gtk_toggle_button_get_inconsistent(m_pToggleButton);
#endif
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pToggleButton, m_nToggledSignalId);
        GtkInstanceButton::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceButton::enable_notify_events();
        g_signal_handler_unblock(m_pToggleButton, m_nToggledSignalId);
    }

    virtual ~GtkInstanceToggleButton() override
    {
        g_signal_handler_disconnect(m_pToggleButton, m_nToggledSignalId);
    }
};

}

#if !GTK_CHECK_VERSION(4, 0, 0)

namespace {

void do_grab(GtkWidget* pWidget)
{
    GdkDisplay *pDisplay = gtk_widget_get_display(pWidget);
    GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
    gdk_seat_grab(pSeat, widget_get_surface(pWidget),
                  GDK_SEAT_CAPABILITY_KEYBOARD, true, nullptr, nullptr, nullptr, nullptr);
}

void do_ungrab(GtkWidget* pWidget)
{
    GdkDisplay *pDisplay = gtk_widget_get_display(pWidget);
    GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
    gdk_seat_ungrab(pSeat);
}

GtkPositionType show_menu_older_gtk(GtkWidget* pMenuButton, GtkWindow* pMenu, const GdkRectangle& rAnchor, weld::Placement ePlace)
{
    //place the toplevel just below its launcher button
    GtkWidget* pToplevel = widget_get_toplevel(pMenuButton);
    gtk_coord x, y, absx, absy;
    gtk_widget_translate_coordinates(pMenuButton, pToplevel, rAnchor.x, rAnchor.y, &x, &y);
    GdkSurface* pWindow = widget_get_surface(pToplevel);
    gdk_window_get_position(pWindow, &absx, &absy);

    x += absx;
    y += absy;

    gint nButtonHeight = rAnchor.height;
    gint nButtonWidth = rAnchor.width;
    if (ePlace == weld::Placement::Under)
        y += nButtonHeight;
    else
        x += nButtonWidth;

    gtk_window_set_attached_to(pMenu, pMenuButton);

    gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(pToplevel)), pMenu);
    gtk_window_set_transient_for(pMenu, GTK_WINDOW(pToplevel));

    gint nMenuWidth, nMenuHeight;
    gtk_widget_get_size_request(GTK_WIDGET(pMenu), &nMenuWidth, &nMenuHeight);

    if (nMenuWidth == -1 || nMenuHeight == -1)
    {
        GtkRequisition req;
        gtk_widget_get_preferred_size(GTK_WIDGET(pMenu), nullptr, &req);
        if (nMenuWidth == -1)
            nMenuWidth = req.width;
        if (nMenuHeight == -1)
            nMenuHeight = req.height;
    }

    bool bSwapForRTL = SwapForRTL(pMenuButton);
    if (bSwapForRTL)
    {
        if (ePlace == weld::Placement::Under)
            x += nButtonWidth;
        else
            x -= nButtonWidth;
        x -= nMenuWidth;
    }

    tools::Rectangle aWorkArea(::get_monitor_workarea(pMenuButton));

    // shrink it a little, I find it reassuring to see a little margin with a
    // long menu to know the menu is fully on screen
    aWorkArea.AdjustTop(8);
    aWorkArea.AdjustBottom(-8);
    aWorkArea.AdjustLeft(8);
    aWorkArea.AdjustRight(-8);

    GtkPositionType ePosUsed;

    if (ePlace == weld::Placement::Under)
    {
        gint endx = x + nMenuWidth;
        if (endx > aWorkArea.Right())
            x -= endx - aWorkArea.Right();
        if (x < 0)
            x = 0;

        ePosUsed = GTK_POS_BOTTOM;
        gint endy = y + nMenuHeight;
        gint nMissingBelow = endy - aWorkArea.Bottom();
        if (nMissingBelow > 0)
        {
            gint nNewY = y - (nButtonHeight + nMenuHeight);
            if (nNewY < aWorkArea.Top())
            {
                gint nMissingAbove = aWorkArea.Top() - nNewY;
                if (nMissingBelow <= nMissingAbove)
                    nMenuHeight -= nMissingBelow;
                else
                {
                    nMenuHeight -= nMissingAbove;
                    y = aWorkArea.Top();
                    ePosUsed = GTK_POS_TOP;
                }
                gtk_widget_set_size_request(GTK_WIDGET(pMenu), nMenuWidth, nMenuHeight);
            }
            else
            {
                y = nNewY;
                ePosUsed = GTK_POS_TOP;
            }
        }
    }
    else
    {
        if (!bSwapForRTL)
        {
            ePosUsed = GTK_POS_RIGHT;
            gint endx = x + nMenuWidth;
            gint nMissingAfter = endx - aWorkArea.Right();
            if (nMissingAfter > 0)
            {
                gint nNewX = x - (nButtonWidth + nMenuWidth);
                if (nNewX >= aWorkArea.Left())
                {
                    x = nNewX;
                    ePosUsed = GTK_POS_LEFT;
                }
            }
        }
        else
        {
            ePosUsed = GTK_POS_LEFT;
            gint startx = x;
            gint nMissingBefore = aWorkArea.Left() - startx;
            if (nMissingBefore > 0)
            {
                gint nNewX = x + (nButtonWidth + nMenuWidth);
                if (nNewX + nMenuWidth < aWorkArea.Right())
                {
                    x = nNewX;
                    ePosUsed = GTK_POS_RIGHT;
                }
            }
        }
    }

    gtk_window_move(pMenu, x, y);

    return ePosUsed;
}

bool show_menu_newer_gtk(GtkWidget* pComboBox, GtkWindow* pMenu, const GdkRectangle &rAnchor, weld::Placement ePlace)
{
    static auto window_move_to_rect = reinterpret_cast<void (*) (GdkWindow*, const GdkRectangle*, GdkGravity,
                                                                 GdkGravity, GdkAnchorHints, gint, gint)>(
                                                                    dlsym(nullptr, "gdk_window_move_to_rect"));
    if (!window_move_to_rect)
        return false;

    // under wayland gdk_window_move_to_rect works great for me, but in my current
    // gtk 3.24 under X it leaves part of long menus outside the work area
    GdkDisplay *pDisplay = gtk_widget_get_display(pComboBox);
    if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
        return false;

    //place the toplevel just below its launcher button
    GtkWidget* pToplevel = widget_get_toplevel(pComboBox);
    gtk_coord x, y;
    gtk_widget_translate_coordinates(pComboBox, pToplevel, rAnchor.x, rAnchor.y, &x, &y);

    gtk_window_set_attached_to(pMenu, pComboBox);

    gtk_widget_realize(GTK_WIDGET(pMenu));
    gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(pToplevel)), pMenu);
    gtk_window_set_transient_for(pMenu, GTK_WINDOW(pToplevel));

    bool bSwapForRTL = SwapForRTL(GTK_WIDGET(pComboBox));

    GdkGravity rect_anchor;
    GdkGravity menu_anchor;

    if (ePlace == weld::Placement::Under)
    {
        rect_anchor = !bSwapForRTL ? GDK_GRAVITY_SOUTH_WEST : GDK_GRAVITY_SOUTH_EAST;
        menu_anchor = !bSwapForRTL ? GDK_GRAVITY_NORTH_WEST : GDK_GRAVITY_NORTH_EAST;
    }
    else
    {
        rect_anchor = !bSwapForRTL ? GDK_GRAVITY_NORTH_EAST : GDK_GRAVITY_NORTH_WEST;
        menu_anchor = !bSwapForRTL ? GDK_GRAVITY_NORTH_WEST : GDK_GRAVITY_NORTH_EAST;
    }

    GdkAnchorHints anchor_hints = static_cast<GdkAnchorHints>(GDK_ANCHOR_FLIP | GDK_ANCHOR_SLIDE | GDK_ANCHOR_RESIZE);

    GdkRectangle rect {x, y, rAnchor.width, rAnchor.height};
    GdkSurface* toplevel = widget_get_surface(GTK_WIDGET(pMenu));

    window_move_to_rect(toplevel, &rect, rect_anchor, menu_anchor, anchor_hints,
                        0, 0);

    return true;
}

GtkPositionType show_menu(GtkWidget* pMenuButton, GtkWindow* pMenu, const GdkRectangle& rAnchor, weld::Placement ePlace)
{
    // we only use ePosUsed in the replacement-for-X-popover case of a
    // MenuButton, so we only need it when show_menu_older_gtk is used
    GtkPositionType ePosUsed = GTK_POS_BOTTOM;

    // tdf#120764 It isn't allowed under wayland to have two visible popups that share
    // the same top level parent. The problem is that since gtk 3.24 tooltips are also
    // implemented as popups, which means that we cannot show any popup if there is a
    // visible tooltip.
    GtkWidget* pParent = widget_get_toplevel(pMenuButton);
    GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
    if (pFrame)
    {
        // hide any current tooltip
        pFrame->HideTooltip();
        // don't allow any more to appear until menu is dismissed
        pFrame->BlockTooltip();
    }

    // try with gdk_window_move_to_rect, but if that's not available, try without
    if (!show_menu_newer_gtk(pMenuButton, pMenu, rAnchor, ePlace))
        ePosUsed = show_menu_older_gtk(pMenuButton, pMenu, rAnchor, ePlace);
    gtk_widget_show_all(GTK_WIDGET(pMenu));
    gtk_widget_grab_focus(GTK_WIDGET(pMenu));
    do_grab(GTK_WIDGET(pMenu));

    return ePosUsed;
}

}
#endif

namespace {

#if !GTK_CHECK_VERSION(4, 0, 0)
bool button_event_is_outside(GtkWidget* pMenuHack, GdkEventButton* pEvent)
{
    //we want to pop down if the button was released outside our popup
    gdouble x = pEvent->x_root;
    gdouble y = pEvent->y_root;

    gint window_x, window_y;
    GdkSurface* pWindow = widget_get_surface(pMenuHack);
    gdk_window_get_position(pWindow, &window_x, &window_y);

    GtkAllocation alloc;
    gtk_widget_get_allocation(pMenuHack, &alloc);
    gint x1 = window_x;
    gint y1 = window_y;
    gint x2 = x1 + alloc.width;
    gint y2 = y1 + alloc.height;

    if (x > x1 && x < x2 && y > y1 && y < y2)
        return false;

    return true;
}

GtkPositionType MovePopoverContentsToWindow(GtkWidget* pPopover, GtkWindow* pMenuHack, GtkWidget* pAnchor,
                                            const GdkRectangle& rAnchor, weld::Placement ePlace)
{
    //set border width
    gtk_container_set_border_width(GTK_CONTAINER(pMenuHack), gtk_container_get_border_width(GTK_CONTAINER(pPopover)));

    //steal popover contents and smuggle into toplevel display window
    GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(pPopover));
    g_object_ref(pChild);
    gtk_container_remove(GTK_CONTAINER(pPopover), pChild);
    gtk_container_add(GTK_CONTAINER(pMenuHack), pChild);
    g_object_unref(pChild);

    GtkPositionType eRet = show_menu(pAnchor, pMenuHack, rAnchor, ePlace);

    gtk_grab_add(GTK_WIDGET(pMenuHack));

    GdkSurface* pSurface = widget_get_surface(GTK_WIDGET(pMenuHack));
    g_object_set_data(G_OBJECT(pSurface), "g-lo-InstancePopup", GINT_TO_POINTER(true));

    return eRet;
}

void MoveWindowContentsToPopover(GtkWindow* pMenuHack, GtkWidget* pPopover, GtkWidget* pAnchor)
{
    bool bHadFocus = gtk_window_has_toplevel_focus(pMenuHack);

    do_ungrab(GTK_WIDGET(pMenuHack));

    gtk_grab_remove(GTK_WIDGET(pMenuHack));

    gtk_widget_hide(GTK_WIDGET(pMenuHack));
    //put contents back from where the came from
    GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(pMenuHack));
    g_object_ref(pChild);
    gtk_container_remove(GTK_CONTAINER(pMenuHack), pChild);
    gtk_container_add(GTK_CONTAINER(pPopover), pChild);
    g_object_unref(pChild);

    GdkSurface* pSurface = widget_get_surface(GTK_WIDGET(pMenuHack));
    g_object_set_data(G_OBJECT(pSurface), "g-lo-InstancePopup", GINT_TO_POINTER(false));

    // so gdk_window_move_to_rect will work again the next time
    gtk_widget_unrealize(GTK_WIDGET(pMenuHack));

    gtk_widget_set_size_request(GTK_WIDGET(pMenuHack), -1, -1);

    // undo show_menu tooltip blocking
    GtkWidget* pParent = widget_get_toplevel(pAnchor);
    GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
    if (pFrame)
        pFrame->UnblockTooltip();

    if (bHadFocus)
    {
        GdkSurface* pParentSurface = pParent ? widget_get_surface(pParent) : nullptr;
        void* pParentIsPopover = pParentSurface ? g_object_get_data(G_OBJECT(pParentSurface), "g-lo-InstancePopup") : nullptr;
        if (pParentIsPopover)
            do_grab(pAnchor);
        gtk_widget_grab_focus(pAnchor);
    }
}

#endif

/* four types of uses of this
   a) textual menubutton, always with pan-down symbol, e.g. math, format, font, modify
   b) image + text, always with additional pan-down symbol, e.g. writer, format, watermark
   c) gear menu, never with text and without pan-down symbol where there is a replacement
      icon for pan-down, e.g.  file, new, templates
   d) image, always with additional pan-down symbol, e.g. calc, insert, header/footer */
#if !GTK_CHECK_VERSION(4, 0, 0)
class GtkInstanceMenuButton : public GtkInstanceToggleButton, public MenuHelper, public virtual weld::MenuButton
#else
class GtkInstanceMenuButton : public GtkInstanceWidget, public MenuHelper, public virtual weld::MenuButton
#endif
{
protected:
    GtkMenuButton* m_pMenuButton;
private:
    GtkBox* m_pBox;
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkImage* m_pImage;
#else
    GtkPicture* m_pImage;
    GtkToggleButton* m_pMenuButtonToggleButton;
#endif
    GtkWidget* m_pLabel;
#if !GTK_CHECK_VERSION(4, 0, 0)
    //popover cannot escape dialog under X so stick up own window instead
    GtkWindow* m_pMenuHack;
    //when doing so, if it's a toolbar menubutton align the menu to the full toolitem
    GtkWidget* m_pMenuHackAlign;
    bool m_nButtonPressSeen;
    gulong m_nSignalId;
#endif
    GtkWidget* m_pPopover;
#if GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nToggledSignalId;
    std::optional<vcl::Font> m_xFont;
    WidgetBackground m_aCustomBackground;
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void signalMenuButtonToggled(GtkWidget*, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->menu_toggled();
    }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    void menu_toggled()
    {
        if (!m_pMenuHack)
            return;
        if (!get_active())
        {
            m_nButtonPressSeen = false;
            MoveWindowContentsToPopover(m_pMenuHack, m_pPopover, GTK_WIDGET(m_pMenuButton));
        }
        else
        {
            GtkWidget* pAnchor = m_pMenuHackAlign ? m_pMenuHackAlign : GTK_WIDGET(m_pMenuButton);
            GdkRectangle aAnchor {0, 0, gtk_widget_get_allocated_width(pAnchor), gtk_widget_get_allocated_height(pAnchor) };
            GtkPositionType ePosUsed = MovePopoverContentsToWindow(m_pPopover, m_pMenuHack, pAnchor, aAnchor, weld::Placement::Under);
            // tdf#132540 keep the placeholder popover on this same side as the replacement menu
            gtk_popover_set_position(gtk_menu_button_get_popover(m_pMenuButton), ePosUsed);
        }
    }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void signalGrabBroken(GtkWidget*, GdkEventGrabBroken *pEvent, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        pThis->grab_broken(pEvent);
    }

    void grab_broken(const GdkEventGrabBroken *event)
    {
        if (event->grab_window == nullptr)
        {
            set_active(false);
        }
        else if (!g_object_get_data(G_OBJECT(event->grab_window), "g-lo-InstancePopup")) // another LibreOffice popover took a grab
        {
            //try and regrab, so when we lose the grab to the menu of the color palette
            //combobox we regain it so the color palette doesn't itself disappear on next
            //click on the color palette combobox
            do_grab(GTK_WIDGET(m_pMenuHack));
        }
    }

    static gboolean signalButtonPress(GtkWidget* /*pWidget*/, GdkEventButton* /*pEvent*/, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        pThis->m_nButtonPressSeen = true;
        return false;
    }

    static gboolean signalButtonRelease(GtkWidget* /*pWidget*/, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        if (pThis->m_nButtonPressSeen && button_event_is_outside(GTK_WIDGET(pThis->m_pMenuHack), pEvent))
            pThis->set_active(false);
        return false;
    }

    static gboolean keyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        return pThis->key_press(pEvent);
    }

    bool key_press(const GdkEventKey* pEvent)
    {
        if (pEvent->keyval == GDK_KEY_Escape)
        {
            set_active(false);
            return true;
        }
        return false;
    }
#endif

    void ensure_image_widget()
    {
        if (m_pImage)
            return;

#if !GTK_CHECK_VERSION(4, 0, 0)
        m_pImage = GTK_IMAGE(gtk_image_new());
        gtk_box_pack_start(m_pBox, GTK_WIDGET(m_pImage), false, false, 0);
        gtk_box_reorder_child(m_pBox, GTK_WIDGET(m_pImage), 0);
#else
        m_pImage = GTK_PICTURE(gtk_picture_new());
        gtk_widget_set_halign(GTK_WIDGET(m_pImage), GTK_ALIGN_CENTER);
        gtk_widget_set_valign(GTK_WIDGET(m_pImage), GTK_ALIGN_CENTER);
        gtk_box_prepend(m_pBox, GTK_WIDGET(m_pImage));
        gtk_widget_set_halign(m_pLabel, GTK_ALIGN_START);
#endif
        gtk_widget_show(GTK_WIDGET(m_pImage));
    }

    static void signalFlagsChanged(GtkToggleButton* pToggleButton, GtkStateFlags flags, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        bool bOldChecked = flags & GTK_STATE_FLAG_CHECKED;
        bool bNewChecked = gtk_widget_get_state_flags(GTK_WIDGET(pToggleButton)) & GTK_STATE_FLAG_CHECKED;
        if (bOldChecked == bNewChecked)
            return;
        if (bOldChecked && gtk_widget_get_focus_on_click(GTK_WIDGET(pToggleButton)))
        {
            // grab focus back to the toggle button if the menu was popped down
            gtk_widget_grab_focus(GTK_WIDGET(pToggleButton));
        }
        SolarMutexGuard aGuard;
        pThis->signal_toggled();
    }

public:
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkInstanceMenuButton(GtkMenuButton* pMenuButton, GtkWidget* pMenuAlign, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pMenuButton), pBuilder, bTakeOwnership)
        , MenuHelper(gtk_menu_button_get_popup(pMenuButton), false)
#else
    GtkInstanceMenuButton(GtkMenuButton* pMenuButton, GtkWidget* pMenuAlign, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pMenuButton), pBuilder, bTakeOwnership)
        , MenuHelper(GTK_POPOVER_MENU(gtk_menu_button_get_popover(pMenuButton)), false)
#endif
        , m_pMenuButton(pMenuButton)
        , m_pImage(nullptr)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_pMenuHack(nullptr)
        , m_pMenuHackAlign(pMenuAlign)
        , m_nButtonPressSeen(true)
        , m_nSignalId(0)
#endif
        , m_pPopover(nullptr)
#if GTK_CHECK_VERSION(4, 0, 0)
        , m_aCustomBackground(GTK_WIDGET(pMenuButton))
#endif
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        // tdf#142924 "toggled" is too late to use to populate changes to the menu,
        // so use "state-flag-changed" on GTK_STATE_FLAG_CHECKED instead which
        // happens before "toggled"
        g_signal_handler_disconnect(m_pToggleButton, m_nToggledSignalId);
        m_nToggledSignalId = g_signal_connect(m_pToggleButton, "state-flags-changed", G_CALLBACK(signalFlagsChanged), this);

        m_pLabel = gtk_bin_get_child(GTK_BIN(m_pMenuButton));
        m_pImage = get_image_widget(GTK_WIDGET(m_pMenuButton));
        m_pBox = formatMenuButton(m_pLabel);
#else
        GtkWidget* pToggleButton = gtk_widget_get_first_child(GTK_WIDGET(m_pMenuButton));
        assert(GTK_IS_TOGGLE_BUTTON(pToggleButton));
        m_pMenuButtonToggleButton = GTK_TOGGLE_BUTTON(pToggleButton);
        m_nToggledSignalId = g_signal_connect(m_pMenuButtonToggleButton, "state-flags-changed", G_CALLBACK(signalFlagsChanged), this);
        GtkWidget* pChild = gtk_button_get_child(GTK_BUTTON(pToggleButton));
        m_pBox = GTK_IS_BOX(pChild) ? GTK_BOX(pChild) : nullptr;
        m_pLabel = m_pBox ? gtk_widget_get_first_child(GTK_WIDGET(m_pBox)) : nullptr;
        (void)pMenuAlign;
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_insert_action_group(GTK_WIDGET(m_pMenuButton), "menu", m_pActionGroup);

        update_action_group_from_popover_model();
#endif
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        // tweak the label to get a narrower size to stick
        if (GTK_IS_LABEL(m_pLabel))
            gtk_label_set_ellipsize(GTK_LABEL(m_pLabel), PANGO_ELLIPSIZE_MIDDLE);
        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual void set_label(const OUString& rText) override
    {
        ::set_label(GTK_LABEL(m_pLabel), rText);
    }

    virtual OUString get_label() const override
    {
        return ::get_label(GTK_LABEL(m_pLabel));
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        ensure_image_widget();
#if GTK_CHECK_VERSION(4, 0, 0)
        picture_set_from_virtual_device(m_pImage, pDevice);
#else
        image_set_from_virtual_device(m_pImage, pDevice);
#endif
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        ensure_image_widget();
#if GTK_CHECK_VERSION(4, 0, 0)
        picture_set_from_xgraphic(m_pImage, rImage);
#else
        image_set_from_xgraphic(m_pImage, rImage);
#endif
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        ensure_image_widget();
        picture_set_from_icon_name(m_pImage, rIconName);
    }

    virtual void set_custom_button(VirtualDevice* pDevice) override
    {
        m_aCustomBackground.use_custom_content(pDevice);
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
        if (inconsistent)
            gtk_widget_set_state_flags(GTK_WIDGET(m_pMenuButton), GTK_STATE_FLAG_INCONSISTENT, false);
        else
            gtk_widget_unset_state_flags(GTK_WIDGET(m_pMenuButton), GTK_STATE_FLAG_INCONSISTENT);
    }

    virtual bool get_inconsistent() const override
    {
        return gtk_widget_get_state_flags(GTK_WIDGET(m_pMenuButton)) & GTK_STATE_FLAG_INCONSISTENT;
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
        set_inconsistent(false);
        if (active)
            gtk_menu_button_popup(m_pMenuButton);
        else
            gtk_menu_button_popdown(m_pMenuButton);
        enable_notify_events();
    }

    virtual bool get_active() const override
    {
        GtkPopover* pPopover = gtk_menu_button_get_popover(m_pMenuButton);
        return pPopover && gtk_widget_get_visible(GTK_WIDGET(pPopover));
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_xFont = rFont;
        GtkLabel* pChild = ::get_label_widget(GTK_WIDGET(m_pMenuButton));
        ::set_font(pChild, rFont);
    }

    virtual vcl::Font get_font() override
    {
        if (m_xFont)
            return *m_xFont;
        return GtkInstanceWidget::get_font();
    }
#else
    virtual void set_active(bool bActive) override
    {
        bool bWasActive = get_active();
        GtkInstanceToggleButton::set_active(bActive);
        if (bWasActive && !bActive && gtk_widget_get_focus_on_click(GTK_WIDGET(m_pMenuButton)))
        {
            // grab focus back to the toggle button if the menu was popped down
            gtk_widget_grab_focus(GTK_WIDGET(m_pMenuButton));
        }
    }
#endif

    virtual void insert_item(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface, TriState eCheckRadioFalse) override
    {
        MenuHelper::insert_item(pos, rId, rStr, pIconName, pImageSurface, eCheckRadioFalse);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        MenuHelper::insert_separator(pos, rId);
    }

    virtual void remove_item(const OString& rId) override
    {
        MenuHelper::remove_item(rId);
    }

    virtual void clear() override
    {
        MenuHelper::clear_items();
    }

    virtual void set_item_active(const OString& rIdent, bool bActive) override
    {
        MenuHelper::set_item_active(rIdent, bActive);
    }

    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override
    {
        MenuHelper::set_item_sensitive(rIdent, bSensitive);
    }

    virtual void set_item_label(const OString& rIdent, const OUString& rLabel) override
    {
        MenuHelper::set_item_label(rIdent, rLabel);
    }

    virtual OUString get_item_label(const OString& rIdent) const override
    {
        return MenuHelper::get_item_label(rIdent);
    }

    virtual void set_item_visible(const OString& rIdent, bool bVisible) override
    {
        MenuHelper::set_item_visible(rIdent, bVisible);
    }

    virtual void signal_item_activate(const OString& rIdent) override
    {
        signal_selected(rIdent);
    }

    virtual void set_popover(weld::Widget* pPopover) override
    {
        GtkInstanceWidget* pPopoverWidget = dynamic_cast<GtkInstanceWidget*>(pPopover);
        m_pPopover = pPopoverWidget ? pPopoverWidget->getWidget() : nullptr;

#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_menu_button_set_popover(m_pMenuButton, m_pPopover);
        update_action_group_from_popover_model();
        return;
#else

        if (!m_pPopover)
        {
            gtk_menu_button_set_popover(m_pMenuButton, nullptr);
            return;
        }

        if (!m_pMenuHack)
        {
            //under wayland a Popover will work to "escape" the parent dialog, not
            //so under X, so come up with this hack to use a raw GtkWindow
            GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
            if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay) && gtk_popover_get_constrain_to(GTK_POPOVER(m_pPopover)) == GTK_POPOVER_CONSTRAINT_NONE)
            {
                m_pMenuHack = GTK_WINDOW(gtk_window_new(GTK_WINDOW_POPUP));
                gtk_window_set_type_hint(m_pMenuHack, GDK_WINDOW_TYPE_HINT_COMBO);
                // See writer "format, watermark" for true here. Can't interact with the replacement popover otherwise.
                gtk_window_set_modal(m_pMenuHack, true);
                gtk_window_set_resizable(m_pMenuHack, false);
                m_nSignalId = g_signal_connect(GTK_TOGGLE_BUTTON(m_pMenuButton), "toggled", G_CALLBACK(signalMenuButtonToggled), this);
                g_signal_connect(m_pMenuHack, "key-press-event", G_CALLBACK(keyPress), this);
                g_signal_connect(m_pMenuHack, "grab-broken-event", G_CALLBACK(signalGrabBroken), this);
                g_signal_connect(m_pMenuHack, "button-press-event", G_CALLBACK(signalButtonPress), this);
                g_signal_connect(m_pMenuHack, "button-release-event", G_CALLBACK(signalButtonRelease), this);
            }
        }

        if (m_pMenuHack)
        {
            GtkWidget* pPlaceHolder = gtk_popover_new(GTK_WIDGET(m_pMenuButton));
            gtk_popover_set_transitions_enabled(GTK_POPOVER(pPlaceHolder), false);

            // tdf#132540 theme the unwanted popover into invisibility
            GtkStyleContext *pPopoverContext = gtk_widget_get_style_context(pPlaceHolder);
            GtkCssProvider *pProvider = gtk_css_provider_new();
            static const gchar data[] = "popover { box-shadow: none; padding: 0 0 0 0; margin: 0 0 0 0; border-image: none; border-image-width: 0 0 0 0; background-image: none; background-color: transparent; border-radius: 0 0 0 0; border-width: 0 0 0 0; border-style: none; border-color: transparent; opacity: 0; min-height: 0; min-width: 0; }";
            css_provider_load_from_data(pProvider, data, -1);
            gtk_style_context_add_provider(pPopoverContext, GTK_STYLE_PROVIDER(pProvider),
                                           GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

            gtk_menu_button_set_popover(m_pMenuButton, pPlaceHolder);
        }
        else
        {
            gtk_menu_button_set_popover(m_pMenuButton, m_pPopover);
            gtk_widget_show_all(m_pPopover);
        }
#endif
    }

    void set_menu(weld::Menu* pMenu);

    static GtkBox* formatMenuButton(GtkWidget* pLabel)
    {
        // format the GtkMenuButton "manually" so we can have the dropdown image in GtkMenuButtons shown
        // on the right at the same time as an image is shown on the left
        g_object_ref(pLabel);
        GtkWidget* pContainer = gtk_widget_get_parent(pLabel);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_remove(GTK_CONTAINER(pContainer), pLabel);
#else
        gtk_box_remove(GTK_BOX(pContainer), pLabel);
#endif

        gint nImageSpacing(2);
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkStyleContext *pContext = gtk_widget_get_style_context(pContainer);
        gtk_style_context_get_style(pContext, "image-spacing", &nImageSpacing, nullptr);
#endif
        GtkBox* pBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, nImageSpacing));

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_box_pack_start(pBox, pLabel, true, true, 0);
#else
        gtk_widget_set_halign(pLabel, GTK_ALIGN_START);
        gtk_box_prepend(pBox, pLabel);
#endif
        g_object_unref(pLabel);

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (gtk_toggle_button_get_mode(GTK_TOGGLE_BUTTON(pContainer)))
            gtk_box_pack_end(pBox, gtk_image_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_BUTTON), false, false, 0);
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_add(GTK_CONTAINER(pContainer), GTK_WIDGET(pBox));
#else
        gtk_box_prepend(GTK_BOX(pContainer), GTK_WIDGET(pBox));
#endif
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_show_all(GTK_WIDGET(pBox));
#else
        gtk_widget_show(GTK_WIDGET(pBox));
#endif

        return pBox;
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pMenuButtonToggleButton, m_nToggledSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pMenuButtonToggleButton, m_nToggledSignalId);
    }
#endif

    virtual ~GtkInstanceMenuButton() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pMenuButtonToggleButton, m_nToggledSignalId);
#else
        if (m_pMenuHack)
        {
            g_signal_handler_disconnect(m_pMenuButton, m_nSignalId);
            gtk_menu_button_set_popover(m_pMenuButton, nullptr);
            gtk_widget_destroy(GTK_WIDGET(m_pMenuHack));
        }
#endif
    }
};

class GtkInstanceMenuToggleButton : public GtkInstanceToggleButton, public MenuHelper
                                  , public virtual weld::MenuToggleButton
{
private:
    GtkBox* m_pContainer;
    GtkButton* m_pToggleMenuButton;
    GtkMenuButton* m_pMenuButton;
    gulong m_nMenuBtnClickedId;
    gulong m_nToggleStateFlagsChangedId;
    gulong m_nMenuBtnStateFlagsChangedId;

    static void signalToggleStateFlagsChanged(GtkWidget* pWidget, GtkStateFlags /*eFlags*/, gpointer widget)
    {
        GtkInstanceMenuToggleButton* pThis = static_cast<GtkInstanceMenuToggleButton*>(widget);
        // mirror togglebutton state to menubutton
        gtk_widget_set_state_flags(GTK_WIDGET(pThis->m_pToggleMenuButton), gtk_widget_get_state_flags(pWidget), true);
    }

    static void signalMenuBtnStateFlagsChanged(GtkWidget* pWidget, GtkStateFlags /*eFlags*/, gpointer widget)
    {
        GtkInstanceMenuToggleButton* pThis = static_cast<GtkInstanceMenuToggleButton*>(widget);
        // mirror menubutton to togglebutton, keeping depressed state of menubutton
        GtkStateFlags eToggleFlags = gtk_widget_get_state_flags(GTK_WIDGET(pThis->m_pToggleButton));
        GtkStateFlags eFlags = gtk_widget_get_state_flags(pWidget);
        GtkStateFlags eFinalFlags = static_cast<GtkStateFlags>((eFlags & ~GTK_STATE_FLAG_ACTIVE) |
                                                               (eToggleFlags & GTK_STATE_FLAG_ACTIVE));
        gtk_widget_set_state_flags(GTK_WIDGET(pThis->m_pToggleButton), eFinalFlags, true);
    }

    static void signalMenuBtnClicked(GtkButton*, gpointer widget)
    {
        GtkInstanceMenuToggleButton* pThis = static_cast<GtkInstanceMenuToggleButton*>(widget);
        pThis->launch_menu();
    }

    void launch_menu()
    {
        gtk_widget_set_state_flags(GTK_WIDGET(m_pToggleMenuButton), gtk_widget_get_state_flags(GTK_WIDGET(m_pToggleButton)), true);
        GtkWidget* pWidget = GTK_WIDGET(m_pToggleButton);

        //run in a sub main loop because we need to keep vcl PopupMenu alive to use
        //it during DispatchCommand, returning now to the outer loop causes the
        //launching PopupMenu to be destroyed, instead run the subloop here
        //until the gtk menu is destroyed
        GMainLoop* pLoop = g_main_loop_new(nullptr, true);

#if  GTK_CHECK_VERSION(4, 0, 0)
        gulong nSignalId = g_signal_connect_swapped(G_OBJECT(m_pMenu), "closed", G_CALLBACK(g_main_loop_quit), pLoop);

        g_object_ref(m_pMenu);
        gtk_menu_button_set_popover(m_pMenuButton, nullptr);
        gtk_widget_set_parent(GTK_WIDGET(m_pMenu), pWidget);
        gtk_popover_set_position(GTK_POPOVER(m_pMenu), GTK_POS_BOTTOM);
        gtk_popover_popup(GTK_POPOVER(m_pMenu));
#else
        gulong nSignalId = g_signal_connect_swapped(G_OBJECT(m_pMenu), "deactivate", G_CALLBACK(g_main_loop_quit), pLoop);

#if GTK_CHECK_VERSION(3,22,0)
        if (gtk_check_version(3, 22, 0) == nullptr)
        {
            // Send a keyboard event through gtk_main_do_event to toggle any active tooltip offs
            // before trying to launch the menu
            // https://gitlab.gnome.org/GNOME/gtk/issues/1785
            // Fixed in GTK 2.34
            GdkEvent *pKeyEvent = GtkSalFrame::makeFakeKeyPress(pWidget);
            gtk_main_do_event(pKeyEvent);

            GdkEvent *pTriggerEvent = gtk_get_current_event();
            if (!pTriggerEvent)
                pTriggerEvent = pKeyEvent;

            gtk_menu_popup_at_widget(m_pMenu, pWidget, GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST, pTriggerEvent);

            gdk_event_free(pKeyEvent);
        }
        else
#endif
        {
            guint nButton;
            guint32 nTime;

            //typically there is an event, and we can then distinguish if this was
            //launched from the keyboard (gets auto-mnemoniced) or the mouse (which
            //doesn't)
            GdkEvent *pEvent = gtk_get_current_event();
            if (pEvent)
            {
                gdk_event_get_button(pEvent, &nButton);
                nTime = gdk_event_get_time(pEvent);
            }
            else
            {
                nButton = 0;
                nTime = GtkSalFrame::GetLastInputEventTime();
            }

            gtk_menu_popup(m_pMenu, nullptr, nullptr, nullptr, nullptr, nButton, nTime);
        }
#endif

        if (g_main_loop_is_running(pLoop))
            main_loop_run(pLoop);

        g_main_loop_unref(pLoop);
        g_signal_handler_disconnect(m_pMenu, nSignalId);

#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_unparent(GTK_WIDGET(m_pMenu));
        gtk_menu_button_set_popover(m_pMenuButton, GTK_WIDGET(m_pMenu));
        g_object_unref(m_pMenu);
#endif

    }

    static gboolean signalMenuToggleButton(GtkWidget*, gboolean bGroupCycling, gpointer widget)
    {
        GtkInstanceMenuToggleButton* pThis = static_cast<GtkInstanceMenuToggleButton*>(widget);
        return gtk_widget_mnemonic_activate(GTK_WIDGET(pThis->m_pToggleButton), bGroupCycling);
    }

public:
    GtkInstanceMenuToggleButton(GtkBuilder* pMenuToggleButtonBuilder, GtkMenuButton* pMenuButton,
        GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pMenuToggleButtonBuilder, "togglebutton")),
                                  pBuilder, bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , MenuHelper(gtk_menu_button_get_popup(pMenuButton), false)
#else
        , MenuHelper(GTK_POPOVER_MENU(gtk_menu_button_get_popover(pMenuButton)), false)
#endif
        , m_pContainer(GTK_BOX(gtk_builder_get_object(pMenuToggleButtonBuilder, "box")))
        , m_pToggleMenuButton(GTK_BUTTON(gtk_builder_get_object(pMenuToggleButtonBuilder, "menubutton")))
        , m_pMenuButton(pMenuButton)
        , m_nMenuBtnClickedId(g_signal_connect(m_pToggleMenuButton, "clicked", G_CALLBACK(signalMenuBtnClicked), this))
        , m_nToggleStateFlagsChangedId(g_signal_connect(m_pToggleButton, "state-flags-changed", G_CALLBACK(signalToggleStateFlagsChanged), this))
        , m_nMenuBtnStateFlagsChangedId(g_signal_connect(m_pToggleMenuButton, "state-flags-changed", G_CALLBACK(signalMenuBtnStateFlagsChanged), this))
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkInstanceMenuButton::formatMenuButton(gtk_bin_get_child(GTK_BIN(m_pMenuButton)));
#endif

        insertAsParent(GTK_WIDGET(m_pMenuButton), GTK_WIDGET(m_pContainer));
        gtk_widget_hide(GTK_WIDGET(m_pMenuButton));

        // move the first GtkMenuButton child, as created by GtkInstanceMenuButton ctor, into the GtkToggleButton
        // instead, leaving just the indicator behind in the GtkMenuButton
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pButtonBox = gtk_bin_get_child(GTK_BIN(m_pMenuButton));
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pButtonBox));
        int nGroup = 0;
        for (GList* pChild = g_list_first(pChildren); pChild && nGroup < 2; pChild = g_list_next(pChild), ++nGroup)
        {
            GtkWidget* pWidget = static_cast<GtkWidget*>(pChild->data);
            g_object_ref(pWidget);
            gtk_container_remove(GTK_CONTAINER(pButtonBox), pWidget);
            if (nGroup == 0)
                gtk_container_add(GTK_CONTAINER(m_pToggleButton), pWidget);
            else
                gtk_container_add(GTK_CONTAINER(m_pToggleMenuButton), pWidget);
            gtk_widget_show_all(pWidget);
            g_object_unref(pWidget);
        }
        g_list_free(pChildren);
#else
        GtkWidget* pChild;
        if (gtk_check_version(4, 5, 0) == nullptr)
        {
            pChild = gtk_widget_get_first_child(GTK_WIDGET(m_pMenuButton));
            pChild = gtk_widget_get_first_child(pChild);
            pChild = gtk_widget_get_first_child(pChild);
        }
        else
            pChild = gtk_widget_get_last_child(GTK_WIDGET(m_pMenuButton));
        g_object_ref(pChild);
        gtk_widget_unparent(pChild);
        gtk_button_set_child(GTK_BUTTON(m_pToggleButton), pChild);
        g_object_unref(pChild);
#endif

        // match the GtkToggleButton relief to the GtkMenuButton
#if !GTK_CHECK_VERSION(4, 0, 0)
        const GtkReliefStyle eStyle = gtk_button_get_relief(GTK_BUTTON(m_pMenuButton));
        gtk_button_set_relief(GTK_BUTTON(m_pToggleButton), eStyle);
        gtk_button_set_relief(GTK_BUTTON(m_pToggleMenuButton), eStyle);
#else
        const bool bStyle = gtk_menu_button_get_has_frame(GTK_MENU_BUTTON(m_pMenuButton));
        gtk_button_set_has_frame(GTK_BUTTON(m_pToggleButton), bStyle);
        gtk_button_set_has_frame(GTK_BUTTON(m_pToggleMenuButton), bStyle);
#endif

        // move the GtkMenuButton margins up to the new parent
        gtk_widget_set_margin_top(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_top(GTK_WIDGET(m_pMenuButton)));
        gtk_widget_set_margin_bottom(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_bottom(GTK_WIDGET(m_pMenuButton)));
        gtk_widget_set_margin_start(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_start(GTK_WIDGET(m_pMenuButton)));
        gtk_widget_set_margin_end(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_end(GTK_WIDGET(m_pMenuButton)));

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_menu_detach(m_pMenu);
        gtk_menu_attach_to_widget(m_pMenu, GTK_WIDGET(m_pToggleButton), nullptr);
#else
        gtk_widget_insert_action_group(GTK_WIDGET(m_pContainer), "menu", m_pActionGroup);

        update_action_group_from_popover_model();
#endif

        g_signal_connect(m_pContainer, "mnemonic-activate", G_CALLBACK(signalMenuToggleButton), this);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pToggleMenuButton, m_nMenuBtnClickedId);
        GtkInstanceToggleButton::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceToggleButton::enable_notify_events();
        g_signal_handler_unblock(m_pToggleMenuButton, m_nMenuBtnClickedId);
    }

    virtual ~GtkInstanceMenuToggleButton()
    {
        g_signal_handler_disconnect(m_pToggleButton, m_nToggleStateFlagsChangedId);
        g_signal_handler_disconnect(m_pToggleMenuButton, m_nMenuBtnStateFlagsChangedId);
        g_signal_handler_disconnect(m_pToggleMenuButton, m_nMenuBtnClickedId);

#if GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pChild = gtk_button_get_child(GTK_BUTTON(m_pToggleButton));
        g_object_ref(pChild);
        gtk_button_set_child(GTK_BUTTON(m_pToggleButton), nullptr);
        gtk_widget_unparent(pChild);
        gtk_widget_set_parent(pChild, GTK_WIDGET(m_pMenuButton));
        g_object_unref(pChild);
#endif
    }

    virtual void insert_item(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface, TriState eCheckRadioFalse) override
    {
        MenuHelper::insert_item(pos, rId, rStr, pIconName, pImageSurface, eCheckRadioFalse);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        MenuHelper::insert_separator(pos, rId);
    }

    virtual void remove_item(const OString& rId) override
    {
        MenuHelper::remove_item(rId);
    }

    virtual void clear() override
    {
        MenuHelper::clear_items();
    }

    virtual void set_item_active(const OString& rIdent, bool bActive) override
    {
        MenuHelper::set_item_active(rIdent, bActive);
    }

    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override
    {
        MenuHelper::set_item_sensitive(rIdent, bSensitive);
    }

    virtual void set_item_label(const OString& rIdent, const OUString& rLabel) override
    {
        MenuHelper::set_item_label(rIdent, rLabel);
    }

    virtual OUString get_item_label(const OString& rIdent) const override
    {
        return MenuHelper::get_item_label(rIdent);
    }

    virtual void set_item_visible(const OString& rIdent, bool bVisible) override
    {
        MenuHelper::set_item_visible(rIdent, bVisible);
    }

    virtual void signal_item_activate(const OString& rIdent) override
    {
        signal_selected(rIdent);
    }

    virtual void set_popover(weld::Widget* /*pPopover*/) override
    {
        assert(false && "not implemented");
    }
};

class GtkInstanceMenu : public MenuHelper, public virtual weld::Menu
{
protected:
#if !GTK_CHECK_VERSION(4, 0, 0)
    std::vector<GtkMenuItem*> m_aExtraItems;
#endif
    OString m_sActivated;
#if !GTK_CHECK_VERSION(4, 0, 0)
    MenuHelper* m_pTopLevelMenuHelper;
#endif

private:
    virtual void signal_item_activate(const OString& rIdent) override
    {
        m_sActivated = rIdent;
        weld::Menu::signal_activate(m_sActivated);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    void clear_extras()
    {
        if (m_aExtraItems.empty())
            return;
        if (m_pTopLevelMenuHelper)
        {
            for (auto a : m_aExtraItems)
                m_pTopLevelMenuHelper->remove_from_map(a);
        }
        m_aExtraItems.clear();
    }
#endif

public:
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkInstanceMenu(GtkMenu* pMenu, bool bTakeOwnership)
#else
    GtkInstanceMenu(GtkPopoverMenu* pMenu, bool bTakeOwnership)
#endif
        : MenuHelper(pMenu, bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_pTopLevelMenuHelper(nullptr)
#endif
    {
        g_object_set_data(G_OBJECT(m_pMenu), "g-lo-GtkInstanceMenu", this);
#if !GTK_CHECK_VERSION(4, 0, 0)
        // tdf#122527 if we're welding a submenu of a menu of a MenuButton,
        // then find that MenuButton parent so that when adding items to this
        // menu we can inform the MenuButton of their addition
        GtkMenu* pTopLevelMenu = pMenu;
        while (true)
        {
            GtkWidget* pAttached = gtk_menu_get_attach_widget(pTopLevelMenu);
            if (!pAttached || !GTK_IS_MENU_ITEM(pAttached))
                break;
            GtkWidget* pParent = gtk_widget_get_parent(pAttached);
            if (!pParent || !GTK_IS_MENU(pParent))
                break;
            pTopLevelMenu = GTK_MENU(pParent);
        }
        if (pTopLevelMenu == pMenu)
            return;

        // maybe the toplevel is a menubutton
        GtkWidget* pAttached = gtk_menu_get_attach_widget(pTopLevelMenu);
        if (pAttached && GTK_IS_MENU_BUTTON(pAttached))
        {
            void* pData = g_object_get_data(G_OBJECT(pAttached), "g-lo-GtkInstanceButton");
            m_pTopLevelMenuHelper = dynamic_cast<GtkInstanceMenuButton*>(static_cast<GtkInstanceButton*>(pData));
        }
        // or maybe a menu
        if (!m_pTopLevelMenuHelper)
        {
            void* pData = g_object_get_data(G_OBJECT(pTopLevelMenu), "g-lo-GtkInstanceMenu");
            m_pTopLevelMenuHelper = static_cast<GtkInstanceMenu*>(pData);
        }
#else
        update_action_group_from_popover_model();
#endif
    }

    virtual OString popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect, weld::Placement ePlace) override
    {
        m_sActivated.clear();

        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pParent);
        assert(pGtkWidget);
        GtkWidget* pWidget = pGtkWidget->getWidget();

        //run in a sub main loop because we need to keep vcl PopupMenu alive to use
        //it during DispatchCommand, returning now to the outer loop causes the
        //launching PopupMenu to be destroyed, instead run the subloop here
        //until the gtk menu is destroyed
        GMainLoop* pLoop = g_main_loop_new(nullptr, true);

#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_insert_action_group(pWidget, "menu", m_pActionGroup);

        gulong nSignalId = g_signal_connect_swapped(G_OBJECT(m_pMenu), "closed", G_CALLBACK(g_main_loop_quit), pLoop);

        GdkRectangle aRect;
        pWidget = getPopupRect(pWidget, rRect, aRect);

        GtkWidget* pOrigParent = gtk_widget_get_parent(GTK_WIDGET(m_pMenu));
        gtk_widget_set_parent(GTK_WIDGET(m_pMenu), pWidget);
        gtk_popover_set_pointing_to(GTK_POPOVER(m_pMenu), &aRect);
        if (ePlace == weld::Placement::Under)
            gtk_popover_set_position(GTK_POPOVER(m_pMenu), GTK_POS_BOTTOM);
        else
        {
            if (SwapForRTL(pWidget))
                gtk_popover_set_position(GTK_POPOVER(m_pMenu), GTK_POS_LEFT);
            else
                gtk_popover_set_position(GTK_POPOVER(m_pMenu), GTK_POS_RIGHT);
        }
        gtk_popover_popup(GTK_POPOVER(m_pMenu));
#else
        gulong nSignalId = g_signal_connect_swapped(G_OBJECT(m_pMenu), "deactivate", G_CALLBACK(g_main_loop_quit), pLoop);

#if GTK_CHECK_VERSION(3,22,0)
        if (gtk_check_version(3, 22, 0) == nullptr)
        {
            GdkRectangle aRect;
            pWidget = getPopupRect(pWidget, rRect, aRect);
            gtk_menu_attach_to_widget(m_pMenu, pWidget, nullptr);

            // Send a keyboard event through gtk_main_do_event to toggle any active tooltip offs
            // before trying to launch the menu
            // https://gitlab.gnome.org/GNOME/gtk/issues/1785
            // Fixed in GTK 2.34
            GdkEvent *pKeyEvent = GtkSalFrame::makeFakeKeyPress(pWidget);
            gtk_main_do_event(pKeyEvent);

            GdkEvent *pTriggerEvent = gtk_get_current_event();
            if (!pTriggerEvent)
                pTriggerEvent = pKeyEvent;

            bool bSwapForRTL = SwapForRTL(pWidget);

            if (ePlace == weld::Placement::Under)
            {
                if (bSwapForRTL)
                    gtk_menu_popup_at_rect(m_pMenu, widget_get_surface(pWidget), &aRect, GDK_GRAVITY_SOUTH_EAST, GDK_GRAVITY_NORTH_EAST, pTriggerEvent);
                else
                    gtk_menu_popup_at_rect(m_pMenu, widget_get_surface(pWidget), &aRect, GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST, pTriggerEvent);
            }
            else
            {
                if (bSwapForRTL)
                    gtk_menu_popup_at_rect(m_pMenu, widget_get_surface(pWidget), &aRect, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_EAST, pTriggerEvent);
                else
                    gtk_menu_popup_at_rect(m_pMenu, widget_get_surface(pWidget), &aRect, GDK_GRAVITY_NORTH_EAST, GDK_GRAVITY_NORTH_WEST, pTriggerEvent);
            }

            gdk_event_free(pKeyEvent);
        }
        else
#else
        (void) rRect;
#endif
        {
            gtk_menu_attach_to_widget(m_pMenu, pWidget, nullptr);

            guint nButton;
            guint32 nTime;

            //typically there is an event, and we can then distinguish if this was
            //launched from the keyboard (gets auto-mnemoniced) or the mouse (which
            //doesn't)
            GdkEvent *pEvent = gtk_get_current_event();
            if (pEvent)
            {
                if (!gdk_event_get_button(pEvent, &nButton))
                    nButton = 0;
                nTime = gdk_event_get_time(pEvent);
            }
            else
            {
                nButton = 0;
                nTime = GtkSalFrame::GetLastInputEventTime();
            }

            gtk_menu_popup(m_pMenu, nullptr, nullptr, nullptr, nullptr, nButton, nTime);
        }
#endif

        if (g_main_loop_is_running(pLoop))
            main_loop_run(pLoop);

        g_main_loop_unref(pLoop);
        g_signal_handler_disconnect(m_pMenu, nSignalId);

#if GTK_CHECK_VERSION(4, 0, 0)
        if (!pOrigParent)
            gtk_widget_unparent(GTK_WIDGET(m_pMenu));
        else
            gtk_widget_set_parent(GTK_WIDGET(m_pMenu), pOrigParent);

        gtk_widget_insert_action_group(pWidget, "menu", nullptr);
#else
        gtk_menu_detach(m_pMenu);
#endif

        return m_sActivated;
    }

    virtual void set_sensitive(const OString& rIdent, bool bSensitive) override
    {
        set_item_sensitive(rIdent, bSensitive);
    }

    virtual bool get_sensitive(const OString& rIdent) const override
    {
        return get_item_sensitive(rIdent);
    }

    virtual void set_active(const OString& rIdent, bool bActive) override
    {
        set_item_active(rIdent, bActive);
    }

    virtual bool get_active(const OString& rIdent) const override
    {
        return get_item_active(rIdent);
    }

    virtual void set_visible(const OString& rIdent, bool bShow) override
    {
        set_item_visible(rIdent, bShow);
    }

    virtual void set_label(const OString& rIdent, const OUString& rLabel) override
    {
        set_item_label(rIdent, rLabel);
    }

    virtual OUString get_label(const OString& rIdent) const override
    {
        return get_item_label(rIdent);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        MenuHelper::insert_separator(pos, rId);
    }

    virtual void clear() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        clear_extras();
#endif
        MenuHelper::clear_items();
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        const css::uno::Reference<css::graphic::XGraphic>& rGraphic,
                        TriState eCheckRadioFalse) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pImage = nullptr;
        if (pIconName)
        {
            if (GdkPixbuf* pixbuf = load_icon_by_name(*pIconName))
            {
                pImage = gtk_image_new_from_pixbuf(pixbuf);
                g_object_unref(pixbuf);
            }
        }
        else if (pImageSurface)
        {
            pImage = image_new_from_virtual_device(*pImageSurface);
        }
        else if (rGraphic)
        {
            if (GdkPixbuf* pixbuf = getPixbuf(rGraphic))
            {
                pImage = gtk_image_new_from_pixbuf(pixbuf);
                g_object_unref(pixbuf);
            }
        }

        GtkWidget *pItem;
        if (pImage)
        {
            GtkBox *pBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
            GtkWidget *pLabel = gtk_label_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
            gtk_label_set_xalign(GTK_LABEL(pLabel), 0.0);
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new() : gtk_menu_item_new();
            gtk_box_pack_start(pBox, pImage, false, true, 0);
            gtk_box_pack_start(pBox, pLabel, true, true, 0);
            gtk_container_add(GTK_CONTAINER(pItem), GTK_WIDGET(pBox));
            gtk_widget_show_all(pItem);
        }
        else
        {
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr())
                                                       : gtk_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
        }

        if (eCheckRadioFalse == TRISTATE_FALSE)
            gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(pItem), true);

        ::set_buildable_id(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8));
        gtk_menu_shell_append(GTK_MENU_SHELL(m_pMenu), pItem);
        gtk_widget_show(pItem);
        GtkMenuItem* pMenuItem = GTK_MENU_ITEM(pItem);
        m_aExtraItems.push_back(pMenuItem);
        add_to_map(pMenuItem);
        if (m_pTopLevelMenuHelper)
            m_pTopLevelMenuHelper->add_to_map(pMenuItem);
        if (pos != -1)
            gtk_menu_reorder_child(m_pMenu, pItem, pos);
#else
        SAL_WARN("vcl.gtk", "needs to be implemented for gtk4");
        (void)pos;
        (void)rId;
        (void)rStr;
        (void)pIconName;
        (void)pImageSurface;
        (void)rGraphic;
        (void)eCheckRadioFalse;
#endif
    }

    virtual OString get_id(int pos) const override
    {
        return get_item_id(pos);
    }

    virtual int n_children() const override
    {
        return get_n_children();
    }

    void remove(const OString& rIdent) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!m_aExtraItems.empty())
        {
            GtkMenuItem* pMenuItem = m_aMap[rIdent];
            auto iter = std::find(m_aExtraItems.begin(), m_aExtraItems.end(), pMenuItem);
            if (iter != m_aExtraItems.end())
            {
                if (m_pTopLevelMenuHelper)
                    m_pTopLevelMenuHelper->remove_from_map(pMenuItem);
                m_aExtraItems.erase(iter);
            }
        }
#endif
        MenuHelper::remove_item(rIdent);
    }

    virtual ~GtkInstanceMenu() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        clear_extras();
#endif
        g_object_steal_data(G_OBJECT(m_pMenu), "g-lo-GtkInstanceMenu");
    }
};

#if !GTK_CHECK_VERSION(4, 0, 0)
    vcl::ImageType GtkToVcl(GtkIconSize eSize)
    {
        vcl::ImageType eRet;
        switch (eSize)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            case GTK_ICON_SIZE_MENU:
            case GTK_ICON_SIZE_SMALL_TOOLBAR:
            case GTK_ICON_SIZE_BUTTON:
                eRet = vcl::ImageType::Size16;
                break;
            case GTK_ICON_SIZE_LARGE_TOOLBAR:
                eRet = vcl::ImageType::Size26;
                break;
            case GTK_ICON_SIZE_DND:
            case GTK_ICON_SIZE_DIALOG:
                eRet = vcl::ImageType::Size32;
                break;
            default:
            case GTK_ICON_SIZE_INVALID:
                eRet = vcl::ImageType::Small;
                break;
#else
            case GTK_ICON_SIZE_LARGE:
                eRet = vcl::ImageType::Size32;
                break;
            case GTK_ICON_SIZE_NORMAL:
            default:
                eRet = vcl::ImageType::Size16;
                break;
#endif
        }
        return eRet;
    }

    GtkIconSize VclToGtk(vcl::ImageType eSize)
    {
        GtkIconSize eRet;
#if !GTK_CHECK_VERSION(4, 0, 0)
        switch (eSize)
        {
            case vcl::ImageType::Size16:
                eRet = GTK_ICON_SIZE_SMALL_TOOLBAR;
                break;
            case vcl::ImageType::Size26:
                eRet = GTK_ICON_SIZE_LARGE_TOOLBAR;
                break;
            case vcl::ImageType::Size32:
                eRet = GTK_ICON_SIZE_DIALOG;
                break;
            default:
                O3TL_UNREACHABLE;
        }
#else
        switch (eSize)
        {
            case vcl::ImageType::Size26:
            case vcl::ImageType::Size32:
                eRet = GTK_ICON_SIZE_LARGE;
                break;
            case vcl::ImageType::Size16:
            default:
                eRet = GTK_ICON_SIZE_NORMAL;
                break;
        }
#endif
        return eRet;
    }
#endif
}

void GtkInstanceMenuButton::set_menu(weld::Menu* pMenu)
{
    GtkInstanceMenu* pPopoverWidget = dynamic_cast<GtkInstanceMenu*>(pMenu);
    m_pPopover = nullptr;
    m_pMenu = pPopoverWidget ? pPopoverWidget->getMenu() : nullptr;

#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_menu_button_set_popup(m_pMenuButton, GTK_WIDGET(m_pMenu));
#else
    gtk_menu_button_set_popover(m_pMenuButton, GTK_WIDGET(m_pMenu));
    update_action_group_from_popover_model();
#endif
}

namespace {

class GtkInstanceToolbar : public GtkInstanceWidget, public virtual weld::Toolbar
{
private:
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkToolbar* m_pToolbar;
#else
    GtkBox* m_pToolbar;
    vcl::ImageType m_eImageType;
#endif
    GtkCssProvider *m_pMenuButtonProvider;

    std::map<OString, GtkWidget*> m_aMap;
    std::map<OString, std::unique_ptr<GtkInstanceMenuButton>> m_aMenuButtonMap;

#if !GTK_CHECK_VERSION(4, 0, 0)
    // at the time of writing there is no gtk_menu_tool_button_set_popover available
    // though there will be in the future
    // https://gitlab.gnome.org/GNOME/gtk/commit/03e30431a8af9a947a0c4ccab545f24da16bfe17?w=1
    static void find_menu_button(GtkWidget *pWidget, gpointer user_data)
    {
        if (g_strcmp0(gtk_widget_get_name(pWidget), "GtkMenuButton") == 0)
        {
            GtkWidget **ppToggleButton = static_cast<GtkWidget**>(user_data);
            *ppToggleButton = pWidget;
        }
        else if (GTK_IS_CONTAINER(pWidget))
            gtk_container_forall(GTK_CONTAINER(pWidget), find_menu_button, user_data);
    }

    static void find_menupeer_button(GtkWidget *pWidget, gpointer user_data)
    {
        if (g_strcmp0(gtk_widget_get_name(pWidget), "GtkButton") == 0)
        {
            GtkWidget **ppButton = static_cast<GtkWidget**>(user_data);
            *ppButton = pWidget;
        }
        else if (GTK_IS_CONTAINER(pWidget))
            gtk_container_forall(GTK_CONTAINER(pWidget), find_menupeer_button, user_data);
    }
#endif

    static void collect(GtkWidget* pItem, gpointer widget)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!GTK_IS_TOOL_ITEM(pItem))
            return;
#endif
        GtkInstanceToolbar* pThis = static_cast<GtkInstanceToolbar*>(widget);

        GtkMenuButton* pMenuButton = nullptr;
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_MENU_TOOL_BUTTON(pItem))
            find_menu_button(pItem, &pMenuButton);
#else
        if (GTK_IS_MENU_BUTTON(pItem))
            pMenuButton = GTK_MENU_BUTTON(pItem);
#endif

        pThis->add_to_map(pItem, pMenuButton);
    }

    void add_to_map(GtkWidget* pToolItem, GtkMenuButton* pMenuButton)
    {
        OString id = ::get_buildable_id(GTK_BUILDABLE(pToolItem));
        m_aMap[id] = pToolItem;
        if (pMenuButton)
        {
            m_aMenuButtonMap[id] = std::make_unique<GtkInstanceMenuButton>(pMenuButton, GTK_WIDGET(pToolItem), m_pBuilder, false);
            // so that, e.g. with focus initially in writer main document then
            // after clicking the heading menu in the writer navigator focus is
            // left in the main document and not in the toolbar
#if !GTK_CHECK_VERSION(4, 0, 0)
            gtk_button_set_focus_on_click(GTK_BUTTON(pMenuButton), false);
            g_signal_connect(pMenuButton, "toggled", G_CALLBACK(signalItemToggled), this);
#else
            gtk_widget_set_focus_on_click(GTK_WIDGET(pMenuButton), false);

            GtkWidget* pToggleButton = gtk_widget_get_first_child(GTK_WIDGET(pMenuButton));
            assert(GTK_IS_TOGGLE_BUTTON(pToggleButton));
            g_signal_connect(pToggleButton, "toggled", G_CALLBACK(signalItemToggled), this);
#endif

            // by default the GtkMenuButton down arrow button is as wide as
            // a normal button and LibreOffice's original ones are very
            // narrow, that assumption is fairly baked into the toolbar and
            // sidebar designs, try and minimize the width of the dropdown
            // zone.
            GtkStyleContext *pButtonContext = gtk_widget_get_style_context(GTK_WIDGET(pMenuButton));

            if (!m_pMenuButtonProvider)
            {
                m_pMenuButtonProvider = gtk_css_provider_new();
                static const gchar data[] = "* { "
                  "padding: 0;"
                  "margin-left: 0px;"
                  "margin-right: 0px;"
                  "min-width: 4px;"
                  "}";
                css_provider_load_from_data(m_pMenuButtonProvider, data, -1);
            }

            gtk_style_context_add_provider(pButtonContext,
                                           GTK_STYLE_PROVIDER(m_pMenuButtonProvider),
                                           GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!GTK_IS_TOOL_BUTTON(pToolItem))
#else
        if (!GTK_IS_BUTTON(pToolItem))
#endif
        {
            return;
        }
        g_signal_connect(pToolItem, "clicked", G_CALLBACK(signalItemClicked), this);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void signalItemClicked(GtkToolButton* pItem, gpointer widget)
#else
    static void signalItemClicked(GtkButton* pItem, gpointer widget)
#endif
    {
        GtkInstanceToolbar* pThis = static_cast<GtkInstanceToolbar*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_item_clicked(pItem);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    void signal_item_clicked(GtkToolButton* pItem)
#else
    void signal_item_clicked(GtkButton* pItem)
#endif
    {
        signal_clicked(::get_buildable_id(GTK_BUILDABLE(pItem)));
    }

    static void signalItemToggled(GtkToggleButton* pItem, gpointer widget)
    {
        GtkInstanceToolbar* pThis = static_cast<GtkInstanceToolbar*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_item_toggled(pItem);
    }

    void signal_item_toggled(GtkToggleButton* pItem)
    {
        for (const auto& a : m_aMenuButtonMap)
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            if (a.second->getWidget() == GTK_WIDGET(pItem))
#else
            if (a.second->getWidget() == gtk_widget_get_parent(GTK_WIDGET(pItem)))
#endif
            {
                signal_toggle_menu(a.first);
                break;
            }
        }
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void set_item_image(GtkWidget* pItem, GtkWidget* pImage)
    {
        if (GTK_IS_BUTTON(pItem))
            gtk_button_set_child(GTK_BUTTON(pItem), pImage);
        else if (GTK_IS_MENU_BUTTON(pItem))
        {
            // TODO after gtk 4.6 is released require that version and drop this
            static auto menu_button_set_child = reinterpret_cast<void (*) (GtkMenuButton*, GtkWidget*)>(dlsym(nullptr, "gtk_menu_button_set_child"));
            if (menu_button_set_child)
                menu_button_set_child(GTK_MENU_BUTTON(pItem), pImage);
        }
        // versions of gtk4 > 4.2.1 might do this on their own
        gtk_widget_remove_css_class(pItem, "text-button");
    }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    static void set_item_image(GtkToolButton* pItem, const css::uno::Reference<css::graphic::XGraphic>& rIcon)
#else
    static void set_item_image(GtkWidget* pItem, const css::uno::Reference<css::graphic::XGraphic>& rIcon)
#endif
    {
        GtkWidget* pImage = nullptr;

        if (GdkPixbuf* pixbuf = getPixbuf(rIcon))
        {
            pImage = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            gtk_widget_show(pImage);
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_tool_button_set_icon_widget(pItem, pImage);
#else
        set_item_image(pItem, pImage);
#endif
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    void set_item_image(GtkToolButton* pItem, const VirtualDevice* pDevice)
#else
    void set_item_image(GtkWidget* pItem, const VirtualDevice* pDevice)
#endif
    {
        GtkWidget* pImage = nullptr;

        if (pDevice)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            pImage = picture_new_from_virtual_device(*pDevice);
#else
            pImage = image_new_from_virtual_device(*pDevice);
#endif
            gtk_widget_show(pImage);
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_tool_button_set_icon_widget(pItem, pImage);
#else
        set_item_image(pItem, pImage);
#endif
        gtk_widget_queue_draw(GTK_WIDGET(m_pToolbar));
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkWidget* toolbar_get_nth_item(int nIndex) const
    {
        return GTK_WIDGET(gtk_toolbar_get_nth_item(m_pToolbar, nIndex));
    }
#else
    GtkWidget* toolbar_get_nth_item(int nIndex) const
    {
        int i = 0;
        for (GtkWidget* pChild = gtk_widget_get_first_child(GTK_WIDGET(m_pToolbar));
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            if (i == nIndex)
                return pChild;
            ++i;
        }
        return nullptr;
    }
#endif
public:
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkInstanceToolbar(GtkToolbar* pToolbar, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
#else
    GtkInstanceToolbar(GtkBox* pToolbar, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
#endif
        : GtkInstanceWidget(GTK_WIDGET(pToolbar), pBuilder, bTakeOwnership)
        , m_pToolbar(pToolbar)
#if GTK_CHECK_VERSION(4, 0, 0)
        , m_eImageType(vcl::ImageType::Size16)
#endif
        , m_pMenuButtonProvider(nullptr)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        for (GtkWidget* pChild = gtk_widget_get_first_child(GTK_WIDGET(pToolbar));
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            collect(pChild, this);
        }
#else
        gtk_container_foreach(GTK_CONTAINER(pToolbar), collect, this);
#endif
    }

    void disable_item_notify_events()
    {
        for (auto& a : m_aMap)
        {
            g_signal_handlers_block_by_func(a.second, reinterpret_cast<void*>(signalItemClicked), this);
        }
    }

    void enable_item_notify_events()
    {
        for (auto& a : m_aMap)
        {
            g_signal_handlers_unblock_by_func(a.second, reinterpret_cast<void*>(signalItemClicked), this);
        }
    }

    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override
    {
        disable_item_notify_events();
        gtk_widget_set_sensitive(GTK_WIDGET(m_aMap[rIdent]), bSensitive);
        enable_item_notify_events();
    }

    virtual bool get_item_sensitive(const OString& rIdent) const override
    {
        return gtk_widget_get_sensitive(GTK_WIDGET(m_aMap.find(rIdent)->second));
    }

    virtual void set_item_visible(const OString& rIdent, bool bVisible) override
    {
        disable_item_notify_events();
        gtk_widget_set_visible(GTK_WIDGET(m_aMap[rIdent]), bVisible);
        enable_item_notify_events();
    }

    virtual void set_item_help_id(const OString& rIdent, const OString& rHelpId) override
    {
        ::set_help_id(GTK_WIDGET(m_aMap[rIdent]), rHelpId);
    }

    virtual bool get_item_visible(const OString& rIdent) const override
    {
        return gtk_widget_get_visible(GTK_WIDGET(m_aMap.find(rIdent)->second));
    }

    virtual void set_item_active(const OString& rIdent, bool bActive) override
    {
        disable_item_notify_events();

        GtkWidget* pToolButton = m_aMap.find(rIdent)->second;

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_TOGGLE_TOOL_BUTTON(pToolButton))
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pToolButton), bActive);
        else
        {
            GtkButton* pButton = nullptr;
            // there is no GtkMenuToggleToolButton so abuse the CHECKED state of the GtkMenuToolButton button
            // to emulate one
            find_menupeer_button(GTK_WIDGET(pToolButton), &pButton);
            if (pButton)
            {
                auto eState = gtk_widget_get_state_flags(GTK_WIDGET(pButton)) & ~GTK_STATE_FLAG_CHECKED;
                if (bActive)
                    eState |= GTK_STATE_FLAG_CHECKED;
                gtk_widget_set_state_flags(GTK_WIDGET(pButton), static_cast<GtkStateFlags>(eState), true);
            }
        }
#else
        GtkWidget* pWidget;
        if (GTK_IS_MENU_BUTTON(pToolButton))
        {
            pWidget = gtk_widget_get_first_child(pToolButton);
            assert(GTK_IS_TOGGLE_BUTTON(pWidget));
        }
        else
            pWidget = pToolButton;
        auto eState = gtk_widget_get_state_flags(pWidget) & ~GTK_STATE_FLAG_CHECKED;
        if (bActive)
            eState |= GTK_STATE_FLAG_CHECKED;
        gtk_widget_set_state_flags(pWidget, static_cast<GtkStateFlags>(eState), true);
#endif

        enable_item_notify_events();
    }

    virtual bool get_item_active(const OString& rIdent) const override
    {
        GtkWidget* pToolButton = m_aMap.find(rIdent)->second;

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_TOGGLE_TOOL_BUTTON(pToolButton))
            return gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pToolButton));
        else
        {
            GtkButton* pButton = nullptr;
            // there is no GtkMenuToggleToolButton so abuse the CHECKED state of the GtkMenuToolButton button
            // to emulate one
            find_menupeer_button(GTK_WIDGET(pToolButton), &pButton);
            if (pButton)
            {
                return gtk_widget_get_state_flags(GTK_WIDGET(pButton)) & GTK_STATE_FLAG_CHECKED;
            }
        }
#else
        GtkWidget* pWidget;
        if (GTK_IS_MENU_BUTTON(pToolButton))
        {
            pWidget = gtk_widget_get_first_child(pToolButton);
            assert(GTK_IS_TOGGLE_BUTTON(pWidget));
        }
        else
            pWidget = pToolButton;
        return gtk_widget_get_state_flags(pWidget) & GTK_STATE_FLAG_CHECKED;
#endif

        return false;
    }

    virtual void set_menu_item_active(const OString& rIdent, bool bActive) override
    {
        disable_item_notify_events();

        auto aFind = m_aMenuButtonMap.find(rIdent);
        assert (aFind != m_aMenuButtonMap.end());
        aFind->second->set_active(bActive);

        enable_item_notify_events();
    }

    virtual bool get_menu_item_active(const OString& rIdent) const override
    {
        auto aFind = m_aMenuButtonMap.find(rIdent);
        assert (aFind != m_aMenuButtonMap.end());
        return aFind->second->get_active();
    }

    virtual void insert_item(int pos, const OUString& rId) override
    {
        OString sId = OUStringToOString(rId, RTL_TEXTENCODING_UTF8);
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkToolItem* pItem = gtk_tool_button_new(nullptr, sId.getStr());
#else
        GtkWidget* pItem = gtk_button_new();
#endif
        ::set_buildable_id(GTK_BUILDABLE(pItem), sId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_toolbar_insert(m_pToolbar, pItem, pos);
#else
        gtk_box_insert_child_after(m_pToolbar, pItem, toolbar_get_nth_item(pos - 1));
#endif
        gtk_widget_show(GTK_WIDGET(pItem));
        add_to_map(GTK_WIDGET(pItem), nullptr);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        OString sId = OUStringToOString(rId, RTL_TEXTENCODING_UTF8);
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkToolItem* pItem = gtk_separator_tool_item_new();
#else
        GtkWidget* pItem = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
#endif
        ::set_buildable_id(GTK_BUILDABLE(pItem), sId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_toolbar_insert(m_pToolbar, pItem, pos);
#else
        gtk_box_insert_child_after(m_pToolbar, pItem, toolbar_get_nth_item(pos - 1));
#endif
        gtk_widget_show(GTK_WIDGET(pItem));
    }

    virtual void set_item_popover(const OString& rIdent, weld::Widget* pPopover) override
    {
        m_aMenuButtonMap[rIdent]->set_popover(pPopover);
    }

    virtual void set_item_menu(const OString& rIdent, weld::Menu* pMenu) override
    {
        m_aMenuButtonMap[rIdent]->set_menu(pMenu);
    }

    virtual int get_n_items() const override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        return gtk_toolbar_get_n_items(m_pToolbar);
#else
        int n_items = 0;
        for (GtkWidget* pChild = gtk_widget_get_first_child(GTK_WIDGET(m_pToolbar));
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            ++n_items;
        }
        return n_items;
#endif
    }

    virtual OString get_item_ident(int nIndex) const override
    {
        auto* pItem = toolbar_get_nth_item(nIndex);
        return ::get_buildable_id(GTK_BUILDABLE(pItem));
    }

    virtual void set_item_ident(int nIndex, const OString& rIdent) override
    {
        OString sOldIdent(get_item_ident(nIndex));
        m_aMap.erase(m_aMap.find(sOldIdent));

        auto* pItem = toolbar_get_nth_item(nIndex);
        ::set_buildable_id(GTK_BUILDABLE(pItem), rIdent);

        // to keep the ids unique, if the new id is already in use by an item,
        // change the id of that item to the now unused old ident of this item
        auto aFind = m_aMap.find(rIdent);
        if (aFind != m_aMap.end())
        {
            GtkWidget* pDupIdItem = aFind->second;
            ::set_buildable_id(GTK_BUILDABLE(pDupIdItem), sOldIdent);
            m_aMap[sOldIdent] = pDupIdItem;
        }

        m_aMap[rIdent] = pItem;
    }

    virtual void set_item_label(int nIndex, const OUString& rLabel) override
    {
        auto* pItem = toolbar_get_nth_item(nIndex);
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!GTK_IS_TOOL_BUTTON(pItem))
            return;
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(pItem), MapToGtkAccelerator(rLabel).getStr());
#else
        if (!GTK_IS_BUTTON(pItem))
            return;
        ::button_set_label(GTK_BUTTON(pItem), rLabel);
#endif
    }

    virtual void set_item_label(const OString& rIdent, const OUString& rLabel) override
    {
        GtkWidget* pItem = m_aMap[rIdent];
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(pItem), MapToGtkAccelerator(rLabel).getStr());
#else
        if (!pItem || !GTK_IS_BUTTON(pItem))
            return;
        ::button_set_label(GTK_BUTTON(pItem), rLabel);
#endif
    }

    OUString get_item_label(const OString& rIdent) const override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        const gchar* pText = gtk_tool_button_get_label(GTK_TOOL_BUTTON(m_aMap.find(rIdent)->second));
#else
        const gchar* pText = gtk_button_get_label(GTK_BUTTON(m_aMap.find(rIdent)->second));
#endif
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void set_item_icon_name(const OString& rIdent, const OUString& rIconName) override
    {
        GtkWidget* pItem = m_aMap[rIdent];
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;
#else
        if (!pItem || !GTK_IS_BUTTON(pItem))
            return;
#endif

        GtkWidget* pImage = nullptr;

        if (GdkPixbuf* pixbuf = getPixbuf(rIconName))
        {
            pImage = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            gtk_widget_show(pImage);
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pItem), pImage);
#else
        gtk_button_set_child(GTK_BUTTON(pItem), pImage);
        // versions of gtk4 > 4.2.1 might do this on their own
        gtk_widget_remove_css_class(GTK_WIDGET(pItem), "text-button");
#endif
    }

    virtual void set_item_image(const OString& rIdent, const css::uno::Reference<css::graphic::XGraphic>& rIcon) override
    {
        GtkWidget* pItem = m_aMap[rIdent];
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;
        set_item_image(GTK_TOOL_BUTTON(pItem), rIcon);
#else
        if (!pItem)
            return;
        set_item_image(pItem, rIcon);
#endif
    }

    virtual void set_item_image(const OString& rIdent, VirtualDevice* pDevice) override
    {
        GtkWidget* pItem = m_aMap[rIdent];
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;
        set_item_image(GTK_TOOL_BUTTON(pItem), pDevice);
#else
        if (!pItem)
            return;
        set_item_image(pItem, pDevice);
#endif
    }

    virtual void set_item_image(int nIndex, const css::uno::Reference<css::graphic::XGraphic>& rIcon) override
    {
        auto* pItem = toolbar_get_nth_item(nIndex);
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!GTK_IS_TOOL_BUTTON(pItem))
            return;
        set_item_image(GTK_TOOL_BUTTON(pItem), rIcon);
#else
        set_item_image(pItem, rIcon);
#endif
    }

    virtual void set_item_tooltip_text(int nIndex, const OUString& rTip) override
    {
        auto* pItem = toolbar_get_nth_item(nIndex);
        gtk_widget_set_tooltip_text(GTK_WIDGET(pItem), OUStringToOString(rTip, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void set_item_tooltip_text(const OString& rIdent, const OUString& rTip) override
    {
        GtkWidget* pItem = GTK_WIDGET(m_aMap[rIdent]);
        gtk_widget_set_tooltip_text(pItem, OUStringToOString(rTip, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_item_tooltip_text(const OString& rIdent) const override
    {
        GtkWidget* pItem = GTK_WIDGET(m_aMap.find(rIdent)->second);
        const gchar* pStr = gtk_widget_get_tooltip_text(pItem);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual vcl::ImageType get_icon_size() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return m_eImageType;
#else
        return GtkToVcl(gtk_toolbar_get_icon_size(m_pToolbar));
#endif
    }

    virtual void set_icon_size(vcl::ImageType eType) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        m_eImageType = eType;
#else
        gtk_toolbar_set_icon_size(m_pToolbar, VclToGtk(eType));
#endif
    }

    virtual sal_uInt16 get_modifier_state() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GdkDisplay* pDisplay = gtk_widget_get_display(GTK_WIDGET(m_pToolbar));
        GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
        GdkDevice* pDevice = gdk_seat_get_keyboard(pSeat);
        guint nState = gdk_device_get_modifier_state(pDevice);
#else
        GdkKeymap* pKeymap = gdk_keymap_get_default();
        guint nState = gdk_keymap_get_modifier_state(pKeymap);
#endif
        return GtkSalFrame::GetKeyModCode(nState);
    }

    virtual int get_drop_index(const Point& rPoint) const override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        return gtk_toolbar_get_drop_index(m_pToolbar, rPoint.X(), rPoint.Y());
#else
        GtkWidget* pToolbar = GTK_WIDGET(m_pToolbar);
        GtkWidget* pTarget = gtk_widget_pick(pToolbar, rPoint.X(), rPoint.Y(), GTK_PICK_DEFAULT);
        if (!pTarget || pTarget == pToolbar)
            return -1;
        int i = 0;
        for (GtkWidget* pChild = gtk_widget_get_first_child(GTK_WIDGET(m_pToolbar));
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            if (pChild == pTarget)
                return i;
            ++i;
        }
        return -1;
#endif
    }

    virtual bool has_focus() const override
    {
        if (gtk_widget_has_focus(m_pWidget))
            return true;

        GtkWidget* pTopLevel = widget_get_toplevel(m_pWidget);
        if (!GTK_IS_WINDOW(pTopLevel))
            return false;
        GtkWidget* pFocus = gtk_window_get_focus(GTK_WINDOW(pTopLevel));
        if (!pFocus)
            return false;
        return gtk_widget_is_ancestor(pFocus, m_pWidget);
    }

    virtual void grab_focus() override
    {
        if (has_focus())
            return;
        gtk_widget_grab_focus(m_pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
        bool bHasFocusChild = gtk_widget_get_focus_child(m_pWidget);
#else
        bool bHasFocusChild = gtk_container_get_focus_child(GTK_CONTAINER(m_pWidget));
#endif
        if (!bHasFocusChild)
        {
            if (auto* pItem = toolbar_get_nth_item(0))
            {
#if GTK_CHECK_VERSION(4, 0, 0)
                gtk_widget_set_focus_child(m_pWidget, GTK_WIDGET(pItem));
#else
                gtk_container_set_focus_child(GTK_CONTAINER(m_pWidget), GTK_WIDGET(pItem));
#endif
                bHasFocusChild = true;
            }
        }
        if (bHasFocusChild)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            gtk_widget_child_focus(gtk_widget_get_focus_child(m_pWidget), GTK_DIR_TAB_FORWARD);
#else
            gtk_widget_child_focus(gtk_container_get_focus_child(GTK_CONTAINER(m_pWidget)), GTK_DIR_TAB_FORWARD);
#endif
        }
    }

    virtual ~GtkInstanceToolbar() override
    {
        for (auto& a : m_aMap)
            g_signal_handlers_disconnect_by_data(a.second, this);
    }
};

}

namespace {

class GtkInstanceLinkButton : public GtkInstanceWidget, public virtual weld::LinkButton
{
private:
    GtkLinkButton* m_pButton;
    gulong m_nSignalId;

    static bool signalActivateLink(GtkButton*, gpointer widget)
    {
        GtkInstanceLinkButton* pThis = static_cast<GtkInstanceLinkButton*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_activate_link();
    }

public:
    GtkInstanceLinkButton(GtkLinkButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_nSignalId(g_signal_connect(pButton, "activate-link", G_CALLBACK(signalActivateLink), this))
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        ::button_set_label(GTK_BUTTON(m_pButton), rText);
    }

    virtual OUString get_label() const override
    {
        return ::button_get_label(GTK_BUTTON(m_pButton));
    }

    virtual void set_uri(const OUString& rText) override
    {
        gtk_link_button_set_uri(m_pButton, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_uri() const override
    {
        const gchar* pStr = gtk_link_button_get_uri(m_pButton);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pButton, m_nSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pButton, m_nSignalId);
    }

    virtual ~GtkInstanceLinkButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nSignalId);
    }
};

}

namespace {

class GtkInstanceCheckButton : public GtkInstanceWidget, public virtual weld::CheckButton
{
private:
    GtkCheckButton* m_pCheckButton;
    gulong m_nSignalId;

    static void signalToggled(void*, gpointer widget)
    {
        GtkInstanceCheckButton* pThis = static_cast<GtkInstanceCheckButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_toggled();
    }

public:
    GtkInstanceCheckButton(GtkCheckButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pButton), pBuilder, bTakeOwnership)
        , m_pCheckButton(pButton)
        , m_nSignalId(g_signal_connect(m_pCheckButton, "toggled", G_CALLBACK(signalToggled), this))
    {
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_check_button_set_inconsistent(m_pCheckButton, false);
        gtk_check_button_set_active(m_pCheckButton, active);
#else
        gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(m_pCheckButton), false);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pCheckButton), active);
#endif
        enable_notify_events();
    }

    virtual bool get_active() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return gtk_check_button_get_active(m_pCheckButton);
#else
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pCheckButton));
#endif
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_check_button_set_inconsistent(m_pCheckButton, inconsistent);
#else
        gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(m_pCheckButton), inconsistent);
#endif
    }

    virtual bool get_inconsistent() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return gtk_check_button_get_inconsistent(m_pCheckButton);
#else
        return gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(m_pCheckButton));
#endif
    }

    virtual void set_label(const OUString& rText) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_check_button_set_label(m_pCheckButton, MapToGtkAccelerator(rText).getStr());
#else
        ::button_set_label(GTK_BUTTON(m_pCheckButton), rText);
#endif
    }

    virtual OUString get_label() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        const gchar* pStr = gtk_check_button_get_label(m_pCheckButton);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
#else
        return ::button_get_label(GTK_BUTTON(m_pCheckButton));
#endif
    }

    virtual void set_label_wrap(bool bWrap) override
    {
        GtkLabel* pChild = ::get_label_widget(GTK_WIDGET(m_pCheckButton));
        ::set_label_wrap(pChild, bWrap);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pCheckButton, m_nSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pCheckButton, m_nSignalId);
    }

    virtual ~GtkInstanceCheckButton() override
    {
        g_signal_handler_disconnect(m_pCheckButton, m_nSignalId);
    }
};

class GtkInstanceRadioButton : public GtkInstanceCheckButton, public virtual weld::RadioButton
{
public:
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkInstanceRadioButton(GtkCheckButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceCheckButton(pButton, pBuilder, bTakeOwnership)
#else
    GtkInstanceRadioButton(GtkRadioButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceCheckButton(GTK_CHECK_BUTTON(pButton), pBuilder, bTakeOwnership)
#endif
    {
    }
};

}

namespace {

class GtkInstanceScale : public GtkInstanceWidget, public virtual weld::Scale
{
private:
    GtkScale* m_pScale;
    gulong m_nValueChangedSignalId;

    static void signalValueChanged(GtkScale*, gpointer widget)
    {
        GtkInstanceScale* pThis = static_cast<GtkInstanceScale*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_value_changed();
    }

public:
    GtkInstanceScale(GtkScale* pScale, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pScale), pBuilder, bTakeOwnership)
        , m_pScale(pScale)
        , m_nValueChangedSignalId(g_signal_connect(m_pScale, "value-changed", G_CALLBACK(signalValueChanged), this))
    {
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pScale, m_nValueChangedSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pScale, m_nValueChangedSignalId);
    }

    virtual void set_value(int value) override
    {
        disable_notify_events();
        gtk_range_set_value(GTK_RANGE(m_pScale), value);
        enable_notify_events();
    }

    virtual void set_range(int min, int max) override
    {
        disable_notify_events();
        gtk_range_set_range(GTK_RANGE(m_pScale), min, max);
        enable_notify_events();
    }

    virtual void set_increments(int step, int page) override
    {
        disable_notify_events();
        gtk_range_set_increments(GTK_RANGE(m_pScale), step, page);
        enable_notify_events();
    }

    virtual void get_increments(int& step, int& page) const override
    {
        GtkAdjustment* pAdjustment = gtk_range_get_adjustment(GTK_RANGE(m_pScale));
        step = gtk_adjustment_get_step_increment(pAdjustment);
        page = gtk_adjustment_get_page_increment(pAdjustment);
    }

    virtual int get_value() const override
    {
        return gtk_range_get_value(GTK_RANGE(m_pScale));
    }

    virtual ~GtkInstanceScale() override
    {
        g_signal_handler_disconnect(m_pScale, m_nValueChangedSignalId);
    }
};

class GtkInstanceProgressBar : public GtkInstanceWidget, public virtual weld::ProgressBar
{
private:
    GtkProgressBar* m_pProgressBar;

public:
    GtkInstanceProgressBar(GtkProgressBar* pProgressBar, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pProgressBar), pBuilder, bTakeOwnership)
        , m_pProgressBar(pProgressBar)
    {
    }

    virtual void set_percentage(int value) override
    {
        gtk_progress_bar_set_fraction(m_pProgressBar, value / 100.0);
    }

    virtual OUString get_text() const override
    {
        const gchar* pText = gtk_progress_bar_get_text(m_pProgressBar);
        OUString sRet(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        return sRet;
    }

    virtual void set_text(const OUString& rText) override
    {
        gtk_progress_bar_set_text(m_pProgressBar, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
    }
};

class GtkInstanceSpinner : public GtkInstanceWidget, public virtual weld::Spinner
{
private:
    GtkSpinner* m_pSpinner;

public:
    GtkInstanceSpinner(GtkSpinner* pSpinner, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pSpinner), pBuilder, bTakeOwnership)
        , m_pSpinner(pSpinner)
    {
    }

    virtual void start() override
    {
        gtk_spinner_start(m_pSpinner);
    }

    virtual void stop() override
    {
        gtk_spinner_stop(m_pSpinner);
    }
};

class GtkInstanceImage : public GtkInstanceWidget, public virtual weld::Image
{
private:
    GtkImage* m_pImage;

public:
    GtkInstanceImage(GtkImage* pImage, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pImage), pBuilder, bTakeOwnership)
        , m_pImage(pImage)
    {
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        image_set_from_icon_name(m_pImage, rIconName);
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        image_set_from_virtual_device(m_pImage, pDevice);
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        image_set_from_xgraphic(m_pImage, rImage);
    }
};

#if GTK_CHECK_VERSION(4, 0, 0)
class GtkInstancePicture: public GtkInstanceWidget, public virtual weld::Image
{
private:
    GtkPicture* m_pPicture;

public:
    GtkInstancePicture(GtkPicture* pPicture, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pPicture), pBuilder, bTakeOwnership)
        , m_pPicture(pPicture)
    {
        gtk_picture_set_can_shrink(m_pPicture, true);
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        picture_set_from_icon_name(m_pPicture, rIconName);
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        picture_set_from_virtual_device(m_pPicture, pDevice);
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rPicture) override
    {
        picture_set_from_xgraphic(m_pPicture, rPicture);
    }
};
#endif

class GtkInstanceCalendar : public GtkInstanceWidget, public virtual weld::Calendar
{
private:
    GtkCalendar* m_pCalendar;
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkEventController* m_pKeyController;
#endif
    gulong m_nDaySelectedSignalId;
    gulong m_nDaySelectedDoubleClickSignalId;
    gulong m_nKeyPressEventSignalId;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nButtonPressEventSignalId;
#endif

    static void signalDaySelected(GtkCalendar*, gpointer widget)
    {
        GtkInstanceCalendar* pThis = static_cast<GtkInstanceCalendar*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_selected();
    }

    static void signalDaySelectedDoubleClick(GtkCalendar*, gpointer widget)
    {
        GtkInstanceCalendar* pThis = static_cast<GtkInstanceCalendar*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_activated();
    }

    bool signal_key_press(guint nKeyVal)
    {
        if (nKeyVal == GDK_KEY_Return || nKeyVal == GDK_KEY_KP_Enter)
        {
            SolarMutexGuard aGuard;
            signal_activated();
            return true;
        }
        return false;
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalKeyPress(GtkEventControllerKey*, guint nKeyVal, guint /*nKeyCode*/, GdkModifierType, gpointer widget)
    {
        GtkInstanceCalendar* pThis = static_cast<GtkInstanceCalendar*>(widget);
        return pThis->signal_key_press(nKeyVal);
    }
#else
    static gboolean signalKeyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceCalendar* pThis = static_cast<GtkInstanceCalendar*>(widget);
        return pThis->signal_key_press(pEvent->keyval);
    }
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalButton(GtkWidget*, GdkEventButton*, gpointer)
    {
        // don't let button press get to parent window, for the case of the
        // ImplCFieldFloatWin floating window belonging to CalendarField where
        // the click on the calendar continues to the parent GtkWindow and
        // closePopup is called by GtkSalFrame::signalButton because the click
        // window isn't that of the floating parent GtkWindow
        return true;
    }
#endif

public:
    GtkInstanceCalendar(GtkCalendar* pCalendar, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pCalendar), pBuilder, bTakeOwnership)
        , m_pCalendar(pCalendar)
#if GTK_CHECK_VERSION(4, 0, 0)
        , m_pKeyController(gtk_event_controller_key_new())
#endif
        , m_nDaySelectedSignalId(g_signal_connect(pCalendar, "day-selected", G_CALLBACK(signalDaySelected), this))
        , m_nDaySelectedDoubleClickSignalId(g_signal_connect(pCalendar, "day-selected-double-click", G_CALLBACK(signalDaySelectedDoubleClick), this))
#if GTK_CHECK_VERSION(4, 0, 0)
        , m_nKeyPressEventSignalId(g_signal_connect(m_pKeyController, "key-pressed", G_CALLBACK(signalKeyPress), this))
#else
        , m_nKeyPressEventSignalId(g_signal_connect(pCalendar, "key-press-event", G_CALLBACK(signalKeyPress), this))
        , m_nButtonPressEventSignalId(g_signal_connect_after(pCalendar, "button-press-event", G_CALLBACK(signalButton), this))
#endif
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_add_controller(GTK_WIDGET(m_pCalendar), m_pKeyController);
#endif
    }

    virtual void set_date(const Date& rDate) override
    {
        if (!rDate.IsValidAndGregorian())
            return;

        disable_notify_events();
#if GTK_CHECK_VERSION(4, 0, 0)
        GDateTime* pDateTime = g_date_time_new_local(rDate.GetYear(), rDate.GetMonth(), rDate.GetDay(), 0, 0, 0);
        gtk_calendar_select_day(m_pCalendar, pDateTime);
        g_date_time_unref(pDateTime);
#else
        gtk_calendar_select_month(m_pCalendar, rDate.GetMonth() - 1, rDate.GetYear());
        gtk_calendar_select_day(m_pCalendar, rDate.GetDay());
#endif
        enable_notify_events();
    }

    virtual Date get_date() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GDateTime* pDateTime = gtk_calendar_get_date(m_pCalendar);
        Date aDate(g_date_time_get_day_of_month(pDateTime),
                   g_date_time_get_month(pDateTime),
                   g_date_time_get_year(pDateTime));
        g_date_time_unref(pDateTime);
        return aDate;
#else
        guint year, month, day;
        gtk_calendar_get_date(m_pCalendar, &year, &month, &day);
        return Date(day, month + 1, year);
#endif
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pCalendar, m_nDaySelectedDoubleClickSignalId);
        g_signal_handler_block(m_pCalendar, m_nDaySelectedSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pCalendar, m_nDaySelectedSignalId);
        g_signal_handler_unblock(m_pCalendar, m_nDaySelectedDoubleClickSignalId);
    }

    virtual ~GtkInstanceCalendar() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pKeyController, m_nKeyPressEventSignalId);
#else
        g_signal_handler_disconnect(m_pCalendar, m_nButtonPressEventSignalId);
        g_signal_handler_disconnect(m_pCalendar, m_nKeyPressEventSignalId);
#endif
        g_signal_handler_disconnect(m_pCalendar, m_nDaySelectedDoubleClickSignalId);
        g_signal_handler_disconnect(m_pCalendar, m_nDaySelectedSignalId);
    }
};

}

namespace
{
#if GTK_CHECK_VERSION(4, 0, 0)
    // speculative, "error" seems to be a thing but "warning" probably isn't
    void set_widget_css_message_type(GtkWidget* pWidget, weld::EntryMessageType eType)
    {
        switch (eType)
        {
            case weld::EntryMessageType::Normal:
                gtk_widget_remove_css_class(pWidget, "error");
                gtk_widget_remove_css_class(pWidget, "warning");
                break;
            case weld::EntryMessageType::Warning:
                gtk_widget_remove_css_class(pWidget, "error");
                gtk_widget_add_css_class(pWidget, "warning");
                break;
            case weld::EntryMessageType::Error:
                gtk_widget_remove_css_class(pWidget, "warning");
                gtk_widget_add_css_class(pWidget, "error");
                break;
        }
    }
#endif

    void set_entry_message_type(GtkEntry* pEntry, weld::EntryMessageType eType)
    {
        switch (eType)
        {
            case weld::EntryMessageType::Normal:
                gtk_entry_set_icon_from_icon_name(pEntry, GTK_ENTRY_ICON_SECONDARY, nullptr);
                break;
            case weld::EntryMessageType::Warning:
                gtk_entry_set_icon_from_icon_name(pEntry, GTK_ENTRY_ICON_SECONDARY, "dialog-warning");
                break;
            case weld::EntryMessageType::Error:
                gtk_entry_set_icon_from_icon_name(pEntry, GTK_ENTRY_ICON_SECONDARY, "dialog-error");
                break;
        }
    }
}

namespace
{

class GtkInstanceEditable : public GtkInstanceWidget, public virtual weld::Entry
{
protected:
    GtkEditable* m_pEditable;
    GtkWidget* m_pDelegate;
    WidgetFont m_aCustomFont;
private:
    gulong m_nChangedSignalId;
    gulong m_nInsertTextSignalId;
    gulong m_nCursorPosSignalId;
    gulong m_nSelectionPosSignalId;
    gulong m_nActivateSignalId;

    static void signalChanged(GtkEditable*, gpointer widget)
    {
        GtkInstanceEditable* pThis = static_cast<GtkInstanceEditable*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_changed();
    }

    static void signalInsertText(GtkEditable* pEditable, const gchar* pNewText, gint nNewTextLength,
                                 gint* position, gpointer widget)
    {
        GtkInstanceEditable* pThis = static_cast<GtkInstanceEditable*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_insert_text(pEditable, pNewText, nNewTextLength, position);
    }

    void signal_insert_text(GtkEditable* pEditable, const gchar* pNewText, gint nNewTextLength, gint* position)
    {
        if (!m_aInsertTextHdl.IsSet())
            return;
        OUString sText(pNewText, nNewTextLength, RTL_TEXTENCODING_UTF8);
        const bool bContinue = m_aInsertTextHdl.Call(sText);
        if (bContinue && !sText.isEmpty())
        {
            OString sFinalText(OUStringToOString(sText, RTL_TEXTENCODING_UTF8));
            g_signal_handlers_block_by_func(pEditable, reinterpret_cast<gpointer>(signalInsertText), this);
            gtk_editable_insert_text(pEditable, sFinalText.getStr(), sFinalText.getLength(), position);
            g_signal_handlers_unblock_by_func(pEditable, reinterpret_cast<gpointer>(signalInsertText), this);
        }
        g_signal_stop_emission_by_name(pEditable, "insert-text");
    }

    static void signalCursorPosition(void*, GParamSpec*, gpointer widget)
    {
        GtkInstanceEditable* pThis = static_cast<GtkInstanceEditable*>(widget);
        pThis->signal_cursor_position();
    }

    static void signalActivate(void*, gpointer widget)
    {
        GtkInstanceEditable* pThis = static_cast<GtkInstanceEditable*>(widget);
        pThis->signal_activate();
    }

    virtual void ensureMouseEventWidget() override
    {
        // The GtkEntry is sufficient to get mouse events without an intermediate GtkEventBox
        if (!m_pMouseEventBox)
            m_pMouseEventBox = m_pDelegate;
    }

protected:

    virtual void signal_activate()
    {
        if (m_aActivateHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            if (m_aActivateHdl.Call(*this))
                g_signal_stop_emission_by_name(m_pDelegate, "activate");
        }
    }

    PangoAttrList* get_attributes()
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return gtk_text_get_attributes(GTK_TEXT(m_pDelegate));
#else
        return gtk_entry_get_attributes(GTK_ENTRY(m_pDelegate));
#endif
    }

    void set_attributes(PangoAttrList* pAttrs)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_text_set_attributes(GTK_TEXT(m_pDelegate), pAttrs);
#else
        gtk_entry_set_attributes(GTK_ENTRY(m_pDelegate), pAttrs);
#endif
    }

public:
    GtkInstanceEditable(GtkWidget* pWidget, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(pWidget, pBuilder, bTakeOwnership)
        , m_pEditable(GTK_EDITABLE(pWidget))
#if GTK_CHECK_VERSION(4, 0, 0)
        , m_pDelegate(GTK_WIDGET(gtk_editable_get_delegate(m_pEditable)))
#else
        , m_pDelegate(pWidget)
#endif
        , m_aCustomFont(m_pWidget)
        , m_nChangedSignalId(g_signal_connect(m_pEditable, "changed", G_CALLBACK(signalChanged), this))
        , m_nInsertTextSignalId(g_signal_connect(m_pEditable, "insert-text", G_CALLBACK(signalInsertText), this))
        , m_nCursorPosSignalId(g_signal_connect(m_pEditable, "notify::cursor-position", G_CALLBACK(signalCursorPosition), this))
        , m_nSelectionPosSignalId(g_signal_connect(m_pEditable, "notify::selection-bound", G_CALLBACK(signalCursorPosition), this))
        , m_nActivateSignalId(g_signal_connect(m_pDelegate, "activate", G_CALLBACK(signalActivate), this))
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_editable_set_text(m_pEditable, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
#else
        gtk_entry_set_text(GTK_ENTRY(m_pDelegate), OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
#endif
        enable_notify_events();
    }

    virtual OUString get_text() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        const gchar* pText = gtk_editable_get_text(m_pEditable);
#else
        const gchar* pText = gtk_entry_get_text(GTK_ENTRY(m_pDelegate));
#endif
        OUString sRet(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        return sRet;
    }

    virtual void set_width_chars(int nChars) override
    {
        disable_notify_events();
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_editable_set_width_chars(m_pEditable, nChars);
        gtk_editable_set_max_width_chars(m_pEditable, nChars);
#else
        gtk_entry_set_width_chars(GTK_ENTRY(m_pDelegate), nChars);
        gtk_entry_set_max_width_chars(GTK_ENTRY(m_pDelegate), nChars);
#endif
        enable_notify_events();
    }

    virtual int get_width_chars() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return gtk_editable_get_width_chars(m_pEditable);
#else
        return gtk_entry_get_width_chars(GTK_ENTRY(m_pDelegate));
#endif
    }

    virtual void set_max_length(int nChars) override
    {
        disable_notify_events();
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_text_set_max_length(GTK_TEXT(m_pDelegate), nChars);
#else
        gtk_entry_set_max_length(GTK_ENTRY(m_pDelegate), nChars);
#endif
        enable_notify_events();
    }

    virtual void select_region(int nStartPos, int nEndPos) override
    {
        disable_notify_events();
        gtk_editable_select_region(m_pEditable, nStartPos, nEndPos);
        enable_notify_events();
    }

    bool get_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        return gtk_editable_get_selection_bounds(m_pEditable, &rStartPos, &rEndPos);
    }

    virtual void replace_selection(const OUString& rText) override
    {
        disable_notify_events();
        gtk_editable_delete_selection(m_pEditable);
        OString sText(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gint position = gtk_editable_get_position(m_pEditable);
        gtk_editable_insert_text(m_pEditable, sText.getStr(), sText.getLength(),
                                 &position);
        enable_notify_events();
    }

    virtual void set_position(int nCursorPos) override
    {
        disable_notify_events();
        gtk_editable_set_position(m_pEditable, nCursorPos);
        enable_notify_events();
    }

    virtual int get_position() const override
    {
        return gtk_editable_get_position(m_pEditable);
    }

    virtual void set_editable(bool bEditable) override
    {
        gtk_editable_set_editable(m_pEditable, bEditable);
    }

    virtual bool get_editable() const override
    {
        return gtk_editable_get_editable(m_pEditable);
    }

    virtual void set_overwrite_mode(bool bOn) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_text_set_overwrite_mode(GTK_TEXT(m_pDelegate), bOn);
#else
        gtk_entry_set_overwrite_mode(GTK_ENTRY(m_pDelegate), bOn);
#endif
    }

    virtual bool get_overwrite_mode() const override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        return gtk_text_get_overwrite_mode(GTK_TEXT(m_pDelegate));
#else
        return gtk_entry_get_overwrite_mode(GTK_ENTRY(m_pDelegate));
#endif
    }

    virtual void set_message_type(weld::EntryMessageType eType) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        if (!GTK_IS_ENTRY(m_pDelegate))
        {
            ::set_widget_css_message_type(m_pDelegate, eType);
            return;
        }
#endif
        ::set_entry_message_type(GTK_ENTRY(m_pDelegate), eType);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pDelegate, m_nActivateSignalId);
        g_signal_handler_block(m_pEditable, m_nSelectionPosSignalId);
        g_signal_handler_block(m_pEditable, m_nCursorPosSignalId);
        g_signal_handler_block(m_pEditable, m_nInsertTextSignalId);
        g_signal_handler_block(m_pEditable, m_nChangedSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pEditable, m_nChangedSignalId);
        g_signal_handler_unblock(m_pEditable, m_nInsertTextSignalId);
        g_signal_handler_unblock(m_pEditable, m_nCursorPosSignalId);
        g_signal_handler_unblock(m_pEditable, m_nSelectionPosSignalId);
        g_signal_handler_unblock(m_pDelegate, m_nActivateSignalId);
    }

    virtual vcl::Font get_font() override
    {
        if (const vcl::Font* pFont = m_aCustomFont.get_custom_font())
            return *pFont;
        return GtkInstanceWidget::get_font();
    }

    void set_font_color(const Color& rColor) override
    {
        PangoAttrList* pOrigList = get_attributes();
        if (rColor == COL_AUTO && !pOrigList) // nothing to do
            return;

        PangoAttrType aFilterAttrs[] = {PANGO_ATTR_FOREGROUND, PANGO_ATTR_INVALID};

        PangoAttrList* pAttrs = pOrigList ? pango_attr_list_copy(pOrigList) : pango_attr_list_new();
        PangoAttrList* pRemovedAttrs = pOrigList ? pango_attr_list_filter(pAttrs, filter_pango_attrs, &aFilterAttrs) : nullptr;

        if (rColor != COL_AUTO)
            pango_attr_list_insert(pAttrs, pango_attr_foreground_new(rColor.GetRed()/255.0, rColor.GetGreen()/255.0, rColor.GetBlue()/255.0));

        set_attributes(pAttrs);
        pango_attr_list_unref(pAttrs);
        pango_attr_list_unref(pRemovedAttrs);
    }

    void fire_signal_changed()
    {
        signal_changed();
    }

    virtual void cut_clipboard() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_activate_action(m_pDelegate, "cut.clipboard", nullptr);
#else
        gtk_editable_cut_clipboard(m_pEditable);
#endif
    }

    virtual void copy_clipboard() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_activate_action(m_pDelegate, "copy.clipboard", nullptr);
#else
        gtk_editable_copy_clipboard(m_pEditable);
#endif
    }

    virtual void paste_clipboard() override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_activate_action(m_pDelegate, "paste.clipboard", nullptr);
#else
        gtk_editable_paste_clipboard(m_pEditable);
#endif
    }

    virtual void set_placeholder_text(const OUString& rText) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_text_set_placeholder_text(GTK_TEXT(m_pDelegate), rText.toUtf8().getStr());
#else
        gtk_entry_set_placeholder_text(GTK_ENTRY(m_pDelegate), rText.toUtf8().getStr());
#endif
    }

    virtual void grab_focus() override
    {
        if (has_focus())
            return;
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_text_grab_focus_without_selecting(GTK_TEXT(m_pDelegate));
#else
        gtk_entry_grab_focus_without_selecting(GTK_ENTRY(m_pDelegate));
#endif
    }

    virtual void set_alignment(TxtAlign eXAlign) override
    {
        gfloat xalign = 0;
        switch (eXAlign)
        {
            case TxtAlign::Left:
                xalign = 0.0;
                break;
            case TxtAlign::Center:
                xalign = 0.5;
                break;
            case TxtAlign::Right:
                xalign = 1.0;
                break;
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_editable_set_alignment(m_pEditable, xalign);
#else
        gtk_entry_set_alignment(GTK_ENTRY(m_pDelegate), xalign);
#endif
    }

    virtual ~GtkInstanceEditable() override
    {
        g_signal_handler_disconnect(m_pDelegate, m_nActivateSignalId);
        g_signal_handler_disconnect(m_pEditable, m_nSelectionPosSignalId);
        g_signal_handler_disconnect(m_pEditable, m_nCursorPosSignalId);
        g_signal_handler_disconnect(m_pEditable, m_nInsertTextSignalId);
        g_signal_handler_disconnect(m_pEditable, m_nChangedSignalId);
    }
};

class GtkInstanceEntry : public GtkInstanceEditable
{
public:
    GtkInstanceEntry(GtkEntry* pEntry, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceEditable(GTK_WIDGET(pEntry), pBuilder, bTakeOwnership)
    {
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_aCustomFont.use_custom_font(&rFont, u"entry");
    }
};

}

namespace
{

    struct Search
    {
        OString str;
        int index;
        int col;
        Search(std::u16string_view rText, int nCol)
            : str(OUStringToOString(rText, RTL_TEXTENCODING_UTF8))
            , index(-1)
            , col(nCol)
        {
        }
    };

    gboolean foreach_find(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer data)
    {
        Search* search = static_cast<Search*>(data);
        gchar *pStr = nullptr;
        gtk_tree_model_get(model, iter, search->col, &pStr, -1);
        bool found = strcmp(pStr, search->str.getStr()) == 0;
        if (found)
        {
            gint depth;
            gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
            search->index = indices[depth-1];
        }
        g_free(pStr);
        return found;
    }

    void insert_row(GtkListStore* pListStore, GtkTreeIter& iter, int pos, const OUString* pId, std::u16string_view rText, const OUString* pIconName, const VirtualDevice* pDevice)
    {
        if (!pIconName && !pDevice)
        {
            gtk_list_store_insert_with_values(pListStore, &iter, pos,
                                              0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
                                              1, !pId ? nullptr : OUStringToOString(*pId, RTL_TEXTENCODING_UTF8).getStr(),
                                              -1);
        }
        else
        {
            if (pIconName)
            {
                GdkPixbuf* pixbuf = getPixbuf(*pIconName);

                gtk_list_store_insert_with_values(pListStore, &iter, pos,
                                                  0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
                                                  1, !pId ? nullptr : OUStringToOString(*pId, RTL_TEXTENCODING_UTF8).getStr(),
                                                  2, pixbuf,
                                                  -1);

                if (pixbuf)
                    g_object_unref(pixbuf);
            }
            else
            {
                cairo_surface_t* surface = get_underlying_cairo_surface(*pDevice);

                Size aSize(pDevice->GetOutputSizePixel());
                cairo_surface_t* target = cairo_surface_create_similar(surface,
                                                                        cairo_surface_get_content(surface),
                                                                        aSize.Width(),
                                                                        aSize.Height());

                cairo_t* cr = cairo_create(target);
                cairo_set_source_surface(cr, surface, 0, 0);
                cairo_paint(cr);
                cairo_destroy(cr);

                gtk_list_store_insert_with_values(pListStore, &iter, pos,
                                                  0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
                                                  1, !pId ? nullptr : OUStringToOString(*pId, RTL_TEXTENCODING_UTF8).getStr(),
                                                  3, target,
                                                  -1);
                cairo_surface_destroy(target);
            }
        }
    }
}

namespace
{
    gint default_sort_func(GtkTreeModel* pModel, GtkTreeIter* a, GtkTreeIter* b, gpointer data)
    {
        comphelper::string::NaturalStringSorter* pSorter = static_cast<comphelper::string::NaturalStringSorter*>(data);
        gchar* pName1;
        gchar* pName2;
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
        gint sort_column_id(0);
        gtk_tree_sortable_get_sort_column_id(pSortable, &sort_column_id, nullptr);
        gtk_tree_model_get(pModel, a, sort_column_id, &pName1, -1);
        gtk_tree_model_get(pModel, b, sort_column_id, &pName2, -1);
        gint ret = pSorter->compare(OUString(pName1, pName1 ? strlen(pName1) : 0, RTL_TEXTENCODING_UTF8),
                                    OUString(pName2, pName2 ? strlen(pName2) : 0, RTL_TEXTENCODING_UTF8));
        g_free(pName1);
        g_free(pName2);
        return ret;
    }

    int starts_with(GtkTreeModel* pTreeModel, const OUString& rStr, int col, int nStartRow, bool bCaseSensitive)
    {
        GtkTreeIter iter;
        if (!gtk_tree_model_iter_nth_child(pTreeModel, &iter, nullptr, nStartRow))
            return -1;

        const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
        int nRet = nStartRow;
        do
        {
            gchar* pStr;
            gtk_tree_model_get(pTreeModel, &iter, col, &pStr, -1);
            OUString aStr(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pStr);
            const bool bMatch = !bCaseSensitive ? rI18nHelper.MatchString(rStr, aStr) : aStr.startsWith(rStr);
            if (bMatch)
                return nRet;
            ++nRet;
        } while (gtk_tree_model_iter_next(pTreeModel, &iter));

        return -1;
    }

    struct GtkInstanceTreeIter : public weld::TreeIter
    {
        GtkInstanceTreeIter(const GtkInstanceTreeIter* pOrig)
        {
            if (pOrig)
                iter = pOrig->iter;
            else
                memset(&iter, 0, sizeof(iter));
        }
        GtkInstanceTreeIter(const GtkTreeIter& rOrig)
        {
            memcpy(&iter, &rOrig, sizeof(iter));
        }
        virtual bool equal(const TreeIter& rOther) const override
        {
            return memcmp(&iter,  &static_cast<const GtkInstanceTreeIter&>(rOther).iter, sizeof(GtkTreeIter)) == 0;
        }
        GtkTreeIter iter;
    };

    class GtkInstanceTreeView;

}

static GtkInstanceTreeView* g_DragSource;

namespace {

struct CompareGtkTreePath
{
    bool operator()(const GtkTreePath* lhs, const GtkTreePath* rhs) const
    {
        return gtk_tree_path_compare(lhs, rhs) < 0;
    }
};

int get_height_row(GtkTreeView* pTreeView, GList* pColumns)
{
    gint nMaxRowHeight = 0;
    for (GList* pEntry = g_list_first(pColumns); pEntry; pEntry = g_list_next(pEntry))
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
        GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
        for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
        {
            GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
            gint nRowHeight;
            gtk_cell_renderer_get_preferred_height(pCellRenderer, GTK_WIDGET(pTreeView), nullptr, &nRowHeight);
            nMaxRowHeight = std::max(nMaxRowHeight, nRowHeight);
        }
        g_list_free(pRenderers);
    }
    return nMaxRowHeight;
}

int get_height_row_separator(GtkTreeView* pTreeView)
{
    // gtk4: _TREE_VIEW_VERTICAL_SEPARATOR define in gtk/gtktreeview.c
    gint nVerticalSeparator = 2;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_style_get(GTK_WIDGET(pTreeView), "vertical-separator", &nVerticalSeparator, nullptr);
#else
    (void)pTreeView;
#endif
    return nVerticalSeparator;
}

int get_height_rows(GtkTreeView* pTreeView, GList* pColumns, int nRows)
{
    gint nMaxRowHeight = get_height_row(pTreeView, pColumns);
    gint nVerticalSeparator = get_height_row_separator(pTreeView);
    return (nMaxRowHeight * nRows) + (nVerticalSeparator * nRows) / 2;
}

#if !GTK_CHECK_VERSION(4, 0, 0)
int get_height_rows(int nRowHeight, int nSeparatorHeight, int nRows)
{
    return (nRowHeight * nRows) + (nSeparatorHeight * (nRows + 1));
}
#endif

tools::Rectangle get_row_area(GtkTreeView* pTreeView, GList* pColumns, GtkTreePath* pPath)
{
    tools::Rectangle aRet;

    GdkRectangle aRect;
    for (GList* pEntry = g_list_last(pColumns); pEntry; pEntry = g_list_previous(pEntry))
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
        gtk_tree_view_get_cell_area(pTreeView, pPath, pColumn, &aRect);
        aRet.Union(tools::Rectangle(aRect.x, aRect.y, aRect.x + aRect.width, aRect.y + aRect.height));
    }

    return aRet;
}

struct GtkTreeRowReferenceDeleter
{
    void operator()(GtkTreeRowReference* p) const
    {
        gtk_tree_row_reference_free(p);
    }
};

bool separator_function(const GtkTreePath* path, const std::vector<std::unique_ptr<GtkTreeRowReference, GtkTreeRowReferenceDeleter>>& rSeparatorRows)
{
    bool bFound = false;
    for (auto& a : rSeparatorRows)
    {
        GtkTreePath* seppath = gtk_tree_row_reference_get_path(a.get());
        if (seppath)
        {
            bFound = gtk_tree_path_compare(path, seppath) == 0;
            gtk_tree_path_free(seppath);
        }
        if (bFound)
            break;
    }
    return bFound;
}

void tree_store_set(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, ...)
{
    va_list args;

    va_start(args, pIter);
    gtk_tree_store_set_valist(GTK_TREE_STORE(pTreeModel), pIter, args);
    va_end(args);
}

void list_store_set(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, ...)
{
    va_list args;

    va_start(args, pIter);
    gtk_list_store_set_valist(GTK_LIST_STORE(pTreeModel), pIter, args);
    va_end(args);
}

void tree_store_insert_with_values(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, GtkTreeIter *pParent, gint nPos,
                                   gint nTextCol, const gchar* pText,
                                   gint nIdCol, const gchar* pId)
{
    gtk_tree_store_insert_with_values(GTK_TREE_STORE(pTreeModel), pIter, pParent, nPos,
                                      nTextCol, pText, nIdCol, pId, -1);
}

void list_store_insert_with_values(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, GtkTreeIter *pParent, gint nPos,
                                   gint nTextCol, const gchar* pText,
                                   gint nIdCol, const gchar* pId)
{
    assert(!pParent); (void)pParent;
    gtk_list_store_insert_with_values(GTK_LIST_STORE(pTreeModel), pIter, nPos,
                                      nTextCol, pText, nIdCol, pId, -1);
}

void tree_store_prepend(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, GtkTreeIter *pParent)
{
    gtk_tree_store_prepend(GTK_TREE_STORE(pTreeModel), pIter, pParent);
}

void list_store_prepend(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, GtkTreeIter *pParent)
{
    assert(!pParent); (void)pParent;
    gtk_list_store_prepend(GTK_LIST_STORE(pTreeModel), pIter);
}

void tree_store_insert(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, GtkTreeIter *pParent, gint nPosition)
{
    gtk_tree_store_insert(GTK_TREE_STORE(pTreeModel), pIter, pParent, nPosition);
}

void list_store_insert(GtkTreeModel* pTreeModel, GtkTreeIter *pIter, GtkTreeIter *pParent, gint nPosition)
{
    assert(!pParent); (void)pParent;
    gtk_list_store_insert(GTK_LIST_STORE(pTreeModel), pIter, nPosition);
}

void tree_store_clear(GtkTreeModel* pTreeModel)
{
    gtk_tree_store_clear(GTK_TREE_STORE(pTreeModel));
}

void list_store_clear(GtkTreeModel* pTreeModel)
{
    gtk_list_store_clear(GTK_LIST_STORE(pTreeModel));
}

bool tree_store_remove(GtkTreeModel* pTreeModel, GtkTreeIter *pIter)
{
    return gtk_tree_store_remove(GTK_TREE_STORE(pTreeModel), pIter);
}

bool list_store_remove(GtkTreeModel* pTreeModel, GtkTreeIter *pIter)
{
    return gtk_list_store_remove(GTK_LIST_STORE(pTreeModel), pIter);
}

void tree_store_swap(GtkTreeModel* pTreeModel, GtkTreeIter* pIter1, GtkTreeIter* pIter2)
{
    gtk_tree_store_swap(GTK_TREE_STORE(pTreeModel), pIter1, pIter2);
}

void list_store_swap(GtkTreeModel* pTreeModel, GtkTreeIter* pIter1, GtkTreeIter* pIter2)
{
    gtk_list_store_swap(GTK_LIST_STORE(pTreeModel), pIter1, pIter2);
}

void tree_store_set_value(GtkTreeModel* pTreeModel, GtkTreeIter* pIter, gint nColumn, GValue* pValue)
{
    gtk_tree_store_set_value(GTK_TREE_STORE(pTreeModel), pIter, nColumn, pValue);
}

void list_store_set_value(GtkTreeModel* pTreeModel, GtkTreeIter* pIter, gint nColumn, GValue* pValue)
{
    gtk_list_store_set_value(GTK_LIST_STORE(pTreeModel), pIter, nColumn, pValue);
}

int promote_arg(bool bArg)
{
    return static_cast<int>(bArg);
}

class GtkInstanceTreeView : public GtkInstanceWidget, public virtual weld::TreeView
{
private:
    GtkTreeView* m_pTreeView;
    GtkTreeModel* m_pTreeModel;

    typedef void(*setterFnc)(GtkTreeModel*, GtkTreeIter*, ...);
    setterFnc m_Setter;

    typedef void(*insertWithValuesFnc)(GtkTreeModel*, GtkTreeIter*, GtkTreeIter*, gint, gint, const gchar*, gint, const gchar*);
    insertWithValuesFnc m_InsertWithValues;

    typedef void(*insertFnc)(GtkTreeModel*, GtkTreeIter*, GtkTreeIter*, gint);
    insertFnc m_Insert;

    typedef void(*prependFnc)(GtkTreeModel*, GtkTreeIter*, GtkTreeIter*);
    prependFnc m_Prepend;

    typedef void(*clearFnc)(GtkTreeModel*);
    clearFnc m_Clear;

    typedef bool(*removeFnc)(GtkTreeModel*, GtkTreeIter*);
    removeFnc m_Remove;

    typedef void(*swapFnc)(GtkTreeModel*, GtkTreeIter*, GtkTreeIter*);
    swapFnc m_Swap;

    typedef void(*setValueFnc)(GtkTreeModel*, GtkTreeIter*, gint, GValue*);
    setValueFnc m_SetValue;

    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    GList *m_pColumns;
    std::vector<gulong> m_aColumnSignalIds;
    // map from toggle column to toggle visibility column
    std::map<int, int> m_aToggleVisMap;
    // map from toggle column to tristate column
    std::map<int, int> m_aToggleTriStateMap;
    // map from text column to text weight column
    std::map<int, int> m_aWeightMap;
    // map from text column to sensitive column
    std::map<int, int> m_aSensitiveMap;
    // map from text column to indent column
    std::map<int, int> m_aIndentMap;
    // map from text column to text align column
    std::map<int, int> m_aAlignMap;
    // currently expanding parent that logically, but not currently physically,
    // contain placeholders
    o3tl::sorted_vector<GtkTreePath*, CompareGtkTreePath> m_aExpandingPlaceHolderParents;
    // which rows are separators (rare)
    std::vector<std::unique_ptr<GtkTreeRowReference, GtkTreeRowReferenceDeleter>> m_aSeparatorRows;
    std::vector<GtkSortType> m_aSavedSortTypes;
    std::vector<int> m_aSavedSortColumns;
    bool m_bWorkAroundBadDragRegion;
    bool m_bInDrag;
    gint m_nTextCol;
    gint m_nTextView;
    gint m_nImageCol;
    gint m_nExpanderToggleCol;
    gint m_nExpanderImageCol;
    gint m_nIdCol;
    int m_nPendingVAdjustment;
    gulong m_nChangedSignalId;
    gulong m_nRowActivatedSignalId;
    gulong m_nTestExpandRowSignalId;
    gulong m_nTestCollapseRowSignalId;
    gulong m_nVAdjustmentChangedSignalId;
    gulong m_nRowDeletedSignalId;
    gulong m_nRowInsertedSignalId;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nPopupMenuSignalId;
    gulong m_nKeyPressSignalId;
#endif
    gulong m_nQueryTooltipSignalId;
    GtkAdjustment* m_pVAdjustment;
    ImplSVEvent* m_pChangeEvent;

    DECL_LINK(async_signal_changed, void*, void);

    void launch_signal_changed()
    {
        //tdf#117991 selection change is sent before the focus change, and focus change
        //is what will cause a spinbutton that currently has the focus to set its contents
        //as the spin button value. So any LibreOffice callbacks on
        //signal-change would happen before the spinbutton value-change occurs.
        //To avoid this, send the signal-change to LibreOffice to occur after focus-change
        //has been processed
        if (m_pChangeEvent)
            Application::RemoveUserEvent(m_pChangeEvent);
        m_pChangeEvent = Application::PostUserEvent(LINK(this, GtkInstanceTreeView, async_signal_changed));
    }

    static void signalChanged(GtkTreeView*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->launch_signal_changed();
    }

    void handle_row_activated()
    {
        if (signal_row_activated())
            return;
        GtkInstanceTreeIter aIter(nullptr);
        if (!get_cursor(&aIter))
            return;
        if (gtk_tree_model_iter_has_child(m_pTreeModel, &aIter.iter))
            get_row_expanded(aIter) ? collapse_row(aIter) : expand_row(aIter);
    }

    static void signalRowActivated(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        SolarMutexGuard aGuard;
        pThis->handle_row_activated();
    }

    virtual bool signal_popup_menu(const CommandEvent& rCEvt) override
    {
        return m_aPopupMenuHdl.Call(rCEvt);
    }

    void insert_row(GtkTreeIter& iter, const GtkTreeIter* parent, int pos, const OUString* pId, const OUString* pText,
                    const OUString* pIconName, const VirtualDevice* pDevice)
    {
        m_InsertWithValues(m_pTreeModel, &iter, const_cast<GtkTreeIter*>(parent), pos,
                           m_nTextCol, !pText ? nullptr : OUStringToOString(*pText, RTL_TEXTENCODING_UTF8).getStr(),
                           m_nIdCol, !pId ? nullptr : OUStringToOString(*pId, RTL_TEXTENCODING_UTF8).getStr());

        if (pIconName)
        {
            GdkPixbuf* pixbuf = getPixbuf(*pIconName);
            m_Setter(m_pTreeModel, &iter, m_nImageCol, pixbuf, -1);
            if (pixbuf)
                g_object_unref(pixbuf);
        }
        else if (pDevice)
        {
            cairo_surface_t* surface = get_underlying_cairo_surface(*pDevice);

            Size aSize(pDevice->GetOutputSizePixel());
            cairo_surface_t* target = cairo_surface_create_similar(surface,
                                                                    cairo_surface_get_content(surface),
                                                                    aSize.Width(),
                                                                    aSize.Height());

            cairo_t* cr = cairo_create(target);
            cairo_set_source_surface(cr, surface, 0, 0);
            cairo_paint(cr);
            cairo_destroy(cr);

            m_Setter(m_pTreeModel, &iter, m_nImageCol, target, -1);
            cairo_surface_destroy(target);
        }
    }

    bool separator_function(const GtkTreePath* path)
    {
        return ::separator_function(path, m_aSeparatorRows);
    }

    static gboolean separatorFunction(GtkTreeModel* pTreeModel, GtkTreeIter* pIter, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        GtkTreePath* path = gtk_tree_model_get_path(pTreeModel, pIter);
        bool bRet = pThis->separator_function(path);
        gtk_tree_path_free(path);
        return bRet;
    }

    OUString get(const GtkTreeIter& iter, int col) const
    {
        gchar* pStr;
        gtk_tree_model_get(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, &pStr, -1);
        OUString sRet(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);
        return sRet;
    }

    OUString get(int pos, int col) const
    {
        OUString sRet;
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            sRet = get(iter, col);
        return sRet;
    }

    gint get_int(const GtkTreeIter& iter, int col) const
    {
        gint nRet(-1);
        gtk_tree_model_get(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, &nRet, -1);
        return nRet;
    }

    gint get_int(int pos, int col) const
    {
        gint nRet(-1);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            nRet = get_int(iter, col);
        gtk_tree_model_get(m_pTreeModel, &iter, col, &nRet, -1);
        return nRet;
    }

    bool get_bool(const GtkTreeIter& iter, int col) const
    {
        gboolean bRet(false);
        gtk_tree_model_get(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, &bRet, -1);
        return bRet;
    }

    bool get_bool(int pos, int col) const
    {
        bool bRet(false);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            bRet = get_bool(iter, col);
        return bRet;
    }

    void set_toggle(const GtkTreeIter& iter, TriState eState, int col)
    {
        if (col == -1)
            col = m_nExpanderToggleCol;
        else
            col = to_internal_model(col);

        if (eState == TRISTATE_INDET)
        {
            m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter),
                     m_aToggleVisMap[col], promote_arg(true), // checkbuttons are invisible until toggled on or off
                     m_aToggleTriStateMap[col], promote_arg(true), // tristate on
                     -1);
        }
        else
        {
            m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter),
                     m_aToggleVisMap[col], promote_arg(true), // checkbuttons are invisible until toggled on or off
                     m_aToggleTriStateMap[col], promote_arg(false), // tristate off
                     col, promote_arg(eState == TRISTATE_TRUE), // set toggle state
                     -1);
        }
    }

    void set(const GtkTreeIter& iter, int col, std::u16string_view rText)
    {
        OString aStr(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, aStr.getStr(), -1);
    }

    void set(int pos, int col, std::u16string_view rText)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            set(iter, col, rText);
    }

    void set(const GtkTreeIter& iter, int col, bool bOn)
    {
        m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, promote_arg(bOn), -1);
    }

    void set(int pos, int col, bool bOn)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            set(iter, col, bOn);
    }

    void set(const GtkTreeIter& iter, int col, gint bInt)
    {
        m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, bInt, -1);
    }

    void set(int pos, int col, gint bInt)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            set(iter, col, bInt);
    }

    void set(const GtkTreeIter& iter, int col, double fValue)
    {
        m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, fValue, -1);
    }

    void set(int pos, int col, double fValue)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            set(iter, col, fValue);
    }

    static gboolean signalTestExpandRow(GtkTreeView*, GtkTreeIter* iter, GtkTreePath*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        return !pThis->signal_test_expand_row(*iter);
    }

    static gboolean signalTestCollapseRow(GtkTreeView*, GtkTreeIter* iter, GtkTreePath*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        return !pThis->signal_test_collapse_row(*iter);
    }

    bool child_is_placeholder(GtkInstanceTreeIter& rGtkIter) const
    {
        GtkTreePath* pPath = gtk_tree_model_get_path(m_pTreeModel, &rGtkIter.iter);
        bool bExpanding = m_aExpandingPlaceHolderParents.count(pPath);
        gtk_tree_path_free(pPath);
        if (bExpanding)
            return true;

        bool bPlaceHolder = false;
        GtkTreeIter tmp;
        if (gtk_tree_model_iter_children(m_pTreeModel, &tmp, &rGtkIter.iter))
        {
            rGtkIter.iter = tmp;
            if (get_text(rGtkIter, -1) == "<dummy>")
            {
                bPlaceHolder = true;
            }
        }
        return bPlaceHolder;
    }

    bool signal_test_expand_row(GtkTreeIter& iter)
    {
        disable_notify_events();

        // if there's a preexisting placeholder child, required to make this
        // potentially expandable in the first place, now we remove it
        GtkInstanceTreeIter aIter(iter);
        GtkTreePath* pPlaceHolderPath = nullptr;
        bool bPlaceHolder = child_is_placeholder(aIter);
        if (bPlaceHolder)
        {
            m_Remove(m_pTreeModel, &aIter.iter);

            pPlaceHolderPath = gtk_tree_model_get_path(m_pTreeModel, &iter);
            m_aExpandingPlaceHolderParents.insert(pPlaceHolderPath);
        }

        aIter.iter = iter;
        bool bRet = signal_expanding(aIter);

        if (bPlaceHolder)
        {
            //expand disallowed, restore placeholder
            if (!bRet)
            {
                GtkTreeIter subiter;
                OUString sDummy("<dummy>");
                insert_row(subiter, &iter, -1, nullptr, &sDummy, nullptr, nullptr);
            }
            m_aExpandingPlaceHolderParents.erase(pPlaceHolderPath);
            gtk_tree_path_free(pPlaceHolderPath);
        }

        enable_notify_events();
        return bRet;
    }

    bool signal_test_collapse_row(const GtkTreeIter& iter)
    {
        disable_notify_events();

        GtkInstanceTreeIter aIter(iter);
        bool bRet = signal_collapsing(aIter);

        enable_notify_events();
        return bRet;
    }

    static void signalCellToggled(GtkCellRendererToggle* pCell, const gchar *path, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        void* pData = g_object_get_data(G_OBJECT(pCell), "g-lo-CellIndex");
        pThis->signal_cell_toggled(path, reinterpret_cast<sal_IntPtr>(pData));
    }

    void signal_cell_toggled(const gchar *path, int nCol)
    {
        GtkTreePath *tree_path = gtk_tree_path_new_from_string(path);

        // additionally set the cursor into the row the toggled element is in
        gtk_tree_view_set_cursor(m_pTreeView, tree_path, nullptr, false);

        GtkTreeIter iter;
        gtk_tree_model_get_iter(m_pTreeModel, &iter, tree_path);

        gboolean bRet(false);
        gtk_tree_model_get(m_pTreeModel, &iter, nCol, &bRet, -1);
        bRet = !bRet;
        m_Setter(m_pTreeModel, &iter, nCol, bRet, -1);

        set(iter, m_aToggleTriStateMap[nCol], false);

        signal_toggled(iter_col(GtkInstanceTreeIter(iter), to_external_model(nCol)));

        gtk_tree_path_free(tree_path);
    }

    DECL_LINK(async_stop_cell_editing, void*, void);

    static void signalCellEditingStarted(GtkCellRenderer*, GtkCellEditable*, const gchar *path, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        if (!pThis->signal_cell_editing_started(path))
            Application::PostUserEvent(LINK(pThis, GtkInstanceTreeView, async_stop_cell_editing));
    }

    bool signal_cell_editing_started(const gchar *path)
    {
        GtkTreePath *tree_path = gtk_tree_path_new_from_string(path);

        GtkInstanceTreeIter aGtkIter(nullptr);
        gtk_tree_model_get_iter(m_pTreeModel, &aGtkIter.iter, tree_path);
        gtk_tree_path_free(tree_path);

        return signal_editing_started(aGtkIter);
    }

    static void signalCellEdited(GtkCellRendererText* pCell, const gchar *path, const gchar *pNewText, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_cell_edited(pCell, path, pNewText);
    }

    static void restoreNonEditable(GObject* pCell)
    {
        if (g_object_get_data(pCell, "g-lo-RestoreNonEditable"))
        {
            g_object_set(pCell, "editable", false, "editable-set", false, nullptr);
            g_object_set_data(pCell, "g-lo-RestoreNonEditable", reinterpret_cast<gpointer>(false));
        }
    }

    void signal_cell_edited(GtkCellRendererText* pCell, const gchar *path, const gchar* pNewText)
    {
        GtkTreePath *tree_path = gtk_tree_path_new_from_string(path);

        GtkInstanceTreeIter aGtkIter(nullptr);
        gtk_tree_model_get_iter(m_pTreeModel, &aGtkIter.iter, tree_path);
        gtk_tree_path_free(tree_path);

        OUString sText(pNewText, pNewText ? strlen(pNewText) : 0, RTL_TEXTENCODING_UTF8);
        if (signal_editing_done(iter_string(aGtkIter, sText)))
        {
            void* pData = g_object_get_data(G_OBJECT(pCell), "g-lo-CellIndex");
            set(aGtkIter.iter, reinterpret_cast<sal_IntPtr>(pData), sText);
        }

        restoreNonEditable(G_OBJECT(pCell));
    }

    static void signalCellEditingCanceled(GtkCellRenderer* pCell, gpointer /*widget*/)
    {
        restoreNonEditable(G_OBJECT(pCell));
    }

    void signal_column_clicked(GtkTreeViewColumn* pClickedColumn)
    {
        int nIndex(0);
        for (GList* pEntry = g_list_first(m_pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            if (pColumn == pClickedColumn)
            {
                TreeView::signal_column_clicked(nIndex);
                break;
            }
            ++nIndex;
        }
    }

    static void signalColumnClicked(GtkTreeViewColumn* pColumn, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_column_clicked(pColumn);
    }

    static void signalVAdjustmentChanged(GtkAdjustment*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_visible_range_changed();
    }

    // The outside concept of a column maps to a gtk CellRenderer, rather than
    // a TreeViewColumn. If the first TreeViewColumn has a leading Toggle Renderer
    // and/or a leading Image Renderer, those are considered special expander
    // columns and precede index 0 and can be accessed via outside index -1
    int to_external_model(int modelcol) const
    {
        if (m_nExpanderToggleCol != -1)
            --modelcol;
        if (m_nExpanderImageCol != -1)
            --modelcol;
        return modelcol;
    }

    int to_internal_model(int modelcol) const
    {
        if (m_nExpanderToggleCol != -1)
            ++modelcol;
        if (m_nExpanderImageCol != -1)
            ++modelcol;
        return modelcol;
    }

    void set_column_editable(int nCol, bool bEditable)
    {
        nCol = to_internal_model(nCol);

        for (GList* pEntry = g_list_first(m_pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
            {
                GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
                void* pData = g_object_get_data(G_OBJECT(pCellRenderer), "g-lo-CellIndex");
                if (reinterpret_cast<sal_IntPtr>(pData) == nCol)
                {
                    g_object_set(G_OBJECT(pCellRenderer), "editable", bEditable, "editable-set", true, nullptr);
                    break;
                }
            }
            g_list_free(pRenderers);
        }
    }

    static void signalRowDeleted(GtkTreeModel*, GtkTreePath*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_model_changed();
    }

    static void signalRowInserted(GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_model_changed();
    }

    static gint sortFunc(GtkTreeModel* pModel, GtkTreeIter* a, GtkTreeIter* b, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        return pThis->sort_func(pModel, a, b);
    }

    gint sort_func(GtkTreeModel* pModel, GtkTreeIter* a, GtkTreeIter* b)
    {
        if (m_aCustomSort)
            return m_aCustomSort(GtkInstanceTreeIter(*a), GtkInstanceTreeIter(*b));
        return default_sort_func(pModel, a, b, m_xSorter.get());
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    bool signal_key_press(GdkEventKey* pEvent)
    {
        if (pEvent->keyval != GDK_KEY_Left && pEvent->keyval != GDK_KEY_Right)
            return false;

        GtkInstanceTreeIter aIter(nullptr);
        if (!get_cursor(&aIter))
            return false;

        bool bHasChild = gtk_tree_model_iter_has_child(m_pTreeModel, &aIter.iter);

        if (pEvent->keyval == GDK_KEY_Right)
        {
            if (bHasChild && !get_row_expanded(aIter))
            {
                expand_row(aIter);
                return true;
            }
            return false;
        }

        if (bHasChild && get_row_expanded(aIter))
        {
            collapse_row(aIter);
            return true;
        }

        if (iter_parent(aIter))
        {
            unselect_all();
            set_cursor(aIter);
            select(aIter);
            return true;
        }

        return false;
    }

    static gboolean signalKeyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        return pThis->signal_key_press(pEvent);
    }
#endif

    static gboolean signalQueryTooltip(GtkWidget* /*pGtkWidget*/, gint x, gint y,
                                         gboolean keyboard_tip, GtkTooltip *tooltip,
                                         gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        GtkTreeIter iter;
        GtkTreeView *pTreeView = pThis->m_pTreeView;
        GtkTreeModel *pModel = gtk_tree_view_get_model(pTreeView);
        GtkTreePath *pPath = nullptr;
#if GTK_CHECK_VERSION(4, 0, 0)
        if (!gtk_tree_view_get_tooltip_context(pTreeView, x, y, keyboard_tip, &pModel, &pPath, &iter))
            return false;
#else
        if (!gtk_tree_view_get_tooltip_context(pTreeView, &x, &y, keyboard_tip, &pModel, &pPath, &iter))
            return false;
#endif
        OUString aTooltip = pThis->signal_query_tooltip(GtkInstanceTreeIter(iter));
        if (aTooltip.isEmpty())
            return false;
        gtk_tooltip_set_text(tooltip, OUStringToOString(aTooltip, RTL_TEXTENCODING_UTF8).getStr());
        gtk_tree_view_set_tooltip_row(pTreeView, tooltip, pPath);
        gtk_tree_path_free(pPath);
        return true;
    }

    void last_child(GtkTreeModel* pModel, GtkTreeIter* result, GtkTreeIter* pParent, int nChildren) const
    {
        gtk_tree_model_iter_nth_child(pModel, result, pParent, nChildren - 1);
        nChildren = gtk_tree_model_iter_n_children(pModel, result);
        if (nChildren)
        {
            GtkTreeIter newparent(*result);
            last_child(pModel, result, &newparent, nChildren);
        }
    }

    GtkTreePath* get_path_of_last_entry(GtkTreeModel *pModel)
    {
        GtkTreePath *lastpath;
        // find the last entry in the model for comparison
        int nChildren = gtk_tree_model_iter_n_children(pModel, nullptr);
        if (!nChildren)
            lastpath = gtk_tree_path_new_from_indices(0, -1);
        else
        {
            GtkTreeIter iter;
            last_child(pModel, &iter, nullptr, nChildren);
            lastpath = gtk_tree_model_get_path(pModel, &iter);
        }
        return lastpath;
    }

    void set_font_color(const GtkTreeIter& iter, const Color& rColor)
    {
        if (rColor == COL_AUTO)
            m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), m_nIdCol + 1, nullptr, -1);
        else
        {
            GdkRGBA aColor{rColor.GetRed()/255.0f, rColor.GetGreen()/255.0f, rColor.GetBlue()/255.0f, 0};
            m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), m_nIdCol + 1, &aColor, -1);
        }
    }

    int get_expander_size() const
    {
        // gtk4: _TREE_VIEW_EXPANDER_SIZE define in gtk/gtktreeview.c
        gint nExpanderSize = 16;
        // gtk4: _TREE_VIEW_HORIZONTAL_SEPARATOR define in gtk/gtktreeview.c
        gint nHorizontalSeparator = 4;

#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_style_get(GTK_WIDGET(m_pTreeView),
                             "expander-size", &nExpanderSize,
                             "horizontal-separator", &nHorizontalSeparator,
                             nullptr);
#endif

        return nExpanderSize + (nHorizontalSeparator/ 2);
    }

    void real_vadjustment_set_value(int value)
    {
        disable_notify_events();
        gtk_adjustment_set_value(m_pVAdjustment, value);
        enable_notify_events();
    }

    static gboolean setAdjustmentCallback(GtkWidget*, GdkFrameClock*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        if (pThis->m_nPendingVAdjustment != -1)
        {
            pThis->real_vadjustment_set_value(pThis->m_nPendingVAdjustment);
            pThis->m_nPendingVAdjustment = -1;
        }
        return false;
    }

    bool iter_next(weld::TreeIter& rIter, bool bOnlyExpanded) const
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeIter tmp;
        GtkTreeIter iter = rGtkIter.iter;

        bool ret = gtk_tree_model_iter_children(m_pTreeModel, &tmp, &iter);
        if (ret && bOnlyExpanded && !get_row_expanded(rGtkIter))
            ret = false;
        rGtkIter.iter = tmp;
        if (ret)
        {
            //on-demand dummy entry doesn't count
            if (get_text(rGtkIter, -1) == "<dummy>")
                return iter_next(rGtkIter, bOnlyExpanded);
            return true;
        }

        tmp = iter;
        if (gtk_tree_model_iter_next(m_pTreeModel, &tmp))
        {
            rGtkIter.iter = tmp;
            //on-demand dummy entry doesn't count
            if (get_text(rGtkIter, -1) == "<dummy>")
                return iter_next(rGtkIter, bOnlyExpanded);
            return true;
        }
        // Move up level(s) until we find the level where the next node exists.
        while (gtk_tree_model_iter_parent(m_pTreeModel, &tmp, &iter))
        {
            iter = tmp;
            if (gtk_tree_model_iter_next(m_pTreeModel, &tmp))
            {
                rGtkIter.iter = tmp;
                //on-demand dummy entry doesn't count
                if (get_text(rGtkIter, -1) == "<dummy>")
                    return iter_next(rGtkIter, bOnlyExpanded);
                return true;
            }
        }
        return false;
    }

public:
    GtkInstanceTreeView(GtkTreeView* pTreeView, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pTreeView), pBuilder, bTakeOwnership)
        , m_pTreeView(pTreeView)
        , m_pTreeModel(gtk_tree_view_get_model(m_pTreeView))
        , m_bWorkAroundBadDragRegion(false)
        , m_bInDrag(false)
        , m_nTextCol(-1)
        , m_nTextView(-1)
        , m_nImageCol(-1)
        , m_nExpanderToggleCol(-1)
        , m_nExpanderImageCol(-1)
        , m_nPendingVAdjustment(-1)
        , m_nChangedSignalId(g_signal_connect(gtk_tree_view_get_selection(pTreeView), "changed",
                             G_CALLBACK(signalChanged), this))
        , m_nRowActivatedSignalId(g_signal_connect(pTreeView, "row-activated", G_CALLBACK(signalRowActivated), this))
        , m_nTestExpandRowSignalId(g_signal_connect(pTreeView, "test-expand-row", G_CALLBACK(signalTestExpandRow), this))
        , m_nTestCollapseRowSignalId(g_signal_connect(pTreeView, "test-collapse-row", G_CALLBACK(signalTestCollapseRow), this))
        , m_nVAdjustmentChangedSignalId(0)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nPopupMenuSignalId(g_signal_connect(pTreeView, "popup-menu", G_CALLBACK(signalPopupMenu), this))
        , m_nKeyPressSignalId(g_signal_connect(pTreeView, "key-press-event", G_CALLBACK(signalKeyPress), this))
#endif
        , m_nQueryTooltipSignalId(0)
        , m_pVAdjustment(gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(pTreeView)))
        , m_pChangeEvent(nullptr)
    {
        if (GTK_IS_TREE_STORE(m_pTreeModel))
        {
            m_Setter = tree_store_set;
            m_InsertWithValues = tree_store_insert_with_values;
            m_Insert = tree_store_insert;
            m_Prepend = tree_store_prepend;
            m_Remove = tree_store_remove;
            m_Swap = tree_store_swap;
            m_SetValue = tree_store_set_value;
            m_Clear = tree_store_clear;
        }
        else
        {
            /*
               tdf#136559 see: https://gitlab.gnome.org/GNOME/gtk/-/issues/2693
               If we only need a list and not a tree we can get a performance boost from using a ListStore
             */
            assert(!gtk_tree_view_get_show_expanders(m_pTreeView) && "a liststore can only be used if no tree structure is needed");
            m_Setter = list_store_set;
            m_InsertWithValues = list_store_insert_with_values;
            m_Insert = list_store_insert;
            m_Prepend = list_store_prepend;
            m_Remove = list_store_remove;
            m_Swap = list_store_swap;
            m_SetValue = list_store_set_value;
            m_Clear = list_store_clear;
        }

        /* The outside concept of a column maps to a gtk CellRenderer, rather than
           a TreeViewColumn. If the first TreeViewColumn has a leading Toggle Renderer
           and/or a leading Image Renderer, those are considered special expander
           columns and precede index 0 and can be accessed via outside index -1
        */
        m_pColumns = gtk_tree_view_get_columns(m_pTreeView);
        int nIndex(0);
        int nViewColumn(0);
        for (GList* pEntry = g_list_first(m_pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            m_aColumnSignalIds.push_back(g_signal_connect(pColumn, "clicked", G_CALLBACK(signalColumnClicked), this));
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
            {
                GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
                if (GTK_IS_CELL_RENDERER_TEXT(pCellRenderer))
                {
                    if (m_nTextCol == -1)
                    {
                        m_nTextCol = nIndex;
                        m_nTextView = nViewColumn;
                    }
                    m_aWeightMap[nIndex] = -1;
                    m_aSensitiveMap[nIndex] = -1;
                    m_aIndentMap[nIndex] = -1;
                    m_aAlignMap[nIndex] = -1;
                    g_signal_connect(G_OBJECT(pCellRenderer), "editing-started", G_CALLBACK(signalCellEditingStarted), this);
                    g_signal_connect(G_OBJECT(pCellRenderer), "editing-canceled", G_CALLBACK(signalCellEditingCanceled), this);
                    g_signal_connect(G_OBJECT(pCellRenderer), "edited", G_CALLBACK(signalCellEdited), this);
                }
                else if (GTK_IS_CELL_RENDERER_TOGGLE(pCellRenderer))
                {
                    const bool bExpander = nIndex == 0 || (nIndex == 1 && m_nExpanderImageCol == 0);
                    if (bExpander)
                        m_nExpanderToggleCol = nIndex;
                    g_signal_connect(G_OBJECT(pCellRenderer), "toggled", G_CALLBACK(signalCellToggled), this);
                    m_aToggleVisMap[nIndex] = -1;
                    m_aToggleTriStateMap[nIndex] = -1;
                }
                else if (GTK_IS_CELL_RENDERER_PIXBUF(pCellRenderer))
                {
                    const bool bExpander = g_list_next(pRenderer) != nullptr;
                    if (bExpander && m_nExpanderImageCol == -1)
                        m_nExpanderImageCol = nIndex;
                    else if (m_nImageCol == -1)
                        m_nImageCol = nIndex;
                }
                g_object_set_data(G_OBJECT(pCellRenderer), "g-lo-CellIndex", reinterpret_cast<gpointer>(nIndex));
                ++nIndex;
            }
            g_list_free(pRenderers);
            ++nViewColumn;
        }

        m_nIdCol = nIndex++;

        for (auto& a : m_aToggleVisMap)
            a.second = nIndex++;
        for (auto& a : m_aToggleTriStateMap)
            a.second = nIndex++;
        for (auto& a : m_aWeightMap)
            a.second = nIndex++;
        for (auto& a : m_aSensitiveMap)
            a.second = nIndex++;
        for (auto& a : m_aIndentMap)
            a.second = nIndex++;
        for (auto& a : m_aAlignMap)
            a.second = nIndex++;

        ensure_drag_begin_end();

        m_nRowDeletedSignalId = g_signal_connect(m_pTreeModel, "row-deleted", G_CALLBACK(signalRowDeleted), this);
        m_nRowInsertedSignalId = g_signal_connect(m_pTreeModel, "row-inserted", G_CALLBACK(signalRowInserted), this);
    }

    virtual void connect_query_tooltip(const Link<const weld::TreeIter&, OUString>& rLink) override
    {
        weld::TreeView::connect_query_tooltip(rLink);
        m_nQueryTooltipSignalId = g_signal_connect(m_pTreeView, "query-tooltip", G_CALLBACK(signalQueryTooltip), this);
    }

    virtual void columns_autosize() override
    {
        gtk_tree_view_columns_autosize(m_pTreeView);
    }

    virtual void set_column_fixed_widths(const std::vector<int>& rWidths) override
    {
        GList* pEntry = g_list_first(m_pColumns);
        for (auto nWidth : rWidths)
        {
            assert(pEntry && "wrong count");
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            gtk_tree_view_column_set_fixed_width(pColumn, nWidth);
            pEntry = g_list_next(pEntry);
        }
    }

    virtual void set_column_editables(const std::vector<bool>& rEditables) override
    {
        size_t nTabCount = rEditables.size();
        for (size_t i = 0 ; i < nTabCount; ++i)
            set_column_editable(i, rEditables[i]);
    }

    virtual void set_centered_column(int nCol) override
    {
        for (GList* pEntry = g_list_first(m_pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
            {
                GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
                void* pData = g_object_get_data(G_OBJECT(pCellRenderer), "g-lo-CellIndex");
                if (reinterpret_cast<sal_IntPtr>(pData) == nCol)
                {
                    g_object_set(G_OBJECT(pCellRenderer), "xalign", 0.5, nullptr);
                    break;
                }
            }
            g_list_free(pRenderers);
        }
    }

    virtual int get_column_width(int nColumn) const override
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, nColumn));
        assert(pColumn && "wrong count");
        int nWidth = gtk_tree_view_column_get_width(pColumn);
        // https://github.com/exaile/exaile/issues/580
        // after setting fixed_width on a column and requesting width before
        // gtk has a chance to do its layout of the column means that the width
        // request hasn't come into effect
        if (!nWidth)
            nWidth = gtk_tree_view_column_get_fixed_width(pColumn);
        return nWidth;
    }

    virtual OUString get_column_title(int nColumn) const override
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, nColumn));
        assert(pColumn && "wrong count");
        const gchar* pTitle = gtk_tree_view_column_get_title(pColumn);
        OUString sRet(pTitle, pTitle ? strlen(pTitle) : 0, RTL_TEXTENCODING_UTF8);
        return sRet;
    }

    virtual void set_column_title(int nColumn, const OUString& rTitle) override
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, nColumn));
        assert(pColumn && "wrong count");
        gtk_tree_view_column_set_title(pColumn, OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void set_column_custom_renderer(int nColumn, bool bEnable) override
    {
        assert(n_children() == 0 && "tree must be empty");
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, nColumn));
        assert(pColumn && "wrong count");

        GtkCellRenderer* pExpander = nullptr;
        GtkCellRenderer* pToggle = nullptr;

        // migrate existing editable setting to the new renderer
        gboolean is_editable(false);
        void* pEditCellData(nullptr);
        GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
        for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
        {
            GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);

            void* pData = g_object_get_data(G_OBJECT(pCellRenderer), "g-lo-CellIndex");
            auto nCellIndex = reinterpret_cast<sal_IntPtr>(pData);

            if (GTK_IS_CELL_RENDERER_TEXT(pCellRenderer))
            {
                g_object_get(pCellRenderer, "editable", &is_editable, nullptr);
                pEditCellData = pData;
                break;
            }
            else if (GTK_IS_CELL_RENDERER_TOGGLE(pCellRenderer))
            {
                if (nCellIndex == m_nExpanderToggleCol)
                {
                    pToggle = pCellRenderer;
                    g_object_ref(pToggle);
                }
            }
            else if (GTK_IS_CELL_RENDERER_PIXBUF(pCellRenderer))
            {
                if (nCellIndex == m_nExpanderImageCol)
                {
                    pExpander = pCellRenderer;
                    g_object_ref(pExpander);
                }
            }

        }
        g_list_free(pRenderers);

        GtkCellRenderer* pRenderer;

        gtk_cell_layout_clear(GTK_CELL_LAYOUT(pColumn));
        if (pExpander)
        {
            gtk_tree_view_column_pack_start(pColumn, pExpander, false);
            gtk_tree_view_column_add_attribute(pColumn, pExpander, "pixbuf", m_nExpanderImageCol);
            g_object_unref(pExpander);
        }
        if (pToggle)
        {
            gtk_tree_view_column_pack_start(pColumn, pToggle, false);
            gtk_tree_view_column_add_attribute(pColumn, pToggle, "active", m_nExpanderToggleCol);
            gtk_tree_view_column_add_attribute(pColumn, pToggle, "active", m_nExpanderToggleCol);
            gtk_tree_view_column_add_attribute(pColumn, pToggle, "visible", m_aToggleTriStateMap[m_nExpanderToggleCol]);
            g_object_unref(pToggle);
        }

        if (bEnable)
        {
            pRenderer = custom_cell_renderer_new();
            GValue value = G_VALUE_INIT;
            g_value_init(&value, G_TYPE_POINTER);
            g_value_set_pointer(&value, static_cast<gpointer>(this));
            g_object_set_property(G_OBJECT(pRenderer), "instance", &value);
            gtk_tree_view_column_pack_start(pColumn, pRenderer, true);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "text", m_nTextCol);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "id", m_nIdCol);
        }
        else
        {
            pRenderer = gtk_cell_renderer_text_new();
            gtk_tree_view_column_pack_start(pColumn, pRenderer, true);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "text", m_nTextCol);
        }

        if (is_editable)
        {
            g_object_set(pRenderer, "editable", true, "editable-set", true, nullptr);
            g_object_set_data(G_OBJECT(pRenderer), "g-lo-CellIndex", pEditCellData);
            g_signal_connect(pRenderer, "editing-started", G_CALLBACK(signalCellEditingStarted), this);
            g_signal_connect(pRenderer, "editing-canceled", G_CALLBACK(signalCellEditingCanceled), this);
            g_signal_connect(pRenderer, "edited", G_CALLBACK(signalCellEdited), this);
        }
    }

    virtual void queue_draw() override
    {
        gtk_widget_queue_draw(GTK_WIDGET(m_pTreeView));
    }

    virtual void insert(const weld::TreeIter* pParent, int pos, const OUString* pText, const OUString* pId, const OUString* pIconName,
                        VirtualDevice* pImageSurface,
                        bool bChildrenOnDemand, weld::TreeIter* pRet) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        const GtkInstanceTreeIter* pGtkIter = static_cast<const GtkInstanceTreeIter*>(pParent);
        insert_row(iter, pGtkIter ? &pGtkIter->iter : nullptr, pos, pId, pText, pIconName, pImageSurface);
        if (bChildrenOnDemand)
        {
            GtkTreeIter subiter;
            OUString sDummy("<dummy>");
            insert_row(subiter, &iter, -1, nullptr, &sDummy, nullptr, nullptr);
        }
        if (pRet)
        {
            GtkInstanceTreeIter* pGtkRetIter = static_cast<GtkInstanceTreeIter*>(pRet);
            pGtkRetIter->iter = iter;
        }
        enable_notify_events();
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        if (!gtk_tree_view_get_row_separator_func(m_pTreeView))
            gtk_tree_view_set_row_separator_func(m_pTreeView, separatorFunction, this, nullptr);
        insert_row(iter, nullptr, pos, &rId, nullptr, nullptr, nullptr);
        GtkTreePath* pPath = gtk_tree_model_get_path(m_pTreeModel, &iter);
        m_aSeparatorRows.emplace_back(gtk_tree_row_reference_new(m_pTreeModel, pPath));
        gtk_tree_path_free(pPath);
        enable_notify_events();
    }

    virtual void set_font_color(int pos, const Color& rColor) override
    {
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos);
        set_font_color(iter, rColor);
    }

    virtual void set_font_color(const weld::TreeIter& rIter, const Color& rColor) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set_font_color(rGtkIter.iter, rColor);
    }

    virtual void remove(int pos) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos);
        m_Remove(m_pTreeModel, &iter);
        enable_notify_events();
    }

    virtual int find_text(const OUString& rText) const override
    {
        Search aSearch(rText, m_nTextCol);
        gtk_tree_model_foreach(m_pTreeModel, foreach_find, &aSearch);
        return aSearch.index;
    }

    virtual int find_id(const OUString& rId) const override
    {
        Search aSearch(rId, m_nIdCol);
        gtk_tree_model_foreach(m_pTreeModel, foreach_find, &aSearch);
        return aSearch.index;
    }

    virtual void bulk_insert_for_each(int nSourceCount, const std::function<void(weld::TreeIter&, int nSourceIndex)>& func,
                                      const weld::TreeIter* pParent,
                                      const std::vector<int>* pFixedWidths) override
    {
        GtkInstanceTreeIter* pGtkIter = const_cast<GtkInstanceTreeIter*>(static_cast<const GtkInstanceTreeIter*>(pParent));

        freeze();
        if (!pGtkIter)
            clear();
        else
        {
            GtkTreeIter restore(pGtkIter->iter);

            if (iter_children(*pGtkIter))
                while (m_Remove(m_pTreeModel, &pGtkIter->iter));

            pGtkIter->iter = restore;
        }
        GtkInstanceTreeIter aGtkIter(nullptr);

        if (pFixedWidths)
            set_column_fixed_widths(*pFixedWidths);

        while (nSourceCount)
        {
            // tdf#125241 inserting backwards is massively faster
            m_Prepend(m_pTreeModel, &aGtkIter.iter, pGtkIter ? &pGtkIter->iter : nullptr);
            func(aGtkIter, --nSourceCount);
        }

        thaw();
    }

    virtual void swap(int pos1, int pos2) override
    {
        disable_notify_events();

        GtkTreeIter iter1;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter1, nullptr, pos1);

        GtkTreeIter iter2;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter2, nullptr, pos2);

        m_Swap(m_pTreeModel, &iter1, &iter2);

        enable_notify_events();
    }

    virtual void clear() override
    {
        disable_notify_events();
        gtk_tree_view_set_row_separator_func(m_pTreeView, nullptr, nullptr, nullptr);
        m_aSeparatorRows.clear();
        m_Clear(m_pTreeModel);
        enable_notify_events();
    }

    virtual void make_sorted() override
    {
        // thaw wants to restore sort state of freeze
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetUILanguageTag().getLocale()));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_set_sort_func(pSortable, m_nTextCol, sortFunc, this, nullptr);
        gtk_tree_sortable_set_sort_column_id(pSortable, m_nTextCol, GTK_SORT_ASCENDING);
    }

    virtual void make_unsorted() override
    {
        m_xSorter.reset();
        int nSortColumn;
        GtkSortType eSortType;
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_get_sort_column_id(pSortable, &nSortColumn, &eSortType);
        gtk_tree_sortable_set_sort_column_id(pSortable, GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, eSortType);
    }

    virtual void set_sort_order(bool bAscending) override
    {
        GtkSortType eSortType = bAscending ? GTK_SORT_ASCENDING : GTK_SORT_DESCENDING;

        gint sort_column_id(0);
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_get_sort_column_id(pSortable, &sort_column_id, nullptr);
        gtk_tree_sortable_set_sort_column_id(pSortable, sort_column_id, eSortType);
    }

    virtual bool get_sort_order() const override
    {
        int nSortColumn;
        GtkSortType eSortType;

        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_get_sort_column_id(pSortable, &nSortColumn, &eSortType);
        return nSortColumn != GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID && eSortType == GTK_SORT_ASCENDING;
    }

    virtual void set_sort_indicator(TriState eState, int col) override
    {
        assert(col >= 0 && "cannot sort on expander column");

        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, col));
        assert(pColumn && "wrong count");
        if (eState == TRISTATE_INDET)
            gtk_tree_view_column_set_sort_indicator(pColumn, false);
        else
        {
            gtk_tree_view_column_set_sort_indicator(pColumn, true);
            GtkSortType eSortType = eState == TRISTATE_TRUE ? GTK_SORT_ASCENDING : GTK_SORT_DESCENDING;
            gtk_tree_view_column_set_sort_order(pColumn, eSortType);
        }
    }

    virtual TriState get_sort_indicator(int col) const override
    {
        assert(col >= 0 && "cannot sort on expander column");

        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, col));
        if (!gtk_tree_view_column_get_sort_indicator(pColumn))
            return TRISTATE_INDET;
        return gtk_tree_view_column_get_sort_order(pColumn) == GTK_SORT_ASCENDING ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    virtual int get_sort_column() const override
    {
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gint sort_column_id(0);
        if (!gtk_tree_sortable_get_sort_column_id(pSortable, &sort_column_id, nullptr))
            return -1;
        return to_external_model(sort_column_id);
    }

    virtual void set_sort_column(int nColumn) override
    {
        if (nColumn == -1)
        {
            make_unsorted();
            return;
        }
        GtkSortType eSortType;
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_get_sort_column_id(pSortable, nullptr, &eSortType);
        int nSortCol = to_internal_model(nColumn);
        gtk_tree_sortable_set_sort_func(pSortable, nSortCol, sortFunc, this, nullptr);
        gtk_tree_sortable_set_sort_column_id(pSortable, nSortCol, eSortType);
    }

    virtual void set_sort_func(const std::function<int(const weld::TreeIter&, const weld::TreeIter&)>& func) override
    {
        weld::TreeView::set_sort_func(func);
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_sort_column_changed(pSortable);
    }

    virtual int n_children() const override
    {
        return gtk_tree_model_iter_n_children(m_pTreeModel, nullptr);
    }

    virtual int iter_n_children(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        return gtk_tree_model_iter_n_children(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
    }

    virtual void select(int pos) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        if (pos == -1 || (pos == 0 && n_children() == 0))
        {
            gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(m_pTreeView));
        }
        else
        {
            GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
            gtk_tree_selection_select_path(gtk_tree_view_get_selection(m_pTreeView), path);
            gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
            gtk_tree_path_free(path);
        }
        enable_notify_events();
    }

    virtual void set_cursor(int pos) override
    {
        disable_notify_events();
        GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
        gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
        gtk_tree_view_set_cursor(m_pTreeView, path, nullptr, false);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual void scroll_to_row(int pos) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
        gtk_tree_view_expand_to_path(m_pTreeView, path);
        gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual bool is_selected(int pos) const override
    {
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos);
        return gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(m_pTreeView), &iter);
    }

    virtual void unselect(int pos) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        if (pos == -1 || (pos == 0 && n_children() == 0))
        {
            gtk_tree_selection_select_all(gtk_tree_view_get_selection(m_pTreeView));
        }
        else
        {
            GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
            gtk_tree_selection_unselect_path(gtk_tree_view_get_selection(m_pTreeView), path);
            gtk_tree_path_free(path);
        }
        enable_notify_events();
    }

    virtual std::vector<int> get_selected_rows() const override
    {
        std::vector<int> aRows;

        GList* pList = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(m_pTreeView), nullptr);
        for (GList* pItem = g_list_first(pList); pItem; pItem = g_list_next(pItem))
        {
            GtkTreePath* path = static_cast<GtkTreePath*>(pItem->data);

            gint depth;
            gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
            int nRow = indices[depth-1];

            aRows.push_back(nRow);
        }
        g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));

        return aRows;
    }

    virtual void all_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        g_object_freeze_notify(G_OBJECT(m_pTreeModel));

        GtkInstanceTreeIter aGtkIter(nullptr);
        if (get_iter_first(aGtkIter))
        {
            do
            {
                if (func(aGtkIter))
                    break;
            } while (iter_next(aGtkIter));
        }

        g_object_thaw_notify(G_OBJECT(m_pTreeModel));
    }

    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        g_object_freeze_notify(G_OBJECT(m_pTreeModel));

        GtkInstanceTreeIter aGtkIter(nullptr);

        GtkTreeModel* pModel;
        GList* pList = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(m_pTreeView), &pModel);
        for (GList* pItem = g_list_first(pList); pItem; pItem = g_list_next(pItem))
        {
            GtkTreePath* path = static_cast<GtkTreePath*>(pItem->data);
            gtk_tree_model_get_iter(pModel, &aGtkIter.iter, path);
            if (func(aGtkIter))
                break;
        }
        g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));

        g_object_thaw_notify(G_OBJECT(m_pTreeModel));
    }

    virtual void visible_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        g_object_freeze_notify(G_OBJECT(m_pTreeModel));

        GtkTreePath* start_path;
        GtkTreePath* end_path;

        if (!gtk_tree_view_get_visible_range(m_pTreeView, &start_path, &end_path))
            return;

        GtkInstanceTreeIter aGtkIter(nullptr);
        gtk_tree_model_get_iter(m_pTreeModel, &aGtkIter.iter, start_path);

        do
        {
            if (func(aGtkIter))
                break;
            GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, &aGtkIter.iter);
            bool bContinue = gtk_tree_path_compare(path, end_path) != 0;
            gtk_tree_path_free(path);
            if (!bContinue)
                break;
            if (!iter_next(aGtkIter))
                break;
        } while(true);

        gtk_tree_path_free(start_path);
        gtk_tree_path_free(end_path);

        g_object_thaw_notify(G_OBJECT(m_pTreeModel));
    }

    virtual void connect_visible_range_changed(const Link<weld::TreeView&, void>& rLink) override
    {
        weld::TreeView::connect_visible_range_changed(rLink);
        if (!m_nVAdjustmentChangedSignalId)
        {
            GtkAdjustment* pVAdjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(m_pTreeView));
            m_nVAdjustmentChangedSignalId = g_signal_connect(pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustmentChanged), this);
        }
    }

    virtual bool is_selected(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        return gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(m_pTreeView), const_cast<GtkTreeIter*>(&rGtkIter.iter));
    }

    virtual OUString get_text(int pos, int col) const override
    {
        if (col == -1)
            col = m_nTextCol;
        else
            col = to_internal_model(col);
        return get(pos, col);
    }

    virtual void set_text(int pos, const OUString& rText, int col) override
    {
        if (col == -1)
            col = m_nTextCol;
        else
            col = to_internal_model(col);
        set(pos, col, rText);
    }

    virtual TriState get_toggle(int pos, int col) const override
    {
        if (col == -1)
            col = m_nExpanderToggleCol;
        else
            col = to_internal_model(col);

        if (get_bool(pos, m_aToggleTriStateMap.find(col)->second))
            return TRISTATE_INDET;
        return get_bool(pos, col) ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    virtual TriState get_toggle(const weld::TreeIter& rIter, int col) const override
    {
        if (col == -1)
            col = m_nExpanderToggleCol;
        else
            col = to_internal_model(col);

        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        if (get_bool(rGtkIter.iter, m_aToggleTriStateMap.find(col)->second))
            return TRISTATE_INDET;
        return get_bool(rGtkIter.iter, col) ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    virtual void set_toggle(const weld::TreeIter& rIter, TriState eState, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set_toggle(rGtkIter.iter, eState, col);
    }

    virtual void set_toggle(int pos, TriState eState, int col) override
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
            set_toggle(iter, eState, col);
    }

    virtual void enable_toggle_buttons(weld::ColumnToggleType eType) override
    {
        for (GList* pEntry = g_list_first(m_pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
            {
                GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
                if (!GTK_IS_CELL_RENDERER_TOGGLE(pCellRenderer))
                    continue;
                GtkCellRendererToggle* pToggle = GTK_CELL_RENDERER_TOGGLE(pCellRenderer);
                gtk_cell_renderer_toggle_set_radio(pToggle, eType == weld::ColumnToggleType::Radio);
            }
            g_list_free(pRenderers);
        }
    }

    virtual void set_clicks_to_toggle(int /*nToggleBehavior*/) override
    {
    }

    virtual void set_extra_row_indent(const weld::TreeIter& rIter, int nIndentLevel) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set(rGtkIter.iter, m_aIndentMap[m_nTextCol], nIndentLevel * get_expander_size());
    }

    virtual void set_text_emphasis(const weld::TreeIter& rIter, bool bOn, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        col = to_internal_model(col);
        set(rGtkIter.iter, m_aWeightMap[col], bOn ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
    }

    virtual void set_text_emphasis(int pos, bool bOn, int col) override
    {
        col = to_internal_model(col);
        set(pos, m_aWeightMap[col], bOn ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
    }

    virtual bool get_text_emphasis(const weld::TreeIter& rIter, int col) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        col = to_internal_model(col);
        return get_int(rGtkIter.iter, m_aWeightMap.find(col)->second) == PANGO_WEIGHT_BOLD;
    }

    virtual bool get_text_emphasis(int pos, int col) const override
    {
        col = to_internal_model(col);
        return get_int(pos, m_aWeightMap.find(col)->second) == PANGO_WEIGHT_BOLD;
    }

    virtual void set_text_align(const weld::TreeIter& rIter, double fAlign, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        col = to_internal_model(col);
        set(rGtkIter.iter, m_aAlignMap[col], fAlign);
    }

    virtual void set_text_align(int pos, double fAlign, int col) override
    {
        col = to_internal_model(col);
        set(pos, m_aAlignMap[col], fAlign);
    }

    using GtkInstanceWidget::set_sensitive;

    virtual void set_sensitive(int pos, bool bSensitive, int col) override
    {
        if (col == -1)
            col = m_nTextCol;
        else
            col = to_internal_model(col);
        set(pos, m_aSensitiveMap[col], bSensitive);
    }

    virtual void set_sensitive(const weld::TreeIter& rIter, bool bSensitive, int col) override
    {
        if (col == -1)
            col = m_nTextCol;
        else
            col = to_internal_model(col);
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set(rGtkIter.iter, m_aSensitiveMap[col], bSensitive);
    }

    void set_image(const GtkTreeIter& iter, int col, GdkPixbuf* pixbuf)
    {
        if (col == -1)
            col = m_nExpanderImageCol;
        else
            col = to_internal_model(col);
        m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, pixbuf, -1);
        if (pixbuf)
            g_object_unref(pixbuf);
    }

    void set_image(int pos, GdkPixbuf* pixbuf, int col)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
        {
            set_image(iter, col, pixbuf);
        }
    }

    virtual void set_image(int pos, const css::uno::Reference<css::graphic::XGraphic>& rImage, int col) override
    {
        set_image(pos, getPixbuf(rImage), col);
    }

    virtual void set_image(int pos, const OUString& rImage, int col) override
    {
        set_image(pos, getPixbuf(rImage), col);
    }

    virtual void set_image(int pos, VirtualDevice& rImage, int col) override
    {
        set_image(pos, getPixbuf(rImage), col);
    }

    virtual void set_image(const weld::TreeIter& rIter, const css::uno::Reference<css::graphic::XGraphic>& rImage, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set_image(rGtkIter.iter, col, getPixbuf(rImage));
    }

    virtual void set_image(const weld::TreeIter& rIter, const OUString& rImage, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set_image(rGtkIter.iter, col, getPixbuf(rImage));
    }

    virtual void set_image(const weld::TreeIter& rIter, VirtualDevice& rImage, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set_image(rGtkIter.iter, col, getPixbuf(rImage));
    }

    virtual OUString get_id(int pos) const override
    {
        return get(pos, m_nIdCol);
    }

    virtual void set_id(int pos, const OUString& rId) override
    {
        return set(pos, m_nIdCol, rId);
    }

    virtual int get_iter_index_in_parent(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);

        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));

        gint depth;
        gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
        int nRet = indices[depth-1];

        gtk_tree_path_free(path);

        return nRet;
    }

    virtual int iter_compare(const weld::TreeIter& a, const weld::TreeIter& b) const override
    {
        const GtkInstanceTreeIter& rGtkIterA = static_cast<const GtkInstanceTreeIter&>(a);
        const GtkInstanceTreeIter& rGtkIterB = static_cast<const GtkInstanceTreeIter&>(b);

        GtkTreePath* pathA = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIterA.iter));
        GtkTreePath* pathB = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIterB.iter));

        int nRet = gtk_tree_path_compare(pathA, pathB);

        gtk_tree_path_free(pathB);
        gtk_tree_path_free(pathA);

        return nRet;
    }

    // by copy and delete of old copy
    void move_subtree(GtkTreeIter& rFromIter, GtkTreeIter* pGtkParentIter, int nIndexInNewParent)
    {
        int nCols = gtk_tree_model_get_n_columns(m_pTreeModel);
        GValue value;

        GtkTreeIter toiter;
        m_Insert(m_pTreeModel, &toiter, pGtkParentIter, nIndexInNewParent);

        for (int i = 0; i < nCols; ++i)
        {
            memset(&value,  0, sizeof(GValue));
            gtk_tree_model_get_value(m_pTreeModel, &rFromIter, i, &value);
            m_SetValue(m_pTreeModel, &toiter, i, &value);
            g_value_unset(&value);
        }

        GtkTreeIter tmpfromiter;
        if (gtk_tree_model_iter_children(m_pTreeModel, &tmpfromiter, &rFromIter))
        {
            int j = 0;
            do
            {
                move_subtree(tmpfromiter, &toiter, j++);
            } while (gtk_tree_model_iter_next(m_pTreeModel, &tmpfromiter));
        }

        m_Remove(m_pTreeModel, &rFromIter);
    }

    virtual void move_subtree(weld::TreeIter& rNode, const weld::TreeIter* pNewParent, int nIndexInNewParent) override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rNode);
        const GtkInstanceTreeIter* pGtkParentIter = static_cast<const GtkInstanceTreeIter*>(pNewParent);
        move_subtree(rGtkIter.iter, pGtkParentIter ? const_cast<GtkTreeIter*>(&pGtkParentIter->iter) : nullptr, nIndexInNewParent);
    }

    virtual int get_selected_index() const override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't request selection when frozen");
        int nRet = -1;
        GtkTreeSelection *selection = gtk_tree_view_get_selection(m_pTreeView);
        if (gtk_tree_selection_get_mode(selection) != GTK_SELECTION_MULTIPLE)
        {
            GtkTreeIter iter;
            GtkTreeModel* pModel;
            if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(m_pTreeView), &pModel, &iter))
            {
                GtkTreePath* path = gtk_tree_model_get_path(pModel, &iter);

                gint depth;
                gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
                nRet = indices[depth-1];

                gtk_tree_path_free(path);
            }
        }
        else
        {
            auto vec = get_selected_rows();
            return vec.empty() ? -1 : vec[0];
        }
        return nRet;
    }

    bool get_selected_iterator(GtkTreeIter* pIter) const
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't request selection when frozen");
        bool bRet = false;
        GtkTreeSelection *selection = gtk_tree_view_get_selection(m_pTreeView);
        if (gtk_tree_selection_get_mode(selection) != GTK_SELECTION_MULTIPLE)
            bRet = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(m_pTreeView), nullptr, pIter);
        else
        {
            GtkTreeModel* pModel;
            GList* pList = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(m_pTreeView), &pModel);
            for (GList* pItem = g_list_first(pList); pItem; pItem = g_list_next(pItem))
            {
                if (pIter)
                {
                    GtkTreePath* path = static_cast<GtkTreePath*>(pItem->data);
                    gtk_tree_model_get_iter(pModel, pIter, path);
                }
                bRet = true;
                break;
            }
            g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));
        }
        return bRet;
    }

    virtual OUString get_selected_text() const override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't request selection when frozen");
        GtkTreeIter iter;
        if (get_selected_iterator(&iter))
            return get(iter, m_nTextCol);
        return OUString();
    }

    virtual OUString get_selected_id() const override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't request selection when frozen");
        GtkTreeIter iter;
        if (get_selected_iterator(&iter))
            return get(iter, m_nIdCol);
        return OUString();
    }

    virtual std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig) const override
    {
        return std::unique_ptr<weld::TreeIter>(new GtkInstanceTreeIter(static_cast<const GtkInstanceTreeIter*>(pOrig)));
    }

    virtual void copy_iterator(const weld::TreeIter& rSource, weld::TreeIter& rDest) const override
    {
        const GtkInstanceTreeIter& rGtkSource(static_cast<const GtkInstanceTreeIter&>(rSource));
        GtkInstanceTreeIter& rGtkDest(static_cast<GtkInstanceTreeIter&>(rDest));
        rGtkDest.iter = rGtkSource.iter;
    }

    virtual bool get_selected(weld::TreeIter* pIter) const override
    {
        GtkInstanceTreeIter* pGtkIter = static_cast<GtkInstanceTreeIter*>(pIter);
        return get_selected_iterator(pGtkIter ? &pGtkIter->iter : nullptr);
    }

    virtual bool get_cursor(weld::TreeIter* pIter) const override
    {
        GtkInstanceTreeIter* pGtkIter = static_cast<GtkInstanceTreeIter*>(pIter);
        GtkTreePath* path;
        gtk_tree_view_get_cursor(m_pTreeView, &path, nullptr);
        if (pGtkIter && path)
        {
            gtk_tree_model_get_iter(m_pTreeModel, &pGtkIter->iter, path);
        }
        if (!path)
            return false;
        gtk_tree_path_free(path);
        return true;
    }

    virtual int get_cursor_index() const override
    {
        int nRet = -1;

        GtkTreePath* path;
        gtk_tree_view_get_cursor(m_pTreeView, &path, nullptr);
        if (path)
        {
            gint depth;
            gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
            nRet = indices[depth-1];
            gtk_tree_path_free(path);
        }

        return nRet;
    }

    virtual void set_cursor(const weld::TreeIter& rIter) override
    {
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeIter Iter;
        if (gtk_tree_model_iter_parent(m_pTreeModel, &Iter, const_cast<GtkTreeIter*>(&rGtkIter.iter)))
        {
            GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, &Iter);
            if (!gtk_tree_view_row_expanded(m_pTreeView, path))
                gtk_tree_view_expand_to_path(m_pTreeView, path);
            gtk_tree_path_free(path);
        }
        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
        gtk_tree_view_set_cursor(m_pTreeView, path, nullptr, false);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual bool get_iter_first(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        return gtk_tree_model_get_iter_first(m_pTreeModel, &rGtkIter.iter);
    }

    virtual bool iter_next_sibling(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        return gtk_tree_model_iter_next(m_pTreeModel, &rGtkIter.iter);
    }

    virtual bool iter_previous_sibling(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        return gtk_tree_model_iter_previous(m_pTreeModel, &rGtkIter.iter);
    }

    virtual bool iter_next(weld::TreeIter& rIter) const override
    {
        return iter_next(rIter, false);
    }

    virtual bool iter_previous(weld::TreeIter& rIter) const override
    {
        bool ret = false;
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeIter iter = rGtkIter.iter;
        GtkTreeIter tmp = iter;
        if (gtk_tree_model_iter_previous(m_pTreeModel, &tmp))
        {
            // Move down level(s) until we find the level where the last node exists.
            int nChildren = gtk_tree_model_iter_n_children(m_pTreeModel, &tmp);
            if (!nChildren)
                rGtkIter.iter = tmp;
            else
                last_child(m_pTreeModel, &rGtkIter.iter, &tmp, nChildren);
            ret = true;
        }
        else
        {
            // Move up level
            if (gtk_tree_model_iter_parent(m_pTreeModel, &tmp, &iter))
            {
                rGtkIter.iter = tmp;
                ret = true;
            }
        }

        if (ret)
        {
            //on-demand dummy entry doesn't count
            if (get_text(rGtkIter, -1) == "<dummy>")
                return iter_previous(rGtkIter);
            return true;
        }

        return false;
    }

    virtual bool iter_children(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeIter tmp;
        bool ret = gtk_tree_model_iter_children(m_pTreeModel, &tmp, &rGtkIter.iter);
        rGtkIter.iter = tmp;
        if (ret)
        {
            //on-demand dummy entry doesn't count
            return get_text(rGtkIter, -1) != "<dummy>";
        }
        return ret;
    }

    virtual bool iter_parent(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeIter tmp;
        bool ret = gtk_tree_model_iter_parent(m_pTreeModel, &tmp, &rGtkIter.iter);
        rGtkIter.iter = tmp;
        return ret;
    }

    virtual void remove(const weld::TreeIter& rIter) override
    {
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        m_Remove(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        enable_notify_events();
    }

    virtual void remove_selection() override
    {
        disable_notify_events();

        std::vector<GtkTreeIter> aIters;
        GtkTreeModel* pModel;
        GList* pList = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(m_pTreeView), &pModel);
        for (GList* pItem = g_list_first(pList); pItem; pItem = g_list_next(pItem))
        {
            GtkTreePath* path = static_cast<GtkTreePath*>(pItem->data);
            aIters.emplace_back();
            gtk_tree_model_get_iter(pModel, &aIters.back(), path);
        }
        g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));

        for (auto& iter : aIters)
            m_Remove(m_pTreeModel, &iter);

        enable_notify_events();
    }

    virtual void select(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        gtk_tree_selection_select_iter(gtk_tree_view_get_selection(m_pTreeView), const_cast<GtkTreeIter*>(&rGtkIter.iter));
        enable_notify_events();
    }

    virtual void scroll_to_row(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        gtk_tree_view_expand_to_path(m_pTreeView, path);
        gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual void unselect(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        gtk_tree_selection_unselect_iter(gtk_tree_view_get_selection(m_pTreeView), const_cast<GtkTreeIter*>(&rGtkIter.iter));
        enable_notify_events();
    }

    virtual int get_iter_depth(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        int ret = gtk_tree_path_get_depth(path) - 1;
        gtk_tree_path_free(path);
        return ret;
    }

    virtual bool iter_has_child(const weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter aTempCopy(static_cast<const GtkInstanceTreeIter*>(&rIter));
        return iter_children(aTempCopy);
    }

    virtual bool get_row_expanded(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        bool ret = gtk_tree_view_row_expanded(m_pTreeView, path);
        gtk_tree_path_free(path);
        return ret;
    }

    virtual bool get_children_on_demand(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkInstanceTreeIter aIter(&rGtkIter);
        return child_is_placeholder(aIter);
    }

    virtual void set_children_on_demand(const weld::TreeIter& rIter, bool bChildrenOnDemand) override
    {
        disable_notify_events();

        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkInstanceTreeIter aPlaceHolderIter(&rGtkIter);

        bool bPlaceHolder = child_is_placeholder(aPlaceHolderIter);

        if (bChildrenOnDemand && !bPlaceHolder)
        {
            GtkTreeIter subiter;
            OUString sDummy("<dummy>");
            insert_row(subiter, &rGtkIter.iter, -1, nullptr, &sDummy, nullptr, nullptr);
        }
        else if (!bChildrenOnDemand && bPlaceHolder)
            remove(aPlaceHolderIter);

        enable_notify_events();
    }

    virtual void expand_row(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't expand when frozen");

        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        if (!gtk_tree_view_row_expanded(m_pTreeView, path))
            gtk_tree_view_expand_to_path(m_pTreeView, path);
        gtk_tree_path_free(path);
    }

    virtual void collapse_row(const weld::TreeIter& rIter) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        if (gtk_tree_view_row_expanded(m_pTreeView, path))
            gtk_tree_view_collapse_row(m_pTreeView, path);
        gtk_tree_path_free(path);
    }

    virtual OUString get_text(const weld::TreeIter& rIter, int col) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        if (col == -1)
            col = m_nTextCol;
        else
            col = to_internal_model(col);
        return get(rGtkIter.iter, col);
    }

    virtual void set_text(const weld::TreeIter& rIter, const OUString& rText, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        if (col == -1)
            col = m_nTextCol;
        else
            col = to_internal_model(col);
        set(rGtkIter.iter, col, rText);
    }

    virtual OUString get_id(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        return get(rGtkIter.iter, m_nIdCol);
    }

    virtual void set_id(const weld::TreeIter& rIter, const OUString& rId) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        set(rGtkIter.iter, m_nIdCol, rId);
    }

    virtual void freeze() override
    {
        disable_notify_events();
        bool bIsFirstFreeze = IsFirstFreeze();
        GtkInstanceWidget::freeze();
        if (bIsFirstFreeze)
        {
            g_object_ref(m_pTreeModel);
            gtk_tree_view_set_model(m_pTreeView, nullptr);
            g_object_freeze_notify(G_OBJECT(m_pTreeModel));
            if (m_xSorter)
            {
                int nSortColumn;
                GtkSortType eSortType;
                GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
                gtk_tree_sortable_get_sort_column_id(pSortable, &nSortColumn, &eSortType);
                gtk_tree_sortable_set_sort_column_id(pSortable, GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, eSortType);

                m_aSavedSortColumns.push_back(nSortColumn);
                m_aSavedSortTypes.push_back(eSortType);
            }
        }
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
        if (IsLastThaw())
        {
            if (m_xSorter)
            {
                GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
                gtk_tree_sortable_set_sort_column_id(pSortable, m_aSavedSortColumns.back(), m_aSavedSortTypes.back());
                m_aSavedSortTypes.pop_back();
                m_aSavedSortColumns.pop_back();
            }
            g_object_thaw_notify(G_OBJECT(m_pTreeModel));
            gtk_tree_view_set_model(m_pTreeView, GTK_TREE_MODEL(m_pTreeModel));
            g_object_unref(m_pTreeModel);
        }
        GtkInstanceWidget::thaw();
        enable_notify_events();
    }

    virtual int get_height_rows(int nRows) const override
    {
        return ::get_height_rows(m_pTreeView, m_pColumns, nRows);
    }

    virtual Size get_size_request() const override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
        {
            return Size(gtk_scrolled_window_get_min_content_width(GTK_SCROLLED_WINDOW(pParent)),
                        gtk_scrolled_window_get_min_content_height(GTK_SCROLLED_WINDOW(pParent)));
        }
        int nWidth, nHeight;
        gtk_widget_get_size_request(m_pWidget, &nWidth, &nHeight);
        return Size(nWidth, nHeight);
    }

    virtual Size get_preferred_size() const override
    {
        Size aRet(-1, -1);
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
        {
            aRet = Size(gtk_scrolled_window_get_min_content_width(GTK_SCROLLED_WINDOW(pParent)),
                        gtk_scrolled_window_get_min_content_height(GTK_SCROLLED_WINDOW(pParent)));
        }
        GtkRequisition size;
        gtk_widget_get_preferred_size(m_pWidget, nullptr, &size);
        if (aRet.Width() == -1)
            aRet.setWidth(size.width);
        if (aRet.Height() == -1)
            aRet.setHeight(size.height);
        return aRet;
    }

    virtual void show() override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
            gtk_widget_show(pParent);
        gtk_widget_show(m_pWidget);
    }

    virtual void hide() override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
            gtk_widget_hide(pParent);
        gtk_widget_hide(m_pWidget);
    }

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants) override
    {
        do_enable_drag_source(rHelper, eDNDConstants);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    virtual void drag_source_set(const std::vector<GtkTargetEntry>& rGtkTargets, GdkDragAction eDragAction) override
    {
        if (rGtkTargets.empty() && !eDragAction)
            gtk_tree_view_unset_rows_drag_source(m_pTreeView);
        else
            gtk_tree_view_enable_model_drag_source(m_pTreeView, GDK_BUTTON1_MASK, rGtkTargets.data(), rGtkTargets.size(), eDragAction);
    }
#endif

    virtual void set_selection_mode(SelectionMode eMode) override
    {
        disable_notify_events();
        gtk_tree_selection_set_mode(gtk_tree_view_get_selection(m_pTreeView), VclToGtk(eMode));
        enable_notify_events();
    }

    virtual int count_selected_rows() const override
    {
        return gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(m_pTreeView));
    }

    int starts_with(const OUString& rStr, int nStartRow, bool bCaseSensitive)
    {
        return ::starts_with(m_pTreeModel, rStr, m_nTextCol, nStartRow, bCaseSensitive);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
        g_signal_handler_block(m_pTreeView, m_nRowActivatedSignalId);

        g_signal_handler_block(m_pTreeModel, m_nRowDeletedSignalId);
        g_signal_handler_block(m_pTreeModel, m_nRowInsertedSignalId);

        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();

        g_signal_handler_unblock(m_pTreeModel, m_nRowDeletedSignalId);
        g_signal_handler_unblock(m_pTreeModel, m_nRowInsertedSignalId);

        g_signal_handler_unblock(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_unblock(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
    }

    virtual void connect_popup_menu(const Link<const CommandEvent&, bool>& rLink) override
    {
        ensureButtonPressSignal();
        weld::TreeView::connect_popup_menu(rLink);
    }

    virtual bool get_dest_row_at_pos(const Point &rPos, weld::TreeIter* pResult, bool bDnDMode) override
    {
        if (rPos.X() < 0 || rPos.Y() < 0)
        {
            // short-circuit to avoid "gtk_tree_view_get_dest_row_at_pos: assertion 'drag_x >= 0'" g_assert
            return false;
        }

        const bool bAsTree = gtk_tree_view_get_enable_tree_lines(m_pTreeView);

        // to keep it simple we'll default to always drop before the current row
        // except for the special edge cases
        GtkTreeViewDropPosition pos = bAsTree ? GTK_TREE_VIEW_DROP_INTO_OR_BEFORE : GTK_TREE_VIEW_DROP_BEFORE;

        // unhighlight current highlighted row
        gtk_tree_view_set_drag_dest_row(m_pTreeView, nullptr, pos);

        if (m_bWorkAroundBadDragRegion)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            gtk_widget_unset_state_flags(GTK_WIDGET(m_pTreeView), GTK_STATE_FLAG_DROP_ACTIVE);
#else
            gtk_drag_unhighlight(GTK_WIDGET(m_pTreeView));
#endif
        }

        GtkTreePath *path = nullptr;
        GtkTreeViewDropPosition gtkpos = bAsTree ? GTK_TREE_VIEW_DROP_INTO_OR_BEFORE : GTK_TREE_VIEW_DROP_BEFORE;
        bool ret = gtk_tree_view_get_dest_row_at_pos(m_pTreeView, rPos.X(), rPos.Y(),
                                                     &path, &gtkpos);

        // find the last entry in the model for comparison
        GtkTreePath *lastpath = get_path_of_last_entry(m_pTreeModel);

        if (!ret)
        {
            // empty space, draw an indicator at the last entry
            assert(!path);
            path = gtk_tree_path_copy(lastpath);
            pos = GTK_TREE_VIEW_DROP_AFTER;
        }
        else if (bDnDMode && gtk_tree_path_compare(path, lastpath) == 0)
        {
            // if we're on the last entry, see if gtk thinks
            // the drop should be before or after it, and if
            // its after, treat it like a drop into empty
            // space, i.e. append it
            if (gtkpos == GTK_TREE_VIEW_DROP_AFTER ||
                gtkpos == GTK_TREE_VIEW_DROP_INTO_OR_AFTER)
            {
                ret = false;
                pos = bAsTree ? gtkpos : GTK_TREE_VIEW_DROP_AFTER;
            }
        }

        if (ret && pResult)
        {
            GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(*pResult);
            gtk_tree_model_get_iter(m_pTreeModel, &rGtkIter.iter, path);
        }

        if (m_bInDrag && bDnDMode)
        {
            // highlight the row
            gtk_tree_view_set_drag_dest_row(m_pTreeView, path, pos);
        }

        assert(path);
        gtk_tree_path_free(path);
        gtk_tree_path_free(lastpath);

        // auto scroll if we're close to the edges
        GtkAdjustment* pVAdjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(m_pTreeView));
        double fStep = gtk_adjustment_get_step_increment(pVAdjustment);
        if (rPos.Y() < fStep)
        {
            double fValue = gtk_adjustment_get_value(pVAdjustment) - fStep;
            if (fValue < 0)
                fValue = 0.0;
            gtk_adjustment_set_value(pVAdjustment, fValue);
        }
        else
        {
            GdkRectangle aRect;
            gtk_tree_view_get_visible_rect(m_pTreeView, &aRect);
            if (rPos.Y() > aRect.height - fStep)
            {
                double fValue = gtk_adjustment_get_value(pVAdjustment) + fStep;
                double fMax = gtk_adjustment_get_upper(pVAdjustment);
                if (fValue > fMax)
                    fValue = fMax;
                gtk_adjustment_set_value(pVAdjustment, fValue);
            }
        }

        return ret;
    }

    virtual void unset_drag_dest_row() override
    {
        gtk_tree_view_set_drag_dest_row(m_pTreeView, nullptr, GTK_TREE_VIEW_DROP_BEFORE);
    }

    virtual tools::Rectangle get_row_area(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreePath* pPath = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        tools::Rectangle aRet = ::get_row_area(m_pTreeView, m_pColumns, pPath);
        gtk_tree_path_free(pPath);
        return aRet;
    }

    virtual void start_editing(const weld::TreeIter& rIter) override
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, m_nTextView));
        assert(pColumn && "wrong column");

        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreePath* path = gtk_tree_model_get_path(m_pTreeModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));

        // allow editing of cells which are not usually editable, so we can have double click
        // do its usual row-activate but if we explicitly want to edit (remote files dialog)
        // we can still do that
        GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
        for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
        {
            GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
            if (GTK_IS_CELL_RENDERER_TEXT(pCellRenderer))
            {
                gboolean is_editable(false);
                g_object_get(pCellRenderer, "editable", &is_editable, nullptr);
                if (!is_editable)
                {
                    g_object_set(pCellRenderer, "editable", true, "editable-set", true, nullptr);
                    g_object_set_data(G_OBJECT(pCellRenderer), "g-lo-RestoreNonEditable", reinterpret_cast<gpointer>(true));
                    break;
                }
            }
        }
        g_list_free(pRenderers);

        gtk_tree_view_scroll_to_cell(m_pTreeView, path, pColumn, false, 0, 0);
        gtk_tree_view_set_cursor(m_pTreeView, path, pColumn, true);

        gtk_tree_path_free(path);
    }

    virtual void end_editing() override
    {
        GtkTreeViewColumn *focus_column = nullptr;
        gtk_tree_view_get_cursor(m_pTreeView, nullptr, &focus_column);
        if (focus_column)
            gtk_cell_area_stop_editing(gtk_cell_layout_get_area(GTK_CELL_LAYOUT(focus_column)), true);
    }

    virtual TreeView* get_drag_source() const override
    {
        return g_DragSource;
    }

    virtual bool do_signal_drag_begin(bool& rUnsetDragIcon) override
    {
        if (m_aDragBeginHdl.Call(rUnsetDragIcon))
            return true;
        g_DragSource = this;
        return false;
    }

    virtual void do_signal_drag_end() override
    {
        g_DragSource = nullptr;
    }

    // Under gtk 3.24.8 dragging into the TreeView is not highlighting
    // entire TreeView widget, just the rectangle which has no entries
    // in it, so as a workaround highlight the parent container
    // on drag start, and undo it on drag end, and trigger removal
    // of the treeview's highlight effort
    virtual void drag_started() override
    {
        m_bInDrag = true;
        GtkWidget* pWidget = GTK_WIDGET(m_pTreeView);
        GtkWidget* pParent = gtk_widget_get_parent(pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            gtk_widget_unset_state_flags(pWidget, GTK_STATE_FLAG_DROP_ACTIVE);
            gtk_widget_set_state_flags(pParent, GTK_STATE_FLAG_DROP_ACTIVE, false);
#else
            gtk_drag_unhighlight(pWidget);
            gtk_drag_highlight(pParent);
#endif
            m_bWorkAroundBadDragRegion = true;
        }
    }

    virtual void drag_ended() override
    {
        m_bInDrag = false;
        if (m_bWorkAroundBadDragRegion)
        {
            GtkWidget* pWidget = GTK_WIDGET(m_pTreeView);
            GtkWidget* pParent = gtk_widget_get_parent(pWidget);
#if GTK_CHECK_VERSION(4, 0, 0)
            gtk_widget_unset_state_flags(pParent, GTK_STATE_FLAG_DROP_ACTIVE);
#else
            gtk_drag_unhighlight(pParent);
#endif
            m_bWorkAroundBadDragRegion = false;
        }
        // unhighlight the row
        gtk_tree_view_set_drag_dest_row(m_pTreeView, nullptr, GTK_TREE_VIEW_DROP_BEFORE);
    }

    virtual int vadjustment_get_value() const override
    {
        if (m_nPendingVAdjustment != -1)
            return m_nPendingVAdjustment;
        return gtk_adjustment_get_value(m_pVAdjustment);
    }

    virtual void vadjustment_set_value(int value) override
    {
        disable_notify_events();

        /* This rube goldberg device is to remove flicker from setting the
           scroll position of a GtkTreeView directly after clearing it and
           filling it. As a specific example the writer navigator with ~100
           tables, scroll to the end, right click on an entry near the end
           and rename it, the tree is cleared and refilled and an attempt
           made to set the scroll position of the freshly refilled tree to
           the same point as before the clear.
        */

        // This forces the tree to recalculate now its preferred size
        // after being cleared
        GtkRequisition size;
        gtk_widget_get_preferred_size(GTK_WIDGET(m_pTreeView), nullptr, &size);

        m_nPendingVAdjustment = value;

        // The value set here just has to be different to the final value
        // set later so that isn't a no-op
        gtk_adjustment_set_value(m_pVAdjustment, value - 0.0001);

        // This will set the desired m_nPendingVAdjustment value right
        // before the tree gets drawn
        gtk_widget_add_tick_callback(GTK_WIDGET(m_pTreeView), setAdjustmentCallback, this, nullptr);

        enable_notify_events();
    }

    void call_signal_custom_render(VirtualDevice& rOutput, const tools::Rectangle& rRect, bool bSelected, const OUString& rId)
    {
        signal_custom_render(rOutput, rRect, bSelected, rId);
    }

    Size call_signal_custom_get_size(VirtualDevice& rOutput, const OUString& rId)
    {
        return signal_custom_get_size(rOutput, rId);
    }

    virtual void set_show_expanders(bool bShow) override
    {
        gtk_tree_view_set_show_expanders(m_pTreeView, bShow);
    }

    virtual ~GtkInstanceTreeView() override
    {
        if (m_pChangeEvent)
            Application::RemoveUserEvent(m_pChangeEvent);
        if (m_nQueryTooltipSignalId)
            g_signal_handler_disconnect(m_pTreeView, m_nQueryTooltipSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pTreeView, m_nKeyPressSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nPopupMenuSignalId);
#endif
        g_signal_handler_disconnect(m_pTreeModel, m_nRowDeletedSignalId);
        g_signal_handler_disconnect(m_pTreeModel, m_nRowInsertedSignalId);

        if (m_nVAdjustmentChangedSignalId)
        {
            GtkAdjustment* pVAdjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(m_pTreeView));
            g_signal_handler_disconnect(pVAdjustment, m_nVAdjustmentChangedSignalId);
        }

        g_signal_handler_disconnect(m_pTreeView, m_nTestCollapseRowSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nTestExpandRowSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_disconnect(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_POINTER);
        g_value_set_pointer(&value, static_cast<gpointer>(nullptr));

        for (GList* pEntry = g_list_last(m_pColumns); pEntry; pEntry = g_list_previous(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            g_signal_handler_disconnect(pColumn, m_aColumnSignalIds.back());
            m_aColumnSignalIds.pop_back();

            // unset "instance" to avoid dangling "instance" points in any CustomCellRenderers
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
            {
                GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
                if (!CUSTOM_IS_CELL_RENDERER(pCellRenderer))
                    continue;
                g_object_set_property(G_OBJECT(pCellRenderer), "instance", &value);
            }
            g_list_free(pRenderers);
        }
        g_list_free(m_pColumns);
    }
};

}

IMPL_LINK_NOARG(GtkInstanceTreeView, async_signal_changed, void*, void)
{
    m_pChangeEvent = nullptr;
    signal_changed();
}

IMPL_LINK_NOARG(GtkInstanceTreeView, async_stop_cell_editing, void*, void)
{
    end_editing();
}

namespace {

class GtkInstanceIconView : public GtkInstanceWidget, public virtual weld::IconView
{
private:
    GtkIconView* m_pIconView;
    GtkTreeStore* m_pTreeStore;
    gint m_nTextCol;
    gint m_nImageCol;
    gint m_nIdCol;
    gulong m_nSelectionChangedSignalId;
    gulong m_nItemActivatedSignalId;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nPopupMenu;
#endif
    ImplSVEvent* m_pSelectionChangeEvent;

    DECL_LINK(async_signal_selection_changed, void*, void);

    bool signal_command(const CommandEvent& rCEvt)
    {
        return m_aCommandHdl.Call(rCEvt);
    }

    virtual bool signal_popup_menu(const CommandEvent& rCEvt) override
    {
        return signal_command(rCEvt);
    }

    void launch_signal_selection_changed()
    {
        //tdf#117991 selection change is sent before the focus change, and focus change
        //is what will cause a spinbutton that currently has the focus to set its contents
        //as the spin button value. So any LibreOffice callbacks on
        //signal-change would happen before the spinbutton value-change occurs.
        //To avoid this, send the signal-change to LibreOffice to occur after focus-change
        //has been processed
        if (m_pSelectionChangeEvent)
            Application::RemoveUserEvent(m_pSelectionChangeEvent);
        m_pSelectionChangeEvent = Application::PostUserEvent(LINK(this, GtkInstanceIconView, async_signal_selection_changed));
    }

    static void signalSelectionChanged(GtkIconView*, gpointer widget)
    {
        GtkInstanceIconView* pThis = static_cast<GtkInstanceIconView*>(widget);
        pThis->launch_signal_selection_changed();
    }

    void handle_item_activated()
    {
        if (signal_item_activated())
            return;
    }

    static void signalItemActivated(GtkIconView*, GtkTreePath*, gpointer widget)
    {
        GtkInstanceIconView* pThis = static_cast<GtkInstanceIconView*>(widget);
        SolarMutexGuard aGuard;
        pThis->handle_item_activated();
    }

    void insert_item(GtkTreeIter& iter, int pos, const OUString* pId, const OUString* pText, const OUString* pIconName)
    {
        gtk_tree_store_insert_with_values(m_pTreeStore, &iter, nullptr, pos,
                                          m_nTextCol, !pText ? nullptr : OUStringToOString(*pText, RTL_TEXTENCODING_UTF8).getStr(),
                                          m_nIdCol, !pId ? nullptr : OUStringToOString(*pId, RTL_TEXTENCODING_UTF8).getStr(),
                                          -1);
        if (pIconName)
        {
            GdkPixbuf* pixbuf = getPixbuf(*pIconName);
            gtk_tree_store_set(m_pTreeStore, &iter, m_nImageCol, pixbuf, -1);
            if (pixbuf)
                g_object_unref(pixbuf);
        }
    }

    void insert_item(GtkTreeIter& iter, int pos, const OUString* pId, const OUString* pText, const VirtualDevice* pIcon)
    {
        gtk_tree_store_insert_with_values(m_pTreeStore, &iter, nullptr, pos,
                                          m_nTextCol, !pText ? nullptr : OUStringToOString(*pText, RTL_TEXTENCODING_UTF8).getStr(),
                                          m_nIdCol, !pId ? nullptr : OUStringToOString(*pId, RTL_TEXTENCODING_UTF8).getStr(),
                                          -1);
        if (pIcon)
        {
            GdkPixbuf* pixbuf = getPixbuf(*pIcon);
            gtk_tree_store_set(m_pTreeStore, &iter, m_nImageCol, pixbuf, -1);
            if (pixbuf)
                g_object_unref(pixbuf);
        }
    }

    OUString get(const GtkTreeIter& iter, int col) const
    {
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        gchar* pStr;
        gtk_tree_model_get(pModel, const_cast<GtkTreeIter*>(&iter), col, &pStr, -1);
        OUString sRet(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);
        return sRet;
    }

    bool get_selected_iterator(GtkTreeIter* pIter) const
    {
        assert(gtk_icon_view_get_model(m_pIconView) && "don't request selection when frozen");
        bool bRet = false;
        {
            GtkTreeModel* pModel = GTK_TREE_MODEL(m_pTreeStore);
            GList* pList = gtk_icon_view_get_selected_items(m_pIconView);
            for (GList* pItem = g_list_first(pList); pItem; pItem = g_list_next(pItem))
            {
                if (pIter)
                {
                    GtkTreePath* path = static_cast<GtkTreePath*>(pItem->data);
                    gtk_tree_model_get_iter(pModel, pIter, path);
                }
                bRet = true;
                break;
            }
            g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));
        }
        return bRet;
    }

public:
    GtkInstanceIconView(GtkIconView* pIconView, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pIconView), pBuilder, bTakeOwnership)
        , m_pIconView(pIconView)
        , m_pTreeStore(GTK_TREE_STORE(gtk_icon_view_get_model(m_pIconView)))
        , m_nTextCol(gtk_icon_view_get_text_column(m_pIconView))
        , m_nImageCol(gtk_icon_view_get_pixbuf_column(m_pIconView))
        , m_nSelectionChangedSignalId(g_signal_connect(pIconView, "selection-changed",
                                      G_CALLBACK(signalSelectionChanged), this))
        , m_nItemActivatedSignalId(g_signal_connect(pIconView, "item-activated", G_CALLBACK(signalItemActivated), this))
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nPopupMenu(g_signal_connect(pIconView, "popup-menu", G_CALLBACK(signalPopupMenu), this))
#endif
        , m_pSelectionChangeEvent(nullptr)
    {
        m_nIdCol = m_nTextCol + 1;
    }

    virtual void insert(int pos, const OUString* pText, const OUString* pId, const OUString* pIconName, weld::TreeIter* pRet) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        insert_item(iter, pos, pId, pText, pIconName);
        if (pRet)
        {
            GtkInstanceTreeIter* pGtkRetIter = static_cast<GtkInstanceTreeIter*>(pRet);
            pGtkRetIter->iter = iter;
        }
        enable_notify_events();
    }

    virtual void insert(int pos, const OUString* pText, const OUString* pId, const VirtualDevice* pIcon, weld::TreeIter* pRet) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        insert_item(iter, pos, pId, pText, pIcon);
        if (pRet)
        {
            GtkInstanceTreeIter* pGtkRetIter = static_cast<GtkInstanceTreeIter*>(pRet);
            pGtkRetIter->iter = iter;
        }
        enable_notify_events();
    }

    virtual OUString get_selected_id() const override
    {
        assert(gtk_icon_view_get_model(m_pIconView) && "don't request selection when frozen");
        GtkTreeIter iter;
        if (get_selected_iterator(&iter))
            return get(iter, m_nIdCol);
        return OUString();
    }

    virtual void clear() override
    {
        disable_notify_events();
        gtk_tree_store_clear(m_pTreeStore);
        enable_notify_events();
    }

    virtual void freeze() override
    {
        disable_notify_events();
        bool bIsFirstFreeze = IsFirstFreeze();
        GtkInstanceWidget::freeze();
        if (bIsFirstFreeze)
        {
            g_object_ref(m_pTreeStore);
            gtk_icon_view_set_model(m_pIconView, nullptr);
            g_object_freeze_notify(G_OBJECT(m_pTreeStore));
        }
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
        if (IsLastThaw())
        {
            g_object_thaw_notify(G_OBJECT(m_pTreeStore));
            gtk_icon_view_set_model(m_pIconView, GTK_TREE_MODEL(m_pTreeStore));
            g_object_unref(m_pTreeStore);
        }
        GtkInstanceWidget::thaw();
        enable_notify_events();
    }

    virtual Size get_size_request() const override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
        {
            return Size(gtk_scrolled_window_get_min_content_width(GTK_SCROLLED_WINDOW(pParent)),
                        gtk_scrolled_window_get_min_content_height(GTK_SCROLLED_WINDOW(pParent)));
        }
        int nWidth, nHeight;
        gtk_widget_get_size_request(m_pWidget, &nWidth, &nHeight);
        return Size(nWidth, nHeight);
    }

    virtual Size get_preferred_size() const override
    {
        Size aRet(-1, -1);
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
        {
            aRet = Size(gtk_scrolled_window_get_min_content_width(GTK_SCROLLED_WINDOW(pParent)),
                        gtk_scrolled_window_get_min_content_height(GTK_SCROLLED_WINDOW(pParent)));
        }
        GtkRequisition size;
        gtk_widget_get_preferred_size(m_pWidget, nullptr, &size);
        if (aRet.Width() == -1)
            aRet.setWidth(size.width);
        if (aRet.Height() == -1)
            aRet.setHeight(size.height);
        return aRet;
    }

    virtual void show() override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
            gtk_widget_show(pParent);
        gtk_widget_show(m_pWidget);
    }

    virtual void hide() override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
            gtk_widget_hide(pParent);
        gtk_widget_hide(m_pWidget);
    }

    virtual OUString get_selected_text() const override
    {
        assert(gtk_icon_view_get_model(m_pIconView) && "don't request selection when frozen");
        GtkTreeIter iter;
        if (get_selected_iterator(&iter))
            return get(iter, m_nTextCol);
        return OUString();
    }

    virtual int count_selected_items() const override
    {
        GList* pList = gtk_icon_view_get_selected_items(m_pIconView);
        int nRet = g_list_length(pList);
        g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));
        return nRet;
    }

    virtual void select(int pos) override
    {
        assert(gtk_icon_view_get_model(m_pIconView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        if (pos == -1 || (pos == 0 && n_children() == 0))
        {
            gtk_icon_view_unselect_all(m_pIconView);
        }
        else
        {
            GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
            gtk_icon_view_select_path(m_pIconView, path);
            gtk_icon_view_scroll_to_path(m_pIconView, path, false, 0, 0);
            gtk_tree_path_free(path);
        }
        enable_notify_events();
    }

    virtual void unselect(int pos) override
    {
        assert(gtk_icon_view_get_model(m_pIconView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        if (pos == -1 || (pos == 0 && n_children() == 0))
        {
            gtk_icon_view_select_all(m_pIconView);
        }
        else
        {
            GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
            gtk_icon_view_select_path(m_pIconView, path);
            gtk_tree_path_free(path);
        }
        enable_notify_events();
    }

    virtual bool get_selected(weld::TreeIter* pIter) const override
    {
        GtkInstanceTreeIter* pGtkIter = static_cast<GtkInstanceTreeIter*>(pIter);
        return get_selected_iterator(pGtkIter ? &pGtkIter->iter : nullptr);
    }

    virtual bool get_cursor(weld::TreeIter* pIter) const override
    {
        GtkInstanceTreeIter* pGtkIter = static_cast<GtkInstanceTreeIter*>(pIter);
        GtkTreePath* path;
        gtk_icon_view_get_cursor(m_pIconView, &path, nullptr);
        if (pGtkIter && path)
        {
            GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
            gtk_tree_model_get_iter(pModel, &pGtkIter->iter, path);
        }
        return path != nullptr;
    }

    virtual void set_cursor(const weld::TreeIter& rIter) override
    {
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        gtk_icon_view_set_cursor(m_pIconView, path, nullptr, false);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual bool get_iter_first(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        return gtk_tree_model_get_iter_first(pModel, &rGtkIter.iter);
    }

    virtual void scroll_to_item(const weld::TreeIter& rIter) override
    {
        assert(gtk_icon_view_get_model(m_pIconView) && "don't select when frozen, select after thaw. Note selection doesn't survive a freeze");
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        gtk_icon_view_scroll_to_path(m_pIconView, path, false, 0, 0);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig) const override
    {
        return std::unique_ptr<weld::TreeIter>(new GtkInstanceTreeIter(static_cast<const GtkInstanceTreeIter*>(pOrig)));
    }

    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        GtkInstanceTreeIter aGtkIter(nullptr);

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GList* pList = gtk_icon_view_get_selected_items(m_pIconView);
        for (GList* pItem = g_list_first(pList); pItem; pItem = g_list_next(pItem))
        {
            GtkTreePath* path = static_cast<GtkTreePath*>(pItem->data);
            gtk_tree_model_get_iter(pModel, &aGtkIter.iter, path);
            if (func(aGtkIter))
                break;
        }
        g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));
    }

    virtual int n_children() const override
    {
        return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_pTreeStore), nullptr);
    }

    virtual OUString get_id(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        return get(rGtkIter.iter, m_nIdCol);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pIconView, m_nSelectionChangedSignalId);
        g_signal_handler_block(m_pIconView, m_nItemActivatedSignalId);

        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();

        g_signal_handler_unblock(m_pIconView, m_nItemActivatedSignalId);
        g_signal_handler_unblock(m_pIconView, m_nSelectionChangedSignalId);
    }

    virtual ~GtkInstanceIconView() override
    {
        if (m_pSelectionChangeEvent)
            Application::RemoveUserEvent(m_pSelectionChangeEvent);

        g_signal_handler_disconnect(m_pIconView, m_nItemActivatedSignalId);
        g_signal_handler_disconnect(m_pIconView, m_nSelectionChangedSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pIconView, m_nPopupMenu);
#endif
    }
};

}

IMPL_LINK_NOARG(GtkInstanceIconView, async_signal_selection_changed, void*, void)
{
    m_pSelectionChangeEvent = nullptr;
    signal_selection_changed();
}

namespace {

class GtkInstanceSpinButton : public GtkInstanceEditable, public virtual weld::SpinButton
{
private:
    GtkSpinButton* m_pButton;
    gulong m_nValueChangedSignalId;
    gulong m_nOutputSignalId;
    gulong m_nInputSignalId;
    bool m_bFormatting;
    bool m_bBlockOutput;
    bool m_bBlank;

    static void signalValueChanged(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->m_bBlank = false;
        pThis->signal_value_changed();
    }

    bool guarded_signal_output()
    {
        if (m_bBlockOutput)
            return true;
        m_bFormatting = true;
        bool bRet = signal_output();
        m_bFormatting = false;
        return bRet;
    }

    static gboolean signalOutput(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        SolarMutexGuard aGuard;
        return pThis->guarded_signal_output();
    }

    static gint signalInput(GtkSpinButton*, gdouble* new_value, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        SolarMutexGuard aGuard;
        int result;
        TriState eHandled = pThis->signal_input(&result);
        if (eHandled == TRISTATE_INDET)
            return 0;
        if (eHandled == TRISTATE_TRUE)
        {
            *new_value = pThis->toGtk(result);
            return 1;
        }
        return GTK_INPUT_ERROR;
    }

    virtual void signal_activate() override
    {
        gtk_spin_button_update(m_pButton);
        GtkInstanceEditable::signal_activate();
    }

    double toGtk(int nValue) const
    {
        return static_cast<double>(nValue) / Power10(get_digits());
    }

    int fromGtk(double fValue) const
    {
        return FRound(fValue * Power10(get_digits()));
    }

public:
    GtkInstanceSpinButton(GtkSpinButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceEditable(GTK_WIDGET(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_nValueChangedSignalId(g_signal_connect(pButton, "value-changed", G_CALLBACK(signalValueChanged), this))
        , m_nOutputSignalId(g_signal_connect(pButton, "output", G_CALLBACK(signalOutput), this))
        , m_nInputSignalId(g_signal_connect(pButton, "input", G_CALLBACK(signalInput), this))
        , m_bFormatting(false)
        , m_bBlockOutput(false)
        , m_bBlank(false)
    {
#if GTK_CHECK_VERSION(4, 0, 0)
          gtk_text_set_activates_default(GTK_TEXT(m_pDelegate), true);
#endif
    }

    virtual int get_value() const override
    {
        return fromGtk(gtk_spin_button_get_value(m_pButton));
    }

    virtual void set_value(int value) override
    {
        disable_notify_events();
        m_bBlank = false;
        gtk_spin_button_set_value(m_pButton, toGtk(value));
        enable_notify_events();
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
        // tdf#122786 if we're just formatting a value, then we're done,
        // however if set_text has been called directly we want to update our
        // value from this new text, but don't want to reformat with that value
        if (!m_bFormatting)
        {
#if GTK_CHECK_VERSION(4, 0, 0)
            gtk_editable_set_text(m_pEditable, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
#else
            gtk_entry_set_text(GTK_ENTRY(m_pButton), OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
#endif

            m_bBlockOutput = true;
            gtk_spin_button_update(m_pButton);
            m_bBlank = rText.isEmpty();
            m_bBlockOutput = false;
        }
        else
        {
            bool bKeepBlank = m_bBlank && get_value() == 0;
            if (!bKeepBlank)
            {
#if GTK_CHECK_VERSION(4, 0, 0)
                gtk_editable_set_text(m_pEditable, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
#else
                gtk_entry_set_text(GTK_ENTRY(m_pButton), OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
#endif
                m_bBlank = false;
            }
        }
        enable_notify_events();
    }

    virtual void set_range(int min, int max) override
    {
        disable_notify_events();
        gtk_spin_button_set_range(m_pButton, toGtk(min), toGtk(max));
        enable_notify_events();
    }

    virtual void get_range(int& min, int& max) const override
    {
        double gtkmin, gtkmax;
        gtk_spin_button_get_range(m_pButton, &gtkmin, &gtkmax);
        min = fromGtk(gtkmin);
        max = fromGtk(gtkmax);
    }

    virtual void set_increments(int step, int page) override
    {
        disable_notify_events();
        gtk_spin_button_set_increments(m_pButton, toGtk(step), toGtk(page));
        enable_notify_events();
    }

    virtual void get_increments(int& step, int& page) const override
    {
        double gtkstep, gtkpage;
        gtk_spin_button_get_increments(m_pButton, &gtkstep, &gtkpage);
        step = fromGtk(gtkstep);
        page = fromGtk(gtkpage);
    }

    virtual void set_digits(unsigned int digits) override
    {
        disable_notify_events();
        gtk_spin_button_set_digits(m_pButton, digits);
        enable_notify_events();
    }

    virtual unsigned int get_digits() const override
    {
        return gtk_spin_button_get_digits(m_pButton);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_aCustomFont.use_custom_font(&rFont, u"spinbutton");
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pButton, m_nValueChangedSignalId);
        GtkInstanceEditable::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceEditable::enable_notify_events();
        g_signal_handler_unblock(m_pButton, m_nValueChangedSignalId);
    }

    virtual ~GtkInstanceSpinButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nInputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nOutputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nValueChangedSignalId);
    }
};

}

namespace {

class GtkInstanceFormattedSpinButton : public GtkInstanceEditable, public virtual weld::FormattedSpinButton
{
private:
    GtkSpinButton* m_pButton;
    std::unique_ptr<weld::EntryFormatter> m_xOwnFormatter;
    weld::EntryFormatter* m_pFormatter;
    gulong m_nValueChangedSignalId;
    gulong m_nOutputSignalId;
    gulong m_nInputSignalId;
    bool m_bEmptyField;
    bool m_bSyncingValue;
    double m_dValueWhenEmpty;

    bool signal_output()
    {
        double fValue = gtk_spin_button_get_value(m_pButton);
        m_bEmptyField &= fValue == m_dValueWhenEmpty;
        if (!m_bEmptyField)
            GetFormatter().SetValue(fValue);
        return true;
    }

    static gboolean signalOutput(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceFormattedSpinButton* pThis = static_cast<GtkInstanceFormattedSpinButton*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_output();
    }

    gint signal_input(double* value)
    {
        Formatter& rFormatter = GetFormatter();
        rFormatter.Modify();
        // if the blank-mode is enabled then if the input is empty don't parse
        // the input but keep the value as it is. store what the value the
        // blank is associated with and until the value is changed, or the text
        // is updated from the outside, don't output that value
        m_bEmptyField = rFormatter.IsEmptyFieldEnabled() && get_text().isEmpty();
        if (m_bEmptyField)
        {
            m_dValueWhenEmpty = gtk_spin_button_get_value(m_pButton);
            *value = m_dValueWhenEmpty;
        }
        else
            *value = rFormatter.GetValue();
        return 1;
    }

    static gint signalInput(GtkSpinButton*, gdouble* new_value, gpointer widget)
    {
        GtkInstanceFormattedSpinButton* pThis = static_cast<GtkInstanceFormattedSpinButton*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_input(new_value);
    }

    static void signalValueChanged(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceFormattedSpinButton* pThis = static_cast<GtkInstanceFormattedSpinButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_value_changed();
    }

public:
    GtkInstanceFormattedSpinButton(GtkSpinButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceEditable(GTK_WIDGET(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_pFormatter(nullptr)
        , m_nValueChangedSignalId(g_signal_connect(pButton, "value-changed", G_CALLBACK(signalValueChanged), this))
        , m_nOutputSignalId(g_signal_connect(pButton, "output", G_CALLBACK(signalOutput), this))
        , m_nInputSignalId(g_signal_connect(pButton, "input", G_CALLBACK(signalInput), this))
        , m_bEmptyField(false)
        , m_bSyncingValue(false)
        , m_dValueWhenEmpty(0.0)
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        GtkInstanceEditable::set_text(rText);
        Formatter& rFormatter = GetFormatter();
        m_bEmptyField = rFormatter.IsEmptyFieldEnabled() && rText.isEmpty();
        if (m_bEmptyField)
            m_dValueWhenEmpty = gtk_spin_button_get_value(m_pButton);
    }

    virtual void connect_changed(const Link<weld::Entry&, void>& rLink) override
    {
        if (!m_pFormatter) // once a formatter is set, it takes over "changed"
        {
            GtkInstanceEditable::connect_changed(rLink);
            return;
        }
        m_pFormatter->connect_changed(rLink);
    }

    virtual void connect_focus_out(const Link<weld::Widget&, void>& rLink) override
    {
        if (!m_pFormatter) // once a formatter is set, it takes over "focus-out"
        {
            GtkInstanceEditable::connect_focus_out(rLink);
            return;
        }
        m_pFormatter->connect_focus_out(rLink);
    }

    virtual void SetFormatter(weld::EntryFormatter* pFormatter) override
    {
        m_xOwnFormatter.reset();
        m_pFormatter = pFormatter;
        sync_range_from_formatter();
        sync_value_from_formatter();
        sync_increments_from_formatter();
    }

    virtual weld::EntryFormatter& GetFormatter() override
    {
        if (!m_pFormatter)
        {
            auto aFocusOutHdl = m_aFocusOutHdl;
            m_aFocusOutHdl = Link<weld::Widget&, void>();
            auto aChangeHdl = m_aChangeHdl;
            m_aChangeHdl = Link<weld::Entry&, void>();

            double fValue = gtk_spin_button_get_value(m_pButton);
            double fMin, fMax;
            gtk_spin_button_get_range(m_pButton, &fMin, &fMax);
            double fStep;
            gtk_spin_button_get_increments(m_pButton, &fStep, nullptr);
            m_xOwnFormatter.reset(new weld::EntryFormatter(*this));
            m_xOwnFormatter->SetMinValue(fMin);
            m_xOwnFormatter->SetMaxValue(fMax);
            m_xOwnFormatter->SetSpinSize(fStep);
            m_xOwnFormatter->SetValue(fValue);

            m_xOwnFormatter->connect_focus_out(aFocusOutHdl);
            m_xOwnFormatter->connect_changed(aChangeHdl);

            m_pFormatter = m_xOwnFormatter.get();
        }
        return *m_pFormatter;
    }

    virtual void sync_value_from_formatter() override
    {
        if (!m_pFormatter)
            return;
        // tdf#135317 avoid reenterence
        if (m_bSyncingValue)
            return;
        m_bSyncingValue = true;
        disable_notify_events();
        // tdf#138519 use gtk_adjustment_set_value instead of gtk_spin_button_set_value because the
        // latter doesn't change the value if the new value is less than an EPSILON diff of 1e-10
        // from the old value
        gtk_adjustment_set_value(gtk_spin_button_get_adjustment(m_pButton), m_pFormatter->GetValue());
        enable_notify_events();
        m_bSyncingValue = false;
    }

    virtual void sync_range_from_formatter() override
    {
        if (!m_pFormatter)
            return;
        disable_notify_events();
        double fMin = m_pFormatter->HasMinValue() ? m_pFormatter->GetMinValue() : std::numeric_limits<double>::lowest();
        double fMax = m_pFormatter->HasMaxValue() ? m_pFormatter->GetMaxValue() : std::numeric_limits<double>::max();
        gtk_spin_button_set_range(m_pButton, fMin, fMax);
        enable_notify_events();
    }

    virtual void sync_increments_from_formatter() override
    {
        if (!m_pFormatter)
            return;
        disable_notify_events();
        double fSpinSize = m_pFormatter->GetSpinSize();
        gtk_spin_button_set_increments(m_pButton, fSpinSize, fSpinSize * 10);
        enable_notify_events();
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_aCustomFont.use_custom_font(&rFont, u"spinbutton");
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pButton, m_nValueChangedSignalId);
        GtkInstanceEditable::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceEditable::enable_notify_events();
        g_signal_handler_unblock(m_pButton, m_nValueChangedSignalId);
    }

    virtual ~GtkInstanceFormattedSpinButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nInputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nOutputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nValueChangedSignalId);

        m_pFormatter = nullptr;
        m_xOwnFormatter.reset();
    }
};

}

namespace {

class GtkInstanceLabel : public GtkInstanceWidget, public virtual weld::Label
{
private:
    GtkLabel* m_pLabel;

    void set_text_background_color(const Color& rColor)
    {
        guint16 nRed = rColor.GetRed() << 8;
        guint16 nGreen = rColor.GetRed() << 8;
        guint16 nBlue = rColor.GetBlue() << 8;

        PangoAttrType aFilterAttrs[] = {PANGO_ATTR_BACKGROUND, PANGO_ATTR_INVALID};

        PangoAttrList* pOrigList = gtk_label_get_attributes(m_pLabel);
        PangoAttrList* pAttrs = pOrigList ? pango_attr_list_copy(pOrigList) : pango_attr_list_new();
        PangoAttrList* pRemovedAttrs = pOrigList ? pango_attr_list_filter(pAttrs, filter_pango_attrs, &aFilterAttrs) : nullptr;
        pango_attr_list_insert(pAttrs, pango_attr_background_new(nRed, nGreen, nBlue));
        gtk_label_set_attributes(m_pLabel, pAttrs);
        pango_attr_list_unref(pAttrs);
        pango_attr_list_unref(pRemovedAttrs);
    }

    void set_text_foreground_color(const Color& rColor, bool bSetBold)
    {
        guint16 nRed = rColor.GetRed() << 8;
        guint16 nGreen = rColor.GetRed() << 8;
        guint16 nBlue = rColor.GetBlue() << 8;

        PangoAttrType aFilterAttrs[] = {PANGO_ATTR_FOREGROUND, PANGO_ATTR_WEIGHT, PANGO_ATTR_INVALID};

        if (!bSetBold)
            aFilterAttrs[1] = PANGO_ATTR_INVALID;

        PangoAttrList* pOrigList = gtk_label_get_attributes(m_pLabel);
        PangoAttrList* pAttrs = pOrigList ? pango_attr_list_copy(pOrigList) : pango_attr_list_new();
        PangoAttrList* pRemovedAttrs = pOrigList ? pango_attr_list_filter(pAttrs, filter_pango_attrs, &aFilterAttrs) : nullptr;
        if (rColor != COL_AUTO)
            pango_attr_list_insert(pAttrs, pango_attr_foreground_new(nRed, nGreen, nBlue));
        if (bSetBold)
            pango_attr_list_insert(pAttrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
        gtk_label_set_attributes(m_pLabel, pAttrs);
        pango_attr_list_unref(pAttrs);
        pango_attr_list_unref(pRemovedAttrs);
    }

public:
    GtkInstanceLabel(GtkLabel* pLabel, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pLabel), pBuilder, bTakeOwnership)
        , m_pLabel(pLabel)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        ::set_label(m_pLabel, rText);
    }

    virtual OUString get_label() const override
    {
        return ::get_label(m_pLabel);
    }

    virtual void set_mnemonic_widget(Widget* pTarget) override
    {
        assert(!gtk_label_get_selectable(m_pLabel) && "don't use set_mnemonic_widget on selectable labels, for consistency with gen backend");
        GtkInstanceWidget* pTargetWidget = dynamic_cast<GtkInstanceWidget*>(pTarget);
        gtk_label_set_mnemonic_widget(m_pLabel, pTargetWidget ? pTargetWidget->getWidget() : nullptr);
    }

    virtual void set_label_type(weld::LabelType eType) override
    {
        switch (eType)
        {
            case weld::LabelType::Normal:
                gtk_label_set_attributes(m_pLabel, nullptr);
                break;
            case weld::LabelType::Warning:
                set_text_background_color(COL_YELLOW);
                break;
            case weld::LabelType::Error:
                set_text_background_color(Application::GetSettings().GetStyleSettings().GetHighlightColor());
                break;
            case weld::LabelType::Title:
                set_text_foreground_color(Application::GetSettings().GetStyleSettings().GetLightColor(), true);
                break;
        }
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        ::set_font(m_pLabel, rFont);
    }

    virtual void set_font_color(const Color& rColor) override
    {
        set_text_foreground_color(rColor, false);
    }
};

}

std::unique_ptr<weld::Label> GtkInstanceFrame::weld_label_widget() const
{
    GtkWidget* pLabel = gtk_frame_get_label_widget(m_pFrame);
    if (!pLabel || !GTK_IS_LABEL(pLabel))
        return nullptr;
    return std::make_unique<GtkInstanceLabel>(GTK_LABEL(pLabel), m_pBuilder, false);
}

namespace {

GdkClipboard* widget_get_clipboard(GtkWidget* pWidget)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_widget_get_clipboard(pWidget);
#else
    return gtk_widget_get_clipboard(pWidget, GDK_SELECTION_CLIPBOARD);
#endif
}

class GtkInstanceTextView : public GtkInstanceWidget, public virtual weld::TextView
{
private:
    GtkTextView* m_pTextView;
    GtkTextBuffer* m_pTextBuffer;
    GtkAdjustment* m_pVAdjustment;
    GtkCssProvider* m_pFgCssProvider;
    WidgetFont m_aCustomFont;
    int m_nMaxTextLength;
    gulong m_nChangedSignalId; // we don't disable/enable this one, it's to implement max-length
    gulong m_nInsertTextSignalId;
    gulong m_nCursorPosSignalId;
    gulong m_nHasSelectionSignalId; // we don't disable/enable this one, it's to implement
                                    // auto-scroll to cursor on losing selection
    gulong m_nVAdjustChangedSignalId;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nButtonPressEvent; // we don't disable/enable this one, it's to block mouse
                                // click down from getting to (potential) toplevel
                                // GtkSalFrame parent, which grabs focus away

    static gboolean signalButtonPressEvent(GtkWidget*, GdkEventButton*, gpointer)
    {
        // e.g. on clicking on the help TextView in OTableDesignHelpBar the currently displayed text shouldn't disappear
        return true;
    }
#endif

    static void signalChanged(GtkTextBuffer*, gpointer widget)
    {
        GtkInstanceTextView* pThis = static_cast<GtkInstanceTextView*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_changed();
    }

    static void signalInserText(GtkTextBuffer *pBuffer, GtkTextIter *pLocation, gchar* /*pText*/, gint /*nLen*/, gpointer widget)
    {
        GtkInstanceTextView* pThis = static_cast<GtkInstanceTextView*>(widget);
        pThis->insert_text(pBuffer, pLocation);
    }

    void insert_text(GtkTextBuffer *pBuffer, GtkTextIter *pLocation)
    {
        if (m_nMaxTextLength)
        {
            gint nCount = gtk_text_buffer_get_char_count(pBuffer);
            if (nCount > m_nMaxTextLength)
            {
                GtkTextIter nStart, nEnd;
                gtk_text_buffer_get_iter_at_offset(m_pTextBuffer, &nStart, m_nMaxTextLength);
                gtk_text_buffer_get_end_iter(m_pTextBuffer, &nEnd);
                gtk_text_buffer_delete(m_pTextBuffer, &nStart, &nEnd);
                gtk_text_iter_assign(pLocation, &nStart);
            }
        }
    }

    static void signalCursorPosition(GtkTextBuffer*, GParamSpec*, gpointer widget)
    {
        GtkInstanceTextView* pThis = static_cast<GtkInstanceTextView*>(widget);
        pThis->signal_cursor_position();
    }

    static void signalHasSelection(GtkTextBuffer*, GParamSpec*, gpointer widget)
    {
        GtkInstanceTextView* pThis = static_cast<GtkInstanceTextView*>(widget);
        pThis->signal_has_selection();
    }

    void signal_has_selection()
    {
        /*
          in the data browser (Data Sources, shift+ctrl+f4), entering a
          multiline cell selects all, on cursoring to the right, the selection
          is lost and the cursor is at the end but gtk doesn't auto-scroll to
          the cursor so if the text needs scrolling to see the cursor it is off
          screen, another cursor makes gtk auto-scroll as wanted. So on losing
          selection help gtk out and do the initial scroll ourselves here
        */
        if (!gtk_text_buffer_get_has_selection(m_pTextBuffer))
        {
            GtkTextMark* pMark = gtk_text_buffer_get_insert(m_pTextBuffer);
            gtk_text_view_scroll_mark_onscreen(m_pTextView, pMark);
        }
    }

    static void signalVAdjustValueChanged(GtkAdjustment*, gpointer widget)
    {
        GtkInstanceTextView* pThis = static_cast<GtkInstanceTextView*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_vadjustment_changed();
    }

public:
    GtkInstanceTextView(GtkTextView* pTextView, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pTextView), pBuilder, bTakeOwnership)
        , m_pTextView(pTextView)
        , m_pTextBuffer(gtk_text_view_get_buffer(pTextView))
        , m_pVAdjustment(gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(pTextView)))
        , m_pFgCssProvider(nullptr)
        , m_aCustomFont(m_pWidget)
        , m_nMaxTextLength(0)
        , m_nChangedSignalId(g_signal_connect(m_pTextBuffer, "changed", G_CALLBACK(signalChanged), this))
        , m_nInsertTextSignalId(g_signal_connect_after(m_pTextBuffer, "insert-text", G_CALLBACK(signalInserText), this))
        , m_nCursorPosSignalId(g_signal_connect(m_pTextBuffer, "notify::cursor-position", G_CALLBACK(signalCursorPosition), this))
        , m_nHasSelectionSignalId(g_signal_connect(m_pTextBuffer, "notify::has-selection", G_CALLBACK(signalHasSelection), this))
        , m_nVAdjustChangedSignalId(g_signal_connect(m_pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustValueChanged), this))
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nButtonPressEvent(g_signal_connect_after(m_pTextView, "button-press-event", G_CALLBACK(signalButtonPressEvent), this))
#endif
    {
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
        {
            gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(pParent), nWidth);
            gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(pParent), nHeight);
            return;
        }
        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
        OString sText(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gtk_text_buffer_set_text(m_pTextBuffer, sText.getStr(), sText.getLength());
        enable_notify_events();
    }

    virtual OUString get_text() const override
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(m_pTextBuffer, &start, &end);
        char* pStr = gtk_text_buffer_get_text(m_pTextBuffer, &start, &end, true);
        OUString sRet(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);
        return sRet;
    }

    virtual void replace_selection(const OUString& rText) override
    {
        disable_notify_events();
        gtk_text_buffer_delete_selection(m_pTextBuffer, false, gtk_text_view_get_editable(m_pTextView));
        OString sText(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gtk_text_buffer_insert_at_cursor(m_pTextBuffer, sText.getStr(), sText.getLength());
        enable_notify_events();
    }

    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_selection_bounds(m_pTextBuffer, &start, &end);
        rStartPos = gtk_text_iter_get_offset(&start);
        rEndPos = gtk_text_iter_get_offset(&end);
        return rStartPos != rEndPos;
    }

    virtual void select_region(int nStartPos, int nEndPos) override
    {
        disable_notify_events();
        GtkTextIter start, end;
        gtk_text_buffer_get_iter_at_offset(m_pTextBuffer, &start, nStartPos);
        gtk_text_buffer_get_iter_at_offset(m_pTextBuffer, &end, nEndPos);
        gtk_text_buffer_select_range(m_pTextBuffer, &start, &end);
        GtkTextMark* mark = gtk_text_buffer_create_mark(m_pTextBuffer, "scroll", &end, true);
        gtk_text_view_scroll_mark_onscreen(m_pTextView, mark);
        enable_notify_events();
    }

    virtual void set_editable(bool bEditable) override
    {
        gtk_text_view_set_editable(m_pTextView, bEditable);
    }

    virtual bool get_editable() const override
    {
        return gtk_text_view_get_editable(m_pTextView);
    }

    virtual void set_max_length(int nChars) override
    {
        m_nMaxTextLength = nChars;
    }

    virtual void set_monospace(bool bMonospace) override
    {
        gtk_text_view_set_monospace(m_pTextView, bMonospace);
    }

    virtual void set_font_color(const Color& rColor) override
    {
        const bool bRemoveColor = rColor == COL_AUTO;
        if (bRemoveColor && !m_pFgCssProvider)
            return;
        GtkStyleContext *pWidgetContext = gtk_widget_get_style_context(GTK_WIDGET(m_pTextView));
        if (m_pFgCssProvider)
        {
            gtk_style_context_remove_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pFgCssProvider));
            m_pFgCssProvider = nullptr;
        }
        if (bRemoveColor)
            return;
        OUString sColor = rColor.AsRGBHexString();
        m_pFgCssProvider = gtk_css_provider_new();
        OUString aBuffer = "textview text { color: #" + sColor + "; }";
        OString aResult = OUStringToOString(aBuffer, RTL_TEXTENCODING_UTF8);
        css_provider_load_from_data(m_pFgCssProvider, aResult.getStr(), aResult.getLength());
        gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pFgCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_aCustomFont.use_custom_font(&rFont, u"textview");
    }

    virtual vcl::Font get_font() override
    {
        if (const vcl::Font* pFont = m_aCustomFont.get_custom_font())
            return *pFont;
        return GtkInstanceWidget::get_font();
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_block(m_pTextBuffer, m_nCursorPosSignalId);
        g_signal_handler_block(m_pTextBuffer, m_nChangedSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pTextBuffer, m_nChangedSignalId);
        g_signal_handler_unblock(m_pTextBuffer, m_nCursorPosSignalId);
        g_signal_handler_unblock(m_pVAdjustment, m_nVAdjustChangedSignalId);
    }

    // in gtk, 'up' when on the first line, will jump to the start of the line
    // if not there already
    virtual bool can_move_cursor_with_up() const override
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_selection_bounds(m_pTextBuffer, &start, &end);
        return !gtk_text_iter_equal(&start, &end) || !gtk_text_iter_is_start(&start);
    }

    // in gtk, 'down' when on the first line, will jump to the end of the line
    // if not there already
    virtual bool can_move_cursor_with_down() const override
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_selection_bounds(m_pTextBuffer, &start, &end);
        return !gtk_text_iter_equal(&start, &end) || !gtk_text_iter_is_end(&end);
    }

    virtual void cut_clipboard() override
    {
        GdkClipboard *pClipboard = widget_get_clipboard(GTK_WIDGET(m_pTextView));
        gtk_text_buffer_cut_clipboard(m_pTextBuffer, pClipboard, get_editable());
    }

    virtual void copy_clipboard() override
    {
        GdkClipboard *pClipboard = widget_get_clipboard(GTK_WIDGET(m_pTextView));
        gtk_text_buffer_copy_clipboard(m_pTextBuffer, pClipboard);
    }

    virtual void paste_clipboard() override
    {
        GdkClipboard *pClipboard = widget_get_clipboard(GTK_WIDGET(m_pTextView));
        gtk_text_buffer_paste_clipboard(m_pTextBuffer, pClipboard, nullptr, get_editable());
    }

    virtual void set_alignment(TxtAlign eXAlign) override
    {
        GtkJustification eJust = GTK_JUSTIFY_LEFT;
        switch (eXAlign)
        {
            case TxtAlign::Left:
                eJust = GTK_JUSTIFY_LEFT;
                break;
            case TxtAlign::Center:
                eJust = GTK_JUSTIFY_CENTER;
                break;
            case TxtAlign::Right:
                eJust = GTK_JUSTIFY_RIGHT;
                break;
        }
        gtk_text_view_set_justification(m_pTextView, eJust);
    }

    virtual int vadjustment_get_value() const override
    {
        return gtk_adjustment_get_value(m_pVAdjustment);
    }

    virtual void vadjustment_set_value(int value) override
    {
        disable_notify_events();
        gtk_adjustment_set_value(m_pVAdjustment, value);
        enable_notify_events();
    }

    virtual int vadjustment_get_upper() const override
    {
         return gtk_adjustment_get_upper(m_pVAdjustment);
    }

    virtual int vadjustment_get_lower() const override
    {
         return gtk_adjustment_get_lower(m_pVAdjustment);
    }

    virtual int vadjustment_get_page_size() const override
    {
        return gtk_adjustment_get_page_size(m_pVAdjustment);
    }

    virtual void show() override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
            gtk_widget_show(pParent);
        gtk_widget_show(m_pWidget);
    }

    virtual void hide() override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (GTK_IS_SCROLLED_WINDOW(pParent))
            gtk_widget_hide(pParent);
        gtk_widget_hide(m_pWidget);
    }

    virtual ~GtkInstanceTextView() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pTextView, m_nButtonPressEvent);
#endif
        g_signal_handler_disconnect(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nInsertTextSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nChangedSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nCursorPosSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nHasSelectionSignalId);
    }
};

}

namespace {

// IMHandler
class IMHandler;

#if !GTK_CHECK_VERSION(4, 0, 0)
AtkObject* (*default_drawing_area_get_accessible)(GtkWidget *widget);
#endif

class GtkInstanceDrawingArea : public GtkInstanceWidget, public virtual weld::DrawingArea
{
private:
    GtkDrawingArea* m_pDrawingArea;
    a11yref m_xAccessible;
#if !GTK_CHECK_VERSION(4, 0, 0)
    AtkObject *m_pAccessible;
#endif
    ScopedVclPtrInstance<VirtualDevice> m_xDevice;
    std::unique_ptr<IMHandler> m_xIMHandler;
    cairo_surface_t* m_pSurface;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nDrawSignalId;
#endif
    gulong m_nQueryTooltip;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nPopupMenu;
    gulong m_nScrollEvent;
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalDraw(GtkDrawingArea*, cairo_t *cr, int /*width*/, int /*height*/, gpointer widget)
#else
    static gboolean signalDraw(GtkWidget*, cairo_t* cr, gpointer widget)
#endif
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_draw(cr);
#if !GTK_CHECK_VERSION(4, 0, 0)
        return false;
#endif
    }
    void signal_draw(cairo_t* cr)
    {
        if (!m_pSurface)
            return;

        GdkRectangle rect;
#if GTK_CHECK_VERSION(4, 0, 0)
        double clip_x1, clip_x2, clip_y1, clip_y2;
        cairo_clip_extents(cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
        rect.x = clip_x1;
        rect.y = clip_y1;
        rect.width = clip_x2 - clip_x1;
        rect.height = clip_y2 - clip_y1;
        if (rect.width <= 0 || rect.height <= 0)
            return;
#else
        if (!gdk_cairo_get_clip_rectangle(cr, &rect))
            return;
#endif

        tools::Rectangle aRect(Point(rect.x, rect.y), Size(rect.width, rect.height));
        aRect = m_xDevice->PixelToLogic(aRect);
        m_xDevice->Erase(aRect);
        m_aDrawHdl.Call(std::pair<vcl::RenderContext&, const tools::Rectangle&>(*m_xDevice, aRect));
        cairo_surface_mark_dirty(m_pSurface);

        cairo_set_source_surface(cr, m_pSurface, 0, 0);
        cairo_paint(cr);

        tools::Rectangle aFocusRect(m_aGetFocusRectHdl.Call(*this));
        if (!aFocusRect.IsEmpty())
        {
            gtk_render_focus(gtk_widget_get_style_context(GTK_WIDGET(m_pDrawingArea)), cr,
                             aFocusRect.Left(), aFocusRect.Top(), aFocusRect.GetWidth(), aFocusRect.GetHeight());
        }
    }
    virtual void signal_size_allocate(guint nWidth, guint nHeight) override
    {
        Size aNewSize(nWidth, nHeight);
        if (m_pSurface && aNewSize == m_xDevice->GetOutputSizePixel())
        {
            // unchanged
            return;
        }
        m_xDevice->SetOutputSizePixel(Size(nWidth, nHeight));
        m_pSurface = get_underlying_cairo_surface(*m_xDevice);
        GtkInstanceWidget::signal_size_allocate(nWidth, nHeight);
    }
    void signal_style_updated()
    {
        m_aStyleUpdatedHdl.Call(*this);
    }
    static gboolean signalQueryTooltip(GtkWidget* pGtkWidget, gint x, gint y,
                                         gboolean /*keyboard_mode*/, GtkTooltip *tooltip,
                                         gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        tools::Rectangle aHelpArea(x, y);
        OUString aTooltip = pThis->signal_query_tooltip(aHelpArea);
        if (aTooltip.isEmpty())
            return false;
        gtk_tooltip_set_text(tooltip, OUStringToOString(aTooltip, RTL_TEXTENCODING_UTF8).getStr());
        GdkRectangle aGdkHelpArea;
        aGdkHelpArea.x = aHelpArea.Left();
        aGdkHelpArea.y = aHelpArea.Top();
        aGdkHelpArea.width = aHelpArea.GetWidth();
        aGdkHelpArea.height = aHelpArea.GetHeight();
        if (pThis->SwapForRTL())
            aGdkHelpArea.x = gtk_widget_get_allocated_width(pGtkWidget) - aGdkHelpArea.width - 1 - aGdkHelpArea.x;
        gtk_tooltip_set_tip_area(tooltip, &aGdkHelpArea);
        return true;
    }
    virtual bool signal_popup_menu(const CommandEvent& rCEvt) override
    {
        return signal_command(rCEvt);
    }
#if !GTK_CHECK_VERSION(4, 0, 0)
    bool signal_scroll(const GdkEventScroll* pEvent)
    {
        SalWheelMouseEvent aEvt(GtkSalFrame::GetWheelEvent(*pEvent));

        if (SwapForRTL())
            aEvt.mnX = gtk_widget_get_allocated_width(m_pWidget) - 1 - aEvt.mnX;

        CommandWheelMode nMode;
        sal_uInt16 nCode = aEvt.mnCode;
        bool bHorz = aEvt.mbHorz;
        if (nCode & KEY_MOD1)
            nMode = CommandWheelMode::ZOOM;
        else if (nCode & KEY_MOD2)
            nMode = CommandWheelMode::DATAZOOM;
        else
        {
            nMode = CommandWheelMode::SCROLL;
            // #i85450# interpret shift-wheel as horizontal wheel action
            if( (nCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3)) == KEY_SHIFT )
                bHorz = true;
        }

        CommandWheelData aWheelData(aEvt.mnDelta, aEvt.mnNotchDelta, aEvt.mnScrollLines,
                                    nMode, nCode, bHorz, aEvt.mbDeltaIsPixel);
        CommandEvent aCEvt(Point(aEvt.mnX, aEvt.mnY), CommandEventId::Wheel, true, &aWheelData);
        return m_aCommandHdl.Call(aCEvt);
    }
    static gboolean signalScroll(GtkWidget*, GdkEventScroll* pEvent, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        return pThis->signal_scroll(pEvent);
    }
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalResize(GtkDrawingArea*, int nWidth, int nHeight, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_size_allocate(nWidth, nHeight);
    }
#endif

    DECL_LINK(SettingsChangedHdl, VclWindowEvent&, void);
public:
    GtkInstanceDrawingArea(GtkDrawingArea* pDrawingArea, GtkInstanceBuilder* pBuilder, const a11yref& rA11y, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pDrawingArea), pBuilder, bTakeOwnership)
        , m_pDrawingArea(pDrawingArea)
        , m_xAccessible(rA11y)
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_pAccessible(nullptr)
#endif
        , m_xDevice(DeviceFormat::DEFAULT)
        , m_pSurface(nullptr)
        , m_nQueryTooltip(g_signal_connect(m_pDrawingArea, "query-tooltip", G_CALLBACK(signalQueryTooltip), this))
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nPopupMenu(g_signal_connect(m_pDrawingArea, "popup-menu", G_CALLBACK(signalPopupMenu), this))
        , m_nScrollEvent(g_signal_connect(m_pDrawingArea, "scroll-event", G_CALLBACK(signalScroll), this))
#endif
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_drawing_area_set_draw_func(m_pDrawingArea, signalDraw, this, nullptr);
#else
        m_nDrawSignalId = g_signal_connect(m_pDrawingArea, "draw", G_CALLBACK(signalDraw), this);
#endif
        gtk_widget_set_has_tooltip(m_pWidget, true);
        g_object_set_data(G_OBJECT(m_pDrawingArea), "g-lo-GtkInstanceDrawingArea", this);
        m_xDevice->EnableRTL(get_direction());

        ImplGetDefaultWindow()->AddEventListener(LINK(this, GtkInstanceDrawingArea, SettingsChangedHdl));
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    AtkObject* GetAtkObject(AtkObject* pDefaultAccessible)
    {
        if (!m_pAccessible && m_xAccessible.is())
        {
            GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
            m_pAccessible = atk_object_wrapper_new(m_xAccessible, gtk_widget_get_accessible(pParent), pDefaultAccessible);
            if (m_pAccessible)
                g_object_ref(m_pAccessible);
        }
        return m_pAccessible;
    }
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override
    {
        m_nSizeAllocateSignalId = g_signal_connect(m_pWidget, "resize", G_CALLBACK(signalResize), this);
        weld::Widget::connect_size_allocate(rLink);
    }
#endif

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!(gtk_widget_get_events(m_pWidget) & GDK_BUTTON_PRESS_MASK))
            gtk_widget_add_events(m_pWidget, GDK_BUTTON_PRESS_MASK);
#endif
        GtkInstanceWidget::connect_mouse_press(rLink);
    }

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (!(gtk_widget_get_events(m_pWidget) & GDK_BUTTON_RELEASE_MASK))
            gtk_widget_add_events(m_pWidget, GDK_BUTTON_RELEASE_MASK);
#endif
        GtkInstanceWidget::connect_mouse_release(rLink);
    }

    virtual void set_direction(bool bRTL) override
    {
        GtkInstanceWidget::set_direction(bRTL);
        m_xDevice->EnableRTL(bRTL);
    }

    virtual void set_cursor(PointerStyle ePointerStyle) override
    {
        GdkCursor *pCursor = GtkSalFrame::getDisplay()->getCursor(ePointerStyle);
        if (!gtk_widget_get_realized(GTK_WIDGET(m_pDrawingArea)))
            gtk_widget_realize(GTK_WIDGET(m_pDrawingArea));
        widget_set_cursor(GTK_WIDGET(m_pDrawingArea), pCursor);
    }

    virtual Point get_pointer_position() const override
    {
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
        GdkDevice* pPointer = gdk_seat_get_pointer(pSeat);
        double x(-1), y(-1);
        GdkSurface* pWin = widget_get_surface(m_pWidget);
        surface_get_device_position(pWin, pPointer, x, y, nullptr);
        return Point(x, y);
    }

    virtual void set_input_context(const InputContext& rInputContext) override;

    virtual void im_context_set_cursor_location(const tools::Rectangle& rCursorRect, int nExtTextInputWidth) override;

    int im_context_get_surrounding(OUString& rSurroundingText)
    {
        return signal_im_context_get_surrounding(rSurroundingText);
    }

    bool im_context_delete_surrounding(const Selection& rRange)
    {
        return signal_im_context_delete_surrounding(rRange);
    }

    virtual void queue_draw() override
    {
        gtk_widget_queue_draw(GTK_WIDGET(m_pDrawingArea));
    }

    virtual void queue_draw_area(int x, int y, int width, int height) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        tools::Rectangle aRect(Point(x, y), Size(width, height));
        aRect = m_xDevice->LogicToPixel(aRect);
        gtk_widget_queue_draw_area(GTK_WIDGET(m_pDrawingArea), aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight());
#else
        (void)x; (void)y; (void)width; (void)height;
        queue_draw();
#endif
    }

    virtual a11yref get_accessible_parent() override
    {
        //get_accessible_parent should only be needed for the vcl implementation,
        //in the gtk impl the native AtkObject parent set via
        //atk_object_wrapper_new(m_xAccessible, gtk_widget_get_accessible(pParent));
        //should negate the need.
        assert(false && "get_accessible_parent should only be called on a vcl impl");
        return uno::Reference<css::accessibility::XAccessible>();
    }

    virtual a11yrelationset get_accessible_relation_set() override
    {
        //get_accessible_relation_set should only be needed for the vcl implementation,
        //in the gtk impl the native equivalent should negate the need.
        assert(false && "get_accessible_relation_set should only be called on a vcl impl");
        return uno::Reference<css::accessibility::XAccessibleRelationSet>();
    }

    virtual Point get_accessible_location_on_screen() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
#endif
        gint x(0), y(0);
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (pAtkObject && ATK_IS_COMPONENT(pAtkObject))
            atk_component_get_extents(ATK_COMPONENT(pAtkObject), &x, &y, nullptr, nullptr, ATK_XY_SCREEN);
#endif
        return Point(x, y);
    }

    virtual void set_accessible_name(const OUString& rName) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        atk_object_set_name(pAtkObject, OUStringToOString(rName, RTL_TEXTENCODING_UTF8).getStr());
#else
        (void)rName;
#endif
    }

    virtual OUString get_accessible_name() const override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_name(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
#else
        return OUString();
#endif
    }

    virtual OUString get_accessible_description() const override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
#else
        return OUString();
#endif
    }

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants) override
    {
        do_enable_drag_source(rHelper, eDNDConstants);
    }

    virtual bool do_signal_drag_begin(bool& rUnsetDragIcon) override
    {
        rUnsetDragIcon = false;
        if (m_aDragBeginHdl.Call(*this))
            return true;
        return false;
    }

    virtual ~GtkInstanceDrawingArea() override
    {
        ImplGetDefaultWindow()->RemoveEventListener(LINK(this, GtkInstanceDrawingArea, SettingsChangedHdl));

        g_object_steal_data(G_OBJECT(m_pDrawingArea), "g-lo-GtkInstanceDrawingArea");
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (m_pAccessible)
            g_object_unref(m_pAccessible);
#endif
        css::uno::Reference<css::lang::XComponent> xComp(m_xAccessible, css::uno::UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pDrawingArea, m_nScrollEvent);
#endif
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pDrawingArea, m_nPopupMenu);
#endif
        g_signal_handler_disconnect(m_pDrawingArea, m_nQueryTooltip);
#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_drawing_area_set_draw_func(m_pDrawingArea, nullptr, nullptr, nullptr);
#else
        g_signal_handler_disconnect(m_pDrawingArea, m_nDrawSignalId);
#endif
    }

    virtual OutputDevice& get_ref_device() override
    {
        return *m_xDevice;
    }

    bool signal_command(const CommandEvent& rCEvt)
    {
        return m_aCommandHdl.Call(rCEvt);
    }

    virtual void click(const Point& rPos) override
    {
        MouseEvent aEvent(rPos);
        m_aMousePressHdl.Call(aEvent);
        m_aMouseReleaseHdl.Call(aEvent);
    }
};

IMPL_LINK(GtkInstanceDrawingArea, SettingsChangedHdl, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() != VclEventId::WindowDataChanged)
        return;

    DataChangedEvent* pData = static_cast<DataChangedEvent*>(rEvent.GetData());
    if (pData->GetType() == DataChangedEventType::SETTINGS)
        signal_style_updated();
}

class IMHandler
{
private:
    GtkInstanceDrawingArea* m_pArea;
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkEventController* m_pFocusController;
#endif
    GtkIMContext* m_pIMContext;
    OUString m_sPreeditText;
    gulong m_nFocusInSignalId;
    gulong m_nFocusOutSignalId;
    bool m_bExtTextInput;

public:
    IMHandler(GtkInstanceDrawingArea* pArea)
        : m_pArea(pArea)
        , m_pIMContext(gtk_im_multicontext_new())
        , m_bExtTextInput(false)
    {
        GtkWidget* pWidget = m_pArea->getWidget();

#if GTK_CHECK_VERSION(4, 0, 0)
        m_pFocusController = gtk_event_controller_focus_new();
        gtk_widget_add_controller(pWidget, m_pFocusController);

        m_nFocusInSignalId = g_signal_connect(m_pFocusController, "enter", G_CALLBACK(signalFocusIn), this);
        m_nFocusOutSignalId = g_signal_connect(m_pFocusController, "leave", G_CALLBACK(signalFocusOut), this);
#else
        m_nFocusInSignalId = g_signal_connect(pWidget, "focus-in-event", G_CALLBACK(signalFocusIn), this);
        m_nFocusOutSignalId = g_signal_connect(pWidget, "focus-out-event", G_CALLBACK(signalFocusOut), this);
#endif

        g_signal_connect(m_pIMContext, "preedit-start", G_CALLBACK(signalIMPreeditStart), this);
        g_signal_connect(m_pIMContext, "preedit-end", G_CALLBACK(signalIMPreeditEnd), this);
        g_signal_connect(m_pIMContext, "commit", G_CALLBACK(signalIMCommit), this);
        g_signal_connect(m_pIMContext, "preedit-changed", G_CALLBACK(signalIMPreeditChanged), this);
        g_signal_connect(m_pIMContext, "retrieve-surrounding", G_CALLBACK(signalIMRetrieveSurrounding), this);
        g_signal_connect(m_pIMContext, "delete-surrounding", G_CALLBACK(signalIMDeleteSurrounding), this);

        if (!gtk_widget_get_realized(pWidget))
            gtk_widget_realize(pWidget);
        im_context_set_client_widget(m_pIMContext, pWidget);
        if (gtk_widget_has_focus(m_pArea->getWidget()))
            gtk_im_context_focus_in(m_pIMContext);
    }

    void signalFocus(bool bIn)
    {
        if (bIn)
            gtk_im_context_focus_in(m_pIMContext);
        else
            gtk_im_context_focus_out(m_pIMContext);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalFocusIn(GtkEventControllerFocus*, gpointer im_handler)
#else
    static gboolean signalFocusIn(GtkWidget*, GdkEvent*, gpointer im_handler)
#endif
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);
        pThis->signalFocus(true);
#if !GTK_CHECK_VERSION(4, 0, 0)
        return false;
#endif
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalFocusOut(GtkEventControllerFocus*, gpointer im_handler)
#else
    static gboolean signalFocusOut(GtkWidget*, GdkEvent*, gpointer im_handler)
#endif
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);
        pThis->signalFocus(false);
#if !GTK_CHECK_VERSION(4, 0, 0)
        return false;
#endif
    }

    ~IMHandler()
    {
        EndExtTextInput();

#if GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pFocusController, m_nFocusOutSignalId);
        g_signal_handler_disconnect(m_pFocusController, m_nFocusInSignalId);
#else
        g_signal_handler_disconnect(m_pArea->getWidget(), m_nFocusOutSignalId);
        g_signal_handler_disconnect(m_pArea->getWidget(), m_nFocusInSignalId);
#endif

        if (gtk_widget_has_focus(m_pArea->getWidget()))
            gtk_im_context_focus_out(m_pIMContext);

        // first give IC a chance to deinitialize
        im_context_set_client_widget(m_pIMContext, nullptr);
        // destroy old IC
        g_object_unref(m_pIMContext);
    }

    void updateIMSpotLocation()
    {
        CommandEvent aCEvt(Point(), CommandEventId::CursorPos);
        // we expect set_cursor_location to get triggered by this
        m_pArea->signal_command(aCEvt);
    }

    void set_cursor_location(const tools::Rectangle& rRect)
    {
        GdkRectangle aArea{static_cast<int>(rRect.Left()), static_cast<int>(rRect.Top()),
                           static_cast<int>(rRect.GetWidth()), static_cast<int>(rRect.GetHeight())};
        gtk_im_context_set_cursor_location(m_pIMContext, &aArea);
    }

    static void signalIMCommit(GtkIMContext* /*pContext*/, gchar* pText, gpointer im_handler)
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);

        SolarMutexGuard aGuard;

        // at least editeng expects to have seen a start before accepting a commit
        pThis->StartExtTextInput();

        OUString sText(pText, strlen(pText), RTL_TEXTENCODING_UTF8);
        CommandExtTextInputData aData(sText, nullptr, sText.getLength(), 0, false);
        CommandEvent aCEvt(Point(), CommandEventId::ExtTextInput, false, &aData);
        pThis->m_pArea->signal_command(aCEvt);

        pThis->updateIMSpotLocation();

        pThis->EndExtTextInput();

        pThis->m_sPreeditText.clear();
    }

    static void signalIMPreeditChanged(GtkIMContext* pIMContext, gpointer im_handler)
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);

        SolarMutexGuard aGuard;

        sal_Int32 nCursorPos(0);
        sal_uInt8 nCursorFlags(0);
        std::vector<ExtTextInputAttr> aInputFlags;
        OUString sText = GtkSalFrame::GetPreeditDetails(pIMContext, aInputFlags, nCursorPos, nCursorFlags);

        // change from nothing to nothing -> do not start preedit e.g. this
        // will activate input into a calc cell without user input
        if (sText.isEmpty() && pThis->m_sPreeditText.isEmpty())
            return;

        pThis->m_sPreeditText = sText;

        CommandExtTextInputData aData(sText, aInputFlags.data(), nCursorPos, nCursorFlags, false);
        CommandEvent aCEvt(Point(), CommandEventId::ExtTextInput, false, &aData);
        pThis->m_pArea->signal_command(aCEvt);

        pThis->updateIMSpotLocation();
    }

    static gboolean signalIMRetrieveSurrounding(GtkIMContext* pContext, gpointer im_handler)
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);

        SolarMutexGuard aGuard;

        OUString sSurroundingText;
        int nCursorIndex = pThis->m_pArea->im_context_get_surrounding(sSurroundingText);

        if (nCursorIndex != -1)
        {
            OString sUTF = OUStringToOString(sSurroundingText, RTL_TEXTENCODING_UTF8);
            OUString sCursorText(sSurroundingText.copy(0, nCursorIndex));
            gtk_im_context_set_surrounding(pContext, sUTF.getStr(), sUTF.getLength(),
                OUStringToOString(sCursorText, RTL_TEXTENCODING_UTF8).getLength());
        }

        return true;
    }

    static gboolean signalIMDeleteSurrounding(GtkIMContext*, gint nOffset, gint nChars,
        gpointer im_handler)
    {
        bool bRet = false;

        IMHandler* pThis = static_cast<IMHandler*>(im_handler);

        SolarMutexGuard aGuard;

        OUString sSurroundingText;
        sal_Int32 nCursorIndex = pThis->m_pArea->im_context_get_surrounding(sSurroundingText);

        Selection aSelection = GtkSalFrame::CalcDeleteSurroundingSelection(sSurroundingText, nCursorIndex, nOffset, nChars);
        if (aSelection != Selection(SAL_MAX_UINT32, SAL_MAX_UINT32))
            bRet = pThis->m_pArea->im_context_delete_surrounding(aSelection);
        return bRet;
    }

    void StartExtTextInput()
    {
        if (m_bExtTextInput)
            return;
        CommandEvent aCEvt(Point(), CommandEventId::StartExtTextInput);
        m_pArea->signal_command(aCEvt);
        m_bExtTextInput = true;
    }

    static void signalIMPreeditStart(GtkIMContext*, gpointer im_handler)
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);
        SolarMutexGuard aGuard;
        pThis->StartExtTextInput();
        pThis->updateIMSpotLocation();
    }

    void EndExtTextInput()
    {
        if (!m_bExtTextInput)
            return;
        CommandEvent aCEvt(Point(), CommandEventId::EndExtTextInput);
        m_pArea->signal_command(aCEvt);
        m_bExtTextInput = false;
    }

    static void signalIMPreeditEnd(GtkIMContext*, gpointer im_handler)
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);
        SolarMutexGuard aGuard;
        pThis->updateIMSpotLocation();
        pThis->EndExtTextInput();
    }
};

void GtkInstanceDrawingArea::set_input_context(const InputContext& rInputContext)
{
    bool bUseIm(rInputContext.GetOptions() & InputContextFlags::Text);
    if (!bUseIm)
    {
        m_xIMHandler.reset();
        return;
    }
    // create a new im context
    if (!m_xIMHandler)
        m_xIMHandler.reset(new IMHandler(this));
}

void GtkInstanceDrawingArea::im_context_set_cursor_location(const tools::Rectangle& rCursorRect, int /*nExtTextInputWidth*/)
{
    if (!m_xIMHandler)
        return;
    m_xIMHandler->set_cursor_location(rCursorRect);
}

}

#if !GTK_CHECK_VERSION(4, 0, 0)
static gboolean signalEntryInsertSpecialCharKeyPress(GtkEntry* pEntry, GdkEventKey* pEvent, gpointer)
{
    if ((pEvent->keyval == GDK_KEY_S || pEvent->keyval == GDK_KEY_s) &&
        (pEvent->state & GDK_MODIFIER_MASK) == static_cast<GdkModifierType>(GDK_SHIFT_MASK|GDK_CONTROL_MASK))
    {
        if (auto pImplFncGetSpecialChars = vcl::GetGetSpecialCharsFunction())
        {
            weld::Window* pDialogParent = nullptr;

            GtkWidget* pTopLevel = widget_get_toplevel(GTK_WIDGET(pEntry));
            if (GtkSalFrame* pFrame = pTopLevel ? GtkSalFrame::getFromWindow(pTopLevel) : nullptr)
                pDialogParent = pFrame->GetFrameWeld();

            std::unique_ptr<GtkInstanceWindow> xFrameWeld;
            if (!pDialogParent && pTopLevel)
            {
                xFrameWeld.reset(new GtkInstanceWindow(GTK_WINDOW(pTopLevel), nullptr, false));
                pDialogParent = xFrameWeld.get();
            }

            OUString aChars = pImplFncGetSpecialChars(pDialogParent, ::get_font(GTK_WIDGET(pEntry)));
            if (!aChars.isEmpty())
            {
                gtk_editable_delete_selection(GTK_EDITABLE(pEntry));
                gint position = gtk_editable_get_position(GTK_EDITABLE(pEntry));
                OString sText(OUStringToOString(aChars, RTL_TEXTENCODING_UTF8));
                gtk_editable_insert_text(GTK_EDITABLE(pEntry), sText.getStr(), sText.getLength(),
                                         &position);
                gtk_editable_set_position(GTK_EDITABLE(pEntry), position);
            }
        }
        return true;
    }
    return false;
}
#endif

namespace {

GtkBuilder* makeMenuToggleButtonBuilder()
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    OUString aUri(AllSettings::GetUIRootDir() + "vcl/ui/menutogglebutton3.ui");
#else
    OUString aUri(AllSettings::GetUIRootDir() + "vcl/ui/menutogglebutton4.ui");
#endif
    OUString aPath;
    osl::FileBase::getSystemPathFromFileURL(aUri, aPath);
    return gtk_builder_new_from_file(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr());
}

#if !GTK_CHECK_VERSION(4, 0, 0)

GtkBuilder* makeComboBoxBuilder()
{
    OUString aUri(AllSettings::GetUIRootDir() + "vcl/ui/combobox.ui");
    OUString aPath;
    osl::FileBase::getSystemPathFromFileURL(aUri, aPath);
    return gtk_builder_new_from_file(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr());
}

// pop down the toplevel combobox menu when something is activated from a custom
// submenu, i.e. wysiwyg style menu
class CustomRenderMenuButtonHelper : public MenuHelper
{
private:
    GtkToggleButton* m_pComboBox;
public:
    CustomRenderMenuButtonHelper(GtkMenu* pMenu, GtkToggleButton* pComboBox)
        : MenuHelper(pMenu, false)
        , m_pComboBox(pComboBox)
    {
    }
    virtual void signal_item_activate(const OString& /*rIdent*/) override
    {
        gtk_toggle_button_set_active(m_pComboBox, false);
    }
};

#endif

#if GTK_CHECK_VERSION(4, 0, 0)

class GtkInstanceComboBox : public GtkInstanceWidget, public vcl::ISearchableStringList, public virtual weld::ComboBox
{
private:
    GtkComboBox* m_pComboBox;
//    GtkOverlay* m_pOverlay;
//    GtkTreeView* m_pTreeView;
//    GtkMenuButton* m_pOverlayButton; // button that the StyleDropdown uses on an active row
    GtkWidget* m_pMenuWindow;
    GtkTreeModel* m_pTreeModel;
    GtkCellRenderer* m_pButtonTextRenderer;
    GtkWidget* m_pEntry;
    GtkEditable* m_pEditable;
//    GtkCellView* m_pCellView;
    GtkEventController* m_pKeyController;
    GtkEventController* m_pEntryKeyController;
    GtkEventController* m_pMenuKeyController;
    GtkEventController* m_pEntryFocusController;
//    std::unique_ptr<CustomRenderMenuButtonHelper> m_xCustomMenuButtonHelper;
    WidgetFont m_aCustomFont;
    std::optional<vcl::Font> m_xEntryFont;
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    vcl::QuickSelectionEngine m_aQuickSelectionEngine;
    std::vector<std::unique_ptr<GtkTreeRowReference, GtkTreeRowReferenceDeleter>> m_aSeparatorRows;
    OUString m_sMenuButtonRow;
//    bool m_bHoverSelection;
//    bool m_bMouseInOverlayButton;
    bool m_bPopupActive;
    bool m_bAutoComplete;
    bool m_bAutoCompleteCaseSensitive;
    bool m_bChangedByMenu;
    bool m_bCustomRenderer;
    bool m_bUserSelectEntry;
    gint m_nTextCol;
    gint m_nIdCol;
//    gulong m_nToggleFocusInSignalId;
//    gulong m_nToggleFocusOutSignalId;
//    gulong m_nRowActivatedSignalId;
    gulong m_nChangedSignalId;
    gulong m_nPopupShownSignalId;
    gulong m_nKeyPressEventSignalId;
    gulong m_nEntryInsertTextSignalId;
    gulong m_nEntryActivateSignalId;
    gulong m_nEntryFocusInSignalId;
    gulong m_nEntryFocusOutSignalId;
    gulong m_nEntryKeyPressEventSignalId;
    guint m_nAutoCompleteIdleId;
//    gint m_nNonCustomLineHeight;
    gint m_nPrePopupCursorPos;
    int m_nMRUCount;
    int m_nMaxMRUCount;

    static gboolean idleAutoComplete(gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->auto_complete();
        return false;
    }

    void auto_complete()
    {
        m_nAutoCompleteIdleId = 0;
        OUString aStartText = get_active_text();
        int nStartPos, nEndPos;
        get_entry_selection_bounds(nStartPos, nEndPos);
        int nMaxSelection = std::max(nStartPos, nEndPos);
        if (nMaxSelection != aStartText.getLength())
            return;

        disable_notify_events();
        int nActive = get_active();
        int nStart = nActive;

        if (nStart == -1)
            nStart = 0;

        int nPos = -1;

        int nZeroRow = 0;
        if (m_nMRUCount)
            nZeroRow += (m_nMRUCount + 1);

        if (!m_bAutoCompleteCaseSensitive)
        {
            // Try match case insensitive from current position
            nPos = starts_with(m_pTreeModel, aStartText, 0, nStart, false);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case insensitive, but from start
                nPos = starts_with(m_pTreeModel, aStartText, 0, nZeroRow, false);
            }
        }

        if (nPos == -1)
        {
            // Try match case sensitive from current position
            nPos = starts_with(m_pTreeModel, aStartText, 0, nStart, true);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case sensitive, but from start
                nPos = starts_with(m_pTreeModel, aStartText, 0, nZeroRow, true);
            }
        }

        if (nPos != -1)
        {
            OUString aText = get_text_including_mru(nPos);
            if (aText != aStartText)
            {
                SolarMutexGuard aGuard;
                set_active_including_mru(nPos, true);
            }
            select_entry_region(aText.getLength(), aStartText.getLength());
        }
        enable_notify_events();
    }

    static void signalEntryInsertText(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength,
                                      gint* position, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_entry_insert_text(pEntry, pNewText, nNewTextLength, position);
    }

    void signal_entry_insert_text(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength, gint* position)
    {
        if (m_bPopupActive) // not entered by the user
            return;

        // first filter inserted text
        if (m_aEntryInsertTextHdl.IsSet())
        {
            OUString sText(pNewText, nNewTextLength, RTL_TEXTENCODING_UTF8);
            const bool bContinue = m_aEntryInsertTextHdl.Call(sText);
            if (bContinue && !sText.isEmpty())
            {
                OString sFinalText(OUStringToOString(sText, RTL_TEXTENCODING_UTF8));
                g_signal_handlers_block_by_func(pEntry, reinterpret_cast<gpointer>(signalEntryInsertText), this);
                gtk_editable_insert_text(GTK_EDITABLE(pEntry), sFinalText.getStr(), sFinalText.getLength(), position);
                g_signal_handlers_unblock_by_func(pEntry, reinterpret_cast<gpointer>(signalEntryInsertText), this);
            }
            g_signal_stop_emission_by_name(pEntry, "insert-text");
        }

        if (m_bAutoComplete)
        {
            // now check for autocompletes
            if (m_nAutoCompleteIdleId)
                g_source_remove(m_nAutoCompleteIdleId);
            m_nAutoCompleteIdleId = g_idle_add(idleAutoComplete, this);
        }
    }

    static void signalChanged(GtkComboBox*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        SolarMutexGuard aGuard;
        pThis->fire_signal_changed();
    }

    void fire_signal_changed()
    {
        m_bUserSelectEntry = true;
        m_bChangedByMenu = m_bPopupActive;
        signal_changed();
        m_bChangedByMenu = false;
    }

    static void signalPopupToggled(GObject*, GParamSpec*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_popup_toggled();
    }

#if 0
    int get_popup_height(gint& rPopupWidth)
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

        int nMaxRows = rSettings.GetListBoxMaximumLineCount();
        bool bAddScrollWidth = false;
        int nRows = get_count_including_mru();
        if (nMaxRows < nRows)
        {
            nRows = nMaxRows;
            bAddScrollWidth = true;
        }

        GList* pColumns = gtk_tree_view_get_columns(m_pTreeView);
        gint nRowHeight = get_height_row(m_pTreeView, pColumns);
        g_list_free(pColumns);

        gint nSeparatorHeight = get_height_row_separator(m_pTreeView);
        gint nHeight = get_height_rows(nRowHeight, nSeparatorHeight, nRows);

        // if we're using a custom renderer, limit the height to the height nMaxRows would be
        // for a normal renderer, and then round down to how many custom rows fit in that
        // space
        if (m_nNonCustomLineHeight != -1 && nRowHeight)
        {
            gint nNormalHeight = get_height_rows(m_nNonCustomLineHeight, nSeparatorHeight, nMaxRows);
            if (nHeight > nNormalHeight)
            {
                gint nRowsOnly = nNormalHeight - get_height_rows(0, nSeparatorHeight, nMaxRows);
                gint nCustomRows = (nRowsOnly + (nRowHeight - 1)) / nRowHeight;
                nHeight = get_height_rows(nRowHeight, nSeparatorHeight, nCustomRows);
            }
        }

        if (bAddScrollWidth)
            rPopupWidth += rSettings.GetScrollBarSize();

        return nHeight;
    }
#endif

    bool toggle_button_get_active()
    {
        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_BOOLEAN);
        g_object_get_property(G_OBJECT(m_pComboBox), "popup-shown", &value);
        return g_value_get_boolean(&value);
    }

    void menu_toggled()
    {
        if (!m_bPopupActive)
        {
#if 0
            if (m_bHoverSelection)
            {
                // turn hover selection back off until mouse is moved again
                // *after* menu is shown again
                gtk_tree_view_set_hover_selection(m_pTreeView, false);
                m_bHoverSelection = false;
            }
#endif

            if (!m_bUserSelectEntry)
                set_active_including_mru(m_nPrePopupCursorPos, true);

#if 0
            // undo show_menu tooltip blocking
            GtkWidget* pParent = widget_get_toplevel(m_pToggleButton);
            GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
            if (pFrame)
                pFrame->UnblockTooltip();
#endif
        }
        else
        {
            m_nPrePopupCursorPos = get_active();

            m_bUserSelectEntry = false;

            // if we are in mru mode always start with the cursor at the top of the menu
            if (m_nMaxMRUCount)
                set_active_including_mru(0, true);
        }
    }

    virtual void signal_popup_toggled() override
    {
        m_aQuickSelectionEngine.Reset();

        bool bOldPopupActive = m_bPopupActive;
        m_bPopupActive = toggle_button_get_active();

        menu_toggled();

        if (bOldPopupActive != m_bPopupActive)
        {
            ComboBox::signal_popup_toggled();
            // restore focus to the GtkEntry when the popup is gone, which
            // is what the vcl case does, to ease the transition a little,
            // but don't do it if the focus was moved out of togglebutton
            // by something else already (e.g. font combobox in toolbar
            // on a "direct pick" from the menu which moves focus into
            // the main document
            if (!m_bPopupActive && m_pEntry && has_child_focus())
            {
                disable_notify_events();
                gtk_widget_grab_focus(m_pEntry);
                enable_notify_events();
            }
        }
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalEntryFocusIn(GtkEventControllerFocus*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_entry_focus_in();
    }
#else
    static gboolean signalEntryFocusIn(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_entry_focus_in();
        return false;
    }
#endif

    void signal_entry_focus_in()
    {
        signal_focus_in();
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    static void signalEntryFocusOut(GtkEventControllerFocus*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_entry_focus_out();
    }
#else
    static gboolean signalEntryFocusOut(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_entry_focus_out();
        return false;
    }
#endif

    void signal_entry_focus_out()
    {
        // if we have an untidy selection on losing focus remove the selection
        int nStartPos, nEndPos;
        if (get_entry_selection_bounds(nStartPos, nEndPos))
        {
            int nMin = std::min(nStartPos, nEndPos);
            int nMax = std::max(nStartPos, nEndPos);
            if (nMin != 0 || nMax != get_active_text().getLength())
                select_entry_region(0, 0);
        }
        signal_focus_out();
    }

    static void signalEntryActivate(GtkEntry*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_entry_activate();
    }

    void signal_entry_activate()
    {
        if (m_aEntryActivateHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            if (m_aEntryActivateHdl.Call(*this))
                g_signal_stop_emission_by_name(m_pEntry, "activate");
        }
        update_mru();
    }

    OUString get(int pos, int col) const
    {
        OUString sRet;
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
        {
            gchar* pStr;
            gtk_tree_model_get(m_pTreeModel, &iter, col, &pStr, -1);
            sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

    void set(int pos, int col, std::u16string_view rText)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
        {
            OString aStr(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
            gtk_list_store_set(GTK_LIST_STORE(m_pTreeModel), &iter, col, aStr.getStr(), -1);
        }
    }

    int find(std::u16string_view rStr, int col, bool bSearchMRUArea) const
    {
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter_first(m_pTreeModel, &iter))
            return -1;

        int nRet = 0;

        if (!bSearchMRUArea && m_nMRUCount)
        {
            if (!gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, m_nMRUCount + 1))
                return -1;
            nRet += (m_nMRUCount + 1);
        }

        OString aStr(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
        do
        {
            gchar* pStr;
            gtk_tree_model_get(m_pTreeModel, &iter, col, &pStr, -1);
            const bool bEqual = g_strcmp0(pStr, aStr.getStr()) == 0;
            g_free(pStr);
            if (bEqual)
                return nRet;
            ++nRet;
        } while (gtk_tree_model_iter_next(m_pTreeModel, &iter));

        return -1;
    }

    bool separator_function(const GtkTreePath* path)
    {
        return ::separator_function(path, m_aSeparatorRows);
    }

    bool separator_function(int pos)
    {
        GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
        bool bRet = separator_function(path);
        gtk_tree_path_free(path);
        return bRet;
    }

    static gboolean separatorFunction(GtkTreeModel* pTreeModel, GtkTreeIter* pIter, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        GtkTreePath* path = gtk_tree_model_get_path(pTreeModel, pIter);
        bool bRet = pThis->separator_function(path);
        gtk_tree_path_free(path);
        return bRet;
    }

    // https://gitlab.gnome.org/GNOME/gtk/issues/310
    //
    // in the absence of a built-in solution
    // a) support typeahead for the case where there is no entry widget, typing ahead
    // into the button itself will select via the vcl selection engine, a matching
    // entry
    static gboolean signalKeyPress(GtkEventControllerKey*, guint keyval, guint keycode, GdkModifierType state, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        return pThis->signal_key_press(CreateKeyEvent(keyval, keycode, state, 0));
    }

    // tdf#131076 we want return in a ComboBox to act like return in a
    // GtkEntry and activate the default dialog/assistant button
    bool combobox_activate()
    {
        GtkWidget *pComboBox = GTK_WIDGET(m_pComboBox);
        GtkWidget *pToplevel = widget_get_toplevel(pComboBox);
        GtkWindow *pWindow = GTK_WINDOW(pToplevel);
        if (!pWindow)
            return false;
        if (!GTK_IS_DIALOG(pWindow) && !GTK_IS_ASSISTANT(pWindow))
            return false;
        bool bDone = false;
        GtkWidget *pDefaultWidget = gtk_window_get_default_widget(pWindow);
        if (pDefaultWidget && pDefaultWidget != pComboBox && gtk_widget_get_sensitive(pDefaultWidget))
            bDone = gtk_widget_activate(pDefaultWidget);
        return bDone;
    }

    static gboolean signalEntryKeyPress(GtkEventControllerKey*, guint keyval, guint keycode, GdkModifierType state, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        LocalizeDecimalSeparator(keyval);
        return pThis->signal_entry_key_press(CreateKeyEvent(keyval, keycode, state, 0));
    }

    bool signal_entry_key_press(const KeyEvent& rKEvt)
    {
        vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

        bool bDone = false;

        auto nCode = aKeyCode.GetCode();
        switch (nCode)
        {
            case KEY_DOWN:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nCount = get_count_including_mru();
                    int nActive = get_active_including_mru() + 1;
                    while (nActive < nCount && separator_function(nActive))
                        ++nActive;
                    if (nActive < nCount)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                else if (nKeyMod == KEY_MOD2 && !m_bPopupActive)
                {
                    gtk_combo_box_popup(m_pComboBox);
                    bDone = true;
                }
                break;
            }
            case KEY_UP:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nStartBound = m_bPopupActive ? 0 : (m_nMRUCount + 1);
                    int nActive = get_active_including_mru() - 1;
                    while (nActive >= nStartBound && separator_function(nActive))
                        --nActive;
                    if (nActive >= nStartBound)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                break;
            }
            case KEY_PAGEUP:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nCount = get_count_including_mru();
                    int nStartBound = m_bPopupActive ? 0 : (m_nMRUCount + 1);
                    int nActive = nStartBound;
                    while (nActive < nCount && separator_function(nActive))
                        ++nActive;
                    if (nActive < nCount)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                break;
            }
            case KEY_PAGEDOWN:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nActive = get_count_including_mru() - 1;
                    int nStartBound = m_bPopupActive ? 0 : (m_nMRUCount + 1);
                    while (nActive >= nStartBound && separator_function(nActive))
                        --nActive;
                    if (nActive >= nStartBound)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                break;
            }
            default:
                break;
        }

        return bDone;
    }

    bool signal_key_press(const KeyEvent& rKEvt)
    {
#if 0
        if (m_bHoverSelection)
        {
            // once a key is pressed, turn off hover selection until mouse is
            // moved again otherwise when the treeview scrolls it jumps to the
            // position under the mouse.
            gtk_tree_view_set_hover_selection(m_pTreeView, false);
            m_bHoverSelection = false;
        }
#endif

        vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

        bool bDone = false;

        auto nCode = aKeyCode.GetCode();
        switch (nCode)
        {
            case KEY_DOWN:
            case KEY_UP:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case KEY_HOME:
            case KEY_END:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_RETURN:
            {
                m_aQuickSelectionEngine.Reset();
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                // tdf#131076 don't let bare return toggle menu popup active, but do allow deactivate
                if (nCode == KEY_RETURN && !nKeyMod)
                {
                    if (!m_bPopupActive)
                        bDone = combobox_activate();
                    else
                    {
                        // treat 'return' as if the active entry was clicked on
                        signalChanged(m_pComboBox, this);
                        gtk_combo_box_popdown(m_pComboBox);
                        bDone = true;
                    }
                }
                else if (nCode == KEY_UP && nKeyMod == KEY_MOD2 && m_bPopupActive)
                {
                    gtk_combo_box_popdown(m_pComboBox);
                    bDone = true;
                }
                else if (nCode == KEY_DOWN && nKeyMod == KEY_MOD2 && !m_bPopupActive)
                {
                    gtk_combo_box_popup(m_pComboBox);
                    bDone = true;
                }
                break;
            }
            case KEY_ESCAPE:
            {
                m_aQuickSelectionEngine.Reset();
                if (m_bPopupActive)
                {
                    gtk_combo_box_popdown(m_pComboBox);
                    bDone = true;
                }
                break;
            }
            default:
                // tdf#131076 let base space toggle menu popup when it's not already visible
                if (nCode == KEY_SPACE && !aKeyCode.GetModifier() && !m_bPopupActive)
                    bDone = false;
                else
                    bDone = m_aQuickSelectionEngine.HandleKeyEvent(rKEvt);
                break;
        }

        if (!bDone)
        {
            if (!m_pEntry)
                bDone = signal_entry_key_press(rKEvt);
            else
            {
                // with gtk4-4.2.1 the unconsumed keystrokes don't appear to get to
                // the GtkEntry for up/down to move to the next entry without this extra help
                // (which means this extra indirection is probably effectively
                // the same as if calling signal_entry_key_press directly here)
                bDone = gtk_event_controller_key_forward(GTK_EVENT_CONTROLLER_KEY(m_pMenuKeyController), m_pEntry);
            }
        }

        return bDone;
    }

    vcl::StringEntryIdentifier typeahead_getEntry(int nPos, OUString& out_entryText) const
    {
        int nEntryCount(get_count_including_mru());
        if (nPos >= nEntryCount)
            nPos = 0;
        out_entryText = get_text_including_mru(nPos);

        // vcl::StringEntryIdentifier does not allow for 0 values, but our position is 0-based
        // => normalize
        return reinterpret_cast<vcl::StringEntryIdentifier>(nPos + 1);
    }

    static int typeahead_getEntryPos(vcl::StringEntryIdentifier entry)
    {
        // our pos is 0-based, but StringEntryIdentifier does not allow for a NULL
        return reinterpret_cast<sal_Int64>(entry) - 1;
    }

    int tree_view_get_cursor() const
    {
        int nRet = -1;
#if 0
        GtkTreePath* path;
        gtk_tree_view_get_cursor(m_pTreeView, &path, nullptr);
        if (path)
        {
            gint depth;
            gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
            nRet = indices[depth-1];
            gtk_tree_path_free(path);
        }
#endif

        return nRet;
    }

    int get_selected_entry() const
    {
        if (m_bPopupActive)
            return tree_view_get_cursor();
        else
            return get_active_including_mru();
    }

    void set_typeahead_selected_entry(int nSelect)
    {
        set_active_including_mru(nSelect, true);
    }

    virtual vcl::StringEntryIdentifier CurrentEntry(OUString& out_entryText) const override
    {
        int nCurrentPos = get_selected_entry();
        return typeahead_getEntry((nCurrentPos == -1) ? 0 : nCurrentPos, out_entryText);
    }

    virtual vcl::StringEntryIdentifier NextEntry(vcl::StringEntryIdentifier currentEntry, OUString& out_entryText) const override
    {
        int nNextPos = typeahead_getEntryPos(currentEntry) + 1;
        return typeahead_getEntry(nNextPos, out_entryText);
    }

    virtual void SelectEntry(vcl::StringEntryIdentifier entry) override
    {
        int nSelect = typeahead_getEntryPos(entry);
        if (nSelect == get_selected_entry())
        {
            // ignore that. This method is a callback from the QuickSelectionEngine, which means the user attempted
            // to select the given entry by typing its starting letters. No need to act.
            return;
        }

        // normalize
        int nCount = get_count_including_mru();
        if (nSelect >= nCount)
            nSelect = nCount ? nCount-1 : -1;

        set_typeahead_selected_entry(nSelect);
    }

#if 0
    static void signalGrabBroken(GtkWidget*, GdkEventGrabBroken *pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->grab_broken(pEvent);
    }

    void grab_broken(const GdkEventGrabBroken *event)
    {
        if (event->grab_window == nullptr)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);
        }
        else if (!g_object_get_data(G_OBJECT(event->grab_window), "g-lo-InstancePopup")) // another LibreOffice popover took a grab
        {
            //try and regrab, so when we lose the grab to the menu of the color palette
            //combobox we regain it so the color palette doesn't itself disappear on next
            //click on the color palette combobox
            do_grab(GTK_WIDGET(m_pMenuWindow));
        }
    }

    static gboolean signalButtonPress(GtkWidget* pWidget, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        return pThis->button_press(pWidget, pEvent);
    }

    bool button_press(GtkWidget* pWidget, GdkEventButton* pEvent)
    {
        //we want to pop down if the button was pressed outside our popup
        gdouble x = pEvent->x_root;
        gdouble y = pEvent->y_root;
        gint xoffset, yoffset;
        gdk_window_get_root_origin(widget_get_surface(pWidget), &xoffset, &yoffset);

        GtkAllocation alloc;
        gtk_widget_get_allocation(pWidget, &alloc);
        xoffset += alloc.x;
        yoffset += alloc.y;

        gtk_widget_get_allocation(GTK_WIDGET(m_pMenuWindow), &alloc);
        gint x1 = alloc.x + xoffset;
        gint y1 = alloc.y + yoffset;
        gint x2 = x1 + alloc.width;
        gint y2 = y1 + alloc.height;

        if (x > x1 && x < x2 && y > y1 && y < y2)
            return false;

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);

        return false;
    }

    static gboolean signalMotion(GtkWidget*, GdkEventMotion*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_motion();
        return false;
    }

    void signal_motion()
    {
        // if hover-selection was disabled after pressing a key, then turn it back on again
        if (!m_bHoverSelection && !m_bMouseInOverlayButton)
        {
            gtk_tree_view_set_hover_selection(m_pTreeView, true);
            m_bHoverSelection = true;
        }
    }
#endif

    static void signalRowActivated(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->handle_row_activated();
    }

    void handle_row_activated()
    {
        m_bUserSelectEntry = true;
        m_bChangedByMenu = true;
        disable_notify_events();
        int nActive = get_active();
        if (m_pEditable)
            gtk_editable_set_text(m_pEditable, OUStringToOString(get_text(nActive), RTL_TEXTENCODING_UTF8).getStr());
#if 0
        else
            tree_view_set_cursor(nActive);
#endif
        enable_notify_events();
//        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);
        fire_signal_changed();
        update_mru();
    }

    void do_clear()
    {
        disable_notify_events();
        gtk_combo_box_set_row_separator_func(m_pComboBox, nullptr, nullptr, nullptr);
        m_aSeparatorRows.clear();
        gtk_list_store_clear(GTK_LIST_STORE(m_pTreeModel));
        m_nMRUCount = 0;
        enable_notify_events();
    }

    virtual int get_max_mru_count() const override
    {
        return m_nMaxMRUCount;
    }

    virtual void set_max_mru_count(int nMaxMRUCount) override
    {
        m_nMaxMRUCount = nMaxMRUCount;
        update_mru();
    }

    void update_mru()
    {
        int nMRUCount = m_nMRUCount;

        if (m_nMaxMRUCount)
        {
            OUString sActiveText = get_active_text();
            OUString sActiveId = get_active_id();
            insert_including_mru(0, sActiveText, &sActiveId, nullptr, nullptr);
            ++m_nMRUCount;

            for (int i = 1; i < m_nMRUCount - 1; ++i)
            {
                if (get_text_including_mru(i) == sActiveText)
                {
                    remove_including_mru(i);
                    --m_nMRUCount;
                    break;
                }
            }
        }

        while (m_nMRUCount > m_nMaxMRUCount)
        {
            remove_including_mru(m_nMRUCount - 1);
            --m_nMRUCount;
        }

        if (m_nMRUCount && !nMRUCount)
            insert_separator_including_mru(m_nMRUCount, "separator");
        else if (!m_nMRUCount && nMRUCount)
            remove_including_mru(m_nMRUCount);  // remove separator
    }

    int get_count_including_mru() const
    {
        return gtk_tree_model_iter_n_children(m_pTreeModel, nullptr);
    }

    int get_active_including_mru() const
    {
        return gtk_combo_box_get_active(m_pComboBox);
    }

    void set_active_including_mru(int pos, bool bInteractive)
    {
        disable_notify_events();

        gtk_combo_box_set_active(m_pComboBox, pos);

        m_bChangedByMenu = false;
        enable_notify_events();

        if (bInteractive && !m_bPopupActive)
            signal_changed();
    }

    int find_text_including_mru(std::u16string_view rStr, bool bSearchMRU) const
    {
        return find(rStr, m_nTextCol, bSearchMRU);
    }

    int find_id_including_mru(std::u16string_view rId, bool bSearchMRU) const
    {
        return find(rId, m_nIdCol, bSearchMRU);
    }

    OUString get_text_including_mru(int pos) const
    {
        return get(pos, m_nTextCol);
    }

    OUString get_id_including_mru(int pos) const
    {
        return get(pos, m_nIdCol);
    }

    void set_id_including_mru(int pos, std::u16string_view rId)
    {
        set(pos, m_nIdCol, rId);
    }

    void remove_including_mru(int pos)
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos);
        if (!m_aSeparatorRows.empty())
        {
            bool bFound = false;

            GtkTreePath* pPath = gtk_tree_path_new_from_indices(pos, -1);

            for (auto aIter = m_aSeparatorRows.begin(); aIter != m_aSeparatorRows.end(); ++aIter)
            {
                GtkTreePath* seppath = gtk_tree_row_reference_get_path(aIter->get());
                if (seppath)
                {
                    if (gtk_tree_path_compare(pPath, seppath) == 0)
                        bFound = true;
                    gtk_tree_path_free(seppath);
                }
                if (bFound)
                {
                    m_aSeparatorRows.erase(aIter);
                    break;
                }
            }

            gtk_tree_path_free(pPath);
        }
        gtk_list_store_remove(GTK_LIST_STORE(m_pTreeModel), &iter);
        enable_notify_events();
    }

    void insert_separator_including_mru(int pos, const OUString& rId)
    {
        disable_notify_events();
        GtkTreeIter iter;
        if (!gtk_combo_box_get_row_separator_func(m_pComboBox))
            gtk_combo_box_set_row_separator_func(m_pComboBox, separatorFunction, this, nullptr);
        insert_row(GTK_LIST_STORE(m_pTreeModel), iter, pos, &rId, u"", nullptr, nullptr);
        GtkTreePath* pPath = gtk_tree_path_new_from_indices(pos, -1);
        m_aSeparatorRows.emplace_back(gtk_tree_row_reference_new(m_pTreeModel, pPath));
        gtk_tree_path_free(pPath);
        enable_notify_events();
    }

    void insert_including_mru(int pos, std::u16string_view rText, const OUString* pId, const OUString* pIconName, const VirtualDevice* pImageSurface)
    {
        disable_notify_events();
        GtkTreeIter iter;
        insert_row(GTK_LIST_STORE(m_pTreeModel), iter, pos, pId, rText, pIconName, pImageSurface);
        enable_notify_events();
    }

#if 0
    static gboolean signalGetChildPosition(GtkOverlay*, GtkWidget*, GdkRectangle* pAllocation, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        return pThis->signal_get_child_position(pAllocation);
    }

    bool signal_get_child_position(GdkRectangle* pAllocation)
    {
        if (!gtk_widget_get_visible(GTK_WIDGET(m_pOverlayButton)))
            return false;
        if (!gtk_widget_get_realized(GTK_WIDGET(m_pTreeView)))
            return false;
        int nRow = find_id_including_mru(m_sMenuButtonRow, true);
        if (nRow == -1)
            return false;

        gtk_widget_get_preferred_width(GTK_WIDGET(m_pOverlayButton), &pAllocation->width, nullptr);

        GtkTreePath* pPath = gtk_tree_path_new_from_indices(nRow, -1);
        GList* pColumns = gtk_tree_view_get_columns(m_pTreeView);
        tools::Rectangle aRect = get_row_area(m_pTreeView, pColumns, pPath);
        gtk_tree_path_free(pPath);
        g_list_free(pColumns);

        pAllocation->x = aRect.Right() - pAllocation->width;
        pAllocation->y = aRect.Top();
        pAllocation->height = aRect.GetHeight();

        return true;
    }

    static gboolean signalOverlayButtonCrossing(GtkWidget*, GdkEventCrossing* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_overlay_button_crossing(pEvent->type == GDK_ENTER_NOTIFY);
        return false;
    }

    void signal_overlay_button_crossing(bool bEnter)
    {
        m_bMouseInOverlayButton = bEnter;
        if (!bEnter)
            return;

        if (m_bHoverSelection)
        {
            // once toggled button is pressed, turn off hover selection until
            // mouse leaves the overlay button
            gtk_tree_view_set_hover_selection(m_pTreeView, false);
            m_bHoverSelection = false;
        }
        int nRow = find_id_including_mru(m_sMenuButtonRow, true);
        assert(nRow != -1);
        tree_view_set_cursor(nRow); // select the buttons row
    }
#endif

    int include_mru(int pos)
    {
        if (m_nMRUCount && pos != -1)
            pos += (m_nMRUCount + 1);
        return pos;
    }

public:
    GtkInstanceComboBox(GtkComboBox* pComboBox, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pComboBox), pBuilder, bTakeOwnership)
        , m_pComboBox(pComboBox)
//        , m_pOverlay(GTK_OVERLAY(gtk_builder_get_object(pComboBuilder, "overlay")))
//        , m_pTreeView(GTK_TREE_VIEW(gtk_builder_get_object(pComboBuilder, "treeview")))
//        , m_pOverlayButton(GTK_MENU_BUTTON(gtk_builder_get_object(pComboBuilder, "overlaybutton")))
        , m_pMenuWindow(nullptr)
        , m_pTreeModel(gtk_combo_box_get_model(pComboBox))
        , m_pButtonTextRenderer(nullptr)
//        , m_pToggleButton(GTK_WIDGET(gtk_builder_get_object(pComboBuilder, "button")))
        , m_pEntry(GTK_IS_ENTRY(gtk_combo_box_get_child(pComboBox)) ? gtk_combo_box_get_child(pComboBox) : nullptr)
        , m_pEditable(GTK_EDITABLE(m_pEntry))
        , m_aCustomFont(m_pWidget)
//        , m_pCellView(nullptr)
        , m_aQuickSelectionEngine(*this)
//        , m_bHoverSelection(false)
//        , m_bMouseInOverlayButton(false)
        , m_bPopupActive(false)
        , m_bAutoComplete(false)
        , m_bAutoCompleteCaseSensitive(false)
        , m_bChangedByMenu(false)
        , m_bCustomRenderer(false)
        , m_bUserSelectEntry(false)
        , m_nTextCol(gtk_combo_box_get_entry_text_column(pComboBox))
        , m_nIdCol(gtk_combo_box_get_id_column(pComboBox))
//        , m_nToggleFocusInSignalId(0)
//        , m_nToggleFocusOutSignalId(0)
//        , m_nRowActivatedSignalId(g_signal_connect(m_pTreeView, "row-activated", G_CALLBACK(signalRowActivated), this))
        , m_nChangedSignalId(g_signal_connect(m_pComboBox, "changed", G_CALLBACK(signalChanged), this))
        , m_nPopupShownSignalId(g_signal_connect(m_pComboBox, "notify::popup-shown", G_CALLBACK(signalPopupToggled), this))
        , m_nAutoCompleteIdleId(0)
//        , m_nNonCustomLineHeight(-1)
        , m_nPrePopupCursorPos(-1)
        , m_nMRUCount(0)
        , m_nMaxMRUCount(0)
    {
        for (GtkWidget* pChild = gtk_widget_get_first_child(GTK_WIDGET(m_pComboBox));
             pChild; pChild = gtk_widget_get_next_sibling(pChild))
        {
            if (GTK_IS_POPOVER(pChild))
            {
                m_pMenuWindow = pChild;
                break;
            }
        }
        SAL_WARN_IF(!m_pMenuWindow, "vcl.gtk", "GtkInstanceComboBox: couldn't find popup menu");

        bool bHasEntry = gtk_combo_box_get_has_entry(m_pComboBox);

        bool bFindButtonTextRenderer = !bHasEntry;
        GtkCellLayout* pCellLayout = GTK_CELL_LAYOUT(m_pComboBox);
        GList* cells = gtk_cell_layout_get_cells(pCellLayout);
        guint i = g_list_length(cells) - 1;;
        // reorder the cell renderers
        for (GList* pRenderer = g_list_first(cells); pRenderer; pRenderer = g_list_next(pRenderer))
        {
            GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
            gtk_cell_layout_reorder(pCellLayout, pCellRenderer, i--);
            if (bFindButtonTextRenderer)
            {
                m_pButtonTextRenderer = pCellRenderer;
                bFindButtonTextRenderer = false;
            }
        }

        if (bHasEntry)
        {
            m_bAutoComplete = true;
            m_nEntryInsertTextSignalId = g_signal_connect(m_pEditable, "insert-text", G_CALLBACK(signalEntryInsertText), this);
            m_nEntryActivateSignalId = g_signal_connect(m_pEntry, "activate", G_CALLBACK(signalEntryActivate), this);
            m_pEntryFocusController = GTK_EVENT_CONTROLLER(gtk_event_controller_focus_new());
            m_nEntryFocusInSignalId = g_signal_connect(m_pEntryFocusController, "enter", G_CALLBACK(signalEntryFocusIn), this);
            m_nEntryFocusOutSignalId = g_signal_connect(m_pEntryFocusController, "leave", G_CALLBACK(signalEntryFocusOut), this);
            gtk_widget_add_controller(m_pEntry, m_pEntryFocusController);
            m_pEntryKeyController = GTK_EVENT_CONTROLLER(gtk_event_controller_key_new());
            m_nEntryKeyPressEventSignalId = g_signal_connect(m_pEntryKeyController, "key-pressed", G_CALLBACK(signalEntryKeyPress), this);
            gtk_widget_add_controller(m_pEntry, m_pEntryKeyController);
            m_nKeyPressEventSignalId = 0;
            m_pKeyController = nullptr;
        }
        else
        {
            m_nEntryInsertTextSignalId = 0;
            m_nEntryActivateSignalId = 0;
            m_pEntryFocusController = nullptr;
            m_nEntryFocusInSignalId = 0;
            m_nEntryFocusOutSignalId = 0;
            m_pEntryKeyController = nullptr;
            m_nEntryKeyPressEventSignalId = 0;
            m_pKeyController = GTK_EVENT_CONTROLLER(gtk_event_controller_key_new());
            m_nKeyPressEventSignalId = g_signal_connect(m_pKeyController, "key-pressed", G_CALLBACK(signalKeyPress), this);
            gtk_widget_add_controller(GTK_WIDGET(m_pComboBox), m_pKeyController);
        }

//        g_signal_connect(m_pMenuWindow, "grab-broken-event", G_CALLBACK(signalGrabBroken), this);
//        g_signal_connect(m_pMenuWindow, "button-press-event", G_CALLBACK(signalButtonPress), this);
//        g_signal_connect(m_pMenuWindow, "motion-notify-event", G_CALLBACK(signalMotion), this);

        // support typeahead for the menu itself, typing into the menu will
        // select via the vcl selection engine, a matching entry.
        if (m_pMenuWindow)
        {
            m_pMenuKeyController = GTK_EVENT_CONTROLLER(gtk_event_controller_key_new());
            g_signal_connect(m_pMenuKeyController, "key-pressed", G_CALLBACK(signalKeyPress), this);
            gtk_widget_add_controller(m_pMenuWindow, m_pMenuKeyController);
        }
        else
            m_pMenuKeyController = nullptr;
#if 0
        g_signal_connect(m_pOverlay, "get-child-position", G_CALLBACK(signalGetChildPosition), this);
        gtk_overlay_add_overlay(m_pOverlay, GTK_WIDGET(m_pOverlayButton));
        g_signal_connect(m_pOverlayButton, "leave-notify-event", G_CALLBACK(signalOverlayButtonCrossing), this);
        g_signal_connect(m_pOverlayButton, "enter-notify-event", G_CALLBACK(signalOverlayButtonCrossing), this);
#endif
    }

    virtual int get_active() const override
    {
        int nActive = get_active_including_mru();
        if (nActive == -1)
            return -1;

        if (m_nMRUCount)
        {
            if (nActive < m_nMRUCount)
                nActive = find_text(get_text_including_mru(nActive));
            else
                nActive -= (m_nMRUCount + 1);
        }

        return nActive;
    }

    virtual OUString get_active_id() const override
    {
        int nActive = get_active();
        return nActive != -1 ? get_id(nActive) : OUString();
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        set_active(find_id(rStr));
        m_bChangedByMenu = false;
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        if (m_pButtonTextRenderer)
        {
            // tweak the cell render to get a narrower size to stick
            if (nWidth != -1)
            {
                // this bit isn't great, I really want to be able to ellipse the text in the comboboxtext itself and let
                // the popup menu render them in full, in the interim ellipse both of them
                g_object_set(G_OBJECT(m_pButtonTextRenderer), "ellipsize", PANGO_ELLIPSIZE_MIDDLE, nullptr);

                // to find out how much of the width of the combobox belongs to the cell, set
                // the cell and widget to the min cell width and see what the difference is
                int min;
                gtk_cell_renderer_get_preferred_width(m_pButtonTextRenderer, m_pWidget, &min, nullptr);
                gtk_cell_renderer_set_fixed_size(m_pButtonTextRenderer, min, -1);
                gtk_widget_set_size_request(m_pWidget, min, -1);
                int nNonCellWidth = get_preferred_size().Width() - min;

                int nCellWidth = nWidth - nNonCellWidth;
                if (nCellWidth >= 0)
                {
                    // now set the cell to the max width which it can be within the
                    // requested widget width
                    gtk_cell_renderer_set_fixed_size(m_pButtonTextRenderer, nWidth - nNonCellWidth, -1);
                }
            }
            else
            {
                g_object_set(G_OBJECT(m_pButtonTextRenderer), "ellipsize", PANGO_ELLIPSIZE_NONE, nullptr);
                gtk_cell_renderer_set_fixed_size(m_pButtonTextRenderer, -1, -1);
            }
        }

        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual void set_active(int pos) override
    {
        set_active_including_mru(include_mru(pos), false);
    }

    virtual OUString get_active_text() const override
    {
        if (m_pEditable)
        {
            const gchar* pText = gtk_editable_get_text(m_pEditable);
            return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        }

        int nActive = get_active();
        if (nActive == -1)
           return OUString();

        return get_text(nActive);
    }

    virtual OUString get_text(int pos) const override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        return get_text_including_mru(pos);
    }

    virtual OUString get_id(int pos) const override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        return get_id_including_mru(pos);
    }

    virtual void set_id(int pos, const OUString& rId) override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        set_id_including_mru(pos, rId);
    }

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting) override
    {
        freeze();

        int nInsertionPoint;
        if (!bKeepExisting)
        {
            clear();
            nInsertionPoint = 0;
        }
        else
            nInsertionPoint = get_count();

        GtkTreeIter iter;
        // tdf#125241 inserting backwards is faster
        for (auto aI = rItems.rbegin(); aI != rItems.rend(); ++aI)
        {
            const auto& rItem = *aI;
            insert_row(GTK_LIST_STORE(m_pTreeModel), iter, nInsertionPoint, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
                       rItem.sString, rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
        }

        thaw();
    }

    virtual void remove(int pos) override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        remove_including_mru(pos);
    }

    virtual void insert(int pos, const OUString& rText, const OUString* pId, const OUString* pIconName, VirtualDevice* pImageSurface) override
    {
        insert_including_mru(include_mru(pos), rText, pId, pIconName, pImageSurface);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        pos = pos == -1 ? get_count() : pos;
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        insert_separator_including_mru(pos, rId);
    }

    virtual int get_count() const override
    {
        int nCount = get_count_including_mru();
        if (m_nMRUCount)
            nCount -= (m_nMRUCount + 1);
        return nCount;
    }

    virtual int find_text(const OUString& rStr) const override
    {
        int nPos = find_text_including_mru(rStr, false);
        if (nPos != -1 && m_nMRUCount)
            nPos -= (m_nMRUCount + 1);
        return nPos;
    }

    virtual int find_id(const OUString& rId) const override
    {
        int nPos = find_id_including_mru(rId, false);
        if (nPos != -1 && m_nMRUCount)
            nPos -= (m_nMRUCount + 1);
        return nPos;
    }

    virtual void clear() override
    {
        do_clear();
    }

    virtual void make_sorted() override
    {
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetUILanguageTag().getLocale()));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_set_sort_column_id(pSortable, m_nTextCol, GTK_SORT_ASCENDING);
        gtk_tree_sortable_set_sort_func(pSortable, m_nTextCol, default_sort_func, m_xSorter.get(), nullptr);
    }

    virtual bool has_entry() const override
    {
        return gtk_combo_box_get_has_entry(m_pComboBox);
    }

    virtual void set_entry_message_type(weld::EntryMessageType eType) override
    {
        assert(m_pEntry);
        ::set_entry_message_type(GTK_ENTRY(m_pEntry), eType);
    }

    virtual void set_entry_text(const OUString& rText) override
    {
        assert(m_pEditable);
        disable_notify_events();
        gtk_editable_set_text(m_pEditable, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
        enable_notify_events();
    }

    virtual void set_entry_width_chars(int nChars) override
    {
        assert(m_pEditable);
        disable_notify_events();
        gtk_editable_set_width_chars(m_pEditable, nChars);
        gtk_editable_set_max_width_chars(m_pEditable, nChars);
        enable_notify_events();
    }

    virtual void set_entry_max_length(int nChars) override
    {
        assert(m_pEntry);
        disable_notify_events();
        gtk_entry_set_max_length(GTK_ENTRY(m_pEntry), nChars);
        enable_notify_events();
    }

    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        assert(m_pEditable);
        disable_notify_events();
        gtk_editable_select_region(m_pEditable, nStartPos, nEndPos);
        enable_notify_events();
    }

    virtual bool get_entry_selection_bounds(int& rStartPos, int &rEndPos) override
    {
        assert(m_pEditable);
        return gtk_editable_get_selection_bounds(m_pEditable, &rStartPos, &rEndPos);
    }

    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive) override
    {
        m_bAutoComplete = bEnable;
        m_bAutoCompleteCaseSensitive = bCaseSensitive;
    }

    virtual void set_entry_placeholder_text(const OUString& rText) override
    {
        assert(m_pEntry);
        gtk_entry_set_placeholder_text(GTK_ENTRY(m_pEntry), rText.toUtf8().getStr());
    }

    virtual void set_entry_editable(bool bEditable) override
    {
        assert(m_pEditable);
        gtk_editable_set_editable(m_pEditable, bEditable);
    }

    virtual void cut_entry_clipboard() override
    {
        assert(m_pEntry);
        gtk_widget_activate_action(m_pEntry, "cut.clipboard", nullptr);
    }

    virtual void copy_entry_clipboard() override
    {
        assert(m_pEntry);
        gtk_widget_activate_action(m_pEntry, "copy.clipboard", nullptr);
    }

    virtual void paste_entry_clipboard() override
    {
        assert(m_pEntry);
        gtk_widget_activate_action(m_pEntry, "paste.clipboard", nullptr);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_aCustomFont.use_custom_font(&rFont, u"combobox");
    }

    virtual vcl::Font get_font() override
    {
        if (const vcl::Font* pFont = m_aCustomFont.get_custom_font())
            return *pFont;
        return GtkInstanceWidget::get_font();
    }

    virtual void set_entry_font(const vcl::Font& rFont) override
    {
        m_xEntryFont = rFont;
        assert(m_pEntry);
        PangoAttrList* pOrigList = gtk_entry_get_attributes(GTK_ENTRY(m_pEntry));
        PangoAttrList* pAttrList = pOrigList ? pango_attr_list_copy(pOrigList) : pango_attr_list_new();
        update_attr_list(pAttrList, rFont);
        gtk_entry_set_attributes(GTK_ENTRY(m_pEntry), pAttrList);
        pango_attr_list_unref(pAttrList);
    }

    virtual vcl::Font get_entry_font() override
    {
        if (m_xEntryFont)
            return *m_xEntryFont;
        assert(m_pEntry);
        PangoContext* pContext = gtk_widget_get_pango_context(m_pEntry);
        return pango_to_vcl(pango_context_get_font_description(pContext),
                            Application::GetSettings().GetUILanguageTag().getLocale());
    }

    virtual void disable_notify_events() override
    {
        if (m_pEditable)
        {
            g_signal_handler_block(m_pEditable, m_nEntryInsertTextSignalId);
            g_signal_handler_block(m_pEntry, m_nEntryActivateSignalId);
            g_signal_handler_block(m_pEntryFocusController, m_nEntryFocusInSignalId);
            g_signal_handler_block(m_pEntryFocusController, m_nEntryFocusOutSignalId);
            g_signal_handler_block(m_pEntryKeyController, m_nEntryKeyPressEventSignalId);
        }
        else
            g_signal_handler_block(m_pKeyController, m_nKeyPressEventSignalId);

//        if (m_nToggleFocusInSignalId)
//            g_signal_handler_block(m_pToggleButton, m_nToggleFocusInSignalId);
//        if (m_nToggleFocusOutSignalId)
//            g_signal_handler_block(m_pToggleButton, m_nToggleFocusOutSignalId);
//        g_signal_handler_block(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_block(m_pComboBox, m_nPopupShownSignalId);
        g_signal_handler_block(m_pComboBox, m_nChangedSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pComboBox, m_nChangedSignalId);
        g_signal_handler_unblock(m_pComboBox, m_nPopupShownSignalId);
//        g_signal_handler_unblock(m_pTreeView, m_nRowActivatedSignalId);
//        if (m_nToggleFocusInSignalId)
//            g_signal_handler_unblock(m_pToggleButton, m_nToggleFocusInSignalId);
//        if (m_nToggleFocusOutSignalId)
//            g_signal_handler_unblock(m_pToggleButton, m_nToggleFocusOutSignalId);
        if (m_pEditable)
        {
            g_signal_handler_unblock(m_pEntry, m_nEntryActivateSignalId);
            g_signal_handler_unblock(m_pEntryFocusController, m_nEntryFocusInSignalId);
            g_signal_handler_unblock(m_pEntryFocusController, m_nEntryFocusOutSignalId);
            g_signal_handler_unblock(m_pEntryKeyController, m_nEntryKeyPressEventSignalId);
            g_signal_handler_unblock(m_pEditable, m_nEntryInsertTextSignalId);
        }
        else
            g_signal_handler_unblock(m_pKeyController, m_nKeyPressEventSignalId);
    }

    virtual void freeze() override
    {
        disable_notify_events();
        bool bIsFirstFreeze = IsFirstFreeze();
        GtkInstanceWidget::freeze();
        if (bIsFirstFreeze)
        {
            g_object_ref(m_pTreeModel);
//            gtk_tree_view_set_model(m_pTreeView, nullptr);
            g_object_freeze_notify(G_OBJECT(m_pTreeModel));
            if (m_xSorter)
            {
                GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
                gtk_tree_sortable_set_sort_column_id(pSortable, GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
            }
        }
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
        if (IsLastThaw())
        {
            if (m_xSorter)
            {
                GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
                gtk_tree_sortable_set_sort_column_id(pSortable, m_nTextCol, GTK_SORT_ASCENDING);
            }
            g_object_thaw_notify(G_OBJECT(m_pTreeModel));
//            gtk_tree_view_set_model(m_pTreeView, m_pTreeModel);
            g_object_unref(m_pTreeModel);
        }
        GtkInstanceWidget::thaw();
        enable_notify_events();
    }

    virtual bool get_popup_shown() const override
    {
        return m_bPopupActive;
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
//        if (!m_nToggleFocusInSignalId)
//            m_nToggleFocusInSignalId = g_signal_connect_after(m_pToggleButton, "focus-in-event", G_CALLBACK(signalFocusIn), this);
        GtkInstanceWidget::connect_focus_in(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
//        if (!m_nToggleFocusOutSignalId)
//            m_nToggleFocusOutSignalId = g_signal_connect_after(m_pToggleButton, "focus-out-event", G_CALLBACK(signalFocusOut), this);
        GtkInstanceWidget::connect_focus_out(rLink);
    }

    virtual void grab_focus() override
    {
        if (has_focus())
            return;
        if (m_pEntry)
            gtk_widget_grab_focus(m_pEntry);
        else
        {
//            gtk_widget_grab_focus(m_pToggleButton);
            gtk_widget_grab_focus(GTK_WIDGET(m_pComboBox));
        }
    }

    virtual bool has_focus() const override
    {
        if (m_pEntry && gtk_widget_has_focus(m_pEntry))
            return true;

//        if (gtk_widget_has_focus(m_pToggleButton))
//            return true;

#if 0
        if (gtk_widget_get_visible(GTK_WIDGET(m_pMenuWindow)))
        {
            if (gtk_widget_has_focus(GTK_WIDGET(m_pOverlayButton)) || gtk_widget_has_focus(GTK_WIDGET(m_pTreeView)))
                return true;
        }
#endif

        return GtkInstanceWidget::has_focus();
    }

    virtual bool changed_by_direct_pick() const override
    {
        return m_bChangedByMenu;
    }

    virtual void set_custom_renderer(bool bOn) override
    {
        if (bOn == m_bCustomRenderer)
            return;
#if 0
        GList* pColumns = gtk_tree_view_get_columns(m_pTreeView);
        // keep the original height around for optimal popup height calculation
        m_nNonCustomLineHeight = bOn ? ::get_height_row(m_pTreeView, pColumns) : -1;
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pColumns->data);
        gtk_cell_layout_clear(GTK_CELL_LAYOUT(pColumn));
        if (bOn)
        {
            GtkCellRenderer *pRenderer = custom_cell_renderer_new();
            GValue value = G_VALUE_INIT;
            g_value_init(&value, G_TYPE_POINTER);
            g_value_set_pointer(&value, static_cast<gpointer>(this));
            g_object_set_property(G_OBJECT(pRenderer), "instance", &value);
            gtk_tree_view_column_pack_start(pColumn, pRenderer, true);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "text", m_nTextCol);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "id", m_nIdCol);
        }
        else
        {
            GtkCellRenderer *pRenderer = gtk_cell_renderer_text_new();
            gtk_tree_view_column_pack_start(pColumn, pRenderer, true);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "text", m_nTextCol);
        }
        g_list_free(pColumns);
        m_bCustomRenderer = bOn;
#endif
    }

    void call_signal_custom_render(VirtualDevice& rOutput, const tools::Rectangle& rRect, bool bSelected, const OUString& rId)
    {
        signal_custom_render(rOutput, rRect, bSelected, rId);
    }

    Size call_signal_custom_get_size(VirtualDevice& rOutput)
    {
        return signal_custom_get_size(rOutput);
    }

    VclPtr<VirtualDevice> create_render_virtual_device() const override
    {
        return create_virtual_device();
    }

    virtual void set_item_menu(const OString& rIdent, weld::Menu* pMenu) override
    {
#if 0
        m_xCustomMenuButtonHelper.reset();
        GtkInstanceMenu* pPopoverWidget = dynamic_cast<GtkInstanceMenu*>(pMenu);
        GtkWidget* pMenuWidget = GTK_WIDGET(pPopoverWidget ? pPopoverWidget->getMenu() : nullptr);
        gtk_menu_button_set_popup(m_pOverlayButton, pMenuWidget);
        gtk_widget_set_visible(GTK_WIDGET(m_pOverlayButton), pMenuWidget != nullptr);
        gtk_widget_queue_resize_no_redraw(GTK_WIDGET(m_pOverlayButton)); // force location recalc
        if (pMenuWidget)
            m_xCustomMenuButtonHelper.reset(new CustomRenderMenuButtonHelper(GTK_MENU(pMenuWidget), GTK_TOGGLE_BUTTON(m_pToggleButton)));
        m_sMenuButtonRow = OUString::fromUtf8(rIdent);
#else
        (void)rIdent; (void)pMenu;
#endif
    }

    OUString get_mru_entries() const override
    {
        const sal_Unicode cSep = ';';

        OUStringBuffer aEntries;
        for (sal_Int32 n = 0; n < m_nMRUCount; n++)
        {
            aEntries.append(get_text_including_mru(n));
            if (n < m_nMRUCount - 1)
                aEntries.append(cSep);
        }
        return aEntries.makeStringAndClear();
    }

    virtual void set_mru_entries(const OUString& rEntries) override
    {
        const sal_Unicode cSep = ';';

        // Remove old MRU entries
        for (sal_Int32 n = m_nMRUCount; n;)
            remove_including_mru(--n);

        sal_Int32 nMRUCount = 0;
        sal_Int32 nIndex = 0;
        do
        {
            OUString aEntry = rEntries.getToken(0, cSep, nIndex);
            // Accept only existing entries
            int nPos = find_text(aEntry);
            if (nPos != -1)
            {
                OUString sId = get_id(nPos);
                insert_including_mru(0, aEntry, &sId, nullptr, nullptr);
                ++nMRUCount;
            }
        }
        while (nIndex >= 0);

        if (nMRUCount && !m_nMRUCount)
            insert_separator_including_mru(nMRUCount, "separator");
        else if (!nMRUCount && m_nMRUCount)
            remove_including_mru(m_nMRUCount);  // remove separator

        m_nMRUCount = nMRUCount;
    }

    int get_menu_button_width() const override
    {
#if 0
        bool bVisible = gtk_widget_get_visible(GTK_WIDGET(m_pOverlayButton));
        if (!bVisible)
            gtk_widget_set_visible(GTK_WIDGET(m_pOverlayButton), true);
        gint nWidth;
        gtk_widget_get_preferred_width(GTK_WIDGET(m_pOverlayButton), &nWidth, nullptr);
        if (!bVisible)
            gtk_widget_set_visible(GTK_WIDGET(m_pOverlayButton), false);
        return nWidth;
#else
        return 0;
#endif
    }

    virtual ~GtkInstanceComboBox() override
    {
//        m_xCustomMenuButtonHelper.reset();
        do_clear();
        if (m_nAutoCompleteIdleId)
            g_source_remove(m_nAutoCompleteIdleId);
        if (m_pEditable)
        {
            g_signal_handler_disconnect(m_pEditable, m_nEntryInsertTextSignalId);
            g_signal_handler_disconnect(m_pEntry, m_nEntryActivateSignalId);
            g_signal_handler_disconnect(m_pEntryFocusController, m_nEntryFocusInSignalId);
            g_signal_handler_disconnect(m_pEntryFocusController, m_nEntryFocusOutSignalId);
            g_signal_handler_disconnect(m_pEntryKeyController, m_nEntryKeyPressEventSignalId);
        }
        else
            g_signal_handler_disconnect(m_pKeyController, m_nKeyPressEventSignalId);
//        if (m_nToggleFocusInSignalId)
//            g_signal_handler_disconnect(m_pToggleButton, m_nToggleFocusInSignalId);
//        if (m_nToggleFocusOutSignalId)
//            g_signal_handler_disconnect(m_pToggleButton, m_nToggleFocusOutSignalId);
//        g_signal_handler_disconnect(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_disconnect(m_pComboBox, m_nPopupShownSignalId);
        g_signal_handler_disconnect(m_pComboBox, m_nChangedSignalId);

//        gtk_tree_view_set_model(m_pTreeView, nullptr);

    }
};

#else

class GtkInstanceComboBox : public GtkInstanceContainer, public vcl::ISearchableStringList, public virtual weld::ComboBox
{
private:
    GtkBuilder* m_pComboBuilder;
    GtkComboBox* m_pComboBox;
    GtkOverlay* m_pOverlay;
    GtkTreeView* m_pTreeView;
    GtkMenuButton* m_pOverlayButton; // button that the StyleDropdown uses on an active row
    GtkWindow* m_pMenuWindow;
    GtkTreeModel* m_pTreeModel;
    GtkCellRenderer* m_pButtonTextRenderer;
    GtkCellRenderer* m_pMenuTextRenderer;
    GtkWidget* m_pToggleButton;
    GtkWidget* m_pEntry;
    GtkCellView* m_pCellView;
    WidgetFont m_aCustomFont;
    std::unique_ptr<CustomRenderMenuButtonHelper> m_xCustomMenuButtonHelper;
    std::optional<vcl::Font> m_xEntryFont;
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    vcl::QuickSelectionEngine m_aQuickSelectionEngine;
    std::vector<std::unique_ptr<GtkTreeRowReference, GtkTreeRowReferenceDeleter>> m_aSeparatorRows;
    OUString m_sMenuButtonRow;
    bool m_bHoverSelection;
    bool m_bMouseInOverlayButton;
    bool m_bPopupActive;
    bool m_bAutoComplete;
    bool m_bAutoCompleteCaseSensitive;
    bool m_bChangedByMenu;
    bool m_bCustomRenderer;
    bool m_bActivateCalled;
    gint m_nTextCol;
    gint m_nIdCol;
    gulong m_nToggleFocusInSignalId;
    gulong m_nToggleFocusOutSignalId;
    gulong m_nRowActivatedSignalId;
    gulong m_nChangedSignalId;
    gulong m_nPopupShownSignalId;
    gulong m_nKeyPressEventSignalId;
    gulong m_nEntryInsertTextSignalId;
    gulong m_nEntryActivateSignalId;
    gulong m_nEntryFocusInSignalId;
    gulong m_nEntryFocusOutSignalId;
    gulong m_nEntryKeyPressEventSignalId;
    guint m_nAutoCompleteIdleId;
    gint m_nNonCustomLineHeight;
    gint m_nPrePopupCursorPos;
    int m_nMRUCount;
    int m_nMaxMRUCount;

    static gboolean idleAutoComplete(gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->auto_complete();
        return false;
    }

    void auto_complete()
    {
        m_nAutoCompleteIdleId = 0;
        OUString aStartText = get_active_text();
        int nStartPos, nEndPos;
        get_entry_selection_bounds(nStartPos, nEndPos);
        int nMaxSelection = std::max(nStartPos, nEndPos);
        if (nMaxSelection != aStartText.getLength())
            return;

        disable_notify_events();
        int nActive = get_active();
        int nStart = nActive;

        if (nStart == -1)
            nStart = 0;

        int nPos = -1;

        int nZeroRow = 0;
        if (m_nMRUCount)
            nZeroRow += (m_nMRUCount + 1);

        if (!m_bAutoCompleteCaseSensitive)
        {
            // Try match case insensitive from current position
            nPos = starts_with(m_pTreeModel, aStartText, 0, nStart, false);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case insensitive, but from start
                nPos = starts_with(m_pTreeModel, aStartText, 0, nZeroRow, false);
            }
        }

        if (nPos == -1)
        {
            // Try match case sensitive from current position
            nPos = starts_with(m_pTreeModel, aStartText, 0, nStart, true);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case sensitive, but from start
                nPos = starts_with(m_pTreeModel, aStartText, 0, nZeroRow, true);
            }
        }

        if (nPos != -1)
        {
            OUString aText = get_text_including_mru(nPos);
            if (aText != aStartText)
            {
                SolarMutexGuard aGuard;
                set_active_including_mru(nPos, true);
            }
            select_entry_region(aText.getLength(), aStartText.getLength());
        }
        enable_notify_events();
    }

    static void signalEntryInsertText(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength,
                                      gint* position, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_entry_insert_text(pEntry, pNewText, nNewTextLength, position);
    }

    void signal_entry_insert_text(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength, gint* position)
    {
        // first filter inserted text
        if (m_aEntryInsertTextHdl.IsSet())
        {
            OUString sText(pNewText, nNewTextLength, RTL_TEXTENCODING_UTF8);
            const bool bContinue = m_aEntryInsertTextHdl.Call(sText);
            if (bContinue && !sText.isEmpty())
            {
                OString sFinalText(OUStringToOString(sText, RTL_TEXTENCODING_UTF8));
                g_signal_handlers_block_by_func(pEntry, reinterpret_cast<gpointer>(signalEntryInsertText), this);
                gtk_editable_insert_text(GTK_EDITABLE(pEntry), sFinalText.getStr(), sFinalText.getLength(), position);
                g_signal_handlers_unblock_by_func(pEntry, reinterpret_cast<gpointer>(signalEntryInsertText), this);
            }
            g_signal_stop_emission_by_name(pEntry, "insert-text");
        }
        if (m_bAutoComplete)
        {
            // now check for autocompletes
            if (m_nAutoCompleteIdleId)
                g_source_remove(m_nAutoCompleteIdleId);
            m_nAutoCompleteIdleId = g_idle_add(idleAutoComplete, this);
        }
    }

    static void signalChanged(GtkEntry*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        SolarMutexGuard aGuard;
        pThis->fire_signal_changed();
    }

    void fire_signal_changed()
    {
        signal_changed();
        m_bChangedByMenu = false;
    }

    static void signalPopupToggled(GtkToggleButton* /*pToggleButton*/, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_popup_toggled();
    }

    int get_popup_height(gint& rPopupWidth)
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

        int nMaxRows = rSettings.GetListBoxMaximumLineCount();
        bool bAddScrollWidth = false;
        int nRows = get_count_including_mru();
        if (nMaxRows < nRows)
        {
            nRows = nMaxRows;
            bAddScrollWidth = true;
        }

        GList* pColumns = gtk_tree_view_get_columns(m_pTreeView);
        gint nRowHeight = get_height_row(m_pTreeView, pColumns);
        g_list_free(pColumns);

        gint nSeparatorHeight = get_height_row_separator(m_pTreeView);
        gint nHeight = get_height_rows(nRowHeight, nSeparatorHeight, nRows);

        // if we're using a custom renderer, limit the height to the height nMaxRows would be
        // for a normal renderer, and then round down to how many custom rows fit in that
        // space
        if (m_nNonCustomLineHeight != -1 && nRowHeight)
        {
            gint nNormalHeight = get_height_rows(m_nNonCustomLineHeight, nSeparatorHeight, nMaxRows);
            if (nHeight > nNormalHeight)
            {
                gint nRowsOnly = nNormalHeight - get_height_rows(0, nSeparatorHeight, nMaxRows);
                gint nCustomRows = (nRowsOnly + (nRowHeight - 1)) / nRowHeight;
                nHeight = get_height_rows(nRowHeight, nSeparatorHeight, nCustomRows);
            }
        }

        if (bAddScrollWidth)
            rPopupWidth += rSettings.GetScrollBarSize();

        return nHeight;
    }

    void menu_toggled()
    {
        if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pToggleButton)))
        {
            if (m_bHoverSelection)
            {
                // turn hover selection back off until mouse is moved again
                // *after* menu is shown again
                gtk_tree_view_set_hover_selection(m_pTreeView, false);
                m_bHoverSelection = false;
            }

            bool bHadFocus = gtk_window_has_toplevel_focus(m_pMenuWindow);

            do_ungrab(GTK_WIDGET(m_pMenuWindow));

            gtk_widget_hide(GTK_WIDGET(m_pMenuWindow));

            GdkSurface* pSurface = widget_get_surface(GTK_WIDGET(m_pMenuWindow));
            g_object_set_data(G_OBJECT(pSurface), "g-lo-InstancePopup", GINT_TO_POINTER(false));

            // so gdk_window_move_to_rect will work again the next time
            gtk_widget_unrealize(GTK_WIDGET(m_pMenuWindow));

            gtk_widget_set_size_request(GTK_WIDGET(m_pMenuWindow), -1, -1);

            if (!m_bActivateCalled)
                tree_view_set_cursor(m_nPrePopupCursorPos);

            // undo show_menu tooltip blocking
            GtkWidget* pParent = widget_get_toplevel(m_pToggleButton);
            GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
            if (pFrame)
                pFrame->UnblockTooltip();

            if (bHadFocus)
            {
                GdkSurface* pParentSurface = pParent ? widget_get_surface(pParent) : nullptr;
                void* pParentIsPopover = pParentSurface ? g_object_get_data(G_OBJECT(pParentSurface), "g-lo-InstancePopup") : nullptr;
                if (pParentIsPopover)
                    do_grab(m_pToggleButton);
                gtk_widget_grab_focus(m_pToggleButton);
            }
        }
        else
        {
            GtkWidget* pComboBox = GTK_WIDGET(getContainer());

            gint nComboWidth = gtk_widget_get_allocated_width(pComboBox);
            GtkRequisition size;
            gtk_widget_get_preferred_size(GTK_WIDGET(m_pMenuWindow), nullptr, &size);

            gint nPopupWidth = size.width;
            gint nPopupHeight = get_popup_height(nPopupWidth);
            nPopupWidth = std::max(nPopupWidth, nComboWidth);

            gtk_widget_set_size_request(GTK_WIDGET(m_pMenuWindow), nPopupWidth, nPopupHeight);

            m_nPrePopupCursorPos = get_active();

            m_bActivateCalled = false;

            // if we are in mru mode always start with the cursor at the top of the menu
            if (m_nMaxMRUCount)
                tree_view_set_cursor(0);

            GdkRectangle aAnchor {0, 0, gtk_widget_get_allocated_width(pComboBox), gtk_widget_get_allocated_height(pComboBox) };
            show_menu(pComboBox, m_pMenuWindow, aAnchor, weld::Placement::Under);
            GdkSurface* pSurface = widget_get_surface(GTK_WIDGET(m_pMenuWindow));
            g_object_set_data(G_OBJECT(pSurface), "g-lo-InstancePopup", GINT_TO_POINTER(true));
        }
    }

    virtual void signal_popup_toggled() override
    {
        m_aQuickSelectionEngine.Reset();

        menu_toggled();

        bool bIsShown = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pToggleButton));
        if (m_bPopupActive == bIsShown)
            return;

        m_bPopupActive = bIsShown;
        ComboBox::signal_popup_toggled();
        if (!m_bPopupActive && m_pEntry)
        {
            disable_notify_events();
            //restore focus to the GtkEntry when the popup is gone, which
            //is what the vcl case does, to ease the transition a little
            gtk_widget_grab_focus(m_pEntry);
            enable_notify_events();
        }
    }

    static gboolean signalEntryFocusIn(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_entry_focus_in();
        return false;
    }

    void signal_entry_focus_in()
    {
        signal_focus_in();
    }

    static gboolean signalEntryFocusOut(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_entry_focus_out();
        return false;
    }

    void signal_entry_focus_out()
    {
        // if we have an untidy selection on losing focus remove the selection
        int nStartPos, nEndPos;
        if (get_entry_selection_bounds(nStartPos, nEndPos))
        {
            int nMin = std::min(nStartPos, nEndPos);
            int nMax = std::max(nStartPos, nEndPos);
            if (nMin != 0 || nMax != get_active_text().getLength())
                select_entry_region(0, 0);
        }
        signal_focus_out();
    }

    static void signalEntryActivate(GtkEntry*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_entry_activate();
    }

    void signal_entry_activate()
    {
        if (m_aEntryActivateHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            if (m_aEntryActivateHdl.Call(*this))
                g_signal_stop_emission_by_name(m_pEntry, "activate");
        }
        update_mru();
    }

    OUString get(int pos, int col) const
    {
        OUString sRet;
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
        {
            gchar* pStr;
            gtk_tree_model_get(m_pTreeModel, &iter, col, &pStr, -1);
            sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

    void set(int pos, int col, std::u16string_view rText)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
        {
            OString aStr(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
            gtk_list_store_set(GTK_LIST_STORE(m_pTreeModel), &iter, col, aStr.getStr(), -1);
        }
    }

    int find(std::u16string_view rStr, int col, bool bSearchMRUArea) const
    {
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter_first(m_pTreeModel, &iter))
            return -1;

        int nRet = 0;

        if (!bSearchMRUArea && m_nMRUCount)
        {
            if (!gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, m_nMRUCount + 1))
                return -1;
            nRet += (m_nMRUCount + 1);
        }

        OString aStr(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
        do
        {
            gchar* pStr;
            gtk_tree_model_get(m_pTreeModel, &iter, col, &pStr, -1);
            const bool bEqual = g_strcmp0(pStr, aStr.getStr()) == 0;
            g_free(pStr);
            if (bEqual)
                return nRet;
            ++nRet;
        } while (gtk_tree_model_iter_next(m_pTreeModel, &iter));

        return -1;
    }

    bool separator_function(const GtkTreePath* path)
    {
        return ::separator_function(path, m_aSeparatorRows);
    }

    bool separator_function(int pos)
    {
        GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
        bool bRet = separator_function(path);
        gtk_tree_path_free(path);
        return bRet;
    }

    static gboolean separatorFunction(GtkTreeModel* pTreeModel, GtkTreeIter* pIter, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        GtkTreePath* path = gtk_tree_model_get_path(pTreeModel, pIter);
        bool bRet = pThis->separator_function(path);
        gtk_tree_path_free(path);
        return bRet;
    }

    // https://gitlab.gnome.org/GNOME/gtk/issues/310
    //
    // in the absence of a built-in solution
    // a) support typeahead for the case where there is no entry widget, typing ahead
    // into the button itself will select via the vcl selection engine, a matching
    // entry
    static gboolean signalKeyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        return pThis->signal_key_press(pEvent);
    }

    // tdf#131076 we want return in a ComboBox to act like return in a
    // GtkEntry and activate the default dialog/assistant button
    bool combobox_activate()
    {
        GtkWidget *pComboBox = GTK_WIDGET(m_pToggleButton);
        GtkWidget *pToplevel = widget_get_toplevel(pComboBox);
        GtkWindow *pWindow = GTK_WINDOW(pToplevel);
        if (!pWindow)
            return false;
        if (!GTK_IS_DIALOG(pWindow) && !GTK_IS_ASSISTANT(pWindow))
            return false;
        bool bDone = false;
        GtkWidget *pDefaultWidget = gtk_window_get_default_widget(pWindow);
        if (pDefaultWidget && pDefaultWidget != m_pToggleButton && gtk_widget_get_sensitive(pDefaultWidget))
            bDone = gtk_widget_activate(pDefaultWidget);
        return bDone;
    }

    static gboolean signalEntryKeyPress(GtkEntry* pEntry, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        LocalizeDecimalSeparator(pEvent->keyval);
        if (signalEntryInsertSpecialCharKeyPress(pEntry, pEvent, nullptr))
            return true;
        return pThis->signal_entry_key_press(pEvent);
    }

    bool signal_entry_key_press(const GdkEventKey* pEvent)
    {
        KeyEvent aKEvt(GtkToVcl(*pEvent));

        vcl::KeyCode aKeyCode = aKEvt.GetKeyCode();

        bool bDone = false;

        auto nCode = aKeyCode.GetCode();
        switch (nCode)
        {
            case KEY_DOWN:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nCount = get_count_including_mru();
                    int nActive = get_active_including_mru() + 1;
                    while (nActive < nCount && separator_function(nActive))
                        ++nActive;
                    if (nActive < nCount)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                else if (nKeyMod == KEY_MOD2 && !m_bPopupActive)
                {
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), true);
                    bDone = true;
                }
                break;
            }
            case KEY_UP:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nStartBound = m_bPopupActive ? 0 : (m_nMRUCount + 1);
                    int nActive = get_active_including_mru() - 1;
                    while (nActive >= nStartBound && separator_function(nActive))
                        --nActive;
                    if (nActive >= nStartBound)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                break;
            }
            case KEY_PAGEUP:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nCount = get_count_including_mru();
                    int nStartBound = m_bPopupActive ? 0 : (m_nMRUCount + 1);
                    int nActive = nStartBound;
                    while (nActive < nCount && separator_function(nActive))
                        ++nActive;
                    if (nActive < nCount)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                break;
            }
            case KEY_PAGEDOWN:
            {
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                if (!nKeyMod)
                {
                    int nActive = get_count_including_mru() - 1;
                    int nStartBound = m_bPopupActive ? 0 : (m_nMRUCount + 1);
                    while (nActive >= nStartBound && separator_function(nActive))
                        --nActive;
                    if (nActive >= nStartBound)
                        set_active_including_mru(nActive, true);
                    bDone = true;
                }
                break;
            }
            default:
                break;
        }

        return bDone;
    }

    bool signal_key_press(const GdkEventKey* pEvent)
    {
        if (m_bHoverSelection)
        {
            // once a key is pressed, turn off hover selection until mouse is
            // moved again otherwise when the treeview scrolls it jumps to the
            // position under the mouse.
            gtk_tree_view_set_hover_selection(m_pTreeView, false);
            m_bHoverSelection = false;
        }

        KeyEvent aKEvt(GtkToVcl(*pEvent));

        vcl::KeyCode aKeyCode = aKEvt.GetKeyCode();

        bool bDone = false;

        auto nCode = aKeyCode.GetCode();
        switch (nCode)
        {
            case KEY_DOWN:
            case KEY_UP:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case KEY_HOME:
            case KEY_END:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_RETURN:
            {
                m_aQuickSelectionEngine.Reset();
                sal_uInt16 nKeyMod = aKeyCode.GetModifier();
                // tdf#131076 don't let bare return toggle menu popup active, but do allow deactivate
                if (nCode == KEY_RETURN && !nKeyMod && !m_bPopupActive)
                    bDone = combobox_activate();
                else if (nCode == KEY_UP && nKeyMod == KEY_MOD2 && m_bPopupActive)
                {
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);
                    bDone = true;
                }
                else if (nCode == KEY_DOWN && nKeyMod == KEY_MOD2 && !m_bPopupActive)
                {
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), true);
                    bDone = true;
                }
                break;
            }
            case KEY_ESCAPE:
            {
                m_aQuickSelectionEngine.Reset();
                if (m_bPopupActive)
                {
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);
                    bDone = true;
                }
                break;
            }
            default:
                // tdf#131076 let base space toggle menu popup when it's not already visible
                if (nCode == KEY_SPACE && !aKeyCode.GetModifier() && !m_bPopupActive)
                    bDone = false;
                else
                    bDone = m_aQuickSelectionEngine.HandleKeyEvent(aKEvt);
                break;
        }

        if (!bDone && !m_pEntry)
            bDone = signal_entry_key_press(pEvent);

        return bDone;
    }

    vcl::StringEntryIdentifier typeahead_getEntry(int nPos, OUString& out_entryText) const
    {
        int nEntryCount(get_count_including_mru());
        if (nPos >= nEntryCount)
            nPos = 0;
        out_entryText = get_text_including_mru(nPos);

        // vcl::StringEntryIdentifier does not allow for 0 values, but our position is 0-based
        // => normalize
        return reinterpret_cast<vcl::StringEntryIdentifier>(nPos + 1);
    }

    static int typeahead_getEntryPos(vcl::StringEntryIdentifier entry)
    {
        // our pos is 0-based, but StringEntryIdentifier does not allow for a NULL
        return reinterpret_cast<sal_Int64>(entry) - 1;
    }

    void tree_view_set_cursor(int pos)
    {
        if (pos == -1)
        {
            gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(m_pTreeView));
            if (m_pCellView)
                gtk_cell_view_set_displayed_row(m_pCellView, nullptr);
        }
        else
        {
            GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
            if (gtk_tree_view_get_model(m_pTreeView))
                gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
            gtk_tree_view_set_cursor(m_pTreeView, path, nullptr, false);
            if (m_pCellView)
                gtk_cell_view_set_displayed_row(m_pCellView, path);
            gtk_tree_path_free(path);
        }
    }

    int tree_view_get_cursor() const
    {
        int nRet = -1;

        GtkTreePath* path;
        gtk_tree_view_get_cursor(m_pTreeView, &path, nullptr);
        if (path)
        {
            gint depth;
            gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
            nRet = indices[depth-1];
            gtk_tree_path_free(path);
        }

        return nRet;
    }

    int get_selected_entry() const
    {
        if (m_bPopupActive)
            return tree_view_get_cursor();
        else
            return get_active_including_mru();
    }

    void set_typeahead_selected_entry(int nSelect)
    {
        if (m_bPopupActive)
            tree_view_set_cursor(nSelect);
        else
            set_active_including_mru(nSelect, true);
    }

    virtual vcl::StringEntryIdentifier CurrentEntry(OUString& out_entryText) const override
    {
        int nCurrentPos = get_selected_entry();
        return typeahead_getEntry((nCurrentPos == -1) ? 0 : nCurrentPos, out_entryText);
    }

    virtual vcl::StringEntryIdentifier NextEntry(vcl::StringEntryIdentifier currentEntry, OUString& out_entryText) const override
    {
        int nNextPos = typeahead_getEntryPos(currentEntry) + 1;
        return typeahead_getEntry(nNextPos, out_entryText);
    }

    virtual void SelectEntry(vcl::StringEntryIdentifier entry) override
    {
        int nSelect = typeahead_getEntryPos(entry);
        if (nSelect == get_selected_entry())
        {
            // ignore that. This method is a callback from the QuickSelectionEngine, which means the user attempted
            // to select the given entry by typing its starting letters. No need to act.
            return;
        }

        // normalize
        int nCount = get_count_including_mru();
        if (nSelect >= nCount)
            nSelect = nCount ? nCount-1 : -1;

        set_typeahead_selected_entry(nSelect);
    }

    static void signalGrabBroken(GtkWidget*, GdkEventGrabBroken *pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->grab_broken(pEvent);
    }

    void grab_broken(const GdkEventGrabBroken *event)
    {
        if (event->grab_window == nullptr)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);
        }
        else if (!g_object_get_data(G_OBJECT(event->grab_window), "g-lo-InstancePopup")) // another LibreOffice popover took a grab
        {
            //try and regrab, so when we lose the grab to the menu of the color palette
            //combobox we regain it so the color palette doesn't itself disappear on next
            //click on the color palette combobox
            do_grab(GTK_WIDGET(m_pMenuWindow));
        }
    }

    static gboolean signalButtonPress(GtkWidget*, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        return pThis->button_press(pEvent);
    }

    bool button_press(GdkEventButton* pEvent)
    {
        //we want to pop down if the button was pressed outside our popup
        if (button_event_is_outside(GTK_WIDGET(m_pMenuWindow), pEvent))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);
        return false;
    }

    static gboolean signalMotion(GtkWidget*, GdkEventMotion*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_motion();
        return false;
    }

    void signal_motion()
    {
        // if hover-selection was disabled after pressing a key, then turn it back on again
        if (!m_bHoverSelection && !m_bMouseInOverlayButton)
        {
            gtk_tree_view_set_hover_selection(m_pTreeView, true);
            m_bHoverSelection = true;
        }
    }

    static void signalRowActivated(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->handle_row_activated();
    }

    void handle_row_activated()
    {
        m_bActivateCalled = true;
        m_bChangedByMenu = true;
        disable_notify_events();
        int nActive = get_active();
        if (m_pEntry)
            gtk_entry_set_text(GTK_ENTRY(m_pEntry), OUStringToOString(get_text(nActive), RTL_TEXTENCODING_UTF8).getStr());
        else
            tree_view_set_cursor(nActive);
        enable_notify_events();
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pToggleButton), false);
        fire_signal_changed();
        update_mru();
    }

    void do_clear()
    {
        disable_notify_events();
        gtk_tree_view_set_row_separator_func(m_pTreeView, nullptr, nullptr, nullptr);
        m_aSeparatorRows.clear();
        gtk_list_store_clear(GTK_LIST_STORE(m_pTreeModel));
        m_nMRUCount = 0;
        enable_notify_events();
    }

    virtual int get_max_mru_count() const override
    {
        return m_nMaxMRUCount;
    }

    virtual void set_max_mru_count(int nMaxMRUCount) override
    {
        m_nMaxMRUCount = nMaxMRUCount;
        update_mru();
    }

    void update_mru()
    {
        int nMRUCount = m_nMRUCount;

        if (m_nMaxMRUCount)
        {
            OUString sActiveText = get_active_text();
            OUString sActiveId = get_active_id();
            insert_including_mru(0, sActiveText, &sActiveId, nullptr, nullptr);
            ++m_nMRUCount;

            for (int i = 1; i < m_nMRUCount - 1; ++i)
            {
                if (get_text_including_mru(i) == sActiveText)
                {
                    remove_including_mru(i);
                    --m_nMRUCount;
                    break;
                }
            }
        }

        while (m_nMRUCount > m_nMaxMRUCount)
        {
            remove_including_mru(m_nMRUCount - 1);
            --m_nMRUCount;
        }

        if (m_nMRUCount && !nMRUCount)
            insert_separator_including_mru(m_nMRUCount, "separator");
        else if (!m_nMRUCount && nMRUCount)
            remove_including_mru(m_nMRUCount);  // remove separator
    }

    int get_count_including_mru() const
    {
        return gtk_tree_model_iter_n_children(m_pTreeModel, nullptr);
    }

    int get_active_including_mru() const
    {
        return tree_view_get_cursor();
    }

    void set_active_including_mru(int pos, bool bInteractive)
    {
        disable_notify_events();

        tree_view_set_cursor(pos);

        if (m_pEntry)
        {
            if (pos != -1)
                gtk_entry_set_text(GTK_ENTRY(m_pEntry), OUStringToOString(get_text_including_mru(pos), RTL_TEXTENCODING_UTF8).getStr());
            else
                gtk_entry_set_text(GTK_ENTRY(m_pEntry), "");
        }

        m_bChangedByMenu = false;
        enable_notify_events();

        if (bInteractive && !m_bPopupActive)
            signal_changed();
    }

    int find_text_including_mru(std::u16string_view rStr, bool bSearchMRU) const
    {
        return find(rStr, m_nTextCol, bSearchMRU);
    }

    int find_id_including_mru(std::u16string_view rId, bool bSearchMRU) const
    {
        return find(rId, m_nIdCol, bSearchMRU);
    }

    OUString get_text_including_mru(int pos) const
    {
        return get(pos, m_nTextCol);
    }

    OUString get_id_including_mru(int pos) const
    {
        return get(pos, m_nIdCol);
    }

    void set_id_including_mru(int pos, std::u16string_view rId)
    {
        set(pos, m_nIdCol, rId);
    }

    void remove_including_mru(int pos)
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos);
        if (!m_aSeparatorRows.empty())
        {
            bool bFound = false;

            GtkTreePath* pPath = gtk_tree_path_new_from_indices(pos, -1);

            for (auto aIter = m_aSeparatorRows.begin(); aIter != m_aSeparatorRows.end(); ++aIter)
            {
                GtkTreePath* seppath = gtk_tree_row_reference_get_path(aIter->get());
                if (seppath)
                {
                    if (gtk_tree_path_compare(pPath, seppath) == 0)
                        bFound = true;
                    gtk_tree_path_free(seppath);
                }
                if (bFound)
                {
                    m_aSeparatorRows.erase(aIter);
                    break;
                }
            }

            gtk_tree_path_free(pPath);
        }
        gtk_list_store_remove(GTK_LIST_STORE(m_pTreeModel), &iter);
        enable_notify_events();
    }

    void insert_separator_including_mru(int pos, const OUString& rId)
    {
        disable_notify_events();
        GtkTreeIter iter;
        if (!gtk_tree_view_get_row_separator_func(m_pTreeView))
            gtk_tree_view_set_row_separator_func(m_pTreeView, separatorFunction, this, nullptr);
        insert_row(GTK_LIST_STORE(m_pTreeModel), iter, pos, &rId, u"", nullptr, nullptr);
        GtkTreePath* pPath = gtk_tree_path_new_from_indices(pos, -1);
        m_aSeparatorRows.emplace_back(gtk_tree_row_reference_new(m_pTreeModel, pPath));
        gtk_tree_path_free(pPath);
        enable_notify_events();
    }

    void insert_including_mru(int pos, std::u16string_view rText, const OUString* pId, const OUString* pIconName, const VirtualDevice* pImageSurface)
    {
        disable_notify_events();
        GtkTreeIter iter;
        insert_row(GTK_LIST_STORE(m_pTreeModel), iter, pos, pId, rText, pIconName, pImageSurface);
        enable_notify_events();
    }

    static gboolean signalGetChildPosition(GtkOverlay*, GtkWidget*, GdkRectangle* pAllocation, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        return pThis->signal_get_child_position(pAllocation);
    }

    bool signal_get_child_position(GdkRectangle* pAllocation)
    {
        if (!gtk_widget_get_visible(GTK_WIDGET(m_pOverlayButton)))
            return false;
        if (!gtk_widget_get_realized(GTK_WIDGET(m_pTreeView)))
            return false;
        int nRow = find_id_including_mru(m_sMenuButtonRow, true);
        if (nRow == -1)
            return false;

        gtk_widget_get_preferred_width(GTK_WIDGET(m_pOverlayButton), &pAllocation->width, nullptr);

        GtkTreePath* pPath = gtk_tree_path_new_from_indices(nRow, -1);
        GList* pColumns = gtk_tree_view_get_columns(m_pTreeView);
        tools::Rectangle aRect = get_row_area(m_pTreeView, pColumns, pPath);
        gtk_tree_path_free(pPath);
        g_list_free(pColumns);

        pAllocation->x = aRect.Right() - pAllocation->width;
        pAllocation->y = aRect.Top();
        pAllocation->height = aRect.GetHeight();

        return true;
    }

    static gboolean signalOverlayButtonCrossing(GtkWidget*, GdkEventCrossing* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_overlay_button_crossing(pEvent->type == GDK_ENTER_NOTIFY);
        return false;
    }

    void signal_overlay_button_crossing(bool bEnter)
    {
        m_bMouseInOverlayButton = bEnter;
        if (!bEnter)
            return;

        if (m_bHoverSelection)
        {
            // once toggled button is pressed, turn off hover selection until
            // mouse leaves the overlay button
            gtk_tree_view_set_hover_selection(m_pTreeView, false);
            m_bHoverSelection = false;
        }
        int nRow = find_id_including_mru(m_sMenuButtonRow, true);
        assert(nRow != -1);
        tree_view_set_cursor(nRow); // select the buttons row
    }

    void signal_combo_mnemonic_activate()
    {
        if (m_pEntry)
            gtk_widget_grab_focus(m_pEntry);
        else
            gtk_widget_grab_focus(m_pToggleButton);
    }

    static gboolean signalComboMnemonicActivate(GtkWidget*, gboolean, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_combo_mnemonic_activate();
        return true;
    }

    int include_mru(int pos)
    {
        if (m_nMRUCount && pos != -1)
            pos += (m_nMRUCount + 1);
        return pos;
    }

public:
    GtkInstanceComboBox(GtkBuilder* pComboBuilder, GtkComboBox* pComboBox, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(gtk_builder_get_object(pComboBuilder, "box")), pBuilder, bTakeOwnership)
        , m_pComboBuilder(pComboBuilder)
        , m_pComboBox(pComboBox)
        , m_pOverlay(GTK_OVERLAY(gtk_builder_get_object(pComboBuilder, "overlay")))
        , m_pTreeView(GTK_TREE_VIEW(gtk_builder_get_object(pComboBuilder, "treeview")))
        , m_pOverlayButton(GTK_MENU_BUTTON(gtk_builder_get_object(pComboBuilder, "overlaybutton")))
        , m_pMenuWindow(GTK_WINDOW(gtk_builder_get_object(pComboBuilder, "popup")))
        , m_pTreeModel(gtk_combo_box_get_model(pComboBox))
        , m_pButtonTextRenderer(nullptr)
        , m_pToggleButton(GTK_WIDGET(gtk_builder_get_object(pComboBuilder, "button")))
        , m_pEntry(GTK_WIDGET(gtk_builder_get_object(pComboBuilder, "entry")))
        , m_pCellView(nullptr)
        , m_aCustomFont(m_pWidget)
        , m_aQuickSelectionEngine(*this)
        , m_bHoverSelection(false)
        , m_bMouseInOverlayButton(false)
        , m_bPopupActive(false)
        , m_bAutoComplete(false)
        , m_bAutoCompleteCaseSensitive(false)
        , m_bChangedByMenu(false)
        , m_bCustomRenderer(false)
        , m_bActivateCalled(false)
        , m_nTextCol(gtk_combo_box_get_entry_text_column(pComboBox))
        , m_nIdCol(gtk_combo_box_get_id_column(pComboBox))
        , m_nToggleFocusInSignalId(0)
        , m_nToggleFocusOutSignalId(0)
        , m_nRowActivatedSignalId(g_signal_connect(m_pTreeView, "row-activated", G_CALLBACK(signalRowActivated), this))
        , m_nChangedSignalId(g_signal_connect(m_pEntry, "changed", G_CALLBACK(signalChanged), this))
        , m_nPopupShownSignalId(g_signal_connect(m_pToggleButton, "toggled", G_CALLBACK(signalPopupToggled), this))
        , m_nAutoCompleteIdleId(0)
        , m_nNonCustomLineHeight(-1)
        , m_nPrePopupCursorPos(-1)
        , m_nMRUCount(0)
        , m_nMaxMRUCount(0)
    {
        int nActive = gtk_combo_box_get_active(m_pComboBox);

        if (gtk_style_context_has_class(gtk_widget_get_style_context(GTK_WIDGET(m_pComboBox)), "small-button"))
            gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(getContainer())), "small-button");

        insertAsParent(GTK_WIDGET(m_pComboBox), GTK_WIDGET(getContainer()));
        gtk_widget_set_visible(GTK_WIDGET(m_pComboBox), false);
        gtk_widget_set_no_show_all(GTK_WIDGET(m_pComboBox), true);

        gtk_tree_view_set_model(m_pTreeView, m_pTreeModel);
        /* tdf#136455 gtk_combo_box_set_model with a null Model should be good
           enough. But in practice, while the ComboBox model is unset, GTK
           doesn't unset the ComboBox menus model, so that remains listening to
           additions to the ListStore and slowing things down massively.
           Using a new model does reset the menu to listen to that unused one instead */
        gtk_combo_box_set_model(m_pComboBox, GTK_TREE_MODEL(gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING)));

        GtkTreeViewColumn* pCol = gtk_tree_view_column_new();
        gtk_tree_view_append_column(m_pTreeView, pCol);

        bool bPixbufUsedSurface = gtk_tree_model_get_n_columns(m_pTreeModel) == 4;

        GList* cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(m_pComboBox));
        // move the cell renderers from the combobox to the replacement treeview
        m_pMenuTextRenderer = static_cast<GtkCellRenderer*>(cells->data);
        for (GList* pRenderer = g_list_first(cells); pRenderer; pRenderer = g_list_next(pRenderer))
        {
            GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
            bool bTextRenderer = pCellRenderer == m_pMenuTextRenderer;
            gtk_tree_view_column_pack_end(pCol, pCellRenderer, bTextRenderer);
            if (!bTextRenderer)
            {
                if (bPixbufUsedSurface)
                    gtk_tree_view_column_set_attributes(pCol, pCellRenderer, "surface", 3, nullptr);
                else
                    gtk_tree_view_column_set_attributes(pCol, pCellRenderer, "pixbuf", 2, nullptr);
            }
        }

        gtk_tree_view_column_set_attributes(pCol, m_pMenuTextRenderer, "text", m_nTextCol, nullptr);

        if (gtk_combo_box_get_has_entry(m_pComboBox))
        {
            m_bAutoComplete = true;
            m_nEntryInsertTextSignalId = g_signal_connect(m_pEntry, "insert-text", G_CALLBACK(signalEntryInsertText), this);
            m_nEntryActivateSignalId = g_signal_connect(m_pEntry, "activate", G_CALLBACK(signalEntryActivate), this);
            m_nEntryFocusInSignalId = g_signal_connect(m_pEntry, "focus-in-event", G_CALLBACK(signalEntryFocusIn), this);
            m_nEntryFocusOutSignalId = g_signal_connect(m_pEntry, "focus-out-event", G_CALLBACK(signalEntryFocusOut), this);
            m_nEntryKeyPressEventSignalId = g_signal_connect(m_pEntry, "key-press-event", G_CALLBACK(signalEntryKeyPress), this);
            m_nKeyPressEventSignalId = 0;
        }
        else
        {
            gtk_widget_set_visible(m_pEntry, false);
            m_pEntry = nullptr;

            GtkWidget* pArrow = GTK_WIDGET(gtk_builder_get_object(pComboBuilder, "arrow"));
            gtk_container_child_set(getContainer(), m_pToggleButton, "expand", true, nullptr);

            auto m_pCellArea = gtk_cell_area_box_new();
            m_pCellView = GTK_CELL_VIEW(gtk_cell_view_new_with_context(m_pCellArea, nullptr));
            gtk_widget_set_hexpand(GTK_WIDGET(m_pCellView), true);
            GtkBox* pBox = GTK_BOX(gtk_widget_get_parent(pArrow));

            gint nImageSpacing(2);
            GtkStyleContext *pContext = gtk_widget_get_style_context(GTK_WIDGET(m_pToggleButton));
            gtk_style_context_get_style(pContext, "image-spacing", &nImageSpacing, nullptr);
            gtk_box_set_spacing(pBox, nImageSpacing);

            gtk_box_pack_start(pBox, GTK_WIDGET(m_pCellView), false, true, 0);

            gtk_cell_view_set_fit_model(m_pCellView, true);
            gtk_cell_view_set_model(m_pCellView, m_pTreeModel);

            m_pButtonTextRenderer = gtk_cell_renderer_text_new();
            gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(m_pCellView), m_pButtonTextRenderer, true);
            gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(m_pCellView), m_pButtonTextRenderer, "text", m_nTextCol, nullptr);
            if (g_list_length(cells) > 1)
            {
                GtkCellRenderer* pCellRenderer = gtk_cell_renderer_pixbuf_new();
                gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(m_pCellView), pCellRenderer, false);
                if (bPixbufUsedSurface)
                    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(m_pCellView), pCellRenderer, "surface", 3, nullptr);
                else
                    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(m_pCellView), pCellRenderer, "pixbuf", 2, nullptr);
            }

            gtk_widget_show_all(GTK_WIDGET(m_pCellView));

            m_nEntryInsertTextSignalId = 0;
            m_nEntryActivateSignalId = 0;
            m_nEntryFocusInSignalId = 0;
            m_nEntryFocusOutSignalId = 0;
            m_nEntryKeyPressEventSignalId = 0;
            m_nKeyPressEventSignalId = g_signal_connect(m_pToggleButton, "key-press-event", G_CALLBACK(signalKeyPress), this);
        }

        g_list_free(cells);

        if (nActive != -1)
            tree_view_set_cursor(nActive);

        g_signal_connect(getContainer(), "mnemonic-activate", G_CALLBACK(signalComboMnemonicActivate), this);

        g_signal_connect(m_pMenuWindow, "grab-broken-event", G_CALLBACK(signalGrabBroken), this);
        g_signal_connect(m_pMenuWindow, "button-press-event", G_CALLBACK(signalButtonPress), this);
        g_signal_connect(m_pMenuWindow, "motion-notify-event", G_CALLBACK(signalMotion), this);
        // support typeahead for the menu itself, typing into the menu will
        // select via the vcl selection engine, a matching entry.
        g_signal_connect(m_pMenuWindow, "key-press-event", G_CALLBACK(signalKeyPress), this);

        g_signal_connect(m_pOverlay, "get-child-position", G_CALLBACK(signalGetChildPosition), this);
        gtk_overlay_add_overlay(m_pOverlay, GTK_WIDGET(m_pOverlayButton));
        g_signal_connect(m_pOverlayButton, "leave-notify-event", G_CALLBACK(signalOverlayButtonCrossing), this);
        g_signal_connect(m_pOverlayButton, "enter-notify-event", G_CALLBACK(signalOverlayButtonCrossing), this);
    }

    virtual int get_active() const override
    {
        int nActive = get_active_including_mru();
        if (nActive == -1)
            return -1;

        if (m_nMRUCount)
        {
            if (nActive < m_nMRUCount)
                nActive = find_text(get_text_including_mru(nActive));
            else
                nActive -= (m_nMRUCount + 1);
        }

        return nActive;
    }

    virtual OUString get_active_id() const override
    {
        int nActive = get_active();
        return nActive != -1 ? get_id(nActive) : OUString();
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        set_active(find_id(rStr));
        m_bChangedByMenu = false;
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        if (m_pButtonTextRenderer)
        {
            // tweak the cell render to get a narrower size to stick
            if (nWidth != -1)
            {
                // this bit isn't great, I really want to be able to ellipse the text in the comboboxtext itself and let
                // the popup menu render them in full, in the interim ellipse both of them
                g_object_set(G_OBJECT(m_pButtonTextRenderer), "ellipsize", PANGO_ELLIPSIZE_MIDDLE, nullptr);

                // to find out how much of the width of the combobox belongs to the cell, set
                // the cell and widget to the min cell width and see what the difference is
                int min;
                gtk_cell_renderer_get_preferred_width(m_pButtonTextRenderer, m_pWidget, &min, nullptr);
                gtk_cell_renderer_set_fixed_size(m_pButtonTextRenderer, min, -1);
                gtk_widget_set_size_request(m_pWidget, min, -1);
                int nNonCellWidth = get_preferred_size().Width() - min;

                int nCellWidth = nWidth - nNonCellWidth;
                if (nCellWidth >= 0)
                {
                    // now set the cell to the max width which it can be within the
                    // requested widget width
                    gtk_cell_renderer_set_fixed_size(m_pButtonTextRenderer, nWidth - nNonCellWidth, -1);
                }
            }
            else
            {
                g_object_set(G_OBJECT(m_pButtonTextRenderer), "ellipsize", PANGO_ELLIPSIZE_NONE, nullptr);
                gtk_cell_renderer_set_fixed_size(m_pButtonTextRenderer, -1, -1);
            }
        }

        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual void set_active(int pos) override
    {
        set_active_including_mru(include_mru(pos), false);
    }

    virtual OUString get_active_text() const override
    {
        if (m_pEntry)
        {
            const gchar* pText = gtk_entry_get_text(GTK_ENTRY(m_pEntry));
            return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        }

        int nActive = get_active();
        if (nActive == -1)
           return OUString();

        return get_text(nActive);
    }

    virtual OUString get_text(int pos) const override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        return get_text_including_mru(pos);
    }

    virtual OUString get_id(int pos) const override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        return get_id_including_mru(pos);
    }

    virtual void set_id(int pos, const OUString& rId) override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        set_id_including_mru(pos, rId);
    }

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting) override
    {
        freeze();

        int nInsertionPoint;
        if (!bKeepExisting)
        {
            clear();
            nInsertionPoint = 0;
        }
        else
            nInsertionPoint = get_count();

        GtkTreeIter iter;
        // tdf#125241 inserting backwards is faster
        for (auto aI = rItems.rbegin(); aI != rItems.rend(); ++aI)
        {
            const auto& rItem = *aI;
            insert_row(GTK_LIST_STORE(m_pTreeModel), iter, nInsertionPoint, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
                       rItem.sString, rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
        }

        thaw();
    }

    virtual void remove(int pos) override
    {
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        remove_including_mru(pos);
    }

    virtual void insert(int pos, const OUString& rText, const OUString* pId, const OUString* pIconName, VirtualDevice* pImageSurface) override
    {
        insert_including_mru(include_mru(pos), rText, pId, pIconName, pImageSurface);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        pos = pos == -1 ? get_count() : pos;
        if (m_nMRUCount)
            pos += (m_nMRUCount + 1);
        insert_separator_including_mru(pos, rId);
    }

    virtual int get_count() const override
    {
        int nCount = get_count_including_mru();
        if (m_nMRUCount)
            nCount -= (m_nMRUCount + 1);
        return nCount;
    }

    virtual int find_text(const OUString& rStr) const override
    {
        int nPos = find_text_including_mru(rStr, false);
        if (nPos != -1 && m_nMRUCount)
            nPos -= (m_nMRUCount + 1);
        return nPos;
    }

    virtual int find_id(const OUString& rId) const override
    {
        int nPos = find_id_including_mru(rId, false);
        if (nPos != -1 && m_nMRUCount)
            nPos -= (m_nMRUCount + 1);
        return nPos;
    }

    virtual void clear() override
    {
        do_clear();
    }

    virtual void make_sorted() override
    {
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetUILanguageTag().getLocale()));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_set_sort_column_id(pSortable, m_nTextCol, GTK_SORT_ASCENDING);
        gtk_tree_sortable_set_sort_func(pSortable, m_nTextCol, default_sort_func, m_xSorter.get(), nullptr);
    }

    virtual bool has_entry() const override
    {
        return gtk_combo_box_get_has_entry(m_pComboBox);
    }

    virtual void set_entry_message_type(weld::EntryMessageType eType) override
    {
        assert(m_pEntry);
        ::set_entry_message_type(GTK_ENTRY(m_pEntry), eType);
    }

    virtual void set_entry_text(const OUString& rText) override
    {
        assert(m_pEntry);
        disable_notify_events();
        gtk_entry_set_text(GTK_ENTRY(m_pEntry), OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
        enable_notify_events();
    }

    virtual void set_entry_width_chars(int nChars) override
    {
        assert(m_pEntry);
        disable_notify_events();
        gtk_entry_set_width_chars(GTK_ENTRY(m_pEntry), nChars);
        gtk_entry_set_max_width_chars(GTK_ENTRY(m_pEntry), nChars);
        enable_notify_events();
    }

    virtual void set_entry_max_length(int nChars) override
    {
        assert(m_pEntry);
        disable_notify_events();
        gtk_entry_set_max_length(GTK_ENTRY(m_pEntry), nChars);
        enable_notify_events();
    }

    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        assert(m_pEntry);
        disable_notify_events();
        gtk_editable_select_region(GTK_EDITABLE(m_pEntry), nStartPos, nEndPos);
        enable_notify_events();
    }

    virtual bool get_entry_selection_bounds(int& rStartPos, int &rEndPos) override
    {
        assert(m_pEntry);
        return gtk_editable_get_selection_bounds(GTK_EDITABLE(m_pEntry), &rStartPos, &rEndPos);
    }

    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive) override
    {
        m_bAutoComplete = bEnable;
        m_bAutoCompleteCaseSensitive = bCaseSensitive;
    }

    virtual void set_entry_placeholder_text(const OUString& rText) override
    {
        assert(m_pEntry);
        gtk_entry_set_placeholder_text(GTK_ENTRY(m_pEntry), rText.toUtf8().getStr());
    }

    virtual void set_entry_editable(bool bEditable) override
    {
        assert(m_pEntry);
        gtk_editable_set_editable(GTK_EDITABLE(m_pEntry), bEditable);
    }

    virtual void cut_entry_clipboard() override
    {
        assert(m_pEntry);
        gtk_editable_cut_clipboard(GTK_EDITABLE(m_pEntry));
    }

    virtual void copy_entry_clipboard() override
    {
        assert(m_pEntry);
        gtk_editable_copy_clipboard(GTK_EDITABLE(m_pEntry));
    }

    virtual void paste_entry_clipboard() override
    {
        assert(m_pEntry);
        gtk_editable_paste_clipboard(GTK_EDITABLE(m_pEntry));
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_aCustomFont.use_custom_font(&rFont, u"box#combobox");
    }

    virtual vcl::Font get_font() override
    {
        if (const vcl::Font* pFont = m_aCustomFont.get_custom_font())
            return *pFont;
        return GtkInstanceWidget::get_font();
    }

    virtual void set_entry_font(const vcl::Font& rFont) override
    {
        m_xEntryFont = rFont;
        assert(m_pEntry);
        PangoAttrList* pOrigList = gtk_entry_get_attributes(GTK_ENTRY(m_pEntry));
        PangoAttrList* pAttrList = pOrigList ? pango_attr_list_copy(pOrigList) : pango_attr_list_new();
        update_attr_list(pAttrList, rFont);
        gtk_entry_set_attributes(GTK_ENTRY(m_pEntry), pAttrList);
        pango_attr_list_unref(pAttrList);
    }

    virtual vcl::Font get_entry_font() override
    {
        if (m_xEntryFont)
            return *m_xEntryFont;
        assert(m_pEntry);
        PangoContext* pContext = gtk_widget_get_pango_context(m_pEntry);
        return pango_to_vcl(pango_context_get_font_description(pContext),
                            Application::GetSettings().GetUILanguageTag().getLocale());
    }

    virtual void disable_notify_events() override
    {
        if (m_pEntry)
        {
            g_signal_handler_block(m_pEntry, m_nEntryInsertTextSignalId);
            g_signal_handler_block(m_pEntry, m_nEntryActivateSignalId);
            g_signal_handler_block(m_pEntry, m_nEntryFocusInSignalId);
            g_signal_handler_block(m_pEntry, m_nEntryFocusOutSignalId);
            g_signal_handler_block(m_pEntry, m_nEntryKeyPressEventSignalId);
            g_signal_handler_block(m_pEntry, m_nChangedSignalId);
        }
        else
            g_signal_handler_block(m_pToggleButton, m_nKeyPressEventSignalId);
        if (m_nToggleFocusInSignalId)
            g_signal_handler_block(m_pToggleButton, m_nToggleFocusInSignalId);
        if (m_nToggleFocusOutSignalId)
            g_signal_handler_block(m_pToggleButton, m_nToggleFocusOutSignalId);
        g_signal_handler_block(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_block(m_pToggleButton, m_nPopupShownSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pToggleButton, m_nPopupShownSignalId);
        g_signal_handler_unblock(m_pTreeView, m_nRowActivatedSignalId);
        if (m_nToggleFocusInSignalId)
            g_signal_handler_unblock(m_pToggleButton, m_nToggleFocusInSignalId);
        if (m_nToggleFocusOutSignalId)
            g_signal_handler_unblock(m_pToggleButton, m_nToggleFocusOutSignalId);
        if (m_pEntry)
        {
            g_signal_handler_unblock(m_pEntry, m_nChangedSignalId);
            g_signal_handler_unblock(m_pEntry, m_nEntryActivateSignalId);
            g_signal_handler_unblock(m_pEntry, m_nEntryFocusInSignalId);
            g_signal_handler_unblock(m_pEntry, m_nEntryFocusOutSignalId);
            g_signal_handler_unblock(m_pEntry, m_nEntryKeyPressEventSignalId);
            g_signal_handler_unblock(m_pEntry, m_nEntryInsertTextSignalId);
        }
        else
            g_signal_handler_unblock(m_pToggleButton, m_nKeyPressEventSignalId);
    }

    virtual void freeze() override
    {
        disable_notify_events();
        bool bIsFirstFreeze = IsFirstFreeze();
        GtkInstanceContainer::freeze();
        if (bIsFirstFreeze)
        {
            g_object_ref(m_pTreeModel);
            gtk_tree_view_set_model(m_pTreeView, nullptr);
            g_object_freeze_notify(G_OBJECT(m_pTreeModel));
            if (m_xSorter)
            {
                GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
                gtk_tree_sortable_set_sort_column_id(pSortable, GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
            }
        }
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
        if (IsLastThaw())
        {
            if (m_xSorter)
            {
                GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
                gtk_tree_sortable_set_sort_column_id(pSortable, m_nTextCol, GTK_SORT_ASCENDING);
            }
            g_object_thaw_notify(G_OBJECT(m_pTreeModel));
            gtk_tree_view_set_model(m_pTreeView, m_pTreeModel);
            g_object_unref(m_pTreeModel);
        }
        GtkInstanceContainer::thaw();
        enable_notify_events();
    }

    virtual bool get_popup_shown() const override
    {
        return m_bPopupActive;
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        if (!m_nToggleFocusInSignalId)
            m_nToggleFocusInSignalId = g_signal_connect_after(m_pToggleButton, "focus-in-event", G_CALLBACK(signalFocusIn), this);
        GtkInstanceContainer::connect_focus_in(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
        if (!m_nToggleFocusOutSignalId)
            m_nToggleFocusOutSignalId = g_signal_connect_after(m_pToggleButton, "focus-out-event", G_CALLBACK(signalFocusOut), this);
        GtkInstanceContainer::connect_focus_out(rLink);
    }

    virtual void grab_focus() override
    {
        if (has_focus())
            return;
        if (m_pEntry)
            gtk_widget_grab_focus(m_pEntry);
        else
            gtk_widget_grab_focus(m_pToggleButton);
    }

    virtual bool has_focus() const override
    {
        if (m_pEntry && gtk_widget_has_focus(m_pEntry))
            return true;

        if (gtk_widget_has_focus(m_pToggleButton))
            return true;

        if (gtk_widget_get_visible(GTK_WIDGET(m_pMenuWindow)))
        {
            if (gtk_widget_has_focus(GTK_WIDGET(m_pOverlayButton)) || gtk_widget_has_focus(GTK_WIDGET(m_pTreeView)))
                return true;
        }

        return GtkInstanceWidget::has_focus();
    }

    virtual bool changed_by_direct_pick() const override
    {
        return m_bChangedByMenu;
    }

    virtual void set_custom_renderer(bool bOn) override
    {
        if (bOn == m_bCustomRenderer)
            return;
        GList* pColumns = gtk_tree_view_get_columns(m_pTreeView);
        // keep the original height around for optimal popup height calculation
        m_nNonCustomLineHeight = bOn ? ::get_height_row(m_pTreeView, pColumns) : -1;
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pColumns->data);
        gtk_cell_layout_clear(GTK_CELL_LAYOUT(pColumn));
        if (bOn)
        {
            GtkCellRenderer *pRenderer = custom_cell_renderer_new();
            GValue value = G_VALUE_INIT;
            g_value_init(&value, G_TYPE_POINTER);
            g_value_set_pointer(&value, static_cast<gpointer>(this));
            g_object_set_property(G_OBJECT(pRenderer), "instance", &value);
            gtk_tree_view_column_pack_start(pColumn, pRenderer, true);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "text", m_nTextCol);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "id", m_nIdCol);
        }
        else
        {
            GtkCellRenderer *pRenderer = gtk_cell_renderer_text_new();
            gtk_tree_view_column_pack_start(pColumn, pRenderer, true);
            gtk_tree_view_column_add_attribute(pColumn, pRenderer, "text", m_nTextCol);
        }
        g_list_free(pColumns);
        m_bCustomRenderer = bOn;
    }

    void call_signal_custom_render(VirtualDevice& rOutput, const tools::Rectangle& rRect, bool bSelected, const OUString& rId)
    {
        signal_custom_render(rOutput, rRect, bSelected, rId);
    }

    Size call_signal_custom_get_size(VirtualDevice& rOutput)
    {
        return signal_custom_get_size(rOutput);
    }

    VclPtr<VirtualDevice> create_render_virtual_device() const override
    {
        return create_virtual_device();
    }

    virtual void set_item_menu(const OString& rIdent, weld::Menu* pMenu) override
    {
        m_xCustomMenuButtonHelper.reset();
        GtkInstanceMenu* pPopoverWidget = dynamic_cast<GtkInstanceMenu*>(pMenu);
        GtkWidget* pMenuWidget = GTK_WIDGET(pPopoverWidget ? pPopoverWidget->getMenu() : nullptr);
        gtk_menu_button_set_popup(m_pOverlayButton, pMenuWidget);
        gtk_widget_set_visible(GTK_WIDGET(m_pOverlayButton), pMenuWidget != nullptr);
        gtk_widget_queue_resize_no_redraw(GTK_WIDGET(m_pOverlayButton)); // force location recalc
        if (pMenuWidget)
            m_xCustomMenuButtonHelper.reset(new CustomRenderMenuButtonHelper(GTK_MENU(pMenuWidget), GTK_TOGGLE_BUTTON(m_pToggleButton)));
        m_sMenuButtonRow = OUString::fromUtf8(rIdent);
    }

    OUString get_mru_entries() const override
    {
        const sal_Unicode cSep = ';';

        OUStringBuffer aEntries;
        for (sal_Int32 n = 0; n < m_nMRUCount; n++)
        {
            aEntries.append(get_text_including_mru(n));
            if (n < m_nMRUCount - 1)
                aEntries.append(cSep);
        }
        return aEntries.makeStringAndClear();
    }

    virtual void set_mru_entries(const OUString& rEntries) override
    {
        const sal_Unicode cSep = ';';

        // Remove old MRU entries
        for (sal_Int32 n = m_nMRUCount; n;)
            remove_including_mru(--n);

        sal_Int32 nMRUCount = 0;
        sal_Int32 nIndex = 0;
        do
        {
            OUString aEntry = rEntries.getToken(0, cSep, nIndex);
            // Accept only existing entries
            int nPos = find_text(aEntry);
            if (nPos != -1)
            {
                OUString sId = get_id(nPos);
                insert_including_mru(0, aEntry, &sId, nullptr, nullptr);
                ++nMRUCount;
            }
        }
        while (nIndex >= 0);

        if (nMRUCount && !m_nMRUCount)
            insert_separator_including_mru(nMRUCount, "separator");
        else if (!nMRUCount && m_nMRUCount)
            remove_including_mru(m_nMRUCount);  // remove separator

        m_nMRUCount = nMRUCount;
    }

    int get_menu_button_width() const override
    {
        bool bVisible = gtk_widget_get_visible(GTK_WIDGET(m_pOverlayButton));
        if (!bVisible)
            gtk_widget_set_visible(GTK_WIDGET(m_pOverlayButton), true);
        gint nWidth;
        gtk_widget_get_preferred_width(GTK_WIDGET(m_pOverlayButton), &nWidth, nullptr);
        if (!bVisible)
            gtk_widget_set_visible(GTK_WIDGET(m_pOverlayButton), false);
        return nWidth;
    }

    virtual ~GtkInstanceComboBox() override
    {
        m_xCustomMenuButtonHelper.reset();
        do_clear();
        if (m_nAutoCompleteIdleId)
            g_source_remove(m_nAutoCompleteIdleId);
        if (m_pEntry)
        {
            g_signal_handler_disconnect(m_pEntry, m_nChangedSignalId);
            g_signal_handler_disconnect(m_pEntry, m_nEntryInsertTextSignalId);
            g_signal_handler_disconnect(m_pEntry, m_nEntryActivateSignalId);
            g_signal_handler_disconnect(m_pEntry, m_nEntryFocusInSignalId);
            g_signal_handler_disconnect(m_pEntry, m_nEntryFocusOutSignalId);
            g_signal_handler_disconnect(m_pEntry, m_nEntryKeyPressEventSignalId);
        }
        else
            g_signal_handler_disconnect(m_pToggleButton, m_nKeyPressEventSignalId);
        if (m_nToggleFocusInSignalId)
            g_signal_handler_disconnect(m_pToggleButton, m_nToggleFocusInSignalId);
        if (m_nToggleFocusOutSignalId)
            g_signal_handler_disconnect(m_pToggleButton, m_nToggleFocusOutSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_disconnect(m_pToggleButton, m_nPopupShownSignalId);

        gtk_combo_box_set_model(m_pComboBox, m_pTreeModel);
        gtk_tree_view_set_model(m_pTreeView, nullptr);

        // restore original hierarchy in dtor so a new GtkInstanceComboBox will
        // result in the same layout each time
        {
            DisconnectMouseEvents();

            g_object_ref(m_pComboBox);

            GtkContainer* pContainer = getContainer();

            gtk_container_remove(pContainer, GTK_WIDGET(m_pComboBox));

            replaceWidget(GTK_WIDGET(pContainer), GTK_WIDGET(m_pComboBox));

            g_object_unref(m_pComboBox);
        }

        g_object_unref(m_pComboBuilder);
    }
};

#endif

}

void custom_cell_renderer_ensure_device(CustomCellRenderer *cellsurface, gpointer user_data)
{
    if (!cellsurface->device)
    {
        cellsurface->device = VclPtr<VirtualDevice>::Create();
        cellsurface->device->SetBackground(COL_TRANSPARENT);
        GtkInstanceWidget* pWidget = static_cast<GtkInstanceWidget*>(user_data);
        // expand the point size of the desired font to the equivalent pixel size
        weld::SetPointFont(*cellsurface->device, pWidget->get_font());
    }
}

Size custom_cell_renderer_get_size(VirtualDevice& rDevice, const OUString& rCellId, gpointer user_data)
{
    GtkInstanceWidget* pWidget = static_cast<GtkInstanceWidget*>(user_data);
    if (GtkInstanceTreeView* pTreeView = dynamic_cast<GtkInstanceTreeView*>(pWidget))
        return pTreeView->call_signal_custom_get_size(rDevice, rCellId);
    else if (GtkInstanceComboBox* pComboBox = dynamic_cast<GtkInstanceComboBox*>(pWidget))
        return pComboBox->call_signal_custom_get_size(rDevice);
    return Size();
}

void custom_cell_renderer_render(VirtualDevice& rDevice, const tools::Rectangle& rRect, bool bSelected, const OUString& rCellId, gpointer user_data)
{
    GtkInstanceWidget* pWidget = static_cast<GtkInstanceWidget*>(user_data);
    if (GtkInstanceTreeView* pTreeView = dynamic_cast<GtkInstanceTreeView*>(pWidget))
        pTreeView->call_signal_custom_render(rDevice, rRect, bSelected, rCellId);
    else if (GtkInstanceComboBox* pComboBox = dynamic_cast<GtkInstanceComboBox*>(pWidget))
        pComboBox->call_signal_custom_render(rDevice, rRect, bSelected, rCellId);
}

namespace {

class GtkInstanceEntryTreeView : public GtkInstanceContainer, public virtual weld::EntryTreeView
{
private:
    GtkInstanceEntry* m_pEntry;
    GtkInstanceTreeView* m_pTreeView;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nKeyPressSignalId;
#endif
    gulong m_nEntryInsertTextSignalId;
    guint m_nAutoCompleteIdleId;
    bool m_bAutoCompleteCaseSensitive;
    bool m_bTreeChange;

#if !GTK_CHECK_VERSION(4, 0, 0)
    bool signal_key_press(GdkEventKey* pEvent)
    {
        if (GtkSalFrame::GetMouseModCode(pEvent->state)) // only with no modifiers held
            return false;

        if (pEvent->keyval == GDK_KEY_KP_Up || pEvent->keyval == GDK_KEY_Up || pEvent->keyval == GDK_KEY_KP_Page_Up || pEvent->keyval == GDK_KEY_Page_Up ||
            pEvent->keyval == GDK_KEY_KP_Down || pEvent->keyval == GDK_KEY_Down || pEvent->keyval == GDK_KEY_KP_Page_Down || pEvent->keyval == GDK_KEY_Page_Down)
        {
            gboolean ret;
            disable_notify_events();
            GtkWidget* pWidget = m_pTreeView->getWidget();
            if (m_pTreeView->get_selected_index() == -1)
            {
                m_pTreeView->set_cursor(0);
                m_pTreeView->select(0);
                m_xEntry->set_text(m_xTreeView->get_selected_text());
            }
            else
            {
                gtk_widget_grab_focus(pWidget);
                g_signal_emit_by_name(pWidget, "key-press-event", pEvent, &ret);
                m_xEntry->set_text(m_xTreeView->get_selected_text());
                gtk_widget_grab_focus(m_pEntry->getWidget());
            }
            m_xEntry->select_region(0, -1);
            enable_notify_events();
            m_bTreeChange = true;
            m_pEntry->fire_signal_changed();
            m_bTreeChange = false;
            return true;
        }
        return false;
    }

    static gboolean signalKeyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceEntryTreeView* pThis = static_cast<GtkInstanceEntryTreeView*>(widget);
        return pThis->signal_key_press(pEvent);
    }
#endif

    static gboolean idleAutoComplete(gpointer widget)
    {
        GtkInstanceEntryTreeView* pThis = static_cast<GtkInstanceEntryTreeView*>(widget);
        pThis->auto_complete();
        return false;
    }

    void auto_complete()
    {
        m_nAutoCompleteIdleId = 0;
        OUString aStartText = get_active_text();
        int nStartPos, nEndPos;
        get_entry_selection_bounds(nStartPos, nEndPos);
        int nMaxSelection = std::max(nStartPos, nEndPos);
        if (nMaxSelection != aStartText.getLength())
            return;

        disable_notify_events();
        int nActive = get_active();
        int nStart = nActive;

        if (nStart == -1)
            nStart = 0;

        // Try match case sensitive from current position
        int nPos = m_pTreeView->starts_with(aStartText, nStart, true);
        if (nPos == -1 && nStart != 0)
        {
            // Try match case insensitive, but from start
            nPos = m_pTreeView->starts_with(aStartText, 0, true);
        }

        if (!m_bAutoCompleteCaseSensitive)
        {
            // Try match case insensitive from current position
            nPos = m_pTreeView->starts_with(aStartText, nStart, false);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case insensitive, but from start
                nPos = m_pTreeView->starts_with(aStartText, 0, false);
            }
        }

        if (nPos == -1)
        {
            // Try match case sensitive from current position
            nPos = m_pTreeView->starts_with(aStartText, nStart, true);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case sensitive, but from start
                nPos = m_pTreeView->starts_with(aStartText, 0, true);
            }
        }

        if (nPos != -1)
        {
            OUString aText = get_text(nPos);
            if (aText != aStartText)
                set_active_text(aText);
            select_entry_region(aText.getLength(), aStartText.getLength());
        }
        enable_notify_events();
    }

    void signal_entry_insert_text(GtkEntry*, const gchar*, gint, gint*)
    {
        // now check for autocompletes
        if (m_nAutoCompleteIdleId)
            g_source_remove(m_nAutoCompleteIdleId);
        m_nAutoCompleteIdleId = g_idle_add(idleAutoComplete, this);
    }

    static void signalEntryInsertText(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength,
                                      gint* position, gpointer widget)
    {
        GtkInstanceEntryTreeView* pThis = static_cast<GtkInstanceEntryTreeView*>(widget);
        pThis->signal_entry_insert_text(pEntry, pNewText, nNewTextLength, position);
    }


public:
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkInstanceEntryTreeView(GtkWidget* pContainer, GtkInstanceBuilder* pBuilder, bool bTakeOwnership,
                             std::unique_ptr<weld::Entry> xEntry, std::unique_ptr<weld::TreeView> xTreeView)
#else
    GtkInstanceEntryTreeView(GtkContainer* pContainer, GtkInstanceBuilder* pBuilder, bool bTakeOwnership,
                             std::unique_ptr<weld::Entry> xEntry, std::unique_ptr<weld::TreeView> xTreeView)
#endif
        : EntryTreeView(std::move(xEntry), std::move(xTreeView))
        , GtkInstanceContainer(pContainer, pBuilder, bTakeOwnership)
        , m_pEntry(dynamic_cast<GtkInstanceEntry*>(m_xEntry.get()))
        , m_pTreeView(dynamic_cast<GtkInstanceTreeView*>(m_xTreeView.get()))
        , m_nAutoCompleteIdleId(0)
        , m_bAutoCompleteCaseSensitive(false)
        , m_bTreeChange(false)
    {
        assert(m_pEntry);
        GtkWidget* pWidget = m_pEntry->getWidget();
#if !GTK_CHECK_VERSION(4, 0, 0)
        m_nKeyPressSignalId = g_signal_connect(pWidget, "key-press-event", G_CALLBACK(signalKeyPress), this);
#endif
        m_nEntryInsertTextSignalId = g_signal_connect(pWidget, "insert-text", G_CALLBACK(signalEntryInsertText), this);
    }

    virtual void insert_separator(int /*pos*/, const OUString& /*rId*/) override
    {
        assert(false);
    }

    virtual void make_sorted() override
    {
        GtkWidget* pTreeView = m_pTreeView->getWidget();
        GtkTreeModel* pModel = gtk_tree_view_get_model(GTK_TREE_VIEW(pTreeView));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
        gtk_tree_sortable_set_sort_column_id(pSortable, 1, GTK_SORT_ASCENDING);
    }

    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive) override
    {
        assert(!bEnable && "not implemented yet"); (void)bEnable;
        m_bAutoCompleteCaseSensitive = bCaseSensitive;
    }

    virtual void set_entry_placeholder_text(const OUString& rText) override
    {
        m_xEntry->set_placeholder_text(rText);
    }

    virtual void set_entry_editable(bool bEditable) override
    {
        m_xEntry->set_editable(bEditable);
    }

    virtual void cut_entry_clipboard() override
    {
        m_xEntry->cut_clipboard();
    }

    virtual void copy_entry_clipboard() override
    {
        m_xEntry->copy_clipboard();
    }

    virtual void paste_entry_clipboard() override
    {
        m_xEntry->paste_clipboard();
    }

    virtual void set_font(const vcl::Font&) override
    {
        assert(false && "not implemented");
    }

    virtual void set_entry_font(const vcl::Font& rFont) override
    {
        m_xEntry->set_font(rFont);
    }

    virtual vcl::Font get_entry_font() override
    {
        return m_xEntry->get_font();
    }

    virtual void grab_focus() override { m_xEntry->grab_focus(); }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        m_xEntry->connect_focus_in(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
        m_xEntry->connect_focus_out(rLink);
    }

    virtual void disable_notify_events() override
    {
        GtkWidget* pWidget = m_pEntry->getWidget();
        g_signal_handler_block(pWidget, m_nEntryInsertTextSignalId);
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_block(pWidget, m_nKeyPressSignalId);
#endif
        m_pTreeView->disable_notify_events();
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkWidget* pWidget = m_pEntry->getWidget();
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_unblock(pWidget, m_nKeyPressSignalId);
#endif
        g_signal_handler_unblock(pWidget, m_nEntryInsertTextSignalId);
        m_pTreeView->enable_notify_events();
        GtkInstanceContainer::disable_notify_events();
    }

    virtual bool changed_by_direct_pick() const override
    {
        return m_bTreeChange;
    }

    virtual void set_custom_renderer(bool /*bOn*/) override
    {
        assert(false && "not implemented");
    }

    virtual int get_max_mru_count() const override
    {
        assert(false && "not implemented");
        return 0;
    }

    virtual void set_max_mru_count(int) override
    {
        assert(false && "not implemented");
    }

    virtual OUString get_mru_entries() const override
    {
        assert(false && "not implemented");
        return OUString();
    }

    virtual void set_mru_entries(const OUString&) override
    {
        assert(false && "not implemented");
    }

    virtual void set_item_menu(const OString&, weld::Menu*) override
    {
        assert(false && "not implemented");
    }

    VclPtr<VirtualDevice> create_render_virtual_device() const override
    {
        return create_virtual_device();
    }

    int get_menu_button_width() const override
    {
        assert(false && "not implemented");
        return 0;
    }

    virtual ~GtkInstanceEntryTreeView() override
    {
        if (m_nAutoCompleteIdleId)
            g_source_remove(m_nAutoCompleteIdleId);
        GtkWidget* pWidget = m_pEntry->getWidget();
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(pWidget, m_nKeyPressSignalId);
#endif
        g_signal_handler_disconnect(pWidget, m_nEntryInsertTextSignalId);
    }
};

}

namespace {

class GtkInstanceExpander : public GtkInstanceWidget, public virtual weld::Expander
{
private:
    GtkExpander* m_pExpander;
    gulong m_nSignalId;
#if !GTK_CHECK_VERSION(4, 0, 0)
    gulong m_nButtonPressEventSignalId;
#endif

    static void signalExpanded(GtkExpander* pExpander, GParamSpec*, gpointer widget)
    {
        GtkInstanceExpander* pThis = static_cast<GtkInstanceExpander*>(widget);
        SolarMutexGuard aGuard;

#if !GTK_CHECK_VERSION(4, 0, 0)
        if (gtk_expander_get_resize_toplevel(pExpander))
        {
            GtkWidget *pToplevel = widget_get_toplevel(GTK_WIDGET(pExpander));

            // https://gitlab.gnome.org/GNOME/gtk/issues/70
            // I imagine at some point a release with a fix will be available in which
            // case this can be avoided depending on version number
            if (pToplevel && GTK_IS_WINDOW(pToplevel) && gtk_widget_get_realized(pToplevel))
            {
                int nToplevelWidth, nToplevelHeight;
                int nChildHeight;

                GtkWidget* child = gtk_bin_get_child(GTK_BIN(pExpander));
                gtk_widget_get_preferred_height(child, &nChildHeight, nullptr);
                gtk_window_get_size(GTK_WINDOW(pToplevel), &nToplevelWidth, &nToplevelHeight);

                if (pThis->get_expanded())
                    nToplevelHeight += nChildHeight;
                else
                    nToplevelHeight -= nChildHeight;

                gtk_window_resize(GTK_WINDOW(pToplevel), nToplevelWidth, nToplevelHeight);
            }
        }
#else
        (void)pExpander;
#endif

        pThis->signal_expanded();
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean signalButton(GtkWidget*, GdkEventButton*, gpointer)
    {
        // don't let button press get to parent window, for the case of the
        // an expander in a sidebar where otherwise single click to expand
        // doesn't work
        return true;
    }
#endif

public:
    GtkInstanceExpander(GtkExpander* pExpander, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pExpander), pBuilder, bTakeOwnership)
        , m_pExpander(pExpander)
        , m_nSignalId(g_signal_connect(m_pExpander, "notify::expanded", G_CALLBACK(signalExpanded), this))
#if !GTK_CHECK_VERSION(4, 0, 0)
        , m_nButtonPressEventSignalId(g_signal_connect_after(m_pExpander, "button-press-event", G_CALLBACK(signalButton), this))
#endif
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        ::set_label(GTK_LABEL(gtk_expander_get_label_widget(m_pExpander)), rText);
    }

    virtual OUString get_label() const override
    {
        return ::get_label(GTK_LABEL(gtk_expander_get_label_widget(m_pExpander)));
    }

    virtual bool get_expanded() const override
    {
        return gtk_expander_get_expanded(m_pExpander);
    }

    virtual void set_expanded(bool bExpand) override
    {
        gtk_expander_set_expanded(m_pExpander, bExpand);
    }

    virtual ~GtkInstanceExpander() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        g_signal_handler_disconnect(m_pExpander, m_nButtonPressEventSignalId);
#endif
        g_signal_handler_disconnect(m_pExpander, m_nSignalId);
    }
};

}

namespace {

    gboolean signalTooltipQuery(GtkWidget* pWidget, gint /*x*/, gint /*y*/,
                                         gboolean /*keyboard_mode*/, GtkTooltip *tooltip)
    {
        const ImplSVHelpData& aHelpData = ImplGetSVHelpData();
        if (aHelpData.mbBalloonHelp) // extended tips
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            // by default use accessible description
            AtkObject* pAtkObject = gtk_widget_get_accessible(pWidget);
            const char* pDesc = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
            if (pDesc && pDesc[0])
            {
                gtk_tooltip_set_text(tooltip, pDesc);
                return true;
            }
#endif

            // fallback to the mechanism which needs help installed
            OString sHelpId = ::get_help_id(pWidget);
            Help* pHelp = !sHelpId.isEmpty() ? Application::GetHelp() : nullptr;
            if (pHelp)
            {
                OUString sHelpText = pHelp->GetHelpText(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), static_cast<weld::Widget*>(nullptr));
                if (!sHelpText.isEmpty())
                {
                    gtk_tooltip_set_text(tooltip, OUStringToOString(sHelpText, RTL_TEXTENCODING_UTF8).getStr());
                    return true;
                }
            }
        }

        const char* pDesc = gtk_widget_get_tooltip_text(pWidget);
        if (pDesc && pDesc[0])
        {
            gtk_tooltip_set_text(tooltip, pDesc);
            return true;
        }

        return false;
    }

}

namespace {

class GtkInstancePopover : public GtkInstanceContainer, public virtual weld::Popover
{
private:
#if !GTK_CHECK_VERSION(4, 0, 0)
    //popover cannot escape dialog under X so we might need to stick up own window instead
    GtkWindow* m_pMenuHack;
    bool m_bMenuPoppedUp;
    bool m_nButtonPressSeen;
#endif
    GtkPopover* m_pPopover;
    gulong m_nSignalId;
    ImplSVEvent* m_pClosedEvent;

    static void signalClosed(GtkPopover*, gpointer widget)
    {
        GtkInstancePopover* pThis = static_cast<GtkInstancePopover*>(widget);
        // call signal-closed async so the closed callback isn't called
        // while the GtkPopover handler is still in-execution
        pThis->launch_signal_closed();
    }

    DECL_LINK(async_signal_closed, void*, void);

    void launch_signal_closed()
    {
        if (m_pClosedEvent)
            Application::RemoveUserEvent(m_pClosedEvent);
        m_pClosedEvent = Application::PostUserEvent(LINK(this, GtkInstancePopover, async_signal_closed));
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean keyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstancePopover* pThis = static_cast<GtkInstancePopover*>(widget);
        return pThis->key_press(pEvent);
    }

    bool key_press(const GdkEventKey* pEvent)
    {
        if (pEvent->keyval == GDK_KEY_Escape)
        {
            popdown();
            return true;
        }
        return false;
    }

    static gboolean signalButtonPress(GtkWidget* /*pWidget*/, GdkEventButton* /*pEvent*/, gpointer widget)
    {
        GtkInstancePopover* pThis = static_cast<GtkInstancePopover*>(widget);
        pThis->m_nButtonPressSeen = true;
        return false;
    }

    static gboolean signalButtonRelease(GtkWidget* /*pWidget*/, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstancePopover* pThis = static_cast<GtkInstancePopover*>(widget);
        if (pThis->m_nButtonPressSeen && button_event_is_outside(GTK_WIDGET(pThis->m_pMenuHack), pEvent))
            pThis->popdown();
        return false;
    }

    bool forward_event_if_popup_under_mouse(GdkEvent* pEvent)
    {
        GtkWidget* pEventWidget = gtk_get_event_widget(pEvent);
        GtkWidget* pTopLevel = widget_get_toplevel(pEventWidget);

        if (pTopLevel == GTK_WIDGET(m_pMenuHack))
            return false;

        GdkSurface* pSurface = widget_get_surface(pTopLevel);
        void* pMouseEnteredAnotherPopup = g_object_get_data(G_OBJECT(pSurface), "g-lo-InstancePopup");
        if (!pMouseEnteredAnotherPopup)
            return false;

        return gtk_widget_event(pEventWidget, reinterpret_cast<GdkEvent*>(pEvent));
    }

    static gboolean signalButtonCrossing(GtkWidget*, GdkEvent* pEvent, gpointer widget)
    {
        GtkInstancePopover* pThis = static_cast<GtkInstancePopover*>(widget);
        return pThis->forward_event_if_popup_under_mouse(pEvent);
    }

    static gboolean signalMotion(GtkWidget*, GdkEvent* pEvent, gpointer widget)
    {
        GtkInstancePopover* pThis = static_cast<GtkInstancePopover*>(widget);
        return pThis->forward_event_if_popup_under_mouse(pEvent);
    }

    static void signalGrabBroken(GtkWidget*, GdkEventGrabBroken *pEvent, gpointer widget)
    {
        GtkInstancePopover* pThis = static_cast<GtkInstancePopover*>(widget);
        pThis->grab_broken(pEvent);
    }

    void grab_broken(const GdkEventGrabBroken *event)
    {
        if (event->grab_window == nullptr)
        {
            popdown();
        }
        else if (!g_object_get_data(G_OBJECT(event->grab_window), "g-lo-InstancePopup")) // another LibreOffice popover took a grab
        {
            //try and regrab, so when we lose the grab to the menu of the color palette
            //combobox we regain it so the color palette doesn't itself disappear on next
            //click on the color palette combobox
            do_grab(GTK_WIDGET(m_pMenuHack));
        }
    }

#endif

public:
    GtkInstancePopover(GtkPopover* pPopover, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
#if !GTK_CHECK_VERSION(4, 0, 0)
        : GtkInstanceContainer(GTK_CONTAINER(pPopover), pBuilder, bTakeOwnership)
        , m_pMenuHack(nullptr)
        , m_bMenuPoppedUp(false)
        , m_nButtonPressSeen(false)
#else
        : GtkInstanceContainer(GTK_WIDGET(pPopover), pBuilder, bTakeOwnership)
#endif
        , m_pPopover(pPopover)
        , m_nSignalId(g_signal_connect(m_pPopover, "closed", G_CALLBACK(signalClosed), this))
        , m_pClosedEvent(nullptr)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        //under wayland a Popover will work to "escape" the parent dialog, not
        //so under X, so come up with this hack to use a raw GtkWindow
        GdkDisplay *pDisplay = gtk_widget_get_display(GTK_WIDGET(m_pPopover));
        if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
        {
            m_pMenuHack = GTK_WINDOW(gtk_window_new(GTK_WINDOW_POPUP));
            gtk_window_set_type_hint(m_pMenuHack, GDK_WINDOW_TYPE_HINT_COMBO);
            gtk_window_set_resizable(m_pMenuHack, false);
            g_signal_connect(m_pMenuHack, "key-press-event", G_CALLBACK(keyPress), this);
            g_signal_connect(m_pMenuHack, "grab-broken-event", G_CALLBACK(signalGrabBroken), this);
            g_signal_connect(m_pMenuHack, "button-press-event", G_CALLBACK(signalButtonPress), this);
            g_signal_connect(m_pMenuHack, "button-release-event", G_CALLBACK(signalButtonRelease), this);
            // to emulate a modeless popover we forward the leave/enter/motion events to the widgets
            // they would have gone to a if we were really modeless
            if (!gtk_popover_get_modal(m_pPopover))
            {
                g_signal_connect(m_pMenuHack, "leave-notify-event", G_CALLBACK(signalButtonCrossing), this);
                g_signal_connect(m_pMenuHack, "enter-notify-event", G_CALLBACK(signalButtonCrossing), this);
                g_signal_connect(m_pMenuHack, "motion-notify-event", G_CALLBACK(signalMotion), this);
            }
        }
#endif
    }

    virtual void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect, weld::Placement ePlace) override
    {
        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pParent);
        assert(pGtkWidget);

        GtkWidget* pWidget = pGtkWidget->getWidget();

        GdkRectangle aRect;
        pWidget = getPopupRect(pWidget, rRect, aRect);

#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_set_parent(GTK_WIDGET(m_pPopover), pWidget);
#else
        gtk_popover_set_relative_to(m_pPopover, pWidget);
#endif
        gtk_popover_set_pointing_to(m_pPopover, &aRect);

        if (ePlace == weld::Placement::Under)
            gtk_popover_set_position(m_pPopover, GTK_POS_BOTTOM);
        else
        {
            if (::SwapForRTL(pWidget))
                gtk_popover_set_position(m_pPopover, GTK_POS_LEFT);
            else
                gtk_popover_set_position(m_pPopover, GTK_POS_RIGHT);
        }

#if !GTK_CHECK_VERSION(4, 0, 0)
        //under wayland a Popover will work to "escape" the parent dialog, not
        //so under X, so come up with this hack to use a raw GtkWindow
        GdkDisplay *pDisplay = gtk_widget_get_display(GTK_WIDGET(m_pPopover));
        if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
        {
            if (!m_bMenuPoppedUp)
            {
                MovePopoverContentsToWindow(GTK_WIDGET(m_pPopover), m_pMenuHack, pWidget, aRect, ePlace);
                m_bMenuPoppedUp = true;
            }
            return;
        }
#endif

        gtk_popover_popup(m_pPopover);
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    virtual bool get_visible() const override
    {
        if (m_pMenuHack)
            return gtk_widget_get_visible(GTK_WIDGET(m_pMenuHack));
        return gtk_widget_get_visible(m_pWidget);
    }

    virtual void ensureMouseEventWidget() override
    {
        if (!m_pMouseEventBox && m_pMenuHack)
        {
            m_pMouseEventBox = GTK_WIDGET(m_pMenuHack);
            return;
        }
        GtkInstanceContainer::ensureMouseEventWidget();
    }
#endif

    virtual void popdown() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        //under wayland a Popover will work to "escape" the parent dialog, not
        //so under X, so come up with this hack to use a raw GtkWindow
        GdkDisplay *pDisplay = gtk_widget_get_display(GTK_WIDGET(m_pPopover));
        if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
        {
            if (m_bMenuPoppedUp)
            {
                m_nButtonPressSeen = false;
                MoveWindowContentsToPopover(m_pMenuHack, GTK_WIDGET(m_pPopover), gtk_popover_get_relative_to(m_pPopover));
                m_bMenuPoppedUp = false;
                signal_closed();
            }
            return;
        }
#endif

        gtk_popover_popdown(m_pPopover);
    }

    void PopdownAndFlushClosedSignal()
    {
        if (get_visible())
            popdown();
        if (m_pClosedEvent)
        {
            Application::RemoveUserEvent(m_pClosedEvent);
            async_signal_closed(nullptr);
        }
    }

    virtual ~GtkInstancePopover() override
    {
        PopdownAndFlushClosedSignal();
        DisconnectMouseEvents();
#if !GTK_CHECK_VERSION(4, 0, 0)
        if (m_pMenuHack)
            gtk_widget_destroy(GTK_WIDGET(m_pMenuHack));
#endif
        g_signal_handler_disconnect(m_pPopover, m_nSignalId);
    }
};

IMPL_LINK_NOARG(GtkInstancePopover, async_signal_closed, void*, void)
{
    m_pClosedEvent = nullptr;
    signal_closed();
}

}

#if !GTK_CHECK_VERSION(4, 0, 0)

namespace
{

AtkObject* drawing_area_get_accessibity(GtkWidget *pWidget)
{
    AtkObject* pDefaultAccessible = default_drawing_area_get_accessible(pWidget);
    void* pData = g_object_get_data(G_OBJECT(pWidget), "g-lo-GtkInstanceDrawingArea");
    GtkInstanceDrawingArea* pDrawingArea = static_cast<GtkInstanceDrawingArea*>(pData);
    AtkObject *pAtkObj = pDrawingArea ? pDrawingArea->GetAtkObject(pDefaultAccessible) : nullptr;
    if (pAtkObj)
        return pAtkObj;
    return pDefaultAccessible;
}

void ensure_intercept_drawing_area_accessibility()
{
    static bool bDone;
    if (!bDone)
    {
        gpointer pClass = g_type_class_ref(GTK_TYPE_DRAWING_AREA);
        GtkWidgetClass* pWidgetClass = GTK_WIDGET_CLASS(pClass);
        default_drawing_area_get_accessible = pWidgetClass->get_accessible;
        pWidgetClass->get_accessible = drawing_area_get_accessibity;
        g_type_class_unref(pClass);
        bDone = true;
    }
}

void ensure_disable_ctrl_page_up_down(GType eType)
{
    gpointer pClass = g_type_class_ref(eType);
    GtkWidgetClass* pWidgetClass = GTK_WIDGET_CLASS(pClass);
    GtkBindingSet* pBindingSet = gtk_binding_set_by_class(pWidgetClass);
    gtk_binding_entry_remove(pBindingSet, GDK_KEY_Page_Up, GDK_CONTROL_MASK);
    gtk_binding_entry_remove(pBindingSet, GDK_KEY_Page_Up, static_cast<GdkModifierType>(GDK_SHIFT_MASK|GDK_CONTROL_MASK));
    gtk_binding_entry_remove(pBindingSet, GDK_KEY_Page_Down, GDK_CONTROL_MASK);
    gtk_binding_entry_remove(pBindingSet, GDK_KEY_Page_Down, static_cast<GdkModifierType>(GDK_SHIFT_MASK|GDK_CONTROL_MASK));
    g_type_class_unref(pClass);
}

// tdf#130400 disable ctrl+page_up and ctrl+page_down bindings so the
// keystrokes are consumed by the surrounding notebook bindings instead
void ensure_disable_ctrl_page_up_down_bindings()
{
    static bool bDone;
    if (!bDone)
    {
        ensure_disable_ctrl_page_up_down(GTK_TYPE_TREE_VIEW);
        ensure_disable_ctrl_page_up_down(GTK_TYPE_SPIN_BUTTON);
        bDone = true;
    }
}

}
#endif

namespace {

bool IsAllowedBuiltInIcon(std::u16string_view iconName)
{
    // limit the named icons to those known by VclBuilder
    return VclBuilder::mapStockToSymbol(iconName) != SymbolType::DONTKNOW;
}

}

namespace {

void load_ui_file(GtkBuilder* pBuilder, const OUString& rUri)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    builder_add_from_gtk3_file(pBuilder, rUri);
#else
    OUString aPath;
    osl::FileBase::getSystemPathFromFileURL(rUri, aPath);
    GError *err = nullptr;
    auto rc = gtk_builder_add_from_file(pBuilder, OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr(), &err);

    if (!rc)
    {
        SAL_WARN( "vcl.gtk", "GtkInstanceBuilder: error when calling gtk_builder_add_from_file: " << err->message);
        g_error_free(err);
    }
    assert(rc && "could not load UI file");
#endif
}

class GtkInstanceBuilder : public weld::Builder
{
private:
    ResHookProc m_pStringReplace;
    OString m_aUtf8HelpRoot;
    OUString m_aIconTheme;
    OUString m_aUILang;
    GtkBuilder* m_pBuilder;
    GSList* m_pObjectList;
    GtkWidget* m_pParentWidget;
    gulong m_nNotifySignalId;
    std::vector<GtkButton*> m_aMnemonicButtons;
#if GTK_CHECK_VERSION(4, 0, 0)
    std::vector<GtkCheckButton*> m_aMnemonicCheckButtons;
#endif
    std::vector<GtkLabel*> m_aMnemonicLabels;

    VclPtr<SystemChildWindow> m_xInterimGlue;
    bool m_bAllowCycleFocusOut;

    void postprocess_widget(GtkWidget* pWidget)
    {
        const bool bHideHelp = comphelper::LibreOfficeKit::isActive() &&
            officecfg::Office::Common::Help::HelpRootURL::get().isEmpty();

        //fixup icons
        //wanted: better way to do this, e.g. make gtk use gio for
        //loading from a filename and provide gio protocol handler
        //for our image in a zip urls
        //
        //unpack the images and keep them as dirs and just
        //add the paths to the gtk icon theme dir
        if (GTK_IS_IMAGE(pWidget))
        {
            GtkImage* pImage = GTK_IMAGE(pWidget);
            if (const gchar* icon_name = image_get_icon_name(pImage))
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);
                if (!IsAllowedBuiltInIcon(aIconName))
                {
                    if (GdkPixbuf* pixbuf = load_icon_by_name_theme_lang(aIconName, m_aIconTheme, m_aUILang))
                    {
                        gtk_image_set_from_pixbuf(pImage, pixbuf);
                        g_object_unref(pixbuf);
                    }
                }
            }
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        else if (GTK_IS_PICTURE(pWidget))
        {
            GtkPicture* pPicture = GTK_PICTURE(pWidget);
            if (GFile* icon_file = gtk_picture_get_file(pPicture))
            {
                char* icon_name = g_file_get_uri(icon_file);
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);
                g_free(icon_name);
                assert(aIconName.startsWith("private:///graphicrepository/"));
                aIconName.startsWith("private:///graphicrepository/", &aIconName);
                if (GdkPixbuf* pixbuf = load_icon_by_name_theme_lang(aIconName, m_aIconTheme, m_aUILang))
                {
                    gtk_picture_set_pixbuf(GTK_PICTURE(pWidget), pixbuf);
                    g_object_unref(pixbuf);
                }
            }
        }
#endif
#if !GTK_CHECK_VERSION(4, 0, 0)
        else if (GTK_IS_TOOL_BUTTON(pWidget))
        {
            GtkToolButton* pToolButton = GTK_TOOL_BUTTON(pWidget);
            if (const gchar* icon_name = gtk_tool_button_get_icon_name(pToolButton))
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);
                if (!IsAllowedBuiltInIcon(aIconName))
                {
                    if (GdkPixbuf* pixbuf = load_icon_by_name_theme_lang(aIconName, m_aIconTheme, m_aUILang))
                    {
                        GtkWidget* pImage = gtk_image_new_from_pixbuf(pixbuf);
                        g_object_unref(pixbuf);
                        gtk_tool_button_set_icon_widget(pToolButton, pImage);
                        gtk_widget_show(pImage);
                    }
                }
            }

            // if no tooltip reuse the label as default tooltip
            if (!gtk_widget_get_tooltip_text(pWidget))
            {
                if (const gchar* label = gtk_tool_button_get_label(pToolButton))
                    gtk_widget_set_tooltip_text(pWidget, label);
            }
        }
#else
        else if (GTK_IS_BUTTON(pWidget))
        {
            GtkButton* pButton = GTK_BUTTON(pWidget);
            if (const gchar* icon_name = gtk_button_get_icon_name(pButton))
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);
                if (!IsAllowedBuiltInIcon(aIconName))
                {
                    if (GdkPixbuf* pixbuf = load_icon_by_name_theme_lang(aIconName, m_aIconTheme, m_aUILang))
                    {
                        GtkWidget* pImage = gtk_image_new_from_pixbuf(pixbuf);
                        gtk_widget_set_halign(pImage, GTK_ALIGN_CENTER);
                        gtk_widget_set_valign(pImage, GTK_ALIGN_CENTER);
                        g_object_unref(pixbuf);
                        gtk_button_set_child(pButton, pImage);
                        gtk_widget_show(pImage);
                    }
                }
            }
        }
        else if (GTK_IS_MENU_BUTTON(pWidget))
        {
            GtkMenuButton* pButton = GTK_MENU_BUTTON(pWidget);
            if (const gchar* icon_name = gtk_menu_button_get_icon_name(pButton))
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);
                if (!IsAllowedBuiltInIcon(aIconName))
                {
                    if (GdkPixbuf* pixbuf = load_icon_by_name_theme_lang(aIconName, m_aIconTheme, m_aUILang))
                    {
                        GtkWidget* pImage = gtk_image_new_from_pixbuf(pixbuf);
                        gtk_widget_set_halign(pImage, GTK_ALIGN_CENTER);
                        gtk_widget_set_valign(pImage, GTK_ALIGN_CENTER);
                        g_object_unref(pixbuf);
                        // TODO after gtk 4.6 is released require that version and drop this
                        static auto menu_button_set_child = reinterpret_cast<void (*) (GtkMenuButton*, GtkWidget*)>(dlsym(nullptr, "gtk_menu_button_set_child"));
                        if (menu_button_set_child)
                            menu_button_set_child(pButton, pImage);
                        gtk_widget_show(pImage);
                    }
                }
            }
        }
#endif

        //set helpids
        OString sBuildableName = ::get_buildable_id(GTK_BUILDABLE(pWidget));
        if (!sBuildableName.isEmpty())
        {
            OString sHelpId = m_aUtf8HelpRoot + sBuildableName;
            set_help_id(pWidget, sHelpId);
            //hook up for extended help
            const ImplSVHelpData& aHelpData = ImplGetSVHelpData();
            if (aHelpData.mbBalloonHelp && !GTK_IS_DIALOG(pWidget) && !GTK_IS_ASSISTANT(pWidget))
            {
                gtk_widget_set_has_tooltip(pWidget, true);
                g_signal_connect(pWidget, "query-tooltip", G_CALLBACK(signalTooltipQuery), nullptr);
            }

            if (bHideHelp && sBuildableName == "help")
                gtk_widget_hide(pWidget);
        }

        if (m_pStringReplace)
        {
            // tdf#136498 %PRODUCTNAME shown in tool tips
            const char* pTooltip = gtk_widget_get_tooltip_text(pWidget);
            if (pTooltip && pTooltip[0])
            {
                OUString aTooltip(pTooltip, strlen(pTooltip), RTL_TEXTENCODING_UTF8);
                aTooltip = (*m_pStringReplace)(aTooltip);
                gtk_widget_set_tooltip_text(pWidget, OUStringToOString(aTooltip, RTL_TEXTENCODING_UTF8).getStr());
            }

#if !GTK_CHECK_VERSION(4, 0, 0)
            // tdf#142704 %PRODUCTNAME shown in extended tips
            AtkObject* pAtkObject = gtk_widget_get_accessible(pWidget);
            const char* pDesc = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
            if (pDesc && pDesc[0])
            {
                OUString aDesc(pDesc, strlen(pDesc), RTL_TEXTENCODING_UTF8);
                aDesc = (*m_pStringReplace)(aDesc);
                atk_object_set_description(pAtkObject, OUStringToOString(aDesc, RTL_TEXTENCODING_UTF8).getStr());
            }
#endif
        }

        // expand placeholder and collect potentially missing mnemonics
        if (GTK_IS_BUTTON(pWidget))
        {
            GtkButton* pButton = GTK_BUTTON(pWidget);
            if (m_pStringReplace)
            {
                OUString aLabel(button_get_label(pButton));
                if (!aLabel.isEmpty())
                    button_set_label(pButton, (*m_pStringReplace)(aLabel));
            }
            if (gtk_button_get_use_underline(pButton))
                m_aMnemonicButtons.push_back(pButton);
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        else if (GTK_IS_CHECK_BUTTON(pWidget))
        {
            GtkCheckButton* pButton = GTK_CHECK_BUTTON(pWidget);
            if (m_pStringReplace)
            {
                OUString aLabel(get_label(pButton));
                if (!aLabel.isEmpty())
                    set_label(pButton, (*m_pStringReplace)(aLabel));
            }
            if (gtk_check_button_get_use_underline(pButton))
                m_aMnemonicCheckButtons.push_back(pButton);
        }
#endif
        else if (GTK_IS_LABEL(pWidget))
        {
            GtkLabel* pLabel = GTK_LABEL(pWidget);
            if (m_pStringReplace)
            {
                OUString aLabel(get_label(pLabel));
                if (!aLabel.isEmpty())
                    set_label(pLabel, (*m_pStringReplace)(aLabel));
            }
            if (gtk_label_get_use_underline(pLabel))
                m_aMnemonicLabels.push_back(pLabel);
        }
        else if (GTK_IS_TEXT_VIEW(pWidget))
        {
            GtkTextView* pTextView = GTK_TEXT_VIEW(pWidget);
            if (m_pStringReplace)
            {
                GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(pTextView);
                GtkTextIter start, end;
                gtk_text_buffer_get_bounds(pBuffer, &start, &end);
                char* pTextStr = gtk_text_buffer_get_text(pBuffer, &start, &end, true);
                int nTextLen = pTextStr ? strlen(pTextStr) : 0;
                if (nTextLen)
                {
                    OUString sOldText(pTextStr, nTextLen, RTL_TEXTENCODING_UTF8);
                    OString sText(OUStringToOString((*m_pStringReplace)(sOldText), RTL_TEXTENCODING_UTF8));
                    gtk_text_buffer_set_text(pBuffer, sText.getStr(), sText.getLength());
                }
                g_free(pTextStr);
            }
        }
#if !GTK_CHECK_VERSION(4, 0, 0)
        else if (GTK_IS_ENTRY(pWidget))
        {
            g_signal_connect(pWidget, "key-press-event", G_CALLBACK(signalEntryInsertSpecialCharKeyPress), nullptr);
        }
#endif
        else if (GTK_IS_WINDOW(pWidget))
        {
            if (m_pStringReplace)
            {
                GtkWindow* pWindow = GTK_WINDOW(pWidget);
                set_title(pWindow, (*m_pStringReplace)(get_title(pWindow)));
                if (GTK_IS_MESSAGE_DIALOG(pWindow))
                {
                    GtkMessageDialog* pMessageDialog = GTK_MESSAGE_DIALOG(pWindow);
                    set_primary_text(pMessageDialog, (*m_pStringReplace)(get_primary_text(pMessageDialog)));
                    set_secondary_text(pMessageDialog, (*m_pStringReplace)(get_secondary_text(pMessageDialog)));
                }
            }
        }
    }

    //GtkBuilder sets translation domain during parse, and unsets it again afterwards.
    //In order for GtkBuilder to find the translations bindtextdomain has to be called
    //for the domain. So here on the first setting of "domain" we call Translate::Create
    //to make sure that happens. Without this, if some other part of LibreOffice has
    //used the translation machinery for this domain it will still work, but if it
    //hasn't, e.g. tdf#119929, then the translation fails
    void translation_domain_set()
    {
        Translate::Create(gtk_builder_get_translation_domain(m_pBuilder), LanguageTag(m_aUILang));
        g_signal_handler_disconnect(m_pBuilder, m_nNotifySignalId);
    }

    static void signalNotify(GObject*, GParamSpec *pSpec, gpointer pData)
    {
        g_return_if_fail(pSpec != nullptr);
        if (strcmp(pSpec->name, "translation-domain") == 0)
        {
            GtkInstanceBuilder* pBuilder = static_cast<GtkInstanceBuilder*>(pData);
            pBuilder->translation_domain_set();
        }
    }

    static void postprocess(gpointer data, gpointer user_data)
    {
        GObject* pObject = static_cast<GObject*>(data);
        if (!GTK_IS_WIDGET(pObject))
            return;
        GtkInstanceBuilder* pThis = static_cast<GtkInstanceBuilder*>(user_data);
        pThis->postprocess_widget(GTK_WIDGET(pObject));
    }

    void DisallowCycleFocusOut()
    {
        assert(!m_bAllowCycleFocusOut); // we only expect this to be called when this holds

        GtkWidget* pTopLevel = widget_get_toplevel(m_pParentWidget);
        assert(pTopLevel);
        GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(pTopLevel);
        assert(pFrame);
        // unhook handler and let gtk cycle its own way through this widget's
        // children because it has no non-gtk siblings
        pFrame->DisallowCycleFocusOut();
    }

    static void signalMap(GtkWidget*, gpointer user_data)
    {
        GtkInstanceBuilder* pThis = static_cast<GtkInstanceBuilder*>(user_data);
        // tdf#138047 wait until map to do this because the final SalFrame may
        // not be the same as at ctor time
        pThis->DisallowCycleFocusOut();
    }

    void AllowCycleFocusOut()
    {
        assert(!m_bAllowCycleFocusOut); // we only expect this to be called when this holds

        GtkWidget* pTopLevel = widget_get_toplevel(m_pParentWidget);
        assert(pTopLevel);
        GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(pTopLevel);
        assert(pFrame);
        // rehook handler and let vcl cycle its own way through this widget's
        // children
        pFrame->AllowCycleFocusOut();

        // tdf#145567 if the focus is in this hierarchy then, now that we are tearing down,
        // move focus to the usual focus candidate for the frame
        GtkWindow* pFocusWin = get_active_window();
        GtkWidget* pFocus = pFocusWin ? gtk_window_get_focus(pFocusWin) : nullptr;
        bool bHasFocus = pFocus && gtk_widget_is_ancestor(pFocus, pTopLevel);
        if (bHasFocus)
            pFrame->GrabFocus();
    }

    static void signalUnmap(GtkWidget*, gpointer user_data)
    {
        GtkInstanceBuilder* pThis = static_cast<GtkInstanceBuilder*>(user_data);
        pThis->AllowCycleFocusOut();
    }

public:
    GtkInstanceBuilder(GtkWidget* pParent, std::u16string_view rUIRoot, const OUString& rUIFile,
                       SystemChildWindow* pInterimGlue, bool bAllowCycleFocusOut)
        : weld::Builder()
        , m_pStringReplace(Translate::GetReadStringHook())
        , m_pParentWidget(pParent)
        , m_nNotifySignalId(0)
        , m_xInterimGlue(pInterimGlue)
        , m_bAllowCycleFocusOut(bAllowCycleFocusOut)
    {
        OUString sHelpRoot(rUIFile);
#if !GTK_CHECK_VERSION(4, 0, 0)
        ensure_intercept_drawing_area_accessibility();
        ensure_disable_ctrl_page_up_down_bindings();
#endif

        sal_Int32 nIdx = sHelpRoot.lastIndexOf('.');
        if (nIdx != -1)
            sHelpRoot = sHelpRoot.copy(0, nIdx);
        sHelpRoot += "/";
        m_aUtf8HelpRoot = OUStringToOString(sHelpRoot, RTL_TEXTENCODING_UTF8);
        m_aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        m_aUILang = Application::GetSettings().GetUILanguageTag().getBcp47();

        OUString aUri(rUIRoot + rUIFile);

        m_pBuilder = gtk_builder_new();
        m_nNotifySignalId = g_signal_connect_data(G_OBJECT(m_pBuilder), "notify", G_CALLBACK(signalNotify), this, nullptr, G_CONNECT_AFTER);

        load_ui_file(m_pBuilder, aUri);

        m_pObjectList = gtk_builder_get_objects(m_pBuilder);
        g_slist_foreach(m_pObjectList, postprocess, this);

        GenerateMissingMnemonics();

        if (m_xInterimGlue)
        {
            assert(m_pParentWidget);
            g_object_set_data(G_OBJECT(m_pParentWidget), "InterimWindowGlue", m_xInterimGlue.get());

            if (!m_bAllowCycleFocusOut)
            {
                g_signal_connect(G_OBJECT(m_pParentWidget), "map", G_CALLBACK(signalMap), this);
                g_signal_connect(G_OBJECT(m_pParentWidget), "unmap", G_CALLBACK(signalUnmap), this);
            }
        }
    }

    void GenerateMissingMnemonics()
    {
        MnemonicGenerator aMnemonicGenerator('_');
        for (const auto a : m_aMnemonicButtons)
            aMnemonicGenerator.RegisterMnemonic(button_get_label(a));
#if GTK_CHECK_VERSION(4, 0, 0)
        for (const auto a : m_aMnemonicCheckButtons)
            aMnemonicGenerator.RegisterMnemonic(get_label(a));
#endif
        for (const auto a : m_aMnemonicLabels)
            aMnemonicGenerator.RegisterMnemonic(get_label(a));

        for (const auto a : m_aMnemonicButtons)
        {
            OUString aLabel(button_get_label(a));
            OUString aNewLabel = aMnemonicGenerator.CreateMnemonic(aLabel);
            if (aLabel == aNewLabel)
                continue;
            button_set_label(a, aNewLabel);
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        for (const auto a : m_aMnemonicCheckButtons)
        {
            OUString aLabel(get_label(a));
            OUString aNewLabel = aMnemonicGenerator.CreateMnemonic(aLabel);
            if (aLabel == aNewLabel)
                continue;
            set_label(a, aNewLabel);
        }
#endif
        for (const auto a : m_aMnemonicLabels)
        {
            OUString aLabel(get_label(a));
            OUString aNewLabel = aMnemonicGenerator.CreateMnemonic(aLabel);
            if (aLabel == aNewLabel)
                continue;
            set_label(a, aNewLabel);
        }

        m_aMnemonicLabels.clear();
#if GTK_CHECK_VERSION(4, 0, 0)
        m_aMnemonicCheckButtons.clear();
#endif
        m_aMnemonicButtons.clear();
    }

    OString get_current_page_help_id()
    {
        OString sPageHelpId;
        // check to see if there is a notebook called tabcontrol and get the
        // helpid for the current page of that
        std::unique_ptr<weld::Notebook> xNotebook(weld_notebook("tabcontrol"));
        if (xNotebook)
        {
            if (GtkInstanceContainer* pPage = dynamic_cast<GtkInstanceContainer*>(xNotebook->get_page(xNotebook->get_current_page_ident())))
            {
                GtkWidget* pContainer = pPage->getWidget();
                if (GtkWidget* pPageWidget = widget_get_first_child(pContainer))
                    sPageHelpId = ::get_help_id(pPageWidget);
            }
        }
        return sPageHelpId;
    }

    virtual ~GtkInstanceBuilder() override
    {
        g_slist_free(m_pObjectList);
        g_object_unref(m_pBuilder);

        if (m_xInterimGlue && !m_bAllowCycleFocusOut)
            AllowCycleFocusOut();

        m_xInterimGlue.disposeAndClear();
    }

    //ideally we would have/use weld::Container add and explicitly
    //call add when we want to do this, but in the vcl impl the
    //parent has to be set when the child is created, so for the
    //gtk impl emulate this by doing this implicitly at weld time
    void auto_add_parentless_widgets_to_container(GtkWidget* pWidget)
    {
        if (GTK_IS_POPOVER(pWidget))
           return;
        if (GTK_IS_WINDOW(pWidget))
            return;
#if GTK_CHECK_VERSION(4, 0, 0)
        if (!gtk_widget_get_parent(pWidget))
            gtk_widget_set_parent(pWidget, m_pParentWidget);
#else
        if (widget_get_toplevel(pWidget) == pWidget)
            gtk_container_add(GTK_CONTAINER(m_pParentWidget), pWidget);
#endif
    }

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString &id) override
    {
        GtkMessageDialog* pMessageDialog = GTK_MESSAGE_DIALOG(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pMessageDialog)
            return nullptr;
        gtk_window_set_transient_for(GTK_WINDOW(pMessageDialog), GTK_WINDOW(widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceMessageDialog>(pMessageDialog, this, true);
    }

    virtual std::unique_ptr<weld::Assistant> weld_assistant(const OString &id) override
    {
        GtkAssistant* pAssistant = GTK_ASSISTANT(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pAssistant)
            return nullptr;
        if (m_pParentWidget)
            gtk_window_set_transient_for(GTK_WINDOW(pAssistant), GTK_WINDOW(widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceAssistant>(pAssistant, this, true);
    }

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString &id) override
    {
        GtkWindow* pDialog = GTK_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pDialog)
            return nullptr;
        if (m_pParentWidget)
            gtk_window_set_transient_for(pDialog, GTK_WINDOW(widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceDialog>(pDialog, this, true);
    }

    virtual std::unique_ptr<weld::Window> create_screenshot_window() override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pTopLevel = nullptr;

        for (GSList* l = m_pObjectList; l; l = g_slist_next(l))
        {
            GObject* pObj = static_cast<GObject*>(l->data);

            if (!GTK_IS_WIDGET(pObj) || gtk_widget_get_parent(GTK_WIDGET(pObj)))
                continue;

            if (!pTopLevel)
                pTopLevel = GTK_WIDGET(pObj);
            else if (GTK_IS_WINDOW(pObj))
                pTopLevel = GTK_WIDGET(pObj);
        }

        if (!pTopLevel)
            return nullptr;

        GtkWindow* pDialog;
        if (GTK_IS_WINDOW(pTopLevel))
            pDialog = GTK_WINDOW(pTopLevel);
        else
        {
            pDialog = GTK_WINDOW(gtk_dialog_new());
            ::set_help_id(GTK_WIDGET(pDialog), ::get_help_id(pTopLevel));

            GtkWidget* pContentArea = gtk_dialog_get_content_area(GTK_DIALOG(pDialog));
            gtk_container_add(GTK_CONTAINER(pContentArea), pTopLevel);
            gtk_widget_show_all(pTopLevel);
        }

        if (m_pParentWidget)
            gtk_window_set_transient_for(pDialog, GTK_WINDOW(widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceDialog>(pDialog, this, true);
#else
        return nullptr;
#endif
    }

    virtual std::unique_ptr<weld::Widget> weld_widget(const OString &id) override
    {
        GtkWidget* pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pWidget)
            return nullptr;
        auto_add_parentless_widgets_to_container(pWidget);
        return std::make_unique<GtkInstanceWidget>(pWidget, this, false);
    }

    virtual std::unique_ptr<weld::Container> weld_container(const OString &id) override
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, id.getStr()));
#else
        GtkWidget* pContainer = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, id.getStr()));
#endif
        if (!pContainer)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pContainer));
        return std::make_unique<GtkInstanceContainer>(pContainer, this, false);
    }

    virtual std::unique_ptr<weld::Box> weld_box(const OString &id) override
    {
        GtkBox* pBox = GTK_BOX(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pBox)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pBox));
        return std::make_unique<GtkInstanceBox>(pBox, this, false);
    }

    virtual std::unique_ptr<weld::Paned> weld_paned(const OString &id) override
    {
        GtkPaned* pPaned = GTK_PANED(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pPaned)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pPaned));
        return std::make_unique<GtkInstancePaned>(pPaned, this, false);
    }

    virtual std::unique_ptr<weld::Frame> weld_frame(const OString &id) override
    {
        GtkFrame* pFrame = GTK_FRAME(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pFrame)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pFrame));
        return std::make_unique<GtkInstanceFrame>(pFrame, this, false);
    }

    virtual std::unique_ptr<weld::ScrolledWindow> weld_scrolled_window(const OString &id, bool bUserManagedScrolling = false) override
    {
        GtkScrolledWindow* pScrolledWindow = GTK_SCROLLED_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pScrolledWindow)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pScrolledWindow));
        return std::make_unique<GtkInstanceScrolledWindow>(pScrolledWindow, this, false, bUserManagedScrolling);
    }

    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString &id) override
    {
        GtkNotebook* pNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pNotebook)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pNotebook));
        return std::make_unique<GtkInstanceNotebook>(pNotebook, this, false);
    }

    virtual std::unique_ptr<weld::Button> weld_button(const OString &id) override
    {
        GtkButton* pButton = GTK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return std::make_unique<GtkInstanceButton>(pButton, this, false);
    }

    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString &id) override
    {
        GtkMenuButton* pButton = GTK_MENU_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return std::make_unique<GtkInstanceMenuButton>(pButton, nullptr, this, false);
    }

    virtual std::unique_ptr<weld::MenuToggleButton> weld_menu_toggle_button(const OString &id) override
    {
        GtkMenuButton* pButton = GTK_MENU_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        // gtk doesn't come with exactly the same concept
        GtkBuilder* pMenuToggleButton = makeMenuToggleButtonBuilder();
        return std::make_unique<GtkInstanceMenuToggleButton>(pMenuToggleButton, pButton, this, false);
    }

    virtual std::unique_ptr<weld::LinkButton> weld_link_button(const OString &id) override
    {
        GtkLinkButton* pButton = GTK_LINK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return std::make_unique<GtkInstanceLinkButton>(pButton, this, false);
    }

    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OString &id) override
    {
        GtkToggleButton* pToggleButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pToggleButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pToggleButton));
        return std::make_unique<GtkInstanceToggleButton>(pToggleButton, this, false);
    }

    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString &id) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkCheckButton* pRadioButton = GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
#else
        GtkRadioButton* pRadioButton = GTK_RADIO_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
#endif
        if (!pRadioButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pRadioButton));
        return std::make_unique<GtkInstanceRadioButton>(pRadioButton, this, false);
    }

    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString &id) override
    {
        GtkCheckButton* pCheckButton = GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pCheckButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pCheckButton));
        return std::make_unique<GtkInstanceCheckButton>(pCheckButton, this, false);
    }

    virtual std::unique_ptr<weld::Scale> weld_scale(const OString &id) override
    {
        GtkScale* pScale = GTK_SCALE(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pScale)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pScale));
        return std::make_unique<GtkInstanceScale>(pScale, this, false);
    }

    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OString &id) override
    {
        GtkProgressBar* pProgressBar = GTK_PROGRESS_BAR(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pProgressBar)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pProgressBar));
        return std::make_unique<GtkInstanceProgressBar>(pProgressBar, this, false);
    }

    virtual std::unique_ptr<weld::Spinner> weld_spinner(const OString &id) override
    {
        GtkSpinner* pSpinner = GTK_SPINNER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pSpinner)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pSpinner));
        return std::make_unique<GtkInstanceSpinner>(pSpinner, this, false);
    }

    virtual std::unique_ptr<weld::Image> weld_image(const OString &id) override
    {
        GtkWidget* pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pWidget)
            return nullptr;
        if (GTK_IS_IMAGE(pWidget))
        {
            auto_add_parentless_widgets_to_container(pWidget);
            return std::make_unique<GtkInstanceImage>(GTK_IMAGE(pWidget), this, false);
        }
#if GTK_CHECK_VERSION(4, 0, 0)
        if (GTK_IS_PICTURE(pWidget))
        {
            auto_add_parentless_widgets_to_container(pWidget);
            return std::make_unique<GtkInstancePicture>(GTK_PICTURE(pWidget), this, false);
        }
#endif
        return nullptr;
    }

    virtual std::unique_ptr<weld::Calendar> weld_calendar(const OString &id) override
    {
        GtkCalendar* pCalendar = GTK_CALENDAR(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pCalendar)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pCalendar));
        return std::make_unique<GtkInstanceCalendar>(pCalendar, this, false);
    }

    virtual std::unique_ptr<weld::Entry> weld_entry(const OString &id) override
    {
        GtkEntry* pEntry = GTK_ENTRY(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pEntry)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pEntry));
        return std::make_unique<GtkInstanceEntry>(pEntry, this, false);
    }

    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString &id) override
    {
        GtkSpinButton* pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pSpinButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pSpinButton));
        return std::make_unique<GtkInstanceSpinButton>(pSpinButton, this, false);
    }

    virtual std::unique_ptr<weld::MetricSpinButton> weld_metric_spin_button(const OString& id, FieldUnit eUnit) override
    {
        return std::make_unique<weld::MetricSpinButton>(weld_spin_button(id), eUnit);
    }

    virtual std::unique_ptr<weld::FormattedSpinButton> weld_formatted_spin_button(const OString &id) override
    {
        GtkSpinButton* pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pSpinButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pSpinButton));
        return std::make_unique<GtkInstanceFormattedSpinButton>(pSpinButton, this, false);
    }

    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString &id) override
    {
        GtkComboBox* pComboBox = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pComboBox)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pComboBox));

#if GTK_CHECK_VERSION(4, 0, 0)
        return std::make_unique<GtkInstanceComboBox>(pComboBox, this, false);
#else
        /* we replace GtkComboBox because of difficulties with too tall menus

           1) https://gitlab.gnome.org/GNOME/gtk/issues/1910
              has_entry long menus take forever to appear (tdf#125388)

              on measuring each row, the GtkComboBox GtkTreeMenu will call
              its area_apply_attributes_cb function on the row, but that calls
              gtk_tree_menu_get_path_item which then loops through each child of the
              menu looking for the widget of the row, so performance drops to useless.

              All area_apply_attributes_cb does it set menu item sensitivity, so block it from running
              with fragile hackery which assumes that the unwanted callback is the only one with a

           2) https://gitlab.gnome.org/GNOME/gtk/issues/94
              when a super tall combobox menu is activated, and the selected
              entry is sufficiently far down the list, then the menu doesn't
              appear under wayland

           3) https://gitlab.gnome.org/GNOME/gtk/issues/310
              no typeahead support

           4) we want to be able to control the width of the button, but have a drop down menu which
              is not limited to the width of the button

           5) https://bugs.documentfoundation.org/show_bug.cgi?id=131120
              super tall menu doesn't appear under X sometimes
        */
        GtkBuilder* pComboBuilder = makeComboBoxBuilder();
        return std::make_unique<GtkInstanceComboBox>(pComboBuilder, pComboBox, this, false);
#endif
    }

    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString &id) override
    {
        GtkTreeView* pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pTreeView)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pTreeView));
        return std::make_unique<GtkInstanceTreeView>(pTreeView, this, false);
    }

    virtual std::unique_ptr<weld::IconView> weld_icon_view(const OString &id) override
    {
        GtkIconView* pIconView = GTK_ICON_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pIconView)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pIconView));
        return std::make_unique<GtkInstanceIconView>(pIconView, this, false);
    }

    virtual std::unique_ptr<weld::EntryTreeView> weld_entry_tree_view(const OString& containerid, const OString& entryid, const OString& treeviewid) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkWidget* pContainer = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, containerid.getStr()));
#else
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, containerid.getStr()));
#endif
        if (!pContainer)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pContainer));
        return std::make_unique<GtkInstanceEntryTreeView>(pContainer, this, false,
                                                          weld_entry(entryid),
                                                          weld_tree_view(treeviewid));
    }

    virtual std::unique_ptr<weld::Label> weld_label(const OString &id) override
    {
        GtkLabel* pLabel = GTK_LABEL(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pLabel)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pLabel));
        return std::make_unique<GtkInstanceLabel>(pLabel, this, false);
    }

    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString &id) override
    {
        GtkTextView* pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pTextView)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pTextView));
        return std::make_unique<GtkInstanceTextView>(pTextView, this, false);
    }

    virtual std::unique_ptr<weld::Expander> weld_expander(const OString &id) override
    {
        GtkExpander* pExpander = GTK_EXPANDER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pExpander)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pExpander));
        return std::make_unique<GtkInstanceExpander>(pExpander, this, false);
    }

    virtual std::unique_ptr<weld::DrawingArea> weld_drawing_area(const OString &id, const a11yref& rA11y,
            FactoryFunction /*pUITestFactoryFunction*/, void* /*pUserData*/) override
    {
        GtkDrawingArea* pDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pDrawingArea)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pDrawingArea));
        return std::make_unique<GtkInstanceDrawingArea>(pDrawingArea, this, rA11y, false);
    }

    virtual std::unique_ptr<weld::Menu> weld_menu(const OString &id) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkPopoverMenu* pMenu = GTK_POPOVER_MENU(gtk_builder_get_object(m_pBuilder, id.getStr()));
#else
        GtkMenu* pMenu = GTK_MENU(gtk_builder_get_object(m_pBuilder, id.getStr()));
#endif
        if (!pMenu)
            return nullptr;
        return std::make_unique<GtkInstanceMenu>(pMenu, true);
    }

    virtual std::unique_ptr<weld::Popover> weld_popover(const OString &id) override
    {
        GtkPopover* pPopover = GTK_POPOVER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pPopover)
            return nullptr;
#if GTK_CHECK_VERSION(4, 0, 0)
        return std::make_unique<GtkInstancePopover>(pPopover, this, false);
#else
        return std::make_unique<GtkInstancePopover>(pPopover, this, true);
#endif
    }

    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OString &id) override
    {
#if GTK_CHECK_VERSION(4, 0, 0)
        GtkBox* pToolbar = GTK_BOX(gtk_builder_get_object(m_pBuilder, id.getStr()));
#else
        GtkToolbar* pToolbar = GTK_TOOLBAR(gtk_builder_get_object(m_pBuilder, id.getStr()));
#endif
        if (!pToolbar)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pToolbar));
        return std::make_unique<GtkInstanceToolbar>(pToolbar, this, false);
    }

    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override
    {
        return std::make_unique<GtkInstanceSizeGroup>();
    }
};

}

void GtkInstanceWindow::help()
{
    //show help for widget with keyboard focus
    GtkWidget* pWidget = gtk_window_get_focus(m_pWindow);
    if (!pWidget)
        pWidget = GTK_WIDGET(m_pWindow);
    OString sHelpId = ::get_help_id(pWidget);
    while (sHelpId.isEmpty())
    {
        pWidget = gtk_widget_get_parent(pWidget);
        if (!pWidget)
            break;
        sHelpId = ::get_help_id(pWidget);
    }
    std::unique_ptr<weld::Widget> xTemp(pWidget != m_pWidget ? new GtkInstanceWidget(pWidget, m_pBuilder, false) : nullptr);
    weld::Widget* pSource = xTemp ? xTemp.get() : this;
    bool bRunNormalHelpRequest = !m_aHelpRequestHdl.IsSet() || m_aHelpRequestHdl.Call(*pSource);
    Help* pHelp = bRunNormalHelpRequest ? Application::GetHelp() : nullptr;
    if (!pHelp)
        return;

#if !GTK_CHECK_VERSION(4, 0, 0)
    // tdf#126007, there's a nice fallback route for offline help where
    // the current page of a notebook will get checked when the help
    // button is pressed and there was no help for the dialog found.
    //
    // But for online help that route doesn't get taken, so bodge this here
    // by using the page help id if available and if the help button itself
    // was the original id
    if (m_pBuilder && sHelpId.endsWith("/help"))
    {
        OString sPageId = m_pBuilder->get_current_page_help_id();
        if (!sPageId.isEmpty())
            sHelpId = sPageId;
        else
        {
            // tdf#129068 likewise the help for the wrapping dialog is less
            // helpful than the help for the content area could be
            GtkContainer* pContainer = nullptr;
            if (GTK_IS_DIALOG(m_pWindow))
                pContainer = GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(m_pWindow)));
            else if (GTK_IS_ASSISTANT(m_pWindow))
            {
                GtkAssistant* pAssistant = GTK_ASSISTANT(m_pWindow);
                pContainer = GTK_CONTAINER(gtk_assistant_get_nth_page(pAssistant, gtk_assistant_get_current_page(pAssistant)));
            }
            if (pContainer)
            {
                GtkWidget* pContentWidget = widget_get_first_child(GTK_WIDGET(pContainer));
                if (pContentWidget)
                    sHelpId = ::get_help_id(pContentWidget);
            }
        }
    }
#endif
    pHelp->Start(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), pSource);
}

//iterate upwards through the hierarchy from this widgets through its parents
//calling func with their helpid until func returns true or we run out of parents
void GtkInstanceWidget::help_hierarchy_foreach(const std::function<bool(const OString&)>& func)
{
    GtkWidget* pParent = m_pWidget;
    while ((pParent = gtk_widget_get_parent(pParent)))
    {
        if (func(::get_help_id(pParent)))
            return;
    }
}

std::unique_ptr<weld::Builder> GtkInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    GtkInstanceWidget* pParentWidget = dynamic_cast<GtkInstanceWidget*>(pParent);
    GtkWidget* pBuilderParent = pParentWidget ? pParentWidget->getWidget() : nullptr;
    return std::make_unique<GtkInstanceBuilder>(pBuilderParent, rUIRoot, rUIFile, nullptr, true);
}

#if !GTK_CHECK_VERSION(4, 0, 0)
// tdf#135965 for the case of native widgets inside a GtkSalFrame and F1 pressed, run help
// on gtk widget help ids until we hit a vcl parent and then use vcl window help ids
gboolean GtkSalFrame::NativeWidgetHelpPressed(GtkAccelGroup*, GObject*, guint, GdkModifierType, gpointer pFrame)
{
    Help* pHelp = Application::GetHelp();
    if (!pHelp)
        return true;

    GtkWindow* pWindow = static_cast<GtkWindow*>(pFrame);

    vcl::Window* pChildWindow = nullptr;

    //show help for widget with keyboard focus
    GtkWidget* pWidget = gtk_window_get_focus(pWindow);
    if (!pWidget)
        pWidget = GTK_WIDGET(pWindow);
    OString sHelpId = ::get_help_id(pWidget);
    while (sHelpId.isEmpty())
    {
        pWidget = gtk_widget_get_parent(pWidget);
        if (!pWidget)
            break;
        pChildWindow = static_cast<vcl::Window*>(g_object_get_data(G_OBJECT(pWidget), "InterimWindowGlue"));
        if (pChildWindow)
        {
            sHelpId = pChildWindow->GetHelpId();
            break;
        }
        sHelpId = ::get_help_id(pWidget);
    }

    if (pChildWindow)
    {
        while (sHelpId.isEmpty())
        {
            pChildWindow = pChildWindow->GetParent();
            if (!pChildWindow)
                break;
            sHelpId = pChildWindow->GetHelpId();
        }
        if (!pChildWindow)
            return true;
        pHelp->Start(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), pChildWindow);
        return true;
    }

    if (!pWidget)
        return true;
    std::unique_ptr<weld::Widget> xTemp(new GtkInstanceWidget(pWidget, nullptr, false));
    pHelp->Start(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), xTemp.get());
    return true;
}
#endif

std::unique_ptr<weld::Builder> GtkInstance::CreateInterimBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                                                 bool bAllowCycleFocusOut, sal_uInt64)
{
    // Create a foreign window which we know is a GtkGrid and make the native widgets a child of that, so we can
    // support GtkWidgets within a vcl::Window
    SystemWindowData winData = {};
    winData.bClipUsingNativeWidget = true;
    auto xEmbedWindow = VclPtr<SystemChildWindow>::Create(pParent, 0, &winData, false);
    xEmbedWindow->Show(true, ShowFlags::NoActivate);
    xEmbedWindow->set_expand(true);

    const SystemEnvData* pEnvData = xEmbedWindow->GetSystemData();
    if (!pEnvData)
        return nullptr;

    GtkWidget *pWindow = static_cast<GtkWidget*>(pEnvData->pWidget);
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_show_all(pWindow);
#else
    gtk_widget_show(pWindow);
#endif

    // build the widget tree as a child of the GtkEventBox GtkGrid parent
    return std::make_unique<GtkInstanceBuilder>(pWindow, rUIRoot, rUIFile, xEmbedWindow.get(), bAllowCycleFocusOut);
}

weld::MessageDialog* GtkInstance::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType, VclButtonsType eButtonsType, const OUString &rPrimaryMessage)
{
    GtkInstanceWidget* pParentInstance = dynamic_cast<GtkInstanceWidget*>(pParent);
    GtkWindow* pParentWindow = pParentInstance ? pParentInstance->getWindow() : nullptr;
    GtkMessageDialog* pMessageDialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(pParentWindow, GTK_DIALOG_MODAL,
                                                          VclToGtk(eMessageType), VclToGtk(eButtonsType), "%s",
                                                          OUStringToOString(rPrimaryMessage, RTL_TEXTENCODING_UTF8).getStr()));
    return new GtkInstanceMessageDialog(pMessageDialog, nullptr, true);
}

weld::Window* GtkInstance::GetFrameWeld(const css::uno::Reference<css::awt::XWindow>& rWindow)
{
    if (SalGtkXWindow* pGtkXWindow = dynamic_cast<SalGtkXWindow*>(rWindow.get()))
        return pGtkXWindow->getFrameWeld();
    return SalInstance::GetFrameWeld(rWindow);
}

weld::Window* GtkSalFrame::GetFrameWeld() const
{
    if (!m_xFrameWeld)
        m_xFrameWeld.reset(new GtkInstanceWindow(GTK_WINDOW(widget_get_toplevel(getWindow())), nullptr, false));
    return m_xFrameWeld.get();
}

void* GtkInstance::CreateGStreamerSink(const SystemChildWindow *pWindow)
{
#if ENABLE_GSTREAMER_1_0
    auto aSymbol = gstElementFactoryNameSymbol();
    if (!aSymbol)
        return nullptr;

    const SystemEnvData* pEnvData = pWindow->GetSystemData();
    if (!pEnvData)
        return nullptr;

    GstElement* pVideosink = aSymbol("gtksink", "gtksink");
    if (!pVideosink)
        return nullptr;

    GtkWidget *pGstWidget;
    g_object_get(pVideosink, "widget", &pGstWidget, nullptr);
    gtk_widget_set_vexpand(pGstWidget, true);
    gtk_widget_set_hexpand(pGstWidget, true);

    GtkWidget *pParent = static_cast<GtkWidget*>(pEnvData->pWidget);
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_container_add(GTK_CONTAINER(pParent), pGstWidget);
#endif
    g_object_unref(pGstWidget);
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_show_all(pParent);
#else
    gtk_widget_show(pParent);
#endif

    return pVideosink;
#else
    (void)pWindow;
    return nullptr;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
