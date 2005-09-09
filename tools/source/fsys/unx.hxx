/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unx.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:18:22 $
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

#ifndef _unx_hxx
#define _unx_hxx

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <unistd.h>
/* #include <sysent.h> */

#define FSYS_UNIX TRUE
#define DRIVE_EXISTS(c) ( TRUE )

#define _mkdir(p)       mkdir(p, 0777)
#define _rmdir          rmdir
#define _chdir          chdir
#define _unlink         unlink
#define _getcwd         getcwd
#define _access         access

#ifdef SYSV3
#define DEFSTYLE        FSYS_STYLE_SYSV
#else
#define DEFSTYLE        FSYS_STYLE_BSD
#endif

#define CMP_LOWER(s)    (s)
#define TEMPNAME()      tmpnam(0)
#define LOWER(aString)  (aString.Lower())

#include <time.h>
#include <datetime.hxx>

inline Time Unx2Time( time_t nTime )
{
    tm atm;
    tm *pTime;
    pTime = localtime_r( &nTime, &atm );
    return Time( pTime->tm_hour,
                 pTime->tm_min,
                 pTime->tm_sec );
}

inline Date Unx2Date( time_t nDate )
{
    tm atm;
    tm *pTime;
    pTime = localtime_r( &nDate, &atm );
    return Date( pTime->tm_mday,
                 pTime->tm_mon + 1,
                 pTime->tm_year + 1900 );
}

inline void Unx2DateAndTime( time_t nDate, Time& rTime, Date& rDate )
{
    tm atm;
    tm *pTime;
    pTime = localtime_r( &nDate, &atm );
    rTime = Time( pTime->tm_hour, pTime->tm_min, pTime->tm_sec );
    rDate = Date( pTime->tm_mday, pTime->tm_mon + 1, pTime->tm_year + 1900 );
}

const char* TempDirImpl( char *pBuf );

#define FSysFailOnErrorImpl()

#endif
