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


#include <unotools/searchopt.hxx>
#include <tools/solar.h>
#include <tools/debug.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>
#include <sal/macros.h>


using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::i18n;


#define MAX_FLAGS_OFFSET    26

//////////////////////////////////////////////////////////////////////


class SvtSearchOptions_Impl : public ConfigItem
{
    sal_Int32   nFlags;
    sal_Bool    bModified;

    // disallow copy-constructor and assignment-operator for now
    SvtSearchOptions_Impl( const SvtSearchOptions_Impl & );
    SvtSearchOptions_Impl & operator = ( const SvtSearchOptions_Impl & );

protected:
    sal_Bool            IsModified() const { return bModified; }
    using ConfigItem::SetModified;
    void            SetModified( sal_Bool bVal );
    sal_Bool            Load();
    sal_Bool            Save();

    Sequence< OUString >    GetPropertyNames() const;

public:
    SvtSearchOptions_Impl();
    virtual ~SvtSearchOptions_Impl();

    // ConfigItem
    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames );

    sal_Bool            GetFlag( sal_uInt16 nOffset ) const;
    void            SetFlag( sal_uInt16 nOffset, sal_Bool bVal );
};



SvtSearchOptions_Impl::SvtSearchOptions_Impl() :
    ConfigItem( OUString("Office.Common/SearchOptions") )
{
    nFlags = 0x0003FFFF;    // set all options values to 'true'
    Load();
    SetModified( sal_False );
}


SvtSearchOptions_Impl::~SvtSearchOptions_Impl()
{
    Commit();
}


void SvtSearchOptions_Impl::Commit()
{
    if (IsModified())
        Save();
}

void SvtSearchOptions_Impl::Notify( const Sequence< OUString >&  )
{
}


sal_Bool SvtSearchOptions_Impl::GetFlag( sal_uInt16 nOffset ) const
{
    DBG_ASSERT( nOffset <= MAX_FLAGS_OFFSET, "offset out of range");
    return ((nFlags >> nOffset) & 0x01) ? sal_True : sal_False;
}


void SvtSearchOptions_Impl::SetFlag( sal_uInt16 nOffset, sal_Bool bVal )
{
    DBG_ASSERT( nOffset <= MAX_FLAGS_OFFSET, "offset out of range");
    sal_Int32 nOldFlags = nFlags;
    sal_Int32 nMask = ((sal_Int32) 1)  << nOffset;
    if (bVal)
        nFlags |= nMask;
    else
        nFlags &= ~nMask;
    if (nFlags != nOldFlags)
        SetModified( sal_True );
}


void SvtSearchOptions_Impl::SetModified( sal_Bool bVal )
{
    bModified = bVal;
    if (bModified)
    {
        ConfigItem::SetModified();
    }
}


Sequence< OUString > SvtSearchOptions_Impl::GetPropertyNames() const
{
    static const char* aPropNames[ MAX_FLAGS_OFFSET + 1 ] =
    {
        "IsWholeWordsOnly",                     //  0
        "IsBackwards",                          //  1
        "IsUseRegularExpression",               //  2
        //"IsCurrentSelectionOnly",             // interactively set or not...
        "IsSearchForStyles",                    //  3
        "IsSimilaritySearch",                   //  4
        "IsUseAsianOptions",                    //  5
        "IsMatchCase",                          //  6
        "Japanese/IsMatchFullHalfWidthForms",   //  7
        "Japanese/IsMatchHiraganaKatakana",     //  8
        "Japanese/IsMatchContractions",         //  9
        "Japanese/IsMatchMinusDashCho-on",      // 10
        "Japanese/IsMatchRepeatCharMarks",      // 11
        "Japanese/IsMatchVariantFormKanji",     // 12
        "Japanese/IsMatchOldKanaForms",         // 13
        "Japanese/IsMatch_DiZi_DuZu",           // 14
        "Japanese/IsMatch_BaVa_HaFa",           // 15
        "Japanese/IsMatch_TsiThiChi_DhiZi",     // 16
        "Japanese/IsMatch_HyuIyu_ByuVyu",       // 17
        "Japanese/IsMatch_SeShe_ZeJe",          // 18
        "Japanese/IsMatch_IaIya",               // 19
        "Japanese/IsMatch_KiKu",                // 20
        "Japanese/IsIgnorePunctuation",         // 21
        "Japanese/IsIgnoreWhitespace",          // 22
        "Japanese/IsIgnoreProlongedSoundMark",      // 23
        "Japanese/IsIgnoreMiddleDot",           // 24
        "IsNotes",                              // 25
        "IsIgnoreDiacritics_CTL"                // 26
    };

    const int nCount = SAL_N_ELEMENTS( aPropNames );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for (sal_Int32 i = 0;  i < nCount;  ++i)
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}


sal_Bool SvtSearchOptions_Impl::Load()
{
    sal_Bool bSucc = sal_False;

    Sequence< OUString > aNames = GetPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    const Sequence< Any > aValues = GetProperties( aNames );
    DBG_ASSERT( aValues.getLength() == aNames.getLength(),
            "GetProperties failed" );
    //EnableNotification( aNames );

    if (nProps  &&  aValues.getLength() == nProps)
    {
        bSucc = sal_True;

        const Any* pValues = aValues.getConstArray();
        for (sal_uInt16 i = 0;  i < nProps;  ++i)
        {
            const Any &rVal = pValues[i];
            DBG_ASSERT( rVal.hasValue(), "property value missing" );
            if (rVal.hasValue())
            {
                sal_Bool bVal = sal_Bool();
                if (rVal >>= bVal)
                {
                    if (i <= MAX_FLAGS_OFFSET)
                    {
                        // use index in sequence as flag index
                        SetFlag( i, bVal );
                    }
                    else {
                        OSL_FAIL( "unexpected index" );
                    }
                }
                else
                {
                    OSL_FAIL( "unexpected type" );
                    bSucc = sal_False;
                }
            }
            else
            {
                OSL_FAIL( "value missing" );
                bSucc = sal_False;
            }
        }
    }
    DBG_ASSERT( bSucc, "LoadConfig failed" );

    return bSucc;
}


sal_Bool SvtSearchOptions_Impl::Save()
{
    sal_Bool bSucc = sal_False;

    const Sequence< OUString > aNames = GetPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( nProps );
    Any *pValue = aValues.getArray();

    DBG_ASSERT( nProps == MAX_FLAGS_OFFSET + 1,
            "unexpected size of index" );
    if (nProps  &&  nProps == MAX_FLAGS_OFFSET + 1)
    {
        for (sal_uInt16 i = 0;  i < nProps;  ++i)
            pValue[i] <<= (sal_Bool) GetFlag(i);
        bSucc |= PutProperties( aNames, aValues );
    }

    if (bSucc)
        SetModified( sal_False );

    return bSucc;
}


//////////////////////////////////////////////////////////////////////

SvtSearchOptions::SvtSearchOptions()
{
    pImpl = new SvtSearchOptions_Impl;
}


SvtSearchOptions::~SvtSearchOptions()
{
    delete pImpl;
}


sal_Int32 SvtSearchOptions::GetTransliterationFlags() const
{
    sal_Int32 nRes = 0;

    if (!IsMatchCase()) // 'IsMatchCase' means act case sensitive
        nRes |= TransliterationModules_IGNORE_CASE;
    if ( IsMatchFullHalfWidthForms())
        nRes |= TransliterationModules_IGNORE_WIDTH;
    if ( IsMatchHiraganaKatakana())
        nRes |= TransliterationModules_IGNORE_KANA;
    if ( IsMatchContractions())
        nRes |= TransliterationModules_ignoreSize_ja_JP;
    if ( IsMatchMinusDashChoon())
        nRes |= TransliterationModules_ignoreMinusSign_ja_JP;
    if ( IsMatchRepeatCharMarks())
        nRes |= TransliterationModules_ignoreIterationMark_ja_JP;
    if ( IsMatchVariantFormKanji())
        nRes |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
    if ( IsMatchOldKanaForms())
        nRes |= TransliterationModules_ignoreTraditionalKana_ja_JP;
    if ( IsMatchDiziDuzu())
        nRes |= TransliterationModules_ignoreZiZu_ja_JP;
    if ( IsMatchBavaHafa())
        nRes |= TransliterationModules_ignoreBaFa_ja_JP;
    if ( IsMatchTsithichiDhizi())
        nRes |= TransliterationModules_ignoreTiJi_ja_JP;
    if ( IsMatchHyuiyuByuvyu())
        nRes |= TransliterationModules_ignoreHyuByu_ja_JP;
    if ( IsMatchSesheZeje())
        nRes |= TransliterationModules_ignoreSeZe_ja_JP;
    if ( IsMatchIaiya())
        nRes |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
    if ( IsMatchKiku())
        nRes |= TransliterationModules_ignoreKiKuFollowedBySa_ja_JP;
    if ( IsIgnorePunctuation())
        nRes |= TransliterationModules_ignoreSeparator_ja_JP;
    if ( IsIgnoreWhitespace())
        nRes |= TransliterationModules_ignoreSpace_ja_JP;
    if ( IsIgnoreProlongedSoundMark())
        nRes |= TransliterationModules_ignoreProlongedSoundMark_ja_JP;
    if ( IsIgnoreMiddleDot())
        nRes |= TransliterationModules_ignoreMiddleDot_ja_JP;
    if ( IsIgnoreDiacritics_CTL())
        nRes |= TransliterationModulesExtra::ignoreDiacritics_CTL;
    return nRes;
}


sal_Bool SvtSearchOptions::IsWholeWordsOnly() const
{
    return pImpl->GetFlag( 0 );
}


void SvtSearchOptions::SetWholeWordsOnly( sal_Bool bVal )
{
    pImpl->SetFlag( 0, bVal );
}


sal_Bool SvtSearchOptions::IsBackwards() const
{
    return pImpl->GetFlag( 1 );
}


void SvtSearchOptions::SetBackwards( sal_Bool bVal )
{
    pImpl->SetFlag( 1, bVal );
}


sal_Bool SvtSearchOptions::IsUseRegularExpression() const
{
    return pImpl->GetFlag( 2 );
}


void SvtSearchOptions::SetUseRegularExpression( sal_Bool bVal )
{
    pImpl->SetFlag( 2, bVal );
}

void SvtSearchOptions::SetSearchForStyles( sal_Bool bVal )
{
    pImpl->SetFlag( 3, bVal );
}


sal_Bool SvtSearchOptions::IsSimilaritySearch() const
{
    return pImpl->GetFlag( 4 );
}


void SvtSearchOptions::SetSimilaritySearch( sal_Bool bVal )
{
    pImpl->SetFlag( 4, bVal );
}


sal_Bool SvtSearchOptions::IsUseAsianOptions() const
{
    return pImpl->GetFlag( 5 );
}


void SvtSearchOptions::SetUseAsianOptions( sal_Bool bVal )
{
    pImpl->SetFlag( 5, bVal );
}


sal_Bool SvtSearchOptions::IsMatchCase() const
{
    return pImpl->GetFlag( 6 );
}


void SvtSearchOptions::SetMatchCase( sal_Bool bVal )
{
    pImpl->SetFlag( 6, bVal );
}


sal_Bool SvtSearchOptions::IsMatchFullHalfWidthForms() const
{
    return pImpl->GetFlag( 7 );
}


void SvtSearchOptions::SetMatchFullHalfWidthForms( sal_Bool bVal )
{
    pImpl->SetFlag( 7, bVal );
}


sal_Bool SvtSearchOptions::IsMatchHiraganaKatakana() const
{
    return pImpl->GetFlag( 8 );
}


void SvtSearchOptions::SetMatchHiraganaKatakana( sal_Bool bVal )
{
    pImpl->SetFlag( 8, bVal );
}


sal_Bool SvtSearchOptions::IsMatchContractions() const
{
    return pImpl->GetFlag( 9 );
}


void SvtSearchOptions::SetMatchContractions( sal_Bool bVal )
{
    pImpl->SetFlag( 9, bVal );
}


sal_Bool SvtSearchOptions::IsMatchMinusDashChoon() const
{
    return pImpl->GetFlag( 10 );
}


void SvtSearchOptions::SetMatchMinusDashChoon( sal_Bool bVal )
{
    pImpl->SetFlag( 10, bVal );
}


sal_Bool SvtSearchOptions::IsMatchRepeatCharMarks() const
{
    return pImpl->GetFlag( 11 );
}


void SvtSearchOptions::SetMatchRepeatCharMarks( sal_Bool bVal )
{
    pImpl->SetFlag( 11, bVal );
}


sal_Bool SvtSearchOptions::IsMatchVariantFormKanji() const
{
    return pImpl->GetFlag( 12 );
}


void SvtSearchOptions::SetMatchVariantFormKanji( sal_Bool bVal )
{
    pImpl->SetFlag( 12, bVal );
}


sal_Bool SvtSearchOptions::IsMatchOldKanaForms() const
{
    return pImpl->GetFlag( 13 );
}


void SvtSearchOptions::SetMatchOldKanaForms( sal_Bool bVal )
{
    pImpl->SetFlag( 13, bVal );
}


sal_Bool SvtSearchOptions::IsMatchDiziDuzu() const
{
    return pImpl->GetFlag( 14 );
}


void SvtSearchOptions::SetMatchDiziDuzu( sal_Bool bVal )
{
    pImpl->SetFlag( 14, bVal );
}


sal_Bool SvtSearchOptions::IsMatchBavaHafa() const
{
    return pImpl->GetFlag( 15 );
}


void SvtSearchOptions::SetMatchBavaHafa( sal_Bool bVal )
{
    pImpl->SetFlag( 15, bVal );
}


sal_Bool SvtSearchOptions::IsMatchTsithichiDhizi() const
{
    return pImpl->GetFlag( 16 );
}


void SvtSearchOptions::SetMatchTsithichiDhizi( sal_Bool bVal )
{
    pImpl->SetFlag( 16, bVal );
}


sal_Bool SvtSearchOptions::IsMatchHyuiyuByuvyu() const
{
    return pImpl->GetFlag( 17 );
}


void SvtSearchOptions::SetMatchHyuiyuByuvyu( sal_Bool bVal )
{
    pImpl->SetFlag( 17, bVal );
}


sal_Bool SvtSearchOptions::IsMatchSesheZeje() const
{
    return pImpl->GetFlag( 18 );
}


void SvtSearchOptions::SetMatchSesheZeje( sal_Bool bVal )
{
    pImpl->SetFlag( 18, bVal );
}


sal_Bool SvtSearchOptions::IsMatchIaiya() const
{
    return pImpl->GetFlag( 19 );
}


void SvtSearchOptions::SetMatchIaiya( sal_Bool bVal )
{
    pImpl->SetFlag( 19, bVal );
}


sal_Bool SvtSearchOptions::IsMatchKiku() const
{
    return pImpl->GetFlag( 20 );
}


void SvtSearchOptions::SetMatchKiku( sal_Bool bVal )
{
    pImpl->SetFlag( 20, bVal );
}


sal_Bool SvtSearchOptions::IsIgnorePunctuation() const
{
    return pImpl->GetFlag( 21 );
}


void SvtSearchOptions::SetIgnorePunctuation( sal_Bool bVal )
{
    pImpl->SetFlag( 21, bVal );
}


sal_Bool SvtSearchOptions::IsIgnoreWhitespace() const
{
    return pImpl->GetFlag( 22 );
}


void SvtSearchOptions::SetIgnoreWhitespace( sal_Bool bVal )
{
    pImpl->SetFlag( 22, bVal );
}


sal_Bool SvtSearchOptions::IsIgnoreProlongedSoundMark() const
{
    return pImpl->GetFlag( 23 );
}


void SvtSearchOptions::SetIgnoreProlongedSoundMark( sal_Bool bVal )
{
    pImpl->SetFlag( 23, bVal );
}


sal_Bool SvtSearchOptions::IsIgnoreMiddleDot() const
{
    return pImpl->GetFlag( 24 );
}


void SvtSearchOptions::SetIgnoreMiddleDot( sal_Bool bVal )
{
    pImpl->SetFlag( 24, bVal );
}

sal_Bool SvtSearchOptions::IsNotes() const
{
        return pImpl->GetFlag( 25 );
}


void SvtSearchOptions::SetNotes( sal_Bool bVal )
{
        pImpl->SetFlag( 25, bVal );
}

sal_Bool SvtSearchOptions::IsIgnoreDiacritics_CTL() const
{
    return pImpl->GetFlag( 26 );
}

void SvtSearchOptions::SetIgnoreDiacritics_CTL( sal_Bool bVal )
{
    pImpl->SetFlag( 26, bVal );
}

//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
