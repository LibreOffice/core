/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: generic_clipboard.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 17:16:43 $
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

#ifndef _DTRANS_GENERIC_CLIPBOARD_HXX_
#define _DTRANS_GENERIC_CLIPBOARD_HXX_

#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

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
