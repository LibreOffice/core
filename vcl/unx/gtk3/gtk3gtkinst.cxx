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
#include <vcl/inputtypes.hxx>
#include <vcl/transfer.hxx>
#include <unx/genpspgraphics.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <rtl/uri.hxx>

#include <vcl/settings.hxx>

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#include <unx/gtk/gtkprintwrapper.hxx>

#include "a11y/atkwrapper.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/string.hxx>
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

        if (gtk_major_version == 3 && gtk_minor_version < 18)
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

GtkInstance::GtkInstance( std::unique_ptr<SalYieldMutex> pMutex )
    : SvpSalInstance( std::move(pMutex) )
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
#else
    pSVData->maAppData.mxToolkitName = OUString("gtk3");
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
    PspSalInfoPrinter* pPrinter = new GtkSalInfoPrinter;
    configurePspInfoPrinter(pPrinter, pQueueInfo, pSetupData);
    return pPrinter;
}

std::unique_ptr<SalPrinter> GtkInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    EnsureInit();
    mbPrinterInit = true;
    return std::unique_ptr<SalPrinter>(new GtkSalPrinter( pInfoPrinter ));
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
        assert(n > 0);
        n--;
        if (n > 0)
            acquire(n);
    }
}

void GtkYieldMutex::ThreadsLeave()
{
    assert(m_nCount != 0);
    yieldCounts.push(m_nCount);
    release(true);
}

std::unique_ptr<SalVirtualDevice> GtkInstance::CreateVirtualDevice( SalGraphics *pG,
                                                    tools::Long &nDX, tools::Long &nDY,
                                                    DeviceFormat eFormat,
                                                    const SystemGraphicsData* pGd )
{
    EnsureInit();
    SvpSalGraphics *pSvpSalGraphics = dynamic_cast<SvpSalGraphics*>(pG);
    assert(pSvpSalGraphics);
    // tdf#127529 see SvpSalInstance::CreateVirtualDevice for the rare case of a non-null pPreExistingTarget
    cairo_surface_t* pPreExistingTarget = pGd ? static_cast<cairo_surface_t*>(pGd->pSurface) : nullptr;
    std::unique_ptr<SalVirtualDevice> pNew(new SvpSalVirtualDevice(eFormat, pSvpSalGraphics->getSurface(), pPreExistingTarget));
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

bool GtkInstance::AnyInput( VclInputFlags nType )
{
    EnsureInit();
    if( (nType & VclInputFlags::TIMER) && IsTimerExpired() )
        return true;
    if (!gdk_events_pending())
        return false;

    if (nType == VCL_INPUT_ANY)
        return true;

    bool bRet = false;
    std::deque<GdkEvent*> aEvents;
    GdkEvent *pEvent = nullptr;
    while ((pEvent = gdk_event_get()))
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
        gdk_event_put(pEvent);
        gdk_event_free(pEvent);
        aEvents.pop_front();
    }
    return bRet;
}

std::unique_ptr<GenPspGraphics> GtkInstance::CreatePrintGraphics()
{
    EnsureInit();
    return std::make_unique<GenPspGraphics>();
}

std::shared_ptr<vcl::unx::GtkPrintWrapper> const &
GtkInstance::getPrintWrapper() const
{
    if (!m_xPrintWrapper)
        m_xPrintWrapper = std::make_shared<vcl::unx::GtkPrintWrapper>();
    return m_xPrintWrapper;
}

const cairo_font_options_t* GtkInstance::GetCairoFontOptions()
{
    const cairo_font_options_t* pCairoFontOptions = gdk_screen_get_font_options(gdk_screen_get_default());
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

std::vector<css::datatransfer::DataFlavor> GtkTransferable::getTransferDataFlavorsAsVector(GdkAtom *targets, gint n_targets)
{
    std::vector<css::datatransfer::DataFlavor> aVector;

    bool bHaveText = false, bHaveUTF16 = false;

    for (gint i = 0; i < n_targets; ++i)
    {
        gchar* pName = gdk_atom_name(targets[i]);
        const char* pFinalName = pName;
        css::datatransfer::DataFlavor aFlavor;

        // omit text/plain;charset=unicode since it is not well defined
        if (rtl_str_compare(pName, "text/plain;charset=unicode") == 0)
        {
            g_free(pName);
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
            g_free(pName);
            continue;
        }

        aFlavor.MimeType = OUString(pFinalName,
                                    strlen(pFinalName),
                                    RTL_TEXTENCODING_UTF8);

        m_aMimeTypeToAtom[aFlavor.MimeType] = targets[i];

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
        g_free(pName);
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

namespace {

class GtkClipboardTransferable : public GtkTransferable
{
private:
    GdkAtom m_nSelection;
public:

    explicit GtkClipboardTransferable(GdkAtom nSelection)
        : m_nSelection(nSelection)
    {
    }

    /*
     * XTransferable
     */

    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override
    {
        GtkClipboard* clipboard = gtk_clipboard_get(m_nSelection);
        if (rFlavor.MimeType == "text/plain;charset=utf-16")
        {
            OUString aStr;
            gchar *pText = gtk_clipboard_wait_for_text(clipboard);
            if (pText)
                aStr = OUString(pText, strlen(pText), RTL_TEXTENCODING_UTF8);
            g_free(pText);
            css::uno::Any aRet;
            aRet <<= aStr.replaceAll("\r\n", "\n");
            return aRet;
        }

        auto it = m_aMimeTypeToAtom.find(rFlavor.MimeType);
        if (it == m_aMimeTypeToAtom.end())
            return css::uno::Any();

        css::uno::Any aRet;
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
        return aRet;
    }

    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector()
        override
    {
        std::vector<css::datatransfer::DataFlavor> aVector;

        GtkClipboard* clipboard = gtk_clipboard_get(m_nSelection);

        GdkAtom *targets;
        gint n_targets;
        if (gtk_clipboard_wait_for_targets(clipboard, &targets, &n_targets))
        {
            aVector = GtkTransferable::getTransferDataFlavorsAsVector(targets, n_targets);
            g_free(targets);
        }

        return aVector;
    }
};

class VclGtkClipboard :
        public cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        datatransfer::clipboard::XFlushableClipboard,
        XServiceInfo>
{
    GdkAtom                                                  m_nSelection;
    osl::Mutex                                               m_aMutex;
    gulong                                                   m_nOwnerChangedSignalId;
    ImplSVEvent*                                             m_pSetClipboardEvent;
    Reference<css::datatransfer::XTransferable>              m_aContents;
    Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector< Reference<css::datatransfer::clipboard::XClipboardListener> > m_aListeners;
    std::vector<GtkTargetEntry> m_aGtkTargets;
    VclToGtkHelper m_aConversionHelper;

    DECL_LINK(AsyncSetGtkClipboard, void*, void);
public:

    explicit VclGtkClipboard(GdkAtom nSelection);
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

    void ClipboardGet(GtkSelectionData *selection_data, guint info);
    void ClipboardClear();
    void OwnerPossiblyChanged(GtkClipboard *clipboard);
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
        m_aContents = new GtkClipboardTransferable(m_nSelection);
    }

    return m_aContents;
}

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
}

namespace
{
    void ClipboardGetFunc(GtkClipboard* /*clipboard*/, GtkSelectionData *selection_data,
                          guint info,
                          gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardGet(selection_data, info);
    }

    void ClipboardClearFunc(GtkClipboard* /*clipboard*/, gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardClear();
    }

    void handle_owner_change(GtkClipboard *clipboard, GdkEvent* /*event*/, gpointer user_data)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data);
        pThis->OwnerPossiblyChanged(clipboard);
    }
}

void VclGtkClipboard::OwnerPossiblyChanged(GtkClipboard* clipboard)
{
    SyncGtkClipboard(); // tdf#138183 do any pending SetGtkClipboard calls
    if (!m_aContents.is())
        return;

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
    for (auto &a : m_aGtkTargets)
        g_free(a.target);
    m_aGtkTargets.clear();
}

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

VclGtkClipboard::VclGtkClipboard(GdkAtom nSelection)
    : cppu::WeakComponentImplHelper<datatransfer::clipboard::XSystemClipboard,
                                    datatransfer::clipboard::XFlushableClipboard, XServiceInfo>
        (m_aMutex)
    , m_nSelection(nSelection)
    , m_pSetClipboardEvent(nullptr)
{
    GtkClipboard* clipboard = gtk_clipboard_get(m_nSelection);
    m_nOwnerChangedSignalId = g_signal_connect(clipboard, "owner-change",
                                               G_CALLBACK(handle_owner_change), this);
}

void VclGtkClipboard::flushClipboard()
{
    SolarMutexGuard aGuard;

    if (GDK_SELECTION_CLIPBOARD != m_nSelection)
        return;

    GtkClipboard* clipboard = gtk_clipboard_get(m_nSelection);
    gtk_clipboard_store(clipboard);
}

VclGtkClipboard::~VclGtkClipboard()
{
    GtkClipboard* clipboard = gtk_clipboard_get(m_nSelection);
    g_signal_handler_disconnect(clipboard, m_nOwnerChangedSignalId);
    if (!m_aGtkTargets.empty())
    {
        gtk_clipboard_clear(clipboard);
        ClipboardClear();
    }
    assert(!m_pSetClipboardEvent);
    assert(m_aGtkTargets.empty());
}

std::vector<GtkTargetEntry> VclToGtkHelper::FormatsToGtk(const css::uno::Sequence<css::datatransfer::DataFlavor> &rFormats)
{
    std::vector<GtkTargetEntry> aGtkTargets;

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
        GtkTargetEntry aEntry(makeGtkTargetEntry(rFlavor));
        aGtkTargets.push_back(aEntry);
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
    GtkClipboard* clipboard = gtk_clipboard_get(m_nSelection);
    gtk_clipboard_set_with_data(clipboard, m_aGtkTargets.data(), m_aGtkTargets.size(),
                                ClipboardGetFunc, ClipboardClearFunc, this);
    gtk_clipboard_set_can_store(clipboard, m_aGtkTargets.data(), m_aGtkTargets.size());
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
    m_aOwner = xClipboardOwner;

    std::vector< Reference< datatransfer::clipboard::XClipboardListener > > aListeners( m_aListeners );
    datatransfer::clipboard::ClipboardEvent aEv;

    GtkClipboard* clipboard = gtk_clipboard_get(m_nSelection);
    if (!m_aGtkTargets.empty())
    {
        gtk_clipboard_clear(clipboard);
        ClipboardClear();
    }
    assert(m_aGtkTargets.empty());
    if (m_aContents.is())
    {
        std::vector<GtkTargetEntry> aGtkTargets(m_aConversionHelper.FormatsToGtk(aFormats));
        if (!aGtkTargets.empty())
        {
            GtkTargetEntry aEntry;
            OString sTunnel = "application/x-libreoffice-internal-id-" + getPID();
            aEntry.target = g_strdup(sTunnel.getStr());
            aEntry.flags = 0;
            aEntry.info = 0;
            aGtkTargets.push_back(aEntry);

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
    return (m_nSelection == GDK_SELECTION_CLIPBOARD) ? OUString("CLIPBOARD") : OUString("PRIMARY");
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

Reference< XInterface > GtkInstance::CreateClipboard(const Sequence< Any >& arguments)
{
    OUString sel;
    if (!arguments.hasElements()) {
        sel = "CLIPBOARD";
    } else if (arguments.getLength() != 1 || !(arguments[0] >>= sel)) {
        throw css::lang::IllegalArgumentException(
            "bad GtkInstance::CreateClipboard arguments",
            css::uno::Reference<css::uno::XInterface>(), -1);
    }

    GdkAtom nSelection = (sel == "CLIPBOARD") ? GDK_SELECTION_CLIPBOARD : GDK_SELECTION_PRIMARY;

    auto it = m_aClipboards.find(nSelection);
    if (it != m_aClipboards.end())
        return it->second;

    Reference<XInterface> xClipboard(static_cast<cppu::OWeakObject *>(new VclGtkClipboard(nSelection)));
    m_aClipboards[nSelection] = xClipboard;

    return xClipboard;
}

GtkDropTarget::GtkDropTarget()
    : WeakComponentImplHelper(m_aMutex)
    , m_pFrame(nullptr)
    , m_pFormatConversionRequest(nullptr)
    , m_bActive(false)
    , m_bInDrag(false)
    , m_nDefaultActions(0)
{
}

OUString SAL_CALL GtkDropTarget::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclGtkDropTarget";
}

sal_Bool SAL_CALL GtkDropTarget::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL GtkDropTarget::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.datatransfer.dnd.GtkDropTarget" };
    return aRet;
}

GtkDropTarget::~GtkDropTarget()
{
    if (m_pFrame)
        m_pFrame->deregisterDropTarget(this);
}

void GtkDropTarget::deinitialize()
{
    m_pFrame = nullptr;
    m_bActive = false;
}

void GtkDropTarget::initialize(const Sequence<Any>& rArguments)
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

void GtkDropTarget::addDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& xListener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.push_back( xListener );
}

void GtkDropTarget::removeDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& xListener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), xListener), m_aListeners.end());
}

void GtkDropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    osl::ClearableGuard<osl::Mutex> aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->drop( dtde );
    }
}

void GtkDropTarget::fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragEnter( dtde );
    }
}

void GtkDropTarget::fire_dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragOver( dtde );
    }
}

void GtkDropTarget::fire_dragExit(const css::datatransfer::dnd::DropTargetEvent& dte)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragExit( dte );
    }
}

sal_Bool GtkDropTarget::isActive()
{
    return m_bActive;
}

void GtkDropTarget::setActive(sal_Bool bActive)
{
    m_bActive = bActive;
}

sal_Int8 GtkDropTarget::getDefaultActions()
{
    return m_nDefaultActions;
}

void GtkDropTarget::setDefaultActions(sal_Int8 nDefaultActions)
{
    m_nDefaultActions = nDefaultActions;
}

Reference< XInterface > GtkInstance::CreateDropTarget()
{
    return Reference<XInterface>(static_cast<cppu::OWeakObject*>(new GtkDropTarget));
}

GtkDragSource::~GtkDragSource()
{
    if (m_pFrame)
        m_pFrame->deregisterDragSource(this);

    if (GtkDragSource::g_ActiveDragSource == this)
    {
        SAL_WARN( "vcl.gtk", "dragEnd should have been called on GtkDragSource before dtor");
        GtkDragSource::g_ActiveDragSource = nullptr;
    }
}

void GtkDragSource::deinitialize()
{
    m_pFrame = nullptr;
}

sal_Bool GtkDragSource::isDragImageSupported()
{
    return true;
}

sal_Int32 GtkDragSource::getDefaultCursor( sal_Int8 )
{
    return 0;
}

void GtkDragSource::initialize(const css::uno::Sequence<css::uno::Any >& rArguments)
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

OUString SAL_CALL GtkDragSource::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.VclGtkDragSource";
}

sal_Bool SAL_CALL GtkDragSource::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL GtkDragSource::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.datatransfer.dnd.GtkDragSource" };
    return aRet;
}

Reference< XInterface > GtkInstance::CreateDragSource()
{
    return Reference< XInterface >( static_cast<cppu::OWeakObject *>(new GtkDragSource()) );
}

namespace {

class GtkOpenGLContext : public OpenGLContext
{
    GLWindow m_aGLWin;
    GtkWidget *m_pGLArea;
    GdkGLContext *m_pContext;
    guint m_nAreaFrameBuffer;
    guint m_nFrameBuffer;
    guint m_nRenderBuffer;
    guint m_nDepthBuffer;
    guint m_nFrameScratchBuffer;
    guint m_nRenderScratchBuffer;
    guint m_nDepthScratchBuffer;

public:
    GtkOpenGLContext()
        : OpenGLContext()
        , m_pGLArea(nullptr)
        , m_pContext(nullptr)
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
        g_signal_connect(G_OBJECT(m_pGLArea), "destroy", G_CALLBACK(signalDestroy), this);
        g_signal_connect(G_OBJECT(m_pGLArea), "render", G_CALLBACK(signalRender), this);
        gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(m_pGLArea), true);
        gtk_gl_area_set_auto_render(GTK_GL_AREA(m_pGLArea), false);
        gtk_widget_set_hexpand(m_pGLArea, true);
        gtk_widget_set_vexpand(m_pGLArea, true);
        gtk_container_add(GTK_CONTAINER(pParent), m_pGLArea);
        gtk_widget_show_all(pParent);

        gtk_gl_area_make_current(GTK_GL_AREA(m_pGLArea));
        if (GError *pError = gtk_gl_area_get_error(GTK_GL_AREA(m_pGLArea)))
        {
            SAL_WARN("vcl.gtk", "gtk gl area error: " << pError->message);
            return false;
        }

        gtk_gl_area_attach_buffers(GTK_GL_AREA(m_pGLArea));
        glGenFramebuffersEXT(1, &m_nAreaFrameBuffer);

        GdkWindow *pWindow = gtk_widget_get_window(pParent);
        m_pContext = gdk_window_create_gl_context(pWindow, nullptr);
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
        if (m_pContext)
        {
            g_clear_object(&m_pContext);
        }
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
    return G_TYPE_CHECK_INSTANCE_TYPE(pDisplay, get_type());
}

bool DLSYM_GDK_IS_X11_DISPLAY(GdkDisplay* pDisplay)
{
    static auto get_type = reinterpret_cast<GType (*) (void)>(dlsym(nullptr, "gdk_x11_display_get_type"));
    if (!get_type)
        return false;
    return G_TYPE_CHECK_INSTANCE_TYPE(pDisplay, get_type());
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

    KeyEvent GtkToVcl(const GdkEventKey& rEvent)
    {
        sal_uInt16 nKeyCode = GtkSalFrame::GetKeyCode(rEvent.keyval);
        if (nKeyCode == 0)
        {
            guint updated_keyval = GtkSalFrame::GetKeyValFor(gdk_keymap_get_default(), rEvent.hardware_keycode, rEvent.group);
            nKeyCode = GtkSalFrame::GetKeyCode(updated_keyval);
        }
        nKeyCode |= GtkSalFrame::GetKeyModCode(rEvent.state);
        return KeyEvent(gdk_keyval_to_unicode(rEvent.keyval), nKeyCode, 0);
    }
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
            gtk_container_child_get(GTK_CONTAINER(pParent), pWidget,
                    "left-attach", &nLeftAttach,
                    "top-attach", &nTopAttach,
                    "width", &nWidth,
                    "height", &nHeight,
                    nullptr);
        }

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

        gtk_container_remove(GTK_CONTAINER(pParent), pWidget);

        gtk_widget_set_visible(pReplacement, gtk_widget_get_visible(pWidget));
        gtk_widget_set_no_show_all(pReplacement, gtk_widget_get_no_show_all(pWidget));

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

        gtk_container_add(GTK_CONTAINER(pParent), pReplacement);

        if (GTK_IS_GRID(pParent))
        {
            gtk_container_child_set(GTK_CONTAINER(pParent), pReplacement,
                    "left-attach", nLeftAttach,
                    "top-attach", nTopAttach,
                    "width", nWidth,
                    "height", nHeight,
                    nullptr);
        }

        if (GTK_IS_BOX(pParent))
        {
            gtk_container_child_set(GTK_CONTAINER(pParent), pReplacement,
                    "expand", bExpand,
                    "fill", bFill,
                    "pack-type", ePackType,
                    "padding", nPadding,
                    "position", nPosition,
                    nullptr);
        }

        if (gtk_widget_get_hexpand_set(pWidget))
            gtk_widget_set_hexpand(pReplacement, gtk_widget_get_hexpand(pWidget));

        if (gtk_widget_get_vexpand_set(pWidget))
            gtk_widget_set_vexpand(pReplacement, gtk_widget_get_vexpand(pWidget));

        gtk_widget_set_halign(pReplacement, gtk_widget_get_halign(pWidget));
        gtk_widget_set_valign(pReplacement, gtk_widget_get_valign(pWidget));

        g_object_unref(pWidget);
    }

    void insertAsParent(GtkWidget* pWidget, GtkWidget* pReplacement)
    {
        g_object_ref(pWidget);

        replaceWidget(pWidget, pReplacement);

        gtk_container_add(GTK_CONTAINER(pReplacement), pWidget);

        g_object_unref(pWidget);
    }

    GtkWidget* ensureEventWidget(GtkWidget* pWidget)
    {
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
    }
}

namespace {

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

class GtkInstanceWidget : public virtual weld::Widget
{
protected:
    GtkWidget* m_pWidget;
    GtkWidget* m_pMouseEventBox;
    GtkInstanceBuilder* m_pBuilder;

    DECL_LINK(async_drag_cancel, void*, void);

    static gboolean signalFocusIn(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->signal_focus_in();
        return false;
    }

    void signal_focus_in()
    {
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

    static gboolean signalFocusOut(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_out();
        return false;
    }

    void launch_drag_cancel(GdkDragContext* context)
    {
        // post our drag cancel to happen at the next available event cycle
        if (m_pDragCancelEvent)
            return;
        g_object_ref(context);
        m_pDragCancelEvent = Application::PostUserEvent(LINK(this, GtkInstanceWidget, async_drag_cancel), context);
    }

    void signal_focus_out()
    {
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
            ensureMouseEventWidget();
            m_nButtonPressSignalId = g_signal_connect(m_pMouseEventBox, "button-press-event", G_CALLBACK(signalButton), this);
        }
    }

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

    bool SwapForRTL() const
    {
        return ::SwapForRTL(m_pWidget);
    }

    void do_enable_drag_source(const rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants)
    {
        css::uno::Reference<css::datatransfer::XTransferable> xTrans(rHelper.get());
        css::uno::Reference<css::datatransfer::dnd::XDragSourceListener> xListener(rHelper.get());

        ensure_drag_source();

        auto aFormats = xTrans->getTransferDataFlavors();
        std::vector<GtkTargetEntry> aGtkTargets(m_xDragSource->FormatsToGtk(aFormats));

        m_eDragAction = VclToGdk(eDNDConstants);
        drag_source_set(aGtkTargets, m_eDragAction);

        for (auto &a : aGtkTargets)
            g_free(a.target);

        m_xDragSource->set_datatransfer(xTrans, xListener);
    }

    void localizeDecimalSeparator()
    {
        // tdf#128867 if localize decimal separator is active we will always
        // need to be able to change the output of the decimal key press
        if (!m_nKeyPressSignalId && Application::GetSettings().GetMiscSettings().GetEnableLocalizedDecimalSep())
            m_nKeyPressSignalId = g_signal_connect(m_pWidget, "key-press-event", G_CALLBACK(signalKey), this);
    }

    void ensure_drag_begin_end()
    {
        if (!m_nDragBeginSignalId)
        {
            // using "after" due to https://gitlab.gnome.org/GNOME/pygobject/issues/251
            m_nDragBeginSignalId = g_signal_connect_after(m_pWidget, "drag-begin", G_CALLBACK(signalDragBegin), this);
        }
        if (!m_nDragEndSignalId)
            m_nDragEndSignalId = g_signal_connect(m_pWidget, "drag-end", G_CALLBACK(signalDragEnd), this);
    }

private:
    bool m_bTakeOwnership;
    bool m_bDraggedOver;
    sal_uInt16 m_nLastMouseButton;
    sal_uInt16 m_nLastMouseClicks;
    int m_nPressedButton;
    int m_nPressStartX;
    int m_nPressStartY;
    ImplSVEvent* m_pDragCancelEvent;
    GtkCssProvider* m_pBgCssProvider;
    GdkDragAction m_eDragAction;
    gulong m_nFocusInSignalId;
    gulong m_nMnemonicActivateSignalId;
    gulong m_nFocusOutSignalId;
    gulong m_nKeyPressSignalId;
    gulong m_nKeyReleaseSignalId;
    gulong m_nSizeAllocateSignalId;
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

    rtl::Reference<GtkDropTarget> m_xDropTarget;
    rtl::Reference<GtkDragSource> m_xDragSource;

    static void signalSizeAllocate(GtkWidget*, GdkRectangle* allocation, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_size_allocate(allocation->width, allocation->height);
    }

    static gboolean signalKey(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        // #i1820# use locale specific decimal separator
        if (pEvent->keyval == GDK_KEY_KP_Decimal && Application::GetSettings().GetMiscSettings().GetEnableLocalizedDecimalSep())
        {
            OUString aSep(Application::GetSettings().GetLocaleDataWrapper().getNumDecimalSep());
            pEvent->keyval = aSep[0];
        }

        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        return pThis->signal_key(pEvent);
    }

    virtual bool signal_popup_menu(const CommandEvent&)
    {
        return false;
    }

    static gboolean signalButton(GtkWidget*, GdkEventButton* pEvent, gpointer widget)
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

        if (!m_aMouseMotionHdl.IsSet())
            return false;

        Point aPos(pEvent->x, pEvent->y);
        if (SwapForRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());
        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(pEvent->state);
        MouseEvent aMEvt(aPos, 0, ImplGetMouseMoveMode(nModCode), nModCode, nModCode);

        m_aMouseMotionHdl.Call(aMEvt);
        return true;
    }

    static gboolean signalCrossing(GtkWidget*, GdkEventCrossing* pEvent, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_crossing(pEvent);
    }

    bool signal_crossing(const GdkEventCrossing* pEvent)
    {
        if (!m_aMouseMotionHdl.IsSet())
            return false;

        Point aPos(pEvent->x, pEvent->y);
        if (SwapForRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());
        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(pEvent->state);
        MouseEventModifiers eModifiers = ImplGetMouseMoveMode(nModCode);
        eModifiers = eModifiers | (pEvent->type == GDK_ENTER_NOTIFY ? MouseEventModifiers::ENTERWINDOW : MouseEventModifiers::LEAVEWINDOW);
        MouseEvent aMEvt(aPos, 0, eModifiers, nModCode, nModCode);

        m_aMouseMotionHdl.Call(aMEvt);
        return true;
    }

    virtual void drag_started()
    {
    }

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

    virtual void drag_ended()
    {
    }

    static void signalDragLeave(GtkWidget *pWidget, GdkDragContext *context, guint time, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->m_xDropTarget->signalDragLeave(pWidget, context, time);
        if (pThis->m_bDraggedOver)
        {
            pThis->m_bDraggedOver = false;
            pThis->drag_ended();
        }
    }

    static void signalDragBegin(GtkWidget*, GdkDragContext* context, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->signal_drag_begin(context);
    }

    void ensure_drag_source()
    {
        if (!m_xDragSource)
        {
            m_xDragSource.set(new GtkDragSource);

            m_nDragFailedSignalId = g_signal_connect(m_pWidget, "drag-failed", G_CALLBACK(signalDragFailed), this);
            m_nDragDataDeleteignalId = g_signal_connect(m_pWidget, "drag-data-delete", G_CALLBACK(signalDragDelete), this);
            m_nDragGetSignalId = g_signal_connect(m_pWidget, "drag-data-get", G_CALLBACK(signalDragDataGet), this);

            ensure_drag_begin_end();
        }
    }

    virtual bool do_signal_drag_begin(bool& rUnsetDragIcon)
    {
        rUnsetDragIcon = false;
        return false;
    }

    void signal_drag_begin(GdkDragContext* context)
    {
        bool bUnsetDragIcon(false);
        if (do_signal_drag_begin(bUnsetDragIcon))
        {
            launch_drag_cancel(context);
            return;
        }
        if (bUnsetDragIcon)
        {
            cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
            gtk_drag_set_icon_surface(context, surface);
        }
        if (!m_xDragSource)
            return;
        m_xDragSource->setActiveDragSource();
    }

    virtual void do_signal_drag_end()
    {
    }

    static void signalDragEnd(GtkWidget* /*widget*/, GdkDragContext* context, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->do_signal_drag_end();
        if (pThis->m_xDragSource.is())
            pThis->m_xDragSource->dragEnd(context);
    }

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

    virtual void drag_source_set(const std::vector<GtkTargetEntry>& rGtkTargets, GdkDragAction eDragAction)
    {
        if (rGtkTargets.empty() && !eDragAction)
            gtk_drag_source_unset(m_pWidget);
        else
            gtk_drag_source_set(m_pWidget, GDK_BUTTON1_MASK, rGtkTargets.data(), rGtkTargets.size(), eDragAction);
    }

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
        gtk_css_provider_load_from_data(m_pBgCssProvider, aResult.getStr(), aResult.getLength(), nullptr);
        gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pBgCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

public:
    GtkInstanceWidget(GtkWidget* pWidget, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : m_pWidget(pWidget)
        , m_pMouseEventBox(nullptr)
        , m_pBuilder(pBuilder)
        , m_bTakeOwnership(bTakeOwnership)
        , m_bDraggedOver(false)
        , m_nLastMouseButton(0)
        , m_nLastMouseClicks(0)
        , m_nPressedButton(-1)
        , m_nPressStartX(-1)
        , m_nPressStartY(-1)
        , m_pDragCancelEvent(nullptr)
        , m_pBgCssProvider(nullptr)
        , m_eDragAction(GdkDragAction(0))
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
    {
        if (!bTakeOwnership)
            g_object_ref(m_pWidget);

        localizeDecimalSeparator();
    }

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override
    {
        if (!m_nKeyPressSignalId)
            m_nKeyPressSignalId = g_signal_connect(m_pWidget, "key-press-event", G_CALLBACK(signalKey), this);
        weld::Widget::connect_key_press(rLink);
    }

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override
    {
        if (!m_nKeyReleaseSignalId)
            m_nKeyReleaseSignalId = g_signal_connect(m_pWidget, "key-release-event", G_CALLBACK(signalKey), this);
        weld::Widget::connect_key_release(rLink);
    }

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensureButtonPressSignal();
        weld::Widget::connect_mouse_press(rLink);
    }

    virtual void connect_mouse_move(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensureMouseEventWidget();
        if (!m_nMotionSignalId)
            m_nMotionSignalId = g_signal_connect(m_pMouseEventBox, "motion-notify-event", G_CALLBACK(signalMotion), this);
        if (!m_nLeaveSignalId)
            m_nLeaveSignalId = g_signal_connect(m_pMouseEventBox, "leave-notify-event", G_CALLBACK(signalCrossing), this);
        if (!m_nEnterSignalId)
            m_nEnterSignalId = g_signal_connect(m_pMouseEventBox, "enter-notify-event", G_CALLBACK(signalCrossing), this);
        weld::Widget::connect_mouse_move(rLink);
    }

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensureMouseEventWidget();
        if (!m_nButtonReleaseSignalId)
            m_nButtonReleaseSignalId = g_signal_connect(m_pMouseEventBox, "button-release-event", G_CALLBACK(signalButton), this);
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
        GtkWindow* pTopLevel = GTK_WINDOW(gtk_widget_get_toplevel(m_pWidget));
        return pTopLevel && gtk_window_is_active(pTopLevel) && has_focus();
    }

    // is the focus in a child of this widget, where a transient popup attached
    // to a widget is considered a child of that widget
    virtual bool has_child_focus() const override
    {
        bool bRet = false;

        GList* pList = gtk_window_list_toplevels();

        for (GList* pEntry = pList; pEntry; pEntry = pEntry->next)
        {
            if (!gtk_window_has_toplevel_focus(GTK_WINDOW(pEntry->data)))
                continue;
            GtkWidget* pFocus = gtk_window_get_focus(GTK_WINDOW(pEntry->data));
            if (pFocus && gtk_widget_is_ancestor(pFocus, m_pWidget))
            {
                bRet = true;
                break;
            }
            GtkWidget* pAttachedTo = gtk_window_get_attached_to(GTK_WINDOW(pEntry->data));
            if (!pAttachedTo)
                continue;
            if (pAttachedTo == m_pWidget || gtk_widget_is_ancestor(pAttachedTo, m_pWidget))
            {
                bRet = true;
                break;
            }
        }

        g_list_free(pList);

        return bRet;
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
        PangoContext* pContext = gtk_widget_get_pango_context(m_pWidget);
        return pango_to_vcl(pango_context_get_font_description(pContext),
                            Application::GetSettings().GetUILanguageTag().getLocale());
    }

    virtual void set_grid_left_attach(int nAttach) override
    {
        GtkContainer* pParent = GTK_CONTAINER(gtk_widget_get_parent(m_pWidget));
        gtk_container_child_set(pParent, m_pWidget, "left-attach", nAttach, nullptr);
    }

    virtual int get_grid_left_attach() const override
    {
        GtkContainer* pParent = GTK_CONTAINER(gtk_widget_get_parent(m_pWidget));
        gint nAttach(0);
        gtk_container_child_get(pParent, m_pWidget, "left-attach", &nAttach, nullptr);
        return nAttach;
    }

    virtual void set_grid_width(int nCols) override
    {
        GtkContainer* pParent = GTK_CONTAINER(gtk_widget_get_parent(m_pWidget));
        gtk_container_child_set(pParent, m_pWidget, "width", nCols, nullptr);
    }

    virtual void set_grid_top_attach(int nAttach) override
    {
        GtkContainer* pParent = GTK_CONTAINER(gtk_widget_get_parent(m_pWidget));
        gtk_container_child_set(pParent, m_pWidget, "top-attach", nAttach, nullptr);
    }

    virtual int get_grid_top_attach() const override
    {
        GtkContainer* pParent = GTK_CONTAINER(gtk_widget_get_parent(m_pWidget));
        gint nAttach(0);
        gtk_container_child_get(pParent, m_pWidget, "top-attach", &nAttach, nullptr);
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

    virtual void set_secondary(bool bSecondary) override
    {
        GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
        if (pParent && GTK_IS_BUTTON_BOX(pParent))
            gtk_button_box_set_child_secondary(GTK_BUTTON_BOX(pParent), m_pWidget, bSecondary);
    }

    virtual void set_margin_top(int nMargin) override
    {
        gtk_widget_set_margin_top(m_pWidget, nMargin);
    }

    virtual void set_margin_bottom(int nMargin) override
    {
        gtk_widget_set_margin_bottom(m_pWidget, nMargin);
    }

    virtual void set_margin_left(int nMargin) override
    {
        gtk_widget_set_margin_left(m_pWidget, nMargin);
    }

    virtual void set_margin_right(int nMargin) override
    {
        gtk_widget_set_margin_right(m_pWidget, nMargin);
    }

    virtual int get_margin_top() const override
    {
        return gtk_widget_get_margin_top(m_pWidget);
    }

    virtual int get_margin_bottom() const override
    {
        return gtk_widget_get_margin_bottom(m_pWidget);
    }

    virtual int get_margin_left() const override
    {
        return gtk_widget_get_margin_left(m_pWidget);
    }

    virtual int get_margin_right() const override
    {
        return gtk_widget_get_margin_right(m_pWidget);
    }

    virtual void set_accessible_name(const OUString& rName) override
    {
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        atk_object_set_name(pAtkObject, OUStringToOString(rName, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void set_accessible_description(const OUString& rDescription) override
    {
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        atk_object_set_description(pAtkObject, OUStringToOString(rDescription, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_accessible_name() const override
    {
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_name(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual OUString get_accessible_description() const override
    {
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void set_accessible_relation_labeled_by(weld::Widget* pLabel) override
    {
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
    }

    virtual void set_accessible_relation_label_for(weld::Widget* pLabeled) override
    {
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
    }

    virtual bool get_extents_relative_to(const weld::Widget& rRelative, int& x, int &y, int& width, int &height) const override
    {
        //for toplevel windows this is sadly futile under wayland, so we can't tell where a dialog is in order to allow
        //the document underneath to auto-scroll to place content in a visible location
        bool ret = gtk_widget_translate_coordinates(m_pWidget,
                                                    dynamic_cast<const GtkInstanceWidget&>(rRelative).getWidget(),
                                                    0, 0, &x, &y);
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
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(m_pWidget));
        return OString(pStr, pStr ? strlen(pStr) : 0);
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
        return GTK_WINDOW(gtk_widget_get_toplevel(m_pWidget));
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        if (!m_nFocusInSignalId)
            m_nFocusInSignalId = g_signal_connect(m_pWidget, "focus-in-event", G_CALLBACK(signalFocusIn), this);
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
            m_nFocusOutSignalId = g_signal_connect(m_pWidget, "focus-out-event", G_CALLBACK(signalFocusOut), this);
        weld::Widget::connect_focus_out(rLink);
    }

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override
    {
        m_nSizeAllocateSignalId = g_signal_connect(m_pWidget, "size_allocate", G_CALLBACK(signalSizeAllocate), this);
        weld::Widget::connect_size_allocate(rLink);
    }

    virtual void signal_size_allocate(guint nWidth, guint nHeight)
    {
        m_aSizeAllocateHdl.Call(Size(nWidth, nHeight));
    }

    bool signal_key(const GdkEventKey* pEvent)
    {
        if (pEvent->type == GDK_KEY_PRESS && m_aKeyPressHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            return m_aKeyPressHdl.Call(GtkToVcl(*pEvent));
        }
        if (pEvent->type == GDK_KEY_RELEASE && m_aKeyReleaseHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            return m_aKeyReleaseHdl.Call(GtkToVcl(*pEvent));
        }
        return false;
    }

    virtual void grab_add() override
    {
        gtk_grab_add(m_pWidget);
    }

    virtual bool has_grab() const override
    {
        return gtk_widget_has_grab(m_pWidget);
    }

    virtual void grab_remove() override
    {
        gtk_grab_remove(m_pWidget);
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
        gtk_widget_freeze_child_notify(m_pWidget);
        g_object_freeze_notify(G_OBJECT(m_pWidget));
    }

    virtual void thaw() override
    {
        g_object_thaw_notify(G_OBJECT(m_pWidget));
        gtk_widget_thaw_child_notify(m_pWidget);
    }

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override
    {
        if (!m_xDropTarget)
        {
            m_xDropTarget.set(new GtkDropTarget);
            if (!gtk_drag_dest_get_track_motion(m_pWidget))
            {
                gtk_drag_dest_set(m_pWidget, GtkDestDefaults(0), nullptr, 0, GdkDragAction(0));
                gtk_drag_dest_set_track_motion(m_pWidget, true);
            }
            m_nDragMotionSignalId = g_signal_connect(m_pWidget, "drag-motion", G_CALLBACK(signalDragMotion), this);
            m_nDragDropSignalId = g_signal_connect(m_pWidget, "drag-drop", G_CALLBACK(signalDragDrop), this);
            m_nDragDropReceivedSignalId = g_signal_connect(m_pWidget, "drag-data-received", G_CALLBACK(signalDragDropReceived), this);
            m_nDragLeaveSignalId = g_signal_connect(m_pWidget, "drag-leave", G_CALLBACK(signalDragLeave), this);
        }
        return m_xDropTarget.get();
    }

    virtual void connect_get_property_tree(const Link<tools::JsonWriter&, void>& /*rLink*/) override
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
            g_signal_handler_disconnect(m_pWidget, m_nDragEndSignalId);
        if (m_nDragBeginSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragBeginSignalId);
        if (m_nDragFailedSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragFailedSignalId);
        if (m_nDragDataDeleteignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragDataDeleteignalId);
        if (m_nDragGetSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragGetSignalId);
        if (m_nKeyPressSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nKeyPressSignalId);
        if (m_nKeyReleaseSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nKeyReleaseSignalId);
        if (m_nButtonPressSignalId)
            g_signal_handler_disconnect(m_pMouseEventBox, m_nButtonPressSignalId);
        if (m_nMotionSignalId)
            g_signal_handler_disconnect(m_pMouseEventBox, m_nMotionSignalId);
        if (m_nLeaveSignalId)
            g_signal_handler_disconnect(m_pMouseEventBox, m_nLeaveSignalId);
        if (m_nEnterSignalId)
            g_signal_handler_disconnect(m_pMouseEventBox, m_nEnterSignalId);
        if (m_nButtonReleaseSignalId)
            g_signal_handler_disconnect(m_pMouseEventBox, m_nButtonReleaseSignalId);
        if (m_nFocusInSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nFocusInSignalId);
        if (m_nMnemonicActivateSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nMnemonicActivateSignalId);
        if (m_nFocusOutSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nFocusOutSignalId);
        if (m_nSizeAllocateSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nSizeAllocateSignalId);

        do_set_background(COL_AUTO);

        if (m_pMouseEventBox && m_pMouseEventBox != m_pWidget)
        {
            // put things back they way we found them
            GtkWidget* pParent = gtk_widget_get_parent(m_pMouseEventBox);

            g_object_ref(m_pWidget);
            gtk_container_remove(GTK_CONTAINER(m_pMouseEventBox), m_pWidget);

            gtk_widget_destroy(m_pMouseEventBox);

            gtk_container_add(GTK_CONTAINER(pParent), m_pWidget);
            g_object_unref(m_pWidget);
        }

        if (m_bTakeOwnership)
            gtk_widget_destroy(m_pWidget);
        else
            g_object_unref(m_pWidget);
    }

    virtual void disable_notify_events()
    {
        if (m_nFocusInSignalId)
            g_signal_handler_block(m_pWidget, m_nFocusInSignalId);
        if (m_nMnemonicActivateSignalId)
            g_signal_handler_block(m_pWidget, m_nMnemonicActivateSignalId);
        if (m_nFocusOutSignalId)
            g_signal_handler_block(m_pWidget, m_nFocusOutSignalId);
        if (m_nSizeAllocateSignalId)
            g_signal_handler_block(m_pWidget, m_nSizeAllocateSignalId);
    }

    virtual void enable_notify_events()
    {
        if (m_nSizeAllocateSignalId)
            g_signal_handler_unblock(m_pWidget, m_nSizeAllocateSignalId);
        if (m_nFocusOutSignalId)
            g_signal_handler_unblock(m_pWidget, m_nFocusOutSignalId);
        if (m_nMnemonicActivateSignalId)
            g_signal_handler_unblock(m_pWidget, m_nMnemonicActivateSignalId);
        if (m_nFocusInSignalId)
            g_signal_handler_unblock(m_pWidget, m_nFocusInSignalId);
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

    virtual void draw(OutputDevice& rOutput, const tools::Rectangle& rRect) override
    {
        // detect if we have to manually setup its size
        bool bAlreadyRealized = gtk_widget_get_realized(m_pWidget);
        // has to be visible for draw to work
        bool bAlreadyVisible = gtk_widget_get_visible(m_pWidget);
        // has to be mapped for draw to work
        bool bAlreadyMapped = gtk_widget_get_mapped(m_pWidget);

        if (!bAlreadyRealized)
            gtk_widget_realize(m_pWidget);
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

        Size aSize(rRect.GetSize());

        GtkAllocation aOrigAllocation;
        gtk_widget_get_allocation(m_pWidget, &aOrigAllocation);

        GtkAllocation aNewAllocation {aOrigAllocation.x,
                                      aOrigAllocation.y,
                                      static_cast<int>(aSize.Width()),
                                      static_cast<int>(aSize.Height()) };
        gtk_widget_size_allocate(m_pWidget, &aNewAllocation);

        if (GTK_IS_CONTAINER(m_pWidget))
            gtk_container_resize_children(GTK_CONTAINER(m_pWidget));

        VclPtr<VirtualDevice> xOutput(VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT));
        xOutput->SetOutputSizePixel(aSize);
        xOutput->DrawOutDev(Point(), aSize, rRect.TopLeft(), aSize, rOutput);

        cairo_surface_t* pSurface = get_underlying_cairo_surface(*xOutput);
        cairo_t* cr = cairo_create(pSurface);

        gtk_widget_draw(m_pWidget, cr);

        cairo_destroy(cr);

        gtk_widget_set_allocation(m_pWidget, &aOrigAllocation);
        gtk_widget_size_allocate(m_pWidget, &aOrigAllocation);

        rOutput.DrawOutDev(rRect.TopLeft(), aSize, Point(), aSize, *xOutput);

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

    OUString get_label(GtkButton* pButton)
    {
        const gchar* pStr = gtk_button_get_label(pButton);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_label(GtkButton* pButton, const OUString& rText)
    {
        gtk_button_set_label(pButton, MapToGtkAccelerator(rText).getStr());
    }

    OUString get_title(GtkWindow* pWindow)
    {
        const gchar* pStr = gtk_window_get_title(pWindow);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_title(GtkWindow* pWindow, const OUString& rTitle)
    {
        gtk_window_set_title(pWindow, OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8).getStr());
    }

    OUString get_primary_text(GtkMessageDialog* pMessageDialog)
    {
        gchar* pText = nullptr;
        g_object_get(G_OBJECT(pMessageDialog), "text", &pText, nullptr);
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_primary_text(GtkMessageDialog* pMessageDialog, const OUString& rText)
    {
        g_object_set(G_OBJECT(pMessageDialog), "text",
            OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
            nullptr);
    }

    void set_secondary_text(GtkMessageDialog* pMessageDialog, const OUString& rText)
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

        std::unique_ptr<SvMemoryStream> xMemStm(new SvMemoryStream);

        css::uno::Sequence<css::beans::PropertyValue> aFilterData(1);
        aFilterData[0].Name = "Compression";
        // We "know" that this gets passed to zlib's deflateInit2_(). 1 means best speed.
        aFilterData[0].Value <<= sal_Int32(1);

        vcl::PNGWriter aWriter(aImage.GetBitmapEx(), &aFilterData);
        aWriter.Write(*xMemStm);

        return load_icon_from_stream(*xMemStm);
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

    GdkPixbuf* getPixbuf(const OUString& rIconName)
    {
        if (rIconName.isEmpty())
            return nullptr;

        GdkPixbuf* pixbuf = nullptr;

        if (rIconName.lastIndexOf('.') != rIconName.getLength() - 4)
        {
            assert((rIconName== "dialog-warning" || rIconName== "dialog-error" || rIconName== "dialog-information") &&
                   "unknown stock image");

            GError *error = nullptr;
            GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
            pixbuf = gtk_icon_theme_load_icon(icon_theme, OUStringToOString(rIconName, RTL_TEXTENCODING_UTF8).getStr(),
                                              16, GTK_ICON_LOOKUP_USE_BUILTIN, &error);
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

class MenuHelper
{
protected:
    GtkMenu* m_pMenu;
    bool m_bTakeOwnership;
    std::map<OString, GtkMenuItem*> m_aMap;
private:

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
        pThis->signal_activate(pItem);
    }

    virtual void signal_activate(GtkMenuItem* pItem) = 0;

public:
    MenuHelper(GtkMenu* pMenu, bool bTakeOwnership)
        : m_pMenu(pMenu)
        , m_bTakeOwnership(bTakeOwnership)
    {
        if (!m_pMenu)
            return;
        gtk_container_foreach(GTK_CONTAINER(m_pMenu), collect, this);
    }

    void add_to_map(GtkMenuItem* pMenuItem)
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pMenuItem));
        OString id(pStr, pStr ? strlen(pStr) : 0);
        m_aMap[id] = pMenuItem;
        g_signal_connect(pMenuItem, "activate", G_CALLBACK(signalActivate), this);
    }

    void remove_from_map(GtkMenuItem* pMenuItem)
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pMenuItem));
        OString id(pStr, pStr ? strlen(pStr) : 0);
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

    void insert_item(int pos, const OUString& rId, const OUString& rStr,
                     const OUString* pIconName, const VirtualDevice* pImageSurface,
                     TriState eCheckRadioFalse)
    {
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
            GtkWidget *pBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
            GtkWidget *pLabel = gtk_label_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new() : gtk_menu_item_new();
            gtk_container_add(GTK_CONTAINER(pBox), pImage);
            gtk_container_add(GTK_CONTAINER(pBox), pLabel);
            gtk_container_add(GTK_CONTAINER(pItem), pBox);
            gtk_widget_show_all(pItem);
        }
        else
        {
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr())
                                                       : gtk_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
        }

        if (eCheckRadioFalse == TRISTATE_FALSE)
            gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(pItem), true);

        gtk_buildable_set_name(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr());
        gtk_menu_shell_append(GTK_MENU_SHELL(m_pMenu), pItem);
        gtk_widget_show(pItem);
        add_to_map(GTK_MENU_ITEM(pItem));
        if (pos != -1)
            gtk_menu_reorder_child(m_pMenu, pItem, pos);
    }

    void insert_separator(int pos, const OUString& rId)
    {
        GtkWidget* pItem = gtk_separator_menu_item_new();
        gtk_buildable_set_name(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr());
        gtk_menu_shell_append(GTK_MENU_SHELL(m_pMenu), pItem);
        gtk_widget_show(pItem);
        add_to_map(GTK_MENU_ITEM(pItem));
        if (pos != -1)
            gtk_menu_reorder_child(m_pMenu, pItem, pos);
    }

    void remove_item(const OString& rIdent)
    {
        GtkMenuItem* pMenuItem = m_aMap[rIdent];
        remove_from_map(pMenuItem);
        gtk_widget_destroy(GTK_WIDGET(pMenuItem));
    }

    void set_item_sensitive(const OString& rIdent, bool bSensitive)
    {
        gtk_widget_set_sensitive(GTK_WIDGET(m_aMap[rIdent]), bSensitive);
    }

    void set_item_active(const OString& rIdent, bool bActive)
    {
        disable_item_notify_events();
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(m_aMap[rIdent]), bActive);
        enable_item_notify_events();
    }

    bool get_item_active(const OString& rIdent) const
    {
        return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(m_aMap.find(rIdent)->second));
    }

    void set_item_label(const OString& rIdent, const OUString& rText)
    {
        gtk_menu_item_set_label(m_aMap[rIdent], MapToGtkAccelerator(rText).getStr());
    }

    OUString get_item_label(const OString& rIdent) const
    {
        const gchar* pText = gtk_menu_item_get_label(m_aMap.find(rIdent)->second);
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }

    void set_item_help_id(const OString& rIdent, const OString& rHelpId)
    {
        set_help_id(GTK_WIDGET(m_aMap[rIdent]), rHelpId);
    }

    OString get_item_help_id(const OString& rIdent) const
    {
        return get_help_id(GTK_WIDGET(m_aMap.find(rIdent)->second));
    }

    void set_item_visible(const OString& rIdent, bool bShow)
    {
        GtkWidget* pWidget = GTK_WIDGET(m_aMap[rIdent]);
        if (bShow)
            gtk_widget_show(pWidget);
        else
            gtk_widget_hide(pWidget);
    }

    void clear_items()
    {
        for (const auto& a : m_aMap)
        {
            GtkMenuItem* pMenuItem = a.second;
            g_signal_handlers_disconnect_by_data(pMenuItem, this);
            gtk_widget_destroy(GTK_WIDGET(pMenuItem));
        }
        m_aMap.clear();
    }

    GtkMenu* getMenu() const
    {
        return m_pMenu;
    }

    virtual ~MenuHelper()
    {
        for (auto& a : m_aMap)
            g_signal_handlers_disconnect_by_data(a.second, this);
        if (m_bTakeOwnership)
            gtk_widget_destroy(GTK_WIDGET(m_pMenu));
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
    {
        maLayoutIdle.SetPriority(TaskPriority::RESIZE);
        maLayoutIdle.SetInvokeHandler( LINK( this, ChildFrame, ImplHandleLayoutTimerHdl ) );
        maLayoutIdle.SetDebugName( "ChildFrame maLayoutIdle" );
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

    virtual void Resize() override
    {
        WorkWindow::Resize();
        queue_resize();
    }
};

IMPL_LINK_NOARG(ChildFrame, ImplHandleLayoutTimerHdl, Timer*, void)
{
    if (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild))
        pChild->SetPosSizePixel(Point(0, 0), GetSizePixel());
}

class GtkInstanceContainer : public GtkInstanceWidget, public virtual weld::Container
{
private:
    GtkContainer* m_pContainer;

    static void implResetDefault(GtkWidget *pWidget, gpointer user_data)
    {
        if (GTK_IS_BUTTON(pWidget))
            g_object_set(G_OBJECT(pWidget), "has-default", false, nullptr);
        if (GTK_IS_CONTAINER(pWidget))
            gtk_container_forall(GTK_CONTAINER(pWidget), implResetDefault, user_data);
    }

public:
    GtkInstanceContainer(GtkContainer* pContainer, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pContainer), pBuilder, bTakeOwnership)
        , m_pContainer(pContainer)
    {
    }

    GtkContainer* getContainer() { return m_pContainer; }

    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override
    {
        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pWidget);
        assert(pGtkWidget);
        GtkWidget* pChild = pGtkWidget->getWidget();
        g_object_ref(pChild);
        gtk_container_remove(getContainer(), pChild);

        GtkInstanceContainer* pNewGtkParent = dynamic_cast<GtkInstanceContainer*>(pNewParent);
        assert(!pNewParent || pNewGtkParent);
        if (pNewGtkParent)
            gtk_container_add(pNewGtkParent->getContainer(), pChild);
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
        gtk_container_remove(GTK_CONTAINER(pParent), pWindow);
        gtk_container_add(m_pContainer, pWindow);
        gtk_container_child_set(m_pContainer, pWindow, "expand", true, "fill", true, nullptr);
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
};

}

std::unique_ptr<weld::Container> GtkInstanceWidget::weld_parent() const
{
    GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
    if (!pParent)
        return nullptr;
    return std::make_unique<GtkInstanceContainer>(GTK_CONTAINER(pParent), m_pBuilder, false);
}

namespace {

struct ButtonOrder
{
    const char * m_aType;
    int m_nPriority;
};

int getButtonPriority(const OString &rType)
{
    static const size_t N_TYPES = 7;
    static const ButtonOrder aDiscardCancelSave[N_TYPES] =
    {
        { "/discard", 0 },
        { "/cancel", 1 },
        { "/no", 2 },
        { "/open", 3 },
        { "/save", 3 },
        { "/yes", 3 },
        { "/ok", 3 }
    };

    static const ButtonOrder aSaveDiscardCancel[N_TYPES] =
    {
        { "/open", 0 },
        { "/save", 0 },
        { "/yes", 0 },
        { "/ok", 0 },
        { "/discard", 1 },
        { "/no", 1 },
        { "/cancel", 2 }
    };

    const ButtonOrder* pOrder = &aDiscardCancelSave[0];

    const OUString &rEnv = Application::GetDesktopEnvironment();

    if (rEnv.equalsIgnoreAsciiCase("windows") ||
        rEnv.equalsIgnoreAsciiCase("tde") ||
        rEnv.startsWithIgnoreAsciiCase("kde"))
    {
        pOrder = &aSaveDiscardCancel[0];
    }

    for (size_t i = 0; i < N_TYPES; ++i, ++pOrder)
    {
        if (rType.endsWith(pOrder->m_aType))
            return pOrder->m_nPriority;
    }

    return -1;
}

bool sortButtons(const GtkWidget* pA, const GtkWidget* pB)
{
    //order within groups according to platform rules
    return getButtonPriority(::get_help_id(pA)) < getButtonPriority(::get_help_id(pB));
}

void sort_native_button_order(GtkBox* pContainer)
{
    std::vector<GtkWidget*> aChildren;
    GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pContainer));
    for (GList* pChild = g_list_first(pChildren); pChild; pChild = g_list_next(pChild))
        aChildren.push_back(static_cast<GtkWidget*>(pChild->data));
    g_list_free(pChildren);

    //sort child order within parent so that we match the platform button order
    std::stable_sort(aChildren.begin(), aChildren.end(), sortButtons);

    for (size_t pos = 0; pos < aChildren.size(); ++pos)
        gtk_box_reorder_child(pContainer, aChildren[pos], pos);
}

class GtkInstanceBox : public GtkInstanceContainer, public virtual weld::Box
{
private:
    GtkBox* m_pBox;

public:
    GtkInstanceBox(GtkBox* pBox, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pBox), pBuilder, bTakeOwnership)
        , m_pBox(pBox)
    {
    }

    virtual void reorder_child(weld::Widget* pWidget, int nNewPosition) override
    {
        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pWidget);
        assert(pGtkWidget);
        GtkWidget* pChild = pGtkWidget->getWidget();
        gtk_box_reorder_child(m_pBox, pChild, nNewPosition);
    }

    virtual void sort_native_button_order() override
    {
        ::sort_native_button_order(m_pBox);
    }
};

    void set_cursor(GtkWidget* pWidget, const char *pName)
    {
        if (!gtk_widget_get_realized(pWidget))
            gtk_widget_realize(pWidget);
        GdkDisplay *pDisplay = gtk_widget_get_display(pWidget);
        GdkCursor *pCursor = pName ? gdk_cursor_new_from_name(pDisplay, pName) : nullptr;
        gdk_window_set_cursor(gtk_widget_get_window(pWidget), pCursor);
        gdk_display_flush(pDisplay);
        if (pCursor)
            g_object_unref(pCursor);
    }
}

namespace
{
    Point get_csd_offset(GtkWidget* pTopLevel)
    {
        // try and omit drawing CSD under wayland
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pTopLevel));
        GList* pChild = g_list_first(pChildren);

        int x, y;
        gtk_widget_translate_coordinates(GTK_WIDGET(pChild->data),
                                         GTK_WIDGET(pTopLevel),
                                         0, 0, &x, &y);

        int innerborder = gtk_container_get_border_width(GTK_CONTAINER(pChild->data));
        g_list_free(pChildren);

        int outerborder = gtk_container_get_border_width(GTK_CONTAINER(pTopLevel));
        int totalborder = outerborder + innerborder;
        x -= totalborder;
        y -= totalborder;

        return Point(x, y);
    }

    void do_collect_screenshot_data(GtkWidget* pItem, gpointer data)
    {
        GtkWidget* pTopLevel = gtk_widget_get_toplevel(pItem);

        int x, y;
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

        if (GTK_IS_CONTAINER(pItem))
            gtk_container_forall(GTK_CONTAINER(pItem), do_collect_screenshot_data, data);
    }

    tools::Rectangle get_monitor_workarea(GtkWidget* pWindow)
    {
        GdkScreen* pScreen = gtk_widget_get_screen(pWindow);
        gint nMonitor = gdk_screen_get_monitor_at_window(pScreen, gtk_widget_get_window(pWindow));
        GdkRectangle aRect;
        gdk_screen_get_monitor_workarea(pScreen, nMonitor, &aRect);
        return tools::Rectangle(aRect.x, aRect.y, aRect.x + aRect.width, aRect.y + aRect.height);
    }


class GtkInstanceWindow : public GtkInstanceContainer, public virtual weld::Window
{
private:
    GtkWindow* m_pWindow;
    rtl::Reference<SalGtkXWindow> m_xWindow; //uno api
    gulong m_nToplevelFocusChangedSignalId;

    static gboolean help_pressed(GtkAccelGroup*, GObject*, guint, GdkModifierType, gpointer widget)
    {
        GtkInstanceWindow* pThis = static_cast<GtkInstanceWindow*>(widget);
        pThis->help();
        return true;
    }

    static void signalToplevelFocusChanged(GtkWindow*, GParamSpec*, gpointer widget)
    {
        GtkInstanceWindow* pThis = static_cast<GtkInstanceWindow*>(widget);
        pThis->signal_toplevel_focus_changed();
    }

    bool isPositioningAllowed() const
    {
        bool bPositioningAllowed = true;
#if defined(GDK_WINDOWING_WAYLAND)
        // no X/Y positioning under Wayland
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        bPositioningAllowed = !DLSYM_GDK_IS_WAYLAND_DISPLAY(pDisplay);
#endif
        return bPositioningAllowed;
    }

protected:
    void help();
public:
    GtkInstanceWindow(GtkWindow* pWindow, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pWindow), pBuilder, bTakeOwnership)
        , m_pWindow(pWindow)
        , m_nToplevelFocusChangedSignalId(0)
    {
        const bool bIsFrameWeld = pBuilder == nullptr;
        if (!bIsFrameWeld)
        {
            //hook up F1 to show help
            GtkAccelGroup *pGroup = gtk_accel_group_new();
            GClosure* closure = g_cclosure_new(G_CALLBACK(help_pressed), this, nullptr);
            gtk_accel_group_connect(pGroup, GDK_KEY_F1, static_cast<GdkModifierType>(0), GTK_ACCEL_LOCKED, closure);
            gtk_window_add_accel_group(pWindow, pGroup);
        }
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
        return css::uno::Reference<css::awt::XWindow>(m_xWindow.get());
    }

    virtual void set_busy_cursor(bool bBusy) override
    {
        set_cursor(m_pWidget, bBusy ? "progress" : nullptr);
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
        gtk_window_resize(m_pWindow, 1, 1);
    }

    virtual void window_move(int x, int y) override
    {
        gtk_window_move(m_pWindow, x, y);
    }

    virtual SystemEnvData get_system_data() const override
    {
        assert(false && "nothing should call this impl, yet anyway, if ever");
        return SystemEnvData();
    }

    virtual Size get_size() const override
    {
        int current_width, current_height;
        gtk_window_get_size(m_pWindow, &current_width, &current_height);
        return Size(current_width, current_height);
    }

    virtual Point get_position() const override
    {
        int current_x, current_y;
        gtk_window_get_position(m_pWindow, &current_x, &current_y);
        return Point(current_x, current_y);
    }

    virtual tools::Rectangle get_monitor_workarea() const override
    {
        return ::get_monitor_workarea(GTK_WIDGET(m_pWindow));
    }

    virtual void set_centered_on_parent(bool bTrackGeometryRequests) override
    {
        if (bTrackGeometryRequests)
            gtk_window_set_position(m_pWindow, GTK_WIN_POS_CENTER_ALWAYS);
        else
            gtk_window_set_position(m_pWindow, GTK_WIN_POS_CENTER_ON_PARENT);
    }

    virtual bool get_resizable() const override
    {
        return gtk_window_get_resizable(m_pWindow);
    }

    virtual bool has_toplevel_focus() const override
    {
        return gtk_window_has_toplevel_focus(m_pWindow);
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

        if (isPositioningAllowed() && (nMask & WindowStateMask::X && nMask & WindowStateMask::Y))
        {
            gtk_window_move(m_pWindow, aData.GetX(), aData.GetY());
        }
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

    virtual void connect_toplevel_focus_changed(const Link<weld::Widget&, void>& rLink) override
    {
        assert(!m_nToplevelFocusChangedSignalId);
        m_nToplevelFocusChangedSignalId = g_signal_connect(m_pWindow, "notify::has-toplevel-focus", G_CALLBACK(signalToplevelFocusChanged), this);
        weld::Window::connect_toplevel_focus_changed(rLink);
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
        if (!bAlreadyVisible)
        {
            if (GTK_IS_DIALOG(m_pWindow))
                sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(m_pWindow))));
            gtk_widget_show(GTK_WIDGET(m_pWindow));
        }

        if (!bAlreadyRealized)
        {
            GtkAllocation allocation;
            gtk_widget_realize(GTK_WIDGET(m_pWindow));
            gtk_widget_get_allocation(GTK_WIDGET(m_pWindow), &allocation);
            gtk_widget_size_allocate(GTK_WIDGET(m_pWindow), &allocation);
        }

        VclPtr<VirtualDevice> xOutput(VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT));
        xOutput->SetOutputSizePixel(get_size());
        cairo_surface_t* pSurface = get_underlying_cairo_surface(*xOutput);
        cairo_t* cr = cairo_create(pSurface);

        Point aOffset = get_csd_offset(GTK_WIDGET(m_pWindow));

        cairo_translate(cr, -aOffset.X(), -aOffset.Y());

        gtk_widget_draw(GTK_WIDGET(m_pWindow), cr);

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

        gtk_container_foreach(GTK_CONTAINER(m_pWindow), do_collect_screenshot_data, &aRet);

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
        gulong nSignalDeleteId = g_signal_connect(m_pDialog, "delete-event", G_CALLBACK(signal_delete), this);
        gulong nSignalDestroyId = g_signal_connect(m_pDialog, "destroy", G_CALLBACK(signal_destroy), this);

        m_pLoop = g_main_loop_new(nullptr, false);
        m_nResponseId = GTK_RESPONSE_NONE;

        gdk_threads_leave();
        g_main_loop_run(m_pLoop);
        gdk_threads_enter();

        g_main_loop_unref(m_pLoop);

        m_pLoop = nullptr;

        if (!bWasModal)
            gtk_window_set_modal(m_pDialog, false);

        if (nSignalResponseId)
            g_signal_handler_disconnect(m_pDialog, nSignalResponseId);
        if (nSignalCancelId)
            g_signal_handler_disconnect(m_pDialog, nSignalCancelId);
        g_signal_handler_disconnect(m_pDialog, nSignalDeleteId);
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
    void hideUnless(GtkContainer *pTop, const winset& rVisibleWidgets,
        std::vector<GtkWidget*> &rWasVisibleWidgets)
    {
        GList* pChildren = gtk_container_get_children(pTop);
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
                hideUnless(GTK_CONTAINER(pChild), rVisibleWidgets, rWasVisibleWidgets);
            }
        }
        g_list_free(pChildren);
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
    int m_nOldBorderWidth; // border width for expanded dialog

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

    static void signalActivate(GtkMenuItem*, gpointer data)
    {
        bool* pActivate = static_cast<bool*>(data);
        *pActivate = true;
    }

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
        {
            gdk_threads_leave();
            g_main_loop_run(pLoop);
            gdk_threads_enter();
        }

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

    static gboolean signalScreenshotPopupMenu(GtkWidget*, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        return pThis->signal_screenshot_popup_menu(nullptr);
    }

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
        , m_nOldBorderWidth(0)
    {
        if (GTK_IS_DIALOG(m_pDialog) || GTK_IS_ASSISTANT(m_pDialog))
            m_nCloseSignalId = g_signal_connect(m_pDialog, "close", G_CALLBACK(signalClose), this);
        else
            m_nCloseSignalId = 0;
        const bool bScreenshotMode(officecfg::Office::Common::Misc::ScreenshotMode::get());
        if (bScreenshotMode)
        {
            g_signal_connect(m_pDialog, "popup-menu", G_CALLBACK(signalScreenshotPopupMenu), this);
            g_signal_connect(m_pDialog, "button-press-event", G_CALLBACK(signalScreenshotButton), this);
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
        m_nSignalDeleteId = g_signal_connect(m_pDialog, "delete-event", G_CALLBACK(signalAsyncDelete), this);

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
        m_nSignalDeleteId = g_signal_connect(m_pDialog, "delete-event", G_CALLBACK(signalAsyncDelete), this);

        return true;
    }

    GtkInstanceButton* has_click_handler(int nResponse);

    virtual int run() override;

    virtual void show() override
    {
        if (gtk_widget_get_visible(m_pWidget))
            return;
        if (GTK_IS_DIALOG(m_pDialog))
            sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog))));
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
        return new GtkInstanceContainer(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(m_pDialog))), m_pBuilder, false);
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
        //same again with pRefBtn, except stop if there's a
        //shared parent in the existing widgets
        for (GtkWidget *pCandidate = pRefBtn;
            pCandidate && pCandidate != pContentArea && gtk_widget_get_visible(pCandidate);
            pCandidate = gtk_widget_get_parent(pCandidate))
        {
            if (aVisibleWidgets.insert(pCandidate).second)
                break;
        }

        //hide everything except the aVisibleWidgets
        hideUnless(GTK_CONTAINER(pContentArea), aVisibleWidgets, m_aHiddenWidgets);

        gtk_widget_set_size_request(pRefEdit, m_nOldEditWidth, -1);
        m_nOldBorderWidth = gtk_container_get_border_width(GTK_CONTAINER(m_pDialog));
        gtk_container_set_border_width(GTK_CONTAINER(m_pDialog), 0);
        if (GtkWidget* pActionArea = gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog)))
            gtk_widget_hide(pActionArea);

        // calc's insert->function is springing back to its original size if the ref-button
        // is used to shrink the dialog down and then the user clicks in the calc area to do
        // the selection
#if defined(GDK_WINDOWING_WAYLAND)
        bool bWorkaroundSizeSpringingBack = DLSYM_GDK_IS_WAYLAND_DISPLAY(gtk_widget_get_display(m_pWidget));
        if (bWorkaroundSizeSpringingBack)
            gtk_widget_unmap(GTK_WIDGET(m_pDialog));
#endif

        resize_to_request();

#if defined(GDK_WINDOWING_WAYLAND)
        if (bWorkaroundSizeSpringingBack)
            gtk_widget_map(GTK_WIDGET(m_pDialog));
#endif

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
        gtk_container_set_border_width(GTK_CONTAINER(m_pDialog), m_nOldBorderWidth);
        if (GtkWidget* pActionArea = gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog)))
            gtk_widget_show(pActionArea);
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
        return new GtkInstanceContainer(GTK_CONTAINER(gtk_message_dialog_get_message_area(m_pMessageDialog)), m_pBuilder, false);
    }
};

class GtkInstanceAssistant : public GtkInstanceDialog, public virtual weld::Assistant
{
private:
    GtkAssistant* m_pAssistant;
    GtkWidget* m_pSidebar;
    GtkWidget* m_pSidebarEventBox;
    GtkButtonBox* m_pButtonBox;
    GtkButton* m_pHelp;
    GtkButton* m_pBack;
    GtkButton* m_pNext;
    GtkButton* m_pFinish;
    GtkButton* m_pCancel;
    gulong m_nButtonPressSignalId;
    std::vector<std::unique_ptr<GtkInstanceContainer>> m_aPages;
    std::map<OString, bool> m_aNotClickable;

    int find_page(const OString& rIdent) const
    {
        int nPages = gtk_assistant_get_n_pages(m_pAssistant);
        for (int i = 0; i < nPages; ++i)
        {
            GtkWidget* pPage = gtk_assistant_get_nth_page(m_pAssistant, i);
            const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pPage));
            if (g_strcmp0(pStr, rIdent.getStr()) == 0)
                return i;
        }
        return -1;
    }

    static void wrap_sidebar_label(GtkWidget *pWidget, gpointer /*user_data*/)
    {
        if (GTK_IS_LABEL(pWidget))
        {
            gtk_label_set_line_wrap(GTK_LABEL(pWidget), true);
            gtk_label_set_width_chars(GTK_LABEL(pWidget), 22);
            gtk_label_set_max_width_chars(GTK_LABEL(pWidget), 22);
        }
    }

    static void find_sidebar(GtkWidget *pWidget, gpointer user_data)
    {
        if (g_strcmp0(gtk_buildable_get_name(GTK_BUILDABLE(pWidget)), "sidebar") == 0)
        {
            GtkWidget **ppSidebar = static_cast<GtkWidget**>(user_data);
            *ppSidebar = pWidget;
        }
        if (GTK_IS_CONTAINER(pWidget))
            gtk_container_forall(GTK_CONTAINER(pWidget), find_sidebar, user_data);
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
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pSidebar));
        for (GList* pChild = g_list_first(pChildren); pChild; pChild = g_list_next(pChild))
        {
            GtkWidget* pWidget = static_cast<GtkWidget*>(pChild->data);
            if (!gtk_widget_get_visible(pWidget))
                continue;

            gtk_widget_get_allocation(pWidget, &allocation);

            gint dest_x1, dest_y1;
            gtk_widget_translate_coordinates(pWidget,
                                             m_pSidebarEventBox,
                                             0,
                                             0,
                                             &dest_x1,
                                             &dest_y1);

            gint dest_x2, dest_y2;
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
        g_list_free(pChildren);

        if (nNewCurrentPage != -1 && nNewCurrentPage != get_current_page())
        {
            OString sIdent = get_page_ident(nNewCurrentPage);
            if (!m_aNotClickable[sIdent] && !signal_jump_page(sIdent))
                set_current_page(nNewCurrentPage);
        }

        return false;
    }

public:
    GtkInstanceAssistant(GtkAssistant* pAssistant, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceDialog(GTK_WINDOW(pAssistant), pBuilder, bTakeOwnership)
        , m_pAssistant(pAssistant)
        , m_pSidebar(nullptr)
    {
        m_pButtonBox = GTK_BUTTON_BOX(gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL));
        gtk_button_box_set_layout(m_pButtonBox, GTK_BUTTONBOX_END);
        gtk_box_set_spacing(GTK_BOX(m_pButtonBox), 6);

        m_pBack = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Back)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pBack), true);
        gtk_buildable_set_name(GTK_BUILDABLE(m_pBack), "previous");
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pBack), false, false, 0);

        m_pNext = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Next)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pNext), true);
        gtk_buildable_set_name(GTK_BUILDABLE(m_pNext), "next");
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pNext), false, false, 0);

        m_pCancel = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Cancel)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pCancel), true);
        gtk_box_pack_end(GTK_BOX(m_pButtonBox), GTK_WIDGET(m_pCancel), false, false, 0);

        m_pFinish = GTK_BUTTON(gtk_button_new_with_mnemonic(MapToGtkAccelerator(GetStandardText(StandardButtonType::Finish)).getStr()));
        gtk_widget_set_can_default(GTK_WIDGET(m_pFinish), true);
        gtk_buildable_set_name(GTK_BUILDABLE(m_pFinish), "finish");
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

        gtk_widget_show_all(GTK_WIDGET(m_pButtonBox));

        find_sidebar(GTK_WIDGET(m_pAssistant), &m_pSidebar);

        m_pSidebarEventBox = ::ensureEventWidget(m_pSidebar);
        m_nButtonPressSignalId = m_pSidebarEventBox ? g_signal_connect(m_pSidebarEventBox, "button-press-event", G_CALLBACK(signalButton), this) : 0;
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
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pWidget));
        return OString(pStr, pStr ? strlen(pStr) : 0);
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
        gtk_container_forall(GTK_CONTAINER(m_pSidebar), wrap_sidebar_label, nullptr);
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
        gtk_container_forall(GTK_CONTAINER(m_pSidebar), wrap_sidebar_label, nullptr);
        g_object_unref(pPage);
    }

    virtual weld::Container* append_page(const OString& rIdent) override
    {
        disable_notify_events();

        GtkWidget *pChild = gtk_grid_new();
        gtk_buildable_set_name(GTK_BUILDABLE(pChild), rIdent.getStr());
        gtk_assistant_append_page(m_pAssistant, pChild);
        gtk_assistant_set_page_type(m_pAssistant, pChild, GTK_ASSISTANT_PAGE_CUSTOM);
        gtk_widget_show(pChild);

        enable_notify_events();

        m_aPages.emplace_back(new GtkInstanceContainer(GTK_CONTAINER(pChild), m_pBuilder, false));

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
        : GtkInstanceContainer(GTK_CONTAINER(pFrame), pBuilder, bTakeOwnership)
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
        : GtkInstanceContainer(GTK_CONTAINER(pPaned), pBuilder, bTakeOwnership)
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

static GType crippled_viewport_get_type();

#define CRIPPLED_TYPE_VIEWPORT            (crippled_viewport_get_type ())
#define CRIPPLED_VIEWPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CRIPPLED_TYPE_VIEWPORT, CrippledViewport))
#ifndef NDEBUG
#   define CRIPPLED_IS_VIEWPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CRIPPLED_TYPE_VIEWPORT))
#endif

namespace {

struct CrippledViewport
{
    GtkViewport viewport;

    GtkAdjustment  *hadjustment;
    GtkAdjustment  *vadjustment;
};

}

enum
{
    PROP_0,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY,
    PROP_SHADOW_TYPE
};

static void viewport_set_adjustment(CrippledViewport *viewport,
                                    GtkOrientation  orientation,
                                    GtkAdjustment  *adjustment)
{
    if (!adjustment)
        adjustment = gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        if (viewport->hadjustment)
            g_object_unref(viewport->hadjustment);
        viewport->hadjustment = adjustment;
    }
    else
    {
        if (viewport->vadjustment)
            g_object_unref(viewport->vadjustment);
        viewport->vadjustment = adjustment;
    }

    g_object_ref_sink(adjustment);
}

static void
crippled_viewport_set_property(GObject* object,
                               guint prop_id,
                               const GValue* value,
                               GParamSpec* /*pspec*/)
{
    CrippledViewport *viewport = CRIPPLED_VIEWPORT(object);

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
crippled_viewport_get_property(GObject* object,
                               guint prop_id,
                               GValue* value,
                               GParamSpec* /*pspec*/)
{
    CrippledViewport *viewport = CRIPPLED_VIEWPORT(object);

    switch (prop_id)
    {
        case PROP_HADJUSTMENT:
            g_value_set_object(value, viewport->hadjustment);
            break;
        case PROP_VADJUSTMENT:
            g_value_set_object(value, viewport->vadjustment);
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

static void crippled_viewport_class_init(GtkViewportClass *klass)
{
    GObjectClass* o_class = G_OBJECT_CLASS(klass);

    /* GObject signals */
    o_class->set_property = crippled_viewport_set_property;
    o_class->get_property = crippled_viewport_get_property;

    /* Properties */
    g_object_class_override_property(o_class, PROP_HADJUSTMENT,    "hadjustment");
    g_object_class_override_property(o_class, PROP_VADJUSTMENT,    "vadjustment");
    g_object_class_override_property(o_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property(o_class, PROP_VSCROLL_POLICY, "vscroll-policy");
}

GType crippled_viewport_get_type()
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo tinfo =
        {
            sizeof (GtkViewportClass),
            nullptr,  /* base init */
            nullptr,  /* base finalize */
            reinterpret_cast<GClassInitFunc>(crippled_viewport_class_init), /* class init */
            nullptr, /* class finalize */
            nullptr,                   /* class data */
            sizeof (CrippledViewport), /* instance size */
            0,                         /* nb preallocs */
            nullptr,  /* instance init */
            nullptr                    /* value table */
        };

        type = g_type_register_static( GTK_TYPE_VIEWPORT, "CrippledViewport",
                                       &tinfo, GTypeFlags(0));
    }

    return type;
}

#define CUSTOM_TYPE_CELL_RENDERER_SURFACE             (custom_cell_renderer_surface_get_type())
#define CUSTOM_CELL_RENDERER_SURFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),  CUSTOM_TYPE_CELL_RENDERER_SURFACE, CustomCellRendererSurface))
#define CUSTOM_IS_CELL_RENDERER_SURFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_TYPE_CELL_RENDERER_SURFACE))

namespace {

    struct CustomCellRendererSurface
    {
        GtkCellRendererText parent;
        VclPtr<VirtualDevice> device;
        gchar *id;
        gpointer instance;
    };

    struct CustomCellRendererSurfaceClass
    {
        GtkCellRendererTextClass parent_class;
    };

    enum
    {
        PROP_ID = 10000,
        PROP_INSTANCE_TREE_VIEW = 10001
    };
}

static gpointer custom_cell_renderer_surface_parent_class;

static GType custom_cell_renderer_surface_get_type();
static void custom_cell_renderer_surface_class_init(CustomCellRendererSurfaceClass *klass);

GType custom_cell_renderer_surface_get_type()
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo tinfo =
        {
            sizeof (CustomCellRendererSurfaceClass),
            nullptr, /* base init */
            nullptr, /* base finalize */
            reinterpret_cast<GClassInitFunc>(custom_cell_renderer_surface_class_init), /* class init */
            nullptr, /* class finalize */
            nullptr, /* class data */
            sizeof (CustomCellRendererSurface), /* instance size */
            0,       /* nb preallocs */
            nullptr, /* instance init */
            nullptr  /* value table */
        };

        // inherit from GtkCellRendererText so we can set the "text" property and get a11y support for that
        type = g_type_register_static(GTK_TYPE_CELL_RENDERER_TEXT, "CustomCellRendererSurface",
                                      &tinfo, GTypeFlags(0));
    }

    return type;
}

static void custom_cell_renderer_surface_get_property(GObject *object,
                                                      guint param_id,
                                                      GValue *value,
                                                      GParamSpec *pspec)
{
    CustomCellRendererSurface *cellsurface = CUSTOM_CELL_RENDERER_SURFACE(object);

    switch (param_id)
    {
        case PROP_ID:
            g_value_set_string(value, cellsurface->id);
            break;
        case PROP_INSTANCE_TREE_VIEW:
            g_value_set_pointer(value, cellsurface->instance);
            break;
        default:
            G_OBJECT_CLASS(custom_cell_renderer_surface_parent_class)->get_property(object, param_id, value, pspec);
            break;
    }
}

static void custom_cell_renderer_surface_set_property(GObject *object,
                                                      guint param_id,
                                                      const GValue *value,
                                                      GParamSpec *pspec)
{
    CustomCellRendererSurface *cellsurface = CUSTOM_CELL_RENDERER_SURFACE(object);

    switch (param_id)
    {
        case PROP_ID:
            g_free(cellsurface->id);
            cellsurface->id = g_value_dup_string(value);
            break;
        case PROP_INSTANCE_TREE_VIEW:
            cellsurface->instance = g_value_get_pointer(value);
            break;
        default:
            G_OBJECT_CLASS(custom_cell_renderer_surface_parent_class)->set_property(object, param_id, value, pspec);
            break;
    }
}

static bool custom_cell_renderer_surface_get_preferred_size(GtkCellRenderer *cell,
                                                            GtkOrientation orientation,
                                                            gint *minimum_size,
                                                            gint *natural_size);

static void custom_cell_renderer_surface_render(GtkCellRenderer* cell,
                                                cairo_t* cr,
                                                GtkWidget* widget,
                                                const GdkRectangle* background_area,
                                                const GdkRectangle* cell_area,
                                                GtkCellRendererState flags);

static void custom_cell_renderer_surface_finalize(GObject *object)
{
    CustomCellRendererSurface *cellsurface = CUSTOM_CELL_RENDERER_SURFACE(object);

    g_free(cellsurface->id);
    cellsurface->device.disposeAndClear();

    G_OBJECT_CLASS(custom_cell_renderer_surface_parent_class)->finalize(object);
}

static void custom_cell_renderer_surface_get_preferred_width(GtkCellRenderer *cell,
                                                             GtkWidget *widget,
                                                             gint *minimum_size,
                                                             gint *natural_size)
{
    if (!custom_cell_renderer_surface_get_preferred_size(cell, GTK_ORIENTATION_HORIZONTAL,
                                                         minimum_size, natural_size))
    {
        // fallback to parent if we're empty
        GTK_CELL_RENDERER_CLASS(custom_cell_renderer_surface_parent_class)->get_preferred_width(cell,
            widget, minimum_size, natural_size);
    }
}

static void custom_cell_renderer_surface_get_preferred_height(GtkCellRenderer *cell,
                                                              GtkWidget *widget,
                                                              gint *minimum_size,
                                                              gint *natural_size)
{
    if (!custom_cell_renderer_surface_get_preferred_size(cell, GTK_ORIENTATION_VERTICAL,
                                                         minimum_size, natural_size))
    {
        // fallback to parent if we're empty
        GTK_CELL_RENDERER_CLASS(custom_cell_renderer_surface_parent_class)->get_preferred_height(cell,
            widget, minimum_size, natural_size);
    }

}

static void custom_cell_renderer_surface_get_preferred_height_for_width(GtkCellRenderer *cell,
                                                                        GtkWidget *widget,
                                                                        gint /*width*/,
                                                                        gint *minimum_height,
                                                                        gint *natural_height)
{
    gtk_cell_renderer_get_preferred_height(cell, widget, minimum_height, natural_height);
}

static void custom_cell_renderer_surface_get_preferred_width_for_height(GtkCellRenderer *cell,
                                                                        GtkWidget *widget,
                                                                        gint /*height*/,
                                                                        gint *minimum_width,
                                                                        gint *natural_width)
{
    gtk_cell_renderer_get_preferred_width(cell, widget, minimum_width, natural_width);
}

void custom_cell_renderer_surface_class_init(CustomCellRendererSurfaceClass *klass)
{
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS(klass);
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    /* Hook up functions to set and get our custom cell renderer properties */
    object_class->get_property = custom_cell_renderer_surface_get_property;
    object_class->set_property = custom_cell_renderer_surface_set_property;

    custom_cell_renderer_surface_parent_class = g_type_class_peek_parent(klass);
    object_class->finalize = custom_cell_renderer_surface_finalize;

    cell_class->get_preferred_width = custom_cell_renderer_surface_get_preferred_width;
    cell_class->get_preferred_height = custom_cell_renderer_surface_get_preferred_height;
    cell_class->get_preferred_width_for_height = custom_cell_renderer_surface_get_preferred_width_for_height;
    cell_class->get_preferred_height_for_width = custom_cell_renderer_surface_get_preferred_height_for_width;

    cell_class->render = custom_cell_renderer_surface_render;

    g_object_class_install_property(object_class,
                                   PROP_ID,
                                   g_param_spec_string("id",
                                                       "ID",
                                                       "The ID of the custom data",
                                                       nullptr,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(object_class,
                                   PROP_INSTANCE_TREE_VIEW,
                                   g_param_spec_pointer("instance",
                                                        "Instance",
                                                        "The GtkInstanceTreeView",
                                                        G_PARAM_READWRITE));

    gtk_cell_renderer_class_set_accessible_type(cell_class, GTK_TYPE_TEXT_CELL_ACCESSIBLE);
}

static GtkCellRenderer* custom_cell_renderer_surface_new()
{
    return GTK_CELL_RENDERER(g_object_new(CUSTOM_TYPE_CELL_RENDERER_SURFACE, nullptr));
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
        : GtkInstanceContainer(GTK_CONTAINER(pScrolledWindow), pBuilder, bTakeOwnership)
        , m_pScrolledWindow(pScrolledWindow)
        , m_pOrigViewport(nullptr)
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
        GtkWidget *pViewport = gtk_bin_get_child(GTK_BIN(m_pScrolledWindow));
        assert(GTK_IS_VIEWPORT(pViewport));
        GtkWidget *pChild = gtk_bin_get_child(GTK_BIN(pViewport));
        g_object_ref(pChild);
        gtk_container_remove(GTK_CONTAINER(pViewport), pChild);
        g_object_ref(pViewport);
        gtk_container_remove(GTK_CONTAINER(m_pScrolledWindow), pViewport);
        GtkWidget* pNewViewport = GTK_WIDGET(g_object_new(crippled_viewport_get_type(), nullptr));
        gtk_widget_show(pNewViewport);
        gtk_container_add(GTK_CONTAINER(m_pScrolledWindow), pNewViewport);
        gtk_container_add(GTK_CONTAINER(pNewViewport), pChild);
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

    virtual int get_hscroll_height() const override
    {
        if (gtk_scrolled_window_get_overlay_scrolling(m_pScrolledWindow))
            return 0;
        return gtk_widget_get_allocated_height(gtk_scrolled_window_get_hscrollbar(m_pScrolledWindow));
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

    virtual int get_vscroll_width() const override
    {
        if (gtk_scrolled_window_get_overlay_scrolling(m_pScrolledWindow))
            return 0;
        return gtk_widget_get_allocated_width(gtk_scrolled_window_get_vscrollbar(m_pScrolledWindow));
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

    virtual ~GtkInstanceScrolledWindow() override
    {
        // we use GtkInstanceContainer::[disable|enable]_notify_events later on
        // to avoid touching these removed handlers
        g_signal_handler_disconnect(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_disconnect(m_pHAdjustment, m_nHAdjustChangedSignalId);

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

        GtkWidget *pViewport = gtk_bin_get_child(GTK_BIN(m_pScrolledWindow));
        assert(CRIPPLED_IS_VIEWPORT(pViewport));
        GtkWidget *pChild = gtk_bin_get_child(GTK_BIN(pViewport));
        g_object_ref(pChild);
        gtk_container_remove(GTK_CONTAINER(pViewport), pChild);
        g_object_ref(pViewport);
        gtk_container_remove(GTK_CONTAINER(m_pScrolledWindow), pViewport);

        gtk_container_add(GTK_CONTAINER(m_pScrolledWindow), m_pOrigViewport);
        g_object_unref(m_pOrigViewport);
        gtk_container_add(GTK_CONTAINER(m_pOrigViewport), pChild);
        g_object_unref(pChild);
        gtk_widget_destroy(pViewport);
        g_object_unref(pViewport);
        m_pOrigViewport = nullptr;
        GtkInstanceContainer::enable_notify_events();
    }
};

class GtkInstanceNotebook : public GtkInstanceContainer, public virtual weld::Notebook
{
private:
    GtkNotebook* m_pNotebook;
    GtkBox* m_pOverFlowBox;
    GtkNotebook* m_pOverFlowNotebook;
    gulong m_nSwitchPageSignalId;
    gulong m_nOverFlowSwitchPageSignalId;
    gulong m_nSizeAllocateSignalId;
    gulong m_nFocusSignalId;
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

        gtk_notebook_set_current_page(m_pNotebook, nNewPage);

        enable_notify_events();

        // trigger main notebook switch-page callback
        OString sNewIdent(get_page_ident(m_pNotebook, nNewPage));
        m_aEnterPageHdl.Call(sNewIdent);
    }

    static OString get_page_ident(GtkNotebook *pNotebook, guint nPage)
    {
        const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(pNotebook, gtk_notebook_get_nth_page(pNotebook, nPage));
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
        return OString(pStr, pStr ? strlen(pStr) : 0);
    }

    static gint get_page_number(GtkNotebook *pNotebook, const OString& rIdent)
    {
        gint nPages = gtk_notebook_get_n_pages(pNotebook);
        for (gint i = 0; i < nPages; ++i)
        {
            const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(pNotebook, gtk_notebook_get_nth_page(pNotebook, i));
            const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
            if (pStr && strcmp(pStr, rIdent.getStr()) == 0)
                return i;
        }
        return -1;
    }

    int remove_page(GtkNotebook *pNotebook, const OString& rIdent)
    {
        disable_notify_events();
        int nPageNumber = get_page_number(pNotebook, rIdent);
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
        gtk_buildable_set_name(GTK_BUILDABLE(pTabWidget), "useless");

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
        gtk_buildable_set_name(GTK_BUILDABLE(pTabWidget), rIdent.getStr());

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
        gtk_container_add(GTK_CONTAINER(pParent), GTK_WIDGET(m_pOverFlowBox));
        gtk_box_pack_start(m_pOverFlowBox, GTK_WIDGET(m_pOverFlowNotebook), false, false, 0);
        g_object_ref(m_pNotebook);
        gtk_container_remove(GTK_CONTAINER(pParent), GTK_WIDGET(m_pNotebook));
        gtk_box_pack_start(m_pOverFlowBox, GTK_WIDGET(m_pNotebook), true, true, 0);
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
        gtk_notebook_set_scrollable(m_pNotebook, false);

        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pNotebook));
        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pOverFlowNotebook));

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

        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pOverFlowNotebook));
        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pNotebook));

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
                if (!gtk_widget_get_child_visible(pTabWidget))
                {
                    m_nLaunchSplitTimeoutId = g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, reinterpret_cast<GSourceFunc>(launch_split_notebooks), this, nullptr);
                    break;
                }
            }
        }
        enable_notify_events();
    }

    static void signalSizeAllocate(GtkWidget*, GdkRectangle*, gpointer widget)
    {
        GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
        pThis->signal_notebook_size_allocate();
    }

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
        : GtkInstanceContainer(GTK_CONTAINER(pNotebook), pBuilder, bTakeOwnership)
        , m_pNotebook(pNotebook)
        , m_pOverFlowBox(nullptr)
        , m_pOverFlowNotebook(GTK_NOTEBOOK(gtk_notebook_new()))
        , m_nSwitchPageSignalId(g_signal_connect(pNotebook, "switch-page", G_CALLBACK(signalSwitchPage), this))
        , m_nOverFlowSwitchPageSignalId(g_signal_connect(m_pOverFlowNotebook, "switch-page", G_CALLBACK(signalOverFlowSwitchPage), this))
        , m_nFocusSignalId(g_signal_connect(pNotebook, "focus", G_CALLBACK(signalFocus), this))
        , m_nChangeCurrentPageId(g_signal_connect(pNotebook, "change-current-page", G_CALLBACK(signalChangeCurrentPage), this))
        , m_nLaunchSplitTimeoutId(0)
        , m_bOverFlowBoxActive(false)
        , m_bOverFlowBoxIsStart(false)
        , m_bInternalPageChange(false)
        , m_nStartTabCount(0)
        , m_nEndTabCount(0)
    {
        gtk_widget_add_events(GTK_WIDGET(pNotebook), GDK_SCROLL_MASK);
        if (get_n_pages() > 6)
            m_nSizeAllocateSignalId = g_signal_connect_after(pNotebook, "size-allocate", G_CALLBACK(signalSizeAllocate), this);
        else
            m_nSizeAllocateSignalId = 0;
        gtk_notebook_set_show_border(m_pOverFlowNotebook, false);

        // tdf#122623 it's nigh impossible to have a GtkNotebook without an active (checked) tab, so try and theme
        // the unwanted tab into invisibility
        GtkStyleContext *pNotebookContext = gtk_widget_get_style_context(GTK_WIDGET(m_pOverFlowNotebook));
        GtkCssProvider *pProvider = gtk_css_provider_new();
        static const gchar data[] = "header.top > tabs > tab:checked { box-shadow: none; padding: 0 0 0 0; margin: 0 0 0 0; border-image: none; border-image-width: 0 0 0 0; background-image: none; background-color: transparent; border-radius: 0 0 0 0; border-width: 0 0 0 0; border-style: none; border-color: transparent; opacity: 0; min-height: 0; min-width: 0; }";
        gtk_css_provider_load_from_data(pProvider, data, -1, nullptr);
        gtk_style_context_add_provider(pNotebookContext, GTK_STYLE_PROVIDER(pProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
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

        GtkContainer* pChild;
        if (m_bOverFlowBoxIsStart)
        {
            auto nOverFlowLen = m_bOverFlowBoxActive ? gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1 : 0;
            if (nPage < nOverFlowLen)
                pChild = GTK_CONTAINER(gtk_notebook_get_nth_page(m_pOverFlowNotebook, nPage));
            else
            {
                nPage -= nOverFlowLen;
                pChild = GTK_CONTAINER(gtk_notebook_get_nth_page(m_pNotebook, nPage));
            }
        }
        else
        {
            auto nMainLen = gtk_notebook_get_n_pages(m_pNotebook);
            if (nPage < nMainLen)
                pChild = GTK_CONTAINER(gtk_notebook_get_nth_page(m_pNotebook, nPage));
            else
            {
                nPage -= nMainLen;
                pChild = GTK_CONTAINER(gtk_notebook_get_nth_page(m_pOverFlowNotebook, nPage));
            }
        }

        unsigned int nPageIndex = static_cast<unsigned int>(nPage);
        if (m_aPages.size() < nPageIndex + 1)
            m_aPages.resize(nPageIndex + 1);
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new GtkInstanceContainer(pChild, m_pBuilder, false));
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
        g_signal_handler_block(m_pNotebook, m_nFocusSignalId);
        g_signal_handler_block(m_pNotebook, m_nChangeCurrentPageId);
        g_signal_handler_block(m_pOverFlowNotebook, m_nOverFlowSwitchPageSignalId);
        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pOverFlowNotebook));
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pOverFlowNotebook));
        g_signal_handler_unblock(m_pOverFlowNotebook, m_nOverFlowSwitchPageSignalId);
        g_signal_handler_unblock(m_pNotebook, m_nSwitchPageSignalId);
        g_signal_handler_unblock(m_pNotebook, m_nFocusSignalId);
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
        if (m_nSizeAllocateSignalId)
            g_signal_handler_disconnect(m_pNotebook, m_nSizeAllocateSignalId);
        g_signal_handler_disconnect(m_pNotebook, m_nSwitchPageSignalId);
        g_signal_handler_disconnect(m_pNotebook, m_nFocusSignalId);
        g_signal_handler_disconnect(m_pNotebook, m_nChangeCurrentPageId);
        g_signal_handler_disconnect(m_pOverFlowNotebook, m_nOverFlowSwitchPageSignalId);
        gtk_widget_destroy(GTK_WIDGET(m_pOverFlowNotebook));
        if (m_pOverFlowBox)
        {
            // put it back to how we found it initially
            GtkWidget* pParent = gtk_widget_get_parent(GTK_WIDGET(m_pOverFlowBox));
            g_object_ref(m_pNotebook);
            gtk_container_remove(GTK_CONTAINER(m_pOverFlowBox), GTK_WIDGET(m_pNotebook));
            gtk_container_add(GTK_CONTAINER(pParent), GTK_WIDGET(m_pNotebook));
            g_object_unref(m_pNotebook);

            gtk_widget_destroy(GTK_WIDGET(m_pOverFlowBox));
        }
    }
};

PangoAttrList* create_attr_list(const vcl::Font& rFont)
{
    PangoAttrList* pAttrList = pango_attr_list_new();
    pango_attr_list_insert(pAttrList, pango_attr_family_new(OUStringToOString(rFont.GetFamilyName(), RTL_TEXTENCODING_UTF8).getStr()));
    pango_attr_list_insert(pAttrList, pango_attr_size_new(rFont.GetFontSize().Height() * PANGO_SCALE));
    switch (rFont.GetItalic())
    {
        case ITALIC_NONE:
            pango_attr_list_insert(pAttrList, pango_attr_style_new(PANGO_STYLE_NORMAL));
            break;
        case ITALIC_NORMAL:
            pango_attr_list_insert(pAttrList, pango_attr_style_new(PANGO_STYLE_ITALIC));
            break;
        case ITALIC_OBLIQUE:
            pango_attr_list_insert(pAttrList, pango_attr_style_new(PANGO_STYLE_OBLIQUE));
            break;
        default:
            break;
    }
    switch (rFont.GetWeight())
    {
        case WEIGHT_ULTRALIGHT:
            pango_attr_list_insert(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_ULTRALIGHT));
            break;
        case WEIGHT_LIGHT:
            pango_attr_list_insert(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_LIGHT));
            break;
        case WEIGHT_NORMAL:
            pango_attr_list_insert(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_NORMAL));
            break;
        case WEIGHT_BOLD:
            pango_attr_list_insert(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
            break;
        case WEIGHT_ULTRABOLD:
            pango_attr_list_insert(pAttrList, pango_attr_weight_new(PANGO_WEIGHT_ULTRABOLD));
            break;
        default:
            break;
    }
    switch (rFont.GetWidthType())
    {
        case WIDTH_ULTRA_CONDENSED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_ULTRA_CONDENSED));
            break;
        case WIDTH_EXTRA_CONDENSED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_EXTRA_CONDENSED));
            break;
        case WIDTH_CONDENSED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_CONDENSED));
            break;
        case WIDTH_SEMI_CONDENSED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_SEMI_CONDENSED));
            break;
        case WIDTH_NORMAL:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_NORMAL));
            break;
        case WIDTH_SEMI_EXPANDED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_SEMI_EXPANDED));
            break;
        case WIDTH_EXPANDED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_EXPANDED));
            break;
        case WIDTH_EXTRA_EXPANDED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_EXTRA_EXPANDED));
            break;
        case WIDTH_ULTRA_EXPANDED:
            pango_attr_list_insert(pAttrList, pango_attr_stretch_new(PANGO_STRETCH_ULTRA_EXPANDED));
            break;
        default:
            break;
    }
    return pAttrList;
}

void set_font(GtkLabel* pLabel, const vcl::Font& rFont)
{
    // TODO?, clear old props like set_text_foreground_color does
    PangoAttrList* pAttrList = create_attr_list(rFont);
    gtk_label_set_attributes(pLabel, pAttrList);
    pango_attr_list_unref(pAttrList);
}

class GtkInstanceButton : public GtkInstanceContainer, public virtual weld::Button
{
private:
    GtkButton* m_pButton;
    gulong m_nSignalId;
    std::unique_ptr<vcl::Font> m_xFont;

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

    static GtkWidget* find_label_widget(GtkContainer* pContainer)
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

protected:
    GtkWidget* get_label_widget()
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pButton));
        if (GTK_IS_ALIGNMENT(pChild))
            pChild = gtk_bin_get_child(GTK_BIN(pChild));

        if (GTK_IS_CONTAINER(pChild))
            pChild = find_label_widget(GTK_CONTAINER(pChild));
        else if (!GTK_IS_LABEL(pChild))
            pChild = nullptr;

        return pChild;
    }

public:
    GtkInstanceButton(GtkButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_nSignalId(g_signal_connect(pButton, "clicked", G_CALLBACK(signalClicked), this))
    {
        g_object_set_data(G_OBJECT(m_pButton), "g-lo-GtkInstanceButton", this);
    }

    virtual void set_label(const OUString& rText) override
    {
        ::set_label(m_pButton, rText);
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        gtk_button_set_always_show_image(m_pButton, true);
        gtk_button_set_image_position(m_pButton, GTK_POS_LEFT);
        if (pDevice)
            gtk_button_set_image(m_pButton, image_new_from_virtual_device(*pDevice));
        else
            gtk_button_set_image(m_pButton, nullptr);
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        GdkPixbuf* pixbuf = load_icon_by_name(rIconName);
        if (!pixbuf)
            gtk_button_set_image(m_pButton, nullptr);
        else
        {
            gtk_button_set_image(m_pButton, gtk_image_new_from_pixbuf(pixbuf));
            g_object_unref(pixbuf);
        }
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        GdkPixbuf* pixbuf = getPixbuf(rImage);
        if (!pixbuf)
            gtk_button_set_image(m_pButton, nullptr);
        else
        {
            gtk_button_set_image(m_pButton, gtk_image_new_from_pixbuf(pixbuf));
            g_object_unref(pixbuf);
        }
    }

    virtual OUString get_label() const override
    {
        return ::get_label(m_pButton);
    }

    virtual void set_label_line_wrap(bool wrap) override
    {
        GtkWidget* pChild = get_label_widget();
        gtk_label_set_line_wrap(GTK_LABEL(pChild), wrap);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_xFont.reset(new vcl::Font(rFont));
        GtkWidget* pChild = get_label_widget();
        ::set_font(GTK_LABEL(pChild), rFont);
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
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
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
    if (GTK_IS_DIALOG(m_pDialog))
        sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(m_pDialog))));
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
private:
    gulong m_nSignalId;

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
        , m_nSignalId(g_signal_connect(m_pToggleButton, "toggled", G_CALLBACK(signalToggled), this))
    {
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
        gtk_toggle_button_set_inconsistent(m_pToggleButton, false);
        gtk_toggle_button_set_active(m_pToggleButton, active);
        enable_notify_events();
    }

    virtual bool get_active() const override
    {
        return gtk_toggle_button_get_active(m_pToggleButton);
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
        gtk_toggle_button_set_inconsistent(m_pToggleButton, inconsistent);
    }

    virtual bool get_inconsistent() const override
    {
        return gtk_toggle_button_get_inconsistent(m_pToggleButton);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pToggleButton, m_nSignalId);
        GtkInstanceButton::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceButton::enable_notify_events();
        g_signal_handler_unblock(m_pToggleButton, m_nSignalId);
    }

    virtual ~GtkInstanceToggleButton() override
    {
        g_signal_handler_disconnect(m_pToggleButton, m_nSignalId);
    }
};

void do_grab(GtkWidget* pWidget)
{
    GdkDisplay *pDisplay = gtk_widget_get_display(pWidget);
    GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
    gdk_seat_grab(pSeat, gtk_widget_get_window(pWidget),
                  GDK_SEAT_CAPABILITY_ALL, true, nullptr, nullptr, nullptr, nullptr);
}

void do_ungrab(GtkWidget* pWidget)
{
    GdkDisplay *pDisplay = gtk_widget_get_display(pWidget);
    GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
    gdk_seat_ungrab(pSeat);
}

GtkPositionType show_menu_older_gtk(GtkWidget* pMenuButton, GtkWindow* pMenu)
{
    //place the toplevel just below its launcher button
    GtkWidget* pToplevel = gtk_widget_get_toplevel(pMenuButton);
    gint x, y, absx, absy;
    gtk_widget_translate_coordinates(pMenuButton, pToplevel, 0, 0, &x, &y);
    GdkWindow *pWindow = gtk_widget_get_window(pToplevel);
    gdk_window_get_position(pWindow, &absx, &absy);

    x += absx;
    y += absy;

    gint nButtonHeight = gtk_widget_get_allocated_height(pMenuButton);
    y += nButtonHeight;

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
        gint nButtonWidth = gtk_widget_get_allocated_width(pMenuButton);
        x += nButtonWidth;
        x -= nMenuWidth;
    }

    tools::Rectangle aWorkArea(::get_monitor_workarea(pMenuButton));

    // shrink it a little, I find it reassuring to see a little margin with a
    // long menu to know the menu is fully on screen
    aWorkArea.AdjustTop(8);
    aWorkArea.AdjustBottom(-8);
    gint endx = x + nMenuWidth;
    if (endx > aWorkArea.Right())
        x -= endx - aWorkArea.Right();
    if (x < 0)
        x = 0;

    GtkPositionType ePosUsed = GTK_POS_BOTTOM;

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

    gtk_window_move(pMenu, x, y);

    return ePosUsed;
}

bool show_menu_newer_gtk(GtkWidget* pComboBox, GtkWindow* pMenu)
{
    static auto window_move_to_rect = reinterpret_cast<void (*) (GdkWindow*, const GdkRectangle*, GdkGravity,
                                                                 GdkGravity, GdkAnchorHints, gint, gint)>(
                                                                    dlsym(nullptr, "gdk_window_move_to_rect"));
    if (!window_move_to_rect)
        return false;

#if defined(GDK_WINDOWING_X11)
    // under wayland gdk_window_move_to_rect works great for me, but in my current
    // gtk 3.24 under X it leaves part of long menus outside the work area
    GdkDisplay *pDisplay = gtk_widget_get_display(pComboBox);
    if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
        return false;
#endif

    //place the toplevel just below its launcher button
    GtkWidget* pToplevel = gtk_widget_get_toplevel(pComboBox);
    gint x, y;
    gtk_widget_translate_coordinates(pComboBox, pToplevel, 0, 0, &x, &y);

    gtk_widget_realize(GTK_WIDGET(pMenu));
    gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(pToplevel)), pMenu);
    gtk_window_set_transient_for(pMenu, GTK_WINDOW(pToplevel));

    gint nComboWidth = gtk_widget_get_allocated_width(pComboBox);
    gint nComboHeight = gtk_widget_get_allocated_height(pComboBox);

    bool bSwapForRTL = SwapForRTL(GTK_WIDGET(pComboBox));

    GdkGravity rect_anchor = !bSwapForRTL ? GDK_GRAVITY_SOUTH_WEST : GDK_GRAVITY_SOUTH_EAST;
    GdkGravity menu_anchor = !bSwapForRTL ? GDK_GRAVITY_NORTH_WEST : GDK_GRAVITY_NORTH_EAST;
    GdkRectangle rect {x, y, nComboWidth, nComboHeight };
    GdkWindow* toplevel = gtk_widget_get_window(GTK_WIDGET(pMenu));

    window_move_to_rect(toplevel, &rect, rect_anchor, menu_anchor,
                        static_cast<GdkAnchorHints>(GDK_ANCHOR_FLIP_Y | GDK_ANCHOR_RESIZE_Y |
                                                    GDK_ANCHOR_SLIDE_X | GDK_ANCHOR_RESIZE_X),
                        0, 0);

    return true;
}

GtkPositionType show_menu(GtkWidget* pMenuButton, GtkWindow* pMenu)
{
    // we only use ePosUsed in the replacement-for-X-popover case of a
    // MenuButton, so we only need it when show_menu_older_gtk is used
    GtkPositionType ePosUsed = GTK_POS_BOTTOM;

    // tdf#120764 It isn't allowed under wayland to have two visible popups that share
    // the same top level parent. The problem is that since gtk 3.24 tooltips are also
    // implemented as popups, which means that we cannot show any popup if there is a
    // visible tooltip.
    GtkWidget* pParent = gtk_widget_get_toplevel(pMenuButton);
    GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
    if (pFrame)
    {
        // hide any current tooltip
        pFrame->HideTooltip();
        // don't allow any more to appear until menu is dismissed
        pFrame->BlockTooltip();
    }

    // try with gdk_window_move_to_rect, but if that's not available, try without
    if (!show_menu_newer_gtk(pMenuButton, pMenu))
        ePosUsed = show_menu_older_gtk(pMenuButton, pMenu);
    gtk_widget_show_all(GTK_WIDGET(pMenu));
    gtk_widget_grab_focus(GTK_WIDGET(pMenu));
    do_grab(GTK_WIDGET(pMenu));

    return ePosUsed;
}

class GtkInstanceMenuButton : public GtkInstanceToggleButton, public MenuHelper, public virtual weld::MenuButton
{
protected:
    GtkMenuButton* m_pMenuButton;
private:
    GtkBox* m_pBox;
    GtkImage* m_pImage;
    GtkWidget* m_pLabel;
    //popover cannot escape dialog under X so stick up own window instead
    GtkWindow* m_pMenuHack;
    //when doing so, if it's a toolbar menubutton align the menu to the full toolitem
    GtkWidget* m_pMenuHackAlign;
    GtkWidget* m_pPopover;
    gulong m_nSignalId;

    static void signalToggled(GtkWidget*, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->toggle_menu();
    }

    void toggle_menu()
    {
        if (!m_pMenuHack)
            return;
        if (!get_active())
        {
            do_ungrab(GTK_WIDGET(m_pMenuHack));

            gtk_widget_hide(GTK_WIDGET(m_pMenuHack));
            //put contents back from where the came from
            GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pMenuHack));
            g_object_ref(pChild);
            gtk_container_remove(GTK_CONTAINER(m_pMenuHack), pChild);
            gtk_container_add(GTK_CONTAINER(m_pPopover), pChild);
            g_object_unref(pChild);

            // so gdk_window_move_to_rect will work again the next time
            gtk_widget_unrealize(GTK_WIDGET(m_pMenuHack));

            gtk_widget_set_size_request(GTK_WIDGET(m_pMenuHack), -1, -1);

            // undo show_menu tooltip blocking
            GtkWidget* pParent = gtk_widget_get_toplevel(GTK_WIDGET(m_pMenuButton));
            GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
            if (pFrame)
                pFrame->UnblockTooltip();
        }
        else
        {
            //set border width
            gtk_container_set_border_width(GTK_CONTAINER(m_pMenuHack), gtk_container_get_border_width(GTK_CONTAINER(m_pPopover)));

            //steal popover contents and smuggle into toplevel display window
            GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pPopover));
            g_object_ref(pChild);
            gtk_container_remove(GTK_CONTAINER(m_pPopover), pChild);
            gtk_container_add(GTK_CONTAINER(m_pMenuHack), pChild);
            g_object_unref(pChild);

            GtkPositionType ePosUsed = show_menu(m_pMenuHackAlign ? m_pMenuHackAlign : GTK_WIDGET(m_pMenuButton), m_pMenuHack);
            // tdf#132540 keep the placeholder popover on this same side as the replacement menu
            gtk_popover_set_position(gtk_menu_button_get_popover(m_pMenuButton), ePosUsed);
        }
    }

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
        else
        {
            //try and regrab, so when we lose the grab to the menu of the color palette
            //combobox we regain it so the color palette doesn't itself disappear on next
            //click on the color palette combobox
            do_grab(GTK_WIDGET(m_pMenuHack));
        }
    }

    static gboolean signalButtonRelease(GtkWidget* pWidget, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        return pThis->button_release(pWidget, pEvent);
    }

    bool button_release(GtkWidget* pWidget, GdkEventButton* pEvent)
    {
        //we want to pop down if the button was released outside our popup
        gdouble x = pEvent->x_root;
        gdouble y = pEvent->y_root;
        gint xoffset, yoffset;
        gdk_window_get_root_origin(gtk_widget_get_window(pWidget), &xoffset, &yoffset);

        GtkAllocation alloc;
        gtk_widget_get_allocation(pWidget, &alloc);
        xoffset += alloc.x;
        yoffset += alloc.y;

        gtk_widget_get_allocation(GTK_WIDGET(m_pMenuHack), &alloc);
        gint x1 = alloc.x + xoffset;
        gint y1 = alloc.y + yoffset;
        gint x2 = x1 + alloc.width;
        gint y2 = y1 + alloc.height;

        if (x > x1 && x < x2 && y > y1 && y < y2)
            return false;

        set_active(false);

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

    void ensure_image_widget()
    {
        if (!m_pImage)
        {
            m_pImage = GTK_IMAGE(gtk_image_new());
            gtk_box_pack_start(m_pBox, GTK_WIDGET(m_pImage), false, false, 0);
            gtk_box_reorder_child(m_pBox, GTK_WIDGET(m_pImage), 0);
            gtk_widget_show(GTK_WIDGET(m_pImage));
        }
    }

public:
    GtkInstanceMenuButton(GtkMenuButton* pMenuButton, GtkWidget* pMenuAlign, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pMenuButton), pBuilder, bTakeOwnership)
        , MenuHelper(gtk_menu_button_get_popup(pMenuButton), false)
        , m_pMenuButton(pMenuButton)
        , m_pImage(nullptr)
        , m_pMenuHack(nullptr)
        , m_pMenuHackAlign(pMenuAlign)
        , m_pPopover(nullptr)
        , m_nSignalId(0)
    {
        m_pLabel = gtk_bin_get_child(GTK_BIN(m_pMenuButton));
        m_pBox = formatMenuButton(m_pLabel);
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
        if (pDevice)
            gtk_image_set_from_surface(m_pImage, get_underlying_cairo_surface(*pDevice));
        else
            gtk_image_set_from_surface(m_pImage, nullptr);
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        ensure_image_widget();
        GdkPixbuf* pixbuf = getPixbuf(rImage);
        if (pixbuf)
        {
            gtk_image_set_from_pixbuf(m_pImage, pixbuf);
            g_object_unref(pixbuf);
        }
        else
            gtk_image_set_from_surface(m_pImage, nullptr);
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
        clear_items();
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

    virtual void set_item_help_id(const OString& rIdent, const OString& rHelpId) override
    {
        MenuHelper::set_item_help_id(rIdent, rHelpId);
    }

    virtual OString get_item_help_id(const OString& rIdent) const override
    {
        return MenuHelper::get_item_help_id(rIdent);
    }

    virtual void signal_activate(GtkMenuItem* pItem) override
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pItem));
        signal_selected(OString(pStr, pStr ? strlen(pStr) : 0));
    }

    virtual void set_popover(weld::Widget* pPopover) override
    {
        GtkInstanceWidget* pPopoverWidget = dynamic_cast<GtkInstanceWidget*>(pPopover);
        m_pPopover = pPopoverWidget ? pPopoverWidget->getWidget() : nullptr;

#if defined(GDK_WINDOWING_X11)
        if (!m_pMenuHack)
        {
            //under wayland a Popover will work to "escape" the parent dialog, not
            //so under X, so come up with this hack to use a raw GtkWindow
            GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
            if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
            {
                m_pMenuHack = GTK_WINDOW(gtk_window_new(GTK_WINDOW_POPUP));
                gtk_window_set_type_hint(m_pMenuHack, GDK_WINDOW_TYPE_HINT_COMBO);
                gtk_window_set_modal(m_pMenuHack, true);
                gtk_window_set_resizable(m_pMenuHack, false);
                m_nSignalId = g_signal_connect(GTK_TOGGLE_BUTTON(m_pMenuButton), "toggled", G_CALLBACK(signalToggled), this);
                g_signal_connect(m_pMenuHack, "grab-broken-event", G_CALLBACK(signalGrabBroken), this);
                g_signal_connect(m_pMenuHack, "button-release-event", G_CALLBACK(signalButtonRelease), this);
                g_signal_connect(m_pMenuHack, "key-press-event", G_CALLBACK(keyPress), this);
            }
        }
#endif

        if (m_pMenuHack)
        {
            GtkWidget* pPlaceHolder = gtk_popover_new(GTK_WIDGET(m_pMenuButton));
            gtk_popover_set_transitions_enabled(GTK_POPOVER(pPlaceHolder), false);

            // tdf#132540 theme the unwanted popover into invisibility
            GtkStyleContext *pPopoverContext = gtk_widget_get_style_context(pPlaceHolder);
            GtkCssProvider *pProvider = gtk_css_provider_new();
            static const gchar data[] = "popover { box-shadow: none; padding: 0 0 0 0; margin: 0 0 0 0; border-image: none; border-image-width: 0 0 0 0; background-image: none; background-color: transparent; border-radius: 0 0 0 0; border-width: 0 0 0 0; border-style: none; border-color: transparent; opacity: 0; min-height: 0; min-width: 0; }";
            gtk_css_provider_load_from_data(pProvider, data, -1, nullptr);
            gtk_style_context_add_provider(pPopoverContext, GTK_STYLE_PROVIDER(pProvider),
                                           GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

            gtk_menu_button_set_popover(m_pMenuButton, pPlaceHolder);
        }
        else
        {
            gtk_menu_button_set_popover(m_pMenuButton, m_pPopover);
            if (m_pPopover)
                gtk_widget_show_all(m_pPopover);
        }
    }

    void set_menu(weld::Menu* pMenu);

    static GtkBox* formatMenuButton(GtkWidget* pLabel)
    {
        // format the GtkMenuButton "manually" so we can have the dropdown image in GtkMenuButtons shown
        // on the right at the same time as an image is shown on the left
        g_object_ref(pLabel);
        GtkWidget* pContainer = gtk_widget_get_parent(pLabel);
        gtk_container_remove(GTK_CONTAINER(pContainer), pLabel);

        gint nImageSpacing(2);
        GtkStyleContext *pContext = gtk_widget_get_style_context(pContainer);
        gtk_style_context_get_style(pContext, "image-spacing", &nImageSpacing, nullptr);
        GtkBox* pBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, nImageSpacing));

        gtk_box_pack_start(pBox, pLabel, true, true, 0);
        g_object_unref(pLabel);

        if (gtk_toggle_button_get_mode(GTK_TOGGLE_BUTTON(pContainer)))
            gtk_box_pack_end(pBox, gtk_image_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_BUTTON), false, false, 0);

        gtk_container_add(GTK_CONTAINER(pContainer), GTK_WIDGET(pBox));
        gtk_widget_show_all(GTK_WIDGET(pBox));

        return pBox;
    }

    virtual ~GtkInstanceMenuButton() override
    {
        if (m_pMenuHack)
        {
            g_signal_handler_disconnect(m_pMenuButton, m_nSignalId);
            gtk_menu_button_set_popover(m_pMenuButton, nullptr);
            gtk_widget_destroy(GTK_WIDGET(m_pMenuHack));
        }
    }
};

class GtkInstanceMenuToggleButton : public GtkInstanceToggleButton, public MenuHelper
                                  , public virtual weld::MenuToggleButton
{
private:
    GtkContainer* m_pContainer;
    GtkButton* m_pToggleMenuButton;
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
        gulong nSignalId = g_signal_connect_swapped(G_OBJECT(m_pMenu), "deactivate", G_CALLBACK(g_main_loop_quit), pLoop);

#if GTK_CHECK_VERSION(3,22,0)
        if (gtk_check_version(3, 22, 0) == nullptr)
        {
            // Send a keyboard event through gtk_main_do_event to toggle any active tooltip offs
            // before trying to launch the menu
            // https://gitlab.gnome.org/GNOME/gtk/issues/1785
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

        if (g_main_loop_is_running(pLoop))
        {
            gdk_threads_leave();
            g_main_loop_run(pLoop);
            gdk_threads_enter();
        }
        g_main_loop_unref(pLoop);
        g_signal_handler_disconnect(m_pMenu, nSignalId);
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
        , MenuHelper(gtk_menu_button_get_popup(pMenuButton), false)
        , m_pContainer(GTK_CONTAINER(gtk_builder_get_object(pMenuToggleButtonBuilder, "box")))
        , m_pToggleMenuButton(GTK_BUTTON(gtk_builder_get_object(pMenuToggleButtonBuilder, "menubutton")))
        , m_nMenuBtnClickedId(g_signal_connect(m_pToggleMenuButton, "clicked", G_CALLBACK(signalMenuBtnClicked), this))
        , m_nToggleStateFlagsChangedId(g_signal_connect(m_pToggleButton, "state-flags-changed", G_CALLBACK(signalToggleStateFlagsChanged), this))
        , m_nMenuBtnStateFlagsChangedId(g_signal_connect(m_pToggleMenuButton, "state-flags-changed", G_CALLBACK(signalMenuBtnStateFlagsChanged), this))
    {
        GtkInstanceMenuButton::formatMenuButton(gtk_bin_get_child(GTK_BIN(pMenuButton)));

        insertAsParent(GTK_WIDGET(pMenuButton), GTK_WIDGET(m_pContainer));
        gtk_widget_hide(GTK_WIDGET(pMenuButton));

        // move the first GtkMenuButton child, as created by GtkInstanceMenuButton ctor, into the GtkToggleButton
        // instead, leaving just the indicator behind in the GtkMenuButton
        GtkWidget* pButtonBox = gtk_bin_get_child(GTK_BIN(pMenuButton));
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

        // match the GtkToggleButton relief to the GtkMenuButton
        GtkReliefStyle eStyle = gtk_button_get_relief(GTK_BUTTON(pMenuButton));
        gtk_button_set_relief(GTK_BUTTON(m_pToggleButton), eStyle);
        gtk_button_set_relief(GTK_BUTTON(m_pToggleMenuButton), eStyle);

        // move the GtkMenuButton margins up to the new parent
        gtk_widget_set_margin_top(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_top(GTK_WIDGET(pMenuButton)));
        gtk_widget_set_margin_bottom(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_bottom(GTK_WIDGET(pMenuButton)));
        gtk_widget_set_margin_left(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_left(GTK_WIDGET(pMenuButton)));
        gtk_widget_set_margin_right(GTK_WIDGET(m_pContainer),
            gtk_widget_get_margin_right(GTK_WIDGET(pMenuButton)));

        gtk_menu_detach(m_pMenu);
        gtk_menu_attach_to_widget(m_pMenu, GTK_WIDGET(m_pToggleButton), nullptr);

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
        clear_items();
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

    virtual void set_item_help_id(const OString& rIdent, const OString& rHelpId) override
    {
        MenuHelper::set_item_help_id(rIdent, rHelpId);
    }

    virtual OString get_item_help_id(const OString& rIdent) const override
    {
        return MenuHelper::get_item_help_id(rIdent);
    }

    virtual void signal_activate(GtkMenuItem* pItem) override
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pItem));
        signal_selected(OString(pStr, pStr ? strlen(pStr) : 0));
    }

    virtual void set_popover(weld::Widget* /*pPopover*/) override
    {
        assert(false && "not implemented");
    }
};

class GtkInstanceMenu : public MenuHelper, public virtual weld::Menu
{
protected:
    std::vector<GtkMenuItem*> m_aExtraItems;
    OString m_sActivated;
    MenuHelper* m_pTopLevelMenuHelper;

private:
    virtual void signal_activate(GtkMenuItem* pItem) override
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pItem));
        m_sActivated = OString(pStr, pStr ? strlen(pStr) : 0);
        weld::Menu::signal_activate(m_sActivated);
    }

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

public:
    GtkInstanceMenu(GtkMenu* pMenu, bool bTakeOwnership)
        : MenuHelper(pMenu, bTakeOwnership)
        , m_pTopLevelMenuHelper(nullptr)
    {
        g_object_set_data(G_OBJECT(m_pMenu), "g-lo-GtkInstanceMenu", this);
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
    }

    virtual OString popup_at_rect(weld::Widget* pParent, const tools::Rectangle &rRect) override
    {
        m_sActivated.clear();

        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pParent);
        assert(pGtkWidget);

        GtkWidget* pWidget = pGtkWidget->getWidget();
        gtk_menu_attach_to_widget(m_pMenu, pWidget, nullptr);

        //run in a sub main loop because we need to keep vcl PopupMenu alive to use
        //it during DispatchCommand, returning now to the outer loop causes the
        //launching PopupMenu to be destroyed, instead run the subloop here
        //until the gtk menu is destroyed
        GMainLoop* pLoop = g_main_loop_new(nullptr, true);
        gulong nSignalId = g_signal_connect_swapped(G_OBJECT(m_pMenu), "deactivate", G_CALLBACK(g_main_loop_quit), pLoop);

#if GTK_CHECK_VERSION(3,22,0)
        if (gtk_check_version(3, 22, 0) == nullptr)
        {
            GdkRectangle aRect{static_cast<int>(rRect.Left()), static_cast<int>(rRect.Top()),
                               static_cast<int>(rRect.GetWidth()), static_cast<int>(rRect.GetHeight())};
            if (SwapForRTL(pWidget))
                aRect.x = gtk_widget_get_allocated_width(pWidget) - aRect.width - 1 - aRect.x;

            // Send a keyboard event through gtk_main_do_event to toggle any active tooltip offs
            // before trying to launch the menu
            // https://gitlab.gnome.org/GNOME/gtk/issues/1785
            GdkEvent *pKeyEvent = GtkSalFrame::makeFakeKeyPress(pWidget);
            gtk_main_do_event(pKeyEvent);

            GdkEvent *pTriggerEvent = gtk_get_current_event();
            if (!pTriggerEvent)
                pTriggerEvent = pKeyEvent;

            gtk_menu_popup_at_rect(m_pMenu, gtk_widget_get_window(pWidget), &aRect, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, pTriggerEvent);

            gdk_event_free(pKeyEvent);
        }
        else
#else
        (void) rRect;
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

        if (g_main_loop_is_running(pLoop))
        {
            gdk_threads_leave();
            g_main_loop_run(pLoop);
            gdk_threads_enter();
        }
        g_main_loop_unref(pLoop);
        g_signal_handler_disconnect(m_pMenu, nSignalId);
        gtk_menu_detach(m_pMenu);

        return m_sActivated;
    }

    virtual void set_sensitive(const OString& rIdent, bool bSensitive) override
    {
        set_item_sensitive(rIdent, bSensitive);
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
        clear_extras();
        clear_items();
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        TriState eCheckRadioFalse) override
    {
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

        GtkWidget *pItem;
        if (pImage)
        {
            GtkWidget *pBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
            GtkWidget *pLabel = gtk_label_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new() : gtk_menu_item_new();
            gtk_container_add(GTK_CONTAINER(pBox), pImage);
            gtk_container_add(GTK_CONTAINER(pBox), pLabel);
            gtk_container_add(GTK_CONTAINER(pItem), pBox);
            gtk_widget_show_all(pItem);
        }
        else
        {
            pItem = eCheckRadioFalse != TRISTATE_INDET ? gtk_check_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr())
                                                       : gtk_menu_item_new_with_mnemonic(MapToGtkAccelerator(rStr).getStr());
        }

        if (eCheckRadioFalse == TRISTATE_FALSE)
            gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(pItem), true);

        gtk_buildable_set_name(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr());
        gtk_menu_shell_append(GTK_MENU_SHELL(m_pMenu), pItem);
        gtk_widget_show(pItem);
        GtkMenuItem* pMenuItem = GTK_MENU_ITEM(pItem);
        m_aExtraItems.push_back(pMenuItem);
        add_to_map(pMenuItem);
        if (m_pTopLevelMenuHelper)
            m_pTopLevelMenuHelper->add_to_map(pMenuItem);
        if (pos != -1)
            gtk_menu_reorder_child(m_pMenu, pItem, pos);
    }

    virtual OString get_id(int pos) const override
    {
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pMenu));
        gpointer pMenuItem = g_list_nth_data(pChildren, pos);
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pMenuItem));
        OString id(pStr, pStr ? strlen(pStr) : 0);
        g_list_free(pChildren);
        return id;
    }

    virtual int n_children() const override
    {
        GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pMenu));
        int nLen = g_list_length(pChildren);
        g_list_free(pChildren);
        return nLen;
    }

    void remove(const OString& rIdent) override
    {
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
        MenuHelper::remove_item(rIdent);
    }

    virtual ~GtkInstanceMenu() override
    {
        clear_extras();
        g_object_steal_data(G_OBJECT(m_pMenu), "g-lo-GtkInstanceMenu");
    }
};

    vcl::ImageType GtkToVcl(GtkIconSize eSize)
    {
        vcl::ImageType eRet;
        switch (eSize)
        {
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
        }
        return eRet;
    }

    GtkIconSize VclToGtk(vcl::ImageType eSize)
    {
        GtkIconSize eRet;
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
        return eRet;
    }
}

void GtkInstanceMenuButton::set_menu(weld::Menu* pMenu)
{
    GtkInstanceMenu* pPopoverWidget = dynamic_cast<GtkInstanceMenu*>(pMenu);
    m_pPopover = nullptr;
    GtkWidget* pMenuWidget = GTK_WIDGET(pPopoverWidget ? pPopoverWidget->getMenu() : nullptr);
    gtk_menu_button_set_popup(m_pMenuButton, pMenuWidget);
}

namespace {

class GtkInstanceToolbar : public GtkInstanceWidget, public virtual weld::Toolbar
{
private:
    GtkToolbar* m_pToolbar;
    GtkCssProvider *m_pMenuButtonProvider;

    std::map<OString, GtkToolItem*> m_aMap;
    std::map<OString, std::unique_ptr<GtkInstanceMenuButton>> m_aMenuButtonMap;

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

    static void collect(GtkWidget* pItem, gpointer widget)
    {
        if (GTK_IS_TOOL_ITEM(pItem))
        {
            GtkToolItem* pToolItem = GTK_TOOL_ITEM(pItem);
            GtkInstanceToolbar* pThis = static_cast<GtkInstanceToolbar*>(widget);

            GtkMenuButton* pMenuButton = nullptr;
            if (GTK_IS_MENU_TOOL_BUTTON(pItem))
                find_menu_button(pItem, &pMenuButton);

            pThis->add_to_map(pToolItem, pMenuButton);
        }
    }

    void add_to_map(GtkToolItem* pToolItem, GtkMenuButton* pMenuButton)
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pToolItem));
        OString id(pStr, pStr ? strlen(pStr) : 0);
        m_aMap[id] = pToolItem;
        if (pMenuButton)
        {
            m_aMenuButtonMap[id] = std::make_unique<GtkInstanceMenuButton>(pMenuButton, GTK_WIDGET(pToolItem), m_pBuilder, false);
            // so that, e.g. with focus initially in writer main document then
            // after clicking the heading menu in the writer navigator focus is
            // left in the main document and not in the toolbar
            gtk_button_set_focus_on_click(GTK_BUTTON(pMenuButton), false);
            g_signal_connect(pMenuButton, "toggled", G_CALLBACK(signalItemToggled), this);

            if (pMenuButton)
            {
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
                    gtk_css_provider_load_from_data(m_pMenuButtonProvider, data, -1, nullptr);
                }

                gtk_style_context_add_provider(pButtonContext,
                                               GTK_STYLE_PROVIDER(m_pMenuButtonProvider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }

        }
        if (!GTK_IS_TOOL_BUTTON(pToolItem))
            return;
        g_signal_connect(pToolItem, "clicked", G_CALLBACK(signalItemClicked), this);
    }

    static void signalItemClicked(GtkToolButton* pItem, gpointer widget)
    {
        GtkInstanceToolbar* pThis = static_cast<GtkInstanceToolbar*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_item_clicked(pItem);
    }

    void signal_item_clicked(GtkToolButton* pItem)
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pItem));
        signal_clicked(OString(pStr, pStr ? strlen(pStr) : 0));
    }

    static void signalItemToggled(GtkToggleButton* pItem, gpointer widget)
    {
        GtkInstanceToolbar* pThis = static_cast<GtkInstanceToolbar*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_item_toggled(pItem);
    }

    void signal_item_toggled(GtkToggleButton* pItem)
    {
        for (auto& a : m_aMenuButtonMap)
        {
            if (a.second->getWidget() == GTK_WIDGET(pItem))
            {
                signal_toggle_menu(a.first);
                break;
            }
        }
    }

    static void set_item_image(GtkToolButton* pItem, const css::uno::Reference<css::graphic::XGraphic>& rIcon)
    {
        GtkWidget* pImage = nullptr;

        if (GdkPixbuf* pixbuf = getPixbuf(rIcon))
        {
            pImage = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            gtk_widget_show(pImage);
        }

        gtk_tool_button_set_icon_widget(pItem, pImage);
    }

    void set_item_image(GtkToolButton* pItem, const VirtualDevice* pDevice)
    {
        GtkWidget* pImage = nullptr;

        if (pDevice)
        {
            pImage = image_new_from_virtual_device(*pDevice);
            gtk_widget_show(pImage);
        }

        gtk_tool_button_set_icon_widget(pItem, pImage);
        gtk_widget_queue_draw(GTK_WIDGET(m_pToolbar));
    }

public:
    GtkInstanceToolbar(GtkToolbar* pToolbar, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pToolbar), pBuilder, bTakeOwnership)
        , m_pToolbar(pToolbar)
        , m_pMenuButtonProvider(nullptr)
    {
        gtk_container_foreach(GTK_CONTAINER(pToolbar), collect, this);
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

        GtkToolItem* pToolButton = m_aMap.find(rIdent)->second;

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

        enable_item_notify_events();
    }

    virtual bool get_item_active(const OString& rIdent) const override
    {
        GtkToolItem* pToolButton = m_aMap.find(rIdent)->second;

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

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        GtkToolItem* pItem = gtk_separator_tool_item_new();
        gtk_buildable_set_name(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr());
        gtk_toolbar_insert(m_pToolbar, pItem, pos);
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
        return gtk_toolbar_get_n_items(m_pToolbar);
    }

    virtual OString get_item_ident(int nIndex) const override
    {
        GtkToolItem* pItem = gtk_toolbar_get_nth_item(m_pToolbar, nIndex);
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pItem));
        return OString(pStr, pStr ? strlen(pStr) : 0);
    }

    virtual void set_item_ident(int nIndex, const OString& rIdent) override
    {
        OString sOldIdent(get_item_ident(nIndex));
        m_aMap.erase(m_aMap.find(sOldIdent));

        GtkToolItem* pItem = gtk_toolbar_get_nth_item(m_pToolbar, nIndex);
        gtk_buildable_set_name(GTK_BUILDABLE(pItem), rIdent.getStr());

        // to keep the ids unique, if the new id is already in use by an item,
        // change the id of that item to the now unused old ident of this item
        auto aFind = m_aMap.find(rIdent);
        if (aFind != m_aMap.end())
        {
            GtkToolItem* pDupIdItem = aFind->second;
            gtk_buildable_set_name(GTK_BUILDABLE(pDupIdItem), sOldIdent.getStr());
            m_aMap[sOldIdent] = pDupIdItem;
        }

        m_aMap[rIdent] = pItem;
    }

    virtual void set_item_label(int nIndex, const OUString& rLabel) override
    {
        GtkToolItem* pItem = gtk_toolbar_get_nth_item(m_pToolbar, nIndex);
        if (!GTK_IS_TOOL_BUTTON(pItem))
            return;
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(pItem), MapToGtkAccelerator(rLabel).getStr());
    }

    virtual void set_item_label(const OString& rIdent, const OUString& rLabel) override
    {
        GtkToolItem* pItem = m_aMap[rIdent];
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(pItem), MapToGtkAccelerator(rLabel).getStr());
    }

    OUString get_item_label(const OString& rIdent) const override
    {
        const gchar* pText = gtk_tool_button_get_label(GTK_TOOL_BUTTON(m_aMap.find(rIdent)->second));
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void set_item_icon_name(const OString& rIdent, const OUString& rIconName) override
    {
        GtkToolItem* pItem = m_aMap[rIdent];
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;

        GtkWidget* pImage = nullptr;

        if (GdkPixbuf* pixbuf = getPixbuf(rIconName))
        {
            pImage = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            gtk_widget_show(pImage);
        }

        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pItem), pImage);
    }

    virtual void set_item_image(const OString& rIdent, const css::uno::Reference<css::graphic::XGraphic>& rIcon) override
    {
        GtkToolItem* pItem = m_aMap[rIdent];
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;
        set_item_image(GTK_TOOL_BUTTON(pItem), rIcon);
    }

    virtual void set_item_image(const OString& rIdent, VirtualDevice* pDevice) override
    {
        GtkToolItem* pItem = m_aMap[rIdent];
        if (!pItem || !GTK_IS_TOOL_BUTTON(pItem))
            return;
        set_item_image(GTK_TOOL_BUTTON(pItem), pDevice);
    }

    virtual void set_item_image(int nIndex, const css::uno::Reference<css::graphic::XGraphic>& rIcon) override
    {
        GtkToolItem* pItem = gtk_toolbar_get_nth_item(m_pToolbar, nIndex);
        if (!GTK_IS_TOOL_BUTTON(pItem))
            return;
        set_item_image(GTK_TOOL_BUTTON(pItem), rIcon);
    }

    virtual void set_item_tooltip_text(int nIndex, const OUString& rTip) override
    {
        GtkToolItem* pItem = gtk_toolbar_get_nth_item(m_pToolbar, nIndex);
        gtk_widget_set_tooltip_text(GTK_WIDGET(pItem), OUStringToOString(rTip, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void set_item_tooltip_text(const OString& rIdent, const OUString& rTip) override
    {
        GtkToolItem* pItem = m_aMap[rIdent];
        gtk_widget_set_tooltip_text(GTK_WIDGET(pItem), OUStringToOString(rTip, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_item_tooltip_text(const OString& rIdent) const override
    {
        GtkToolItem* pItem = m_aMap.find(rIdent)->second;
        const gchar* pStr = gtk_widget_get_tooltip_text(GTK_WIDGET(pItem));
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual vcl::ImageType get_icon_size() const override
    {
        return GtkToVcl(gtk_toolbar_get_icon_size(m_pToolbar));
    }

    virtual void set_icon_size(vcl::ImageType eType) override
    {
        return gtk_toolbar_set_icon_size(m_pToolbar, VclToGtk(eType));
    }

    virtual sal_uInt16 get_modifier_state() const override
    {
        GdkKeymap* pKeymap = gdk_keymap_get_default();
        guint nState = gdk_keymap_get_modifier_state(pKeymap);
        return GtkSalFrame::GetKeyModCode(nState);
    }

    int get_drop_index(const Point& rPoint) const override
    {
        return gtk_toolbar_get_drop_index(m_pToolbar, rPoint.X(), rPoint.Y());
    }

    virtual bool has_focus() const override
    {
        if (gtk_widget_has_focus(m_pWidget))
            return true;
        GtkWidget* pTopLevel = gtk_widget_get_toplevel(m_pWidget);
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
        if (!gtk_container_get_focus_child(GTK_CONTAINER(m_pWidget)))
        {
            GtkToolItem* pItem = gtk_toolbar_get_nth_item(m_pToolbar, 0);
            gtk_container_set_focus_child(GTK_CONTAINER(m_pWidget), GTK_WIDGET(pItem));
        }
        gtk_widget_child_focus(gtk_container_get_focus_child(GTK_CONTAINER(m_pWidget)), GTK_DIR_TAB_FORWARD);
    }

    virtual ~GtkInstanceToolbar() override
    {
        for (auto& a : m_aMap)
            g_signal_handlers_disconnect_by_data(a.second, this);
    }
};

class GtkInstanceLinkButton : public GtkInstanceContainer, public virtual weld::LinkButton
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
        : GtkInstanceContainer(GTK_CONTAINER(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_nSignalId(g_signal_connect(pButton, "activate-link", G_CALLBACK(signalActivateLink), this))
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        ::set_label(GTK_BUTTON(m_pButton), rText);
    }

    virtual OUString get_label() const override
    {
        return ::get_label(GTK_BUTTON(m_pButton));
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
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pButton, m_nSignalId);
    }

    virtual ~GtkInstanceLinkButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nSignalId);
    }
};

class GtkInstanceRadioButton : public GtkInstanceToggleButton, public virtual weld::RadioButton
{
public:
    GtkInstanceRadioButton(GtkRadioButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), pBuilder, bTakeOwnership)
    {
    }
};

class GtkInstanceCheckButton : public GtkInstanceToggleButton, public virtual weld::CheckButton
{
public:
    GtkInstanceCheckButton(GtkCheckButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), pBuilder, bTakeOwnership)
    {
    }
};

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
        GdkPixbuf* pixbuf = load_icon_by_name(rIconName);
        if (!pixbuf)
            return;
        gtk_image_set_from_pixbuf(m_pImage, pixbuf);
        g_object_unref(pixbuf);
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        if (pDevice)
            gtk_image_set_from_surface(m_pImage, get_underlying_cairo_surface(*pDevice));
        else
            gtk_image_set_from_surface(m_pImage, nullptr);
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        GdkPixbuf* pixbuf = getPixbuf(rImage);
        gtk_image_set_from_pixbuf(m_pImage, pixbuf);
        if (pixbuf)
            g_object_unref(pixbuf);
    }
};

class GtkInstanceCalendar : public GtkInstanceWidget, public virtual weld::Calendar
{
private:
    GtkCalendar* m_pCalendar;
    gulong m_nDaySelectedSignalId;
    gulong m_nDaySelectedDoubleClickSignalId;
    gulong m_nKeyPressEventSignalId;
    gulong m_nButtonPressEventSignalId;

    static void signalDaySelected(GtkCalendar*, gpointer widget)
    {
        GtkInstanceCalendar* pThis = static_cast<GtkInstanceCalendar*>(widget);
        pThis->signal_selected();
    }

    static void signalDaySelectedDoubleClick(GtkCalendar*, gpointer widget)
    {
        GtkInstanceCalendar* pThis = static_cast<GtkInstanceCalendar*>(widget);
        pThis->signal_activated();
    }

    bool signal_key_press(GdkEventKey* pEvent)
    {
        if (pEvent->keyval == GDK_KEY_Return || pEvent->keyval == GDK_KEY_KP_Enter)
        {
            signal_activated();
            return true;
        }
        return false;
    }

    static gboolean signalKeyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceCalendar* pThis = static_cast<GtkInstanceCalendar*>(widget);
        return pThis->signal_key_press(pEvent);
    }

    static gboolean signalButton(GtkWidget*, GdkEventButton*, gpointer)
    {
        // don't let button press get to parent window, for the case of the
        // ImplCFieldFloatWin floating window belonging to CalendarField where
        // the click on the calendar continues to the parent GtkWindow and
        // closePopup is called by GtkSalFrame::signalButton because the click
        // window isn't that of the floating parent GtkWindow
        return true;
    }

public:
    GtkInstanceCalendar(GtkCalendar* pCalendar, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pCalendar), pBuilder, bTakeOwnership)
        , m_pCalendar(pCalendar)
        , m_nDaySelectedSignalId(g_signal_connect(pCalendar, "day-selected", G_CALLBACK(signalDaySelected), this))
        , m_nDaySelectedDoubleClickSignalId(g_signal_connect(pCalendar, "day-selected-double-click", G_CALLBACK(signalDaySelectedDoubleClick), this))
        , m_nKeyPressEventSignalId(g_signal_connect(pCalendar, "key-press-event", G_CALLBACK(signalKeyPress), this))
        , m_nButtonPressEventSignalId(g_signal_connect_after(pCalendar, "button-press-event", G_CALLBACK(signalButton), this))
    {
    }

    virtual void set_date(const Date& rDate) override
    {
        if (!rDate.IsValidAndGregorian())
            return;

        disable_notify_events();
        gtk_calendar_select_month(m_pCalendar, rDate.GetMonth() - 1, rDate.GetYear());
        gtk_calendar_select_day(m_pCalendar, rDate.GetDay());
        enable_notify_events();
    }

    virtual Date get_date() const override
    {
        guint year, month, day;
        gtk_calendar_get_date(m_pCalendar, &year, &month, &day);
        return Date(day, month + 1, year);
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
        g_signal_handler_disconnect(m_pCalendar, m_nButtonPressEventSignalId);
        g_signal_handler_disconnect(m_pCalendar, m_nKeyPressEventSignalId);
        g_signal_handler_disconnect(m_pCalendar, m_nDaySelectedDoubleClickSignalId);
        g_signal_handler_disconnect(m_pCalendar, m_nDaySelectedSignalId);
    }
};
}

namespace
{
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

class GtkInstanceEntry : public GtkInstanceWidget, public virtual weld::Entry
{
private:
    GtkEntry* m_pEntry;
    std::unique_ptr<vcl::Font> m_xFont;
    gulong m_nChangedSignalId;
    gulong m_nInsertTextSignalId;
    gulong m_nCursorPosSignalId;
    gulong m_nSelectionPosSignalId;
    gulong m_nActivateSignalId;

    static void signalChanged(GtkEntry*, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_changed();
    }

    static void signalInsertText(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength,
                                 gint* position, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_insert_text(pEntry, pNewText, nNewTextLength, position);
    }

    void signal_insert_text(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength, gint* position)
    {
        if (!m_aInsertTextHdl.IsSet())
            return;
        OUString sText(pNewText, nNewTextLength, RTL_TEXTENCODING_UTF8);
        const bool bContinue = m_aInsertTextHdl.Call(sText);
        if (bContinue && !sText.isEmpty())
        {
            OString sFinalText(OUStringToOString(sText, RTL_TEXTENCODING_UTF8));
            g_signal_handlers_block_by_func(pEntry, reinterpret_cast<gpointer>(signalInsertText), this);
            gtk_editable_insert_text(GTK_EDITABLE(pEntry), sFinalText.getStr(), sFinalText.getLength(), position);
            g_signal_handlers_unblock_by_func(pEntry, reinterpret_cast<gpointer>(signalInsertText), this);
        }
        g_signal_stop_emission_by_name(pEntry, "insert-text");
    }

    static void signalCursorPosition(GtkEntry*, GParamSpec*, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
        pThis->signal_cursor_position();
    }

    static void signalActivate(GtkEntry*, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
        pThis->signal_activate();
    }

    virtual void ensureMouseEventWidget() override
    {
        // The GtkEntry is sufficient to get mouse events without an intermediate GtkEventBox
        if (!m_pMouseEventBox)
            m_pMouseEventBox = m_pWidget;
    }

protected:

    virtual void signal_activate()
    {
        if (m_aActivateHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            if (m_aActivateHdl.Call(*this))
                g_signal_stop_emission_by_name(m_pEntry, "activate");
        }
    }

public:
    GtkInstanceEntry(GtkEntry* pEntry, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pEntry), pBuilder, bTakeOwnership)
        , m_pEntry(pEntry)
        , m_nChangedSignalId(g_signal_connect(pEntry, "changed", G_CALLBACK(signalChanged), this))
        , m_nInsertTextSignalId(g_signal_connect(pEntry, "insert-text", G_CALLBACK(signalInsertText), this))
        , m_nCursorPosSignalId(g_signal_connect(pEntry, "notify::cursor-position", G_CALLBACK(signalCursorPosition), this))
        , m_nSelectionPosSignalId(g_signal_connect(pEntry, "notify::selection-bound", G_CALLBACK(signalCursorPosition), this))
        , m_nActivateSignalId(g_signal_connect(pEntry, "activate", G_CALLBACK(signalActivate), this))
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
        gtk_entry_set_text(m_pEntry, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
        enable_notify_events();
    }

    virtual OUString get_text() const override
    {
        const gchar* pText = gtk_entry_get_text(m_pEntry);
        OUString sRet(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        return sRet;
    }

    virtual void set_width_chars(int nChars) override
    {
        disable_notify_events();
        gtk_entry_set_width_chars(m_pEntry, nChars);
        gtk_entry_set_max_width_chars(m_pEntry, nChars);
        enable_notify_events();
    }

    virtual int get_width_chars() const override
    {
        return gtk_entry_get_width_chars(m_pEntry);
    }

    virtual void set_max_length(int nChars) override
    {
        disable_notify_events();
        gtk_entry_set_max_length(m_pEntry, nChars);
        enable_notify_events();
    }

    virtual void select_region(int nStartPos, int nEndPos) override
    {
        disable_notify_events();
        gtk_editable_select_region(GTK_EDITABLE(m_pEntry), nStartPos, nEndPos);
        enable_notify_events();
    }

    bool get_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        return gtk_editable_get_selection_bounds(GTK_EDITABLE(m_pEntry), &rStartPos, &rEndPos);
    }

    virtual void replace_selection(const OUString& rText) override
    {
        disable_notify_events();
        gtk_editable_delete_selection(GTK_EDITABLE(m_pEntry));
        OString sText(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gint position = gtk_editable_get_position(GTK_EDITABLE(m_pEntry));
        gtk_editable_insert_text(GTK_EDITABLE(m_pEntry), sText.getStr(), sText.getLength(),
                                 &position);
        enable_notify_events();
    }

    virtual void set_position(int nCursorPos) override
    {
        disable_notify_events();
        gtk_editable_set_position(GTK_EDITABLE(m_pEntry), nCursorPos);
        enable_notify_events();
    }

    virtual int get_position() const override
    {
        return gtk_editable_get_position(GTK_EDITABLE(m_pEntry));
    }

    virtual void set_editable(bool bEditable) override
    {
        gtk_editable_set_editable(GTK_EDITABLE(m_pEntry), bEditable);
    }

    virtual bool get_editable() const override
    {
        return gtk_editable_get_editable(GTK_EDITABLE(m_pEntry));
    }

    virtual void set_overwrite_mode(bool bOn) override
    {
        gtk_entry_set_overwrite_mode(m_pEntry, bOn);
    }

    virtual bool get_overwrite_mode() const override
    {
        return gtk_entry_get_overwrite_mode(m_pEntry);
    }

    virtual void set_message_type(weld::EntryMessageType eType) override
    {
        ::set_entry_message_type(m_pEntry, eType);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pEntry, m_nActivateSignalId);
        g_signal_handler_block(m_pEntry, m_nSelectionPosSignalId);
        g_signal_handler_block(m_pEntry, m_nCursorPosSignalId);
        g_signal_handler_block(m_pEntry, m_nInsertTextSignalId);
        g_signal_handler_block(m_pEntry, m_nChangedSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pEntry, m_nChangedSignalId);
        g_signal_handler_unblock(m_pEntry, m_nInsertTextSignalId);
        g_signal_handler_unblock(m_pEntry, m_nCursorPosSignalId);
        g_signal_handler_unblock(m_pEntry, m_nSelectionPosSignalId);
        g_signal_handler_unblock(m_pEntry, m_nActivateSignalId);
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_xFont.reset(new vcl::Font(rFont));
        PangoAttrList* pAttrList = create_attr_list(rFont);
        gtk_entry_set_attributes(m_pEntry, pAttrList);
        pango_attr_list_unref(pAttrList);
    }

    virtual vcl::Font get_font() override
    {
        if (m_xFont)
            return *m_xFont;
        return GtkInstanceWidget::get_font();
    }

    void set_font_color(const Color& rColor) override
    {
        PangoAttrList* pOrigList = gtk_entry_get_attributes(m_pEntry);
        if (rColor == COL_AUTO && !pOrigList) // nothing to do
            return;

        PangoAttrType aFilterAttrs[] = {PANGO_ATTR_FOREGROUND, PANGO_ATTR_INVALID};

        PangoAttrList* pAttrs = pOrigList ? pango_attr_list_copy(pOrigList) : pango_attr_list_new();
        PangoAttrList* pRemovedAttrs = pOrigList ? pango_attr_list_filter(pAttrs, filter_pango_attrs, &aFilterAttrs) : nullptr;

        if (rColor != COL_AUTO)
            pango_attr_list_insert(pAttrs, pango_attr_foreground_new(rColor.GetRed()/255.0, rColor.GetGreen()/255.0, rColor.GetBlue()/255.0));

        gtk_entry_set_attributes(m_pEntry, pAttrs);
        pango_attr_list_unref(pAttrs);
        pango_attr_list_unref(pRemovedAttrs);
    }

    void fire_signal_changed()
    {
        signal_changed();
    }

    virtual void cut_clipboard() override
    {
        gtk_editable_cut_clipboard(GTK_EDITABLE(m_pEntry));
    }

    virtual void copy_clipboard() override
    {
        gtk_editable_copy_clipboard(GTK_EDITABLE(m_pEntry));
    }

    virtual void paste_clipboard() override
    {
        gtk_editable_paste_clipboard(GTK_EDITABLE(m_pEntry));
    }

    virtual void set_placeholder_text(const OUString& rText) override
    {
        gtk_entry_set_placeholder_text(m_pEntry, rText.toUtf8().getStr());
    }

    virtual void grab_focus() override
    {
        if (has_focus())
            return;
        gtk_entry_grab_focus_without_selecting(m_pEntry);
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
        gtk_entry_set_alignment(m_pEntry, xalign);
    }

    virtual ~GtkInstanceEntry() override
    {
        g_signal_handler_disconnect(m_pEntry, m_nActivateSignalId);
        g_signal_handler_disconnect(m_pEntry, m_nSelectionPosSignalId);
        g_signal_handler_disconnect(m_pEntry, m_nCursorPosSignalId);
        g_signal_handler_disconnect(m_pEntry, m_nInsertTextSignalId);
        g_signal_handler_disconnect(m_pEntry, m_nChangedSignalId);
    }
};

    struct Search
    {
        OString str;
        int index;
        int col;
        Search(const OUString& rText, int nCol)
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

    void insert_row(GtkListStore* pListStore, GtkTreeIter& iter, int pos, const OUString* pId, const OUString& rText, const OUString* pIconName, const VirtualDevice* pDevice)
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
    gint nVerticalSeparator;
    gtk_widget_style_get(GTK_WIDGET(pTreeView), "vertical-separator", &nVerticalSeparator, nullptr);
    return nVerticalSeparator;
}

int get_height_rows(GtkTreeView* pTreeView, GList* pColumns, int nRows)
{
    gint nMaxRowHeight = get_height_row(pTreeView, pColumns);
    gint nVerticalSeparator = get_height_row_separator(pTreeView);
    return (nMaxRowHeight * nRows) + (nVerticalSeparator * (nRows + 1));
}

int get_height_rows(int nRowHeight, int nSeparatorHeight, int nRows)
{
    return (nRowHeight * nRows) + (nSeparatorHeight * (nRows + 1));
}

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

void tree_store_remove(GtkTreeModel* pTreeModel, GtkTreeIter *pIter)
{
    gtk_tree_store_remove(GTK_TREE_STORE(pTreeModel), pIter);
}

void list_store_remove(GtkTreeModel* pTreeModel, GtkTreeIter *pIter)
{
    gtk_list_store_remove(GTK_LIST_STORE(pTreeModel), pIter);
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

class GtkInstanceTreeView : public GtkInstanceContainer, public virtual weld::TreeView
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

    typedef void(*removeFnc)(GtkTreeModel*, GtkTreeIter*);
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
    gulong m_nPopupMenuSignalId;
    gulong m_nKeyPressSignalId;
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

    void set(const GtkTreeIter& iter, int col, const OUString& rText)
    {
        OString aStr(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), col, aStr.getStr(), -1);
    }

    void set(int pos, int col, const OUString& rText)
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

    static gboolean signalQueryTooltip(GtkWidget* /*pGtkWidget*/, gint x, gint y,
                                         gboolean keyboard_tip, GtkTooltip *tooltip,
                                         gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        GtkTreeIter iter;
        GtkTreeView *pTreeView = pThis->m_pTreeView;
        GtkTreeModel *pModel = gtk_tree_view_get_model(pTreeView);
        GtkTreePath *pPath = nullptr;
        if (!gtk_tree_view_get_tooltip_context(pTreeView, &x, &y, keyboard_tip, &pModel, &pPath, &iter))
            return false;
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
            GdkRGBA aColor{rColor.GetRed()/255.0, rColor.GetGreen()/255.0, rColor.GetBlue()/255.0, 0};
            m_Setter(m_pTreeModel, const_cast<GtkTreeIter*>(&iter), m_nIdCol + 1, &aColor, -1);
        }
    }

    int get_expander_size() const
    {
        gint nExpanderSize;
        gint nHorizontalSeparator;

        gtk_widget_style_get(GTK_WIDGET(m_pTreeView),
                             "expander-size", &nExpanderSize,
                             "horizontal-separator", &nHorizontalSeparator,
                             nullptr);

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
        : GtkInstanceContainer(GTK_CONTAINER(pTreeView), pBuilder, bTakeOwnership)
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
        , m_nPopupMenuSignalId(g_signal_connect(pTreeView, "popup-menu", G_CALLBACK(signalPopupMenu), this))
        , m_nKeyPressSignalId(g_signal_connect(pTreeView, "key-press-event", G_CALLBACK(signalKeyPress), this))
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
            pRenderer = custom_cell_renderer_surface_new();
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
                                      const std::vector<int>* pFixedWidths) override
    {
        freeze();
        clear();
        GtkInstanceTreeIter aGtkIter(nullptr);

        if (pFixedWidths)
            set_column_fixed_widths(*pFixedWidths);

        while (nSourceCount)
        {
            // tdf#125241 inserting backwards is massively faster
            m_Prepend(m_pTreeModel, &aGtkIter.iter, nullptr);
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
        auto ret = gtk_tree_model_iter_parent(m_pTreeModel, &tmp, &rGtkIter.iter);
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
        weld::TreeIter& rNonConstIter = const_cast<weld::TreeIter&>(rIter);
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rNonConstIter);
        GtkTreeIter restore(rGtkIter.iter);
        bool ret = iter_children(rNonConstIter);
        rGtkIter.iter = restore;
        return ret;
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
        GtkInstanceContainer::freeze();
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
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
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
        GtkInstanceContainer::thaw();
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

    virtual void drag_source_set(const std::vector<GtkTargetEntry>& rGtkTargets, GdkDragAction eDragAction) override
    {
        if (rGtkTargets.empty() && !eDragAction)
            gtk_tree_view_unset_rows_drag_source(m_pTreeView);
        else
            gtk_tree_view_enable_model_drag_source(m_pTreeView, GDK_BUTTON1_MASK, rGtkTargets.data(), rGtkTargets.size(), eDragAction);
    }

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

        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();

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
            gtk_drag_unhighlight(GTK_WIDGET(m_pTreeView));

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
            gtk_drag_unhighlight(pWidget);
            gtk_drag_highlight(pParent);
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
            gtk_drag_unhighlight(pParent);
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
        g_signal_handler_disconnect(m_pTreeView, m_nKeyPressSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nPopupMenuSignalId);
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
                if (!CUSTOM_IS_CELL_RENDERER_SURFACE(pCellRenderer))
                    continue;
                g_object_set_property(G_OBJECT(pCellRenderer), "instance", &value);
            }
            g_list_free(pRenderers);
        }
        g_list_free(m_pColumns);
    }
};

void ensure_device(CustomCellRendererSurface *cellsurface, weld::Widget* pWidget)
{
    if (!cellsurface->device)
    {
        cellsurface->device = VclPtr<VirtualDevice>::Create();
        cellsurface->device->SetBackground(COL_TRANSPARENT);
        // expand the point size of the desired font to the equivalent pixel size
        if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
            pDefaultDevice->SetPointFont(*cellsurface->device, pWidget->get_font());
    }
}

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

class GtkInstanceIconView : public GtkInstanceContainer, public virtual weld::IconView
{
private:
    GtkIconView* m_pIconView;
    GtkTreeStore* m_pTreeStore;
    gint m_nTextCol;
    gint m_nImageCol;
    gint m_nIdCol;
    gulong m_nSelectionChangedSignalId;
    gulong m_nItemActivatedSignalId;
    ImplSVEvent* m_pSelectionChangeEvent;

    DECL_LINK(async_signal_selection_changed, void*, void);

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
        : GtkInstanceContainer(GTK_CONTAINER(pIconView), pBuilder, bTakeOwnership)
        , m_pIconView(pIconView)
        , m_pTreeStore(GTK_TREE_STORE(gtk_icon_view_get_model(m_pIconView)))
        , m_nTextCol(gtk_icon_view_get_text_column(m_pIconView))
        , m_nImageCol(gtk_icon_view_get_pixbuf_column(m_pIconView))
        , m_nSelectionChangedSignalId(g_signal_connect(pIconView, "selection-changed",
                                      G_CALLBACK(signalSelectionChanged), this))
        , m_nItemActivatedSignalId(g_signal_connect(pIconView, "item-activated", G_CALLBACK(signalItemActivated), this))
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
        GtkInstanceContainer::freeze();
        g_object_ref(m_pTreeStore);
        gtk_icon_view_set_model(m_pIconView, nullptr);
        g_object_freeze_notify(G_OBJECT(m_pTreeStore));
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
        g_object_thaw_notify(G_OBJECT(m_pTreeStore));
        gtk_icon_view_set_model(m_pIconView, GTK_TREE_MODEL(m_pTreeStore));
        g_object_unref(m_pTreeStore);
        GtkInstanceContainer::thaw();
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

        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();

        g_signal_handler_unblock(m_pIconView, m_nItemActivatedSignalId);
        g_signal_handler_unblock(m_pIconView, m_nSelectionChangedSignalId);
    }

    virtual ~GtkInstanceIconView() override
    {
        if (m_pSelectionChangeEvent)
            Application::RemoveUserEvent(m_pSelectionChangeEvent);

        g_signal_handler_disconnect(m_pIconView, m_nItemActivatedSignalId);
        g_signal_handler_disconnect(m_pIconView, m_nSelectionChangedSignalId);
    }
};

}

IMPL_LINK_NOARG(GtkInstanceIconView, async_signal_selection_changed, void*, void)
{
    m_pSelectionChangeEvent = nullptr;
    signal_selection_changed();
}

namespace {

class GtkInstanceSpinButton : public GtkInstanceEntry, public virtual weld::SpinButton
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
        GtkInstanceEntry::signal_activate();
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
        : GtkInstanceEntry(GTK_ENTRY(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_nValueChangedSignalId(g_signal_connect(pButton, "value-changed", G_CALLBACK(signalValueChanged), this))
        , m_nOutputSignalId(g_signal_connect(pButton, "output", G_CALLBACK(signalOutput), this))
        , m_nInputSignalId(g_signal_connect(pButton, "input", G_CALLBACK(signalInput), this))
        , m_bFormatting(false)
        , m_bBlockOutput(false)
        , m_bBlank(false)
    {
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
            gtk_entry_set_text(GTK_ENTRY(m_pButton), OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());

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
                gtk_entry_set_text(GTK_ENTRY(m_pButton), OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
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

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pButton, m_nValueChangedSignalId);
        GtkInstanceEntry::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceEntry::enable_notify_events();
        g_signal_handler_unblock(m_pButton, m_nValueChangedSignalId);
    }

    virtual ~GtkInstanceSpinButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nInputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nOutputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nValueChangedSignalId);
    }
};

class GtkInstanceFormattedSpinButton : public GtkInstanceEntry, public virtual weld::FormattedSpinButton
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
        : GtkInstanceEntry(GTK_ENTRY(pButton), pBuilder, bTakeOwnership)
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
        GtkInstanceEntry::set_text(rText);
        Formatter& rFormatter = GetFormatter();
        m_bEmptyField = rFormatter.IsEmptyFieldEnabled() && rText.isEmpty();
        if (m_bEmptyField)
            m_dValueWhenEmpty = gtk_spin_button_get_value(m_pButton);
    }

    virtual void connect_changed(const Link<weld::Entry&, void>& rLink) override
    {
        if (!m_pFormatter) // once a formatter is set, it takes over "changed"
        {
            GtkInstanceEntry::connect_changed(rLink);
            return;
        }
        m_pFormatter->connect_changed(rLink);
    }

    virtual void connect_focus_out(const Link<weld::Widget&, void>& rLink) override
    {
        if (!m_pFormatter) // once a formatter is set, it takes over "focus-out"
        {
            GtkInstanceEntry::connect_focus_out(rLink);
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

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pButton, m_nValueChangedSignalId);
        GtkInstanceEntry::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceEntry::enable_notify_events();
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

class GtkInstanceTextView : public GtkInstanceContainer, public virtual weld::TextView
{
private:
    GtkTextView* m_pTextView;
    GtkTextBuffer* m_pTextBuffer;
    GtkAdjustment* m_pVAdjustment;
    GtkCssProvider* m_pFgCssProvider;
    int m_nMaxTextLength;
    gulong m_nChangedSignalId; // we don't disable/enable this one, it's to implement max-length
    gulong m_nInsertTextSignalId;
    gulong m_nCursorPosSignalId;
    gulong m_nHasSelectionSignalId; // we don't disable/enable this one, it's to implement
                                    // auto-scroll to cursor on losing selection
    gulong m_nVAdjustChangedSignalId;
    gulong m_nButtonPressEvent; // we don't disable/enable this one, it's to block mouse
                                // click down from getting to (potential) toplevel
                                // GtkSalFrame parent, which grabs focus away

    static gboolean signalButtonPressEvent(GtkWidget*, GdkEventButton*, gpointer)
    {
        // e.g. on clicking on the help TextView in OTableDesignHelpBar the currently displayed text shouldn't disappear
        return true;
    }

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
        : GtkInstanceContainer(GTK_CONTAINER(pTextView), pBuilder, bTakeOwnership)
        , m_pTextView(pTextView)
        , m_pTextBuffer(gtk_text_view_get_buffer(pTextView))
        , m_pVAdjustment(gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(pTextView)))
        , m_pFgCssProvider(nullptr)
        , m_nMaxTextLength(0)
        , m_nChangedSignalId(g_signal_connect(m_pTextBuffer, "changed", G_CALLBACK(signalChanged), this))
        , m_nInsertTextSignalId(g_signal_connect_after(m_pTextBuffer, "insert-text", G_CALLBACK(signalInserText), this))
        , m_nCursorPosSignalId(g_signal_connect(m_pTextBuffer, "notify::cursor-position", G_CALLBACK(signalCursorPosition), this))
        , m_nHasSelectionSignalId(g_signal_connect(m_pTextBuffer, "notify::has-selection", G_CALLBACK(signalHasSelection), this))
        , m_nVAdjustChangedSignalId(g_signal_connect(m_pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustValueChanged), this))
        , m_nButtonPressEvent(g_signal_connect_after(m_pTextView, "button-press-event", G_CALLBACK(signalButtonPressEvent), this))
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
        gtk_css_provider_load_from_data(m_pFgCssProvider, aResult.getStr(), aResult.getLength(), nullptr);
        gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(m_pFgCssProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_block(m_pTextBuffer, m_nCursorPosSignalId);
        g_signal_handler_block(m_pTextBuffer, m_nChangedSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
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
        GtkClipboard *pClipboard = gtk_widget_get_clipboard(GTK_WIDGET(m_pTextView),
                                                            GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_cut_clipboard(m_pTextBuffer, pClipboard, get_editable());
    }

    virtual void copy_clipboard() override
    {
        GtkClipboard *pClipboard = gtk_widget_get_clipboard(GTK_WIDGET(m_pTextView),
                                                            GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_copy_clipboard(m_pTextBuffer, pClipboard);
    }

    virtual void paste_clipboard() override
    {
        GtkClipboard *pClipboard = gtk_widget_get_clipboard(GTK_WIDGET(m_pTextView),
                                                            GDK_SELECTION_CLIPBOARD);
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
        g_signal_handler_disconnect(m_pTextView, m_nButtonPressEvent);
        g_signal_handler_disconnect(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nInsertTextSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nChangedSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nCursorPosSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nHasSelectionSignalId);
    }
};

// IMHandler
class IMHandler;

AtkObject* (*default_drawing_area_get_accessible)(GtkWidget *widget);

class GtkInstanceDrawingArea : public GtkInstanceWidget, public virtual weld::DrawingArea
{
private:
    GtkDrawingArea* m_pDrawingArea;
    a11yref m_xAccessible;
    AtkObject *m_pAccessible;
    ScopedVclPtrInstance<VirtualDevice> m_xDevice;
    std::unique_ptr<IMHandler> m_xIMHandler;
    cairo_surface_t* m_pSurface;
    gulong m_nDrawSignalId;
    gulong m_nQueryTooltip;
    gulong m_nPopupMenu;
    gulong m_nScrollEvent;

    static gboolean signalDraw(GtkWidget*, cairo_t* cr, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_draw(cr);
        return false;
    }
    void signal_draw(cairo_t* cr)
    {
        GdkRectangle rect;
        if (!m_pSurface || !gdk_cairo_get_clip_rectangle(cr, &rect))
            return;
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
    DECL_LINK(SettingsChangedHdl, VclWindowEvent&, void);
public:
    GtkInstanceDrawingArea(GtkDrawingArea* pDrawingArea, GtkInstanceBuilder* pBuilder, const a11yref& rA11y, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pDrawingArea), pBuilder, bTakeOwnership)
        , m_pDrawingArea(pDrawingArea)
        , m_xAccessible(rA11y)
        , m_pAccessible(nullptr)
        , m_xDevice(DeviceFormat::DEFAULT)
        , m_pSurface(nullptr)
        , m_nDrawSignalId(g_signal_connect(m_pDrawingArea, "draw", G_CALLBACK(signalDraw), this))
        , m_nQueryTooltip(g_signal_connect(m_pDrawingArea, "query-tooltip", G_CALLBACK(signalQueryTooltip), this))
        , m_nPopupMenu(g_signal_connect(m_pDrawingArea, "popup-menu", G_CALLBACK(signalPopupMenu), this))
        , m_nScrollEvent(g_signal_connect(m_pDrawingArea, "scroll-event", G_CALLBACK(signalScroll), this))
    {
        gtk_widget_set_has_tooltip(m_pWidget, true);
        g_object_set_data(G_OBJECT(m_pDrawingArea), "g-lo-GtkInstanceDrawingArea", this);
        m_xDevice->EnableRTL(get_direction());

        ImplGetDefaultWindow()->AddEventListener(LINK(this, GtkInstanceDrawingArea, SettingsChangedHdl));
    }

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

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink) override
    {
        if (!(gtk_widget_get_events(m_pWidget) & GDK_BUTTON_PRESS_MASK))
            gtk_widget_add_events(m_pWidget, GDK_BUTTON_PRESS_MASK);
        GtkInstanceWidget::connect_mouse_press(rLink);
    }

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink) override
    {
        if (!(gtk_widget_get_events(m_pWidget) & GDK_BUTTON_RELEASE_MASK))
            gtk_widget_add_events(m_pWidget, GDK_BUTTON_RELEASE_MASK);
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
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(m_pDrawingArea)), pCursor);
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
        tools::Rectangle aRect(Point(x, y), Size(width, height));
        aRect = m_xDevice->LogicToPixel(aRect);
        gtk_widget_queue_draw_area(GTK_WIDGET(m_pDrawingArea), aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight());
    }

    virtual void queue_resize() override
    {
        gtk_widget_queue_resize(GTK_WIDGET(m_pDrawingArea));
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
        assert(false && "get_accessible_parent should only be called on a vcl impl");
        return uno::Reference<css::accessibility::XAccessibleRelationSet>();
    }

    virtual Point get_accessible_location() override
    {
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
        gint x(0), y(0);
        if (pAtkObject && ATK_IS_COMPONENT(pAtkObject))
            atk_component_get_extents(ATK_COMPONENT(pAtkObject), &x, &y, nullptr, nullptr, ATK_XY_WINDOW);
        return Point(x, y);
    }

    virtual void set_accessible_name(const OUString& rName) override
    {
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        atk_object_set_name(pAtkObject, OUStringToOString(rName, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_accessible_name() const override
    {
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_name(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual OUString get_accessible_description() const override
    {
        AtkObject* pAtkObject = default_drawing_area_get_accessible(m_pWidget);
        const char* pStr = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
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
        if (m_pAccessible)
            g_object_unref(m_pAccessible);
        css::uno::Reference<css::lang::XComponent> xComp(m_xAccessible, css::uno::UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
        g_signal_handler_disconnect(m_pDrawingArea, m_nScrollEvent);
        g_signal_handler_disconnect(m_pDrawingArea, m_nPopupMenu);
        g_signal_handler_disconnect(m_pDrawingArea, m_nQueryTooltip);
        g_signal_handler_disconnect(m_pDrawingArea, m_nDrawSignalId);
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
    GtkIMContext* m_pIMContext;
    OUString m_sPreeditText;
    gulong m_nFocusInSignalId;
    gulong m_nFocusOutSignalId;
    bool m_bExtTextInput;

public:
    IMHandler(GtkInstanceDrawingArea* pArea)
        : m_pArea(pArea)
        , m_pIMContext(gtk_im_multicontext_new())
        , m_nFocusInSignalId(g_signal_connect(m_pArea->getWidget(), "focus-in-event", G_CALLBACK(signalFocusIn), this))
        , m_nFocusOutSignalId(g_signal_connect(m_pArea->getWidget(), "focus-out-event", G_CALLBACK(signalFocusOut), this))
        , m_bExtTextInput(false)
    {
        g_signal_connect(m_pIMContext, "preedit-start", G_CALLBACK(signalIMPreeditStart), this);
        g_signal_connect(m_pIMContext, "preedit-end", G_CALLBACK(signalIMPreeditEnd), this);
        g_signal_connect(m_pIMContext, "commit", G_CALLBACK(signalIMCommit), this);
        g_signal_connect(m_pIMContext, "preedit-changed", G_CALLBACK(signalIMPreeditChanged), this);
        g_signal_connect(m_pIMContext, "retrieve-surrounding", G_CALLBACK(signalIMRetrieveSurrounding), this);
        g_signal_connect(m_pIMContext, "delete-surrounding", G_CALLBACK(signalIMDeleteSurrounding), this);

        GtkWidget* pWidget = m_pArea->getWidget();
        if (!gtk_widget_get_realized(pWidget))
            gtk_widget_realize(pWidget);
        GdkWindow* pWin = gtk_widget_get_window(pWidget);
        gtk_im_context_set_client_window(m_pIMContext, pWin);
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

    static gboolean signalFocusIn(GtkWidget*, GdkEvent*, gpointer im_handler)
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);
        pThis->signalFocus(true);
        return false;
    }

    static gboolean signalFocusOut(GtkWidget*, GdkEvent*, gpointer im_handler)
    {
        IMHandler* pThis = static_cast<IMHandler*>(im_handler);
        pThis->signalFocus(false);
        return false;
    }

    ~IMHandler()
    {
        EndExtTextInput();

        g_signal_handler_disconnect(m_pArea->getWidget(), m_nFocusOutSignalId);
        g_signal_handler_disconnect(m_pArea->getWidget(), m_nFocusInSignalId);

        if (gtk_widget_has_focus(m_pArea->getWidget()))
            gtk_im_context_focus_out(m_pIMContext);

        // first give IC a chance to deinitialize
        gtk_im_context_set_client_window(m_pIMContext, nullptr);
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

namespace {

GtkBuilder* makeMenuToggleButtonBuilder()
{
    OUString aUri(AllSettings::GetUIRootDir() + "vcl/ui/menutogglebutton.ui");
    OUString aPath;
    osl::FileBase::getSystemPathFromFileURL(aUri, aPath);
    return gtk_builder_new_from_file(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr());
}

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
    virtual void signal_activate(GtkMenuItem*) override
    {
        gtk_toggle_button_set_active(m_pComboBox, false);
    }
};

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
    std::unique_ptr<CustomRenderMenuButtonHelper> m_xCustomMenuButtonHelper;
    std::unique_ptr<vcl::Font> m_xFont;
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

    void toggle_menu()
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

            do_ungrab(GTK_WIDGET(m_pMenuWindow));

            gtk_widget_hide(GTK_WIDGET(m_pMenuWindow));

            // so gdk_window_move_to_rect will work again the next time
            gtk_widget_unrealize(GTK_WIDGET(m_pMenuWindow));

            gtk_widget_set_size_request(GTK_WIDGET(m_pMenuWindow), -1, -1);

            if (!m_bActivateCalled)
                tree_view_set_cursor(m_nPrePopupCursorPos);

            // undo show_menu tooltip blocking
            GtkWidget* pParent = gtk_widget_get_toplevel(m_pToggleButton);
            GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
            if (pFrame)
                pFrame->UnblockTooltip();
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

            show_menu(pComboBox, m_pMenuWindow);
        }
    }

    virtual void signal_popup_toggled() override
    {
        m_aQuickSelectionEngine.Reset();

        toggle_menu();

        bool bIsShown = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pToggleButton));
        if (m_bPopupActive != bIsShown)
        {
            m_bPopupActive = bIsShown;
            ComboBox::signal_popup_toggled();
            if (!m_bPopupActive)
            {
                //restore focus to the entry view when the popup is gone, which
                //is what the vcl case does, to ease the transition a little
                grab_focus();
            }
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

    void set(int pos, int col, const OUString& rText)
    {
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos))
        {
            OString aStr(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
            gtk_list_store_set(GTK_LIST_STORE(m_pTreeModel), &iter, col, aStr.getStr(), -1);
        }
    }

    int find(const OUString& rStr, int col, bool bSearchMRUArea) const
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
        GtkWidget *pToplevel = gtk_widget_get_toplevel(pComboBox);
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

    static gboolean signalEntryKeyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
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
                // tdf#131076 don't let bare return toggle menu popup active, but do allow deactive
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
        else
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
        gdk_window_get_root_origin(gtk_widget_get_window(pWidget), &xoffset, &yoffset);

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

    int find_text_including_mru(const OUString& rStr, bool bSearchMRU) const
    {
        return find(rStr, m_nTextCol, bSearchMRU);
    }

    int find_id_including_mru(const OUString& rId, bool bSearchMRU) const
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

    void set_id_including_mru(int pos, const OUString& rId)
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
        insert_row(GTK_LIST_STORE(m_pTreeModel), iter, pos, &rId, "", nullptr, nullptr);
        GtkTreePath* pPath = gtk_tree_path_new_from_indices(pos, -1);
        m_aSeparatorRows.emplace_back(gtk_tree_row_reference_new(m_pTreeModel, pPath));
        gtk_tree_path_free(pPath);
        enable_notify_events();
    }

    void insert_including_mru(int pos, const OUString& rText, const OUString* pId, const OUString* pIconName, const VirtualDevice* pImageSurface)
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

    virtual void set_entry_font(const vcl::Font& rFont) override
    {
        m_xFont.reset(new vcl::Font(rFont));
        PangoAttrList* pAttrList = create_attr_list(rFont);
        assert(m_pEntry);
        gtk_entry_set_attributes(GTK_ENTRY(m_pEntry), pAttrList);
        pango_attr_list_unref(pAttrList);
    }

    virtual vcl::Font get_entry_font() override
    {
        if (m_xFont)
            return *m_xFont;
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
        GtkInstanceContainer::freeze();
        g_object_ref(m_pTreeModel);
        gtk_tree_view_set_model(m_pTreeView, nullptr);
        g_object_freeze_notify(G_OBJECT(m_pTreeModel));
        if (m_xSorter)
        {
            GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
            gtk_tree_sortable_set_sort_column_id(pSortable, GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
        }
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
        if (m_xSorter)
        {
            GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
            gtk_tree_sortable_set_sort_column_id(pSortable, m_nTextCol, GTK_SORT_ASCENDING);
        }
        g_object_thaw_notify(G_OBJECT(m_pTreeModel));
        gtk_tree_view_set_model(m_pTreeView, m_pTreeModel);
        g_object_unref(m_pTreeModel);

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
            GtkCellRenderer *pRenderer = custom_cell_renderer_surface_new();
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
            g_object_ref(m_pComboBox);

            GtkContainer* pContainer = getContainer();

            gtk_container_remove(pContainer, GTK_WIDGET(m_pComboBox));

            replaceWidget(GTK_WIDGET(pContainer), GTK_WIDGET(m_pComboBox));

            g_object_unref(m_pComboBox);
        }

        g_object_unref(m_pComboBuilder);
    }
};

}

bool custom_cell_renderer_surface_get_preferred_size(GtkCellRenderer *cell,
                                                     GtkOrientation orientation,
                                                     gint *minimum_size,
                                                     gint *natural_size)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_object_get_property(G_OBJECT(cell), "id", &value);

    const char* pStr = g_value_get_string(&value);

    OUString sId(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);

    value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_POINTER);
    g_object_get_property(G_OBJECT(cell), "instance", &value);

    CustomCellRendererSurface *cellsurface = CUSTOM_CELL_RENDERER_SURFACE(cell);

    GtkInstanceWidget* pWidget = static_cast<GtkInstanceWidget*>(g_value_get_pointer(&value));

    Size aSize;

    if (pWidget)
    {
        ensure_device(cellsurface, pWidget);
        if (GtkInstanceTreeView* pTreeView = dynamic_cast<GtkInstanceTreeView*>(pWidget))
            aSize = pTreeView->call_signal_custom_get_size(*cellsurface->device, sId);
        else if (GtkInstanceComboBox* pComboBox = dynamic_cast<GtkInstanceComboBox*>(pWidget))
            aSize = pComboBox->call_signal_custom_get_size(*cellsurface->device);
    }

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        if (minimum_size)
            *minimum_size = aSize.Width();

        if (natural_size)
            *natural_size = aSize.Width();
    }
    else
    {
        if (minimum_size)
            *minimum_size = aSize.Height();

        if (natural_size)
            *natural_size = aSize.Height();
    }

    return true;
}

void custom_cell_renderer_surface_render(GtkCellRenderer* cell,
                                         cairo_t* cr,
                                         GtkWidget* /*widget*/,
                                         const GdkRectangle* /*background_area*/,
                                         const GdkRectangle* cell_area,
                                         GtkCellRendererState flags)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_object_get_property(G_OBJECT(cell), "id", &value);

    const char* pStr = g_value_get_string(&value);
    OUString sId(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);

    value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_POINTER);
    g_object_get_property(G_OBJECT(cell), "instance", &value);

    CustomCellRendererSurface *cellsurface = CUSTOM_CELL_RENDERER_SURFACE(cell);

    GtkInstanceWidget* pWidget = static_cast<GtkInstanceWidget*>(g_value_get_pointer(&value));

    if (!pWidget)
        return;

    ensure_device(cellsurface, pWidget);

    Size aSize(cell_area->width, cell_area->height);
    // false to not bother setting the bg on resize as we'll do that
    // ourself via cairo
    cellsurface->device->SetOutputSizePixel(aSize, false);

    cairo_surface_t* pSurface = get_underlying_cairo_surface(*cellsurface->device);

    // fill surface as transparent so it can be blended with the potentially
    // selected background
    cairo_t* tempcr = cairo_create(pSurface);
    cairo_set_source_rgba(tempcr, 0, 0, 0, 0);
    cairo_set_operator(tempcr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(tempcr);
    cairo_destroy(tempcr);
    cairo_surface_flush(pSurface);

    if (GtkInstanceTreeView* pTreeView = dynamic_cast<GtkInstanceTreeView*>(pWidget))
        pTreeView->call_signal_custom_render(*cellsurface->device, tools::Rectangle(Point(0, 0), aSize), flags & GTK_CELL_RENDERER_SELECTED, sId);
    else if (GtkInstanceComboBox* pComboBox = dynamic_cast<GtkInstanceComboBox*>(pWidget))
        pComboBox->call_signal_custom_render(*cellsurface->device, tools::Rectangle(Point(0, 0), aSize), flags & GTK_CELL_RENDERER_SELECTED, sId);
    cairo_surface_mark_dirty(pSurface);

    cairo_set_source_surface(cr, pSurface, cell_area->x, cell_area->y);
    cairo_paint(cr);
}

namespace {

class GtkInstanceEntryTreeView : public GtkInstanceContainer, public virtual weld::EntryTreeView
{
private:
    GtkInstanceEntry* m_pEntry;
    GtkInstanceTreeView* m_pTreeView;
    gulong m_nKeyPressSignalId;
    gulong m_nEntryInsertTextSignalId;
    guint m_nAutoCompleteIdleId;
    bool m_bAutoCompleteCaseSensitive;
    bool m_bTreeChange;

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
    GtkInstanceEntryTreeView(GtkContainer* pContainer, GtkInstanceBuilder* pBuilder, bool bTakeOwnership,
                             std::unique_ptr<weld::Entry> xEntry, std::unique_ptr<weld::TreeView> xTreeView)
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
        m_nKeyPressSignalId = g_signal_connect(pWidget, "key-press-event", G_CALLBACK(signalKeyPress), this);
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
        g_signal_handler_block(pWidget, m_nKeyPressSignalId);
        m_pTreeView->disable_notify_events();
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkWidget* pWidget = m_pEntry->getWidget();
        g_signal_handler_unblock(pWidget, m_nKeyPressSignalId);
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
        g_signal_handler_disconnect(pWidget, m_nKeyPressSignalId);
        g_signal_handler_disconnect(pWidget, m_nEntryInsertTextSignalId);
    }
};

class GtkInstanceExpander : public GtkInstanceContainer, public virtual weld::Expander
{
private:
    GtkExpander* m_pExpander;
    gulong m_nSignalId;
    gulong m_nButtonPressEventSignalId;

    static void signalExpanded(GtkExpander* pExpander, GParamSpec*, gpointer widget)
    {
        GtkInstanceExpander* pThis = static_cast<GtkInstanceExpander*>(widget);
        SolarMutexGuard aGuard;

        if (gtk_expander_get_resize_toplevel(pExpander))
        {
            GtkWidget *pToplevel = gtk_widget_get_toplevel(GTK_WIDGET(pExpander));

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

        pThis->signal_expanded();
    }

    static gboolean signalButton(GtkWidget*, GdkEventButton*, gpointer)
    {
        // don't let button press get to parent window, for the case of the
        // an expander in a sidebar where otherwise single click to expand
        // doesn't work
        return true;
    }

public:
    GtkInstanceExpander(GtkExpander* pExpander, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pExpander), pBuilder, bTakeOwnership)
        , m_pExpander(pExpander)
        , m_nSignalId(g_signal_connect(m_pExpander, "notify::expanded", G_CALLBACK(signalExpanded), this))
        , m_nButtonPressEventSignalId(g_signal_connect_after(m_pExpander, "button-press-event", G_CALLBACK(signalButton), this))
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
        g_signal_handler_disconnect(m_pExpander, m_nButtonPressEventSignalId);
        g_signal_handler_disconnect(m_pExpander, m_nSignalId);
    }
};

    gboolean signalTooltipQuery(GtkWidget* pWidget, gint /*x*/, gint /*y*/,
                                         gboolean /*keyboard_mode*/, GtkTooltip *tooltip)
    {
        const ImplSVHelpData& aHelpData = ImplGetSVHelpData();
        if (aHelpData.mbBalloonHelp) // extended tips
        {
            // by default use accessible description
            AtkObject* pAtkObject = gtk_widget_get_accessible(pWidget);
            const char* pDesc = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
            if (pDesc && pDesc[0])
            {
                gtk_tooltip_set_text(tooltip, pDesc);
                return true;
            }

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
            const gchar* icon_name;
            gtk_image_get_icon_name(pImage, &icon_name, nullptr);
            if (icon_name)
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);
                if (aIconName != "pan-up-symbolic" && aIconName != "pan-down-symbolic")
                {
                    if (GdkPixbuf* pixbuf = load_icon_by_name_theme_lang(aIconName, m_aIconTheme, m_aUILang))
                    {
                        gtk_image_set_from_pixbuf(pImage, pixbuf);
                        g_object_unref(pixbuf);
                    }
                }
            }
        }
        else if (GTK_IS_TOOL_BUTTON(pWidget))
        {
            GtkToolButton* pToolButton = GTK_TOOL_BUTTON(pWidget);
            if (const gchar* icon_name = gtk_tool_button_get_icon_name(pToolButton))
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);
                if (aIconName != "pan-up-symbolic" && aIconName != "pan-down-symbolic")
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

        //set helpids
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pWidget));
        size_t nLen = pStr ? strlen(pStr) : 0;
        if (nLen)
        {
            OString sBuildableName(pStr, nLen);
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

        // expand placeholder and collect potentially missing mnemonics
        if (GTK_IS_BUTTON(pWidget))
        {
            GtkButton* pButton = GTK_BUTTON(pWidget);
            if (m_pStringReplace != nullptr)
            {
                OUString aLabel(get_label(pButton));
                if (!aLabel.isEmpty())
                    set_label(pButton, (*m_pStringReplace)(aLabel));
            }
            if (gtk_button_get_use_underline(pButton) && !gtk_button_get_use_stock(pButton))
                m_aMnemonicButtons.push_back(pButton);
        }
        else if (GTK_IS_LABEL(pWidget))
        {
            GtkLabel* pLabel = GTK_LABEL(pWidget);
            if (m_pStringReplace != nullptr)
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
            if (m_pStringReplace != nullptr)
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
        else if (GTK_IS_WINDOW(pWidget))
        {
            if (m_pStringReplace != nullptr) {
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

        GtkWidget* pTopLevel = gtk_widget_get_toplevel(m_pParentWidget);
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

        GtkWidget* pTopLevel = gtk_widget_get_toplevel(m_pParentWidget);
        assert(pTopLevel);
        GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(pTopLevel);
        assert(pFrame);
        // rehook handler and let vcl cycle its own way through this widget's
        // children
        pFrame->AllowCycleFocusOut();
    }

    static void signalUnmap(GtkWidget*, gpointer user_data)
    {
        GtkInstanceBuilder* pThis = static_cast<GtkInstanceBuilder*>(user_data);
        pThis->AllowCycleFocusOut();
    }

public:
    GtkInstanceBuilder(GtkWidget* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                       SystemChildWindow* pInterimGlue, bool bAllowCycleFocusOut)
        : weld::Builder()
        , m_pStringReplace(Translate::GetReadStringHook())
        , m_pParentWidget(pParent)
        , m_nNotifySignalId(0)
        , m_xInterimGlue(pInterimGlue)
        , m_bAllowCycleFocusOut(bAllowCycleFocusOut)
    {
        OUString sHelpRoot(rUIFile);
        ensure_intercept_drawing_area_accessibility();
        ensure_disable_ctrl_page_up_down_bindings();

        sal_Int32 nIdx = sHelpRoot.lastIndexOf('.');
        if (nIdx != -1)
            sHelpRoot = sHelpRoot.copy(0, nIdx);
        sHelpRoot += OUString('/');
        m_aUtf8HelpRoot = OUStringToOString(sHelpRoot, RTL_TEXTENCODING_UTF8);
        m_aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        m_aUILang = Application::GetSettings().GetUILanguageTag().getBcp47();

        OUString aUri(rUIRoot + rUIFile);
        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aUri, aPath);
        m_pBuilder = gtk_builder_new();
        m_nNotifySignalId = g_signal_connect_data(G_OBJECT(m_pBuilder), "notify", G_CALLBACK(signalNotify), this, nullptr, G_CONNECT_AFTER);
        auto rc = gtk_builder_add_from_file(m_pBuilder, OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr(), nullptr);
        assert(rc && "could not load UI file");
        (void) rc;

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
            aMnemonicGenerator.RegisterMnemonic(get_label(a));
        for (const auto a : m_aMnemonicLabels)
            aMnemonicGenerator.RegisterMnemonic(get_label(a));

        for (const auto a : m_aMnemonicButtons)
        {
            OUString aLabel(get_label(a));
            OUString aNewLabel = aMnemonicGenerator.CreateMnemonic(aLabel);
            if (aLabel == aNewLabel)
                continue;
            set_label(a, aNewLabel);
        }
        for (const auto a : m_aMnemonicLabels)
        {
            OUString aLabel(get_label(a));
            OUString aNewLabel = aMnemonicGenerator.CreateMnemonic(aLabel);
            if (aLabel == aNewLabel)
                continue;
            set_label(a, aNewLabel);
        }

        m_aMnemonicLabels.clear();
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
                GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pContainer));
                GList* pChild = g_list_first(pChildren);
                if (pChild)
                {
                    GtkWidget* pPageWidget = static_cast<GtkWidget*>(pChild->data);
                    sPageHelpId = ::get_help_id(pPageWidget);
                }
                g_list_free(pChildren);
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
        if (gtk_widget_get_toplevel(pWidget) == pWidget && !GTK_IS_POPOVER(pWidget) && !GTK_IS_WINDOW(pWidget))
            gtk_container_add(GTK_CONTAINER(m_pParentWidget), pWidget);
    }

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString &id) override
    {
        GtkMessageDialog* pMessageDialog = GTK_MESSAGE_DIALOG(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pMessageDialog)
            return nullptr;
        gtk_window_set_transient_for(GTK_WINDOW(pMessageDialog), GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceMessageDialog>(pMessageDialog, this, true);
    }

    virtual std::unique_ptr<weld::Assistant> weld_assistant(const OString &id) override
    {
        GtkAssistant* pAssistant = GTK_ASSISTANT(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pAssistant)
            return nullptr;
        if (m_pParentWidget)
            gtk_window_set_transient_for(GTK_WINDOW(pAssistant), GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceAssistant>(pAssistant, this, true);
    }

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString &id) override
    {
        GtkWindow* pDialog = GTK_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pDialog)
            return nullptr;
        if (m_pParentWidget)
            gtk_window_set_transient_for(pDialog, GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceDialog>(pDialog, this, true);
    }

    virtual std::unique_ptr<weld::Window> create_screenshot_window() override
    {
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
            gtk_window_set_transient_for(pDialog, GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceDialog>(pDialog, this, true);
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
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, id.getStr()));
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
        // gtk doesn't come with exactly the the concept
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
        GtkRadioButton* pRadioButton = GTK_RADIO_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
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
        GtkImage* pImage = GTK_IMAGE(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pImage)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pImage));
        return std::make_unique<GtkInstanceImage>(pImage, this, false);
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
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, containerid.getStr()));
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
        GtkMenu* pMenu = GTK_MENU(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pMenu)
            return nullptr;
        return std::make_unique<GtkInstanceMenu>(pMenu, true);
    }

    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OString &id) override
    {
        GtkToolbar* pToolbar = GTK_TOOLBAR(gtk_builder_get_object(m_pBuilder, id.getStr()));
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
                GList* pChildren = gtk_container_get_children(pContainer);
                GList* pChild = g_list_first(pChildren);
                if (pChild)
                {
                    GtkWidget* pContentWidget = static_cast<GtkWidget*>(pChild->data);
                    sHelpId = ::get_help_id(pContentWidget);
                }
                g_list_free(pChildren);
            }
        }
    }
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

weld::Builder* GtkInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    GtkInstanceWidget* pParentWidget = dynamic_cast<GtkInstanceWidget*>(pParent);
    if (pParent && !pParentWidget) //remove when complete
        return SalInstance::CreateBuilder(pParent, rUIRoot, rUIFile);
    GtkWidget* pBuilderParent = pParentWidget ? pParentWidget->getWidget() : nullptr;
    return new GtkInstanceBuilder(pBuilderParent, rUIRoot, rUIFile, nullptr, true);
}

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

weld::Builder* GtkInstance::CreateInterimBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
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
    gtk_widget_show_all(pWindow);

    // build the widget tree as a child of the GtkEventBox GtkGrid parent
    return new GtkInstanceBuilder(pWindow, rUIRoot, rUIFile, xEmbedWindow.get(), bAllowCycleFocusOut);
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
        m_xFrameWeld.reset(new GtkInstanceWindow(GTK_WINDOW(gtk_widget_get_toplevel(getWindow())), nullptr, false));
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
    gtk_container_add(GTK_CONTAINER(pParent), pGstWidget);
    g_object_unref(pGstWidget);
    gtk_widget_show_all(pParent);

    return pVideosink;
#else
    (void)pWindow;
    return nullptr;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
