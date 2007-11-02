/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Os2Clipboard.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:15:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OS2CLIPBOARD_HXX_
#define _OS2CLIPBOARD_HXX_

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDOWNER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_RENDERINGCAPABILITIES_HPP_
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#endif

#ifndef _DTRANS_OS2_TRANSFERABLE_HXX_
#include "Os2Transferable.hxx"
#endif

// the service names
#define OS2_CLIPBOARD_SERVICE_NAME "com.sun.star.datatransfer.clipboard.SystemClipboard"

// the implementation names
#define OS2_CLIPBOARD_IMPL_NAME "com.sun.star.datatransfer.clipboard.Os2Clipboard"

// the registry key names
#define OS2_CLIPBOARD_REGKEY_NAME "/com.sun.star.datatransfer.clipboard.Os2Clipboard/UNO/SERVICES/com.sun.star.datatransfer.clipboard.SystemClipboard"

namespace os2 {

class Os2Clipboard :
    //public cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::clipboard::XClipboardEx, ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, ::com::sun::star::lang::XServiceInfo >
    public ::cppu::WeakComponentImplHelper4 < \
    ::com::sun::star::datatransfer::clipboard::XClipboardEx, \
    ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, \
    ::com::sun::star::lang::XServiceInfo, \
    ::com::sun::star::lang::XInitialization >
{

public:
    Os2Clipboard();
    ~Os2Clipboard();

    /*
     * XInitialization
     */
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    /*
     * XServiceInfo
     */
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw(::com::sun::star::uno::RuntimeException);

    /*
     * XClipboard
     */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getName()
        throw( ::com::sun::star::uno::RuntimeException );

    /*
     * XClipboardEx
     */
    virtual sal_Int8 SAL_CALL getRenderingCapabilities()
        throw( ::com::sun::star::uno::RuntimeException );

    /*
     * XClipboardNotifier
     */
    virtual void SAL_CALL addClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL notifyAllClipboardListener( );

public:
    sal_Bool m_bInSetClipboardData;

private:
    HAB hAB;
    HWND    hObjWnd;
    ULONG   hText, hBitmap; // handles to previous clipboard data

    ::osl::Mutex m_aMutex;
    ::rtl::OUString m_aName;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > m_aContents;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;

    sal_Bool m_bInitialized;

};

} // namespace Os2

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL Os2Clipboard_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Os2Clipboard_createInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif

