/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: diagnose_ex.h,v $
 * $Revision: 1.4 $
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

#ifndef TOOLS_DIAGNOSE_EX_H
#define TOOLS_DIAGNOSE_EX_H

#define OSL_UNUSED( expression ) \
    (void)(expression)

#if OSL_DEBUG_LEVEL > 0

    #include <cppuhelper/exc_hlp.hxx>
    #include <osl/diagnose.h>
    #include <osl/thread.h>
    #include <boost/current_function.hpp>
    #include <typeinfo>

    /** reports a caught UNO exception via OSL diagnostics

        Note that whenever you use this, it might be an indicator that your error
        handling is not correct ....
    */
    #define DBG_UNHANDLED_EXCEPTION()   \
        ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() ); \
        ::rtl::OString sMessage( "caught an exception!" ); \
        sMessage += "\nin function:"; \
        sMessage += BOOST_CURRENT_FUNCTION; \
        sMessage += "\ntype: "; \
        sMessage += ::rtl::OString( caught.getValueTypeName().getStr(), caught.getValueTypeName().getLength(), osl_getThreadTextEncoding() ); \
        ::com::sun::star::uno::Exception exception; \
        caught >>= exception; \
        if ( exception.Message.getLength() ) \
        { \
            sMessage += "\nmessage: "; \
            sMessage += ::rtl::OString( exception.Message.getStr(), exception.Message.getLength(), osl_getThreadTextEncoding() ); \
        } \
        if ( exception.Context.is() ) \
        { \
            const char* pContext = typeid( *exception.Context.get() ).name(); \
            sMessage += "\ncontext: "; \
            sMessage += pContext; \
        } \
        sMessage += "\n"; \
        OSL_ENSURE( false, sMessage )

#else   // OSL_DEBUG_LEVEL

    #define DBG_UNHANDLED_EXCEPTION()

#endif  // OSL_DEBUG_LEVEL

#endif // TOOLS_DIAGNOSE_EX_H
