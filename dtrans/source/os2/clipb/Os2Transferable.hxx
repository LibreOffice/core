/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _DTRANS_OS2_TRANSFERABLE_HXX_
#define _DTRANS_OS2_TRANSFERABLE_HXX_

#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/thread.h>

#include <errno.h>

#include <uclip.h>

#define CHAR_POINTER(THE_OUSTRING) ::rtl::OUStringToOString (THE_OUSTRING, RTL_TEXTENCODING_UTF8).pData->buffer

#if OSL_DEBUG_LEVEL > 1
//extern "C" int debug_printf(const char *f, ...);
#define debug_printf( ...) { 1; }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
