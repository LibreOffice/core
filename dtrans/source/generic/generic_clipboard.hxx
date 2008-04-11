/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: generic_clipboard.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _DTRANS_GENERIC_CLIPBOARD_HXX_
#define _DTRANS_GENERIC_CLIPBOARD_HXX_

#include <cppuhelper/compbase4.hxx>

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

// ------------------------------------------------------------------------

#define GENERIC_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.clipboard.GenericClipboard"

// ------------------------------------------------------------------------

namespace dtrans
{

    class GenericClipboard : public ::cppu::WeakComponentImplHelper4 < \
    ::com::sun::star::datatransfer::clipboard::XClipboardEx, \
    ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, \
    ::com::sun::star::lang::XServiceInfo, \
    ::com::sun::star::lang::XInitialization >
    {
        ::osl::Mutex m_aMutex;
        ::rtl::OUString m_aName;

        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > m_aContents;
        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;

        sal_Bool m_bInitialized;
        virtual ~GenericClipboard();

    public:

        GenericClipboard();

        /*
         * XInitialization
         */

        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        /*
         * XServiceInfo
         */

        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboard
         */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
            throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setContents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans,
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL getName()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardNotifier
         */

        virtual void SAL_CALL addClipboardListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeClipboardListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(::com::sun::star::uno::RuntimeException);

    };

}

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL GenericClipboard_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GenericClipboard_createInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif
