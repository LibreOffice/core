/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

// static
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

// static
void ScModule::SetAutoSpellProperty( sal_Bool bSet )
{
    //  use SvtLinguConfig instead of service LinguProperties to avoid
    //  loading the linguistic component
    SvtLinguConfig aConfig;

    uno::Any aAny;
    aAny <<= bSet;
    aConfig.SetProperty( rtl::OUString::createFromAscii( LINGUPROP_AUTOSPELL ), aAny );
}



// static
sal_Bool ScModule::HasThesaurusLanguage( sal_uInt16 nLang )
{
    if ( nLang == LANGUAGE_NONE )
        return sal_False;

    lang::Locale aLocale;
    SvxLanguageToLocale( aLocale, nLang );

    sal_Bool bHasLang = sal_False;
    try
    {
        uno::Reference< linguistic2::XThesaurus > xThes(LinguMgr::GetThesaurus());
        if ( xThes.is() )
            bHasLang = xThes->hasLocale( aLocale );
    }
    catch( uno::Exception& )
    {
        DBG_ERROR("Error in Thesaurus");
    }

    return bHasLang;
}


