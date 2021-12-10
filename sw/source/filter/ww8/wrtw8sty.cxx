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

#include <algorithm>

#include <memory>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <hintids.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdouno.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <doc.hxx>
#include "wrtww8.hxx"
#include <docary.hxx>
#include <poolfmt.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <ndtxt.hxx>
#include <ftninfo.hxx>
#include <fmthdft.hxx>
#include <section.hxx>
#include <fmtcntnt.hxx>
#include <fmtftn.hxx>
#include <ndindex.hxx>
#include <txtftn.hxx>
#include <charfmt.hxx>
#include <docufld.hxx>
#include <dcontact.hxx>
#include <fmtcnct.hxx>
#include <ftnidx.hxx>
#include <fmtclds.hxx>
#include <lineinfo.hxx>
#include <fmtline.hxx>
#include <swtable.hxx>
#include <redline.hxx>
#include <msfilter.hxx>
#include <swmodule.hxx>
#include <charatr.hxx>

#include "sprmids.hxx"

#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include <wwstyles.hxx>
#include "ww8par.hxx"
#include "ww8attributeoutput.hxx"
#include "docxattributeoutput.hxx"
#include "rtfattributeoutput.hxx"

#include <unordered_set>

using namespace css;
using namespace sw::util;
using namespace nsHdFtFlags;

/// For the output of sections.
struct WW8_PdAttrDesc
{
    std::unique_ptr<sal_uInt8[]> m_pData;
    sal_uInt16 m_nLen;
    WW8_FC m_nSepxFcPos;
    WW8_PdAttrDesc() : m_nLen(0), m_nSepxFcPos(0xffffffff) /*default: none*/
        { }
};

namespace {

struct WW8_SED
{
    SVBT16 aBits1;      // orientation change + internal, Default: 6
    SVBT32 fcSepx;      //  FC  file offset to beginning of SEPX for section.
                        //  0xFFFFFFFF for no Sprms
    SVBT16 fnMpr;       //  used internally by Windows Word, Default: 0
    SVBT32 fcMpr;       //  FC, points to offset in FC space for MacWord
                        // Default: 0xffffffff ( nothing )
                        //  cbSED is 12 (decimal)), C (hex).
};

}

// class WW8_WrPlc0 is only used for header and footer positioning
// ie there is no content support structure
class WW8_WrPlc0
{
private:
    std::vector<sal_uLong> aPos;      // PTRARR of CPs / FCs
    sal_uLong nOfs;

    WW8_WrPlc0(WW8_WrPlc0 const&) = delete;
    WW8_WrPlc0& operator=(WW8_WrPlc0 const&) = delete;

public:
    explicit WW8_WrPlc0( sal_uLong nOffset );
    sal_uInt16 Count() const                { return aPos.size(); }
    void Append( sal_uLong nStartCpOrFc );
    void Write( SvStream& rStrm );
};

//  Styles

#define WW8_RESERVED_SLOTS 15

// GetId( SwCharFormat ) for use in text -> zero is not allowed,
// use "Default Char Style" instead
sal_uInt16 MSWordExportBase::GetId( const SwCharFormat* pFormat ) const
{
    sal_uInt16 nRet = m_pStyles->GetSlot( pFormat );
    return ( nRet != 0x0fff ) ? nRet : 10;      // Default Char Style
}

// GetId( SwTextFormatColl ) for use in TextNodes -> zero is not allowed,
// "Standard" instead
sal_uInt16 MSWordExportBase::GetId( const SwTextFormatColl& rColl ) const
{
    sal_uInt16 nRet = m_pStyles->GetSlot( &rColl );
    return ( nRet != 0xfff ) ? nRet : 0;        // Default TextFormatColl
}

//typedef pFormatT
MSWordStyles::MSWordStyles( MSWordExportBase& rExport, bool bListStyles )
    : m_rExport( rExport ),
    m_bListStyles(bListStyles)
{
    // if exist any Foot-/End-Notes then get from the EndNoteInfo struct
    // the CharFormats. They will create it!
    if ( !m_rExport.m_rDoc.GetFootnoteIdxs().empty() )
    {
        m_rExport.m_rDoc.GetEndNoteInfo().GetAnchorCharFormat( m_rExport.m_rDoc );
        m_rExport.m_rDoc.GetEndNoteInfo().GetCharFormat( m_rExport.m_rDoc );
        m_rExport.m_rDoc.GetFootnoteInfo().GetAnchorCharFormat( m_rExport.m_rDoc );
        m_rExport.m_rDoc.GetFootnoteInfo().GetCharFormat( m_rExport.m_rDoc );
    }
    sal_uInt16 nAlloc = WW8_RESERVED_SLOTS + m_rExport.m_rDoc.GetCharFormats()->size() - 1 +
                                         m_rExport.m_rDoc.GetTextFormatColls()->size() - 1 +
                                         (bListStyles ? m_rExport.m_rDoc.GetNumRuleTable().size() - 1 : 0);

    // somewhat generous ( free for up to 15 )
    m_pFormatA.reset( new SwFormat*[ nAlloc ] );
    memset( m_pFormatA.get(), 0, nAlloc * sizeof( SwFormat* ) );
    memset( m_aHeadingParagraphStyles, -1 , MAXLEVEL * sizeof( sal_uInt16));

    BuildStylesTable();
    BuildStyleIds();
}

MSWordStyles::~MSWordStyles()
{
}

// Sty_SetWWSlot() dependencies for the styles -> zero is allowed
sal_uInt16 MSWordStyles::GetSlot( const SwFormat* pFormat ) const
{
    sal_uInt16 n;
    for ( n = 0; n < m_nUsedSlots; n++ )
        if ( m_pFormatA[n] == pFormat )
            return n;
    return 0xfff;                   // 0xfff: WW: zero
}

sal_uInt16 MSWordStyles::BuildGetSlot( const SwFormat& rFormat )
{
    sal_uInt16 nRet = rFormat.GetPoolFormatId();
    switch ( nRet )
    {
        case RES_POOLCOLL_STANDARD:
            nRet = 0;
            break;

        case RES_POOLCOLL_HEADLINE1:
        case RES_POOLCOLL_HEADLINE2:
        case RES_POOLCOLL_HEADLINE3:
        case RES_POOLCOLL_HEADLINE4:
        case RES_POOLCOLL_HEADLINE5:
        case RES_POOLCOLL_HEADLINE6:
        case RES_POOLCOLL_HEADLINE7:
        case RES_POOLCOLL_HEADLINE8:
        case RES_POOLCOLL_HEADLINE9:
            nRet -= RES_POOLCOLL_HEADLINE1-1;
            break;

        default:
            nRet = m_nUsedSlots++;
            break;
    }
    return nRet;
}


sal_uInt16 MSWordStyles::GetWWId( const SwFormat& rFormat )
{
    sal_uInt16 nRet = ww::stiUser;    // user style as default
    sal_uInt16 nPoolId = rFormat.GetPoolFormatId();
    if( nPoolId == RES_POOLCOLL_STANDARD )
        nRet = 0;
    else if( nPoolId >= RES_POOLCOLL_HEADLINE1 &&
             nPoolId <= RES_POOLCOLL_HEADLINE9 )
        nRet = static_cast< sal_uInt16 >(nPoolId + 1 - RES_POOLCOLL_HEADLINE1);
    else if( nPoolId >= RES_POOLCOLL_TOX_IDX1 &&
             nPoolId <= RES_POOLCOLL_TOX_IDX3 )
        nRet = static_cast< sal_uInt16 >(nPoolId + 10 - RES_POOLCOLL_TOX_IDX1);
    else if( nPoolId >= RES_POOLCOLL_TOX_CNTNT1 &&
             nPoolId <= RES_POOLCOLL_TOX_CNTNT5 )
        nRet = static_cast< sal_uInt16 >(nPoolId + 19 - RES_POOLCOLL_TOX_CNTNT1);
    else if( nPoolId >= RES_POOLCOLL_TOX_CNTNT6 &&
             nPoolId <= RES_POOLCOLL_TOX_CNTNT9 )
        nRet = static_cast< sal_uInt16 >(nPoolId + 24 - RES_POOLCOLL_TOX_CNTNT6);
    else
        switch( nPoolId )
        {
        case RES_POOLCOLL_FOOTNOTE:         nRet = 29;  break;
        case RES_POOLCOLL_MARGINAL:         nRet = 30;  break;
        case RES_POOLCOLL_HEADER:           nRet = 31;  break;
        case RES_POOLCOLL_FOOTER:           nRet = 32;  break;
        case RES_POOLCOLL_TOX_IDXH:         nRet = 33;  break;
        case RES_POOLCOLL_LABEL:            nRet = 34;  break;
        case RES_POOLCOLL_LABEL_DRAWING:    nRet = 35;  break;
        case RES_POOLCOLL_ENVELOPE_ADDRESS: nRet = 36;  break;
        case RES_POOLCOLL_SEND_ADDRESS:     nRet = 37;  break;
        case RES_POOLCOLL_ENDNOTE:          nRet = 43;  break;
        case RES_POOLCOLL_TOX_AUTHORITIESH: nRet = 44;  break;
        case RES_POOLCOLL_TOX_CNTNTH:       nRet = 46;  break;
        case RES_POOLCOLL_BULLET_LEVEL1:    nRet = 48;  break;
        case RES_POOLCOLL_LISTS_BEGIN:      nRet = 47;  break;
        case RES_POOLCOLL_NUM_LEVEL1:       nRet = 49;  break;
        case RES_POOLCOLL_BULLET_LEVEL2:    nRet = 54;  break;
        case RES_POOLCOLL_BULLET_LEVEL3:    nRet = 55;  break;
        case RES_POOLCOLL_BULLET_LEVEL4:    nRet = 56;  break;
        case RES_POOLCOLL_BULLET_LEVEL5:    nRet = 57;  break;
        case RES_POOLCOLL_NUM_LEVEL2:       nRet = 58;  break;
        case RES_POOLCOLL_NUM_LEVEL3:       nRet = 59;  break;
        case RES_POOLCOLL_NUM_LEVEL4:       nRet = 60;  break;
        case RES_POOLCOLL_NUM_LEVEL5:       nRet = 61;  break;
        case RES_POOLCOLL_DOC_TITLE:        nRet = 62;  break;
        case RES_POOLCOLL_DOC_APPENDIX:     nRet = 63;  break;
        case RES_POOLCOLL_SIGNATURE:        nRet = 64;  break;
        case RES_POOLCOLL_TEXT:             nRet = 66;  break;
        case RES_POOLCOLL_TEXT_MOVE:        nRet = 67;  break;
        case RES_POOLCOLL_BULLET_NONUM1:    nRet = 68;  break;
        case RES_POOLCOLL_BULLET_NONUM2:    nRet = 69;  break;
        case RES_POOLCOLL_BULLET_NONUM3:    nRet = 70;  break;
        case RES_POOLCOLL_BULLET_NONUM4:    nRet = 71;  break;
        case RES_POOLCOLL_BULLET_NONUM5:    nRet = 72;  break;
        case RES_POOLCOLL_DOC_SUBTITLE:     nRet = 74;  break;
        case RES_POOLCOLL_GREETING:         nRet = 75;  break;
        case RES_POOLCOLL_TEXT_IDENT:       nRet = 77;  break;

        case RES_POOLCHR_FOOTNOTE_ANCHOR:   nRet = 38;  break;
        case RES_POOLCHR_ENDNOTE_ANCHOR:    nRet = 42;  break;
        case RES_POOLCHR_INET_NORMAL:       nRet = 85;  break;
        case RES_POOLCHR_INET_VISIT:        nRet = 86;  break;
        case RES_POOLCHR_HTML_STRONG:       nRet = 87;  break;
        case RES_POOLCHR_HTML_EMPHASIS:     nRet = 88;  break;
        case RES_POOLCHR_LINENUM:           nRet = 40;  break;
        case RES_POOLCHR_PAGENO:            nRet = 41;  break;
        }
    return nRet;
}

void MSWordStyles::BuildStylesTable()
{
    m_nUsedSlots = WW8_RESERVED_SLOTS;  // reserved slots for standard, headingX, and others

    const SwCharFormats& rArr = *m_rExport.m_rDoc.GetCharFormats();       // first CharFormat
    // the default character style ( 0 ) will not be outputted !
    for( size_t n = 1; n < rArr.size(); n++ )
    {
        SwCharFormat* pFormat = rArr[n];
        m_pFormatA[ BuildGetSlot( *pFormat ) ] = pFormat;
    }

    const SwTextFormatColls& rArr2 = *m_rExport.m_rDoc.GetTextFormatColls();   // then TextFormatColls
    // the default character style ( 0 ) will not be outputted !
    for( size_t n = 1; n < rArr2.size(); n++ )
    {
        SwTextFormatColl* pFormat = rArr2[n];
        sal_uInt16 nId = BuildGetSlot( *pFormat ) ;
        m_pFormatA[ nId ] = pFormat;
        if ( pFormat->IsAssignedToListLevelOfOutlineStyle() )
        {
            int nLvl = pFormat->GetAssignedOutlineStyleLevel() ;
            if (nLvl >= 0 && nLvl < MAXLEVEL)
                m_aHeadingParagraphStyles[nLvl] = nId ;
        }
    }

    if (!m_bListStyles)
        return;

    const SwNumRuleTable& rNumRuleTable = m_rExport.m_rDoc.GetNumRuleTable();
    for (size_t i = 0; i < rNumRuleTable.size(); ++i)
    {
        const SwNumRule* pNumRule = rNumRuleTable[i];
        if (pNumRule->IsAutoRule() || pNumRule->GetName().startsWith("WWNum"))
            continue;
        sal_uInt16 nSlot = BuildGetSlot(*pNumRule);
        m_aNumRules[nSlot] = pNumRule;
    }
}

OString MSWordStyles::CreateStyleId(const OUString &rName)
{
    OStringBuffer aStyleIdBuf(rName.getLength());
    for (int i = 0; i < rName.getLength(); ++i)
    {
        sal_Unicode nChar = rName[i];
        if (('0' <= nChar && nChar <= '9') ||
            ('a' <= nChar && nChar <= 'z') ||
            ('A' <= nChar && nChar <= 'Z'))
        {
            // first letter should be uppercase
            if (aStyleIdBuf.isEmpty() && 'a' <= nChar && nChar <= 'z')
                aStyleIdBuf.append(char(nChar - ('a' - 'A')));
            else
                aStyleIdBuf.append(char(nChar));
        }
    }
    return aStyleIdBuf.makeStringAndClear();
}

void MSWordStyles::BuildStyleIds()
{
    std::unordered_set<OString> aUsed;

    m_aStyleIds.emplace_back("Normal");
    aUsed.insert("normal");

    for (sal_uInt16 n = 1; n < m_nUsedSlots; ++n)
    {
        OUString aName;
        if(m_pFormatA[n])
            aName = m_pFormatA[n]->GetName();
        else if (m_aNumRules.find(n) != m_aNumRules.end())
            aName = m_aNumRules[n]->GetName();

        OString aStyleId = CreateStyleId(aName);

        if (aStyleId.isEmpty())
            aStyleId = "Style";

        OString aLower(aStyleId.toAsciiLowerCase());

        // check for uniqueness & construct something unique if we have to
        if (aUsed.insert(aLower).second)
        {
            m_aStyleIds.push_back(aStyleId);
        }
        else
        {
            int nFree = 1;
            while (!aUsed.insert(aLower + OString::number(nFree)).second)
                ++nFree;

            m_aStyleIds.emplace_back(aStyleId + OString::number(nFree));
        }
    }
}

OString const & MSWordStyles::GetStyleId(sal_uInt16 nId) const
{
    return m_aStyleIds[nId];
}

/// For WW8 only - extend pO so that the size of pTableStrm is even.
static void impl_SkipOdd(std::unique_ptr<ww::bytes> const& pO, std::size_t nTableStrmTell)
{
    if ( ( nTableStrmTell + pO->size() ) & 1 )     // start on even
        pO->push_back( sal_uInt8(0) );         // Address
}

void WW8AttributeOutput::EndStyle()
{
    impl_SkipOdd( m_rWW8Export.pO, m_rWW8Export.pTableStrm->Tell() );

    short nLen = m_rWW8Export.pO->size() - 2;            // length of the style
    sal_uInt8* p = m_rWW8Export.pO->data() + nPOPosStdLen1;
    ShortToSVBT16( nLen, p );               // add
    p = m_rWW8Export.pO->data() + nPOPosStdLen2;
    ShortToSVBT16( nLen, p );               // also

    m_rWW8Export.pTableStrm->WriteBytes(m_rWW8Export.pO->data(), m_rWW8Export.pO->size());
    m_rWW8Export.pO->clear();
}

void WW8AttributeOutput::StartStyle( const OUString& rName, StyleType eType, sal_uInt16 nWwBase,
    sal_uInt16 nWwNext, sal_uInt16 /*nWwLink*/, sal_uInt16 nWwId, sal_uInt16 /*nId*/, bool bAutoUpdate )
{
    sal_uInt8 aWW8_STD[ sizeof( WW8_STD ) ] = {};
    sal_uInt8* pData = aWW8_STD;

    sal_uInt16 nBit16 = 0x1000;         // fInvalHeight
    nBit16 |= (ww::stiNil & nWwId);
    Set_UInt16( pData, nBit16 );

    nBit16 = nWwBase << 4;          // istdBase
    nBit16 |= (eType == STYLE_TYPE_PARA ? 1 : 2);      // sgc
    Set_UInt16( pData, nBit16 );

    nBit16 = nWwNext << 4;          // istdNext
    nBit16 |= (eType == STYLE_TYPE_PARA ? 2 : 1);      // cupx
    Set_UInt16( pData, nBit16 );

    pData += sizeof( sal_uInt16 );      // bchUpe

    nBit16 = bAutoUpdate ? 1 : 0;  // fAutoRedef : 1
    Set_UInt16( pData, nBit16 );
    // now new:
    // from Ver8 there are two fields more:
    // sal_uInt16    fHidden : 1;       /* hidden from UI?
    // sal_uInt16    : 14;              /* unused bits

    sal_uInt16 nLen = static_cast< sal_uInt16 >( ( pData - aWW8_STD ) + 1 +
                (2 * (rName.getLength() + 1)) );  // temporary

    nPOPosStdLen1 = m_rWW8Export.pO->size();        // Adr1 for adding the length

    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nLen );
    m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), aWW8_STD, pData );

    nPOPosStdLen2 = nPOPosStdLen1 + 8;  // Adr2 for adding of "end of upx"

    // write names
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, rName.getLength() ); // length
    SwWW8Writer::InsAsString16( *m_rWW8Export.pO, rName );
    m_rWW8Export.pO->push_back( sal_uInt8(0) );             // Despite P-String 0 at the end!
}

void MSWordStyles::SetStyleDefaults( const SwFormat& rFormat, bool bPap )
{
    const sw::BroadcastingModify* pOldMod = m_rExport.m_pOutFormatNode;
    m_rExport.m_pOutFormatNode = &rFormat;
    bool aFlags[ RES_FRMATR_END - RES_CHRATR_BEGIN ];
    sal_uInt16 nStt, nEnd, n;
    if( bPap )
    {
       nStt = RES_PARATR_BEGIN;
       nEnd = RES_FRMATR_END;
    }
    else
    {
       nStt = RES_CHRATR_BEGIN;
       nEnd = RES_TXTATR_END;
    }

    // dynamic defaults
    const SfxItemPool& rPool = *rFormat.GetAttrSet().GetPool();
    for( n = nStt; n < nEnd; ++n )
        aFlags[ n - RES_CHRATR_BEGIN ] = nullptr != rPool.GetPoolDefaultItem( n )
            || SfxItemState::SET == m_rExport.m_rDoc.GetDfltTextFormatColl()->GetItemState( n, false );

    // static defaults, that differs between WinWord and SO
    if( bPap )
    {
        aFlags[ static_cast< sal_uInt16 >(RES_PARATR_WIDOWS) - RES_CHRATR_BEGIN ] = true;
        aFlags[ static_cast< sal_uInt16 >(RES_PARATR_HYPHENZONE) - RES_CHRATR_BEGIN ] = true;
        aFlags[ static_cast< sal_uInt16 >(RES_FRAMEDIR) - RES_CHRATR_BEGIN ] = true;
    }
    else
    {
        aFlags[ RES_CHRATR_FONTSIZE - RES_CHRATR_BEGIN ] = true;
        aFlags[ RES_CHRATR_LANGUAGE - RES_CHRATR_BEGIN ] = true;
    }

    const SfxItemSet* pOldI = m_rExport.GetCurItemSet();
    m_rExport.SetCurItemSet( &rFormat.GetAttrSet() );

    const bool* pFlags = aFlags + ( nStt - RES_CHRATR_BEGIN );
    for ( n = nStt; n < nEnd; ++n, ++pFlags )
    {
        if ( *pFlags && !m_rExport.ignoreAttributeForStyleDefaults( n )
            && SfxItemState::SET != rFormat.GetItemState(n, false))
        {
            //If we are a character property then see if it is one of the
            //western/asian ones that must be collapsed together for export to
            //word. If so default to the western variant.
            if ( bPap || m_rExport.CollapseScriptsforWordOk(
                i18n::ScriptType::LATIN, n) )
            {
                m_rExport.AttrOutput().OutputItem( rFormat.GetFormatAttr( n ) );
            }
        }
    }

    m_rExport.SetCurItemSet( pOldI );
    m_rExport.m_pOutFormatNode = pOldMod;
}

void WW8AttributeOutput::StartStyleProperties( bool bParProp, sal_uInt16 nStyle )
{
    impl_SkipOdd( m_rWW8Export.pO, m_rWW8Export.pTableStrm->Tell() );

    sal_uInt16 nLen = bParProp ? 2 : 0;         // default length
    m_nStyleLenPos = m_rWW8Export.pO->size();   // adding length
                                                // Don't save pointer, because it
                                                // changes by _grow!

    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nLen );        // Style-Len

    m_nStyleStartSize = m_rWW8Export.pO->size();

    if ( bParProp )
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nStyle );     // Style-Number
}

void MSWordStyles::WriteProperties( const SwFormat* pFormat, bool bParProp, sal_uInt16 nPos,
    bool bInsDefCharSiz )
{
    m_rExport.AttrOutput().StartStyleProperties( bParProp, nPos );

    OSL_ENSURE( m_rExport.m_pCurrentStyle == nullptr, "Current style not NULL" ); // set current style before calling out
    m_rExport.m_pCurrentStyle = pFormat;

    m_rExport.OutputFormat( *pFormat, bParProp, !bParProp );

    OSL_ENSURE( m_rExport.m_pCurrentStyle == pFormat, "current style was changed" );
    // reset current style...
    m_rExport.m_pCurrentStyle = nullptr;

    if ( bInsDefCharSiz  )                   // not derived from other Style
        SetStyleDefaults( *pFormat, bParProp );

    m_rExport.AttrOutput().EndStyleProperties( bParProp );
}

void WW8AttributeOutput::EndStyleProperties( bool /*bParProp*/ )
{
    sal_uInt16 nLen = m_rWW8Export.pO->size() - m_nStyleStartSize;
    sal_uInt8* pUpxLen = m_rWW8Export.pO->data() + m_nStyleLenPos; // adding length
    ShortToSVBT16( nLen, pUpxLen );                 // add default length
}

void MSWordStyles::GetStyleData( SwFormat* pFormat, bool& bFormatColl, sal_uInt16& nBase, sal_uInt16& nNext, sal_uInt16& nLink )
{
    bFormatColl = pFormat->Which() == RES_TXTFMTCOLL || pFormat->Which() == RES_CONDTXTFMTCOLL;

    // Default: none
    nBase = 0xfff;

    // Derived from?
    if ( !pFormat->IsDefault() )
        nBase = GetSlot( pFormat->DerivedFrom() );

    SwFormat* pNext;
    const SwFormat* pLink = nullptr;
    if ( bFormatColl )
    {
        auto pFormatColl = static_cast<SwTextFormatColl*>(pFormat);
        pNext = &pFormatColl->GetNextTextFormatColl();
        pLink = pFormatColl->GetLinkedCharFormat();
    }
    else
    {
        pNext = pFormat; // CharFormat: next CharFormat == self
        auto pCharFormat = static_cast<SwCharFormat*>(pFormat);
        pLink = pCharFormat->GetLinkedParaFormat();
    }

    nNext = GetSlot( pNext );

    if (pLink)
    {
        nLink = GetSlot(pLink);
    }
}

void WW8AttributeOutput::DefaultStyle()
{
    m_rWW8Export.pTableStrm->WriteUInt16(0);   // empty Style
}

void MSWordStyles::OutputStyle(const SwNumRule* pNumRule, sal_uInt16 nPos)
{
    m_rExport.AttrOutput().StartStyle( pNumRule->GetName(), STYLE_TYPE_LIST,
            /*nBase =*/ 0, /*nWwNext =*/ 0, /*nWwLink =*/ 0, /*nWWId =*/ 0, nPos,
            /*bAutoUpdateFormat =*/ false );

    m_rExport.AttrOutput().EndStyle();
}

// OutputStyle applies for TextFormatColls and CharFormats
void MSWordStyles::OutputStyle( SwFormat* pFormat, sal_uInt16 nPos )
{
    if ( !pFormat )
        m_rExport.AttrOutput().DefaultStyle();
    else
    {
        bool bFormatColl;
        sal_uInt16 nBase, nWwNext;
        sal_uInt16 nWwLink = 0x0FFF;

        GetStyleData(pFormat, bFormatColl, nBase, nWwNext, nWwLink);

        OUString aName = pFormat->GetName();
        // We want to map LO's default style to Word's "Normal" style.
        // Word looks for this specific style name when reading docx files.
        // (It must be the English word regardless of language settings)
        if ( nPos == 0 )
        {
            assert( pFormat->GetPoolFormatId() == RES_POOLCOLL_STANDARD );
            aName = "Normal";
        }
        else if (aName.equalsIgnoreAsciiCase("Normal"))
        {
            // If LO has a style named "Normal"(!) rename it to something unique
            const OUString aBaseName = "LO-" + aName;
            aName = aBaseName;
            // Check if we still have a clash, in which case we add a suffix
            for ( int nSuffix = 0; ; ++nSuffix ) {
                bool clash=false;
                for ( sal_uInt16 n = 1; n < m_nUsedSlots; ++n )
                    if ( m_pFormatA[n] &&
                         m_pFormatA[n]->GetName().equalsIgnoreAsciiCase(aName) )
                    {
                        clash = true;
                        break;
                    }
                if (!clash)
                    break;
                // TODO: verify if we really need to increment nSuffix in 2 places
                aName = aBaseName + OUString::number(++nSuffix);
            }
        }
        else if (!bFormatColl && m_rExport.GetExportFormat() == MSWordExportBase::DOCX &&
                        m_rExport.m_pStyles->GetStyleId(nPos).startsWith("ListLabel"))
        {
            // tdf#92335 don't export redundant DOCX import style "ListLabel"
            return;
        }
        else if (aName.equalsIgnoreAsciiCase("Internet Link"))
        {
            aName = "Hyperlink";
        }
        else if (aName.equalsIgnoreAsciiCase("Visited Internet Link"))
        {
            aName = "FollowedHyperlink";
        }

        m_rExport.AttrOutput().StartStyle( aName, (bFormatColl ? STYLE_TYPE_PARA : STYLE_TYPE_CHAR),
                nBase, nWwNext, nWwLink, GetWWId( *pFormat ), nPos,
                pFormat->IsAutoUpdateFormat() );

        if ( bFormatColl )
            WriteProperties( pFormat, true, nPos, nBase==0xfff );           // UPX.papx

        WriteProperties( pFormat, false, nPos, bFormatColl && nBase==0xfff );  // UPX.chpx

        m_rExport.AttrOutput().EndStyle();
    }
}

void WW8AttributeOutput::StartStyles()
{
    WW8Fib& rFib = *m_rWW8Export.pFib;

    sal_uInt64 nCurPos = m_rWW8Export.pTableStrm->Tell();
    if ( nCurPos & 1 )                   // start on even
    {
        m_rWW8Export.pTableStrm->WriteChar( char(0) );        // Address
        ++nCurPos;
    }
    rFib.m_fcStshfOrig = rFib.m_fcStshf = nCurPos;
    m_nStyleCountPos = nCurPos + 2;     // count is added later

    static sal_uInt8 aStShi[] = {
        0x12, 0x00,
        0x0F, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x5B, 0x00,
        0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00 };

    m_rWW8Export.pTableStrm->WriteBytes(&aStShi, sizeof(aStShi));
}

void WW8AttributeOutput::EndStyles( sal_uInt16 nNumberOfStyles )
{
    WW8Fib& rFib = *m_rWW8Export.pFib;

    rFib.m_lcbStshfOrig = rFib.m_lcbStshf = m_rWW8Export.pTableStrm->Tell() - rFib.m_fcStshf;
    SwWW8Writer::WriteShort( *m_rWW8Export.pTableStrm, m_nStyleCountPos, nNumberOfStyles );
}

void MSWordStyles::OutputStylesTable()
{
    m_rExport.m_bStyDef = true;

    m_rExport.AttrOutput().StartStyles();

    sal_uInt16 n;
    // HACK
    // Ms Office seems to have an internal limitation of 4091 styles
    // and refuses to load .docx with more, even though the spec seems to allow that;
    // so simply if there are more styles, don't export those
    // Implementing check for all exports DOCX, DOC, RTF
    sal_uInt16 const nLimit = MSWORD_MAX_STYLES_LIMIT;
    m_nUsedSlots = std::min(nLimit, m_nUsedSlots);

    for ( n = 0; n < m_nUsedSlots; n++ )
    {
        if (m_aNumRules.find(n) != m_aNumRules.end())
            OutputStyle(m_aNumRules[n], n);
        else
            OutputStyle( m_pFormatA[n], n );
    }

    m_rExport.AttrOutput().EndStyles( m_nUsedSlots );

    m_rExport.m_bStyDef = false;
}

const SwNumRule* MSWordStyles::GetSwNumRule(sal_uInt16 nId) const
{
    std::map<sal_uInt16, const SwNumRule*>::const_iterator it = m_aNumRules.find(nId);
    assert(it != m_aNumRules.end());
    return it->second;
}

//          Fonts

wwFont::wwFont(const OUString &rFamilyName, FontPitch ePitch, FontFamily eFamily,
        rtl_TextEncoding eChrSet)
    : mbAlt(false), mePitch(ePitch), meFamily(eFamily), meChrSet(eChrSet)
{
    FontMapExport aResult(rFamilyName);
    msFamilyNm = aResult.msPrimary;
    msAltNm = aResult.msSecondary;
    if (!msAltNm.isEmpty() && msAltNm != msFamilyNm &&
        (msFamilyNm.getLength() + msAltNm.getLength() + 2 <= 65) )
    {
        //max size of szFfn in 65 chars
        mbAlt = true;
    }

    maWW8_FFN[0] = static_cast<sal_uInt8>( 6 - 1 + 0x22 + ( 2 * ( 1 + msFamilyNm.getLength() ) ));
    if (mbAlt)
        maWW8_FFN[0] = static_cast< sal_uInt8 >(maWW8_FFN[0] + 2 * ( 1 + msAltNm.getLength()));

    sal_uInt8 aB = 0;
    switch(ePitch)
    {
        case PITCH_VARIABLE:
            aB |= 2;    // aF.prg = 2
            break;
        case PITCH_FIXED:
            aB |= 1;
            break;
        default:        // aF.prg = 0 : DEFAULT_PITCH (windows.h)
            break;
    }
    aB |= 1 << 2;   // aF.fTrueType = 1; don't know any better;

    switch(eFamily)
    {
        case FAMILY_ROMAN:
            aB |= 1 << 4;   // aF.ff = 1;
            break;
        case FAMILY_SWISS:
            aB |= 2 << 4;   // aF.ff = 2;
            break;
        case FAMILY_MODERN:
            aB |= 3 << 4;   // aF.ff = 3;
            break;
        case FAMILY_SCRIPT:
            aB |= 4 << 4;   // aF.ff = 4;
            break;
        case FAMILY_DECORATIVE:
            aB |= 5 << 4;   // aF.ff = 5;
            break;
        default:            // aF.ff = 0; FF_DONTCARE (windows.h)
            break;
    }
    maWW8_FFN[1] = aB;

    ShortToSVBT16( 400, &maWW8_FFN[2] );        // don't know any better
                                                // 400 == FW_NORMAL (windows.h)

    //#i61927# For unicode fonts like Arial Unicode, Word 97+ sets the chs
    //to SHIFTJIS presumably to capture that it's a multi-byte encoding font
    //but Word95 doesn't do this, and sets it to 0 (ANSI), so we should do the
    //same
    maWW8_FFN[4] = sw::ms::rtl_TextEncodingToWinCharset(eChrSet);

    if (mbAlt)
        maWW8_FFN[5] = static_cast< sal_uInt8 >(msFamilyNm.getLength() + 1);
}

void wwFont::Write(SvStream *pTableStrm) const
{
    pTableStrm->WriteBytes(maWW8_FFN, sizeof(maWW8_FFN));    // fixed part
    // from Ver8 following two fields intersected,
    // we ignore them.
    //char  panose[ 10 ];       //  0x6   PANOSE
    //char  fs[ 24     ];       //  0x10  FONTSIGNATURE
    SwWW8Writer::FillCount(*pTableStrm, 0x22);
    SwWW8Writer::WriteString16(*pTableStrm, msFamilyNm, true);
    if (mbAlt)
        SwWW8Writer::WriteString16(*pTableStrm, msAltNm, true);
}

void wwFont::WriteDocx( DocxAttributeOutput* rAttrOutput ) const
{
    // no font embedding, panose id, subsetting, ... implemented

    if (msFamilyNm.isEmpty())
        return;

    rAttrOutput->StartFont( msFamilyNm );

    if ( mbAlt )
        rAttrOutput->FontAlternateName( msAltNm );
    rAttrOutput->FontCharset( sw::ms::rtl_TextEncodingToWinCharset( meChrSet ), meChrSet );
    rAttrOutput->FontFamilyType( meFamily );
    rAttrOutput->FontPitchType( mePitch );
    rAttrOutput->EmbedFont( msFamilyNm, meFamily, mePitch );

    rAttrOutput->EndFont();
}

void wwFont::WriteRtf( const RtfAttributeOutput* rAttrOutput ) const
{
    rAttrOutput->FontFamilyType( meFamily, *this );
    rAttrOutput->FontPitchType( mePitch );
    rAttrOutput->FontCharset(
        sw::ms::rtl_TextEncodingToWinCharsetRTF(msFamilyNm, msAltNm, meChrSet));
    rAttrOutput->StartFont( msFamilyNm );
    if ( mbAlt )
        rAttrOutput->FontAlternateName( msAltNm );
    rAttrOutput->EndFont();
}

bool operator<(const wwFont &r1, const wwFont &r2)
{
    int nRet = memcmp(r1.maWW8_FFN, r2.maWW8_FFN, sizeof(r1.maWW8_FFN));
    if (nRet == 0)
    {
        nRet = r1.msFamilyNm.compareTo(r2.msFamilyNm);
        if (nRet == 0)
            nRet = r1.msAltNm.compareTo(r2.msAltNm);
    }
    return nRet < 0;
}

sal_uInt16 wwFontHelper::GetId(const wwFont &rFont)
{
    sal_uInt16 nRet;
    std::map<wwFont, sal_uInt16>::const_iterator aIter = maFonts.find(rFont);
    if (aIter != maFonts.end())
        nRet = aIter->second;
    else
    {
        nRet = static_cast< sal_uInt16 >(maFonts.size());
        maFonts[rFont] = nRet;
    }
    return nRet;
}

void wwFontHelper::InitFontTable(const SwDoc& rDoc)
{
    GetId(wwFont("Times New Roman", PITCH_VARIABLE,
        FAMILY_ROMAN, RTL_TEXTENCODING_MS_1252));

    GetId(wwFont("Symbol", PITCH_VARIABLE, FAMILY_ROMAN,
        RTL_TEXTENCODING_SYMBOL));

    GetId(wwFont("Arial", PITCH_VARIABLE, FAMILY_SWISS,
        RTL_TEXTENCODING_MS_1252));

    const SvxFontItem* pFont = GetDfltAttr(RES_CHRATR_FONT);

    GetId(wwFont(pFont->GetFamilyName(), pFont->GetPitch(),
        pFont->GetFamily(), pFont->GetCharSet()));

    const SfxItemPool& rPool = rDoc.GetAttrPool();
    pFont = rPool.GetPoolDefaultItem(RES_CHRATR_FONT);
    if (nullptr != pFont)
    {
        GetId(wwFont(pFont->GetFamilyName(), pFont->GetPitch(),
            pFont->GetFamily(), pFont->GetCharSet()));
    }

    if (!bLoadAllFonts)
        return;

    const sal_uInt16 aTypes[] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_FONT, 0 };
    for (const sal_uInt16* pId = aTypes; *pId; ++pId)
    {
        for (const SfxPoolItem* pItem : rPool.GetItemSurrogates(*pId))
        {
            pFont = static_cast<const SvxFontItem*>(pItem);
            GetId(wwFont(pFont->GetFamilyName(), pFont->GetPitch(),
                         pFont->GetFamily(), pFont->GetCharSet()));
        }
    }
}

sal_uInt16 wwFontHelper::GetId(const SvxFontItem& rFont)
{
    wwFont aFont(rFont.GetFamilyName(), rFont.GetPitch(), rFont.GetFamily(),
        rFont.GetCharSet());
    return GetId(aFont);
}

std::vector< const wwFont* > wwFontHelper::AsVector() const
{
    std::vector<const wwFont *> aFontList( maFonts.size() );

    for ( const auto& aFont : maFonts )
        aFontList[aFont.second] = &aFont.first;

    return aFontList;
}

void wwFontHelper::WriteFontTable(SvStream *pTableStream, WW8Fib& rFib)
{
    rFib.m_fcSttbfffn = pTableStream->Tell();
    /*
     * Reserve some space to fill in the len after we know how big it is
     */
    SwWW8Writer::WriteLong(*pTableStream, 0);

    /*
     * Convert from fast insertion map to linear vector in the order that we
     * want to write.
     */
    std::vector<const wwFont *> aFontList( AsVector() );

    /*
     * Write them all to pTableStream
     */
    for ( auto aFont : aFontList )
        aFont->Write(pTableStream);

    /*
     * Write the position and len in the FIB
     */
    rFib.m_lcbSttbfffn = pTableStream->Tell() - rFib.m_fcSttbfffn;
    SwWW8Writer::WriteLong( *pTableStream, rFib.m_fcSttbfffn, maFonts.size());
}

void wwFontHelper::WriteFontTable( DocxAttributeOutput& rAttrOutput )
{
    std::vector<const wwFont *> aFontList( AsVector() );

    for ( auto aFont : aFontList )
        aFont->WriteDocx(&rAttrOutput);
}

void wwFontHelper::WriteFontTable( const RtfAttributeOutput& rAttrOutput )
{
    std::vector<const wwFont *> aFontList( AsVector() );

    for ( auto aFont : aFontList )
        aFont->WriteRtf(&rAttrOutput);
}

WW8_WrPlc0::WW8_WrPlc0( sal_uLong nOffset )
    : nOfs( nOffset )
{
}

void WW8_WrPlc0::Append( sal_uLong nStartCpOrFc )
{
    aPos.push_back( nStartCpOrFc - nOfs );
}

void WW8_WrPlc0::Write( SvStream& rStrm )
{
    for( const auto& rPos : aPos )
    {
        rStrm.WriteUInt32(rPos);
    }
}

// class MSWordSections : translate PageDescs into Sections
//      also deals with header and footer

MSWordSections::MSWordSections( MSWordExportBase& rExport )
    : mbDocumentIsProtected( false )
{
    const SwSectionFormat *pFormat = nullptr;
    rExport.m_pCurrentPageDesc = &rExport.m_rDoc.GetPageDesc( 0 );

    const SfxPoolItem* pI;
    const SwNode* pNd = rExport.m_pCurPam->GetContentNode();
    const SfxItemSet* pSet = pNd ? &static_cast<const SwContentNode*>(pNd)->GetSwAttrSet() : nullptr;

    sal_uLong nRstLnNum =  pSet ? pSet->Get( RES_LINENUMBER ).GetStartValue() : 0;

    const SwTableNode* pTableNd = rExport.m_pCurPam->GetNode().FindTableNode();
    const SwSectionNode* pSectNd = nullptr;
    if ( pTableNd )
    {
        pSet = &pTableNd->GetTable().GetFrameFormat()->GetAttrSet();
        pNd = pTableNd;
    }
    else if (pNd && nullptr != ( pSectNd = pNd->FindSectionNode() ))
    {
        if ( SectionType::ToxHeader == pSectNd->GetSection().GetType() &&
             pSectNd->StartOfSectionNode()->IsSectionNode() )
        {
            pSectNd = pSectNd->StartOfSectionNode()->GetSectionNode();
        }

        if ( SectionType::ToxContent == pSectNd->GetSection().GetType() )
        {
            pNd = pSectNd;
            rExport.m_pCurPam->GetPoint()->nNode = *pNd;
        }

        if ( SectionType::Content == pSectNd->GetSection().GetType() )
            pFormat = pSectNd->GetSection().GetFormat();
    }

    // tdf#118393: FILESAVE: DOCX Export loses header/footer
    rExport.m_bFirstTOCNodeWithSection = pSectNd &&
        (   SectionType::ToxHeader  == pSectNd->GetSection().GetType() ||
            SectionType::ToxContent == pSectNd->GetSection().GetType()  );

    // Try to get page descriptor of the first node
    if ( pSet &&
         SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC, true, &pI ) &&
         static_cast<const SwFormatPageDesc*>(pI)->GetPageDesc() )
    {
        AppendSection( *static_cast<const SwFormatPageDesc*>(pI), *pNd, pFormat, nRstLnNum );
    }
    else
        AppendSection( rExport.m_pCurrentPageDesc, pFormat, nRstLnNum, /*bIsFirstParagraph=*/true );
}

WW8_WrPlcSepx::WW8_WrPlcSepx( MSWordExportBase& rExport )
    : MSWordSections( rExport )
    , m_bHeaderFooterWritten( false )
{
    // to be in sync with the AppendSection() call in the MSWordSections
    // constructor
    aCps.push_back( 0 );
}

MSWordSections::~MSWordSections()
{
}

WW8_WrPlcSepx::~WW8_WrPlcSepx()
{
}

bool MSWordSections::HeaderFooterWritten()
{
    return false; // only relevant for WW8
}

bool WW8_WrPlcSepx::HeaderFooterWritten()
{
    return m_bHeaderFooterWritten;
}

sal_uInt16 MSWordSections::CurrentNumberOfColumns( const SwDoc &rDoc ) const
{
    OSL_ENSURE( !aSects.empty(), "no segment inserted yet" );
    if ( aSects.empty() )
        return 1;

    return NumberOfColumns( rDoc, aSects.back() );
}

sal_uInt16 MSWordSections::NumberOfColumns( const SwDoc &rDoc, const WW8_SepInfo& rInfo )
{
    const SwPageDesc* pPd = rInfo.pPageDesc;
    if ( !pPd )
        pPd = &rDoc.GetPageDesc( 0 );

    const SfxItemSet &rSet = pPd->GetMaster().GetAttrSet();
    SfxItemSetFixed<RES_COL, RES_COL> aSet( *rSet.GetPool() );
    aSet.SetParent( &rSet );

    //0xffffffff, what the hell is going on with that!, fixme most terribly
    if ( rInfo.pSectionFormat && reinterpret_cast<SwSectionFormat*>(sal_IntPtr(-1)) != rInfo.pSectionFormat )
        aSet.Put( rInfo.pSectionFormat->GetFormatAttr( RES_COL ) );

    const SwFormatCol& rCol = aSet.Get( RES_COL );
    const SwColumns& rColumns = rCol.GetColumns();
    return rColumns.size();
}

const WW8_SepInfo* MSWordSections::CurrentSectionInfo()
{
    if ( !aSects.empty() )
        return &aSects.back();

    return nullptr;
}

void MSWordSections::AppendSection( const SwPageDesc* pPd,
    const SwSectionFormat* pSectionFormat, sal_uLong nLnNumRestartNo, bool bIsFirstParagraph )
{
    if (HeaderFooterWritten()) {
        return; // #i117955# prevent new sections in endnotes
    }
    aSects.emplace_back( pPd, pSectionFormat, nLnNumRestartNo, std::nullopt, nullptr, bIsFirstParagraph );
    NeedsDocumentProtected( aSects.back() );
}

void WW8_WrPlcSepx::AppendSep( WW8_CP nStartCp, const SwPageDesc* pPd,
    const SwSectionFormat* pSectionFormat, sal_uLong nLnNumRestartNo )
{
    if (HeaderFooterWritten()) {
        return; // #i117955# prevent new sections in endnotes
    }
    aCps.push_back( nStartCp );
    AppendSection( pPd, pSectionFormat, nLnNumRestartNo );
}

void MSWordSections::AppendSection( const SwFormatPageDesc& rPD,
    const SwNode& rNd, const SwSectionFormat* pSectionFormat, sal_uLong nLnNumRestartNo )
{
    if (HeaderFooterWritten()) {
        return; // #i117955# prevent new sections in endnotes
    }

    WW8_SepInfo aI( rPD.GetPageDesc(), pSectionFormat, nLnNumRestartNo, rPD.GetNumOffset(), &rNd );

    aSects.push_back( aI );
    NeedsDocumentProtected( aI );
}

void WW8_WrPlcSepx::AppendSep( WW8_CP nStartCp, const SwFormatPageDesc& rPD,
    const SwNode& rNd, const SwSectionFormat* pSectionFormat, sal_uLong nLnNumRestartNo )
{
    if (HeaderFooterWritten()) {
        return; // #i117955# prevent new sections in endnotes
    }
    aCps.push_back( nStartCp );
    AppendSection( rPD, rNd, pSectionFormat, nLnNumRestartNo );
}

void WW8_WrPlcSepx::WriteFootnoteEndText( WW8Export& rWrt, sal_uLong nCpStt )
{
    sal_uInt8 nInfoFlags = 0;
    const SwFootnoteInfo& rInfo = rWrt.m_rDoc.GetFootnoteInfo();
    if( !rInfo.m_aErgoSum.isEmpty() )  nInfoFlags |= 0x02;
    if( !rInfo.m_aQuoVadis.isEmpty() ) nInfoFlags |= 0x04;

    sal_uInt8 nEmptyStt = 0;
    if( nInfoFlags )
    {
        pTextPos->Append( nCpStt );  // empty footnote separator

        if( 0x02 & nInfoFlags )         // Footnote continuation separator
        {
            pTextPos->Append( nCpStt );
            rWrt.WriteStringAsPara( rInfo.m_aErgoSum );
            rWrt.WriteStringAsPara( OUString() );
            nCpStt = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        }
        else
            pTextPos->Append( nCpStt );

        if( 0x04 & nInfoFlags )         // Footnote continuation notice
        {
            pTextPos->Append( nCpStt );
            rWrt.WriteStringAsPara( rInfo.m_aQuoVadis );
            rWrt.WriteStringAsPara( OUString() );
            nCpStt = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        }
        else
            pTextPos->Append( nCpStt );

        nEmptyStt = 3;
    }

    while( 6 > nEmptyStt++ )
        pTextPos->Append( nCpStt );

    // set the flags at the Dop right away
    WW8Dop& rDop = *rWrt.pDop;
    // Footnote Info
    switch( rInfo.m_eNum )
    {
    case FTNNUM_PAGE:       rDop.rncFootnote = 2; break;
    case FTNNUM_CHAPTER:    rDop.rncFootnote  = 1; break;
    default: rDop.rncFootnote  = 0; break;
    }                                   // rncFootnote
    rDop.nfcFootnoteRef = WW8Export::GetNumId( rInfo.m_aFormat.GetNumberingType() );
    rDop.nFootnote = rInfo.m_nFootnoteOffset + 1;
    rDop.fpc = rWrt.m_bFootnoteAtTextEnd ? 2 : 1;

    // Endnote Info
    rDop.rncEdn = 0;                        // rncEdn: Don't Restart
    const SwEndNoteInfo& rEndInfo = rWrt.m_rDoc.GetEndNoteInfo();
    rDop.nfcEdnRef = WW8Export::GetNumId( rEndInfo.m_aFormat.GetNumberingType() );
    rDop.nEdn = rEndInfo.m_nFootnoteOffset + 1;
    rDop.epc = rWrt.m_bEndAtTextEnd ? 3 : 0;
}

void MSWordSections::SetHeaderFlag( sal_uInt8& rHeadFootFlags, const SwFormat& rFormat,
    sal_uInt8 nFlag )
{
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rFormat.GetItemState(RES_HEADER, true, &pItem)
        && static_cast<const SwFormatHeader*>(pItem)->IsActive() &&
        static_cast<const SwFormatHeader*>(pItem)->GetHeaderFormat() )
        rHeadFootFlags |= nFlag;
}

void MSWordSections::SetFooterFlag( sal_uInt8& rHeadFootFlags, const SwFormat& rFormat,
    sal_uInt8 nFlag )
{
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rFormat.GetItemState(RES_FOOTER, true, &pItem)
        && static_cast<const SwFormatFooter*>(pItem)->IsActive() &&
        static_cast<const SwFormatFooter*>(pItem)->GetFooterFormat() )
        rHeadFootFlags |= nFlag;
}

void WW8_WrPlcSepx::OutHeaderFooter( WW8Export& rWrt, bool bHeader,
                     const SwFormat& rFormat, sal_uLong& rCpPos, sal_uInt8 nHFFlags,
                     sal_uInt8 nFlag,  sal_uInt8 nBreakCode)
{
    if ( nFlag & nHFFlags )
    {
        pTextPos->Append( rCpPos );
        rWrt.WriteHeaderFooterText( rFormat, bHeader);
        rWrt.WriteStringAsPara( OUString() ); // CR to the end ( otherwise WW complains )
        rCpPos = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    }
    else
    {
        pTextPos->Append( rCpPos );
        if ((bHeader? rWrt.m_bHasHdr : rWrt.m_bHasFtr) && nBreakCode!=0)
        {
            rWrt.WriteStringAsPara( OUString() ); // Empty paragraph for empty header/footer
            rWrt.WriteStringAsPara( OUString() ); // a CR that WW8 needs for end of the stream
            rCpPos = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        }
    }
}

void MSWordSections::NeedsDocumentProtected(const WW8_SepInfo &rInfo)
{
    if (rInfo.IsProtected())
        mbDocumentIsProtected = true;
}

bool WW8_SepInfo::IsProtected() const
{
    bool bRet = false;
    if (
         pSectionFormat &&
         (reinterpret_cast<SwSectionFormat*>(sal_IntPtr(-1)) != pSectionFormat)
       )
    {
        const SwSection *pSection = pSectionFormat->GetSection();
        if (pSection && pSection->IsProtect())
        {
            bRet = true;
        }
    }
    return bRet;
}

void MSWordSections::CheckForFacinPg( const WW8Export& rWrt ) const
{
    // 2 values getting set
    //      Dop.fFacingPages            == Header and Footer different
    //      Dop.fSwapBordersFacingPgs   == mirrored borders
    sal_uInt16 nEnd = 0;
    for( const WW8_SepInfo& rSepInfo : aSects )
    {
        if( !rSepInfo.pSectionFormat )
        {
            const SwPageDesc* pPd = rSepInfo.pPageDesc;
            if( pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd->GetFollow() &&
                rSepInfo.pPDNd &&
                pPd->IsFollowNextPageOfNode( *rSepInfo.pPDNd ) )
                // so this is first page and subsequent, so only respect follow
                pPd = pPd->GetFollow();

            // left-/right chain of pagedescs ?
            else if( !( 1 & nEnd ) &&
                pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd &&
                (( UseOnPage::Left == ( UseOnPage::All & pPd->ReadUseOn() ) &&
                   UseOnPage::Right == ( UseOnPage::All & pPd->GetFollow()->ReadUseOn() )) ||
                 ( UseOnPage::Right == ( UseOnPage::All & pPd->ReadUseOn() ) &&
                   UseOnPage::Left == ( UseOnPage::All & pPd->GetFollow()->ReadUseOn() )) ))
            {
                rWrt.pDop->fFacingPages = rWrt.pDop->fMirrorMargins = true;
                nEnd |= 1;
            }

            if( !( 1 & nEnd ) &&
                ( !pPd->IsHeaderShared() || !pPd->IsFooterShared() ))
            {
                rWrt.pDop->fFacingPages = true;
                nEnd |= 1;
            }
            if( !( 2 & nEnd ) &&
                UseOnPage::Mirror == ( UseOnPage::Mirror & pPd->ReadUseOn() ))
            {
                rWrt.pDop->fSwapBordersFacingPgs =
                    rWrt.pDop->fMirrorMargins = true;
                nEnd |= 2;
            }

            if( 3 == nEnd )
                break;      // We do not need to go any further
        }
    }
}

bool MSWordSections::HasBorderItem( const SwFormat& rFormat )
{
    const SfxPoolItem* pItem;
    return SfxItemState::SET == rFormat.GetItemState(RES_BOX, true, &pItem) &&
            (   static_cast<const SvxBoxItem*>(pItem)->GetTop() ||
                static_cast<const SvxBoxItem*>(pItem)->GetBottom()  ||
                static_cast<const SvxBoxItem*>(pItem)->GetLeft()  ||
                static_cast<const SvxBoxItem*>(pItem)->GetRight() );
}

void WW8AttributeOutput::StartSection()
{
    m_rWW8Export.pO->clear();
}

void WW8AttributeOutput::SectFootnoteEndnotePr()
{
    const SwFootnoteInfo& rInfo = m_rWW8Export.m_rDoc.GetFootnoteInfo();
    const SwEndNoteInfo& rEndNoteInfo = m_rWW8Export.m_rDoc.GetEndNoteInfo();
    m_rWW8Export.InsUInt16( NS_sprm::SRncFtn::val );
    switch( rInfo.m_eNum )
    {
    case FTNNUM_PAGE:     m_rWW8Export.pO->push_back( sal_uInt8/*rncRstPage*/ (2) ); break;
    case FTNNUM_CHAPTER:  m_rWW8Export.pO->push_back( sal_uInt8/*rncRstSect*/ (1) ); break;
    default:              m_rWW8Export.pO->push_back( sal_uInt8/*rncCont*/ (0) ); break;
    }

    m_rWW8Export.InsUInt16(NS_sprm::SNfcFtnRef::val);
    sal_uInt8 nId = WW8Export::GetNumId(rInfo.m_aFormat.GetNumberingType());
    SwWW8Writer::InsUInt16(*m_rWW8Export.pO, nId);
    m_rWW8Export.InsUInt16(NS_sprm::SNfcEdnRef::val);
    nId = WW8Export::GetNumId(rEndNoteInfo.m_aFormat.GetNumberingType());
    SwWW8Writer::InsUInt16(*m_rWW8Export.pO, nId);
}

void WW8AttributeOutput::SectionFormProtection( bool bProtected )
{
    //If the document is to be exported as protected, then if a segment
    //is not protected, set the unlocked flag
    if ( m_rWW8Export.pSepx->DocumentIsProtected() && !bProtected )
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SFProtected::val );
        m_rWW8Export.pO->push_back( 1 );
    }
}

void WW8AttributeOutput::SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo )
{
    // sprmSNLnnMod - activate Line Numbering and define Modulo
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SNLnnMod::val );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, rLnNumInfo.GetCountBy() );

    // sprmSDxaLnn - xPosition of Line Number
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SDxaLnn::val );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, rLnNumInfo.GetPosFromLeft() );

    // sprmSLnc - restart number: 0 per page, 1 per section, 2 never restart
    if ( nRestartNo || !rLnNumInfo.IsRestartEachPage() )
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SLnc::val );
        m_rWW8Export.pO->push_back( nRestartNo ? 1 : 2 );
    }

    // sprmSLnnMin - Restart the Line Number with given value
    if ( nRestartNo )
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SLnnMin::val );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, o3tl::narrowing<sal_uInt16>(nRestartNo) - 1 );
    }
}

void WW8AttributeOutput::SectionTitlePage()
{
    // sprmSFTitlePage
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SFTitlePage::val );
    m_rWW8Export.pO->push_back( 1 );
}

void WW8AttributeOutput::SectionPageBorders( const SwFrameFormat* pPdFormat, const SwFrameFormat* pPdFirstPgFormat )
{
    // write border of page
    sal_uInt16 nPgBorder = MSWordSections::HasBorderItem( *pPdFormat ) ? 0 : USHRT_MAX;
    if ( pPdFormat != pPdFirstPgFormat )
    {
        if ( MSWordSections::HasBorderItem( *pPdFirstPgFormat ) )
        {
            if ( USHRT_MAX == nPgBorder )
            {
                nPgBorder = 1;
                // only the first page outlined -> Get the BoxItem from the correct format
                m_rWW8Export.m_pISet = &pPdFirstPgFormat->GetAttrSet();
                OutputItem( pPdFirstPgFormat->GetFormatAttr( RES_BOX ) );
            }
        }
        else if ( !nPgBorder )
            nPgBorder = 2;
    }

    // [MS-DOC] 2.9.255 SPgbPropOperand; 2.9.185 PgbOffsetFrom
    if (m_bFromEdge)
        nPgBorder |= (1<<5);

    if ( USHRT_MAX != nPgBorder )
    {
        // write the Flag and Border Attribute
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SPgbProp::val );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nPgBorder );
    }
}

void WW8AttributeOutput::SectionBiDi( bool bBiDi )
{
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SFBiDi::val );
    m_rWW8Export.pO->push_back( bBiDi? 1: 0 );
}

void WW8AttributeOutput::SectionPageNumbering( sal_uInt16 nNumType, const ::std::optional<sal_uInt16>& oPageRestartNumber )
{
    // sprmSNfcPgn
    sal_uInt8 nb = WW8Export::GetNumId( nNumType );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SNfcPgn::val );
    m_rWW8Export.pO->push_back( nb );

    if ( oPageRestartNumber )
    {
        // sprmSFPgnRestart
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SFPgnRestart::val );
        m_rWW8Export.pO->push_back( 1 );

        // sprmSPgnStart
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SPgnStart97::val );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, *oPageRestartNumber );
    }
}

void WW8AttributeOutput::SectionType( sal_uInt8 nBreakCode )
{
    if ( 2 != nBreakCode ) // new page is the default
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SBkc::val );
        m_rWW8Export.pO->push_back( nBreakCode );
    }
}

void WW8Export::SetupSectionPositions( WW8_PdAttrDesc* pA )
{
    if ( !pA )
        return;

    if ( !pO->empty() ) // are there attributes ?
    {
        pA->m_nLen = pO->size();
        pA->m_pData.reset(new sal_uInt8 [pO->size()]);
        // store for later
        memcpy( pA->m_pData.get(), pO->data(), pO->size() );
        pO->clear(); // clear HdFt-Text
    }
    else // no attributes there
    {
        pA->m_pData.reset();
        pA->m_nLen = 0;
    }
}

void WW8AttributeOutput::TextVerticalAdjustment( const drawing::TextVerticalAdjust nVA )
{
    if ( drawing::TextVerticalAdjust_TOP == nVA ) // top alignment is the default
        return;

    sal_uInt8 nMSVA = 0;
    switch( nVA )
    {
        case drawing::TextVerticalAdjust_CENTER:
            nMSVA = 1;
            break;
        case drawing::TextVerticalAdjust_BOTTOM:  //Writer = 2, Word = 3
            nMSVA = 3;
            break;
        case drawing::TextVerticalAdjust_BLOCK:   //Writer = 3, Word = 2
            nMSVA = 2;
            break;
        default:
            break;
    }
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::SVjc::val );
    m_rWW8Export.pO->push_back( nMSVA );
}

void WW8Export::WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
        const SwFrameFormat& rFormat, const SwFrameFormat& rLeftHeaderFormat, const SwFrameFormat& rLeftFooterFormat, const SwFrameFormat& rFirstPageFormat, sal_uInt8 nBreakCode, bool /*bEvenAndOddHeaders*/ )
{
    sal_uLong nCpPos = Fc2Cp( Strm().Tell() );

    IncrementHdFtIndex();
    if ( !(nHeadFootFlags & WW8_HEADER_EVEN) && pDop->fFacingPages )
        pSepx->OutHeaderFooter( *this, true, rFormat, nCpPos, nHeadFootFlags, WW8_HEADER_ODD, nBreakCode );
    else
        pSepx->OutHeaderFooter( *this, true, rLeftHeaderFormat, nCpPos, nHeadFootFlags, WW8_HEADER_EVEN, nBreakCode );
    IncrementHdFtIndex();
    pSepx->OutHeaderFooter( *this, true, rFormat, nCpPos, nHeadFootFlags, WW8_HEADER_ODD, nBreakCode );

    IncrementHdFtIndex();
    if ( !(nHeadFootFlags & WW8_FOOTER_EVEN) && pDop->fFacingPages )
        pSepx->OutHeaderFooter( *this, false, rFormat, nCpPos, nHeadFootFlags, WW8_FOOTER_ODD, nBreakCode );
    else
        pSepx->OutHeaderFooter( *this, false, rLeftFooterFormat, nCpPos, nHeadFootFlags, WW8_FOOTER_EVEN, nBreakCode );
    IncrementHdFtIndex();
    pSepx->OutHeaderFooter( *this, false, rFormat, nCpPos, nHeadFootFlags, WW8_FOOTER_ODD, nBreakCode );

    //#i24344# Drawing objects cannot be directly shared between main hd/ft
    //and title hd/ft so we need to differentiate them
    IncrementHdFtIndex();
    pSepx->OutHeaderFooter( *this, true, rFirstPageFormat, nCpPos, nHeadFootFlags, WW8_HEADER_FIRST, nBreakCode );
    pSepx->OutHeaderFooter( *this, false, rFirstPageFormat, nCpPos, nHeadFootFlags, WW8_FOOTER_FIRST, nBreakCode );
}

namespace
{
/**
 * Determines if the continuous section break we start should use page style properties (header,
 * footer, margins) from the next page style of the previous section.
 */
bool UsePrevSectionNextStyle(sal_uInt8 nBreakCode, const SwPageDesc* pPd,
                             const WW8_SepInfo& rSepInfo)
{
    if (nBreakCode != 0)
    {
        // Not a continuous section break.
        return false;
    }

    if (!pPd->GetFollow())
    {
        // Page style has no follow style.
        return false;
    }

    // We start a continuous section break without headers/footers. Possibly the importer had
    // headers/footers for this section break and put them to the closest page break's page style's
    // next page style. See "find a node in the section that has a page break" in writerfilter/.
    // Try the last-in-practice paragraph of the previous section.
    const SwSectionFormat* pSection = rSepInfo.pSectionFormat;
    if (pSection == reinterpret_cast<SwSectionFormat*>(sal_IntPtr(-1)))
    {
        return false;
    }

    const SwNodeIndex* pSectionStart = pSection->GetContent().GetContentIdx();
    if (!pSectionStart)
    {
        return false;
    }

    SwPaM aPaM(*pSectionStart);
    aPaM.Move(fnMoveBackward);
    if (!aPaM.GetNode().IsTextNode())
    {
        return false;
    }

    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();
    const SwAttrSet* pParaProps = &pTextNode->GetSwAttrSet();
    sal_uInt32 nCharHeight = pParaProps->GetSize().GetHeight();
    if (nCharHeight > 20)
    {
        return false;
    }

    aPaM.Move(fnMoveBackward);
    if (!aPaM.GetNode().IsTextNode())
    {
        return false;
    }

    pTextNode = aPaM.GetNode().GetTextNode();
    pParaProps = &pTextNode->GetSwAttrSet();
    return pParaProps->HasItem(RES_PAGEDESC);
}
}

void MSWordExportBase::SectionProperties( const WW8_SepInfo& rSepInfo, WW8_PdAttrDesc* pA )
{
    const SwPageDesc* pPd = rSepInfo.pPageDesc;

    if ( rSepInfo.pSectionFormat && !pPd )
        pPd = &m_rDoc.GetPageDesc( 0 );

    m_pCurrentPageDesc = pPd;

    if ( !pPd )
        return;

    bool bOldPg = m_bOutPageDescs;
    m_bOutPageDescs = true;

    AttrOutput().StartSection();

    AttrOutput().SectFootnoteEndnotePr();

    // forms
    AttrOutput().SectionFormProtection( rSepInfo.IsProtected() );

    // line numbers
    const SwLineNumberInfo& rLnNumInfo = m_rDoc.GetLineNumberInfo();
    if ( rLnNumInfo.IsPaintLineNumbers() )
        AttrOutput().SectionLineNumbering( rSepInfo.nLnNumRestartNo, rLnNumInfo );

    /*  sprmSBkc, break code:   0 No break, 1 New column
        2 New page, 3 Even page, 4 Odd page
        */
    sal_uInt8 nBreakCode = 2;            // default start new page
    bool bOutPgDscSet = true, bLeftRightPgChain = false, bOutputStyleItemSet = false;
    bool bEnsureHeaderFooterWritten = rSepInfo.pSectionFormat && rSepInfo.bIsFirstParagraph;
    const SwFrameFormat* pPdFormat = &pPd->GetMaster();
    bool bUsePrevSectionNextStyle = false;
    if ( rSepInfo.pSectionFormat )
    {
        // if pSectionFormat is set, then there is a SectionNode
        //  valid pointer -> start Section ,
        //  0xfff -> Section terminated
        nBreakCode = 0;         // consecutive section

        if ( rSepInfo.pPDNd && rSepInfo.pPDNd->IsContentNode() )
        {
            if ( !NoPageBreakSection( &rSepInfo.pPDNd->GetContentNode()->GetSwAttrSet() ) )
            {
                nBreakCode = 2;
            }
        }

        if ( reinterpret_cast<SwSectionFormat*>(sal_IntPtr(-1)) == rSepInfo.pSectionFormat )
            bEnsureHeaderFooterWritten |= !rSepInfo.pPDNd && GetExportFormat() != ExportFormat::RTF;
        else
        {
            if ( nBreakCode == 0 )
                bOutPgDscSet = false;

            // produce Itemset, which inherits PgDesk-Attr-Set:
            // as child also the parent is searched if 'deep'-OutputItemSet
            const SfxItemSet* pPdSet = &pPdFormat->GetAttrSet();

            bUsePrevSectionNextStyle = GetExportFormat() == ExportFormat::DOCX
                                       && UsePrevSectionNextStyle(nBreakCode, pPd, rSepInfo);
            if (bUsePrevSectionNextStyle)
            {
                // Take page margins from the previous section's next style.
                pPdSet = &pPd->GetFollow()->GetMaster().GetAttrSet();
            }

            SfxItemSet aSet( *pPdSet->GetPool(), pPdSet->GetRanges() );
            aSet.SetParent( pPdSet );

            // at the child ONLY change column structure according to Sect-Attr.

            const SvxLRSpaceItem &rSectionLR =
                ItemGet<SvxLRSpaceItem>( *(rSepInfo.pSectionFormat), RES_LR_SPACE );
            const SvxLRSpaceItem &rPageLR =
                ItemGet<SvxLRSpaceItem>( *pPdFormat, RES_LR_SPACE );

            SvxLRSpaceItem aResultLR( rPageLR.GetLeft() +
                    rSectionLR.GetLeft(), rPageLR.GetRight() +
                    rSectionLR.GetRight(), 0, 0, RES_LR_SPACE );
            //i120133: The Section width should consider section indent value.
            if (rSectionLR.GetLeft()+rSectionLR.GetRight()!=0)
            {
                const SwFormatCol& rCol = dynamic_cast<const SwFormatCol&>(rSepInfo.pSectionFormat->GetFormatAttr(RES_COL));
                SwFormatCol aCol(rCol);
                aCol.SetAdjustValue(rSectionLR.GetLeft()+rSectionLR.GetRight());
                aSet.Put(aCol);
            }
            else
                aSet.Put(rSepInfo.pSectionFormat->GetFormatAttr(RES_COL));

            aSet.Put( aResultLR );

            // and write into the WW-File
            const SfxItemSet* pOldI = m_pISet;
            m_pISet = &aSet;

            // Switch off test on default item values, if page description
            // set (value of <bOutPgDscSet>) isn't written.
            AttrOutput().OutputStyleItemSet( aSet, bOutPgDscSet );
            bOutputStyleItemSet = true;

            //Cannot export as normal page framedir, as continuous sections
            //cannot contain any grid settings like proper sections
            AttrOutput().SectionBiDi( SvxFrameDirection::Horizontal_RL_TB == TrueFrameDirection( *rSepInfo.pSectionFormat ) );

            m_pISet = pOldI;
        }
    }

    // Libreoffice 4.0 introduces support for page styles (SwPageDesc) with
    // a different header/footer for the first page.  The same effect can be
    // achieved by chaining two page styles together (SwPageDesc::GetFollow)
    // which are identical except for header/footer.
    // The latter method was previously used by the doc/docx import filter.
    // In both of these cases, we emit a single Word section with different
    // first page header/footer.
    const SwFrameFormat* pPdFirstPgFormat = &pPd->GetFirstMaster();
    bool titlePage = !pPd->IsFirstShared();
    if ( bOutPgDscSet )
    {
        // if a Follow is set and it does not point to itself,
        // then there is a page chain.
        // If this emulates a "first page", we can detect it here and write
        // it as title page.
        // With Left/Right changes it's different - we have to detect where
        // the change of pages is, but here it's too late for that!
        if ( pPd->GetFollow() && pPd != pPd->GetFollow() &&
             pPd->GetFollow()->GetFollow() == pPd->GetFollow() &&
             pPd->IsHeaderShared() && pPd->IsFooterShared() &&
             ( !rSepInfo.pPDNd || pPd->IsFollowNextPageOfNode( *rSepInfo.pPDNd ) ) )
        {
            const SwPageDesc *pFollow = pPd->GetFollow();
            const SwFrameFormat& rFollowFormat = pFollow->GetMaster();
            if (sw::util::IsPlausableSingleWordSection(*pPdFirstPgFormat, rFollowFormat))
            {
                if (titlePage)
                {
                    // Do nothing. First format is already set.
                }
                else if (rSepInfo.pPDNd)
                    pPdFirstPgFormat = pPd->GetPageFormatOfNode( *rSepInfo.pPDNd );
                else
                    pPdFirstPgFormat = &pPd->GetMaster();

                m_pCurrentPageDesc = pPd = pFollow;
                pPdFormat = &rFollowFormat;

                // has different headers/footers for the title page
                titlePage = true;
            }
        }

        const SfxItemSet* pOldI = m_pISet;

        const SfxPoolItem* pItem;
        if ( titlePage && SfxItemState::SET ==
                pPdFirstPgFormat->GetItemState( RES_PAPER_BIN, true, &pItem ) )
        {
            m_pISet = &pPdFirstPgFormat->GetAttrSet();
            m_bOutFirstPage = true;
            AttrOutput().OutputItem( *pItem );
            m_bOutFirstPage = false;
        }

        // left-/right chain of pagedescs ?
        if ( pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd &&
                (( UseOnPage::Left == ( UseOnPage::All & pPd->ReadUseOn() ) &&
                   UseOnPage::Right == ( UseOnPage::All & pPd->GetFollow()->ReadUseOn() )) ||
                 ( UseOnPage::Right == ( UseOnPage::All & pPd->ReadUseOn() ) &&
                   UseOnPage::Left == ( UseOnPage::All & pPd->GetFollow()->ReadUseOn() )) ))
        {
            bLeftRightPgChain = true;

            // which is the reference point? (left or right?)
            // assume it is on the right side!
            if ( UseOnPage::Left == ( UseOnPage::All & pPd->ReadUseOn() ) )
            {
                nBreakCode = 3;
                pPdFormat = &pPd->GetMaster();  //use the current page for settings (margins/width etc)
                pPd = pPd->GetFollow(); //switch to the right page for the right/odd header/footer
            }
            else
                nBreakCode = 4;
        }

        m_pISet = &pPdFormat->GetAttrSet();
        if (!bOutputStyleItemSet)
        {
            if (titlePage)
            {
                m_pFirstPageFormat = pPdFirstPgFormat;
            }

            AttrOutput().OutputStyleItemSet( pPdFormat->GetAttrSet(), false );

            if (titlePage)
            {
                m_pFirstPageFormat = nullptr;
            }
        }
        AttrOutput().SectionPageBorders( pPdFormat, pPdFirstPgFormat );
        m_pISet = pOldI;

        // then the rest of the settings from PageDesc
        AttrOutput().SectionPageNumbering( pPd->GetNumType().GetNumberingType(), rSepInfo.oPgRestartNo );

        // will it be only left or only right pages?
        if ( 2 == nBreakCode )
        {
            if ( UseOnPage::Left == ( UseOnPage::All & pPd->ReadUseOn() ) )
                nBreakCode = 3;
            else if ( UseOnPage::Right == ( UseOnPage::All & pPd->ReadUseOn() ) )
                nBreakCode = 4;
        }
    }

    if (titlePage)
        AttrOutput().SectionTitlePage();

    AttrOutput().SectionType( nBreakCode );

    if( rSepInfo.pPageDesc ) {
        AttrOutput().TextVerticalAdjustment( rSepInfo.pPageDesc->GetVerticalAdjustment() );
    }

    // Header or Footer
    sal_uInt8 nHeadFootFlags = 0;
    // Should we output a w:evenAndOddHeaders tag or not?
    // N.B.: despite its name this tag affects _both_ headers and footers!
    bool bEvenAndOddHeaders = true;
    bool bEvenAndOddFooters = true;

    const SwFrameFormat* pPdLeftHeaderFormat = nullptr;
    const SwFrameFormat* pPdLeftFooterFormat = nullptr;
    if (bLeftRightPgChain)
    {
        const SwFrameFormat* pHeaderFormat = pPd->GetStashedFrameFormat(true, true, true);
        const SwFrameFormat* pFooterFormat = pPd->GetStashedFrameFormat(false, true, true);
        if (pHeaderFormat)
        {
            pPdLeftHeaderFormat = pHeaderFormat;
            bEvenAndOddHeaders = false;
        }
        else
        {
            pPdLeftHeaderFormat = &pPd->GetFollow()->GetFirstLeft();
        }
        if (pFooterFormat)
        {
            pPdLeftFooterFormat = pFooterFormat;
            bEvenAndOddFooters = false;
        }
        else
        {
            pPdLeftFooterFormat = &pPd->GetFollow()->GetFirstLeft();
        }
    }
    else
    {
        const SwFrameFormat* pHeaderFormat = pPd->GetStashedFrameFormat(true, true, false);
        const SwFrameFormat* pFooterFormat = pPd->GetStashedFrameFormat(false, true, false);
        if (pHeaderFormat)
        {
            pPdLeftHeaderFormat = pHeaderFormat;
            bEvenAndOddHeaders = false;
        }
        else
        {
            pPdLeftHeaderFormat = &pPd->GetLeft();
        }
        if (pFooterFormat)
        {
            pPdLeftFooterFormat = pFooterFormat;
            bEvenAndOddFooters = false;
        }
        else
        {
            pPdLeftFooterFormat = &pPd->GetLeft();
        }
    }

    // Ensure that headers are written if section is first paragraph
    if ( nBreakCode != 0 || bEnsureHeaderFooterWritten )
    {
        if ( titlePage )
        {
            // there is a First Page:
            MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdFirstPgFormat, WW8_HEADER_FIRST );
            MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdFirstPgFormat, WW8_FOOTER_FIRST );
        }
        else
        {
            if ( pPd->GetStashedFrameFormat(true, true, true) && pPdLeftHeaderFormat && pPdLeftHeaderFormat->GetHeader().GetHeaderFormat() )
            {
                MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdLeftHeaderFormat, WW8_HEADER_FIRST );
            }
            if ( pPd->GetStashedFrameFormat(false, true, true) && pPdLeftFooterFormat && pPdLeftFooterFormat->GetFooter().GetFooterFormat() )
            {
                MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdLeftFooterFormat, WW8_FOOTER_FIRST );
            }
        }

        MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdFormat, WW8_HEADER_ODD );
        MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdFormat, WW8_FOOTER_ODD );

        if ( !pPd->IsHeaderShared() || bLeftRightPgChain )
        {
            MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdLeftHeaderFormat, WW8_HEADER_EVEN );
        }
        else if ( pPd->IsHeaderShared() && pPd->GetStashedFrameFormat(true, true, false) && pPdLeftHeaderFormat && pPdLeftHeaderFormat->GetHeader().GetHeaderFormat() )
        {
            MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdLeftHeaderFormat, WW8_HEADER_EVEN );
            bEvenAndOddHeaders = false;
        }

        if ( !pPd->IsFooterShared() || bLeftRightPgChain )
        {
            MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdLeftFooterFormat, WW8_FOOTER_EVEN );
        }
        else if ( pPd->IsFooterShared() && pPd->GetStashedFrameFormat(false, true, false) && pPdLeftFooterFormat && pPdLeftFooterFormat->GetFooter().GetFooterFormat() )
        {
            MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdLeftFooterFormat, WW8_FOOTER_EVEN );
            bEvenAndOddFooters = false;
        }
    }

    // binary filters only
    SetupSectionPositions( pA );

    /*
       !!!!!!!!!!!
    // borders at header and footer texts would be done like this:
    // This should use something like pOut,
    // which is repeated with every special text line.
    const SwFrameFormat* pFFormat = rFt.GetFooterFormat();
    const SvxBoxItem& rBox = pFFormat->GetBox(false);
    OutWW8_SwFormatBox1( m_rWW8Export.pOut, rBox, false);
    !!!!!!!!!!!
    You can turn this into paragraph attributes, which are then observed in each paragraph.
    Applies to background / border.
    !!!!!!!!!!!
    */

    const SwTextNode *pOldPageRoot = GetHdFtPageRoot();
    SetHdFtPageRoot( rSepInfo.pPDNd ? rSepInfo.pPDNd->GetTextNode() : nullptr );

    if (bUsePrevSectionNextStyle && nHeadFootFlags == 0)
    {
        // Take headers/footers from the previous section's next style.
        pPdFormat = &pPd->GetFollow()->GetMaster();
        MSWordSections::SetHeaderFlag(nHeadFootFlags, *pPdFormat, WW8_HEADER_ODD);
        MSWordSections::SetFooterFlag(nHeadFootFlags, *pPdFormat, WW8_FOOTER_ODD);
    }

    WriteHeadersFooters( nHeadFootFlags, *pPdFormat, *pPdLeftHeaderFormat, *pPdLeftFooterFormat, *pPdFirstPgFormat, nBreakCode, bEvenAndOddHeaders && bEvenAndOddFooters );

    SetHdFtPageRoot( pOldPageRoot );

    AttrOutput().EndSection();

    // outside of the section properties again
    m_bOutPageDescs = bOldPg;
}

bool WW8_WrPlcSepx::WriteKFText( WW8Export& rWrt )
{
    sal_uLong nCpStart = rWrt.Fc2Cp( rWrt.Strm().Tell() );

    OSL_ENSURE( !pTextPos, "who set the pointer?" );
    pTextPos.reset( new WW8_WrPlc0( nCpStart ) );

    WriteFootnoteEndText( rWrt, nCpStart );
    CheckForFacinPg( rWrt );

    unsigned int nOldIndex = rWrt.GetHdFtIndex();
    rWrt.SetHdFtIndex( 0 );

    for (const WW8_SepInfo & rSepInfo : aSects)
    {
        auto pAttrDesc = std::make_shared<WW8_PdAttrDesc>();
        m_SectionAttributes.push_back(pAttrDesc);

        rWrt.SectionProperties( rSepInfo, pAttrDesc.get() );

        // FIXME: this writes the section properties, but not of all sections;
        // it's possible that later in the document (e.g. in endnotes) sections
        // are added, but they won't have their properties written here!
        m_bHeaderFooterWritten = true;
    }
    rWrt.SetHdFtIndex( nOldIndex ); //0

    if ( pTextPos->Count() )
    {
        // HdFt available?
        sal_uLong nCpEnd = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        pTextPos->Append( nCpEnd );  // End of last Header/Footer for PlcfHdd

        if ( nCpEnd > nCpStart )
        {
            ++nCpEnd;
            pTextPos->Append( nCpEnd + 1 );  // End of last Header/Footer for PlcfHdd

            rWrt.WriteStringAsPara( OUString() ); // CR to the end ( otherwise WW complains )
        }
        rWrt.m_pFieldHdFt->Finish( nCpEnd, rWrt.pFib->m_ccpText + rWrt.pFib->m_ccpFootnote );
        rWrt.pFib->m_ccpHdr = nCpEnd - nCpStart;
    }
    else
    {
        pTextPos.reset();
    }

    return rWrt.pFib->m_ccpHdr != 0;
}

void WW8_WrPlcSepx::WriteSepx( SvStream& rStrm ) const
{
    OSL_ENSURE(m_SectionAttributes.size() == static_cast<size_t>(aSects.size())
        , "WriteSepx(): arrays out of sync!");
    for (const auto & rSectionAttribute : m_SectionAttributes) // all sections
    {
        WW8_PdAttrDesc *const pA = rSectionAttribute.get();
        if (pA->m_nLen && pA->m_pData != nullptr)
        {
            pA->m_nSepxFcPos = rStrm.Tell();
            rStrm.WriteUInt16(pA->m_nLen);
            rStrm.WriteBytes(pA->m_pData.get(), pA->m_nLen);
        }
    }
}

void WW8_WrPlcSepx::WritePlcSed( WW8Export& rWrt ) const
{
    OSL_ENSURE(m_SectionAttributes.size() == static_cast<size_t>(aSects.size())
        , "WritePlcSed(): arrays out of sync!");
    OSL_ENSURE( aCps.size() == aSects.size() + 1, "WrPlcSepx: DeSync" );
    sal_uInt64 nFcStart = rWrt.pTableStrm->Tell();

    for( decltype(aSects)::size_type i = 0; i <= aSects.size(); i++ )
    {
        sal_uInt32 nP = aCps[i];
        rWrt.pTableStrm->WriteUInt32(nP);
    }

    static WW8_SED aSed = {{4, 0},{0, 0, 0, 0},{0, 0},{0xff, 0xff, 0xff, 0xff}};

    for (const auto & rSectionAttribute : m_SectionAttributes)
    {
        // Sepx-Pos
        UInt32ToSVBT32( rSectionAttribute->m_nSepxFcPos, aSed.fcSepx );
        rWrt.pTableStrm->WriteBytes(&aSed, sizeof(aSed));
    }
    rWrt.pFib->m_fcPlcfsed = nFcStart;
    rWrt.pFib->m_lcbPlcfsed = rWrt.pTableStrm->Tell() - nFcStart;
}

void WW8_WrPlcSepx::WritePlcHdd( WW8Export& rWrt ) const
{
    // Don't write out the PlcfHdd if ccpHdd is 0: it's a validation failure case.
    if( rWrt.pFib->m_ccpHdr != 0 && pTextPos && pTextPos->Count() )
    {
        rWrt.pFib->m_fcPlcfhdd = rWrt.pTableStrm->Tell();
        pTextPos->Write( *rWrt.pTableStrm );             // Plc0
        rWrt.pFib->m_lcbPlcfhdd = rWrt.pTableStrm->Tell() -
                                rWrt.pFib->m_fcPlcfhdd;
    }
}

void MSWordExportBase::WriteHeaderFooterText( const SwFormat& rFormat, bool bHeader )
{
    const SwFormatContent *pContent;
    if ( bHeader )
    {
        m_bHasHdr = true;
        const SwFormatHeader& rHd = rFormat.GetHeader();
        OSL_ENSURE( rHd.GetHeaderFormat(), "Header text is not here" );
        pContent = &rHd.GetHeaderFormat()->GetContent();
    }
    else
    {
        m_bHasFtr = true;
        const SwFormatFooter& rFt = rFormat.GetFooter();
        OSL_ENSURE( rFt.GetFooterFormat(), "Footer text is not here" );
        pContent = &rFt.GetFooterFormat()->GetContent();
    }

    const SwNodeIndex* pSttIdx = pContent->GetContentIdx();

    if ( pSttIdx )
    {
        SwNodeIndex aIdx( *pSttIdx, 1 ),
        aEnd( *pSttIdx->GetNode().EndOfSectionNode() );
        SwNodeOffset nStart = aIdx.GetIndex();
        SwNodeOffset nEnd = aEnd.GetIndex();

        // range, i.e. valid node
        if ( nStart < nEnd )
        {
            bool bOldKF = m_bOutKF;
            m_bOutKF = true;
            WriteSpecialText( nStart, nEnd, TXT_HDFT );
            m_bOutKF = bOldKF;
        }
        else
            pSttIdx = nullptr;
    }

    if ( !pSttIdx )
    {
        // there is no Header/Footer, but a CR is still necessary
        OSL_ENSURE( pSttIdx, "Header/Footer text is not really present" );
        AttrOutput().EmptyParagraph();
    }
}

// class WW8_WrPlcFootnoteEdn : Collect the Footnotes and Endnotes and output their text
// and Plcs at the end of the document.
// WW8_WrPlcFootnoteEdn is the class for Footnotes and Endnotes

WW8_WrPlcSubDoc::WW8_WrPlcSubDoc()
{
}

WW8_WrPlcSubDoc::~WW8_WrPlcSubDoc()
{
}

void WW8_WrPlcFootnoteEdn::Append( WW8_CP nCp, const SwFormatFootnote& rFootnote )
{
    aCps.push_back( nCp );
    aContent.push_back( &rFootnote );
}

WW8_Annotation::WW8_Annotation(const SwPostItField* pPostIt, WW8_CP nRangeStart, WW8_CP nRangeEnd)
    :
        maDateTime( DateTime::EMPTY ),
        m_nRangeStart(nRangeStart),
        m_nRangeEnd(nRangeEnd)
{
    mpRichText = pPostIt->GetTextObject();
    if (!mpRichText)
        msSimpleText = pPostIt->GetText();
    msOwner = pPostIt->GetPar1();
    m_sInitials = pPostIt->GetInitials();
    maDateTime = DateTime(pPostIt->GetDate(), pPostIt->GetTime());
}

WW8_Annotation::WW8_Annotation(const SwRedlineData* pRedline)
    :
        mpRichText(nullptr),
        msSimpleText(pRedline->GetComment()),
        msOwner(SW_MOD()->GetRedlineAuthor(pRedline->GetAuthor())),
        maDateTime(pRedline->GetTimeStamp()),
        m_nRangeStart(0),
        m_nRangeEnd(0)
{
}

bool WW8_Annotation::HasRange() const
{
    if (m_nRangeStart != m_nRangeEnd)
    {
        return true;
    }

    return !m_bIgnoreEmpty;
}

void WW8_WrPlcAnnotations::AddRangeStartPosition(const OUString& rName, WW8_CP nStartCp,
                                                 bool bIgnoreEmpty)
{
    m_aRangeStartPositions[rName] = std::make_pair(nStartCp, bIgnoreEmpty);
}

void WW8_WrPlcAnnotations::Append( WW8_CP nCp, const SwPostItField *pPostIt )
{
    aCps.push_back( nCp );
    WW8_Annotation* p;
    if( m_aRangeStartPositions.find(pPostIt->GetName()) != m_aRangeStartPositions.end() )
    {
        auto [nStartCp, bIgnoreEmpty] = m_aRangeStartPositions[pPostIt->GetName()];
        p = new WW8_Annotation(pPostIt, nStartCp, nCp);
        p->m_bIgnoreEmpty = bIgnoreEmpty;
        m_aRangeStartPositions.erase(pPostIt->GetName());
    }
    else
    {
        p = new WW8_Annotation(pPostIt, nCp, nCp);
    }
    aContent.push_back( p );
}

void WW8_WrPlcAnnotations::Append( WW8_CP nCp, const SwRedlineData *pRedline )
{
    maProcessedRedlines.insert(pRedline);
    aCps.push_back( nCp );
    WW8_Annotation* p = new WW8_Annotation(pRedline);
    aContent.push_back( p );
}

bool WW8_WrPlcAnnotations::IsNewRedlineComment( const SwRedlineData *pRedline )
{
    return maProcessedRedlines.find(pRedline) == maProcessedRedlines.end();
}

WW8_WrPlcAnnotations::~WW8_WrPlcAnnotations()
{
    for(const void * p : aContent)
        delete static_cast<WW8_Annotation const *>(p);
}

bool WW8_WrPlcSubDoc::WriteGenericText( WW8Export& rWrt, sal_uInt8 nTTyp,
    WW8_CP& rCount )
{
    sal_uInt16 nLen = aContent.size();
    if ( !nLen )
        return false;

    sal_uLong nCpStart = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    pTextPos.reset( new WW8_WrPlc0( nCpStart ) );
    sal_uInt16 i;

    switch ( nTTyp )
    {
        case TXT_ATN:
            for ( i = 0; i < nLen; i++ )
            {
                // beginning for PlcfAtnText
                pTextPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));

                rWrt.WritePostItBegin();
                const WW8_Annotation& rAtn = *static_cast<const WW8_Annotation*>(aContent[i]);
                if (rAtn.mpRichText)
                    rWrt.WriteOutliner(*rAtn.mpRichText, nTTyp);
                else
                {
                    OUString sText(rAtn.msSimpleText);
                    rWrt.WriteStringAsPara(sText.replace(0x0A, 0x0B));
                }
            }
            break;

        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            for ( i = 0; i < nLen; i++ )
            {
                // textbox content
                WW8_CP nCP = rWrt.Fc2Cp( rWrt.Strm().Tell() );
                aCps.insert( aCps.begin()+i, nCP );
                pTextPos->Append( nCP );

                if( aContent[ i ] != nullptr )
                {
                    // is it a writer or sdr - textbox?
                    const SdrObject& rObj = *static_cast<SdrObject const *>(aContent[ i ]);
                    if (rObj.GetObjInventor() == SdrInventor::FmForm)
                    {
                        sal_uInt8 nOldTyp = rWrt.m_nTextTyp;
                        rWrt.m_nTextTyp = nTTyp;
                        rWrt.GetOCXExp().ExportControl(rWrt, dynamic_cast<const SdrUnoObj&>(rObj));
                        rWrt.m_nTextTyp = nOldTyp;
                    }
                    else if( dynamic_cast<const SdrTextObj*>( &rObj) !=  nullptr )
                        rWrt.WriteSdrTextObj(dynamic_cast<const SdrTextObj&>(rObj), nTTyp);
                    else
                    {
                        const SwFrameFormat* pFormat = ::FindFrameFormat( &rObj );
                        OSL_ENSURE( pFormat, "where is the format?" );

                        const SwNodeIndex* pNdIdx = pFormat->GetContent().GetContentIdx();
                        OSL_ENSURE( pNdIdx, "where is the StartNode of the Textbox?" );
                        rWrt.WriteSpecialText( pNdIdx->GetIndex() + 1,
                                               pNdIdx->GetNode().EndOfSectionIndex(),
                                               nTTyp );
                        {
                            SwNodeIndex aContentIdx = *pNdIdx;
                            ++aContentIdx;
                            if ( aContentIdx.GetNode().IsTableNode() )
                            {
                                bool bContainsOnlyTables = true;
                                do {
                                    aContentIdx = *(aContentIdx.GetNode().EndOfSectionNode());
                                    ++aContentIdx;
                                    if ( !aContentIdx.GetNode().IsTableNode() &&
                                         aContentIdx.GetIndex() != pNdIdx->GetNode().EndOfSectionIndex() )
                                    {
                                        bContainsOnlyTables = false;
                                    }
                                } while ( aContentIdx.GetNode().IsTableNode() );
                                if ( bContainsOnlyTables )
                                {
                                    // Additional paragraph containing a space to
                                    // assure that by WW created RTF from written WW8
                                    // does not crash WW.
                                    rWrt.WriteStringAsPara( " " );
                                }
                            }
                        }
                    }
                }
                else if (i < aSpareFormats.size() && aSpareFormats[i])
                {
                    const SwFrameFormat& rFormat = *aSpareFormats[i];
                    const SwNodeIndex* pNdIdx = rFormat.GetContent().GetContentIdx();
                    rWrt.WriteSpecialText( pNdIdx->GetIndex() + 1,
                               pNdIdx->GetNode().EndOfSectionIndex(), nTTyp );
                }

                // CR at end of one textbox text ( otherwise WW gpft :-( )
                rWrt.WriteStringAsPara( OUString() );
            }
            break;

        case TXT_EDN:
        case TXT_FTN:
            for ( i = 0; i < nLen; i++ )
            {
                // beginning for PlcfFootnoteText/PlcfEdnText
                pTextPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));

                // Note content
                const SwFormatFootnote* pFootnote = static_cast<SwFormatFootnote const *>(aContent[ i ]);
                rWrt.WriteFootnoteBegin( *pFootnote );
                const SwNodeIndex* pIdx = pFootnote->GetTextFootnote()->GetStartNode();
                OSL_ENSURE( pIdx, "Where is the start node of Foot-/Endnote?" );
                rWrt.WriteSpecialText( pIdx->GetIndex() + 1,
                                       pIdx->GetNode().EndOfSectionIndex(),
                                       nTTyp );
            }
            break;

        default:
            OSL_ENSURE( false, "What kind of SubDocType is that?" );
    }

    pTextPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));
    // CR to the end ( otherwise WW complains )
    rWrt.WriteStringAsPara( OUString() );

    WW8_CP nCpEnd = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    pTextPos->Append( nCpEnd );
    rCount = nCpEnd - nCpStart;

    return ( rCount != 0 );
}

static bool lcl_AuthorComp( const std::pair<OUString,OUString>& aFirst, const std::pair<OUString,OUString>& aSecond)
{
    return aFirst.first < aSecond.first;
}

static bool lcl_PosComp( const std::pair<WW8_CP, int>& aFirst, const std::pair<WW8_CP, int>& aSecond)
{
    return aFirst.first < aSecond.first;
}

void WW8_WrPlcSubDoc::WriteGenericPlc( WW8Export& rWrt, sal_uInt8 nTTyp,
    WW8_FC& rTextStart, sal_Int32& rTextCount, WW8_FC& rRefStart, sal_Int32& rRefCount ) const
{

    sal_uInt64 nFcStart = rWrt.pTableStrm->Tell();
    sal_uInt16 nLen = aCps.size();
    if ( !nLen )
        return;

    OSL_ENSURE( aCps.size() + 2 == pTextPos->Count(), "WritePlc: DeSync" );

    std::vector<std::pair<OUString,OUString> > aStrArr;
    WW8Fib& rFib = *rWrt.pFib;              // n+1-th CP-Pos according to the manual
    bool bWriteCP = true;

    switch ( nTTyp )
    {
        case TXT_ATN:
            {
                std::vector< std::pair<WW8_CP, int> > aRangeStartPos; // The second of the pair is the original index before sorting.
                std::vector< std::pair<WW8_CP, int> > aRangeEndPos; // Same, so we can map between the indexes before/after sorting.
                std::map<int, int> aAtnStartMap; // Maps from annotation index to start index.
                std::map<int, int> aStartAtnMap; // Maps from start index to annotation index.
                std::map<int, int> aStartEndMap; // Maps from start index to end index.
                // then write first the GrpXstAtnOwners
                int nIdx = 0;
                for ( sal_uInt16 i = 0; i < nLen; ++i )
                {
                    const WW8_Annotation& rAtn = *static_cast<const WW8_Annotation*>(aContent[i]);
                    aStrArr.emplace_back(rAtn.msOwner,rAtn.m_sInitials);
                    // record start and end positions for ranges
                    if (rAtn.HasRange())
                    {
                        aRangeStartPos.emplace_back(rAtn.m_nRangeStart, nIdx);
                        aRangeEndPos.emplace_back(rAtn.m_nRangeEnd, nIdx);
                        ++nIdx;
                    }
                }

                //sort and remove duplicates
                std::sort(aStrArr.begin(), aStrArr.end(),&lcl_AuthorComp);
                auto aIter = std::unique(aStrArr.begin(), aStrArr.end());
                aStrArr.erase(aIter, aStrArr.end());

                // Also sort the start and end positions. We need to reference
                // the start index in the annotation table and also need to
                // reference the end index in the start table, so build a map
                // that knows what index to reference, after sorting.
                std::sort(aRangeStartPos.begin(), aRangeStartPos.end(), &lcl_PosComp);
                for (decltype(aRangeStartPos)::size_type i = 0; i < aRangeStartPos.size(); ++i)
                {
                    aAtnStartMap[aRangeStartPos[i].second] = i;
                    aStartAtnMap[i] = aRangeStartPos[i].second;
                }
                std::sort(aRangeEndPos.begin(), aRangeEndPos.end(), &lcl_PosComp);
                for (decltype(aRangeEndPos)::size_type i = 0; i < aRangeEndPos.size(); ++i)
                    aStartEndMap[aAtnStartMap[ aRangeEndPos[i].second ]] = i;

                for ( decltype(aStrArr)::size_type i = 0; i < aStrArr.size(); ++i )
                {
                    const OUString& sAuthor = aStrArr[i].first;
                    SwWW8Writer::WriteShort(*rWrt.pTableStrm, sAuthor.getLength());
                    SwWW8Writer::WriteString16(*rWrt.pTableStrm, sAuthor,
                            false);
                }

                rFib.m_fcGrpStAtnOwners = nFcStart;
                nFcStart = rWrt.pTableStrm->Tell();
                rFib.m_lcbGrpStAtnOwners = nFcStart - rFib.m_fcGrpStAtnOwners;

                // Commented text ranges
                if( !aRangeStartPos.empty() )
                {
                    // Commented text ranges starting positions (Plcfbkf.aCP)
                    rFib.m_fcPlcfAtnbkf = nFcStart;
                    for ( decltype(aRangeStartPos)::size_type i = 0; i < aRangeStartPos.size(); ++i )
                    {
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRangeStartPos[i].first );
                    }
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, rFib.m_ccpText + 1);

                    // Commented text ranges additional information (Plcfbkf.aFBKF)
                    for ( decltype(aRangeStartPos)::size_type i = 0; i < aRangeStartPos.size(); ++i )
                    {
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, aStartEndMap[i] ); // FBKF.ibkl
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 ); // FBKF.bkc
                    }

                    nFcStart = rWrt.pTableStrm->Tell();
                    rFib.m_lcbPlcfAtnbkf = nFcStart - rFib.m_fcPlcfAtnbkf;

                    // Commented text ranges ending positions (PlcfBkl.aCP)
                    rFib.m_fcPlcfAtnbkl = nFcStart;
                    for ( decltype(aRangeEndPos)::size_type i = 0; i < aRangeEndPos.size(); ++i )
                    {
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRangeEndPos[i].first );
                    }
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, rFib.m_ccpText + 1);

                    nFcStart = rWrt.pTableStrm->Tell();
                    rFib.m_lcbPlcfAtnbkl = nFcStart - rFib.m_fcPlcfAtnbkl;

                    // Commented text ranges as bookmarks (SttbfAtnBkmk)
                    rFib.m_fcSttbfAtnbkmk = nFcStart;
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, sal_Int16(sal_uInt16(0xFFFF)) ); // SttbfAtnBkmk.fExtend
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, aRangeStartPos.size() ); // SttbfAtnBkmk.cData
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0xA );                   // SttbfAtnBkmk.cbExtra

                    for ( decltype(aRangeStartPos)::size_type i = 0; i < aRangeStartPos.size(); ++i )
                    {
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );         // SttbfAtnBkmk.cchData
                        // One ATNBE structure for all text ranges
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0x0100 );    // ATNBE.bmc
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, aStartAtnMap[i] );          // ATNBE.lTag
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, -1 );         // ATNBE.lTagOld
                    }

                    nFcStart = rWrt.pTableStrm->Tell();
                    rFib.m_lcbSttbfAtnbkmk = nFcStart - rFib.m_fcSttbfAtnbkmk;
                }

                // Write the extended >= Word XP ATRD records
                for( sal_uInt16 i = 0; i < nLen; ++i )
                {
                    const WW8_Annotation& rAtn = *static_cast<const WW8_Annotation*>(aContent[i]);

                    sal_uInt32 nDTTM = sw::ms::DateTime2DTTM(rAtn.maDateTime);

                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nDTTM );
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                }

                rFib.m_fcAtrdExtra = nFcStart;
                nFcStart = rWrt.pTableStrm->Tell();
                rFib.m_lcbAtrdExtra = nFcStart - rFib.m_fcAtrdExtra;
                rFib.m_fcHplxsdr = 0x01010002;  //WTF, but apparently necessary
                rFib.m_lcbHplxsdr = 0;
            }
            break;
        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            {
                pTextPos->Write( *rWrt.pTableStrm );
                const std::vector<sal_uInt32>* pShapeIds = GetShapeIdArr();
                OSL_ENSURE( pShapeIds, "Where are the ShapeIds?" );

                for ( sal_uInt16 i = 0; i < nLen; ++i )
                {
                    // write textbox story - FTXBXS
                    // is it a writer or sdr - textbox?
                    const SdrObject* pObj = static_cast<SdrObject const *>(aContent[ i ]);
                    sal_Int32 nCnt = 1;
                    if (dynamic_cast< const SdrTextObj *>( pObj ))
                    {
                        // find the "highest" SdrObject of this
                        const SwFrameFormat& rFormat = *::FindFrameFormat( pObj );

                        const SwFormatChain* pChn = &rFormat.GetChain();
                        while ( pChn->GetNext() )
                        {
                            // has a chain?
                            // then calc the cur pos in the chain
                            ++nCnt;
                            pChn = &pChn->GetNext()->GetChain();
                        }
                    }
                    if( nullptr == pObj )
                    {
                        if (i < aSpareFormats.size() && aSpareFormats[i])
                        {
                            const SwFrameFormat& rFormat = *aSpareFormats[i];

                            const SwFormatChain* pChn = &rFormat.GetChain();
                            while( pChn->GetNext() )
                            {
                                // has a chain?
                                // then calc the cur pos in the chain
                                ++nCnt;
                                pChn = &pChn->GetNext()->GetChain();
                            }
                        }
                    }
                    // long cTxbx / iNextReuse
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nCnt );
                    // long cReusable
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                    // short fReusable
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                    // long reserved
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, -1 );
                    // long lid
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                            (*pShapeIds)[i]);
                    // long txidUndo
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                }
                SwWW8Writer::FillCount( *rWrt.pTableStrm, 22 );
                bWriteCP = false;
            }
            break;
    }

    if ( bWriteCP )
    {
        // write CP Positions
        for ( sal_uInt16 i = 0; i < nLen; i++ )
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aCps[ i ] );

        // n+1-th CP-Pos according to the manual
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                rFib.m_ccpText + rFib.m_ccpFootnote + rFib.m_ccpHdr + rFib.m_ccpEdn +
                rFib.m_ccpTxbx + rFib.m_ccpHdrTxbx + 1 );

        if ( TXT_ATN == nTTyp )
        {
            sal_uInt16 nlTag = 0;
            for ( sal_uInt16 i = 0; i < nLen; ++i )
            {
                const WW8_Annotation& rAtn = *static_cast<const WW8_Annotation*>(aContent[i]);

                //aStrArr is sorted
                auto aIter = std::lower_bound(aStrArr.begin(),
                        aStrArr.end(), std::pair<OUString,OUString>(rAtn.msOwner,OUString()),
                        &lcl_AuthorComp);
                OSL_ENSURE(aIter != aStrArr.end() && aIter->first == rAtn.msOwner,
                        "Impossible");
                sal_uInt16 nFndPos = static_cast< sal_uInt16 >(aIter - aStrArr.begin());
                OUString sInitials( aIter->second );
                sal_uInt8 nInitialsLen = static_cast<sal_uInt8>(sInitials.getLength());
                if ( nInitialsLen > 9 )
                {
                    sInitials = sInitials.copy( 0, 9 );
                    nInitialsLen = 9;
                }

                // xstUsrInitl[ 10 ] pascal-style String holding initials
                // of annotation author
                SwWW8Writer::WriteShort(*rWrt.pTableStrm, nInitialsLen);
                SwWW8Writer::WriteString16(*rWrt.pTableStrm, sInitials,
                        false);
                SwWW8Writer::FillCount( *rWrt.pTableStrm,
                        (9 - nInitialsLen) * 2 );

                // documents layout of WriteShort's below:

                // SVBT16 ibst;      // index into GrpXstAtnOwners
                // SVBT16 ak;        // not used
                // SVBT16 grfbmc;    // not used
                // SVBT32 ITagBkmk;  // when not -1, this tag identifies the ATNBE

                SwWW8Writer::WriteShort( *rWrt.pTableStrm, nFndPos );
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                if (rAtn.HasRange())
                {
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nlTag );
                    ++nlTag;
                }
                else
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, -1 );
            }
        }
        else
        {
            sal_uInt16 nNo = 0;
            for ( sal_uInt16 i = 0; i < nLen; ++i )             // write Flags
            {
                const SwFormatFootnote* pFootnote = static_cast<SwFormatFootnote const *>(aContent[ i ]);
                SwWW8Writer::WriteShort( *rWrt.pTableStrm,
                        !pFootnote->GetNumStr().isEmpty() ? 0 : ++nNo );
            }
        }
    }
    rRefStart = nFcStart;
    nFcStart = rWrt.pTableStrm->Tell();
    rRefCount = nFcStart - rRefStart;

    pTextPos->Write( *rWrt.pTableStrm );

    switch ( nTTyp )
    {
        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            for ( sal_uInt16 i = 0; i < nLen; ++i )
            {
                // write break descriptor (BKD)
                // short itxbxs
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, i );
                // short dcpDepend
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                // short flags : icol/fTableBreak/fColumnBreak/fMarked/
                //               fUnk/fTextOverflow
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0x800 );
            }
            SwWW8Writer::FillCount( *rWrt.pTableStrm, 6 );
            break;
    }

    rTextStart = nFcStart;
    rTextCount = rWrt.pTableStrm->Tell() - nFcStart;
}

const std::vector<sal_uInt32>* WW8_WrPlcSubDoc::GetShapeIdArr() const
{
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
