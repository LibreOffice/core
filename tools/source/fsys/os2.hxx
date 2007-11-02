/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: os2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:02:40 $
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

#ifndef _os2_hxx
#define _os2_hxx


#define INCL_DOSEXCEPTIONS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_DOSNLS   /* National Language Support values */
#include <svpm.h>

#include <dirent.h>
#include <string.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <ctype.h>
#include <emx/syscalls.h>

//#include <string.hxx>

#define FSYS_UNIX FALSE

#define DOS_DIRECT      _A_SUBDIR
#define DOS_VOLUMEID    _A_VOLID

#define _mkdir(p)       mkdir(p, 0777)

const char* TempDirImpl( char *pBuf );
String ToLowerImpl( const String& );

#define DEFSTYLE            FSYS_STYLE_OS2
#define MKDIR( p )          mkdir( (unsigned char*) p )
#define CMP_LOWER(s)    ( s.ToLowerAscii() )

#define START_DRV       'a'

inline BOOL DRIVE_EXISTS( char c )
{
    ULONG  nCur, nMap;
    APIRET nRet = DosQueryCurrentDisk( &nCur, &nMap );
    return ( nMap & 1 << (c - 'a') ) != 0;
}

#include <time.h>
//#include <datetime.hxx>

inline Time MsDos2Time( FTIME* aTime )
{
    return Time( aTime->hours, aTime->minutes, 2*aTime->twosecs );
}

inline Date MsDos2Date( FDATE* aDate )
{
    return Date( aDate->day, aDate->month, aDate->year );
}

Time MsDos2Time( const time_t *pTimeT );

Date MsDos2Date( const time_t *pTimeT );

#define FSysFailOnErrorImpl()

#endif

