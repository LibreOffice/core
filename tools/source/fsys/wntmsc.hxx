/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wntmsc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 14:15:15 $
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

#ifndef _dosmsc_hxx
#define _dosmsc_hxx

#include <string.h>

#ifndef ICC
#include <io.h>
#endif
#include <sys\types.h>
#include <sys\stat.h>
#include <direct.h>

#include <tools/svwin.h>
#ifdef _MSC_VER
#pragma warning (push,1)
#endif
#include <winbase.h>
#ifdef _MSC_VER
#pragma warning (pop)
#endif
#include <tools/solar.h>

#include <tools/string.hxx>

//--------------------------------------------------------------------

#define FSYS_UNIX FALSE

#define DOS_DIRECT      _A_SUBDIR
#define DOS_VOLUMEID    0x08
#ifndef S_IFBLK
#define S_IFBLK         0x6000
#endif
#define setdrive(n,a)   _chdrive(n)
#define GETDRIVE(n)     (n = _getdrive())

#define dirent          _WIN32_FIND_DATAA
#define d_name          cFileName
#define d_type          dwFileAttributes

#if defined (TCPP) || defined (tcpp)
#define _mkdir          mkdir
#define _rmdir          rmdir
#define _chdir          chdir
#define _unlink         unlink
#define _getcwd         getcwd
#define _access         access
#endif

typedef struct
{
    _WIN32_FIND_DATAA aDirEnt;
    HANDLE           h;
    const char      *p;
} DIR;

#define PATHDELIMITER   ";"
#define DEFSTYLE        FSYS_STYLE_NTFS
#define MKDIR( p )      mkdir( p )
#define CMP_LOWER(s)    ( ByteString(s).ToLowerAscii() )

#define START_DRV 'a'

inline BOOL DRIVE_EXISTS(char c)
{
    ByteString aDriveRoot( c );
    aDriveRoot += ":\\";
    return GetDriveType( aDriveRoot.GetBuffer() ) > 1;
}

const char* TempDirImpl( char *pBuf );

#define FSysFailOnErrorImpl()

#endif
