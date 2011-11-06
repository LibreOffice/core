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
#include "precompiled_svl.hxx"

#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <i18npool/mslangid.hxx>
#include <vos/mutex.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/syslocale.hxx>

using namespace ::com::sun::star;
// global ----------------------------------------------------------------------

namespace { struct ALMutex : public rtl::Static< ::osl::Mutex, ALMutex > {}; }

// class SvtLanguageOptions ----------------------------------------------------

SvtLanguageOptions::SvtLanguageOptions( sal_Bool _bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

    m_pCJKOptions = new SvtCJKOptions( _bDontLoad );
    m_pCTLOptions = new SvtCTLOptions( _bDontLoad );
    m_pCTLOptions->AddListener(this);
    m_pCJKOptions->AddListener(this);
}
//------------------------------------------------------------------------------
SvtLanguageOptions::~SvtLanguageOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

    m_pCTLOptions->RemoveListener(this);
    m_pCJKOptions->RemoveListener(this);

    delete m_pCJKOptions;
    delete m_pCTLOptions;
}
//------------------------------------------------------------------------------
// CJK options -----------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCJKFontEnabled() const
{
    return m_pCJKOptions->IsCJKFontEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsVerticalTextEnabled() const
{
    return m_pCJKOptions->IsVerticalTextEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsAsianTypographyEnabled() const
{
    return m_pCJKOptions->IsAsianTypographyEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsJapaneseFindEnabled() const
{
    return m_pCJKOptions->IsJapaneseFindEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsRubyEnabled() const
{
    return m_pCJKOptions->IsRubyEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsChangeCaseMapEnabled() const
{
    return m_pCJKOptions->IsChangeCaseMapEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsDoubleLinesEnabled() const
{
    return m_pCJKOptions->IsDoubleLinesEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsEmphasisMarksEnabled() const
{
    return m_pCJKOptions->IsEmphasisMarksEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsVerticalCallOutEnabled() const
{
    return m_pCJKOptions->IsVerticalCallOutEnabled();
}
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetAll( sal_Bool _bSet )
{
    m_pCJKOptions->SetAll( _bSet );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsAnyEnabled() const
{
    return m_pCJKOptions->IsAnyEnabled();
}
//------------------------------------------------------------------------------
// CTL options -----------------------------------------------------------------
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetCTLFontEnabled( sal_Bool _bEnabled )
{
    m_pCTLOptions->SetCTLFontEnabled( _bEnabled );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCTLFontEnabled() const
{
    return m_pCTLOptions->IsCTLFontEnabled();
}
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetCTLSequenceChecking( sal_Bool _bEnabled )
{
    m_pCTLOptions->SetCTLSequenceChecking( _bEnabled );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCTLSequenceChecking() const
{
    return m_pCTLOptions->IsCTLSequenceChecking();
}
/*-- 26.09.2005 15:48:23---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtLanguageOptions::SetCTLSequenceCheckingRestricted( sal_Bool _bEnable )
{
    m_pCTLOptions->SetCTLSequenceCheckingRestricted( _bEnable );
}
/*-- 26.09.2005 15:48:23---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SvtLanguageOptions::IsCTLSequenceCheckingRestricted( void ) const
{
    return m_pCTLOptions->IsCTLSequenceCheckingRestricted();
}
/*-- 26.09.2005 15:48:23---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtLanguageOptions::SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable )
{
    m_pCTLOptions->SetCTLSequenceCheckingTypeAndReplace( _bEnable );
}
/*-- 26.09.2005 15:48:24---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SvtLanguageOptions::IsCTLSequenceCheckingTypeAndReplace() const
{
    return m_pCTLOptions->IsCTLSequenceCheckingTypeAndReplace();
}

//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsReadOnly(SvtLanguageOptions::EOption eOption) const
{
    sal_Bool bReadOnly = sal_False;
    switch(eOption)
    {
        // cjk options
        case SvtLanguageOptions::E_CJKFONT          : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_CJKFONT        ); break;
        case SvtLanguageOptions::E_VERTICALTEXT     : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_VERTICALTEXT   ); break;
        case SvtLanguageOptions::E_ASIANTYPOGRAPHY  : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_ASIANTYPOGRAPHY); break;
        case SvtLanguageOptions::E_JAPANESEFIND     : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_JAPANESEFIND   ); break;
        case SvtLanguageOptions::E_RUBY             : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_RUBY           ); break;
        case SvtLanguageOptions::E_CHANGECASEMAP    : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_CHANGECASEMAP  ); break;
        case SvtLanguageOptions::E_DOUBLELINES      : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_DOUBLELINES    ); break;
        case SvtLanguageOptions::E_EMPHASISMARKS    : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_EMPHASISMARKS  ); break;
        case SvtLanguageOptions::E_VERTICALCALLOUT  : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_VERTICALCALLOUT); break;
        case SvtLanguageOptions::E_ALLCJK           : bReadOnly = m_pCJKOptions->IsReadOnly(SvtCJKOptions::E_ALL            ); break;
        // ctl options
        case SvtLanguageOptions::E_CTLFONT              : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLFONT            ); break;
        case SvtLanguageOptions::E_CTLSEQUENCECHECKING  : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLSEQUENCECHECKING); break;
        case SvtLanguageOptions::E_CTLCURSORMOVEMENT    : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLCURSORMOVEMENT  ); break;
        case SvtLanguageOptions::E_CTLTEXTNUMERALS      : bReadOnly = m_pCTLOptions->IsReadOnly(SvtCTLOptions::E_CTLTEXTNUMERALS    ); break;
    }
    return bReadOnly;
}
/* -----------------30.04.2003 11:03-----------------*/

// returns for a language the scripttype
sal_uInt16 SvtLanguageOptions::GetScriptTypeOfLanguage( sal_uInt16 nLang )
{
    if( LANGUAGE_DONTKNOW == nLang )
        nLang = LANGUAGE_ENGLISH_US;
    else if( LANGUAGE_SYSTEM == nLang  )
        nLang = SvtSysLocale().GetLanguage();

    sal_Int16 nScriptType = MsLangId::getScriptType( nLang );
    sal_uInt16 nScript;
    switch (nScriptType)
    {
        case ::com::sun::star::i18n::ScriptType::ASIAN:
            nScript = SCRIPTTYPE_ASIAN;
            break;
        case ::com::sun::star::i18n::ScriptType::COMPLEX:
            nScript = SCRIPTTYPE_COMPLEX;
            break;
        default:
            nScript = SCRIPTTYPE_LATIN;
    }
    return nScript;
}
// -----------------------------------------------------------------------------


/*-- 27.10.2005 08:18:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SvtSystemLanguageOptions::SvtSystemLanguageOptions() :
    utl::ConfigItem( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("System/L10N") ))
{
    uno::Sequence< rtl::OUString > aPropertyNames(1);
    rtl::OUString* pNames = aPropertyNames.getArray();
    pNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SystemLocale"));
    uno::Sequence< uno::Any > aValues = GetProperties( aPropertyNames );

    if ( aValues.getLength() )
    {
        aValues[0]>>= m_sWin16SystemLocale;
    }
}
/*-- 27.10.2005 08:18:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SvtSystemLanguageOptions::~SvtSystemLanguageOptions()
{
}
/*-- 27.10.2005 08:18:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SvtSystemLanguageOptions::Commit()
{
    //does nothing
}

void    SvtSystemLanguageOptions::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
    // no listeners supported yet
}

/*-- 27.10.2005 08:36:14---------------------------------------------------

  -----------------------------------------------------------------------*/
LanguageType SvtSystemLanguageOptions::GetWin16SystemLanguage()
{
    if( m_sWin16SystemLocale.getLength() == 0 )
        return LANGUAGE_NONE;
    return MsLangId::convertIsoStringToLanguage( m_sWin16SystemLocale );
}


