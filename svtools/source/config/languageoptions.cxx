/*************************************************************************
 *
 *  $RCSfile: languageoptions.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 15:30:02 $
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

#pragma hdrstop

#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include "languageoptions.hxx"
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <cjkoptions.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <ctloptions.hxx>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

// global ----------------------------------------------------------------------

namespace { struct ALMutex : public rtl::Static< ::osl::Mutex, ALMutex > {}; }

// class SvtLanguageOptions ----------------------------------------------------

SvtLanguageOptions::SvtLanguageOptions( sal_Bool _bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

    m_pCJKOptions = new SvtCJKOptions( _bDontLoad );
    m_pCTLOptions = new SvtCTLOptions( _bDontLoad );
    StartListening(*m_pCTLOptions);
}
//------------------------------------------------------------------------------
SvtLanguageOptions::~SvtLanguageOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ALMutex::get() );

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
/* -----------------30.04.2003 11:03-----------------

 --------------------------------------------------*/
void SvtLanguageOptions::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );
    Broadcast( rHint );
}

// -----------------------------------------------------------------------------
// returns for a language the scripttype
sal_uInt16 SvtLanguageOptions::GetScriptTypeOfLanguage( sal_uInt16 nLang )
{
    if( LANGUAGE_DONTKNOW == nLang )
        nLang = LANGUAGE_ENGLISH_US;
    else if( LANGUAGE_SYSTEM == nLang  )
        nLang = Application::GetSettings().GetLanguage();

    USHORT nScript;
    switch( nLang )
    {
        // CJK
        case LANGUAGE_CHINESE:
        case LANGUAGE_CHINESE_TRADITIONAL:
        case LANGUAGE_CHINESE_SIMPLIFIED:
        case LANGUAGE_CHINESE_HONGKONG:
        case LANGUAGE_CHINESE_SINGAPORE:
        case LANGUAGE_CHINESE_MACAU:
        case LANGUAGE_JAPANESE:
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
        case LANGUAGE_USER_KOREAN_NORTH:
            nScript = SCRIPTTYPE_ASIAN;
            break;

        // CTL
        case LANGUAGE_ARABIC:
        case LANGUAGE_ARABIC_SAUDI_ARABIA:
        case LANGUAGE_ARABIC_IRAQ:
        case LANGUAGE_ARABIC_EGYPT:
        case LANGUAGE_ARABIC_LIBYA:
        case LANGUAGE_ARABIC_ALGERIA:
        case LANGUAGE_ARABIC_MOROCCO:
        case LANGUAGE_ARABIC_TUNISIA:
        case LANGUAGE_ARABIC_OMAN:
        case LANGUAGE_ARABIC_YEMEN:
        case LANGUAGE_ARABIC_SYRIA:
        case LANGUAGE_ARABIC_JORDAN:
        case LANGUAGE_ARABIC_LEBANON:
        case LANGUAGE_ARABIC_KUWAIT:
        case LANGUAGE_ARABIC_UAE:
        case LANGUAGE_ARABIC_BAHRAIN:
        case LANGUAGE_ARABIC_QATAR:
        case LANGUAGE_FARSI:
        case LANGUAGE_HEBREW:
        case LANGUAGE_MARATHI:
        case LANGUAGE_PUNJABI:
        case LANGUAGE_GUJARATI:
        case LANGUAGE_HINDI:
        case LANGUAGE_KANNADA:
        case LANGUAGE_TAMIL:
        case LANGUAGE_TELUGU:
        case LANGUAGE_THAI:
        case LANGUAGE_URDU:
        case LANGUAGE_URDU_PAKISTAN:
        case LANGUAGE_URDU_INDIA:
        case LANGUAGE_VIETNAMESE: // not included in langtab.src?
            nScript = SCRIPTTYPE_COMPLEX;
            break;

// currently not knowing scripttype - defaultet to LATIN:
/*
#define LANGUAGE_AFRIKAANS                  0x0436
#define LANGUAGE_ARMENIAN                   0x042B
#define LANGUAGE_ASSAMESE                   0x044D
#define LANGUAGE_AZERI                      0x002C
#define LANGUAGE_AZERI_LATIN                0x042C
#define LANGUAGE_AZERI_CYRILLIC             0x082C
#define LANGUAGE_BASQUE                     0x042D
#define LANGUAGE_BELARUSIAN                 0x0423
#define LANGUAGE_BENGALI                    0x0445
#define LANGUAGE_INDONESIAN                 0x0421
#define LANGUAGE_KASHMIRI                   0x0460
#define LANGUAGE_KASHMIRI_INDIA             0x0860
#define LANGUAGE_KAZAK                      0x043F
#define LANGUAGE_KONKANI                    0x0457
#define LANGUAGE_LATVIAN                    0x0426
#define LANGUAGE_LITHUANIAN                 0x0427
#define LANGUAGE_LITHUANIAN_CLASSIC         0x0827
#define LANGUAGE_MACEDONIAN                 0x042F
#define LANGUAGE_MALAY                      0x003E
#define LANGUAGE_MALAY_MALAYSIA             0x043E
#define LANGUAGE_MALAY_BRUNEI_DARUSSALAM    0x083E
#define LANGUAGE_MALAYALAM                  0x044C
#define LANGUAGE_MANIPURI                   0x0458
#define LANGUAGE_NEPALI                     0x0461
#define LANGUAGE_NEPALI_INDIA               0x0861
#define LANGUAGE_ORIYA                      0x0448
#define LANGUAGE_SANSKRIT                   0x044F
#define LANGUAGE_SERBIAN                    0x041A
#define LANGUAGE_SERBIAN_LATIN              0x081A
#define LANGUAGE_SERBIAN_CYRILLIC           0x0C1A
#define LANGUAGE_SINDHI                     0x0459
#define LANGUAGE_SWAHILI                    0x5041
#define LANGUAGE_TATAR                      0x0444
#define LANGUAGE_TURKISH                    0x041F
#define LANGUAGE_UKRAINIAN                  0x0422
#define LANGUAGE_UZBEK                      0x0043
#define LANGUAGE_UZBEK_LATIN                0x0443
#define LANGUAGE_UZBEK_CYRILLIC             0x0843
*/

    default:
        nScript = SCRIPTTYPE_LATIN;
        break;
    }
    return nScript;
}
// -----------------------------------------------------------------------------
