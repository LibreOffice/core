/*************************************************************************
 *
 *  $RCSfile: MTypeConverter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mmaher $ $Date: 2001-10-11 10:07:55 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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
 *  Contributor(s): Willem van Dorp, Darren Kenny
 *
 *
 ************************************************************************/

#include <MNSInclude.hxx>

#ifndef _CONNECTIVITY_MAB_TYPECONVERTER_HXX_
#include "MTypeConverter.hxx"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

using namespace connectivity::mozab;

// -------------------------------------------------------------------------
void MTypeConverter::ouStringToNsString(::rtl::OUString const &ous, nsString &nss)
{
    OSL_TRACE( "IN MTypeConverter::OUStringToNsString()\n" );

    // Convert to ::rtl::OString (utf-8 encoding).
    ::rtl::OString os(ous,ous.getLength(), RTL_TEXTENCODING_UTF8);

    const char *cs = os.getStr();
    PRUint32 csLen = os.getLength();

    NS_ConvertUTF8toUCS2 mozString(cs, csLen);
    //const PRUnichar* uniMozString = (const PRUnichar*) mozString;
    nss = mozString; // temp.

    OSL_TRACE( "\tOUT MTypeConverter::OUStringToNsString()\n" );
}
// -------------------------------------------------------------------------
void MTypeConverter::nsStringToOUString(nsString const &nss, ::rtl::OUString &ous)
{
    OSL_TRACE( "IN MTypeConverter::nsStringToOUString()\n" );

    // Get clone of buffer.
    PRUnichar *uc = nss.ToNewUnicode();
    sal_Int32 nssLen = nss.Length();

    // TODO check if this is ok.
    ::rtl::OUString _ous(uc, nssLen);
    ous = _ous;

    nsMemory::Free(uc);

    OSL_TRACE( "\tOUT MTypeConverter::nsStringToOUString()\n" );
}
// -------------------------------------------------------------------------
void MTypeConverter::prUnicharToOUString(PRUnichar const *pru, ::rtl::OUString &ous)
{
    OSL_TRACE( "IN MTypeConverter::prUniCharToOUString()\n" );

    // TODO, specify length.
    ::rtl::OUString _ous(pru);
    ous = _ous;

    OSL_TRACE( "\tOUT MTypeConverter::prUniCharToOUString()\n" );
}
// -------------------------------------------------------------------------
char *MTypeConverter::ouStringToCCharStringUtf8(::rtl::OUString const &ous)
{
    OSL_TRACE( "IN MTypeConverter::OUStringToCCharStringUtf8()\n" );

    // Convert to ::rtl::OString,
    ::rtl::OString os(ous,ous.getLength(), RTL_TEXTENCODING_UTF8);

    const char *cs = os.getStr();

    OSL_TRACE( "\tOUT MTypeConverter::OUStringToCCharStringUtf8()\n" );
    return(strdup(cs));
}
// -------------------------------------------------------------------------
char *MTypeConverter::ouStringToCCharStringAscii(::rtl::OUString const &ous)
{
    OSL_TRACE( "IN MTypeConverter::OUStringToCCharStringAscii()\n" );

    // Convert ::rtl::OUString to ::rtl::OString,
    ::rtl::OString os(ous,ous.getLength(), RTL_TEXTENCODING_ASCII_US);

    OSL_TRACE( "\tOUT MTypeConverter::OUStringToCCharStringAscii()\n" );
    return(strdup(os.getStr()));
}
// -------------------------------------------------------------------------
char *MTypeConverter::nsStringToCCharStringAscii(nsString const &nss)
{
    OSL_TRACE( "IN MTypeConverter::nsStringToCCharStringAscii()\n" );

    char cs[1024];
    nss.ToCString(cs, 1024);

    OSL_TRACE( "\tOUT MTypeConverter::nsStringToCCharStringAscii()\n" );
    return(strdup(cs));
}
// -------------------------------------------------------------------------
::std::string MTypeConverter::ouStringToStlString(::rtl::OUString const &ous)
{
    OSL_TRACE( "IN MTypeConverter::ouStringToStlString()\n" );

    // Convert ::rtl::OUString to ::rtl::OString.
    ::rtl::OString os(ous,ous.getLength(),RTL_TEXTENCODING_ASCII_US);

    OSL_TRACE( "\tOUT MTypeConverter::ouStringToStlString()\n" );
    return( ::std::string(os.getStr()));
}
// -------------------------------------------------------------------------
::std::string MTypeConverter::nsStringToStlString(nsString const &nss)
{
    OSL_TRACE( "IN MTypeConverter::nsStringToStlString()\n" );

    OSL_TRACE( "\tOUT MTypeConverter::nsStringToStlString()\n" );
    return( ::std::string(nss.GetBuffer()));
}
// -------------------------------------------------------------------------
