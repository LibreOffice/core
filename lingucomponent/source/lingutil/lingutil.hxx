/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lingutil.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:49:59 $
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

#ifndef _LINGUTIL_HXX_
#define _LINGUTIL_HXX_


#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <vector>
#include <list>


#define A2OU(x) ::rtl::OUString::createFromAscii( x )

#define OU2A(rtlOUString) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    RTL_TEXTENCODING_ASCII_US).getStr()

#define OU2UTF8(rtlOUString) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    RTL_TEXTENCODING_UTF8).getStr()

#define OU2ISO_1(rtlOUString) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    RTL_TEXTENCODING_ISO_8859_1).getStr()

#define OU2ENC(rtlOUString, rtlEncoding) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    rtlEncoding, RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK).getStr()


struct SvtLinguConfigDictionaryEntry;

///////////////////////////////////////////////////////////////////////////

struct lt_rtl_OUString
{
    bool operator() (const rtl::OUString &r1, const rtl::OUString &r2) const
    {
        return r1 < r2;
    }
};

inline BOOL operator == ( const ::com::sun::star::lang::Locale &rL1, const ::com::sun::star::lang::Locale &rL2 )
{
    return  rL1.Language ==  rL2.Language   &&
            rL1.Country  ==  rL2.Country    &&
            rL1.Variant  ==  rL2.Variant;
}

///////////////////////////////////////////////////////////////////////////

String GetDirectoryPathFromFileURL( const String &rFileURL );

///////////////////////////////////////////////////////////////////////////

// to be use to get a short path name under Windows that still can be used with
// the 'fopen' call. This is necessary since under Windows there seems to be
// a restriction of only about 110-130 characters length to a path name in order
// for it to work with 'fopen'. And that length is usually easily exceeded
// when using extensions...
rtl::OString Win_GetShortPathName( const rtl::OUString &rLongPathName );

///////////////////////////////////////////////////////////////////////////

// temporary function, to be removed when new style dictionaries
// using configuration entries are fully implemented and provided
std::vector< SvtLinguConfigDictionaryEntry > GetOldStyleDics( const char * pDicType );
void MergeNewStyleDicsAndOldStyleDics( std::list< SvtLinguConfigDictionaryEntry > &rNewStyleDics, const std::vector< SvtLinguConfigDictionaryEntry > &rOldStyleDics );

///////////////////////////////////////////////////////////////////////////

#endif

