/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../gtk/gtkinst.cxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/datatransfer/XTransferable.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardEx.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardListener.hpp"
#include "com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp"
#include "com/sun/star/datatransfer/dnd/DNDConstants.hpp"
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include "cppuhelper/compbase.hxx"
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/fract.hxx>
#include <vcl/pngwrite.hxx>
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
                                    rtl_str_getLength(pFinalName),
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
    std::list< Reference<css::datatransfer::clipboard::XClipboardListener> > m_aListeners;
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

    std::list< Reference< datatransfer::clipboard::XClipboardListener > > aListeners( m_aListeners );
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
    for( std::list< Reference< datatransfer::clipboard::XClipboardListener > >::iterator it =
         aListeners.begin(); it != aListeners.end() ; ++it )
    {
        (*it)->changedContents( aEv );
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

    m_aListeners.remove( listener );
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

    m_aListeners.remove( xListener );
}

void GtkDropTarget::fire_drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
{
    osl::ClearableGuard<osl::Mutex> aGuard( m_aMutex );
    std::list<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto it = aListeners.begin(); it != aListeners.end(); ++it)
    {
        (*it)->drop( dtde );
    }
}

void GtkDropTarget::fire_dragEnter(const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::list<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto it = aListeners.begin(); it != aListeners.end(); ++it)
    {
        (*it)->dragEnter( dtde );
    }
}

void GtkDropTarget::fire_dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::list<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto it = aListeners.begin(); it != aListeners.end(); ++it)
    {
        (*it)->dragOver( dtde );
    }
}

void GtkDropTarget::fire_dragExit(const css::datatransfer::dnd::DropTargetEvent& dte)
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::list<css::uno::Reference<css::datatransfer::dnd::XDropTargetListener>> aListeners(m_aListeners);
    aGuard.clear();

    for (auto it = aListeners.begin(); it != aListeners.end(); ++it)
    {
        (*it)->dragExit( dte );
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
#if GTK_CHECK_VERSION(3,16,0)
    GtkWidget *m_pGLArea;
    GdkGLContext *m_pContext;
    guint m_nAreaFrameBuffer;
    guint m_nFrameBuffer;
    guint m_nRenderBuffer;
    guint m_nDepthBuffer;
    guint m_nFrameScratchBuffer;
    guint m_nRenderScratchBuffer;
    guint m_nDepthScratchBuffer;
#endif

public:
    GtkOpenGLContext()
        : OpenGLContext()
#if GTK_CHECK_VERSION(3,16,0)
        , m_pGLArea(nullptr)
        , m_pContext(nullptr)
        , m_nAreaFrameBuffer(0)
        , m_nFrameBuffer(0)
        , m_nRenderBuffer(0)
        , m_nDepthBuffer(0)
        , m_nFrameScratchBuffer(0)
        , m_nRenderScratchBuffer(0)
        , m_nDepthScratchBuffer(0)
#endif
    {
    }

    virtual bool initWindow() override
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

        return true;
    }

private:
    virtual const GLWindow& getOpenGLWindow() const override { return m_aGLWin; }
    virtual GLWindow& getModifiableOpenGLWindow() override { return m_aGLWin; }

#if GTK_CHECK_VERSION(3,16,0)
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

#endif

    virtual void adjustToNewSize() override
    {
#if GTK_CHECK_VERSION(3,16,0)
        if (m_pGLArea)
        {
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
#endif
    }

    virtual bool ImplInit() override
    {
#if GTK_CHECK_VERSION(3,16,0)
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
#endif
        bool bRet = InitGL();
        InitGLDebugging();
        return bRet;
    }

#if GTK_CHECK_VERSION(3,16,0)
    virtual void restoreDefaultFramebuffer() override
    {
        OpenGLContext::restoreDefaultFramebuffer();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_nFrameScratchBuffer);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                     GL_RENDERBUFFER_EXT, m_nRenderScratchBuffer);
    }
#endif

    virtual void makeCurrent() override
    {
        if (isCurrent())
            return;

        clearCurrent();

#if GTK_CHECK_VERSION(3,16,0)
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
#endif

        registerAsCurrent();
    }

    virtual void destroyCurrentContext() override
    {
#if GTK_CHECK_VERSION(3,16,0)
        gdk_gl_context_clear_current();
#endif
    }

    virtual bool isCurrent() override
    {
#if GTK_CHECK_VERSION(3,16,0)
        return m_pGLArea && gdk_gl_context_get_current() == m_pContext;
#else
        return false;
#endif
    }

    virtual void sync() override
    {
    }

    virtual void resetCurrent() override
    {
        clearCurrent();
#if GTK_CHECK_VERSION(3,16,0)
        gdk_gl_context_clear_current();
#endif
    }

    virtual void swapBuffers() override
    {
#if GTK_CHECK_VERSION(3,16,0)
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
#endif
        BuffersSwapped();
    }
#if GTK_CHECK_VERSION(3,16,0)
    virtual ~GtkOpenGLContext() override
    {
        if (m_pContext)
        {
            g_clear_object(&m_pContext);
        }
    }
#endif
};

OpenGLContext* GtkInstance::CreateOpenGLContext()
{
    return new GtkOpenGLContext;
}

class GtkInstanceWidget : public virtual Weld::Widget
{
private:
    GtkWidget* m_pWidget;
    bool m_bTakeOwnership;

public:
    GtkInstanceWidget(GtkWidget* pWidget, bool bTakeOwnership)
        : m_pWidget(pWidget)
        , m_bTakeOwnership(bTakeOwnership)
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

    virtual void grab_focus() override
    {
        gtk_widget_grab_focus(m_pWidget);
    }

    virtual bool has_focus() const override
    {
        return gtk_widget_has_focus(m_pWidget);
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
        gtk_widget_set_size_request(m_pWidget, nWidth, nHeight);
    }

    virtual Size get_preferred_size() const override
    {
        GtkRequisition size;
        gtk_widget_get_preferred_size(m_pWidget, nullptr, &size);
        return Size(size.width, size.height);
    }

    virtual float approximate_char_width() const override
    {
        PangoContext* pContext = gtk_widget_get_pango_context(m_pWidget);
        PangoFontMetrics* pMetrics = pango_context_get_metrics(pContext,
                                         pango_context_get_font_description(pContext),
                                         pango_context_get_language(pContext));
        float nCharWidth = pango_font_metrics_get_approximate_char_width(pMetrics);
        float nDigitWidth = pango_font_metrics_get_approximate_digit_width(pMetrics);
        pango_font_metrics_unref(pMetrics);

        return std::max(nCharWidth, nDigitWidth) / PANGO_SCALE;
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

    OString get_buildable_name() const override
    {
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(m_pWidget));
        return OString(pStr, pStr ? strlen(pStr) : 0);
    }

    virtual ~GtkInstanceWidget() override
    {
        if (m_bTakeOwnership)
            gtk_widget_destroy(m_pWidget);
    }

    GtkWidget* get_widget()
    {
        return m_pWidget;
    }
};

class GtkInstanceContainer : public GtkInstanceWidget, public virtual Weld::Container
{
private:
    GtkContainer* m_pContainer;
public:
    GtkInstanceContainer(GtkContainer* pContainer, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pContainer), bTakeOwnership)
        , m_pContainer(pContainer)
    {
    }

    virtual void add(Weld::Widget* pWidget) override
    {
        GtkInstanceWidget* pInstanceWidget = dynamic_cast<GtkInstanceWidget*>(pWidget);
        gtk_container_add(m_pContainer, pInstanceWidget->get_widget());
    }
};


class GtkInstanceWindow : public GtkInstanceContainer, public virtual Weld::Window
{
private:
    GtkWindow* m_pWindow;

    static void help_pressed(GtkAccelGroup*, GObject*, guint, GdkModifierType, gpointer widget)
    {
        GtkInstanceWindow* pThis = static_cast<GtkInstanceWindow*>(widget);
        pThis->help();
    }
protected:
    void help()
    {
        //show help for widget with keyboard focus
        GtkWidget* pFocus = gtk_window_get_focus(m_pWindow);
        if (!pFocus)
            pFocus = GTK_WIDGET(m_pWindow);
        fprintf(stderr, "help for %p\n", pFocus);
    }
public:
    GtkInstanceWindow(GtkWindow* pWindow, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pWindow), bTakeOwnership)
        , m_pWindow(pWindow)
    {
        //hook up F1 to show help
        GtkAccelGroup *pGroup = gtk_accel_group_new();
        GClosure* closure = g_cclosure_new(G_CALLBACK(help_pressed), this, nullptr);
        gtk_accel_group_connect(pGroup, GDK_KEY_F1, (GdkModifierType)0, GTK_ACCEL_LOCKED, closure);
        gtk_window_add_accel_group(pWindow, pGroup);
    }

    virtual void set_transient_for(Weld::Window* pParent) override
    {
        GtkInstanceWindow* pParentFrame = dynamic_cast<GtkInstanceWindow*>(pParent);
        GtkWindow* pParentWidget = pParentFrame->getWindow();
        gtk_window_set_transient_for(m_pWindow, pParentWidget);
    }

    virtual void set_title(const OUString& rTitle) override
    {
        gtk_window_set_title(m_pWindow, OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_title() const override
    {
        const gchar* pStr = gtk_window_get_title(m_pWindow);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    GtkWindow* getWindow()
    {
        return m_pWindow;
    }
};

class GtkInstanceDialog : public GtkInstanceWindow, public virtual Weld::Dialog
{
private:
    GtkDialog* m_pDialog;
    gulong m_nCloseSignalId;

    static void signalClose(GtkWidget *, gpointer widget)
    {
        GtkInstanceDialog* pThis = static_cast<GtkInstanceDialog*>(widget);
        pThis->response(RET_CANCEL);
    }
public:
    GtkInstanceDialog(GtkDialog* pDialog, bool bTakeOwnership)
        : GtkInstanceWindow(GTK_WINDOW(pDialog), bTakeOwnership)
        , m_pDialog(pDialog)
        , m_nCloseSignalId(g_signal_connect(m_pDialog, "close", G_CALLBACK(signalClose), this))
    {
    }

    virtual int run() override
    {
        int ret;
        while (1)
        {
            ret = gtk_dialog_run(m_pDialog);
            if (ret == GTK_RESPONSE_HELP)
            {
                help();
                continue;
            }
            else if (ret == GTK_RESPONSE_OK)
                ret = RET_OK;
            else if (ret == GTK_RESPONSE_CANCEL)
                ret = RET_CANCEL;
            break;
        }
        return ret;
    }

    virtual void response(int nResponse) override
    {
        if (nResponse == RET_OK)
            nResponse = GTK_RESPONSE_OK;
        else if (nResponse == RET_CANCEL)
            nResponse = GTK_RESPONSE_CANCEL;
        else if (nResponse == RET_HELP)
            nResponse = GTK_RESPONSE_HELP;
        gtk_dialog_response(m_pDialog, nResponse);
    }

    virtual ~GtkInstanceDialog() override
    {
        g_signal_handler_disconnect(m_pDialog, m_nCloseSignalId);
    }
};

static OString MapToGtkAccelerator(const OUString &rStr)
{
    return OUStringToOString(rStr.replaceFirst("~", "_"), RTL_TEXTENCODING_UTF8);
}

class GtkInstanceFrame : public GtkInstanceContainer, public virtual Weld::Frame
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

class GtkInstanceNotebook : public GtkInstanceContainer, public virtual Weld::Notebook
{
private:
    GtkNotebook* m_pNotebook;
    gulong m_nSignalId;
    mutable std::vector<std::unique_ptr<GtkInstanceContainer>> m_aPages;

    static void signalSwitchPage(GtkNotebook*, GtkWidget*, guint nNewPage, gpointer widget)
    {
        GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
        pThis->signal_switch_page(nNewPage);
    }

    void signal_switch_page(guint nNewPage)
    {
        bool bAllow = m_aLeavePageHdl.Call(get_current_page_ident());
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

    virtual Weld::Container* get_page(const OString& rIdent) const override
    {
        int nPage = get_page_number(rIdent);
        if (nPage < 0)
            return nullptr;
        GtkContainer* pChild = GTK_CONTAINER(gtk_notebook_get_nth_page(m_pNotebook, nPage));
        unsigned int nPageIndex = (unsigned int)nPage;
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

    virtual ~GtkInstanceNotebook() override
    {
        g_signal_handler_disconnect(m_pNotebook, m_nSignalId);
    }
};

class GtkInstanceButton : public GtkInstanceContainer, public virtual Weld::Button
{
private:
    GtkButton* m_pButton;
    gulong m_nSignalId;

    static void signalClicked(GtkButton*, gpointer widget)
    {
        GtkInstanceButton* pThis = static_cast<GtkInstanceButton*>(widget);
        pThis->signal_clicked();
    }
public:
    GtkInstanceButton(GtkButton* pButton, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pButton), bTakeOwnership)
        , m_pButton(pButton)
        , m_nSignalId(g_signal_connect(pButton, "clicked", G_CALLBACK(signalClicked), this))
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        gtk_button_set_label(m_pButton, MapToGtkAccelerator(rText).getStr());
    }

    virtual OUString get_label() const override
    {
        const gchar* pStr = gtk_button_get_label(m_pButton);
        return OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
    }

    virtual ~GtkInstanceButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nSignalId);
    }
};

class GtkInstanceToggleButton : public GtkInstanceButton, public virtual Weld::ToggleButton
{
private:
    GtkToggleButton* m_pToggleButton;
    gulong m_nSignalId;

    static void signalToggled(GtkToggleButton*, gpointer widget)
    {
        GtkInstanceToggleButton* pThis = static_cast<GtkInstanceToggleButton*>(widget);
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
        gtk_toggle_button_set_active(m_pToggleButton, active);
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

    virtual ~GtkInstanceToggleButton() override
    {
        g_signal_handler_disconnect(m_pToggleButton, m_nSignalId);
    }
};

class GtkInstanceRadioButton : public GtkInstanceToggleButton, public virtual Weld::RadioButton
{
public:
    GtkInstanceRadioButton(GtkRadioButton* pButton, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), bTakeOwnership)
    {
    }
};

class GtkInstanceCheckButton : public GtkInstanceToggleButton, public virtual Weld::CheckButton
{
public:
    GtkInstanceCheckButton(GtkCheckButton* pButton, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), bTakeOwnership)
    {
    }
};

class GtkInstanceEntry : public GtkInstanceWidget, public virtual Weld::Entry
{
private:
    GtkEntry* m_pEntry;
    gulong m_nChangedSignalId;
    gulong m_nInsertTextSignalId;

    static void signalChanged(GtkEntry*, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
        pThis->signal_changed();
    }

    static void signalInsertText(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength,
                                 gint* position, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
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
public:
    GtkInstanceEntry(GtkEntry* pEntry, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pEntry), bTakeOwnership)
        , m_pEntry(pEntry)
        , m_nChangedSignalId(g_signal_connect(pEntry, "changed", G_CALLBACK(signalChanged), this))
        , m_nInsertTextSignalId(g_signal_connect(pEntry, "insert-text", G_CALLBACK(signalInsertText), this))
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        gtk_entry_set_text(m_pEntry, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_text() const override
    {
        const gchar* pText = gtk_entry_get_text(m_pEntry);
        OUString sRet(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        return sRet;
    }

    virtual void set_width_chars(int nChars) override
    {
        gtk_entry_set_width_chars(m_pEntry, nChars);
    }

    virtual ~GtkInstanceEntry() override
    {
        g_signal_handler_disconnect(m_pEntry, m_nInsertTextSignalId);
        g_signal_handler_disconnect(m_pEntry, m_nChangedSignalId);
    }
};

namespace
{
    struct Search
    {
        OString str;
        int index = -1;
        Search(const OUString& rText)
            : str(OUStringToOString(rText, RTL_TEXTENCODING_UTF8))
            , index(-1)
        {
        }
    };

    bool foreach_find(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, Search* search)
    {
        gchar *pStr = nullptr;
        gtk_tree_model_get(model, iter, 0, &pStr, -1);
        bool found = strcmp(pStr, search->str.getStr()) == 0;
        if (found)
            search->index = gtk_tree_path_get_indices(path)[0];
        g_free(pStr);
        return found;
    }
}

class GtkInstanceTreeView : public GtkInstanceContainer, public virtual Weld::TreeView
{
private:
    GtkTreeView* m_pTreeView;
    GtkListStore* m_pListStore;
    gulong m_nChangedSignalId;
    gulong m_nRowActivatedSignalId;

    static void signalChanged(GtkTreeView*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_changed();
    }

    static void signalRowActivated(GtkTreeView*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_row_activated();
    }
public:
    GtkInstanceTreeView(GtkTreeView* pTreeView, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pTreeView), bTakeOwnership)
        , m_pTreeView(pTreeView)
        , m_pListStore(GTK_LIST_STORE(gtk_tree_view_get_model(m_pTreeView)))
        , m_nChangedSignalId(g_signal_connect(gtk_tree_view_get_selection(pTreeView), "changed",
                             G_CALLBACK(signalChanged), this))
        , m_nRowActivatedSignalId(g_signal_connect(pTreeView, "row-activated", G_CALLBACK(signalChanged), this))
    {
    }

    virtual void append(const OUString& rText) override
    {
        insert(rText, -1);
    }

    virtual void insert(const OUString& rText, int pos) override
    {
        GtkTreeIter iter;
        gtk_list_store_insert(m_pListStore, &iter, pos);
        gtk_list_store_set(m_pListStore, &iter, 0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(), -1);
    }

    virtual void remove(int pos) override
    {
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);
        gtk_list_store_remove(m_pListStore, &iter);
    }

    virtual int find(const OUString& rText) const override
    {
        Search aSearch(rText);
        gtk_tree_model_foreach(GTK_TREE_MODEL(m_pListStore), (GtkTreeModelForeachFunc)foreach_find, &aSearch);
        return aSearch.index;
    }

    void move_before(int pos, int before)
    {
        if (pos == before)
            return;

        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);

        GtkTreeIter position;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &position, nullptr, before);

        gtk_list_store_move_before(m_pListStore, &iter, &position);
    }

    virtual void set_top_entry(int pos) override
    {
        move_before(pos, 0);
    }

    virtual void clear() override
    {
        gtk_list_store_clear(m_pListStore);
    }

    virtual int n_children() const override
    {
        return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_pListStore), nullptr);
    }

    virtual void select(int pos) override
    {
        if (pos != -1)
        {
            GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
            gtk_tree_selection_select_path(gtk_tree_view_get_selection(m_pTreeView), path);
            gtk_tree_path_free(path);
        }
        else
        {
            gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(m_pTreeView));
        }
    }

    virtual OUString get_selected() override
    {
        OUString sRet;
        GtkTreeIter iter;
        GtkTreeModel* pModel;
        if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(m_pTreeView), &pModel, &iter))
        {
            gchar *pStr = nullptr;
            gtk_tree_model_get(pModel, &iter, 0, &pStr, -1);
            sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

    virtual OUString get(int pos) override
    {
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);
        gchar *pStr = nullptr;
        gtk_tree_model_get(GTK_TREE_MODEL(m_pListStore), &iter, 0, &pStr, -1);
        OUString sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);
        return sRet;
    }

    virtual int get_selected_index() override
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
        g_object_ref(m_pListStore);
        gtk_tree_view_set_model(m_pTreeView, nullptr);
    }

    virtual void thaw() override
    {
        gtk_tree_view_set_model(m_pTreeView, GTK_TREE_MODEL(m_pListStore));
        g_object_unref(m_pListStore);
    }

    virtual int get_height_rows(int nRows) const override
    {
        GtkTreeViewColumn* pColumn = gtk_tree_view_get_column(m_pTreeView, 0);
        GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
        GtkCellRenderer* pRenderer = GTK_CELL_RENDERER(g_list_nth_data(pRenderers, 0));
        gint nRowHeight;
        gtk_cell_renderer_get_preferred_height(pRenderer, GTK_WIDGET(m_pTreeView), nullptr, &nRowHeight);
        g_list_free(pRenderers);

        gint nVerticalSeparator;
        gtk_widget_style_get(GTK_WIDGET(m_pTreeView), "vertical-separator", &nVerticalSeparator, nullptr);

        return (nRowHeight * nRows) + (nVerticalSeparator * (nRows + 1));
    }

    virtual ~GtkInstanceTreeView() override
    {
        g_signal_handler_disconnect(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nRowActivatedSignalId);
    }
};


class GtkInstanceSpinButton : public GtkInstanceEntry, public virtual Weld::SpinButton
{
private:
    GtkSpinButton* m_pButton;
    gulong m_nValueChangedSignalId;
    gulong m_nOutputSignalId;

    static void signalValueChanged(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        pThis->signal_value_changed();
    }

    static gboolean signalOutput(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        return pThis->signal_output();
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
    {
    }

    virtual int get_value() const override
    {
        return fromGtk(gtk_spin_button_get_value(m_pButton));
    }

    virtual void set_value(int value) override
    {
        gtk_spin_button_set_value(m_pButton, toGtk(value));
    }

    virtual void set_range(int min, int max) override
    {
        gtk_spin_button_set_range(m_pButton, toGtk(min), toGtk(max));
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
        gtk_spin_button_set_increments(m_pButton, toGtk(step), toGtk(page));
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
        gtk_spin_button_set_digits(m_pButton, digits);
    }

    virtual unsigned int get_digits() const override
    {
        return gtk_spin_button_get_digits(m_pButton);
    }

    virtual ~GtkInstanceSpinButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nOutputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nValueChangedSignalId);
    }
};

class GtkInstanceLabel : public GtkInstanceWidget, public virtual Weld::Label
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
        gtk_label_set_label(m_pLabel, MapToGtkAccelerator(rText).getStr());
    }
};

class GtkInstanceTextView : public GtkInstanceContainer, public virtual Weld::TextView
{
private:
    GtkTextView* m_pTextView;
public:
    GtkInstanceTextView(GtkTextView* pTextView, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pTextView), bTakeOwnership)
        , m_pTextView(pTextView)
    {
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

    virtual Selection get_selection() const override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_selection_bounds(pBuffer, &start, &end);
        return Selection(gtk_text_iter_get_offset(&start), gtk_text_iter_get_offset(&end));
    }

    virtual void set_selection(const Selection& rSelection) override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_iter_at_offset(pBuffer, &start, rSelection.Min());
        gtk_text_buffer_get_iter_at_offset(pBuffer, &end, rSelection.Max());
        gtk_text_buffer_select_range(pBuffer, &start, &end);
        GtkTextMark* mark = gtk_text_buffer_create_mark(pBuffer, "scroll", &end, true);
        gtk_text_view_scroll_mark_onscreen(m_pTextView, mark);
    }

};

class GtkInstanceDrawingArea : public GtkInstanceWidget, public virtual Weld::DrawingArea
{
private:
    GtkDrawingArea* m_pDrawingArea;
    ScopedVclPtrInstance<VirtualDevice> m_xDevice;
    std::vector<unsigned char> m_aBuffer;
    cairo_surface_t* m_pSurface;
    gulong m_nDrawSignalId;
    gulong m_nSizeAllocateSignalId;
    static gboolean signalDraw(GtkWidget*, cairo_t* cr, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        pThis->signal_draw(cr);
        return false;
    }
    static void signalSizeAllocate(GtkWidget*, GdkRectangle* allocation, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        pThis->signal_size_allocate(allocation->width, allocation->height);
    }
    void signal_size_allocate(guint nWidth, guint nHeight)
    {
        if (m_pSurface)
            cairo_surface_destroy(m_pSurface);

        const int nScale = gtk_widget_get_scale_factor(GTK_WIDGET(m_pDrawingArea));
        const int nStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, nWidth * nScale);
        m_aBuffer.resize(nHeight * nScale * nStride);
        m_xDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nWidth, nHeight), Fraction(1.0), Point(),
                                                          m_aBuffer.data());
        m_pSurface = cairo_image_surface_create_for_data(m_aBuffer.data(), CAIRO_FORMAT_ARGB32,
                                                         nWidth * nScale, nHeight * nScale, nStride);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 14, 0)
        cairo_surface_set_device_scale(m_pSurface, nScale, nScale);
#endif
        m_aSizeAllocateHdl.Call(Size(nWidth, nHeight));
    }
    void signal_draw(cairo_t* cr)
    {
        m_aDrawHdl.Call(*m_xDevice);
        cairo_surface_mark_dirty(m_pSurface);

        cairo_set_source_surface(cr, m_pSurface, 0, 0);
        cairo_paint(cr);
    }
public:
    GtkInstanceDrawingArea(GtkDrawingArea* pDrawingArea, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pDrawingArea), bTakeOwnership)
        , m_pDrawingArea(pDrawingArea)
        , m_xDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT)
        , m_pSurface(nullptr)
        , m_nDrawSignalId(g_signal_connect(pDrawingArea, "draw", G_CALLBACK(signalDraw), this))
        , m_nSizeAllocateSignalId(g_signal_connect(pDrawingArea, "size_allocate", G_CALLBACK(signalSizeAllocate), this))
    {
    }

    virtual void queue_draw() override
    {
        gtk_widget_queue_draw(GTK_WIDGET(m_pDrawingArea));
    }

    virtual ~GtkInstanceDrawingArea() override
    {
        if (m_pSurface)
            cairo_surface_destroy(m_pSurface);
        g_signal_handler_disconnect(m_pDrawingArea, m_nSizeAllocateSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nDrawSignalId);
    }
};

namespace
{
    gint sort_func(GtkTreeModel* pModel, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
    {
        comphelper::string::NaturalStringSorter* pSorter = (comphelper::string::NaturalStringSorter*)user_data;
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

class GtkInstanceComboBoxText : public GtkInstanceContainer, public virtual Weld::ComboBoxText
{
private:
    GtkComboBoxText* m_pComboBoxText;
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    gulong m_nSignalId;

    static void signalChanged(GtkComboBox*, gpointer widget)
    {
        GtkInstanceComboBoxText* pThis = static_cast<GtkInstanceComboBoxText*>(widget);
        pThis->signal_changed();
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
            sRet = OUString(pStr, strlen(pStr), RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

public:
    GtkInstanceComboBoxText(GtkComboBoxText* pComboBoxText, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pComboBoxText), bTakeOwnership)
        , m_pComboBoxText(pComboBoxText)
        , m_nSignalId(g_signal_connect(pComboBoxText, "changed", G_CALLBACK(signalChanged), this))
    {
    }

    virtual int get_active() const override
    {
        return gtk_combo_box_get_active(GTK_COMBO_BOX(m_pComboBoxText));
    }

    virtual OUString get_active_id() const override
    {
        const gchar* pText = gtk_combo_box_get_active_id(GTK_COMBO_BOX(m_pComboBoxText));
        return OUString(pText, strlen(pText), RTL_TEXTENCODING_UTF8);
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        OString aId(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8));
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(m_pComboBoxText), aId.getStr());
    }

    virtual void set_active(int pos) override
    {
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_pComboBoxText), pos);
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

    virtual void append_text(const OUString& rStr) override
    {
        gtk_combo_box_text_append_text(m_pComboBoxText, OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void insert_text(int pos, const OUString& rStr) override
    {
        gtk_combo_box_text_insert_text(m_pComboBoxText, pos, OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void append(const OUString& rId, const OUString& rStr) override
    {
        gtk_combo_box_text_append(m_pComboBoxText,
                                  OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr(),
                                  OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr) override
    {
        gtk_combo_box_text_insert(m_pComboBoxText, pos,
                                  OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr(),
                                  OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual int get_count() const override
    {
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        return gtk_tree_model_iter_n_children(pModel, nullptr);
    }

    virtual int find_text(const OUString& rStr) const override
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
            gtk_tree_model_get(pModel, &iter, 0, &pStr, -1);
            const bool bEqual = strcmp(pStr, aStr.getStr()) == 0;
            g_free(pStr);
            if (bEqual)
                return nRet;
            ++nRet;
        } while (gtk_tree_model_iter_next(pModel, &iter));

        return -1;
    }

    virtual void clear() override
    {
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        gtk_list_store_clear(GTK_LIST_STORE(pModel));
    }

    virtual void make_sorted() override
    {
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetLanguageTag().getLocale()));
        GtkTreeModel* pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
        gtk_tree_sortable_set_sort_func(pSortable, 0, sort_func, m_xSorter.get(), nullptr);
        gtk_tree_sortable_set_sort_column_id(pSortable, 0, GTK_SORT_ASCENDING);
    }

    virtual ~GtkInstanceComboBoxText() override
    {
        g_signal_handler_disconnect(m_pComboBoxText, m_nSignalId);
    }
};

namespace
{
    void fixup_icons(GObject *pObject)
    {
        //wanted: better way to do this, e.g. make gtk use gio for
        //loading from a filename and provide gio protocol handler
        //for our image in a zip urls
        //
        //unpack the images and keep them as dirs and just
        //add the paths to the gtk icon theme dir
        if (GTK_IS_IMAGE(pObject))
        {
            GtkImage* pImage = GTK_IMAGE(pObject);
            const gchar* icon_name;
            gtk_image_get_icon_name(pImage, &icon_name, nullptr);
            GtkIconSize size;
            g_object_get(pImage, "icon-size", &size, nullptr);
            if (icon_name)
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);

                SvMemoryStream aMemStm;
                BitmapEx aBitmap(aIconName);
                vcl::PNGWriter aWriter(aBitmap);
                aWriter.Write(aMemStm);

                GdkPixbufLoader *pixbuf_loader = gdk_pixbuf_loader_new();
                gdk_pixbuf_loader_write(pixbuf_loader, (const guchar*)aMemStm.GetData(),
                                        aMemStm.Seek(STREAM_SEEK_TO_END), nullptr);
                gdk_pixbuf_loader_close(pixbuf_loader, nullptr);
                GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);

                gtk_image_set_from_pixbuf(pImage, pixbuf);
                g_object_unref(pixbuf_loader);
            }
        }
    }
}

class GtkInstanceBuilder : public Weld::Builder
{
private:
    GtkBuilder* m_pBuilder;
public:
    GtkInstanceBuilder(const OUString& rUIRoot, const OUString& rUIFile)
        : Weld::Builder(rUIFile)
    {
        OUString aUri(rUIRoot + rUIFile);
        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aUri, aPath);
        m_pBuilder = gtk_builder_new_from_file(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr());

        GSList* pObjectList = gtk_builder_get_objects(m_pBuilder);
        g_slist_foreach(pObjectList, (GFunc)fixup_icons, nullptr);
        g_slist_free(pObjectList);
    }

    virtual ~GtkInstanceBuilder() override
    {
        g_object_unref(m_pBuilder);
    }

    virtual Weld::Dialog* weld_dialog(const OString &id) override
    {
        GtkDialog* pDialog = GTK_DIALOG(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pDialog ? new GtkInstanceDialog(pDialog, false) : nullptr;
    }

    virtual Weld::Window* weld_window(const OString &id) override
    {
        GtkWindow* pWindow = GTK_WINDOW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pWindow ? new GtkInstanceWindow(pWindow, false) : nullptr;
    }

    virtual Weld::Widget* weld_widget(const OString &id) override
    {
        GtkWidget* pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pWidget ? new GtkInstanceWidget(pWidget, false) : nullptr;
    }

    virtual Weld::Container* weld_container(const OString &id) override
    {
        GtkContainer* pContainer = GTK_CONTAINER(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pContainer ? new GtkInstanceContainer(pContainer, false) : nullptr;
    }

    virtual Weld::Frame* weld_frame(const OString &id) override
    {
        GtkFrame* pFrame = GTK_FRAME(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pFrame ? new GtkInstanceFrame(pFrame, false) : nullptr;
    }

    virtual Weld::Notebook* weld_notebook(const OString &id) override
    {
        GtkNotebook* pNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pNotebook ? new GtkInstanceNotebook(pNotebook, false) : nullptr;
    }

    virtual Weld::Button* weld_button(const OString &id) override
    {
        GtkButton* pButton = GTK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pButton ? new GtkInstanceButton(pButton, false) : nullptr;
    }

    virtual Weld::RadioButton* weld_radio_button(const OString &id) override
    {
        GtkRadioButton* pRadioButton = GTK_RADIO_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pRadioButton ? new GtkInstanceRadioButton(pRadioButton, false) : nullptr;
    }

    virtual Weld::CheckButton* weld_check_button(const OString &id) override
    {
        GtkCheckButton* pCheckButton = GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pCheckButton ? new GtkInstanceCheckButton(pCheckButton, false) : nullptr;
    }

    virtual Weld::Entry* weld_entry(const OString &id) override
    {
        GtkEntry* pEntry = GTK_ENTRY(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pEntry ? new GtkInstanceEntry(pEntry, false) : nullptr;
    }

    virtual Weld::SpinButton* weld_spin_button(const OString &id) override
    {
        GtkSpinButton* pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pSpinButton ? new GtkInstanceSpinButton(pSpinButton, false) : nullptr;
    }

    virtual Weld::ComboBoxText* weld_combo_box_text(const OString &id) override
    {
        GtkComboBoxText* pComboBoxText = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pComboBoxText ? new GtkInstanceComboBoxText(pComboBoxText, false) : nullptr;
    }

    virtual Weld::TreeView* weld_tree_view(const OString &id) override
    {
        GtkTreeView* pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pTreeView ? new GtkInstanceTreeView(pTreeView, false) : nullptr;
    }

    virtual Weld::Label* weld_label(const OString &id) override
    {
        GtkLabel* pLabel = GTK_LABEL(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pLabel ? new GtkInstanceLabel(pLabel, false) : nullptr;
    }

    virtual Weld::TextView* weld_text_view(const OString &id) override
    {
        GtkTextView* pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pTextView ? new GtkInstanceTextView(pTextView, false) : nullptr;
    }

    virtual Weld::DrawingArea* weld_drawing_area(const OString &id) override
    {
        GtkDrawingArea* pDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pDrawingArea ? new GtkInstanceDrawingArea(pDrawingArea, false) : nullptr;
    }
};

Weld::Builder* GtkInstance::CreateBuilder(const OUString& rUIRoot, const OUString& rUIFile)
{
    return new GtkInstanceBuilder(rUIRoot, rUIFile);
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

Weld::Dialog* GtkInstance::CreateMessageDialog(Weld::Window *pParent, VclMessageType eMessageType, VclButtonsType eButtonsType, const OUString &rPrimaryMessage)
{
    GtkInstanceWindow* pParentInstance = dynamic_cast<GtkInstanceWindow*>(pParent);
    GtkWindow* pParentWidget = pParentInstance ? pParentInstance->getWindow() : nullptr;
    GtkDialog* pDialog = GTK_DIALOG(gtk_message_dialog_new(pParentWidget, GTK_DIALOG_MODAL, VclToGtk(eMessageType), VclToGtk(eButtonsType),
                                                           "%s", OUStringToOString(rPrimaryMessage, RTL_TEXTENCODING_UTF8).getStr()));
    return new GtkInstanceDialog(pDialog, true);
}

Weld::Window* GtkSalFrame::GetFrameWeld() const
{
    if (!m_xFrameWeld)
        m_xFrameWeld.reset(new GtkInstanceWindow(GTK_WINDOW(getWindow()), false));
    return m_xFrameWeld.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
