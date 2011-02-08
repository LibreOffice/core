/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _OSL_MODULE_CONST_H_
#define _OSL_MODULE_CONST_H_


#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#if ( defined UNX ) || ( defined OS2 )  //Unix
#   include <unistd.h>
#endif
#if ( defined WNT )                     // Windows
#include <prewin.h>
#   include <io.h>
#include <postwin.h>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
