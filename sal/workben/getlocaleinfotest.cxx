/*************************************************************************
 *
 *  $RCSfile: getlocaleinfotest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2001-09-11 12:49:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <rtl/locale.h>
#include <osl/nlsupport.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>

#include <stdio.h>

#ifdef WNT
#define _CDECL _cdecl
#else
#define _CDECL
#endif

int _CDECL main( int argc, char * argv[] )
{
/*
    rtl::OUString lang = rtl::OUString::createFromAscii( "zh" );
    rtl::OUString country = rtl::OUString::createFromAscii( "TW" );
*/
    rtl::OUString lang = rtl::OUString::createFromAscii( argv[1] );
    rtl::OUString country = rtl::OUString::createFromAscii( argv[2] );
    rtl_TextEncoding rtlTextEnc;
    rtl_Locale* rtlLocale = NULL;

    osl_getProcessLocale( &rtlLocale );

    if ( rtlLocale )
    {
        rtlTextEnc = osl_getTextEncodingFromLocale( rtlLocale );
        printf( "default text encoding is %d.\n", rtlTextEnc );
    }
    else
        fprintf( stderr, "osl_getProcessLocale did not return a locale !!\n" );

    rtlLocale = rtl_locale_register( lang.getStr( ), country.getStr( ), NULL );

    if ( rtlLocale )
    {
        rtlTextEnc = osl_getTextEncodingFromLocale( rtlLocale );
        printf( "text encoding for %s_%s is %d.\n", argv[1], argv[2], rtlTextEnc );
    }
    else
        fprintf( stderr, "rtl_locale_register did not return a locale !!\n" );

    rtlTextEnc = osl_getTextEncodingFromLocale( NULL );
    printf( "process text encoding is %d.\n", rtlTextEnc );

    if( osl_setProcessLocale( rtlLocale ) )
        fprintf( stderr, "osl_setProcessLocale failed !!\n" );


    rtlTextEnc = osl_getTextEncodingFromLocale( NULL );
    printf( "process text encoding is now: %d.\n", rtlTextEnc );

    osl_getProcessLocale( &rtlLocale );

    if ( rtlLocale )
    {
        rtlTextEnc = osl_getTextEncodingFromLocale( rtlLocale );
        printf( "text encoding for process locale is now: %d.\n", rtlTextEnc );
    }
    else
        fprintf( stderr, "osl_getProcessLocale did not return a locale !!\n" );

    return(0);
}


