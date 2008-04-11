/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MimeAttrib.hxx,v $
 * $Revision: 1.6 $
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


#ifndef _MIMEATTRIB_HXX_
#define _MIMEATTRIB_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <rtl/ustring.hxx>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

const rtl::OUString TEXTPLAIN_PARAM_CHARSET = rtl::OUString::createFromAscii( "charset" );

const rtl::OUString PRE_WINDOWS_CODEPAGE = rtl::OUString::createFromAscii( "windows" );
const rtl::OUString PRE_OEM_CODEPAGE     = rtl::OUString::createFromAscii( "cp" );
const rtl::OUString CHARSET_UTF16        = rtl::OUString::createFromAscii( "utf-16" );
const rtl::OUString CHARSET_UNICODE      = rtl::OUString::createFromAscii( "unicode" );


#endif
