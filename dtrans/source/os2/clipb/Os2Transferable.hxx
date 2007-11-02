/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Os2Transferable.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:15:31 $
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

#ifndef _DTRANS_OS2_TRANSFERABLE_HXX_
#define _DTRANS_OS2_TRANSFERABLE_HXX_

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HDL_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#include <errno.h>

#include <uclip.h>

#define CHAR_POINTER(THE_OUSTRING) ::rtl::OUStringToOString (THE_OUSTRING, RTL_TEXTENCODING_UTF8).pData->buffer

#if OSL_DEBUG_LEVEL>1
extern "C" int debug_printf(const char *f, ...);
#else
#define debug_printf( ...) { 1; }
#endif

#define CPPUTYPE_SEQSALINT8       getCppuType( (const Sequence< sal_Int8 >*) 0 )
#define CPPUTYPE_DEFAULT          CPPUTYPE_SEQSALINT8

#ifdef OOO_VENDOR
using namespace com::sun::star::uno;
HBITMAP OOoBmpToOS2Handle( Any &aAnyB);
int OS2HandleToOOoBmp( HBITMAP hbm, Sequence< sal_Int8 >* winDIBStream);
#else
#define OOoBmpToOS2Handle(a)    0
#define OS2HandleToOOoBmp(a,b)  0
#endif

namespace os2 {

    class Os2Transferable : public ::cppu::WeakImplHelper1 <
        ::com::sun::star::datatransfer::XTransferable >
    {
        HAB         hAB;
        ::rtl::OUString clipText;
        ::com::sun::star::datatransfer::DataFlavor aFlavor;
        ::osl::Mutex m_aMutex;
        ::com::sun::star::uno::Reference< XInterface >          m_xCreator;

    public:
        Os2Transferable( const ::com::sun::star::uno::Reference< XInterface >& xCreator);
        virtual ~Os2Transferable();

        /*
         * XTransferable
         */

        virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
            throw(::com::sun::star::datatransfer::UnsupportedFlavorException,
                  ::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException
                  );

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
            throw(::com::sun::star::uno::RuntimeException);
    };

} // namespace

#endif

