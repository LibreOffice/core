/*************************************************************************
 *
 *  $RCSfile: hhconvdic.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 16:07:31 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
sal_Int16 SAL_CALL checkScriptType(sal_Unicode c)
{
    static ScriptTypeList typeList[] = {
        { UnicodeScript_kHangulJamo, SCRIPT_HANGUL }, // 29
        { UnicodeScript_kCJKRadicalsSupplement, SCRIPT_HANJA },     // 57,
        { UnicodeScript_kKangxiRadicals,        SCRIPT_HANJA },     // 58,
        { UnicodeScript_kIdeographicDescriptionCharacters, SCRIPT_HANJA },  // 59,
        { UnicodeScript_kCJKSymbolPunctuation,  SCRIPT_HANJA },     // 60,
        { UnicodeScript_kHiragana,          SCRIPT_HANJA },     // 61,
        { UnicodeScript_kKatakana,          SCRIPT_HANJA },     // 62,
        { UnicodeScript_kBopomofo,          SCRIPT_HANJA },     // 63,
        { UnicodeScript_kHangulCompatibilityJamo,   SCRIPT_HANGUL },        // 64,
        { UnicodeScript_kKanbun,            SCRIPT_HANJA },     // 65,
        { UnicodeScript_kBopomofoExtended,      SCRIPT_HANJA },     // 66,
        { UnicodeScript_kEnclosedCJKLetterMonth,    SCRIPT_HANJA },     // 67,
        { UnicodeScript_kCJKCompatibility,      SCRIPT_HANJA },     // 68,
        { UnicodeScript_k_CJKUnifiedIdeographsExtensionA, SCRIPT_HANJA },   // 69,
        { UnicodeScript_kCJKUnifiedIdeograph,   SCRIPT_HANJA },     // 70,
        { UnicodeScript_kYiSyllables,       SCRIPT_HANJA },     // 71,
        { UnicodeScript_kYiRadicals,        SCRIPT_HANJA },     // 72,
        { UnicodeScript_kHangulSyllable,        SCRIPT_HANGUL },        // 73,
        { UnicodeScript_kCJKCompatibilityIdeograph, SCRIPT_HANJA },     // 78,
        { UnicodeScript_kCombiningHalfMark,     SCRIPT_HANJA },     // 81,
        { UnicodeScript_kCJKCompatibilityForm,  SCRIPT_HANJA },     // 82,
        { UnicodeScript_kSmallFormVariant,      SCRIPT_HANJA },     // 83,
        { UnicodeScript_kHalfwidthFullwidthForm,    SCRIPT_HANJA },     // 86,

        { UnicodeScript_kScriptCount, SCRIPT_OTHERS } // 87,
    };

    return unicode::getUnicodeScriptType(c, typeList, SCRIPT_OTHERS);
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
    ConvDic( rName, LANGUAGE_KOREAN, ConversionDictionaryType::HANGUL_HANJA, rMainURL )
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

