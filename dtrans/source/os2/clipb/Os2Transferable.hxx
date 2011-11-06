/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DTRANS_OS2_TRANSFERABLE_HXX_
#define _DTRANS_OS2_TRANSFERABLE_HXX_

#include <com/sun/star/datatransfer/XTransferable.hpp>

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HDL_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#include <cppuhelper/implbase1.hxx>
#include <osl/thread.h>

#include <errno.h>

#include <uclip.h>

#define CHAR_POINTER(THE_OUSTRING) ::rtl::OUStringToOString (THE_OUSTRING, RTL_TEXTENCODING_UTF8).pData->buffer

#if OSL_DEBUG_LEVEL>1
//extern "C" int debug_printf(const char *f, ...);
#define debug_printf( ...) { 1; }
#else
#define debug_printf( ...) { 1; }
#endif

#define CPPUTYPE_SEQSALINT8       getCppuType( (const Sequence< sal_Int8 >*) 0 )
#define CPPUTYPE_DEFAULT          CPPUTYPE_SEQSALINT8

using namespace com::sun::star::uno;
HBITMAP OOoBmpToOS2Handle( Any &aAnyB);
int OS2HandleToOOoBmp( HBITMAP hbm, Sequence< sal_Int8 >* winDIBStream);

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

