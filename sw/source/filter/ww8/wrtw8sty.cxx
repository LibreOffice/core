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
#include <functional>

#include <boost/scoped_array.hpp>
#include <boost/unordered_set.hpp>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <rtl/tencinfo.h>
#include <hintids.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/fmglob.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <doc.hxx>
#include <wrtww8.hxx>
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
#include <msfilter.hxx>
#include <swmodule.hxx>

#include <filter/msfilter/sprmids.hxx>

#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "../inc/wwstyles.hxx"
#include "ww8par.hxx"
#include "ww8attributeoutput.hxx"
#include "docxattributeoutput.hxx"
#include "rtfattributeoutput.hxx"

using namespace css;
using namespace sw::util;
using namespace nsHdFtFlags;

/
struct WW8_PdAttrDesc
{
    ::boost::scoped_array<sal_uInt8> m_pData;
    sal_uInt16 m_nLen;
    WW8_FC m_nSepxFcPos;
    WW8_PdAttrDesc() : m_nLen(0), m_nSepxFcPos(0xffffffff) /*default: none*/
        { }
};

struct WW8_SED
{
    SVBT16 aBits1;      
    SVBT32 fcSepx;      
                        
    SVBT16 fnMpr;       
    SVBT32 fcMpr;       
                        
                        
};



class WW8_WrPlc0
{
private:
    std::vector<sal_uLong> aPos;      
    sal_uLong nOfs;

    //No copying
    WW8_WrPlc0(const WW8_WrPlc0&);
    WW8_WrPlc0 &operator=(const WW8_WrPlc0&);
public:
    WW8_WrPlc0( sal_uLong nOffset );
    sal_uInt16 Count() const                { return aPos.size(); }
    void Append( sal_uLong nStartCpOrFc );
    void Write( SvStream& rStrm );
};





#define WW8_RESERVED_SLOTS 15



sal_uInt16 MSWordExportBase::GetId( const SwCharFmt& rFmt ) const
{
    sal_uInt16 nRet = pStyles->GetSlot( rFmt );
    return ( nRet != 0x0fff ) ? nRet : 10;      
}



sal_uInt16 MSWordExportBase::GetId( const SwTxtFmtColl& rColl ) const
{
    sal_uInt16 nRet = pStyles->GetSlot( rColl );
    return ( nRet != 0xfff ) ? nRet : 0;        
}



//typedef pFmtT
MSWordStyles::MSWordStyles( MSWordExportBase& rExport, bool bListStyles )
    : m_rExport( rExport ),
    m_bListStyles(bListStyles)
{
    
    
    if ( !m_rExport.pDoc->GetFtnIdxs().empty() )
    {
        m_rExport.pDoc->GetEndNoteInfo().GetAnchorCharFmt( *m_rExport.pDoc );
        m_rExport.pDoc->GetEndNoteInfo().GetCharFmt( *m_rExport.pDoc );
        m_rExport.pDoc->GetFtnInfo().GetAnchorCharFmt( *m_rExport.pDoc );
        m_rExport.pDoc->GetFtnInfo().GetCharFmt( *m_rExport.pDoc );
    }
    sal_uInt16 nAlloc = WW8_RESERVED_SLOTS + m_rExport.pDoc->GetCharFmts()->size() - 1 +
                                         m_rExport.pDoc->GetTxtFmtColls()->size() - 1 +
                                         (bListStyles ? m_rExport.pDoc->GetNumRuleTbl().size() - 1 : 0);

    
    pFmtA = new SwFmt*[ nAlloc ];
    memset( pFmtA, 0, nAlloc * sizeof( SwFmt* ) );

    BuildStylesTable();
    BuildStyleIds();
}

MSWordStyles::~MSWordStyles()
{
    delete[] pFmtA;
}


sal_uInt16 MSWordStyles::GetSlot( const SwFmt& rFmt ) const
{
    sal_uInt16 n;
    for ( n = 0; n < nUsedSlots; n++ )
        if ( pFmtA[n] == &rFmt )
            return n;
    return 0xfff;                   
}

sal_uInt16 MSWordStyles::BuildGetSlot( const SwFmt& rFmt )
{
    sal_uInt16 nRet;
    switch ( nRet = rFmt.GetPoolFmtId() )
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
            nRet = nUsedSlots++;
            break;
    }
    return nRet;
}

sal_uInt16 MSWordStyles::BuildGetSlot(const SwNumRule&)
{
    return nUsedSlots++;
}

sal_uInt16 MSWordStyles::GetWWId( const SwFmt& rFmt ) const
{
    sal_uInt16 nRet = ww::stiUser;    
    sal_uInt16 nPoolId = rFmt.GetPoolFmtId();
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
        case RES_POOLCOLL_JAKETADRESS:      nRet = 36;  break;
        case RES_POOLCOLL_SENDADRESS:       nRet = 37;  break;
        case RES_POOLCOLL_ENDNOTE:          nRet = 43;  break;
        case RES_POOLCOLL_TOX_AUTHORITIESH: nRet = 44;  break;
        case RES_POOLCOLL_TOX_CNTNTH:       nRet = 46;  break;
        case RES_POOLCOLL_BUL_LEVEL1:       nRet = 48;  break;
        case RES_POOLCOLL_LISTS_BEGIN:      nRet = 47;  break;
        case RES_POOLCOLL_NUM_LEVEL1:       nRet = 49;  break;
        case RES_POOLCOLL_BUL_LEVEL2:       nRet = 54;  break;
        case RES_POOLCOLL_BUL_LEVEL3:       nRet = 55;  break;
        case RES_POOLCOLL_BUL_LEVEL4:       nRet = 56;  break;
        case RES_POOLCOLL_BUL_LEVEL5:       nRet = 57;  break;
        case RES_POOLCOLL_NUM_LEVEL2:       nRet = 58;  break;
        case RES_POOLCOLL_NUM_LEVEL3:       nRet = 59;  break;
        case RES_POOLCOLL_NUM_LEVEL4:       nRet = 60;  break;
        case RES_POOLCOLL_NUM_LEVEL5:       nRet = 61;  break;
        case RES_POOLCOLL_DOC_TITEL:        nRet = 62;  break;
        case RES_POOLCOLL_SIGNATURE:        nRet = 64;  break;
        case RES_POOLCOLL_TEXT:             nRet = 66;  break;
        case RES_POOLCOLL_TEXT_MOVE:        nRet = 67;  break;
        case RES_POOLCOLL_BUL_NONUM1:       nRet = 68;  break;
        case RES_POOLCOLL_BUL_NONUM2:       nRet = 69;  break;
        case RES_POOLCOLL_BUL_NONUM3:       nRet = 70;  break;
        case RES_POOLCOLL_BUL_NONUM4:       nRet = 71;  break;
        case RES_POOLCOLL_BUL_NONUM5:       nRet = 72;  break;
        case RES_POOLCOLL_DOC_SUBTITEL:     nRet = 74;  break;
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
    nUsedSlots = WW8_RESERVED_SLOTS;    
                                        
    const SwCharFmts& rArr = *m_rExport.pDoc->GetCharFmts();       
    
    for( sal_uInt16 n = 1; n < rArr.size(); n++ )
    {
        SwCharFmt* pFmt = rArr[n];
        pFmtA[ BuildGetSlot( *pFmt ) ] = pFmt;
    }

    const SwTxtFmtColls& rArr2 = *m_rExport.pDoc->GetTxtFmtColls();   
    
    for( sal_uInt16 n = 1; n < rArr2.size(); n++ )
    {
        SwTxtFmtColl* pFmt = rArr2[n];
        pFmtA[ BuildGetSlot( *pFmt ) ] = pFmt;
    }

    if (!m_bListStyles)
        return;

    const SwNumRuleTbl& rNumRuleTbl = m_rExport.pDoc->GetNumRuleTbl();
    for (size_t i = 0; i < rNumRuleTbl.size(); ++i)
    {
        const SwNumRule* pNumRule = rNumRuleTbl[i];
        if (pNumRule->IsAutoRule() || pNumRule->GetName().startsWith("WWNum"))
            continue;
        sal_uInt16 nSlot = BuildGetSlot(*pNumRule);
        m_aNumRules[nSlot] = pNumRule;
    }
}

void MSWordStyles::BuildStyleIds()
{
    boost::unordered_set<OString, OStringHash> aUsed;

    m_aStyleIds.push_back("Normal");
    aUsed.insert("normal");

    for (sal_uInt16 n = 1; n < nUsedSlots; ++n)
    {
        const OUString aName(pFmtA[n]? pFmtA[n]->GetName(): (m_aNumRules.find(n) != m_aNumRules.end() ? m_aNumRules[n]->GetName() : OUString()));

        OStringBuffer aStyleIdBuf(aName.getLength());
        for (int i = 0; i < aName.getLength(); ++i)
        {
            sal_Unicode nChar = aName[i];
            if (('0' <= nChar && nChar <= '9') ||
                ('a' <= nChar && nChar <= 'z') ||
                ('A' <= nChar && nChar <= 'Z'))
            {
                
                if (aStyleIdBuf.isEmpty() && 'a' <= nChar && nChar <= 'z')
                    aStyleIdBuf.append(char(nChar - ('a' - 'A')));
                else
                    aStyleIdBuf.append(char(nChar));
            }
        }

        OString aStyleId(aStyleIdBuf.makeStringAndClear());
        if (aStyleId.isEmpty())
            aStyleId = "Style";

        OString aLower(aStyleId.toAsciiLowerCase());

        
        if (aUsed.find(aLower) == aUsed.end())
        {
            aUsed.insert(aLower);
            m_aStyleIds.push_back(aStyleId);
        }
        else
        {
            int nFree = 1;
            while (aUsed.find(aLower + OString::number(nFree)) != aUsed.end())
                ++nFree;

            aUsed.insert(aLower + OString::number(nFree));
            m_aStyleIds.push_back(aStyleId + OString::number(nFree));
        }
    }
}

OString MSWordStyles::GetStyleId(sal_uInt16 nId) const
{
    return m_aStyleIds[nId];
}

/
static void impl_SkipOdd( ww::bytes* pO, sal_Size nTableStrmTell )
{
    if ( ( nTableStrmTell + pO->size() ) & 1 )     
        pO->push_back( (sal_uInt8)0 );         
}

void WW8AttributeOutput::EndStyle()
{
    impl_SkipOdd( m_rWW8Export.pO, m_rWW8Export.pTableStrm->Tell() );

    short nLen = m_rWW8Export.pO->size() - 2;            
    sal_uInt8* p = &m_rWW8Export.pO->front() + nPOPosStdLen1;
    ShortToSVBT16( nLen, p );               
    p = &m_rWW8Export.pO->front() + nPOPosStdLen2;
    ShortToSVBT16( nLen, p );               

    m_rWW8Export.pTableStrm->Write( m_rWW8Export.pO->data(), m_rWW8Export.pO->size() );      
    m_rWW8Export.pO->clear();
}

void WW8AttributeOutput::StartStyle( const OUString& rName, StyleType eType, sal_uInt16 nWwBase,
    sal_uInt16 nWwNext, sal_uInt16 nWwId, sal_uInt16 /*nId*/, bool bAutoUpdate )
{
    sal_uInt8 aWW8_STD[ sizeof( WW8_STD ) ];
    sal_uInt8* pData = aWW8_STD;
    memset( &aWW8_STD, 0, sizeof( WW8_STD ) );

    sal_uInt16 nBit16 = 0x1000;         
    nBit16 |= (ww::stiNil & nWwId);
    Set_UInt16( pData, nBit16 );

    nBit16 = nWwBase << 4;          
    nBit16 |= (eType == STYLE_TYPE_PARA ? 1 : 2);      
    Set_UInt16( pData, nBit16 );

    nBit16 = nWwNext << 4;          
    nBit16 |= (eType == STYLE_TYPE_PARA ? 2 : 1);      
    Set_UInt16( pData, nBit16 );

    pData += sizeof( sal_uInt16 );      

    if( m_rWW8Export.bWrtWW8 )
    {
        nBit16 = bAutoUpdate ? 1 : 0;  
        Set_UInt16( pData, nBit16 );
        //-------- jetzt neu:
        
        //sal_uInt16    fHidden : 1;       /* hidden from UI? */
        //sal_uInt16    : 14;              /* unused bits */
    }


    sal_uInt16 nLen = static_cast< sal_uInt16 >( ( pData - aWW8_STD ) + 1 +
                ((m_rWW8Export.bWrtWW8 ? 2 : 1 ) * (rName.getLength() + 1)) );  

    nPOPosStdLen1 = m_rWW8Export.pO->size();        

    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nLen );
    m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), aWW8_STD, pData );

    nPOPosStdLen2 = nPOPosStdLen1 + 8;  

    
    if( m_rWW8Export.bWrtWW8 )
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, rName.getLength() ); 
        SwWW8Writer::InsAsString16( *m_rWW8Export.pO, rName );
    }
    else
    {
        m_rWW8Export.pO->push_back( (sal_uInt8)rName.getLength() );       
        SwWW8Writer::InsAsString8( *m_rWW8Export.pO, rName, RTL_TEXTENCODING_MS_1252 );
    }
    m_rWW8Export.pO->push_back( (sal_uInt8)0 );             
}

void MSWordStyles::SetStyleDefaults( const SwFmt& rFmt, bool bPap )
{
    const SwModify* pOldMod = m_rExport.pOutFmtNode;
    m_rExport.pOutFmtNode = &rFmt;
    bool aFlags[ static_cast< sal_uInt16 >(RES_FRMATR_END) - RES_CHRATR_BEGIN ];
    sal_uInt16 nStt, nEnd, n;
    if( bPap )
       nStt = RES_PARATR_BEGIN, nEnd = RES_FRMATR_END;
    else
       nStt = RES_CHRATR_BEGIN, nEnd = RES_TXTATR_END;

    
    const SfxItemPool& rPool = *rFmt.GetAttrSet().GetPool();
    for( n = nStt; n < nEnd; ++n )
        aFlags[ n - RES_CHRATR_BEGIN ] = 0 != rPool.GetPoolDefaultItem( n );

    
    if( bPap )
    {
        aFlags[ static_cast< sal_uInt16 >(RES_PARATR_WIDOWS) - RES_CHRATR_BEGIN ] = true;
        aFlags[ static_cast< sal_uInt16 >(RES_PARATR_HYPHENZONE) - RES_CHRATR_BEGIN ] = true;
    }
    else
    {
        aFlags[ RES_CHRATR_FONTSIZE - RES_CHRATR_BEGIN ] = true;
        aFlags[ RES_CHRATR_LANGUAGE - RES_CHRATR_BEGIN ] = true;
    }

    const SfxItemSet* pOldI = m_rExport.GetCurItemSet();
    m_rExport.SetCurItemSet( &rFmt.GetAttrSet() );

    const bool* pFlags = aFlags + ( nStt - RES_CHRATR_BEGIN );
    for ( n = nStt; n < nEnd; ++n, ++pFlags )
    {
        if ( *pFlags && !m_rExport.ignoreAttributeForStyles( n )
            && SFX_ITEM_SET != rFmt.GetItemState(n, false))
        {
            //If we are a character property then see if it is one of the
            //western/asian ones that must be collapsed together for export to
            //word. If so default to the western varient.
            if ( bPap || m_rExport.CollapseScriptsforWordOk(
                i18n::ScriptType::LATIN, n) )
            {
                m_rExport.AttrOutput().OutputItem( rFmt.GetFmtAttr( n, true ) );
            }
        }
    }

    m_rExport.SetCurItemSet( pOldI );
    m_rExport.pOutFmtNode = pOldMod;
}

void WW8AttributeOutput::StartStyleProperties( bool bParProp, sal_uInt16 nStyle )
{
    impl_SkipOdd( m_rWW8Export.pO, m_rWW8Export.pTableStrm->Tell() );

    sal_uInt16 nLen = ( bParProp ) ? 2 : 0;             
    m_nStyleLenPos = m_rWW8Export.pO->size();               
                                    
                                    

    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nLen );        

    m_nStyleStartSize = m_rWW8Export.pO->size();

    if ( bParProp )
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nStyle );     
}

void MSWordStyles::WriteProperties( const SwFmt* pFmt, bool bParProp, sal_uInt16 nPos,
    bool bInsDefCharSiz )
{
    m_rExport.AttrOutput().StartStyleProperties( bParProp, nPos );

    OSL_ENSURE( m_rExport.pCurrentStyle == NULL, "Current style not NULL" ); 
    m_rExport.pCurrentStyle = pFmt;

    m_rExport.OutputFormat( *pFmt, bParProp, !bParProp );

    OSL_ENSURE( m_rExport.pCurrentStyle == pFmt, "current style was changed" );
    
    m_rExport.pCurrentStyle = NULL;

    if ( bInsDefCharSiz  )                   
        SetStyleDefaults( *pFmt, bParProp );

    m_rExport.AttrOutput().EndStyleProperties( bParProp );
}

void WW8AttributeOutput::EndStyleProperties( bool /*bParProp*/ )
{
    sal_uInt16 nLen = m_rWW8Export.pO->size() - m_nStyleStartSize;
    sal_uInt8* pUpxLen = &m_rWW8Export.pO->front() + m_nStyleLenPos; 
    ShortToSVBT16( nLen, pUpxLen );                 
}

void MSWordStyles::GetStyleData( SwFmt* pFmt, bool& bFmtColl, sal_uInt16& nBase, sal_uInt16& nNext )
{
    bFmtColl = pFmt->Which() == RES_TXTFMTCOLL || pFmt->Which() == RES_CONDTXTFMTCOLL;

    
    nBase = 0xfff;

    
    if ( !pFmt->IsDefault() )
        nBase = GetSlot( *pFmt->DerivedFrom() );

    SwFmt* pNext;
    if ( bFmtColl )
        pNext = &((SwTxtFmtColl*)pFmt)->GetNextTxtFmtColl();
    else
        pNext = pFmt; 

    nNext = GetSlot( *pNext );
}

void WW8AttributeOutput::DefaultStyle( sal_uInt16 nStyle )
{
    if ( nStyle == 10 )           
    {
        if ( m_rWW8Export.bWrtWW8 )
        {
            sal_uInt16 n = 0;
            m_rWW8Export.pTableStrm->Write( &n , 2 );   
        }
        else
        {
            static sal_uInt8 aDefCharSty[] = {
                0x26, 0x00,
                0x41, 0x40, 0xF2, 0xFF, 0xA1, 0x00, 0x26, 0x00,
                0x19, 0x41, 0x62, 0x73, 0x61, 0x74, 0x7A, 0x2D,
                0x53, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x72, 0x64,
                0x73, 0x63, 0x68, 0x72, 0x69, 0x66, 0x74, 0x61,
                0x72, 0x74, 0x00, 0x00, 0x00, 0x00 };
            m_rWW8Export.pTableStrm->Write( &aDefCharSty, sizeof( aDefCharSty ) );
        }
    }
    else
    {
        sal_uInt16 n = 0;
        m_rWW8Export.pTableStrm->Write( &n , 2 );   
    }
}

void MSWordStyles::OutputStyle(const SwNumRule* pNumRule, sal_uInt16 nPos)
{
    m_rExport.AttrOutput().StartStyle( pNumRule->GetName(), STYLE_TYPE_LIST,
            /*nBase =*/ 0, /*nWwNext =*/ 0, /*nWWId =*/ 0, nPos,
            /*bAutoUpdateFmt =*/ false );

    m_rExport.AttrOutput().EndStyle();
}


void MSWordStyles::OutputStyle( SwFmt* pFmt, sal_uInt16 nPos )
{
    if ( !pFmt )
        m_rExport.AttrOutput().DefaultStyle( nPos );
    else
    {
        bool bFmtColl;
        sal_uInt16 nBase, nWwNext;

        GetStyleData( pFmt, bFmtColl, nBase, nWwNext );

        OUString aName = pFmt->GetName();
        
        
        
        if ( nPos == 0 )
        {
            assert( pFmt->GetPoolFmtId() == RES_POOLCOLL_STANDARD );
            aName = "Normal";
        }
        else if (aName.equalsIgnoreAsciiCase("Normal"))
        {
            
            const OUString aBaseName = "LO-" + aName;
            aName = aBaseName;
            
            for ( int nSuffix = 0; ; ++nSuffix ) {
                bool clash=false;
                for ( sal_uInt16 n = 1; n < nUsedSlots; ++n )
                    if ( pFmtA[n] &&
                         pFmtA[n]->GetName().equalsIgnoreAsciiCase(aName) )
                    {
                        clash = true;
                        break;
                    }
                if (!clash)
                    break;
                
                aName = aBaseName + OUString::number(++nSuffix);
            }
        }

        m_rExport.AttrOutput().StartStyle( aName, (bFmtColl ? STYLE_TYPE_PARA : STYLE_TYPE_CHAR),
                nBase, nWwNext, GetWWId( *pFmt ), nPos,
                pFmt->IsAutoUpdateFmt() );

        if ( bFmtColl )
            WriteProperties( pFmt, true, nPos, nBase==0xfff );           

        WriteProperties( pFmt, false, nPos, bFmtColl && nBase==0xfff );  

        m_rExport.AttrOutput().EndStyle();
    }
}

void WW8AttributeOutput::StartStyles()
{
    WW8Fib& rFib = *m_rWW8Export.pFib;

    sal_uLong nCurPos = m_rWW8Export.pTableStrm->Tell();
    if ( nCurPos & 1 )                   
    {
        m_rWW8Export.pTableStrm->WriteChar( (char)0 );        
        ++nCurPos;
    }
    rFib.fcStshfOrig = rFib.fcStshf = nCurPos;
    m_nStyAnzPos = nCurPos + 2;     

    if ( m_rWW8Export.bWrtWW8 )
    {
        static sal_uInt8 aStShi[] = {
            0x12, 0x00,
            0x0F, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x5B, 0x00,
            0x0F, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00 };

        m_rWW8Export.pTableStrm->Write( &aStShi, sizeof( aStShi ) );
    }
    else
    {
        static sal_uInt8 aStShi[] = {
            0x0E, 0x00,
            0x0F, 0x00, 0x08, 0x00, 0x01, 0x00, 0x4B, 0x00,
            0x0F, 0x00, 0x00, 0x00, 0x00, 0x00 };
        m_rWW8Export.pTableStrm->Write( &aStShi, sizeof( aStShi ) );
    }
}

void WW8AttributeOutput::EndStyles( sal_uInt16 nNumberOfStyles )
{
    WW8Fib& rFib = *m_rWW8Export.pFib;

    rFib.lcbStshfOrig = rFib.lcbStshf = m_rWW8Export.pTableStrm->Tell() - rFib.fcStshf;
    SwWW8Writer::WriteShort( *m_rWW8Export.pTableStrm, m_nStyAnzPos, nNumberOfStyles );
}

void MSWordStyles::OutputStylesTable()
{
    m_rExport.bStyDef = true;

    m_rExport.AttrOutput().StartStyles();

    sal_uInt16 n;
    for ( n = 0; n < nUsedSlots; n++ )
    {
        if (m_aNumRules.find(n) != m_aNumRules.end())
            OutputStyle(m_aNumRules[n], n);
        else
            OutputStyle( pFmtA[n], n );
    }

    m_rExport.AttrOutput().EndStyles( nUsedSlots );

    m_rExport.bStyDef = false;
}

const SwNumRule* MSWordStyles::GetSwNumRule(sal_uInt16 nId) const
{
    std::map<sal_uInt16, const SwNumRule*>::const_iterator it = m_aNumRules.find(nId);
    assert(it != m_aNumRules.end());
    return it->second;
}




wwFont::wwFont(const OUString &rFamilyName, FontPitch ePitch, FontFamily eFamily,
    rtl_TextEncoding eChrSet, bool bWrtWW8) : mbAlt(false), mbWrtWW8(bWrtWW8), mePitch(ePitch), meFamily(eFamily), meChrSet(eChrSet)
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

    memset(maWW8_FFN, 0, sizeof(maWW8_FFN));

    if (bWrtWW8)
    {
        maWW8_FFN[0] = (sal_uInt8)( 6 - 1 + 0x22 + ( 2 * ( 1 + msFamilyNm.getLength() ) ));
        if (mbAlt)
            maWW8_FFN[0] = static_cast< sal_uInt8 >(maWW8_FFN[0] + 2 * ( 1 + msAltNm.getLength()));
    }
    else
    {
        maWW8_FFN[0] = (sal_uInt8)( 6 - 1 + 1 + msFamilyNm.getLength() );
        if (mbAlt)
            maWW8_FFN[0] = static_cast< sal_uInt8 >(maWW8_FFN[0] + 1 + msAltNm.getLength());
    }

    sal_uInt8 aB = 0;
    switch(ePitch)
    {
        case PITCH_VARIABLE:
            aB |= 2;    
            break;
        case PITCH_FIXED:
            aB |= 1;
            break;
        default:        
            break;
    }
    aB |= 1 << 2;   

    switch(eFamily)
    {
        case FAMILY_ROMAN:
            aB |= 1 << 4;   
            break;
        case FAMILY_SWISS:
            aB |= 2 << 4;   
            break;
        case FAMILY_MODERN:
            aB |= 3 << 4;   
            break;
        case FAMILY_SCRIPT:
            aB |= 4 << 4;   
            break;
        case FAMILY_DECORATIVE:
            aB |= 5 << 4;   
            break;
        default:            
            break;
    }
    maWW8_FFN[1] = aB;

    ShortToSVBT16( 400, &maWW8_FFN[2] );        
                                                
                                                //
    //#i61927# For unicode fonts like Arial Unicode, Word 97+ sets the chs
    //to SHIFTJIS presumably to capture that it's a multi-byte encoding font
    //but Word95 doesn't do this, and sets it to 0 (ANSI), so we should do the
    //same
    maWW8_FFN[4] = bWrtWW8 ?
        sw::ms::rtl_TextEncodingToWinCharset(eChrSet) :
        rtl_getBestWindowsCharsetFromTextEncoding(eChrSet);

    if (mbAlt)
        maWW8_FFN[5] = static_cast< sal_uInt8 >(msFamilyNm.getLength() + 1);
}

bool wwFont::Write(SvStream *pTableStrm) const
{
    pTableStrm->Write(maWW8_FFN, sizeof(maWW8_FFN));    
    if (mbWrtWW8)
    {
        
        
        //char  panose[ 10 ];       
        //char  fs[ 24     ];       
        SwWW8Writer::FillCount(*pTableStrm, 0x22);
        SwWW8Writer::WriteString16(*pTableStrm, msFamilyNm, true);
        if (mbAlt)
            SwWW8Writer::WriteString16(*pTableStrm, msAltNm, true);
    }
    else
    {
        SwWW8Writer::WriteString8(*pTableStrm, msFamilyNm, true,
            RTL_TEXTENCODING_MS_1252);
        if (mbAlt)
        {
            SwWW8Writer::WriteString8( *pTableStrm, msAltNm, true,
                RTL_TEXTENCODING_MS_1252);
        }
    }
    return true;
}

void wwFont::WriteDocx( DocxAttributeOutput* rAttrOutput ) const
{
    

    if (!msFamilyNm.isEmpty())
    {
        rAttrOutput->StartFont( msFamilyNm );

        if ( mbAlt )
            rAttrOutput->FontAlternateName( msAltNm );
        rAttrOutput->FontCharset( sw::ms::rtl_TextEncodingToWinCharset( meChrSet ), meChrSet );
        rAttrOutput->FontFamilyType( meFamily );
        rAttrOutput->FontPitchType( mePitch );
        rAttrOutput->EmbedFont( msFamilyNm, meFamily, mePitch, meChrSet );

        rAttrOutput->EndFont();
    }
}

void wwFont::WriteRtf( const RtfAttributeOutput* rAttrOutput ) const
{
    rAttrOutput->FontFamilyType( meFamily, *this );
    rAttrOutput->FontPitchType( mePitch );
    rAttrOutput->FontCharset( rtl_getBestWindowsCharsetFromTextEncoding( meChrSet ) );
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
    ::std::map<wwFont, sal_uInt16>::const_iterator aIter = maFonts.find(rFont);
    if (aIter != maFonts.end())
        nRet = aIter->second;
    else
    {
        nRet = static_cast< sal_uInt16 >(maFonts.size());
        maFonts[rFont] = nRet;
    }
    return nRet;
}

void wwFontHelper::InitFontTable(bool bWrtWW8,const SwDoc& rDoc)
{
    mbWrtWW8 = bWrtWW8;

    GetId(wwFont(OUString("Times New Roman"), PITCH_VARIABLE,
        FAMILY_ROMAN, RTL_TEXTENCODING_MS_1252,bWrtWW8));

    GetId(wwFont(OUString("Symbol"), PITCH_VARIABLE, FAMILY_ROMAN,
        RTL_TEXTENCODING_SYMBOL,bWrtWW8));

    GetId(wwFont(OUString("Arial"), PITCH_VARIABLE, FAMILY_SWISS,
        RTL_TEXTENCODING_MS_1252,bWrtWW8));

    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr(RES_CHRATR_FONT);

    GetId(wwFont(pFont->GetFamilyName(), pFont->GetPitch(),
        pFont->GetFamily(), pFont->GetCharSet(),bWrtWW8));

    const SfxItemPool& rPool = rDoc.GetAttrPool();
    if (0 != (pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(RES_CHRATR_FONT)))
    {
        GetId(wwFont(pFont->GetFamilyName(), pFont->GetPitch(),
            pFont->GetFamily(), pFont->GetCharSet(),bWrtWW8));
    }

    if (!bLoadAllFonts)
        return;

    const sal_uInt16 aTypes[] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_FONT, 0 };
    for (const sal_uInt16* pId = aTypes; *pId; ++pId)
    {
        sal_uInt32 const nMaxItem = rPool.GetItemCount2( *pId );
        for (sal_uInt32 nGet = 0; nGet < nMaxItem; ++nGet)
        {
            pFont = (const SvxFontItem*)rPool.GetItem2( *pId, nGet );
            if (0 != pFont)
            {
                GetId(wwFont(pFont->GetFamilyName(), pFont->GetPitch(),
                            pFont->GetFamily(), pFont->GetCharSet(),bWrtWW8));
            }
        }
    }
}

sal_uInt16 wwFontHelper::GetId(const Font& rFont)
{
    wwFont aFont(rFont.GetName(), rFont.GetPitch(), rFont.GetFamily(),
        rFont.GetCharSet(), mbWrtWW8);
    return GetId(aFont);
}

sal_uInt16 wwFontHelper::GetId(const SvxFontItem& rFont)
{
    wwFont aFont(rFont.GetFamilyName(), rFont.GetPitch(), rFont.GetFamily(),
        rFont.GetCharSet(), mbWrtWW8);
    return GetId(aFont);
}

::std::vector< const wwFont* > wwFontHelper::AsVector() const
{
    ::std::vector<const wwFont *> aFontList( maFonts.size() );

    typedef ::std::map<wwFont, sal_uInt16>::const_iterator myiter;
    myiter aEnd = maFonts.end();
    for ( myiter aIter = maFonts.begin(); aIter != aEnd; ++aIter )
        aFontList[aIter->second] = &aIter->first;

    return aFontList;
}

void wwFontHelper::WriteFontTable(SvStream *pTableStream, WW8Fib& rFib)
{
    rFib.fcSttbfffn = pTableStream->Tell();
    /*
     * Reserve some space to fill in the len after we know how big it is
     */
    if (mbWrtWW8)
        SwWW8Writer::WriteLong(*pTableStream, 0);
    else
        SwWW8Writer::WriteShort(*pTableStream, 0);

    /*
     * Convert from fast insertion map to linear vector in the order that we
     * want to write.
     */
    ::std::vector<const wwFont *> aFontList( AsVector() );

    /*
     * Write them all to pTableStream
     */
    ::std::for_each(aFontList.begin(), aFontList.end(),
        ::std::bind2nd(::std::mem_fun(&wwFont::Write),pTableStream));

    /*
     * Write the position and len in the FIB
     */
    rFib.lcbSttbfffn = pTableStream->Tell() - rFib.fcSttbfffn;
    if (mbWrtWW8)
        SwWW8Writer::WriteLong( *pTableStream, rFib.fcSttbfffn, maFonts.size());
    else
    {
        SwWW8Writer::WriteShort( *pTableStream, rFib.fcSttbfffn,
            (sal_Int16)rFib.lcbSttbfffn );
    }
}

void wwFontHelper::WriteFontTable( DocxAttributeOutput& rAttrOutput )
{
    ::std::vector<const wwFont *> aFontList( AsVector() );

    ::std::for_each( aFontList.begin(), aFontList.end(),
        ::std::bind2nd( ::std::mem_fun( &wwFont::WriteDocx ), &rAttrOutput ) );
}

void wwFontHelper::WriteFontTable( const RtfAttributeOutput& rAttrOutput )
{
    ::std::vector<const wwFont *> aFontList( AsVector() );

    ::std::for_each( aFontList.begin(), aFontList.end(),
        ::std::bind2nd( ::std::mem_fun( &wwFont::WriteRtf ), &rAttrOutput ) );
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
    std::vector<sal_uLong>::const_iterator iter;
    for( iter = aPos.begin(); iter != aPos.end(); ++iter )
    {
        SVBT32 nP;
        UInt32ToSVBT32( *iter, nP );
        rStrm.Write( nP, 4 );
    }
}








MSWordSections::MSWordSections( MSWordExportBase& rExport )
    : mbDocumentIsProtected( false )
{
    const SwSectionFmt *pFmt = 0;
    rExport.pAktPageDesc = &rExport.pDoc->GetPageDesc( 0 );

    const SfxPoolItem* pI;
    const SwNode* pNd = rExport.pCurPam->GetCntntNode();
    const SfxItemSet* pSet = pNd ? &((SwCntntNode*)pNd)->GetSwAttrSet() : 0;

    sal_uLong nRstLnNum =  pSet ? ((SwFmtLineNumber&)pSet->Get( RES_LINENUMBER )).GetStartValue() : 0;

    const SwTableNode* pTblNd = rExport.pCurPam->GetNode()->FindTableNode();
    const SwSectionNode* pSectNd;
    if ( pTblNd )
    {
        pSet = &pTblNd->GetTable().GetFrmFmt()->GetAttrSet();
        pNd = pTblNd;
    }
    else if ( 0 != ( pSectNd = pNd->FindSectionNode() ) )
    {
        if ( TOX_HEADER_SECTION == pSectNd->GetSection().GetType() &&
             pSectNd->StartOfSectionNode()->IsSectionNode() )
        {
            pSectNd = pSectNd->StartOfSectionNode()->GetSectionNode();
        }

        if ( TOX_CONTENT_SECTION == pSectNd->GetSection().GetType() )
        {
            pNd = pSectNd;
            rExport.pCurPam->GetPoint()->nNode = *pNd;
        }

        if ( CONTENT_SECTION == pSectNd->GetSection().GetType() )
            pFmt = pSectNd->GetSection().GetFmt();
    }

    
    if ( pSet &&
         SFX_ITEM_ON == pSet->GetItemState( RES_PAGEDESC, true, &pI ) &&
         ( (SwFmtPageDesc*)pI )->GetPageDesc() )
    {
        AppendSection( *(SwFmtPageDesc*)pI, *pNd, pFmt, nRstLnNum );
    }
    else
        AppendSection( rExport.pAktPageDesc, pFmt, nRstLnNum );
}

WW8_WrPlcSepx::WW8_WrPlcSepx( MSWordExportBase& rExport )
    : MSWordSections( rExport )
    , m_bHeaderFooterWritten( false )
    , pTxtPos( 0 )
{
    
    
    aCps.push_back( 0 );
}

MSWordSections::~MSWordSections()
{
}

WW8_WrPlcSepx::~WW8_WrPlcSepx()
{
    delete pTxtPos;
}

bool MSWordSections::HeaderFooterWritten()
{
    return false; 
}

bool WW8_WrPlcSepx::HeaderFooterWritten()
{
    return m_bHeaderFooterWritten;
}

sal_uInt16 MSWordSections::CurrentNumberOfColumns( const SwDoc &rDoc ) const
{
    OSL_ENSURE( !aSects.empty(), "no segement inserted yet" );
    if ( aSects.empty() )
        return 1;

    return NumberOfColumns( rDoc, aSects.back() );
}

sal_uInt16 MSWordSections::NumberOfColumns( const SwDoc &rDoc, const WW8_SepInfo& rInfo ) const
{
    const SwPageDesc* pPd = rInfo.pPageDesc;
    if ( !pPd )
        pPd = &rDoc.GetPageDesc( 0 );

    if ( !pPd )
    {
        OSL_ENSURE( pPd, "totally impossible" );
        return 1;
    }

    const SfxItemSet &rSet = pPd->GetMaster().GetAttrSet();
    SfxItemSet aSet( *rSet.GetPool(), RES_COL, RES_COL );
    aSet.SetParent( &rSet );

    //0xffffffff, what the hell is going on with that!, fixme most terribly
    if ( rInfo.pSectionFmt && (SwSectionFmt*)0xFFFFFFFF != rInfo.pSectionFmt )
        aSet.Put( rInfo.pSectionFmt->GetFmtAttr( RES_COL ) );

    const SwFmtCol& rCol = (const SwFmtCol&)aSet.Get( RES_COL );
    const SwColumns& rColumns = rCol.GetColumns();
    return rColumns.size();
}

const WW8_SepInfo* MSWordSections::CurrentSectionInfo()
{
    if ( !aSects.empty() )
        return &aSects.back();

    return NULL;
}

void MSWordSections::AppendSection( const SwPageDesc* pPd,
    const SwSectionFmt* pSectionFmt, sal_uLong nLnNumRestartNo )
{
    if (HeaderFooterWritten()) {
        return; 
    }
    aSects.push_back( WW8_SepInfo( pPd, pSectionFmt, nLnNumRestartNo ) );
    NeedsDocumentProtected( aSects.back() );
}

void WW8_WrPlcSepx::AppendSep( WW8_CP nStartCp, const SwPageDesc* pPd,
    const SwSectionFmt* pSectionFmt, sal_uLong nLnNumRestartNo )
{
    if (HeaderFooterWritten()) {
        return; 
    }
    aCps.push_back( nStartCp );
    AppendSection( pPd, pSectionFmt, nLnNumRestartNo );
}

void MSWordSections::AppendSection( const SwFmtPageDesc& rPD,
    const SwNode& rNd, const SwSectionFmt* pSectionFmt, sal_uLong nLnNumRestartNo )
{
    if (HeaderFooterWritten()) {
        return; 
    }

    WW8_SepInfo aI( rPD.GetPageDesc(), pSectionFmt, nLnNumRestartNo, rPD.GetNumOffset(), &rNd );

    aSects.push_back( aI );
    NeedsDocumentProtected( aI );
}

void WW8_WrPlcSepx::AppendSep( WW8_CP nStartCp, const SwFmtPageDesc& rPD,
    const SwNode& rNd, const SwSectionFmt* pSectionFmt, sal_uLong nLnNumRestartNo )
{
    if (HeaderFooterWritten()) {
        return; 
    }
    aCps.push_back( nStartCp );
    AppendSection( rPD, rNd, pSectionFmt, nLnNumRestartNo );
}





void MSWordSections::SetNum( const SwTxtNode* pNumNd )
{
    WW8_SepInfo& rInfo = aSects.back();
    if ( !rInfo.pNumNd ) 
        rInfo.pNumNd = pNumNd;
}

void WW8_WrPlcSepx::WriteFtnEndTxt( WW8Export& rWrt, sal_uLong nCpStt )
{
    sal_uInt8 nInfoFlags = 0;
    const SwFtnInfo& rInfo = rWrt.pDoc->GetFtnInfo();
    if( !rInfo.aErgoSum.isEmpty() )  nInfoFlags |= 0x02;
    if( !rInfo.aQuoVadis.isEmpty() ) nInfoFlags |= 0x04;

    sal_uInt8 nEmptyStt = rWrt.bWrtWW8 ? 0 : 6;
    if( nInfoFlags )
    {
        if( rWrt.bWrtWW8 )
            pTxtPos->Append( nCpStt );  

        if( 0x02 & nInfoFlags )         
        {
            pTxtPos->Append( nCpStt );
            rWrt.WriteStringAsPara( rInfo.aErgoSum );
            rWrt.WriteStringAsPara( OUString() );
            nCpStt = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        }
        else if( rWrt.bWrtWW8 )
            pTxtPos->Append( nCpStt );

        if( 0x04 & nInfoFlags )         
        {
            pTxtPos->Append( nCpStt );
            rWrt.WriteStringAsPara( rInfo.aQuoVadis );
            rWrt.WriteStringAsPara( OUString() );
            nCpStt = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        }
        else if( rWrt.bWrtWW8 )
            pTxtPos->Append( nCpStt );

        if( rWrt.bWrtWW8 )
            nEmptyStt = 3;
        else
            rWrt.pDop->grpfIhdt = nInfoFlags;
    }

    while( 6 > nEmptyStt++ )
        pTxtPos->Append( nCpStt );

    
    WW8Dop& rDop = *rWrt.pDop;
    
    switch( rInfo.eNum )
    {
    case FTNNUM_PAGE:       rDop.rncFtn = 2; break;
    case FTNNUM_CHAPTER:    rDop.rncFtn  = 1; break;
    default: rDop.rncFtn  = 0; break;
    }                                   
    rDop.nfcFtnRef = WW8Export::GetNumId( rInfo.aFmt.GetNumberingType() );
    rDop.nFtn = rInfo.nFtnOffset + 1;
    rDop.fpc = rWrt.bFtnAtTxtEnd ? 2 : 1;

    
    rDop.rncEdn = 0;                        
    const SwEndNoteInfo& rEndInfo = rWrt.pDoc->GetEndNoteInfo();
    rDop.nfcEdnRef = WW8Export::GetNumId( rEndInfo.aFmt.GetNumberingType() );
    rDop.nEdn = rEndInfo.nFtnOffset + 1;
    rDop.epc = rWrt.bEndAtTxtEnd ? 3 : 0;
}

void MSWordSections::SetHeaderFlag( sal_uInt8& rHeadFootFlags, const SwFmt& rFmt,
    sal_uInt8 nFlag )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rFmt.GetItemState(RES_HEADER, true, &pItem)
        && ((SwFmtHeader*)pItem)->IsActive() &&
        ((SwFmtHeader*)pItem)->GetHeaderFmt() )
        rHeadFootFlags |= nFlag;
}

void MSWordSections::SetFooterFlag( sal_uInt8& rHeadFootFlags, const SwFmt& rFmt,
    sal_uInt8 nFlag )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rFmt.GetItemState(RES_FOOTER, true, &pItem)
        && ((SwFmtFooter*)pItem)->IsActive() &&
        ((SwFmtFooter*)pItem)->GetFooterFmt() )
        rHeadFootFlags |= nFlag;
}

void WW8_WrPlcSepx::OutHeaderFooter( WW8Export& rWrt, bool bHeader,
                     const SwFmt& rFmt, sal_uLong& rCpPos, sal_uInt8 nHFFlags,
                     sal_uInt8 nFlag,  sal_uInt8 nBreakCode)
{
    if ( nFlag & nHFFlags )
    {
        pTxtPos->Append( rCpPos );
        rWrt.WriteHeaderFooterText( rFmt, bHeader);
        rWrt.WriteStringAsPara( OUString() ); 
        rCpPos = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    }
    else if ( rWrt.bWrtWW8 )
    {
        pTxtPos->Append( rCpPos );
        if ((bHeader? rWrt.bHasHdr : rWrt.bHasFtr) && nBreakCode!=0)
        {
            rWrt.WriteStringAsPara( OUString() ); 
            rWrt.WriteStringAsPara( OUString() ); 
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
         pSectionFmt &&
         ((SwSectionFmt*)0xFFFFFFFF != pSectionFmt)
       )
    {
        const SwSection *pSection = pSectionFmt->GetSection();
        if (pSection && pSection->IsProtect())
        {
            bRet = true;
        }
    }
    return bRet;
}


void MSWordSections::CheckForFacinPg( WW8Export& rWrt ) const
{
    
    
    
    std::vector<WW8_SepInfo>::const_iterator iter = aSects.begin();
    for( sal_uInt16 nEnde = 0; iter != aSects.end(); ++iter )
    {
        const WW8_SepInfo& rSepInfo = *iter;
        if( !rSepInfo.pSectionFmt )
        {
            const SwPageDesc* pPd = rSepInfo.pPageDesc;
            if( pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd->GetFollow() &&
                rSepInfo.pPDNd &&
                pPd->IsFollowNextPageOfNode( *rSepInfo.pPDNd ) )
                
                
                pPd = pPd->GetFollow();

            
            else if( !( 1 & nEnde ) &&
                pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd &&
                (( nsUseOnPage::PD_LEFT == ( nsUseOnPage::PD_ALL & pPd->ReadUseOn() ) &&
                   nsUseOnPage::PD_RIGHT == ( nsUseOnPage::PD_ALL & pPd->GetFollow()->ReadUseOn() )) ||
                 ( nsUseOnPage::PD_RIGHT == ( nsUseOnPage::PD_ALL & pPd->ReadUseOn() ) &&
                   nsUseOnPage::PD_LEFT == ( nsUseOnPage::PD_ALL & pPd->GetFollow()->ReadUseOn() )) ))
            {
                rWrt.pDop->fFacingPages = rWrt.pDop->fMirrorMargins = true;
                nEnde |= 1;
            }

            if( !( 1 & nEnde ) &&
                ( !pPd->IsHeaderShared() || !pPd->IsFooterShared() ))
            {
                rWrt.pDop->fFacingPages = true;
                nEnde |= 1;
            }
            if( !( 2 & nEnde ) &&
                nsUseOnPage::PD_MIRROR == ( nsUseOnPage::PD_MIRROR & pPd->ReadUseOn() ))
            {
                rWrt.pDop->fSwapBordersFacingPgs =
                    rWrt.pDop->fMirrorMargins = true;
                nEnde |= 2;
            }

            if( 3 == nEnde )
                break;      
        }
    }
}

bool MSWordSections::HasBorderItem( const SwFmt& rFmt )
{
    const SfxPoolItem* pItem;
    return SFX_ITEM_SET == rFmt.GetItemState(RES_BOX, true, &pItem) &&
            (   ((SvxBoxItem*)pItem)->GetTop() ||
                ((SvxBoxItem*)pItem)->GetBottom()  ||
                ((SvxBoxItem*)pItem)->GetLeft()  ||
                ((SvxBoxItem*)pItem)->GetRight() );
}

void WW8AttributeOutput::StartSection()
{
    m_rWW8Export.pO->clear();
}

void WW8AttributeOutput::SectionFormProtection( bool bProtected )
{
    //If the document is to be exported as protected, then if a segment
    //is not protected, set the unlocked flag
    if ( m_rWW8Export.pSepx->DocumentIsProtected() && !bProtected )
    {
        if ( m_rWW8Export.bWrtWW8 )
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SFProtected );
        else
            m_rWW8Export.pO->push_back( 139 );
        m_rWW8Export.pO->push_back( 1 );
    }
}

void WW8AttributeOutput::SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo )
{
    
    if ( m_rWW8Export.bWrtWW8 )
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SNLnnMod );
    else
        m_rWW8Export.pO->push_back( 154 );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, (sal_uInt16)rLnNumInfo.GetCountBy() );

    
    if ( m_rWW8Export.bWrtWW8 )
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SDxaLnn );
    else
        m_rWW8Export.pO->push_back( 155 );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, (sal_uInt16)rLnNumInfo.GetPosFromLeft() );

    
    if ( nRestartNo || !rLnNumInfo.IsRestartEachPage() )
    {
        if ( m_rWW8Export.bWrtWW8 )
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SLnc );
        else
            m_rWW8Export.pO->push_back( 152 );
        m_rWW8Export.pO->push_back( nRestartNo ? 1 : 2 );
    }

    
    if ( nRestartNo )
    {
        if ( m_rWW8Export.bWrtWW8 )
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SLnnMin );
        else
            m_rWW8Export.pO->push_back( 160 );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, (sal_uInt16)nRestartNo - 1 );
    }
}

void WW8AttributeOutput::SectionTitlePage()
{
    
    if ( m_rWW8Export.bWrtWW8 )
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SFTitlePage );
    else
        m_rWW8Export.pO->push_back( 143 );
    m_rWW8Export.pO->push_back( 1 );
}

void WW8AttributeOutput::SectionPageBorders( const SwFrmFmt* pPdFmt, const SwFrmFmt* pPdFirstPgFmt )
{
    if ( m_rWW8Export.bWrtWW8 )              
    {
        sal_uInt16 nPgBorder = MSWordSections::HasBorderItem( *pPdFmt ) ? 0 : USHRT_MAX;
        if ( pPdFmt != pPdFirstPgFmt )
        {
            if ( MSWordSections::HasBorderItem( *pPdFirstPgFmt ) )
            {
                if ( USHRT_MAX == nPgBorder )
                {
                    nPgBorder = 1;
                    
                    m_rWW8Export.pISet = &pPdFirstPgFmt->GetAttrSet();
                    OutputItem( pPdFirstPgFmt->GetFmtAttr( RES_BOX ) );
                }
            }
            else if ( !nPgBorder )
                nPgBorder = 2;
        }

        if ( USHRT_MAX != nPgBorder )
        {
            
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SPgbProp );
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, nPgBorder );
        }
    }
}

void WW8AttributeOutput::SectionBiDi( bool bBiDi )
{
    if ( m_rWW8Export.bWrtWW8 )
    {
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SFBiDi );
        m_rWW8Export.pO->push_back( bBiDi? 1: 0 );
    }
}

void WW8AttributeOutput::SectionPageNumbering( sal_uInt16 nNumType, ::boost::optional<sal_uInt16> oPageRestartNumber )
{
    
    sal_uInt8 nb = WW8Export::GetNumId( nNumType );
    if ( m_rWW8Export.bWrtWW8 )
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SNfcPgn );
    else
        m_rWW8Export.pO->push_back( 147 );
    m_rWW8Export.pO->push_back( nb );

    if ( oPageRestartNumber )
    {
        
        if ( m_rWW8Export.bWrtWW8 )
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SFPgnRestart );
        else
            m_rWW8Export.pO->push_back( 150 );
        m_rWW8Export.pO->push_back( 1 );

        
        if ( m_rWW8Export.bWrtWW8 )
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SPgnStart );
        else
            m_rWW8Export.pO->push_back( 161 );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, oPageRestartNumber.get() );
    }
}

void WW8AttributeOutput::SectionType( sal_uInt8 nBreakCode )
{
    if ( 2 != nBreakCode ) 
    {
        if ( m_rWW8Export.bWrtWW8 )
            SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_SBkc );
        else
            m_rWW8Export.pO->push_back( 142 );
        m_rWW8Export.pO->push_back( nBreakCode );
    }
}

void WW8AttributeOutput::SectionWW6HeaderFooterFlags( sal_uInt8 nHeadFootFlags )
{
    if ( nHeadFootFlags && !m_rWW8Export.bWrtWW8 )
    {
        sal_uInt8 nTmpFlags = nHeadFootFlags;
        if ( m_rWW8Export.pDop->fFacingPages )
        {
            if ( !(nTmpFlags & WW8_FOOTER_EVEN) && (nTmpFlags & WW8_FOOTER_ODD ) )
                nTmpFlags |= WW8_FOOTER_EVEN;

            if ( !(nTmpFlags & WW8_HEADER_EVEN) && (nTmpFlags & WW8_HEADER_ODD ) )
                nTmpFlags |= WW8_HEADER_EVEN;
        }

        
        m_rWW8Export.pO->push_back( 153 );
        m_rWW8Export.pO->push_back( nTmpFlags );
    }
}

void WW8Export::SetupSectionPositions( WW8_PdAttrDesc* pA )
{
    if ( !pA )
        return;

    if ( !pO->empty() ) 
    {
        pA->m_nLen = pO->size();
        pA->m_pData.reset(new sal_uInt8 [pO->size()]);
        
        memcpy( pA->m_pData.get(), pO->data(), pO->size() );
        pO->clear(); 
    }
    else 
    {
        pA->m_pData.reset();
        pA->m_nLen = 0;
    }
}

void WW8Export::WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
        const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt, sal_uInt8 nBreakCode )
{
    sal_uLong nCpPos = Fc2Cp( Strm().Tell() );

    IncrementHdFtIndex();
    if ( !(nHeadFootFlags & WW8_HEADER_EVEN) && pDop->fFacingPages )
        pSepx->OutHeaderFooter( *this, true, rFmt, nCpPos, nHeadFootFlags, WW8_HEADER_ODD, nBreakCode );
    else
        pSepx->OutHeaderFooter( *this, true, rLeftFmt, nCpPos, nHeadFootFlags, WW8_HEADER_EVEN, nBreakCode );
    IncrementHdFtIndex();
    pSepx->OutHeaderFooter( *this, true, rFmt, nCpPos, nHeadFootFlags, WW8_HEADER_ODD, nBreakCode );

    IncrementHdFtIndex();
    if ( !(nHeadFootFlags & WW8_FOOTER_EVEN) && pDop->fFacingPages )
        pSepx->OutHeaderFooter( *this, false, rFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_ODD, nBreakCode );
    else
        pSepx->OutHeaderFooter( *this, false, rLeftFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_EVEN, nBreakCode );
    IncrementHdFtIndex();
    pSepx->OutHeaderFooter( *this, false, rFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_ODD, nBreakCode );

    //#i24344# Drawing objects cannot be directly shared between main hd/ft
    //and title hd/ft so we need to differenciate them
    IncrementHdFtIndex();
    pSepx->OutHeaderFooter( *this, true, rFirstPageFmt, nCpPos, nHeadFootFlags, WW8_HEADER_FIRST, nBreakCode );
    pSepx->OutHeaderFooter( *this, false, rFirstPageFmt, nCpPos, nHeadFootFlags, WW8_FOOTER_FIRST, nBreakCode );
}

void MSWordExportBase::SectionProperties( const WW8_SepInfo& rSepInfo, WW8_PdAttrDesc* pA )
{
    const SwPageDesc* pPd = rSepInfo.pPageDesc;

    if ( rSepInfo.pSectionFmt && !pPd )
        pPd = &pDoc->GetPageDesc( 0 );

    pAktPageDesc = pPd;

    if ( !pPd )
        return;

    bool bOldPg = bOutPageDescs;
    bOutPageDescs = true;

    AttrOutput().StartSection();

    AttrOutput().SectFootnoteEndnotePr();

    
    AttrOutput().SectionFormProtection( rSepInfo.IsProtected() );

    
    const SwLineNumberInfo& rLnNumInfo = pDoc->GetLineNumberInfo();
    if ( rLnNumInfo.IsPaintLineNumbers() )
        AttrOutput().SectionLineNumbering( rSepInfo.nLnNumRestartNo, rLnNumInfo );

    /*  sprmSBkc, break code:   0 No break, 1 New column
        2 New page, 3 Even page, 4 Odd page
        */
    sal_uInt8 nBreakCode = 2;            
    bool bOutPgDscSet = true, bLeftRightPgChain = false, bOutputStyleItemSet = false;
    const SwFrmFmt* pPdFmt = &pPd->GetMaster();
    if ( rSepInfo.pSectionFmt )
    {
        
        
        
        nBreakCode = 0;         

        if ( rSepInfo.pPDNd && rSepInfo.pPDNd->IsCntntNode() )
        {
            if ( !NoPageBreakSection( &rSepInfo.pPDNd->GetCntntNode()->GetSwAttrSet() ) )
            {
                nBreakCode = 2;
            }
        }

        if ( (SwSectionFmt*)0xFFFFFFFF != rSepInfo.pSectionFmt )
        {
            if ( nBreakCode == 0 )
                bOutPgDscSet = false;

            
            
            
            const SfxItemSet* pPdSet = &pPdFmt->GetAttrSet();
            SfxItemSet aSet( *pPdSet->GetPool(), pPdSet->GetRanges() );
            aSet.SetParent( pPdSet );

            
            

            const SvxLRSpaceItem &rSectionLR =
                ItemGet<SvxLRSpaceItem>( *(rSepInfo.pSectionFmt), RES_LR_SPACE );
            const SvxLRSpaceItem &rPageLR =
                ItemGet<SvxLRSpaceItem>( *pPdFmt, RES_LR_SPACE );

            SvxLRSpaceItem aResultLR( rPageLR.GetLeft() +
                    rSectionLR.GetLeft(), rPageLR.GetRight() +
                    rSectionLR.GetRight(), 0, 0, RES_LR_SPACE );
            //i120133: The Section width should consider section indent value.
            if (rSectionLR.GetLeft()+rSectionLR.GetRight()!=0)
            {
                const SwFmtCol& rCol = dynamic_cast<const SwFmtCol&>(rSepInfo.pSectionFmt->GetFmtAttr(RES_COL));
                SwFmtCol aCol(rCol);
                aCol.SetAdjustValue(rSectionLR.GetLeft()+rSectionLR.GetRight());
                aSet.Put(aCol);
            }
            else
                aSet.Put(rSepInfo.pSectionFmt->GetFmtAttr(RES_COL));


            aSet.Put( aResultLR );

            
            const SfxItemSet* pOldI = pISet;
            pISet = &aSet;

            
            
            AttrOutput().OutputStyleItemSet( aSet, true, bOutPgDscSet );
            bOutputStyleItemSet = true;

            //Cannot export as normal page framedir, as continuous sections
            //cannot contain any grid settings like proper sections
            AttrOutput().SectionBiDi( FRMDIR_HORI_RIGHT_TOP == TrueFrameDirection( *rSepInfo.pSectionFmt ) );

            pISet = pOldI;
        }
    }

    
    
    
    
    
    
    
    const SwFrmFmt* pPdFirstPgFmt = &pPd->GetFirstMaster();
    bool titlePage = !pPd->IsFirstShared();
    if ( bOutPgDscSet )
    {
        
        
        
        
        
        
        
        if ( pPd->GetFollow() && pPd != pPd->GetFollow() &&
             pPd->GetFollow()->GetFollow() == pPd->GetFollow() &&
             ( !rSepInfo.pPDNd || pPd->IsFollowNextPageOfNode( *rSepInfo.pPDNd ) ) )
        {
            const SwPageDesc *pFollow = pPd->GetFollow();
            const SwFrmFmt& rFollowFmt = pFollow->GetMaster();
            if ( sw::util::IsPlausableSingleWordSection( *pPdFirstPgFmt, rFollowFmt ) || titlePage )
            {
                if (rSepInfo.pPDNd)
                    pPdFirstPgFmt = pPd->GetPageFmtOfNode( *rSepInfo.pPDNd );
                else
                    pPdFirstPgFmt = &pPd->GetMaster();

                pAktPageDesc = pPd = pFollow;
                pPdFmt = &rFollowFmt;

                
                titlePage = true;
            }
        }

        if( titlePage )
            AttrOutput().SectionTitlePage();

        const SfxItemSet* pOldI = pISet;

        const SfxPoolItem* pItem;
        if ( titlePage && SFX_ITEM_SET ==
                pPdFirstPgFmt->GetItemState( RES_PAPER_BIN, true, &pItem ) )
        {
            pISet = &pPdFirstPgFmt->GetAttrSet();
            bOutFirstPage = true;
            AttrOutput().OutputItem( *pItem );
            bOutFirstPage = false;
        }

        
        if ( pPd->GetFollow() && pPd != pPd->GetFollow() &&
                pPd->GetFollow()->GetFollow() == pPd &&
                (( nsUseOnPage::PD_LEFT == ( nsUseOnPage::PD_ALL & pPd->ReadUseOn() ) &&
                   nsUseOnPage::PD_RIGHT == ( nsUseOnPage::PD_ALL & pPd->GetFollow()->ReadUseOn() )) ||
                 ( nsUseOnPage::PD_RIGHT == ( nsUseOnPage::PD_ALL & pPd->ReadUseOn() ) &&
                   nsUseOnPage::PD_LEFT == ( nsUseOnPage::PD_ALL & pPd->GetFollow()->ReadUseOn() )) ))
        {
            bLeftRightPgChain = true;

            
            
            if ( nsUseOnPage::PD_LEFT == ( nsUseOnPage::PD_ALL & pPd->ReadUseOn() ) )
            {
                nBreakCode = 3;
                pPd = pPd->GetFollow();
                pPdFmt = &pPd->GetMaster();
            }
            else
                nBreakCode = 4;
        }

        pISet = &pPdFmt->GetAttrSet();
        if (!bOutputStyleItemSet)
            AttrOutput().OutputStyleItemSet( pPdFmt->GetAttrSet(), true, false );
        AttrOutput().SectionPageBorders( pPdFmt, pPdFirstPgFmt );
        pISet = pOldI;

        
        AttrOutput().SectionPageNumbering( pPd->GetNumType().GetNumberingType(), rSepInfo.oPgRestartNo );

        
        if ( 2 == nBreakCode )
        {
            if ( nsUseOnPage::PD_LEFT == ( nsUseOnPage::PD_ALL & pPd->ReadUseOn() ) )
                nBreakCode = 3;
            else if ( nsUseOnPage::PD_RIGHT == ( nsUseOnPage::PD_ALL & pPd->ReadUseOn() ) )
                nBreakCode = 4;
        }
    }

    AttrOutput().SectionType( nBreakCode );

    const SwTxtNode* pNd = rSepInfo.pNumNd;
    if ( pNd )
    {
        const SwNumRule* pRule = pNd->GetNumRule();
        if ( pRule )
            OutputOlst( *pRule );
    }

    
    sal_uInt8 nHeadFootFlags = 0;

    const SwFrmFmt* pPdLeftFmt = bLeftRightPgChain
        ? &pPd->GetFollow()->GetMaster()
        : &pPd->GetLeft();

    if ( nBreakCode != 0 )
    {
        if ( titlePage )
        {
            
            MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdFirstPgFmt, WW8_HEADER_FIRST );
            MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdFirstPgFmt, WW8_FOOTER_FIRST );
        }
        MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdFmt, WW8_HEADER_ODD );
        MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdFmt, WW8_FOOTER_ODD );

        if ( !pPd->IsHeaderShared() || bLeftRightPgChain )
            MSWordSections::SetHeaderFlag( nHeadFootFlags, *pPdLeftFmt, WW8_HEADER_EVEN );

        if ( !pPd->IsFooterShared() || bLeftRightPgChain )
            MSWordSections::SetFooterFlag( nHeadFootFlags, *pPdLeftFmt, WW8_FOOTER_EVEN );
        AttrOutput().SectionWW6HeaderFooterFlags( nHeadFootFlags );
    }

    
    SetupSectionPositions( pA );

    /*
       !!!!!!!!!!!
    
    
    
    const SwFrmFmt* pFFmt = rFt.GetFooterFmt();
    const SvxBoxItem& rBox = pFFmt->GetBox(false);
    OutWW8_SwFmtBox1( m_rWW8Export.pOut, rBox, false);
    !!!!!!!!!!!
    You can turn this into paragraph attributes, which are then observed in each paragraph.
    Applies to background / border.
    !!!!!!!!!!!
    */

    const SwTxtNode *pOldPageRoot = GetHdFtPageRoot();
    SetHdFtPageRoot( rSepInfo.pPDNd ? rSepInfo.pPDNd->GetTxtNode() : 0 );

    WriteHeadersFooters( nHeadFootFlags, *pPdFmt, *pPdLeftFmt, *pPdFirstPgFmt, nBreakCode );

    SetHdFtPageRoot( pOldPageRoot );

    AttrOutput().EndSection();

    
    bOutPageDescs = bOldPg;
}

bool WW8_WrPlcSepx::WriteKFTxt( WW8Export& rWrt )
{
    sal_uLong nCpStart = rWrt.Fc2Cp( rWrt.Strm().Tell() );

    OSL_ENSURE( !pTxtPos, "who set the pointer?" );
    pTxtPos = new WW8_WrPlc0( nCpStart );

    WriteFtnEndTxt( rWrt, nCpStart );
    CheckForFacinPg( rWrt );

    unsigned int nOldIndex = rWrt.GetHdFtIndex();
    rWrt.SetHdFtIndex( 0 );

    for ( sal_uInt16 i = 0; i < aSects.size(); ++i )
    {
        ::boost::shared_ptr<WW8_PdAttrDesc> const pAttrDesc(new WW8_PdAttrDesc);
        m_SectionAttributes.push_back(pAttrDesc);

        WW8_SepInfo& rSepInfo = aSects[i];
        rWrt.SectionProperties( rSepInfo, pAttrDesc.get() );

        
        
        
        m_bHeaderFooterWritten = true;
    }
    rWrt.SetHdFtIndex( nOldIndex ); //0

    if ( pTxtPos->Count() )
    {
        
        sal_uLong nCpEnd = rWrt.Fc2Cp( rWrt.Strm().Tell() );
        pTxtPos->Append( nCpEnd );  

        if ( nCpEnd > nCpStart )
        {
            ++nCpEnd;
            pTxtPos->Append( nCpEnd + 1 );  

            rWrt.WriteStringAsPara( OUString() ); 
        }
        rWrt.pFldHdFt->Finish( nCpEnd, rWrt.pFib->ccpText + rWrt.pFib->ccpFtn );
        rWrt.pFib->ccpHdr = nCpEnd - nCpStart;
    }
    else
        delete pTxtPos, pTxtPos = 0;

    return rWrt.pFib->ccpHdr != 0;
}

void WW8_WrPlcSepx::WriteSepx( SvStream& rStrm ) const
{
    OSL_ENSURE(m_SectionAttributes.size() == static_cast<size_t>(aSects.size())
        , "WriteSepx(): arrays out of sync!");
    for (size_t i = 0; i < m_SectionAttributes.size(); i++) 
    {
        WW8_PdAttrDesc *const pA = m_SectionAttributes[i].get();
        if (pA->m_nLen && pA->m_pData != 0)
        {
            SVBT16 nL;
            pA->m_nSepxFcPos = rStrm.Tell();
            ShortToSVBT16( pA->m_nLen, nL );
            rStrm.Write( nL, 2 );
            rStrm.Write( pA->m_pData.get(), pA->m_nLen );
        }
    }
}

void WW8_WrPlcSepx::WritePlcSed( WW8Export& rWrt ) const
{
    OSL_ENSURE(m_SectionAttributes.size() == static_cast<size_t>(aSects.size())
        , "WritePlcSed(): arrays out of sync!");
    OSL_ENSURE( aCps.size() == aSects.size() + 1, "WrPlcSepx: DeSync" );
    sal_uLong nFcStart = rWrt.pTableStrm->Tell();

    sal_uInt16 i;
    for( i = 0; i <= aSects.size(); i++ )
    {
        sal_uInt32 nP = aCps[i];
        SVBT32 nPos;
        UInt32ToSVBT32( nP, nPos );
        rWrt.pTableStrm->Write( nPos, 4 );
    }

    static WW8_SED aSed = {{4, 0},{0, 0, 0, 0},{0, 0},{0xff, 0xff, 0xff, 0xff}};

    for (size_t j = 0; j < m_SectionAttributes.size(); j++ )
    {
        
        UInt32ToSVBT32( m_SectionAttributes[j]->m_nSepxFcPos, aSed.fcSepx );
        rWrt.pTableStrm->Write( &aSed, sizeof( aSed ) );
    }
    rWrt.pFib->fcPlcfsed = nFcStart;
    rWrt.pFib->lcbPlcfsed = rWrt.pTableStrm->Tell() - nFcStart;
}


void WW8_WrPlcSepx::WritePlcHdd( WW8Export& rWrt ) const
{
    
    if( rWrt.pFib->ccpHdr != 0 && pTxtPos && pTxtPos->Count() )
    {
        rWrt.pFib->fcPlcfhdd = rWrt.pTableStrm->Tell();
        pTxtPos->Write( *rWrt.pTableStrm );             
        rWrt.pFib->lcbPlcfhdd = rWrt.pTableStrm->Tell() -
                                rWrt.pFib->fcPlcfhdd;
    }
}

void MSWordExportBase::WriteHeaderFooterText( const SwFmt& rFmt, bool bHeader )
{
    const SwFmtCntnt *pCntnt;
    if ( bHeader )
    {
        bHasHdr = true;
        const SwFmtHeader& rHd = rFmt.GetHeader();
        OSL_ENSURE( rHd.GetHeaderFmt(), "Header text is not here" );
        pCntnt = &rHd.GetHeaderFmt()->GetCntnt();
    }
    else
    {
        bHasFtr = true;
        const SwFmtFooter& rFt = rFmt.GetFooter();
        OSL_ENSURE( rFt.GetFooterFmt(), "Footer text is not here" );
        pCntnt = &rFt.GetFooterFmt()->GetCntnt();
    }

    const SwNodeIndex* pSttIdx = pCntnt->GetCntntIdx();

    if ( pSttIdx )
    {
        SwNodeIndex aIdx( *pSttIdx, 1 ),
        aEnd( *pSttIdx->GetNode().EndOfSectionNode() );
        sal_uLong nStart = aIdx.GetIndex();
        sal_uLong nEnd = aEnd.GetIndex();

        
        if ( nStart < nEnd )
        {
            bool bOldKF = bOutKF;
            bOutKF = true;
            WriteSpecialText( nStart, nEnd, TXT_HDFT );
            bOutKF = bOldKF;
        }
        else
            pSttIdx = 0;
    }

    if ( !pSttIdx )
    {
        
        OSL_ENSURE( pSttIdx, "Header/Footer text is not really present" );
        AttrOutput().EmptyParagraph(); 
    }
}






WW8_WrPlcSubDoc::WW8_WrPlcSubDoc()
    : pTxtPos( 0 )
{
}

WW8_WrPlcSubDoc::~WW8_WrPlcSubDoc()
{
    delete pTxtPos;
}

void WW8_WrPlcFtnEdn::Append( WW8_CP nCp, const SwFmtFtn& rFtn )
{
    aCps.push_back( nCp );
    aCntnt.push_back( &rFtn );
}

WW8_Annotation::WW8_Annotation(const SwPostItField* pPostIt, WW8_CP nRangeStart, WW8_CP nRangeEnd)
    :
        maDateTime( DateTime::EMPTY ),
        m_nRangeStart(nRangeStart),
        m_nRangeEnd(nRangeEnd)
{
    mpRichText = pPostIt->GetTextObject();
    if (!mpRichText)
        msSimpleText = pPostIt->GetTxt();
    msOwner = pPostIt->GetPar1();
    m_sInitials = pPostIt->GetInitials();
    maDateTime = DateTime(pPostIt->GetDate(), pPostIt->GetTime());
}

WW8_Annotation::WW8_Annotation(const SwRedlineData* pRedline)
    :
        mpRichText(0),
        maDateTime( DateTime::EMPTY ),
        m_nRangeStart(0),
        m_nRangeEnd(0)
{
    msSimpleText = pRedline->GetComment();
    msOwner = SW_MOD()->GetRedlineAuthor(pRedline->GetAuthor());
    maDateTime = pRedline->GetTimeStamp();
}

void WW8_WrPlcAnnotations::AddRangeStartPosition(const OUString& rName, WW8_CP nStartCp)
{
    m_aRangeStartPositions[rName] = nStartCp;
}

void WW8_WrPlcAnnotations::Append( WW8_CP nCp, const SwPostItField *pPostIt )
{
    aCps.push_back( nCp );
    WW8_Annotation* p;
    if( m_aRangeStartPositions.find(pPostIt->GetName()) != m_aRangeStartPositions.end() )
    {
        p = new WW8_Annotation(pPostIt, m_aRangeStartPositions[pPostIt->GetName()], nCp);
        m_aRangeStartPositions.erase(pPostIt->GetName());
    }
    else
    {
        p = new WW8_Annotation(pPostIt, nCp, nCp);
    }
    aCntnt.push_back( p );
}

void WW8_WrPlcAnnotations::Append( WW8_CP nCp, const SwRedlineData *pRedline )
{
    maProcessedRedlines.insert(pRedline);
    aCps.push_back( nCp );
    WW8_Annotation* p = new WW8_Annotation(pRedline);
    aCntnt.push_back( p );
}

bool WW8_WrPlcAnnotations::IsNewRedlineComment( const SwRedlineData *pRedline )
{
    return maProcessedRedlines.find(pRedline) == maProcessedRedlines.end();
}

WW8_WrPlcAnnotations::~WW8_WrPlcAnnotations()
{
    for( sal_uInt16 n=0; n < aCntnt.size(); n++ )
        delete (WW8_Annotation*)aCntnt[n];
}

bool WW8_WrPlcSubDoc::WriteGenericTxt( WW8Export& rWrt, sal_uInt8 nTTyp,
    WW8_CP& rCount )
{
    sal_uInt16 nLen = aCntnt.size();
    if ( !nLen )
        return false;

    sal_uLong nCpStart = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    pTxtPos = new WW8_WrPlc0( nCpStart );
    sal_uInt16 i;

    switch ( nTTyp )
    {
        case TXT_ATN:
            for ( i = 0; i < nLen; i++ )
            {
                
                pTxtPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));

                rWrt.WritePostItBegin();
                const WW8_Annotation& rAtn = *(const WW8_Annotation*)aCntnt[i];
                if (rAtn.mpRichText)
                    rWrt.WriteOutliner(*rAtn.mpRichText, nTTyp);
                else
                {
                    OUString sTxt(rAtn.msSimpleText);
                    rWrt.WriteStringAsPara(sTxt.replace(0x0A, 0x0B));
                }
            }
            break;

        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            for ( i = 0; i < nLen; i++ )
            {
                
                WW8_CP nCP = rWrt.Fc2Cp( rWrt.Strm().Tell() );
                aCps.insert( aCps.begin()+i, nCP );
                pTxtPos->Append( nCP );

                if( aCntnt[ i ] != NULL )
                {
                
                const SdrObject& rObj = *(SdrObject*)aCntnt[ i ];
                if (rObj.GetObjInventor() == FmFormInventor)
                {
                    sal_uInt8 nOldTyp = rWrt.nTxtTyp;
                    rWrt.nTxtTyp = nTTyp;
                    rWrt.GetOCXExp().ExportControl(rWrt,&rObj);
                    rWrt.nTxtTyp = nOldTyp;
                }
                else if( rObj.ISA( SdrTextObj ) )
                    rWrt.WriteSdrTextObj(rObj, nTTyp);
                else
                {
                    const SwFrmFmt* pFmt = ::FindFrmFmt( &rObj );
                    OSL_ENSURE( pFmt, "where is the format?" );

                    const SwNodeIndex* pNdIdx = pFmt->GetCntnt().GetCntntIdx();
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
                                
                                
                                
                                rWrt.WriteStringAsPara( OUString(" ") );
                            }
                        }
                    }
                }
                }
                else if( i < aSpareFmts.size() )
                {
                    if( const SwFrmFmt* pFmt = (const SwFrmFmt*)aSpareFmts[ i ] )
                    {
                        const SwNodeIndex* pNdIdx = pFmt->GetCntnt().GetCntntIdx();
                        rWrt.WriteSpecialText( pNdIdx->GetIndex() + 1,
                                   pNdIdx->GetNode().EndOfSectionIndex(), nTTyp );
                    }
                }

                
                rWrt.WriteStringAsPara( OUString() );
            }
            break;

        case TXT_EDN:
        case TXT_FTN:
            for ( i = 0; i < nLen; i++ )
            {
                
                pTxtPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));

                
                const SwFmtFtn* pFtn = (SwFmtFtn*)aCntnt[ i ];
                rWrt.WriteFtnBegin( *pFtn );
                const SwNodeIndex* pIdx = pFtn->GetTxtFtn()->GetStartNode();
                OSL_ENSURE( pIdx, "wo ist der StartNode der Fuss-/EndNote?" );
                rWrt.WriteSpecialText( pIdx->GetIndex() + 1,
                                       pIdx->GetNode().EndOfSectionIndex(),
                                       nTTyp );
            }
            break;

        default:
            OSL_ENSURE( !this, "was ist das fuer ein SubDocType?" );
    }

    pTxtPos->Append( rWrt.Fc2Cp( rWrt.Strm().Tell() ));
    
    rWrt.WriteStringAsPara( OUString() );

    WW8_CP nCpEnd = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    pTxtPos->Append( nCpEnd );
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
    WW8_FC& rTxtStart, sal_Int32& rTxtCount, WW8_FC& rRefStart, sal_Int32& rRefCount ) const
{

    sal_uLong nFcStart = rWrt.pTableStrm->Tell();
    sal_uInt16 nLen = aCps.size();
    if ( !nLen )
        return;

    OSL_ENSURE( aCps.size() + 2 == pTxtPos->Count(), "WritePlc: DeSync" );

    ::std::vector<std::pair<OUString,OUString> > aStrArr;
    typedef ::std::vector<std::pair<OUString,OUString> >::iterator myiter;
    WW8Fib& rFib = *rWrt.pFib;              
    sal_uInt16 i;
    bool bWriteCP = true;

    switch ( nTTyp )
    {
        case TXT_ATN:
            {
                std::vector< std::pair<WW8_CP, int> > aRangeStartPos; 
                std::vector< std::pair<WW8_CP, int> > aRangeEndPos; 
                std::map<int, int> aAtnStartMap; 
                std::map<int, int> aStartEndMap; 
                
                for ( i = 0; i < nLen; ++i )
                {
                    const WW8_Annotation& rAtn = *(const WW8_Annotation*)aCntnt[i];
                    aStrArr.push_back(std::pair<OUString,OUString>(rAtn.msOwner,rAtn.m_sInitials));
                    if( rAtn.m_nRangeStart != rAtn.m_nRangeEnd )
                    {
                        aRangeStartPos.push_back(std::make_pair(rAtn.m_nRangeStart, i));
                        aRangeEndPos.push_back(std::make_pair(rAtn.m_nRangeEnd, i));
                    }
                }

                //sort and remove duplicates
                ::std::sort(aStrArr.begin(), aStrArr.end(),&lcl_AuthorComp);
                myiter aIter = ::std::unique(aStrArr.begin(), aStrArr.end());
                aStrArr.erase(aIter, aStrArr.end());

                
                
                
                
                std::sort(aRangeStartPos.begin(), aRangeStartPos.end(), &lcl_PosComp);
                for (i = 0; i < aRangeStartPos.size(); ++i)
                    aAtnStartMap[aRangeStartPos[i].second] = i;
                std::sort(aRangeEndPos.begin(), aRangeEndPos.end(), &lcl_PosComp);
                for (i = 0; i < aRangeEndPos.size(); ++i)
                    aStartEndMap[aRangeEndPos[ aAtnStartMap[i] ].second] = i;

                if ( rWrt.bWrtWW8 )
                {
                    for ( i = 0; i < aStrArr.size(); ++i )
                    {
                        const OUString& sAuthor = aStrArr[i].first;
                        SwWW8Writer::WriteShort(*rWrt.pTableStrm, sAuthor.getLength());
                        SwWW8Writer::WriteString16(*rWrt.pTableStrm, sAuthor,
                                false);
                    }
                }
                else
                {
                    for ( i = 0; i < aStrArr.size(); ++i )
                    {
                        const OUString& sAuthor = aStrArr[i].first;
                        rWrt.pTableStrm->WriteUChar( (sal_uInt8)sAuthor.getLength() );
                        SwWW8Writer::WriteString8(*rWrt.pTableStrm, sAuthor, false,
                                RTL_TEXTENCODING_MS_1252);
                    }
                }

                rFib.fcGrpStAtnOwners = nFcStart;
                nFcStart = rWrt.pTableStrm->Tell();
                rFib.lcbGrpStAtnOwners = nFcStart - rFib.fcGrpStAtnOwners;

                
                if ( rWrt.bWrtWW8 )
                {
                    if( aRangeStartPos.size() > 0 )
                    {
                        
                        rFib.fcPlcfAtnbkf = nFcStart;
                        for ( i = 0; i < aRangeStartPos.size(); ++i )
                        {
                            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRangeStartPos[i].first );
                        }
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRangeStartPos[i-1].first + 1);

                        
                        for ( i = 0; i < aRangeStartPos.size(); ++i )
                        {
                            SwWW8Writer::WriteShort( *rWrt.pTableStrm, aStartEndMap[i] ); 
                            SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 ); 
                        }

                        nFcStart = rWrt.pTableStrm->Tell();
                        rFib.lcbPlcfAtnbkf = nFcStart - rFib.fcPlcfAtnbkf;

                        
                        rFib.fcPlcfAtnbkl = nFcStart;
                        for ( i = 0; i < aRangeEndPos.size(); ++i )
                        {
                            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRangeEndPos[i].first );
                        }
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, aRangeEndPos[i-1].first + 1);

                        nFcStart = rWrt.pTableStrm->Tell();
                        rFib.lcbPlcfAtnbkl = nFcStart - rFib.fcPlcfAtnbkl;

                        
                        rFib.fcSttbfAtnbkmk = nFcStart;
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, (sal_Int16)(sal_uInt16)0xFFFF ); 
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, aRangeStartPos.size() ); 
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0xA );                   

                        for ( i = 0; i < aRangeStartPos.size(); ++i )
                        {
                            SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );         
                            
                            SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0x0100 );    
                            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aAtnStartMap[i] );          
                            SwWW8Writer::WriteLong( *rWrt.pTableStrm, -1 );         
                        }

                        nFcStart = rWrt.pTableStrm->Tell();
                        rFib.lcbSttbfAtnbkmk = nFcStart - rFib.fcSttbfAtnbkmk;
                    }
                }

                
                if( rWrt.bWrtWW8 )
                {
                    for( i = 0; i < nLen; ++i )
                    {
                        const WW8_Annotation& rAtn = *(const WW8_Annotation*)aCntnt[i];

                        sal_uInt32 nDTTM = sw::ms::DateTime2DTTM(rAtn.maDateTime);

                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, nDTTM );
                        SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                        SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                    }

                    rFib.fcAtrdExtra = nFcStart;
                    nFcStart = rWrt.pTableStrm->Tell();
                    rFib.lcbAtrdExtra = nFcStart - rFib.fcAtrdExtra;
                    rFib.fcHplxsdr = 0x01010002;  //WTF, but apparently necessary
                    rFib.lcbHplxsdr = 0;
                }
            }
            break;
        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            {
                pTxtPos->Write( *rWrt.pTableStrm );
                const std::vector<sal_uInt32>* pShapeIds = GetShapeIdArr();
                OSL_ENSURE( pShapeIds, "Where are the ShapeIds?" );

                for ( i = 0; i < nLen; ++i )
                {
                    
                    
                    const SdrObject* pObj = (SdrObject*)aCntnt[ i ];
                    sal_Int32 nCnt = 1;
                    if (pObj && !pObj->ISA( SdrTextObj ) )
                    {
                        
                        const SwFrmFmt& rFmt = *::FindFrmFmt( pObj );

                        const SwFmtChain* pChn = &rFmt.GetChain();
                        while ( pChn->GetNext() )
                        {
                            
                            
                            ++nCnt;
                            pChn = &pChn->GetNext()->GetChain();
                        }
                    }
                    if( NULL == pObj )
                    {
                        if( i < aSpareFmts.size() && aSpareFmts[ i ] )
                        {
                            const SwFrmFmt& rFmt = *(const SwFrmFmt*)aSpareFmts[ i ];

                            const SwFmtChain* pChn = &rFmt.GetChain();
                            while( pChn->GetNext() )
                            {
                                
                                
                                ++nCnt;
                                pChn = &pChn->GetNext()->GetChain();
                            }
                        }
                    }
                    
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, nCnt );
                    
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                    
                    SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                    
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, -1 );
                    
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                            (*pShapeIds)[i]);
                    
                    SwWW8Writer::WriteLong( *rWrt.pTableStrm, 0 );
                }
                SwWW8Writer::FillCount( *rWrt.pTableStrm, 22 );
                bWriteCP = false;
            }
            break;
    }

    if ( bWriteCP )
    {
        
        for ( i = 0; i < nLen; i++ )
            SwWW8Writer::WriteLong( *rWrt.pTableStrm, aCps[ i ] );

        
        SwWW8Writer::WriteLong( *rWrt.pTableStrm,
                rFib.ccpText + rFib.ccpFtn + rFib.ccpHdr + rFib.ccpEdn +
                rFib.ccpTxbx + rFib.ccpHdrTxbx + 1 );

        if ( TXT_ATN == nTTyp )
        {
            sal_uInt16 nlTag = 0;
            for ( i = 0; i < nLen; ++i )
            {
                const WW8_Annotation& rAtn = *(const WW8_Annotation*)aCntnt[i];

                //aStrArr is sorted
                myiter aIter = ::std::lower_bound(aStrArr.begin(),
                        aStrArr.end(), std::pair<OUString,OUString>(rAtn.msOwner,OUString()),
                        &lcl_AuthorComp);
                OSL_ENSURE(aIter != aStrArr.end() && aIter->first == rAtn.msOwner,
                        "Impossible");
                sal_uInt16 nFndPos = static_cast< sal_uInt16 >(aIter - aStrArr.begin());
                OUString sInitials( aIter->second );
                sal_uInt8 nInitialsLen = (sal_uInt8)sInitials.getLength();
                if ( nInitialsLen > 9 )
                {
                    sInitials = sInitials.copy( 0, 9 );
                    nInitialsLen = 9;
                }

                
                
                if ( rWrt.bWrtWW8 )
                {
                    SwWW8Writer::WriteShort(*rWrt.pTableStrm, nInitialsLen);
                    SwWW8Writer::WriteString16(*rWrt.pTableStrm, sInitials,
                            false);
                    SwWW8Writer::FillCount( *rWrt.pTableStrm,
                            (9 - nInitialsLen) * 2 );

                }
                else
                {
                    rWrt.pTableStrm->WriteUChar( nInitialsLen );
                    SwWW8Writer::WriteString8(*rWrt.pTableStrm, sInitials,
                            false, RTL_TEXTENCODING_MS_1252);
                    SwWW8Writer::FillCount(*rWrt.pTableStrm, 9 - nInitialsLen);
                }

                
                //
                
                
                
                

                SwWW8Writer::WriteShort( *rWrt.pTableStrm, nFndPos );
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                if( rAtn.m_nRangeStart != rAtn.m_nRangeEnd )
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
            for ( i = 0; i < nLen; ++i )             
            {
                const SwFmtFtn* pFtn = (SwFmtFtn*)aCntnt[ i ];
                SwWW8Writer::WriteShort( *rWrt.pTableStrm,
                        !pFtn->GetNumStr().isEmpty() ? 0 : ++nNo );
            }
        }
    }
    rRefStart = nFcStart;
    nFcStart = rWrt.pTableStrm->Tell();
    rRefCount = nFcStart - rRefStart;

    pTxtPos->Write( *rWrt.pTableStrm );

    switch ( nTTyp )
    {
        case TXT_TXTBOX:
        case TXT_HFTXTBOX:
            for ( i = 0; i < nLen; ++i )
            {
                
                
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, i );
                
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0 );
                
                
                SwWW8Writer::WriteShort( *rWrt.pTableStrm, 0x800 );
            }
            SwWW8Writer::FillCount( *rWrt.pTableStrm, 6 );
            break;
    }

    rTxtStart = nFcStart;
    rTxtCount = rWrt.pTableStrm->Tell() - nFcStart;
}

const std::vector<sal_uInt32>* WW8_WrPlcSubDoc::GetShapeIdArr() const
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
