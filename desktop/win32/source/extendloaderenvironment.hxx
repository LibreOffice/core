/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extendloaderenvironment.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:52:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_WIN32_SOURCE_EXTENDLOADERENVIRONMENT_HXX
#define INCLUDED_DESKTOP_WIN32_SOURCE_EXTENDLOADERENVIRONMENT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <tchar.h>

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

namespace desktop_win32 {

// Set the PATH environment variable in the current (loader) process, so that a
// following CreateProcess has the necessary environment:
//
// @param binPath
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full path to the "bin" file corresponding to the current
// executable.
//
// @param iniDirectory
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full directory path (ending in "\") to the "ini" file
// corresponding to the current executable.
void extendLoaderEnvironment(WCHAR * binPath, WCHAR * iniDirectory);

}

#endif
