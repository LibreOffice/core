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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"
#include <unicode/uscript.h>
#include <i18npool/lang.h>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <osl/mutex.hxx>
#include <unotools/processfactory.hxx>
#include <ucbhelper/content.hxx>

#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include "hhconvdic.hxx"
#include "linguistic/misc.hxx"
#include "defs.hxx"

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;

#define SN_HH_CONV_DICTIONARY   "com.sun.star.linguistic2.HangulHanjaConversionDictionary"


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



sal_Bool TextIsAllScriptType( const OUString &rTxt, sal_Int16 nScriptType )
{
    sal_Bool bIsAll = sal_True;
    for (sal_Int32 i = 0;  i < rTxt.getLength() && bIsAll;  ++i)
    {
        if (checkScriptType( rTxt.getStr()[i]) != nScriptType)
            bIsAll = sal_False;
    }
    return bIsAll;
}



HHConvDic::HHConvDic( const String &rName, const String &rMainURL ) :
    ConvDic( rName, LANGUAGE_KOREAN, ConversionDictionaryType::HANGUL_HANJA, sal_True, rMainURL )
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
    if (rServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(SN_CONV_DICTIONARY)) ||
        rServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(SN_HH_CONV_DICTIONARY)))
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
