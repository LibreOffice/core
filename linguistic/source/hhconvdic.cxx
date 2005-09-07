/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hhconvdic.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:51:22 $
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

#ifndef USCRIPT_H
#include <external/unicode/uscript.h>
#endif

#ifndef _LANG_HXX //autogen wg. LANGUAGE_ENGLISH_US
#include <tools/lang.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#include <cppuhelper/factory.hxx>   // helper for factories

#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_CONVERSIONDICTIONARYTYPE_HPP_
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#include "hhconvdic.hxx"
#include "misc.hxx"
#include "defs.hxx"

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

#define SN_HH_CONV_DICTIONARY   "com.sun.star.linguistic2.HangulHanjaConversionDictionary"

///////////////////////////////////////////////////////////////////////////

#include <i18nutil/unicode.hxx>
#include <com/sun/star/i18n/UnicodeScript.hpp>

using namespace i18n;

#define SCRIPT_OTHERS   0
#define SCRIPT_HANJA    1
#define SCRIPT_HANGUL   2

// from i18npool/source/textconversion/textconversion_ko.cxx
sal_Int16 SAL_CALL checkScriptType(sal_Unicode c) throw (RuntimeException)
{
  UErrorCode status = U_ZERO_ERROR;

  UScriptCode scriptCode = uscript_getScript(c, &status);

  if ( !U_SUCCESS(status) ) throw RuntimeException();

  return scriptCode == USCRIPT_HANGUL ? SCRIPT_HANGUL :
            scriptCode == USCRIPT_HAN ? SCRIPT_HANJA : SCRIPT_OTHERS;
}



BOOL TextIsAllScriptType( const OUString &rTxt, INT16 nScriptType )
{
    BOOL bIsAll = TRUE;
    for (INT32 i = 0;  i < rTxt.getLength() && bIsAll;  ++i)
    {
        if (checkScriptType( rTxt.getStr()[i]) != nScriptType)
            bIsAll = FALSE;
    }
    return bIsAll;
}


///////////////////////////////////////////////////////////////////////////

HHConvDic::HHConvDic( const String &rName, const String &rMainURL ) :
    ConvDic( rName, LANGUAGE_KOREAN, ConversionDictionaryType::HANGUL_HANJA, TRUE, rMainURL )
{
}


HHConvDic::~HHConvDic()
{
}


void SAL_CALL HHConvDic::addEntry(
        const OUString& aLeftText,
        const OUString& aRightText )
    throw (IllegalArgumentException, container::ElementExistException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if ((aLeftText.getLength() != aRightText.getLength()) ||
        !TextIsAllScriptType( aLeftText,  SCRIPT_HANGUL ) ||
        !TextIsAllScriptType( aRightText, SCRIPT_HANJA ))
        throw IllegalArgumentException();
    ConvDic::addEntry( aLeftText, aRightText );
}


OUString SAL_CALL HHConvDic::getImplementationName(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL HHConvDic::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    sal_Bool bRes = sal_False;
    if (rServiceName.equalsAscii( SN_CONV_DICTIONARY )||
        rServiceName.equalsAscii( SN_HH_CONV_DICTIONARY ))
        bRes = sal_True;
    return bRes;
}


uno::Sequence< OUString > SAL_CALL HHConvDic::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > HHConvDic::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = A2OU( SN_CONV_DICTIONARY );
    aSNS.getArray()[1] = A2OU( SN_HH_CONV_DICTIONARY );
    return aSNS;
}

///////////////////////////////////////////////////////////////////////////

