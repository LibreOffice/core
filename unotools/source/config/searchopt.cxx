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
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.h>
#include <osl/diagnose.h>
#include <i18nutil/transliteration.hxx>

using namespace utl;
using namespace com::sun::star::uno;

#define MAX_FLAGS_OFFSET    29

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
    void            Load();
    bool            Save();

    static Sequence< OUString >    GetPropertyNames();

public:
    SvtSearchOptions_Impl();
    virtual ~SvtSearchOptions_Impl() override;

    virtual void    Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    bool            GetFlag( sal_uInt16 nOffset ) const;
    void            SetFlag( sal_uInt16 nOffset, bool bVal );
    void            SetSearchAlgorithm( sal_uInt16 nOffset, bool bVal );
};

SvtSearchOptions_Impl::SvtSearchOptions_Impl() :
    ConfigItem( "Office.Common/SearchOptions" ),
    nFlags(0x0003FFFF) // set all options values to 'true'

{
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
    sal_Int32 nMask = (sal_Int32(1))  << nOffset;
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
    static constexpr OUString aPropNames[ MAX_FLAGS_OFFSET + 1 ] =
    {
        u"IsWholeWordsOnly"_ustr,                     //  0
        u"IsBackwards"_ustr,                          //  1
        u"IsUseRegularExpression"_ustr,               //  2
        //"IsCurrentSelectionOnly",             // interactively set or not...
        u"IsSearchForStyles"_ustr,                    //  3
        u"IsSimilaritySearch"_ustr,                   //  4
        u"IsUseAsianOptions"_ustr,                    //  5
        u"IsMatchCase"_ustr,                          //  6
        u"Japanese/IsMatchFullHalfWidthForms"_ustr,   //  7
        u"Japanese/IsMatchHiraganaKatakana"_ustr,     //  8
        u"Japanese/IsMatchContractions"_ustr,         //  9
        u"Japanese/IsMatchMinusDashCho-on"_ustr,      // 10
        u"Japanese/IsMatchRepeatCharMarks"_ustr,      // 11
        u"Japanese/IsMatchVariantFormKanji"_ustr,     // 12
        u"Japanese/IsMatchOldKanaForms"_ustr,         // 13
        u"Japanese/IsMatch_DiZi_DuZu"_ustr,           // 14
        u"Japanese/IsMatch_BaVa_HaFa"_ustr,           // 15
        u"Japanese/IsMatch_TsiThiChi_DhiZi"_ustr,     // 16
        u"Japanese/IsMatch_HyuIyu_ByuVyu"_ustr,       // 17
        u"Japanese/IsMatch_SeShe_ZeJe"_ustr,          // 18
        u"Japanese/IsMatch_IaIya"_ustr,               // 19
        u"Japanese/IsMatch_KiKu"_ustr,                // 20
        u"Japanese/IsIgnorePunctuation"_ustr,         // 21
        u"Japanese/IsIgnoreWhitespace"_ustr,          // 22
        u"Japanese/IsIgnoreProlongedSoundMark"_ustr,  // 23
        u"Japanese/IsIgnoreMiddleDot"_ustr,           // 24
        u"IsNotes"_ustr,                              // 25
        u"IsIgnoreDiacritics_CTL"_ustr,               // 26
        u"IsIgnoreKashida_CTL"_ustr,                  // 27
        u"IsSearchFormatted"_ustr,                    // 28
        u"IsUseWildcard"_ustr                         // 29
    };

    Sequence< OUString > aNames(std::size(aPropNames));
    OUString* pNames = aNames.getArray();
    for (std::size_t i = 0;  i < std::size(aPropNames); ++i)
        pNames[i] = aPropNames[i];

    return aNames;
}

void SvtSearchOptions_Impl::SetSearchAlgorithm( sal_uInt16 nOffset, bool bVal )
{
    if (bVal)
    {
        // Search algorithms are mutually exclusive.
        if (nOffset != 2 && GetFlag(2))
            SetFlag( 2, false );
        if (nOffset != 4 && GetFlag(4))
            SetFlag( 4, false );
        if (nOffset != 29 && GetFlag(29))
            SetFlag( 29, false );
    }
    SetFlag( nOffset, bVal );
}

void SvtSearchOptions_Impl::Load()
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
    if (nProps == MAX_FLAGS_OFFSET + 1)
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
    : pImpl( new SvtSearchOptions_Impl )
{
}

SvtSearchOptions::~SvtSearchOptions()
{
}

void SvtSearchOptions::Commit()
{
    pImpl->Commit();
}

TransliterationFlags SvtSearchOptions::GetTransliterationFlags() const
{
    TransliterationFlags nRes = TransliterationFlags::NONE;

    if (!IsMatchCase()) // 'IsMatchCase' means act case sensitive
        nRes |= TransliterationFlags::IGNORE_CASE;
    if ( IsMatchFullHalfWidthForms())
        nRes |= TransliterationFlags::IGNORE_WIDTH;
    if ( IsMatchHiraganaKatakana())
        nRes |= TransliterationFlags::IGNORE_KANA;
    if ( IsMatchContractions())
        nRes |= TransliterationFlags::ignoreSize_ja_JP;
    if ( IsMatchMinusDashChoon())
        nRes |= TransliterationFlags::ignoreMinusSign_ja_JP;
    if ( IsMatchRepeatCharMarks())
        nRes |= TransliterationFlags::ignoreIterationMark_ja_JP;
    if ( IsMatchVariantFormKanji())
        nRes |= TransliterationFlags::ignoreTraditionalKanji_ja_JP;
    if ( IsMatchOldKanaForms())
        nRes |= TransliterationFlags::ignoreTraditionalKana_ja_JP;
    if ( IsMatchDiziDuzu())
        nRes |= TransliterationFlags::ignoreZiZu_ja_JP;
    if ( IsMatchBavaHafa())
        nRes |= TransliterationFlags::ignoreBaFa_ja_JP;
    if ( IsMatchTsithichiDhizi())
        nRes |= TransliterationFlags::ignoreTiJi_ja_JP;
    if ( IsMatchHyuiyuByuvyu())
        nRes |= TransliterationFlags::ignoreHyuByu_ja_JP;
    if ( IsMatchSesheZeje())
        nRes |= TransliterationFlags::ignoreSeZe_ja_JP;
    if ( IsMatchIaiya())
        nRes |= TransliterationFlags::ignoreIandEfollowedByYa_ja_JP;
    if ( IsMatchKiku())
        nRes |= TransliterationFlags::ignoreKiKuFollowedBySa_ja_JP;
    if ( IsIgnorePunctuation())
        nRes |= TransliterationFlags::ignoreSeparator_ja_JP;
    if ( IsIgnoreWhitespace())
        nRes |= TransliterationFlags::ignoreSpace_ja_JP;
    if ( IsIgnoreProlongedSoundMark())
        nRes |= TransliterationFlags::ignoreProlongedSoundMark_ja_JP;
    if ( IsIgnoreMiddleDot())
        nRes |= TransliterationFlags::ignoreMiddleDot_ja_JP;
    if ( IsIgnoreDiacritics_CTL())
        nRes |= TransliterationFlags::IGNORE_DIACRITICS_CTL;
    if ( IsIgnoreKashida_CTL())
        nRes |= TransliterationFlags::IGNORE_KASHIDA_CTL;
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
    pImpl->SetSearchAlgorithm( 2, bVal );
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
    pImpl->SetSearchAlgorithm( 4, bVal );
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

bool SvtSearchOptions::IsUseWildcard() const
{
    return pImpl->GetFlag( 29 );
}

void SvtSearchOptions::SetUseWildcard( bool bVal )
{
    pImpl->SetSearchAlgorithm( 29, bVal );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
