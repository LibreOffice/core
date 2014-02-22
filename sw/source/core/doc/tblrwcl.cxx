/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <hintids.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/boxitem.hxx>
#include <tools/fract.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <doc.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <frmtool.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <tblsel.hxx>
#include <fldbas.hxx>
#include <swundo.hxx>
#include <rowfrm.hxx>
#include <ddefld.hxx>
#include <hints.hxx>
#include <UndoTable.hxx>
#include <cellatr.hxx>
#include <mvsave.hxx>
#include <swtblfmt.hxx>
#include <swddetbl.hxx>
#include <poolfmt.hxx>
#include <tblrwcl.hxx>
#include <unochart.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>
#include <switerator.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

#define COLFUZZY 20
#define ROWFUZZY 10

using namespace ::com::sun::star;

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

typedef std::map<SwTableLine*, sal_uInt16> SwTableLineWidthMap_t;



struct _CpyTabFrm
{
    union {
        SwTableBoxFmt *pFrmFmt;     
        SwTwips nSize;              
    } Value;
    SwTableBoxFmt *pNewFrmFmt;

    _CpyTabFrm( SwTableBoxFmt* pAktFrmFmt ) : pNewFrmFmt( 0 )
    {   Value.pFrmFmt = pAktFrmFmt; }

    _CpyTabFrm& operator=( const _CpyTabFrm& );

    bool operator==( const _CpyTabFrm& rCpyTabFrm ) const
        { return  (sal_uLong)Value.nSize == (sal_uLong)rCpyTabFrm.Value.nSize; }
    bool operator<( const _CpyTabFrm& rCpyTabFrm ) const
        { return  (sal_uLong)Value.nSize < (sal_uLong)rCpyTabFrm.Value.nSize; }
};

struct CR_SetBoxWidth
{
    SwSelBoxes m_Boxes;
    SwTableLineWidthMap_t m_LineWidthMap;
    SwShareBoxFmts aShareFmts;
    SwTableNode* pTblNd;
    SwUndoTblNdsChg* pUndo;
    SwTwips nDiff, nSide, nMaxSize, nLowerDiff;
    TblChgMode nMode;
    sal_uInt16 nTblWidth, nRemainWidth, nBoxWidth;
    bool bBigger, bLeft, bSplittBox, bAnyBoxFnd;

    CR_SetBoxWidth( sal_uInt16 eType, SwTwips nDif, SwTwips nSid, SwTwips nTblW,
                    SwTwips nMax, SwTableNode* pTNd )
        : pTblNd( pTNd ), pUndo( 0 ),
        nDiff( nDif ), nSide( nSid ), nMaxSize( nMax ), nLowerDiff( 0 ),
        nTblWidth( (sal_uInt16)nTblW ), nRemainWidth( 0 ), nBoxWidth( 0 ),
        bSplittBox( false ), bAnyBoxFnd( false )
    {
        bLeft = nsTblChgWidthHeightType::WH_COL_LEFT == ( eType & 0xff ) ||
                nsTblChgWidthHeightType::WH_CELL_LEFT == ( eType & 0xff );
        bBigger = 0 != (eType & nsTblChgWidthHeightType::WH_FLAG_BIGGER );
        nMode = pTblNd->GetTable().GetTblChgMode();
    }
    CR_SetBoxWidth( const CR_SetBoxWidth& rCpy )
        : m_LineWidthMap(rCpy.m_LineWidthMap)
        ,
        pTblNd( rCpy.pTblNd ),
        pUndo( rCpy.pUndo ),
        nDiff( rCpy.nDiff ), nSide( rCpy.nSide ),
        nMaxSize( rCpy.nMaxSize ), nLowerDiff( 0 ),
        nMode( rCpy.nMode ), nTblWidth( rCpy.nTblWidth ),
        nRemainWidth( rCpy.nRemainWidth ), nBoxWidth( rCpy.nBoxWidth ),
        bBigger( rCpy.bBigger ), bLeft( rCpy.bLeft ),
        bSplittBox( rCpy.bSplittBox ), bAnyBoxFnd( rCpy.bAnyBoxFnd )
    {
    }

    SwUndoTblNdsChg* CreateUndo( SwUndoId eUndoType )
    {
        return pUndo = new SwUndoTblNdsChg( eUndoType, m_Boxes, *pTblNd );
    }

    void LoopClear()
    {
        nLowerDiff = 0; nRemainWidth = 0;
    }

    void AddBoxWidth( const SwTableBox& rBox, sal_uInt16 nWidth )
    {
        SwTableLine* p = (SwTableLine*)rBox.GetUpper();
        std::pair<SwTableLineWidthMap_t::iterator, bool> aPair =
            m_LineWidthMap.insert(std::make_pair(p,nWidth));
        if (!aPair.second)
        {
            aPair.first->second += nWidth;
        }
    }

    sal_uInt16 GetBoxWidth( const SwTableLine& rLn ) const
    {
        SwTableLine* p = (SwTableLine*)&rLn;
        SwTableLineWidthMap_t::const_iterator const it = m_LineWidthMap.find(p);
        return (it != m_LineWidthMap.end()) ? it->second : 0;
    }
};

static bool lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, bool bCheck );
static bool lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_DelSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_DelOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck );

typedef bool (*FN_lcl_SetBoxWidth)(SwTableLine*, CR_SetBoxWidth&, SwTwips, bool );

#ifdef DBG_UTIL

void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize );

#define CHECKBOXWIDTH                                           \
    {                                                           \
        SwTwips nSize = GetFrmFmt()->GetFrmSize().GetWidth();   \
        for (size_t nTmp = 0; nTmp < aLines.size(); ++nTmp)   \
            ::_CheckBoxWidth( *aLines[ nTmp ], nSize );         \
    }

#define CHECKTABLELAYOUT                                            \
    {                                                               \
        for ( sal_uInt16 i = 0; i < GetTabLines().size(); ++i )        \
        {                                                           \
            SwFrmFmt* pFmt = GetTabLines()[i]->GetFrmFmt();  \
            SwIterator<SwRowFrm,SwFmt> aIter( *pFmt );              \
            for (SwRowFrm* pFrm=aIter.First(); pFrm; pFrm=aIter.Next())\
            {                                                       \
                if ( pFrm->GetTabLine() == GetTabLines()[i] )       \
                {                                               \
                    OSL_ENSURE( pFrm->GetUpper()->IsTabFrm(),       \
                                "Table layout does not match table structure" );       \
                }                                               \
            }                                                       \
        }                                                           \
    }

#else

#define CHECKBOXWIDTH
#define CHECKTABLELAYOUT

#endif 

struct CR_SetLineHeight
{
    SwSelBoxes m_Boxes;
    SwShareBoxFmts aShareFmts;
    SwTableNode* pTblNd;
    SwUndoTblNdsChg* pUndo;
    SwTwips nMaxSpace, nMaxHeight;
    TblChgMode nMode;
    sal_uInt16 nLines;
    bool bBigger, bTop, bSplittBox, bAnyBoxFnd;

    CR_SetLineHeight( sal_uInt16 eType, SwTableNode* pTNd )
        : pTblNd( pTNd ), pUndo( 0 ),
        nMaxSpace( 0 ), nMaxHeight( 0 ), nLines( 0 ),
        bSplittBox( false ), bAnyBoxFnd( false )
    {
        bTop = nsTblChgWidthHeightType::WH_ROW_TOP == ( eType & 0xff ) || nsTblChgWidthHeightType::WH_CELL_TOP == ( eType & 0xff );
        bBigger = 0 != (eType & nsTblChgWidthHeightType::WH_FLAG_BIGGER );
        if( eType & nsTblChgWidthHeightType::WH_FLAG_INSDEL )
            bBigger = !bBigger;
        nMode = pTblNd->GetTable().GetTblChgMode();
    }
    CR_SetLineHeight( const CR_SetLineHeight& rCpy )
        : pTblNd( rCpy.pTblNd ), pUndo( rCpy.pUndo ),
        nMaxSpace( rCpy.nMaxSpace ), nMaxHeight( rCpy.nMaxHeight ),
        nMode( rCpy.nMode ), nLines( rCpy.nLines ),
        bBigger( rCpy.bBigger ), bTop( rCpy.bTop ),
        bSplittBox( rCpy.bSplittBox ), bAnyBoxFnd( rCpy.bAnyBoxFnd )
    {}

    SwUndoTblNdsChg* CreateUndo( SwUndoId nUndoType )
    {
        return pUndo = new SwUndoTblNdsChg( nUndoType, m_Boxes, *pTblNd );
    }
};

static bool lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                         SwTwips nDist, bool bCheck );
static bool lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, bool bCheck );
static bool lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, bool bCheck );

typedef bool (*FN_lcl_SetLineHeight)(SwTableLine*, CR_SetLineHeight&, SwTwips, bool );

_CpyTabFrm& _CpyTabFrm::operator=( const _CpyTabFrm& rCpyTabFrm )
{
    pNewFrmFmt = rCpyTabFrm.pNewFrmFmt;
    Value = rCpyTabFrm.Value;
    return *this;
}

typedef o3tl::sorted_vector<_CpyTabFrm> _CpyTabFrms;

struct _CpyPara
{
    boost::shared_ptr< std::vector< std::vector< sal_uLong > > > pWidths;
    SwDoc* pDoc;
    SwTableNode* pTblNd;
    _CpyTabFrms& rTabFrmArr;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    sal_uLong nOldSize, nNewSize;           
    sal_uLong nMinLeft, nMaxRight;
    sal_uInt16 nCpyCnt, nInsPos;
    sal_uInt16 nLnIdx, nBoxIdx;
    sal_uInt8 nDelBorderFlag;
    bool bCpyCntnt;

    _CpyPara( SwTableNode* pNd, sal_uInt16 nCopies, _CpyTabFrms& rFrmArr,
              bool bCopyContent = true )
        : pDoc( pNd->GetDoc() ), pTblNd( pNd ), rTabFrmArr(rFrmArr),
        pInsLine(0), pInsBox(0), nOldSize(0), nNewSize(0),
        nMinLeft(ULONG_MAX), nMaxRight(0),
        nCpyCnt(nCopies), nInsPos(0),
        nLnIdx(0), nBoxIdx(0),
        nDelBorderFlag(0), bCpyCntnt( bCopyContent )
        {}
    _CpyPara( const _CpyPara& rPara, SwTableLine* pLine )
        : pWidths( rPara.pWidths ), pDoc(rPara.pDoc), pTblNd(rPara.pTblNd),
        rTabFrmArr(rPara.rTabFrmArr), pInsLine(pLine), pInsBox(rPara.pInsBox),
        nOldSize(0), nNewSize(rPara.nNewSize), nMinLeft( rPara.nMinLeft ),
        nMaxRight( rPara.nMaxRight ), nCpyCnt(rPara.nCpyCnt), nInsPos(0),
        nLnIdx( rPara.nLnIdx), nBoxIdx( rPara.nBoxIdx ),
        nDelBorderFlag( rPara.nDelBorderFlag ), bCpyCntnt( rPara.bCpyCntnt )
        {}
    _CpyPara( const _CpyPara& rPara, SwTableBox* pBox )
        : pWidths( rPara.pWidths ), pDoc(rPara.pDoc), pTblNd(rPara.pTblNd),
        rTabFrmArr(rPara.rTabFrmArr), pInsLine(rPara.pInsLine), pInsBox(pBox),
        nOldSize(rPara.nOldSize), nNewSize(rPara.nNewSize),
        nMinLeft( rPara.nMinLeft ), nMaxRight( rPara.nMaxRight ),
        nCpyCnt(rPara.nCpyCnt), nInsPos(0), nLnIdx(rPara.nLnIdx), nBoxIdx(rPara.nBoxIdx),
        nDelBorderFlag( rPara.nDelBorderFlag ), bCpyCntnt( rPara.bCpyCntnt )
        {}
    void SetBoxWidth( SwTableBox* pBox );
};

static void lcl_CopyRow(_FndLine & rFndLine, _CpyPara *const pCpyPara);

static void lcl_CopyCol( _FndBox & rFndBox, _CpyPara *const pCpyPara)
{
    
    SwTableBox* pBox = rFndBox.GetBox();
    _CpyTabFrm aFindFrm( (SwTableBoxFmt*)pBox->GetFrmFmt() );

    sal_uInt16 nFndPos;
    if( pCpyPara->nCpyCnt )
    {
        _CpyTabFrms::const_iterator itFind = pCpyPara->rTabFrmArr.lower_bound( aFindFrm );
        nFndPos = itFind - pCpyPara->rTabFrmArr.begin();
        if( itFind == pCpyPara->rTabFrmArr.end() || !(*itFind == aFindFrm) )
        {
            
            SwTableBoxFmt* pNewFmt = (SwTableBoxFmt*)pBox->ClaimFrmFmt();

            
            _FndLine const* pCmpLine = NULL;
            SwFmtFrmSize aFrmSz( pNewFmt->GetFrmSize() );

            bool bDiffCount = false;
            if( pBox->GetTabLines().size() )
            {
                pCmpLine = &rFndBox.GetLines().front();
                if ( pCmpLine->GetBoxes().size() != pCmpLine->GetLine()->GetTabBoxes().size() )
                    bDiffCount = true;
            }

            if( bDiffCount )
            {
                
                _FndBoxes const& rFndBoxes = pCmpLine->GetBoxes();
                long nSz = 0;
                for( sal_uInt16 n = rFndBoxes.size(); n; )
                {
                    nSz += rFndBoxes[--n].GetBox()->
                            GetFrmFmt()->GetFrmSize().GetWidth();
                }
                aFrmSz.SetWidth( aFrmSz.GetWidth() -
                                            nSz / ( pCpyPara->nCpyCnt + 1 ) );
                pNewFmt->SetFmtAttr( aFrmSz );
                aFrmSz.SetWidth( nSz / ( pCpyPara->nCpyCnt + 1 ) );

                
                aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pNewFmt->GetDoc()->
                                            MakeTableLineFmt();
                *aFindFrm.pNewFrmFmt = *pNewFmt;
                aFindFrm.pNewFrmFmt->SetFmtAttr( aFrmSz );
            }
            else
            {
                aFrmSz.SetWidth( aFrmSz.GetWidth() / ( pCpyPara->nCpyCnt + 1 ) );
                pNewFmt->SetFmtAttr( aFrmSz );

                aFindFrm.pNewFrmFmt = pNewFmt;
                pCpyPara->rTabFrmArr.insert( aFindFrm );
                aFindFrm.Value.pFrmFmt = pNewFmt;
                pCpyPara->rTabFrmArr.insert( aFindFrm );
            }
        }
        else
        {
            aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ];
            pBox->ChgFrmFmt( (SwTableBoxFmt*)aFindFrm.pNewFrmFmt );
        }
    }
    else
    {
        _CpyTabFrms::const_iterator itFind = pCpyPara->rTabFrmArr.find( aFindFrm );
        if( pCpyPara->nDelBorderFlag &&
            itFind != pCpyPara->rTabFrmArr.end() )
            aFindFrm = *itFind;
        else
            aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
    }

    if (!rFndBox.GetLines().empty())
    {
        pBox = new SwTableBox( aFindFrm.pNewFrmFmt,
                    rFndBox.GetLines().size(), pCpyPara->pInsLine );
        pCpyPara->pInsLine->GetTabBoxes().insert( pCpyPara->pInsLine->GetTabBoxes().begin() + pCpyPara->nInsPos++, pBox );
        _CpyPara aPara( *pCpyPara, pBox );
        aPara.nDelBorderFlag &= 7;

        BOOST_FOREACH( _FndLine & rFndLine, rFndBox.GetLines() )
            lcl_CopyRow( rFndLine, &aPara );
    }
    else
    {
        ::_InsTblBox( pCpyPara->pDoc, pCpyPara->pTblNd, pCpyPara->pInsLine,
                    aFindFrm.pNewFrmFmt, pBox, pCpyPara->nInsPos++ );

        const _FndBoxes& rFndBxs = rFndBox.GetUpper()->GetBoxes();
        if( 8 > pCpyPara->nDelBorderFlag
                ? pCpyPara->nDelBorderFlag
                : &rFndBox == &rFndBxs[rFndBxs.size() - 1] )
        {
            const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();
            if( 8 > pCpyPara->nDelBorderFlag
                    ? rBoxItem.GetTop()
                    : rBoxItem.GetRight() )
            {
                aFindFrm.Value.pFrmFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();

                SvxBoxItem aNew( rBoxItem );
                if( 8 > pCpyPara->nDelBorderFlag )
                    aNew.SetLine( 0, BOX_LINE_TOP );
                else
                    aNew.SetLine( 0, BOX_LINE_RIGHT );

                if( 1 == pCpyPara->nDelBorderFlag ||
                    8 == pCpyPara->nDelBorderFlag )
                {
                    
                    pBox = pCpyPara->pInsLine->GetTabBoxes()[
                                            pCpyPara->nInsPos - 1 ];
                }

                aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();

                
                
                pBox->ClaimFrmFmt()->SetFmtAttr( aNew );

                if( !pCpyPara->nCpyCnt )
                    pCpyPara->rTabFrmArr.insert( aFindFrm );
            }
        }
    }
}

static void lcl_CopyRow(_FndLine& rFndLine, _CpyPara *const pCpyPara)
{
    SwTableLine* pNewLine = new SwTableLine(
                            (SwTableLineFmt*)rFndLine.GetLine()->GetFrmFmt(),
                        rFndLine.GetBoxes().size(), pCpyPara->pInsBox );
    if( pCpyPara->pInsBox )
    {
        SwTableLines& rLines = pCpyPara->pInsBox->GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine );
    }
    else
    {
        SwTableLines& rLines = pCpyPara->pTblNd->GetTable().GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine );
    }

    _CpyPara aPara( *pCpyPara, pNewLine );
    for (_FndBoxes::iterator it = rFndLine.GetBoxes().begin();
         it != rFndLine.GetBoxes().end(); ++it)
    {
        lcl_CopyCol(*it, &aPara);
    }

    pCpyPara->nDelBorderFlag &= 0xf8;
}

static void lcl_InsCol( _FndLine* pFndLn, _CpyPara& rCpyPara, sal_uInt16 nCpyCnt,
                bool bBehind )
{
    
    _FndBox* pFBox;
    if( 1 == pFndLn->GetBoxes().size() &&
        !( pFBox = &pFndLn->GetBoxes()[0] )->GetBox()->GetSttNd() )
    {
        
        for( sal_uInt16 n = 0; n < pFBox->GetLines().size(); ++n )
            lcl_InsCol( &pFBox->GetLines()[ n ], rCpyPara, nCpyCnt, bBehind );
    }
    else
    {
        rCpyPara.pInsLine = pFndLn->GetLine();
        SwTableBox* pBox = pFndLn->GetBoxes()[ bBehind ?
                    pFndLn->GetBoxes().size()-1 : 0 ].GetBox();
        rCpyPara.nInsPos = pFndLn->GetLine()->GetTabBoxes().GetPos( pBox );
        if( bBehind )
            ++rCpyPara.nInsPos;

        for( sal_uInt16 n = 0; n < nCpyCnt; ++n )
        {
            if( n + 1 == nCpyCnt && bBehind )
                rCpyPara.nDelBorderFlag = 9;
            else
                rCpyPara.nDelBorderFlag = 8;
            for (_FndBoxes::iterator it = pFndLn->GetBoxes().begin();
                 it != pFndLn->GetBoxes().end(); ++it)
            {
                lcl_CopyCol(*it, &rCpyPara);
            }
        }
    }
}

SwRowFrm* GetRowFrm( SwTableLine& rLine )
{
    SwIterator<SwRowFrm,SwFmt> aIter( *rLine.GetFrmFmt() );
    for( SwRowFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
        if( pFrm->GetTabLine() == &rLine )
            return pFrm;
    return 0;
}

bool SwTable::InsertCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid Box List" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

    bool bRes = true;
    if( IsNewModel() )
        bRes = NewInsertCol( pDoc, rBoxes, nCnt, bBehind );
    else
    {
        
        _FndBox aFndBox( 0, 0 );
        {
            _FndPara aPara( rBoxes, &aFndBox );
            ForEach_FndLineCopyCol( GetTabLines(), &aPara );
        }
        if( aFndBox.GetLines().empty() )
            return false;

        SetHTMLTableLayout( 0 );    

        
        aFndBox.SetTableLines( *this );
        aFndBox.DelFrms( *this );

        
        

        _CpyTabFrms aTabFrmArr;
        _CpyPara aCpyPara( pTblNd, nCnt, aTabFrmArr );

        for( sal_uInt16 n = 0; n < aFndBox.GetLines().size(); ++n )
            lcl_InsCol( &aFndBox.GetLines()[ n ], aCpyPara, nCnt, bBehind );

        
        GCLines();

        
        aFndBox.MakeFrms( *this );

        CHECKBOXWIDTH;
        CHECKTABLELAYOUT;
        bRes = true;
    }

    SwChartDataProvider *pPCD = pDoc->GetChartDataProvider();
    if (pPCD && nCnt)
        pPCD->AddRowCols( *this, rBoxes, nCnt, bBehind );
    pDoc->UpdateCharts( GetFrmFmt()->GetName() );

    return bRes;
}

bool SwTable::_InsertRow( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        sal_uInt16 nCnt, bool bBehind )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid Box List" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

     
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    SetHTMLTableLayout( 0 );   

    _FndBox* pFndBox = &aFndBox;
    {
        _FndLine* pFndLine;
        while( 1 == pFndBox->GetLines().size() &&
                1 == ( pFndLine = &pFndBox->GetLines()[ 0 ])->GetBoxes().size() )
        {
            
            _FndBox* pTmpBox = &pFndLine->GetBoxes().front();
            if( !pTmpBox->GetLines().empty() )
                pFndBox = pTmpBox;
            else
                break;
        }
    }

    
    const bool bLayout = !IsNewModel() &&
        0 != SwIterator<SwTabFrm,SwFmt>::FirstElement( *GetFrmFmt() );

    if ( bLayout )
    {
        aFndBox.SetTableLines( *this );
        if( pFndBox != &aFndBox )
            aFndBox.DelFrms( *this );
        
        
    }

    _CpyTabFrms aTabFrmArr;
    _CpyPara aCpyPara( pTblNd, 0, aTabFrmArr );

    SwTableLine* pLine = pFndBox->GetLines()[ bBehind ?
                    pFndBox->GetLines().size()-1 : 0 ].GetLine();
    if( &aFndBox == pFndBox )
        aCpyPara.nInsPos = GetTabLines().GetPos( pLine );
    else
    {
        aCpyPara.pInsBox = pFndBox->GetBox();
        aCpyPara.nInsPos = pFndBox->GetBox()->GetTabLines().GetPos( pLine );
    }

    if( bBehind )
    {
        ++aCpyPara.nInsPos;
        aCpyPara.nDelBorderFlag = 1;
    }
    else
        aCpyPara.nDelBorderFlag = 2;

    for( sal_uInt16 nCpyCnt = 0; nCpyCnt < nCnt; ++nCpyCnt )
    {
        if( bBehind )
            aCpyPara.nDelBorderFlag = 1;
        BOOST_FOREACH( _FndLine& rFndLine, pFndBox->GetLines() )
            lcl_CopyRow( rFndLine, &aCpyPara );
    }

    
    if( !pDoc->IsInReading() )
        GCLines();

    
    if ( bLayout )
    {
        if( pFndBox != &aFndBox )
            aFndBox.MakeFrms( *this );
        else
            aFndBox.MakeNewFrms( *this, nCnt, bBehind );
    }

    CHECKBOXWIDTH;
    CHECKTABLELAYOUT;

    SwChartDataProvider *pPCD = pDoc->GetChartDataProvider();
    if (pPCD && nCnt)
        pPCD->AddRowCols( *this, rBoxes, nCnt, bBehind );
    pDoc->UpdateCharts( GetFrmFmt()->GetName() );

    return true;
}

static void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            bool bFirst, SwShareBoxFmts& rShareFmts );

static void lcl_LastBoxSetWidthLine( SwTableLines &rLines, const long nOffset,
                                bool bFirst, SwShareBoxFmts& rShareFmts )
{
    for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
        ::lcl_LastBoxSetWidth( rLines[i]->GetTabBoxes(), nOffset, bFirst,
                                rShareFmts );
}

static void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            bool bFirst, SwShareBoxFmts& rShareFmts )
{
    SwTableBox& rBox = *(bFirst ? rBoxes.front() : rBoxes.back());
    if( !rBox.GetSttNd() )
        ::lcl_LastBoxSetWidthLine( rBox.GetTabLines(), nOffset,
                                    bFirst, rShareFmts );

    
    const SwFrmFmt *pBoxFmt = rBox.GetFrmFmt();
    SwFmtFrmSize aNew( pBoxFmt->GetFrmSize() );
    aNew.SetWidth( aNew.GetWidth() + nOffset );
    SwFrmFmt *pFmt = rShareFmts.GetFormat( *pBoxFmt, aNew );
    if( pFmt )
        rBox.ChgFrmFmt( (SwTableBoxFmt*)pFmt );
    else
    {
        pFmt = rBox.ClaimFrmFmt();

        pFmt->LockModify();
        pFmt->SetFmtAttr( aNew );
        pFmt->UnlockModify();

        rShareFmts.AddFormat( *pBoxFmt, *pFmt );
    }
}

void _DeleteBox( SwTable& rTbl, SwTableBox* pBox, SwUndo* pUndo,
                bool bCalcNewSize, const bool bCorrBorder,
                SwShareBoxFmts* pShareFmts )
{
    do {
        SwTwips nBoxSz = bCalcNewSize ?
                pBox->GetFrmFmt()->GetFrmSize().GetWidth() : 0;
        SwTableLine* pLine = pBox->GetUpper();
        SwTableBoxes& rTblBoxes = pLine->GetTabBoxes();
        sal_uInt16 nDelPos = rTblBoxes.GetPos( pBox );
        SwTableBox* pUpperBox = pBox->GetUpper()->GetUpper();

        
        if( bCorrBorder && 1 < rTblBoxes.size() )
        {
            bool bChgd = false;
            const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();

            if( rBoxItem.GetLeft() || rBoxItem.GetRight() )
            {
                
                
                if( nDelPos + 1 < (sal_uInt16)rTblBoxes.size() )
                {
                    SwTableBox* pNxtBox = rTblBoxes[ nDelPos + 1 ];
                    const SvxBoxItem& rNxtBoxItem = pNxtBox->GetFrmFmt()->GetBox();

                    SwTableBox* pPrvBox = nDelPos ? rTblBoxes[ nDelPos - 1 ] : 0;

                    if( pNxtBox->GetSttNd() && !rNxtBoxItem.GetLeft() &&
                        ( !pPrvBox || !pPrvBox->GetFrmFmt()->GetBox().GetRight()) )
                    {
                        SvxBoxItem aTmp( rNxtBoxItem );
                        aTmp.SetLine( rBoxItem.GetLeft() ? rBoxItem.GetLeft()
                                                         : rBoxItem.GetRight(),
                                                            BOX_LINE_LEFT );
                        if( pShareFmts )
                            pShareFmts->SetAttr( *pNxtBox, aTmp );
                        else
                            pNxtBox->ClaimFrmFmt()->SetFmtAttr( aTmp );
                        bChgd = true;
                    }
                }
                if( !bChgd && nDelPos )
                {
                    SwTableBox* pPrvBox = rTblBoxes[ nDelPos - 1 ];
                    const SvxBoxItem& rPrvBoxItem = pPrvBox->GetFrmFmt()->GetBox();

                    SwTableBox* pNxtBox = nDelPos + 1 < (sal_uInt16)rTblBoxes.size()
                                            ? rTblBoxes[ nDelPos + 1 ] : 0;

                    if( pPrvBox->GetSttNd() && !rPrvBoxItem.GetRight() &&
                        ( !pNxtBox || !pNxtBox->GetFrmFmt()->GetBox().GetLeft()) )
                    {
                        SvxBoxItem aTmp( rPrvBoxItem );
                        aTmp.SetLine( rBoxItem.GetLeft() ? rBoxItem.GetLeft()
                                                         : rBoxItem.GetRight(),
                                                            BOX_LINE_RIGHT );
                        if( pShareFmts )
                            pShareFmts->SetAttr( *pPrvBox, aTmp );
                        else
                            pPrvBox->ClaimFrmFmt()->SetFmtAttr( aTmp );
                    }
                }
            }
        }

        
        SwStartNode* pSttNd = (SwStartNode*)pBox->GetSttNd();
        if( pShareFmts )
            pShareFmts->RemoveFormat( *rTblBoxes[ nDelPos ]->GetFrmFmt() );
        delete rTblBoxes[nDelPos];
        rTblBoxes.erase( rTblBoxes.begin() + nDelPos );

        if( pSttNd )
        {
            
            if( pUndo && pUndo->IsDelBox() )
                ((SwUndoTblNdsChg*)pUndo)->SaveSection( pSttNd );
            else
                pSttNd->GetDoc()->DeleteSection( pSttNd );
        }

        
        if( !rTblBoxes.empty() )
        {
            
            bool bLastBox = nDelPos == rTblBoxes.size();
            if( bLastBox )
                --nDelPos;
            pBox = rTblBoxes[nDelPos];
            if( bCalcNewSize )
            {
                SwFmtFrmSize aNew( pBox->GetFrmFmt()->GetFrmSize() );
                aNew.SetWidth( aNew.GetWidth() + nBoxSz );
                if( pShareFmts )
                    pShareFmts->SetSize( *pBox, aNew );
                else
                    pBox->ClaimFrmFmt()->SetFmtAttr( aNew );

                if( !pBox->GetSttNd() )
                {
                    
                    SwShareBoxFmts aShareFmts;
                    ::lcl_LastBoxSetWidthLine( pBox->GetTabLines(), nBoxSz,
                                                !bLastBox,
                                                pShareFmts ? *pShareFmts
                                                           : aShareFmts );
                }
            }
            break;      
        }
        
        if( !pUpperBox )
        {
            
            nDelPos = rTbl.GetTabLines().GetPos( pLine );
            if( pShareFmts )
                pShareFmts->RemoveFormat( *rTbl.GetTabLines()[ nDelPos ]->GetFrmFmt() );
            delete rTbl.GetTabLines()[ nDelPos ];
            rTbl.GetTabLines().erase( rTbl.GetTabLines().begin() + nDelPos );
            break;      
        }

        
        pBox = pUpperBox;
        nDelPos = pBox->GetTabLines().GetPos( pLine );
        if( pShareFmts )
            pShareFmts->RemoveFormat( *pBox->GetTabLines()[ nDelPos ]->GetFrmFmt() );
        delete pBox->GetTabLines()[ nDelPos ];
        pBox->GetTabLines().erase( pBox->GetTabLines().begin() + nDelPos );
    } while( pBox->GetTabLines().empty() );
}

static SwTableBox*
lcl_FndNxtPrvDelBox( const SwTableLines& rTblLns,
                                SwTwips nBoxStt, SwTwips nBoxWidth,
                                sal_uInt16 nLinePos, bool bNxt,
                                SwSelBoxes* pAllDelBoxes, size_t *const pCurPos)
{
    SwTableBox* pFndBox = 0;
    do {
        if( bNxt )
            ++nLinePos;
        else
            --nLinePos;
        SwTableLine* pLine = rTblLns[ nLinePos ];
        SwTwips nFndBoxWidth = 0;
        SwTwips nFndWidth = nBoxStt + nBoxWidth;
        sal_uInt16 nBoxCnt = pLine->GetTabBoxes().size();

        pFndBox = pLine->GetTabBoxes()[ 0 ];
        for( sal_uInt16 n = 0; 0 < nFndWidth && n < nBoxCnt; ++n )
        {
            pFndBox = pLine->GetTabBoxes()[ n ];
            nFndWidth -= (nFndBoxWidth = pFndBox->GetFrmFmt()->
                                        GetFrmSize().GetWidth());
        }

        
        while( !pFndBox->GetSttNd() )
        {
            const SwTableLines& rLowLns = pFndBox->GetTabLines();
            if( bNxt )
                pFndBox = rLowLns.front()->GetTabBoxes().front();
            else
                pFndBox = rLowLns.back()->GetTabBoxes().front();
        }

        if( std::abs( nFndWidth ) > COLFUZZY ||
            std::abs( nBoxWidth - nFndBoxWidth ) > COLFUZZY )
            pFndBox = 0;
        else if( pAllDelBoxes )
        {
            
            SwSelBoxes::const_iterator aFndIt = pAllDelBoxes->find( pFndBox);
            if( aFndIt == pAllDelBoxes->end() )
                break;
            size_t const nFndPos = aFndIt - pAllDelBoxes->begin() ;

            
            
            pFndBox = 0;
            if( nFndPos <= *pCurPos )
                --*pCurPos;
            pAllDelBoxes->erase( pAllDelBoxes->begin() + nFndPos );
        }
    } while( bNxt ? ( nLinePos + 1 < (sal_uInt16)rTblLns.size() ) : nLinePos != 0 );
    return pFndBox;
}

static void
lcl_SaveUpperLowerBorder( SwTable& rTbl, const SwTableBox& rBox,
                                SwShareBoxFmts& rShareFmts,
                                SwSelBoxes* pAllDelBoxes = 0,
                                size_t *const pCurPos = 0 )
{

    bool bChgd = false;
    const SwTableLine* pLine = rBox.GetUpper();
    const SwTableBoxes& rTblBoxes = pLine->GetTabBoxes();
    const SwTableBox* pUpperBox = &rBox;
    sal_uInt16 nDelPos = rTblBoxes.GetPos( pUpperBox );
    pUpperBox = rBox.GetUpper()->GetUpper();
    const SvxBoxItem& rBoxItem = rBox.GetFrmFmt()->GetBox();

    
    if( rBoxItem.GetTop() || rBoxItem.GetBottom() )
    {
        bChgd = false;
        const SwTableLines* pTblLns;
        if( pUpperBox )
            pTblLns = &pUpperBox->GetTabLines();
        else
            pTblLns = &rTbl.GetTabLines();

        sal_uInt16 nLnPos = pTblLns->GetPos( pLine );

        
        
        SwTwips nBoxStt = 0;
        for( sal_uInt16 n = 0; n < nDelPos; ++n )
            nBoxStt += rTblBoxes[ n ]->GetFrmFmt()->GetFrmSize().GetWidth();
        SwTwips nBoxWidth = rBox.GetFrmFmt()->GetFrmSize().GetWidth();

        SwTableBox *pPrvBox = 0, *pNxtBox = 0;
        if( nLnPos )        
            pPrvBox = ::lcl_FndNxtPrvDelBox( *pTblLns, nBoxStt, nBoxWidth,
                                nLnPos, false, pAllDelBoxes, pCurPos );

        if( nLnPos + 1 < (sal_uInt16)pTblLns->size() )     
            pNxtBox = ::lcl_FndNxtPrvDelBox( *pTblLns, nBoxStt, nBoxWidth,
                                nLnPos, true, pAllDelBoxes, pCurPos );

        if( pNxtBox && pNxtBox->GetSttNd() )
        {
            const SvxBoxItem& rNxtBoxItem = pNxtBox->GetFrmFmt()->GetBox();
            if( !rNxtBoxItem.GetTop() && ( !pPrvBox ||
                !pPrvBox->GetFrmFmt()->GetBox().GetBottom()) )
            {
                SvxBoxItem aTmp( rNxtBoxItem );
                aTmp.SetLine( rBoxItem.GetTop() ? rBoxItem.GetTop()
                                                : rBoxItem.GetBottom(),
                                                BOX_LINE_TOP );
                rShareFmts.SetAttr( *pNxtBox, aTmp );
                bChgd = true;
            }
        }
        if( !bChgd && pPrvBox && pPrvBox->GetSttNd() )
        {
            const SvxBoxItem& rPrvBoxItem = pPrvBox->GetFrmFmt()->GetBox();
            if( !rPrvBoxItem.GetTop() && ( !pNxtBox ||
                !pNxtBox->GetFrmFmt()->GetBox().GetTop()) )
            {
                SvxBoxItem aTmp( rPrvBoxItem );
                aTmp.SetLine( rBoxItem.GetTop() ? rBoxItem.GetTop()
                                                : rBoxItem.GetBottom(),
                                                BOX_LINE_BOTTOM );
                rShareFmts.SetAttr( *pPrvBox, aTmp );
            }
        }
    }
}

bool SwTable::DeleteSel(
    SwDoc*     pDoc
    ,
    const SwSelBoxes& rBoxes,
    const SwSelBoxes* pMerged, SwUndo* pUndo,
    const bool bDelMakeFrms, const bool bCorrBorder )
{
    OSL_ENSURE( pDoc, "No doc?" );
    SwTableNode* pTblNd = 0;
    if( !rBoxes.empty() )
    {
        pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
        if( !pTblNd )
            return false;
    }

    SetHTMLTableLayout( 0 );    

    
    _FndBox aFndBox( 0, 0 );
    if ( bDelMakeFrms )
    {
        if( pMerged && !pMerged->empty() )
            aFndBox.SetTableLines( *pMerged, *this );
        else if( !rBoxes.empty() )
            aFndBox.SetTableLines( rBoxes, *this );
        aFndBox.DelFrms( *this );
    }

    SwShareBoxFmts aShareFmts;

    
    if( bCorrBorder )
    {
        SwSelBoxes aBoxes( rBoxes );
        for (size_t n = 0; n < aBoxes.size(); ++n)
        {
            ::lcl_SaveUpperLowerBorder( *this, *rBoxes[ n ], aShareFmts,
                                        &aBoxes, &n );
        }
    }

    PrepareDelBoxes( rBoxes );

    SwChartDataProvider *pPCD = pDoc->GetChartDataProvider();
    
    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        size_t const nIdx = rBoxes.size() - 1 - n;

        
        
        
        if (pPCD && pTblNd)
            pPCD->DeleteBox( &pTblNd->GetTable(), *rBoxes[nIdx] );

        
        _DeleteBox( *this, rBoxes[nIdx], pUndo, true, bCorrBorder, &aShareFmts );
    }

    
    GCLines();

    if( bDelMakeFrms && aFndBox.AreLinesToRestore( *this ) )
        aFndBox.MakeFrms( *this );

    
    pDoc->UpdateCharts( GetFrmFmt()->GetName() );

    CHECKTABLELAYOUT;
    CHECK_TABLE( *this );

    return true;
}

bool SwTable::OldSplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt,
                        bool bSameHeight )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

    
    
    
    pDoc->CreateChartInternalDataProviders( this );

    SetHTMLTableLayout( 0 );    

    
    
    long* pRowHeights = 0;
    if ( bSameHeight )
    {
        pRowHeights = new long[ rBoxes.size() ];
        for (size_t n = 0; n < rBoxes.size(); ++n)
        {
            SwTableBox* pSelBox = rBoxes[n];
            const SwRowFrm* pRow = GetRowFrm( *pSelBox->GetUpper() );
            OSL_ENSURE( pRow, "Where is the SwTableLine's Frame?" );
            SWRECTFN( pRow )
            pRowHeights[ n ] = (pRow->Frm().*fnRect->fnGetHeight)();
        }
    }

    
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rBoxes, *this );
    aFndBox.DelFrms( *this );

    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        SwTableBox* pSelBox = rBoxes[n];
        OSL_ENSURE( pSelBox, "Box is not within the Table" );

        
        SwTableLine* pInsLine = pSelBox->GetUpper();
        SwTableBoxFmt* pFrmFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();

        
        SwFmtFrmSize aFSz( pInsLine->GetFrmFmt()->GetFrmSize() );
        if ( bSameHeight && ATT_VAR_SIZE == aFSz.GetHeightSizeType() )
            aFSz.SetHeightSizeType( ATT_MIN_SIZE );

        bool bChgLineSz = 0 != aFSz.GetHeight() || bSameHeight;
        if ( bChgLineSz )
            aFSz.SetHeight( ( bSameHeight ? pRowHeights[ n ] : aFSz.GetHeight() ) /
                             (nCnt + 1) );

        SwTableBox* pNewBox = new SwTableBox( pFrmFmt, nCnt, pInsLine );
        sal_uInt16 nBoxPos = pInsLine->GetTabBoxes().GetPos( pSelBox );
        pInsLine->GetTabBoxes()[nBoxPos] = pNewBox; 

        
        SwTableBox* pLastBox = pSelBox;         
        
        
        bool bMoveNodes = true;
        {
            sal_uLong nSttNd = pLastBox->GetSttIdx() + 1,
                    nEndNd = pLastBox->GetSttNd()->EndOfSectionIndex();
            while( nSttNd < nEndNd )
                if( !pDoc->GetNodes()[ nSttNd++ ]->IsTxtNode() )
                {
                    bMoveNodes = false;
                    break;
                }
        }

        SwTableBoxFmt* pCpyBoxFrmFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();
        bool bChkBorder = 0 != pCpyBoxFrmFmt->GetBox().GetTop();
        if( bChkBorder )
            pCpyBoxFrmFmt = (SwTableBoxFmt*)pSelBox->ClaimFrmFmt();

        for( sal_uInt16 i = 0; i <= nCnt; ++i )
        {
            
            SwTableLine* pNewLine = new SwTableLine(
                    (SwTableLineFmt*)pInsLine->GetFrmFmt(), 1, pNewBox );
            if( bChgLineSz )
            {
                pNewLine->ClaimFrmFmt()->SetFmtAttr( aFSz );
            }

            pNewBox->GetTabLines().insert( pNewBox->GetTabLines().begin() + i, pNewLine );
            
            if( !i )        
            {
                pSelBox->SetUpper( pNewLine );
                pNewLine->GetTabBoxes().insert( pNewLine->GetTabBoxes().begin(), pSelBox );
            }
            else
            {
                ::_InsTblBox( pDoc, pTblNd, pNewLine, pCpyBoxFrmFmt,
                                pLastBox, 0 );

                if( bChkBorder )
                {
                    pCpyBoxFrmFmt = (SwTableBoxFmt*)pNewLine->GetTabBoxes()[ 0 ]->ClaimFrmFmt();
                    SvxBoxItem aTmp( pCpyBoxFrmFmt->GetBox() );
                    aTmp.SetLine( 0, BOX_LINE_TOP );
                    pCpyBoxFrmFmt->SetFmtAttr( aTmp );
                    bChkBorder = false;
                }

                if( bMoveNodes )
                {
                    const SwNode* pEndNd = pLastBox->GetSttNd()->EndOfSectionNode();
                    if( pLastBox->GetSttIdx()+2 != pEndNd->GetIndex() )
                    {
                        
                        SwNodeRange aRg( *pLastBox->GetSttNd(), +2, *pEndNd );
                        pLastBox = pNewLine->GetTabBoxes()[0];  
                        SwNodeIndex aInsPos( *pLastBox->GetSttNd(), 1 );
                        pDoc->GetNodes()._MoveNodes(aRg, pDoc->GetNodes(), aInsPos, sal_False);
                        pDoc->GetNodes().Delete( aInsPos, 1 ); 
                    }
                }
            }
        }
        
        pFrmFmt = (SwTableBoxFmt*)pNewBox->ClaimFrmFmt();
        pFrmFmt->ResetFmtAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
        pFrmFmt->ResetFmtAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
    }

    delete[] pRowHeights;

    GCLines();

    aFndBox.MakeFrms( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT
    return true;
}

bool SwTable::SplitCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

    
    
    
    pDoc->CreateChartInternalDataProviders( this );

    SetHTMLTableLayout( 0 );    
    SwSelBoxes aSelBoxes(rBoxes);
    ExpandSelection( aSelBoxes );

    
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( aSelBoxes, *this );
    aFndBox.DelFrms( *this );

    _CpyTabFrms aFrmArr;
    std::vector<SwTableBoxFmt*> aLastBoxArr;
    sal_uInt16 nFndPos;
    for (size_t n = 0; n < aSelBoxes.size(); ++n)
    {
        SwTableBox* pSelBox = aSelBoxes[n];
        OSL_ENSURE( pSelBox, "Box is not in the table" );

        
        if( pSelBox->GetFrmFmt()->GetFrmSize().GetWidth()/( nCnt + 1 ) < 10 )
            continue;

        
        SwTableLine* pInsLine = pSelBox->GetUpper();
        sal_uInt16 nBoxPos = pInsLine->GetTabBoxes().GetPos( pSelBox );

        
        SwTableBoxFmt* pLastBoxFmt;
        _CpyTabFrm aFindFrm( (SwTableBoxFmt*)pSelBox->GetFrmFmt() );
        _CpyTabFrms::const_iterator itFind = aFrmArr.lower_bound( aFindFrm );
        nFndPos = itFind - aFrmArr.begin();
        if( itFind == aFrmArr.end() || !(*itFind == aFindFrm) )
        {
            
            aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pSelBox->ClaimFrmFmt();
            SwTwips nBoxSz = aFindFrm.pNewFrmFmt->GetFrmSize().GetWidth();
            SwTwips nNewBoxSz = nBoxSz / ( nCnt + 1 );
            aFindFrm.pNewFrmFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                                        nNewBoxSz, 0 ) );
            aFrmArr.insert( aFindFrm );

            pLastBoxFmt = aFindFrm.pNewFrmFmt;
            if( nBoxSz != ( nNewBoxSz * (nCnt + 1)))
            {
                
                
                pLastBoxFmt = new SwTableBoxFmt( *aFindFrm.pNewFrmFmt );
                pLastBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                nBoxSz - ( nNewBoxSz * nCnt ), 0 ) );
            }
            aLastBoxArr.insert( aLastBoxArr.begin() + nFndPos, pLastBoxFmt );
        }
        else
        {
            aFindFrm = aFrmArr[ nFndPos ];
            pSelBox->ChgFrmFmt( (SwTableBoxFmt*)aFindFrm.pNewFrmFmt );
            pLastBoxFmt = aLastBoxArr[ nFndPos ];
        }

        
        for( sal_uInt16 i = 1; i < nCnt; ++i )
            ::_InsTblBox( pDoc, pTblNd, pInsLine, aFindFrm.pNewFrmFmt,
                        pSelBox, nBoxPos + i ); 

        ::_InsTblBox( pDoc, pTblNd, pInsLine, pLastBoxFmt,
                    pSelBox, nBoxPos + nCnt );  

        
        const SvxBoxItem& aSelBoxItem = aFindFrm.pNewFrmFmt->GetBox();
        if( aSelBoxItem.GetRight() )
        {
            pInsLine->GetTabBoxes()[ nBoxPos + nCnt ]->ClaimFrmFmt();

            SvxBoxItem aTmp( aSelBoxItem );
            aTmp.SetLine( 0, BOX_LINE_RIGHT );
            aFindFrm.pNewFrmFmt->SetFmtAttr( aTmp );

            
            for( sal_uInt16 i = aFrmArr.size(); i; )
            {
                const _CpyTabFrm& rCTF = aFrmArr[ --i ];
                if( rCTF.pNewFrmFmt == aFindFrm.pNewFrmFmt ||
                    rCTF.Value.pFrmFmt == aFindFrm.pNewFrmFmt )
                {
                    aFrmArr.erase( aFrmArr.begin() + i );
                    aLastBoxArr.erase( aLastBoxArr.begin() + i );
                }
            }
        }
    }

    
    aFndBox.MakeFrms( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT
    return true;
}

/*
    ----------------------- >> MERGE << ------------------------
     Algorithm:
        If we only have one Line in the _FndBox, take this Line and test
        the Box count:
        - If we have more than one Box, we merge on Box level, meaning
          the new Box will be as wide as the old ones.
            - All Lines that are above/under the Area, are inserted into
            the Box as Line + Box.
            - All Lines that come before/after the Area, are inserted into
              the Boxes Left/Right.

    ----------------------- >> MERGE << ------------------------
*/
static void lcl_CpyLines( sal_uInt16 nStt, sal_uInt16 nEnd,
                                SwTableLines& rLines,
                                SwTableBox* pInsBox,
                                sal_uInt16 nPos = USHRT_MAX )
{
    for( sal_uInt16 n = nStt; n < nEnd; ++n )
        rLines[n]->SetUpper( pInsBox );
    if( USHRT_MAX == nPos )
        nPos = pInsBox->GetTabLines().size();
    pInsBox->GetTabLines().insert( pInsBox->GetTabLines().begin() + nPos,
                             rLines.begin() + nStt, rLines.begin() + nEnd );
    rLines.erase( rLines.begin() + nStt, rLines.begin() + nEnd );
}

static void lcl_CpyBoxes( sal_uInt16 nStt, sal_uInt16 nEnd,
                                SwTableBoxes& rBoxes,
                                SwTableLine* pInsLine,
                                sal_uInt16 nPos = USHRT_MAX )
{
    for( sal_uInt16 n = nStt; n < nEnd; ++n )
        rBoxes[n]->SetUpper( pInsLine );
    if( USHRT_MAX == nPos )
        nPos = pInsLine->GetTabBoxes().size();
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin() + nPos,
                              rBoxes.begin() + nStt, rBoxes.begin() + nEnd );
    rBoxes.erase( rBoxes.begin() + nStt, rBoxes.begin() + nEnd );
}

static void lcl_CalcWidth( SwTableBox* pBox )
{
    
    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
    OSL_ENSURE( pBox->GetTabLines().size(), "Box does not have any Lines" );

    SwTableLine* pLine = pBox->GetTabLines()[0];
    OSL_ENSURE( pLine, "Box is not within a Line" );

    long nWidth = 0;
    for( sal_uInt16 n = 0; n < pLine->GetTabBoxes().size(); ++n )
        nWidth += pLine->GetTabBoxes()[n]->GetFrmFmt()->GetFrmSize().GetWidth();

    pFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));

    
    pFmt->ResetFmtAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
    pFmt->ResetFmtAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
}

struct _InsULPara
{
    SwTableNode* pTblNd;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    bool bUL_LR : 1;        
    bool bUL : 1;           

    SwTableBox* pLeftBox;
    SwTableBox* pRightBox;
    SwTableBox* pMergeBox;

    _InsULPara( SwTableNode* pTNd, bool bUpperLower, bool bUpper,
                SwTableBox* pLeft, SwTableBox* pMerge, SwTableBox* pRight,
                SwTableLine* pLine=0, SwTableBox* pBox=0 )
        : pTblNd( pTNd ), pInsLine( pLine ), pInsBox( pBox ),
        pLeftBox( pLeft ), pRightBox( pRight ), pMergeBox( pMerge )
        {   bUL_LR = bUpperLower; bUL = bUpper; }

    void SetLeft( SwTableBox* pBox=0 )
        { bUL_LR = false;   bUL = true; if( pBox ) pInsBox = pBox; }
    void SetRight( SwTableBox* pBox=0 )
        { bUL_LR = false;   bUL = false; if( pBox ) pInsBox = pBox; }
    void SetUpper( SwTableLine* pLine=0 )
        { bUL_LR = true;    bUL = true;  if( pLine ) pInsLine = pLine; }
    void SetLower( SwTableLine* pLine=0 )
        { bUL_LR = true;    bUL = false; if( pLine ) pInsLine = pLine; }
};

static void lcl_Merge_MoveLine(_FndLine & rFndLine, _InsULPara *const pULPara);

static void lcl_Merge_MoveBox(_FndBox & rFndBox, _InsULPara *const pULPara)
{
    SwTableBoxes* pBoxes;

    sal_uInt16 nStt = 0, nEnd = rFndBox.GetLines().size();
    sal_uInt16 nInsPos = USHRT_MAX;
    if( !pULPara->bUL_LR )  
    {
        sal_uInt16 nPos;
        SwTableBox* pFndTableBox = rFndBox.GetBox();
        pBoxes = &pFndTableBox->GetUpper()->GetTabBoxes();
        if( pULPara->bUL )  
        {
            
            if( 0 != ( nPos = pBoxes->GetPos( pFndTableBox ) ) )
                lcl_CpyBoxes( 0, nPos, *pBoxes, pULPara->pInsLine );
        }
        else                
            
            if( (nPos = pBoxes->GetPos( pFndTableBox )) +1 < (sal_uInt16)pBoxes->size() )
            {
                nInsPos = pULPara->pInsLine->GetTabBoxes().size();
                lcl_CpyBoxes( nPos+1, pBoxes->size(),
                                    *pBoxes, pULPara->pInsLine );
            }
    }
    
    else if (!rFndBox.GetLines().empty())
    {
        
        nStt = pULPara->bUL ? 0 : rFndBox.GetLines().size()-1;
        nEnd = nStt+1;
    }

    pBoxes = &pULPara->pInsLine->GetTabBoxes();

    
    if (rFndBox.GetBox()->GetTabLines().size())
    {
        SwTableBox* pBox = new SwTableBox(
                static_cast<SwTableBoxFmt*>(rFndBox.GetBox()->GetFrmFmt()),
                0, pULPara->pInsLine );
        _InsULPara aPara( *pULPara );
        aPara.pInsBox = pBox;
        for (_FndLines::iterator it = rFndBox.GetLines().begin() + nStt;
             it != rFndBox.GetLines().begin() + nEnd; ++it )
        {
            lcl_Merge_MoveLine(*it, &aPara );
        }
        if( pBox->GetTabLines().size() )
        {
            if( USHRT_MAX == nInsPos )
                nInsPos = pBoxes->size();
            pBoxes->insert( pBoxes->begin() + nInsPos, pBox );
            lcl_CalcWidth( pBox );      
        }
        else
            delete pBox;
    }
}

static void lcl_Merge_MoveLine(_FndLine& rFndLine, _InsULPara *const pULPara)
{
    SwTableLines* pLines;

    sal_uInt16 nStt = 0, nEnd = rFndLine.GetBoxes().size();
    sal_uInt16 nInsPos = USHRT_MAX;
    if( pULPara->bUL_LR )   
    {
        sal_uInt16 nPos;
        SwTableLine* pFndLn = (SwTableLine*)rFndLine.GetLine();
        pLines = pFndLn->GetUpper() ?
                        &pFndLn->GetUpper()->GetTabLines() :
                        &pULPara->pTblNd->GetTable().GetTabLines();

        SwTableBox* pLBx = rFndLine.GetBoxes().front().GetBox();
        SwTableBox* pRBx = rFndLine.GetBoxes().back().GetBox();
        sal_uInt16 nLeft = pFndLn->GetTabBoxes().GetPos( pLBx );
        sal_uInt16 nRight = pFndLn->GetTabBoxes().GetPos( pRBx );

        if( !nLeft || nRight == pFndLn->GetTabBoxes().size() )
        {
            if( pULPara->bUL )  
            {
                
                if( 0 != ( nPos = pLines->GetPos( pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pULPara->pInsBox );
            }
            else
                
                if( (nPos = pLines->GetPos( pFndLn )) + 1 < (sal_uInt16)pLines->size() )
                {
                    nInsPos = pULPara->pInsBox->GetTabLines().size();
                    lcl_CpyLines( nPos+1, pLines->size(), *pLines,
                                        pULPara->pInsBox );
                }
        }
        else if( nLeft )
        {
            
            
            
            
            SwTableLine* pInsLine = pULPara->pLeftBox->GetUpper();
            SwTableBox* pLMBox = new SwTableBox(
                (SwTableBoxFmt*)pULPara->pLeftBox->GetFrmFmt(), 0, pInsLine );
            SwTableLine* pLMLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pLMBox );
            pLMLn->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );

            pLMBox->GetTabLines().insert( pLMBox->GetTabLines().begin(), pLMLn );

            lcl_CpyBoxes( 0, 2, pInsLine->GetTabBoxes(), pLMLn );

            pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin(), pLMBox );

            if( pULPara->bUL )  
            {
                
                if( 0 != ( nPos = pLines->GetPos( pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pLMBox, 0 );
            }
            else
                
                if( (nPos = pLines->GetPos( pFndLn )) + 1 < (sal_uInt16)pLines->size() )
                    lcl_CpyLines( nPos+1, pLines->size(), *pLines,
                                        pLMBox );
            lcl_CalcWidth( pLMBox );        
        }
        else if( nRight+1 < (sal_uInt16)pFndLn->GetTabBoxes().size() )
        {
            
            
            
            
            SwTableLine* pInsLine = pULPara->pRightBox->GetUpper();
            SwTableBox* pRMBox;
            if( pULPara->pLeftBox->GetUpper() == pInsLine )
            {
                pRMBox = new SwTableBox(
                    (SwTableBoxFmt*)pULPara->pRightBox->GetFrmFmt(), 0, pInsLine );
                SwTableLine* pRMLn = new SwTableLine(
                    (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pRMBox );
                pRMLn->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );
                pRMBox->GetTabLines().insert( pRMBox->GetTabLines().begin(), pRMLn );

                lcl_CpyBoxes( 1, 3, pInsLine->GetTabBoxes(), pRMLn );

                pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin(), pRMBox );
            }
            else
            {
                
                pInsLine = pULPara->pLeftBox->GetUpper();
                sal_uInt16 nMvPos = pULPara->pRightBox->GetUpper()->GetTabBoxes().GetPos(
                                      pULPara->pRightBox );
                lcl_CpyBoxes( nMvPos, nMvPos+1,
                            pULPara->pRightBox->GetUpper()->GetTabBoxes(),
                            pInsLine );
                pRMBox = pInsLine->GetUpper();

                
                nMvPos = pRMBox->GetTabLines().GetPos( pInsLine );
                if( pULPara->bUL ? nMvPos != 0
                                : nMvPos+1 < (sal_uInt16)pRMBox->GetTabLines().size() )
                {
                    
                    SwTableLine* pNewLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 1, pRMBox );
                    pNewLn->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );
                    pRMBox->GetTabLines().insert(
                                pRMBox->GetTabLines().begin() + (pULPara->bUL ? nMvPos : nMvPos+1),
                                pNewLn );
                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().insert( pNewLn->GetTabBoxes().begin(), pRMBox );

                    sal_uInt16 nPos1, nPos2;
                    if( pULPara->bUL )
                        nPos1 = 0,
                        nPos2 = nMvPos;
                    else
                        nPos1 = nMvPos+2,
                        nPos2 = pNewLn->GetUpper()->GetTabLines().size();

                    lcl_CpyLines( nPos1, nPos2,
                                pNewLn->GetUpper()->GetTabLines(), pRMBox );
                    lcl_CalcWidth( pRMBox );        

                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().push_back( pRMBox );
                }
            }
            if( pULPara->bUL )  
            {
                
                if( 0 != ( nPos = pLines->GetPos( pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pRMBox, 0 );
            }
            else
                
                if( (nPos = pLines->GetPos( pFndLn )) + 1 < (sal_uInt16)pLines->size() )
                    lcl_CpyLines( nPos+1, pLines->size(), *pLines,
                                        pRMBox );
            lcl_CalcWidth( pRMBox );        
        }
        else {
            OSL_FAIL( "So ... what do we do now?" );
        }
    }
    
    else
    {
        
        nStt = pULPara->bUL ? 0 : rFndLine.GetBoxes().size()-1;
        nEnd = nStt+1;
    }
    pLines = &pULPara->pInsBox->GetTabLines();

    SwTableLine* pNewLine = new SwTableLine(
        (SwTableLineFmt*)rFndLine.GetLine()->GetFrmFmt(), 0, pULPara->pInsBox );
    _InsULPara aPara( *pULPara );       
    aPara.pInsLine = pNewLine;
    _FndBoxes & rLineBoxes = rFndLine.GetBoxes();
    for (_FndBoxes::iterator it = rLineBoxes.begin() + nStt;
         it != rLineBoxes.begin() + nEnd; ++it)
    {
        lcl_Merge_MoveBox(*it, &aPara);
    }

    if( !pNewLine->GetTabBoxes().empty() )
    {
        if( USHRT_MAX == nInsPos )
            nInsPos = pLines->size();
        pLines->insert( pLines->begin() + nInsPos, pNewLine );
    }
    else
        delete pNewLine;
}

static void lcl_BoxSetHeadCondColl( const SwTableBox* pBox );

bool SwTable::OldMerge( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        SwTableBox* pMergeBox, SwUndoTblMerge* pUndo )
{
    OSL_ENSURE( !rBoxes.empty() && pMergeBox, "no valid values" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

    
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    
    
    
    pDoc->CreateChartInternalDataProviders( this );

    SetHTMLTableLayout( 0 );    

    if( pUndo )
        pUndo->SetSelBoxes( rBoxes );

    
    aFndBox.SetTableLines( *this );
    aFndBox.DelFrms( *this );

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().size() &&
            1 == pFndBox->GetLines().front().GetBoxes().size() )
    {
        pFndBox = &pFndBox->GetLines().front().GetBoxes().front();
    }

    SwTableLine* pInsLine = new SwTableLine(
                (SwTableLineFmt*)pFndBox->GetLines().front().GetLine()->GetFrmFmt(), 0,
                !pFndBox->GetUpper() ? 0 : pFndBox->GetBox() );
    pInsLine->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );

    
    SwTableLines* pLines =  pFndBox->GetUpper() ?
                  &pFndBox->GetBox()->GetTabLines() :  &GetTabLines();

    SwTableLine* pNewLine = pFndBox->GetLines().front().GetLine();
    sal_uInt16 nInsPos = pLines->GetPos( pNewLine );
    pLines->insert( pLines->begin() + nInsPos, pInsLine );

    SwTableBox* pLeftBox = new SwTableBox( (SwTableBoxFmt*)pMergeBox->GetFrmFmt(), 0, pInsLine );
    SwTableBox* pRightBox = new SwTableBox( (SwTableBoxFmt*)pMergeBox->GetFrmFmt(), 0, pInsLine );
    pMergeBox->SetUpper( pInsLine );
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin(), pLeftBox );
    pLeftBox->ClaimFrmFmt();
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin() + 1, pMergeBox);
    pInsLine->GetTabBoxes().insert( pInsLine->GetTabBoxes().begin() + 2, pRightBox );
    pRightBox->ClaimFrmFmt();

    
    
    _InsULPara aPara( pTblNd, true, true, pLeftBox, pMergeBox, pRightBox, pInsLine );

    
    _FndBoxes& rLineBoxes = pFndBox->GetLines().front().GetBoxes();
    for (_FndBoxes::iterator it = rLineBoxes.begin(); it != rLineBoxes.end(); ++it)
    {
        lcl_Merge_MoveBox(*it, &aPara);
    }
    aPara.SetLower( pInsLine );
    sal_uInt16 nEnd = pFndBox->GetLines().size()-1;
    rLineBoxes = pFndBox->GetLines()[nEnd].GetBoxes();
    for (_FndBoxes::iterator it = rLineBoxes.begin(); it != rLineBoxes.end(); ++it)
    {
        lcl_Merge_MoveBox(*it, &aPara);
    }

    
    aPara.SetLeft( pLeftBox );
    BOOST_FOREACH(_FndLine& rFndLine, pFndBox->GetLines() )
        lcl_Merge_MoveLine( rFndLine, &aPara );

    aPara.SetRight( pRightBox );
    BOOST_FOREACH(_FndLine& rFndLine, pFndBox->GetLines() )
        lcl_Merge_MoveLine( rFndLine, &aPara );

    if( pLeftBox->GetTabLines().empty() )
        _DeleteBox( *this, pLeftBox, 0, false, false );
    else
    {
        lcl_CalcWidth( pLeftBox );      
        if( pUndo && pLeftBox->GetSttNd() )
            pUndo->AddNewBox( pLeftBox->GetSttIdx() );
    }
    if( pRightBox->GetTabLines().empty() )
        _DeleteBox( *this, pRightBox, 0, false, false );
    else
    {
        lcl_CalcWidth( pRightBox );     
        if( pUndo && pRightBox->GetSttNd() )
            pUndo->AddNewBox( pRightBox->GetSttIdx() );
    }

    DeleteSel( pDoc, rBoxes, 0, 0, false, false );

    
    GCLines();

    for( SwTableBoxes::iterator it = GetTabLines()[0]->GetTabBoxes().begin();
             it != GetTabLines()[0]->GetTabBoxes().end(); ++it)
        lcl_BoxSetHeadCondColl(*it);

    aFndBox.MakeFrms( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT

    return true;
}

static void lcl_CheckRowSpan( SwTable &rTbl )
{
    sal_uInt16 nLineCount = rTbl.GetTabLines().size();
    sal_uInt16 nMaxSpan = nLineCount;
    long nMinSpan = 1;
    while( nMaxSpan )
    {
        SwTableLine* pLine = rTbl.GetTabLines()[ nLineCount - nMaxSpan ];
        for( sal_uInt16 nBox = 0; nBox < pLine->GetTabBoxes().size(); ++nBox )
        {
            SwTableBox* pBox = pLine->GetTabBoxes()[nBox];
            long nRowSpan = pBox->getRowSpan();
            if( nRowSpan > nMaxSpan )
                pBox->setRowSpan( nMaxSpan );
            else if( nRowSpan < nMinSpan )
                pBox->setRowSpan( nMinSpan > 0 ? nMaxSpan : nMinSpan );
        }
        --nMaxSpan;
        nMinSpan = -nMaxSpan;
    }
}

static sal_uInt16 lcl_GetBoxOffset( const _FndBox& rBox )
{
    
    const _FndBox* pFirstBox = &rBox;
    while( !pFirstBox->GetLines().empty() )
        pFirstBox = &pFirstBox->GetLines().front().GetBoxes().front();

    sal_uInt16 nRet = 0;
    
    const SwTableBox* pBox = pFirstBox->GetBox();
    do {
        const SwTableBoxes& rBoxes = pBox->GetUpper()->GetTabBoxes();
        const SwTableBox* pCmp;
        for( sal_uInt16 n = 0; pBox != ( pCmp = rBoxes[ n ] ); ++n )
            nRet = nRet + (sal_uInt16) pCmp->GetFrmFmt()->GetFrmSize().GetWidth();
        pBox = pBox->GetUpper()->GetUpper();
    } while( pBox );
    return nRet;
}

static sal_uInt16 lcl_GetLineWidth( const _FndLine& rLine )
{
    sal_uInt16 nRet = 0;
    for( sal_uInt16 n = rLine.GetBoxes().size(); n; )
    {
        nRet = nRet + static_cast<sal_uInt16>(rLine.GetBoxes()[--n].GetBox()
                            ->GetFrmFmt()->GetFrmSize().GetWidth());
    }
    return nRet;
}

static void lcl_CalcNewWidths( const _FndLines& rFndLines, _CpyPara& rPara )
{
    rPara.pWidths.reset();
    sal_uInt16 nLineCount = rFndLines.size();
    if( nLineCount )
    {
        rPara.pWidths = boost::shared_ptr< std::vector< std::vector< sal_uLong > > >
                        ( new std::vector< std::vector< sal_uLong > >( nLineCount ));
        
        
        for( sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            std::vector< sal_uLong > &rWidth = (*rPara.pWidths.get())[ nLine ];
            const _FndLine *pFndLine = &rFndLines[ nLine ];
            if( pFndLine && pFndLine->GetBoxes().size() )
            {
                const SwTableLine *pLine = pFndLine->GetLine();
                if( pLine && !pLine->GetTabBoxes().empty() )
                {
                    sal_uInt16 nBoxCount = pLine->GetTabBoxes().size();
                    sal_uLong nPos = 0;
                    
                    const SwTableBox *const pSel =
                        pFndLine->GetBoxes().front().GetBox();
                    sal_uInt16 nBox = 0;
                    
                    while( nBox < nBoxCount )
                    {
                        SwTableBox* pBox = pLine->GetTabBoxes()[nBox++];
                        if( pBox != pSel )
                            nPos += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
                        else
                            break;
                    }
                    
                    if( rPara.nMinLeft > nPos )
                        rPara.nMinLeft = nPos;
                    nBoxCount = pFndLine->GetBoxes().size();
                    rWidth = std::vector< sal_uLong >( nBoxCount+2 );
                    rWidth[ 0 ] = nPos;
                    
                    
                    for( nBox = 0; nBox < nBoxCount; )
                    {
                        nPos += pFndLine->GetBoxes()[nBox]
                            .GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
                        rWidth[ ++nBox ] = nPos;
                    }
                    
                    if( rPara.nMaxRight < nPos )
                        rPara.nMaxRight = nPos;
                    if( nPos <= rWidth[ 0 ] )
                        rWidth.clear();
                }
            }
        }
    }
    
    sal_uLong nSelSize = rPara.nMaxRight - rPara.nMinLeft;
    if( nSelSize )
    {
        for( sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            std::vector< sal_uLong > &rWidth = (*rPara.pWidths.get())[ nLine ];
            sal_uInt16 nCount = (sal_uInt16)rWidth.size();
            if( nCount > 2 )
            {
                rWidth[ nCount - 1 ] = rPara.nMaxRight;
                sal_uLong nLastPos = 0;
                for( sal_uInt16 nBox = 0; nBox < nCount; ++nBox )
                {
                    sal_uInt64 nNextPos = rWidth[ nBox ];
                    nNextPos -= rPara.nMinLeft;
                    nNextPos *= rPara.nNewSize;
                    nNextPos /= nSelSize;
                    rWidth[ nBox ] = (sal_uLong)(nNextPos - nLastPos);
                    nLastPos = (sal_uLong)nNextPos;
                }
            }
        }
    }
}

static void
lcl_CopyLineToDoc(_FndLine const& rpFndLn, _CpyPara *const pCpyPara);

static void lcl_CopyBoxToDoc(_FndBox const& rFndBox, _CpyPara *const pCpyPara)
{
    
    sal_uLong nRealSize;
    sal_uLong nDummy1 = 0;
    sal_uLong nDummy2 = 0;
    if( pCpyPara->pTblNd->GetTable().IsNewModel() )
    {
        if( pCpyPara->nBoxIdx == 1 )
            nDummy1 = (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx][0];
        nRealSize = (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx][pCpyPara->nBoxIdx++];
        if( pCpyPara->nBoxIdx == (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx].size()-1 )
            nDummy2 = (*pCpyPara->pWidths.get())[pCpyPara->nLnIdx][pCpyPara->nBoxIdx];
    }
    else
    {
        nRealSize = pCpyPara->nNewSize;
        nRealSize *= rFndBox.GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
        nRealSize /= pCpyPara->nOldSize;
    }

    sal_uLong nSize;
    bool bDummy = nDummy1 > 0;
    if( bDummy )
        nSize = nDummy1;
    else
    {
        nSize = nRealSize;
        nRealSize = 0;
    }
    do
    {
        
        _CpyTabFrm aFindFrm(static_cast<SwTableBoxFmt*>(rFndBox.GetBox()->GetFrmFmt()));

        SwFmtFrmSize aFrmSz;
        _CpyTabFrms::const_iterator itFind = pCpyPara->rTabFrmArr.lower_bound( aFindFrm );
        sal_uInt16 nFndPos = itFind - pCpyPara->rTabFrmArr.begin();
        if( itFind == pCpyPara->rTabFrmArr.end() || !(*itFind == aFindFrm) ||
            ( aFrmSz = ( aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ]).pNewFrmFmt->
                GetFrmSize()).GetWidth() != (SwTwips)nSize )
        {
            
            aFindFrm.pNewFrmFmt = pCpyPara->pDoc->MakeTableBoxFmt();
            aFindFrm.pNewFrmFmt->CopyAttrs( *rFndBox.GetBox()->GetFrmFmt() );
            if( !pCpyPara->bCpyCntnt )
                aFindFrm.pNewFrmFmt->ResetFmtAttr(  RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
            aFrmSz.SetWidth( nSize );
            aFindFrm.pNewFrmFmt->SetFmtAttr( aFrmSz );
            pCpyPara->rTabFrmArr.insert( aFindFrm );
        }

        SwTableBox* pBox;
        if (!rFndBox.GetLines().empty())
        {
            pBox = new SwTableBox( aFindFrm.pNewFrmFmt,
                        rFndBox.GetLines().size(), pCpyPara->pInsLine );
            pCpyPara->pInsLine->GetTabBoxes().insert( pCpyPara->pInsLine->GetTabBoxes().begin() + pCpyPara->nInsPos++, pBox );
            _CpyPara aPara( *pCpyPara, pBox );
            aPara.nNewSize = nSize;     
            BOOST_FOREACH(_FndLine const& rFndLine, rFndBox.GetLines())
                lcl_CopyLineToDoc( rFndLine, &aPara );
        }
        else
        {
            
            pCpyPara->pDoc->GetNodes().InsBoxen( pCpyPara->pTblNd, pCpyPara->pInsLine,
                            aFindFrm.pNewFrmFmt,
                            (SwTxtFmtColl*)pCpyPara->pDoc->GetDfltTxtFmtColl(),
                            0, pCpyPara->nInsPos );
            pBox = pCpyPara->pInsLine->GetTabBoxes()[ pCpyPara->nInsPos ];
            if( bDummy )
                pBox->setDummyFlag( true );
            else if( pCpyPara->bCpyCntnt )
            {
                
                pBox->setRowSpan(rFndBox.GetBox()->getRowSpan());

                
                {
                    SfxItemSet aBoxAttrSet( pCpyPara->pDoc->GetAttrPool(),
                                            RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
                    aBoxAttrSet.Put(rFndBox.GetBox()->GetFrmFmt()->GetAttrSet());
                    if( aBoxAttrSet.Count() )
                    {
                        const SfxPoolItem* pItem;
                        SvNumberFormatter* pN = pCpyPara->pDoc->GetNumberFormatter( sal_False );
                        if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == aBoxAttrSet.
                            GetItemState( RES_BOXATR_FORMAT, false, &pItem ) )
                        {
                            sal_uLong nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
                            sal_uLong nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                            if( nNewIdx != nOldIdx )
                                aBoxAttrSet.Put( SwTblBoxNumFormat( nNewIdx ));
                        }
                        pBox->ClaimFrmFmt()->SetFmtAttr( aBoxAttrSet );
                    }
                }
                SwDoc* pFromDoc = rFndBox.GetBox()->GetFrmFmt()->GetDoc();
                SwNodeRange aCpyRg( *rFndBox.GetBox()->GetSttNd(), 1,
                        *rFndBox.GetBox()->GetSttNd()->EndOfSectionNode() );
                SwNodeIndex aInsIdx( *pBox->GetSttNd(), 1 );

                pFromDoc->CopyWithFlyInFly( aCpyRg, 0, aInsIdx, NULL, sal_False );
                
                pCpyPara->pDoc->GetNodes().Delete( aInsIdx, 1 );
            }
            ++pCpyPara->nInsPos;
        }
        if( nRealSize )
        {
            bDummy = false;
            nSize = nRealSize;
            nRealSize = 0;
        }
        else
        {
            bDummy = true;
            nSize = nDummy2;
            nDummy2 = 0;
        }
    }
    while( nSize );
}

static void
lcl_CopyLineToDoc(const _FndLine& rFndLine, _CpyPara *const pCpyPara)
{
    
    _CpyTabFrm aFindFrm( (SwTableBoxFmt*)rFndLine.GetLine()->GetFrmFmt() );
    _CpyTabFrms::const_iterator itFind = pCpyPara->rTabFrmArr.find( aFindFrm );
    if( itFind == pCpyPara->rTabFrmArr.end() )
    {
        
        aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pCpyPara->pDoc->MakeTableLineFmt();
        aFindFrm.pNewFrmFmt->CopyAttrs( *rFndLine.GetLine()->GetFrmFmt() );
        pCpyPara->rTabFrmArr.insert( aFindFrm );
    }
    else
        aFindFrm = *itFind;

    SwTableLine* pNewLine = new SwTableLine( (SwTableLineFmt*)aFindFrm.pNewFrmFmt,
                        rFndLine.GetBoxes().size(), pCpyPara->pInsBox );
    if( pCpyPara->pInsBox )
    {
        SwTableLines& rLines = pCpyPara->pInsBox->GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine );
    }
    else
    {
        SwTableLines& rLines = pCpyPara->pTblNd->GetTable().GetTabLines();
        rLines.insert( rLines.begin() + pCpyPara->nInsPos++, pNewLine);
    }

    _CpyPara aPara( *pCpyPara, pNewLine );

    if( pCpyPara->pTblNd->GetTable().IsNewModel() )
    {
        aPara.nOldSize = 0; 
        aPara.nBoxIdx = 1;
    }
    else if( rFndLine.GetBoxes().size() ==
                    rFndLine.GetLine()->GetTabBoxes().size() )
    {
        
        const SwFrmFmt* pFmt;

        if( rFndLine.GetLine()->GetUpper() )
            pFmt = rFndLine.GetLine()->GetUpper()->GetFrmFmt();
        else
            pFmt = pCpyPara->pTblNd->GetTable().GetFrmFmt();
        aPara.nOldSize = pFmt->GetFrmSize().GetWidth();
    }
    else
        
        for( sal_uInt16 n = 0; n < rFndLine.GetBoxes().size(); ++n )
        {
            aPara.nOldSize += rFndLine.GetBoxes()[n]
                        .GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
        }

    const _FndBoxes& rBoxes = rFndLine.GetBoxes();
    for (_FndBoxes::const_iterator it = rBoxes.begin(); it != rBoxes.end(); ++it)
        lcl_CopyBoxToDoc(*it, &aPara);
    if( pCpyPara->pTblNd->GetTable().IsNewModel() )
        ++pCpyPara->nLnIdx;
}

bool SwTable::CopyHeadlineIntoTable( SwTableNode& rTblNd )
{
    
    SwSelBoxes aSelBoxes;
    SwTableBox* pBox = GetTabSortBoxes()[ 0 ];
    pBox = GetTblBox( pBox->GetSttNd()->StartOfSectionNode()->GetIndex() + 1 );
    SelLineFromBox( pBox, aSelBoxes, true );

    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( aSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    {
        
        SwTableFmlUpdate aMsgHnt( this );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        GetFrmFmt()->GetDoc()->UpdateTblFlds( &aMsgHnt );
    }

    _CpyTabFrms aCpyFmt;
    _CpyPara aPara( &rTblNd, 1, aCpyFmt, true );
    aPara.nNewSize = aPara.nOldSize = rTblNd.GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
    
    if( IsNewModel() )
        lcl_CalcNewWidths( aFndBox.GetLines(), aPara );
    BOOST_FOREACH( _FndLine& rFndLine, aFndBox.GetLines() )
         lcl_CopyLineToDoc( rFndLine, &aPara );
    if( rTblNd.GetTable().IsNewModel() )
    {   
        SwTableLine* pLine = rTblNd.GetTable().GetTabLines()[0];
        sal_uInt16 nColCount = pLine->GetTabBoxes().size();
        OSL_ENSURE( nColCount, "Empty Table Line" );
        for( sal_uInt16 nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
        {
            SwTableBox* pTableBox = pLine->GetTabBoxes()[nCurrCol];
            OSL_ENSURE( pTableBox, "Missing Table Box" );
            pTableBox->setRowSpan( 1 );
        }
    }

    return true;
}

bool SwTable::MakeCopy( SwDoc* pInsDoc, const SwPosition& rPos,
                        const SwSelBoxes& rSelBoxes, bool bCpyNds,
                        bool bCpyName ) const
{
    
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( (SwTableLines&)GetTabLines(), &aPara );
    }
    if( aFndBox.GetLines().empty() )
        return false;

    
    
    SwDoc* pSrcDoc = GetFrmFmt()->GetDoc();
    if( pSrcDoc != pInsDoc )
    {
        pInsDoc->CopyTxtColl( *pSrcDoc->GetTxtCollFromPool( RES_POOLCOLL_TABLE ) );
        pInsDoc->CopyTxtColl( *pSrcDoc->GetTxtCollFromPool( RES_POOLCOLL_TABLE_HDLN ) );
    }

    SwTable* pNewTbl = (SwTable*)pInsDoc->InsertTable(
            SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
            rPos, 1, 1, GetFrmFmt()->GetHoriOrient().GetHoriOrient(),
            0, 0, sal_False, IsNewModel() );
    if( !pNewTbl )
        return false;

    SwNodeIndex aIdx( rPos.nNode, -1 );
    SwTableNode* pTblNd = aIdx.GetNode().FindTableNode();
    ++aIdx;
    OSL_ENSURE( pTblNd, "Where is the TableNode now?" );

    pTblNd->GetTable().SetRowsToRepeat( GetRowsToRepeat() );

    if( IS_TYPE( SwDDETable, this ))
    {
        
        
        SwFieldType* pFldType = pInsDoc->InsertFldType(
                                    *((SwDDETable*)this)->GetDDEFldType() );
        OSL_ENSURE( pFldType, "unknown FieldType" );

        
        pNewTbl = new SwDDETable( *pNewTbl,
                                 (SwDDEFieldType*)pFldType );
        pTblNd->SetNewTable( pNewTbl, sal_False );
    }

    pNewTbl->GetFrmFmt()->CopyAttrs( *GetFrmFmt() );
    pNewTbl->SetTblChgMode( GetTblChgMode() );

    
    pTblNd->DelFrms();

    {
        
        SwTableFmlUpdate aMsgHnt( this );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pSrcDoc->UpdateTblFlds( &aMsgHnt );
    }

    SwTblNumFmtMerge aTNFM( *pSrcDoc, *pInsDoc );

    
    if( bCpyName )
        pNewTbl->GetFrmFmt()->SetName( GetFrmFmt()->GetName() );

    _CpyTabFrms aCpyFmt;
    _CpyPara aPara( pTblNd, 1, aCpyFmt, bCpyNds );
    aPara.nNewSize = aPara.nOldSize = GetFrmFmt()->GetFrmSize().GetWidth();

    if( IsNewModel() )
        lcl_CalcNewWidths( aFndBox.GetLines(), aPara );
    
    BOOST_FOREACH( _FndLine& rFndLine, aFndBox.GetLines() )
         lcl_CopyLineToDoc( rFndLine, &aPara );

    
    {
        _FndLine* pFndLn = &aFndBox.GetLines().front();
        SwTableLine* pLn = pFndLn->GetLine();
        const SwTableLine* pTmp = pLn;
        sal_uInt16 nLnPos = GetTabLines().GetPos( pTmp );
        if( USHRT_MAX != nLnPos && nLnPos )
        {
            
            SwCollectTblLineBoxes aLnPara( false, HEADLINE_BORDERCOPY );

            pLn = GetTabLines()[ nLnPos - 1 ];
            for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                     it != pLn->GetTabBoxes().end(); ++it)
                sw_Box_CollectBox( *it, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( true );
                pLn = pNewTbl->GetTabLines()[ 0 ];
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_BoxSetSplitBoxFmts(*it, &aLnPara );
            }
        }

        pFndLn = &aFndBox.GetLines().back();
        pLn = pFndLn->GetLine();
        pTmp = pLn;
        nLnPos = GetTabLines().GetPos( pTmp );
        if( nLnPos < GetTabLines().size() - 1 )
        {
            
            SwCollectTblLineBoxes aLnPara( true, HEADLINE_BORDERCOPY );

            pLn = GetTabLines()[ nLnPos + 1 ];
            for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                     it != pLn->GetTabBoxes().end(); ++it)
                sw_Box_CollectBox( *it, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( false );
                pLn = pNewTbl->GetTabLines().back();
                for( SwTableBoxes::iterator it = pLn->GetTabBoxes().begin();
                         it != pLn->GetTabBoxes().end(); ++it)
                    sw_BoxSetSplitBoxFmts(*it, &aLnPara );
            }
        }
    }

    
    _DeleteBox( *pNewTbl, pNewTbl->GetTabLines().back()->GetTabBoxes()[0],
                0, false, false );

    if( pNewTbl->IsNewModel() )
        lcl_CheckRowSpan( *pNewTbl );
    
    pNewTbl->GCLines();

    pTblNd->MakeFrms( &aIdx );  

    CHECKTABLELAYOUT

    return true;
}


SwTableBox* SwTableLine::FindNextBox( const SwTable& rTbl,
                     const SwTableBox* pSrchBox, bool bOvrTblLns ) const
{
    const SwTableLine* pLine = this;            
    SwTableBox* pBox;
    sal_uInt16 nFndPos;
    if( !GetTabBoxes().empty() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetTabBoxes().GetPos( pSrchBox )) &&
        nFndPos + 1 != (sal_uInt16)GetTabBoxes().size() )
    {
        pBox = GetTabBoxes()[ nFndPos + 1 ];
        while( !pBox->GetTabLines().empty() )
            pBox = pBox->GetTabLines().front()->GetTabBoxes()[0];
        return pBox;
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line is not in the Table" );
        
        if( nFndPos+1 >= (sal_uInt16)GetUpper()->GetTabLines().size() )
            return GetUpper()->GetUpper()->FindNextBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos+1];
    }
    else if( bOvrTblLns )       
    {
        
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( nFndPos + 1 >= (sal_uInt16)rTbl.GetTabLines().size() )
            return 0;           

        pLine = rTbl.GetTabLines()[ nFndPos+1 ];
    }
    else
        return 0;

    if( !pLine->GetTabBoxes().empty() )
    {
        pBox = pLine->GetTabBoxes().front();
        while( !pBox->GetTabLines().empty() )
            pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
        return pBox;
    }
    return pLine->FindNextBox( rTbl, 0, bOvrTblLns );
}


SwTableBox* SwTableLine::FindPreviousBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, bool bOvrTblLns ) const
{
    const SwTableLine* pLine = this;            
    SwTableBox* pBox;
    sal_uInt16 nFndPos;
    if( !GetTabBoxes().empty() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetTabBoxes().GetPos( pSrchBox )) &&
        nFndPos )
    {
        pBox = GetTabBoxes()[ nFndPos - 1 ];
        while( !pBox->GetTabLines().empty() )
        {
            pLine = pBox->GetTabLines().back();
            pBox = pLine->GetTabBoxes().back();
        }
        return pBox;
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line is not in the Table" );
        
        if( !nFndPos )
            return GetUpper()->GetUpper()->FindPreviousBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos-1];
    }
    else if( bOvrTblLns )       
    {
        
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( !nFndPos )
            return 0;           

        pLine = rTbl.GetTabLines()[ nFndPos-1 ];
    }
    else
        return 0;

    if( !pLine->GetTabBoxes().empty() )
    {
        pBox = pLine->GetTabBoxes().back();
        while( !pBox->GetTabLines().empty() )
        {
            pLine = pBox->GetTabLines().back();
            pBox = pLine->GetTabBoxes().back();
        }
        return pBox;
    }
    return pLine->FindPreviousBox( rTbl, 0, bOvrTblLns );
}


SwTableBox* SwTableBox::FindNextBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, bool bOvrTblLns ) const
{
    if( !pSrchBox  && GetTabLines().empty() )
        return (SwTableBox*)this;
    return GetUpper()->FindNextBox( rTbl, pSrchBox ? pSrchBox : this,
                                        bOvrTblLns );

}


SwTableBox* SwTableBox::FindPreviousBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, bool bOvrTblLns ) const
{
    if( !pSrchBox && GetTabLines().empty() )
        return (SwTableBox*)this;
    return GetUpper()->FindPreviousBox( rTbl, pSrchBox ? pSrchBox : this,
                                        bOvrTblLns );
}

static void lcl_BoxSetHeadCondColl( const SwTableBox* pBox )
{
    
    const SwStartNode* pSttNd = pBox->GetSttNd();
    if( pSttNd )
        pSttNd->CheckSectionCondColl();
    else
        BOOST_FOREACH( const SwTableLine* pLine, pBox->GetTabLines() )
            sw_LineSetHeadCondColl( pLine );
}

void sw_LineSetHeadCondColl( const SwTableLine* pLine )
{
    BOOST_FOREACH( const SwTableBox* pBox, pLine->GetTabBoxes() )
        lcl_BoxSetHeadCondColl(pBox);
}

static SwTwips lcl_GetDistance( SwTableBox* pBox, bool bLeft )
{
    bool bFirst = true;
    SwTwips nRet = 0;
    SwTableLine* pLine;
    while( pBox && 0 != ( pLine = pBox->GetUpper() ) )
    {
        sal_uInt16 nStt = 0, nPos = pLine->GetTabBoxes().GetPos( pBox );

        if( bFirst && !bLeft )
            ++nPos;
        bFirst = false;

        while( nStt < nPos )
            nRet += pLine->GetTabBoxes()[ nStt++ ]->GetFrmFmt()
                            ->GetFrmSize().GetWidth();
        pBox = pLine->GetUpper();
    }
    return nRet;
}

static bool lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();
        bool bGreaterBox = false;

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
                if( !::lcl_SetSelBoxWidth( pBox->GetTabLines()[ i ], rParam,
                                            nDist, true ))
                    return false;

            
            if( (bGreaterBox = TBLFIX_CHGABS != rParam.nMode && ( nDist + ( rParam.bLeft ? 0 : nWidth ) ) >= rParam.nSide) ||
                ( !rParam.bBigger && ( std::abs( nDist + (( rParam.nMode && rParam.bLeft ) ? 0 : nWidth ) - rParam.nSide ) < COLFUZZY ) ) )
            {
                rParam.bAnyBoxFnd = true;
                SwTwips nLowerDiff;
                if( bGreaterBox && TBLFIX_CHGPROP == rParam.nMode )
                {
                    
                    nLowerDiff = (nDist + ( rParam.bLeft ? 0 : nWidth ) ) - rParam.nSide;
                    nLowerDiff *= rParam.nDiff;
                    nLowerDiff /= rParam.nMaxSize;
                    nLowerDiff = rParam.nDiff - nLowerDiff;
                }
                else
                    nLowerDiff = rParam.nDiff;

                if( nWidth < nLowerDiff || nWidth - nLowerDiff < MINLAY )
                    return false;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_SetSelBoxWidth( pBox->GetTabLines()[ i ], rParam, nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                 (bGreaterBox = !nOldLower && TBLFIX_CHGABS != rParam.nMode &&
                    ( nDist + ( rParam.bLeft ? 0 : nWidth ) ) >= rParam.nSide) ||
                ( std::abs( nDist + ( (rParam.nMode && rParam.bLeft) ? 0 : nWidth )
                            - rParam.nSide ) < COLFUZZY ))
            {
                
                SwFmtFrmSize aNew( rSz );

                if( !nLowerDiff )
                {
                    if( bGreaterBox && TBLFIX_CHGPROP == rParam.nMode )
                    {
                        
                        nLowerDiff = (nDist + ( rParam.bLeft ? 0 : nWidth ) ) - rParam.nSide;
                        nLowerDiff *= rParam.nDiff;
                        nLowerDiff /= rParam.nMaxSize;
                        nLowerDiff = rParam.nDiff - nLowerDiff;
                    }
                    else
                        nLowerDiff = rParam.nDiff;
                }

                rParam.nLowerDiff += nLowerDiff;

                if( rParam.bBigger )
                    aNew.SetWidth( nWidth + nLowerDiff );
                else
                    aNew.SetWidth( nWidth - nLowerDiff );
                rParam.aShareFmts.SetSize( *pBox, aNew );
                break;
            }
        }

        if( rParam.bLeft && rParam.nMode && nDist >= rParam.nSide )
            break;

        nDist += nWidth;

        
        if( ( TBLFIX_CHGABS == rParam.nMode || !rParam.bLeft ) &&
                nDist >= rParam.nSide )
            break;
    }
    return true;
}

static bool lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
                if( !::lcl_SetOtherBoxWidth( pBox->GetTabLines()[ i ],
                                                    rParam, nDist, true ))
                    return false;

            if( rParam.bBigger && ( TBLFIX_CHGABS == rParam.nMode
                    ? std::abs( nDist - rParam.nSide ) < COLFUZZY
                    : ( rParam.bLeft ? nDist < rParam.nSide - COLFUZZY
                                     : nDist >= rParam.nSide - COLFUZZY )) )
            {
                rParam.bAnyBoxFnd = true;
                SwTwips nDiff;
                if( TBLFIX_CHGPROP == rParam.nMode )        
                {
                    
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nMaxSize;
                }
                else
                    nDiff = rParam.nDiff;

                if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                    return false;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_SetOtherBoxWidth( pBox->GetTabLines()[ i ], rParam,
                                            nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                ( TBLFIX_CHGABS == rParam.nMode
                        ? std::abs( nDist - rParam.nSide ) < COLFUZZY
                        : ( rParam.bLeft ? nDist < rParam.nSide - COLFUZZY
                                         : nDist >= rParam.nSide - COLFUZZY)
                 ) )
            {
                SwFmtFrmSize aNew( rSz );

                if( !nLowerDiff )
                {
                    if( TBLFIX_CHGPROP == rParam.nMode )        
                    {
                        
                        nLowerDiff = nWidth;
                        nLowerDiff *= rParam.nDiff;
                        nLowerDiff /= rParam.nMaxSize;
                    }
                    else
                        nLowerDiff = rParam.nDiff;
                }

                rParam.nLowerDiff += nLowerDiff;

                if( rParam.bBigger )
                    aNew.SetWidth( nWidth - nLowerDiff );
                else
                    aNew.SetWidth( nWidth + nLowerDiff );

                rParam.aShareFmts.SetSize( *pBox, aNew );
            }
        }

        nDist += nWidth;
        if( ( TBLFIX_CHGABS == rParam.nMode || rParam.bLeft ) &&
            nDist > rParam.nSide )
            break;
    }
    return true;
}

static bool lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                            SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    sal_uInt16 n, nCmp;
    for( n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwTableBoxFmt* pFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
                if( !::lcl_InsSelBox( pBox->GetTabLines()[ i ], rParam,
                                            nDist, true ))
                    return false;

            
            if( std::abs( nDist + ( rParam.bLeft ? 0 : nWidth )
                    - rParam.nSide ) < COLFUZZY )
                nCmp = 1;
            else if( nDist + ( rParam.bLeft ? 0 : nWidth/2 ) > rParam.nSide )
                nCmp = 2;
            else
                nCmp = 0;

            if( nCmp )
            {
                rParam.bAnyBoxFnd = true;
                if( pFmt->GetProtect().IsCntntProtected() )
                    return false;

                if( rParam.bSplittBox &&
                    nWidth - rParam.nDiff <= COLFUZZY +
                        ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                    return false;

                if( pBox->GetSttNd() )
                {
                    rParam.m_Boxes.insert(pBox);
                }

                break;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_InsSelBox( pBox->GetTabLines()[ i ], rParam, nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff )
                nCmp = 1;
            else if( std::abs( nDist + ( rParam.bLeft ? 0 : nWidth )
                                - rParam.nSide ) < COLFUZZY )
                nCmp = 2;
            else if( nDist + nWidth / 2 > rParam.nSide )
                nCmp = 3;
            else
                nCmp = 0;

            if( nCmp )
            {
                
                if( 1 == nCmp )
                {
                    if( !rParam.bSplittBox )
                    {
                        
                        SwFmtFrmSize aNew( rSz );
                        aNew.SetWidth( nWidth + rParam.nDiff );
                        rParam.aShareFmts.SetSize( *pBox, aNew );
                    }
                }
                else
                {
                    OSL_ENSURE( pBox->GetSttNd(), "This must be an EndBox!");

                    if( !rParam.bLeft && 3 != nCmp )
                        ++n;

                    ::_InsTblBox( pFmt->GetDoc(), rParam.pTblNd,
                                        pLine, pFmt, pBox, n );

                    SwTableBox* pNewBox = rBoxes[ n ];
                    SwFmtFrmSize aNew( rSz );
                    aNew.SetWidth( rParam.nDiff );
                    rParam.aShareFmts.SetSize( *pNewBox, aNew );

                    
                    if( rParam.bSplittBox )
                    {
                        
                        SwFmtFrmSize aNewSize( rSz );
                        aNewSize.SetWidth( nWidth - rParam.nDiff );
                        rParam.aShareFmts.SetSize( *pBox, aNewSize );
                    }

                    
                    
                    {
                        const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();
                        if( rBoxItem.GetRight() )
                        {
                            SvxBoxItem aTmp( rBoxItem );
                            aTmp.SetLine( 0, BOX_LINE_RIGHT );
                            rParam.aShareFmts.SetAttr( rParam.bLeft
                                                            ? *pNewBox
                                                            : *pBox, aTmp );
                        }
                    }
                }

                rParam.nLowerDiff = rParam.nDiff;
                break;
            }
        }

        if( rParam.bLeft && rParam.nMode && nDist >= rParam.nSide )
            break;

        nDist += nWidth;
    }
    return true;
}

static bool lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, bool bCheck )
{
    
    if( rParam.bSplittBox )
        return true;

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    sal_uInt16 n;

    
    if( !rParam.nRemainWidth && TBLFIX_CHGPROP == rParam.nMode )
    {
        
        
        SwTwips nTmpDist = nDist;
        for( n = 0; n < rBoxes.size(); ++n )
        {
            SwTwips nWidth = rBoxes[ n ]->GetFrmFmt()->GetFrmSize().GetWidth();
            if( (nTmpDist + nWidth / 2 ) > rParam.nSide )
            {
                rParam.nRemainWidth = rParam.bLeft
                                        ? sal_uInt16(nTmpDist)
                                        : sal_uInt16(rParam.nTblWidth - nTmpDist);
                break;
            }
            nTmpDist += nWidth;
        }
    }

    for( n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
                if( !::lcl_InsOtherBox( pBox->GetTabLines()[ i ],
                                                    rParam, nDist, true ))
                    return false;

            if(
                rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TBLFIX_CHGABS != rParam.nMode ||
                                (n < rBoxes.size() &&
                                  (nDist + nWidth + rBoxes[ n+1 ]->
                                   GetFrmFmt()->GetFrmSize().GetWidth() / 2)
                                  > rParam.nSide) ))
                             : (nDist + nWidth / 2 ) > rParam.nSide
                )
            {
                rParam.bAnyBoxFnd = true;
                SwTwips nDiff;
                if( TBLFIX_CHGPROP == rParam.nMode )        
                {
                    
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nRemainWidth;

                    if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                        return false;
                }
                else
                {
                    nDiff = rParam.nDiff;

                    
                    
                    SwTwips nTmpWidth = nWidth;
                    if( rParam.bLeft && pBox->GetUpper()->GetUpper() )
                    {
                        const SwTableBox* pTmpBox = pBox;
                        sal_uInt16 nBoxPos = n;
                        while( !nBoxPos && pTmpBox->GetUpper()->GetUpper() )
                        {
                            pTmpBox = pTmpBox->GetUpper()->GetUpper();
                            nBoxPos = pTmpBox->GetUpper()->GetTabBoxes().GetPos( pTmpBox );
                        }
                        nTmpWidth = pTmpBox->GetFrmFmt()->GetFrmSize().GetWidth();
                    }

                    if( nTmpWidth < nDiff || nTmpWidth - nDiff < MINLAY )
                        return false;
                    break;
                }
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_InsOtherBox( pBox->GetTabLines()[ i ], rParam,
                                        nDist, false );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                (rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TBLFIX_CHGABS != rParam.nMode ||
                                (n < rBoxes.size() &&
                                  (nDist + nWidth + rBoxes[ n+1 ]->
                                   GetFrmFmt()->GetFrmSize().GetWidth() / 2)
                                  > rParam.nSide) ))
                              : (nDist + nWidth / 2 ) > rParam.nSide ))
            {
                if( !nLowerDiff )
                {
                    if( TBLFIX_CHGPROP == rParam.nMode )        
                    {
                        
                        nLowerDiff = nWidth;
                        nLowerDiff *= rParam.nDiff;
                        nLowerDiff /= rParam.nRemainWidth;
                    }
                    else
                        nLowerDiff = rParam.nDiff;
                }

                SwFmtFrmSize aNew( rSz );
                rParam.nLowerDiff += nLowerDiff;

                if( rParam.bBigger )
                    aNew.SetWidth( nWidth - nLowerDiff );
                else
                    aNew.SetWidth( nWidth + nLowerDiff );
                rParam.aShareFmts.SetSize( *pBox, aNew );

                if( TBLFIX_CHGABS == rParam.nMode )
                    break;
            }
        }

        nDist += nWidth;
    }
    return true;
}









SwComparePosition _CheckBoxInRange( sal_uInt16 nStt, sal_uInt16 nEnd,
                                    sal_uInt16 nBoxStt, sal_uInt16 nBoxEnd )
{
    
    SwComparePosition nRet;
    if( nBoxStt + COLFUZZY < nStt )
    {
        if( nBoxEnd > nStt + COLFUZZY )
        {
            if( nBoxEnd >= nEnd + COLFUZZY )
                nRet = POS_OUTSIDE;
            else
                nRet = POS_OVERLAP_BEFORE;
        }
        else
            nRet = POS_BEFORE;
    }
    else if( nEnd > nBoxStt + COLFUZZY )
    {
        if( nEnd + COLFUZZY >= nBoxEnd )
        {
            if( COLFUZZY > std::abs( long(nEnd) - long(nBoxEnd) ) &&
                COLFUZZY > std::abs( long(nStt) - long(nBoxStt) ) )
                nRet = POS_EQUAL;
            else
                nRet = POS_INSIDE;
        }
        else
            nRet = POS_OVERLAP_BEHIND;
    }
    else
        nRet = POS_BEHIND;

    return nRet;
}

static void lcl_DelSelBox_CorrLowers( SwTableLine& rLine, CR_SetBoxWidth& rParam,
                                SwTwips nWidth )
{
    
    SwTableBoxes& rBoxes = rLine.GetTabBoxes();
    SwTwips nBoxWidth = 0;
    sal_uInt16 n;

    for( n = rBoxes.size(); n; )
        nBoxWidth += rBoxes[ --n ]->GetFrmFmt()->GetFrmSize().GetWidth();

    if( COLFUZZY < std::abs( nWidth - nBoxWidth ))
    {
        
        for( n = rBoxes.size(); n; )
        {
            SwTableBox* pBox = rBoxes[ --n ];
            SwFmtFrmSize aNew( pBox->GetFrmFmt()->GetFrmSize() );
            long nDiff = aNew.GetWidth();
            nDiff *= nWidth;
            nDiff /= nBoxWidth;
            aNew.SetWidth( nDiff );

            rParam.aShareFmts.SetSize( *pBox, aNew );

            if( !pBox->GetSttNd() )
            {
                
                for( sal_uInt16 i = pBox->GetTabLines().size(); i; )
                    ::lcl_DelSelBox_CorrLowers( *pBox->GetTabLines()[ --i ],
                                                rParam, nDiff  );
            }
        }
    }
}

static void lcl_ChgBoxSize( SwTableBox& rBox, CR_SetBoxWidth& rParam,
                    const SwFmtFrmSize& rOldSz,
                    sal_uInt16& rDelWidth, SwTwips nDist )
{
    long nDiff = 0;
    bool bSetSize = false;

    switch( rParam.nMode )
    {
    case TBLFIX_CHGABS:     
        nDiff = rDelWidth + rParam.nLowerDiff;
        bSetSize = true;
        break;

    case TBLFIX_CHGPROP:    
        if( !rParam.nRemainWidth )
        {
            
            if( rParam.bLeft )
                rParam.nRemainWidth = sal_uInt16(nDist);
            else
                rParam.nRemainWidth = sal_uInt16(rParam.nTblWidth - nDist);
        }

        
        nDiff = rOldSz.GetWidth();
        nDiff *= rDelWidth + rParam.nLowerDiff;
        nDiff /= rParam.nRemainWidth;

        bSetSize = true;
        break;

    case TBLVAR_CHGABS:     
        if( COLFUZZY < std::abs( rParam.nBoxWidth -
                            ( rDelWidth + rParam.nLowerDiff )))
        {
            nDiff = rDelWidth + rParam.nLowerDiff - rParam.nBoxWidth;
            if( 0 < nDiff )
                rDelWidth = rDelWidth - sal_uInt16(nDiff);
            else
                rDelWidth = rDelWidth + sal_uInt16(-nDiff);
            bSetSize = true;
        }
        break;
    }

    if( bSetSize )
    {
        SwFmtFrmSize aNew( rOldSz );
        aNew.SetWidth( aNew.GetWidth() + nDiff );
        rParam.aShareFmts.SetSize( rBox, aNew );

        
        for( sal_uInt16 i = rBox.GetTabLines().size(); i; )
            ::lcl_DelSelBox_CorrLowers( *rBox.GetTabLines()[ --i ], rParam,
                                            aNew.GetWidth() );
    }
}

static bool lcl_DeleteBox_Rekursiv( CR_SetBoxWidth& rParam, SwTableBox& rBox,
                            bool bCheck )
{
    bool bRet = true;
    if( rBox.GetSttNd() )
    {
        if( bCheck )
        {
            rParam.bAnyBoxFnd = true;
            if( rBox.GetFrmFmt()->GetProtect().IsCntntProtected() )
                bRet = false;
            else
            {
                SwTableBox* pBox = &rBox;
                rParam.m_Boxes.insert(pBox);
            }
        }
        else
            ::_DeleteBox( rParam.pTblNd->GetTable(), &rBox,
                            rParam.pUndo, false, true, &rParam.aShareFmts );
    }
    else
    {
        
        for( sal_uInt16 i = rBox.GetTabLines().size(); i; )
        {
            SwTableLine& rLine = *rBox.GetTabLines()[ --i ];
            for( sal_uInt16 n = rLine.GetTabBoxes().size(); n; )
                if( !::lcl_DeleteBox_Rekursiv( rParam,
                                *rLine.GetTabBoxes()[ --n ], bCheck ))
                    return false;
        }
    }
    return bRet;
}

static bool lcl_DelSelBox( SwTableLine* pTabLine, CR_SetBoxWidth& rParam,
                    SwTwips nDist, bool bCheck )
{
    SwTableBoxes& rBoxes = pTabLine->GetTabBoxes();
    sal_uInt16 n, nCntEnd, nBoxChkStt, nBoxChkEnd, nDelWidth = 0;
    if( rParam.bLeft )
    {
        n = rBoxes.size();
        nCntEnd = 0;
        nBoxChkStt = (sal_uInt16)rParam.nSide;
        nBoxChkEnd = static_cast<sal_uInt16>(rParam.nSide + rParam.nBoxWidth);
    }
    else
    {
        n = 0;
        nCntEnd = rBoxes.size();
        nBoxChkStt = static_cast<sal_uInt16>(rParam.nSide - rParam.nBoxWidth);
        nBoxChkEnd = (sal_uInt16)rParam.nSide;
    }

    while( n != nCntEnd )
    {
        SwTableBox* pBox;
        if( rParam.bLeft )
            pBox = rBoxes[ --n ];
        else
            pBox = rBoxes[ n++ ];

        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        long nWidth = rSz.GetWidth();
        bool bDelBox = false, bChgLowers = false;

        
        SwComparePosition ePosType = ::_CheckBoxInRange(
                            nBoxChkStt, nBoxChkEnd,
                            sal_uInt16(rParam.bLeft ? nDist - nWidth : nDist),
                            sal_uInt16(rParam.bLeft ? nDist : nDist + nWidth));

        switch( ePosType )
        {
        case POS_BEFORE:
            if( bCheck )
            {
                if( rParam.bLeft )
                    return true;
            }
            else if( rParam.bLeft )
            {
                ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                if( TBLFIX_CHGABS == rParam.nMode )
                    n = nCntEnd;
            }
            break;

        case POS_BEHIND:
            if( bCheck )
            {
                if( !rParam.bLeft )
                    return true;
            }
            else if( !rParam.bLeft )
            {
                ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                if( TBLFIX_CHGABS == rParam.nMode )
                    n = nCntEnd;
            }
            break;

        case POS_OUTSIDE:           
        case POS_INSIDE:            
        case POS_EQUAL:             
            bDelBox = true;
            break;

        case POS_OVERLAP_BEFORE:     
            if( nBoxChkStt <= ( nDist + (rParam.bLeft ? - nWidth / 2
                                                      : nWidth / 2 )))
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = true;
                else
                    bDelBox = true;
            }
            else if( !bCheck && rParam.bLeft )
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = true;
                else
                {
                    ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                    if( TBLFIX_CHGABS == rParam.nMode )
                        n = nCntEnd;
                }
            }
            break;

        case POS_OVERLAP_BEHIND:     
            
            
            if( !pBox->GetSttNd() )
                bChgLowers = true;
            else
                bDelBox = true;
            break;
        default: break;
        }

        if( bDelBox )
        {
            nDelWidth = nDelWidth + sal_uInt16(nWidth);
            if( bCheck )
            {
                
                
                if( (( TBLVAR_CHGABS != rParam.nMode ||
                        nDelWidth != rParam.nBoxWidth ) &&
                     COLFUZZY > std::abs( rParam.bLeft
                                    ? nWidth - nDist
                                    : (nDist + nWidth - rParam.nTblWidth )))
                    || !::lcl_DeleteBox_Rekursiv( rParam, *pBox, bCheck ) )
                    return false;

                if( pFmt->GetProtect().IsCntntProtected() )
                    return false;
            }
            else
            {
                ::lcl_DeleteBox_Rekursiv( rParam, *pBox, bCheck );

                if( !rParam.bLeft )
                    --n, --nCntEnd;
            }
        }
        else if( bChgLowers )
        {
            bool bFirst = true, bCorrLowers = false;
            long nLowerDiff = 0;
            long nOldLower = rParam.nLowerDiff;
            sal_uInt16 nOldRemain = rParam.nRemainWidth;
            sal_uInt16 i;

            for( i = pBox->GetTabLines().size(); i; )
            {
                rParam.nLowerDiff = nDelWidth + nOldLower;
                rParam.nRemainWidth = nOldRemain;
                SwTableLine* pLine = pBox->GetTabLines()[ --i ];
                if( !::lcl_DelSelBox( pLine, rParam, nDist, bCheck ))
                    return false;

                
                if( n < rBoxes.size() &&
                    pBox == rBoxes[ rParam.bLeft ? n : n-1 ] &&
                    i < pBox->GetTabLines().size() &&
                    pLine == pBox->GetTabLines()[ i ] )
                {
                    if( !bFirst && !bCorrLowers &&
                        COLFUZZY < std::abs( nLowerDiff - rParam.nLowerDiff ) )
                        bCorrLowers = true;

                    
                    
                    if( nLowerDiff < rParam.nLowerDiff )
                        nLowerDiff = rParam.nLowerDiff;

                    bFirst = false;
                }
            }
            rParam.nLowerDiff = nOldLower;
            rParam.nRemainWidth = nOldRemain;

            
            if( !nLowerDiff )
                nLowerDiff = nWidth;

            
            nDelWidth = nDelWidth + sal_uInt16(nLowerDiff);

            if( !bCheck )
            {
                
                if( n > rBoxes.size() ||
                    pBox != rBoxes[ ( rParam.bLeft ? n : n-1 ) ] )
                {
                    
                    if( !rParam.bLeft )
                        --n, --nCntEnd;
                }
                else
                {
                    
                    SwFmtFrmSize aNew( rSz );
                    bool bCorrRel = false;

                    if( TBLVAR_CHGABS != rParam.nMode )
                    {
                        switch( ePosType )
                        {
                        case POS_OVERLAP_BEFORE:    
                            if( TBLFIX_CHGPROP == rParam.nMode )
                                bCorrRel = rParam.bLeft;
                            else if( rParam.bLeft ) 
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = true;
                                n = nCntEnd;
                            }
                            break;

                        case POS_OVERLAP_BEHIND:    
                            if( TBLFIX_CHGPROP == rParam.nMode )
                                bCorrRel = !rParam.bLeft;
                            else if( !rParam.bLeft )    
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = true;
                                n = nCntEnd;
                            }
                            break;

                        default:
                            OSL_ENSURE( !pBox, "we should never reach this!" );
                            break;
                        }
                    }

                    if( bCorrRel )
                    {
                        if( !rParam.nRemainWidth )
                        {
                            
                            if( rParam.bLeft )
                                rParam.nRemainWidth = sal_uInt16(nDist - nLowerDiff);
                            else
                                rParam.nRemainWidth = sal_uInt16(rParam.nTblWidth - nDist
                                                                - nLowerDiff );
                        }

                        long nDiff = aNew.GetWidth() - nLowerDiff;
                        nDiff *= nDelWidth + rParam.nLowerDiff;
                        nDiff /= rParam.nRemainWidth;

                        aNew.SetWidth( aNew.GetWidth() - nLowerDiff + nDiff );
                    }
                    else
                        aNew.SetWidth( aNew.GetWidth() - nLowerDiff );
                    rParam.aShareFmts.SetSize( *pBox, aNew );

                    if( bCorrLowers )
                    {
                        
                        for( i = pBox->GetTabLines().size(); i; )
                            ::lcl_DelSelBox_CorrLowers( *pBox->
                                GetTabLines()[ --i ], rParam, aNew.GetWidth() );
                    }
                }
            }
        }

        if( rParam.bLeft )
            nDist -= nWidth;
        else
            nDist += nWidth;
    }
    rParam.nLowerDiff = nDelWidth;
    return true;
}


static bool lcl_DelOtherBox( SwTableLine* , CR_SetBoxWidth& , SwTwips , bool )
{
    return true;
}

static void lcl_AjustLines( SwTableLine* pLine, CR_SetBoxWidth& rParam )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];

        SwFmtFrmSize aSz( pBox->GetFrmFmt()->GetFrmSize() );
        SwTwips nWidth = aSz.GetWidth();
        nWidth *= rParam.nDiff;
        nWidth /= rParam.nMaxSize;
        aSz.SetWidth( nWidth );
        rParam.aShareFmts.SetSize( *pBox, aSz );

        for( sal_uInt16 i = 0; i < pBox->GetTabLines().size(); ++i )
            ::lcl_AjustLines( pBox->GetTabLines()[ i ], rParam );
    }
}

#ifdef DBG_UTIL
void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize )
{
    const SwTableBoxes& rBoxes = rLine.GetTabBoxes();

    SwTwips nAktSize = 0;
    
    for (SwTableBoxes::const_iterator i(rBoxes.begin()); i != rBoxes.end(); ++i)
    {
        const SwTableBox* pBox = *i;
        const SwTwips nBoxW = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        nAktSize += nBoxW;

        for( sal_uInt16 j = 0; j < pBox->GetTabLines().size(); ++j )
            _CheckBoxWidth( *pBox->GetTabLines()[ j ], nBoxW );
    }

    if (sal::static_int_cast< unsigned long >(std::abs(nAktSize - nSize)) >
        (COLFUZZY * rBoxes.size()))
    {
        OSL_FAIL( "Line's Boxes are too small or too large" );
    }
}
#endif

static _FndBox* lcl_SaveInsDelData( CR_SetBoxWidth& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst, SwTwips nDistStt )
{
    
    SwTable& rTbl = rParam.pTblNd->GetTable();

    if (rParam.m_Boxes.empty())
    {
        
        if( rParam.bBigger )
            for( sal_uInt16 n = 0; n < rTbl.GetTabLines().size(); ++n )
                ::lcl_DelSelBox( rTbl.GetTabLines()[ n ], rParam, nDistStt, true );
        else
            for( sal_uInt16 n = 0; n < rTbl.GetTabLines().size(); ++n )
                ::lcl_InsSelBox( rTbl.GetTabLines()[ n ], rParam, nDistStt, true );
    }

    
    if (rParam.bBigger
        && rParam.m_Boxes.size() == rTbl.GetTabSortBoxes().size())
    {
        return 0;
    }

    _FndBox* pFndBox = new _FndBox( 0, 0 );
    if( rParam.bBigger )
        pFndBox->SetTableLines( rParam.m_Boxes, rTbl );
    else
    {
        _FndPara aPara(rParam.m_Boxes, pFndBox);
        ForEach_FndLineCopyCol( rTbl.GetTabLines(), &aPara );
        OSL_ENSURE( pFndBox->GetLines().size(), "Where are the Boxes" );
        pFndBox->SetTableLines( rTbl );

        if( ppUndo )
            rTmpLst.insert( rTbl.GetTabSortBoxes() );
    }

    
    pFndBox->DelFrms( rTbl );

    
    
    

    return pFndBox;
}

bool SwTable::SetColWidth( SwTableBox& rAktBox, sal_uInt16 eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo )
{
    SetHTMLTableLayout( 0 );    

    const SwFmtFrmSize& rSz = GetFrmFmt()->GetFrmSize();
    const SvxLRSpaceItem& rLR = GetFrmFmt()->GetLRSpace();

    boost::scoped_ptr<_FndBox> xFndBox;                
    SwTableSortBoxes aTmpLst;       
    bool bBigger,
        bRet = false,
        bLeft = nsTblChgWidthHeightType::WH_COL_LEFT == ( eType & 0xff ) ||
                nsTblChgWidthHeightType::WH_CELL_LEFT == ( eType & 0xff ),
        bInsDel = 0 != (eType & nsTblChgWidthHeightType::WH_FLAG_INSDEL );
    sal_uInt16 n;
    sal_uLong nBoxIdx = rAktBox.GetSttIdx();

    
    
    const SwTwips nDist = ::lcl_GetDistance( &rAktBox, bLeft );
    SwTwips nDistStt = 0;
    CR_SetBoxWidth aParam( eType, nRelDiff, nDist, rSz.GetWidth(),
                            bLeft ? nDist : rSz.GetWidth() - nDist,
                            (SwTableNode*)rAktBox.GetSttNd()->FindTableNode() );
    bBigger = aParam.bBigger;

    FN_lcl_SetBoxWidth fnSelBox, fnOtherBox;
    if( bInsDel )
    {
        if( bBigger )
        {
            fnSelBox = lcl_DelSelBox;
            fnOtherBox = lcl_DelOtherBox;
            aParam.nBoxWidth = (sal_uInt16)rAktBox.GetFrmFmt()->GetFrmSize().GetWidth();
            if( bLeft )
                nDistStt = rSz.GetWidth();
        }
        else
        {
            fnSelBox = lcl_InsSelBox;
            fnOtherBox = lcl_InsOtherBox;
        }
    }
    else
    {
        fnSelBox = lcl_SetSelBoxWidth;
        fnOtherBox = lcl_SetOtherBoxWidth;
    }

    switch( eType & 0xff )
    {
    case nsTblChgWidthHeightType::WH_COL_RIGHT:
    case nsTblChgWidthHeightType::WH_COL_LEFT:
        if( TBLVAR_CHGABS == eTblChgMode )
        {
            if( bInsDel )
                bBigger = !bBigger;

            
            bool bChgLRSpace = true;
            if( bBigger )
            {
                if( GetFrmFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
                    !rSz.GetWidthPercent() )
                {
                    bRet = rSz.GetWidth() < USHRT_MAX - nRelDiff;
                    bChgLRSpace = bLeft ? rLR.GetLeft() >= nAbsDiff
                                        : rLR.GetRight() >= nAbsDiff;
                }
                else
                    bRet = bLeft ? rLR.GetLeft() >= nAbsDiff
                                 : rLR.GetRight() >= nAbsDiff;

                if( !bRet && bInsDel &&
                    
                    ( bLeft ? rLR.GetRight() >= nAbsDiff
                            : rLR.GetLeft() >= nAbsDiff ))
                {
                    bRet = true; bLeft = !bLeft;
                }

                if( !bRet )
                {
                    
                    TblChgMode eOld = eTblChgMode;
                    eTblChgMode = TBLFIX_CHGPROP;

                    bRet = SetColWidth( rAktBox, eType, nAbsDiff, nRelDiff,
                                        ppUndo );
                    eTblChgMode = eOld;
                    return bRet;
                }
            }
            else
            {
                bRet = true;
                for( n = 0; n < aLines.size(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, true ))
                    {
                        bRet = false;
                        break;
                    }
                }
            }

            if( bRet )
            {
                if( bInsDel )
                {
                    xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo,
                                                    aTmpLst, nDistStt));
                    if (aParam.bBigger &&
                        aParam.m_Boxes.size() == m_TabSortContentBoxes.size())
                    {
                        
                        GetFrmFmt()->GetDoc()->DeleteRowCol(aParam.m_Boxes);
                        return false;
                    }

                    if( ppUndo )
                        *ppUndo = aParam.CreateUndo(
                                        aParam.bBigger ? UNDO_COL_DELETE
                                                       : UNDO_TABLE_INSCOL );
                }
                else if( ppUndo )
                    *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, sal_True );

                long nFrmWidth = LONG_MAX;
                LockModify();
                SwFmtFrmSize aSz( rSz );
                SvxLRSpaceItem aLR( rLR );
                if( bBigger )
                {
                    
                    if( aSz.GetWidth() + nRelDiff > USHRT_MAX )
                    {
                        
                        CR_SetBoxWidth aTmpPara( 0, aSz.GetWidth() / 2,
                                        0, aSz.GetWidth(), aSz.GetWidth(), aParam.pTblNd );
                        for( sal_uInt16 nLn = 0; nLn < aLines.size(); ++nLn )
                            ::lcl_AjustLines( aLines[ nLn ], aTmpPara );
                        aSz.SetWidth( aSz.GetWidth() / 2 );
                        aParam.nDiff = nRelDiff /= 2;
                        aParam.nSide /= 2;
                        aParam.nMaxSize /= 2;
                    }

                    if( bLeft )
                        aLR.SetLeft( sal_uInt16( aLR.GetLeft() - nAbsDiff ) );
                    else
                        aLR.SetRight( sal_uInt16( aLR.GetRight() - nAbsDiff ) );
                }
                else if( bLeft )
                    aLR.SetLeft( sal_uInt16( aLR.GetLeft() + nAbsDiff ) );
                else
                    aLR.SetRight( sal_uInt16( aLR.GetRight() + nAbsDiff ) );

                if( bChgLRSpace )
                    GetFrmFmt()->SetFmtAttr( aLR );
                const SwFmtHoriOrient& rHOri = GetFrmFmt()->GetHoriOrient();
                if( text::HoriOrientation::FULL == rHOri.GetHoriOrient() ||
                    (text::HoriOrientation::LEFT == rHOri.GetHoriOrient() && aLR.GetLeft()) ||
                    (text::HoriOrientation::RIGHT == rHOri.GetHoriOrient() && aLR.GetRight()))
                {
                    SwFmtHoriOrient aHOri( rHOri );
                    aHOri.SetHoriOrient( text::HoriOrientation::NONE );
                    GetFrmFmt()->SetFmtAttr( aHOri );

                    
                    
                    
                    if( GetFrmFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
                        !rSz.GetWidthPercent() )
                    {
                        SwTabFrm* pTabFrm = SwIterator<SwTabFrm,SwFmt>::FirstElement( *GetFrmFmt() );
                        if( pTabFrm &&
                            pTabFrm->Prt().Width() != rSz.GetWidth() )
                        {
                            nFrmWidth = pTabFrm->Prt().Width();
                            if( bBigger )
                                nFrmWidth += nAbsDiff;
                            else
                                nFrmWidth -= nAbsDiff;
                        }
                    }
                }

                if( bBigger )
                    aSz.SetWidth( aSz.GetWidth() + nRelDiff );
                else
                    aSz.SetWidth( aSz.GetWidth() - nRelDiff );

                if( rSz.GetWidthPercent() )
                    aSz.SetWidthPercent( static_cast<sal_uInt8>(( aSz.GetWidth() * 100 ) /
                        ( aSz.GetWidth() + aLR.GetRight() + aLR.GetLeft())));

                GetFrmFmt()->SetFmtAttr( aSz );
                aParam.nTblWidth = sal_uInt16( aSz.GetWidth() );

                UnlockModify();

                for( n = aLines.size(); n; )
                {
                    --n;
                    aParam.LoopClear();
                    (*fnSelBox)( aLines[ n ], aParam, nDistStt, false );
                }

                
                
                
                if( LONG_MAX != nFrmWidth )
                {
                    SwFmtFrmSize aAbsSz( aSz );
                    aAbsSz.SetWidth( nFrmWidth );
                    GetFrmFmt()->SetFmtAttr( aAbsSz );
                }
            }
        }
        else if( bInsDel ||
                ( bLeft ? nDist != 0 : std::abs( rSz.GetWidth() - nDist ) > COLFUZZY ) )
        {
            bRet = true;
            if( bLeft && TBLFIX_CHGABS == eTblChgMode && !bInsDel )
                aParam.bBigger = !bBigger;

            
            if( bInsDel )
            {
                if( aParam.bBigger )
                {
                    for( n = 0; n < aLines.size(); ++n )
                    {
                        aParam.LoopClear();
                        if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, true ))
                        {
                            bRet = false;
                            break;
                        }
                    }
                }
                else
                {
                    if( ( bRet = bLeft ? nDist != 0
                                            : ( rSz.GetWidth() - nDist ) > COLFUZZY ) )
                    {
                        for( n = 0; n < aLines.size(); ++n )
                        {
                            aParam.LoopClear();
                            if( !(*fnOtherBox)( aLines[ n ], aParam, 0, true ))
                            {
                                bRet = false;
                                break;
                            }
                        }
                        if( bRet && !aParam.bAnyBoxFnd )
                            bRet = false;
                    }

                    if( !bRet && rAktBox.GetFrmFmt()->GetFrmSize().GetWidth()
                        - nRelDiff > COLFUZZY +
                            ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                    {
                        
                        aParam.bSplittBox = true;
                        
                        bRet = true;

                        for( n = 0; n < aLines.size(); ++n )
                        {
                            aParam.LoopClear();
                            if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, true ))
                            {
                                bRet = false;
                                break;
                            }
                        }
                    }
                }
            }
            else if( aParam.bBigger )
            {
                for( n = 0; n < aLines.size(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnOtherBox)( aLines[ n ], aParam, 0, true ))
                    {
                        bRet = false;
                        break;
                    }
                }
            }
            else
            {
                for( n = 0; n < aLines.size(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, true ))
                    {
                        bRet = false;
                        break;
                    }
                }
            }

            
            if( bRet )
            {
                CR_SetBoxWidth aParam1( aParam );
                if( bInsDel )
                {
                    aParam1.bBigger = !aParam.bBigger;
                    xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo,
                                                    aTmpLst, nDistStt));
                    if( ppUndo )
                        *ppUndo = aParam.CreateUndo(
                                        aParam.bBigger ? UNDO_TABLE_DELBOX
                                                       : UNDO_TABLE_INSCOL );
                }
                else if( ppUndo )
                    *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, sal_True );

                if( bInsDel
                    ? ( TBLFIX_CHGABS == eTblChgMode ? bLeft : bLeft )
                    : ( TBLFIX_CHGABS != eTblChgMode && bLeft ) )
                {
                    for( n = aLines.size(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnSelBox)( aLines[ n ], aParam, nDistStt, false );
                        (*fnOtherBox)( aLines[ n ], aParam1, nDistStt, false );
                    }
                }
                else
                    for( n = aLines.size(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnOtherBox)( aLines[ n ], aParam1, nDistStt, false );
                        (*fnSelBox)( aLines[ n ], aParam, nDistStt, false );
                    }
            }
        }
        break;

    case nsTblChgWidthHeightType::WH_CELL_RIGHT:
    case nsTblChgWidthHeightType::WH_CELL_LEFT:
        if( TBLVAR_CHGABS == eTblChgMode )
        {
            
            TblChgMode eOld = eTblChgMode;
            eTblChgMode = TBLFIX_CHGABS;

            bRet = SetColWidth( rAktBox, eType, nAbsDiff, nRelDiff,
                                ppUndo );
            eTblChgMode = eOld;
            return bRet;
        }
        else if( bInsDel || ( bLeft ? nDist != 0
                                    : (rSz.GetWidth() - nDist) > COLFUZZY ))
        {
            if( bLeft && TBLFIX_CHGABS == eTblChgMode && !bInsDel )
                aParam.bBigger = !bBigger;

            
            SwTableBox* pBox = &rAktBox;
            SwTableLine* pLine = rAktBox.GetUpper();
            while( pLine->GetUpper() )
            {
                sal_uInt16 nPos = pLine->GetTabBoxes().GetPos( pBox );
                if( bLeft ? nPos != 0 : nPos + 1 != (sal_uInt16)pLine->GetTabBoxes().size() )
                    break;

                pBox = pLine->GetUpper();
                pLine = pBox->GetUpper();
            }

            if( pLine->GetUpper() )
            {
                
                aParam.nSide -= ::lcl_GetDistance( pLine->GetUpper(), true );

                if( bLeft )
                    aParam.nMaxSize = aParam.nSide;
                else
                    aParam.nMaxSize = pLine->GetUpper()->GetFrmFmt()->
                                    GetFrmSize().GetWidth() - aParam.nSide;
            }

            
            if( bInsDel )
            {
                if( ( bRet = bLeft ? nDist != 0
                                : ( rSz.GetWidth() - nDist ) > COLFUZZY ) &&
                    !aParam.bBigger )
                {
                    bRet = (*fnOtherBox)( pLine, aParam, 0, true );
                    if( bRet && !aParam.bAnyBoxFnd )
                        bRet = false;
                }

                if( !bRet && !aParam.bBigger && rAktBox.GetFrmFmt()->
                    GetFrmSize().GetWidth() - nRelDiff > COLFUZZY +
                        ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                {
                    
                    aParam.bSplittBox = true;
                    bRet = true;
                }
            }
            else
            {
                FN_lcl_SetBoxWidth fnTmp = aParam.bBigger ? fnOtherBox : fnSelBox;
                bRet = (*fnTmp)( pLine, aParam, nDistStt, true );
            }

            
            if( bRet )
            {
                CR_SetBoxWidth aParam1( aParam );
                if( bInsDel )
                {
                    aParam1.bBigger = !aParam.bBigger;
                    xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst, nDistStt ));
                    if( ppUndo )
                        *ppUndo = aParam.CreateUndo(
                                        aParam.bBigger ? UNDO_TABLE_DELBOX
                                                       : UNDO_TABLE_INSCOL );
                }
                else if( ppUndo )
                    *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, sal_True );

                if( bInsDel
                    ? ( TBLFIX_CHGABS == eTblChgMode ? (bBigger && bLeft) : bLeft )
                    : ( TBLFIX_CHGABS != eTblChgMode && bLeft ) )
                {
                    (*fnSelBox)( pLine, aParam, nDistStt, false );
                    (*fnOtherBox)( pLine, aParam1, nDistStt, false );
                }
                else
                {
                    (*fnOtherBox)( pLine, aParam1, nDistStt, false );
                    (*fnSelBox)( pLine, aParam, nDistStt, false );
                }
            }
        }
        break;

    }

    if( xFndBox )
    {
        
        GCLines();

        
        if( !bBigger || xFndBox->AreLinesToRestore( *this ) )
            xFndBox->MakeFrms( *this );

        
        
        

        xFndBox.reset();

        if( ppUndo && *ppUndo )
        {
            aParam.pUndo->SetColWidthParam( nBoxIdx, static_cast<sal_uInt16>(eTblChgMode), eType,
                                            nAbsDiff, nRelDiff );
            if( !aParam.bBigger )
                aParam.pUndo->SaveNewBoxes( *aParam.pTblNd, aTmpLst );
        }
    }

    if( bRet )
    {
        CHECKBOXWIDTH
        CHECKTABLELAYOUT
    }

    return bRet;
}

static _FndBox* lcl_SaveInsDelData( CR_SetLineHeight& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst )
{
    
    SwTable& rTbl = rParam.pTblNd->GetTable();

    OSL_ENSURE( !rParam.m_Boxes.empty(), "We can't go on without Boxes!" );

    
    if (!rParam.bBigger
        && rParam.m_Boxes.size() == rTbl.GetTabSortBoxes().size())
    {
        return 0;
    }

    _FndBox* pFndBox = new _FndBox( 0, 0 );
    if( !rParam.bBigger )
        pFndBox->SetTableLines( rParam.m_Boxes, rTbl );
    else
    {
        _FndPara aPara(rParam.m_Boxes, pFndBox);
        ForEach_FndLineCopyCol( rTbl.GetTabLines(), &aPara );
        OSL_ENSURE( pFndBox->GetLines().size(), "Where are the Boxes?" );
        pFndBox->SetTableLines( rTbl );

        if( ppUndo )
            rTmpLst.insert( rTbl.GetTabSortBoxes() );
    }

    
    pFndBox->DelFrms( rTbl );

    

    return pFndBox;
}

void SetLineHeight( SwTableLine& rLine, SwTwips nOldHeight, SwTwips nNewHeight,
                    bool bMinSize )
{
    SwLayoutFrm* pLineFrm = GetRowFrm( rLine );
    OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine?" );

    SwFrmFmt* pFmt = rLine.ClaimFrmFmt();

    SwTwips nMyNewH, nMyOldH = pLineFrm->Frm().Height();
    if( !nOldHeight )                       
        nMyNewH = nMyOldH + nNewHeight;
    else
    {
        
        Fraction aTmp( nMyOldH );
        aTmp *= Fraction( nNewHeight, nOldHeight );
        aTmp += Fraction( 1, 2 );       
        nMyNewH = aTmp;
    }

    SwFrmSize eSize = ATT_MIN_SIZE;
    if( !bMinSize &&
        ( nMyOldH - nMyNewH ) > ( CalcRowRstHeight( pLineFrm ) + ROWFUZZY ))
        eSize = ATT_FIX_SIZE;

    pFmt->SetFmtAttr( SwFmtFrmSize( eSize, 0, nMyNewH ) );

    
    SwTableBoxes& rBoxes = rLine.GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.size(); ++n )
    {
        SwTableBox& rBox = *rBoxes[ n ];
        for( sal_uInt16 i = 0; i < rBox.GetTabLines().size(); ++i )
            SetLineHeight( *rBox.GetTabLines()[ i ], nMyOldH, nMyNewH, bMinSize );
    }
}

static bool lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                             SwTwips nDist, bool bCheck )
{
    bool bRet = true;
    if( !bCheck )
    {
        
        SetLineHeight( *pLine, 0, rParam.bBigger ? nDist : -nDist,
                        rParam.bBigger );
    }
    else if( !rParam.bBigger )
    {
        
        SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
        OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine?" );
        SwTwips nRstHeight = CalcRowRstHeight( pLineFrm );
        if( (nRstHeight + ROWFUZZY) < nDist )
            bRet = false;
    }
    return bRet;
}

static bool lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, bool bCheck )
{
    bool bRet = true;
    if( bCheck )
    {
        if( rParam.bBigger )
        {
            
            SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
            OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine?" );

            if( TBLFIX_CHGPROP == rParam.nMode )
            {
                nDist *= pLineFrm->Frm().Height();
                nDist /= rParam.nMaxHeight;
            }
            bRet = nDist <= CalcRowRstHeight( pLineFrm );
        }
    }
    else
    {
        
        
        if( TBLFIX_CHGPROP == rParam.nMode )
        {
            SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
            OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine??" );

            
            
            
            if( true /*!rParam.bBigger*/ )
            {
                nDist *= pLineFrm->Frm().Height();
                nDist /= rParam.nMaxHeight;
            }
            else
            {
                
                nDist *= CalcRowRstHeight( pLineFrm );
                nDist /= rParam.nMaxSpace;
            }
        }
        SetLineHeight( *pLine, 0, rParam.bBigger ? -nDist : nDist,
                        !rParam.bBigger );
    }
    return bRet;
}

static bool lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                            SwTwips nDist, bool bCheck )
{
    bool bRet = true;
    if( !bCheck )
    {
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwDoc* pDoc = pLine->GetFrmFmt()->GetDoc();
        if( !rParam.bBigger )
        {
            for (size_t n = rBoxes.size(); n; )
            {
                ::lcl_SaveUpperLowerBorder( rParam.pTblNd->GetTable(),
                                                    *rBoxes[ --n ],
                                                    rParam.aShareFmts );
            }
            for (size_t n = rBoxes.size(); n; )
            {
                ::_DeleteBox( rParam.pTblNd->GetTable(),
                                    rBoxes[ --n ], rParam.pUndo, false,
                                    false, &rParam.aShareFmts );
            }
        }
        else
        {
            
            SwTableLine* pNewLine = new SwTableLine( (SwTableLineFmt*)pLine->GetFrmFmt(),
                                        rBoxes.size(), pLine->GetUpper() );
            SwTableLines* pLines;
            if( pLine->GetUpper() )
                pLines = &pLine->GetUpper()->GetTabLines();
            else
                pLines = &rParam.pTblNd->GetTable().GetTabLines();
            sal_uInt16 nPos = pLines->GetPos( pLine );
            if( !rParam.bTop )
                ++nPos;
            pLines->insert( pLines->begin() + nPos, pNewLine );

            SwFrmFmt* pNewFmt = pNewLine->ClaimFrmFmt();
            pNewFmt->SetFmtAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nDist ) );

            
            SwTableBoxes& rNewBoxes = pNewLine->GetTabBoxes();
            for( sal_uInt16 n = 0; n < rBoxes.size(); ++n )
            {
                SwTwips nWidth = 0;
                SwTableBox* pOld = rBoxes[ n ];
                if( !pOld->GetSttNd() )
                {
                    
                    nWidth = pOld->GetFrmFmt()->GetFrmSize().GetWidth();
                    while( !pOld->GetSttNd() )
                        pOld = pOld->GetTabLines()[ 0 ]->GetTabBoxes()[ 0 ];
                }
                ::_InsTblBox( pDoc, rParam.pTblNd, pNewLine,
                                    (SwTableBoxFmt*)pOld->GetFrmFmt(), pOld, n );

                
                
                const SvxBoxItem& rBoxItem = pOld->GetFrmFmt()->GetBox();
                if( rBoxItem.GetTop() )
                {
                    SvxBoxItem aTmp( rBoxItem );
                    aTmp.SetLine( 0, BOX_LINE_TOP );
                    rParam.aShareFmts.SetAttr( rParam.bTop
                                                ? *pOld
                                                : *rNewBoxes[ n ], aTmp );
                }

                if( nWidth )
                    rParam.aShareFmts.SetAttr( *rNewBoxes[ n ],
                                SwFmtFrmSize( ATT_FIX_SIZE, nWidth, 0 ) );
            }
        }
    }
    else
    {
        
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( sal_uInt16 n = rBoxes.size(); n; )
        {
            SwTableBox* pBox = rBoxes[ --n ];
            if( pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                return false;

            if( pBox->GetSttNd() )
            {
                rParam.m_Boxes.insert(pBox);
            }
            else
            {
                for( sal_uInt16 i = pBox->GetTabLines().size(); i; )
                    lcl_InsDelSelLine( pBox->GetTabLines()[ --i ],
                                        rParam, 0, true );
            }
        }
    }
    return bRet;
}

bool SwTable::SetRowHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff,SwUndo** ppUndo )
{
    SwTableLine* pLine = rAktBox.GetUpper();

    SwTableLine* pBaseLine = pLine;
    while( pBaseLine->GetUpper() )
        pBaseLine = pBaseLine->GetUpper()->GetUpper();

    boost::scoped_ptr<_FndBox> xFndBox;                
    SwTableSortBoxes aTmpLst;       
    bool bBigger,
        bRet = false,
        bTop = nsTblChgWidthHeightType::WH_ROW_TOP == ( eType & 0xff ) ||
                nsTblChgWidthHeightType::WH_CELL_TOP == ( eType & 0xff ),
        bInsDel = 0 != (eType & nsTblChgWidthHeightType::WH_FLAG_INSDEL );
    sal_uInt16 n, nBaseLinePos = GetTabLines().GetPos( pBaseLine );
    sal_uLong nBoxIdx = rAktBox.GetSttIdx();

    CR_SetLineHeight aParam( eType,
                        (SwTableNode*)rAktBox.GetSttNd()->FindTableNode() );
    bBigger = aParam.bBigger;

    FN_lcl_SetLineHeight fnSelLine, fnOtherLine = lcl_SetOtherLineHeight;
    if( bInsDel )
        fnSelLine = lcl_InsDelSelLine;
    else
        fnSelLine = lcl_SetSelLineHeight;

    SwTableLines* pLines = &aLines;

    
    switch( eType & 0xff )
    {
    case nsTblChgWidthHeightType::WH_CELL_TOP:
    case nsTblChgWidthHeightType::WH_CELL_BOTTOM:
        if( pLine == pBaseLine )
            break;  

        
        pLines = &pLine->GetUpper()->GetTabLines();
        nBaseLinePos = pLines->GetPos( pLine );
        pBaseLine = pLine;
        

    case nsTblChgWidthHeightType::WH_ROW_TOP:
    case nsTblChgWidthHeightType::WH_ROW_BOTTOM:
        {
            if( bInsDel && !bBigger )       
            {
                nAbsDiff = GetRowFrm( *pBaseLine )->Frm().Height();
            }

            if( TBLVAR_CHGABS == eTblChgMode )
            {
                
                if( bBigger )
                {
                    bRet = true;

                    if( !bRet )
                    {
                        
                        TblChgMode eOld = eTblChgMode;
                        eTblChgMode = TBLFIX_CHGPROP;

                        bRet = SetRowHeight( rAktBox, eType, nAbsDiff,
                                            nRelDiff, ppUndo );

                        eTblChgMode = eOld;
                        return bRet;
                    }
                }
                else
                    bRet = (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, true );

                if( bRet )
                {
                    if( bInsDel )
                    {
                        if (aParam.m_Boxes.empty())
                        {
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, true );
                        }

                        xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst ));

                        
                        if( !bBigger &&
                            aParam.m_Boxes.size() == m_TabSortContentBoxes.size())
                        {
                            GetFrmFmt()->GetDoc()->DeleteRowCol(aParam.m_Boxes);
                            return false;
                        }


                        if( ppUndo )
                            *ppUndo = aParam.CreateUndo(
                                        bBigger ? UNDO_TABLE_INSROW
                                                : UNDO_ROW_DELETE );
                    }
                    else if( ppUndo )
                        *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, sal_True );

                    (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                    nAbsDiff, false );
                }
            }
            else
            {
                bRet = true;
                sal_uInt16 nStt, nEnd;
                if( bTop )
                    nStt = 0, nEnd = nBaseLinePos;
                else
                    nStt = nBaseLinePos + 1, nEnd = pLines->size();

                
                if( TBLFIX_CHGPROP == eTblChgMode )
                {
                    for( n = nStt; n < nEnd; ++n )
                    {
                        SwLayoutFrm* pLineFrm = GetRowFrm( *(*pLines)[ n ] );
                        OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine??" );
                        aParam.nMaxSpace += CalcRowRstHeight( pLineFrm );
                        aParam.nMaxHeight += pLineFrm->Frm().Height();
                    }
                    if( bBigger && aParam.nMaxSpace < nAbsDiff )
                        bRet = false;
                }
                else
                {
                    if( bTop ? nEnd != 0 : nStt < nEnd  )
                    {
                        if( bTop )
                            nStt = nEnd - 1;
                        else
                            nEnd = nStt + 1;
                    }
                    else
                        bRet = false;
                }

                if( bRet )
                {
                    if( bBigger )
                    {
                        for( n = nStt; n < nEnd; ++n )
                        {
                            if( !(*fnOtherLine)( (*pLines)[ n ], aParam,
                                                    nAbsDiff, true ))
                            {
                                bRet = false;
                                break;
                            }
                        }
                    }
                    else
                        bRet = (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                                nAbsDiff, true );
                }

                if( bRet )
                {
                    
                    if( bInsDel )
                    {
                        if (aParam.m_Boxes.empty())
                        {
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, true );
                        }
                        xFndBox.reset(::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst ));
                        if( ppUndo )
                            *ppUndo = aParam.CreateUndo(
                                        bBigger ? UNDO_TABLE_INSROW
                                                : UNDO_ROW_DELETE );
                    }
                    else if( ppUndo )
                        *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, sal_True );

                    CR_SetLineHeight aParam1( aParam );
                    if( TBLFIX_CHGPROP == eTblChgMode && !bBigger &&
                        !aParam.nMaxSpace )
                    {
                        
                        
                        aParam1.nLines = nEnd - nStt;
                    }

                    if( bTop )
                    {
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, false );
                        for( n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, false );
                    }
                    else
                    {
                        for( n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, false );
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, false );
                    }
                }
                else
                {
                    
                    TblChgMode eOld = eTblChgMode;
                    eTblChgMode = TBLVAR_CHGABS;

                    bRet = SetRowHeight( rAktBox, eType, nAbsDiff,
                                        nRelDiff, ppUndo );

                    eTblChgMode = eOld;
                    xFndBox.reset();
                }
            }
        }
        break;
    }

    if( xFndBox )
    {
        
        GCLines();

        
        if( bBigger || xFndBox->AreLinesToRestore( *this ) )
            xFndBox->MakeFrms( *this );

        

        xFndBox.reset();

        if( ppUndo && *ppUndo )
        {
            aParam.pUndo->SetColWidthParam( nBoxIdx, static_cast<sal_uInt16>(eTblChgMode), eType,
                                            nAbsDiff, nRelDiff );
            if( bBigger )
                aParam.pUndo->SaveNewBoxes( *aParam.pTblNd, aTmpLst );
        }
    }

    CHECKTABLELAYOUT

    return bRet;
}

SwFrmFmt* SwShareBoxFmt::GetFormat( long nWidth ) const
{
    SwFrmFmt *pRet = 0, *pTmp;
    for( sal_uInt16 n = aNewFmts.size(); n; )
        if( ( pTmp = aNewFmts[ --n ])->GetFrmSize().GetWidth()
                == nWidth )
        {
            pRet = pTmp;
            break;
        }
    return pRet;
}

SwFrmFmt* SwShareBoxFmt::GetFormat( const SfxPoolItem& rItem ) const
{
    const SfxPoolItem* pItem;
    sal_uInt16 nWhich = rItem.Which();
    SwFrmFmt *pRet = 0, *pTmp;
    const SfxPoolItem& rFrmSz = pOldFmt->GetFmtAttr( RES_FRM_SIZE, sal_False );
    for( sal_uInt16 n = aNewFmts.size(); n; )
        if( SFX_ITEM_SET == ( pTmp = aNewFmts[ --n ])->
            GetItemState( nWhich, sal_False, &pItem ) && *pItem == rItem &&
            pTmp->GetFmtAttr( RES_FRM_SIZE, sal_False ) == rFrmSz )
        {
            pRet = pTmp;
            break;
        }
    return pRet;
}

void SwShareBoxFmt::AddFormat( SwFrmFmt& rNew )
{
    aNewFmts.push_back( &rNew );
}

bool SwShareBoxFmt::RemoveFormat( const SwFrmFmt& rFmt )
{
    
    if( pOldFmt == &rFmt )
        return true;

    std::vector<SwFrmFmt*>::iterator it = std::find( aNewFmts.begin(), aNewFmts.end(), &rFmt );
    if( aNewFmts.end() != it )
        aNewFmts.erase( it );
    return aNewFmts.empty();
}

SwShareBoxFmts::~SwShareBoxFmts()
{
}

SwFrmFmt* SwShareBoxFmts::GetFormat( const SwFrmFmt& rFmt, long nWidth ) const
{
    sal_uInt16 nPos;
    return Seek_Entry( rFmt, &nPos )
                    ? aShareArr[ nPos ].GetFormat( nWidth )
                    : 0;
}
SwFrmFmt* SwShareBoxFmts::GetFormat( const SwFrmFmt& rFmt,
                                     const SfxPoolItem& rItem ) const
{
    sal_uInt16 nPos;
    return Seek_Entry( rFmt, &nPos )
                    ? aShareArr[ nPos ].GetFormat( rItem )
                    : 0;
}

void SwShareBoxFmts::AddFormat( const SwFrmFmt& rOld, SwFrmFmt& rNew )
{
    {
        sal_uInt16 nPos;
        SwShareBoxFmt* pEntry;
        if( !Seek_Entry( rOld, &nPos ))
        {
            pEntry = new SwShareBoxFmt( rOld );
            aShareArr.insert( aShareArr.begin() + nPos, pEntry );
        }
        else
            pEntry = &aShareArr[ nPos ];

        pEntry->AddFormat( rNew );
    }
}

void SwShareBoxFmts::ChangeFrmFmt( SwTableBox* pBox, SwTableLine* pLn,
                                    SwFrmFmt& rFmt )
{
    SwClient aCl;
    SwFrmFmt* pOld = 0;
    if( pBox )
    {
        pOld = pBox->GetFrmFmt();
        pOld->Add( &aCl );
        pBox->ChgFrmFmt( (SwTableBoxFmt*)&rFmt );
    }
    else if( pLn )
    {
        pOld = pLn->GetFrmFmt();
        pOld->Add( &aCl );
        pLn->ChgFrmFmt( (SwTableLineFmt*)&rFmt );
    }
    if( pOld && pOld->IsLastDepend() )
    {
        RemoveFormat( *pOld );
        delete pOld;
    }
}

void SwShareBoxFmts::SetSize( SwTableBox& rBox, const SwFmtFrmSize& rSz )
{
    SwFrmFmt *pBoxFmt = rBox.GetFrmFmt(),
             *pRet = GetFormat( *pBoxFmt, rSz.GetWidth() );
    if( pRet )
        ChangeFrmFmt( &rBox, 0, *pRet );
    else
    {
        pRet = rBox.ClaimFrmFmt();
        pRet->SetFmtAttr( rSz );
        AddFormat( *pBoxFmt, *pRet );
    }
}

void SwShareBoxFmts::SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem )
{
    SwFrmFmt *pBoxFmt = rBox.GetFrmFmt(),
             *pRet = GetFormat( *pBoxFmt, rItem );
    if( pRet )
        ChangeFrmFmt( &rBox, 0, *pRet );
    else
    {
        pRet = rBox.ClaimFrmFmt();
        pRet->SetFmtAttr( rItem );
        AddFormat( *pBoxFmt, *pRet );
    }
}

void SwShareBoxFmts::SetAttr( SwTableLine& rLine, const SfxPoolItem& rItem )
{
    SwFrmFmt *pLineFmt = rLine.GetFrmFmt(),
             *pRet = GetFormat( *pLineFmt, rItem );
    if( pRet )
        ChangeFrmFmt( 0, &rLine, *pRet );
    else
    {
        pRet = rLine.ClaimFrmFmt();
        pRet->SetFmtAttr( rItem );
        AddFormat( *pLineFmt, *pRet );
    }
}

void SwShareBoxFmts::RemoveFormat( const SwFrmFmt& rFmt )
{
    for( sal_uInt16 i = aShareArr.size(); i; )
        if( aShareArr[ --i ].RemoveFormat( rFmt ))
            aShareArr.erase( aShareArr.begin() + i );
}

bool SwShareBoxFmts::Seek_Entry( const SwFrmFmt& rFmt, sal_uInt16* pPos ) const
{
    sal_uLong nIdx = (sal_uLong)&rFmt;
    sal_uInt16 nO = aShareArr.size(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            sal_uLong nFmt = (sal_uLong)&aShareArr[ nM ].GetOldFormat();
            if( nFmt == nIdx )
            {
                if( pPos )
                    *pPos = nM;
                return true;
            }
            else if( nFmt < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pPos )
                    *pPos = nU;
                return false;
            }
            else
                nO = nM - 1;
        }
    }
    if( pPos )
        *pPos = nU;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
