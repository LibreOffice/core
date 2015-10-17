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

#include <sal/config.h>

#include <unotools/searchopt.hxx>
#include <tools/debug.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>
#include <sal/macros.h>
#include <osl/diagnose.h>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::i18n;

#define MAX_FLAGS_OFFSET    28

class SvtSearchOptions_Impl : public ConfigItem
{
    sal_Int32   nFlags;
    bool    bModified;

    SvtSearchOptions_Impl(const SvtSearchOptions_Impl&) = delete;
    SvtSearchOptions_Impl& operator=(const SvtSearchOptions_Impl&) = delete;

    // ConfigItem
    virtual void    ImplCommit() override;

protected:
    bool            IsModified() const { return bModified; }
    using ConfigItem::SetModified;
    void            SetModified( bool bVal );
    bool            Load();
    bool            Save();

    static Sequence< OUString >    GetPropertyNames();

public:
    SvtSearchOptions_Impl();
    virtual ~SvtSearchOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;

    bool            GetFlag( sal_uInt16 nOffset ) const;
    void            SetFlag( sal_uInt16 nOffset, bool bVal );
};

SvtSearchOptions_Impl::SvtSearchOptions_Impl() :
    ConfigItem( OUString("Office.Common/SearchOptions") )
{
    nFlags = 0x0003FFFF;    // set all options values to 'true'
    Load();
    SetModified( false );
}

SvtSearchOptions_Impl::~SvtSearchOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

void SvtSearchOptions_Impl::ImplCommit()
{
    if (IsModified())
        Save();
}

void SvtSearchOptions_Impl::Notify( const Sequence< OUString >&  )
{
}

bool SvtSearchOptions_Impl::GetFlag( sal_uInt16 nOffset ) const
{
    DBG_ASSERT( nOffset <= MAX_FLAGS_OFFSET, "offset out of range");
    return ((nFlags >> nOffset) & 0x01) != 0;
}

void SvtSearchOptions_Impl::SetFlag( sal_uInt16 nOffset, bool bVal )
{
    DBG_ASSERT( nOffset <= MAX_FLAGS_OFFSET, "offset out of range");
    sal_Int32 nOldFlags = nFlags;
    sal_Int32 nMask = ((sal_Int32) 1)  << nOffset;
    if (bVal)
        nFlags |= nMask;
    else
        nFlags &= ~nMask;
    if (nFlags != nOldFlags)
        SetModified( true );
}

void SvtSearchOptions_Impl::SetModified( bool bVal )
{
    bModified = bVal;
    if (bModified)
    {
        ConfigItem::SetModified();
    }
}

Sequence< OUString > SvtSearchOptions_Impl::GetPropertyNames()
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
        "IsIgnoreDiacritics_CTL",               // 26
        "IsIgnoreKashida_CTL",                  // 27
        "IsSearchFormatted"                     // 28
    };

    const int nCount = SAL_N_ELEMENTS( aPropNames );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for (sal_Int32 i = 0;  i < nCount;  ++i)
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

bool SvtSearchOptions_Impl::Load()
{
    bool bSucc = false;

    Sequence< OUString > aNames = GetPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    const Sequence< Any > aValues = GetProperties( aNames );
    DBG_ASSERT( aValues.getLength() == aNames.getLength(),
            "GetProperties failed" );
    //EnableNotification( aNames );

    if (nProps  &&  aValues.getLength() == nProps)
    {
        bSucc = true;

        const Any* pValues = aValues.getConstArray();
        for (sal_Int32 i = 0;  i < nProps;  ++i)
        {
            const Any &rVal = pValues[i];
            DBG_ASSERT( rVal.hasValue(), "property value missing" );
            if (rVal.hasValue())
            {
                bool bVal = bool();
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
                    bSucc = false;
                }
            }
            else
            {
                OSL_FAIL( "value missing" );
                bSucc = false;
            }
        }
    }
    DBG_ASSERT( bSucc, "LoadConfig failed" );

    return bSucc;
}

bool SvtSearchOptions_Impl::Save()
{
    bool bSucc = false;

    const Sequence< OUString > aNames = GetPropertyNames();
    sal_Int32 nProps = aNames.getLength();

    Sequence< Any > aValues( nProps );
    Any *pValue = aValues.getArray();

    DBG_ASSERT( nProps == MAX_FLAGS_OFFSET + 1,
            "unexpected size of index" );
    if (nProps  &&  nProps == MAX_FLAGS_OFFSET + 1)
    {
        for (sal_Int32 i = 0;  i < nProps;  ++i)
            pValue[i] <<= GetFlag(i);
        bSucc |= PutProperties( aNames, aValues );
    }

    if (bSucc)
        SetModified( false );

    return bSucc;
}

SvtSearchOptions::SvtSearchOptions()
{
    pImpl = new SvtSearchOptions_Impl;
}

SvtSearchOptions::~SvtSearchOptions()
{
    delete pImpl;
}

void SvtSearchOptions::Commit()
{
    pImpl->Commit();
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
        nRes |= TransliterationModulesExtra::IGNORE_DIACRITICS_CTL;
    if ( IsIgnoreKashida_CTL())
        nRes |= TransliterationModulesExtra::IGNORE_KASHIDA_CTL;
    return nRes;
}

bool SvtSearchOptions::IsWholeWordsOnly() const
{
    return pImpl->GetFlag( 0 );
}

void SvtSearchOptions::SetWholeWordsOnly( bool bVal )
{
    pImpl->SetFlag( 0, bVal );
}

bool SvtSearchOptions::IsBackwards() const
{
    return pImpl->GetFlag( 1 );
}

void SvtSearchOptions::SetBackwards( bool bVal )
{
    pImpl->SetFlag( 1, bVal );
}

bool SvtSearchOptions::IsUseRegularExpression() const
{
    return pImpl->GetFlag( 2 );
}

void SvtSearchOptions::SetUseRegularExpression( bool bVal )
{
    pImpl->SetFlag( 2, bVal );
}

void SvtSearchOptions::SetSearchForStyles( bool bVal )
{
    pImpl->SetFlag( 3, bVal );
}

bool SvtSearchOptions::IsSimilaritySearch() const
{
    return pImpl->GetFlag( 4 );
}

void SvtSearchOptions::SetSimilaritySearch( bool bVal )
{
    pImpl->SetFlag( 4, bVal );
}

bool SvtSearchOptions::IsUseAsianOptions() const
{
    return pImpl->GetFlag( 5 );
}

void SvtSearchOptions::SetUseAsianOptions( bool bVal )
{
    pImpl->SetFlag( 5, bVal );
}

bool SvtSearchOptions::IsMatchCase() const
{
    return pImpl->GetFlag( 6 );
}

void SvtSearchOptions::SetMatchCase( bool bVal )
{
    pImpl->SetFlag( 6, bVal );
}

bool SvtSearchOptions::IsMatchFullHalfWidthForms() const
{
    return pImpl->GetFlag( 7 );
}

void SvtSearchOptions::SetMatchFullHalfWidthForms( bool bVal )
{
    pImpl->SetFlag( 7, bVal );
}

bool SvtSearchOptions::IsMatchHiraganaKatakana() const
{
    return pImpl->GetFlag( 8 );
}

void SvtSearchOptions::SetMatchHiraganaKatakana( bool bVal )
{
    pImpl->SetFlag( 8, bVal );
}

bool SvtSearchOptions::IsMatchContractions() const
{
    return pImpl->GetFlag( 9 );
}

void SvtSearchOptions::SetMatchContractions( bool bVal )
{
    pImpl->SetFlag( 9, bVal );
}

bool SvtSearchOptions::IsMatchMinusDashChoon() const
{
    return pImpl->GetFlag( 10 );
}

void SvtSearchOptions::SetMatchMinusDashChoon( bool bVal )
{
    pImpl->SetFlag( 10, bVal );
}

bool SvtSearchOptions::IsMatchRepeatCharMarks() const
{
    return pImpl->GetFlag( 11 );
}

void SvtSearchOptions::SetMatchRepeatCharMarks( bool bVal )
{
    pImpl->SetFlag( 11, bVal );
}

bool SvtSearchOptions::IsMatchVariantFormKanji() const
{
    return pImpl->GetFlag( 12 );
}

void SvtSearchOptions::SetMatchVariantFormKanji( bool bVal )
{
    pImpl->SetFlag( 12, bVal );
}

bool SvtSearchOptions::IsMatchOldKanaForms() const
{
    return pImpl->GetFlag( 13 );
}

void SvtSearchOptions::SetMatchOldKanaForms( bool bVal )
{
    pImpl->SetFlag( 13, bVal );
}

bool SvtSearchOptions::IsMatchDiziDuzu() const
{
    return pImpl->GetFlag( 14 );
}

void SvtSearchOptions::SetMatchDiziDuzu( bool bVal )
{
    pImpl->SetFlag( 14, bVal );
}

bool SvtSearchOptions::IsMatchBavaHafa() const
{
    return pImpl->GetFlag( 15 );
}

void SvtSearchOptions::SetMatchBavaHafa( bool bVal )
{
    pImpl->SetFlag( 15, bVal );
}

bool SvtSearchOptions::IsMatchTsithichiDhizi() const
{
    return pImpl->GetFlag( 16 );
}

void SvtSearchOptions::SetMatchTsithichiDhizi( bool bVal )
{
    pImpl->SetFlag( 16, bVal );
}

bool SvtSearchOptions::IsMatchHyuiyuByuvyu() const
{
    return pImpl->GetFlag( 17 );
}

void SvtSearchOptions::SetMatchHyuiyuByuvyu( bool bVal )
{
    pImpl->SetFlag( 17, bVal );
}

bool SvtSearchOptions::IsMatchSesheZeje() const
{
    return pImpl->GetFlag( 18 );
}

void SvtSearchOptions::SetMatchSesheZeje( bool bVal )
{
    pImpl->SetFlag( 18, bVal );
}

bool SvtSearchOptions::IsMatchIaiya() const
{
    return pImpl->GetFlag( 19 );
}

void SvtSearchOptions::SetMatchIaiya( bool bVal )
{
    pImpl->SetFlag( 19, bVal );
}

bool SvtSearchOptions::IsMatchKiku() const
{
    return pImpl->GetFlag( 20 );
}

void SvtSearchOptions::SetMatchKiku( bool bVal )
{
    pImpl->SetFlag( 20, bVal );
}

bool SvtSearchOptions::IsIgnorePunctuation() const
{
    return pImpl->GetFlag( 21 );
}

void SvtSearchOptions::SetIgnorePunctuation( bool bVal )
{
    pImpl->SetFlag( 21, bVal );
}

bool SvtSearchOptions::IsIgnoreWhitespace() const
{
    return pImpl->GetFlag( 22 );
}

void SvtSearchOptions::SetIgnoreWhitespace( bool bVal )
{
    pImpl->SetFlag( 22, bVal );
}

bool SvtSearchOptions::IsIgnoreProlongedSoundMark() const
{
    return pImpl->GetFlag( 23 );
}

void SvtSearchOptions::SetIgnoreProlongedSoundMark( bool bVal )
{
    pImpl->SetFlag( 23, bVal );
}

bool SvtSearchOptions::IsIgnoreMiddleDot() const
{
    return pImpl->GetFlag( 24 );
}

void SvtSearchOptions::SetIgnoreMiddleDot( bool bVal )
{
    pImpl->SetFlag( 24, bVal );
}

bool SvtSearchOptions::IsNotes() const
{
    return pImpl->GetFlag( 25 );
}

void SvtSearchOptions::SetNotes( bool bVal )
{
    pImpl->SetFlag( 25, bVal );
}

bool SvtSearchOptions::IsIgnoreDiacritics_CTL() const
{
    return pImpl->GetFlag( 26 );
}

void SvtSearchOptions::SetIgnoreDiacritics_CTL( bool bVal )
{
    pImpl->SetFlag( 26, bVal );
}

bool SvtSearchOptions::IsIgnoreKashida_CTL() const
{
    return pImpl->GetFlag( 27 );
}

void SvtSearchOptions::SetIgnoreKashida_CTL( bool bVal )
{
    pImpl->SetFlag( 27, bVal );
}

bool SvtSearchOptions::IsSearchFormatted() const
{
    return pImpl->GetFlag( 28 );
}

void SvtSearchOptions::SetSearchFormatted( bool bVal )
{
    pImpl->SetFlag( 28, bVal );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
