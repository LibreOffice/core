/*************************************************************************
 *
 *  $RCSfile: loadlib.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:07 $
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
#include <string.h>
#include <stdlib.h>

#include <tools/string.hxx>
#include <libcall.hxx>


extern "C" {

struct VersionInfo
{
    char    aTime[20];
    char    aDate[20];
    char    aUpd[5];
    char    aMinor;
    char    aBuild[5];
#if SUPD>564
    char    aInpath[20];
#endif
};

#ifdef WNT
__declspec(dllexport)
#endif
    VersionInfo *GetVersionInfo();
};

typedef VersionInfo*(__LOADONCALLAPI *PFUNC)(void);

int __LOADONCALLAPI main( int argc, char **argv )
{
    VersionInfo *pInfo = 0L;
    PFUNC pFunc;

    if ( argc != 2 )
    {
        fprintf( stderr, "USAGE: %s DllName \n", argv[0] );
        exit(0);
    }
    SvLibrary aLibrary(argv[1]);
    pFunc = (PFUNC) aLibrary.GetFunction( "GetVersionInfo" );
    if ( pFunc )
        pInfo = (*pFunc)();
    if ( pInfo )
    {
        fprintf( stdout, "Date : %s\n", pInfo->aDate );
        fprintf( stdout, "Time : %s\n", pInfo->aTime );
        fprintf( stdout, "UPD : %s\n", pInfo->aUpd );
        fprintf( stdout, "Minor : %c\n", pInfo->aMinor );
        fprintf( stdout, "Build : %s\n", pInfo->aBuild );
#if SUPD>564
        fprintf( stdout, "Inpath : %s\n", pInfo->aInpath );
#endif
    }
    else
        fprintf( stderr, "VersionInfo not Found !\n" );
    return 0;
}

