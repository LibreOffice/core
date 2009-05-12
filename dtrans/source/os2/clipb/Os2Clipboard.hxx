/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Os2Clipboard.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _OS2CLIPBOARD_HXX_
#define _OS2CLIPBOARD_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include "Os2Transferable.hxx"

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

