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

#include <memory>
#include <queue>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/tencinfo.h>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svtools/rtftoken.h>
#include <svl/itempool.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/debug.hxx>
#include <comphelper/configuration.hxx>

#include <comphelper/string.hxx>

#include <editeng/scriptspaceitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/svxrtf.hxx>
#include <editeng/editids.hrc>
#include <vcl/font.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>


using namespace ::com::sun::star;


static rtl_TextEncoding lcl_GetDefaultTextEncodingForRTF()
{

    OUString aLangString( Application::GetSettings().GetLanguageTag().getLanguage());

    if ( aLangString == "ru" || aLangString == "uk" )
        return RTL_TEXTENCODING_MS_1251;
    if ( aLangString == "tr" )
        return RTL_TEXTENCODING_MS_1254;
    else
        return RTL_TEXTENCODING_MS_1252;
}

// -------------- Methods --------------------

SvxRTFParser::SvxRTFParser( SfxItemPool& rPool, SvStream& rIn )
    : SvRTFParser( rIn, 5 )
    , pAttrPool( &rPool )
    , nDfltFont( 0)
    , bNewDoc( true )
    , bNewGroup( false)
    , bIsSetDfltTab( false)
    , bChkStyleAttr( false )
    , bCalcValue( false )
    , bIsLeftToRightDef( true)
    , bIsInReadStyleTab( false)
{
    pDfltFont.emplace();
    mxDefaultColor = Color();

    // generate the correct WhichId table from the set WhichIds.
    BuildWhichTable();
}

SvxRTFParser::~SvxRTFParser()
{
    if( !aAttrStack.empty() )
        ClearAttrStack();
}

void SvxRTFParser::SetInsPos( const EditPosition& rNew )
{
    mxInsertPosition = rNew;
}

SvParserState SvxRTFParser::CallParser()
{
    DBG_ASSERT( mxInsertPosition, "no insertion position");

    if( !mxInsertPosition )
        return SvParserState::Error;

    if( !maColorTable.empty() )
        ClearColorTbl();
    m_FontTable.clear();
    m_StyleTable.clear();
    if( !aAttrStack.empty() )
        ClearAttrStack();

    bIsSetDfltTab = false;
    bNewGroup = false;
    nDfltFont = 0;

    return SvRTFParser::CallParser();
}

void SvxRTFParser::Continue( int nToken )
{
    SvRTFParser::Continue( nToken );

    SvParserState eStatus = GetStatus();
    if (eStatus != SvParserState::Pending && eStatus != SvParserState::Error)
    {
        SetAllAttrOfStk();
    //Regardless of what "color 0" is, word defaults to auto as the default colour.
    //e.g. see #i7713#
    }
}


// is called for each token that is recognized in CallParser
void SvxRTFParser::NextToken( int nToken )
{
    sal_Unicode cCh;
    switch( nToken )
    {
    case RTF_COLORTBL:      ReadColorTable();       break;
    case RTF_FONTTBL:       ReadFontTable();        break;
    case RTF_STYLESHEET:    ReadStyleTable();       break;

    case RTF_DEFF:
            if( bNewDoc )
            {
                if (!m_FontTable.empty())
                    // Can immediately be set
                    SetDefault( nToken, nTokenValue );
                else
                    // is set after reading the font table
                    nDfltFont = int(nTokenValue);
            }
            break;

    case RTF_DEFTAB:
    case RTF_DEFLANG:
            if( bNewDoc )
                SetDefault( nToken, nTokenValue );
            break;


    case RTF_PICT:          ReadBitmapData();       break;

    case RTF_LINE:          cCh = '\n'; goto INSINGLECHAR;
    case RTF_TAB:           cCh = '\t'; goto INSINGLECHAR;
    case RTF_SUBENTRYINDEX: cCh = ':';  goto INSINGLECHAR;

    case RTF_EMDASH:        cCh = 0x2014;   goto INSINGLECHAR;
    case RTF_ENDASH:        cCh = 0x2013;   goto INSINGLECHAR;
    case RTF_BULLET:        cCh = 0x2022;   goto INSINGLECHAR;
    case RTF_LQUOTE:        cCh = 0x2018;   goto INSINGLECHAR;
    case RTF_RQUOTE:        cCh = 0x2019;   goto INSINGLECHAR;
    case RTF_LDBLQUOTE:     cCh = 0x201C;   goto INSINGLECHAR;
    case RTF_RDBLQUOTE:     cCh = 0x201D;   goto INSINGLECHAR;
INSINGLECHAR:
        aToken = OUStringChar(cCh);
        [[fallthrough]]; // aToken is set as Text
    case RTF_TEXTTOKEN:
        {
            InsertText();
            // all collected Attributes are set
            for (size_t n = m_AttrSetList.size(); n; )
            {
                auto const& pStkSet = m_AttrSetList[--n];
                SetAttrSet( *pStkSet );
                m_AttrSetList.pop_back();
            }
        }
        break;


    case RTF_PAR:
        InsertPara();
        break;
    case '{':
        if (bNewGroup)          // Nesting!
            GetAttrSet_();
        bNewGroup = true;
        break;
    case '}':
        if( !bNewGroup )        // Empty Group ??
            AttrGroupEnd();
        bNewGroup = false;
        break;
    case RTF_INFO:
        SkipGroup();
        break;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // First overwrite all (all have to be in one group!!)
    // Could also appear in the RTF-file without the IGNORE-Flag; all Groups
    // with the IGNORE-Flag are overwritten in the default branch.

    case RTF_SWG_PRTDATA:
    case RTF_FIELD:
    case RTF_ATNID:
    case RTF_ANNOTATION:

    case RTF_BKMKSTART:
    case RTF_BKMKEND:
    case RTF_BKMK_KEY:
    case RTF_XE:
    case RTF_TC:
    case RTF_NEXTFILE:
    case RTF_TEMPLATE:
    // RTF_SHPRSLT disabled for #i19718#
                            SkipGroup();
                            break;
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    case RTF_PGDSCNO:
    case RTF_PGBRK:
    case RTF_SHADOW:
            if( RTF_IGNOREFLAG != GetStackPtr( -1 )->nTokenId )
                break;
            nToken = SkipToken();
            if( '{' == GetStackPtr( -1 )->nTokenId )
                nToken = SkipToken();

            ReadAttr( nToken, &GetAttrSet() );
            break;

    default:
        switch( nToken & ~(0xff | RTF_SWGDEFS) )
        {
        case RTF_PARFMT:        // here are no SWGDEFS
            ReadAttr( nToken, &GetAttrSet() );
            break;

        case RTF_CHRFMT:
        case RTF_BRDRDEF:
        case RTF_TABSTOPDEF:

            if( RTF_SWGDEFS & nToken)
            {
                if( RTF_IGNOREFLAG != GetStackPtr( -1 )->nTokenId )
                    break;
                nToken = SkipToken();
                if( '{' == GetStackPtr( -1 )->nTokenId )
                {
                    nToken = SkipToken();
                }
            }
            ReadAttr( nToken, &GetAttrSet() );
            break;
        default:
            {
                if( RTF_IGNOREFLAG == GetStackPtr( -1 )->nTokenId &&
                      '{' == GetStackPtr( -2 )->nTokenId )
                    SkipGroup();
            }
            break;
        }
        break;
    }
}

void SvxRTFParser::ReadStyleTable()
{
    int bSaveChkStyleAttr = bChkStyleAttr ? 1 : 0;
    sal_uInt16 nStyleNo = 0;
    bool bHasStyleNo = false;
    int _nOpenBrackets = 1;      // the first was already detected earlier!!
    std::optional<SvxRTFStyleType> xStyle(SvxRTFStyleType(*pAttrPool, aWhichMap));
    xStyle->aAttrSet.Put( GetRTFDefaults() );

    bIsInReadStyleTab = true;
    bChkStyleAttr = false;      // Do not check Attribute against the Styles

    while( _nOpenBrackets && IsParserWorking() )
    {
        int nToken = GetNextToken();
        switch( nToken )
        {
        case '}':       if( --_nOpenBrackets && IsParserWorking() )
                            // Style has been completely read,
                            // so this is still a stable status
                            SaveState( RTF_STYLESHEET );
                        break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    SkipToken();
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                            RTF_PN != nToken )
                    SkipToken( -2 );
                else
                {
                    // filter out at once
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SvParserState::Error;
                    break;
                }
                ++_nOpenBrackets;
            }
            break;

        case RTF_SBASEDON:  xStyle->nBasedOn = sal_uInt16(nTokenValue); break;
        case RTF_SNEXT:     break;
        case RTF_OUTLINELEVEL:
        case RTF_SOUTLVL:   xStyle->nOutlineNo = sal_uInt8(nTokenValue);    break;
        case RTF_S:         nStyleNo = static_cast<short>(nTokenValue);
                            bHasStyleNo = true;
                            break;
        case RTF_CS:        nStyleNo = static_cast<short>(nTokenValue);
                            bHasStyleNo = true;
                            break;

        case RTF_TEXTTOKEN:
            if (bHasStyleNo)
            {
                DelCharAtEnd( aToken, ';' );
                xStyle->sName = aToken.toString();

                if (!m_StyleTable.empty())
                {
                    m_StyleTable.erase(nStyleNo);
                }
                // All data from the font is available, so off to the table
                m_StyleTable.emplace(nStyleNo, std::move(*xStyle));
                xStyle.emplace(*pAttrPool, aWhichMap);
                xStyle->aAttrSet.Put( GetRTFDefaults() );
                nStyleNo = 0;
                bHasStyleNo = false;
            }
            break;
        default:
            switch( nToken & ~(0xff | RTF_SWGDEFS) )
            {
            case RTF_PARFMT:        // here are no SWGDEFS
                ReadAttr( nToken, &xStyle->aAttrSet );
                break;

            case RTF_CHRFMT:
            case RTF_BRDRDEF:
            case RTF_TABSTOPDEF:
#ifndef NDEBUG
                auto nEnteringToken = nToken;
#endif
                auto nEnteringIndex = m_nTokenIndex;
                int nSkippedTokens = 0;
                if( RTF_SWGDEFS & nToken)
                {
                    if( RTF_IGNOREFLAG != GetStackPtr( -1 )->nTokenId )
                        break;
                    nToken = SkipToken();
                    ++nSkippedTokens;
                    if( '{' == GetStackPtr( -1 )->nTokenId )
                    {
                        nToken = SkipToken();
                        ++nSkippedTokens;
                    }
                }
                ReadAttr( nToken, &xStyle->aAttrSet );
                if (nSkippedTokens && m_nTokenIndex == nEnteringIndex - nSkippedTokens)
                {
                    // we called SkipToken to go back one or two, but ReadAttrs
                    // read nothing, so on next loop of the outer while we
                    // would end up in the same state again (assert that)
                    assert(nEnteringToken == GetNextToken());
                    // and loop endlessly, skip format a token
                    // instead to avoid that
                    SkipToken(nSkippedTokens);
                }
                break;
            }
            break;
        }
    }
    xStyle.reset();         // Delete the Last Style
    SkipToken();        // the closing brace is evaluated "above"

    // Flag back to old state
    bChkStyleAttr = bSaveChkStyleAttr;
    bIsInReadStyleTab = false;
}

void SvxRTFParser::ReadColorTable()
{
    int nToken;
    sal_uInt8 nRed = 0xff, nGreen = 0xff, nBlue = 0xff;

    for (;;)
    {
        nToken = GetNextToken();
        if ( '}' == nToken || !IsParserWorking() )
            break;
        switch( nToken )
        {
        case RTF_RED:   nRed = sal_uInt8(nTokenValue);      break;
        case RTF_GREEN: nGreen = sal_uInt8(nTokenValue);        break;
        case RTF_BLUE:  nBlue = sal_uInt8(nTokenValue);     break;

        case RTF_TEXTTOKEN:
            if( 1 == aToken.getLength()
                    ? aToken[ 0 ] != ';'
                    : -1 == aToken.indexOf( ";" ) )
                break;      // At least the ';' must be found

            [[fallthrough]];

        case ';':
            if( IsParserWorking() )
            {
                // one color is finished, fill in the table
                // try to map the values to SV internal names
                Color aColor( nRed, nGreen, nBlue );
                if( maColorTable.empty() &&
                    sal_uInt8(-1) == nRed && sal_uInt8(-1) == nGreen && sal_uInt8(-1) == nBlue )
                    aColor = COL_AUTO;
                maColorTable.push_back( aColor );
                nRed = 0;
                nGreen = 0;
                nBlue = 0;

                // Color has been completely read,
                // so this is still a stable status
                SaveState( RTF_COLORTBL );
            }
            break;
        }
    }
    SkipToken();        // the closing brace is evaluated "above"
}

void SvxRTFParser::ReadFontTable()
{
    int _nOpenBrackets = 1;      // the first was already detected earlier!!
    vcl::Font aFont;
    short nFontNo(0), nInsFontNo (0);
    OUString sAltNm, sFntNm;
    bool bIsAltFntNm = false;

    rtl_TextEncoding nSystemChar = lcl_GetDefaultTextEncodingForRTF();
    aFont.SetCharSet( nSystemChar );
    SetEncoding( nSystemChar );

    while( _nOpenBrackets && IsParserWorking() )
    {
        bool bCheckNewFont = false;
        int nToken = GetNextToken();
        switch( nToken )
        {
            case '}':
                bIsAltFntNm = false;
                // Style has been completely read,
                // so this is still a stable status
                if( --_nOpenBrackets <= 1 && IsParserWorking() )
                    SaveState( RTF_FONTTBL );
                bCheckNewFont = true;
                nInsFontNo = nFontNo;
                break;
            case '{':
                if( RTF_IGNOREFLAG != GetNextToken() )
                    SkipToken();
                // immediately skip unknown and all known but non-evaluated
                // groups
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                        RTF_PANOSE != nToken && RTF_FNAME != nToken &&
                        RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
                    SkipToken( -2 );
                else
                {
                    // filter out at once
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SvParserState::Error;
                    break;
                }
                ++_nOpenBrackets;
                break;
            case RTF_FROMAN:
                aFont.SetFamily( FAMILY_ROMAN );
                break;
            case RTF_FSWISS:
                aFont.SetFamily( FAMILY_SWISS );
                break;
            case RTF_FMODERN:
                aFont.SetFamily( FAMILY_MODERN );
                break;
            case RTF_FSCRIPT:
                aFont.SetFamily( FAMILY_SCRIPT );
                break;
            case RTF_FDECOR:
                aFont.SetFamily( FAMILY_DECORATIVE );
                break;
            // for technical/symbolic font of the rtl_TextEncoding is changed!
            case RTF_FTECH:
                aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
                [[fallthrough]];
            case RTF_FNIL:
                aFont.SetFamily( FAMILY_DONTKNOW );
                break;
            case RTF_FCHARSET:
                if (-1 != nTokenValue)
                {
                    rtl_TextEncoding nrtl_TextEncoding = rtl_getTextEncodingFromWindowsCharset(
                        static_cast<sal_uInt8>(nTokenValue));
                    aFont.SetCharSet(nrtl_TextEncoding);
                    //When we're in a font, the fontname is in the font
                    //charset, except for symbol fonts I believe
                    if (nrtl_TextEncoding == RTL_TEXTENCODING_SYMBOL)
                        nrtl_TextEncoding = RTL_TEXTENCODING_DONTKNOW;
                    SetEncoding(nrtl_TextEncoding);
                }
                break;
            case RTF_FPRQ:
                switch( nTokenValue )
                {
                    case 1:
                        aFont.SetPitch( PITCH_FIXED );
                        break;
                    case 2:
                        aFont.SetPitch( PITCH_VARIABLE );
                        break;
                }
                break;
            case RTF_F:
                bCheckNewFont = true;
                nInsFontNo = nFontNo;
                nFontNo = static_cast<short>(nTokenValue);
                break;
            case RTF_FALT:
                bIsAltFntNm = true;
                break;
            case RTF_TEXTTOKEN:
                DelCharAtEnd( aToken, ';' );
                if ( !aToken.isEmpty() )
                {
                    if( bIsAltFntNm )
                        sAltNm = aToken;
                    else
                        sFntNm = aToken;
                }
                break;
        }

        if( bCheckNewFont && 1 >= _nOpenBrackets && !sFntNm.isEmpty() )  // one font is ready
        {
            // All data from the font is available, so off to the table
            if (!sAltNm.isEmpty())
                sFntNm += ";" + sAltNm;

            aFont.SetFamilyName( sFntNm );
            m_FontTable.insert(std::make_pair(nInsFontNo, aFont));
            aFont = vcl::Font();
            aFont.SetCharSet( nSystemChar );
            sAltNm.clear();
            sFntNm.clear();
        }
    }
    SkipToken();        // the closing brace is evaluated "above"

    // set the default font in the Document
    if( bNewDoc && IsParserWorking() )
        SetDefault( RTF_DEFF, nDfltFont );
}

void SvxRTFParser::ClearColorTbl()
{
    maColorTable.clear();
}

void SvxRTFParser::ClearAttrStack()
{
    aAttrStack.clear();
}

void SvxRTFParser::DelCharAtEnd( OUStringBuffer& rStr, const sal_Unicode cDel )
{
    rStr.strip(' ');
    if( !rStr.isEmpty() && cDel == rStr[ rStr.getLength()-1 ])
        rStr.setLength( rStr.getLength()-1 );
}


const vcl::Font& SvxRTFParser::GetFont( sal_uInt16 nId )
{
    SvxRTFFontTbl::const_iterator it = m_FontTable.find( nId );
    if (it != m_FontTable.end())
    {
        return it->second;
    }
    const SvxFontItem& rDfltFont =
        pAttrPool->GetUserOrPoolDefaultItem(aPlainMap[SID_ATTR_CHAR_FONT]);
    pDfltFont->SetFamilyName( rDfltFont.GetStyleName() );
    pDfltFont->SetFamily( rDfltFont.GetFamily() );
    return *pDfltFont;
}

std::unique_ptr<SvxRTFItemStackType> SvxRTFItemStackType::createSvxRTFItemStackType(
    SfxItemPool& rPool, const WhichRangesContainer& pWhichRange, const EditPosition& rEditPosition)
{
    struct MakeUniqueEnabler : public SvxRTFItemStackType
    {
        MakeUniqueEnabler(SfxItemPool& rPool, const WhichRangesContainer& pWhichRange, const EditPosition& rEditPosition)
            : SvxRTFItemStackType(rPool, pWhichRange, rEditPosition)
        {
        }
    };
    return std::make_unique<MakeUniqueEnabler>(rPool, pWhichRange, rEditPosition);
}

SvxRTFItemStackType* SvxRTFParser::GetAttrSet_()
{
    SvxRTFItemStackType* pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();
    std::unique_ptr<SvxRTFItemStackType> xNew;
    if( pCurrent )
        xNew = std::make_unique<SvxRTFItemStackType>(*pCurrent, *mxInsertPosition, false/*bCopyAttr*/);
    else
        xNew = SvxRTFItemStackType::createSvxRTFItemStackType(*pAttrPool, aWhichMap, *mxInsertPosition);
    xNew->SetRTFDefaults( GetRTFDefaults() );

    aAttrStack.push_back( std::move(xNew) );

    if (aAttrStack.size() > 96 && comphelper::IsFuzzing())
        throw std::range_error("ecStackOverflow");

    bNewGroup = false;
    return aAttrStack.back().get();
}

void SvxRTFParser::ClearStyleAttr_( SvxRTFItemStackType& rStkType )
{
    // check attributes to the attributes of the stylesheet or to
    // the default attrs of the document
    SfxItemSet &rSet = rStkType.GetAttrSet();
    const SfxItemPool& rPool = *rSet.GetPool();
    const SfxPoolItem* pItem;
    SfxWhichIter aIter( rSet );

    if( !IsChkStyleAttr() ||
        !rStkType.GetAttrSet().Count() ||
        m_StyleTable.count( rStkType.nStyleNo ) == 0 )
    {
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if (SfxItemPool::IsWhich(nWhich) &&
                SfxItemState::SET == aIter.GetItemState( false, &pItem ) &&
                     rPool.GetUserOrPoolDefaultItem( nWhich ) == *pItem )
                aIter.ClearItem();       // delete
        }
    }
    else
    {
        // Delete all Attributes, which are already defined in the Style,
        // from the current AttrSet.
        auto & rStyle = m_StyleTable.find(rStkType.nStyleNo)->second;
        SfxItemSet &rStyleSet = rStyle.aAttrSet;
        const SfxPoolItem* pSItem;
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SfxItemState::SET == rStyleSet.GetItemState( nWhich, true, &pSItem ))
            {
                if( SfxItemState::SET == aIter.GetItemState( false, &pItem )
                    && *pItem == *pSItem )
                    rSet.ClearItem( nWhich );       // delete
            }
            else if (SfxItemPool::IsWhich(nWhich) &&
                    SfxItemState::SET == aIter.GetItemState( false, &pItem ) &&
                     rPool.GetUserOrPoolDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // delete
        }
    }
}

void SvxRTFParser::AttrGroupEnd()   // process the current, delete from Stack
{
    if( aAttrStack.empty() )
        return;

    std::unique_ptr<SvxRTFItemStackType> pOld = std::move(aAttrStack.back());
    aAttrStack.pop_back();
    SvxRTFItemStackType *pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();

    do {        // middle check loop
        sal_Int32 nOldSttNdIdx = pOld->mxStartNodeIdx->GetIdx();
        if (pOld->maChildList.empty() &&
            ((!pOld->aAttrSet.Count() && !pOld->nStyleNo ) ||
            (nOldSttNdIdx == mxInsertPosition->GetNodeIdx() &&
            pOld->nSttCnt == mxInsertPosition->GetCntIdx() )))
            break;          // no attributes or Area

        // set only the attributes that are different from the parent
        if( pCurrent && pOld->aAttrSet.Count() )
        {
            // ITEM: SfxItemIter and removing SfxPoolItems:
            // iterating and clearing Items on the same incarnation is in
            // general a bad idea, it invalidates iterators. Work around
            // this by remembering the WhichIDs of Items to delete
            std::vector<sal_uInt16> aDeleteWhichIDs;

            for (SfxItemIter aIter(pOld->aAttrSet); !aIter.IsAtEnd(); aIter.NextItem())
            {
                const SfxPoolItem* pGet(nullptr);
                if (SfxItemState::SET == pCurrent->aAttrSet.GetItemState(aIter.GetCurWhich(), false, &pGet)
                    && *aIter.GetCurItem() == *pGet)
                    aDeleteWhichIDs.push_back(aIter.GetCurWhich());
            }

            for (auto nDelWhich : aDeleteWhichIDs)
                pOld->aAttrSet.ClearItem(nDelWhich);

            if (!pOld->aAttrSet.Count() && pOld->maChildList.empty() &&
                !pOld->nStyleNo )
                break;
        }

        // Set all attributes which have been defined from start until here
        bool bCrsrBack = !mxInsertPosition->GetCntIdx();
        if( bCrsrBack )
        {
            // at the beginning of a paragraph? Move back one position
            sal_Int32 nNd = mxInsertPosition->GetNodeIdx();
            MovePos(false);
            // if can not move backward then later don't move forward !
            bCrsrBack = nNd != mxInsertPosition->GetNodeIdx();
        }

        if( pOld->mxStartNodeIdx->GetIdx() < mxInsertPosition->GetNodeIdx() ||
            ( pOld->mxStartNodeIdx->GetIdx() == mxInsertPosition->GetNodeIdx() &&
              pOld->nSttCnt <= mxInsertPosition->GetCntIdx() ) )
        {
            if( !bCrsrBack )
            {
                // all pard attributes are only valid until the previous
                // paragraph !!
                if( nOldSttNdIdx == mxInsertPosition->GetNodeIdx() )
                {
                }
                else
                {
                    // Now it gets complicated:
                    // - all character attributes sre keep the area
                    // - all paragraph attributes to get the area
                    //   up to the previous paragraph
                    auto xNew = std::make_unique<SvxRTFItemStackType>(*pOld, *mxInsertPosition, true);
                    xNew->aAttrSet.SetParent( pOld->aAttrSet.GetParent() );

                    // Delete all paragraph attributes from xNew
                    for (const auto& pair : aPardMap.data)
                        if (sal_uInt16 wid = pair.second)
                            xNew->aAttrSet.ClearItem(wid);
                    xNew->SetRTFDefaults( GetRTFDefaults() );

                    // Were there any?
                    if( xNew->aAttrSet.Count() == pOld->aAttrSet.Count() )
                    {
                        xNew.reset();
                    }
                    else
                    {
                        xNew->nStyleNo = 0;

                        // Now span the real area of xNew from old
                        SetEndPrevPara( pOld->mxEndNodeIdx, pOld->nEndCnt );
                        xNew->nSttCnt = 0;

                        if( IsChkStyleAttr() )
                        {
                            ClearStyleAttr_( *pOld );
                            ClearStyleAttr_( *xNew );   //#i10381#, methinks.
                        }

                        if( pCurrent )
                        {
                            pCurrent->Add(std::move(pOld));
                            pCurrent->Add(std::move(xNew));
                        }
                        else
                        {
                            // Last off the stack, thus cache it until the next text was
                            // read. (Span no attributes!)

                            m_AttrSetList.push_back(std::move(pOld));
                            m_AttrSetList.push_back(std::move(xNew));
                        }
                        break;
                    }
                }
            }

            pOld->mxEndNodeIdx = mxInsertPosition->MakeNodeIdx();
            pOld->nEndCnt = mxInsertPosition->GetCntIdx();

            /*
            #i21422#
            If the parent (pCurrent) sets something e.g. , and the child (pOld)
            unsets it and the style both are based on has it unset then
            clearing the pOld by looking at the style is clearly a disaster
            as the text ends up with pCurrents bold and not pOlds no bold, this
            should be rethought out. For the moment its safest to just do
            the clean if we have no parent, all we suffer is too many
            redundant properties.
            */
            if (IsChkStyleAttr() && !pCurrent)
                ClearStyleAttr_( *pOld );

            if( pCurrent )
            {
                pCurrent->Add(std::move(pOld));
                // split up and create new entry, because it makes no sense
                // to create a "so long" depend list. Bug 95010
                if (bCrsrBack && 50 < pCurrent->maChildList.size())
                {
                    // at the beginning of a paragraph? Move back one position
                    MovePos();
                    bCrsrBack = false;

                    // Open a new Group.
                    auto xNew(std::make_unique<SvxRTFItemStackType>(*pCurrent, *mxInsertPosition, true));
                    xNew->SetRTFDefaults( GetRTFDefaults() );

                    // Set all until here valid Attributes
                    AttrGroupEnd();
                    pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();  // can be changed after AttrGroupEnd!
                    xNew->aAttrSet.SetParent( pCurrent ? &pCurrent->aAttrSet : nullptr );
                    aAttrStack.push_back( std::move(xNew) );
                }
            }
            else
                // Last off the stack, thus cache it until the next text was
                // read. (Span no attributes!)
                m_AttrSetList.push_back(std::move(pOld));
        }

        if( bCrsrBack )
            // at the beginning of a paragraph? Move back one position
            MovePos();

    } while( false );

    bNewGroup = false;
}

void SvxRTFParser::SetAllAttrOfStk()        // end all Attr. and set it into doc
{
    // repeat until all attributes will be taken from stack
    while( !aAttrStack.empty() )
        AttrGroupEnd();

    for (size_t n = m_AttrSetList.size(); n; )
    {
        auto const& pStkSet = m_AttrSetList[--n];
        SetAttrSet( *pStkSet );
        pStkSet->DropChildList();
        m_AttrSetList.pop_back();
    }
}

// sets all the attributes that are different from the current
void SvxRTFParser::SetAttrSet( SvxRTFItemStackType &rSet )
{
    // Was DefTab never read? then set to default
    if( !bIsSetDfltTab )
        SetDefault( RTF_DEFTAB, 720 );

    if (!rSet.maChildList.empty())
        rSet.Compress( *this );
    if( rSet.aAttrSet.Count() || rSet.nStyleNo )
        SetAttrInDoc( rSet );

    // then process all the children
    for (size_t n = 0; n < rSet.maChildList.size(); ++n)
        SetAttrSet( *(rSet.maChildList[ n ]) );
}

// Has no text been inserted yet? (SttPos from the top Stack entry!)
bool SvxRTFParser::IsAttrSttPos()
{
    SvxRTFItemStackType* pCurrent = aAttrStack.empty() ? nullptr : aAttrStack.back().get();
    return !pCurrent || (pCurrent->mxStartNodeIdx->GetIdx() == mxInsertPosition->GetNodeIdx() &&
        pCurrent->nSttCnt == mxInsertPosition->GetCntIdx());
}


void SvxRTFParser::SetAttrInDoc( SvxRTFItemStackType & )
{
}

void SvxRTFParser::BuildWhichTable()
{
    aWhichMap.reset();

    // Here are the IDs for all paragraph attributes, which can be detected by
    // SvxParser and can be set in a SfxItemSet. The IDs are set correctly through
    // the SlotIds from POOL.
    static constexpr sal_uInt16 WIDS1[] {
             SID_ATTR_PARA_LINESPACE,
             SID_ATTR_PARA_ADJUST,
             SID_ATTR_TABSTOP,
             SID_ATTR_PARA_HYPHENZONE,
             SID_ATTR_LRSPACE,
             SID_ATTR_ULSPACE,
             SID_ATTR_BRUSH,
             SID_ATTR_BORDER_OUTER,
             SID_ATTR_BORDER_SHADOW,
             SID_ATTR_PARA_OUTLLEVEL,
             SID_ATTR_PARA_SPLIT,
             SID_ATTR_PARA_KEEP,
             SID_PARA_VERTALIGN,
             SID_ATTR_PARA_SCRIPTSPACE,
             SID_ATTR_PARA_HANGPUNCTUATION,
             SID_ATTR_PARA_FORBIDDEN_RULES,
             SID_ATTR_FRAMEDIRECTION,
         };
    for (sal_uInt16 nWid : WIDS1)
    {
        sal_uInt16 nTrueWid = pAttrPool->GetTrueWhichIDFromSlotID(nWid, false);
        aPardMap.data[nWid] = nTrueWid;
        if (nTrueWid == 0)
            continue;
        aWhichMap = aWhichMap.MergeRange(nTrueWid, nTrueWid);
    }

    // Here are the IDs for all character attributes, which can be detected by
    // SvxParser and can be set in a SfxItemSet. The IDs are set correctly through
    // the SlotIds from POOL.
    static constexpr sal_uInt16 WIDS[] {
             SID_ATTR_CHAR_CASEMAP,        SID_ATTR_BRUSH_CHAR,        SID_ATTR_CHAR_COLOR,
             SID_ATTR_CHAR_CONTOUR,        SID_ATTR_CHAR_STRIKEOUT,    SID_ATTR_CHAR_ESCAPEMENT,
             SID_ATTR_CHAR_FONT,           SID_ATTR_CHAR_FONTHEIGHT,   SID_ATTR_CHAR_KERNING,
             SID_ATTR_CHAR_LANGUAGE,       SID_ATTR_CHAR_POSTURE,      SID_ATTR_CHAR_SHADOWED,
             SID_ATTR_CHAR_UNDERLINE,      SID_ATTR_CHAR_OVERLINE,     SID_ATTR_CHAR_WEIGHT,
             SID_ATTR_CHAR_WORDLINEMODE,   SID_ATTR_CHAR_AUTOKERN,     SID_ATTR_CHAR_CJK_FONT,
             SID_ATTR_CHAR_CJK_FONTHEIGHT, sal_uInt16(SID_ATTR_CHAR_CJK_LANGUAGE), SID_ATTR_CHAR_CJK_POSTURE,
             SID_ATTR_CHAR_CJK_WEIGHT,     SID_ATTR_CHAR_CTL_FONT,     SID_ATTR_CHAR_CTL_FONTHEIGHT,
             SID_ATTR_CHAR_CTL_LANGUAGE,   SID_ATTR_CHAR_CTL_POSTURE,  SID_ATTR_CHAR_CTL_WEIGHT,
             SID_ATTR_CHAR_EMPHASISMARK,   SID_ATTR_CHAR_TWO_LINES,    SID_ATTR_CHAR_SCALEWIDTH,
             SID_ATTR_CHAR_ROTATED,        SID_ATTR_CHAR_RELIEF,       SID_ATTR_CHAR_HIDDEN,
         };
    for (sal_uInt16 nWid : WIDS)
    {
        sal_uInt16 nTrueWid = pAttrPool->GetTrueWhichIDFromSlotID(nWid, false);
        aPlainMap.data[nWid] = nTrueWid;
        if (nTrueWid == 0)
            continue;
        aWhichMap = aWhichMap.MergeRange(nTrueWid, nTrueWid);
    }
}

const SfxItemSet& SvxRTFParser::GetRTFDefaults()
{
    if( !pRTFDefaults )
    {
        pRTFDefaults.reset(new SfxItemSet(*pAttrPool, aWhichMap));
        if (const sal_uInt16 nId = aPardMap[SID_ATTR_PARA_SCRIPTSPACE])
        {
            SvxScriptSpaceItem aItem( false, nId );
            if( bNewDoc )
                pAttrPool->SetUserDefaultItem( aItem );
            else
                pRTFDefaults->Put( aItem );
        }
    }
    return *pRTFDefaults;
}


SvxRTFStyleType::SvxRTFStyleType(SfxItemPool& rPool, const WhichRangesContainer& pWhichRange)
    : aAttrSet(rPool, pWhichRange)
    , nBasedOn(0)
    , nOutlineNo(sal_uInt8(-1))         // not set
{
}

SvxRTFItemStackType::SvxRTFItemStackType(
        SfxItemPool& rPool, const WhichRangesContainer& pWhichRange,
        const EditPosition& rPos )
    : aAttrSet( rPool, pWhichRange )
    , mxStartNodeIdx(rPos.MakeNodeIdx())
#if !defined(__COVERITY__)
    // coverity 2020 has difficulty wrt std::optional leading to bogus 'Uninitialized scalar variable'
    , mxEndNodeIdx(mxStartNodeIdx)
#endif
    , nSttCnt(rPos.GetCntIdx())
    , nEndCnt(nSttCnt)
    , nStyleNo(0)
{
}

SvxRTFItemStackType::SvxRTFItemStackType(
        const SvxRTFItemStackType& rCpy,
        const EditPosition& rPos,
        bool const bCopyAttr )
    : aAttrSet( *rCpy.aAttrSet.GetPool(), rCpy.aAttrSet.GetRanges() )
    , mxStartNodeIdx(rPos.MakeNodeIdx())
#if !defined(__COVERITY__)
    // coverity 2020 has difficulty wrt std::optional leading to bogus 'Uninitialized scalar variable'
    , mxEndNodeIdx(mxStartNodeIdx)
#endif
    , nSttCnt(rPos.GetCntIdx())
    , nEndCnt(nSttCnt)
    , nStyleNo(rCpy.nStyleNo)
{
    aAttrSet.SetParent( &rCpy.aAttrSet );
    if( bCopyAttr )
        aAttrSet.Put( rCpy.aAttrSet );
}

/* ofz#13491 SvxRTFItemStackType dtor recursively
   calls the dtor of its m_pChildList. The recurse
   depth can grow sufficiently to trigger asan.

   So breadth-first iterate through the nodes
   and make a flat vector of them which can
   be iterated through in order of most
   distant from root first and release
   their children linearly
*/
void SvxRTFItemStackType::DropChildList()
{
    if (maChildList.empty())
        return;

    std::vector<SvxRTFItemStackType*> bfs;
    std::queue<SvxRTFItemStackType*> aQueue;
    aQueue.push(this);

    while (!aQueue.empty())
    {
        auto* front = aQueue.front();
        aQueue.pop();
        if (!front->maChildList.empty())
        {
            for (const auto& a : front->maChildList)
                aQueue.push(a.get());
            bfs.push_back(front);
        }
    }

    for (auto it = bfs.rbegin(); it != bfs.rend(); ++it)
    {
        SvxRTFItemStackType* pNode = *it;
        pNode->maChildList.clear();
    }
}

SvxRTFItemStackType::~SvxRTFItemStackType()
{
}

void SvxRTFItemStackType::Add(std::unique_ptr<SvxRTFItemStackType> pIns)
{
    maChildList.push_back(std::move(pIns));
}

void SvxRTFItemStackType::SetStartPos( const EditPosition& rPos )
{
    mxStartNodeIdx = rPos.MakeNodeIdx();
    mxEndNodeIdx = mxStartNodeIdx;
    nSttCnt = rPos.GetCntIdx();
}

void SvxRTFItemStackType::Compress( const SvxRTFParser& rParser )
{
    ENSURE_OR_RETURN_VOID(!maChildList.empty(), "Compress: ChildList empty");

    SvxRTFItemStackType* pTmp = maChildList[0].get();

    if( !pTmp->aAttrSet.Count() ||
        mxStartNodeIdx->GetIdx() != pTmp->mxStartNodeIdx->GetIdx() ||
        nSttCnt != pTmp->nSttCnt )
        return;

    EditNodeIdx aLastNd = *pTmp->mxEndNodeIdx;
    sal_Int32 nLastCnt = pTmp->nEndCnt;

    SfxItemSet aMrgSet( pTmp->aAttrSet );
    for (size_t n = 1; n < maChildList.size(); ++n)
    {
        pTmp = maChildList[n].get();
        if (!pTmp->maChildList.empty())
            pTmp->Compress( rParser );

        if( !pTmp->nSttCnt
            ? (aLastNd.GetIdx()+1 != pTmp->mxStartNodeIdx->GetIdx() ||
               !rParser.IsEndPara( &aLastNd, nLastCnt ) )
            : ( pTmp->nSttCnt != nLastCnt ||
                aLastNd.GetIdx() != pTmp->mxStartNodeIdx->GetIdx() ))
        {
            while (++n < maChildList.size())
            {
                pTmp = maChildList[n].get();
                if (!pTmp->maChildList.empty())
                    pTmp->Compress( rParser );
            }
            return;
        }

        if( n )
        {
            // Search for all which are set over the whole area
            // ITEM: SfxItemIter and removing SfxPoolItems:
            std::vector<sal_uInt16> aDeleteWhichIDs;

            for (SfxItemIter aIter(aMrgSet); !aIter.IsAtEnd(); aIter.NextItem())
            {
                const SfxPoolItem* pGet(nullptr);
                if (SfxItemState::SET != pTmp->aAttrSet.GetItemState(aIter.GetCurWhich(), false, &pGet)
                    || *aIter.GetCurItem() != *pGet)
                    aDeleteWhichIDs.push_back(aIter.GetCurWhich());
            }

            for (auto nDelWhich : aDeleteWhichIDs)
                aMrgSet.ClearItem(nDelWhich);

            if( !aMrgSet.Count() )
                return;
        }

        aLastNd = *pTmp->mxEndNodeIdx;
        nLastCnt = pTmp->nEndCnt;
    }

    if( mxEndNodeIdx->GetIdx() != aLastNd.GetIdx() || nEndCnt != nLastCnt )
        return;

    // It can be merged
    aAttrSet.Put( aMrgSet );

    size_t n = 0, nChildLen = maChildList.size();
    while (n < nChildLen)
    {
        pTmp = maChildList[n].get();
        pTmp->aAttrSet.Differentiate( aMrgSet );

        if (pTmp->maChildList.empty() && !pTmp->aAttrSet.Count() && !pTmp->nStyleNo)
        {
            maChildList.erase( maChildList.begin() + n );
            --nChildLen;
            continue;
        }
        ++n;
    }
}
void SvxRTFItemStackType::SetRTFDefaults( const SfxItemSet& rDefaults )
{
    if( rDefaults.Count() )
    {
        SfxItemIter aIter( rDefaults );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        do {
            sal_uInt16 nWhich = pItem->Which();
            if( SfxItemState::SET != aAttrSet.GetItemState( nWhich, false ))
                aAttrSet.Put(*pItem);

            pItem = aIter.NextItem();
        } while(pItem);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
