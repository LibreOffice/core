/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unicode/uscript.h>
#include <i18npool/lang.h>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <osl/mutex.hxx>
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
    if ( rServiceName == SN_CONV_DICTIONARY || rServiceName == SN_HH_CONV_DICTIONARY )
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
