/*************************************************************************
 *
 *  $RCSfile: os2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
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

#ifndef _os2_hxx
#define _os2_hxx

#include <string.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <ctype.h>

#define INCL_DOSEXCEPTIONS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_DOSNLS   /* National Language Support values */
#include <svpm.h>

#include <string.hxx>

#define FSYS_UNIX FALSE

#define DOS_DIRECT      16
#define setdrive(n,a)   dos_setdrive(n,a)
#define GETDRIVE(n)     dos_getdrive(&n)

#define _mkdir(p)       DosCreateDir( (PSZ)p, 0 )
#define _rmdir(p)       DosDeleteDir( (PSZ)p )
#define _unlink(p)      DosForceDelete( (PSZ)p )
#define _chdir          chdir
#define _getcwd         getcwd
#define _access         access

const char* TempDirImpl( char *pBuf );
String ToLowerImpl( const String& );

inline char *getcwd(char *p, ULONG nLen )
{
    return DosQueryCurrentDir( 0, (PBYTE)p, &nLen ) ? p : 0;
}

#define dirent _FILEFINDBUF3
#define d_name achName
#define d_type attrFile
#define d_size cbFile
#define d_attr attrFile
#define d_date fdateLastWrite
#define d_time ftimeLastWrite

typedef struct
{
    HDIR          aDirHdl;
    FILEFINDBUF3  aDirEnt;
    char         *p;
} DIR;

#define DEFSTYLE            FSYS_STYLE_OS2
#define MKDIR( p )          mkdir( (unsigned char*) p )
#define CMP_LOWER(aString)  ToLowerImpl(aString)

#define START_DRV       'a'

inline BOOL DRIVE_EXISTS( char c )
{
    ULONG  nCur, nMap;
    APIRET nRet = DosQueryCurrentDisk( &nCur, &nMap );
    return ( nMap & 1 << (c - 'a') ) != 0;
}

#include <time.h>
#include <datetime.hxx>

inline Time MsDos2Time( FTIME aTime )
{
    return Time( aTime.hours, aTime.minutes, 2*aTime.twosecs );
}

inline Date MsDos2Date( FDATE aDate )
{
    return Date( aDate.day, aDate.month, aDate.year );
}

Time MsDos2Time( const time_t *pTimeT );

Date MsDos2Date( const time_t *pTimeT );

#define FSysFailOnErrorImpl()

#endif
