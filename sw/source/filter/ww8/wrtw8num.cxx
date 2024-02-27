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

#include <hintids.hxx>
#include <vcl/font.hxx>
#include <editeng/langitem.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <numrule.hxx>
#include <charfmt.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

#include "sprmids.hxx"

#include "ww8attributeoutput.hxx"
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "wrtww8.hxx"
#include "ww8par.hxx"

using namespace ::com::sun::star;
using namespace sw::types;
using namespace sw::util;

SwNumRule* MSWordExportBase::DuplicateNumRuleImpl(const SwNumRule *pRule)
{
    const OUString sPrefix("WW8TempExport" + OUString::number( m_nUniqueList++ ));
    SwNumRule* pMyNumRule =
            new SwNumRule( m_rDoc.GetUniqueNumRuleName( &sPrefix ),
                           SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    m_pUsedNumTable->push_back( pMyNumRule );

    for ( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
    {
        const SwNumFormat& rSubRule = pRule->Get(i);
        pMyNumRule->Set( i, rSubRule );
    }
    return pMyNumRule;
}

sal_uInt16 MSWordExportBase::DuplicateNumRule(const SwNumRule* pRule, sal_uInt8 nLevel, sal_uInt16 nVal)
{
    SwNumRule* const pMyNumRule = DuplicateNumRuleImpl(pRule);

    SwNumFormat aNumFormat(pMyNumRule->Get(nLevel));
    aNumFormat.SetStart(nVal);
    pMyNumRule->Set(nLevel, aNumFormat);

    return GetNumberingId(*pMyNumRule);
}

// multiple SwList can be based on the same SwNumRule; ensure one w:abstractNum
// per SwList
sal_uInt16 MSWordExportBase::DuplicateAbsNum(OUString const& rListId,
        SwNumRule const& rAbstractRule)
{
    auto const it(m_Lists.find(rListId));
    if (it != m_Lists.end())
    {
        return it->second;
    }
    else
    {
        auto const pNewAbstractRule = DuplicateNumRuleImpl(&rAbstractRule);
        assert(GetNumberingId(*pNewAbstractRule) == m_pUsedNumTable->size() - 1);
        (void) pNewAbstractRule;
        m_Lists.insert(std::make_pair(rListId, m_pUsedNumTable->size() - 1));
        return m_pUsedNumTable->size() - 1;
    }
}

// Ideally we want to map SwList to w:abstractNum and SwNumRule to w:num
// The current approach is to keep exporting every SwNumRule to
// 1 w:abstractNum and 1 w:num, and then add extra w:num via this function
// that reference an existing w:abstractNum and may override its formatting;
// of course this will end up exporting some w:num that aren't actually used.
sal_uInt16 MSWordExportBase::OverrideNumRule(
        SwNumRule const& rExistingRule,
        OUString const& rListId,
        SwNumRule const& rAbstractRule)
{
    const sal_uInt16 numdef = GetNumberingId(rExistingRule);

    const sal_uInt16 absnumdef = rListId == rAbstractRule.GetDefaultListId()
        ? GetNumberingId(rAbstractRule)
        : DuplicateAbsNum(rListId, rAbstractRule);
    assert(numdef != USHRT_MAX);
    assert(absnumdef != USHRT_MAX);
    auto const mapping = std::make_pair(numdef, absnumdef);

    auto it = m_OverridingNums.insert(std::make_pair(m_pUsedNumTable->size(), mapping));

    m_pUsedNumTable->push_back(nullptr); // dummy, it's unique_ptr...
    ++m_nUniqueList; // counter for DuplicateNumRule...

    return it.first->first;
}

void MSWordExportBase::AddListLevelOverride(sal_uInt16 nListId,
    sal_uInt16 nLevelNum,
    sal_uInt16 nStartAt)
{
    m_ListLevelOverrides[nListId][nLevelNum] = nStartAt;
}

sal_uInt16 MSWordExportBase::GetNumberingId( const SwNumRule& rNumRule )
{
    if ( !m_pUsedNumTable )
    {
        m_pUsedNumTable.reset(new SwNumRuleTable);
        m_pUsedNumTable->insert( m_pUsedNumTable->begin(), m_rDoc.GetNumRuleTable().begin(), m_rDoc.GetNumRuleTable().end() );
        // Check, if the outline rule is already inserted into <pUsedNumTable>.
        // If yes, do not insert it again.
        bool bOutlineRuleAdded( false );
        for ( sal_uInt16 n = m_pUsedNumTable->size(); n; )
        {
            const SwNumRule& rRule = *(*m_pUsedNumTable)[ --n ];
            if (!m_rDoc.IsUsed(rRule))
            {
                m_pUsedNumTable->erase( m_pUsedNumTable->begin() + n );
            }
            else if ( &rRule == m_rDoc.GetOutlineNumRule() )
            {
                bOutlineRuleAdded = true;
            }
        }

        if ( !bOutlineRuleAdded )
        {
            // still need to paste the OutlineRule
            SwNumRule* pR = m_rDoc.GetOutlineNumRule();
            m_pUsedNumTable->push_back( pR );
        }
    }
    SwNumRule* p = const_cast<SwNumRule*>(&rNumRule);
    sal_uInt16 nRet = o3tl::narrowing<sal_uInt16>(m_pUsedNumTable->GetPos(p));

    return nRet;
}

// GetFirstLineOffset should problem never appear unadorned apart from
// here in the ww export filter
sal_Int16 GetWordFirstLineOffset(const SwNumFormat &rFormat)
{
    OSL_ENSURE( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<GetWordFirstLineOffset> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    short nFirstLineOffset;
    if (rFormat.GetNumAdjust() == SvxAdjust::Right)
        nFirstLineOffset = -rFormat.GetCharTextDistance();
    else
        nFirstLineOffset = rFormat.GetFirstLineOffset(); //TODO: overflow
    return nFirstLineOffset;
}

void WW8Export::WriteNumbering()
{
    if ( !m_pUsedNumTable )
        return; // no numbering is used

    // list formats - LSTF
    m_pFib->m_fcPlcfLst = m_pTableStrm->Tell();
    m_pTableStrm->WriteUInt16( m_pUsedNumTable->size() );
    NumberingDefinitions();
    // set len to FIB
    m_pFib->m_lcbPlcfLst = m_pTableStrm->Tell() - m_pFib->m_fcPlcfLst;

    // list formats - LVLF
    AbstractNumberingDefinitions();

    // list formats - LFO
    OutOverrideListTab();

    // list formats - ListNames
    OutListNamesTab();
}

void WW8AttributeOutput::NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule )
{
    m_rWW8Export.m_pTableStrm->WriteUInt32( nId );
    m_rWW8Export.m_pTableStrm->WriteUInt32( nId );

    // not associated with a Style
    for ( int i = 0; i < WW8ListManager::nMaxLevel; ++i )
        m_rWW8Export.m_pTableStrm->WriteUInt16( 0xFFF );

    sal_uInt8 nFlags = 0;
    if ( rRule.IsContinusNum() )
        nFlags |= 0x1;

    m_rWW8Export.m_pTableStrm->WriteUChar( nFlags ).WriteUChar( 0/*nDummy*/ );
}

void MSWordExportBase::NumberingDefinitions()
{
    if ( !m_pUsedNumTable )
        return; // no numbering is used

    sal_uInt16 nCount = m_pUsedNumTable->size();

    // Write static data of SwNumRule - LSTF
    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        const SwNumRule * pRule = (*m_pUsedNumTable)[ n ];
        if (pRule)
        {
            AttrOutput().NumberingDefinition(n + 1, *pRule);
        }
        else
        {
            auto it = m_OverridingNums.find(n);
            assert(it != m_OverridingNums.end());
            pRule = (*m_pUsedNumTable)[it->second.first];
            assert(pRule);
            AttrOutput().OverrideNumberingDefinition(*pRule, n + 1, it->second.second + 1, m_ListLevelOverrides[n]);
        }
    }
}

/**
 * Converts the SVX numbering type to MSONFC.
 *
 * This is used for special paragraph numbering considerations.
 */
static sal_uInt8 GetLevelNFC(sal_uInt16 eNumType, const SfxItemSet* pOutSet, sal_uInt8 nDefault)
{
    sal_uInt8 nRet = nDefault;
    switch( eNumType )
    {
    case SVX_NUM_NUMBER_LOWER_ZH:
        nRet = 35;
        if ( pOutSet ) {
            const SvxLanguageItem& rLang = pOutSet->Get( RES_CHRATR_CJK_LANGUAGE);
            const LanguageType eLang = rLang.GetLanguage();
            if (LANGUAGE_CHINESE_SIMPLIFIED ==eLang) {
                nRet = 39;
            }
        }
        break;

    // LVLF can't contain 0x08, msonfcHex.
    case style::NumberingType::SYMBOL_CHICAGO:
        // No SVX_NUM_SYMBOL_CHICAGO here: LVLF can't contain 0x09, msonfcChiManSty.
        nRet = 0;
        break;
    // LVLF can't contain 0x0F / 15, msonfcSbChar / decimalHalfWidth.
    // LVLF can't contain 0x13 / 19, msonfcDArabic / decimalFullWidth2
    }
    return nRet;
}


void WW8AttributeOutput::NumberingLevel( sal_uInt8 /*nLevel*/,
        sal_uInt16 nStart,
        sal_uInt16 nNumberingType,
        SvxAdjust eAdjust,
        const sal_uInt8 *pNumLvlPos,
        sal_uInt8 nFollow,
        const wwFont *pFont,
        const SfxItemSet *pOutSet,
        sal_Int16 nIndentAt,
        sal_Int16 nFirstLineIndex,
        sal_Int16 nListTabPos,
        const OUString &rNumberingString,
        const SvxBrushItem* pBrush, //For i120928,to transfer graphic of bullet
        bool isLegal
    )
{
    // Start value
    m_rWW8Export.m_pTableStrm->WriteUInt32( nStart );

    // Type
    sal_uInt8 nNumId = GetLevelNFC(nNumberingType, pOutSet, WW8Export::GetNumId(nNumberingType));
    m_rWW8Export.m_pTableStrm->WriteUChar(nNumId);

    // Justification
    sal_uInt8 nAlign;
    switch ( eAdjust )
    {
    case SvxAdjust::Center:
        nAlign = 1;
        break;
    case SvxAdjust::Right:
        nAlign = 2;
        break;
    default:
        nAlign = 0;
        break;
    }

    if (isLegal)
    {
        // 3rd bit.
        nAlign |= 0x04;
    }

    m_rWW8Export.m_pTableStrm->WriteUChar( nAlign );

    // Write the rgbxchNums[9], positions of placeholders for paragraph
    // numbers in the text
    m_rWW8Export.m_pTableStrm->WriteBytes(pNumLvlPos, WW8ListManager::nMaxLevel);

    // Type of the character between the bullet and the text
    m_rWW8Export.m_pTableStrm->WriteUChar( nFollow );

    // dxaSoace/dxaIndent (Word 6 compatibility)
    m_rWW8Export.m_pTableStrm->WriteUInt32( 0 );
    m_rWW8Export.m_pTableStrm->WriteUInt32( 0 );

    // cbGrpprlChpx
    std::unique_ptr<ww::bytes> pCharAtrs;
    if ( pOutSet )
    {
        std::unique_ptr<ww::bytes> pOldpO = std::move(m_rWW8Export.m_pO);
        m_rWW8Export.m_pO.reset(new ww::bytes);
        if ( pFont )
        {
            sal_uInt16 nFontID = m_rWW8Export.m_aFontHelper.GetId( *pFont );

            m_rWW8Export.InsUInt16( NS_sprm::CRgFtc0::val );
            m_rWW8Export.InsUInt16( nFontID );
            m_rWW8Export.InsUInt16( NS_sprm::CRgFtc2::val );
            m_rWW8Export.InsUInt16( nFontID );
        }

        m_rWW8Export.OutputItemSet( *pOutSet, false, true, i18n::ScriptType::LATIN, m_rWW8Export.m_bExportModeRTF );
        //For i120928,achieve graphic's index of bullet from the bullet bookmark
        if (SVX_NUM_BITMAP == nNumberingType && pBrush)
        {
            int nIndex = m_rWW8Export.GetGrfIndex(*pBrush);
            if ( nIndex != -1 )
            {
                m_rWW8Export.InsUInt16(NS_sprm::CPbiIBullet::val);
                m_rWW8Export.InsUInt32(nIndex);
                m_rWW8Export.InsUInt16(NS_sprm::CPbiGrf::val);
                m_rWW8Export.InsUInt16(1);
            }
        }

        pCharAtrs = std::move(m_rWW8Export.m_pO);
        m_rWW8Export.m_pO = std::move(pOldpO);
    }
    m_rWW8Export.m_pTableStrm->WriteUChar(sal_uInt8(pCharAtrs ? pCharAtrs->size() : 0));

    // cbGrpprlPapx
    sal_uInt8 aPapSprms [] = {
        0x5e, 0x84, 0, 0,               // sprmPDxaLeft
        0x60, 0x84, 0, 0,               // sprmPDxaLeft1
        0x15, 0xc6, 0x05, 0x00, 0x01, 0, 0, 0x06
    };
    m_rWW8Export.m_pTableStrm->WriteUChar( sal_uInt8( sizeof( aPapSprms ) ) );

    // reserved
    m_rWW8Export.m_pTableStrm->WriteUInt16( 0 );

    // pap sprms
    sal_uInt8* pData = aPapSprms + 2;
    Set_UInt16( pData, nIndentAt );
    pData += 2;
    Set_UInt16( pData, nFirstLineIndex );
    pData += 5;
    Set_UInt16( pData, nListTabPos );

    m_rWW8Export.m_pTableStrm->WriteBytes(aPapSprms, sizeof(aPapSprms));

    // write Chpx
    if (pCharAtrs && !pCharAtrs->empty())
        m_rWW8Export.m_pTableStrm->WriteBytes(pCharAtrs->data(), pCharAtrs->size());

    // write the num string
    m_rWW8Export.m_pTableStrm->WriteUInt16( rNumberingString.getLength() );
    SwWW8Writer::WriteString16( *m_rWW8Export.m_pTableStrm, rNumberingString, false );
}

void MSWordExportBase::AbstractNumberingDefinitions()
{
    sal_uInt16 nCount = m_pUsedNumTable->size();
    sal_uInt16 n;

    for( n = 0; n < nCount; ++n )
    {
        if (nullptr == (*m_pUsedNumTable)[ n ])
        {
            continue;
        }

        AttrOutput().StartAbstractNumbering( n + 1 );

        const SwNumRule& rRule = *(*m_pUsedNumTable)[ n ];
        sal_uInt8 nLvl;
        sal_uInt8 nLevels = static_cast< sal_uInt8 >(rRule.IsContinusNum() ?
            WW8ListManager::nMinLevel : WW8ListManager::nMaxLevel);
        for( nLvl = 0; nLvl < nLevels; ++nLvl )
        {
            NumberingLevel(rRule, nLvl);
        }

        AttrOutput().EndAbstractNumbering();
    }
}

void MSWordExportBase::NumberingLevel(
        SwNumRule const& rRule, sal_uInt8 const nLvl)
{
    // write the static data of the SwNumFormat of this level
    sal_uInt8 aNumLvlPos[WW8ListManager::nMaxLevel] = { 0,0,0,0,0,0,0,0,0 };

    const SwNumFormat& rFormat = rRule.Get( nLvl );

    sal_uInt8 nFollow = 0;
    // #i86652#
    if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
    {
        // <nFollow = 2>, if minimum label width equals 0 and
        // minimum distance between label and text equals 0
        nFollow = (rFormat.GetFirstLineOffset() == 0 &&
            rFormat.GetCharTextDistance() == 0)
            ? 2 : 0;     // ixchFollow: 0 - tab, 1 - blank, 2 - nothing
    }
    else if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
    {
        switch (rFormat.GetLabelFollowedBy())
        {
            case SvxNumberFormat::LISTTAB:
            {
                // 0 (tab) unless there would be no content before the tab, in which case 2 (nothing)
                nFollow = (SVX_NUM_NUMBER_NONE != rFormat.GetNumberingType()) ? 0 : 2;
            }
            break;
            case SvxNumberFormat::SPACE:
            {
                // 1 (space) unless there would be no content before the space in which case 2 (nothing)
                nFollow = (SVX_NUM_NUMBER_NONE != rFormat.GetNumberingType()) ? 1 : 2;
            }
            break;
            case SvxNumberFormat::NOTHING:
            {
                nFollow = 2;
            }
            break;
            default:
            {
                nFollow = 0;
                OSL_FAIL( "unknown GetLabelFollowedBy() return value" );
            }
        }
    }

    // Build the NumString for this Level
    OUString sNumStr;
    OUString sFontName;
    bool bWriteBullet = false;
    std::optional<vcl::Font> pBulletFont;
    rtl_TextEncoding eChrSet=0;
    FontFamily eFamily=FAMILY_DECORATIVE;
    if (SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType() ||
        SVX_NUM_BITMAP == rFormat.GetNumberingType())
    {
        // Use bullet
        sal_UCS4 cBullet = rFormat.GetBulletChar();
        sNumStr = OUString(&cBullet, 1);
    }
    else
    {
        // Create level string
        if (rFormat.HasListFormat())
        {
            sal_uInt8* pLvlPos = aNumLvlPos;
            sNumStr = rFormat.GetListFormat();

            // now search the nums in the string
            for (sal_uInt8 i = 0; i <= nLvl; ++i)
            {
                OUString sSrch("%" + OUString::number(i+1) + "%");
                sal_Int32 nFnd = sNumStr.indexOf(sSrch);
                if (-1 != nFnd)
                {
                    *pLvlPos = static_cast<sal_uInt8>(nFnd + 1);
                    ++pLvlPos;
                    sNumStr = sNumStr.replaceAt(nFnd, sSrch.getLength(), rtl::OUStringChar(static_cast<char>(i)));
                }
            }
        }
        else if (rFormat.GetNumberingType() != SVX_NUM_NUMBER_NONE)
            assert(false && "deprecated format still exists and is unhandled. Inform Vasily or Justin");
    }

    if (SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType() ||
        SVX_NUM_BITMAP == rFormat.GetNumberingType())
    {
        bWriteBullet = true;

        pBulletFont = rFormat.GetBulletFont();
        if (!pBulletFont)
        {
            pBulletFont = numfunc::GetDefBulletFont();
        }

        eChrSet = pBulletFont->GetCharSet();
        sFontName = pBulletFont->GetFamilyName();
        eFamily = pBulletFont->GetFamilyType();

        if (IsOpenSymbol(sFontName))
            SubstituteBullet(sNumStr, eChrSet, sFontName);
    }

    // Attributes of the numbering
    std::unique_ptr<wwFont> pPseudoFont;
    const SfxItemSet* pOutSet = nullptr;

    // cbGrpprlChpx
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aSet( m_rDoc.GetAttrPool() );
    if (rFormat.GetCharFormat() || bWriteBullet)
    {
        if (bWriteBullet)
        {
            pOutSet = &aSet;

            if (rFormat.GetCharFormat())
                aSet.Put( rFormat.GetCharFormat()->GetAttrSet() );
            aSet.ClearItem( RES_CHRATR_CJK_FONT );
            aSet.ClearItem( RES_CHRATR_FONT );

            if (sFontName.isEmpty())
                sFontName = pBulletFont->GetFamilyName();

            pPseudoFont.reset(new wwFont( sFontName, pBulletFont->GetPitch(),
                eFamily, eChrSet));
        }
        else
            pOutSet = &rFormat.GetCharFormat()->GetAttrSet();
    }

    sal_Int16 nIndentAt = 0;
    sal_Int16 nFirstLineIndex = 0;
    sal_Int16 nListTabPos = -1;

    // #i86652#
    if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
    {
        nIndentAt = nListTabPos = rFormat.GetAbsLSpace(); //TODO: overflow
        nFirstLineIndex = GetWordFirstLineOffset(rFormat);
    }
    else if (rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
    {
        nIndentAt = static_cast<sal_Int16>(rFormat.GetIndentAt());
        nFirstLineIndex = static_cast<sal_Int16>(rFormat.GetFirstLineIndent());
        nListTabPos = rFormat.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB?
                      static_cast<sal_Int16>( rFormat.GetListtabPos() ) : 0;
    }

    AttrOutput().NumberingLevel( nLvl,
            rFormat.GetStart(),
            rFormat.GetNumberingType(),
            rFormat.GetNumAdjust(),
            aNumLvlPos,
            nFollow,
            pPseudoFont.get(), pOutSet,
            nIndentAt, nFirstLineIndex, nListTabPos,
            sNumStr,
            rFormat.GetNumberingType()==SVX_NUM_BITMAP ? rFormat.GetBrush() : nullptr, rFormat.GetIsLegal());
}

void WW8Export::OutOverrideListTab()
{
    if( !m_pUsedNumTable )
        return ;            // no numbering is used

    // write the "list format override" - LFO
    sal_uInt16 nCount = m_pUsedNumTable->size();
    sal_uInt16 n;

    m_pFib->m_fcPlfLfo = m_pTableStrm->Tell();
    m_pTableStrm->WriteUInt32( nCount );

    // LFO ([MS-DOC] 2.9.131)
    for( n = 0; n < nCount; ++n )
    {
        m_pTableStrm->WriteUInt32( n + 1 );
        SwWW8Writer::FillCount( *m_pTableStrm, 12 );
    }
    // LFOData ([MS-DOC] 2.9.132)
    for( n = 0; n < nCount; ++n )
        m_pTableStrm->WriteInt32( -1 );  // no overwrite

    // set len to FIB
    m_pFib->m_lcbPlfLfo = m_pTableStrm->Tell() - m_pFib->m_fcPlfLfo;
}

void WW8Export::OutListNamesTab()
{
    if( !m_pUsedNumTable )
        return ;            // no numbering is used

    // write the "list format override" - LFO
    sal_uInt16 nNms = 0, nCount = m_pUsedNumTable->size();

    m_pFib->m_fcSttbListNames = m_pTableStrm->Tell();
    m_pTableStrm->WriteInt16( -1 );
    m_pTableStrm->WriteUInt32( nCount );

    for( ; nNms < nCount; ++nNms )
    {
        const SwNumRule& rRule = *(*m_pUsedNumTable)[ nNms ];
        OUString sNm;
        if( !rRule.IsAutoRule() )
            sNm = rRule.GetName();

        m_pTableStrm->WriteUInt16( sNm.getLength() );
        if (!sNm.isEmpty())
            SwWW8Writer::WriteString16(*m_pTableStrm, sNm, false);
    }

    SwWW8Writer::WriteLong( *m_pTableStrm, m_pFib->m_fcSttbListNames + 2, nNms );
    // set len to FIB
    m_pFib->m_lcbSttbListNames = m_pTableStrm->Tell() - m_pFib->m_fcSttbListNames;
}

void MSWordExportBase::SubstituteBullet( OUString& rNumStr,
    rtl_TextEncoding& rChrSet, OUString& rFontName ) const
{
    if (!m_bSubstituteBullets)
        return;
    OUString sFontName = rFontName;

    // If Bullet char is "", don't change
    if (rNumStr[0] != u'\0')
    {
        rNumStr = rNumStr.replaceAt(0, 1, rtl::OUStringChar(
            msfilter::util::bestFitOpenSymbolToMSFont(rNumStr[0], rChrSet, sFontName)));
    }

    rFontName = sFontName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
