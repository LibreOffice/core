/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unohelp2.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _VCL_UNOHELP2_HXX
#define _VCL_UNOHELP2_HXX

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <cppuhelper/weak.hxx>
#include <tools/string.hxx>
#include <osl/mutex.hxx>

namespace com { namespace sun { namespace star { namespace datatransfer { namespace clipboard {
    class XClipboard;
} } } } }
#include <vcl/dllapi.h>

namespace vcl { namespace unohelper {

    class VCL_DLLPUBLIC TextDataObject :
                            public ::com::sun::star::datatransfer::XTransferable,
                            public ::cppu::OWeakObject
    {
    private:
        String          maText;

    public:
                        TextDataObject( const String& rText );
                        ~TextDataObject();

        String&         GetString() { return maText; }

        // ::com::sun::star::uno::XInterface
        ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
        void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

        // ::com::sun::star::datatransfer::XTransferable
        ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw(::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(::com::sun::star::uno::RuntimeException);
        sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw(::com::sun::star::uno::RuntimeException);

        /// copies a given string to a given clipboard
        static  void    CopyStringTo(
            const String& rContent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard
        );
    };

    struct MutexHelper
    {
        private:
            ::osl::Mutex maMutex;
        public:
            ::osl::Mutex& GetMutex() { return maMutex; }
    };

}}  // namespace vcl::unohelper

#endif  // _VCL_UNOHELP2_HXX

