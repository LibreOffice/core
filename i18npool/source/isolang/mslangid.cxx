/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mslangid.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:46:48 $
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

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

#include "i18npool/mslangid.hxx"


LanguageType MsLangId::nConfiguredSystemLanguage   = LANGUAGE_SYSTEM;
LanguageType MsLangId::nConfiguredSystemUILanguage = LANGUAGE_SYSTEM;


// static
void MsLangId::setConfiguredSystemLanguage( LanguageType nLang )
{
    nConfiguredSystemLanguage = nLang;
}


// static
void MsLangId::setConfiguredSystemUILanguage( LanguageType nLang )
{
    nConfiguredSystemUILanguage = nLang;
}


// static
inline LanguageType MsLangId::simplifySystemLanguages( LanguageType nLang )
{
    switch (nLang)
    {
        case LANGUAGE_PROCESS_OR_USER_DEFAULT :
        case LANGUAGE_SYSTEM_DEFAULT :
        case LANGUAGE_SYSTEM :
            nLang = LANGUAGE_SYSTEM;
            break;
        default:
            ;   // nothing
    }
    return nLang;
}


// static
LanguageType MsLangId::getRealLanguageWithoutConfig( LanguageType nLang )
{
    switch (simplifySystemLanguages( nLang))
    {
        case LANGUAGE_SYSTEM :
            nLang = getSystemLanguage();
            break;
        case LANGUAGE_NONE :
            nLang = getSystemUILanguage();
            break;
        default:
            ;   // nothing
    }
    if (nLang == LANGUAGE_DONTKNOW)
        nLang = LANGUAGE_ENGLISH_US;
    return nLang;
}


// static
LanguageType MsLangId::getRealLanguage( LanguageType nLang )
{
    switch (simplifySystemLanguages( nLang))
    {
        case LANGUAGE_SYSTEM :
            if (nConfiguredSystemLanguage == LANGUAGE_SYSTEM)
                nLang = getSystemLanguage();
            else
                nLang = nConfiguredSystemLanguage;
            break;
        case LANGUAGE_NONE :
            if (nConfiguredSystemUILanguage == LANGUAGE_SYSTEM)
                nLang = getSystemUILanguage();
            else
                nLang = nConfiguredSystemUILanguage;
            break;
        default:
            ;   // nothing
    }
    if (nLang == LANGUAGE_DONTKNOW)
        nLang = LANGUAGE_ENGLISH_US;
    return nLang;
}


// static
void MsLangId::convertLanguageToLocale( LanguageType nLang,
        ::com::sun::star::lang::Locale & rLocale )
{
    if (rLocale.Variant.getLength())
        rLocale.Variant = rtl::OUString();
    convertLanguageToIsoNames( nLang, rLocale.Language, rLocale.Country);
}


// static
::com::sun::star::lang::Locale MsLangId::convertLanguageToLocale(
        LanguageType nLang, bool bResolveSystem )
{
    ::com::sun::star::lang::Locale aLocale;
    if (!bResolveSystem && simplifySystemLanguages( nLang) == LANGUAGE_SYSTEM)
        ;   // nothing => empty locale
    else
    {
        nLang = MsLangId::getRealLanguage( nLang);
        convertLanguageToLocale( nLang, aLocale);
    }
    return aLocale;
}


// static
LanguageType MsLangId::convertLocaleToLanguage(
        const ::com::sun::star::lang::Locale& rLocale )
{
    // empty language => LANGUAGE_SYSTEM
    if (rLocale.Language.getLength() == 0)
        return LANGUAGE_SYSTEM;

    LanguageType nRet = convertIsoNamesToLanguage( rLocale.Language,
            rLocale.Country);
    if (nRet == LANGUAGE_DONTKNOW)
        nRet = LANGUAGE_SYSTEM;

    return nRet;
}


// static
bool MsLangId::isRightToLeft( LanguageType nLang )
{
    switch( nLang & LANGUAGE_MASK_PRIMARY )
    {
        case LANGUAGE_ARABIC & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_HEBREW & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_URDU   & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_FARSI  & LANGUAGE_MASK_PRIMARY :
            return true;

        default:
            break;
    }
    return false;
}


// static
bool MsLangId::hasForbiddenCharacters( LanguageType nLang )
{
    switch (nLang & LANGUAGE_MASK_PRIMARY)
    {
        case LANGUAGE_CHINESE  & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_JAPANESE & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_KOREAN   & LANGUAGE_MASK_PRIMARY:
            return true;
        default:
            break;
    }
    return false;
}


// static
sal_Int16 MsLangId::getScriptType( LanguageType nLang )
{
    sal_Int16 nScript;
    switch( nLang )
    {
        // CJK
        // all LANGUAGE_CHINESE_... are caught below
        case LANGUAGE_JAPANESE:
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
        case LANGUAGE_USER_KOREAN_NORTH:
            nScript = ::com::sun::star::i18n::ScriptType::ASIAN;
            break;

        // CTL
        // all LANGUAGE_ARABIC_... are caught below
        case LANGUAGE_ASSAMESE:
        case LANGUAGE_BENGALI:
        case LANGUAGE_BENGALI_BANGLADESH:
        case LANGUAGE_FARSI:
        case LANGUAGE_HEBREW:
        case LANGUAGE_MARATHI:
        case LANGUAGE_PUNJABI:
        case LANGUAGE_GUJARATI:
        case LANGUAGE_HINDI:
        case LANGUAGE_KANNADA:
        case LANGUAGE_KASHMIRI:
        case LANGUAGE_KASHMIRI_INDIA:
        case LANGUAGE_KHMER:
        case LANGUAGE_LAO:
        case LANGUAGE_MALAYALAM:
        case LANGUAGE_MANIPURI:
        case LANGUAGE_MONGOLIAN_MONGOLIAN:
        case LANGUAGE_NEPALI:
        case LANGUAGE_NEPALI_INDIA:
        case LANGUAGE_ORIYA:
        case LANGUAGE_SANSKRIT:
        case LANGUAGE_SINDHI:
        case LANGUAGE_SINDHI_PAKISTAN:
        case LANGUAGE_SINHALESE_SRI_LANKA:
        case LANGUAGE_SYRIAC:
        case LANGUAGE_TAMIL:
        case LANGUAGE_TELUGU:
        case LANGUAGE_THAI:
        case LANGUAGE_TIBETAN:
        case LANGUAGE_DZONGKHA:
        case LANGUAGE_URDU:
        case LANGUAGE_URDU_PAKISTAN:
        case LANGUAGE_URDU_INDIA:
        case LANGUAGE_USER_KURDISH_IRAQ:
        case LANGUAGE_USER_KURDISH_IRAN:
        case LANGUAGE_VIETNAMESE:
            nScript = ::com::sun::star::i18n::ScriptType::COMPLEX;
            break;

// currently not knowing scripttype - defaulted to LATIN:
/*
#define LANGUAGE_ARMENIAN                   0x042B
#define LANGUAGE_INDONESIAN                 0x0421
#define LANGUAGE_KAZAK                      0x043F
#define LANGUAGE_KONKANI                    0x0457
#define LANGUAGE_MACEDONIAN                 0x042F
#define LANGUAGE_TATAR                      0x0444
*/

    default:
        switch ( nLang & LANGUAGE_MASK_PRIMARY )
        {
            // CJK catcher
            case LANGUAGE_CHINESE & LANGUAGE_MASK_PRIMARY:
                nScript = ::com::sun::star::i18n::ScriptType::ASIAN;
                break;
            // CTL catcher
            case LANGUAGE_ARABIC & LANGUAGE_MASK_PRIMARY:
                nScript = ::com::sun::star::i18n::ScriptType::COMPLEX;
                break;
            // Western (actually not necessarily Latin but also Cyrillic, for example)
            default:
                nScript = ::com::sun::star::i18n::ScriptType::LATIN;
        }
        break;
    }
    return nScript;
}


// static
LanguageType MsLangId::getReplacementForObsoleteLanguage( LanguageType nLang )
{
    switch (nLang)
    {
        case LANGUAGE_OBSOLETE_USER_LATIN:
            nLang = LANGUAGE_LATIN;
        case LANGUAGE_OBSOLETE_USER_MAORI:
            nLang = LANGUAGE_MAORI_NEW_ZEALAND;
        case LANGUAGE_OBSOLETE_USER_KINYARWANDA:
            nLang = LANGUAGE_KINYARWANDA_RWANDA;
        case LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN:
            nLang = LANGUAGE_UPPER_SORBIAN_GERMANY;
        case LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN:
            nLang = LANGUAGE_LOWER_SORBIAN_GERMANY;
        case LANGUAGE_OBSOLETE_USER_OCCITAN:
            nLang = LANGUAGE_OCCITAN_FRANCE;
        case LANGUAGE_OBSOLETE_USER_BRETON:
            nLang = LANGUAGE_BRETON_FRANCE;
        case LANGUAGE_OBSOLETE_USER_KALAALLISUT:
            nLang = LANGUAGE_KALAALLISUT_GREENLAND;
        case LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH:
            nLang = LANGUAGE_LUXEMBOURGISH_LUXEMBOURG;
        default:
            ;   // nothing
    }
    return nLang;
}
