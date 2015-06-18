/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../gtk/app/gtkinst.cxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/datatransfer/XTransferable.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardEx.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardListener.hpp"
#include "com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/datatransfer/dnd/DNDConstants.hpp"
#include <comphelper/sequence.hxx>
#include "cppuhelper/compbase.hxx"
#include "cppuhelper/implbase1.hxx"
#include <cppuhelper/supportsservice.hxx>

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

    static TypeEntry aConversionTab[] =
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

    class DataFlavorEq : public std::unary_function<const css::datatransfer::DataFlavor&, bool>
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

class GtkTransferable : public ::cppu::WeakImplHelper1 <
    css::datatransfer::XTransferable >
{
private:
    std::map<OUString, GdkAtom> m_aMimeTypeToAtom;
public:
    /*
     * XTransferable
     */

    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor)
        throw(css::datatransfer::UnsupportedFlavorException,
              css::io::IOException,
              css::uno::RuntimeException, std::exception
              ) SAL_OVERRIDE
    {
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        if (rFlavor.MimeType == "text/plain;charset=utf-16")
        {
            gchar *pText = gtk_clipboard_wait_for_text(clipboard);
            OUString aStr(pText, rtl_str_getLength(pText),
                RTL_TEXTENCODING_UTF8);
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
        gint length;
        const guchar *rawdata = gtk_selection_data_get_data_with_length(data,
                                                                        &length);
        Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(rawdata), length);
        gtk_selection_data_free(data);
        aRet <<= aSeq;
        return aRet;
    }

    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector()
    {
        std::vector<css::datatransfer::DataFlavor> aVector;

        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

        GdkAtom *targets;
        gint n_targets;
        if (gtk_clipboard_wait_for_targets(clipboard, &targets, &n_targets))
        {
            bool bHaveText = false, bHaveUTF16 = false;

            for (gint i = 0; i < n_targets; ++i)
            {
                gchar* pName = gdk_atom_name(targets[i]);
                const char* pFinalName = pName;
                css::datatransfer::DataFlavor aFlavor;

                for (size_t j = 0; j < SAL_N_ELEMENTS(aConversionTab); ++j)
                {
                    if (rtl_str_compare(pName, aConversionTab[j].pNativeType) == 0)
                    {
                        pFinalName = aConversionTab[j].pType;
                        break;
                    }
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

            g_free(targets);

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
        }

        return aVector;
    }

    virtual css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors()
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return comphelper::containerToSequence(getTransferDataFlavorsAsVector());
    }

    virtual sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        const std::vector<css::datatransfer::DataFlavor> aAll =
            getTransferDataFlavorsAsVector();

        return std::find_if(aAll.begin(), aAll.end(), DataFlavorEq(rFlavor)) != aAll.end();
    }
};

class VclGtkClipboard :
        public cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        XServiceInfo>
{
    osl::Mutex                                               m_aMutex;
    Reference<css::datatransfer::XTransferable>              m_aContents;
    Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::list< Reference<css::datatransfer::clipboard::XClipboardListener> > m_aListeners;
    std::vector<GtkTargetEntry> m_aGtkTargets;
    std::vector<css::datatransfer::DataFlavor> m_aInfoToFlavor;

public:

    VclGtkClipboard() : cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        XServiceInfo
        >( m_aMutex )
    {
    }

    virtual ~VclGtkClipboard()
    {
    }

    /*
     * XServiceInfo
     */

    virtual OUString SAL_CALL getImplementationName() throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( RuntimeException, std::exception ) SAL_OVERRIDE;

    static OUString getImplementationName_static();
    static Sequence< OUString > getSupportedServiceNames_static();

    /*
     * XClipboard
     */

    virtual Reference< css::datatransfer::XTransferable > SAL_CALL getContents()
        throw(RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setContents(
        const Reference< css::datatransfer::XTransferable >& xTrans,
        const Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
        throw(RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getName()
        throw(RuntimeException, std::exception) SAL_OVERRIDE;

    /*
     * XClipboardEx
     */

    virtual sal_Int8 SAL_CALL getRenderingCapabilities()
        throw(RuntimeException, std::exception) SAL_OVERRIDE;

    /*
     * XClipboardNotifier
     */
    virtual void SAL_CALL addClipboardListener(
        const Reference< css::datatransfer::clipboard::XClipboardListener >& listener )
        throw(RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeClipboardListener(
        const Reference< css::datatransfer::clipboard::XClipboardListener >& listener )
        throw(RuntimeException, std::exception) SAL_OVERRIDE;

    void ClipboardGet(GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info);
    void ClipboardClear(GtkClipboard *clipboard);
private:
    GtkTargetEntry makeGtkTargetEntry(const css::datatransfer::DataFlavor& rFlavor);
};

OUString VclGtkClipboard::getImplementationName_static()
{
    return OUString( "com.sun.star.datatransfer.VclGtkClipboard"  );
}

Sequence< OUString > VclGtkClipboard::getSupportedServiceNames_static()
{
    Sequence< OUString > aRet(1);
    aRet[0] = "com.sun.star.datatransfer.clipboard.SystemClipboard";
    return aRet;
}

OUString VclGtkClipboard::getImplementationName() throw( RuntimeException, std::exception )
{
    return getImplementationName_static();
}

Sequence< OUString > VclGtkClipboard::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    return getSupportedServiceNames_static();
}

sal_Bool VclGtkClipboard::supportsService( const OUString& ServiceName ) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

Reference< css::datatransfer::XTransferable > VclGtkClipboard::getContents() throw( RuntimeException, std::exception )
{
    if (! m_aContents.is())
        m_aContents = new GtkTransferable();

    return m_aContents;
}

namespace
{
    void ClipboardGetFunc(GtkClipboard *clipboard, GtkSelectionData *selection_data,
                          guint info,
                          gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardGet(clipboard, selection_data, info);
    }

    void ClipboardClearFunc(GtkClipboard *clipboard, gpointer user_data_or_owner)
    {
        VclGtkClipboard* pThis = static_cast<VclGtkClipboard*>(user_data_or_owner);
        pThis->ClipboardClear(clipboard);
    }
}

void VclGtkClipboard::ClipboardGet(GtkClipboard* /*clipboard*/, GtkSelectionData *selection_data,
                                   guint info)
{

    GdkAtom type(gdk_atom_intern(OUStringToOString(m_aInfoToFlavor[info].MimeType,
                                                   RTL_TEXTENCODING_UTF8).getStr(),
                                 false));

    css::datatransfer::DataFlavor aFlavor(m_aInfoToFlavor[info]);
    if (aFlavor.MimeType == "UTF8_STRING" || aFlavor.MimeType == "STRING")
        aFlavor.MimeType = "text/plain;charset=utf-8";

    Sequence<sal_Int8> aData;
    Any aValue;

    try
    {
        aValue = m_aContents->getTransferData(aFlavor);
    }
    catch(...)
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
            aValue = m_aContents->getTransferData(aFlavor);
        }
        catch(...)
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

void VclGtkClipboard::ClipboardClear(GtkClipboard * /*clipboard*/)
{
    for (auto &a : m_aGtkTargets)
        free(a.target);
    m_aGtkTargets.clear();
}

GtkTargetEntry VclGtkClipboard::makeGtkTargetEntry(const css::datatransfer::DataFlavor& rFlavor)
{
    GtkTargetEntry aEntry;
    aEntry.target =
        g_strdup(OUStringToOString(rFlavor.MimeType, RTL_TEXTENCODING_UTF8).getStr());
    aEntry.flags = 0;
    auto it = std::find_if(m_aInfoToFlavor.begin(), m_aInfoToFlavor.end(),
                        DataFlavorEq(rFlavor));
    if (it != m_aInfoToFlavor.end())
        aEntry.info = std::distance(m_aInfoToFlavor.begin(), it);
    else
    {
        aEntry.info = m_aInfoToFlavor.size();
        m_aInfoToFlavor.push_back(rFlavor);
    }
    return aEntry;
}

void VclGtkClipboard::setContents(
        const Reference< css::datatransfer::XTransferable >& xTrans,
        const Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
    throw( RuntimeException, std::exception )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );
    Reference< datatransfer::clipboard::XClipboardOwner > xOldOwner( m_aOwner );
    Reference< datatransfer::XTransferable > xOldContents( m_aContents );
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    std::list< Reference< datatransfer::clipboard::XClipboardListener > > xListeners( m_aListeners );
    datatransfer::clipboard::ClipboardEvent aEv;
    aEv.Contents = m_aContents;

    if (m_aContents.is())
    {
        css::uno::Sequence<css::datatransfer::DataFlavor> aFormats = xTrans->getTransferDataFlavors();
        std::vector<GtkTargetEntry> aGtkTargets;
        bool bHaveText(false), bHaveUTF8(false);
        for (int i = 0; i < aFormats.getLength(); ++i)
        {
            const css::datatransfer::DataFlavor& rFlavor = aFormats[i];

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

        //if there was a previous gtk_clipboard_set_with_data call then
        //ClipboardClearFunc will be called now
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        gtk_clipboard_set_with_data(clipboard, aGtkTargets.data(), aGtkTargets.size(),
                                    ClipboardGetFunc, ClipboardClearFunc, this);
        m_aGtkTargets = aGtkTargets;
    }

    aGuard.clear();

    if( xOldOwner.is() && xOldOwner != xClipboardOwner )
        xOldOwner->lostOwnership( this, xOldContents );
    for( std::list< Reference< datatransfer::clipboard::XClipboardListener > >::iterator it =
         xListeners.begin(); it != xListeners.end() ; ++it )
    {
        (*it)->changedContents( aEv );
    }
}

OUString VclGtkClipboard::getName() throw( RuntimeException, std::exception )
{
    return OUString( "CLIPBOARD"  );
}

sal_Int8 VclGtkClipboard::getRenderingCapabilities() throw( RuntimeException, std::exception )
{
    return 0;
}

void VclGtkClipboard::addClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
    throw( RuntimeException, std::exception )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    m_aListeners.push_back( listener );
}

void VclGtkClipboard::removeClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
    throw( RuntimeException, std::exception )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    m_aListeners.remove( listener );
}

Reference< XInterface > GtkInstance::CreateClipboard( const Sequence< Any >& )
{
    return Reference< XInterface >( static_cast<cppu::OWeakObject *>(new VclGtkClipboard()) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
