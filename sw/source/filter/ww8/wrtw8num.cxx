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
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/langitem.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <unotools/fontcfg.hxx>
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

sal_uInt16 MSWordExportBase::DuplicateNumRule( const SwNumRule *pRule, sal_uInt8 nLevel, sal_uInt16 nVal )
{
    sal_uInt16 nNumId = USHRT_MAX;
    const OUString sPrefix("WW8TempExport" + OUString::number( m_nUniqueList++ ));
    SwNumRule* pMyNumRule =
            new SwNumRule( m_pDoc->GetUniqueNumRuleName( &sPrefix ),
                           SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    m_pUsedNumTable->push_back( pMyNumRule );

    for ( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
    {
        const SwNumFormat& rSubRule = pRule->Get(i);
        pMyNumRule->Set( i, rSubRule );
    }

    SwNumFormat aNumFormat( pMyNumRule->Get( nLevel ) );
    aNumFormat.SetStart( nVal );
    pMyNumRule->Set( nLevel, aNumFormat );

    nNumId = GetId( *pMyNumRule );

    // Map the old list to our new list
    m_aRuleDuplicates[GetId( *pRule )] = nNumId;

    return nNumId;
}

sal_uInt16 MSWordExportBase::GetId( const SwNumRule& rNumRule )
{
    if ( !m_pUsedNumTable )
    {
        m_pUsedNumTable.reset(new SwNumRuleTable);
        m_pUsedNumTable->insert( m_pUsedNumTable->begin(), m_pDoc->GetNumRuleTable().begin(), m_pDoc->GetNumRuleTable().end() );
        // Check, if the outline rule is already inserted into <pUsedNumTable>.
        // If yes, do not insert it again.
        bool bOutlineRuleAdded( false );
        for ( sal_uInt16 n = m_pUsedNumTable->size(); n; )
        {
            const SwNumRule& rRule = *(*m_pUsedNumTable)[ --n ];
            if ( !SwDoc::IsUsed( rRule ) )
            {
                m_pUsedNumTable->erase( m_pUsedNumTable->begin() + n );
            }
            else if ( &rRule == m_pDoc->GetOutlineNumRule() )
            {
                bOutlineRuleAdded = true;
            }
        }

        if ( !bOutlineRuleAdded )
        {
            // still need to paste the OutlineRule
            SwNumRule* pR = m_pDoc->GetOutlineNumRule();
            m_pUsedNumTable->push_back( pR );
        }
    }
    SwNumRule* p = const_cast<SwNumRule*>(&rNumRule);
    sal_uInt16 nRet = static_cast<sal_uInt16>(m_pUsedNumTable->GetPos(p));

    // Is this list now duplicated into a new list which we should use
    // #i77812# - perform 'deep' search in duplication map
    std::map<sal_uInt16,sal_uInt16>::const_iterator aResult = m_aRuleDuplicates.end();
    do {
        aResult = m_aRuleDuplicates.find(nRet);
        if ( aResult != m_aRuleDuplicates.end() )
        {
            nRet = (*aResult).second;
        }
    } while ( aResult != m_aRuleDuplicates.end() );

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
    pFib->m_fcPlcfLst = pTableStrm->Tell();
    SwWW8Writer::WriteShort( *pTableStrm, m_pUsedNumTable->size() );
    NumberingDefinitions();
    // set len to FIB
    pFib->m_lcbPlcfLst = pTableStrm->Tell() - pFib->m_fcPlcfLst;

    // list formats - LVLF
    AbstractNumberingDefinitions();

    // list formats - LFO
    OutOverrideListTab();

    // list formats - ListNames
    OutListNamesTab();
}

void WW8AttributeOutput::NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule )
{
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, nId );
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, nId );

    // not associated with a Style
    for ( int i = 0; i < WW8ListManager::nMaxLevel; ++i )
        SwWW8Writer::WriteShort( *m_rWW8Export.pTableStrm, 0xFFF );

    sal_uInt8 nFlags = 0;
    if ( rRule.IsContinusNum() )
        nFlags |= 0x1;

    m_rWW8Export.pTableStrm->WriteUChar( nFlags ).WriteUChar( 0/*nDummy*/ );
}

void MSWordExportBase::NumberingDefinitions()
{
    if ( !m_pUsedNumTable )
        return; // no numbering is used

    sal_uInt16 nCount = m_pUsedNumTable->size();

    // Write static data of SwNumRule - LSTF
    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        const SwNumRule& rRule = *(*m_pUsedNumTable)[ n ];

        AttrOutput().NumberingDefinition( n + 1, rRule );
    }
}

static sal_uInt8 GetLevelNFC(  sal_uInt16 eNumType, const SfxItemSet *pOutSet)
{
    sal_uInt8 nRet = 0;
    switch( eNumType )
    {
    case SVX_NUM_CHARS_UPPER_LETTER:
    case SVX_NUM_CHARS_UPPER_LETTER_N:  nRet = 3;       break;
    case SVX_NUM_CHARS_LOWER_LETTER:
    case SVX_NUM_CHARS_LOWER_LETTER_N:  nRet = 4;       break;
    case SVX_NUM_ROMAN_UPPER:           nRet = 1;       break;
    case SVX_NUM_ROMAN_LOWER:           nRet = 2;       break;

    case SVX_NUM_BITMAP:
    case SVX_NUM_CHAR_SPECIAL:         nRet = 23;      break;
    case SVX_NUM_FULL_WIDTH_ARABIC: nRet = 14; break;
    case SVX_NUM_CIRCLE_NUMBER: nRet = 18;break;
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
    case SVX_NUM_NUMBER_UPPER_ZH: nRet = 38; break;
    case SVX_NUM_NUMBER_UPPER_ZH_TW: nRet = 34;break;
    case SVX_NUM_TIAN_GAN_ZH: nRet = 30; break;
    case SVX_NUM_DI_ZI_ZH: nRet = 31; break;
    case SVX_NUM_NUMBER_TRADITIONAL_JA: nRet = 16; break;
    case SVX_NUM_AIU_FULLWIDTH_JA: nRet = 20; break;
    case SVX_NUM_AIU_HALFWIDTH_JA: nRet = 12; break;
    case SVX_NUM_IROHA_FULLWIDTH_JA: nRet = 21; break;
    case SVX_NUM_IROHA_HALFWIDTH_JA: nRet = 13; break;
    case style::NumberingType::HANGUL_SYLLABLE_KO: nRet = 24; break;// ganada
    case style::NumberingType::HANGUL_JAMO_KO: nRet = 25; break;// chosung
    case style::NumberingType::HANGUL_CIRCLED_SYLLABLE_KO: nRet = 24; break;
    case style::NumberingType::HANGUL_CIRCLED_JAMO_KO: nRet = 25; break;
    case style::NumberingType::NUMBER_HANGUL_KO: nRet = 41; break;
    case style::NumberingType::NUMBER_UPPER_KO: nRet = 44; break;
    case SVX_NUM_NUMBER_NONE:           nRet = 0xff;    break;
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
        const SvxBrushItem* pBrush //For i120928,to transfer graphic of bullet
    )
{
    // Start value
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, nStart );

    // Type
    m_rWW8Export.pTableStrm->WriteUChar( GetLevelNFC( nNumberingType ,pOutSet) );

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
    m_rWW8Export.pTableStrm->WriteUChar( nAlign );

    // Write the rgbxchNums[9], positions of placeholders for paragraph
    // numbers in the text
    m_rWW8Export.pTableStrm->WriteBytes(pNumLvlPos, WW8ListManager::nMaxLevel);

    // Type of the character between the bullet and the text
    m_rWW8Export.pTableStrm->WriteUChar( nFollow );

    // dxaSoace/dxaIndent (Word 6 compatibility)
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, 0 );
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, 0 );

    // cbGrpprlChpx
    std::unique_ptr<ww::bytes> pCharAtrs;
    if ( pOutSet )
    {
        std::unique_ptr<ww::bytes> pOldpO = std::move(m_rWW8Export.pO);
        m_rWW8Export.pO.reset(new ww::bytes);
        if ( pFont )
        {
            sal_uInt16 nFontID = m_rWW8Export.m_aFontHelper.GetId( *pFont );

            m_rWW8Export.InsUInt16( NS_sprm::sprmCRgFtc0 );
            m_rWW8Export.InsUInt16( nFontID );
            m_rWW8Export.InsUInt16( NS_sprm::sprmCRgFtc2 );
            m_rWW8Export.InsUInt16( nFontID );
        }

        m_rWW8Export.OutputItemSet( *pOutSet, false, true, i18n::ScriptType::LATIN, m_rWW8Export.m_bExportModeRTF );
        //For i120928,achieve graphic's index of bullet from the bullet bookmark
        if (SVX_NUM_BITMAP == nNumberingType && pBrush)
        {
            int nIndex = m_rWW8Export.GetGrfIndex(*pBrush);
            if ( nIndex != -1 )
            {
                m_rWW8Export.InsUInt16(NS_sprm::sprmCPbiIBullet);
                m_rWW8Export.InsUInt32(nIndex);
                m_rWW8Export.InsUInt16(NS_sprm::sprmCPbiGrf);
                m_rWW8Export.InsUInt16(1);
            }
        }

        pCharAtrs = std::move(m_rWW8Export.pO);
        m_rWW8Export.pO = std::move(pOldpO);
    }
    m_rWW8Export.pTableStrm->WriteUChar(sal_uInt8(pCharAtrs ? pCharAtrs->size() : 0));

    // cbGrpprlPapx
    sal_uInt8 aPapSprms [] = {
        0x5e, 0x84, 0, 0,               // sprmPDxaLeft
        0x60, 0x84, 0, 0,               // sprmPDxaLeft1
        0x15, 0xc6, 0x05, 0x00, 0x01, 0, 0, 0x06
    };
    m_rWW8Export.pTableStrm->WriteUChar( sal_uInt8( sizeof( aPapSprms ) ) );

    // reserved
    SwWW8Writer::WriteShort( *m_rWW8Export.pTableStrm, 0 );

    // pap sprms
    sal_uInt8* pData = aPapSprms + 2;
    Set_UInt16( pData, nIndentAt );
    pData += 2;
    Set_UInt16( pData, nFirstLineIndex );
    pData += 5;
    Set_UInt16( pData, nListTabPos );

    m_rWW8Export.pTableStrm->WriteBytes(aPapSprms, sizeof(aPapSprms));

    // write Chpx
    if (pCharAtrs && !pCharAtrs->empty())
        m_rWW8Export.pTableStrm->WriteBytes(pCharAtrs->data(), pCharAtrs->size());

    // write the num string
    SwWW8Writer::WriteShort( *m_rWW8Export.pTableStrm, rNumberingString.getLength() );
    SwWW8Writer::WriteString16( *m_rWW8Export.pTableStrm, rNumberingString, false );
}

void MSWordExportBase::AbstractNumberingDefinitions()
{
    sal_uInt16 nCount = m_pUsedNumTable->size();
    sal_uInt16 n;

    // prepare the NodeNum to generate the NumString
    SwNumberTree::tNumberVector aNumVector;
    for ( n = 0; n < WW8ListManager::nMaxLevel; ++n )
        aNumVector.push_back( n );

    for( n = 0; n < nCount; ++n )
    {
        AttrOutput().StartAbstractNumbering( n + 1 );

        const SwNumRule& rRule = *(*m_pUsedNumTable)[ n ];
        sal_uInt8 nLvl;
        sal_uInt8 nLevels = static_cast< sal_uInt8 >(rRule.IsContinusNum() ?
            WW8ListManager::nMinLevel : WW8ListManager::nMaxLevel);
        for( nLvl = 0; nLvl < nLevels; ++nLvl )
        {
            // write the static data of the SwNumFormat of this level
            sal_uInt8 aNumLvlPos[WW8ListManager::nMaxLevel] = { 0,0,0,0,0,0,0,0,0 };

            const SwNumFormat& rFormat = rRule.Get( nLvl );

            sal_uInt8 nFollow = 0;
            // #i86652#
            if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                nFollow = 2;     // ixchFollow: 0 - tab, 1 - blank, 2 - nothing
            }
            else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                switch ( rFormat.GetLabelFollowedBy() )
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
            const vcl::Font* pBulletFont=nullptr;
            rtl_TextEncoding eChrSet=0;
            FontFamily eFamily=FAMILY_DECORATIVE;
            if( SVX_NUM_CHAR_SPECIAL == rFormat.GetNumberingType() ||
                SVX_NUM_BITMAP == rFormat.GetNumberingType() )
            {
                sNumStr = OUString(rFormat.GetBulletChar());
                bWriteBullet = true;

                pBulletFont = rFormat.GetBulletFont();
                if (!pBulletFont)
                {
                    pBulletFont = &numfunc::GetDefBulletFont();
                }

                eChrSet = pBulletFont->GetCharSet();
                sFontName = pBulletFont->GetFamilyName();
                eFamily = pBulletFont->GetFamilyType();

                if ( IsStarSymbol(sFontName) )
                    SubstituteBullet( sNumStr, eChrSet, sFontName );

                // #i86652#
                if ( rFormat.GetPositionAndSpaceMode() ==
                                        SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    // <nFollow = 2>, if minimum label width equals 0 and
                    // minimum distance between label and text equals 0
                    nFollow = ( rFormat.GetFirstLineOffset() == 0 &&
                                rFormat.GetCharTextDistance() == 0 )
                              ? 2 : 0;     // ixchFollow: 0 - tab, 1 - blank, 2 - nothing
                }
            }
            else
            {
                if (SVX_NUM_NUMBER_NONE != rFormat.GetNumberingType())
                {
                    sal_uInt8* pLvlPos = aNumLvlPos;
                    // the numbering string has to be restrict
                    // to the level currently working on.
                    sNumStr = rRule.MakeNumString(aNumVector, false, true, nLvl);

                    // now search the nums in the string
                    for( sal_uInt8 i = 0; i <= nLvl; ++i )
                    {
                        OUString sSrch( OUString::number( i ));
                        sal_Int32 nFnd = sNumStr.indexOf( sSrch );
                        if( -1 != nFnd )
                        {
                            *pLvlPos = static_cast<sal_uInt8>(nFnd + rFormat.GetPrefix().getLength() + 1 );
                            ++pLvlPos;
                            sNumStr = sNumStr.replaceAt( nFnd, 1, OUString(static_cast<char>(i)) );
                        }
                    }
                    // #i86652#
                    if ( rFormat.GetPositionAndSpaceMode() ==
                                            SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                    {
                        // <nFollow = 2>, if minimum label width equals 0 and
                        // minimum distance between label and text equals 0
                        nFollow = ( rFormat.GetFirstLineOffset() == 0 &&
                                    rFormat.GetCharTextDistance() == 0 )
                                  ? 2 : 0;     // ixchFollow: 0 - tab, 1 - blank, 2 - nothing
                    }
                }

                if( !rFormat.GetPrefix().isEmpty() )
                    sNumStr = rFormat.GetPrefix() + sNumStr;
                sNumStr += rFormat.GetSuffix();
            }

            // Attributes of the numbering
            std::unique_ptr<wwFont> pPseudoFont;
            const SfxItemSet* pOutSet = nullptr;

            // cbGrpprlChpx
            SfxItemSet aSet( m_pDoc->GetAttrPool(), svl::Items<RES_CHRATR_BEGIN,
                                                  RES_CHRATR_END>{} );
            if ( rFormat.GetCharFormat() || bWriteBullet )
            {
                if ( bWriteBullet )
                {
                    pOutSet = &aSet;

                    if ( rFormat.GetCharFormat() )
                        aSet.Put( rFormat.GetCharFormat()->GetAttrSet() );
                    aSet.ClearItem( RES_CHRATR_CJK_FONT );
                    aSet.ClearItem( RES_CHRATR_FONT );

                    if ( sFontName.isEmpty() )
                        sFontName = pBulletFont->GetFamilyName();

                    pPseudoFont.reset(new wwFont( sFontName, pBulletFont->GetPitch(),
                        eFamily, eChrSet));
                }
                else
                    pOutSet = &rFormat.GetCharFormat()->GetAttrSet();
            }

            sal_Int16 nIndentAt = 0;
            sal_Int16 nFirstLineIndex = 0;
            sal_Int16 nListTabPos = 0;

            // #i86652#
            if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                nIndentAt = nListTabPos = rFormat.GetAbsLSpace(); //TODO: overflow
                nFirstLineIndex = GetWordFirstLineOffset(rFormat);
            }
            else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
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
                    rFormat.GetNumberingType()==SVX_NUM_BITMAP ? rFormat.GetBrush():nullptr);
        }
        AttrOutput().EndAbstractNumbering();
    }
}

void WW8Export::OutOverrideListTab()
{
    if( !m_pUsedNumTable )
        return ;            // no numbering is used

    // write the "list format override" - LFO
    sal_uInt16 nCount = m_pUsedNumTable->size();
    sal_uInt16 n;

    pFib->m_fcPlfLfo = pTableStrm->Tell();
    SwWW8Writer::WriteLong( *pTableStrm, nCount );

    for( n = 0; n < nCount; ++n )
    {
        SwWW8Writer::WriteLong( *pTableStrm, n + 1 );
        SwWW8Writer::FillCount( *pTableStrm, 12 );
    }
    for( n = 0; n < nCount; ++n )
        SwWW8Writer::WriteLong( *pTableStrm, -1 );  // no overwrite

    // set len to FIB
    pFib->m_lcbPlfLfo = pTableStrm->Tell() - pFib->m_fcPlfLfo;
}

void WW8Export::OutListNamesTab()
{
    if( !m_pUsedNumTable )
        return ;            // no numbering is used

    // write the "list format override" - LFO
    sal_uInt16 nNms = 0, nCount = m_pUsedNumTable->size();

    pFib->m_fcSttbListNames = pTableStrm->Tell();
    SwWW8Writer::WriteShort( *pTableStrm, -1 );
    SwWW8Writer::WriteLong( *pTableStrm, nCount );

    for( ; nNms < nCount; ++nNms )
    {
        const SwNumRule& rRule = *(*m_pUsedNumTable)[ nNms ];
        OUString sNm;
        if( !rRule.IsAutoRule() )
            sNm = rRule.GetName();

        SwWW8Writer::WriteShort( *pTableStrm, sNm.getLength() );
        if (!sNm.isEmpty())
            SwWW8Writer::WriteString16(*pTableStrm, sNm, false);
    }

    SwWW8Writer::WriteLong( *pTableStrm, pFib->m_fcSttbListNames + 2, nNms );
    // set len to FIB
    pFib->m_lcbSttbListNames = pTableStrm->Tell() - pFib->m_fcSttbListNames;
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
        rNumStr = rNumStr.replaceAt(0, 1, OUString(
            msfilter::util::bestFitOpenSymbolToMSFont(rNumStr[0], rChrSet, sFontName)));
    }

    rFontName = sFontName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
