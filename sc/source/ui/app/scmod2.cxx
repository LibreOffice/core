/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scmod2.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:10:31 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include <svx/unolingu.hxx>
#include <svtools/lingucfg.hxx>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/lang/Locale.hpp>

using namespace com::sun::star;

#include "scmod.hxx"

//------------------------------------------------------------------

#define LINGUPROP_AUTOSPELL         "IsSpellAuto"
#define LINGUPROP_HIDEAUTO          "IsSpellHide"

//------------------------------------------------------------------

// static
void ScModule::GetSpellSettings( USHORT& rDefLang, USHORT& rCjkLang, USHORT& rCtlLang,
                                    BOOL& rAutoSpell, BOOL& rHideAuto )
{
    //  use SvtLinguConfig instead of service LinguProperties to avoid
    //  loading the linguistic component
    SvtLinguConfig aConfig;

    SvtLinguOptions aOptions;
    aConfig.GetOptions( aOptions );

    rDefLang = aOptions.nDefaultLanguage;
    rCjkLang = aOptions.nDefaultLanguage_CJK;
    rCtlLang = aOptions.nDefaultLanguage_CTL;
    rAutoSpell = aOptions.bIsSpellAuto;
    rHideAuto = aOptions.bIsSpellHideMarkings;
}

// static
void ScModule::SetAutoSpellProperty( BOOL bSet )
{
    //  use SvtLinguConfig instead of service LinguProperties to avoid
    //  loading the linguistic component
    SvtLinguConfig aConfig;

    uno::Any aAny;
    aAny <<= bSet;
    aConfig.SetProperty( rtl::OUString::createFromAscii( LINGUPROP_AUTOSPELL ), aAny );
}

// static
void ScModule::SetHideAutoProperty( BOOL bSet )
{
    //  use SvtLinguConfig instead of service LinguProperties to avoid
    //  loading the linguistic component
    SvtLinguConfig aConfig;

    uno::Any aAny;
    aAny <<= bSet;
    aConfig.SetProperty( rtl::OUString::createFromAscii( LINGUPROP_HIDEAUTO ), aAny );
}


// static
BOOL ScModule::HasThesaurusLanguage( USHORT nLang )
{
    if ( nLang == LANGUAGE_NONE )
        return FALSE;

    lang::Locale aLocale;
    SvxLanguageToLocale( aLocale, nLang );

    BOOL bHasLang = FALSE;
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


