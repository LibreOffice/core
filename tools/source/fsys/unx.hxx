/*************************************************************************
 *
 *  $RCSfile: unx.hxx,v $
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

#ifndef _unx_hxx
#define _unx_hxx

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef S390
#include <sys/param.h>
#endif

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

#ifndef localtime_r
extern "C" {
struct tm *localtime_r(const time_t *timep, struct tm *buffer);
}
#endif

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
