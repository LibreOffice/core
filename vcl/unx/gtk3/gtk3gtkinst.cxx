/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../gtk/gtkinst.cxx"
#include "../gtk/a11y/atkwrapper.hxx"
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
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <tools/helpers.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/quickselectionengine.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/syswin.hxx>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>
#include <vcl/event.hxx>
#include <window.h>
#include <numeric>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace
{
    struct TypeEntry
    {
        const char*     pNativeType;        // string corresponding to nAtom for the case of nAtom being uninitialized
        const char*     pType;              // Mime encoding on our side
    };

    static const TypeEntry aConversionTab[] =
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
    Reference<css::datatransfer::XTransferable>              m_aContents;
    Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector< Reference<css::datatransfer::clipboard::XClipboardListener> > m_aListeners;
    std::vector<GtkTargetEntry> m_aGtkTargets;
    VclToGtkHelper m_aConversionHelper;

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
};

OUString VclGtkClipboard::getImplementationName()
{
    return OUString("com.sun.star.datatransfer.VclGtkClipboard");
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
    m_aConversionHelper.setSelectionData(m_aContents, selection_data, info);
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
    assert(m_aGtkTargets.empty());
}

std::vector<GtkTargetEntry> VclToGtkHelper::FormatsToGtk(const css::uno::Sequence<css::datatransfer::DataFlavor> &rFormats)
{
    std::vector<GtkTargetEntry> aGtkTargets;

    bool bHaveText(false), bHaveUTF8(false);
    for (int i = 0; i < rFormats.getLength(); ++i)
    {
        const css::datatransfer::DataFlavor& rFlavor = rFormats[i];

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

            gtk_clipboard_set_with_data(clipboard, aGtkTargets.data(), aGtkTargets.size(),
                                        ClipboardGetFunc, ClipboardClearFunc, this);
            gtk_clipboard_set_can_store(clipboard, aGtkTargets.data(), aGtkTargets.size());
        }

        m_aGtkTargets = aGtkTargets;
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
    if (arguments.getLength() == 0) {
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
    return OUString("com.sun.star.datatransfer.dnd.VclGtkDropTarget");
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
    return OUString("com.sun.star.datatransfer.dnd.VclGtkDragSource");
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

        gtk_gl_area_make_current(GTK_GL_AREA(m_pGLArea));
        glBindRenderbuffer(GL_RENDERBUFFER, m_nRenderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
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
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, m_nDepthScratchBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
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
        gtk_gl_area_attach_buffers(GTK_GL_AREA(m_pGLArea));
        glGenFramebuffersEXT(1, &m_nAreaFrameBuffer);

        GdkWindow *pWindow = gtk_widget_get_window(pParent);
        m_pContext = gdk_window_create_gl_context(pWindow, nullptr);
        gdk_gl_context_realize(m_pContext, nullptr);
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

OpenGLContext* GtkInstance::CreateOpenGLContext()
{
    return new GtkOpenGLContext;
}

// tdf#123800 avoid requiring wayland at runtime just because it existed at buildtime
bool DLSYM_GDK_IS_WAYLAND_DISPLAY(GdkDisplay* pDisplay)
{
    auto get_type = reinterpret_cast<GType (*) (void)>(dlsym(nullptr, "gdk_wayland_display_get_type"));
    if (!get_type)
        return false;
    return G_TYPE_CHECK_INSTANCE_TYPE(pDisplay, get_type());
}

bool DLSYM_GDK_IS_X11_DISPLAY(GdkDisplay* pDisplay)
{
    auto get_type = reinterpret_cast<GType (*) (void)>(dlsym(nullptr, "gdk_x11_display_get_type"));
    if (!get_type)
        return false;
    return G_TYPE_CHECK_INSTANCE_TYPE(pDisplay, get_type());
}

class GtkInstanceBuilder;

namespace
{
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

class GtkInstanceWidget : public virtual weld::Widget
{
protected:
    GtkWidget* m_pWidget;
    GtkInstanceBuilder* m_pBuilder;

    static gboolean signalFocusIn(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_in();
        return false;
    }

    void signal_focus_in()
    {
        m_aFocusInHdl.Call(*this);
    }

    static gboolean signalFocusOut(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_out();
        return false;
    }

    void signal_focus_out()
    {
        m_aFocusOutHdl.Call(*this);
    }

private:
    bool m_bTakeOwnership;
    bool m_bFrozen;
    sal_uInt16 m_nLastMouseButton;
    gulong m_nFocusInSignalId;
    gulong m_nFocusOutSignalId;
    gulong m_nKeyPressSignalId;
    gulong m_nKeyReleaseSignalId;
    gulong m_nSizeAllocateSignalId;
    gulong m_nButtonPressSignalId;
    gulong m_nMotionSignalId;
    gulong m_nButtonReleaseSignalId;
    gulong m_nDragMotionSignalId;
    gulong m_nDragDropSignalId;
    gulong m_nDragDropReceivedSignalId;
    gulong m_nDragLeaveSignalId;

    rtl::Reference<GtkDropTarget> m_xDropTarget;

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

    virtual bool signal_popup_menu(const Point&)
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
        int nClicks = 1;

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
                        return true;
                    }
                }
                nEventType = SalEvent::MouseButtonDown;
                break;
            case GDK_2BUTTON_PRESS:
                nClicks = 2;
                nEventType = SalEvent::MouseButtonDown;
                break;
            case GDK_3BUTTON_PRESS:
                nClicks = 3;
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

        Point aPos(pEvent->x, pEvent->y);
        if (AllSettings::GetLayoutRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());

        if (gdk_event_triggers_context_menu(reinterpret_cast<GdkEvent*>(pEvent)) && pEvent->type == GDK_BUTTON_PRESS)
        {
            //if handled for context menu, stop processing
            if (signal_popup_menu(aPos))
                return true;
        }

        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(pEvent->state);
        sal_uInt16 nCode = m_nLastMouseButton | (nModCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2));
        MouseEvent aMEvt(aPos, nClicks, ImplGetMouseButtonMode(m_nLastMouseButton, nModCode), nCode, nCode);

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
        if (!m_aMouseMotionHdl.IsSet())
            return false;

        Point aPos(pEvent->x, pEvent->y);
        if (AllSettings::GetLayoutRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());
        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(pEvent->state);
        sal_uInt16 nCode = m_nLastMouseButton | (nModCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2));
        MouseEvent aMEvt(aPos, 0, ImplGetMouseMoveMode(nModCode), nCode, nCode);

        m_aMouseMotionHdl.Call(aMEvt);
        return true;
    }

    static gboolean signalDragMotion(GtkWidget *pWidget, GdkDragContext *context, gint x, gint y, guint time, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
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

    static void signalDragLeave(GtkWidget *pWidget, GdkDragContext *context, guint time, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        pThis->m_xDropTarget->signalDragLeave(pWidget, context, time);
    }

public:
    GtkInstanceWidget(GtkWidget* pWidget, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : m_pWidget(pWidget)
        , m_pBuilder(pBuilder)
        , m_bTakeOwnership(bTakeOwnership)
        , m_bFrozen(false)
        , m_nLastMouseButton(0)
        , m_nFocusInSignalId(0)
        , m_nFocusOutSignalId(0)
        , m_nKeyPressSignalId(0)
        , m_nKeyReleaseSignalId(0)
        , m_nSizeAllocateSignalId(0)
        , m_nButtonPressSignalId(0)
        , m_nMotionSignalId(0)
        , m_nButtonReleaseSignalId(0)
        , m_nDragMotionSignalId(0)
        , m_nDragDropSignalId(0)
        , m_nDragDropReceivedSignalId(0)
        , m_nDragLeaveSignalId(0)
    {
    }

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override
    {
        m_nKeyPressSignalId = g_signal_connect(m_pWidget, "key-press-event", G_CALLBACK(signalKey), this);
        weld::Widget::connect_key_press(rLink);
    }

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override
    {
        m_nKeyReleaseSignalId = g_signal_connect(m_pWidget, "key-release-event", G_CALLBACK(signalKey), this);
        weld::Widget::connect_key_release(rLink);
    }

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink) override
    {
        m_nButtonPressSignalId = g_signal_connect(m_pWidget, "button-press-event", G_CALLBACK(signalButton), this);
        weld::Widget::connect_mouse_press(rLink);
    }

    virtual void connect_mouse_move(const Link<const MouseEvent&, bool>& rLink) override
    {
        m_nMotionSignalId = g_signal_connect(m_pWidget, "motion-notify-event", G_CALLBACK(signalMotion), this);
        weld::Widget::connect_mouse_move(rLink);
    }

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink) override
    {
        m_nButtonReleaseSignalId = g_signal_connect(m_pWidget, "button-release-event", G_CALLBACK(signalButton), this);
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
        gtk_widget_grab_focus(m_pWidget);
    }

    virtual bool has_focus() const override
    {
        return gtk_widget_has_focus(m_pWidget);
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

    virtual void set_margin_top(int nMargin) override
    {
        gtk_widget_set_margin_top(m_pWidget, nMargin);
    }

    virtual void set_margin_bottom(int nMargin) override
    {
        gtk_widget_set_margin_bottom(m_pWidget, nMargin);
    }

    virtual void set_accessible_name(const OUString& rName) override
    {
        AtkObject* pAtkObject = gtk_widget_get_accessible(m_pWidget);
        if (!pAtkObject)
            return;
        atk_object_set_name(pAtkObject, OUStringToOString(rName, RTL_TEXTENCODING_UTF8).getStr());
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

    virtual bool get_extents_relative_to(weld::Widget& rRelative, int& x, int &y, int& width, int &height) override
    {
        //for toplevel windows this is sadly futile under wayland, so we can't tell where a dialog is in order to allow
        //the document underneath to auto-scroll to place content in a visible location
        gboolean ret = gtk_widget_translate_coordinates(m_pWidget,
                                                        dynamic_cast<GtkInstanceWidget&>(rRelative).getWidget(),
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

    virtual weld::Container* weld_parent() const override;

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

    GtkWidget* getWidget()
    {
        return m_pWidget;
    }

    GtkWindow* getWindow()
    {
        return GTK_WINDOW(gtk_widget_get_toplevel(m_pWidget));
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        m_nFocusInSignalId = g_signal_connect(m_pWidget, "focus-in-event", G_CALLBACK(signalFocusIn), this);
        weld::Widget::connect_focus_in(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
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

    gboolean signal_key(const GdkEventKey* pEvent)
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
        m_bFrozen = true;
    }

    virtual void thaw() override
    {
        gtk_widget_thaw_child_notify(m_pWidget);
        m_bFrozen = false;
    }

    bool get_frozen() const { return m_bFrozen; }

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override
    {
        if (!m_xDropTarget)
        {
            m_xDropTarget.set(new GtkDropTarget);
            m_nDragMotionSignalId = g_signal_connect(m_pWidget, "drag-motion", G_CALLBACK(signalDragMotion), this);
            m_nDragDropSignalId = g_signal_connect(m_pWidget, "drag-drop", G_CALLBACK(signalDragDrop), this);
            m_nDragDropReceivedSignalId = g_signal_connect(m_pWidget, "drag-data-received", G_CALLBACK(signalDragDropReceived), this);
            m_nDragLeaveSignalId = g_signal_connect(m_pWidget, "drag-leave", G_CALLBACK(signalDragLeave), this);
        }
        return m_xDropTarget.get();
    }

    virtual ~GtkInstanceWidget() override
    {
        if (m_nDragMotionSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragMotionSignalId);
        if (m_nDragDropSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragDropSignalId);
        if (m_nDragDropReceivedSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragDropReceivedSignalId);
        if (m_nDragLeaveSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nDragLeaveSignalId);
        if (m_nKeyPressSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nKeyPressSignalId);
        if (m_nKeyReleaseSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nKeyReleaseSignalId);
        if (m_nButtonPressSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nButtonPressSignalId);
        if (m_nMotionSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nMotionSignalId);
        if (m_nButtonReleaseSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nButtonReleaseSignalId);
        if (m_nFocusInSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nFocusInSignalId);
        if (m_nFocusOutSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nFocusOutSignalId);
        if (m_nSizeAllocateSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nSizeAllocateSignalId);
        if (m_bTakeOwnership)
            gtk_widget_destroy(m_pWidget);
    }

    virtual void disable_notify_events()
    {
        if (m_nFocusInSignalId)
            g_signal_handler_block(m_pWidget, m_nFocusInSignalId);
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
        // create with no seperate alpha layer like everything sane does
        auto xRet = VclPtr<VirtualDevice>::Create();
        xRet->SetBackground(COL_TRANSPARENT);
        return xRet;
    }
};

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
        GdkPixbufLoader *pixbuf_loader = gdk_pixbuf_loader_new();
        gdk_pixbuf_loader_write(pixbuf_loader, static_cast<const guchar*>(rStream.GetData()),
                                rStream.TellEnd(), nullptr);
        gdk_pixbuf_loader_close(pixbuf_loader, nullptr);
        GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);
        if (pixbuf)
            g_object_ref(pixbuf);
        g_object_unref(pixbuf_loader);
        return pixbuf;
    }

    GdkPixbuf* load_icon_by_name(const OUString& rIconName, const OUString& rIconTheme, const OUString& rUILang)
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
    return load_icon_by_name(rIconName, sIconTheme, sUILang);
}

namespace
{
    GdkPixbuf* getPixbuf(const VirtualDevice& rDevice)
    {
        Size aSize(rDevice.GetOutputSizePixel());
        cairo_surface_t* surface = get_underlying_cairo_surface(rDevice);
        double m_fXScale, m_fYScale;
        cairo_surface_get_device_scale(surface, &m_fXScale, &m_fYScale);
        return gdk_pixbuf_get_from_surface(surface, 0, 0, aSize.Width() * m_fXScale, aSize.Height() * m_fYScale);
    }

    GtkWidget* image_new_from_virtual_device(const VirtualDevice& rImageSurface)
    {
        GtkWidget* pImage = nullptr;
        if (gtk_check_version(3, 20, 0) == nullptr)
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
            pImage = gtk_image_new_from_surface(target);
            cairo_surface_destroy(target);
        }
        else
        {
            GdkPixbuf* pixbuf = getPixbuf(rImageSurface);
            pImage = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
        }
        return pImage;
    }
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
        else
        {
            MenuHelper* pThis = static_cast<MenuHelper*>(widget);
            pThis->add_to_map(pMenuItem);
        }
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
        g_signal_handlers_disconnect_by_data(iter->second, this);
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
                     bool bCheck)
    {
        GtkWidget* pImage = nullptr;
        if (pIconName)
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
            GtkWidget *pLabel = gtk_label_new(MapToGtkAccelerator(rStr).getStr());
            pItem = bCheck ? gtk_check_menu_item_new() : gtk_menu_item_new();
            gtk_container_add(GTK_CONTAINER(pBox), pImage);
            gtk_container_add(GTK_CONTAINER(pBox), pLabel);
            gtk_container_add(GTK_CONTAINER(pItem), pBox);
            gtk_widget_show_all(pItem);
        }
        else
        {
            pItem = bCheck ? gtk_check_menu_item_new_with_label(MapToGtkAccelerator(rStr).getStr())
                           : gtk_menu_item_new_with_label(MapToGtkAccelerator(rStr).getStr());
        }
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

    void set_item_label(const OString& rIdent, const OUString& rText)
    {
        gtk_menu_item_set_label(m_aMap[rIdent], MapToGtkAccelerator(rText).getStr());
    }

    void set_item_help_id(const OString& rIdent, const OString& rHelpId)
    {
        set_help_id(GTK_WIDGET(m_aMap[rIdent]), rHelpId);
    }

    OString get_item_help_id(const OString& rIdent) const
    {
        return get_help_id(GTK_WIDGET(m_aMap.find(rIdent)->second));
    }

    void show_item(const OString& rIdent, bool bShow)
    {
        GtkWidget* pWidget = GTK_WIDGET(m_aMap[rIdent]);
        if (bShow)
            gtk_widget_show(pWidget);
        else
            gtk_widget_hide(pWidget);
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
        : m_pGroup(gtk_size_group_new(GTK_SIZE_GROUP_NONE))
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

class GtkInstanceContainer : public GtkInstanceWidget, public virtual weld::Container
{
private:
    GtkContainer* m_pContainer;
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
};

weld::Container* GtkInstanceWidget::weld_parent() const
{
    GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
    return pParent ? new GtkInstanceContainer(GTK_CONTAINER(pParent), m_pBuilder, false) : nullptr;
}

class GtkInstanceWindow : public GtkInstanceContainer, public virtual weld::Window
{
private:
    GtkWindow* m_pWindow;
    rtl::Reference<SalGtkXWindow> m_xWindow; //uno api

    static void help_pressed(GtkAccelGroup*, GObject*, guint, GdkModifierType, gpointer widget)
    {
        GtkInstanceWindow* pThis = static_cast<GtkInstanceWindow*>(widget);
        pThis->help();
    }
protected:
    void help();
public:
    GtkInstanceWindow(GtkWindow* pWindow, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pWindow), pBuilder, bTakeOwnership)
        , m_pWindow(pWindow)
    {
        //hook up F1 to show help
        GtkAccelGroup *pGroup = gtk_accel_group_new();
        GClosure* closure = g_cclosure_new(G_CALLBACK(help_pressed), this, nullptr);
        gtk_accel_group_connect(pGroup, GDK_KEY_F1, static_cast<GdkModifierType>(0), GTK_ACCEL_LOCKED, closure);
        gtk_window_add_accel_group(pWindow, pGroup);
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
        if (!gtk_widget_get_realized(m_pWidget))
            gtk_widget_realize(m_pWidget);
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        GdkCursor *pCursor = bBusy ? gdk_cursor_new_from_name(pDisplay, "progress") : nullptr;
        gdk_window_set_cursor(gtk_widget_get_window(m_pWidget), pCursor);
        gdk_display_flush(pDisplay);
        if (pCursor)
            g_object_unref(pCursor);
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
    }

    virtual OString get_window_state(WindowStateMask nMask) const override
    {
        bool bPositioningAllowed = true;
#if defined(GDK_WINDOWING_WAYLAND)
        // drop x/y when under wayland
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        bPositioningAllowed = !DLSYM_GDK_IS_WAYLAND_DISPLAY(pDisplay);
#endif

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

        return ImplWindowStateToStr(aData);
    }

    virtual ~GtkInstanceWindow() override
    {
        if (m_xWindow.is())
            m_xWindow->clear();
    }
};

namespace
{
    struct ButtonOrder
    {
        const char * m_aType;
        int m_nPriority;
    };

    int getButtonPriority(const OString &rType)
    {
        static const size_t N_TYPES = 6;
        static const ButtonOrder aDiscardCancelSave[N_TYPES] =
        {
            { "/discard", 0 },
            { "/cancel", 1 },
            { "/no", 2 },
            { "/save", 3 },
            { "/yes", 3 },
            { "/ok", 3 }
        };

        static const ButtonOrder aSaveDiscardCancel[N_TYPES] =
        {
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
}

struct DialogRunner
{
    GtkDialog *m_pDialog;
    gint m_nResponseId;
    GMainLoop *m_pLoop;
    VclPtr<vcl::Window> m_xFrameWindow;

    DialogRunner(GtkDialog* pDialog)
       : m_pDialog(pDialog)
       , m_nResponseId(GTK_RESPONSE_NONE)
       , m_pLoop(nullptr)
    {
        GtkWindow* pParent = gtk_window_get_transient_for(GTK_WINDOW(m_pDialog));
        GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(pParent) : nullptr;
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

    static void signal_response(GtkDialog*, gint nResponseId, gpointer data)
    {
        DialogRunner* pThis = static_cast<DialogRunner*>(data);
        pThis->m_nResponseId = nResponseId;
        pThis->loop_quit();
    }

    static gboolean signal_delete(GtkDialog*, GdkEventAny*, gpointer data)
    {
        DialogRunner* pThis = static_cast<DialogRunner*>(data);
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
            m_xFrameWindow->IncModalCount();
    }

    void dec_modal_count()
    {
        if (m_xFrameWindow)
            m_xFrameWindow->DecModalCount();
    }

    // same as gtk_dialog_run except that unmap doesn't auto-respond
    // so we can hide the dialog and restore it without a response getting
    // triggered
    gint run()
    {
        g_object_ref(m_pDialog);

        inc_modal_count();

        bool bWasModal = gtk_window_get_modal(GTK_WINDOW(m_pDialog));
        if (!bWasModal)
            gtk_window_set_modal(GTK_WINDOW(m_pDialog), true);

        if (!gtk_widget_get_visible(GTK_WIDGET(m_pDialog)))
            gtk_widget_show(GTK_WIDGET(m_pDialog));

        gulong nSignalResponseId = g_signal_connect(m_pDialog, "response", G_CALLBACK(signal_response), this);
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
            gtk_window_set_modal(GTK_WINDOW(m_pDialog), false);

        g_signal_handler_disconnect(m_pDialog, nSignalResponseId);
        g_signal_handler_disconnect(m_pDialog, nSignalDeleteId);
        g_signal_handler_disconnect(m_pDialog, nSignalDestroyId);

        dec_modal_count();

        g_object_unref(m_pDialog);

        return m_nResponseId;
    }
};

class GtkInstanceDialog : public GtkInstanceWindow, public virtual weld::Dialog
{
private:
    GtkDialog* m_pDialog;
    DialogRunner m_aDialogRun;
    std::shared_ptr<weld::DialogController> m_xDialogController;
    // Used to keep ourself alive during a runAsync(when doing runAsync without a DialogController)
    std::shared_ptr<GtkInstanceDialog> m_xRunAsyncSelf;
    std::function<void(sal_Int32)> m_aFunc;
    gulong m_nCloseSignalId;
    gulong m_nResponseSignalId;

    static void signalClose(GtkWidget*, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        pThis->response(RET_CANCEL);
    }

    static void signalAsyncResponse(GtkWidget*, gint ret, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        pThis->asyncresponse(ret);
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
        return ret;
    }

    void asyncresponse(gint ret)
    {
        if (ret == GTK_RESPONSE_HELP)
        {
            help();
            return;
        }
        else if (has_click_handler(ret))
            return;

        hide();
        m_aFunc(GtkToVcl(ret));
        m_aFunc = nullptr;
        // move the self pointer, otherwise it might be de-allocated by time we try to reset it
        std::shared_ptr<GtkInstanceDialog> me = std::move(m_xRunAsyncSelf);
        m_xDialogController.reset();
        me.reset();
    }

public:
    GtkInstanceDialog(GtkDialog* pDialog, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWindow(GTK_WINDOW(pDialog), pBuilder, bTakeOwnership)
        , m_pDialog(pDialog)
        , m_aDialogRun(pDialog)
        , m_nCloseSignalId(g_signal_connect(m_pDialog, "close", G_CALLBACK(signalClose), this))
        , m_nResponseSignalId(0)
    {
    }

    virtual bool runAsync(std::shared_ptr<weld::DialogController> rDialogController, const std::function<void(sal_Int32)>& func) override
    {
        assert(!m_nResponseSignalId);

        m_xDialogController = rDialogController;
        m_aFunc = func;

        show();

        m_nResponseSignalId = g_signal_connect(m_pDialog, "response", G_CALLBACK(signalAsyncResponse), this);

        return true;
    }

    virtual bool runAsync(const std::function<void(sal_Int32)>& func) override
    {
        assert(!m_nResponseSignalId);

        m_xRunAsyncSelf.reset(this);
        m_aFunc = func;

        show();

        m_nResponseSignalId = g_signal_connect(m_pDialog, "response", G_CALLBACK(signalAsyncResponse), this);

        return true;
    }

    bool has_click_handler(int nResponse);

    virtual int run() override
    {
        sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(m_pDialog)));
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

    virtual void hide() override
    {
        if (!gtk_widget_get_visible(m_pWidget))
            return;
        gtk_widget_hide(m_pWidget);
        // if we hide the dialog while its running, then decrement the parent LibreOffice window
        // modal count, we expect the dialog to restored while its running and match up with
        // the inc_modal_count of show()
        //
        // This hide while running case is for the calc/chart dialogs which put
        // up an extra range chooser dialog, hides the original, the user can
        // select a range of cells and on completion the original dialog is
        // restored
        if (m_aDialogRun.loop_is_running())
            m_aDialogRun.dec_modal_count();
    }

    virtual void show() override
    {
        if (gtk_widget_get_visible(m_pWidget))
            return;
        sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(m_pDialog)));
        gtk_widget_show(m_pWidget);
        // see hide comment
        if (m_aDialogRun.loop_is_running())
            m_aDialogRun.inc_modal_count();
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

    virtual void response(int nResponse) override;

    virtual void add_button(const OUString& rText, int nResponse, const OString& rHelpId) override
    {
        GtkWidget* pWidget = gtk_dialog_add_button(m_pDialog, MapToGtkAccelerator(rText).getStr(), VclToGtk(nResponse));
        if (!rHelpId.isEmpty())
            ::set_help_id(pWidget, rHelpId);
    }

    virtual void set_default_response(int nResponse) override
    {
        gtk_dialog_set_default_response(m_pDialog, VclToGtk(nResponse));
    }

    virtual weld::Button* get_widget_for_response(int nResponse) override;

    virtual Container* weld_content_area() override
    {
        return new GtkInstanceContainer(GTK_CONTAINER(gtk_dialog_get_content_area(m_pDialog)), m_pBuilder, false);
    }

    virtual void SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>&) override
    {
        //not implemented for the gtk variant
    }

    virtual ~GtkInstanceDialog() override
    {
        g_signal_handler_disconnect(m_pDialog, m_nCloseSignalId);
        if (m_nResponseSignalId)
            g_signal_handler_disconnect(m_pDialog, m_nResponseSignalId);
    }
};

class GtkInstanceMessageDialog : public GtkInstanceDialog, public virtual weld::MessageDialog
{
private:
    GtkMessageDialog* m_pMessageDialog;
public:
    GtkInstanceMessageDialog(GtkMessageDialog* pMessageDialog, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceDialog(GTK_DIALOG(pMessageDialog), pBuilder, bTakeOwnership)
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
        gtk_label_set_label(GTK_LABEL(gtk_frame_get_label_widget(m_pFrame)), MapToGtkAccelerator(rText).getStr());
    }

    virtual OUString get_label() const override
    {
        const gchar* pStr = gtk_frame_get_label(m_pFrame);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }
};

static GType crippled_viewport_get_type();

#define CRIPPLED_TYPE_VIEWPORT            (crippled_viewport_get_type ())
#define CRIPPLED_VIEWPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CRIPPLED_TYPE_VIEWPORT, CrippledViewport))
#ifndef NDEBUG
#   define CRIPPLED_IS_VIEWPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CRIPPLED_TYPE_VIEWPORT))
#endif

struct CrippledViewport
{
    GtkViewport viewport;

    GtkAdjustment  *hadjustment;
    GtkAdjustment  *vadjustment;
};

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

class GtkInstanceScrolledWindow : public GtkInstanceContainer, public virtual weld::ScrolledWindow
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
    GtkInstanceScrolledWindow(GtkScrolledWindow* pScrolledWindow, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pScrolledWindow), pBuilder, bTakeOwnership)
        , m_pScrolledWindow(pScrolledWindow)
        , m_pOrigViewport(nullptr)
        , m_pVAdjustment(gtk_scrolled_window_get_vadjustment(m_pScrolledWindow))
        , m_pHAdjustment(gtk_scrolled_window_get_hadjustment(m_pScrolledWindow))
        , m_nVAdjustChangedSignalId(g_signal_connect(m_pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustValueChanged), this))
        , m_nHAdjustChangedSignalId(g_signal_connect(m_pHAdjustment, "value-changed", G_CALLBACK(signalHAdjustValueChanged), this))
    {
    }

    virtual void set_user_managed_scrolling() override
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
        gtk_adjustment_configure(m_pHAdjustment, value, lower, upper, step_increment, page_increment, page_size);
        enable_notify_events();
    }

    virtual int hadjustment_get_value() const override
    {
        return gtk_adjustment_get_value(m_pHAdjustment);
    }

    virtual void hadjustment_set_value(int value) override
    {
        disable_notify_events();
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

    virtual void set_hpolicy(VclPolicyType eHPolicy) override
    {
        GtkPolicyType eGtkVPolicy;
        gtk_scrolled_window_get_policy(m_pScrolledWindow, nullptr, &eGtkVPolicy);
        gtk_scrolled_window_set_policy(m_pScrolledWindow, eGtkVPolicy, VclToGtk(eHPolicy));
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
        //put it back the way it was
        if (m_pOrigViewport)
        {
            disable_notify_events();
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
            enable_notify_events();
        }
        g_signal_handler_disconnect(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_disconnect(m_pHAdjustment, m_nHAdjustChangedSignalId);
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

        bool bAllow = !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
        if (!bAllow)
        {
            g_signal_stop_emission_by_name(m_pNotebook, "switch-page");
            return;
        }
        if (m_bOverFlowBoxActive)
            gtk_notebook_set_current_page(m_pOverFlowNotebook, gtk_notebook_get_n_pages(m_pOverFlowNotebook) - 1);
        OString sNewIdent(get_page_ident(nNewPage));
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
            append_page(m_pNotebook, sIdent, sLabel, pPage);

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

        disable_notify_events();

        // take the overflow pages, and put them back at the end of the normal one
        unsplit_notebooks();

        // now redo the split, the pages will be split the other way around this time
        std::swap(m_nStartTabCount, m_nEndTabCount);
        split_notebooks();

        enable_notify_events();

        // we want to call this outside enable_notify_events so that the main
        // notebook switch-page callback is triggered
        gtk_notebook_set_current_page(m_pNotebook, nNewPage);
    }

    static OString get_page_ident(GtkNotebook *pNotebook, guint nPage)
    {
        const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(pNotebook, gtk_notebook_get_nth_page(pNotebook, nPage));
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
        return OString(pStr, pStr ? strlen(pStr) : 0);
    }

    OString get_page_ident(int nPage) const
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

    static gint get_page_number(GtkNotebook *pNotebook, const OString& rIdent)
    {
        gint nPages = gtk_notebook_get_n_pages(pNotebook);
        for (gint i = 0; i < nPages; ++i)
        {
            const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(pNotebook, gtk_notebook_get_nth_page(pNotebook, i));
            const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
            if (strcmp(pStr, rIdent.getStr()) == 0)
                return i;
        }
        return -1;
    }

    void remove_page(GtkNotebook *pNotebook, const OString& rIdent)
    {
        disable_notify_events();
        gtk_notebook_remove_page(pNotebook, get_page_number(pNotebook, rIdent));
        enable_notify_events();
    }

    static OUString get_tab_label_text(GtkNotebook *pNotebook, guint nPage)
    {
        const gchar* pStr = gtk_notebook_get_tab_label_text(pNotebook, gtk_notebook_get_nth_page(pNotebook, nPage));
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
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

    void append_page(GtkNotebook *pNotebook, const OString& rIdent, const OUString& rLabel, GtkWidget *pChild)
    {
        disable_notify_events();

        GtkWidget *pTabWidget = gtk_label_new(MapToGtkAccelerator(rLabel).getStr());
        gtk_buildable_set_name(GTK_BUILDABLE(pTabWidget), rIdent.getStr());

        gtk_notebook_append_page(pNotebook, pChild, pTabWidget);
        gtk_widget_show(pChild);
        gtk_widget_show(pTabWidget);

        enable_notify_events();
    }

    gint get_page_number(const OString& rIdent) const
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
            append_page(m_pOverFlowNotebook, sIdent, sLabel, gtk_grid_new());
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
        static const gchar olddata[] = "tab.top:active { box-shadow: none; padding: 0 0 0 0; margin: 0 0 0 0; border-image: none; border-image-width: 0 0 0 0; background-image: none; background-color: transparent; border-radius: 0 0 0 0; border-width: 0 0 0 0; border-style: none; border-color: transparent; opacity: 0; }";
        gtk_css_provider_load_from_data(pProvider, gtk_check_version(3, 20, 0) == nullptr ? data : olddata, -1, nullptr);
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

    virtual OString get_current_page_ident() const override
    {
        return get_page_ident(get_current_page());
    }

    virtual weld::Container* get_page(const OString& rIdent) const override
    {
        int nPage = get_page_number(rIdent);
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
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        gint nPage = get_page_number(rIdent);
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
        remove_page(m_pNotebook, rIdent);
    }

    virtual void append_page(const OString& rIdent, const OUString& rLabel) override
    {
        if (m_bOverFlowBoxActive)
        {
            unsplit_notebooks();
            reset_split_data();
        }

        // reset overflow and allow it to be recalculated if necessary
        gtk_widget_hide(GTK_WIDGET(m_pOverFlowNotebook));
        m_bOverFlowBoxActive = false;

        append_page(m_pNotebook, rIdent, rLabel, gtk_grid_new());
    }

    virtual ~GtkInstanceNotebook() override
    {
        if (m_nSizeAllocateSignalId)
            g_signal_handler_disconnect(m_pNotebook, m_nSizeAllocateSignalId);
        g_signal_handler_disconnect(m_pNotebook, m_nSwitchPageSignalId);
        g_signal_handler_disconnect(m_pNotebook, m_nFocusSignalId);
        g_signal_handler_disconnect(m_pNotebook, m_nChangeCurrentPageId);
        g_signal_handler_disconnect(m_pOverFlowNotebook, m_nOverFlowSwitchPageSignalId);
        gtk_widget_destroy(GTK_WIDGET(m_pOverFlowNotebook));
        if (m_pOverFlowBox)
            gtk_widget_destroy(GTK_WIDGET(m_pOverFlowBox));
    }
};

#include <vcl/pngwrite.hxx>

class GtkInstanceButton : public GtkInstanceContainer, public virtual weld::Button
{
private:
    GtkButton* m_pButton;
    gulong m_nSignalId;

    static void signalClicked(GtkButton*, gpointer widget)
    {
        GtkInstanceButton* pThis = static_cast<GtkInstanceButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_clicked();
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
            return;
        gtk_button_set_image(m_pButton, gtk_image_new_from_pixbuf(pixbuf));
        g_object_unref(pixbuf);
    }

    virtual OUString get_label() const override
    {
        return ::get_label(m_pButton);
    }

    virtual void set_label_line_wrap(bool wrap) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pButton));
        gtk_label_set_line_wrap(GTK_LABEL(pChild), wrap);
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

weld::Button* GtkInstanceDialog::get_widget_for_response(int nResponse)
{
    GtkButton* pButton = GTK_BUTTON(gtk_dialog_get_widget_for_response(m_pDialog, VclToGtk(nResponse)));
    if (!pButton)
        return nullptr;
    return new GtkInstanceButton(pButton, m_pBuilder, false);
}

void GtkInstanceDialog::response(int nResponse)
{
    //unblock this response now when activated through code
    if (GtkWidget* pWidget = gtk_dialog_get_widget_for_response(m_pDialog, VclToGtk(nResponse)))
    {
        void* pData = g_object_get_data(G_OBJECT(pWidget), "g-lo-GtkInstanceButton");
        GtkInstanceButton* pButton = static_cast<GtkInstanceButton*>(pData);
        if (pButton)
            pButton->clear_click_handler();
    }
    gtk_dialog_response(m_pDialog, VclToGtk(nResponse));
}

bool GtkInstanceDialog::has_click_handler(int nResponse)
{
    if (GtkWidget* pWidget = gtk_dialog_get_widget_for_response(m_pDialog, VclToGtk(nResponse)))
    {
        void* pData = g_object_get_data(G_OBJECT(pWidget), "g-lo-GtkInstanceButton");
        GtkInstanceButton* pButton = static_cast<GtkInstanceButton*>(pData);
        return pButton && pButton->has_click_handler();
    }
    return false;
}

class GtkInstanceToggleButton : public GtkInstanceButton, public virtual weld::ToggleButton
{
private:
    GtkToggleButton* m_pToggleButton;
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

class GtkInstanceMenuButton : public GtkInstanceToggleButton, public MenuHelper, public virtual weld::MenuButton
{
private:
    GtkMenuButton* m_pMenuButton;
    GtkBox* m_pBox;
    GtkImage* m_pImage;
    GtkWidget* m_pLabel;
    //popover cannot escape dialog under X so stick up own window instead
    GtkWindow* m_pMenuHack;
    GtkWidget* m_pPopover;
    gulong m_nSignalId;

    static void signalToggled(GtkWidget*, gpointer widget)
    {
        GtkInstanceMenuButton* pThis = static_cast<GtkInstanceMenuButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->toggle_menu();
    }

    void do_grab()
    {
        GdkDisplay *pDisplay = gtk_widget_get_display(GTK_WIDGET(m_pMenuHack));
#if GTK_CHECK_VERSION(3, 20, 0)
        if (gtk_check_version(3, 20, 0) == nullptr)
        {
            GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
            gdk_seat_grab(pSeat, gtk_widget_get_window(GTK_WIDGET(m_pMenuHack)),
                          GDK_SEAT_CAPABILITY_ALL, true, nullptr, nullptr, nullptr, nullptr);
            return;
        }
#endif
        //else older gtk3
        const int nMask = (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

        GdkDeviceManager* pDeviceManager = gdk_display_get_device_manager(pDisplay);
        GdkDevice* pPointer = gdk_device_manager_get_client_pointer(pDeviceManager);
        gdk_device_grab(pPointer, gtk_widget_get_window(GTK_WIDGET(m_pMenuHack)), GDK_OWNERSHIP_NONE,
                        true, GdkEventMask(nMask), nullptr, gtk_get_current_event_time());
    }

    void do_ungrab()
    {
        GdkDisplay *pDisplay = gtk_widget_get_display(GTK_WIDGET(m_pMenuHack));
#if GTK_CHECK_VERSION(3, 20, 0)
        if (gtk_check_version(3, 20, 0) == nullptr)
        {
            GdkSeat* pSeat = gdk_display_get_default_seat(pDisplay);
            gdk_seat_ungrab(pSeat);
            return;
        }
#endif
        //else older gtk3
        GdkDeviceManager* pDeviceManager = gdk_display_get_device_manager(pDisplay);
        GdkDevice* pPointer = gdk_device_manager_get_client_pointer(pDeviceManager);
        gdk_device_ungrab(pPointer, gtk_get_current_event_time());
    }

    void toggle_menu()
    {
        if (!m_pMenuHack)
            return;
        if (!get_active())
        {
            do_ungrab();

            gtk_widget_hide(GTK_WIDGET(m_pMenuHack));
            //put contents back from where the came from
            GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pMenuHack));
            g_object_ref(pChild);
            gtk_container_remove(GTK_CONTAINER(m_pMenuHack), pChild);
            gtk_container_add(GTK_CONTAINER(m_pPopover), pChild);
            g_object_unref(pChild);
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

            //place the toplevel just below its launcher button
            GtkWidget* pToplevel = gtk_widget_get_toplevel(GTK_WIDGET(m_pMenuButton));
            gint x, y, absx, absy;
            gtk_widget_translate_coordinates(GTK_WIDGET(m_pMenuButton), pToplevel, 0, 0, &x, &y);
            GdkWindow *pWindow = gtk_widget_get_window(pToplevel);
            gdk_window_get_position(pWindow, &absx, &absy);

            gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(pToplevel)), m_pMenuHack);
            gtk_window_set_transient_for(m_pMenuHack, GTK_WINDOW(pToplevel));

            gtk_widget_show_all(GTK_WIDGET(m_pMenuHack));
            gtk_window_move(m_pMenuHack, x + absx, y + absy + gtk_widget_get_allocated_height(GTK_WIDGET(m_pMenuButton)));

            gtk_widget_grab_focus(GTK_WIDGET(m_pMenuHack));

            do_grab();
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
            do_grab();
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

public:
    GtkInstanceMenuButton(GtkMenuButton* pMenuButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pMenuButton), pBuilder, bTakeOwnership)
        , MenuHelper(gtk_menu_button_get_popup(pMenuButton), false)
        , m_pMenuButton(pMenuButton)
        , m_pImage(nullptr)
        , m_pMenuHack(nullptr)
        , m_pPopover(nullptr)
        , m_nSignalId(0)
    {
        m_pLabel = gtk_bin_get_child(GTK_BIN(m_pMenuButton));
        //do it "manually" so we can have the dropdown image in GtkMenuButtons shown
        //on the right at the same time as this image is shown on the left
        g_object_ref(m_pLabel);
        gtk_container_remove(GTK_CONTAINER(m_pMenuButton), m_pLabel);

        m_pBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

        gtk_box_pack_start(m_pBox, m_pLabel, false, false, 0);
        g_object_unref(m_pLabel);

        if (gtk_toggle_button_get_mode(GTK_TOGGLE_BUTTON(m_pMenuButton)))
            gtk_box_pack_end(m_pBox, gtk_image_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_BUTTON), false, false, 0);

        gtk_container_add(GTK_CONTAINER(m_pMenuButton), GTK_WIDGET(m_pBox));
        gtk_widget_show_all(GTK_WIDGET(m_pBox));
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

    virtual void set_image(VirtualDevice* pDevice) override
    {
        if (!m_pImage)
        {
            m_pImage = GTK_IMAGE(gtk_image_new());
            GtkStyleContext *pContext = gtk_widget_get_style_context(GTK_WIDGET(m_pMenuButton));
            gint nImageSpacing(0);
            gtk_style_context_get_style(pContext, "image-spacing", &nImageSpacing, nullptr);
            gtk_box_pack_start(m_pBox, GTK_WIDGET(m_pImage), false, false, nImageSpacing);
            gtk_box_reorder_child(m_pBox, GTK_WIDGET(m_pImage), 0);
            gtk_widget_show(GTK_WIDGET(m_pImage));
        }
        if (pDevice)
        {
            if (gtk_check_version(3, 20, 0) == nullptr)
                gtk_image_set_from_surface(m_pImage, get_underlying_cairo_surface(*pDevice));
            else
            {
                GdkPixbuf* pixbuf = getPixbuf(*pDevice);
                gtk_image_set_from_pixbuf(m_pImage, pixbuf);
                g_object_unref(pixbuf);
            }
        }
        else
            gtk_image_set_from_surface(m_pImage, nullptr);
    }

    virtual void insert_item(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface, bool bCheck) override
    {
        MenuHelper::insert_item(pos, rId, rStr, pIconName, pImageSurface, bCheck);
    }

    virtual void remove_item(const OString& rId) override
    {
        MenuHelper::remove_item(rId);
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
            gtk_menu_button_set_popover(m_pMenuButton, gtk_popover_new(GTK_WIDGET(m_pMenuButton)));
        }
        else
        {
            gtk_menu_button_set_popover(m_pMenuButton, m_pPopover);
            if (m_pPopover)
                gtk_widget_show_all(m_pPopover);
        }
    }

    virtual ~GtkInstanceMenuButton() override
    {
        if (m_pMenuHack)
        {
            g_signal_handler_disconnect(m_pMenuButton, m_nSignalId);
            gtk_widget_destroy(GTK_WIDGET(m_pMenuHack));
        }
    }
};

class GtkInstanceMenu : public MenuHelper, public virtual weld::Menu
{
protected:
    std::vector<GtkMenuItem*> m_aExtraItems;
    OString m_sActivated;
    GtkInstanceMenuButton* m_pTopLevelMenuButton;

private:
    virtual void signal_activate(GtkMenuItem* pItem) override
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pItem));
        m_sActivated = OString(pStr, pStr ? strlen(pStr) : 0);
    }

public:
    GtkInstanceMenu(GtkMenu* pMenu, bool bTakeOwnership)
        : MenuHelper(pMenu, bTakeOwnership)
        , m_pTopLevelMenuButton(nullptr)
    {
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
        if (pTopLevelMenu != pMenu)
        {
            GtkWidget* pAttached = gtk_menu_get_attach_widget(pTopLevelMenu);
            if (pAttached && GTK_IS_MENU_BUTTON(pAttached))
            {
                void* pData = g_object_get_data(G_OBJECT(pAttached), "g-lo-GtkInstanceButton");
                m_pTopLevelMenuButton = dynamic_cast<GtkInstanceMenuButton*>(static_cast<GtkInstanceButton*>(pData));
            }
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
            if (AllSettings::GetLayoutRTL())
                aRect.x = gtk_widget_get_allocated_width(pWidget) - aRect.width - 1 - aRect.x;
            gtk_menu_popup_at_rect(m_pMenu, gtk_widget_get_window(pWidget), &aRect, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, nullptr);
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

    virtual void show(const OString& rIdent, bool bShow) override
    {
        show_item(rIdent, bShow);
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        bool bCheck) override
    {
        GtkWidget* pImage = nullptr;
        if (pIconName)
        {
            GdkPixbuf* pixbuf = load_icon_by_name(*pIconName);
            if (!pixbuf)
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
            GtkWidget *pLabel = gtk_label_new(MapToGtkAccelerator(rStr).getStr());
            pItem = bCheck ? gtk_check_menu_item_new() : gtk_menu_item_new();
            gtk_container_add(GTK_CONTAINER(pBox), pImage);
            gtk_container_add(GTK_CONTAINER(pBox), pLabel);
            gtk_container_add(GTK_CONTAINER(pItem), pBox);
            gtk_widget_show_all(pItem);
        }
        else
        {
            pItem = bCheck ? gtk_check_menu_item_new_with_label(MapToGtkAccelerator(rStr).getStr())
                           : gtk_menu_item_new_with_label(MapToGtkAccelerator(rStr).getStr());
        }
        gtk_buildable_set_name(GTK_BUILDABLE(pItem), OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr());
        gtk_menu_shell_append(GTK_MENU_SHELL(m_pMenu), pItem);
        gtk_widget_show(pItem);
        GtkMenuItem* pMenuItem = GTK_MENU_ITEM(pItem);
        m_aExtraItems.push_back(pMenuItem);
        add_to_map(pMenuItem);
        if (m_pTopLevelMenuButton)
            m_pTopLevelMenuButton->add_to_map(pMenuItem);
        if (pos != -1)
            gtk_menu_reorder_child(m_pMenu, pItem, pos);
    }

    virtual ~GtkInstanceMenu() override
    {
        if (m_pTopLevelMenuButton)
        {
            for (auto a : m_aExtraItems)
                m_pTopLevelMenuButton->remove_from_map(a);
        }
    }
};

class GtkInstanceLinkButton : public GtkInstanceContainer, public virtual weld::LinkButton
{
private:
    GtkLinkButton* m_pButton;
    gulong m_nSignalId;

    static void signalClicked(GtkButton*, gpointer widget)
    {
        GtkInstanceLinkButton* pThis = static_cast<GtkInstanceLinkButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_clicked();
    }

public:
    GtkInstanceLinkButton(GtkLinkButton* pButton, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pButton), pBuilder, bTakeOwnership)
        , m_pButton(pButton)
        , m_nSignalId(g_signal_connect(pButton, "clicked", G_CALLBACK(signalClicked), this))
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
};

class GtkInstanceCalendar : public GtkInstanceWidget, public virtual weld::Calendar
{
private:
    GtkCalendar* m_pCalendar;
    gulong m_nDaySelectedSignalId;
    gulong m_nDaySelectedDoubleClickSignalId;
    gulong m_nKeyPressEventSignalId;

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

    gboolean signal_key_press(GdkEventKey* pEvent)
    {
        if (pEvent->keyval == GDK_KEY_Return)
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

public:
    GtkInstanceCalendar(GtkCalendar* pCalendar, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pCalendar), pBuilder, bTakeOwnership)
        , m_pCalendar(pCalendar)
        , m_nDaySelectedSignalId(g_signal_connect(pCalendar, "day-selected", G_CALLBACK(signalDaySelected), this))
        , m_nDaySelectedDoubleClickSignalId(g_signal_connect(pCalendar, "day-selected-double-click", G_CALLBACK(signalDaySelectedDoubleClick), this))
        , m_nKeyPressEventSignalId(g_signal_connect(pCalendar, "key-press-event", G_CALLBACK(signalKeyPress), this))
    {
    }

    virtual void set_date(const Date& rDate) override
    {
        gtk_calendar_select_month(m_pCalendar, rDate.GetMonth(), rDate.GetYear());
        gtk_calendar_select_day(m_pCalendar, rDate.GetDay());
    }

    virtual Date get_date() const override
    {
        guint year, month, day;
        gtk_calendar_get_date(m_pCalendar, &year, &month, &day);
        return Date(day, month, year);
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
        g_signal_handler_disconnect(m_pCalendar, m_nKeyPressEventSignalId);
        g_signal_handler_disconnect(m_pCalendar, m_nDaySelectedDoubleClickSignalId);
        g_signal_handler_disconnect(m_pCalendar, m_nDaySelectedSignalId);
    }
};

class GtkInstanceEntry : public GtkInstanceWidget, public virtual weld::Entry
{
private:
    GtkEntry* m_pEntry;
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
            g_signal_handlers_block_by_func(pEntry, gpointer(signalInsertText), this);
            gtk_editable_insert_text(GTK_EDITABLE(pEntry), sFinalText.getStr(), sFinalText.getLength(), position);
            g_signal_handlers_unblock_by_func(pEntry, gpointer(signalInsertText), this);
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

    void signal_activate()
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

    virtual void set_error(bool bError) override
    {
        if (bError)
            gtk_entry_set_icon_from_icon_name(m_pEntry, GTK_ENTRY_ICON_SECONDARY, "dialog-error");
        else
            gtk_entry_set_icon_from_icon_name(m_pEntry, GTK_ENTRY_ICON_SECONDARY, nullptr);
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
        gtk_entry_set_attributes(m_pEntry, pAttrList);
        pango_attr_list_unref(pAttrList);
    }

    void fire_signal_changed()
    {
        signal_changed();
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

namespace
{
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
            pixbuf = load_icon_by_name(rIconName,
                                       rSettings.GetStyleSettings().DetermineIconTheme(),
                                       rSettings.GetUILanguageTag().getBcp47());
        }

        return pixbuf;
    }

    GdkPixbuf* getPixbuf(const css::uno::Reference<css::graphic::XGraphic>& rImage)
    {
        Image aImage(rImage);

        std::unique_ptr<SvMemoryStream> xMemStm(new SvMemoryStream);
        vcl::PNGWriter aWriter(aImage.GetBitmapEx());
        aWriter.Write(*xMemStm);

        return load_icon_from_stream(*xMemStm);
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
    gint sort_func(GtkTreeModel* pModel, GtkTreeIter* a, GtkTreeIter* b, gpointer data)
    {
        comphelper::string::NaturalStringSorter* pSorter = static_cast<comphelper::string::NaturalStringSorter*>(data);
        gchar* pName1;
        gchar* pName2;
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
        gint sort_column_id(0);
        gtk_tree_sortable_get_sort_column_id(pSortable, &sort_column_id, nullptr);
        gtk_tree_model_get(pModel, a, sort_column_id, &pName1, -1);
        gtk_tree_model_get(pModel, b, sort_column_id, &pName2, -1);
        gint ret = pSorter->compare(OUString(pName1, strlen(pName1), RTL_TEXTENCODING_UTF8),
                                    OUString(pName2, strlen(pName2), RTL_TEXTENCODING_UTF8));
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
    GtkTreeIter iter;
};

class GtkInstanceTreeView : public GtkInstanceContainer, public virtual weld::TreeView
{
private:
    GtkTreeView* m_pTreeView;
    GtkTreeStore* m_pTreeStore;
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    GList *m_pColumns;
    std::vector<gulong> m_aColumnSignalIds;
    // map from toggle column to toggle visibility column
    std::map<int, int> m_aToggleVisMap;
    std::vector<GtkSortType> m_aSavedSortTypes;
    std::vector<int> m_aSavedSortColumns;
    std::vector<int> m_aViewColToModelCol;
    std::vector<int> m_aModelColToViewCol;
    gint m_nTextCol;
    gint m_nImageCol;
    gint m_nExpanderImageCol;
    gint m_nIdCol;
    gulong m_nChangedSignalId;
    gulong m_nRowActivatedSignalId;
    gulong m_nTestExpandRowSignalId;
    gulong m_nVAdjustmentChangedSignalId;
    gulong m_nRowDeletedSignalId;
    gulong m_nRowInsertedSignalId;

    DECL_LINK(async_signal_changed, void*, void);

    static void signalChanged(GtkTreeView*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        //tdf#117991 selection change is sent before the focus change, and focus change
        //is what will cause a spinbutton that currently has the focus to set its contents
        //as the spin button value. So any LibreOffice callbacks on
        //signal-change would happen before the spinbutton value-change occurs.
        //To avoid this, send the signal-change to LibreOffice to occur after focus-change
        //has been processed
        Application::PostUserEvent(LINK(pThis, GtkInstanceTreeView, async_signal_changed));
    }

    static void signalRowActivated(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_row_activated();
    }

    void insert_row(GtkTreeIter& iter, const GtkTreeIter* parent, int pos, const OUString* pId, const OUString* pText,
                    const OUString* pIconName, const VirtualDevice* pDevice, const OUString* pExpanderName)
    {
        gtk_tree_store_insert_with_values(m_pTreeStore, &iter, const_cast<GtkTreeIter*>(parent), pos,
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

            gtk_tree_store_set(m_pTreeStore, &iter, m_nImageCol, target, -1);
            cairo_surface_destroy(target);
        }

        if (pExpanderName)
        {
            GdkPixbuf* pixbuf = getPixbuf(*pExpanderName);
            gtk_tree_store_set(m_pTreeStore, &iter, m_nExpanderImageCol, pixbuf, -1);
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

    OUString get(int pos, int col) const
    {
        OUString sRet;
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
            sRet = get(iter, col);
        return sRet;
    }

    bool get_bool(int pos, int col) const
    {
        gboolean bRet(false);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
        {
            gtk_tree_model_get(pModel, &iter, col, &bRet, -1);
        }
        return bRet;
    }

    void set(const GtkTreeIter& iter, int col, const OUString& rText)
    {
        OString aStr(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gtk_tree_store_set(m_pTreeStore, const_cast<GtkTreeIter*>(&iter), col, aStr.getStr(), -1);
    }

    void set(int pos, int col, const OUString& rText)
    {
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
        {
            set(iter, col, rText);
        }
    }

    void set(int pos, int col, bool bOn)
    {
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
        {
            gtk_tree_store_set(m_pTreeStore, &iter, col, bOn, -1);
        }
    }

    static gboolean signalTestExpandRow(GtkTreeView*, GtkTreeIter* iter, GtkTreePath*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        return !pThis->signal_test_expand_row(*iter);
    }

    bool signal_test_expand_row(GtkTreeIter& iter)
    {
        disable_notify_events();
        GtkInstanceTreeIter aIter(nullptr);

        // if there's a preexisting placeholder child, required to make this
        // potentially expandable in the first place, now we remove it
        bool bPlaceHolder = false;
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter tmp;
        if (gtk_tree_model_iter_children(pModel, &tmp, &iter))
        {
            aIter.iter = tmp;
            if (get_text(aIter, -1) == "<dummy>")
            {
                gtk_tree_store_remove(m_pTreeStore, &tmp);
                bPlaceHolder = true;
            }
        }

        aIter.iter = iter;
        bool bRet = signal_expanding(aIter);

        //expand disallowed, restore placeholder
        if (!bRet && bPlaceHolder)
        {
            GtkTreeIter subiter;
            OUString sDummy("<dummy>");
            insert_row(subiter, &iter, -1, nullptr, &sDummy, nullptr, nullptr, nullptr);
        }

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

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter iter;
        gtk_tree_model_get_iter(pModel, &iter, tree_path);

        gboolean bRet(false);
        gtk_tree_model_get(pModel, &iter, nCol, &bRet, -1);
        bRet = !bRet;
        gtk_tree_store_set(m_pTreeStore, &iter, nCol, bRet, -1);

        gint depth;
        gint* indices = gtk_tree_path_get_indices_with_depth(tree_path, &depth);
        int nRow = indices[depth-1];

        signal_toggled(std::make_pair(nRow, nCol));

        gtk_tree_path_free(tree_path);
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

    int get_model_col(int viewcol) const
    {
        return m_aViewColToModelCol[viewcol];
    }

    int get_view_col(int modelcol) const
    {
        return m_aModelColToViewCol[modelcol];
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

public:
    GtkInstanceTreeView(GtkTreeView* pTreeView, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pTreeView), pBuilder, bTakeOwnership)
        , m_pTreeView(pTreeView)
        , m_pTreeStore(GTK_TREE_STORE(gtk_tree_view_get_model(m_pTreeView)))
        , m_nTextCol(-1)
        , m_nImageCol(-1)
        , m_nExpanderImageCol(-1)
        , m_nChangedSignalId(g_signal_connect(gtk_tree_view_get_selection(pTreeView), "changed",
                             G_CALLBACK(signalChanged), this))
        , m_nRowActivatedSignalId(g_signal_connect(pTreeView, "row-activated", G_CALLBACK(signalRowActivated), this))
        , m_nTestExpandRowSignalId(g_signal_connect(pTreeView, "test-expand-row", G_CALLBACK(signalTestExpandRow), this))
        , m_nVAdjustmentChangedSignalId(0)
    {
        m_pColumns = gtk_tree_view_get_columns(m_pTreeView);
        int nIndex(0);
        for (GList* pEntry = g_list_first(m_pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            m_aColumnSignalIds.push_back(g_signal_connect(pColumn, "clicked", G_CALLBACK(signalColumnClicked), this));
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
            {
                GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
                if (m_nTextCol == -1 && GTK_IS_CELL_RENDERER_TEXT(pCellRenderer))
                {
                    m_nTextCol = nIndex;
                }
                else if (GTK_IS_CELL_RENDERER_TOGGLE(pCellRenderer))
                {
                    g_object_set_data(G_OBJECT(pCellRenderer), "g-lo-CellIndex", reinterpret_cast<gpointer>(nIndex));
                    g_signal_connect(G_OBJECT(pCellRenderer), "toggled", G_CALLBACK(signalCellToggled), this);
                    m_aToggleVisMap[nIndex] = -1;
                }
                else if (GTK_IS_CELL_RENDERER_PIXBUF(pCellRenderer))
                {
                    const bool bExpander = g_list_next(pRenderer) != nullptr;
                    if (bExpander && m_nExpanderImageCol == -1)
                        m_nExpanderImageCol = nIndex;
                    else if (m_nImageCol == -1)
                        m_nImageCol = nIndex;
                }
                m_aModelColToViewCol.push_back(m_aViewColToModelCol.size());
                ++nIndex;
            }
            g_list_free(pRenderers);
            m_aViewColToModelCol.push_back(nIndex - 1);
        }
        m_nIdCol = nIndex++;
        for (auto& a : m_aToggleVisMap)
        {
            a.second = nIndex++;
        }

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        m_nRowDeletedSignalId = g_signal_connect(pModel, "row-deleted", G_CALLBACK(signalRowDeleted), this);
        m_nRowInsertedSignalId = g_signal_connect(pModel, "row-inserted", G_CALLBACK(signalRowInserted), this);
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

    virtual int get_column_width(int nColumn) const override
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, nColumn));
        assert(pColumn && "wrong count");
        return gtk_tree_view_column_get_fixed_width(pColumn) - gtk_tree_view_column_get_spacing(pColumn);
    }

    virtual OUString get_column_title(int nColumn) const override
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, nColumn));
        assert(pColumn && "wrong count");
        const gchar* pTitle = gtk_tree_view_column_get_title(pColumn);
        OUString sRet = OUString(pTitle, pTitle ? strlen(pTitle) : 0, RTL_TEXTENCODING_UTF8);
        return sRet;
    }

    virtual void set_column_title(int nColumn, const OUString& rTitle) override
    {
        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, nColumn));
        assert(pColumn && "wrong count");
        gtk_tree_view_column_set_title(pColumn, OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void insert(const weld::TreeIter* pParent, int pos, const OUString* pText, const OUString* pId, const OUString* pIconName,
                        VirtualDevice* pImageSurface, const OUString* pExpanderName,
                        bool bChildrenOnDemand, weld::TreeIter* pRet) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        const GtkInstanceTreeIter* pGtkIter = static_cast<const GtkInstanceTreeIter*>(pParent);
        insert_row(iter, pGtkIter ? &pGtkIter->iter : nullptr, pos, pId, pText, pIconName, pImageSurface, pExpanderName);
        if (bChildrenOnDemand)
        {
            GtkTreeIter subiter;
            OUString sDummy("<dummy>");
            insert_row(subiter, &iter, -1, nullptr, &sDummy, nullptr, nullptr, nullptr);
        }
        if (pRet)
        {
            GtkInstanceTreeIter* pGtkRetIter = static_cast<GtkInstanceTreeIter*>(pRet);
            pGtkRetIter->iter = iter;
        }
        enable_notify_events();
    }

    virtual void set_font_color(int pos, const Color& rColor) const override
    {
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pTreeStore), &iter, nullptr, pos);
        GdkRGBA aColor{rColor.GetRed()/255.0, rColor.GetGreen()/255.0, rColor.GetBlue()/255.0, 0};
        gtk_tree_store_set(m_pTreeStore, &iter, m_nIdCol + 1, &aColor, -1);
    }

    virtual void remove(int pos) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pTreeStore), &iter, nullptr, pos);
        gtk_tree_store_remove(m_pTreeStore, &iter);
        enable_notify_events();
    }

    virtual int find_text(const OUString& rText) const override
    {
        Search aSearch(rText, m_nTextCol);
        gtk_tree_model_foreach(GTK_TREE_MODEL(m_pTreeStore), foreach_find, &aSearch);
        return aSearch.index;
    }

    virtual int find_id(const OUString& rId) const override
    {
        Search aSearch(rId, m_nIdCol);
        gtk_tree_model_foreach(GTK_TREE_MODEL(m_pTreeStore), foreach_find, &aSearch);
        return aSearch.index;
    }

    void move_before(int pos, int before)
    {
        if (pos == before)
            return;

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);

        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos);

        GtkTreeIter position;
        gtk_tree_model_iter_nth_child(pModel, &position, nullptr, before);

        gtk_tree_store_move_before(m_pTreeStore, &iter, &position);
        enable_notify_events();
    }

    virtual void set_top_entry(int pos) override
    {
        disable_notify_events();
        move_before(pos, 0);
        enable_notify_events();
    }

    virtual void swap(int pos1, int pos2) override
    {
        disable_notify_events();

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);

        GtkTreeIter iter1;
        gtk_tree_model_iter_nth_child(pModel, &iter1, nullptr, pos1);

        GtkTreeIter iter2;
        gtk_tree_model_iter_nth_child(pModel, &iter2, nullptr, pos2);

        gtk_tree_store_swap(m_pTreeStore, &iter1, &iter2);

        enable_notify_events();
    }

    virtual void clear() override
    {
        disable_notify_events();
        gtk_tree_store_clear(m_pTreeStore);
        enable_notify_events();
    }

    virtual void make_sorted() override
    {
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetUILanguageTag().getLocale()));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
        gtk_tree_sortable_set_sort_column_id(pSortable, m_nTextCol, GTK_SORT_ASCENDING);
    }

    virtual void make_unsorted() override
    {
        m_xSorter.reset();
        int nSortColumn;
        GtkSortType eSortType;
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
        gtk_tree_sortable_get_sort_column_id(pSortable, &nSortColumn, &eSortType);
        gtk_tree_sortable_set_sort_column_id(pSortable, GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, eSortType);
    }

    virtual void set_sort_order(bool bAscending) override
    {
        GtkSortType eSortType = bAscending ? GTK_SORT_ASCENDING : GTK_SORT_DESCENDING;

        gint sort_column_id(0);
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
        gtk_tree_sortable_get_sort_column_id(pSortable, &sort_column_id, nullptr);
        gtk_tree_sortable_set_sort_column_id(pSortable, sort_column_id, eSortType);
    }

    virtual bool get_sort_order() const override
    {
        GtkSortType eSortType;

        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
        gtk_tree_sortable_get_sort_column_id(pSortable, nullptr, &eSortType);
        return eSortType == GTK_SORT_ASCENDING;
    }

    virtual void set_sort_indicator(TriState eState, int col) override
    {
        if (col == -1)
            col = get_view_col(m_nTextCol);

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
        if (col == -1)
            col = get_view_col(m_nTextCol);

        GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(g_list_nth_data(m_pColumns, col));
        if (!gtk_tree_view_column_get_sort_indicator(pColumn))
            return TRISTATE_INDET;
        return gtk_tree_view_column_get_sort_order(pColumn) == GTK_SORT_ASCENDING ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    virtual int get_sort_column() const override
    {
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
        gint sort_column_id(0);
        if (!gtk_tree_sortable_get_sort_column_id(pSortable, &sort_column_id, nullptr))
            return -1;
        return get_view_col(sort_column_id);
    }

    virtual void set_sort_column(int nColumn) override
    {
        GtkSortType eSortType;
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
        gtk_tree_sortable_get_sort_column_id(pSortable, nullptr, &eSortType);
        gtk_tree_sortable_set_sort_column_id(pSortable, get_model_col(nColumn), eSortType);
    }

    virtual int n_children() const override
    {
        return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_pTreeStore), nullptr);
    }

    virtual void select(int pos) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen");
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
        GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
        gtk_tree_view_set_cursor(m_pTreeView, path, nullptr, false);
        gtk_tree_path_free(path);
    }

    virtual void scroll_to_row(int pos) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen");
        disable_notify_events();
        GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
        gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual void unselect(int pos) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen");
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

    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
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
    }

    virtual void visible_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        GtkTreePath* start_path;
        GtkTreePath* end_path;

        if (gtk_tree_view_get_visible_range(m_pTreeView, &start_path, &end_path))
        {
            GtkInstanceTreeIter aGtkIter(nullptr);
            GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
            gtk_tree_model_get_iter(pModel, &aGtkIter.iter, start_path);

            do
            {
                if (func(aGtkIter))
                    break;
                GtkTreePath* path = gtk_tree_model_get_path(pModel, &aGtkIter.iter);
                bool bContinue = gtk_tree_path_compare(path, end_path) != 0;
                gtk_tree_path_free(path);
                if (!bContinue)
                    break;
                if (!iter_next(aGtkIter))
                    break;
            } while(true);

            gtk_tree_path_free(start_path);
            gtk_tree_path_free(end_path);
        }
    }

    virtual void connect_visible_range_changed(const Link<weld::TreeView&, void>& rLink) override
    {
        weld::TreeView::connect_visible_range_changed(rLink);
        GtkAdjustment* pVAdjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(m_pTreeView));
        m_nVAdjustmentChangedSignalId = g_signal_connect(pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustmentChanged), this);
    }

    virtual bool is_selected(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        return gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(m_pTreeView), const_cast<GtkTreeIter*>(&rGtkIter.iter));
    }

    virtual OUString get_text(int pos, int col) const override
    {
        if (col == -1)
            return get(pos, m_nTextCol);
        return get(pos, get_model_col(col));
    }

    virtual void set_text(int pos, const OUString& rText, int col) override
    {
        if (col == -1)
            col = m_nTextCol;
        else
            col = get_model_col(col);
        set(pos, col, rText);
    }

    virtual bool get_toggle(int pos, int col) const override
    {
        return get_bool(pos, get_model_col(col));
    }

    virtual void set_toggle(int pos, bool bOn, int col) override
    {
        col = get_model_col(col);
        // checkbuttons are invisible until toggled on or off
        set(pos, m_aToggleVisMap[col], true);
        set(pos, col, bOn);
    }

    using GtkInstanceWidget::set_sensitive;

    virtual void set_sensitive(int pos, bool bSensitive, int col) override
    {
        if (col == -1)
            col = m_nTextCol;
        else
            col = get_model_col(col);
        col += m_nIdCol + 1; // skip over id column
        col += m_aToggleVisMap.size(); // skip over toggle columns
        set(pos, col, bSensitive);
    }

    void set_image(const GtkTreeIter& iter, int col, GdkPixbuf* pixbuf)
    {
        gtk_tree_store_set(m_pTreeStore, const_cast<GtkTreeIter*>(&iter), col, pixbuf, -1);
        if (pixbuf)
            g_object_unref(pixbuf);
    }

    void set_image(int pos, GdkPixbuf* pixbuf, int col)
    {
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
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
        if (col == -1)
            col = m_nExpanderImageCol;
        else
            col = get_model_col(col);
        set_image(rGtkIter.iter, col, getPixbuf(rImage));
    }

    virtual void set_image(const weld::TreeIter& rIter, const OUString& rImage, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        if (col == -1)
            col = m_nExpanderImageCol;
        else
            col = get_model_col(col);
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

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));

        gint depth;
        gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
        int nRet = indices[depth-1];

        gtk_tree_path_free(path);

        return nRet;
    }

    // by copy and delete of old copy
    void move_subtree(GtkTreeIter& rFromIter, GtkTreeIter* pGtkParentIter, int nIndexInNewParent)
    {
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);

        int nCols = gtk_tree_model_get_n_columns(pModel);
        GValue value;

        GtkTreeIter toiter;
        gtk_tree_store_insert(m_pTreeStore, &toiter, pGtkParentIter, nIndexInNewParent);

        for (int i = 0; i < nCols; ++i)
        {
            memset(&value,  0, sizeof(GValue));
            gtk_tree_model_get_value(pModel, &rFromIter, i, &value);
            gtk_tree_store_set_value(m_pTreeStore, &toiter, i, &value);
            g_value_unset(&value);
        }

        GtkTreeIter tmpfromiter;
        if (gtk_tree_model_iter_children(pModel, &tmpfromiter, &rFromIter))
        {
            int j = 0;
            do
            {
                move_subtree(tmpfromiter, &toiter, j++);
            } while (gtk_tree_model_iter_next(pModel, &tmpfromiter));
        }

        gtk_tree_store_remove(m_pTreeStore, &rFromIter);
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
            GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
            gtk_tree_model_get_iter(pModel, &pGtkIter->iter, path);
        }
        return path != nullptr;
    }

    virtual void set_cursor(const weld::TreeIter& rIter) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        gtk_tree_view_set_cursor(m_pTreeView, path, nullptr, false);
        gtk_tree_path_free(path);
    }

    virtual bool get_iter_first(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        return gtk_tree_model_get_iter_first(pModel, &rGtkIter.iter);
    }

    virtual bool iter_next_sibling(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        return gtk_tree_model_iter_next(pModel, &rGtkIter.iter);
    }

    virtual bool iter_next(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter iter = rGtkIter.iter;
        if (iter_children(rGtkIter))
            return true;
        GtkTreeIter tmp = iter;
        if (gtk_tree_model_iter_next(pModel, &tmp))
        {
            rGtkIter.iter = tmp;
            return true;
        }
        if (!gtk_tree_model_iter_parent(pModel, &tmp, &iter))
            return false;
        tmp = iter;
        if (gtk_tree_model_iter_next(pModel, &tmp))
        {
            rGtkIter.iter = tmp;
            return true;
        }
        return false;
    }

    virtual bool iter_children(weld::TreeIter& rIter) const override
    {
        GtkInstanceTreeIter& rGtkIter = static_cast<GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter tmp;
        gboolean ret = gtk_tree_model_iter_children(pModel, &tmp, &rGtkIter.iter);
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
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreeIter tmp;
        auto ret = gtk_tree_model_iter_parent(pModel, &tmp, &rGtkIter.iter);
        rGtkIter.iter = tmp;
        return ret;
    }

    virtual void remove(const weld::TreeIter& rIter) override
    {
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        gtk_tree_store_remove(m_pTreeStore, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        enable_notify_events();
    }

    virtual void select(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen");
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        gtk_tree_selection_select_iter(gtk_tree_view_get_selection(m_pTreeView), const_cast<GtkTreeIter*>(&rGtkIter.iter));
        enable_notify_events();
    }

    virtual void scroll_to_row(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen");
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        gtk_tree_view_scroll_to_cell(m_pTreeView, path, nullptr, false, 0, 0);
        gtk_tree_path_free(path);
        enable_notify_events();
    }

    virtual void unselect(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't select when frozen");
        disable_notify_events();
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        gtk_tree_selection_unselect_iter(gtk_tree_view_get_selection(m_pTreeView), const_cast<GtkTreeIter*>(&rGtkIter.iter));
        enable_notify_events();
    }

    virtual int get_iter_depth(const weld::TreeIter& rIter) const override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
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
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        bool ret = gtk_tree_view_row_expanded(m_pTreeView, path);
        gtk_tree_path_free(path);
        return ret;
    }

    virtual void expand_row(const weld::TreeIter& rIter) override
    {
        assert(gtk_tree_view_get_model(m_pTreeView) && "don't expand when frozen");

        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
        if (!gtk_tree_view_row_expanded(m_pTreeView, path))
            gtk_tree_view_expand_to_path(m_pTreeView, path);
        gtk_tree_path_free(path);

    }

    virtual void collapse_row(const weld::TreeIter& rIter) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        GtkTreePath* path = gtk_tree_model_get_path(pModel, const_cast<GtkTreeIter*>(&rGtkIter.iter));
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
            col = get_model_col(col);
        return get(rGtkIter.iter, col);
    }

    virtual void set_text(const weld::TreeIter& rIter, const OUString& rText, int col) override
    {
        const GtkInstanceTreeIter& rGtkIter = static_cast<const GtkInstanceTreeIter&>(rIter);
        if (col == -1)
            col = m_nTextCol;
        else
            col = get_model_col(col);
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
        g_object_ref(m_pTreeStore);
        GtkInstanceContainer::freeze();
        gtk_tree_view_set_model(m_pTreeView, nullptr);
        if (m_xSorter)
        {
            int nSortColumn;
            GtkSortType eSortType;
            GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
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
            GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeStore);
            gtk_tree_sortable_set_sort_column_id(pSortable, m_aSavedSortColumns.back(), m_aSavedSortTypes.back());
            m_aSavedSortTypes.pop_back();
            m_aSavedSortColumns.pop_back();
        }
        gtk_tree_view_set_model(m_pTreeView, GTK_TREE_MODEL(m_pTreeStore));
        GtkInstanceContainer::thaw();
        g_object_unref(m_pTreeStore);
        enable_notify_events();
    }

    virtual int get_height_rows(int nRows) const override
    {
        gint nMaxRowHeight = 0;
        for (GList* pEntry = g_list_first(m_pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            for (GList* pRenderer = g_list_first(pRenderers); pRenderer; pRenderer = g_list_next(pRenderer))
            {
                GtkCellRenderer* pCellRenderer = GTK_CELL_RENDERER(pRenderer->data);
                gint nRowHeight;
                gtk_cell_renderer_get_preferred_height(pCellRenderer, GTK_WIDGET(m_pTreeView), nullptr, &nRowHeight);
                nMaxRowHeight = std::max(nMaxRowHeight, nRowHeight);
            }
            g_list_free(pRenderers);
        }

        gint nVerticalSeparator;
        gtk_widget_style_get(GTK_WIDGET(m_pTreeView), "vertical-separator", &nVerticalSeparator, nullptr);

        return (nMaxRowHeight * nRows) + (nVerticalSeparator * (nRows + 1));
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

    int starts_with(const OUString& rStr, int col, int nStartRow, bool bCaseSensitive)
    {
        return ::starts_with(GTK_TREE_MODEL(m_pTreeStore), rStr, get_model_col(col), nStartRow, bCaseSensitive);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
        g_signal_handler_block(m_pTreeView, m_nRowActivatedSignalId);

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        g_signal_handler_block(pModel, m_nRowDeletedSignalId);
        g_signal_handler_block(pModel, m_nRowInsertedSignalId);

        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();

        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        g_signal_handler_unblock(pModel, m_nRowDeletedSignalId);
        g_signal_handler_unblock(pModel, m_nRowInsertedSignalId);

        g_signal_handler_unblock(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_unblock(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
    }

    virtual ~GtkInstanceTreeView() override
    {
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pTreeStore);
        g_signal_handler_disconnect(pModel, m_nRowDeletedSignalId);
        g_signal_handler_disconnect(pModel, m_nRowInsertedSignalId);

        if (m_nVAdjustmentChangedSignalId)
        {
            GtkAdjustment* pVAdjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(m_pTreeView));
            g_signal_handler_disconnect(pVAdjustment, m_nVAdjustmentChangedSignalId);
        }

        g_signal_handler_disconnect(m_pTreeView, m_nTestExpandRowSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_disconnect(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);

        for (GList* pEntry = g_list_last(m_pColumns); pEntry; pEntry = g_list_previous(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            g_signal_handler_disconnect(pColumn, m_aColumnSignalIds.back());
            m_aColumnSignalIds.pop_back();
        }
        g_list_free(m_pColumns);
    }
};

IMPL_LINK_NOARG(GtkInstanceTreeView, async_signal_changed, void*, void)
{
    signal_changed();
}

class GtkInstanceSpinButton : public GtkInstanceEntry, public virtual weld::SpinButton
{
private:
    GtkSpinButton* m_pButton;
    gulong m_nValueChangedSignalId;
    gulong m_nOutputSignalId;
    gulong m_nInputSignalId;
    bool m_bFormatting;
    bool m_bBlockOutput;

    static void signalValueChanged(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        SolarMutexGuard aGuard;
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
    {
    }

    virtual int get_value() const override
    {
        return fromGtk(gtk_spin_button_get_value(m_pButton));
    }

    virtual void set_value(int value) override
    {
        disable_notify_events();
        gtk_spin_button_set_value(m_pButton, toGtk(value));
        enable_notify_events();
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
        gtk_entry_set_text(GTK_ENTRY(m_pButton), OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
        // tdf#122786 if we're just formatting a value, then we're done,
        // however if set_text has been called directly we want to update our
        // value from this new text, but don't want to reformat with that value
        if (!m_bFormatting)
        {
            m_bBlockOutput = true;
            gtk_spin_button_update(m_pButton);
            m_bBlockOutput = false;
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
    SvNumberFormatter* m_pFormatter;
    Color* m_pLastOutputColor;
    sal_uInt32 m_nFormatKey;
    gulong m_nValueChangedSignalId;
    gulong m_nOutputSignalId;
    gulong m_nInputSignalId;

    bool signal_output()
    {
        if (!m_pFormatter)
            return false;
        double dVal = get_value();
        OUString sNewText;
        if (m_pFormatter->IsTextFormat(m_nFormatKey))
        {
            // first convert the number as string in standard format
            OUString sTemp;
            m_pFormatter->GetOutputString(dVal, 0, sTemp, &m_pLastOutputColor);
            // then encode the string in the corresponding text format
            m_pFormatter->GetOutputString(sTemp, m_nFormatKey, sNewText, &m_pLastOutputColor);
        }
        else
        {
            m_pFormatter->GetInputLineString(dVal, m_nFormatKey, sNewText);
        }
        set_text(sNewText);
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
        if (!m_pFormatter)
            return 0;

        sal_uInt32 nFormatKey = m_nFormatKey; // IsNumberFormat changes the FormatKey!

        if (m_pFormatter->IsTextFormat(nFormatKey))
            // for detection of values like "1,1" in fields that are formatted as text
            nFormatKey = 0;

        OUString sText(get_text());

        // special treatment for percentage formatting
        if (m_pFormatter->GetType(m_nFormatKey) == SvNumFormatType::PERCENT)
        {
            // the language of our format
            LanguageType eLanguage = m_pFormatter->GetEntry(m_nFormatKey)->GetLanguage();
            // the default number format for this language
            sal_uLong nStandardNumericFormat = m_pFormatter->GetStandardFormat(SvNumFormatType::NUMBER, eLanguage);

            sal_uInt32 nTempFormat = nStandardNumericFormat;
            double dTemp;
            if (m_pFormatter->IsNumberFormat(sText, nTempFormat, dTemp) &&
                SvNumFormatType::NUMBER == m_pFormatter->GetType(nTempFormat))
                // the string is equivalent to a number formatted one (has no % sign) -> append it
                sText += "%";
            // (with this, a input of '3' becomes '3%', which then by the formatter is translated
            // into 0.03. Without this, the formatter would give us the double 3 for an input '3',
            // which equals 300 percent.
        }
        if (!m_pFormatter->IsNumberFormat(sText, nFormatKey, *value))
            return GTK_INPUT_ERROR;

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
        , m_pLastOutputColor(nullptr)
        , m_nFormatKey(0)
        , m_nValueChangedSignalId(g_signal_connect(pButton, "value-changed", G_CALLBACK(signalValueChanged), this))
        , m_nOutputSignalId(g_signal_connect(pButton, "output", G_CALLBACK(signalOutput), this))
        , m_nInputSignalId(g_signal_connect(pButton, "input", G_CALLBACK(signalInput), this))
    {
    }

    virtual double get_value() const override
    {
        return gtk_spin_button_get_value(m_pButton);
    }

    virtual void set_value(double value) override
    {
        disable_notify_events();
        gtk_spin_button_set_value(m_pButton, value);
        enable_notify_events();
    }

    virtual void set_range(double min, double max) override
    {
        disable_notify_events();
        gtk_spin_button_set_range(m_pButton, min, max);
        enable_notify_events();
    }

    virtual void get_range(double& min, double& max) const override
    {
        gtk_spin_button_get_range(m_pButton, &min, &max);
    }

    virtual void set_formatter(SvNumberFormatter* pFormatter) override
    {
        m_pFormatter = pFormatter;

        // calc the default format key from the Office's UI locale
        if (m_pFormatter)
        {
            // get the Office's locale and translate
            LanguageType eSysLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType( false);
            // get the standard numeric format for this language
            m_nFormatKey = m_pFormatter->GetStandardFormat( SvNumFormatType::NUMBER, eSysLanguage );
        }
        else
            m_nFormatKey = 0;
        signal_output();
    }

    virtual sal_Int32 get_format_key() const override
    {
        return m_nFormatKey;
    }

    virtual void set_format_key(sal_Int32 nFormatKey) override
    {
        m_nFormatKey = nFormatKey;
    }

    virtual ~GtkInstanceFormattedSpinButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nInputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nOutputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nValueChangedSignalId);
    }
};

class GtkInstanceLabel : public GtkInstanceWidget, public virtual weld::Label
{
private:
    GtkLabel* m_pLabel;
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
};

class GtkInstanceTextView : public GtkInstanceContainer, public virtual weld::TextView
{
private:
    GtkTextView* m_pTextView;
    GtkTextBuffer* m_pTextBuffer;
    GtkAdjustment* m_pVAdjustment;
    gulong m_nChangedSignalId;
    gulong m_nVAdjustChangedSignalId;

    static void signalChanged(GtkTextView*, gpointer widget)
    {
        GtkInstanceTextView* pThis = static_cast<GtkInstanceTextView*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_changed();
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
        , m_nChangedSignalId(g_signal_connect(m_pTextBuffer, "changed", G_CALLBACK(signalChanged), this))
        , m_nVAdjustChangedSignalId(g_signal_connect(m_pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustValueChanged), this))
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
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        OString sText(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gtk_text_buffer_set_text(pBuffer, sText.getStr(), sText.getLength());
    }

    virtual OUString get_text() const override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(pBuffer, &start, &end);
        char* pStr = gtk_text_buffer_get_text(pBuffer, &start, &end, true);
        OUString sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);
        return sRet;
    }

    virtual void replace_selection(const OUString& rText) override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        gtk_text_buffer_delete_selection(pBuffer, false, gtk_text_view_get_editable(m_pTextView));
        OString sText(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gtk_text_buffer_insert_at_cursor(pBuffer, sText.getStr(), sText.getLength());
    }

    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_selection_bounds(pBuffer, &start, &end);
        rStartPos = gtk_text_iter_get_offset(&start);
        rEndPos = gtk_text_iter_get_offset(&end);
        return rStartPos != rEndPos;
    }

    virtual void select_region(int nStartPos, int nEndPos) override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_iter_at_offset(pBuffer, &start, nStartPos);
        gtk_text_buffer_get_iter_at_offset(pBuffer, &end, nEndPos);
        gtk_text_buffer_select_range(pBuffer, &start, &end);
        GtkTextMark* mark = gtk_text_buffer_create_mark(pBuffer, "scroll", &end, true);
        gtk_text_view_scroll_mark_onscreen(m_pTextView, mark);
    }

    virtual void set_editable(bool bEditable) override
    {
        gtk_text_view_set_editable(m_pTextView, bEditable);
    }

    virtual void set_monospace(bool bMonospace) override
    {
        gtk_text_view_set_monospace(m_pTextView, bMonospace);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_block(m_pTextBuffer, m_nChangedSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pTextBuffer, m_nChangedSignalId);
        g_signal_handler_unblock(m_pVAdjustment, m_nVAdjustChangedSignalId);
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
        g_signal_handler_disconnect(m_pVAdjustment, m_nVAdjustChangedSignalId);
        g_signal_handler_disconnect(m_pTextBuffer, m_nChangedSignalId);
    }
};

namespace
{
    AtkObject* (*default_drawing_area_get_accessible)(GtkWidget *widget);
}

class GtkInstanceDrawingArea : public GtkInstanceWidget, public virtual weld::DrawingArea
{
private:
    GtkDrawingArea* m_pDrawingArea;
    a11yref m_xAccessible;
    AtkObject *m_pAccessible;
    ScopedVclPtrInstance<VirtualDevice> m_xDevice;
    cairo_surface_t* m_pSurface;
    gulong m_nDrawSignalId;
    gulong m_nStyleUpdatedSignalId;
    gulong m_nQueryTooltip;
    gulong m_nPopupMenu;

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
        if (!gdk_cairo_get_clip_rectangle(cr, &rect))
            return;
        tools::Rectangle aRect(Point(rect.x, rect.y), Size(rect.width, rect.height));
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
        m_xDevice->SetOutputSizePixel(Size(nWidth, nHeight));
        m_pSurface = get_underlying_cairo_surface(*m_xDevice);
        GtkInstanceWidget::signal_size_allocate(nWidth, nHeight);
    }
    static void signalStyleUpdated(GtkWidget*, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_style_updated();
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
        if (AllSettings::GetLayoutRTL())
            aGdkHelpArea.x = gtk_widget_get_allocated_width(pGtkWidget) - aGdkHelpArea.width - 1 - aGdkHelpArea.x;
        gtk_tooltip_set_tip_area(tooltip, &aGdkHelpArea);
        return true;
    }
    virtual bool signal_popup_menu(const Point& rPos) override
    {
        return m_aPopupMenuHdl.Call(rPos);
    }
    static gboolean signalPopupMenu(GtkWidget* pWidget, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        SolarMutexGuard aGuard;
        //center it when we don't know where else to use
        Point aPos(gtk_widget_get_allocated_width(pWidget) / 2,
                   gtk_widget_get_allocated_height(pWidget) / 2);
        return pThis->signal_popup_menu(aPos);
    }
public:
    GtkInstanceDrawingArea(GtkDrawingArea* pDrawingArea, GtkInstanceBuilder* pBuilder, const a11yref& rA11y, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pDrawingArea), pBuilder, bTakeOwnership)
        , m_pDrawingArea(pDrawingArea)
        , m_xAccessible(rA11y)
        , m_pAccessible(nullptr)
        , m_xDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT)
        , m_pSurface(nullptr)
        , m_nDrawSignalId(g_signal_connect(m_pDrawingArea, "draw", G_CALLBACK(signalDraw), this))
        , m_nStyleUpdatedSignalId(g_signal_connect(m_pDrawingArea,"style-updated", G_CALLBACK(signalStyleUpdated), this))
        , m_nQueryTooltip(g_signal_connect(m_pDrawingArea, "query-tooltip", G_CALLBACK(signalQueryTooltip), this))
        , m_nPopupMenu(g_signal_connect(m_pDrawingArea, "popup-menu", G_CALLBACK(signalPopupMenu), this))
    {
        gtk_widget_set_has_tooltip(m_pWidget, true);
        g_object_set_data(G_OBJECT(m_pDrawingArea), "g-lo-GtkInstanceDrawingArea", this);
        m_xDevice->EnableRTL(get_direction());
    }

    AtkObject* GetAtkObject(AtkObject* pDefaultAccessible)
    {
        if (!m_pAccessible && m_xAccessible.is())
        {
            GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
            m_pAccessible = atk_object_wrapper_new(m_xAccessible, gtk_widget_get_accessible(pParent), pDefaultAccessible);
            g_object_ref(m_pAccessible);
        }
        return m_pAccessible;
    }

    virtual void set_direction(bool bRTL) override
    {
        GtkInstanceWidget::set_direction(bRTL);
        m_xDevice->EnableRTL(bRTL);
    }

    virtual void queue_draw() override
    {
        gtk_widget_queue_draw(GTK_WIDGET(m_pDrawingArea));
    }

    virtual void queue_draw_area(int x, int y, int width, int height) override
    {
        gtk_widget_queue_draw_area(GTK_WIDGET(m_pDrawingArea), x, y, width, height);
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

    virtual ~GtkInstanceDrawingArea() override
    {
        g_object_steal_data(G_OBJECT(m_pDrawingArea), "g-lo-GtkInstanceDrawingArea");
        if (m_pAccessible)
            g_object_unref(m_pAccessible);
        css::uno::Reference<css::lang::XComponent> xComp(m_xAccessible, css::uno::UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
        g_signal_handler_disconnect(m_pDrawingArea, m_nPopupMenu);
        g_signal_handler_disconnect(m_pDrawingArea, m_nQueryTooltip);
        g_signal_handler_disconnect(m_pDrawingArea, m_nStyleUpdatedSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nDrawSignalId);
    }

    virtual OutputDevice& get_ref_device() override
    {
        return *m_xDevice;
    }
};

class GtkInstanceComboBox : public GtkInstanceContainer, public vcl::ISearchableStringList, public virtual weld::ComboBox
{
private:
    GtkComboBox* m_pComboBox;
    GtkTreeModel* m_pTreeModel;
    GtkCellRenderer* m_pTextRenderer;
    GtkMenu* m_pMenu;
    GtkWidget* m_pToggleButton;
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    vcl::QuickSelectionEngine m_aQuickSelectionEngine;
    std::vector<int> m_aSeparatorRows;
    gboolean m_bPopupActive;
    bool m_bAutoComplete;
    bool m_bAutoCompleteCaseSensitive;
    gulong m_nToggleFocusInSignalId;
    gulong m_nToggleFocusOutSignalId;
    gulong m_nChangedSignalId;
    gulong m_nPopupShownSignalId;
    gulong m_nKeyPressEventSignalId;
    gulong m_nEntryInsertTextSignalId;
    gulong m_nEntryActivateSignalId;
    gulong m_nEntryFocusOutSignalId;
    guint m_nAutoCompleteIdleId;

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

        if (!m_bAutoCompleteCaseSensitive)
        {
            // Try match case insensitive from current position
            nPos = starts_with(m_pTreeModel, aStartText, 0, nStart, false);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case insensitive, but from start
                nPos = starts_with(m_pTreeModel, aStartText, 0, 0, false);
            }
        }

        if (nPos == -1)
        {
            // Try match case sensitive from current position
            nPos = starts_with(m_pTreeModel, aStartText, 0, nStart, true);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case sensitive, but from start
                nPos = starts_with(m_pTreeModel, aStartText, 0, 0, true);
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
                g_signal_handlers_block_by_func(pEntry, gpointer(signalEntryInsertText), this);
                gtk_editable_insert_text(GTK_EDITABLE(pEntry), sFinalText.getStr(), sFinalText.getLength(), position);
                g_signal_handlers_unblock_by_func(pEntry, gpointer(signalEntryInsertText), this);
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
        pThis->signal_changed();
    }

    static void signalPopupToggled(GtkComboBox*, GParamSpec*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_popup_toggled();
    }

    virtual void signal_popup_toggled() override
    {
        m_aQuickSelectionEngine.Reset();
        gboolean bIsShown(false);
        g_object_get(m_pComboBox, "popup-shown", &bIsShown, nullptr);
        if (m_bPopupActive != bIsShown)
        {
            m_bPopupActive = bIsShown;
            ComboBox::signal_popup_toggled();
            //restore focus to the entry view when the popup is gone, which
            //is what the vcl case does, to ease the transition a little
            gtk_widget_grab_focus(m_pWidget);
        }
    }

    static void signalEntryFocusOut(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        pThis->signal_entry_focus_out();
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
                g_signal_stop_emission_by_name(get_entry(), "activate");
        }
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

    int find(const OUString& rStr, int col) const
    {
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter_first(m_pTreeModel, &iter))
            return -1;

        OString aStr(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
        int nRet = 0;
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

    GtkEntry* get_entry()
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBox));
        if (!GTK_IS_ENTRY(pChild))
            return nullptr;
        return GTK_ENTRY(pChild);
    }

    bool separator_function(int nIndex)
    {
        return std::find(m_aSeparatorRows.begin(), m_aSeparatorRows.end(), nIndex) != m_aSeparatorRows.end();
    }

    static gboolean separatorFunction(GtkTreeModel* pTreeModel, GtkTreeIter* pIter, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        GtkTreePath* path = gtk_tree_model_get_path(pTreeModel, pIter);

        gint depth;
        gint* indices = gtk_tree_path_get_indices_with_depth(path, &depth);
        int nIndex = indices[depth-1];

        gtk_tree_path_free(path);
        return pThis->separator_function(nIndex);
    }

    // in the absence of a built-in solution for https://gitlab.gnome.org/GNOME/gtk/issues/310
    // a) support typeahead for the case where there is no entry widget, typing ahead
    // into the button itself will select via the vcl selection engine, a matching
    // entry
    static gboolean signalKeyPress(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceComboBox* pThis = static_cast<GtkInstanceComboBox*>(widget);
        return pThis->signal_key_press(pEvent);
    }

    bool signal_key_press(const GdkEventKey* pEvent)
    {
        KeyEvent aKEvt(GtkToVcl(*pEvent));

        vcl::KeyCode aKeyCode = aKEvt.GetKeyCode();

        bool bDone = false;

        switch (aKeyCode.GetCode())
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
                m_aQuickSelectionEngine.Reset();
                break;
            default:
                bDone = m_aQuickSelectionEngine.HandleKeyEvent(aKEvt);
                break;
        }

        return bDone;
    }

    vcl::StringEntryIdentifier typeahead_getEntry(int nPos, OUString& out_entryText) const
    {
        int nEntryCount(get_count());
        if (nPos >= nEntryCount)
            nPos = 0;
        out_entryText = get_text(nPos);

        // vcl::StringEntryIdentifier does not allow for 0 values, but our position is 0-based
        // => normalize
        return reinterpret_cast<vcl::StringEntryIdentifier>(nPos + 1);
    }

    static int typeahead_getEntryPos(vcl::StringEntryIdentifier entry)
    {
        // our pos is 0-based, but StringEntryIdentifier does not allow for a NULL
        return reinterpret_cast<sal_Int64>(entry) - 1;
    }

    int get_selected_entry() const
    {
        if (m_bPopupActive && m_pMenu)
        {
            GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pMenu));
            auto nRet = g_list_index(pChildren, gtk_menu_shell_get_selected_item(GTK_MENU_SHELL(m_pMenu)));
            g_list_free(pChildren);
            return nRet;
        }
        else
            return get_active();
    }

    void set_selected_entry(int nSelect)
    {
        if (m_bPopupActive && m_pMenu)
        {
            GList* pChildren = gtk_container_get_children(GTK_CONTAINER(m_pMenu));
            gtk_menu_shell_select_item(GTK_MENU_SHELL(m_pMenu), GTK_WIDGET(g_list_nth_data(pChildren, nSelect)));
            g_list_free(pChildren);
        }
        else
            set_active(nSelect);
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
        int nCount = get_count();
        if (nSelect >= nCount)
            nSelect = nCount ? nCount-1 : -1;

        set_selected_entry(nSelect);
    }

    // b) support typeahead for the menu itself, typing into the menu will
    // select via the vcl selection engine, a matching entry. Clearly
    // this is cheating, brittle and not a long term solution.
    void install_menu_typeahead()
    {
        AtkObject* pAtkObj = gtk_combo_box_get_popup_accessible(m_pComboBox);
        if (!pAtkObj)
            return;
        if (!GTK_IS_ACCESSIBLE(pAtkObj))
            return;
        GtkWidget* pWidget = gtk_accessible_get_widget(GTK_ACCESSIBLE(pAtkObj));
        if (!pWidget)
            return;
        if (!GTK_IS_MENU(pWidget))
            return;
        m_pMenu = GTK_MENU(pWidget);

        guint nSignalId = g_signal_lookup("key-press-event", GTK_TYPE_MENU);
        gulong nOriginalMenuKeyPressEventId = g_signal_handler_find(m_pMenu, G_SIGNAL_MATCH_DATA, nSignalId, 0,
                                                                    nullptr, nullptr, m_pComboBox);

        g_signal_handler_block(m_pMenu, nOriginalMenuKeyPressEventId);
        g_signal_connect(m_pMenu, "key-press-event", G_CALLBACK(signalKeyPress), this);
    }

    static void find_toggle_button(GtkWidget *pWidget, gpointer user_data)
    {
        if (g_strcmp0(gtk_widget_get_name(pWidget), "GtkToggleButton") == 0)
        {
            GtkWidget **ppToggleButton = static_cast<GtkWidget**>(user_data);
            *ppToggleButton = pWidget;
        }
        else if (GTK_IS_CONTAINER(pWidget))
            gtk_container_forall(GTK_CONTAINER(pWidget), find_toggle_button, user_data);
    }

public:
    GtkInstanceComboBox(GtkComboBox* pComboBox, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pComboBox), pBuilder, bTakeOwnership)
        , m_pComboBox(pComboBox)
        , m_pTreeModel(gtk_combo_box_get_model(m_pComboBox))
        , m_pMenu(nullptr)
        , m_pToggleButton(nullptr)
        , m_aQuickSelectionEngine(*this)
        , m_bPopupActive(false)
        , m_bAutoComplete(false)
        , m_bAutoCompleteCaseSensitive(false)
        , m_nToggleFocusInSignalId(0)
        , m_nToggleFocusOutSignalId(0)
        , m_nChangedSignalId(g_signal_connect(m_pComboBox, "changed", G_CALLBACK(signalChanged), this))
        , m_nPopupShownSignalId(g_signal_connect(m_pComboBox, "notify::popup-shown", G_CALLBACK(signalPopupToggled), this))
        , m_nAutoCompleteIdleId(0)
    {
        GList* cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(m_pComboBox));
        if (!g_list_length(cells))
        {
            //Always use the same text column renderer layout
            m_pTextRenderer = gtk_cell_renderer_text_new();
            gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(m_pComboBox), m_pTextRenderer, true);
            gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(m_pComboBox), m_pTextRenderer, "text", 0, nullptr);
        }
        else
        {
            m_pTextRenderer = static_cast<GtkCellRenderer*>(cells->data);
            if (g_list_length(cells) == 2)
            {
                //The ComboBox is always going to show the column associated with
                //the entry when there is one, left to its own devices this image
                //column will be after it, but we want it before
                gtk_cell_layout_reorder(GTK_CELL_LAYOUT(m_pComboBox), m_pTextRenderer, 1);
            }
        }
        g_list_free(cells);

        if (GtkEntry* pEntry = get_entry())
        {
            m_bAutoComplete = true;
            m_nEntryInsertTextSignalId = g_signal_connect(pEntry, "insert-text", G_CALLBACK(signalEntryInsertText), this);
            m_nEntryActivateSignalId = g_signal_connect(pEntry, "activate", G_CALLBACK(signalEntryActivate), this);
            m_nEntryFocusOutSignalId = g_signal_connect(pEntry, "focus-out-event", G_CALLBACK(signalEntryFocusOut), this);
            m_nKeyPressEventSignalId = 0;
        }
        else
        {
            m_nEntryInsertTextSignalId = 0;
            m_nEntryActivateSignalId = 0;
            m_nEntryFocusOutSignalId = 0;
            m_nKeyPressEventSignalId = g_signal_connect(m_pWidget, "key-press-event", G_CALLBACK(signalKeyPress), this);
        }

        find_toggle_button(GTK_WIDGET(m_pComboBox), &m_pToggleButton);

        install_menu_typeahead();
    }

    virtual int get_active() const override
    {
        return gtk_combo_box_get_active(m_pComboBox);
    }

    virtual OUString get_active_id() const override
    {
        const gchar* pText = gtk_combo_box_get_active_id(m_pComboBox);
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        disable_notify_events();
        OString aId(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8));
        gtk_combo_box_set_active_id(m_pComboBox, aId.getStr());
        enable_notify_events();
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        // tweak the cell render to get a narrower size to stick
        GList* cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(m_pComboBox));
        GtkCellRenderer* cell = static_cast<GtkCellRenderer*>(cells->data);

        if (nWidth != -1)
        {
            // this bit isn't great, I really want to be able to ellipse the text in the comboboxtext itself and let
            // the popup menu render them in full, in the interim ellipse both of them
            g_object_set(G_OBJECT(m_pTextRenderer), "ellipsize", PANGO_ELLIPSIZE_MIDDLE, nullptr);

            // to find out how much of the width of the combobox belongs to the cell, set
            // the cell and widget to the min cell width and see what the difference is
            int min;
            gtk_cell_renderer_get_preferred_width(cell, m_pWidget, &min, nullptr);
            gtk_cell_renderer_set_fixed_size(cell, min, -1);
            gtk_widget_set_size_request(m_pWidget, min, -1);
            int nNonCellWidth = get_preferred_size().Width() - min;

            // now set the cell to the max width which it can be within the
            // requested widget width
            gtk_cell_renderer_set_fixed_size(cell, nWidth - nNonCellWidth, -1);
        }
        else
        {
            g_object_set(G_OBJECT(m_pTextRenderer), "ellipsize", PANGO_ELLIPSIZE_NONE, nullptr);
            gtk_cell_renderer_set_fixed_size(cell, -1, -1);
        }

        g_list_free(cells);

        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual void set_active(int pos) override
    {
        disable_notify_events();
        gtk_combo_box_set_active(m_pComboBox, pos);
        enable_notify_events();
    }

    virtual OUString get_active_text() const override
    {
        if (gtk_combo_box_get_has_entry(m_pComboBox))
        {
            GtkWidget *pEntry = gtk_bin_get_child(GTK_BIN(m_pComboBox));
            const gchar* pText = gtk_entry_get_text(GTK_ENTRY(pEntry));
            return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        }

        GtkTreeIter iter;
        if (!gtk_combo_box_get_active_iter(m_pComboBox, &iter))
            return OUString();

        gint col = gtk_combo_box_get_entry_text_column(m_pComboBox);
        gchar* pStr = nullptr;
        gtk_tree_model_get(m_pTreeModel, &iter, col, &pStr, -1);
        OUString sRet(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);

        return sRet;
    }

    virtual OUString get_text(int pos) const override
    {
        return get(pos, 0);
    }

    virtual OUString get_id(int pos) const override
    {
        gint id_column = gtk_combo_box_get_id_column(m_pComboBox);
        return get(pos, id_column);
    }

    virtual void set_id(int pos, const OUString& rId) override
    {
        gint id_column = gtk_combo_box_get_id_column(m_pComboBox);
        set(pos, id_column, rId);
    }

    // https://gitlab.gnome.org/GNOME/gtk/issues/94
    // when a super tall combobox menu is activated, and the selected entry is sufficiently
    // far down the list, then the menu doesn't appear under wayland
    void bodge_wayland_menu_not_appearing()
    {
        if (get_frozen())
            return;
        if (has_entry())
            return;
#if defined(GDK_WINDOWING_WAYLAND)
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        if (DLSYM_GDK_IS_WAYLAND_DISPLAY(pDisplay))
        {
            gtk_combo_box_set_wrap_width(m_pComboBox, get_count() > 30 ? 1 : 0);
        }
#endif
    }

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting) override
    {
        freeze();
        if (!bKeepExisting)
            clear();
        GtkTreeIter iter;
        for (const auto& rItem : rItems)
        {
            insert_row(GTK_LIST_STORE(m_pTreeModel), iter, -1, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
                       rItem.sString, rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
        }
        thaw();
    }

    virtual void remove(int pos) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(m_pTreeModel, &iter, nullptr, pos);
        gtk_list_store_remove(GTK_LIST_STORE(m_pTreeModel), &iter);
        m_aSeparatorRows.erase(std::remove(m_aSeparatorRows.begin(), m_aSeparatorRows.end(), pos), m_aSeparatorRows.end());
        enable_notify_events();
        bodge_wayland_menu_not_appearing();
    }

    virtual void insert(int pos, const OUString& rText, const OUString* pId, const OUString* pIconName, VirtualDevice* pImageSurface) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        insert_row(GTK_LIST_STORE(m_pTreeModel), iter, pos, pId, rText, pIconName, pImageSurface);
        enable_notify_events();
        bodge_wayland_menu_not_appearing();
    }

    virtual void insert_separator(int pos) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        pos = pos == -1 ? get_count() : pos;
        m_aSeparatorRows.push_back(pos);
        if (!gtk_combo_box_get_row_separator_func(m_pComboBox))
            gtk_combo_box_set_row_separator_func(m_pComboBox, separatorFunction, this, nullptr);
        insert_row(GTK_LIST_STORE(m_pTreeModel), iter, pos, nullptr, "", nullptr, nullptr);
        enable_notify_events();
        bodge_wayland_menu_not_appearing();
    }

    virtual int get_count() const override
    {
        return gtk_tree_model_iter_n_children(m_pTreeModel, nullptr);
    }

    virtual int find_text(const OUString& rStr) const override
    {
        return find(rStr, 0);
    }

    virtual int find_id(const OUString& rId) const override
    {
        gint id_column = gtk_combo_box_get_id_column(m_pComboBox);
        return find(rId, id_column);
    }

    virtual void clear() override
    {
        disable_notify_events();
        gtk_list_store_clear(GTK_LIST_STORE(m_pTreeModel));
        m_aSeparatorRows.clear();
        gtk_combo_box_set_row_separator_func(m_pComboBox, nullptr, nullptr, nullptr);
        enable_notify_events();
        bodge_wayland_menu_not_appearing();
    }

    virtual void make_sorted() override
    {
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetUILanguageTag().getLocale()));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(m_pTreeModel);
        gtk_tree_sortable_set_sort_column_id(pSortable, 0, GTK_SORT_ASCENDING);
        gtk_tree_sortable_set_sort_func(pSortable, 0, sort_func, m_xSorter.get(), nullptr);
    }

    virtual bool has_entry() const override
    {
        return gtk_combo_box_get_has_entry(m_pComboBox);
    }

    virtual void set_entry_error(bool bError) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBox));
        assert(GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        if (bError)
            gtk_entry_set_icon_from_icon_name(pEntry, GTK_ENTRY_ICON_SECONDARY, "dialog-error");
        else
            gtk_entry_set_icon_from_icon_name(pEntry, GTK_ENTRY_ICON_SECONDARY, nullptr);
    }

    virtual void set_entry_text(const OUString& rText) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBox));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        disable_notify_events();
        gtk_entry_set_text(pEntry, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
        enable_notify_events();
    }

    virtual void set_entry_width_chars(int nChars) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBox));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        disable_notify_events();
        gtk_entry_set_width_chars(pEntry, nChars);
        gtk_entry_set_max_width_chars(pEntry, nChars);
        enable_notify_events();
    }

    virtual void set_entry_max_length(int nChars) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBox));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        disable_notify_events();
        gtk_entry_set_max_length(pEntry, nChars);
        enable_notify_events();
    }

    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBox));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        disable_notify_events();
        gtk_editable_select_region(GTK_EDITABLE(pEntry), nStartPos, nEndPos);
        enable_notify_events();
    }

    virtual bool get_entry_selection_bounds(int& rStartPos, int &rEndPos) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBox));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        return gtk_editable_get_selection_bounds(GTK_EDITABLE(pEntry), &rStartPos, &rEndPos);
    }

    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive) override
    {
        m_bAutoComplete = bEnable;
        m_bAutoCompleteCaseSensitive = bCaseSensitive;
    }

    virtual void disable_notify_events() override
    {
        if (GtkEntry* pEntry = get_entry())
        {
            g_signal_handler_block(pEntry, m_nEntryInsertTextSignalId);
            g_signal_handler_block(pEntry, m_nEntryActivateSignalId);
            g_signal_handler_block(pEntry, m_nEntryFocusOutSignalId);
        }
        else
            g_signal_handler_block(m_pComboBox, m_nKeyPressEventSignalId);
        if (m_nToggleFocusInSignalId)
            g_signal_handler_block(m_pToggleButton, m_nToggleFocusInSignalId);
        if (m_nToggleFocusOutSignalId)
            g_signal_handler_block(m_pToggleButton, m_nToggleFocusOutSignalId);
        g_signal_handler_block(m_pComboBox, m_nChangedSignalId);
        g_signal_handler_block(m_pComboBox, m_nPopupShownSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pComboBox, m_nPopupShownSignalId);
        g_signal_handler_unblock(m_pComboBox, m_nChangedSignalId);
        if (m_nToggleFocusInSignalId)
            g_signal_handler_unblock(m_pToggleButton, m_nToggleFocusInSignalId);
        if (m_nToggleFocusOutSignalId)
            g_signal_handler_unblock(m_pToggleButton, m_nToggleFocusOutSignalId);
        if (GtkEntry* pEntry = get_entry())
        {
            g_signal_handler_unblock(pEntry, m_nEntryActivateSignalId);
            g_signal_handler_unblock(pEntry, m_nEntryFocusOutSignalId);
            g_signal_handler_unblock(pEntry, m_nEntryInsertTextSignalId);
        }
        else
            g_signal_handler_unblock(m_pComboBox, m_nKeyPressEventSignalId);
    }

    virtual void freeze() override
    {
        disable_notify_events();
        g_object_ref(m_pTreeModel);
        GtkInstanceContainer::freeze();
        gtk_combo_box_set_model(m_pComboBox, nullptr);
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
            gtk_tree_sortable_set_sort_column_id(pSortable, 0, GTK_SORT_ASCENDING);
        }
        gtk_combo_box_set_model(m_pComboBox, m_pTreeModel);
        GtkInstanceContainer::thaw();
        g_object_unref(m_pTreeModel);
        enable_notify_events();

        bodge_wayland_menu_not_appearing();
    }

    virtual bool get_popup_shown() const override
    {
        return m_bPopupActive;
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        m_nToggleFocusInSignalId = g_signal_connect(m_pToggleButton, "focus-in-event", G_CALLBACK(signalFocusIn), this);
        weld::Widget::connect_focus_in(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
        m_nToggleFocusOutSignalId = g_signal_connect(m_pToggleButton, "focus-out-event", G_CALLBACK(signalFocusOut), this);
        weld::Widget::connect_focus_out(rLink);
    }

    virtual ~GtkInstanceComboBox() override
    {
        if (m_nAutoCompleteIdleId)
            g_source_remove(m_nAutoCompleteIdleId);
        if (GtkEntry* pEntry = get_entry())
        {
            g_signal_handler_disconnect(pEntry, m_nEntryInsertTextSignalId);
            g_signal_handler_disconnect(pEntry, m_nEntryActivateSignalId);
            g_signal_handler_disconnect(pEntry, m_nEntryFocusOutSignalId);
        }
        else
            g_signal_handler_disconnect(m_pComboBox, m_nKeyPressEventSignalId);
        if (m_nToggleFocusInSignalId)
            g_signal_handler_disconnect(m_pToggleButton, m_nToggleFocusInSignalId);
        if (m_nToggleFocusOutSignalId)
            g_signal_handler_disconnect(m_pToggleButton, m_nToggleFocusOutSignalId);
        g_signal_handler_disconnect(m_pComboBox, m_nChangedSignalId);
        g_signal_handler_disconnect(m_pComboBox, m_nPopupShownSignalId);
    }
};

class GtkInstanceEntryTreeView : public GtkInstanceContainer, public virtual weld::EntryTreeView
{
private:
    GtkInstanceEntry* m_pEntry;
    GtkInstanceTreeView* m_pTreeView;
    gulong m_nKeyPressSignalId;
    gulong m_nEntryInsertTextSignalId;
    guint m_nAutoCompleteIdleId;
    bool m_bAutoCompleteCaseSensitive;

    gboolean signal_key_press(GdkEventKey* pEvent)
    {
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
            m_pEntry->fire_signal_changed();
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

        // Try match case insensitive from current position
        int nPos = m_pTreeView->starts_with(aStartText, 0, nStart, true);
        if (nPos == -1 && nStart != 0)
        {
            // Try match case insensitive, but from start
            nPos = m_pTreeView->starts_with(aStartText, 0, 0, true);
        }

        if (!m_bAutoCompleteCaseSensitive)
        {
            // Try match case insensitive from current position
            nPos = m_pTreeView->starts_with(aStartText, 0, nStart, false);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case insensitive, but from start
                nPos = m_pTreeView->starts_with(aStartText, 0, 0, false);
            }
        }

        if (nPos == -1)
        {
            // Try match case sensitive from current position
            nPos = m_pTreeView->starts_with(aStartText, 0, nStart, true);
            if (nPos == -1 && nStart != 0)
            {
                // Try match case sensitive, but from start
                nPos = m_pTreeView->starts_with(aStartText, 0, 0, true);
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
    {
        assert(m_pEntry);
        GtkWidget* pWidget = m_pEntry->getWidget();
        m_nKeyPressSignalId = g_signal_connect(pWidget, "key-press-event", G_CALLBACK(signalKeyPress), this);
        m_nEntryInsertTextSignalId = g_signal_connect(pWidget, "insert-text", G_CALLBACK(signalEntryInsertText), this);
    }

    virtual void insert_separator(int /*pos*/) override
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

    virtual ~GtkInstanceEntryTreeView() override
    {
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

    static void signalExpanded(GtkExpander* pExpander, GParamSpec*, gpointer widget)
    {
        GtkInstanceExpander* pThis = static_cast<GtkInstanceExpander*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_expanded();

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

public:
    GtkInstanceExpander(GtkExpander* pExpander, GtkInstanceBuilder* pBuilder, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pExpander), pBuilder, bTakeOwnership)
        , m_pExpander(pExpander)
        , m_nSignalId(g_signal_connect(m_pExpander, "notify::expanded", G_CALLBACK(signalExpanded), this))
    {
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
        g_signal_handler_disconnect(m_pExpander, m_nSignalId);
    }
};

namespace
{
    gboolean signalTooltipQuery(GtkWidget* pWidget, gint /*x*/, gint /*y*/,
                                         gboolean /*keyboard_mode*/, GtkTooltip *tooltip)
    {
        const ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->maHelpData.mbBalloonHelp)
        {
            /*This is how I would prefer things to be, only a few like this though*/
            AtkObject* pAtkObject = gtk_widget_get_accessible(pWidget);
            const char* pDesc = pAtkObject ? atk_object_get_description(pAtkObject) : nullptr;
            if (pDesc)
            {
                gtk_tooltip_set_text(tooltip, pDesc);
                return true;
            }

            /*So fallback to existing mechanism which needs help installed*/
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
        if (pDesc)
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

}

class GtkInstanceBuilder : public weld::Builder
{
private:
    ResHookProc m_pStringReplace;
    OUString m_sHelpRoot;
    OString m_aUtf8HelpRoot;
    OUString m_aIconTheme;
    OUString m_aUILang;
    GtkBuilder* m_pBuilder;
    GSList* m_pObjectList;
    GtkWidget* m_pParentWidget;
    gulong m_nNotifySignalId;
    std::vector<GtkButton*> m_aMnemonicButtons;
    std::vector<GtkLabel*> m_aMnemonicLabels;

    void postprocess_widget(GtkWidget* pWidget)
    {
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
                GdkPixbuf* pixbuf = load_icon_by_name(aIconName, m_aIconTheme, m_aUILang);
                if (pixbuf)
                {
                    gtk_image_set_from_pixbuf(pImage, pixbuf);
                    g_object_unref(pixbuf);
                }
            }
        }
        //set helpids
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pWidget));
        size_t nLen = pStr ? strlen(pStr) : 0;
        if (!nLen)
            return;
        OString sHelpId = m_aUtf8HelpRoot + OString(pStr, nLen);
        set_help_id(pWidget, sHelpId);
        //hook up for extended help
        const ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->maHelpData.mbBalloonHelp && !GTK_IS_DIALOG(pWidget))
        {
            gtk_widget_set_has_tooltip(pWidget, true);
            g_signal_connect(pWidget, "query-tooltip", G_CALLBACK(signalTooltipQuery), nullptr);
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
public:
    GtkInstanceBuilder(GtkWidget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
        : weld::Builder(rUIFile)
        , m_pStringReplace(Translate::GetReadStringHook())
        , m_sHelpRoot(rUIFile)
        , m_pParentWidget(pParent)
        , m_nNotifySignalId(0)
    {
        ensure_intercept_drawing_area_accessibility();

        sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
        if (nIdx != -1)
            m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
        m_sHelpRoot = m_sHelpRoot + OUString('/');
        m_aUtf8HelpRoot = OUStringToOString(m_sHelpRoot, RTL_TEXTENCODING_UTF8);
        m_aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        m_aUILang = Application::GetSettings().GetUILanguageTag().getBcp47();

        OUString aUri(rUIRoot + rUIFile);
        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aUri, aPath);
        m_pBuilder = gtk_builder_new();
        m_nNotifySignalId = g_signal_connect_data(G_OBJECT(m_pBuilder), "notify", G_CALLBACK(signalNotify), this, nullptr, G_CONNECT_AFTER);
        gtk_builder_add_from_file(m_pBuilder, OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr(), nullptr);

        m_pObjectList = gtk_builder_get_objects(m_pBuilder);
        g_slist_foreach(m_pObjectList, postprocess, this);

        GenerateMissingMnemonics();
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

    virtual ~GtkInstanceBuilder() override
    {
        g_slist_free(m_pObjectList);
        g_object_unref(m_pBuilder);
    }

    //ideally we would have/use weld::Container add and explicitly
    //call add when we want to do this, but in the vcl impl the
    //parent has to be set when the child is created, so for the
    //gtk impl emulate this by doing this implicitly at weld time
    void auto_add_parentless_widgets_to_container(GtkWidget* pWidget)
    {
        if (gtk_widget_get_toplevel(pWidget) == pWidget && !GTK_IS_POPOVER(pWidget))
            gtk_container_add(GTK_CONTAINER(m_pParentWidget), pWidget);
    }

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString &id, bool bTakeOwnership) override
    {
        GtkMessageDialog* pMessageDialog = GTK_MESSAGE_DIALOG(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pMessageDialog)
            return nullptr;
        gtk_window_set_transient_for(GTK_WINDOW(pMessageDialog), GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceMessageDialog>(pMessageDialog, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString &id, bool bTakeOwnership) override
    {
        GtkDialog* pDialog = GTK_DIALOG(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pDialog)
            return nullptr;
        if (m_pParentWidget)
            gtk_window_set_transient_for(GTK_WINDOW(pDialog), GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return std::make_unique<GtkInstanceDialog>(pDialog, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Window> weld_window(const OString &id, bool bTakeOwnership) override
    {
        GtkWindow* pWindow = GTK_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pWindow ? std::make_unique<GtkInstanceWindow>(pWindow, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Widget> weld_widget(const OString &id, bool bTakeOwnership) override
    {
        GtkWidget* pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pWidget)
            return nullptr;
        auto_add_parentless_widgets_to_container(pWidget);
        return std::make_unique<GtkInstanceWidget>(pWidget, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Container> weld_container(const OString &id, bool bTakeOwnership) override
    {
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pContainer)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pContainer));
        return std::make_unique<GtkInstanceContainer>(pContainer, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Frame> weld_frame(const OString &id, bool bTakeOwnership) override
    {
        GtkFrame* pFrame = GTK_FRAME(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pFrame)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pFrame));
        return std::make_unique<GtkInstanceFrame>(pFrame, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::ScrolledWindow> weld_scrolled_window(const OString &id, bool bTakeOwnership) override
    {
        GtkScrolledWindow* pScrolledWindow = GTK_SCROLLED_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pScrolledWindow)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pScrolledWindow));
        return std::make_unique<GtkInstanceScrolledWindow>(pScrolledWindow, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString &id, bool bTakeOwnership) override
    {
        GtkNotebook* pNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pNotebook)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pNotebook));
        return std::make_unique<GtkInstanceNotebook>(pNotebook, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Button> weld_button(const OString &id, bool bTakeOwnership) override
    {
        GtkButton* pButton = GTK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return std::make_unique<GtkInstanceButton>(pButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString &id, bool bTakeOwnership) override
    {
        GtkMenuButton* pButton = GTK_MENU_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return std::make_unique<GtkInstanceMenuButton>(pButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::LinkButton> weld_link_button(const OString &id, bool bTakeOwnership) override
    {
        GtkLinkButton* pButton = GTK_LINK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return std::make_unique<GtkInstanceLinkButton>(pButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OString &id, bool bTakeOwnership) override
    {
        GtkToggleButton* pToggleButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pToggleButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pToggleButton));
        return std::make_unique<GtkInstanceToggleButton>(pToggleButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString &id, bool bTakeOwnership) override
    {
        GtkRadioButton* pRadioButton = GTK_RADIO_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pRadioButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pRadioButton));
        return std::make_unique<GtkInstanceRadioButton>(pRadioButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString &id, bool bTakeOwnership) override
    {
        GtkCheckButton* pCheckButton = GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pCheckButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pCheckButton));
        return std::make_unique<GtkInstanceCheckButton>(pCheckButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Scale> weld_scale(const OString &id, bool bTakeOwnership) override
    {
        GtkScale* pScale = GTK_SCALE(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pScale)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pScale));
        return std::make_unique<GtkInstanceScale>(pScale, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OString &id, bool bTakeOwnership) override
    {
        GtkProgressBar* pProgressBar = GTK_PROGRESS_BAR(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pProgressBar)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pProgressBar));
        return std::make_unique<GtkInstanceProgressBar>(pProgressBar, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Spinner> weld_spinner(const OString &id, bool bTakeOwnership) override
    {
        GtkSpinner* pSpinner = GTK_SPINNER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pSpinner)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pSpinner));
        return std::make_unique<GtkInstanceSpinner>(pSpinner, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Image> weld_image(const OString &id, bool bTakeOwnership) override
    {
        GtkImage* pImage = GTK_IMAGE(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pImage)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pImage));
        return std::make_unique<GtkInstanceImage>(pImage, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Calendar> weld_calendar(const OString &id, bool bTakeOwnership) override
    {
        GtkCalendar* pCalendar = GTK_CALENDAR(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pCalendar)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pCalendar));
        return std::make_unique<GtkInstanceCalendar>(pCalendar, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Entry> weld_entry(const OString &id, bool bTakeOwnership) override
    {
        GtkEntry* pEntry = GTK_ENTRY(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pEntry)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pEntry));
        return std::make_unique<GtkInstanceEntry>(pEntry, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString &id, bool bTakeOwnership) override
    {
        GtkSpinButton* pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pSpinButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pSpinButton));
        return std::make_unique<GtkInstanceSpinButton>(pSpinButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::MetricSpinButton> weld_metric_spin_button(const OString& id, FieldUnit eUnit,
                                                                      bool bTakeOwnership) override
    {
        return std::make_unique<weld::MetricSpinButton>(weld_spin_button(id, bTakeOwnership), eUnit);
    }

    virtual std::unique_ptr<weld::FormattedSpinButton> weld_formatted_spin_button(const OString &id, bool bTakeOwnership) override
    {
        GtkSpinButton* pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pSpinButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pSpinButton));
        return std::make_unique<GtkInstanceFormattedSpinButton>(pSpinButton, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::TimeSpinButton> weld_time_spin_button(const OString& id, TimeFieldFormat eFormat,
                                                        bool bTakeOwnership) override
    {
        return std::make_unique<weld::TimeSpinButton>(weld_spin_button(id, bTakeOwnership), eFormat);
    }

    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString &id, bool bTakeOwnership) override
    {
        GtkComboBox* pComboBox = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pComboBox)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pComboBox));
        return std::make_unique<GtkInstanceComboBox>(pComboBox, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString &id, bool bTakeOwnership) override
    {
        GtkTreeView* pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pTreeView)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pTreeView));
        return std::make_unique<GtkInstanceTreeView>(pTreeView, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::EntryTreeView> weld_entry_tree_view(const OString& containerid, const OString& entryid, const OString& treeviewid, bool bTakeOwnership) override
    {
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, containerid.getStr()));
        if (!pContainer)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pContainer));
        return std::make_unique<GtkInstanceEntryTreeView>(pContainer, this, bTakeOwnership,
                                                          weld_entry(entryid, bTakeOwnership),
                                                          weld_tree_view(treeviewid, bTakeOwnership));
    }

    virtual std::unique_ptr<weld::Label> weld_label(const OString &id, bool bTakeOwnership) override
    {
        GtkLabel* pLabel = GTK_LABEL(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pLabel)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pLabel));
        return std::make_unique<GtkInstanceLabel>(pLabel, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString &id, bool bTakeOwnership) override
    {
        GtkTextView* pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pTextView)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pTextView));
        return std::make_unique<GtkInstanceTextView>(pTextView, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Expander> weld_expander(const OString &id, bool bTakeOwnership) override
    {
        GtkExpander* pExpander = GTK_EXPANDER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pExpander)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pExpander));
        return std::make_unique<GtkInstanceExpander>(pExpander, this, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::DrawingArea> weld_drawing_area(const OString &id, const a11yref& rA11y,
            FactoryFunction /*pUITestFactoryFunction*/, void* /*pUserData*/, bool bTakeOwnership) override
    {
        GtkDrawingArea* pDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pDrawingArea)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pDrawingArea));
        return std::make_unique<GtkInstanceDrawingArea>(pDrawingArea, this, rA11y, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::Menu> weld_menu(const OString &id, bool bTakeOwnership) override
    {
        GtkMenu* pMenu = GTK_MENU(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pMenu)
            return nullptr;
        return std::make_unique<GtkInstanceMenu>(pMenu, bTakeOwnership);
    }

    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override
    {
        return std::make_unique<GtkInstanceSizeGroup>();
    }
};

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
    if (pHelp)
        pHelp->Start(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), pSource);
}

//iterate upwards through the hierarchy from this widgets through its parents
//calling func with their helpid until func returns true or we run out of parents
void GtkInstanceWidget::help_hierarchy_foreach(const std::function<bool(const OString&)>& func)
{
    GtkWidget* pParent = m_pWidget;
    while ((pParent = gtk_widget_get_parent(pParent)))
    {
        // tdf#122355 before trying dialog help, check to see if there is a notebook
        // called tabcontrol, and try the help for the current page of that first
        if (m_pBuilder && GTK_IS_DIALOG(pParent))
        {
            std::unique_ptr<weld::Notebook> xNotebook(m_pBuilder->weld_notebook("tabcontrol", false));
            if (xNotebook)
            {
                if (GtkInstanceContainer* pPage = dynamic_cast<GtkInstanceContainer*>(xNotebook->get_page(xNotebook->get_current_page_ident())))
                {
                    bool bFinished = false;
                    GtkWidget* pContainer = pPage->getWidget();
                    GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pContainer));
                    GList* pChild = g_list_first(pChildren);
                    if (pChild)
                    {
                        GtkWidget* pPageWidget = static_cast<GtkWidget*>(pChild->data);
                        bFinished = func(::get_help_id(pPageWidget));
                    }
                    g_list_free(pChildren);
                    if (bFinished)
                        return;
                }
            }
        }
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
    return new GtkInstanceBuilder(pBuilderParent, rUIRoot, rUIFile);
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
        m_xFrameWeld.reset(new GtkInstanceWindow(GTK_WINDOW(getWindow()), nullptr, false));
    return m_xFrameWeld.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
