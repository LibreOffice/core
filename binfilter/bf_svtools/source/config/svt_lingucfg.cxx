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



#include <com/sun/star/lang/Locale.hpp>

#include <i18npool/mslangid.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <sal/macros.h>

#include <bf_svtools/lingucfg.hxx>
#include <linguprops.hxx>

#include <itemholder1.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

namespace binfilter
{

#define A2OU(x)        ::rtl::OUString::createFromAscii( x )

///////////////////////////////////////////////////////////////////////////


static osl::Mutex &  GetOwnMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}


///////////////////////////////////////////////////////////////////////////


static inline INT16 lcl_CfgLocaleStrToLanguage( const OUString &rCfgLocaleStr )
{
    INT16 nRes = LANGUAGE_NONE;
    if (0 != rCfgLocaleStr.getLength())
        nRes = MsLangId::convertIsoStringToLanguage( rCfgLocaleStr );
    return nRes;
}


static inline const OUString lcl_LanguageToCfgLocaleStr( INT16 nLanguage )
{
    OUString aRes;
    if (LANGUAGE_NONE != nLanguage)
        aRes = MsLangId::convertLanguageToIsoString( nLanguage );
    return aRes;
}


static void lcl_CfgAnyToLanguage( const Any &rVal, INT16& rLanguage )
{
    OUString aTmp;
    if ((rVal >>= aTmp)  &&  0 != aTmp.getLength())
       rLanguage = MsLangId::convertIsoStringToLanguage( aTmp );
}


//////////////////////////////////////////////////////////////////////

SvtLinguOptions::SvtLinguOptions()
{
    nDefaultLanguage = LANGUAGE_NONE;

    nDefaultLanguage_CJK = LANGUAGE_NONE;
    nDefaultLanguage_CTL = LANGUAGE_NONE;

    // general options
    bIsGermanPreReform		= FALSE;
    bIsUseDictionaryList	=
    bIsIgnoreControlCharacters	= TRUE;

    // spelling options
    bIsSpellCapitalization	=
    bIsSpellSpecial			= TRUE;
    bIsSpellAuto			=
    bIsSpellInAllLanguages	=
    bIsSpellHideMarkings	=
    bIsSpellReverse			=
    bIsSpellWithDigits		=
    bIsSpellUpperCase		= FALSE;

    // text conversion options
    bIsIgnorePostPositionalWord     = TRUE;
    bIsAutoCloseDialog              = 
    bIsShowEntriesRecentlyUsedFirst = 
    bIsAutoReplaceUniqueEntries     = FALSE;
    bIsDirectionToSimplified        = TRUE;
    bIsUseCharacterVariants         =
    bIsTranslateCommonTerms         =
    bIsReverseMapping               = FALSE;

    bROIsDirectionToSimplified      =
    bROIsUseCharacterVariants       =
    bROIsTranslateCommonTerms       =
    bROIsReverseMapping             = FALSE;

    // hyphenation options
    bIsHyphSpecial			= TRUE;
    bIsHyphAuto				= FALSE;
    nHyphMinLeading			=
    nHyphMinTrailing		= 2;
    nHyphMinWordLength		= 0;

    nDataFilesChangedCheckValue = 0;
}


//////////////////////////////////////////////////////////////////////


class SvtLinguConfigItem : public utl::ConfigItem
{
    SvtLinguOptions     aOpt;

    // disallow copy-constructor and assignment-operator for now
    SvtLinguConfigItem( const SvtLinguConfigItem & );
    SvtLinguConfigItem & operator = ( const SvtLinguConfigItem & );

    static BOOL GetHdlByName( INT32 &rnHdl, const OUString &rPropertyName, sal_Bool bFullPropName = sal_False );
    static const Sequence< OUString > & GetPropertyNames();
    BOOL                LoadOptions( const Sequence< OUString > &rProperyNames );
    BOOL                SaveOptions( const Sequence< OUString > &rProperyNames );

public:
    SvtLinguConfigItem();
    virtual ~SvtLinguConfigItem();

    // utl::ConfigItem
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void    Commit();

    // make some protected functions of utl::ConfigItem public
    using utl::ConfigItem::GetNodeNames;
    using utl::ConfigItem::GetProperties;
    //using utl::ConfigItem::PutProperties;
    //using utl::ConfigItem::SetSetProperties;
    using utl::ConfigItem::ReplaceSetProperties;
    //using utl::ConfigItem::GetReadOnlyStates;

    BOOL    GetOptions( SvtLinguOptions &rOptions ) const;
};


SvtLinguConfigItem::SvtLinguConfigItem() : 
    utl::ConfigItem( String::CreateFromAscii( "Office.Linguistic" ) )
{
    LoadOptions( GetPropertyNames() );
    ClearModified();

    // request notify events when properties change
    EnableNotification( GetPropertyNames() );
}


SvtLinguConfigItem::~SvtLinguConfigItem()
{
    //! Commit (SaveOptions) will be called by the d-tor of the base called !
}


void SvtLinguConfigItem::Notify( const Sequence< OUString > &rPropertyNames )
{
    LoadOptions( rPropertyNames );
}


void SvtLinguConfigItem::Commit()
{
    SaveOptions( GetPropertyNames() );
}


static struct NamesToHdl
{
    const char   *pFullPropName;      // full qualified name as used in configuration
    const char   *pPropName;          // property name only (atom) of above
    INT32   nHdl;               // numeric handle representing the property
}aNamesToHdl[] =
{
{/*  0 */    "General/DefaultLocale",                         UPN_DEFAULT_LOCALE,                    UPH_DEFAULT_LOCALE},
{/*  1 */    "General/DictionaryList/ActiveDictionaries",     UPN_ACTIVE_DICTIONARIES,               UPH_ACTIVE_DICTIONARIES},
{/*  2 */    "General/DictionaryList/IsUseDictionaryList",    UPN_IS_USE_DICTIONARY_LIST,            UPH_IS_USE_DICTIONARY_LIST},
{/*  3 */    "General/IsIgnoreControlCharacters",             UPN_IS_IGNORE_CONTROL_CHARACTERS,      UPH_IS_IGNORE_CONTROL_CHARACTERS},
{/*  4 */    "General/IsGermanPreReform",                     UPN_IS_GERMAN_PRE_REFORM,              UPH_IS_GERMAN_PRE_REFORM},
{/*  5 */    "General/DefaultLocale_CJK",                     UPN_DEFAULT_LOCALE_CJK,                UPH_DEFAULT_LOCALE_CJK},
{/*  6 */    "General/DefaultLocale_CTL",                     UPN_DEFAULT_LOCALE_CTL,                UPH_DEFAULT_LOCALE_CTL},

{/*  7 */    "SpellChecking/IsSpellUpperCase",                UPN_IS_SPELL_UPPER_CASE,               UPH_IS_SPELL_UPPER_CASE},
{/*  8 */    "SpellChecking/IsSpellWithDigits",               UPN_IS_SPELL_WITH_DIGITS,              UPH_IS_SPELL_WITH_DIGITS},
{/*  9 */    "SpellChecking/IsSpellCapitalization",           UPN_IS_SPELL_CAPITALIZATION,           UPH_IS_SPELL_CAPITALIZATION},
{/* 10 */    "SpellChecking/IsSpellAuto",                     UPN_IS_SPELL_AUTO,                     UPH_IS_SPELL_AUTO},
{/* 11 */    "SpellChecking/IsSpellSpecial",                  UPN_IS_SPELL_SPECIAL,                  UPH_IS_SPELL_SPECIAL},
{/* 12 */    "SpellChecking/IsSpellInAllLocales",             UPN_IS_SPELL_IN_ALL_LANGUAGES,         UPH_IS_SPELL_IN_ALL_LANGUAGES},
{/* 13 */    "SpellChecking/IsHideMarkings",                  UPN_IS_SPELL_HIDE,                     UPH_IS_SPELL_HIDE},
{/* 14 */    "SpellChecking/IsReverseDirection",              UPN_IS_WRAP_REVERSE,                   UPH_IS_WRAP_REVERSE},

{/* 15 */    "Hyphenation/MinLeading",                        UPN_HYPH_MIN_LEADING,                  UPH_HYPH_MIN_LEADING},
{/* 16 */    "Hyphenation/MinTrailing",                       UPN_HYPH_MIN_TRAILING,                 UPH_HYPH_MIN_TRAILING},
{/* 17 */    "Hyphenation/MinWordLength",                     UPN_HYPH_MIN_WORD_LENGTH,              UPH_HYPH_MIN_WORD_LENGTH},
{/* 18 */    "Hyphenation/IsHyphSpecial",                     UPN_IS_HYPH_SPECIAL,                   UPH_IS_HYPH_SPECIAL},
{/* 19 */    "Hyphenation/IsHyphAuto",                        UPN_IS_HYPH_AUTO,                      UPH_IS_HYPH_AUTO},

{/* 20 */    "TextConversion/ActiveConversionDictionaries",   UPN_ACTIVE_CONVERSION_DICTIONARIES,        UPH_ACTIVE_CONVERSION_DICTIONARIES},
{/* 21 */    "TextConversion/IsIgnorePostPositionalWord",     UPN_IS_IGNORE_POST_POSITIONAL_WORD,        UPH_IS_IGNORE_POST_POSITIONAL_WORD},
{/* 22 */    "TextConversion/IsAutoCloseDialog",              UPN_IS_AUTO_CLOSE_DIALOG,                  UPH_IS_AUTO_CLOSE_DIALOG},
{/* 23 */    "TextConversion/IsShowEntriesRecentlyUsedFirst", UPN_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST,   UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST},
{/* 24 */    "TextConversion/IsAutoReplaceUniqueEntries",     UPN_IS_AUTO_REPLACE_UNIQUE_ENTRIES,        UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES},
{/* 25 */    "TextConversion/IsDirectionToSimplified",        UPN_IS_DIRECTION_TO_SIMPLIFIED,            UPH_IS_DIRECTION_TO_SIMPLIFIED},
{/* 26 */    "TextConversion/IsUseCharacterVariants",         UPN_IS_USE_CHARACTER_VARIANTS,             UPH_IS_USE_CHARACTER_VARIANTS},
{/* 27 */    "TextConversion/IsTranslateCommonTerms",         UPN_IS_TRANSLATE_COMMON_TERMS,             UPH_IS_TRANSLATE_COMMON_TERMS},
{/* 28 */    "TextConversion/IsReverseMapping",               UPN_IS_REVERSE_MAPPING,                    UPH_IS_REVERSE_MAPPING},

{/* 29 */    "ServiceManager/DataFilesChangedCheckValue",     UPN_DATA_FILES_CHANGED_CHECK_VALUE,        UPH_DATA_FILES_CHANGED_CHECK_VALUE},

            /* similar to entry 0 (thus no own configuration entry) but with different property name and type */
{            NULL,											 UPN_DEFAULT_LANGUAGE,                      UPH_DEFAULT_LANGUAGE},

{            NULL,                                            NULL,                                      -1}
};


const Sequence< OUString > & SvtLinguConfigItem::GetPropertyNames()
{
    static Sequence< OUString > aNames;
    static sal_Bool bInitialized = sal_False;

    if (!bInitialized)
    {
        INT32 nMax = SAL_N_ELEMENTS(aNamesToHdl);

        aNames.realloc( nMax );
        OUString *pNames = aNames.getArray();
        INT32 nIdx = 0;
        for (INT32 i = 0; i < nMax;  ++i)
        {
            const sal_Char *pFullPropName = aNamesToHdl[i].pFullPropName;
            if (pFullPropName)
                pNames[ nIdx++ ] = A2OU( pFullPropName );
        }
        aNames.realloc( nIdx );
        bInitialized = sal_True;
    }
    return aNames;
}


BOOL SvtLinguConfigItem::GetHdlByName( 
    INT32 &rnHdl, 
    const OUString &rPropertyName,
    sal_Bool bFullPropName )
{
    NamesToHdl *pEntry = &aNamesToHdl[0];

    if (bFullPropName)
    {
        while (pEntry && pEntry->pFullPropName != NULL)
        {
            if (0 == rPropertyName.compareToAscii( pEntry->pFullPropName ))
            {
                rnHdl = pEntry->nHdl;
                break;
            }
            ++pEntry;
        }
        return pEntry && pEntry->pFullPropName != NULL;
    }
    else
    {
        while (pEntry && pEntry->pPropName != NULL)
        {
            if (0 == rPropertyName.compareToAscii( pEntry->pPropName ))
            {
                rnHdl = pEntry->nHdl;
                break;
            }
            ++pEntry;
        }
        return pEntry && pEntry->pPropName != NULL;
    }
}

BOOL SvtLinguConfigItem::GetOptions( SvtLinguOptions &rOptions ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    rOptions = aOpt;
    return TRUE;
}

BOOL SvtLinguConfigItem::LoadOptions( const Sequence< OUString > &rProperyNames )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bRes = FALSE;

    const OUString *pProperyNames = rProperyNames.getConstArray();
    INT32 nProps = rProperyNames.getLength();

    const Sequence< Any > aValues = GetProperties( rProperyNames );
    const Sequence< sal_Bool > aROStates = GetReadOnlyStates( rProperyNames );

    if (nProps  &&  aValues.getLength() == nProps &&  aROStates.getLength() == nProps)
    {
        SvtLinguOptions &rOpt = aOpt;
        
        const Any *pValue = aValues.getConstArray();
        const sal_Bool *pROStates = aROStates.getConstArray();
        for (INT32 i = 0;  i < nProps;  ++i)
        {
            const Any &rVal = pValue[i];
            INT32 nPropertyHandle;
            GetHdlByName( nPropertyHandle, pProperyNames[i], sal_True );
            switch ( nPropertyHandle )
            {
                case UPH_DEFAULT_LOCALE : 
                    { rOpt.bRODefaultLanguage = pROStates[i]; lcl_CfgAnyToLanguage( rVal, rOpt.nDefaultLanguage ); } break;
                case UPH_ACTIVE_DICTIONARIES : 
                    { rOpt.bROActiveDics = pROStates[i]; rVal >>= rOpt.aActiveDics;   } break;
                case UPH_IS_USE_DICTIONARY_LIST : 
                    { rOpt.bROIsUseDictionaryList = pROStates[i]; rVal >>= rOpt.bIsUseDictionaryList;  } break;
                case UPH_IS_IGNORE_CONTROL_CHARACTERS : 
                    { rOpt.bROIsIgnoreControlCharacters = pROStates[i]; rVal >>= rOpt.bIsIgnoreControlCharacters;    } break;
                case UPH_IS_GERMAN_PRE_REFORM : 
                    { rOpt.bROIsGermanPreReform = pROStates[i]; rVal >>= rOpt.bIsGermanPreReform;    } break;
                case UPH_DEFAULT_LOCALE_CJK : 
                    { rOpt.bRODefaultLanguage_CJK = pROStates[i]; lcl_CfgAnyToLanguage( rVal, rOpt.nDefaultLanguage_CJK );    } break;
                case UPH_DEFAULT_LOCALE_CTL : 
                    { rOpt.bRODefaultLanguage_CTL = pROStates[i]; lcl_CfgAnyToLanguage( rVal, rOpt.nDefaultLanguage_CTL );    } break;

                case UPH_IS_SPELL_UPPER_CASE : 
                    { rOpt.bROIsSpellUpperCase = pROStates[i]; rVal >>= rOpt.bIsSpellUpperCase; } break;
                case UPH_IS_SPELL_WITH_DIGITS : 
                    { rOpt.bROIsSpellWithDigits = pROStates[i]; rVal >>= rOpt.bIsSpellWithDigits;    } break;
                case UPH_IS_SPELL_CAPITALIZATION : 
                    { rOpt.bROIsSpellCapitalization = pROStates[i]; rVal >>= rOpt.bIsSpellCapitalization;    } break;
                case UPH_IS_SPELL_AUTO : 
                    { rOpt.bROIsSpellAuto = pROStates[i]; rVal >>= rOpt.bIsSpellAuto;  } break;
                case UPH_IS_SPELL_SPECIAL : 
                    { rOpt.bROIsSpellSpecial = pROStates[i]; rVal >>= rOpt.bIsSpellSpecial;   } break;
                case UPH_IS_SPELL_IN_ALL_LANGUAGES : 
                    { rOpt.bROIsSpellInAllLanguages = pROStates[i]; rVal >>= rOpt.bIsSpellInAllLanguages;    } break;
                case UPH_IS_SPELL_HIDE : 
                    { rOpt.bROIsSpellHideMarkings = pROStates[i]; rVal >>= rOpt.bIsSpellHideMarkings;  } break;
                case UPH_IS_WRAP_REVERSE : 
                    { rOpt.bROIsSpellReverse = pROStates[i]; rVal >>= rOpt.bIsSpellReverse;   } break;

                case UPH_HYPH_MIN_LEADING : 
                    { rOpt.bROHyphMinLeading = pROStates[i]; rVal >>= rOpt.nHyphMinLeading;   } break;
                case UPH_HYPH_MIN_TRAILING : 
                    { rOpt.bROHyphMinTrailing = pROStates[i]; rVal >>= rOpt.nHyphMinTrailing;  } break;
                case UPH_HYPH_MIN_WORD_LENGTH : 
                    { rOpt.bROHyphMinWordLength = pROStates[i]; rVal >>= rOpt.nHyphMinWordLength;    } break;
                case UPH_IS_HYPH_SPECIAL : 
                    { rOpt.bROIsHyphSpecial = pROStates[i]; rVal >>= rOpt.bIsHyphSpecial;    } break;
                case UPH_IS_HYPH_AUTO : 
                    { rOpt.bROIsHyphAuto = pROStates[i]; rVal >>= rOpt.bIsHyphAuto;   } break;

                case UPH_ACTIVE_CONVERSION_DICTIONARIES : { rOpt.bROActiveConvDics = pROStates[i]; rVal >>= rOpt.aActiveConvDics;   } break;

                case UPH_IS_IGNORE_POST_POSITIONAL_WORD : 
                    { rOpt.bROIsIgnorePostPositionalWord = pROStates[i]; rVal >>= rOpt.bIsIgnorePostPositionalWord;  } break;
                case UPH_IS_AUTO_CLOSE_DIALOG : 
                    { rOpt.bROIsAutoCloseDialog = pROStates[i]; rVal >>= rOpt.bIsAutoCloseDialog;  } break;
                case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST : 
                    { rOpt.bROIsShowEntriesRecentlyUsedFirst = pROStates[i]; rVal >>= rOpt.bIsShowEntriesRecentlyUsedFirst;  } break;
                case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES : 
                    { rOpt.bROIsAutoReplaceUniqueEntries = pROStates[i]; rVal >>= rOpt.bIsAutoReplaceUniqueEntries;  } break;

                case UPH_IS_DIRECTION_TO_SIMPLIFIED : 
                    { rOpt.bROIsDirectionToSimplified = pROStates[i];
                            if( ! (rVal >>= rOpt.bIsDirectionToSimplified) )
                            {
                                //default is locale dependent:
                                if(  rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_HONGKONG
                                  || rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_MACAU
                                  || rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_TRADITIONAL )
                                {
                                    rOpt.bIsDirectionToSimplified = FALSE;
                                }
                                else
                                {
                                    rOpt.bIsDirectionToSimplified = TRUE;
                                }
                            }
                    } break;
                case UPH_IS_USE_CHARACTER_VARIANTS : 
                    { rOpt.bROIsUseCharacterVariants = pROStates[i]; rVal >>= rOpt.bIsUseCharacterVariants;  } break;
                case UPH_IS_TRANSLATE_COMMON_TERMS : 
                    { rOpt.bROIsTranslateCommonTerms = pROStates[i]; rVal >>= rOpt.bIsTranslateCommonTerms;  } break;
                case UPH_IS_REVERSE_MAPPING : 
                    { rOpt.bROIsReverseMapping = pROStates[i]; rVal >>= rOpt.bIsReverseMapping;  } break;

                case UPH_DATA_FILES_CHANGED_CHECK_VALUE : 
                    { rOpt.bRODataFilesChangedCheckValue = pROStates[i]; rVal >>= rOpt.nDataFilesChangedCheckValue;  } break;
                default:
                    DBG_ERROR( "unexpected case" );
            }
        }

        bRes = TRUE;
    }
    DBG_ASSERT( bRes, "LoadOptions failed" );

    return bRes;
}


BOOL SvtLinguConfigItem::SaveOptions( const Sequence< OUString > &rProperyNames )
{
    if (!IsModified())
        return TRUE;

    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bRet = FALSE;
    const Type &rBOOL     = ::getBooleanCppuType();
    const Type &rINT16    = ::getCppuType( (INT16 *) NULL );
    const Type &rINT32    = ::getCppuType( (INT32 *) NULL );

    INT32 nProps = rProperyNames.getLength();
    Sequence< Any > aValues( nProps );
    Any *pValue = aValues.getArray();

    if (nProps  &&  aValues.getLength() == nProps)
    {
        const SvtLinguOptions &rOpt = aOpt;
        
        OUString aTmp( lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage ) );
        *pValue++ = makeAny( aTmp );                                    //   0
        *pValue++ = makeAny( rOpt.aActiveDics );                        //   1
        pValue++->setValue( &rOpt.bIsUseDictionaryList, rBOOL );        //   2
        pValue++->setValue( &rOpt.bIsIgnoreControlCharacters, rBOOL );  //   3
        pValue++->setValue( &rOpt.bIsGermanPreReform, rBOOL );          //   4
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CJK );
        *pValue++ = makeAny( aTmp );                                    //   5
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CTL );
        *pValue++ = makeAny( aTmp );                                    //   6

        pValue++->setValue( &rOpt.bIsSpellUpperCase, rBOOL );          //   7
        pValue++->setValue( &rOpt.bIsSpellWithDigits, rBOOL );         //   8
        pValue++->setValue( &rOpt.bIsSpellCapitalization, rBOOL );     //   9
        pValue++->setValue( &rOpt.bIsSpellAuto, rBOOL );               //  10
        pValue++->setValue( &rOpt.bIsSpellSpecial, rBOOL );                //  11
        pValue++->setValue( &rOpt.bIsSpellInAllLanguages, rBOOL );     //  12
        pValue++->setValue( &rOpt.bIsSpellHideMarkings, rBOOL );       //  13
        pValue++->setValue( &rOpt.bIsSpellReverse, rBOOL );                //  14

        pValue++->setValue( &rOpt.nHyphMinLeading, rINT16 );           //  15
        pValue++->setValue( &rOpt.nHyphMinTrailing, rINT16 );          //  16
        pValue++->setValue( &rOpt.nHyphMinWordLength, rINT16 );            //  17
        pValue++->setValue( &rOpt.bIsHyphSpecial, rBOOL );             //  18
        pValue++->setValue( &rOpt.bIsHyphAuto, rBOOL );                    //  19

        *pValue++ = makeAny( rOpt.aActiveConvDics );                    //   20

        pValue++->setValue( &rOpt.bIsIgnorePostPositionalWord, rBOOL ); //  21
        pValue++->setValue( &rOpt.bIsAutoCloseDialog, rBOOL );          //  22
        pValue++->setValue( &rOpt.bIsShowEntriesRecentlyUsedFirst, rBOOL ); //  23
        pValue++->setValue( &rOpt.bIsAutoReplaceUniqueEntries, rBOOL ); //  24

        pValue++->setValue( &rOpt.bIsDirectionToSimplified, rBOOL ); //  25
        pValue++->setValue( &rOpt.bIsUseCharacterVariants, rBOOL ); //  26
        pValue++->setValue( &rOpt.bIsTranslateCommonTerms, rBOOL ); //  27
        pValue++->setValue( &rOpt.bIsReverseMapping, rBOOL ); //  28

        pValue++->setValue( &rOpt.nDataFilesChangedCheckValue, rINT32 ); //  29

        bRet |= PutProperties( rProperyNames, aValues );
    }

    if (bRet)
        ClearModified();

    return bRet;
}

//////////////////////////////////////////////////////////////////////

static SvtLinguConfigItem *pCfgItem = 0;
static sal_Int32           nCfgItemRefCount = 0;


SvtLinguConfig::SvtLinguConfig()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetOwnMutex() );
    ++nCfgItemRefCount;
}
   
    
SvtLinguConfig::~SvtLinguConfig()
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    if (pCfgItem && pCfgItem->IsModified())
        pCfgItem->Commit();

    if (--nCfgItemRefCount <= 0)
    {
        if (pCfgItem)
            delete pCfgItem;
        pCfgItem = 0;
    }
}


SvtLinguConfigItem & SvtLinguConfig::GetConfigItem()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetOwnMutex() );
    if (!pCfgItem)
    {
        pCfgItem = new SvtLinguConfigItem;
        ItemHolder1::holdConfigItem(E_LINGUCFG);
    }
    ++nCfgItemRefCount;
    return *pCfgItem;
}


Sequence< OUString > SvtLinguConfig::GetNodeNames( const OUString &rNode )
{
    return GetConfigItem().GetNodeNames( rNode );
}


BOOL SvtLinguConfig::GetOptions( SvtLinguOptions &rOptions ) const
{
    return GetConfigItem().GetOptions( rOptions );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
