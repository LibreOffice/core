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

#include <editeng/unolingu.hxx>
#include <unotools/lingucfg.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
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
    aConfig.SetProperty( OUString( LINGUPROP_AUTOSPELL ), aAny );
}



sal_Bool ScModule::HasThesaurusLanguage( sal_uInt16 nLang )
{
    if ( nLang == LANGUAGE_NONE )
        return false;

    sal_Bool bHasLang = false;
    try
    {
        uno::Reference< linguistic2::XThesaurus > xThes(LinguMgr::GetThesaurus());
        if ( xThes.is() )
            bHasLang = xThes->hasLocale( LanguageTag::convertToLocale( nLang ) );
    }
    catch( uno::Exception& )
    {
        OSL_FAIL("Error in Thesaurus");
    }

    return bHasLang;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
