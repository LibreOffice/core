/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: osl_Module_Const.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:30:17 $
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

#ifndef _OSL_MODULE_CONST_H_
#define _OSL_MODULE_CONST_H_


#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <cppunit/simpleheader.hxx>

#if ( defined UNX ) || ( defined OS2 )  //Unix
#   include <unistd.h>
#endif
#if ( defined WNT )                     // Windows
#include <tools/prewin.h>
// #    include <windows.h>
#   include <io.h>
#include <tools/postwin.h>
#endif

#   define FILE_PREFIX          "file:///"

//Korea charactors
::rtl::OUString aKname(
    RTL_CONSTASCII_STRINGPARAM(
        "/\xEC\x95\x88\xEB\x85\x95\xED\x95\x98\xEC\x84\xB8\xEC\x9A\x94"),
    RTL_TEXTENCODING_ISO_8859_1);
    // zero-extend the individual byte-sized characters one-to-one to individual
    // sal_Unicode-sized characters; not sure whether this is what was
    // intended...

//------------------------------------------------------------------------
// function pointer type.
//------------------------------------------------------------------------
typedef sal_Bool (* FuncPtr )( sal_Bool );


#endif /* _OSL_MODULE_CONST_H_ */
