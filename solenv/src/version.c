/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: version.c,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 17:18:25 $
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



#include <_version.h>


struct VersionInfo
{
    const char* pTime;
    const char* pDate;
    const char* pUpd;
    const char* pMinor;
    const char* pBuild;
    const char* pInpath;
};

static const struct VersionInfo g_aVersionInfo =
{
    __TIME__,
    __DATE__,
    _UPD,
    _LAST_MINOR,
    _BUILD,
    _INPATH
};

#ifdef WNT
__declspec(dllexport) const struct VersionInfo* GetVersionInfo(void);
#endif

#ifdef WNT
__declspec(dllexport) const struct VersionInfo* GetVersionInfo(void)
#else
const struct VersionInfo *GetVersionInfo(void)
#endif
{
    return &g_aVersionInfo;
}

#if 0
#include <stdio.h>

int main( int argc, char **argv )
{
    const VersionInfo *pInfo = GetVersionInfo();
    fprintf( stderr, "Date : %s\n", pInfo->pDate);
    fprintf( stderr, "Time : %s\n", pInfo->pTime);
    fprintf( stderr, "UPD : %s\n", pInfo->pUpd);
    delete pInfo;
    return 0;
}
#endif

