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
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/weld.hxx>

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

    return std::find_if(aAll.begin(), aAll.end(), DataFlavorEq(rFlavor)) != aAll.end();
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
                aStr = OUString(pText, rtl_str_getLength(pText), RTL_TEXTENCODING_UTF8);
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
    void ClipboardGetFunc(GtkClipboard */*clipboard*/, GtkSelectionData *selection_data,
                          guint info,
                          gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardGet(selection_data, info);
    }

    void ClipboardClearFunc(GtkClipboard */*clipboard*/, gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardClear();
    }

    void handle_owner_change(GtkClipboard *clipboard, GdkEvent */*event*/, gpointer user_data)
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
        css::uno::Sequence<css::datatransfer::DataFlavor> aFormats = xTrans->getTransferDataFlavors();
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
    , m_bActive(false)
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
    return Reference< XInterface >( static_cast<cppu::OWeakObject *>(new GtkDropTarget()) );
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
}

class GtkInstanceWidget : public virtual weld::Widget
{
protected:
    GtkWidget* m_pWidget;
private:
    bool m_bTakeOwnership;
    gulong m_nFocusInSignalId;
    gulong m_nFocusOutSignalId;

    static void signalFocusIn(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_in();
    }

    static void signalFocusOut(GtkWidget*, GdkEvent*, gpointer widget)
    {
        GtkInstanceWidget* pThis = static_cast<GtkInstanceWidget*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_focus_out();
    }

public:
    GtkInstanceWidget(GtkWidget* pWidget, bool bTakeOwnership)
        : m_pWidget(pWidget)
        , m_bTakeOwnership(bTakeOwnership)
        , m_nFocusInSignalId(0)
        , m_nFocusOutSignalId(0)
    {
    }

    virtual void set_sensitive(bool sensitive) override
    {
        gtk_widget_set_sensitive(m_pWidget, sensitive);
    }

    virtual bool get_sensitive() const override
    {
        return gtk_widget_get_sensitive(m_pWidget);
    }

    virtual void set_visible(bool visible) override
    {
        gtk_widget_set_visible(m_pWidget, visible);
    }

    virtual bool get_visible() const override
    {
        return gtk_widget_get_visible(m_pWidget);
    }

    virtual bool is_visible() const override
    {
        return gtk_widget_is_visible(m_pWidget);
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

    virtual void set_tooltip_text(const OUString& rTip) override
    {
        gtk_widget_set_tooltip_text(m_pWidget, OUStringToOString(rTip, RTL_TEXTENCODING_UTF8).getStr());
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

    virtual ~GtkInstanceWidget() override
    {
        if (m_nFocusInSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nFocusInSignalId);
        if (m_nFocusOutSignalId)
            g_signal_handler_disconnect(m_pWidget, m_nFocusOutSignalId);
        if (m_bTakeOwnership)
            gtk_widget_destroy(m_pWidget);
    }

    virtual void disable_notify_events()
    {
        if (m_nFocusInSignalId)
            g_signal_handler_block(m_pWidget, m_nFocusInSignalId);
        if (m_nFocusOutSignalId)
            g_signal_handler_block(m_pWidget, m_nFocusOutSignalId);
    }

    virtual void enable_notify_events()
    {
        if (m_nFocusOutSignalId)
            g_signal_handler_unblock(m_pWidget, m_nFocusOutSignalId);
        if (m_nFocusInSignalId)
            g_signal_handler_unblock(m_pWidget, m_nFocusInSignalId);
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

    void add_to_map(GtkMenuItem* pMenuItem)
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pMenuItem));
        OString id(pStr, pStr ? strlen(pStr) : 0);
        m_aMap[id] = pMenuItem;
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
        for (auto& a : m_aMap)
            g_signal_connect(a.second, "activate", G_CALLBACK(signalActivate), this);
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

class GtkInstanceMenu : public MenuHelper, public virtual weld::Menu
{
protected:
    OString m_sActivated;

private:
    virtual void signal_activate(GtkMenuItem* pItem) override
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pItem));
        m_sActivated = OString(pStr, pStr ? strlen(pStr) : 0);
    }

public:
    GtkInstanceMenu(GtkMenu* pMenu, bool bTakeOwnership)
        : MenuHelper(pMenu, bTakeOwnership)
    {
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
        GtkSizeGroupMode eGtkMode;
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
    GtkInstanceContainer(GtkContainer* pContainer, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pContainer), bTakeOwnership)
        , m_pContainer(pContainer)
    {
    }

    GtkContainer* getContainer() { return m_pContainer; }

    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override
    {
        GtkInstanceWidget* pGtkWidget = dynamic_cast<GtkInstanceWidget*>(pWidget);
        assert(pGtkWidget);
        GtkInstanceContainer* pNewGtkParent = dynamic_cast<GtkInstanceContainer*>(pNewParent);
        assert(pNewGtkParent);
        GtkWidget* pChild = pGtkWidget->getWidget();
        g_object_ref(pChild);
        gtk_container_remove(getContainer(), pChild);
        gtk_container_add(pNewGtkParent->getContainer(), pChild);
        g_object_unref(pChild);
    }
};

weld::Container* GtkInstanceWidget::weld_parent() const
{
    GtkWidget* pParent = gtk_widget_get_parent(m_pWidget);
    return pParent ? new GtkInstanceContainer(GTK_CONTAINER(pParent), false) : nullptr;
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
    GtkInstanceWindow(GtkWindow* pWindow, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pWindow), bTakeOwnership)
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
        gtk_widget_realize(m_pWidget);
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        GdkCursor *pCursor = bBusy ? gdk_cursor_new_from_name(pDisplay, "progress") : nullptr;
        gdk_window_set_cursor(gtk_widget_get_window(m_pWidget), pCursor);
        gdk_display_flush(pDisplay);
        if (pCursor)
            g_object_unref(pCursor);
    }

    virtual void resize_to_request() override
    {
        gtk_window_resize(m_pWindow, 1, 1);
    }

    virtual void window_move(int x, int y) override
    {
        gtk_window_move(m_pWindow, x, y);
    }

    virtual bool get_extents_relative_to(Window& rRelative, int& x, int &y, int& width, int &height) override
    {
        //this is sadly futile under wayland, so we can't tell where a dialog is in order to allow
        //the document underneath to auto-scroll to place content in a visible location
        gboolean ret = gtk_widget_translate_coordinates(dynamic_cast<GtkInstanceWindow&>(rRelative).getWidget(), m_pWidget, 0, 0, &x, &y);
        width = gtk_widget_get_allocated_width(m_pWidget);
        height = gtk_widget_get_allocated_height(m_pWidget);
        return ret;
    }

    virtual SystemEnvData get_system_data() const override
    {
        assert(false && "nothing should call this impl, yet anyway, if ever");
        return SystemEnvData();
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
        OString m_aType;
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

class GtkInstanceDialog : public GtkInstanceWindow, public virtual weld::Dialog
{
private:
    GtkDialog* m_pDialog;
    std::shared_ptr<weld::DialogController> m_xDialogController;
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
        m_xDialogController.reset();
    }
public:
    GtkInstanceDialog(GtkDialog* pDialog, bool bTakeOwnership)
        : GtkInstanceWindow(GTK_WINDOW(pDialog), bTakeOwnership)
        , m_pDialog(pDialog)
        , m_nCloseSignalId(g_signal_connect(m_pDialog, "close", G_CALLBACK(signalClose), this))
        , m_nResponseSignalId(0)
    {
    }

    virtual bool runAsync(std::shared_ptr<weld::DialogController> rDialogController, const std::function<void(sal_Int32)>& func) override
    {
        assert(!m_nResponseSignalId);

        m_xDialogController = rDialogController;
        m_aFunc = func;

        if (!gtk_widget_get_visible(m_pWidget))
        {
            sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(m_pDialog)));
            gtk_widget_show(m_pWidget);
        }

        m_nResponseSignalId = g_signal_connect(m_pDialog, "response", G_CALLBACK(signalAsyncResponse), this);

        return true;
    }

    bool has_click_handler(int nResponse);

    virtual int run() override
    {
        sort_native_button_order(GTK_BOX(gtk_dialog_get_action_area(m_pDialog)));
        int ret;
        GtkWindow* pParent = gtk_window_get_transient_for(GTK_WINDOW(m_pDialog));
        GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(pParent);
        vcl::Window* pFrameWindow = pFrame ? pFrame->GetWindow() : nullptr;
        while (true)
        {
            if (pFrameWindow)
                pFrameWindow->IncModalCount();
            ret = gtk_dialog_run(m_pDialog);
            if (pFrameWindow)
                pFrameWindow->DecModalCount();
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
        return new GtkInstanceContainer(GTK_CONTAINER(gtk_dialog_get_content_area(m_pDialog)), false);
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
    GtkInstanceMessageDialog(GtkMessageDialog* pMessageDialog, bool bTakeOwnership)
        : GtkInstanceDialog(GTK_DIALOG(pMessageDialog), bTakeOwnership)
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
        return new GtkInstanceContainer(GTK_CONTAINER(gtk_message_dialog_get_message_area(m_pMessageDialog)), false);
    }
};

class GtkInstanceFrame : public GtkInstanceContainer, public virtual weld::Frame
{
private:
    GtkFrame* m_pFrame;
public:
    GtkInstanceFrame(GtkFrame* pFrame, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pFrame), bTakeOwnership)
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

GType crippled_viewport_get_type();

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
//    o_class->finalize = gtk_tree_view_finalize;

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

VclPolicyType GtkToVcl(GtkPolicyType eType)
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

GtkPolicyType VclToGtk(VclPolicyType eType)
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

GtkMessageType VclToGtk(VclMessageType eType)
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

GtkButtonsType VclToGtk(VclButtonsType eType)
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

class GtkInstanceScrolledWindow : public GtkInstanceContainer, public virtual weld::ScrolledWindow
{
private:
    GtkScrolledWindow* m_pScrolledWindow;
    GtkWidget *m_pOrigViewport;
    GtkAdjustment* m_pVAdjustment;
    gulong m_nVAdjustChangedSignalId;

    static void signalVAdjustValueChanged(GtkAdjustment*, gpointer widget)
    {
        GtkInstanceScrolledWindow* pThis = static_cast<GtkInstanceScrolledWindow*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_vadjustment_changed();
    }

public:
    GtkInstanceScrolledWindow(GtkScrolledWindow* pScrolledWindow, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pScrolledWindow), bTakeOwnership)
        , m_pScrolledWindow(pScrolledWindow)
        , m_pOrigViewport(nullptr)
        , m_pVAdjustment(gtk_scrolled_window_get_vadjustment(m_pScrolledWindow))
        , m_nVAdjustChangedSignalId(g_signal_connect(m_pVAdjustment, "value-changed", G_CALLBACK(signalVAdjustValueChanged), this))
    {
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
        return gtk_widget_get_allocated_width(GTK_WIDGET(m_pScrolledWindow));
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pVAdjustment, m_nVAdjustChangedSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pVAdjustment, m_nVAdjustChangedSignalId);
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
    }
};

class GtkInstanceNotebook : public GtkInstanceContainer, public virtual weld::Notebook
{
private:
    GtkNotebook* m_pNotebook;
    gulong m_nSignalId;
    mutable std::vector<std::unique_ptr<GtkInstanceContainer>> m_aPages;

    static void signalSwitchPage(GtkNotebook*, GtkWidget*, guint nNewPage, gpointer widget)
    {
        GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_switch_page(nNewPage);
    }

    void signal_switch_page(guint nNewPage)
    {
        bool bAllow = !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
        if (!bAllow)
        {
            g_signal_stop_emission_by_name(m_pNotebook, "switch-page");
            OString sNewIdent(get_page_ident(nNewPage));
            m_aEnterPageHdl.Call(sNewIdent);
        }
    }

    OString get_page_ident(guint nPage) const
    {
        const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pNotebook, gtk_notebook_get_nth_page(m_pNotebook, nPage));
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
        return OString(pStr, pStr ? strlen(pStr) : 0);
    }

    gint get_page_number(const OString& rIdent) const
    {
        gint nPages = gtk_notebook_get_n_pages(m_pNotebook);
        for (gint i = 0; i < nPages; ++i)
        {
            const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pNotebook, gtk_notebook_get_nth_page(m_pNotebook, i));
            const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
            if (strcmp(pStr, rIdent.getStr()) == 0)
                return i;
        }
        return -1;
    }

public:
    GtkInstanceNotebook(GtkNotebook* pNotebook, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pNotebook), bTakeOwnership)
        , m_pNotebook(pNotebook)
        , m_nSignalId(g_signal_connect(pNotebook, "switch-page", G_CALLBACK(signalSwitchPage), this))
    {
    }

    virtual int get_current_page() const override
    {
        return gtk_notebook_get_current_page(m_pNotebook);
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
        GtkContainer* pChild = GTK_CONTAINER(gtk_notebook_get_nth_page(m_pNotebook, nPage));
        unsigned int nPageIndex = static_cast<unsigned int>(nPage);
        if (m_aPages.size() < nPageIndex + 1)
            m_aPages.resize(nPageIndex + 1);
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new GtkInstanceContainer(pChild, false));
        return m_aPages[nPageIndex].get();
    }

    virtual void set_current_page(int nPage) override
    {
        gtk_notebook_set_current_page(m_pNotebook, nPage);
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        gint nPage = get_page_number(rIdent);
        set_current_page(nPage);
    }

    virtual int get_n_pages() const override
    {
        return gtk_notebook_get_n_pages(m_pNotebook);
    }

    virtual OUString get_tab_label_text(const OString& rIdent) const override
    {
        gint nPage = get_page_number(rIdent);
        const gchar* pStr = gtk_notebook_get_tab_label_text(m_pNotebook, gtk_notebook_get_nth_page(m_pNotebook, nPage));
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pNotebook, m_nSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pNotebook, m_nSignalId);
    }

    virtual void remove_page(const OString& rIdent) override
    {
        disable_notify_events();
        gtk_notebook_remove_page(m_pNotebook, get_page_number(rIdent));
        enable_notify_events();
    }

    virtual ~GtkInstanceNotebook() override
    {
        g_signal_handler_disconnect(m_pNotebook, m_nSignalId);
    }
};

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
    GtkInstanceButton(GtkButton* pButton, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pButton), bTakeOwnership)
        , m_pButton(pButton)
        , m_nSignalId(g_signal_connect(pButton, "clicked", G_CALLBACK(signalClicked), this))
    {
        g_object_set_data(G_OBJECT(m_pButton), "g-lo-GtkInstanceButton", this);
    }

    virtual void set_label(const OUString& rText) override
    {
        ::set_label(m_pButton, rText);
    }

    virtual void set_image(VirtualDevice& rDevice) override
    {
        gtk_button_set_always_show_image(m_pButton, true);
        gtk_button_set_image_position(m_pButton, GTK_POS_LEFT);
        gtk_button_set_image(m_pButton, gtk_image_new_from_surface(get_underlying_cairo_surface(rDevice)));
    }

    virtual OUString get_label() const override
    {
        return ::get_label(m_pButton);
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
    return new GtkInstanceButton(pButton, false);
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
    GtkInstanceToggleButton(GtkToggleButton* pButton, bool bTakeOwnership)
        : GtkInstanceButton(GTK_BUTTON(pButton), bTakeOwnership)
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
    GtkLabel* m_pLabel;
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

    void grab_broken(GdkEventGrabBroken *event)
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

    bool key_press(GdkEventKey* pEvent)
    {
        if (pEvent->keyval == GDK_KEY_Escape)
        {
            set_active(false);
            return true;
        }
        return false;
    }

public:
    GtkInstanceMenuButton(GtkMenuButton* pMenuButton, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pMenuButton), bTakeOwnership)
        , MenuHelper(gtk_menu_button_get_popup(pMenuButton), false)
        , m_pMenuButton(pMenuButton)
        , m_pImage(nullptr)
        , m_pMenuHack(nullptr)
        , m_pPopover(nullptr)
        , m_nSignalId(0)
    {
        //do it "manually" so we can have the dropdown image in GtkMenuButtons shown
        //on the right at the same time as this image is shown on the left
        OString sLabel(gtk_button_get_label(GTK_BUTTON(m_pMenuButton)));
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pMenuButton));
        gtk_container_remove(GTK_CONTAINER(m_pMenuButton), pChild);

        m_pBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

        m_pLabel = GTK_LABEL(gtk_label_new_with_mnemonic(sLabel.getStr()));
        gtk_label_set_ellipsize(m_pLabel, PANGO_ELLIPSIZE_MIDDLE);
        gtk_label_set_mnemonic_widget(m_pLabel, GTK_WIDGET(m_pMenuButton));
        gtk_box_pack_start(m_pBox, GTK_WIDGET(m_pLabel), false, false, 0);

        gtk_box_pack_end(m_pBox, gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_BUTTON), false, false, 0);
        gtk_container_add(GTK_CONTAINER(m_pMenuButton), GTK_WIDGET(m_pBox));
        gtk_widget_show_all(GTK_WIDGET(m_pBox));

#if defined(GDK_WINDOWING_X11)
        //under wayland a Popover will work to "escape" the parent dialog, not
        //so under X, so come up with this hack to use a raw GtkWindow
        GdkDisplay *pDisplay = gtk_widget_get_display(m_pWidget);
        if (GDK_IS_X11_DISPLAY(pDisplay))
        {
            m_pMenuHack = GTK_WINDOW(gtk_window_new(GTK_WINDOW_POPUP));
            gtk_window_set_type_hint(m_pMenuHack, GDK_WINDOW_TYPE_HINT_COMBO);
            gtk_window_set_modal(m_pMenuHack, true);
            gtk_window_set_resizable(m_pMenuHack, false);
            m_nSignalId = g_signal_connect(GTK_TOGGLE_BUTTON(pMenuButton), "toggled", G_CALLBACK(signalToggled), this);
            g_signal_connect(m_pMenuHack, "grab-broken-event", G_CALLBACK(signalGrabBroken), this);
            g_signal_connect(m_pMenuHack, "button-release-event", G_CALLBACK(signalButtonRelease), this);
            g_signal_connect(m_pMenuHack, "key-press-event", G_CALLBACK(keyPress), this);
        }
#endif
    }

    virtual void set_label(const OUString& rText) override
    {
        ::set_label(m_pLabel, rText);
    }

    virtual void set_image(VirtualDevice& rDevice) override
    {
        if (!m_pImage)
        {
            m_pImage = GTK_IMAGE(gtk_image_new_from_surface(get_underlying_cairo_surface(rDevice)));
            GtkStyleContext *pContext = gtk_widget_get_style_context(GTK_WIDGET(m_pMenuButton));
            gint nImageSpacing(0);
            gtk_style_context_get_style(pContext, "image-spacing", &nImageSpacing, nullptr);
            gtk_box_pack_start(m_pBox, GTK_WIDGET(m_pImage), false, false, nImageSpacing);
            gtk_box_reorder_child(m_pBox, GTK_WIDGET(m_pImage), 0);
            gtk_widget_show(GTK_WIDGET(m_pImage));
        }
        else
            gtk_image_set_from_surface(m_pImage, get_underlying_cairo_surface(rDevice));
    }

    virtual void set_item_active(const OString& rIdent, bool bActive) override
    {
        MenuHelper::set_item_active(rIdent, bActive);
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
        m_pPopover = pPopoverWidget->getWidget();
        if (m_pMenuHack)
        {
            gtk_menu_button_set_popover(m_pMenuButton, gtk_popover_new(GTK_WIDGET(m_pMenuButton)));
        }
        else
        {
            gtk_menu_button_set_popover(m_pMenuButton, m_pPopover);
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

class GtkInstanceRadioButton : public GtkInstanceToggleButton, public virtual weld::RadioButton
{
public:
    GtkInstanceRadioButton(GtkRadioButton* pButton, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), bTakeOwnership)
    {
    }
};

class GtkInstanceCheckButton : public GtkInstanceToggleButton, public virtual weld::CheckButton
{
public:
    GtkInstanceCheckButton(GtkCheckButton* pButton, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), bTakeOwnership)
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
    GtkInstanceScale(GtkScale* pScale, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pScale), bTakeOwnership)
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
        gtk_range_set_value(GTK_RANGE(m_pScale), value);
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
    GtkInstanceProgressBar(GtkProgressBar* pProgressBar, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pProgressBar), bTakeOwnership)
        , m_pProgressBar(pProgressBar)
    {
    }

    virtual void set_percentage(int value) override
    {
        gtk_progress_bar_set_fraction(m_pProgressBar, value / 100.0);
    }
};

class GtkInstanceEntry : public GtkInstanceWidget, public virtual weld::Entry
{
private:
    GtkEntry* m_pEntry;
    gulong m_nChangedSignalId;
    gulong m_nInsertTextSignalId;
    gulong m_nCursorPosSignalId;

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

public:
    GtkInstanceEntry(GtkEntry* pEntry, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pEntry), bTakeOwnership)
        , m_pEntry(pEntry)
        , m_nChangedSignalId(g_signal_connect(pEntry, "changed", G_CALLBACK(signalChanged), this))
        , m_nInsertTextSignalId(g_signal_connect(pEntry, "insert-text", G_CALLBACK(signalInsertText), this))
        , m_nCursorPosSignalId(g_signal_connect(pEntry, "notify::cursor-position", G_CALLBACK(signalCursorPosition), this))
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
        enable_notify_events();
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
        gtk_editable_set_position(GTK_EDITABLE(m_pEntry), nCursorPos);
    }

    virtual void set_editable(bool bEditable) override
    {
        gtk_editable_set_editable(GTK_EDITABLE(m_pEntry), bEditable);
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(m_pEntry, m_nInsertTextSignalId);
        g_signal_handler_block(m_pEntry, m_nChangedSignalId);
        GtkInstanceWidget::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceWidget::enable_notify_events();
        g_signal_handler_unblock(m_pEntry, m_nChangedSignalId);
        g_signal_handler_unblock(m_pEntry, m_nInsertTextSignalId);
    }

    virtual vcl::Font get_font() override
    {
        PangoContext* pContext = gtk_widget_get_pango_context(m_pWidget);
        return pango_to_vcl(pango_context_get_font_description(pContext),
                            Application::GetSettings().GetUILanguageTag().getLocale());
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

    virtual ~GtkInstanceEntry() override
    {
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
            search->index = gtk_tree_path_get_indices(path)[0];
        g_free(pStr);
        return found;
    }

    GdkPixbuf* load_icon_by_name(const OUString& rIconName, const OUString& rIconTheme, const OUString& rUILang)
    {
        GdkPixbuf* pixbuf = nullptr;
        auto xMemStm = ImageTree::get().getImageStream(rIconName, rIconTheme, rUILang);
        if (xMemStm)
        {
            GdkPixbufLoader *pixbuf_loader = gdk_pixbuf_loader_new();
            gdk_pixbuf_loader_write(pixbuf_loader, static_cast<const guchar*>(xMemStm->GetData()),
                                    xMemStm->Seek(STREAM_SEEK_TO_END), nullptr);
            gdk_pixbuf_loader_close(pixbuf_loader, nullptr);
            pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);
            if (pixbuf)
                g_object_ref(pixbuf);
            g_object_unref(pixbuf_loader);
        }
        return pixbuf;
    }
}

class GtkInstanceTreeView : public GtkInstanceContainer, public virtual weld::TreeView
{
private:
    GtkTreeView* m_pTreeView;
    GtkListStore* m_pListStore;
    gulong m_nChangedSignalId;
    gulong m_nRowActivatedSignalId;

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

    OUString get(int pos, int col) const
    {
        OUString sRet;
        GtkTreeModel *pModel = GTK_TREE_MODEL(m_pListStore);
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
        {
            gchar* pStr;
            gtk_tree_model_get(pModel, &iter, col, &pStr, -1);
            sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

public:
    GtkInstanceTreeView(GtkTreeView* pTreeView, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pTreeView), bTakeOwnership)
        , m_pTreeView(pTreeView)
        , m_pListStore(GTK_LIST_STORE(gtk_tree_view_get_model(m_pTreeView)))
        , m_nChangedSignalId(g_signal_connect(gtk_tree_view_get_selection(pTreeView), "changed",
                             G_CALLBACK(signalChanged), this))
        , m_nRowActivatedSignalId(g_signal_connect(pTreeView, "row-activated", G_CALLBACK(signalRowActivated), this))
    {
    }

    virtual void insert_text(const OUString& rText, int pos) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_list_store_insert(m_pListStore, &iter, pos);
        gtk_list_store_set(m_pListStore, &iter, 0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(), -1);
        enable_notify_events();
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rText, const OUString& rImage) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_list_store_insert(m_pListStore, &iter, pos);
        if (rImage.isEmpty())
        {
            gtk_list_store_set(m_pListStore, &iter,
                    0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
                    1, OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr(),
                    -1);
        }
        else
        {
            GdkPixbuf* pixbuf = nullptr;

            if (rImage.lastIndexOf('.') != rImage.getLength() - 4)
            {
                assert((rImage == "dialog-warning" || rImage == "dialog-error" || rImage == "dialog-information") && "unknown stock image");

                GError *error = nullptr;
                GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
                pixbuf = gtk_icon_theme_load_icon(icon_theme, OUStringToOString(rImage, RTL_TEXTENCODING_UTF8).getStr(),
                                                  16, GTK_ICON_LOOKUP_USE_BUILTIN, &error);
            }
            else
            {
                const AllSettings& rSettings = Application::GetSettings();
                pixbuf = load_icon_by_name(rImage,
                                           rSettings.GetStyleSettings().DetermineIconTheme(),
                                           rSettings.GetUILanguageTag().getBcp47());
            }

            gtk_list_store_set(m_pListStore, &iter,
                    0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(),
                    1, OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr(),
                    2, pixbuf,
                    -1);

            if (pixbuf)
                g_object_unref(pixbuf);
        }
        enable_notify_events();
    }

    virtual void remove(int pos) override
    {
        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);
        gtk_list_store_remove(m_pListStore, &iter);
        enable_notify_events();
    }

    virtual int find_text(const OUString& rText) const override
    {
        Search aSearch(rText, 0);
        gtk_tree_model_foreach(GTK_TREE_MODEL(m_pListStore), foreach_find, &aSearch);
        return aSearch.index;
    }

    virtual int find_id(const OUString& rId) const override
    {
        Search aSearch(rId, 1);
        gtk_tree_model_foreach(GTK_TREE_MODEL(m_pListStore), foreach_find, &aSearch);
        return aSearch.index;
    }

    void move_before(int pos, int before)
    {
        if (pos == before)
            return;

        disable_notify_events();
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);

        GtkTreeIter position;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &position, nullptr, before);

        gtk_list_store_move_before(m_pListStore, &iter, &position);
        enable_notify_events();
    }

    virtual void set_top_entry(int pos) override
    {
        disable_notify_events();
        move_before(pos, 0);
        enable_notify_events();
    }

    virtual void clear() override
    {
        disable_notify_events();
        gtk_list_store_clear(m_pListStore);
        enable_notify_events();
    }

    virtual int n_children() const override
    {
        return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_pListStore), nullptr);
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

        GtkTreeModel* pModel;
        GList* pList = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(m_pTreeView), &pModel);
        for (GList* pItem = g_list_first(pList); pItem; pItem = g_list_next(pItem))
        {
            GtkTreePath* path = static_cast<GtkTreePath*>(pItem->data);
            aRows.push_back(gtk_tree_path_get_indices(path)[0]);
        }
        g_list_free_full(pList, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));

        return aRows;
    }

    virtual OUString get_text(int pos) const override
    {
        return get(pos, 0);
    }

    virtual OUString get_id(int pos) const override
    {
        return get(pos, 1);
    }

    virtual int get_selected_index() const override
    {
        int nRet = -1;
        GtkTreeIter iter;
        GtkTreeModel* pModel;
        if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(m_pTreeView), &pModel, &iter))
        {
            GtkTreePath* path = gtk_tree_model_get_path(pModel, &iter);
            nRet = gtk_tree_path_get_indices(path)[0];
            gtk_tree_path_free(path);
        }
        return nRet;
    }

    virtual void freeze() override
    {
        disable_notify_events();
        g_object_ref(m_pListStore);
        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pTreeView));
        gtk_tree_view_set_model(m_pTreeView, nullptr);
        enable_notify_events();
    }

    virtual void thaw() override
    {
        disable_notify_events();
        gtk_tree_view_set_model(m_pTreeView, GTK_TREE_MODEL(m_pListStore));
        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pTreeView));
        g_object_unref(m_pListStore);
        enable_notify_events();
    }

    virtual int get_height_rows(int nRows) const override
    {
        gint nMaxRowHeight = 0;
        GList *pColumns = gtk_tree_view_get_columns(m_pTreeView);
        for (GList* pEntry = g_list_first(pColumns); pEntry; pEntry = g_list_next(pEntry))
        {
            GtkTreeViewColumn* pColumn = GTK_TREE_VIEW_COLUMN(pEntry->data);
            GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
            GtkCellRenderer* pRenderer = GTK_CELL_RENDERER(g_list_nth_data(pRenderers, 0));
            gint nRowHeight;
            gtk_cell_renderer_get_preferred_height(pRenderer, GTK_WIDGET(m_pTreeView), nullptr, &nRowHeight);
            nMaxRowHeight = std::max(nMaxRowHeight, nRowHeight);
            g_list_free(pRenderers);
        }
        g_list_free(pColumns);

        gint nVerticalSeparator;
        gtk_widget_style_get(GTK_WIDGET(m_pTreeView), "vertical-separator", &nVerticalSeparator, nullptr);

        return (nMaxRowHeight * nRows) + (nVerticalSeparator * (nRows + 1));
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

    virtual void set_selection_mode(bool bMultiple) override
    {
        disable_notify_events();
        gtk_tree_selection_set_mode(gtk_tree_view_get_selection(m_pTreeView), bMultiple ? GTK_SELECTION_MULTIPLE : GTK_SELECTION_SINGLE);
        enable_notify_events();
    }

    virtual int count_selected_rows() const override
    {
        return gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(m_pTreeView));
    }

    virtual void disable_notify_events() override
    {
        g_signal_handler_block(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
        g_signal_handler_block(m_pTreeView, m_nRowActivatedSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pTreeView, m_nRowActivatedSignalId);
        g_signal_handler_unblock(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
    }

    virtual ~GtkInstanceTreeView() override
    {
        g_signal_handler_disconnect(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nRowActivatedSignalId);
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

    static void signalValueChanged(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_value_changed();
    }

    static gboolean signalOutput(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_output();
    }

    static gboolean signalInput(GtkSpinButton*, gdouble* new_value, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        SolarMutexGuard aGuard;
        int result;
        TriState eHandled = pThis->signal_input(&result);
        if (eHandled == TRISTATE_INDET)
            return false;
        if (eHandled == TRISTATE_TRUE)
        {
            *new_value = result;
            return true;
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
    GtkInstanceSpinButton(GtkSpinButton* pButton, bool bTakeOwnership)
        : GtkInstanceEntry(GTK_ENTRY(pButton), bTakeOwnership)
        , m_pButton(pButton)
        , m_nValueChangedSignalId(g_signal_connect(pButton, "value-changed", G_CALLBACK(signalValueChanged), this))
        , m_nOutputSignalId(g_signal_connect(pButton, "output", G_CALLBACK(signalOutput), this))
        , m_nInputSignalId(g_signal_connect(pButton, "input", G_CALLBACK(signalInput), this))
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

class GtkInstanceLabel : public GtkInstanceWidget, public virtual weld::Label
{
private:
    GtkLabel* m_pLabel;
public:
    GtkInstanceLabel(GtkLabel* pLabel, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pLabel), bTakeOwnership)
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
        GtkInstanceWidget* pTargetWidget = dynamic_cast<GtkInstanceWidget*>(pTarget);
        gtk_label_set_mnemonic_widget(m_pLabel, pTargetWidget ? pTargetWidget->getWidget() : nullptr);
    }
};

class GtkInstanceTextView : public GtkInstanceContainer, public virtual weld::TextView
{
private:
    GtkTextView* m_pTextView;
public:
    GtkInstanceTextView(GtkTextView* pTextView, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pTextView), bTakeOwnership)
        , m_pTextView(pTextView)
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
};

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
    sal_uInt16 m_nLastMouseButton;
    gulong m_nDrawSignalId;
    gulong m_nSizeAllocateSignalId;
    gulong m_nButtonPressSignalId;
    gulong m_nMotionSignalId;
    gulong m_nButtonReleaseSignalId;
    gulong m_nKeyPressSignalId;
    gulong m_nKeyReleaseSignalId;
    gulong m_nStyleUpdatedSignalId;
    gulong m_nQueryTooltip;

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
    static void signalSizeAllocate(GtkWidget*, GdkRectangle* allocation, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_size_allocate(allocation->width, allocation->height);
    }
    void signal_size_allocate(guint nWidth, guint nHeight)
    {
        m_xDevice->SetOutputSizePixel(Size(nWidth, nHeight));
        m_pSurface = get_underlying_cairo_surface(*m_xDevice);
        m_aSizeAllocateHdl.Call(Size(nWidth, nHeight));
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
    static gboolean signalButton(GtkWidget*, GdkEventButton* pEvent, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
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
        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(pEvent->state);
        sal_uInt16 nCode = m_nLastMouseButton | (nModCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2));
        MouseEvent aMEvt(aPos, nClicks, ImplGetMouseButtonMode(m_nLastMouseButton, nModCode), nCode, nCode);

        if (nEventType == SalEvent::MouseButtonDown)
            m_aMousePressHdl.Call(aMEvt);
        else
            m_aMouseReleaseHdl.Call(aMEvt);

        return true;
    }
    static gboolean signalMotion(GtkWidget*, GdkEventMotion* pEvent, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_motion(pEvent);
    }
    bool signal_motion(GdkEventMotion* pEvent)
    {
        Point aPos(pEvent->x, pEvent->y);
        if (AllSettings::GetLayoutRTL())
            aPos.setX(gtk_widget_get_allocated_width(m_pWidget) - 1 - aPos.X());
        sal_uInt32 nModCode = GtkSalFrame::GetMouseModCode(pEvent->state);
        sal_uInt16 nCode = m_nLastMouseButton | (nModCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2));
        MouseEvent aMEvt(aPos, 0, ImplGetMouseMoveMode(nModCode), nCode, nCode);

        m_aMouseMotionHdl.Call(aMEvt);
        return true;
    }
    static gboolean signalKey(GtkWidget*, GdkEventKey* pEvent, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        SolarMutexGuard aGuard;
        return pThis->signal_key(pEvent);
    }
    gboolean signal_key(GdkEventKey* pEvent)
    {
        sal_uInt16 nKeyCode = GtkSalFrame::GetKeyCode(pEvent->keyval);
        if (nKeyCode == 0)
        {
            guint updated_keyval = GtkSalFrame::GetKeyValFor(gdk_keymap_get_default(), pEvent->hardware_keycode, pEvent->group);
            nKeyCode = GtkSalFrame::GetKeyCode(updated_keyval);
        }
        nKeyCode |= GtkSalFrame::GetKeyModCode(pEvent->state);
        KeyEvent aKeyEvt(gdk_keyval_to_unicode(pEvent->keyval), nKeyCode, 0);

        bool bProcessed;
        if (pEvent->type == GDK_KEY_PRESS)
            bProcessed = m_aKeyPressHdl.Call(aKeyEvt);
        else
            bProcessed = m_aKeyReleaseHdl.Call(aKeyEvt);

        return bProcessed;
    }

public:
    GtkInstanceDrawingArea(GtkDrawingArea* pDrawingArea, const a11yref& rA11y, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pDrawingArea), bTakeOwnership)
        , m_pDrawingArea(pDrawingArea)
        , m_xAccessible(rA11y)
        , m_pAccessible(nullptr)
        , m_xDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT)
        , m_pSurface(nullptr)
        , m_nLastMouseButton(0)
        , m_nDrawSignalId(g_signal_connect(m_pDrawingArea, "draw", G_CALLBACK(signalDraw), this))
        , m_nSizeAllocateSignalId(g_signal_connect(m_pDrawingArea, "size_allocate", G_CALLBACK(signalSizeAllocate), this))
        , m_nButtonPressSignalId(g_signal_connect(m_pDrawingArea, "button-press-event", G_CALLBACK(signalButton), this))
        , m_nMotionSignalId(g_signal_connect(m_pDrawingArea, "motion-notify-event", G_CALLBACK(signalMotion), this))
        , m_nButtonReleaseSignalId(g_signal_connect(m_pDrawingArea, "button-release-event", G_CALLBACK(signalButton), this))
        , m_nKeyPressSignalId(g_signal_connect(m_pDrawingArea, "key-press-event", G_CALLBACK(signalKey), this))
        , m_nKeyReleaseSignalId(g_signal_connect(m_pDrawingArea,"key-release-event", G_CALLBACK(signalKey), this))
        , m_nStyleUpdatedSignalId(g_signal_connect(m_pDrawingArea,"style-updated", G_CALLBACK(signalStyleUpdated), this))
        , m_nQueryTooltip(g_signal_connect(m_pDrawingArea, "query-tooltip", G_CALLBACK(signalQueryTooltip), this))
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
        g_signal_handler_disconnect(m_pDrawingArea, m_nQueryTooltip);
        g_signal_handler_disconnect(m_pDrawingArea, m_nStyleUpdatedSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nKeyPressSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nKeyReleaseSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nButtonPressSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nMotionSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nButtonReleaseSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nSizeAllocateSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nDrawSignalId);
    }

    virtual OutputDevice& get_ref_device() override
    {
        return *m_xDevice;
    }
};

namespace
{
    gint sort_func(GtkTreeModel* pModel, GtkTreeIter* a, GtkTreeIter* b, gpointer data)
    {
        comphelper::string::NaturalStringSorter* pSorter = static_cast<comphelper::string::NaturalStringSorter*>(data);
        gchar* pName1;
        gchar* pName2;
        gtk_tree_model_get(pModel, a, 0, &pName1, -1);
        gtk_tree_model_get(pModel, b, 0, &pName2, -1);
        gint ret = pSorter->compare(OUString(pName1, strlen(pName1), RTL_TEXTENCODING_UTF8),
                                    OUString(pName2, strlen(pName2), RTL_TEXTENCODING_UTF8));
        g_free(pName1);
        g_free(pName2);
        return ret;
    }
}

class GtkInstanceComboBoxText : public GtkInstanceContainer, public virtual weld::ComboBoxText
{
private:
    GtkComboBoxText* m_pComboBoxText;
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    gboolean m_bPopupActive;
    gulong m_nChangedSignalId;
    gulong m_nPopupShownSignalId;
    gulong m_nEntryActivateSignalId;

    static void signalChanged(GtkComboBox*, gpointer widget)
    {
        GtkInstanceComboBoxText* pThis = static_cast<GtkInstanceComboBoxText*>(widget);
        SolarMutexGuard aGuard;
        pThis->signal_changed();
    }

    static void signalPopupShown(GtkComboBox*, GParamSpec*, gpointer widget)
    {
        GtkInstanceComboBoxText* pThis = static_cast<GtkInstanceComboBoxText*>(widget);
        pThis->signal_popup_shown();
    }

    void signal_popup_shown()
    {
        gboolean bIsShown(false);
        g_object_get(m_pComboBoxText, "popup-shown", &bIsShown, nullptr);
        if (m_bPopupActive != bIsShown)
        {
            m_bPopupActive = bIsShown;
            //restore focus to the entry view when the popup is gone, which
            //is what the vcl case does, to ease the transition a little
            gtk_widget_grab_focus(m_pWidget);
        }
    }

    static void signalEntryActivate(GtkComboBox*, gpointer widget)
    {
        GtkInstanceComboBoxText* pThis = static_cast<GtkInstanceComboBoxText*>(widget);
        pThis->signal_entry_activate();
    }

    void signal_entry_activate()
    {
        if (m_aEntryActivateHdl.IsSet())
        {
            SolarMutexGuard aGuard;
            m_aEntryActivateHdl.Call(*this);
            g_signal_stop_emission_by_name(get_entry(), "activate");
        }
    }

    OUString get(int pos, int col) const
    {
        OUString sRet;
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
        {
            gchar* pStr;
            gtk_tree_model_get(pModel, &iter, col, &pStr, -1);
            sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

    int find(const OUString& rStr, int col) const
    {
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter_first(pModel, &iter))
            return -1;

        OString aStr(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
        int nRet = 0;
        do
        {
            gchar* pStr;
            gtk_tree_model_get(pModel, &iter, col, &pStr, -1);
            const bool bEqual = g_strcmp0(pStr, aStr.getStr()) == 0;
            g_free(pStr);
            if (bEqual)
                return nRet;
            ++nRet;
        } while (gtk_tree_model_iter_next(pModel, &iter));

        return -1;
    }

    GtkEntry* get_entry()
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBoxText));
        if (!GTK_IS_ENTRY(pChild))
            return nullptr;
        return GTK_ENTRY(pChild);
    }

    void setup_completion(GtkEntry* pEntry)
    {
        if (gtk_entry_get_completion(pEntry))
            return;
        GtkEntryCompletion* pCompletion = gtk_entry_completion_new();
        gtk_entry_completion_set_model(pCompletion, gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText)));
        gtk_entry_completion_set_text_column(pCompletion, 0);
        gtk_entry_completion_set_inline_selection(pCompletion, true);
        gtk_entry_completion_set_inline_completion(pCompletion, true);
        gtk_entry_completion_set_popup_completion(pCompletion, false);
        gtk_entry_set_completion(pEntry, pCompletion);
        g_object_unref(pCompletion);
    }

public:
    GtkInstanceComboBoxText(GtkComboBoxText* pComboBoxText, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pComboBoxText), bTakeOwnership)
        , m_pComboBoxText(pComboBoxText)
        , m_bPopupActive(false)
        , m_nChangedSignalId(g_signal_connect(m_pComboBoxText, "changed", G_CALLBACK(signalChanged), this))
        , m_nPopupShownSignalId(g_signal_connect(m_pComboBoxText, "notify::popup-shown", G_CALLBACK(signalPopupShown), this))
    {
        // this bit isn't great, I really want to be able to ellipse the text in the comboboxtext itself and let
        // the popup menu render them in full, in the interim allow the text to wrap in both cases
        GList* cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(m_pComboBoxText));
        GtkCellRenderer* cell = static_cast<GtkCellRenderer*>(cells->data);
        g_object_set(G_OBJECT(cell), "ellipsize", PANGO_ELLIPSIZE_MIDDLE, nullptr);
        g_list_free(cells);

        if (GtkEntry* pEntry = get_entry())
        {
            setup_completion(pEntry);
            m_nEntryActivateSignalId = g_signal_connect(pEntry, "activate", G_CALLBACK(signalEntryActivate), this);
        }
        else
            m_nEntryActivateSignalId = 0;
    }

    virtual int get_active() const override
    {
        return gtk_combo_box_get_active(GTK_COMBO_BOX(m_pComboBoxText));
    }

    virtual OUString get_active_id() const override
    {
        const gchar* pText = gtk_combo_box_get_active_id(GTK_COMBO_BOX(m_pComboBoxText));
        return OUString(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        disable_notify_events();
        OString aId(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8));
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(m_pComboBoxText), aId.getStr());
        enable_notify_events();
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        // tweak the cell render to get a narrower size to stick
        GList* cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(m_pComboBoxText));
        GtkCellRenderer* cell = static_cast<GtkCellRenderer*>(cells->data);
        GtkRequisition size;
        gtk_cell_renderer_get_preferred_size(cell, m_pWidget, &size, nullptr);
        gtk_cell_renderer_set_fixed_size(cell, nWidth, size.height);
        g_list_free(cells);

        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual void set_active(int pos) override
    {
        disable_notify_events();
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_pComboBoxText), pos);
        enable_notify_events();
    }

    virtual OUString get_active_text() const override
    {
        gchar* pText = gtk_combo_box_text_get_active_text(m_pComboBoxText);
        OUString sRet(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pText);
        return sRet;
    }

    virtual OUString get_text(int pos) const override
    {
        return get(pos, 0);
    }

    virtual OUString get_id(int pos) const override
    {
        gint id_column = gtk_combo_box_get_id_column(GTK_COMBO_BOX(m_pComboBoxText));
        return get(pos, id_column);
    }

    virtual void insert_text(int pos, const OUString& rStr) override
    {
        disable_notify_events();
        gtk_combo_box_text_insert_text(m_pComboBoxText, pos, OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
        enable_notify_events();
    }

    virtual void remove(int pos) override
    {
        disable_notify_events();
        gtk_combo_box_text_remove(m_pComboBoxText, pos);
        enable_notify_events();
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr) override
    {
        disable_notify_events();
        gtk_combo_box_text_insert(m_pComboBoxText, pos,
                                  OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr(),
                                  OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
        enable_notify_events();
    }

    virtual int get_count() const override
    {
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        return gtk_tree_model_iter_n_children(pModel, nullptr);
    }

    virtual int find_text(const OUString& rStr) const override
    {
        return find(rStr, 0);
    }

    virtual int find_id(const OUString& rId) const override
    {
        return find(rId, 1);
    }

    virtual void clear() override
    {
        disable_notify_events();
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        gtk_list_store_clear(GTK_LIST_STORE(pModel));
        enable_notify_events();
    }

    virtual void make_sorted() override
    {
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetUILanguageTag().getLocale()));
        GtkTreeModel* pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
        gtk_tree_sortable_set_sort_func(pSortable, 0, sort_func, m_xSorter.get(), nullptr);
        gtk_tree_sortable_set_sort_column_id(pSortable, 0, GTK_SORT_ASCENDING);
    }

    virtual void set_entry_error(bool bError) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBoxText));
        assert(GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        if (bError)
            gtk_entry_set_icon_from_icon_name(pEntry, GTK_ENTRY_ICON_SECONDARY, "dialog-error");
        else
            gtk_entry_set_icon_from_icon_name(pEntry, GTK_ENTRY_ICON_SECONDARY, nullptr);
    }

    virtual void set_entry_text(const OUString& rText) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBoxText));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        disable_notify_events();
        gtk_entry_set_text(pEntry, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
        enable_notify_events();
    }

    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBoxText));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        disable_notify_events();
        gtk_editable_select_region(GTK_EDITABLE(pEntry), nStartPos, nEndPos);
        enable_notify_events();
    }

    virtual bool get_entry_selection_bounds(int& rStartPos, int &rEndPos) override
    {
        GtkWidget* pChild = gtk_bin_get_child(GTK_BIN(m_pComboBoxText));
        assert(pChild && GTK_IS_ENTRY(pChild));
        GtkEntry* pEntry = GTK_ENTRY(pChild);
        return gtk_editable_get_selection_bounds(GTK_EDITABLE(pEntry), &rStartPos, &rEndPos);
    }

    virtual void set_entry_completion(bool bEnable) override
    {
        GtkEntry* pEntry = get_entry();
        assert(pEntry);
        if (bEnable)
            setup_completion(pEntry);
        else
            gtk_entry_set_completion(pEntry, nullptr);
    }

    virtual void disable_notify_events() override
    {
        if (GtkEntry* pEntry = get_entry())
            g_signal_handler_block(pEntry, m_nEntryActivateSignalId);
        g_signal_handler_block(m_pComboBoxText, m_nChangedSignalId);
        g_signal_handler_block(m_pComboBoxText, m_nPopupShownSignalId);
        GtkInstanceContainer::disable_notify_events();
    }

    virtual void enable_notify_events() override
    {
        GtkInstanceContainer::enable_notify_events();
        g_signal_handler_unblock(m_pComboBoxText, m_nPopupShownSignalId);
        g_signal_handler_unblock(m_pComboBoxText, m_nChangedSignalId);
        if (GtkEntry* pEntry = get_entry())
            g_signal_handler_unblock(pEntry, m_nEntryActivateSignalId);
    }

    virtual void freeze() override
    {
        gtk_widget_freeze_child_notify(GTK_WIDGET(m_pComboBoxText));
        if (m_xSorter)
        {
            GtkTreeModel* pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
            GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
            gtk_tree_sortable_set_sort_column_id(pSortable, GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
        }
    }

    virtual void thaw() override
    {
        if (m_xSorter)
        {
            GtkTreeModel* pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
            GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
            gtk_tree_sortable_set_sort_column_id(pSortable, 0, GTK_SORT_ASCENDING);
        }
        gtk_widget_thaw_child_notify(GTK_WIDGET(m_pComboBoxText));
    }

    virtual ~GtkInstanceComboBoxText() override
    {
        if (GtkEntry* pEntry = get_entry())
            g_signal_handler_disconnect(pEntry, m_nEntryActivateSignalId);
        g_signal_handler_disconnect(m_pComboBoxText, m_nChangedSignalId);
        g_signal_handler_disconnect(m_pComboBoxText, m_nPopupShownSignalId);
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
    GtkInstanceExpander(GtkExpander* pExpander, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pExpander), bTakeOwnership)
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
                OUString sHelpText = pHelp->GetHelpText(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), nullptr);
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
            GtkIconSize size;
            g_object_get(pImage, "icon-size", &size, nullptr);
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
    {
        ensure_intercept_drawing_area_accessibility();

        OUString aUri(rUIRoot + rUIFile);
        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aUri, aPath);
        m_pBuilder = gtk_builder_new_from_file(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr());

        sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
        if (nIdx != -1)
            m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
        m_sHelpRoot = m_sHelpRoot + OUString('/');
        m_aUtf8HelpRoot = OUStringToOString(m_sHelpRoot, RTL_TEXTENCODING_UTF8);
        m_aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        m_aUILang = Application::GetSettings().GetUILanguageTag().getBcp47();

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

    virtual weld::MessageDialog* weld_message_dialog(const OString &id, bool bTakeOwnership) override
    {
        GtkMessageDialog* pMessageDialog = GTK_MESSAGE_DIALOG(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pMessageDialog)
            return nullptr;
        gtk_window_set_transient_for(GTK_WINDOW(pMessageDialog), GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return new GtkInstanceMessageDialog(pMessageDialog, bTakeOwnership);
    }

    virtual weld::Dialog* weld_dialog(const OString &id, bool bTakeOwnership) override
    {
        GtkDialog* pDialog = GTK_DIALOG(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pDialog)
            return nullptr;
        gtk_window_set_transient_for(GTK_WINDOW(pDialog), GTK_WINDOW(gtk_widget_get_toplevel(m_pParentWidget)));
        return new GtkInstanceDialog(pDialog, bTakeOwnership);
    }

    virtual weld::Window* weld_window(const OString &id, bool bTakeOwnership) override
    {
        GtkWindow* pWindow = GTK_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pWindow ? new GtkInstanceWindow(pWindow, bTakeOwnership) : nullptr;
    }

    virtual weld::Widget* weld_widget(const OString &id, bool bTakeOwnership) override
    {
        GtkWidget* pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pWidget)
            return nullptr;
        auto_add_parentless_widgets_to_container(pWidget);
        return new GtkInstanceWidget(pWidget, bTakeOwnership);
    }

    virtual weld::Container* weld_container(const OString &id, bool bTakeOwnership) override
    {
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pContainer)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pContainer));
        return new GtkInstanceContainer(pContainer, bTakeOwnership);
    }

    virtual weld::Frame* weld_frame(const OString &id, bool bTakeOwnership) override
    {
        GtkFrame* pFrame = GTK_FRAME(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pFrame)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pFrame));
        return new GtkInstanceFrame(pFrame, bTakeOwnership);
    }

    virtual weld::ScrolledWindow* weld_scrolled_window(const OString &id, bool bTakeOwnership) override
    {
        GtkScrolledWindow* pScrolledWindow = GTK_SCROLLED_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pScrolledWindow)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pScrolledWindow));
        return new GtkInstanceScrolledWindow(pScrolledWindow, bTakeOwnership);
    }

    virtual weld::Notebook* weld_notebook(const OString &id, bool bTakeOwnership) override
    {
        GtkNotebook* pNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pNotebook)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pNotebook));
        return new GtkInstanceNotebook(pNotebook, bTakeOwnership);
    }

    virtual weld::Button* weld_button(const OString &id, bool bTakeOwnership) override
    {
        GtkButton* pButton = GTK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return new GtkInstanceButton(pButton, bTakeOwnership);
    }

    virtual weld::MenuButton* weld_menu_button(const OString &id, bool bTakeOwnership) override
    {
        GtkMenuButton* pButton = GTK_MENU_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pButton));
        return new GtkInstanceMenuButton(pButton, bTakeOwnership);
    }

    virtual weld::ToggleButton* weld_toggle_button(const OString &id, bool bTakeOwnership) override
    {
        GtkToggleButton* pToggleButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pToggleButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pToggleButton));
        return new GtkInstanceToggleButton(pToggleButton, bTakeOwnership);
    }

    virtual weld::RadioButton* weld_radio_button(const OString &id, bool bTakeOwnership) override
    {
        GtkRadioButton* pRadioButton = GTK_RADIO_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pRadioButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pRadioButton));
        return new GtkInstanceRadioButton(pRadioButton, bTakeOwnership);
    }

    virtual weld::CheckButton* weld_check_button(const OString &id, bool bTakeOwnership) override
    {
        GtkCheckButton* pCheckButton = GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pCheckButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pCheckButton));
        return new GtkInstanceCheckButton(pCheckButton, bTakeOwnership);
    }

    virtual weld::Scale* weld_scale(const OString &id, bool bTakeOwnership) override
    {
        GtkScale* pScale = GTK_SCALE(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pScale)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pScale));
        return new GtkInstanceScale(pScale, bTakeOwnership);
    }

    virtual weld::ProgressBar* weld_progress_bar(const OString &id, bool bTakeOwnership) override
    {
        GtkProgressBar* pProgressBar = GTK_PROGRESS_BAR(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pProgressBar)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pProgressBar));
        return new GtkInstanceProgressBar(pProgressBar, bTakeOwnership);
    }

    virtual weld::Entry* weld_entry(const OString &id, bool bTakeOwnership) override
    {
        GtkEntry* pEntry = GTK_ENTRY(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pEntry)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pEntry));
        return new GtkInstanceEntry(pEntry, bTakeOwnership);
    }

    virtual weld::SpinButton* weld_spin_button(const OString &id, bool bTakeOwnership) override
    {
        GtkSpinButton* pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pSpinButton)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pSpinButton));
        return new GtkInstanceSpinButton(pSpinButton, bTakeOwnership);
    }

    virtual weld::TimeSpinButton* weld_time_spin_button(const OString& id, TimeFieldFormat eFormat,
                                                        bool bTakeOwnership) override
    {
        return new weld::TimeSpinButton(weld_spin_button(id, bTakeOwnership), eFormat);
    }

    virtual weld::ComboBoxText* weld_combo_box_text(const OString &id, bool bTakeOwnership) override
    {
        GtkComboBoxText* pComboBoxText = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pComboBoxText)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pComboBoxText));
        return new GtkInstanceComboBoxText(pComboBoxText, bTakeOwnership);
    }

    virtual weld::TreeView* weld_tree_view(const OString &id, bool bTakeOwnership) override
    {
        GtkTreeView* pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pTreeView)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pTreeView));
        return new GtkInstanceTreeView(pTreeView, bTakeOwnership);
    }

    virtual weld::Label* weld_label(const OString &id, bool bTakeOwnership) override
    {
        GtkLabel* pLabel = GTK_LABEL(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pLabel)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pLabel));
        return new GtkInstanceLabel(pLabel, bTakeOwnership);
    }

    virtual weld::TextView* weld_text_view(const OString &id, bool bTakeOwnership) override
    {
        GtkTextView* pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pTextView)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pTextView));
        return new GtkInstanceTextView(pTextView, bTakeOwnership);
    }

    virtual weld::Expander* weld_expander(const OString &id, bool bTakeOwnership) override
    {
        GtkExpander* pExpander = GTK_EXPANDER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pExpander)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pExpander));
        return new GtkInstanceExpander(pExpander, bTakeOwnership);
    }

    virtual weld::DrawingArea* weld_drawing_area(const OString &id, const a11yref& rA11y,
            FactoryFunction /*pUITestFactoryFunction*/, void* /*pUserData*/, bool bTakeOwnership) override
    {
        GtkDrawingArea* pDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pDrawingArea)
            return nullptr;
        auto_add_parentless_widgets_to_container(GTK_WIDGET(pDrawingArea));
        return new GtkInstanceDrawingArea(pDrawingArea, rA11y, bTakeOwnership);
    }

    virtual weld::Menu* weld_menu(const OString &id, bool bTakeOwnership) override
    {
        GtkMenu* pMenu = GTK_MENU(gtk_builder_get_object(m_pBuilder, id.getStr()));
        if (!pMenu)
            return nullptr;
        return new GtkInstanceMenu(pMenu, bTakeOwnership);
    }

    virtual weld::SizeGroup* create_size_group() override
    {
        return new GtkInstanceSizeGroup;
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
    std::unique_ptr<weld::Widget> xTemp(pWidget != m_pWidget ? new GtkInstanceWidget(pWidget, false) : nullptr);
    weld::Widget* pSource = xTemp ? xTemp.get() : this;
    bool bRunNormalHelpRequest = !m_aHelpRequestHdl.IsSet() || m_aHelpRequestHdl.Call(*pSource);
    Help* pHelp = bRunNormalHelpRequest ? Application::GetHelp() : nullptr;
    if (pHelp)
        pHelp->Start(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), pSource);
}

weld::Builder* GtkInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    GtkInstanceWidget* pParentWidget = dynamic_cast<GtkInstanceWidget*>(pParent);
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
    return new GtkInstanceMessageDialog(pMessageDialog, true);
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
        m_xFrameWeld.reset(new GtkInstanceWindow(GTK_WINDOW(getWindow()), false));
    return m_xFrameWeld.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
