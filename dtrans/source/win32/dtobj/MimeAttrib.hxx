/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MimeAttrib.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:05:55 $
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


#ifndef _MIMEATTRIB_HXX_
#define _MIMEATTRIB_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

const rtl::OUString TEXTPLAIN_PARAM_CHARSET = rtl::OUString::createFromAscii( "charset" );

const rtl::OUString PRE_WINDOWS_CODEPAGE = rtl::OUString::createFromAscii( "windows" );
const rtl::OUString PRE_OEM_CODEPAGE     = rtl::OUString::createFromAscii( "cp" );
const rtl::OUString CHARSET_UTF16        = rtl::OUString::createFromAscii( "utf-16" );
const rtl::OUString CHARSET_UNICODE      = rtl::OUString::createFromAscii( "unicode" );


#endif
