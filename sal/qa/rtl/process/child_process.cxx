/*************************************************************************
 *
 *  $RCSfile: child_process.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 09:13:58 $
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

#include <stdio.h>

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

#ifndef _RTL_PROCESS_CONST_H_
#include <rtl_Process_Const.h>
#endif

using namespace ::rtl;

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    printf("# %s is called.\n", argv[0]);

    sal_Bool bRes = sal_True;
    sal_Int32 nCount = rtl_getAppCommandArgCount();
    if ( nCount != 4 )
    {
        printf("# not enough arguments found, need 4 found %d.\n", nCount);
        return 0;
    }

    OUString suArg[4];
    for( sal_Int32 i = 0 ; i < nCount ; i ++ )
    {
        rtl_getAppCommandArg( i , &(suArg[i].pData) );
        rtl::OString aString;
        aString = ::rtl::OUStringToOString( suArg[i], RTL_TEXTENCODING_ASCII_US );
        printf("# Parameter[%d] is %s\n", i, aString.getStr());
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
