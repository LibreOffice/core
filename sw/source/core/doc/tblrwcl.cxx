/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <hintids.hxx>

#include <editeng/brshitem.hxx>
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

typedef SwTableLine* SwTableLinePtr;
SV_DECL_PTRARR_SORT( SwSortTableLines, SwTableLinePtr, 16 )
SV_IMPL_PTRARR_SORT( SwSortTableLines, SwTableLinePtr );

SV_IMPL_PTRARR( _SwShareBoxFmts, SwShareBoxFmt* )

// In order to set the Frame Formats for the Boxes, it's enough to look
// up the current one in the array. If it's already there return the new one.
struct _CpyTabFrm
{
    union {
        SwTableBoxFmt *pFrmFmt;     // for CopyCol
        SwTwips nSize;              // for DelCol
    } Value;
    SwTableBoxFmt *pNewFrmFmt;

    _CpyTabFrm( SwTableBoxFmt* pAktFrmFmt ) : pNewFrmFmt( 0 )
    {   Value.pFrmFmt = pAktFrmFmt; }

    _CpyTabFrm& operator=( const _CpyTabFrm& );

    sal_Bool operator==( const _CpyTabFrm& rCpyTabFrm ) const
        { return  (sal_uLong)Value.nSize == (sal_uLong)rCpyTabFrm.Value.nSize; }
    sal_Bool operator<( const _CpyTabFrm& rCpyTabFrm ) const
        { return  (sal_uLong)Value.nSize < (sal_uLong)rCpyTabFrm.Value.nSize; }
};

struct CR_SetBoxWidth
{
    SwSelBoxes aBoxes;
    SwSortTableLines aLines;
    std::vector<sal_uInt16> aLinesWidth;
    SwShareBoxFmts aShareFmts;
    SwTableNode* pTblNd;
    SwUndoTblNdsChg* pUndo;
    SwTwips nDiff, nSide, nMaxSize, nLowerDiff;
    TblChgMode nMode;
    sal_uInt16 nTblWidth, nRemainWidth, nBoxWidth;
    sal_Bool bBigger, bLeft, bSplittBox, bAnyBoxFnd;

    CR_SetBoxWidth( sal_uInt16 eType, SwTwips nDif, SwTwips nSid, SwTwips nTblW,
                    SwTwips nMax, SwTableNode* pTNd )
        : pTblNd( pTNd ),
        nDiff( nDif ), nSide( nSid ), nMaxSize( nMax ), nLowerDiff( 0 ),
        nTblWidth( (sal_uInt16)nTblW ), nRemainWidth( 0 ), nBoxWidth( 0 ),
        bSplittBox( sal_False ), bAnyBoxFnd( sal_False )
    {
        bLeft = nsTblChgWidthHeightType::WH_COL_LEFT == ( eType & 0xff ) ||
                nsTblChgWidthHeightType::WH_CELL_LEFT == ( eType & 0xff );
        bBigger = 0 != (eType & nsTblChgWidthHeightType::WH_FLAG_BIGGER );
        nMode = pTblNd->GetTable().GetTblChgMode();
    }
    CR_SetBoxWidth( const CR_SetBoxWidth& rCpy )
        : pTblNd( rCpy.pTblNd ),
        pUndo( rCpy.pUndo ),
        nDiff( rCpy.nDiff ), nSide( rCpy.nSide ),
        nMaxSize( rCpy.nMaxSize ), nLowerDiff( 0 ),
        nMode( rCpy.nMode ), nTblWidth( rCpy.nTblWidth ),
        nRemainWidth( rCpy.nRemainWidth ), nBoxWidth( rCpy.nBoxWidth ),
        bBigger( rCpy.bBigger ), bLeft( rCpy.bLeft ),
        bSplittBox( rCpy.bSplittBox ), bAnyBoxFnd( rCpy.bAnyBoxFnd )
    {
        aLines.Insert( &rCpy.aLines );
        aLinesWidth = rCpy.aLinesWidth;
    }

    SwUndoTblNdsChg* CreateUndo( SwUndoId eUndoType )
    {
        return pUndo = new SwUndoTblNdsChg( eUndoType, aBoxes, *pTblNd );
    }

    void LoopClear()
    {
        nLowerDiff = 0; nRemainWidth = 0;
    }

    void AddBoxWidth( const SwTableBox& rBox, sal_uInt16 nWidth )
    {
        SwTableLinePtr p = (SwTableLine*)rBox.GetUpper();
        sal_uInt16 nFndPos;
        if( aLines.Insert( p, nFndPos ))
            aLinesWidth.insert( aLinesWidth.begin()+nFndPos, nWidth );
        else
            aLinesWidth[ nFndPos ] = aLinesWidth[ nFndPos ] + nWidth;
    }

    sal_uInt16 GetBoxWidth( const SwTableLine& rLn ) const
    {
        SwTableLinePtr p = (SwTableLine*)&rLn;
        sal_uInt16 nFndPos;
        if( aLines.Seek_Entry( p, &nFndPos ) )
            nFndPos = aLinesWidth[ nFndPos ];
        else
            nFndPos = 0;
        return nFndPos;
    }
};

sal_Bool lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, sal_Bool bCheck );
sal_Bool lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, sal_Bool bCheck );
sal_Bool lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, sal_Bool bCheck );
sal_Bool lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, sal_Bool bCheck );
sal_Bool lcl_DelSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, sal_Bool bCheck );
sal_Bool lcl_DelOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, sal_Bool bCheck );

typedef sal_Bool (*FN_lcl_SetBoxWidth)(SwTableLine*, CR_SetBoxWidth&, SwTwips, sal_Bool );

#ifdef DBG_UTIL

void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize );

#define CHECKBOXWIDTH                                           \
    {                                                           \
        SwTwips nSize = GetFrmFmt()->GetFrmSize().GetWidth();   \
        for( sal_uInt16 nTmp = 0; nTmp < aLines.Count(); ++nTmp )   \
            ::_CheckBoxWidth( *aLines[ nTmp ], nSize );         \
    }

#define CHECKTABLELAYOUT                                            \
    {                                                               \
        for ( sal_uInt16 i = 0; i < GetTabLines().Count(); ++i )        \
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

#endif // DBG_UTIL

struct CR_SetLineHeight
{
    SwSelBoxes aBoxes;
    SwShareBoxFmts aShareFmts;
    SwTableNode* pTblNd;
    SwUndoTblNdsChg* pUndo;
    SwTwips nMaxSpace, nMaxHeight;
    TblChgMode nMode;
    sal_uInt16 nLines;
    sal_Bool bBigger, bTop, bSplittBox, bAnyBoxFnd;

    CR_SetLineHeight( sal_uInt16 eType, SwTableNode* pTNd )
        : pTblNd( pTNd ), pUndo( 0 ),
        nMaxSpace( 0 ), nMaxHeight( 0 ), nLines( 0 ),
        bSplittBox( sal_False ), bAnyBoxFnd( sal_False )
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
        return pUndo = new SwUndoTblNdsChg( nUndoType, aBoxes, *pTblNd );
    }
};

sal_Bool lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                         SwTwips nDist, sal_Bool bCheck );
sal_Bool lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, sal_Bool bCheck );
sal_Bool lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, sal_Bool bCheck );

typedef sal_Bool (*FN_lcl_SetLineHeight)(SwTableLine*, CR_SetLineHeight&, SwTwips, sal_Bool );

_CpyTabFrm& _CpyTabFrm::operator=( const _CpyTabFrm& rCpyTabFrm )
{
    pNewFrmFmt = rCpyTabFrm.pNewFrmFmt;
    Value = rCpyTabFrm.Value;
    return *this;
}

SV_DECL_VARARR_SORT( _CpyTabFrms, _CpyTabFrm, 0 )
SV_IMPL_VARARR_SORT( _CpyTabFrms, _CpyTabFrm )

void lcl_DelCpyTabFrmFmts( _CpyTabFrm& rArr );

struct _CpyPara
{
    boost::shared_ptr< std::vector< std::vector< sal_uLong > > > pWidths;
    SwDoc* pDoc;
    SwTableNode* pTblNd;
    _CpyTabFrms& rTabFrmArr;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    sal_uLong nOldSize, nNewSize;           // in order to correct the size attributes
    sal_uLong nMinLeft, nMaxRight;
    sal_uInt16 nCpyCnt, nInsPos;
    sal_uInt16 nLnIdx, nBoxIdx;
    sal_uInt8 nDelBorderFlag;
    sal_Bool bCpyCntnt;

    _CpyPara( SwTableNode* pNd, sal_uInt16 nCopies, _CpyTabFrms& rFrmArr,
              sal_Bool bCopyContent = sal_True )
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

sal_Bool lcl_CopyCol( _FndBox* pFndBox, _CpyPara* pCpyPara )
{
    // Look up the Frame Format in the Frame Format Array
    SwTableBox* pBox = (SwTableBox*)pFndBox->GetBox();
    _CpyTabFrm aFindFrm( (SwTableBoxFmt*)pBox->GetFrmFmt() );

    sal_uInt16 nFndPos;
    if( pCpyPara->nCpyCnt )
    {
        if( !pCpyPara->rTabFrmArr.Seek_Entry( aFindFrm, &nFndPos ))
        {
            // For nested copying, also save the new Format as an old one.
            SwTableBoxFmt* pNewFmt = (SwTableBoxFmt*)pBox->ClaimFrmFmt();

            // Find the selected Boxes in the Line:
            _FndLine* pCmpLine = NULL;
            SwFmtFrmSize aFrmSz( pNewFmt->GetFrmSize() );

            bool bDiffCount = false;
            if( pBox->GetTabLines().Count() )
            {
                pCmpLine = pFndBox->GetLines()[ 0 ];
                if ( pCmpLine->GetBoxes().size() != pCmpLine->GetLine()->GetTabBoxes().Count() )
                    bDiffCount = true;
            }

            if( bDiffCount )
            {
                // The first Line should be enough
                _FndBoxes& rFndBoxes = pCmpLine->GetBoxes();
                long nSz = 0;
                for( sal_uInt16 n = rFndBoxes.size(); n; )
                    nSz += rFndBoxes[ --n ]->GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
                aFrmSz.SetWidth( aFrmSz.GetWidth() -
                                            nSz / ( pCpyPara->nCpyCnt + 1 ) );
                pNewFmt->SetFmtAttr( aFrmSz );
                aFrmSz.SetWidth( nSz / ( pCpyPara->nCpyCnt + 1 ) );

                // Create a new Format for the new Box, specifying it's size.
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
                pCpyPara->rTabFrmArr.Insert( aFindFrm );
                aFindFrm.Value.pFrmFmt = pNewFmt;
                pCpyPara->rTabFrmArr.Insert( aFindFrm );
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
        if( pCpyPara->nDelBorderFlag &&
            pCpyPara->rTabFrmArr.Seek_Entry( aFindFrm, &nFndPos ))
            aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ];
        else
            aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
    }

    if( pFndBox->GetLines().Count() )
    {
        pBox = new SwTableBox( aFindFrm.pNewFrmFmt,
                    pFndBox->GetLines().Count(), pCpyPara->pInsLine );
        pCpyPara->pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, pCpyPara->nInsPos++);
        _CpyPara aPara( *pCpyPara, pBox );
        aPara.nDelBorderFlag &= 7;

        pFndBox->GetLines().ForEach( &lcl_CopyRow, &aPara );
    }
    else
    {
        ::_InsTblBox( pCpyPara->pDoc, pCpyPara->pTblNd, pCpyPara->pInsLine,
                    aFindFrm.pNewFrmFmt, pBox, pCpyPara->nInsPos++ );

        const _FndBoxes& rFndBxs = pFndBox->GetUpper()->GetBoxes();
        if( 8 > pCpyPara->nDelBorderFlag
                ? pCpyPara->nDelBorderFlag
                : pFndBox == rFndBxs[ rFndBxs.size() - 1 ] )
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
                    // For all Boxes that delete TopBorderLine, we copy after that
                    pBox = pCpyPara->pInsLine->GetTabBoxes()[
                                            pCpyPara->nInsPos - 1 ];
                }

                aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();

                // Else we copy before that and the first Line keeps the TopLine
                // and we remove it at the original
                pBox->ClaimFrmFmt()->SetFmtAttr( aNew );

                if( !pCpyPara->nCpyCnt )
                    pCpyPara->rTabFrmArr.Insert( aFindFrm );
            }
        }
    }
    return sal_True;
}

sal_Bool lcl_CopyRow( const _FndLine*& rpFndLine, void* pPara )
{
    _CpyPara* pCpyPara = (_CpyPara*)pPara;
    SwTableLine* pNewLine = new SwTableLine(
                            (SwTableLineFmt*)rpFndLine->GetLine()->GetFrmFmt(),
                        rpFndLine->GetBoxes().size(), pCpyPara->pInsBox );
    if( pCpyPara->pInsBox )
    {
        pCpyPara->pInsBox->GetTabLines().C40_INSERT( SwTableLine, pNewLine, pCpyPara->nInsPos++ );
    }
    else
    {
        pCpyPara->pTblNd->GetTable().GetTabLines().C40_INSERT( SwTableLine, pNewLine,
                                                pCpyPara->nInsPos++ );
    }

    _CpyPara aPara( *pCpyPara, pNewLine );
    for (_FndBoxes::const_iterator it = ((_FndLine*)rpFndLine)->GetBoxes().begin();
         it != ((_FndLine*)rpFndLine)->GetBoxes().end(); ++it)
        lcl_CopyCol(*it, &aPara);

    pCpyPara->nDelBorderFlag &= 0xf8;
    return sal_True;
}

void lcl_InsCol( _FndLine* pFndLn, _CpyPara& rCpyPara, sal_uInt16 nCpyCnt,
                sal_Bool bBehind )
{
    // Bug 29124: Not only copy in the BaseLines. If possible, we go down as far as possible
    _FndBox* pFBox;
    if( 1 == pFndLn->GetBoxes().size() &&
        !( pFBox = pFndLn->GetBoxes()[ 0 ] )->GetBox()->GetSttNd() )
    {
        // A Box with multiple Lines, so insert into these Lines
        for( sal_uInt16 n = 0; n < pFBox->GetLines().Count(); ++n )
            lcl_InsCol( pFBox->GetLines()[ n ], rCpyPara, nCpyCnt, bBehind );
    }
    else
    {
        rCpyPara.pInsLine = pFndLn->GetLine();
        SwTableBox* pBox = pFndLn->GetBoxes()[ bBehind ?
                    pFndLn->GetBoxes().size()-1 : 0 ]->GetBox();
        rCpyPara.nInsPos = pFndLn->GetLine()->GetTabBoxes().C40_GETPOS( SwTableBox, pBox );
        if( bBehind )
            ++rCpyPara.nInsPos;

        for( sal_uInt16 n = 0; n < nCpyCnt; ++n )
        {
            if( n + 1 == nCpyCnt && bBehind )
                rCpyPara.nDelBorderFlag = 9;
            else
                rCpyPara.nDelBorderFlag = 8;
            for (_FndBoxes::const_iterator it = pFndLn->GetBoxes().begin();
                 it != pFndLn->GetBoxes().end(); ++it)
                lcl_CopyCol(*it, &rCpyPara);
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

sal_Bool SwTable::InsertCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt, sal_Bool bBehind )
{
    OSL_ENSURE( !rBoxes.empty() && nCnt, "No valid Box List" );
    SwTableNode* pTblNd = const_cast<SwTableNode*>( rBoxes.begin()->second->GetSttNd()->FindTableNode() );
    if( !pTblNd )
        return sal_False;

    sal_Bool bRes = sal_True;
    if( IsNewModel() )
        bRes = NewInsertCol( pDoc, rBoxes, nCnt, bBehind );
    else
    {
        // Find all Boxes/Lines
        _FndBox aFndBox( 0, 0 );
        {
            _FndPara aPara( rBoxes, &aFndBox );
            GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
        }
        if( !aFndBox.GetLines().Count() )
            return sal_False;

        SetHTMLTableLayout( 0 );    // Delete HTML Layout

        // Find Lines for the layout update
        aFndBox.SetTableLines( *this );
        aFndBox.DelFrms( *this );

        // TL_CHART2: nothing to be done since chart2 currently does not want to
        // get notified about new rows/cols.

        _CpyTabFrms aTabFrmArr;
        _CpyPara aCpyPara( pTblNd, nCnt, aTabFrmArr );

        for( sal_uInt16 n = 0; n < aFndBox.GetLines().Count(); ++n )
            lcl_InsCol( aFndBox.GetLines()[ n ], aCpyPara, nCnt, bBehind );

        // clean up this Line's structure once again, generally all of them
        GCLines();

        // Update Layout
        aFndBox.MakeFrms( *this );

        CHECKBOXWIDTH;
        CHECKTABLELAYOUT;
        bRes = sal_True;
    }

    SwChartDataProvider *pPCD = pDoc->GetChartDataProvider();
    if (pPCD && nCnt)
        pPCD->AddRowCols( *this, rBoxes, nCnt, bBehind );
    pDoc->UpdateCharts( GetFrmFmt()->GetName() );

    return bRes;
}

sal_Bool SwTable::_InsertRow( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        sal_uInt16 nCnt, sal_Bool bBehind )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid Box List" );
    SwTableNode* pTblNd = const_cast<SwTableNode*>( rBoxes.begin()->second->GetSttNd()->FindTableNode() );
    if( !pTblNd )
        return sal_False;

     // Find all Boxes/Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return sal_False;

    SetHTMLTableLayout( 0 );   // Delete HTML Layout

    _FndBox* pFndBox = &aFndBox;
    {
        _FndLine* pFndLine;
        while( 1 == pFndBox->GetLines().Count() &&
                1 == ( pFndLine = pFndBox->GetLines()[ 0 ])->GetBoxes().size() )
        {
            // Don't go down too far! One Line with Box needs to remain!
            _FndBox* pTmpBox = pFndLine->GetBoxes()[ 0 ];
            if( pTmpBox->GetLines().Count() )
                pFndBox = pTmpBox;
            else
                break;
        }
    }

    // Find Lines for the layout update
    const sal_Bool bLayout = !IsNewModel() &&
        0 != SwIterator<SwTabFrm,SwFmt>::FirstElement( *GetFrmFmt() );

    if ( bLayout )
    {
        aFndBox.SetTableLines( *this );
        if( pFndBox != &aFndBox )
            aFndBox.DelFrms( *this );
        // TL_CHART2: nothing to be done since chart2 currently does not want to
        // get notified about new rows/cols.
    }

    _CpyTabFrms aTabFrmArr;
    _CpyPara aCpyPara( pTblNd, 0, aTabFrmArr );

    SwTableLine* pLine = pFndBox->GetLines()[ bBehind ?
                    pFndBox->GetLines().Count()-1 : 0 ]->GetLine();
    if( &aFndBox == pFndBox )
        aCpyPara.nInsPos = GetTabLines().C40_GETPOS( SwTableLine, pLine );
    else
    {
        aCpyPara.pInsBox = pFndBox->GetBox();
        aCpyPara.nInsPos = pFndBox->GetBox()->GetTabLines().C40_GETPOS( SwTableLine, pLine );
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
        pFndBox->GetLines().ForEach( &lcl_CopyRow, &aCpyPara );
    }

    // clean up this Line's structure once again, generally all of them
    if( !pDoc->IsInReading() )
        GCLines();

    // Update Layout
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

    return sal_True;
}

sal_Bool _FndBoxAppendRowLine( const SwTableLine*& rpLine, void* pPara );

sal_Bool _FndBoxAppendRowBox( const SwTableBox*& rpBox, void* pPara )
{
    _FndPara* pFndPara = (_FndPara*)pPara;
    _FndBox* pFndBox = new _FndBox( (SwTableBox*)rpBox, pFndPara->pFndLine );
    if( rpBox->GetTabLines().Count() )
    {
        _FndPara aPara( *pFndPara, pFndBox );
        pFndBox->GetBox()->GetTabLines().ForEach( &_FndBoxAppendRowLine, &aPara );
        if( !pFndBox->GetLines().Count() )
            delete pFndBox;
    }
    else
        pFndPara->pFndLine->GetBoxes().push_back( pFndBox );
    return sal_True;
}

sal_Bool _FndBoxAppendRowLine( const SwTableLine*& rpLine, void* pPara )
{
    _FndPara* pFndPara = (_FndPara*)pPara;
    _FndLine* pFndLine = new _FndLine( (SwTableLine*)rpLine, pFndPara->pFndBox );
    _FndPara aPara( *pFndPara, pFndLine );
    pFndLine->GetLine()->GetTabBoxes().ForEach( &_FndBoxAppendRowBox, &aPara );
    if( pFndLine->GetBoxes().size() )
    {
        pFndPara->pFndBox->GetLines().C40_INSERT( _FndLine, pFndLine,
                pFndPara->pFndBox->GetLines().Count() );
    }
    else
        delete pFndLine;
    return sal_True;
}

sal_Bool SwTable::AppendRow( SwDoc* pDoc, sal_uInt16 nCnt )
{
    SwTableNode* pTblNd = (SwTableNode*)aSortCntBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return sal_False;

    // Find all Boxes/Lines
    _FndBox aFndBox( 0, 0 );
    {
        const SwTableLine* pLLine = GetTabLines()[ GetTabLines().Count()-1 ];

        const SwSelBoxes* pBxs = 0;     // Dummy!!!
        _FndPara aPara( *pBxs, &aFndBox );

        _FndBoxAppendRowLine( pLLine, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return sal_False;

    SetHTMLTableLayout( 0 );    // Delete HTML Layout

    // Find Lines for the Layout update
    bool bLayout = 0 != SwIterator<SwTabFrm,SwFmt>::FirstElement( *GetFrmFmt() );
    if( bLayout )
    {
        aFndBox.SetTableLines( *this );
        // TL_CHART2: nothing to be done since chart2 currently does not want to
        // get notified about new rows/cols.
    }

    _CpyTabFrms aTabFrmArr;
    _CpyPara aCpyPara( pTblNd, 0, aTabFrmArr );
    aCpyPara.nInsPos = GetTabLines().Count();
    aCpyPara.nDelBorderFlag = 1;

    for( sal_uInt16 nCpyCnt = 0; nCpyCnt < nCnt; ++nCpyCnt )
    {
        aCpyPara.nDelBorderFlag = 1;
        aFndBox.GetLines().ForEach( &lcl_CopyRow, &aCpyPara );
    }

    // Clean up this Line's structure once again, generally all of them
    if( !pDoc->IsInReading() )
        GCLines();

    // Update Layout
    if ( bLayout )
    {
        aFndBox.MakeNewFrms( *this, nCnt, sal_True );
    }
    // TL_CHART2: need to inform chart of probably changed cell names
    pDoc->UpdateCharts( GetFrmFmt()->GetName() );

    CHECKBOXWIDTH;
    CHECKTABLELAYOUT;

    return sal_True;
}

void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            sal_Bool bFirst, SwShareBoxFmts& rShareFmts );

void lcl_LastBoxSetWidthLine( SwTableLines &rLines, const long nOffset,
                                sal_Bool bFirst, SwShareBoxFmts& rShareFmts )
{
    for ( sal_uInt16 i = 0; i < rLines.Count(); ++i )
        ::lcl_LastBoxSetWidth( rLines[i]->GetTabBoxes(), nOffset, bFirst,
                                rShareFmts );
}

void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            sal_Bool bFirst, SwShareBoxFmts& rShareFmts )
{
    SwTableBox& rBox = *rBoxes[ bFirst ? 0 : rBoxes.Count() - 1 ];
    if( !rBox.GetSttNd() )
        ::lcl_LastBoxSetWidthLine( rBox.GetTabLines(), nOffset,
                                    bFirst, rShareFmts );

    // Adapt the Box
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
                sal_Bool bCalcNewSize, const sal_Bool bCorrBorder,
                SwShareBoxFmts* pShareFmts )
{
    do {
        SwTwips nBoxSz = bCalcNewSize ?
                pBox->GetFrmFmt()->GetFrmSize().GetWidth() : 0;
        SwTableLine* pLine = pBox->GetUpper();
        SwTableBoxes& rTblBoxes = pLine->GetTabBoxes();
        sal_uInt16 nDelPos = rTblBoxes.C40_GETPOS( SwTableBox, pBox );
        SwTableBox* pUpperBox = pBox->GetUpper()->GetUpper();

        // Special treatment for the border:
        if( bCorrBorder && 1 < rTblBoxes.Count() )
        {
            sal_Bool bChgd = sal_False;
            const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();

            if( rBoxItem.GetLeft() || rBoxItem.GetRight() )
            {
                // JP 02.04.97: 1st part for Bug 36271
                // First the left/right edges
                if( nDelPos + 1 < rTblBoxes.Count() )
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
                        bChgd = sal_True;
                    }
                }
                if( !bChgd && nDelPos )
                {
                    SwTableBox* pPrvBox = rTblBoxes[ nDelPos - 1 ];
                    const SvxBoxItem& rPrvBoxItem = pPrvBox->GetFrmFmt()->GetBox();

                    SwTableBox* pNxtBox = nDelPos + 1 < rTblBoxes.Count()
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

        // Delete the Box first, then the Nodes!
        SwStartNode* pSttNd = (SwStartNode*)pBox->GetSttNd();
        if( pShareFmts )
            pShareFmts->RemoveFormat( *rTblBoxes[ nDelPos ]->GetFrmFmt() );
        rTblBoxes.DeleteAndDestroy( nDelPos );

        if( pSttNd )
        {
            // Has the UndoObject been prepared to save the Section?
            if( pUndo && pUndo->IsDelBox() )
                ((SwUndoTblNdsChg*)pUndo)->SaveSection( pSttNd );
            else
                pSttNd->GetDoc()->DeleteSection( pSttNd );
        }

        // Also delete the Line?
        if( rTblBoxes.Count() )
        {
            // Then adapt the Frame-SSize
            sal_Bool bLastBox = nDelPos == rTblBoxes.Count();
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
                    // We need to this recursively in all Lines in all Cells!
                    SwShareBoxFmts aShareFmts;
                    ::lcl_LastBoxSetWidthLine( pBox->GetTabLines(), nBoxSz,
                                                !bLastBox,
                                                pShareFmts ? *pShareFmts
                                                           : aShareFmts );
                }
            }
            break;      // Stop deleting
        }
        // Delete the Line from the Table/Box
        if( !pUpperBox )
        {
            // Also delete the Line from the Table
            nDelPos = rTbl.GetTabLines().C40_GETPOS( SwTableLine, pLine );
            if( pShareFmts )
                pShareFmts->RemoveFormat( *rTbl.GetTabLines()[ nDelPos ]->GetFrmFmt() );
            rTbl.GetTabLines().DeleteAndDestroy( nDelPos );
            break;      // we cannot delete more
        }

        // finally also delete the Line
        pBox = pUpperBox;
        nDelPos = pBox->GetTabLines().C40_GETPOS( SwTableLine, pLine );
        if( pShareFmts )
            pShareFmts->RemoveFormat( *pBox->GetTabLines()[ nDelPos ]->GetFrmFmt() );
        pBox->GetTabLines().DeleteAndDestroy( nDelPos );
    } while( !pBox->GetTabLines().Count() );
}

SwTableBox* lcl_FndNxtPrvDelBox( const SwTableLines& rTblLns,
                                SwTwips nBoxStt, SwTwips nBoxWidth,
                                sal_uInt16 nLinePos, sal_Bool bNxt,
                                SwSelBoxes* pAllDelBoxes, SwSelBoxes::iterator* pCurPos )
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
        sal_uInt16 nBoxCnt = pLine->GetTabBoxes().Count();

        pFndBox = pLine->GetTabBoxes()[ 0 ];
        for( sal_uInt16 n = 0; 0 < nFndWidth && n < nBoxCnt; ++n )
        {
            pFndBox = pLine->GetTabBoxes()[ n ];
            nFndWidth -= (nFndBoxWidth = pFndBox->GetFrmFmt()->
                                        GetFrmSize().GetWidth());
        }

        // Find the first ContentBox
        while( !pFndBox->GetSttNd() )
        {
            const SwTableLines& rLowLns = pFndBox->GetTabLines();
            if( bNxt )
                pFndBox = rLowLns[ 0 ]->GetTabBoxes()[ 0 ];
            else
                pFndBox = rLowLns[ rLowLns.Count() - 1 ]->GetTabBoxes()[ 0 ];
        }

        if( Abs( nFndWidth ) > COLFUZZY ||
            Abs( nBoxWidth - nFndBoxWidth ) > COLFUZZY )
            pFndBox = 0;
        else if( pAllDelBoxes )
        {
            // If the predecessor will also be deleted, there's nothing to do
            SwSelBoxes::iterator it = pAllDelBoxes->find( pFndBox );
            if( it == pAllDelBoxes->end() )
                break;

            // else, we keep on searching.
            // We do not need to recheck the Box, however
            if( pFndBox->GetSttIdx() <= (*pCurPos)->second->GetSttIdx() )
                --*pCurPos;
            pFndBox = 0;
            pAllDelBoxes->erase( it );
        }
    } while( bNxt ? ( nLinePos + 1 < rTblLns.Count() ) : nLinePos );
    return pFndBox;
}

void lcl_SaveUpperLowerBorder( SwTable& rTbl, const SwTableBox& rBox,
                                SwShareBoxFmts& rShareFmts,
                                SwSelBoxes* pAllDelBoxes = 0,
                                SwSelBoxes::iterator* pCurPos = 0 )
{
//JP 16.04.97:  2. part for Bug 36271
    sal_Bool bChgd = sal_False;
    const SwTableLine* pLine = rBox.GetUpper();
    const SwTableBoxes& rTblBoxes = pLine->GetTabBoxes();
    const SwTableBox* pUpperBox = &rBox;
    sal_uInt16 nDelPos = rTblBoxes.C40_GETPOS( SwTableBox, pUpperBox );
    pUpperBox = rBox.GetUpper()->GetUpper();
    const SvxBoxItem& rBoxItem = rBox.GetFrmFmt()->GetBox();

    // then the top/bottom edges
    if( rBoxItem.GetTop() || rBoxItem.GetBottom() )
    {
        bChgd = sal_False;
        const SwTableLines* pTblLns;
        if( pUpperBox )
            pTblLns = &pUpperBox->GetTabLines();
        else
            pTblLns = &rTbl.GetTabLines();

        sal_uInt16 nLnPos = pTblLns->GetPos( pLine );

        // Calculate the attribute position of the top-be-deleted Box and then
        // search in the top/bottom Line of the respective counterparts.
        SwTwips nBoxStt = 0;
        for( sal_uInt16 n = 0; n < nDelPos; ++n )
            nBoxStt += rTblBoxes[ n ]->GetFrmFmt()->GetFrmSize().GetWidth();
        SwTwips nBoxWidth = rBox.GetFrmFmt()->GetFrmSize().GetWidth();

        SwTableBox *pPrvBox = 0, *pNxtBox = 0;
        if( nLnPos )        // Predecessor?
            pPrvBox = ::lcl_FndNxtPrvDelBox( *pTblLns, nBoxStt, nBoxWidth,
                                nLnPos, sal_False, pAllDelBoxes, pCurPos );

        if( nLnPos + 1 < pTblLns->Count() )     // Successor?
            pNxtBox = ::lcl_FndNxtPrvDelBox( *pTblLns, nBoxStt, nBoxWidth,
                                nLnPos, sal_True, pAllDelBoxes, pCurPos );

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
                bChgd = sal_True;
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

sal_Bool SwTable::DeleteSel(
    SwDoc*     pDoc
    ,
    const SwSelBoxes& rBoxes,
    const SwSelBoxes* pMerged, SwUndo* pUndo,
    const sal_Bool bDelMakeFrms, const sal_Bool bCorrBorder )
{
    OSL_ENSURE( pDoc, "No doc?" );
    SwTableNode* pTblNd = 0;
    if( !rBoxes.empty() )
    {
        pTblNd = const_cast<SwTableNode*>(rBoxes.begin()->second->GetSttNd()->FindTableNode());
        if( !pTblNd )
            return sal_False;
    }

    SetHTMLTableLayout( 0 );    // Delete HTML Layout

    // Find Lines for the Layout update
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

    // First switch the Border, then delete
    if( bCorrBorder )
    {
        SwSelBoxes aBoxes( rBoxes );
        for( SwSelBoxes::iterator it = aBoxes.begin(); it != aBoxes.end(); ++it )
            ::lcl_SaveUpperLowerBorder( *this, *it->second, aShareFmts,
                                        &aBoxes, &it );
    }

    PrepareDelBoxes( rBoxes );

    SwChartDataProvider *pPCD = pDoc->GetChartDataProvider();
    // Delete boxes from last to first
    for( SwSelBoxes::const_reverse_iterator it = rBoxes.rbegin(); it != rBoxes.rend(); ++it )
    {
        // First adapt the data-sequence for chart if necessary
        // (needed to move the implementation cursor properly to it's new
        // position which can't be done properly if the cell is already gone)
        if (pPCD && pTblNd)
            pPCD->DeleteBox( &pTblNd->GetTable(), *it->second );

        // ... then delete the boxes
        _DeleteBox( *this, it->second, pUndo, sal_True, bCorrBorder, &aShareFmts );
    }

    // then clean up the structure of all Lines
    GCLines();

    if( bDelMakeFrms && aFndBox.AreLinesToRestore( *this ) )
        aFndBox.MakeFrms( *this );

    // TL_CHART2: now inform chart that sth has changed
    pDoc->UpdateCharts( GetFrmFmt()->GetName() );

    CHECKTABLELAYOUT;
    CHECK_TABLE( *this );

    return sal_True;
}

sal_Bool SwTable::OldSplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt,
                        sal_Bool bSameHeight )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTblNd = const_cast<SwTableNode*>(rBoxes.begin()->second->GetSttNd()->FindTableNode());
    if( !pTblNd )
        return sal_False;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table to complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    pDoc->CreateChartInternalDataProviders( this );

    SetHTMLTableLayout( 0 );    // Delete HTML Layout

    // If the rows should get the same (min) height, we first have
    // to store the old row heights before deleting the frames
    std::vector<long> aRowHeights;
    if ( bSameHeight )
    {
        aRowHeights.reserve(rBoxes.size());
        for( SwSelBoxes::const_iterator it = rBoxes.begin(); it != rBoxes.end(); ++it )
        {
            SwTableBox* pSelBox = it->second;
            const SwRowFrm* pRow = GetRowFrm( *pSelBox->GetUpper() );
            OSL_ENSURE( pRow, "Where is the SwTableLine's Frame?" );
            SWRECTFN( pRow )
            aRowHeights.push_back((pRow->Frm().*fnRect->fnGetHeight)());
        }
    }

    // Find Lines for the Layout update
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rBoxes, *this );
    aFndBox.DelFrms( *this );

    sal_uInt16 n = 0;
    for( SwSelBoxes::const_iterator it = rBoxes.begin(); it != rBoxes.end(); ++it, ++n )
    {
        SwTableBox* pSelBox = it->second;
        OSL_ENSURE( pSelBox, "Box is not within the Table" );

        // Insert nCnt new Lines into the Box
        SwTableLine* pInsLine = pSelBox->GetUpper();
        SwTableBoxFmt* pFrmFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();

        // Respect the Line's height, reset if needed
        SwFmtFrmSize aFSz( pInsLine->GetFrmFmt()->GetFrmSize() );
        if ( bSameHeight && ATT_VAR_SIZE == aFSz.GetHeightSizeType() )
            aFSz.SetHeightSizeType( ATT_MIN_SIZE );

        sal_Bool bChgLineSz = 0 != aFSz.GetHeight() || bSameHeight;
        if ( bChgLineSz )
            aFSz.SetHeight( ( bSameHeight ? aRowHeights[ n ] : aFSz.GetHeight() ) /
                             (nCnt + 1) );

        SwTableBox* pNewBox = new SwTableBox( pFrmFmt, nCnt, pInsLine );
        sal_uInt16 nBoxPos = pInsLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSelBox );
        pInsLine->GetTabBoxes().Remove( nBoxPos );  // delete old ones
        pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pNewBox, nBoxPos );

        // Delete background/border attribute
        SwTableBox* pLastBox = pSelBox;         // To distribute the TextNodes!
        // If Areas are contained in the Box, it stays as is
        // !! If this is changed we need to adapt the Undo, too !!!
        sal_Bool bMoveNodes = sal_True;
        {
            sal_uLong nSttNd = pLastBox->GetSttIdx() + 1,
                    nEndNd = pLastBox->GetSttNd()->EndOfSectionIndex();
            while( nSttNd < nEndNd )
                if( !pDoc->GetNodes()[ nSttNd++ ]->IsTxtNode() )
                {
                    bMoveNodes = sal_False;
                    break;
                }
        }

        SwTableBoxFmt* pCpyBoxFrmFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();
        sal_Bool bChkBorder = 0 != pCpyBoxFrmFmt->GetBox().GetTop();
        if( bChkBorder )
            pCpyBoxFrmFmt = (SwTableBoxFmt*)pSelBox->ClaimFrmFmt();

        for( sal_uInt16 i = 0; i <= nCnt; ++i )
        {
            // Create a new Line in the new Box
            SwTableLine* pNewLine = new SwTableLine(
                    (SwTableLineFmt*)pInsLine->GetFrmFmt(), 1, pNewBox );
            if( bChgLineSz )
            {
                pNewLine->ClaimFrmFmt()->SetFmtAttr( aFSz );
            }

            pNewBox->GetTabLines().C40_INSERT( SwTableLine, pNewLine, i );
            // then a new Box in the Line
            if( !i )        // hang up the original Box
            {
                pSelBox->SetUpper( pNewLine );
                pNewLine->GetTabBoxes().C40_INSERT( SwTableBox, pSelBox, 0 );
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
                    bChkBorder = sal_False;
                }

                if( bMoveNodes )
                {
                    const SwNode* pEndNd = pLastBox->GetSttNd()->EndOfSectionNode();
                    if( pLastBox->GetSttIdx()+2 != pEndNd->GetIndex() )
                    {
                        // Move TextNodes
                        SwNodeRange aRg( *pLastBox->GetSttNd(), +2, *pEndNd );
                        pLastBox = pNewLine->GetTabBoxes()[0];  // reset
                        SwNodeIndex aInsPos( *pLastBox->GetSttNd(), 1 );
                        pDoc->GetNodes()._MoveNodes(aRg, pDoc->GetNodes(), aInsPos, sal_False);
                        pDoc->GetNodes().Delete( aInsPos, 1 ); // delete the empty one
                    }
                }
            }
        }
        // In Boxes with Lines, we can only have Size/Fillorder
        pFrmFmt = (SwTableBoxFmt*)pNewBox->ClaimFrmFmt();
        pFrmFmt->ResetFmtAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
        pFrmFmt->ResetFmtAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
    }

    GCLines();

    aFndBox.MakeFrms( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT
    return sal_True;
}

sal_Bool SwTable::SplitCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt )
{
    OSL_ENSURE( pDoc && !rBoxes.empty() && nCnt, "No valid values" );
    SwTableNode* pTblNd = const_cast<SwTableNode*>(rBoxes.begin()->second->GetSttNd()->FindTableNode());
    if( !pTblNd )
        return sal_False;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table to complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    pDoc->CreateChartInternalDataProviders( this );

    SetHTMLTableLayout( 0 );    // Delete HTML Layout
    SwSelBoxes aSelBoxes( rBoxes );
    ExpandSelection( aSelBoxes );

    // Find Lines for the Layout update
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( aSelBoxes, *this );
    aFndBox.DelFrms( *this );

    _CpyTabFrms aFrmArr;
    SvPtrarr aLastBoxArr;
    sal_uInt16 nFndPos;
    for( SwSelBoxes::const_iterator it = aSelBoxes.begin(); it != aSelBoxes.end(); ++it )
    {
        SwTableBox* pSelBox = it->second;
        OSL_ENSURE( pSelBox, "Box steht nicht in der Tabelle" );

        // We don't want to split small table cells into very very small cells
        if( pSelBox->GetFrmFmt()->GetFrmSize().GetWidth()/( nCnt + 1 ) < 10 )
            continue;

        // Then split the nCnt Box up into nCnt Boxes
        SwTableLine* pInsLine = pSelBox->GetUpper();
        sal_uInt16 nBoxPos = pInsLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSelBox );

        // Find the Frame Format in the Frame Format Array
        SwTableBoxFmt* pLastBoxFmt;
        _CpyTabFrm aFindFrm( (SwTableBoxFmt*)pSelBox->GetFrmFmt() );
        if( !aFrmArr.Seek_Entry( aFindFrm, &nFndPos ))
        {
            // Change the FrmFmt
            aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pSelBox->ClaimFrmFmt();
            SwTwips nBoxSz = aFindFrm.pNewFrmFmt->GetFrmSize().GetWidth();
            SwTwips nNewBoxSz = nBoxSz / ( nCnt + 1 );
            aFindFrm.pNewFrmFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                                        nNewBoxSz, 0 ) );
            aFrmArr.Insert( aFindFrm );

            pLastBoxFmt = aFindFrm.pNewFrmFmt;
            if( nBoxSz != ( nNewBoxSz * (nCnt + 1)))
            {
                // We have a remainder, so we need to define an own Format
                // for the last Box.
                pLastBoxFmt = new SwTableBoxFmt( *aFindFrm.pNewFrmFmt );
                pLastBoxFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                nBoxSz - ( nNewBoxSz * nCnt ), 0 ) );
            }
            void* p = pLastBoxFmt;
            aLastBoxArr.Insert( p, nFndPos );
        }
        else
        {
            aFindFrm = aFrmArr[ nFndPos ];
            pSelBox->ChgFrmFmt( (SwTableBoxFmt*)aFindFrm.pNewFrmFmt );
            pLastBoxFmt = (SwTableBoxFmt*)aLastBoxArr[ nFndPos ];
        }

        // Insert the Boxes at the Position
        for( sal_uInt16 i = 1; i < nCnt; ++i )
            ::_InsTblBox( pDoc, pTblNd, pInsLine, aFindFrm.pNewFrmFmt,
                        pSelBox, nBoxPos + i ); // insert after

        ::_InsTblBox( pDoc, pTblNd, pInsLine, pLastBoxFmt,
                    pSelBox, nBoxPos + nCnt );  // insert after

        // Special treatment for the Border:
        const SvxBoxItem& aSelBoxItem = aFindFrm.pNewFrmFmt->GetBox();
        if( aSelBoxItem.GetRight() )
        {
            pInsLine->GetTabBoxes()[ nBoxPos + nCnt ]->ClaimFrmFmt();

            SvxBoxItem aTmp( aSelBoxItem );
            aTmp.SetLine( 0, BOX_LINE_RIGHT );
            aFindFrm.pNewFrmFmt->SetFmtAttr( aTmp );

            // Remove the Format from the "cache"
            for( sal_uInt16 i = aFrmArr.Count(); i; )
            {
                const _CpyTabFrm& rCTF = aFrmArr[ --i ];
                if( rCTF.pNewFrmFmt == aFindFrm.pNewFrmFmt ||
                    rCTF.Value.pFrmFmt == aFindFrm.pNewFrmFmt )
                {
                    aFrmArr.Remove( i );
                    aLastBoxArr.Remove( i );
                }
            }
        }
    }

    // Update Layout
    aFndBox.MakeFrms( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT
    return sal_True;
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
void lcl_CpyLines( sal_uInt16 nStt, sal_uInt16 nEnd,
                                SwTableLines& rLines,
                                SwTableBox* pInsBox,
                                sal_uInt16 nPos = USHRT_MAX )
{
    for( sal_uInt16 n = nStt; n < nEnd; ++n )
        rLines[n]->SetUpper( pInsBox );
    if( USHRT_MAX == nPos )
        nPos = pInsBox->GetTabLines().Count();
    pInsBox->GetTabLines().Insert( &rLines, nPos, nStt, nEnd );
    rLines.Remove( nStt, nEnd - nStt );
}

void lcl_CpyBoxes( sal_uInt16 nStt, sal_uInt16 nEnd,
                                SwTableBoxes& rBoxes,
                                SwTableLine* pInsLine,
                                sal_uInt16 nPos = USHRT_MAX )
{
    for( sal_uInt16 n = nStt; n < nEnd; ++n )
        rBoxes[n]->SetUpper( pInsLine );
    if( USHRT_MAX == nPos )
        nPos = pInsLine->GetTabBoxes().Count();
    pInsLine->GetTabBoxes().Insert( &rBoxes, nPos, nStt, nEnd );
    rBoxes.Remove( nStt, nEnd - nStt );
}

void lcl_CalcWidth( SwTableBox* pBox )
{
    // Assertion: Every Line in the Box is as large
    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
    OSL_ENSURE( pBox->GetTabLines().Count(), "Box does not have any Lines" );

    SwTableLine* pLine = pBox->GetTabLines()[0];
    OSL_ENSURE( pLine, "Box is not within a Line" );

    long nWidth = 0;
    for( sal_uInt16 n = 0; n < pLine->GetTabBoxes().Count(); ++n )
        nWidth += pLine->GetTabBoxes()[n]->GetFrmFmt()->GetFrmSize().GetWidth();

    pFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));

    // Boxes with Lines can only have Size/Fillorder
    pFmt->ResetFmtAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
    pFmt->ResetFmtAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
}

struct _InsULPara
{
    SwTableNode* pTblNd;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    sal_Bool bUL_LR : 1;        // Upper-Lower(sal_True) or Left-Right(sal_False) ?
    sal_Bool bUL : 1;           // Upper-Left(sal_True) or Lower-Right(sal_False) ?

    SwTableBox* pLeftBox;
    SwTableBox* pRightBox;
    SwTableBox* pMergeBox;

    _InsULPara( SwTableNode* pTNd, sal_Bool bUpperLower, sal_Bool bUpper,
                SwTableBox* pLeft, SwTableBox* pMerge, SwTableBox* pRight,
                SwTableLine* pLine=0, SwTableBox* pBox=0 )
        : pTblNd( pTNd ), pInsLine( pLine ), pInsBox( pBox ),
        pLeftBox( pLeft ), pRightBox( pRight ), pMergeBox( pMerge )
        {   bUL_LR = bUpperLower; bUL = bUpper; }

    void SetLeft( SwTableBox* pBox=0 )
        { bUL_LR = sal_False;   bUL = sal_True; if( pBox ) pInsBox = pBox; }
    void SetRight( SwTableBox* pBox=0 )
        { bUL_LR = sal_False;   bUL = sal_False; if( pBox ) pInsBox = pBox; }
    void SetUpper( SwTableLine* pLine=0 )
        { bUL_LR = sal_True;    bUL = sal_True;  if( pLine ) pInsLine = pLine; }
    void SetLower( SwTableLine* pLine=0 )
        { bUL_LR = sal_True;    bUL = sal_False; if( pLine ) pInsLine = pLine; }
};

sal_Bool lcl_Merge_MoveBox( _FndBox* pFndBox, _InsULPara* pULPara )
{
    SwTableBoxes* pBoxes;

    sal_uInt16 nStt = 0, nEnd = pFndBox->GetLines().Count();
    sal_uInt16 nInsPos = USHRT_MAX;
    if( !pULPara->bUL_LR )  // Left/Right
    {
        sal_uInt16 nPos;
        SwTableBox* pFndTableBox = (SwTableBox*)pFndBox->GetBox();
        pBoxes = &pFndTableBox->GetUpper()->GetTabBoxes();
        if( pULPara->bUL )  // Left ?
        {
            // if there are Boxes before it, move them
            if( 0 != ( nPos = pBoxes->C40_GETPOS( SwTableBox, pFndTableBox )) )
                lcl_CpyBoxes( 0, nPos, *pBoxes, pULPara->pInsLine );
        }
        else                // Right
            // if there are Boxes behind it, move them
            if( (nPos = pBoxes->C40_GETPOS( SwTableBox, pFndTableBox )) +1 < pBoxes->Count() )
            {
                nInsPos = pULPara->pInsLine->GetTabBoxes().Count();
                lcl_CpyBoxes( nPos+1, pBoxes->Count(),
                                    *pBoxes, pULPara->pInsLine );
            }
    }
    // Upper/Lower and still deeper?
    else if( pFndBox->GetLines().Count() )
    {
        // Only search the Line from which we need to move
        nStt = pULPara->bUL ? 0 : pFndBox->GetLines().Count()-1;
        nEnd = nStt+1;
    }

    pBoxes = &pULPara->pInsLine->GetTabBoxes();

    // Is there still a level to step down to?
    if( pFndBox->GetBox()->GetTabLines().Count() )
    {
        SwTableBox* pBox = new SwTableBox(
                (SwTableBoxFmt*)pFndBox->GetBox()->GetFrmFmt(), 0, pULPara->pInsLine );
        _InsULPara aPara( *pULPara );
        aPara.pInsBox = pBox;
        pFndBox->GetLines().ForEach( nStt, nEnd,
                                                &lcl_Merge_MoveLine, &aPara );
        if( pBox->GetTabLines().Count() )
        {
            if( USHRT_MAX == nInsPos )
                nInsPos = pBoxes->Count();
            pBoxes->C40_INSERT( SwTableBox, pBox, nInsPos );
            lcl_CalcWidth( pBox );      // calculate the Box's width
        }
        else
            delete pBox;
    }
    return sal_True;
}

sal_Bool lcl_Merge_MoveLine( const _FndLine*& rpFndLine, void* pPara )
{
    _InsULPara* pULPara = (_InsULPara*)pPara;
    SwTableLines* pLines;

    sal_uInt16 nStt = 0, nEnd = rpFndLine->GetBoxes().size();
    sal_uInt16 nInsPos = USHRT_MAX;
    if( pULPara->bUL_LR )   // UpperLower ?
    {
        sal_uInt16 nPos;
        SwTableLine* pFndLn = (SwTableLine*)rpFndLine->GetLine();
        pLines = pFndLn->GetUpper() ?
                        &pFndLn->GetUpper()->GetTabLines() :
                        &pULPara->pTblNd->GetTable().GetTabLines();

        SwTableBox* pLBx = rpFndLine->GetBoxes()[0]->GetBox();
        SwTableBox* pRBx = rpFndLine->GetBoxes()[
                            rpFndLine->GetBoxes().size()-1]->GetBox();
        sal_uInt16 nLeft = pFndLn->GetTabBoxes().C40_GETPOS( SwTableBox, pLBx );
        sal_uInt16 nRight = pFndLn->GetTabBoxes().C40_GETPOS( SwTableBox, pRBx );

        if( !nLeft || nRight == pFndLn->GetTabBoxes().Count() )
        {
            if( pULPara->bUL )  // Upper ?
            {
                // If there are Lines before it, move them
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pULPara->pInsBox );
            }
            else
                // If there are Lines after it, move them
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                {
                    nInsPos = pULPara->pInsBox->GetTabLines().Count();
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pULPara->pInsBox );
                }
        }
        else if( nLeft )
        {
            // There are still Boxes on the left side, so put the Left-
            // and Merge-Box into one Box and Line, insert before/after
            // a Line with a Box, into which the upper/lower Lines are
            // inserted
            SwTableLine* pInsLine = pULPara->pLeftBox->GetUpper();
            SwTableBox* pLMBox = new SwTableBox(
                (SwTableBoxFmt*)pULPara->pLeftBox->GetFrmFmt(), 0, pInsLine );
            SwTableLine* pLMLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pLMBox );
            pLMLn->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );

            pLMBox->GetTabLines().C40_INSERT( SwTableLine, pLMLn, 0 );

            lcl_CpyBoxes( 0, 2, pInsLine->GetTabBoxes(), pLMLn );

            pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pLMBox, 0 );

            if( pULPara->bUL )  // Upper ?
            {
                // If there are Lines before it, move them
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pLMBox, 0 );
            }
            else
                // If there are Lines after it, move them
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pLMBox );
            lcl_CalcWidth( pLMBox );        // calculate the Box's width
        }
        else if( nRight+1 < pFndLn->GetTabBoxes().Count() )
        {
            // There are still Boxes on the right, so put the Right-
            // and Merge-Box into one Box and Line, insert before/after
            // a Line with a Box, into which the upper/lower Lines are
            // inserted
            SwTableLine* pInsLine = pULPara->pRightBox->GetUpper();
            SwTableBox* pRMBox;
            if( pULPara->pLeftBox->GetUpper() == pInsLine )
            {
                pRMBox = new SwTableBox(
                    (SwTableBoxFmt*)pULPara->pRightBox->GetFrmFmt(), 0, pInsLine );
                SwTableLine* pRMLn = new SwTableLine(
                    (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pRMBox );
                pRMLn->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );
                pRMBox->GetTabLines().C40_INSERT( SwTableLine, pRMLn, 0 );

                lcl_CpyBoxes( 1, 3, pInsLine->GetTabBoxes(), pRMLn );

                pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox, 0 );
            }
            else
            {
                // Left and Merge have been merged, so also move Right into the Line
                pInsLine = pULPara->pLeftBox->GetUpper();
                sal_uInt16 nMvPos = pULPara->pRightBox->GetUpper()->GetTabBoxes().
                                    C40_GETPOS( SwTableBox, pULPara->pRightBox );
                lcl_CpyBoxes( nMvPos, nMvPos+1,
                            pULPara->pRightBox->GetUpper()->GetTabBoxes(),
                            pInsLine );
                pRMBox = pInsLine->GetUpper();

                // If there are already Lines, then these need to go into a new Line and Box
                nMvPos = pRMBox->GetTabLines().C40_GETPOS( SwTableLine, pInsLine );
                if( pULPara->bUL ? nMvPos
                                : nMvPos+1 < pRMBox->GetTabLines().Count() )
                {
                    // Merge all Lines into a new Line and Box
                    SwTableLine* pNewLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 1, pRMBox );
                    pNewLn->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );
                    pRMBox->GetTabLines().C40_INSERT( SwTableLine, pNewLn,
                            pULPara->bUL ? nMvPos : nMvPos+1 );
                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox, 0 );

                    sal_uInt16 nPos1, nPos2;
                    if( pULPara->bUL )
                        nPos1 = 0,
                        nPos2 = nMvPos;
                    else
                        nPos1 = nMvPos+2,
                        nPos2 = pNewLn->GetUpper()->GetTabLines().Count();

                    lcl_CpyLines( nPos1, nPos2,
                                pNewLn->GetUpper()->GetTabLines(), pRMBox );
                    lcl_CalcWidth( pRMBox );        // calculate the Box's width

                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox,
                                    pNewLn->GetTabBoxes().Count() );
                }
            }
            if( pULPara->bUL )  // Upper ?
            {
                // If there are Lines before it, move them
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pRMBox, 0 );
            }
            else
                // If there are Lines after it, move them
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pRMBox );
            lcl_CalcWidth( pRMBox );        // calculate the Box's width
        }
        else {
            OSL_FAIL( "So ... what do we do now?" );
        }
    }
    // Left/Right
    else
    {
        // Find only the Line from which we need to move
        nStt = pULPara->bUL ? 0 : rpFndLine->GetBoxes().size()-1;
        nEnd = nStt+1;
    }
    pLines = &pULPara->pInsBox->GetTabLines();

    SwTableLine* pNewLine = new SwTableLine(
        (SwTableLineFmt*)rpFndLine->GetLine()->GetFrmFmt(), 0, pULPara->pInsBox );
    _InsULPara aPara( *pULPara );       // kopieren
    aPara.pInsLine = pNewLine;
    _FndBoxes& rLineBoxes = ((_FndLine*)rpFndLine)->GetBoxes();
    for (_FndBoxes::const_iterator it = rLineBoxes.begin() + nStt;
         it != rLineBoxes.begin() + nEnd; ++it)
        lcl_Merge_MoveBox(*it, &aPara);

    if( pNewLine->GetTabBoxes().Count() )
    {
        if( USHRT_MAX == nInsPos )
            nInsPos = pLines->Count();
        pLines->C40_INSERT( SwTableLine, pNewLine, nInsPos );
    }
    else
        delete pNewLine;

    return sal_True;
}

sal_Bool SwTable::OldMerge( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        SwTableBox* pMergeBox, SwUndoTblMerge* pUndo )
{
    OSL_ENSURE( !rBoxes.empty() && pMergeBox, "no valid values" );
    SwTableNode* pTblNd = const_cast<SwTableNode*>(rBoxes.begin()->second->GetSttNd()->FindTableNode());
    if( !pTblNd )
        return sal_False;

    // Find all Boxes/Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return sal_False;

    // TL_CHART2: splitting/merging of a number of cells or rows will usually make
    // the table to complex to be handled with chart.
    // Thus we tell the charts to use their own data provider and forget about this table
    pDoc->CreateChartInternalDataProviders( this );

    SetHTMLTableLayout( 0 );    // Delete HTML Layout

    if( pUndo )
        pUndo->SetSelBoxes( rBoxes );

    // Find Lines for the Layout update
    aFndBox.SetTableLines( *this );
    aFndBox.DelFrms( *this );

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().Count() &&
            1 == pFndBox->GetLines()[0]->GetBoxes().size() )
        pFndBox = pFndBox->GetLines()[0]->GetBoxes().front();

    SwTableLine* pInsLine = new SwTableLine(
                (SwTableLineFmt*)pFndBox->GetLines()[0]->GetLine()->GetFrmFmt(), 0,
                !pFndBox->GetUpper() ? 0 : pFndBox->GetBox() );
    pInsLine->ClaimFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );

    // Add the new Line
    SwTableLines* pLines =  pFndBox->GetUpper() ?
                  &pFndBox->GetBox()->GetTabLines() :  &GetTabLines();

    SwTableLine* pNewLine = pFndBox->GetLines()[0]->GetLine();
    sal_uInt16 nInsPos = pLines->C40_GETPOS( SwTableLine, pNewLine );
    pLines->C40_INSERT( SwTableLine, pInsLine, nInsPos );

    SwTableBox* pLeftBox = new SwTableBox( (SwTableBoxFmt*)pMergeBox->GetFrmFmt(), 0, pInsLine );
    SwTableBox* pRightBox = new SwTableBox( (SwTableBoxFmt*)pMergeBox->GetFrmFmt(), 0, pInsLine );
    pMergeBox->SetUpper( pInsLine );
    pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pLeftBox, 0 );
    pLeftBox->ClaimFrmFmt();
    pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pMergeBox, 1 );
    pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pRightBox, 2 );
    pRightBox->ClaimFrmFmt();

    // This contains all Lines that are above the selected Area,
    // thus they form a Upper/Lower Line
    _InsULPara aPara( pTblNd, sal_True, sal_True, pLeftBox, pMergeBox, pRightBox, pInsLine );

    // Move the overlapping upper/lower Lines of the selected Area
    _FndBoxes& rLineBoxes = pFndBox->GetLines()[0]->GetBoxes();
    for (_FndBoxes::const_iterator it = rLineBoxes.begin(); it != rLineBoxes.end(); ++it)
        lcl_Merge_MoveBox(*it, &aPara);
    aPara.SetLower( pInsLine );
    sal_uInt16 nEnd = pFndBox->GetLines().Count()-1;
    rLineBoxes = pFndBox->GetLines()[nEnd]->GetBoxes();
    for (_FndBoxes::const_iterator it = rLineBoxes.begin(); it != rLineBoxes.end(); ++it)
        lcl_Merge_MoveBox(*it, &aPara);

    // Move the Boxes extending into the selected Area from left/right
    aPara.SetLeft( pLeftBox );
    pFndBox->GetLines().ForEach( &lcl_Merge_MoveLine, &aPara );

    aPara.SetRight( pRightBox );
    pFndBox->GetLines().ForEach( &lcl_Merge_MoveLine, &aPara );

    if( !pLeftBox->GetTabLines().Count() )
        _DeleteBox( *this, pLeftBox, 0, sal_False, sal_False );
    else
    {
        lcl_CalcWidth( pLeftBox );      // calculate the Box's width
        if( pUndo && pLeftBox->GetSttNd() )
            pUndo->AddNewBox( pLeftBox->GetSttIdx() );
    }
    if( !pRightBox->GetTabLines().Count() )
        _DeleteBox( *this, pRightBox, 0, sal_False, sal_False );
    else
    {
        lcl_CalcWidth( pRightBox );     // calculate the Box's width
        if( pUndo && pRightBox->GetSttNd() )
            pUndo->AddNewBox( pRightBox->GetSttIdx() );
    }

    DeleteSel( pDoc, rBoxes, 0, 0, sal_False, sal_False );

    // Clean up this Line's structure once again, generally all of them
    GCLines();

    GetTabLines()[0]->GetTabBoxes().ForEach( &lcl_BoxSetHeadCondColl, 0 );

    aFndBox.MakeFrms( *this );

    CHECKBOXWIDTH
    CHECKTABLELAYOUT

    return sal_True;
}

void lcl_CheckRowSpan( SwTable &rTbl )
{
    sal_uInt16 nLineCount = rTbl.GetTabLines().Count();
    sal_uInt16 nMaxSpan = nLineCount;
    long nMinSpan = 1;
    while( nMaxSpan )
    {
        SwTableLine* pLine = rTbl.GetTabLines()[ nLineCount - nMaxSpan ];
        for( sal_uInt16 nBox = 0; nBox < pLine->GetTabBoxes().Count(); ++nBox )
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

sal_uInt16 lcl_GetBoxOffset( const _FndBox& rBox )
{
    // Find the first Box
    const _FndBox* pFirstBox = &rBox;
    while( pFirstBox->GetLines().Count() )
        pFirstBox = pFirstBox->GetLines()[ 0 ]->GetBoxes()[ 0 ];

    sal_uInt16 nRet = 0;
    // Calculate the position relative to above via the Lines
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

sal_uInt16 lcl_GetLineWidth( const _FndLine& rLine )
{
    sal_uInt16 nRet = 0;
    for( sal_uInt16 n = rLine.GetBoxes().size(); n; )
        nRet = nRet + (sal_uInt16)rLine.GetBoxes()[ --n ]->GetBox()->GetFrmFmt()
                        ->GetFrmSize().GetWidth();
    return nRet;
}

void lcl_CalcNewWidths( const _FndLines& rFndLines, _CpyPara& rPara )
{
    rPara.pWidths.reset();
    sal_uInt16 nLineCount = rFndLines.Count();
    if( nLineCount )
    {
        rPara.pWidths = boost::shared_ptr< std::vector< std::vector< sal_uLong > > >
                        ( new std::vector< std::vector< sal_uLong > >( nLineCount ));
        // First we collect information about the left/right borders of all
        // selected cells
        for( sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            std::vector< sal_uLong > &rWidth = (*rPara.pWidths.get())[ nLine ];
            const _FndLine *pFndLine = rFndLines[ nLine ];
            if( pFndLine && pFndLine->GetBoxes().size() )
            {
                const SwTableLine *pLine = pFndLine->GetLine();
                if( pLine && pLine->GetTabBoxes().Count() )
                {
                    sal_uInt16 nBoxCount = pLine->GetTabBoxes().Count();
                    sal_uLong nPos = 0;
                    // The first selected box...
                    const SwTableBox *pSel = pFndLine->GetBoxes()[0]->GetBox();
                    sal_uInt16 nBox = 0;
                    // Sum up the width of all boxes before the first selected box
                    while( nBox < nBoxCount )
                    {
                        SwTableBox* pBox = pLine->GetTabBoxes()[nBox++];
                        if( pBox != pSel )
                            nPos += pBox->GetFrmFmt()->GetFrmSize().GetWidth();
                        else
                            break;
                    }
                    // nPos is now the left border of the first selected box
                    if( rPara.nMinLeft > nPos )
                        rPara.nMinLeft = nPos;
                    nBoxCount = pFndLine->GetBoxes().size();
                    rWidth = std::vector< sal_uLong >( nBoxCount+2 );
                    rWidth[ 0 ] = nPos;
                    // Add now the widths of all selected boxes and store
                    // the positions in the vector
                    for( nBox = 0; nBox < nBoxCount; )
                    {
                        nPos += pFndLine->GetBoxes()[nBox]
                            ->GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
                        rWidth[ ++nBox ] = nPos;
                    }
                    // nPos: The right border of the last selected box
                    if( rPara.nMaxRight < nPos )
                        rPara.nMaxRight = nPos;
                    if( nPos <= rWidth[ 0 ] )
                        rWidth.clear();
                }
            }
        }
    }
    // Second step: calculate the new widths for the copied cells
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

sal_Bool lcl_CopyBoxToDoc( _FndBox* pFndBox, _CpyPara* pCpyPara )
{
    // Calculation of new size
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
        nRealSize *= pFndBox->GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
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
        // Find the Frame Format in the list of all Frame Formats
        _CpyTabFrm aFindFrm( (SwTableBoxFmt*)pFndBox->GetBox()->GetFrmFmt() );

        SwFmtFrmSize aFrmSz;
        sal_uInt16 nFndPos;
        if( !pCpyPara->rTabFrmArr.Seek_Entry( aFindFrm, &nFndPos ) ||
            ( aFrmSz = ( aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ]).pNewFrmFmt->
                GetFrmSize()).GetWidth() != (SwTwips)nSize )
        {
            // It doesn't exist yet, so copy it
            aFindFrm.pNewFrmFmt = pCpyPara->pDoc->MakeTableBoxFmt();
            aFindFrm.pNewFrmFmt->CopyAttrs( *pFndBox->GetBox()->GetFrmFmt() );
            if( !pCpyPara->bCpyCntnt )
                aFindFrm.pNewFrmFmt->ResetFmtAttr(  RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
            aFrmSz.SetWidth( nSize );
            aFindFrm.pNewFrmFmt->SetFmtAttr( aFrmSz );
            pCpyPara->rTabFrmArr.Insert( aFindFrm );
        }

        SwTableBox* pBox;
        if( pFndBox->GetLines().Count() )
        {
            pBox = new SwTableBox( aFindFrm.pNewFrmFmt,
                        pFndBox->GetLines().Count(), pCpyPara->pInsLine );
            pCpyPara->pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, pCpyPara->nInsPos++ );
            _CpyPara aPara( *pCpyPara, pBox );
            aPara.nNewSize = nSize;     // get the size
            pFndBox->GetLines().ForEach( &lcl_CopyLineToDoc, &aPara );
        }
        else
        {
            // Create an empty Box
            pCpyPara->pDoc->GetNodes().InsBoxen( pCpyPara->pTblNd, pCpyPara->pInsLine,
                            aFindFrm.pNewFrmFmt,
                            (SwTxtFmtColl*)pCpyPara->pDoc->GetDfltTxtFmtColl(),
                            0, pCpyPara->nInsPos );
            pBox = pCpyPara->pInsLine->GetTabBoxes()[ pCpyPara->nInsPos ];
            if( bDummy )
                pBox->setDummyFlag( true );
            else if( pCpyPara->bCpyCntnt )
            {
                // Copy the content into this empty Box
                pBox->setRowSpan( pFndBox->GetBox()->getRowSpan() );

                // We can also copy formulas and values, if we copy the content
                {
                    SfxItemSet aBoxAttrSet( pCpyPara->pDoc->GetAttrPool(),
                                            RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
                    aBoxAttrSet.Put( pFndBox->GetBox()->GetFrmFmt()->GetAttrSet() );
                    if( aBoxAttrSet.Count() )
                    {
                        const SfxPoolItem* pItem;
                        SvNumberFormatter* pN = pCpyPara->pDoc->GetNumberFormatter( sal_False );
                        if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == aBoxAttrSet.
                            GetItemState( RES_BOXATR_FORMAT, sal_False, &pItem ) )
                        {
                            sal_uLong nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
                            sal_uLong nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                            if( nNewIdx != nOldIdx )
                                aBoxAttrSet.Put( SwTblBoxNumFormat( nNewIdx ));
                        }
                        pBox->ClaimFrmFmt()->SetFmtAttr( aBoxAttrSet );
                    }
                }
                SwDoc* pFromDoc = pFndBox->GetBox()->GetFrmFmt()->GetDoc();
                SwNodeRange aCpyRg( *pFndBox->GetBox()->GetSttNd(), 1,
                            *pFndBox->GetBox()->GetSttNd()->EndOfSectionNode() );
                SwNodeIndex aInsIdx( *pBox->GetSttNd(), 1 );

                pFromDoc->CopyWithFlyInFly( aCpyRg, 0, aInsIdx, sal_False );
                // Delete the initial TextNode
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
    return sal_True;
}

sal_Bool lcl_CopyLineToDoc( const _FndLine*& rpFndLine, void* pPara )
{
    _CpyPara* pCpyPara = (_CpyPara*)pPara;

    // Find the Frame Format in the list of all Frame Formats
    _CpyTabFrm aFindFrm( (SwTableBoxFmt*)rpFndLine->GetLine()->GetFrmFmt() );
    sal_uInt16 nFndPos;
    if( !pCpyPara->rTabFrmArr.Seek_Entry( aFindFrm, &nFndPos ))
    {
        // It doesn't exist yet, so copy it
        aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pCpyPara->pDoc->MakeTableLineFmt();
        aFindFrm.pNewFrmFmt->CopyAttrs( *rpFndLine->GetLine()->GetFrmFmt() );
        pCpyPara->rTabFrmArr.Insert( aFindFrm );
    }
    else
        aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ];

    SwTableLine* pNewLine = new SwTableLine( (SwTableLineFmt*)aFindFrm.pNewFrmFmt,
                        rpFndLine->GetBoxes().size(), pCpyPara->pInsBox );
    if( pCpyPara->pInsBox )
    {
        pCpyPara->pInsBox->GetTabLines().C40_INSERT( SwTableLine, pNewLine, pCpyPara->nInsPos++ );
    }
    else
    {
        pCpyPara->pTblNd->GetTable().GetTabLines().C40_INSERT( SwTableLine, pNewLine,
                            pCpyPara->nInsPos++ );
    }

    _CpyPara aPara( *pCpyPara, pNewLine );

    if( pCpyPara->pTblNd->GetTable().IsNewModel() )
    {
        aPara.nOldSize = 0; // will not be used
        aPara.nBoxIdx = 1;
    }
    else if( rpFndLine->GetBoxes().size() ==
                    rpFndLine->GetLine()->GetTabBoxes().Count() )
    {
        // Get the Parent's size
        const SwFrmFmt* pFmt;

        if( rpFndLine->GetLine()->GetUpper() )
            pFmt = rpFndLine->GetLine()->GetUpper()->GetFrmFmt();
        else
            pFmt = pCpyPara->pTblNd->GetTable().GetFrmFmt();
        aPara.nOldSize = pFmt->GetFrmSize().GetWidth();
    }
    else
        // Calculate it
        for( sal_uInt16 n = 0; n < rpFndLine->GetBoxes().size(); ++n )
            aPara.nOldSize += rpFndLine->GetBoxes()[n]
                        ->GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();

    _FndBoxes& rBoxes = ((_FndLine*)rpFndLine)->GetBoxes();
    for (_FndBoxes::const_iterator it = rBoxes.begin(); it != rBoxes.end(); ++it)
        lcl_CopyBoxToDoc(*it, &aPara);
    if( pCpyPara->pTblNd->GetTable().IsNewModel() )
        ++pCpyPara->nLnIdx;
    return sal_True;
}

sal_Bool SwTable::CopyHeadlineIntoTable( SwTableNode& rTblNd )
{
    // Find all Boxes/Lines
    SwSelBoxes aSelBoxes;
    SwTableBox* pBox = GetTabSortBoxes()[ 0 ];
    pBox = GetTblBox( pBox->GetSttNd()->StartOfSectionNode()->GetIndex() + 1 );
    SelLineFromBox( pBox, aSelBoxes, sal_True );

    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( aSelBoxes, &aFndBox );
        ((SwTableLines&)GetTabLines()).ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return sal_False;

    {
        // Convert Table formulas to their relative representation
        SwTableFmlUpdate aMsgHnt( this );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        GetFrmFmt()->GetDoc()->UpdateTblFlds( &aMsgHnt );
    }

    _CpyTabFrms aCpyFmt;
    _CpyPara aPara( &rTblNd, 1, aCpyFmt, sal_True );
    aPara.nNewSize = aPara.nOldSize = rTblNd.GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
    // Copy
    if( IsNewModel() )
        lcl_CalcNewWidths( aFndBox.GetLines(), aPara );
    aFndBox.GetLines().ForEach( &lcl_CopyLineToDoc, &aPara );
    if( rTblNd.GetTable().IsNewModel() )
    {   // The copied line must not contain any row span attributes > 1
        SwTableLine* pLine = rTblNd.GetTable().GetTabLines()[0];
        sal_uInt16 nColCount = pLine->GetTabBoxes().Count();
        OSL_ENSURE( nColCount, "Empty Table Line" );
        for( sal_uInt16 nCurrCol = 0; nCurrCol < nColCount; ++nCurrCol )
        {
            SwTableBox* pTableBox = pLine->GetTabBoxes()[nCurrCol];
            OSL_ENSURE( pTableBox, "Missing Table Box" );
            pTableBox->setRowSpan( 1 );
        }
    }

    return sal_True;
}

sal_Bool SwTable::MakeCopy( SwDoc* pInsDoc, const SwPosition& rPos,
                        const SwSelBoxes& rSelBoxes, sal_Bool bCpyNds,
                        sal_Bool bCpyName ) const
{
    // Find all Boxes/Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rSelBoxes, &aFndBox );
        ((SwTableLines&)GetTabLines()).ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return sal_False;

    // First copy the PoolTemplates for the Table, so that the Tables are
    // actually copied and have valid values.
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
        return sal_False;

    SwNodeIndex aIdx( rPos.nNode, -1 );
    SwTableNode* pTblNd = aIdx.GetNode().FindTableNode();
    aIdx++;
    OSL_ENSURE( pTblNd, "Where is the TableNode now?" );

    pTblNd->GetTable().SetRowsToRepeat( GetRowsToRepeat() );

    if( IS_TYPE( SwDDETable, this ))
    {
        // A DDE-Table is being copied
        // Does the new Document actually have it's FieldType?
        SwFieldType* pFldType = pInsDoc->InsertFldType(
                                    *((SwDDETable*)this)->GetDDEFldType() );
        OSL_ENSURE( pFldType, "unknown FieldType" );

        // Change the Table Pointer at the Node
        pNewTbl = new SwDDETable( *pNewTbl,
                                 (SwDDEFieldType*)pFldType );
        pTblNd->SetNewTable( pNewTbl, sal_False );
    }

    pNewTbl->GetFrmFmt()->CopyAttrs( *GetFrmFmt() );
    pNewTbl->SetTblChgMode( GetTblChgMode() );

    // Destroy the already created Frames
    pTblNd->DelFrms();

    {
        // Conver the Table formulas to their relative representation
        SwTableFmlUpdate aMsgHnt( this );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pSrcDoc->UpdateTblFlds( &aMsgHnt );
    }

    SwTblNumFmtMerge aTNFM( *pSrcDoc, *pInsDoc );

    // Also copy Names or enforce a new unique one
    if( bCpyName )
        pNewTbl->GetFrmFmt()->SetName( GetFrmFmt()->GetName() );

    _CpyTabFrms aCpyFmt;
    _CpyPara aPara( pTblNd, 1, aCpyFmt, bCpyNds );
    aPara.nNewSize = aPara.nOldSize = GetFrmFmt()->GetFrmSize().GetWidth();

    if( IsNewModel() )
        lcl_CalcNewWidths( aFndBox.GetLines(), aPara );
    // Copy
    aFndBox.GetLines().ForEach( &lcl_CopyLineToDoc, &aPara );

    // Set the "right" margin above/below
    {
        _FndLine* pFndLn = aFndBox.GetLines()[ 0 ];
        SwTableLine* pLn = pFndLn->GetLine();
        const SwTableLine* pTmp = pLn;
        sal_uInt16 nLnPos = GetTabLines().GetPos( pTmp );
        if( USHRT_MAX != nLnPos && nLnPos )
        {
            // There is a Line before it
            SwCollectTblLineBoxes aLnPara( sal_False, HEADLINE_BORDERCOPY );

            pLn = GetTabLines()[ nLnPos - 1 ];
            pLn->GetTabBoxes().ForEach( &lcl_Box_CollectBox, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( sal_True );
                pLn = pNewTbl->GetTabLines()[ 0 ];
                pLn->GetTabBoxes().ForEach( &lcl_BoxSetSplitBoxFmts, &aLnPara );
            }
        }

        pFndLn = aFndBox.GetLines()[ aFndBox.GetLines().Count() -1 ];
        pLn = pFndLn->GetLine();
        pTmp = pLn;
        nLnPos = GetTabLines().GetPos( pTmp );
        if( nLnPos < GetTabLines().Count() - 1 )
        {
            // There is a Line following it
            SwCollectTblLineBoxes aLnPara( sal_True, HEADLINE_BORDERCOPY );

            pLn = GetTabLines()[ nLnPos + 1 ];
            pLn->GetTabBoxes().ForEach( &lcl_Box_CollectBox, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( sal_False );
                pLn = pNewTbl->GetTabLines()[ pNewTbl->GetTabLines().Count()-1 ];
                pLn->GetTabBoxes().ForEach( &lcl_BoxSetSplitBoxFmts, &aLnPara );
            }
        }
    }

    // We need to delete the initial Box
    _DeleteBox( *pNewTbl, pNewTbl->GetTabLines()[
                pNewTbl->GetTabLines().Count() - 1 ]->GetTabBoxes()[0],
                0, sal_False, sal_False );

    if( pNewTbl->IsNewModel() )
        lcl_CheckRowSpan( *pNewTbl );
    // Clean up
    pNewTbl->GCLines();

    pTblNd->MakeFrms( &aIdx );  // re-generate the Frames

    CHECKTABLELAYOUT

    return sal_True;
}

// Find the next Box with content from this Line
SwTableBox* SwTableLine::FindNextBox( const SwTable& rTbl,
                     const SwTableBox* pSrchBox, sal_Bool bOvrTblLns ) const
{
    const SwTableLine* pLine = this;            // for M800
    SwTableBox* pBox;
    sal_uInt16 nFndPos;
    if( GetTabBoxes().Count() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetTabBoxes().GetPos( pSrchBox )) &&
        nFndPos + 1 != GetTabBoxes().Count() )
    {
        pBox = GetTabBoxes()[ nFndPos + 1 ];
        while( pBox->GetTabLines().Count() )
            pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];
        return pBox;
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line is not in the Table" );
        // Is there another Line?
        if( nFndPos+1 >= GetUpper()->GetTabLines().Count() )
            return GetUpper()->GetUpper()->FindNextBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos+1];
    }
    else if( bOvrTblLns )       // Over a Table's the "BaseLines"??
    {
        // Search for the next Line in the Table
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( nFndPos + 1 >= rTbl.GetTabLines().Count() )
            return 0;           // there are no more Boxes

        pLine = rTbl.GetTabLines()[ nFndPos+1 ];
    }
    else
        return 0;

    if( pLine->GetTabBoxes().Count() )
    {
        pBox = pLine->GetTabBoxes()[0];
        while( pBox->GetTabLines().Count() )
            pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];
        return pBox;
    }
    return pLine->FindNextBox( rTbl, 0, bOvrTblLns );
}

// Find the previous Box from this Line
SwTableBox* SwTableLine::FindPreviousBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, sal_Bool bOvrTblLns ) const
{
    const SwTableLine* pLine = this;            // for M800
    SwTableBox* pBox;
    sal_uInt16 nFndPos;
    if( GetTabBoxes().Count() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetTabBoxes().GetPos( pSrchBox )) &&
        nFndPos )
    {
        pBox = GetTabBoxes()[ nFndPos - 1 ];
        while( pBox->GetTabLines().Count() )
        {
            pLine = pBox->GetTabLines()[pBox->GetTabLines().Count()-1];
            pBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
        }
        return pBox;
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line is not in the Table" );
        // Is there another Line?
        if( !nFndPos )
            return GetUpper()->GetUpper()->FindPreviousBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos-1];
    }
    else if( bOvrTblLns )       // Over a Table's the "BaseLines"??
    {
        // Search for the next Line in the Table
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( !nFndPos )
            return 0;           // there are no more Boxes

        pLine = rTbl.GetTabLines()[ nFndPos-1 ];
    }
    else
        return 0;

    if( pLine->GetTabBoxes().Count() )
    {
        pBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
        while( pBox->GetTabLines().Count() )
        {
            pLine = pBox->GetTabLines()[pBox->GetTabLines().Count()-1];
            pBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
        }
        return pBox;
    }
    return pLine->FindPreviousBox( rTbl, 0, bOvrTblLns );
}

// Find the next Box with content from this Line
SwTableBox* SwTableBox::FindNextBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, sal_Bool bOvrTblLns ) const
{
    if( !pSrchBox  && !GetTabLines().Count() )
        return (SwTableBox*)this;
    return GetUpper()->FindNextBox( rTbl, pSrchBox ? pSrchBox : this,
                                        bOvrTblLns );

}

// Find the next Box with content from this Line
SwTableBox* SwTableBox::FindPreviousBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, sal_Bool bOvrTblLns ) const
{
    if( !pSrchBox && !GetTabLines().Count() )
        return (SwTableBox*)this;
    return GetUpper()->FindPreviousBox( rTbl, pSrchBox ? pSrchBox : this,
                                        bOvrTblLns );
}

sal_Bool lcl_BoxSetHeadCondColl( const SwTableBox*& rpBox, void* )
{
    // We need to adapt the paragraphs with conditional templates in the HeadLine
    const SwStartNode* pSttNd = rpBox->GetSttNd();
    if( pSttNd )
        pSttNd->CheckSectionCondColl();
    else
        ((SwTableBox*)rpBox)->GetTabLines().ForEach( &lcl_LineSetHeadCondColl, 0 );
    return sal_True;
}

sal_Bool lcl_LineSetHeadCondColl( const SwTableLine*& rpLine, void* )
{
    ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_BoxSetHeadCondColl, 0 );
    return sal_True;
}

SwTwips lcl_GetDistance( SwTableBox* pBox, sal_Bool bLeft )
{
    sal_Bool bFirst = sal_True;
    SwTwips nRet = 0;
    SwTableLine* pLine;
    while( pBox && 0 != ( pLine = pBox->GetUpper() ) )
    {
        sal_uInt16 nStt = 0, nPos = pLine->GetTabBoxes().C40_GETPOS( SwTableBox, pBox );

        if( bFirst && !bLeft )
            ++nPos;
        bFirst = sal_False;

        while( nStt < nPos )
            nRet += pLine->GetTabBoxes()[ nStt++ ]->GetFrmFmt()
                            ->GetFrmSize().GetWidth();
        pBox = pLine->GetUpper();
    }
    return nRet;
}

sal_Bool lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, sal_Bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();
        sal_Bool bGreaterBox = sal_False;

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_SetSelBoxWidth( pBox->GetTabLines()[ i ], rParam,
                                            nDist, sal_True ))
                    return sal_False;

            // Collect all "ContentBoxes"
            if( ( 0 != ( bGreaterBox = TBLFIX_CHGABS != rParam.nMode && ( nDist + ( rParam.bLeft ? 0 : nWidth ) ) >= rParam.nSide)) ||
                ( !rParam.bBigger && ( Abs( nDist + (( rParam.nMode && rParam.bLeft ) ? 0 : nWidth ) - rParam.nSide ) < COLFUZZY ) ) )
            {
                rParam.bAnyBoxFnd = sal_True;
                SwTwips nLowerDiff;
                if( bGreaterBox && TBLFIX_CHGPROP == rParam.nMode )
                {
                    // The "other Boxes" have been adapted, so change by this value
                    nLowerDiff = (nDist + ( rParam.bLeft ? 0 : nWidth ) ) - rParam.nSide;
                    nLowerDiff *= rParam.nDiff;
                    nLowerDiff /= rParam.nMaxSize;
                    nLowerDiff = rParam.nDiff - nLowerDiff;
                }
                else
                    nLowerDiff = rParam.nDiff;

                if( nWidth < nLowerDiff || nWidth - nLowerDiff < MINLAY )
                    return sal_False;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_SetSelBoxWidth( pBox->GetTabLines()[ i ], rParam, nDist, sal_False );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;


            if( nLowerDiff ||
                 ( 0 != ( bGreaterBox = !nOldLower && TBLFIX_CHGABS != rParam.nMode &&
                    ( nDist + ( rParam.bLeft ? 0 : nWidth ) ) >= rParam.nSide)) ||
                ( Abs( nDist + ( (rParam.nMode && rParam.bLeft) ? 0 : nWidth )
                            - rParam.nSide ) < COLFUZZY ))
            {
                // This column contains the Cursor - so decrease/increase
                SwFmtFrmSize aNew( rSz );

                if( !nLowerDiff )
                {
                    if( bGreaterBox && TBLFIX_CHGPROP == rParam.nMode )
                    {
                        // The "other Boxes" have been adapted, so change by this value
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

        // If it gets bigger, then that's it
        if( ( TBLFIX_CHGABS == rParam.nMode || !rParam.bLeft ) &&
                nDist >= rParam.nSide )
            break;
    }
    return sal_True;
}

sal_Bool lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, sal_Bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_SetOtherBoxWidth( pBox->GetTabLines()[ i ],
                                                    rParam, nDist, sal_True ))
                    return sal_False;

            if( rParam.bBigger && ( TBLFIX_CHGABS == rParam.nMode
                    ? Abs( nDist - rParam.nSide ) < COLFUZZY
                    : ( rParam.bLeft ? nDist < rParam.nSide - COLFUZZY
                                     : nDist >= rParam.nSide - COLFUZZY )) )
            {
                rParam.bAnyBoxFnd = sal_True;
                SwTwips nDiff;
                if( TBLFIX_CHGPROP == rParam.nMode )        // Table fixed, proportional
                {
                    // calculate relative
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nMaxSize;
                }
                else
                    nDiff = rParam.nDiff;

                if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                    return sal_False;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_SetOtherBoxWidth( pBox->GetTabLines()[ i ], rParam,
                                            nDist, sal_False );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                ( TBLFIX_CHGABS == rParam.nMode
                        ? Abs( nDist - rParam.nSide ) < COLFUZZY
                        : ( rParam.bLeft ? nDist < rParam.nSide - COLFUZZY
                                         : nDist >= rParam.nSide - COLFUZZY)
                 ) )
            {
                SwFmtFrmSize aNew( rSz );

                if( !nLowerDiff )
                {
                    if( TBLFIX_CHGPROP == rParam.nMode )        // Table fixed, proportional
                    {
                        // calculate relative
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
    return sal_True;
}

sal_Bool lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                            SwTwips nDist, sal_Bool bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    sal_uInt16 n, nCmp;
    for( n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwTableBoxFmt* pFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_InsSelBox( pBox->GetTabLines()[ i ], rParam,
                                            nDist, sal_True ))
                    return sal_False;

            // Collect all "ContentBoxes"
            if( Abs( nDist + ( rParam.bLeft ? 0 : nWidth )
                    - rParam.nSide ) < COLFUZZY )
                nCmp = 1;
            else if( nDist + ( rParam.bLeft ? 0 : nWidth/2 ) > rParam.nSide )
                nCmp = 2;
            else
                nCmp = 0;

            if( nCmp )
            {
                rParam.bAnyBoxFnd = sal_True;
                if( pFmt->GetProtect().IsCntntProtected() )
                    return sal_False;

                if( rParam.bSplittBox &&
                    nWidth - rParam.nDiff <= COLFUZZY +
                        ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                    return sal_False;

                if( pBox->GetSttNd() )
                    rParam.aBoxes.insert( pBox );

                break;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_InsSelBox( pBox->GetTabLines()[ i ], rParam, nDist, sal_False );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff )
                nCmp = 1;
            else if( Abs( nDist + ( rParam.bLeft ? 0 : nWidth )
                                - rParam.nSide ) < COLFUZZY )
                nCmp = 2;
            else if( nDist + nWidth / 2 > rParam.nSide )
                nCmp = 3;
            else
                nCmp = 0;

            if( nCmp )
            {
                // This column contains the Cursor - so decrease/increase
                if( 1 == nCmp )
                {
                    if( !rParam.bSplittBox )
                    {
                        // the current Box on
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

                    // Special case: There is no space in the other Boxes, but in the Cell
                    if( rParam.bSplittBox )
                    {
                        // the current Box on
                        SwFmtFrmSize aNewSize( rSz );
                        aNewSize.SetWidth( nWidth - rParam.nDiff );
                        rParam.aShareFmts.SetSize( *pBox, aNewSize );
                    }

                    // Special treatment for the Border
                    // The right one needs to be removed
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
    return sal_True;
}

sal_Bool lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, sal_Bool bCheck )
{
    // Special case: There is no space in the other Boxes, but in the cell
    if( rParam.bSplittBox )
        return sal_True;

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    sal_uInt16 n;

    // Table fixed, proportional
    if( !rParam.nRemainWidth && TBLFIX_CHGPROP == rParam.nMode )
    {
        // Find the right width to which the relative width adjustment
        // corresponds to
        SwTwips nTmpDist = nDist;
        for( n = 0; n < rBoxes.Count(); ++n )
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

    for( n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_InsOtherBox( pBox->GetTabLines()[ i ],
                                                    rParam, nDist, sal_True ))
                    return sal_False;

            if(
                rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TBLFIX_CHGABS != rParam.nMode ||
                                (n < rBoxes.Count() &&
                                  (nDist + nWidth + rBoxes[ n+1 ]->
                                   GetFrmFmt()->GetFrmSize().GetWidth() / 2)
                                  > rParam.nSide) ))
                             : (nDist + nWidth / 2 ) > rParam.nSide
                )
            {
                rParam.bAnyBoxFnd = sal_True;
                SwTwips nDiff;
                if( TBLFIX_CHGPROP == rParam.nMode )        // Table fixed, proportional
                {
                    // relativ berechnen
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nRemainWidth;

                    if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                        return sal_False;
                }
                else
                {
                    nDiff = rParam.nDiff;

                    // See if the left or right Box is big enough to give up space.
                    // We're inserting a Box before or after.
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
//                      if( nBoxPos )
                            nTmpWidth = pTmpBox->GetFrmFmt()->GetFrmSize().GetWidth();
//                      else
//                          nTmpWidth = 0;
                    }

                    if( nTmpWidth < nDiff || nTmpWidth - nDiff < MINLAY )
                        return sal_False;
                    break;
                }
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_InsOtherBox( pBox->GetTabLines()[ i ], rParam,
                                        nDist, sal_False );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                (rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TBLFIX_CHGABS != rParam.nMode ||
                                (n < rBoxes.Count() &&
                                  (nDist + nWidth + rBoxes[ n+1 ]->
                                   GetFrmFmt()->GetFrmSize().GetWidth() / 2)
                                  > rParam.nSide) ))
                              : (nDist + nWidth / 2 ) > rParam.nSide ))
            {
                if( !nLowerDiff )
                {
                    if( TBLFIX_CHGPROP == rParam.nMode )        // Table fixed, proportional
                    {
                        // Calculate relatively
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
    return sal_True;
}

// The position comparison's result
//  POS_BEFORE,             // Box comes before
//  POS_BEHIND,             // Box comes after
//  POS_INSIDE,             // Box is completely wthin start/end
//  POS_OUTSIDE,            // Box overlaps start/end completely
//  POS_EQUAL,              // Box and start/end are the same
//  POS_OVERLAP_BEFORE,     // Box overlapps the start
//  POS_OVERLAP_BEHIND      // Box overlapps the end
SwComparePosition _CheckBoxInRange( sal_uInt16 nStt, sal_uInt16 nEnd,
                                    sal_uInt16 nBoxStt, sal_uInt16 nBoxEnd )
{
    // Still treat COLFUZZY!
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
            if( COLFUZZY > Abs( long(nEnd) - long(nBoxEnd) ) &&
                COLFUZZY > Abs( long(nStt) - long(nBoxStt) ) )
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

void lcl_DelSelBox_CorrLowers( SwTableLine& rLine, CR_SetBoxWidth& rParam,
                                SwTwips nWidth )
{
    // 1. step: Calculate own width
    SwTableBoxes& rBoxes = rLine.GetTabBoxes();
    SwTwips nBoxWidth = 0;
    sal_uInt16 n;

    for( n = rBoxes.Count(); n; )
        nBoxWidth += rBoxes[ --n ]->GetFrmFmt()->GetFrmSize().GetWidth();

    if( COLFUZZY < Abs( nWidth - nBoxWidth ))
    {
        // Thus, they need to be adjusted
        for( n = rBoxes.Count(); n; )
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
                // Has Lower itself, so also adjust that
                for( sal_uInt16 i = pBox->GetTabLines().Count(); i; )
                    ::lcl_DelSelBox_CorrLowers( *pBox->GetTabLines()[ --i ],
                                                rParam, nDiff  );
            }
        }
    }
}

void lcl_ChgBoxSize( SwTableBox& rBox, CR_SetBoxWidth& rParam,
                    const SwFmtFrmSize& rOldSz,
                    sal_uInt16& rDelWidth, SwTwips nDist )
{
    long nDiff = 0;
    sal_Bool bSetSize = sal_False;

    switch( rParam.nMode )
    {
    case TBLFIX_CHGABS:     // Fixed width table, change neighbor
        nDiff = rDelWidth + rParam.nLowerDiff;
        bSetSize = sal_True;
        break;

    case TBLFIX_CHGPROP:    // Fixed width table, change all neighbors
        if( !rParam.nRemainWidth )
        {
            // Calculate
            if( rParam.bLeft )
                rParam.nRemainWidth = sal_uInt16(nDist);
            else
                rParam.nRemainWidth = sal_uInt16(rParam.nTblWidth - nDist);
        }

        // Calculate relatively
        nDiff = rOldSz.GetWidth();
        nDiff *= rDelWidth + rParam.nLowerDiff;
        nDiff /= rParam.nRemainWidth;

        bSetSize = sal_True;
        break;

    case TBLVAR_CHGABS:     // Variable table, change all neighbors
        if( COLFUZZY < Abs( rParam.nBoxWidth -
                            ( rDelWidth + rParam.nLowerDiff )))
        {
            nDiff = rDelWidth + rParam.nLowerDiff - rParam.nBoxWidth;
            if( 0 < nDiff )
                rDelWidth = rDelWidth - sal_uInt16(nDiff);
            else
                rDelWidth = rDelWidth + sal_uInt16(-nDiff);
            bSetSize = sal_True;
        }
        break;
    }

    if( bSetSize )
    {
        SwFmtFrmSize aNew( rOldSz );
        aNew.SetWidth( aNew.GetWidth() + nDiff );
        rParam.aShareFmts.SetSize( rBox, aNew );

        // Change the Lower once again
        for( sal_uInt16 i = rBox.GetTabLines().Count(); i; )
            ::lcl_DelSelBox_CorrLowers( *rBox.GetTabLines()[ --i ], rParam,
                                            aNew.GetWidth() );
    }
}

sal_Bool lcl_DeleteBox_Rekursiv( CR_SetBoxWidth& rParam, SwTableBox& rBox,
                            sal_Bool bCheck )
{
    sal_Bool bRet = sal_True;
    if( rBox.GetSttNd() )
    {
        if( bCheck )
        {
            rParam.bAnyBoxFnd = sal_True;
            if( rBox.GetFrmFmt()->GetProtect().IsCntntProtected() )
                bRet = sal_False;
            else
            {
                SwTableBox* pBox = &rBox;
                rParam.aBoxes.insert( pBox );
            }
        }
        else
            ::_DeleteBox( rParam.pTblNd->GetTable(), &rBox,
                            rParam.pUndo, sal_False, sal_True, &rParam.aShareFmts );
    }
    else
    {
        // We need to delete these sequentially via the ContentBoxes
        for( sal_uInt16 i = rBox.GetTabLines().Count(); i; )
        {
            SwTableLine& rLine = *rBox.GetTabLines()[ --i ];
            for( sal_uInt16 n = rLine.GetTabBoxes().Count(); n; )
                if( !::lcl_DeleteBox_Rekursiv( rParam,
                                *rLine.GetTabBoxes()[ --n ], bCheck ))
                    return sal_False;
        }
    }
    return bRet;
}

sal_Bool lcl_DelSelBox( SwTableLine* pTabLine, CR_SetBoxWidth& rParam,
                    SwTwips nDist, sal_Bool bCheck )
{
    SwTableBoxes& rBoxes = pTabLine->GetTabBoxes();
    sal_uInt16 n, nCntEnd, nBoxChkStt, nBoxChkEnd, nDelWidth = 0;
    if( rParam.bLeft )
    {
        n = rBoxes.Count();
        nCntEnd = 0;
        nBoxChkStt = (sal_uInt16)rParam.nSide;
        nBoxChkEnd = static_cast<sal_uInt16>(rParam.nSide + rParam.nBoxWidth);
    }
    else
    {
        n = 0;
        nCntEnd = rBoxes.Count();
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
        sal_Bool bDelBox = sal_False, bChgLowers = sal_False;

        // Test the Box width and react accordingly
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
                    return sal_True;
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
                    return sal_True;
            }
            else if( !rParam.bLeft )
            {
                ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                if( TBLFIX_CHGABS == rParam.nMode )
                    n = nCntEnd;
            }
            break;

        case POS_OUTSIDE:           // Box fully overlaps start/end
        case POS_INSIDE:            // Box is completely within start/end
        case POS_EQUAL:             // Box and start/end are the same
            bDelBox = sal_True;
            break;

        case POS_OVERLAP_BEFORE:     // Box overlaps the start
            if( nBoxChkStt <= ( nDist + (rParam.bLeft ? - nWidth / 2
                                                      : nWidth / 2 )))
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = sal_True;
                else
                    bDelBox = sal_True;
            }
            else if( !bCheck && rParam.bLeft )
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = sal_True;
                else
                {
                    ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                    if( TBLFIX_CHGABS == rParam.nMode )
                        n = nCntEnd;
                }
            }
            break;

        case POS_OVERLAP_BEHIND:     // Box overlaps the end
            // JP 10.02.99:
            // Delete generally or (like in OVERLAP_BEFORE) only delete the one who reaches up to the half into the delete Box?
            if( !pBox->GetSttNd() )
                bChgLowers = sal_True;
            else
                bDelBox = sal_True;
            break;
        default: break;
        }

        if( bDelBox )
        {
            nDelWidth = nDelWidth + sal_uInt16(nWidth);
            if( bCheck )
            {
                // The last/first Box can only be deleted for the variable Table,
                // if it's as large as the change in the Table.
                if( (( TBLVAR_CHGABS != rParam.nMode ||
                        nDelWidth != rParam.nBoxWidth ) &&
                     COLFUZZY > Abs( rParam.bLeft
                                    ? nWidth - nDist
                                    : (nDist + nWidth - rParam.nTblWidth )))
                    || !::lcl_DeleteBox_Rekursiv( rParam, *pBox, bCheck ) )
                    return sal_False;

                if( pFmt->GetProtect().IsCntntProtected() )
                    return sal_False;
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
            sal_Bool bFirst = sal_True, bCorrLowers = sal_False;
            long nLowerDiff = 0;
            long nOldLower = rParam.nLowerDiff;
            sal_uInt16 nOldRemain = rParam.nRemainWidth;
            sal_uInt16 i;

            for( i = pBox->GetTabLines().Count(); i; )
            {
                rParam.nLowerDiff = nDelWidth + nOldLower;
                rParam.nRemainWidth = nOldRemain;
                SwTableLine* pLine = pBox->GetTabLines()[ --i ];
                if( !::lcl_DelSelBox( pLine, rParam, nDist, bCheck ))
                    return sal_False;

                // Do the Box and its Lines still exist?
                if( n < rBoxes.Count() &&
                    pBox == rBoxes[ rParam.bLeft ? n : n-1 ] &&
                    i < pBox->GetTabLines().Count() &&
                    pLine == pBox->GetTabLines()[ i ] )
                {
                    if( !bFirst && !bCorrLowers &&
                        COLFUZZY < Abs( nLowerDiff - rParam.nLowerDiff ) )
                        bCorrLowers = sal_True;

                    // The largest deletion width counts, but only if we don't
                    // delete the whole Line
                    if( nLowerDiff < rParam.nLowerDiff )
                        nLowerDiff = rParam.nLowerDiff;

                    bFirst = sal_False;
                }
            }
            rParam.nLowerDiff = nOldLower;
            rParam.nRemainWidth = nOldRemain;

            // Did we delete all Boxes? Then the deletion width = the Box width, of course
            if( !nLowerDiff )
                nLowerDiff = nWidth;

            // Adjust deletion width!
            nDelWidth = nDelWidth + sal_uInt16(nLowerDiff);

            if( !bCheck )
            {
                // Has the Box already been removed?
                if( n > rBoxes.Count() ||
                    pBox != rBoxes[ ( rParam.bLeft ? n : n-1 ) ] )
                {
                    // Then change the loop variable when deleting to the right
                    if( !rParam.bLeft )
                        --n, --nCntEnd;
                }
                else
                {
                    // Or else we need to adapt the Box's size
                    SwFmtFrmSize aNew( rSz );
                    sal_Bool bCorrRel = sal_False;

                    if( TBLVAR_CHGABS != rParam.nMode )
                    {
                        switch( ePosType )
                        {
                        case POS_OVERLAP_BEFORE:    // Box overlaps the start
                            if( TBLFIX_CHGPROP == rParam.nMode )
                                bCorrRel = rParam.bLeft;
                            else if( rParam.bLeft ) // TBLFIX_CHGABS
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = sal_True;
                                n = nCntEnd;
                            }
                            break;

                        case POS_OVERLAP_BEHIND:    // Box overlaps the end
                            if( TBLFIX_CHGPROP == rParam.nMode )
                                bCorrRel = !rParam.bLeft;
                            else if( !rParam.bLeft )    // TBLFIX_CHGABS
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = sal_True;
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
                            // Calculate
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
                        // Adapt the Lower once again
                        for( i = pBox->GetTabLines().Count(); i; )
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
    return sal_True;
}

// Dummy function for the method SetColWidth
sal_Bool lcl_DelOtherBox( SwTableLine* , CR_SetBoxWidth& , SwTwips , sal_Bool )
{
    return sal_True;
}

void lcl_AjustLines( SwTableLine* pLine, CR_SetBoxWidth& rParam )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];

        SwFmtFrmSize aSz( pBox->GetFrmFmt()->GetFrmSize() );
        SwTwips nWidth = aSz.GetWidth();
        nWidth *= rParam.nDiff;
        nWidth /= rParam.nMaxSize;
        aSz.SetWidth( nWidth );
        rParam.aShareFmts.SetSize( *pBox, aSz );

        for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
            ::lcl_AjustLines( pBox->GetTabLines()[ i ], rParam );
    }
}

#ifdef DBG_UTIL
void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize )
{
    const SwTableBoxes& rBoxes = rLine.GetTabBoxes();

    SwTwips nAktSize = 0;
    // See if the tables have a correct width
    for( sal_uInt16 n = 0; n < rBoxes.Count(); ++n  )
    {
        const SwTableBox* pBox = rBoxes[ n ];
        const SwTwips nBoxW = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        nAktSize += nBoxW;

        for( sal_uInt16 i = 0; i < pBox->GetTabLines().Count(); ++i )
            _CheckBoxWidth( *pBox->GetTabLines()[ i ], nBoxW );
    }

    if( Abs( nAktSize - nSize ) > ( COLFUZZY * rBoxes.Count() ) )
    {
        OSL_FAIL( "Line's Boxes are too small or too large" );
    }
}
#endif

_FndBox* lcl_SaveInsDelData( CR_SetBoxWidth& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst, SwTwips nDistStt )
{
    // Find all Boxes/Lines
    SwTable& rTbl = rParam.pTblNd->GetTable();

    if( rParam.aBoxes.empty() )
    {
        // Get the Boxes
        if( rParam.bBigger )
            for( sal_uInt16 n = 0; n < rTbl.GetTabLines().Count(); ++n )
                ::lcl_DelSelBox( rTbl.GetTabLines()[ n ], rParam, nDistStt, sal_True );
        else
            for( sal_uInt16 n = 0; n < rTbl.GetTabLines().Count(); ++n )
                ::lcl_InsSelBox( rTbl.GetTabLines()[ n ], rParam, nDistStt, sal_True );
    }

    // Prevent deleting the whole Table
    if( rParam.bBigger && rParam.aBoxes.size() == rTbl.GetTabSortBoxes().Count() )
        return 0;

    _FndBox* pFndBox = new _FndBox( 0, 0 );
    if( rParam.bBigger )
        pFndBox->SetTableLines( rParam.aBoxes, rTbl );
    else
    {
        _FndPara aPara( rParam.aBoxes, pFndBox );
        rTbl.GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
        OSL_ENSURE( pFndBox->GetLines().Count(), "Where are the Boxes" );
        pFndBox->SetTableLines( rTbl );

        if( ppUndo )
            rTmpLst.Insert( &rTbl.GetTabSortBoxes(), 0, rTbl.GetTabSortBoxes().Count() );
    }

    // Find Lines for the Layout update
    pFndBox->DelFrms( rTbl );

    // TL_CHART2: this function gest called from SetColWidth exclusively,
    // thus it is currently speculated that nothing needs to be done here.
    // Note: that SetColWidth is currently not completely understood though :-(

    return pFndBox;
}

sal_Bool SwTable::SetColWidth( SwTableBox& rAktBox, sal_uInt16 eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo )
{
    SetHTMLTableLayout( 0 );    // Delete HTML Layout

    const SwFmtFrmSize& rSz = GetFrmFmt()->GetFrmSize();
    const SvxLRSpaceItem& rLR = GetFrmFmt()->GetLRSpace();

    _FndBox* pFndBox = 0;                // for insertion/deletion
    SwTableSortBoxes aTmpLst( 0 );       // for Undo
    sal_Bool bBigger,
        bRet = sal_False,
        bLeft = nsTblChgWidthHeightType::WH_COL_LEFT == ( eType & 0xff ) ||
                nsTblChgWidthHeightType::WH_CELL_LEFT == ( eType & 0xff ),
        bInsDel = 0 != (eType & nsTblChgWidthHeightType::WH_FLAG_INSDEL );
    sal_uInt16 n;
    sal_uLong nBoxIdx = rAktBox.GetSttIdx();

    // Get the current Box's edge
    // Only needed for manipulating the width
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

            // First test if we have room at all
            sal_Bool bChgLRSpace = sal_True;
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
                    // Is the room on the other side?
                    ( bLeft ? rLR.GetRight() >= nAbsDiff
                            : rLR.GetLeft() >= nAbsDiff ))
                {
                    bRet = sal_True; bLeft = !bLeft;
                }

                if( !bRet )
                {
                    // Then call itself recursively; only with another mode (proportional)
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
                bRet = sal_True;
                for( n = 0; n < aLines.Count(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, sal_True ))
                    {
                        bRet = sal_False;
                        break;
                    }
                }
            }

            if( bRet )
            {
                if( bInsDel )
                {
                    pFndBox = ::lcl_SaveInsDelData( aParam, ppUndo,
                                                    aTmpLst, nDistStt );
                    if( aParam.bBigger && aParam.aBoxes.size() == aSortCntBoxes.Count() )
                    {
                        // This whole Table is to be deleted!
                        GetFrmFmt()->GetDoc()->DeleteRowCol( aParam.aBoxes );
                        return sal_False;
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
                    // If the Table does not have any room to grow, we need to create some!
                    if( aSz.GetWidth() + nRelDiff > USHRT_MAX )
                    {
                        // Break down to USHRT_MAX / 2
                        CR_SetBoxWidth aTmpPara( 0, aSz.GetWidth() / 2,
                                        0, aSz.GetWidth(), aSz.GetWidth(), aParam.pTblNd );
                        for( sal_uInt16 nLn = 0; nLn < aLines.Count(); ++nLn )
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

                    // If the Table happens to contain relative values (USHORT_MAX),
                    // we need to convert them to absolute ones now.
                    // Bug 61494
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

                for( n = aLines.Count(); n; )
                {
                    --n;
                    aParam.LoopClear();
                    (*fnSelBox)( aLines[ n ], aParam, nDistStt, sal_False );
                }

                // If the Table happens to contain relative values (USHORT_MAX),
                // we need to convert them to absolute ones now.
                // Bug 61494
                if( LONG_MAX != nFrmWidth )
                {
                    SwFmtFrmSize aAbsSz( aSz );
                    aAbsSz.SetWidth( nFrmWidth );
                    GetFrmFmt()->SetFmtAttr( aAbsSz );
                }
            }
        }
        else if( bInsDel ||
                ( bLeft ? nDist : Abs( rSz.GetWidth() - nDist ) > COLFUZZY ) )
        {
            bRet = sal_True;
            if( bLeft && TBLFIX_CHGABS == eTblChgMode && !bInsDel )
                aParam.bBigger = !bBigger;

            // First test if we have room at all
            if( bInsDel )
            {
                if( aParam.bBigger )
                {
                    for( n = 0; n < aLines.Count(); ++n )
                    {
                        aParam.LoopClear();
                        if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, sal_True ))
                        {
                            bRet = sal_False;
                            break;
                        }
                    }
                }
                else
                {
                    if( 0 != ( bRet = bLeft ? nDist != 0
                                            : ( rSz.GetWidth() - nDist ) > COLFUZZY ) )
                    {
                        for( n = 0; n < aLines.Count(); ++n )
                        {
                            aParam.LoopClear();
                            if( !(*fnOtherBox)( aLines[ n ], aParam, 0, sal_True ))
                            {
                                bRet = sal_False;
                                break;
                            }
                        }
                        if( bRet && !aParam.bAnyBoxFnd )
                            bRet = sal_False;
                    }

                    if( !bRet && rAktBox.GetFrmFmt()->GetFrmSize().GetWidth()
                        - nRelDiff > COLFUZZY +
                            ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                    {
                        // Consume the space from the current Cell
                        aParam.bSplittBox = sal_True;
                        // We also need to test this!
                        bRet = sal_True;

                        for( n = 0; n < aLines.Count(); ++n )
                        {
                            aParam.LoopClear();
                            if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, sal_True ))
                            {
                                bRet = sal_False;
                                break;
                            }
                        }
                    }
                }
            }
            else if( aParam.bBigger )
            {
                for( n = 0; n < aLines.Count(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnOtherBox)( aLines[ n ], aParam, 0, sal_True ))
                    {
                        bRet = sal_False;
                        break;
                    }
                }
            }
            else
            {
                for( n = 0; n < aLines.Count(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, sal_True ))
                    {
                        bRet = sal_False;
                        break;
                    }
                }
            }

            // If true, set it
            if( bRet )
            {
                CR_SetBoxWidth aParam1( aParam );
                if( bInsDel )
                {
                    aParam1.bBigger = !aParam.bBigger;
                    pFndBox = ::lcl_SaveInsDelData( aParam, ppUndo,
                                                    aTmpLst, nDistStt );
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
                    for( n = aLines.Count(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnSelBox)( aLines[ n ], aParam, nDistStt, sal_False );
                        (*fnOtherBox)( aLines[ n ], aParam1, nDistStt, sal_False );
                    }
                }
                else
                    for( n = aLines.Count(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnOtherBox)( aLines[ n ], aParam1, nDistStt, sal_False );
                        (*fnSelBox)( aLines[ n ], aParam, nDistStt, sal_False );
                    }
            }
        }
        break;

    case nsTblChgWidthHeightType::WH_CELL_RIGHT:
    case nsTblChgWidthHeightType::WH_CELL_LEFT:
        if( TBLVAR_CHGABS == eTblChgMode )
        {
            // Then call itself recursively; only with another mode (proportional)
            TblChgMode eOld = eTblChgMode;
            eTblChgMode = TBLFIX_CHGABS;

            bRet = SetColWidth( rAktBox, eType, nAbsDiff, nRelDiff,
                                ppUndo );
            eTblChgMode = eOld;
            return bRet;
        }
        else if( bInsDel || ( bLeft ? nDist
                                    : (rSz.GetWidth() - nDist) > COLFUZZY ))
        {
            if( bLeft && TBLFIX_CHGABS == eTblChgMode && !bInsDel )
                aParam.bBigger = !bBigger;

            // First, see if there is enough room at all
            SwTableBox* pBox = &rAktBox;
            SwTableLine* pLine = rAktBox.GetUpper();
            while( pLine->GetUpper() )
            {
                sal_uInt16 nPos = pLine->GetTabBoxes().C40_GETPOS( SwTableBox, pBox );
                if( bLeft ? nPos : nPos + 1 != pLine->GetTabBoxes().Count() )
                    break;

                pBox = pLine->GetUpper();
                pLine = pBox->GetUpper();
            }

            if( pLine->GetUpper() )
            {
                // We need to correct the distance once again!
                aParam.nSide -= ::lcl_GetDistance( pLine->GetUpper(), sal_True );

                if( bLeft )
                    aParam.nMaxSize = aParam.nSide;
                else
                    aParam.nMaxSize = pLine->GetUpper()->GetFrmFmt()->
                                    GetFrmSize().GetWidth() - aParam.nSide;
            }

            // First, see if there is enough room at all
            if( bInsDel )
            {
                if( 0 != ( bRet = bLeft ? nDist != 0
                                : ( rSz.GetWidth() - nDist ) > COLFUZZY ) &&
                    !aParam.bBigger )
                {
                    bRet = (*fnOtherBox)( pLine, aParam, 0, sal_True );
                    if( bRet && !aParam.bAnyBoxFnd )
                        bRet = sal_False;
                }

                if( !bRet && !aParam.bBigger && rAktBox.GetFrmFmt()->
                    GetFrmSize().GetWidth() - nRelDiff > COLFUZZY +
                        ( 567 / 2 /*leave room for at least 0.5 cm*/) )
                {
                    // Consume the room from the current Cell
                    aParam.bSplittBox = sal_True;
                    bRet = sal_True;
                }
            }
            else
            {
                FN_lcl_SetBoxWidth fnTmp = aParam.bBigger ? fnOtherBox : fnSelBox;
                bRet = (*fnTmp)( pLine, aParam, nDistStt, sal_True );
            }

            // If true, set it
            if( bRet )
            {
                CR_SetBoxWidth aParam1( aParam );
                if( bInsDel )
                {
                    aParam1.bBigger = !aParam.bBigger;
                    pFndBox = ::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst, nDistStt );
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
                    (*fnSelBox)( pLine, aParam, nDistStt, sal_False );
                    (*fnOtherBox)( pLine, aParam1, nDistStt, sal_False );
                }
                else
                {
                    (*fnOtherBox)( pLine, aParam1, nDistStt, sal_False );
                    (*fnSelBox)( pLine, aParam, nDistStt, sal_False );
                }
            }
        }
        break;

    }

    if( pFndBox )
    {
        // Clean up the structure of all Lines
        GCLines();

        // Update Layout
        if( !bBigger || pFndBox->AreLinesToRestore( *this ) )
            pFndBox->MakeFrms( *this );

        // TL_CHART2: it is currently unclear if sth has to be done here.
        // The function name hints that nothing needs to be done, on the other
        // hand there is a case where sth gets deleted.  :-(

        delete pFndBox;

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

_FndBox* lcl_SaveInsDelData( CR_SetLineHeight& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst )
{
    // Find all Boxes/Lines
    SwTable& rTbl = rParam.pTblNd->GetTable();

    OSL_ENSURE( !rParam.aBoxes.empty(), "We can't go on without Boxes!" );

    // Prevent deleting the whole Table
    if( !rParam.bBigger && rParam.aBoxes.size() == rTbl.GetTabSortBoxes().Count() )
        return 0;

    _FndBox* pFndBox = new _FndBox( 0, 0 );
    if( !rParam.bBigger )
        pFndBox->SetTableLines( rParam.aBoxes, rTbl );
    else
    {
        _FndPara aPara( rParam.aBoxes, pFndBox );
        rTbl.GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
        OSL_ENSURE( pFndBox->GetLines().Count(), "Where are the Boxes?" );
        pFndBox->SetTableLines( rTbl );

        if( ppUndo )
            rTmpLst.Insert( &rTbl.GetTabSortBoxes(), 0, rTbl.GetTabSortBoxes().Count() );
    }

    // Find Lines for the Layout update
    pFndBox->DelFrms( rTbl );

    // TL_CHART2: it is currently unclear if sth has to be done here.

    return pFndBox;
}

void SetLineHeight( SwTableLine& rLine, SwTwips nOldHeight, SwTwips nNewHeight,
                    sal_Bool bMinSize )
{
    SwLayoutFrm* pLineFrm = GetRowFrm( rLine );
    OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine?" );

    SwFrmFmt* pFmt = rLine.ClaimFrmFmt();

    SwTwips nMyNewH, nMyOldH = pLineFrm->Frm().Height();
    if( !nOldHeight )                       // the BaseLine and absolute
        nMyNewH = nMyOldH + nNewHeight;
    else
    {
        // Calculate as exactly as possible
        Fraction aTmp( nMyOldH );
        aTmp *= Fraction( nNewHeight, nOldHeight );
        aTmp += Fraction( 1, 2 );       // round up if needed
        nMyNewH = aTmp;
    }

    SwFrmSize eSize = ATT_MIN_SIZE;
    if( !bMinSize &&
        ( nMyOldH - nMyNewH ) > ( CalcRowRstHeight( pLineFrm ) + ROWFUZZY ))
        eSize = ATT_FIX_SIZE;

    pFmt->SetFmtAttr( SwFmtFrmSize( eSize, 0, nMyNewH ) );

    // First adapt all internal ones
    SwTableBoxes& rBoxes = rLine.GetTabBoxes();
    for( sal_uInt16 n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox& rBox = *rBoxes[ n ];
        for( sal_uInt16 i = 0; i < rBox.GetTabLines().Count(); ++i )
            SetLineHeight( *rBox.GetTabLines()[ i ], nMyOldH, nMyNewH, bMinSize );
    }
}

sal_Bool lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                             SwTwips nDist, sal_Bool bCheck )
{
    sal_Bool bRet = sal_True;
    if( !bCheck )
    {
        // Set line height
        SetLineHeight( *pLine, 0, rParam.bBigger ? nDist : -nDist,
                        rParam.bBigger );
    }
    else if( !rParam.bBigger )
    {
        // Calculate the new relative size by means of the old one
        SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
        OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine?" );
        SwTwips nRstHeight = CalcRowRstHeight( pLineFrm );
        if( (nRstHeight + ROWFUZZY) < nDist )
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, sal_Bool bCheck )
{
    sal_Bool bRet = sal_True;
    if( bCheck )
    {
        if( rParam.bBigger )
        {
            // Calculate the new relative size by means of the old one
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
        // Set line height
        // pLine is the following/preceeding, thus adjust it
        if( TBLFIX_CHGPROP == rParam.nMode )
        {
            SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
            OSL_ENSURE( pLineFrm, "Where is the Frame from the SwTableLine??" );

            // Calculate the new relative size by means of the old one
            // If the selected Box get bigger, adjust via the max space else
            // via the max height.
            if( 1 /*!rParam.bBigger*/ )
            {
                nDist *= pLineFrm->Frm().Height();
                nDist /= rParam.nMaxHeight;
            }
            else
            {
                // Calculate the new relative size by means of the old one
                nDist *= CalcRowRstHeight( pLineFrm );
                nDist /= rParam.nMaxSpace;
            }
        }
        SetLineHeight( *pLine, 0, rParam.bBigger ? -nDist : nDist,
                        !rParam.bBigger );
    }
    return bRet;
}

sal_Bool lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                            SwTwips nDist, sal_Bool bCheck )
{
    sal_Bool bRet = sal_True;
    if( !bCheck )
    {
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwDoc* pDoc = pLine->GetFrmFmt()->GetDoc();
        if( !rParam.bBigger )
        {
            sal_uInt16 n;

            for( n = rBoxes.Count(); n; )
                ::lcl_SaveUpperLowerBorder( rParam.pTblNd->GetTable(),
                                                    *rBoxes[ --n ],
                                                    rParam.aShareFmts );
            for( n = rBoxes.Count(); n; )
                ::_DeleteBox( rParam.pTblNd->GetTable(),
                                    rBoxes[ --n ], rParam.pUndo, sal_False,
                                    sal_False, &rParam.aShareFmts );
        }
        else
        {
            // Insert Line
            SwTableLine* pNewLine = new SwTableLine( (SwTableLineFmt*)pLine->GetFrmFmt(),
                                        rBoxes.Count(), pLine->GetUpper() );
            SwTableLines* pLines;
            if( pLine->GetUpper() )
                pLines = &pLine->GetUpper()->GetTabLines();
            else
                pLines = &rParam.pTblNd->GetTable().GetTabLines();
            sal_uInt16 nPos = pLines->C40_GETPOS( SwTableLine, pLine );
            if( !rParam.bTop )
                ++nPos;
            pLines->C40_INSERT( SwTableLine, pNewLine, nPos );

            SwFrmFmt* pNewFmt = pNewLine->ClaimFrmFmt();
            pNewFmt->SetFmtAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nDist ) );

            // And once again calculate the Box count
            SwTableBoxes& rNewBoxes = pNewLine->GetTabBoxes();
            for( sal_uInt16 n = 0; n < rBoxes.Count(); ++n )
            {
                SwTwips nWidth = 0;
                SwTableBox* pOld = rBoxes[ n ];
                if( !pOld->GetSttNd() )
                {
                    // Not a normal content Box, so fall back to the 1st next Box
                    nWidth = pOld->GetFrmFmt()->GetFrmSize().GetWidth();
                    while( !pOld->GetSttNd() )
                        pOld = pOld->GetTabLines()[ 0 ]->GetTabBoxes()[ 0 ];
                }
                ::_InsTblBox( pDoc, rParam.pTblNd, pNewLine,
                                    (SwTableBoxFmt*)pOld->GetFrmFmt(), pOld, n );

                // Special treatment for the border:
                // The top one needs to be removed
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
        // Collect Boxes!
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( sal_uInt16 n = rBoxes.Count(); n; )
        {
            SwTableBox* pBox = rBoxes[ --n ];
            if( pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                return sal_False;

            if( pBox->GetSttNd() )
                rParam.aBoxes.insert( pBox );
            else
            {
                for( sal_uInt16 i = pBox->GetTabLines().Count(); i; )
                    lcl_InsDelSelLine( pBox->GetTabLines()[ --i ],
                                        rParam, 0, sal_True );
            }
        }
    }
    return bRet;
}

sal_Bool SwTable::SetRowHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff,SwUndo** ppUndo )
{
    SwTableLine* pLine = rAktBox.GetUpper();

    SwTableLine* pBaseLine = pLine;
    while( pBaseLine->GetUpper() )
        pBaseLine = pBaseLine->GetUpper()->GetUpper();

    _FndBox* pFndBox = 0;                // for insertion/deletion
    SwTableSortBoxes aTmpLst( 0 );       // for Undo
    sal_Bool bBigger,
        bRet = sal_False,
        bTop = nsTblChgWidthHeightType::WH_ROW_TOP == ( eType & 0xff ) ||
                nsTblChgWidthHeightType::WH_CELL_TOP == ( eType & 0xff ),
        bInsDel = 0 != (eType & nsTblChgWidthHeightType::WH_FLAG_INSDEL );
    sal_uInt16 n, nBaseLinePos = GetTabLines().C40_GETPOS( SwTableLine, pBaseLine );
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

    // How do we get to the height?
    switch( eType & 0xff )
    {
    case nsTblChgWidthHeightType::WH_CELL_TOP:
    case nsTblChgWidthHeightType::WH_CELL_BOTTOM:
        if( pLine == pBaseLine )
            break;  // it doesn't work then!

        // Is a nested Line (Box!)
        pLines = &pLine->GetUpper()->GetTabLines();
        nBaseLinePos = pLines->C40_GETPOS( SwTableLine, pLine );
        pBaseLine = pLine;
        // no break!

    case nsTblChgWidthHeightType::WH_ROW_TOP:
    case nsTblChgWidthHeightType::WH_ROW_BOTTOM:
        {
            if( bInsDel && !bBigger )       // By how much does it get higher?
            {
                nAbsDiff = GetRowFrm( *pBaseLine )->Frm().Height();
            }

            if( TBLVAR_CHGABS == eTblChgMode )
            {
                // First test if we have room at all
                if( bBigger )
                {
                    bRet = sal_True;
// What's up with Top, Table in a Frame or Header/Footer with fixed width??
                    if( !bRet )
                    {
                        // Then call itself recursively; only with another mode (proportional)
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
                                        nAbsDiff, sal_True );

                if( bRet )
                {
                    if( bInsDel )
                    {
                        if( aParam.aBoxes.empty() )
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, sal_True );

                        pFndBox = ::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst );

                        // delete complete table when last row is deleted
                        if( !bBigger &&
                            aParam.aBoxes.size() == aSortCntBoxes.Count() )
                        {
                            GetFrmFmt()->GetDoc()->DeleteRowCol( aParam.aBoxes );
                            return sal_False;
                        }


                        if( ppUndo )
                            *ppUndo = aParam.CreateUndo(
                                        bBigger ? UNDO_TABLE_INSROW
                                                : UNDO_ROW_DELETE );
                    }
                    else if( ppUndo )
                        *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, sal_True );

                    (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                    nAbsDiff, sal_False );
                }
            }
            else
            {
                bRet = sal_True;
                sal_uInt16 nStt, nEnd;
                if( bTop )
                    nStt = 0, nEnd = nBaseLinePos;
                else
                    nStt = nBaseLinePos + 1, nEnd = pLines->Count();

                // Get the current Lines' height
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
                        bRet = sal_False;
                }
                else
                {
                    if( bTop ? nEnd : nStt < nEnd  )
                    {
                        if( bTop )
                            nStt = nEnd - 1;
                        else
                            nEnd = nStt + 1;
                    }
                    else
                        bRet = sal_False;
                }

                if( bRet )
                {
                    if( bBigger )
                    {
                        for( n = nStt; n < nEnd; ++n )
                        {
                            if( !(*fnOtherLine)( (*pLines)[ n ], aParam,
                                                    nAbsDiff, sal_True ))
                            {
                                bRet = sal_False;
                                break;
                            }
                        }
                    }
                    else
                        bRet = (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                                nAbsDiff, sal_True );
                }

                if( bRet )
                {
                    // Adjust
                    if( bInsDel )
                    {
                        if( aParam.aBoxes.empty() )
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, sal_True );
                        pFndBox = ::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst );
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
                        // We need to distribute the space evenly among all the Lines.
                        // That's why we need their count.
                        aParam1.nLines = nEnd - nStt;
                    }

                    if( bTop )
                    {
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, sal_False );
                        for( n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, sal_False );
                    }
                    else
                    {
                        for( n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, sal_False );
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, sal_False );
                    }
                }
                else
                {
                    // Then call itself recursively; only with another mode (proportional)
                    TblChgMode eOld = eTblChgMode;
                    eTblChgMode = TBLVAR_CHGABS;

                    bRet = SetRowHeight( rAktBox, eType, nAbsDiff,
                                        nRelDiff, ppUndo );

                    eTblChgMode = eOld;
                    pFndBox = 0;
                }
            }
        }
        break;
    }

    if( pFndBox )
    {
        // then clean up the structure of all Lines
        GCLines();

        // Update Layout
        if( bBigger || pFndBox->AreLinesToRestore( *this ) )
            pFndBox->MakeFrms( *this );

        // TL_CHART2: it is currently unclear if sth has to be done here.

        delete pFndBox;

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
    for( sal_uInt16 n = aNewFmts.Count(); n; )
        if( ( pTmp = (SwFrmFmt*)aNewFmts[ --n ])->GetFrmSize().GetWidth()
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
    for( sal_uInt16 n = aNewFmts.Count(); n; )
        if( SFX_ITEM_SET == ( pTmp = (SwFrmFmt*)aNewFmts[ --n ])->
            GetItemState( nWhich, sal_False, &pItem ) && *pItem == rItem &&
            pTmp->GetFmtAttr( RES_FRM_SIZE, sal_False ) == rFrmSz )
        {
            pRet = pTmp;
            break;
        }
    return pRet;
}

void SwShareBoxFmt::AddFormat( const SwFrmFmt& rNew )
{
    void* pFmt = (void*)&rNew;
    aNewFmts.Insert( pFmt, aNewFmts.Count() );
}

sal_Bool SwShareBoxFmt::RemoveFormat( const SwFrmFmt& rFmt )
{
    // returns sal_True, if we can delete
    if( pOldFmt == &rFmt )
        return sal_True;

    void* p = (void*)&rFmt;
    sal_uInt16 nFnd = aNewFmts.GetPos( p );
    if( USHRT_MAX != nFnd )
        aNewFmts.Remove( nFnd );
    return 0 == aNewFmts.Count();
}

SwShareBoxFmts::~SwShareBoxFmts()
{
}

SwFrmFmt* SwShareBoxFmts::GetFormat( const SwFrmFmt& rFmt, long nWidth ) const
{
    sal_uInt16 nPos;
    return Seek_Entry( rFmt, &nPos )
                    ? aShareArr[ nPos ]->GetFormat( nWidth )
                    : 0;
}
SwFrmFmt* SwShareBoxFmts::GetFormat( const SwFrmFmt& rFmt,
                                     const SfxPoolItem& rItem ) const
{
    sal_uInt16 nPos;
    return Seek_Entry( rFmt, &nPos )
                    ? aShareArr[ nPos ]->GetFormat( rItem )
                    : 0;
}

void SwShareBoxFmts::AddFormat( const SwFrmFmt& rOld, const SwFrmFmt& rNew )
{
    {
        sal_uInt16 nPos;
        SwShareBoxFmt* pEntry;
        if( !Seek_Entry( rOld, &nPos ))
        {
            pEntry = new SwShareBoxFmt( rOld );
            aShareArr.C40_INSERT( SwShareBoxFmt, pEntry, nPos );
        }
        else
            pEntry = aShareArr[ nPos ];

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
    for( sal_uInt16 i = aShareArr.Count(); i; )
        if( aShareArr[ --i ]->RemoveFormat( rFmt ))
            aShareArr.DeleteAndDestroy( i );
}

sal_Bool SwShareBoxFmts::Seek_Entry( const SwFrmFmt& rFmt, sal_uInt16* pPos ) const
{
    sal_uLong nIdx = (sal_uLong)&rFmt;
    sal_uInt16 nO = aShareArr.Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            sal_uLong nFmt = (sal_uLong)&aShareArr[ nM ]->GetOldFormat();
            if( nFmt == nIdx )
            {
                if( pPos )
                    *pPos = nM;
                return sal_True;
            }
            else if( nFmt < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pPos )
                    *pPos = nU;
                return sal_False;
            }
            else
                nO = nM - 1;
        }
    }
    if( pPos )
        *pPos = nU;
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
