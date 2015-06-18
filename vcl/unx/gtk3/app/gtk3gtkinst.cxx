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
}

class GtkTransferable : public ::cppu::WeakImplHelper1 <
    css::datatransfer::XTransferable >
{
public:
    GtkTransferable()
    {
    }

    virtual ~GtkTransferable()
    {
    }

    /*
     * XTransferable
     */

    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor )
        throw(css::datatransfer::UnsupportedFlavorException,
              css::io::IOException,
              css::uno::RuntimeException, std::exception
              ) SAL_OVERRIDE
    {
        css::uno::Any aRet;
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        if (aFlavor.MimeType == "text/plain;charset=utf-16")
        {
            gchar *pText = gtk_clipboard_wait_for_text(clipboard);
            OUString aStr(pText, rtl_str_getLength(pText),
                RTL_TEXTENCODING_UTF8);
            g_free(pText);
            aRet <<= aStr.replaceAll("\r\n", "\n");
        }
        else
            fprintf(stderr, "TO-DO getTransferData %s\n", OUStringToOString(aFlavor.MimeType, RTL_TEXTENCODING_UTF8).getStr());
        return aRet;
    }

    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

        std::vector<css::datatransfer::DataFlavor> aVector;
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

        return comphelper::containerToSequence(aVector);
    }

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor )
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        fprintf(stderr, "TODO isDataFlavorSupported\n");

        (void)aFlavor;
        return false;
    }
};

class VclGtkClipboard :
        public cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        XServiceInfo
        >
{
    osl::Mutex                                                              m_aMutex;
    Reference< css::datatransfer::XTransferable >              m_aContents;
    Reference< css::datatransfer::clipboard::XClipboardOwner > m_aOwner;
    std::list< Reference< css::datatransfer::clipboard::XClipboardListener > > m_aListeners;

public:

    VclGtkClipboard() : cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        XServiceInfo
        >( m_aMutex )
    {
    }
    virtual ~VclGtkClipboard();

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
};

VclGtkClipboard::~VclGtkClipboard()
{
}

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
