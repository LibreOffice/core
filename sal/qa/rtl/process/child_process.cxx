/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: child_process.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:27:01 $
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

#include <stdio.h>

#ifndef _SAL_MAIN_H_
#include "sal/main.h"
#endif

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

#ifndef _RTL_PROCESS_CONST_H_
#include <rtl_Process_Const.h>
#endif

using namespace ::rtl;

// ----------------------------------- Main -----------------------------------
SAL_IMPLEMENT_MAIN_WITH_ARGS(, argv)
{
    printf("# %s is called.\n", argv[0]);

    sal_Int32 nCount = rtl_getAppCommandArgCount();
    if ( nCount != 4 )
    {
        printf(
            "# not enough arguments found, need 4 found %ld.\n",
            sal::static_int_cast< long >(nCount));
        return 0;
    }

    OUString suArg[4];
    for( sal_Int32 i = 0 ; i < nCount ; i ++ )
    {
        rtl_getAppCommandArg( i , &(suArg[i].pData) );
        rtl::OString aString;
        aString = ::rtl::OUStringToOString( suArg[i], RTL_TEXTENCODING_ASCII_US );
        printf(
            "# Parameter[%ld] is %s\n", sal::static_int_cast< long >(i),
            aString.getStr());
    }

    if ( suArg[0].compareTo( suParam0) != 0 ||
         suArg[1].compareTo( suParam1) != 0 ||
         suArg[2].compareTo( suParam2) != 0 ||
         suArg[3].compareTo( suParam3) != 0 )
    {
        return 0;
    }
    return 2;
}
