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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include <editeng/unolingu.hxx>
#include <unotools/lingucfg.hxx>
#include <i18npool/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/lang/Locale.hpp>

using namespace com::sun::star;

#include "scmod.hxx"

//------------------------------------------------------------------

#define LINGUPROP_AUTOSPELL         "IsSpellAuto"

//------------------------------------------------------------------

void ScModule::GetSpellSettings( sal_uInt16& rDefLang, sal_uInt16& rCjkLang, sal_uInt16& rCtlLang,
                                    sal_Bool& rAutoSpell )
{
    //  use SvtLinguConfig instead of service LinguProperties to avoid
    //  loading the linguistic component
    SvtLinguConfig aConfig;

    SvtLinguOptions aOptions;
    aConfig.GetOptions( aOptions );

    rDefLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN);
    rCjkLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN);
    rCtlLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);
    rAutoSpell = aOptions.bIsSpellAuto;
}

void ScModule::SetAutoSpellProperty( sal_Bool bSet )
{
    //  use SvtLinguConfig instead of service LinguProperties to avoid
    //  loading the linguistic component
    SvtLinguConfig aConfig;

    uno::Any aAny;
    aAny <<= bSet;
    aConfig.SetProperty( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( LINGUPROP_AUTOSPELL )), aAny );
}



sal_Bool ScModule::HasThesaurusLanguage( sal_uInt16 nLang )
{
    if ( nLang == LANGUAGE_NONE )
        return false;

    lang::Locale aLocale;
    SvxLanguageToLocale( aLocale, nLang );

    sal_Bool bHasLang = false;
    try
    {
        uno::Reference< linguistic2::XThesaurus > xThes(LinguMgr::GetThesaurus());
        if ( xThes.is() )
            bHasLang = xThes->hasLocale( aLocale );
    }
    catch( uno::Exception& )
    {
        OSL_FAIL("Error in Thesaurus");
    }

    return bHasLang;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
