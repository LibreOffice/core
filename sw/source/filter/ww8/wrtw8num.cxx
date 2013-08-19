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
#include <doc.hxx>
#include <docary.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <unotools/fontcfg.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <doctok/sprmids.hxx>

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
    const OUString sPrefix("WW8TempExport" + OUString::number( nUniqueList++ ));
    SwNumRule* pMyNumRule =
            new SwNumRule( pDoc->GetUniqueNumRuleName( &sPrefix ),
                           SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    pUsedNumTbl->push_back( pMyNumRule );

    for ( sal_uInt16 i = 0; i < MAXLEVEL; i++ )
    {
        const SwNumFmt& rSubRule = pRule->Get(i);
        pMyNumRule->Set( i, rSubRule );
    }

    SwNumFmt aNumFmt( pMyNumRule->Get( nLevel ) );
    aNumFmt.SetStart( nVal );
    pMyNumRule->Set( nLevel, aNumFmt );

    nNumId = GetId( *pMyNumRule );

    // Map the old list to our new list
    aRuleDuplicates[GetId( *pRule )] = nNumId;

    return nNumId;
}

sal_uInt16 MSWordExportBase::GetId( const SwNumRule& rNumRule )
{
    if ( !pUsedNumTbl )
    {
        pUsedNumTbl = new SwNumRuleTbl;
        pUsedNumTbl->insert( pUsedNumTbl->begin(), pDoc->GetNumRuleTbl().begin(), pDoc->GetNumRuleTbl().end() );
        // Check, if the outline rule is already inserted into <pUsedNumTbl>.
        // If yes, do not insert it again.
        bool bOutlineRuleAdded( false );
        for ( sal_uInt16 n = pUsedNumTbl->size(); n; )
        {
            const SwNumRule& rRule = *(*pUsedNumTbl)[ --n ];
            if ( !pDoc->IsUsed( rRule ) )
            {
                pUsedNumTbl->erase( pUsedNumTbl->begin() + n );
            }
            else if ( &rRule == pDoc->GetOutlineNumRule() )
            {
                bOutlineRuleAdded = true;
            }
        }

        if ( !bOutlineRuleAdded )
        {
            // still need to paste the OutlineRule
            SwNumRule* pR = (SwNumRule*)pDoc->GetOutlineNumRule();
            pUsedNumTbl->push_back( pR );
        }
    }
    SwNumRule* p = (SwNumRule*)&rNumRule;
    sal_uInt16 nRet = pUsedNumTbl->GetPos(p);

    // Is this list now duplicated into a new list which we should use
    // #i77812# - perform 'deep' search in duplication map
    ::std::map<sal_uInt16,sal_uInt16>::const_iterator aResult = aRuleDuplicates.end();
    do {
        aResult = aRuleDuplicates.find(nRet);
        if ( aResult != aRuleDuplicates.end() )
        {
            nRet = (*aResult).second;
        }
    } while ( aResult != aRuleDuplicates.end() );

    return nRet;
}

// GetFirstLineOffset should problem never appear unadorned apart from
// here in the ww export filter
sal_Int16 GetWordFirstLineOffset(const SwNumFmt &rFmt)
{
    OSL_ENSURE( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<GetWordFirstLineOffset> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    short nFirstLineOffset;
    if (rFmt.GetNumAdjust() == SVX_ADJUST_RIGHT)
        nFirstLineOffset = -rFmt.GetCharTextDistance();
    else
        nFirstLineOffset = rFmt.GetFirstLineOffset();
    return nFirstLineOffset;
}

void WW8Export::WriteNumbering()
{
    if ( !pUsedNumTbl )
        return; // no numbering is used

    // list formats - LSTF
    pFib->fcPlcfLst = pTableStrm->Tell();
    SwWW8Writer::WriteShort( *pTableStrm, pUsedNumTbl->size() );
    NumberingDefinitions();
    // set len to FIB
    pFib->lcbPlcfLst = pTableStrm->Tell() - pFib->fcPlcfLst;

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

    sal_uInt8 nFlags = 0, nDummy = 0;
    if ( rRule.IsContinusNum() )
        nFlags |= 0x1;

    *m_rWW8Export.pTableStrm << nFlags << nDummy;
}

void MSWordExportBase::NumberingDefinitions()
{
    if ( !pUsedNumTbl )
        return; // no numbering is used

    sal_uInt16 nCount = pUsedNumTbl->size();

    // Write static data of SwNumRule - LSTF
    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        const SwNumRule& rRule = *(*pUsedNumTbl)[ n ];

        AttrOutput().NumberingDefinition( n + 1, rRule );
    }
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
        const String &rNumberingString,
        const SvxBrushItem* pBrush //For i120928,to transfer graphic of bullet
    )
{
    // Start value
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, nStart );

    // Type
    *m_rWW8Export.pTableStrm << WW8Export::GetNumId( nNumberingType );

    // Justification
    sal_uInt8 nAlign;
    switch ( eAdjust )
    {
    case SVX_ADJUST_CENTER:
        nAlign = 1;
        break;
    case SVX_ADJUST_RIGHT:
        nAlign = 2;
        break;
    default:
        nAlign = 0;
        break;
    }
    *m_rWW8Export.pTableStrm << nAlign;

    // Write the rgbxchNums[9], positions of placeholders for paragraph
    // numbers in the text
    m_rWW8Export.pTableStrm->Write( pNumLvlPos, WW8ListManager::nMaxLevel );

    // Type of the character between the bullet and the text
    *m_rWW8Export.pTableStrm << nFollow;

    // dxaSoace/dxaIndent (Word 6 compatibility)
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, 0 );
    SwWW8Writer::WriteLong( *m_rWW8Export.pTableStrm, 0 );

    // cbGrpprlChpx
    ww::bytes aCharAtrs;
    if ( pOutSet )
    {
        ww::bytes* pOldpO = m_rWW8Export.pO;
        m_rWW8Export.pO = &aCharAtrs;
        if ( pFont )
        {
            sal_uInt16 nFontID = m_rWW8Export.maFontHelper.GetId( *pFont );

            if ( m_rWW8Export.bWrtWW8 )
            {
                m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc0 );
                m_rWW8Export.InsUInt16( nFontID );
                m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc2 );
            }
            else
                m_rWW8Export.pO->push_back( 93 );
            m_rWW8Export.InsUInt16( nFontID );
        }

        m_rWW8Export.OutputItemSet( *pOutSet, false, true, i18n::ScriptType::LATIN, m_rWW8Export.mbExportModeRTF );
        //For i120928,achieve graphic's index of bullet from the bullet bookmark
        if (SVX_NUM_BITMAP == nNumberingType && pBrush)
        {
            int nIndex = m_rWW8Export.GetGrfIndex(*pBrush);
            if ( nIndex != -1 )
            {
                m_rWW8Export.InsUInt16(0x6887);
                m_rWW8Export.InsUInt32(nIndex);
                m_rWW8Export.InsUInt16(0x4888);
                m_rWW8Export.InsUInt16(1);
            }
        }

        m_rWW8Export.pO = pOldpO;
    }
    *m_rWW8Export.pTableStrm << sal_uInt8( aCharAtrs.size() );

    // cbGrpprlPapx
    sal_uInt8 aPapSprms [] = {
        0x5e, 0x84, 0, 0,               // sprmPDxaLeft
        0x60, 0x84, 0, 0,               // sprmPDxaLeft1
        0x15, 0xc6, 0x05, 0x00, 0x01, 0, 0, 0x06
    };
    *m_rWW8Export.pTableStrm << sal_uInt8( sizeof( aPapSprms ) );

    // reserved
    SwWW8Writer::WriteShort( *m_rWW8Export.pTableStrm, 0 );

    // pap sprms
    sal_uInt8* pData = aPapSprms + 2;
    Set_UInt16( pData, nIndentAt );
    pData += 2;
    Set_UInt16( pData, nFirstLineIndex );
    pData += 5;
    Set_UInt16( pData, nListTabPos );

    m_rWW8Export.pTableStrm->Write( aPapSprms, sizeof( aPapSprms ));

    // write Chpx
    if( !aCharAtrs.empty() )
        m_rWW8Export.pTableStrm->Write( aCharAtrs.data(), aCharAtrs.size() );

    // write the num string
    SwWW8Writer::WriteShort( *m_rWW8Export.pTableStrm, rNumberingString.Len() );
    SwWW8Writer::WriteString16( *m_rWW8Export.pTableStrm, rNumberingString, false );
}

void MSWordExportBase::AbstractNumberingDefinitions()
{
    sal_uInt16 nCount = pUsedNumTbl->size();
    sal_uInt16 n;

    // prepare the NodeNum to generate the NumString
    SwNumberTree::tNumberVector aNumVector;
    for ( n = 0; n < WW8ListManager::nMaxLevel; ++n )
        aNumVector.push_back( n );

    for( n = 0; n < nCount; ++n )
    {
        AttrOutput().StartAbstractNumbering( n + 1 );

        const SwNumRule& rRule = *(*pUsedNumTbl)[ n ];
        sal_uInt8 nLvl;
        sal_uInt8 nLevels = static_cast< sal_uInt8 >(rRule.IsContinusNum() ?
            WW8ListManager::nMinLevel : WW8ListManager::nMaxLevel);
        for( nLvl = 0; nLvl < nLevels; ++nLvl )
        {
            // write the static data of the SwNumFmt of this level
            sal_uInt8 aNumLvlPos[WW8ListManager::nMaxLevel] = { 0,0,0,0,0,0,0,0,0 };

            const SwNumFmt& rFmt = rRule.Get( nLvl );

            sal_uInt8 nFollow = 0;
            // #i86652#
            if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                nFollow = 2;     // ixchFollow: 0 - tab, 1 - blank, 2 - nothing
            }
            else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                switch ( rFmt.GetLabelFollowedBy() )
                {
                    case SvxNumberFormat::LISTTAB:
                    {
            // 0 (tab) unless there would be no content before the tab, in which case 2 (nothing)
            nFollow = (SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType()) ? 0 : 2;
                    }
                    break;
                    case SvxNumberFormat::SPACE:
                    {
            // 1 (space) unless there would be no content before the space in which case 2 (nothing)
            nFollow = (SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType()) ? 1 : 2;
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
            String sNumStr;
            String sFontName;
            bool bWriteBullet = false;
            const Font* pBulletFont=0;
            rtl_TextEncoding eChrSet=0;
            FontFamily eFamily=FAMILY_DECORATIVE;
            if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType() ||
                SVX_NUM_BITMAP == rFmt.GetNumberingType() )
            {
                sNumStr = rFmt.GetBulletChar();
                bWriteBullet = true;

                pBulletFont = rFmt.GetBulletFont();
                if (!pBulletFont)
                {
                    pBulletFont = &numfunc::GetDefBulletFont();
                }

                eChrSet = pBulletFont->GetCharSet();
                sFontName = pBulletFont->GetName();
                eFamily = pBulletFont->GetFamily();

                if ( IsStarSymbol(sFontName) )
                    SubstituteBullet( sNumStr, eChrSet, sFontName );

                // #i86652#
                if ( rFmt.GetPositionAndSpaceMode() ==
                                        SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    // <nFollow = 2>, if minimum label width equals 0 and
                    // minimum distance between label and text equals 0
                    nFollow = ( rFmt.GetFirstLineOffset() == 0 &&
                                rFmt.GetCharTextDistance() == 0 )
                              ? 2 : 0;     // ixchFollow: 0 - tab, 1 - blank, 2 - nothing
                }
            }
            else
            {
                if (SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType())
                {
                    sal_uInt8* pLvlPos = aNumLvlPos;
                    // the numbering string has to be restrict
                    // to the level currently working on.
                    sNumStr = rRule.MakeNumString(aNumVector, false, true, nLvl);

                    // now search the nums in the string
                    for( sal_uInt8 i = 0; i <= nLvl; ++i )
                    {
                        String sSrch( OUString::number( i ));
                        xub_StrLen nFnd = sNumStr.Search( sSrch );
                        if( STRING_NOTFOUND != nFnd )
                        {
                            *pLvlPos = (sal_uInt8)(nFnd + rFmt.GetPrefix().getLength() + 1 );
                            ++pLvlPos;
                            sNumStr.SetChar( nFnd, (char)i );
                        }
                    }
                    // #i86652#
                    if ( rFmt.GetPositionAndSpaceMode() ==
                                            SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                    {
                        // <nFollow = 2>, if minimum label width equals 0 and
                        // minimum distance between label and text equals 0
                        nFollow = ( rFmt.GetFirstLineOffset() == 0 &&
                                    rFmt.GetCharTextDistance() == 0 )
                                  ? 2 : 0;     // ixchFollow: 0 - tab, 1 - blank, 2 - nothing
                    }
                }

                if( !rFmt.GetPrefix().isEmpty() )
                    sNumStr.Insert( rFmt.GetPrefix(), 0 );
                sNumStr += rFmt.GetSuffix();
            }

            // Attributes of the numbering
            wwFont *pPseudoFont = NULL;
            const SfxItemSet* pOutSet = NULL;

            // cbGrpprlChpx
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_CHRATR_BEGIN,
                                                  RES_CHRATR_END );
            if ( rFmt.GetCharFmt() || bWriteBullet )
            {
                if ( bWriteBullet )
                {
                    pOutSet = &aSet;

                    if ( rFmt.GetCharFmt() )
                        aSet.Put( rFmt.GetCharFmt()->GetAttrSet() );
                    aSet.ClearItem( RES_CHRATR_CJK_FONT );
                    aSet.ClearItem( RES_CHRATR_FONT );

                    if ( !sFontName.Len() )
                        sFontName = pBulletFont->GetName();

                    pPseudoFont = new wwFont( sFontName, pBulletFont->GetPitch(),
                        eFamily, eChrSet, SupportsUnicode() );
                }
                else
                    pOutSet = &rFmt.GetCharFmt()->GetAttrSet();
            }

            sal_Int16 nIndentAt = 0;
            sal_Int16 nFirstLineIndex = 0;
            sal_Int16 nListTabPos = 0;

            // #i86652#
            if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                nIndentAt = nListTabPos = rFmt.GetAbsLSpace();
                nFirstLineIndex = GetWordFirstLineOffset(rFmt);
            }
            else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                nIndentAt = static_cast<sal_Int16>(rFmt.GetIndentAt());
                nFirstLineIndex = static_cast<sal_Int16>(rFmt.GetFirstLineIndent());
                nListTabPos = rFmt.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB?
                              static_cast<sal_Int16>( rFmt.GetListtabPos() ) : 0;
            }

            AttrOutput().NumberingLevel( nLvl,
                    rFmt.GetStart(),
                    rFmt.GetNumberingType(),
                    rFmt.GetNumAdjust(),
                    aNumLvlPos,
                    nFollow,
                    pPseudoFont, pOutSet,
                    nIndentAt, nFirstLineIndex, nListTabPos,
                    sNumStr,
                    rFmt.GetNumberingType()==SVX_NUM_BITMAP ? rFmt.GetBrush():0);

            delete pPseudoFont;
        }
        AttrOutput().EndAbstractNumbering();
    }
}

void WW8Export::OutOverrideListTab()
{
    if( !pUsedNumTbl )
        return ;            // no numbering is used

    // write the "list format override" - LFO
    sal_uInt16 nCount = pUsedNumTbl->size();
    sal_uInt16 n;

    pFib->fcPlfLfo = pTableStrm->Tell();
    SwWW8Writer::WriteLong( *pTableStrm, nCount );

    for( n = 0; n < nCount; ++n )
    {
        SwWW8Writer::WriteLong( *pTableStrm, n + 1 );
        SwWW8Writer::FillCount( *pTableStrm, 12 );
    }
    for( n = 0; n < nCount; ++n )
        SwWW8Writer::WriteLong( *pTableStrm, -1 );  // no overwrite

    // set len to FIB
    pFib->lcbPlfLfo = pTableStrm->Tell() - pFib->fcPlfLfo;
}

void WW8Export::OutListNamesTab()
{
    if( !pUsedNumTbl )
        return ;            // no numbering is used

    // write the "list format override" - LFO
    sal_uInt16 nNms = 0, nCount = pUsedNumTbl->size();

    pFib->fcSttbListNames = pTableStrm->Tell();
    SwWW8Writer::WriteShort( *pTableStrm, -1 );
    SwWW8Writer::WriteLong( *pTableStrm, nCount );

    for( ; nNms < nCount; ++nNms )
    {
        const SwNumRule& rRule = *(*pUsedNumTbl)[ nNms ];
        String sNm;
        if( !rRule.IsAutoRule() )
            sNm = rRule.GetName();

        SwWW8Writer::WriteShort( *pTableStrm, sNm.Len() );
        if (sNm.Len())
            SwWW8Writer::WriteString16(*pTableStrm, sNm, false);
    }

    SwWW8Writer::WriteLong( *pTableStrm, pFib->fcSttbListNames + 2, nNms );
    // set len to FIB
    pFib->lcbSttbListNames = pTableStrm->Tell() - pFib->fcSttbListNames;
}


// old WW95-Code

void WW8Export::OutputOlst( const SwNumRule& rRule )
{
    if ( bWrtWW8 )
        return;

    static const sal_uInt8 aAnlvBase[] = { // Defaults
                                1,0,0,          // Upper Roman
                                0x0C,           // Hanging Indent, fPrev
                                0,0,1,0x80,0,0,1,0,0x1b,1,0,0 };

    static const sal_uInt8 aSprmOlstHdr[] = { 133, 212 };

    pO->insert( pO->end(), aSprmOlstHdr, aSprmOlstHdr+sizeof( aSprmOlstHdr ) );
    WW8_OLST aOlst;
    memset( &aOlst, 0, sizeof(aOlst) );
    sal_uInt8* pC = aOlst.rgch;
    sal_uInt8* pChars = (sal_uInt8*)pC;
    sal_uInt16 nCharLen = 64;

    for (sal_uInt16 j = 0; j < WW8ListManager::nMaxLevel; j++ ) // 9 variable ANLVs
    {
        memcpy( &aOlst.rganlv[j], aAnlvBase, sizeof( WW8_ANLV ) );  // Defaults

        const SwNumFmt* pFmt = rRule.GetNumFmt( j );
        if( pFmt )
            BuildAnlvBase( aOlst.rganlv[j], pChars, nCharLen, rRule,
                            *pFmt, (sal_uInt8)j );
    }

    pO->insert( pO->end(), (sal_uInt8*)&aOlst, (sal_uInt8*)&aOlst+sizeof( aOlst ) );
}


void WW8Export::Out_WwNumLvl( sal_uInt8 nWwLevel )
{
    pO->push_back( 13 );
    pO->push_back( nWwLevel );
}

void WW8Export::Out_SwNumLvl( sal_uInt8 nSwLevel )
{
    OSL_ENSURE( nSwLevel < MAXLEVEL, "numbered?");
    Out_WwNumLvl( nSwLevel + 1 );
}

void WW8Export::BuildAnlvBulletBase(WW8_ANLV& rAnlv, sal_uInt8*& rpCh,
    sal_uInt16& rCharLen, const SwNumFmt& rFmt)
{
    ByteToSVBT8(11, rAnlv.nfc);

    sal_uInt8 nb = 0;                                // type of number
    switch (rFmt.GetNumAdjust())
    {
        case SVX_ADJUST_RIGHT:
            nb = 2;
            break;
        case SVX_ADJUST_CENTER:
            nb = 1;
            break;
        case SVX_ADJUST_BLOCK:
        case SVX_ADJUST_BLOCKLINE:
            nb = 3;
            break;
        case SVX_ADJUST_LEFT:
        case SVX_ADJUST_END:
            break;
    }

    // #i86652#
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        if (GetWordFirstLineOffset(rFmt) < 0)
            nb |= 0x8;          // number will be displayed using a hanging indent
    }
    ByteToSVBT8(nb, rAnlv.aBits1);

    if (1 < rCharLen)
    {
        const Font& rFont = rFmt.GetBulletFont()
                            ? *rFmt.GetBulletFont()
                            : numfunc::GetDefBulletFont();
        String sNumStr = OUString(rFmt.GetBulletChar());
        rtl_TextEncoding eChrSet = rFont.GetCharSet();
        String sFontName = rFont.GetName();

        sal_uInt16 nFontId;
        if ( IsStarSymbol(sFontName) )
        {
            /*
            If we are starsymbol then in ww7- mode we will always convert to a
            windows 8bit symbol font and an index into it, to conversion to
            8 bit is complete at this stage.
            */
            SubstituteBullet(sNumStr, eChrSet, sFontName);
            wwFont aPseudoFont(sFontName, rFont.GetPitch(), rFont.GetFamily(),
                eChrSet, bWrtWW8);
            nFontId = maFontHelper.GetId(aPseudoFont);
            *rpCh = static_cast<sal_uInt8>(sNumStr.GetChar(0));
        }
        else
        {
            /*
            Otherwise we are a unicode char and need to be converted back to
            an 8 bit format. We happen to know that if the font is already an
            8 bit windows font currently, staroffice promotes the char into
            the F000->F0FF range, so we can undo this, and we'll be back to
            the equivalent 8bit location, otherwise we have to convert from
            true unicode to an 8bit charset
            */
            nFontId = maFontHelper.GetId(rFont);
            sal_Unicode cChar = sNumStr.GetChar(0);
            if ( (eChrSet == RTL_TEXTENCODING_SYMBOL) && (cChar >= 0xF000) && (
                cChar <= 0xF0FF) )
            {
                *rpCh = static_cast< sal_uInt8 >(cChar - 0xF000);
            }
            else
                *rpCh = OUStringToOString(OUString(cChar), eChrSet).toChar();
        }
        rpCh++;
        rCharLen--;
        ShortToSVBT16(nFontId, rAnlv.ftc);
        ByteToSVBT8( 1, rAnlv.cbTextBefore );
    }
    // #i86652#
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        ShortToSVBT16( -GetWordFirstLineOffset(rFmt), rAnlv.dxaIndent );
        ShortToSVBT16( rFmt.GetCharTextDistance(), rAnlv.dxaSpace );
    }
    else
    {
        ShortToSVBT16( 0, rAnlv.dxaIndent );
        ShortToSVBT16( 0, rAnlv.dxaSpace );
    }
}

void MSWordExportBase::SubstituteBullet( String& rNumStr,
    rtl_TextEncoding& rChrSet, String& rFontName ) const
{
    if (!bSubstituteBullets)
        return;
    OUString sFontName = rFontName;

    // If Bullet char is "", don't change
    if (rNumStr.GetChar(0) != sal_Unicode(0x0))
    {
        rNumStr.SetChar(0, msfilter::util::bestFitOpenSymbolToMSFont(rNumStr.GetChar(0), rChrSet, sFontName, !SupportsUnicode()));
    }

    rFontName = sFontName;
}

static void SwWw8_InsertAnlText( const String& rStr, sal_uInt8*& rpCh,
                                 sal_uInt16& rCharLen, SVBT8& r8Len )
{
    sal_uInt8 nb = 0;
    ww::bytes aO;
    SwWW8Writer::InsAsString8( aO, rStr, RTL_TEXTENCODING_MS_1252 );

    sal_uInt16 nCnt = aO.size();
    if( nCnt && nCnt < rCharLen )
    {
        nb = (sal_uInt8)nCnt;
        std::copy( aO.begin(), aO.end(), rpCh );
        rpCh += nCnt;
        rCharLen = rCharLen - nCnt;
    }
    ByteToSVBT8( nb, r8Len );
}

void WW8Export::BuildAnlvBase(WW8_ANLV& rAnlv, sal_uInt8*& rpCh,
    sal_uInt16& rCharLen, const SwNumRule& rRul, const SwNumFmt& rFmt,
    sal_uInt8 nSwLevel)
{
    ByteToSVBT8(WW8Export::GetNumId(rFmt.GetNumberingType()), rAnlv.nfc);

    sal_uInt8 nb = 0;
    switch (rFmt.GetNumAdjust())
    {
        case SVX_ADJUST_RIGHT:
            nb = 2;
            break;
        case SVX_ADJUST_CENTER:
            nb = 1;
            break;
        case SVX_ADJUST_BLOCK:
        case SVX_ADJUST_BLOCKLINE:
            nb = 3;
            break;
        case SVX_ADJUST_LEFT:
        case SVX_ADJUST_END:
            break;
    }

    bool bInclUpper = rFmt.GetIncludeUpperLevels() > 0;
    if( bInclUpper )
        nb |= 0x4;          // include previous levels

    if (GetWordFirstLineOffset(rFmt) < 0)
        nb |= 0x8;          // number will be displayed using a hanging indent
    ByteToSVBT8( nb, rAnlv.aBits1 );

    if( bInclUpper && !rRul.IsContinusNum() )
    {
        if( (nSwLevel >= WW8ListManager::nMinLevel )
            && (nSwLevel<= WW8ListManager::nMaxLevel )
            && (rFmt.GetNumberingType() != SVX_NUM_NUMBER_NONE ) )  // UEberhaupt Nummerierung ?
        {                                               // -> suche, ob noch Zahlen davor
            sal_uInt8 nUpper = rFmt.GetIncludeUpperLevels();
            if( (nUpper <= WW8ListManager::nMaxLevel) &&
                (rRul.Get(nUpper).GetNumberingType() != SVX_NUM_NUMBER_NONE ) ) // Nummerierung drueber ?
            {
                                                    // dann Punkt einfuegen
                SwWw8_InsertAnlText( aDotStr, rpCh, rCharLen,
                                    rAnlv.cbTextBefore );
            }

        }
    }
    else
    {
        SwWw8_InsertAnlText( rFmt.GetPrefix(), rpCh, rCharLen,
                             rAnlv.cbTextBefore );
        SwWw8_InsertAnlText( rFmt.GetSuffix(), rpCh, rCharLen,
                             rAnlv.cbTextAfter );
    }

    ShortToSVBT16( rFmt.GetStart(), rAnlv.iStartAt );
    // #i86652#
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        ShortToSVBT16( -GetWordFirstLineOffset(rFmt), rAnlv.dxaIndent );
        ShortToSVBT16( rFmt.GetCharTextDistance(), rAnlv.dxaSpace );
    }
    else
    {
        ShortToSVBT16( 0, rAnlv.dxaIndent );
        ShortToSVBT16( 0, rAnlv.dxaSpace );
    }
}

void WW8Export::Out_NumRuleAnld( const SwNumRule& rRul, const SwNumFmt& rFmt,
                                   sal_uInt8 nSwLevel )
{
    static const sal_uInt8 aSprmAnldDefault[54] = {
                         12, 52,
                         1,0,0,0x0c,0,0,1,0x80,0,0,1,0,0x1b,1,0,0,0x2e,
                         0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    sal_uInt8 aSprmAnld[54];

    memcpy( aSprmAnld, aSprmAnldDefault, sizeof( aSprmAnld ) );
    WW8_ANLD* pA = (WW8_ANLD*)(aSprmAnld + 2);  // handy pointer

    sal_uInt8* pChars = (sal_uInt8*)(pA->rgchAnld);
    sal_uInt16 nCharLen = 31;

    if( nSwLevel == 11 )
        BuildAnlvBulletBase( pA->eAnlv, pChars, nCharLen, rFmt );
    else
        BuildAnlvBase( pA->eAnlv, pChars, nCharLen, rRul, rFmt, nSwLevel );

    // ... spit it out
    OutSprmBytes( (sal_uInt8*)&aSprmAnld, sizeof( aSprmAnld ) );
}


// Return: is it an outline?
bool WW8Export::Out_SwNum(const SwTxtNode* pNd)
{
    int nLevel = pNd->GetActualListLevel();

    if (nLevel < 0 || nLevel >= MAXLEVEL)
    {
        OSL_FAIL("Invalid level");

        return false;
    }

    sal_uInt8 nSwLevel = static_cast< sal_uInt8 >(nLevel);

    const SwNumRule* pRul = pNd->GetNumRule();
    if( !pRul || nSwLevel == WW8ListManager::nMaxLevel)
        return false;

    bool bRet = true;

    SwNumFmt aFmt(pRul->Get(nSwLevel));
    // #i86652#
    if ( aFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        const SvxLRSpaceItem& rLR = ItemGet<SvxLRSpaceItem>(*pNd, RES_LR_SPACE);
        aFmt.SetAbsLSpace(writer_cast<short>(aFmt.GetAbsLSpace() + rLR.GetLeft()));
    }

    if (
         aFmt.GetNumberingType() == SVX_NUM_NUMBER_NONE ||
         aFmt.GetNumberingType() == SVX_NUM_CHAR_SPECIAL ||
         aFmt.GetNumberingType() == SVX_NUM_BITMAP
       )
    {
        Out_WwNumLvl(11);
        Out_NumRuleAnld(*pRul, aFmt, 11);
        bRet = false;
    }
    else if (
              pRul->IsContinusNum() ||
              (pRul->Get(1).GetIncludeUpperLevels() <= 1)
            )
    {
        Out_WwNumLvl(10);
        Out_NumRuleAnld(*pRul, aFmt, 10);
        bRet = false;
    }
    else
    {
        Out_SwNumLvl(nSwLevel);
        Out_NumRuleAnld(*pRul, aFmt, nSwLevel);
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
