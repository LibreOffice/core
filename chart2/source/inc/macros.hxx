/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macros.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:25:29 $
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
#ifndef CHART_MACROS_HXX
#define CHART_MACROS_HXX

#include <typeinfo>

/// creates a unicode-string from an ASCII string
#define C2U(constAsciiStr) (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) ))

/** shows an error-box for an exception ex
    else-branch necessary to avoid warning
*/
#if OSL_DEBUG_LEVEL > 0
#define ASSERT_EXCEPTION(ex)                   \
  OSL_ENSURE( false, ::rtl::OUStringToOString( \
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +\
    ::rtl::OUString::createFromAscii( typeid( ex ).name()) +\
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +\
    ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr())
#else
//avoid compilation warnings
#define ASSERT_EXCEPTION(ex) ex.Context.is();
#endif

#define U2C(ouString) (::rtl::OUStringToOString(ouString,RTL_TEXTENCODING_ASCII_US).getStr())

// CHART_MACROS_HXX
#endif
