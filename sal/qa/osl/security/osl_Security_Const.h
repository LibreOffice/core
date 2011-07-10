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

#ifndef _OSL_SECURITY_CONST_H_
#define _OSL_SECURITY_CONST_H_

#if ( defined WNT )                     // Windows
#include <io.h>
#endif

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>

#include <stdlib.h>
#include <stdio.h>

#if ( defined UNX )
#include <unistd.h>
#include <pwd.h>
#endif

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/plugin/TestPlugInDefaultImpl.h>

#define t_print printf

#define BUFSIZE 1024
const char pTestString[17] = "Sun Microsystems";


#define OSLTEST_DECLARE_USTRING( str_name, str_value ) \
    ::rtl::OUString a##str_name = rtl::OUString::createFromAscii( str_value )

//------------------------------------------------------------------------
// condition names
//------------------------------------------------------------------------

// Intentionally different from the aNullURL in osl_File_Const.h to avoid
// duplicate symbols as all the unit tests here get linked together for iOS...

OSLTEST_DECLARE_USTRING( NullUrl,  "" );

::rtl::OUString aLogonUser( aNullUrl ), aLogonPasswd( aNullUrl ), aFileServer( aNullUrl ), aStringForward( aNullUrl );
::rtl::OUString strUserName( aNullUrl ) , strComputerName( aNullUrl ) , strHomeDirectory( aNullUrl );
::rtl::OUString strConfigDirectory( aNullUrl ), strUserID( aNullUrl );

sal_Bool isAdmin = sal_False;

#endif /* _OSL_SECURITY_CONST_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
