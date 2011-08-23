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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <i18npool/mslangid.hxx>

#include "convuno.hxx"

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

//	everything is static...

LanguageType ScUnoConversion::GetLanguage( const lang::Locale& rLocale )
{
    //	empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_SYSTEM;

    LanguageType eRet = MsLangId::convertLocaleToLanguage( rLocale );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM;			//! or throw an exception?

    return eRet;
}

void ScUnoConversion::FillLocale( lang::Locale& rLocale, LanguageType eLang )
{
    MsLangId::convertLanguageToLocale( eLang, rLocale );
}



}
