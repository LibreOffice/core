/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unohelp2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:15:28 $
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

#ifndef _VCL_UNOHELP2_HXX
#define _VCL_UNOHELP2_HXX

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

namespace com { namespace sun { namespace star { namespace datatransfer { namespace clipboard {
    class XClipboard;
} } } } }

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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

