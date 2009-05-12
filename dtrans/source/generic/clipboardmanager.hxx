/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: clipboardmanager.hxx,v $
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

#ifndef _DTRANS_CLIPBOARDMANAGER_HXX_
#define _DTRANS_CLIPBOARDMANAGER_HXX_

#include <cppuhelper/compbase3.hxx>

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDMANAGER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardManager.hpp>
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <map>

// ------------------------------------------------------------------------

#define CLIPBOARDMANAGER_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.ClipboardManager"

// ------------------------------------------------------------------------

typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > > ClipboardMap;

// ------------------------------------------------------------------------

namespace dtrans
{

    class ClipboardManager : public ::cppu::WeakComponentImplHelper3 < \
    ::com::sun::star::datatransfer::clipboard::XClipboardManager, \
    ::com::sun::star::lang::XEventListener, \
    ::com::sun::star::lang::XServiceInfo >
    {
        ClipboardMap m_aClipboardMap;
        ::osl::Mutex m_aMutex;

        const ::rtl::OUString m_aDefaultName;

        virtual ~ClipboardManager();
    protected:
        using WeakComponentImplHelperBase::disposing;
    public:

        ClipboardManager();

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
         * XComponent
         */

        virtual void SAL_CALL dispose()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XEventListener
         */

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardManager
         */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const ::rtl::OUString& aName )
            throw(::com::sun::star::container::NoSuchElementException,
                  ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL addClipboard( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& xClipboard )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeClipboard( const ::rtl::OUString& aName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL listClipboardNames(  )
            throw(::com::sun::star::uno::RuntimeException);


    };

}

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ClipboardManager_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ClipboardManager_createInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif
