/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macro.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:18:19 $
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

#ifndef _RTL_MACRO_HXX
#define _RTL_MACRO_HXX

#include <rtl/bootstrap.h>
#include <rtl/ustring.hxx>

#if defined WIN32
#define THIS_OS "Windows"
#elif defined OS2
#define THIS_OS "OS/2"
#elif defined SOLARIS
#define THIS_OS "Solaris"
#elif defined LINUX
#define THIS_OS "Linux"
#elif defined MACOSX
#define THIS_OS "MacOSX"
#elif defined NETBSD
#define THIS_OS "NetBSD"
#elif defined FREEBSD
#define THIS_OS "FreeBSD"
#elif defined IRIX
#define THIS_OS "Irix"
#endif

#if ! defined THIS_OS
#error "unknown OS -- insert your OS identifier above"
this is inserted for the case that the preprocessor ignores error
#endif

#if defined INTEL
#define THIS_ARCH "x86"
#elif defined POWERPC
#define THIS_ARCH "PowerPC"
#elif defined S390
#define THIS_ARCH "S390"
#elif defined SPARC
#define THIS_ARCH "SPARC"
#elif defined IRIX
#define THIS_ARCH "MIPS"
#elif defined X86_64
#define THIS_ARCH "X86_64"
#elif defined MIPS
#define THIS_ARCH "MIPS"
#elif defined ARM
#define THIS_ARCH "ARM"
#elif defined IA64
#define THIS_ARCH "IA64"
#endif

#if ! defined THIS_ARCH
#error "unknown ARCH -- insert your ARCH identifier above"
this is inserted for the case that the preprocessor ignores error
#endif

#endif





