/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagnose_ex.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:10:07 $
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

#ifndef TOOLS_DIAGNOSE_EX_H
#define TOOLS_DIAGNOSE_EX_H

#define OSL_UNUSED( expression ) \
    (void)(expression)

#if OSL_DEBUG_LEVEL > 0

    #ifndef _CPPUHELPER_EXC_HLP_HXX_
    #include <cppuhelper/exc_hlp.hxx>
    #endif
    #ifndef _OSL_DIAGNOSE_H_
    #include <osl/diagnose.h>
    #endif
    #ifndef _OSL_THREAD_H_
    #include <osl/thread.h>
    #endif
    #include <boost/current_function.hpp>

    /** reports a caught UNO exception via OSL diagnostics

        Note that whenever you use this, it might be an indicator that your error
        handling is not correct ....
    */
    #define DBG_UNHANDLED_EXCEPTION()   \
        ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() ); \
        ::rtl::OString sMessage( "caught an exception!" ); \
        sMessage += "\ntype   : "; \
        sMessage += ::rtl::OString( caught.getValueTypeName().getStr(), caught.getValueTypeName().getLength(), osl_getThreadTextEncoding() ); \
        sMessage += "\nmessage: "; \
        ::com::sun::star::uno::Exception exception; \
        caught >>= exception; \
        sMessage += ::rtl::OString( exception.Message.getStr(), exception.Message.getLength(), osl_getThreadTextEncoding() ); \
        sMessage += "\nin function:\n"; \
        sMessage += BOOST_CURRENT_FUNCTION; \
        sMessage += "\n"; \
        OSL_ENSURE( false, sMessage )

#else   // OSL_DEBUG_LEVEL

    #define DBG_UNHANDLED_EXCEPTION()

#endif  // OSL_DEBUG_LEVEL

#endif // TOOLS_DIAGNOSE_EX_H
