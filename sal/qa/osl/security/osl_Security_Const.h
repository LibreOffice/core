/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: osl_Security_Const.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:34:06 $
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

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _OSL_SECURITY_CONST_H_
#define _OSL_SECURITY_CONST_H_

#if ( defined WNT )                     // Windows
//#define UNICODE
#include <tools/prewin.h>
// #include <windows.h>
#include <io.h>
#include <tools/postwin.h>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _OSL_SECURITY_HXX_
#include <osl/security.hxx>
#endif

#include <stdlib.h>
#include <stdio.h>

#if ( defined UNX ) || ( defined OS2 )
#include <unistd.h>
#include <pwd.h>
#endif

#include <cppunit/simpleheader.hxx>
// LLA: #include <testshl2/cmdlinebits.hxx>


#define BUFSIZE 1024
const char pTestString[17] = "Sun Microsystems";


#define OSLTEST_DECLARE_USTRING( str_name, str_value ) \
    ::rtl::OUString a##str_name = rtl::OUString::createFromAscii( str_value )

//------------------------------------------------------------------------
// condition names
//------------------------------------------------------------------------
OSLTEST_DECLARE_USTRING( TestSec,  "testsecurity" );
OSLTEST_DECLARE_USTRING( NullURL,  "" );

::rtl::OUString aLogonUser( aNullURL ), aLogonPasswd( aNullURL ), aFileServer( aNullURL ), aStringForward( aNullURL );
::rtl::OUString strUserName( aNullURL ) , strComputerName( aNullURL ) , strHomeDirectory( aNullURL );
::rtl::OUString strConfigDirectory( aNullURL ), strUserID( aNullURL );

sal_Bool isAdmin = sal_False;

#endif /* _OSL_SECURITY_CONST_H_ */
