/*************************************************************************
 *
 *  $RCSfile: version.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2002-03-07 17:24:11 $
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


#if 0
#include <stdio.h>
#include <string.h>
#else
#ifdef WNT
char *  __cdecl strcpy(char *, const char *);
#endif
#if defined( OS2 ) || defined( UNX )
char * strcpy(char*, const char* );
#endif
#endif

#include <_version.h>


typedef struct _VersionInfo
{
    char    aTime[20];
    char    aDate[20];
    char    aUpd[5];
    char    aMinor;
    char    aBuild[5];
    char    aInpath[20];
} VersionInfo;


#ifdef WNT
__declspec(dllexport) VersionInfo *GetVersionInfo();
#endif
#ifdef OS2
VersionInfo *GetVersionInfo();
#endif

#ifdef WNT
__declspec(dllexport) VersionInfo *GetVersionInfo()
#endif
#if defined( OS2 ) || defined( UNX )
VersionInfo *GetVersionInfo()
#endif
{
    VersionInfo *pInfo;
    pInfo = (VersionInfo*)malloc(sizeof(VersionInfo));
    (void) strcpy(pInfo->aUpd, _UPD);
    (void) strcpy(pInfo->aDate, __DATE__);
    (void) strcpy(pInfo->aTime, __TIME__);
    pInfo->aMinor = _LAST_MINOR;
    (void) strcpy(pInfo->aBuild, _BUILD );
    (void) strcpy(pInfo->aInpath, _INPATH );
    return pInfo;
}

#if 0
int main( int argc, char **argv )
{
    VersionInfo *pInfo = GetVersionInfo();
    fprintf( stderr, "Date : %s\n", pInfo->aDate);
    fprintf( stderr, "Time : %s\n", pInfo->aTime);
    fprintf( stderr, "UPD : %s\n", pInfo->aUpd);
    delete pInfo;
    return 0;
}
#endif

