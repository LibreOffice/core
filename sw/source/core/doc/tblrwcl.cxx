/*************************************************************************
 *
 *  $RCSfile: tblrwcl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#define _ZFORLIST_DECLARE_TABLE
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _TABFRM_HXX //autogen
#include <tabfrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _TBLRWCL_HXX
#include <tblrwcl.hxx>
#endif

#define COLFUZZY 20

typedef SwTableLine* SwTableLinePtr;
SV_DECL_PTRARR_SORT( SwSortTableLines, SwTableLinePtr, 16, 16 );
SV_IMPL_PTRARR_SORT( SwSortTableLines, SwTableLinePtr );

SV_IMPL_PTRARR( _SwShareBoxFmts, SwShareBoxFmt* )

// fuers setzen der Frame-Formate an den Boxen reicht es, das aktuelle
// im Array zu suchen. Ist es vorhanden, so gebe das neue zurueck
struct _CpyTabFrm
{
    union {
        SwTableBoxFmt *pFrmFmt;     // fuer CopyCol
        SwTwips nSize;              // fuer DelCol
    } Value;
    SwTableBoxFmt *pNewFrmFmt;

    _CpyTabFrm( SwTableBoxFmt* pAktFrmFmt ) : pNewFrmFmt( 0 )
    {   Value.pFrmFmt = pAktFrmFmt; }

    _CpyTabFrm& operator=( const _CpyTabFrm& );

    BOOL operator==( const _CpyTabFrm& rCpyTabFrm )
        { return  (ULONG)Value.nSize == (ULONG)rCpyTabFrm.Value.nSize; }
    BOOL operator<( const _CpyTabFrm& rCpyTabFrm )
        { return  (ULONG)Value.nSize < (ULONG)rCpyTabFrm.Value.nSize; }
};

struct CR_SetBoxWidth
{
    SwSelBoxes aBoxes;
    SwSortTableLines aLines;
    SvUShorts aLinesWidth;
    SwShareBoxFmts aShareFmts;
    SwTableNode* pTblNd;
    SwUndoTblNdsChg* pUndo;
    SwTwips nDiff, nSide, nMaxSize, nLowerDiff;
    USHORT nMode, nTblWidth, nRemainWidth, nBoxWidth;
    BOOL bBigger, bLeft, bSplittBox, bAnyBoxFnd;

    CR_SetBoxWidth( USHORT eType, SwTwips nDif, SwTwips nSid, SwTwips nTblW,
                    SwTwips nMax, SwTableNode* pTNd )
        : nDiff( nDif ), nSide( nSid ), nMaxSize( nMax ),
        nTblWidth( (USHORT)nTblW ), nRemainWidth( 0 ),
        nLowerDiff( 0 ), bAnyBoxFnd( FALSE ),
        pTblNd( pTNd ), bSplittBox( FALSE ), nBoxWidth( 0 )
    {
        bLeft = WH_COL_LEFT == ( eType & 0xff ) ||
                WH_CELL_LEFT == ( eType & 0xff );
        bBigger = 0 != (eType & WH_FLAG_BIGGER );
        nMode = pTblNd->GetTable().GetTblChgMode();
    }
    CR_SetBoxWidth( const CR_SetBoxWidth& rCpy )
        : nDiff( rCpy.nDiff ), nMode( rCpy.nMode ),
        nSide( rCpy.nSide ), bBigger( rCpy.bBigger ), bLeft( rCpy.bLeft ),
        bSplittBox( rCpy.bSplittBox ), bAnyBoxFnd( rCpy.bAnyBoxFnd ),
        nTblWidth( rCpy.nTblWidth ), nRemainWidth( rCpy.nRemainWidth ),
        pTblNd( rCpy.pTblNd ), nMaxSize( rCpy.nMaxSize ), nLowerDiff( 0 ),
        pUndo( rCpy.pUndo ), nBoxWidth( nBoxWidth )
    {
        aLines.Insert( &rCpy.aLines );
        aLinesWidth.Insert( &rCpy.aLinesWidth, 0 );
    }

    SwUndoTblNdsChg* CreateUndo( USHORT nUndoType )
    {
        return pUndo = new SwUndoTblNdsChg( nUndoType, aBoxes, *pTblNd );
    }

    void LoopClear()
    {
        nLowerDiff = 0; nRemainWidth = 0;
    }

    void AddBoxWidth( const SwTableBox& rBox, USHORT nWidth )
    {
        SwTableLinePtr p = (SwTableLine*)rBox.GetUpper();
        USHORT nFndPos;
        if( aLines.Insert( p, nFndPos ))
            aLinesWidth.Insert( nWidth, nFndPos );
        else
            aLinesWidth[ nFndPos ] += nWidth;
    }

    USHORT GetBoxWidth( const SwTableLine& rLn ) const
    {
        SwTableLinePtr p = (SwTableLine*)&rLn;
        USHORT nFndPos;
        if( aLines.Seek_Entry( p, &nFndPos ) )
            nFndPos = aLinesWidth[ nFndPos ];
        else
            nFndPos = 0;
        return nFndPos;
    }
};

BOOL lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, BOOL bCheck );
BOOL lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, BOOL bCheck );
BOOL lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, BOOL bCheck );
BOOL lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, BOOL bCheck );
BOOL lcl_DelSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, BOOL bCheck );
BOOL lcl_DelOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, BOOL bCheck );

typedef BOOL (*FN_lcl_SetBoxWidth)(SwTableLine*, CR_SetBoxWidth&, SwTwips, BOOL );

#if !defined( PRODUCT ) || defined( JP_DEBUG )

void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize );

#define CHECKBOXWIDTH                                           \
    {                                                           \
        SwTwips nSize = GetFrmFmt()->GetFrmSize().GetWidth();   \
        for( USHORT n = 0; n < aLines.Count(); ++n  )           \
            ::_CheckBoxWidth( *aLines[ n ], nSize );            \
    }

#else

#define CHECKBOXWIDTH

#endif


struct CR_SetLineHeight
{
    SwSelBoxes aBoxes;
    SwShareBoxFmts aShareFmts;
    SwTableNode* pTblNd;
    SwUndoTblNdsChg* pUndo;
    SwTwips nMaxSpace, nMaxHeight;
    USHORT nMode, nLines;
    BOOL bBigger, bTop, bSplittBox, bAnyBoxFnd;

    CR_SetLineHeight( USHORT eType, SwTableNode* pTNd )
        : nMaxSpace( 0 ), nLines( 0 ), nMaxHeight( 0 ),
        bAnyBoxFnd( FALSE ), bSplittBox( FALSE ) ,
        pTblNd( pTNd ), pUndo( 0 )
    {
        bTop = WH_ROW_TOP == ( eType & 0xff ) || WH_CELL_TOP == ( eType & 0xff );
        bBigger = 0 != (eType & WH_FLAG_BIGGER );
        if( eType & WH_FLAG_INSDEL )
            bBigger = !bBigger;
        BOOL bTst = (0 != (eType & WH_FLAG_BIGGER )) ^ (0 != ( eType & WH_FLAG_INSDEL ));
        nMode = pTblNd->GetTable().GetTblChgMode();
    }
    CR_SetLineHeight( const CR_SetLineHeight& rCpy )
        : nMode( rCpy.nMode ), nMaxSpace( rCpy.nMaxSpace ),
        bBigger( rCpy.bBigger ), bTop( rCpy.bTop ),
        bSplittBox( rCpy.bSplittBox ), bAnyBoxFnd( rCpy.bAnyBoxFnd ),
        pTblNd( rCpy.pTblNd ), nLines( rCpy.nLines ),
        nMaxHeight( rCpy.nMaxHeight ),
        pUndo( rCpy.pUndo )
    {}

    SwUndoTblNdsChg* CreateUndo( USHORT nUndoType )
    {
        return pUndo = new SwUndoTblNdsChg( nUndoType, aBoxes, *pTblNd );
    }
};

BOOL lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                         SwTwips nDist, BOOL bCheck );
BOOL lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, BOOL bCheck );
BOOL lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, BOOL bCheck );

typedef BOOL (*FN_lcl_SetLineHeight)(SwTableLine*, CR_SetLineHeight&, SwTwips, BOOL );

_CpyTabFrm& _CpyTabFrm::operator=( const _CpyTabFrm& rCpyTabFrm )
{
    pNewFrmFmt = rCpyTabFrm.pNewFrmFmt;
    Value = rCpyTabFrm.Value;
    return *this;
}

SV_DECL_VARARR_SORT( _CpyTabFrms, _CpyTabFrm, 0, 50 )
SV_IMPL_VARARR_SORT( _CpyTabFrms, _CpyTabFrm )

void lcl_DelCpyTabFrmFmts( _CpyTabFrm& rArr );

// ---------------------------------------------------------------

struct _CpyPara
{
    SwDoc* pDoc;
    SwTableNode* pTblNd;
    _CpyTabFrms& rTabFrmArr;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    ULONG nOldSize, nNewSize;           // zum Korrigieren der Size-Attribute
    USHORT nCpyCnt, nInsPos;
    BYTE nDelBorderFlag;
    BOOL bCpyCntnt;

    _CpyPara( SwTableNode* pNd, USHORT nCopies, _CpyTabFrms& rFrmArr,
                BOOL bCopyContent = TRUE )
        : pDoc( pNd->GetDoc() ), pTblNd( pNd ), nCpyCnt(nCopies), rTabFrmArr(rFrmArr),
        pInsLine(0), pInsBox(0), nInsPos(0), nNewSize(0), nOldSize(0),
        bCpyCntnt( bCopyContent ), nDelBorderFlag( 0 )
        {}
    _CpyPara( const _CpyPara& rPara, SwTableLine* pLine )
        : pDoc(rPara.pDoc), pTblNd(rPara.pTblNd), nCpyCnt(rPara.nCpyCnt),
        rTabFrmArr(rPara.rTabFrmArr), pInsLine(pLine), pInsBox(rPara.pInsBox),
        nInsPos(0), nNewSize(rPara.nNewSize), nOldSize(0),
        bCpyCntnt( rPara.bCpyCntnt ), nDelBorderFlag( rPara.nDelBorderFlag )
        {}
    _CpyPara( const _CpyPara& rPara, SwTableBox* pBox )
        : pDoc(rPara.pDoc), pTblNd(rPara.pTblNd), nCpyCnt(rPara.nCpyCnt),
        rTabFrmArr(rPara.rTabFrmArr), pInsLine(rPara.pInsLine), pInsBox(pBox),
        nInsPos(0), nNewSize(rPara.nNewSize),nOldSize(rPara.nOldSize),
        bCpyCntnt( rPara.bCpyCntnt ), nDelBorderFlag( rPara.nDelBorderFlag )
        {}
    void SetBoxWidth( SwTableBox* pBox );
};


BOOL lcl_CopyCol( const _FndBox*& rpFndBox, void* pPara )
{
    _CpyPara* pCpyPara = (_CpyPara*)pPara;

    // suche das FrmFmt im Array aller Frame-Formate
    SwTableBox* pBox = (SwTableBox*)rpFndBox->GetBox();
    _CpyTabFrm aFindFrm( (SwTableBoxFmt*)pBox->GetFrmFmt() );

    USHORT nFndPos;
    if( pCpyPara->nCpyCnt )
    {
        if( !pCpyPara->rTabFrmArr.Seek_Entry( aFindFrm, &nFndPos ))
        {
            // fuer das verschachtelte Kopieren sicher auch das neue Format
            // als alt.
            SwTableBoxFmt* pNewFmt = (SwTableBoxFmt*)pBox->ClaimFrmFmt();

            // suche die selektierten Boxen in der Line:
            _FndLine* pCmpLine;
            SwFmtFrmSize aFrmSz( pNewFmt->GetFrmSize() );
            if( pBox->GetTabLines().Count() &&
                ( pCmpLine = rpFndBox->GetLines()[ 0 ])->GetBoxes().Count()
                    != pCmpLine->GetLine()->GetTabBoxes().Count() )
            {
                // die erste Line sollte reichen
                _FndBoxes& rFndBoxes = pCmpLine->GetBoxes();
                long nSz = 0;
                for( USHORT n = rFndBoxes.Count(); n; )
                    nSz += rFndBoxes[ --n ]->GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
                aFrmSz.SetWidth( aFrmSz.GetWidth() -
                                            nSz / ( pCpyPara->nCpyCnt + 1 ) );
                pNewFmt->SetAttr( aFrmSz );
                aFrmSz.SetWidth( nSz / ( pCpyPara->nCpyCnt + 1 ) );

                // fuer die neue Box ein neues Format mit der Groesse anlegen!
                aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pNewFmt->GetDoc()->
                                            MakeTableLineFmt();
                *aFindFrm.pNewFrmFmt = *pNewFmt;
                aFindFrm.pNewFrmFmt->SetAttr( aFrmSz );
            }
            else
            {
                aFrmSz.SetWidth( aFrmSz.GetWidth() / ( pCpyPara->nCpyCnt + 1 ) );
                pNewFmt->SetAttr( aFrmSz );

                aFindFrm.pNewFrmFmt = pNewFmt;
                pCpyPara->rTabFrmArr.Insert( aFindFrm );
                aFindFrm.Value.pFrmFmt = pNewFmt;
                pCpyPara->rTabFrmArr.Insert( aFindFrm );
            }
        }
        else
        {
            aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ];
//          aFindFrm.pNewFrmFmt->Add( pBox );
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

    if( rpFndBox->GetLines().Count() )
    {
        pBox = new SwTableBox( aFindFrm.pNewFrmFmt,
                    rpFndBox->GetLines().Count(), pCpyPara->pInsLine );
        pCpyPara->pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, pCpyPara->nInsPos++);
        _CpyPara aPara( *pCpyPara, pBox );
        aPara.nDelBorderFlag &= 7;

        ((_FndBox*)rpFndBox)->GetLines().ForEach( &lcl_CopyRow, &aPara );
    }
    else
    {
        ::_InsTblBox( pCpyPara->pDoc, pCpyPara->pTblNd, pCpyPara->pInsLine,
                    aFindFrm.pNewFrmFmt, pBox, pCpyPara->nInsPos++ );

        const _FndBoxes& rFndBxs = rpFndBox->GetUpper()->GetBoxes();
        if( 8 > pCpyPara->nDelBorderFlag
                ? pCpyPara->nDelBorderFlag
                : rpFndBox == rFndBxs[ rFndBxs.Count() - 1 ] )
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
                    // es wird dahinter kopiert, bei allen Boxen die
                    // TopBorderLine loeschen
                    pBox = pCpyPara->pInsLine->GetTabBoxes()[
                                            pCpyPara->nInsPos - 1 ];
                }

                aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();

                // ansonsten wird davor kopiert und die erste Line behaelt
                // die TopLine und an der originalen wird sie entfernt
                pBox->ClaimFrmFmt()->SetAttr( aNew );

                if( !pCpyPara->nCpyCnt )
                    pCpyPara->rTabFrmArr.Insert( aFindFrm );
            }
        }
    }
    return TRUE;
}

BOOL lcl_CopyRow( const _FndLine*& rpFndLine, void* pPara )
{
    _CpyPara* pCpyPara = (_CpyPara*)pPara;
    SwTableLine* pNewLine = new SwTableLine(
                            (SwTableLineFmt*)rpFndLine->GetLine()->GetFrmFmt(),
                        rpFndLine->GetBoxes().Count(), pCpyPara->pInsBox );
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
    ((_FndLine*)rpFndLine)->GetBoxes().ForEach( &lcl_CopyCol, &aPara );

    pCpyPara->nDelBorderFlag &= 0xf8;
    return TRUE;
}

//-----------------------------------------------------------

void lcl_InsCol( _FndLine* pFndLn, _CpyPara& rCpyPara, USHORT nCpyCnt,
                BOOL bBehind )
{
    // Bug 29124: nicht nur in den Grundlines kopieren. Wenns geht, so weit
    //              runter wie moeglich.
    _FndBox* pFBox;
    if( 1 == pFndLn->GetBoxes().Count() &&
        !( pFBox = pFndLn->GetBoxes()[ 0 ] )->GetBox()->GetSttNd() )
    {
        // eine Box mit mehreren Lines, also in diese Lines einfuegen
        for( USHORT n = 0; n < pFBox->GetLines().Count(); ++n )
            lcl_InsCol( pFBox->GetLines()[ n ], rCpyPara, nCpyCnt, bBehind );
    }
    else
    {
        rCpyPara.pInsLine = pFndLn->GetLine();
        SwTableBox* pBox = pFndLn->GetBoxes()[ bBehind ?
                    pFndLn->GetBoxes().Count()-1 : 0 ]->GetBox();
        rCpyPara.nInsPos = pFndLn->GetLine()->GetTabBoxes().C40_GETPOS( SwTableBox, pBox );
        USHORT nBoxPos = rCpyPara.nInsPos;
        if( bBehind )
            ++rCpyPara.nInsPos;

        for( USHORT n = 0; n < nCpyCnt; ++n )
        {
            if( n + 1 == nCpyCnt && bBehind )
                rCpyPara.nDelBorderFlag = 9;
            else
                rCpyPara.nDelBorderFlag = 8;
            pFndLn->GetBoxes().ForEach( &lcl_CopyCol, &rCpyPara );
        }
    }
}


BOOL SwTable::InsertCol( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        USHORT nCnt, BOOL bBehind )
{
    ASSERT( pDoc && rBoxes.Count() && nCnt, "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return FALSE;

    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return FALSE;

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    //Lines fuer das Layout-Update herausuchen.
    aFndBox.SetTableLines( *this );
    aFndBox.DelFrms( *this );
    aFndBox.SaveChartData( *this );

    _CpyTabFrms aTabFrmArr;
    _CpyPara aCpyPara( pTblNd, nCnt, aTabFrmArr );

    for( USHORT n = 0; n < aFndBox.GetLines().Count(); ++n )
        lcl_InsCol( aFndBox.GetLines()[ n ], aCpyPara, nCnt, bBehind );

    // dann raeume die Struktur dieser Line noch mal auf, generell alle
    GCLines();

    //Layout updaten
    aFndBox.MakeFrms( *this );
    aFndBox.RestoreChartData( *this );
    CHECKBOXWIDTH
    return TRUE;
}


BOOL SwTable::InsertRow( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                        USHORT nCnt, BOOL bBehind )
{
    ASSERT( pDoc && rBoxes.Count() && nCnt, "keine gueltige Box-Liste" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return FALSE;

    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return FALSE;

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    _FndBox* pFndBox = &aFndBox;
    {
        _FndLine* pFndLine;
        while( 1 == pFndBox->GetLines().Count() &&
                1 == ( pFndLine = pFndBox->GetLines()[ 0 ])->GetBoxes().Count() )
        {
            // nicht zu weit runter, eine Line mit Boxen muss nachbleiben!!
            _FndBox* pTmpBox = pFndLine->GetBoxes()[ 0 ];
            if( pTmpBox->GetLines().Count() )
                pFndBox = pTmpBox;
            else
                break;
        }
    }

    //Lines fuer das Layout-Update herausuchen.
    const FASTBOOL bLayout = 0 != SwClientIter( *GetFrmFmt() ).First( TYPE(SwTabFrm) );
    if ( bLayout )
    {
        aFndBox.SetTableLines( *this );
        if( pFndBox != &aFndBox )
            aFndBox.DelFrms( *this );
        aFndBox.SaveChartData( *this );
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

    USHORT nLinePos = aCpyPara.nInsPos;
    if( bBehind )
    {
        ++aCpyPara.nInsPos;
        aCpyPara.nDelBorderFlag = 1;
    }
    else
        aCpyPara.nDelBorderFlag = 2;

    for( USHORT nCpyCnt = 0; nCpyCnt < nCnt; ++nCpyCnt )
    {
        if( bBehind )
            aCpyPara.nDelBorderFlag = 1;
        pFndBox->GetLines().ForEach( &lcl_CopyRow, &aCpyPara );
    }

    // dann raeume die Struktur dieser Line noch mal auf, generell alle
    if( !pDoc->IsInReading() )
        GCLines();

    //Layout updaten
    if ( bLayout )
    {
        if( pFndBox != &aFndBox )
            aFndBox.MakeFrms( *this );
        else
            aFndBox.MakeNewFrms( *this, nCnt, bBehind );
        aFndBox.RestoreChartData( *this );
    }
    CHECKBOXWIDTH
    return TRUE;
}

BOOL _FndBoxAppendRowLine( const SwTableLine*& rpLine, void* pPara );

BOOL _FndBoxAppendRowBox( const SwTableBox*& rpBox, void* pPara )
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
        pFndPara->pFndLine->GetBoxes().C40_INSERT( _FndBox, pFndBox,
                        pFndPara->pFndLine->GetBoxes().Count() );
    return TRUE;
}

BOOL _FndBoxAppendRowLine( const SwTableLine*& rpLine, void* pPara )
{
    _FndPara* pFndPara = (_FndPara*)pPara;
    _FndLine* pFndLine = new _FndLine( (SwTableLine*)rpLine, pFndPara->pFndBox );
    _FndPara aPara( *pFndPara, pFndLine );
    pFndLine->GetLine()->GetTabBoxes().ForEach( &_FndBoxAppendRowBox, &aPara );
    if( pFndLine->GetBoxes().Count() )
    {
        pFndPara->pFndBox->GetLines().C40_INSERT( _FndLine, pFndLine,
                pFndPara->pFndBox->GetLines().Count() );
    }
    else
        delete pFndLine;
    return TRUE;
}


BOOL SwTable::AppendRow( SwDoc* pDoc, USHORT nCnt )
{
    SwTableNode* pTblNd = (SwTableNode*)aSortCntBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return FALSE;

    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        const SwTableLine* pLLine = GetTabLines()[ GetTabLines().Count()-1 ];

        const SwSelBoxes* pBxs = 0;     // Dummy !!!
        _FndPara aPara( *pBxs, &aFndBox );

        _FndBoxAppendRowLine( pLLine, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return FALSE;

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    //Lines fuer das Layout-Update herausuchen.
    const FASTBOOL bLayout = 0 != SwClientIter( *GetFrmFmt() ).First( TYPE(SwTabFrm) );
    if( bLayout )
    {
        aFndBox.SetTableLines( *this );
        aFndBox.SaveChartData( *this );
    }

    _CpyTabFrms aTabFrmArr;
    _CpyPara aCpyPara( pTblNd, 0, aTabFrmArr );
    aCpyPara.nInsPos = GetTabLines().Count();
    aCpyPara.nDelBorderFlag = 1;

    for( USHORT nCpyCnt = 0; nCpyCnt < nCnt; ++nCpyCnt )
    {
        aCpyPara.nDelBorderFlag = 1;
        aFndBox.GetLines().ForEach( &lcl_CopyRow, &aCpyPara );
    }

    // dann raeume die Struktur dieser Line noch mal auf, generell alle
    if( !pDoc->IsInReading() )
        GCLines();

    //Layout updaten
    if ( bLayout )
    {
        aFndBox.MakeNewFrms( *this, nCnt, TRUE );
        aFndBox.RestoreChartData( *this );
    }
    CHECKBOXWIDTH
    return TRUE;
}


void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            SwShareBoxFmts& rShareFmts );

void lcl_LastBoxSetWidthLine( SwTableLines &rLines, const long nOffset,
                                SwShareBoxFmts& rShareFmts )
{
    for ( USHORT i = 0; i < rLines.Count(); ++i )
        ::lcl_LastBoxSetWidth( rLines[i]->GetTabBoxes(), nOffset, rShareFmts) ;
}

void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            SwShareBoxFmts& rShareFmts )
{
    SwTableBox& rBox = *rBoxes[ rBoxes.Count() - 1 ];
    if( !rBox.GetSttNd() )
        ::lcl_LastBoxSetWidthLine( rBox.GetTabLines(), nOffset, rShareFmts );

    //Die Box anpassen
    SwFrmFmt *pBoxFmt = rBox.GetFrmFmt();
    SwFmtFrmSize aNew( pBoxFmt->GetFrmSize() );
    aNew.SetWidth( aNew.GetWidth() + nOffset );
    SwFrmFmt *pFmt = rShareFmts.GetFormat( *pBoxFmt, aNew );
    if( pFmt )
        rBox.ChgFrmFmt( (SwTableBoxFmt*)pFmt );
    else
    {
        pFmt = rBox.ClaimFrmFmt();

        pFmt->LockModify();
        pFmt->SetAttr( aNew );
        pFmt->UnlockModify();

        rShareFmts.AddFormat( *pBoxFmt, *pFmt );
    }
}

void _DeleteBox( SwTable& rTbl, SwTableBox* pBox, SwUndo* pUndo,
                BOOL bCalcNewSize, const BOOL bCorrBorder,
                SwShareBoxFmts* pShareFmts )
{
    do {
        SwTwips nBoxSz = bCalcNewSize ?
                pBox->GetFrmFmt()->GetFrmSize().GetWidth() : 0;
        SwTableLine* pLine = pBox->GetUpper();
        SwTableBoxes& rTblBoxes = pLine->GetTabBoxes();
        USHORT nDelPos = rTblBoxes.C40_GETPOS( SwTableBox, pBox );
        SwTableBox* pUpperBox = pBox->GetUpper()->GetUpper();

        // Sonderbehandlung fuer Umrandung:
        if( bCorrBorder && 1 < rTblBoxes.Count() )
        {
            BOOL bChgd = FALSE;
            const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();

            if( rBoxItem.GetLeft() || rBoxItem.GetRight() )
            {
                //JP 02.04.97:  1.Teil fuer Bug 36271
                // zuerst die linken/rechten Kanten
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
                            pNxtBox->ClaimFrmFmt()->SetAttr( aTmp );
                        bChgd = TRUE;
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
                            pPrvBox->ClaimFrmFmt()->SetAttr( aTmp );
                    }
                }
            }

        }

        // erst die Box, dann die Nodes loeschen!!
        SwStartNode* pSttNd = (SwStartNode*)pBox->GetSttNd();
        if( pShareFmts )
            pShareFmts->RemoveFormat( *rTblBoxes[ nDelPos ]->GetFrmFmt() );
        rTblBoxes.DeleteAndDestroy( nDelPos );

        if( pSttNd )
        {
            // ist das UndoObject zum speichern der Section vorbereitet?
            if( pUndo && UNDO_TABLE_DELBOX == pUndo->GetId() )
                ((SwUndoTblNdsChg*)pUndo)->SaveSection( pSttNd );
            else
                pSttNd->GetDoc()->DeleteSection( pSttNd );
        }

        // auch die Zeile noch loeschen ??
        if( rTblBoxes.Count() )
        {
            // dann passe noch die Frame-SSize an
            if( nDelPos == rTblBoxes.Count() )
                --nDelPos;
            pBox = rTblBoxes[nDelPos];
            if( bCalcNewSize )
            {
                SwFmtFrmSize aNew( pBox->GetFrmFmt()->GetFrmSize() );
                aNew.SetWidth( aNew.GetWidth() + nBoxSz );
                if( pShareFmts )
                    pShareFmts->SetSize( *pBox, aNew );
                else
                    pBox->ClaimFrmFmt()->SetAttr( aNew );

                if( !pBox->GetSttNd() )
                {
                    // dann muss es auch rekursiv in allen Zeilen, in allen
                    // Zellen erfolgen!
                    SwShareBoxFmts aShareFmts;
                    ::lcl_LastBoxSetWidthLine( pBox->GetTabLines(), nBoxSz,
                                                pShareFmts ? *pShareFmts
                                                           : aShareFmts );
                }
            }
            break;      // nichts mehr loeschen
        }
        // loesche die Line aus Tabelle/Box
        if( !pUpperBox )
        {
            // dann loesche auch noch die Line aus der Tabelle
            nDelPos = rTbl.GetTabLines().C40_GETPOS( SwTableLine, pLine );
            if( pShareFmts )
                pShareFmts->RemoveFormat( *rTbl.GetTabLines()[ nDelPos ]->GetFrmFmt() );
            rTbl.GetTabLines().DeleteAndDestroy( nDelPos );
            break;      // mehr kann nicht geloescht werden
        }

        // dann loesche auch noch die Line
        pBox = pUpperBox;
        nDelPos = pBox->GetTabLines().C40_GETPOS( SwTableLine, pLine );
        if( pShareFmts )
            pShareFmts->RemoveFormat( *pBox->GetTabLines()[ nDelPos ]->GetFrmFmt() );
        pBox->GetTabLines().DeleteAndDestroy( nDelPos );
    } while( !pBox->GetTabLines().Count() );
}

SwTableBox* lcl_FndNxtPrvDelBox( const SwTableLines& rTblLns,
                                SwTwips nBoxStt, SwTwips nBoxWidth,
                                USHORT nLinePos, BOOL bNxt,
                                SwSelBoxes* pAllDelBoxes, USHORT* pCurPos )
{
    SwTableBox* pFndBox = 0;
    do {
        if( bNxt )
            ++nLinePos;
        else
            --nLinePos;
        SwTableLine* pLine = rTblLns[ nLinePos ];
        SwTwips nFndBoxWidth, nFndWidth = nBoxStt + nBoxWidth;
        USHORT nBoxCnt = pLine->GetTabBoxes().Count();

        for( USHORT n = 0; 0 < nFndWidth && n < nBoxCnt; ++n )
        {
            pFndBox = pLine->GetTabBoxes()[ n ];
            nFndWidth -= (nFndBoxWidth = pFndBox->GetFrmFmt()->
                                        GetFrmSize().GetWidth());
        }

        // suche die erste ContentBox
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
            // falls der Vorganger auch geloscht wird, ist nicht zu tun
            USHORT nFndPos;
            if( !pAllDelBoxes->Seek_Entry( pFndBox, &nFndPos ) )
                break;

            // sonst noch mal weitersuchen
            // Die Box muessen wir aber nicht nochmal abpruefen
            pFndBox = 0;
            if( nFndPos <= *pCurPos )
                --*pCurPos;
            pAllDelBoxes->Remove( nFndPos );
        }
    } while( bNxt ? ( nLinePos + 1 < rTblLns.Count() ) : nLinePos );
    return pFndBox;
}

void lcl_SaveUpperLowerBorder( SwTable& rTbl, const SwTableBox& rBox,
                                SwShareBoxFmts& rShareFmts,
                                SwSelBoxes* pAllDelBoxes = 0,
                                USHORT* pCurPos = 0 )
{
//JP 16.04.97:  2.Teil fuer Bug 36271
    BOOL bChgd = FALSE;
    const SwTableLine* pLine = rBox.GetUpper();
    const SwTableBoxes& rTblBoxes = pLine->GetTabBoxes();
    const SwTableBox* pUpperBox = &rBox;
    USHORT nDelPos = rTblBoxes.C40_GETPOS( SwTableBox, pUpperBox );
    pUpperBox = rBox.GetUpper()->GetUpper();
    const SvxBoxItem& rBoxItem = rBox.GetFrmFmt()->GetBox();

    // dann die unteren/oberen Kanten
    if( rBoxItem.GetTop() || rBoxItem.GetBottom() )
    {
        bChgd = FALSE;
        const SwTableLines* pTblLns;
        if( pUpperBox )
            pTblLns = &pUpperBox->GetTabLines();
        else
            pTblLns = &rTbl.GetTabLines();

        USHORT nLnPos = pTblLns->GetPos( pLine );

        // bestimme die Attr.Position der akt. zu loeschenden Box
        // und suche dann in der unteren / oberen Line die entspr.
        // Gegenstuecke
        SwTwips nBoxStt = 0;
        for( USHORT n = 0; n < nDelPos; ++n )
            nBoxStt += rTblBoxes[ n ]->GetFrmFmt()->GetFrmSize().GetWidth();
        SwTwips nBoxWidth = rBox.GetFrmFmt()->GetFrmSize().GetWidth();

        SwTableBox *pPrvBox = 0, *pNxtBox = 0;
        if( nLnPos )        // Vorgaenger?
            pPrvBox = ::lcl_FndNxtPrvDelBox( *pTblLns, nBoxStt, nBoxWidth,
                                nLnPos, FALSE, pAllDelBoxes, pCurPos );

        if( nLnPos + 1 < pTblLns->Count() )     // Nachfolger?
            pNxtBox = ::lcl_FndNxtPrvDelBox( *pTblLns, nBoxStt, nBoxWidth,
                                nLnPos, TRUE, pAllDelBoxes, pCurPos );

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
                bChgd = TRUE;
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


BOOL SwTable::DeleteSel( SwDoc* pDoc, const SwSelBoxes& rBoxes, SwUndo* pUndo,
                         const BOOL bDelMakeFrms, const BOOL bCorrBorder )
{
    ASSERT( pDoc && rBoxes.Count(), "keine gueltigen Werte" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return FALSE;

    // es darf nie die gesamte Tabelle geloescht werden
    SwNodes& rNds = pDoc->GetNodes();
    if( rBoxes[0]->GetSttIdx()-1 == pTblNd->GetIndex() &&
        rBoxes[rBoxes.Count()-1]->GetSttNd()->EndOfSectionIndex()+1
        == pTblNd->EndOfSectionIndex() )
        return FALSE;

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    //Lines fuer das Layout-Update herausuchen.
    _FndBox aFndBox( 0, 0 );
    if ( bDelMakeFrms )
    {
        aFndBox.SetTableLines( rBoxes, *this );
        aFndBox.DelFrms( *this );
    }
    aFndBox.SaveChartData( *this );

    SwShareBoxFmts aShareFmts;

    // erst die Umrandung umsetzen, dann loeschen
    if( bCorrBorder )
    {
        SwSelBoxes aBoxes;
        aBoxes.Insert( &rBoxes );
        for( USHORT n = 0; n < aBoxes.Count(); ++n )
            ::lcl_SaveUpperLowerBorder( *this, *rBoxes[ n ], aShareFmts,
                                        &aBoxes, &n );
    }

    for( USHORT n = 0; n < rBoxes.Count(); ++n )
        _DeleteBox( *this, rBoxes[n], pUndo, TRUE, bCorrBorder, &aShareFmts );

    // dann raeume die Struktur aller Lines auf
    GCLines();

    if( bDelMakeFrms && aFndBox.AreLinesToRestore( *this ) )
        aFndBox.MakeFrms( *this );
    aFndBox.RestoreChartData( *this );
    return TRUE;
}


// ---------------------------------------------------------------

BOOL SwTable::SplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, USHORT nCnt )
{
    ASSERT( pDoc && rBoxes.Count() && nCnt, "keine gueltigen Werte" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return FALSE;

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    //Lines fuer das Layout-Update herausuchen.
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rBoxes, *this );
    aFndBox.DelFrms( *this );
    aFndBox.SaveChartData( *this );

    for( USHORT n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pSelBox = *( rBoxes.GetData() + n );
        ASSERT( pSelBox, "Box steht nicht in der Tabelle" );

        // dann fuege in die Box nCnt neue Zeilen ein
        SwTableLine* pInsLine = pSelBox->GetUpper();
        SwTableBoxFmt* pFrmFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();

        // Hoehe der Line beachten, gegebenenfalls neu setzen
        SwFmtFrmSize aFSz( pInsLine->GetFrmFmt()->GetFrmSize() );
        BOOL bChgLineSz = 0 != aFSz.GetHeight();
        if( bChgLineSz )
            aFSz.SetHeight( aFSz.GetHeight() / (nCnt + 1) );

        SwTableBox* pNewBox = new SwTableBox( pFrmFmt, nCnt, pInsLine );
        USHORT nBoxPos = pInsLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSelBox );
        pInsLine->GetTabBoxes().Remove( nBoxPos );  // alte loeschen
        pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pNewBox, nBoxPos );

        // Hintergrund- / Rand Attribut loeschen
        SwTableBox* pLastBox = pSelBox;         // zum verteilen der TextNodes !!
        // sollte Bereiche in der Box stehen, dann bleibt sie so bestehen
        // !! FALLS DAS GEAENDERT WIRD MUSS DAS UNDO ANGEPASST WERDEN !!!
        BOOL bMoveNodes = TRUE;
        {
            ULONG nSttNd = pLastBox->GetSttIdx() + 1,
                    nEndNd = pLastBox->GetSttNd()->EndOfSectionIndex();
            while( nSttNd < nEndNd )
                if( !pDoc->GetNodes()[ nSttNd++ ]->IsTxtNode() )
                {
                    bMoveNodes = FALSE;
                    break;
                }
        }

        SwTableBoxFmt* pCpyBoxFrmFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();
        BOOL bChkBorder = 0 != pCpyBoxFrmFmt->GetBox().GetTop();
        if( bChkBorder )
            pCpyBoxFrmFmt = (SwTableBoxFmt*)pSelBox->ClaimFrmFmt();

        for( USHORT i = 0; i <= nCnt; ++i )
        {
            // also erstmal eine neue Linie in der neuen Box
            SwTableLine* pNewLine = new SwTableLine(
                    (SwTableLineFmt*)pInsLine->GetFrmFmt(), 1, pNewBox );
            if( bChgLineSz )
            {
                pNewLine->ClaimFrmFmt()->SetAttr( aFSz );
            }

            pNewBox->GetTabLines().C40_INSERT( SwTableLine, pNewLine, i );
            // dann eine neue Box in der Line
            if( !i )        // haenge die originale Box ein
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
                    pCpyBoxFrmFmt->SetAttr( aTmp );
                    bChkBorder = FALSE;
                }

                if( bMoveNodes )
                {
                    const SwNode* pEndNd = pLastBox->GetSttNd()->EndOfSectionNode();
                    if( pLastBox->GetSttIdx()+2 != pEndNd->GetIndex() )
                    {
                        // TextNodes verschieben
                        SwNodeRange aRg( *pLastBox->GetSttNd(), +2, *pEndNd );
                        pLastBox = pNewLine->GetTabBoxes()[0];  // neu setzen
                        SwNodeIndex aInsPos( *pLastBox->GetSttNd(), 1 );
                        pDoc->GetNodes()._MoveNodes(aRg, pDoc->GetNodes(), aInsPos, FALSE);
                        pDoc->GetNodes().Delete( aInsPos, 1 ); // den leeren noch loeschen
                    }
                }
            }
        }
        // in Boxen mit Lines darf es nur noch Size/Fillorder geben
        pFrmFmt = (SwTableBoxFmt*)pNewBox->ClaimFrmFmt();
        pFrmFmt->ResetAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
        pFrmFmt->ResetAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
    }

    //Layout updaten
    aFndBox.MakeFrms( *this );
    aFndBox.RestoreChartData( *this );
    CHECKBOXWIDTH
    return TRUE;
}

BOOL SwTable::SplitCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, USHORT nCnt )
{
    ASSERT( pDoc && rBoxes.Count() && nCnt, "keine gueltigen Werte" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return FALSE;

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    //Lines fuer das Layout-Update herausuchen.
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( rBoxes, *this );
    aFndBox.DelFrms( *this );
    aFndBox.SaveChartData( *this );

    _CpyTabFrms aFrmArr;
    SvPtrarr aLastBoxArr;
    USHORT nFndPos;
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pSelBox = *( rBoxes.GetData() + n );
        ASSERT( pSelBox, "Box steht nicht in der Tabelle" );

        // dann teile die Box nCnt in nCnt Boxen
        SwTableLine* pInsLine = pSelBox->GetUpper();
        USHORT nBoxPos = pInsLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSelBox );

        // suche das FrmFmt im Array aller Frame-Formate
        SwTableBoxFmt* pLastBoxFmt;
        _CpyTabFrm aFindFrm( (SwTableBoxFmt*)pSelBox->GetFrmFmt() );
        if( !aFrmArr.Seek_Entry( aFindFrm, &nFndPos ))
        {
            // aender das FrmFmt
            aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pSelBox->ClaimFrmFmt();
            SwTwips nBoxSz = aFindFrm.pNewFrmFmt->GetFrmSize().GetWidth();
            SwTwips nNewBoxSz = nBoxSz / ( nCnt + 1 );
            aFindFrm.pNewFrmFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                                        nNewBoxSz, 0 ) );
            aFrmArr.Insert( aFindFrm );

            pLastBoxFmt = aFindFrm.pNewFrmFmt;
            if( nBoxSz != ( nNewBoxSz * (nCnt + 1)))
            {
                // es bleibt ein Rest, also muss fuer die letzte Box ein
                // eigenes Format definiert werden
                pLastBoxFmt = new SwTableBoxFmt( *aFindFrm.pNewFrmFmt );
                pLastBoxFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE,
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

        // dann fuege mal an der Position die neuen Boxen ein
        for( USHORT i = 1; i < nCnt; ++i )
            ::_InsTblBox( pDoc, pTblNd, pInsLine, aFindFrm.pNewFrmFmt,
                        pSelBox, nBoxPos + i ); // dahinter einfuegen

        ::_InsTblBox( pDoc, pTblNd, pInsLine, pLastBoxFmt,
                    pSelBox, nBoxPos + nCnt );  // dahinter einfuegen

        // Sonderbehandlung fuer die Umrandung:
        const SvxBoxItem& rBoxItem = aFindFrm.pNewFrmFmt->GetBox();
        if( rBoxItem.GetRight() )
        {
            pInsLine->GetTabBoxes()[ nBoxPos + nCnt ]->ClaimFrmFmt();

            SvxBoxItem aTmp( rBoxItem );
            aTmp.SetLine( 0, BOX_LINE_RIGHT );
            aFindFrm.pNewFrmFmt->SetAttr( aTmp );

            // und dann das Format aus dem "cache" entfernen
            for( USHORT i = aFrmArr.Count(); i; )
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

    //Layout updaten
    aFndBox.MakeFrms( *this );
    aFndBox.RestoreChartData( *this );
    CHECKBOXWIDTH
    return TRUE;
}

// ---------------------------------------------------------------

/*
    ----------------------- >> MERGE << ------------------------
     Algorythmus:
        ist in der _FndBox nur eine Line angegeben, nehme die Line
        und teste die Anzahl der Boxen
        - ist mehr als 1 Box angegeben, so wird auf Boxenebene zusammen-
            gefasst, d.H. die neue Box wird so Breit wie die alten.
            - Alle Lines die ueber/unter dem Bereich liegen werden in die
            Box als Line + Box mit Lines eingefuegt
            - Alle Lines die vor/hinter dem Bereich liegen werden in
            die Boxen Left/Right eingetragen

    ----------------------- >> MERGE << ------------------------
*/

void lcl_CpyLines( USHORT nStt, USHORT nEnd,
                                SwTableLines& rLines,
                                SwTableBox* pInsBox,
                                USHORT nPos = USHRT_MAX )
{
    for( USHORT n = nStt; n < nEnd; ++n )
        rLines[n]->SetUpper( pInsBox );
    if( USHRT_MAX == nPos )
        nPos = pInsBox->GetTabLines().Count();
    pInsBox->GetTabLines().Insert( &rLines, nPos, nStt, nEnd );
    rLines.Remove( nStt, nEnd - nStt );
}

void lcl_CpyBoxes( USHORT nStt, USHORT nEnd,
                                SwTableBoxes& rBoxes,
                                SwTableLine* pInsLine,
                                USHORT nPos = USHRT_MAX )
{
    for( USHORT n = nStt; n < nEnd; ++n )
        rBoxes[n]->SetUpper( pInsLine );
    if( USHRT_MAX == nPos )
        nPos = pInsLine->GetTabBoxes().Count();
    pInsLine->GetTabBoxes().Insert( &rBoxes, nPos, nStt, nEnd );
    rBoxes.Remove( nStt, nEnd - nStt );
}

void lcl_CalcWidth( SwTableBox* pBox )
{
    // Annahme: jede Line in der Box ist gleich gross
    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
    ASSERT( pBox->GetTabLines().Count(), "Box hat keine Lines" );

    SwTableLine* pLine = pBox->GetTabLines()[0];
    ASSERT( pLine, "Box steht in keiner Line" );

    long nWidth = 0;
    for( USHORT n = 0; n < pLine->GetTabBoxes().Count(); ++n )
        nWidth += pLine->GetTabBoxes()[n]->GetFrmFmt()->GetFrmSize().GetWidth();

    pFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));

    // in Boxen mit Lines darf es nur noch Size/Fillorder geben
    pFmt->ResetAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
    pFmt->ResetAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
}



struct _InsULPara
{
    SwTableNode* pTblNd;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    BOOL bUL_LR : 1;        // Upper-Lower(TRUE) oder Left-Right(FALSE) ?
    BOOL bUL : 1;           // Upper-Left(TRUE) oder Lower-Right(FALSE) ?

    SwTableBox* pLeftBox;
    SwTableBox* pRightBox;
    SwTableBox* pMergeBox;

    _InsULPara( SwTableNode* pTNd, BOOL bUpperLower, BOOL bUpper,
                SwTableBox* pLeft, SwTableBox* pMerge, SwTableBox* pRight,
                SwTableLine* pLine=0, SwTableBox* pBox=0 )
        : pTblNd( pTNd ), pInsLine( pLine ), pInsBox( pBox ),
        pLeftBox( pLeft ), pMergeBox( pMerge ), pRightBox( pRight )
        {   bUL_LR = bUpperLower; bUL = bUpper; }

    void SetLeft( SwTableBox* pBox=0 )
        { bUL_LR = FALSE;   bUL = TRUE; if( pBox ) pInsBox = pBox; }
    void SetRight( SwTableBox* pBox=0 )
        { bUL_LR = FALSE;   bUL = FALSE; if( pBox ) pInsBox = pBox; }
    void SetUpper( SwTableLine* pLine=0 )
        { bUL_LR = TRUE;    bUL = TRUE;  if( pLine ) pInsLine = pLine; }
    void SetLower( SwTableLine* pLine=0 )
        { bUL_LR = TRUE;    bUL = FALSE; if( pLine ) pInsLine = pLine; }
};


BOOL lcl_Merge_MoveBox( const _FndBox*& rpFndBox, void* pPara )
{
    _InsULPara* pULPara = (_InsULPara*)pPara;
    SwTableBoxes* pBoxes;

    USHORT nStt = 0, nEnd = rpFndBox->GetLines().Count();
    USHORT nInsPos = USHRT_MAX;
    if( !pULPara->bUL_LR )  // Left/Right
    {
        USHORT nPos;
        SwTableBox* pFndBox = (SwTableBox*)rpFndBox->GetBox();
        pBoxes = &pFndBox->GetUpper()->GetTabBoxes();
        if( pULPara->bUL )  // Left ?
        {
            // gibt es noch davor Boxen, dann move sie
            if( 0 != ( nPos = pBoxes->C40_GETPOS( SwTableBox, pFndBox )) )
                lcl_CpyBoxes( 0, nPos, *pBoxes, pULPara->pInsLine );
        }
        else                // Right
            // gibt es noch dahinter Boxen, dann move sie
            if( (nPos = pBoxes->C40_GETPOS( SwTableBox, pFndBox )) +1 < pBoxes->Count() )
            {
                nInsPos = pULPara->pInsLine->GetTabBoxes().Count();
                lcl_CpyBoxes( nPos+1, pBoxes->Count(),
                                    *pBoxes, pULPara->pInsLine );
            }
    }
    // Upper/Lower und gehts noch tiefer ??
    else if( rpFndBox->GetLines().Count() )
    {
        // suche nur die Line, ab der Verschoben werden muss
        nStt = pULPara->bUL ? 0 : rpFndBox->GetLines().Count()-1;
        nEnd = nStt+1;
    }

    pBoxes = &pULPara->pInsLine->GetTabBoxes();

    // geht es noch eine weitere Stufe runter?
    if( rpFndBox->GetBox()->GetTabLines().Count() )
    {
        SwTableBox* pBox = new SwTableBox(
                (SwTableBoxFmt*)rpFndBox->GetBox()->GetFrmFmt(), 0, pULPara->pInsLine );
        _InsULPara aPara( *pULPara );
        aPara.pInsBox = pBox;
        ((_FndBox*)rpFndBox)->GetLines().ForEach( nStt, nEnd,
                                                &lcl_Merge_MoveLine, &aPara );
        if( pBox->GetTabLines().Count() )
        {
            if( USHRT_MAX == nInsPos )
                nInsPos = pBoxes->Count();
            pBoxes->C40_INSERT( SwTableBox, pBox, nInsPos );
            lcl_CalcWidth( pBox );      // bereche die Breite der Box
        }
        else
            delete pBox;
    }
    return TRUE;
}

BOOL lcl_Merge_MoveLine( const _FndLine*& rpFndLine, void* pPara )
{
    _InsULPara* pULPara = (_InsULPara*)pPara;
    SwTableLines* pLines;

    USHORT nStt = 0, nEnd = rpFndLine->GetBoxes().Count();
    USHORT nInsPos = USHRT_MAX;
    if( pULPara->bUL_LR )   // UpperLower ?
    {
        USHORT nPos;
        SwTableLine* pFndLn = (SwTableLine*)rpFndLine->GetLine();
        pLines = pFndLn->GetUpper() ?
                        &pFndLn->GetUpper()->GetTabLines() :
                        &pULPara->pTblNd->GetTable().GetTabLines();

        SwTableBox* pLBx = rpFndLine->GetBoxes()[0]->GetBox();
        SwTableBox* pRBx = rpFndLine->GetBoxes()[
                            rpFndLine->GetBoxes().Count()-1]->GetBox();
        USHORT nLeft = pFndLn->GetTabBoxes().C40_GETPOS( SwTableBox, pLBx );
        USHORT nRight = pFndLn->GetTabBoxes().C40_GETPOS( SwTableBox, pRBx );

//      if( ( nLeft && nRight+1 < pFndLn->GetTabBoxes().Count() ) ||
//          ( !nLeft && nRight+1 >= pFndLn->GetTabBoxes().Count() ) )
        if( !nLeft || nRight == pFndLn->GetTabBoxes().Count() )
        {
            if( pULPara->bUL )  // Upper ?
            {
                // gibt es noch davor Zeilen, dann move sie
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pULPara->pInsBox );
            }
            else
                // gibt es noch dahinter Zeilen, dann move sie
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                {
                    nInsPos = pULPara->pInsBox->GetTabLines().Count();
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pULPara->pInsBox );
                }
        }
        else if( nLeft )
        {
            // es gibt links noch weitere Boxen, also setze Left-
            // und Merge-Box in eine Box und Line, fuege davor/dahinter
            // eine Line mit Box ein, in die die oberen/unteren Lines
            // eingefuegt werden
            SwTableLine* pInsLine = pULPara->pLeftBox->GetUpper();
            SwTableBox* pLMBox = new SwTableBox(
                (SwTableBoxFmt*)pULPara->pLeftBox->GetFrmFmt(), 0, pInsLine );
            SwTableLine* pLMLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pLMBox );
            pLMLn->ClaimFrmFmt()->ResetAttr( RES_FRM_SIZE );

            pLMBox->GetTabLines().C40_INSERT( SwTableLine, pLMLn, 0 );

            lcl_CpyBoxes( 0, 2, pInsLine->GetTabBoxes(), pLMLn );

            pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pLMBox, 0 );

            if( pULPara->bUL )  // Upper ?
            {
                // gibt es noch davor Zeilen, dann move sie
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pLMBox, 0 );
            }
            else
                // gibt es noch dahinter Zeilen, dann move sie
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pLMBox );
            lcl_CalcWidth( pLMBox );        // bereche die Breite der Box
        }
        else if( nRight+1 < pFndLn->GetTabBoxes().Count() )
        {
            // es gibt rechts noch weitere Boxen, also setze Right-
            // und Merge-Box in eine Box und Line, fuege davor/dahinter
            // eine Line mit Box ein, in die die oberen/unteren Lines
            // eingefuegt werden
            SwTableLine* pInsLine = pULPara->pRightBox->GetUpper();
            SwTableBox* pRMBox;
            if( pULPara->pLeftBox->GetUpper() == pInsLine )
            {
                pRMBox = new SwTableBox(
                    (SwTableBoxFmt*)pULPara->pRightBox->GetFrmFmt(), 0, pInsLine );
                SwTableLine* pRMLn = new SwTableLine(
                    (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pRMBox );
                pRMLn->ClaimFrmFmt()->ResetAttr( RES_FRM_SIZE );
                pRMBox->GetTabLines().C40_INSERT( SwTableLine, pRMLn, 0 );

                lcl_CpyBoxes( 1, 3, pInsLine->GetTabBoxes(), pRMLn );

                pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox, 0 );
            }
            else
            {
                // Left und Merge wurden schon zusammengefuegt, also move
                // Right auch mit in die Line

                pInsLine = pULPara->pLeftBox->GetUpper();
                USHORT nMvPos = pULPara->pRightBox->GetUpper()->GetTabBoxes().
                                    C40_GETPOS( SwTableBox, pULPara->pRightBox );
                lcl_CpyBoxes( nMvPos, nMvPos+1,
                            pULPara->pRightBox->GetUpper()->GetTabBoxes(),
                            pInsLine );
                pRMBox = pInsLine->GetUpper();

                // sind schon Lines vorhanden, dann muessen diese in eine
                // neue Line und Box
                nMvPos = pRMBox->GetTabLines().C40_GETPOS( SwTableLine, pInsLine );
                if( pULPara->bUL ? nMvPos
                                : nMvPos+1 < pRMBox->GetTabLines().Count() )
                {
                    // alle Lines zu einer neuen Line und Box zusammenfassen
                    SwTableLine* pNewLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 1, pRMBox );
                    pNewLn->ClaimFrmFmt()->ResetAttr( RES_FRM_SIZE );
                    pRMBox->GetTabLines().C40_INSERT( SwTableLine, pNewLn,
                            pULPara->bUL ? nMvPos : nMvPos+1 );
                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox, 0 );

                    USHORT nPos1, nPos2;
                    if( pULPara->bUL )
                        nPos1 = 0,
                        nPos2 = nMvPos;
                    else
                        nPos1 = nMvPos+2,
                        nPos2 = pNewLn->GetUpper()->GetTabLines().Count();

                    lcl_CpyLines( nPos1, nPos2,
                                pNewLn->GetUpper()->GetTabLines(), pRMBox );
                    lcl_CalcWidth( pRMBox );        // bereche die Breite der Box

                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox,
                                    pNewLn->GetTabBoxes().Count() );
                }
            }
            if( pULPara->bUL )  // Upper ?
            {
                // gibt es noch davor Zeilen, dann move sie
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pRMBox, 0 );
            }
            else
                // gibt es noch dahinter Zeilen, dann move sie
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pRMBox );
            lcl_CalcWidth( pRMBox );        // bereche die Breite der Box
        }
        else
            ASSERT( FALSE , "Was denn nun" );
    }
    // Left/Right
    else
    {
        // suche nur die Line, ab der Verschoben werden muss
        nStt = pULPara->bUL ? 0 : rpFndLine->GetBoxes().Count()-1;
        nEnd = nStt+1;
    }
    pLines = &pULPara->pInsBox->GetTabLines();

    SwTableLine* pNewLine = new SwTableLine(
        (SwTableLineFmt*)rpFndLine->GetLine()->GetFrmFmt(), 0, pULPara->pInsBox );
    _InsULPara aPara( *pULPara );       // kopieren
    aPara.pInsLine = pNewLine;
    ((_FndLine*)rpFndLine)->GetBoxes().ForEach( nStt, nEnd,
                                                &lcl_Merge_MoveBox, &aPara );
    if( pNewLine->GetTabBoxes().Count() )
    {
        if( USHRT_MAX == nInsPos )
            nInsPos = pLines->Count();
        pLines->C40_INSERT( SwTableLine, pNewLine, nInsPos );
    }
    else
        delete pNewLine;

    return TRUE;
}


BOOL SwTable::Merge( SwDoc* pDoc, const SwSelBoxes& rBoxes,
                    SwTableBox* pMergeBox, SwUndoTblMerge* pUndo )
{
    ASSERT( pDoc && rBoxes.Count() && pMergeBox, "keine gueltigen Werte" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return FALSE;

    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return FALSE;

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    if( pUndo )
        pUndo->SetSelBoxes( rBoxes );

    //Lines fuer das Layout-Update herausuchen.
    aFndBox.SetTableLines( *this );
    aFndBox.DelFrms( *this );
    aFndBox.SaveChartData( *this );

    _FndBox* pFndBox = &aFndBox;
    while( 1 == pFndBox->GetLines().Count() &&
            1 == pFndBox->GetLines()[0]->GetBoxes().Count() )
        pFndBox = pFndBox->GetLines()[0]->GetBoxes()[0];

    SwTableLine* pInsLine = new SwTableLine(
                (SwTableLineFmt*)pFndBox->GetLines()[0]->GetLine()->GetFrmFmt(), 0,
                !pFndBox->GetUpper() ? 0 : pFndBox->GetBox() );
    pInsLine->ClaimFrmFmt()->ResetAttr( RES_FRM_SIZE );

    // trage die neue Line ein
    SwTableLines* pLines =  pFndBox->GetUpper() ?
                  &pFndBox->GetBox()->GetTabLines() :  &GetTabLines();

    SwTableLine* pNewLine = pFndBox->GetLines()[0]->GetLine();
    USHORT nInsPos = pLines->C40_GETPOS( SwTableLine, pNewLine );
    pLines->C40_INSERT( SwTableLine, pInsLine, nInsPos );

    SwTableBox* pLeft = new SwTableBox( (SwTableBoxFmt*)pMergeBox->GetFrmFmt(), 0, pInsLine );
    SwTableBox* pRight = new SwTableBox( (SwTableBoxFmt*)pMergeBox->GetFrmFmt(), 0, pInsLine );
    pMergeBox->SetUpper( pInsLine );
    pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pLeft, 0 );
    pLeft->ClaimFrmFmt();
    pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pMergeBox, 1 );
    pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pRight, 2 );
    pRight->ClaimFrmFmt();

    // in diese kommen alle Lines, die ueber dem selektierten Bereich stehen
    // Sie bilden also eine Upper/Lower Line
    _InsULPara aPara( pTblNd, TRUE, TRUE, pLeft, pMergeBox, pRight, pInsLine );

    // move die oben/unten ueberhaengenden Lines vom selektierten Bereich
    pFndBox->GetLines()[0]->GetBoxes().ForEach( &lcl_Merge_MoveBox,
                                                &aPara );
    aPara.SetLower( pInsLine );
    USHORT nEnd = pFndBox->GetLines().Count()-1;
    pFndBox->GetLines()[nEnd]->GetBoxes().ForEach( &lcl_Merge_MoveBox,
                                                    &aPara );

    // move die links/rechts hereinreichenden Boxen vom selektierten Bereich
    aPara.SetLeft( pLeft );
    pFndBox->GetLines().ForEach( &lcl_Merge_MoveLine, &aPara );

    aPara.SetRight( pRight );
    pFndBox->GetLines().ForEach( &lcl_Merge_MoveLine, &aPara );

    if( !pLeft->GetTabLines().Count() )
        _DeleteBox( *this, pLeft, 0, FALSE, FALSE );
    else
    {
        lcl_CalcWidth( pLeft );     // bereche die Breite der Box
        if( pUndo && pLeft->GetSttNd() )
            pUndo->AddNewBox( pLeft->GetSttIdx() );
    }
    if( !pRight->GetTabLines().Count() )
        _DeleteBox( *this, pRight, 0, FALSE, FALSE );
    else
    {
        lcl_CalcWidth( pRight );        // bereche die Breite der Box
        if( pUndo && pRight->GetSttNd() )
            pUndo->AddNewBox( pRight->GetSttIdx() );
    }

    DeleteSel( pDoc, rBoxes, 0, FALSE, FALSE );

    // dann raeume die Struktur dieser Line noch mal auf:
    // generell alle Aufraeumen
    GCLines();

    GetTabLines()[0]->GetTabBoxes().ForEach( &lcl_BoxSetHeadCondColl, 0 );

    aFndBox.MakeFrms( *this );
    aFndBox.RestoreChartData( *this );
    CHECKBOXWIDTH
    return TRUE;
}

// ---------------------------------------------------------------

USHORT lcl_GetBoxOffset( const _FndBox& rBox )
{
    // suche die erste Box
    const _FndBox* pFirstBox = &rBox;
    while( pFirstBox->GetLines().Count() )
        pFirstBox = pFirstBox->GetLines()[ 0 ]->GetBoxes()[ 0 ];

    USHORT nRet = 0;
    // dann ueber die Lines nach oben die Position bestimmen
    const SwTableBox* pBox = pFirstBox->GetBox();
    do {
        const SwTableBoxes& rBoxes = pBox->GetUpper()->GetTabBoxes();
        const SwTableBox* pCmp;
        for( USHORT n = 0; pBox != ( pCmp = rBoxes[ n ] ); ++n )
            nRet += (USHORT) pCmp->GetFrmFmt()->GetFrmSize().GetWidth();
        pBox = pBox->GetUpper()->GetUpper();
    } while( pBox );
    return nRet;
}

USHORT lcl_GetLineWidth( const _FndLine& rLine )
{
    USHORT nRet = 0;
    for( USHORT n = rLine.GetBoxes().Count(); n; )
        nRet += (USHORT)rLine.GetBoxes()[ --n ]->GetBox()->GetFrmFmt()
                        ->GetFrmSize().GetWidth();
    return nRet;
}

BOOL lcl_CopyBoxToDoc( const _FndBox*& rpFndBox, void* pPara )
{
    _CpyPara* pCpyPara = (_CpyPara*)pPara;

    // Berechne die neue Size
    ULONG nSize = pCpyPara->nNewSize;
    nSize *= rpFndBox->GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();
    nSize /= pCpyPara->nOldSize;

    // suche das Frame-Format in der Liste aller Frame-Formate
    _CpyTabFrm aFindFrm( (SwTableBoxFmt*)rpFndBox->GetBox()->GetFrmFmt() );

    SwFmtFrmSize aFrmSz;
    USHORT nFndPos;
    if( !pCpyPara->rTabFrmArr.Seek_Entry( aFindFrm, &nFndPos ) ||
        ( aFrmSz = ( aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ]).pNewFrmFmt->
            GetFrmSize()).GetWidth() != (SwTwips)nSize )
    {
        // es ist noch nicht vorhanden, also kopiere es
        aFindFrm.pNewFrmFmt = pCpyPara->pDoc->MakeTableBoxFmt();
        aFindFrm.pNewFrmFmt->CopyAttrs( *rpFndBox->GetBox()->GetFrmFmt() );
        if( !pCpyPara->bCpyCntnt )
            aFindFrm.pNewFrmFmt->ResetAttr(  RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
        aFrmSz.SetWidth( nSize );
        aFindFrm.pNewFrmFmt->SetAttr( aFrmSz );
        pCpyPara->rTabFrmArr.Insert( aFindFrm );
    }

    SwTableBox* pBox;
    if( rpFndBox->GetLines().Count() )
    {
        pBox = new SwTableBox( aFindFrm.pNewFrmFmt,
                    rpFndBox->GetLines().Count(), pCpyPara->pInsLine );
        pCpyPara->pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, pCpyPara->nInsPos++ );
        _CpyPara aPara( *pCpyPara, pBox );
        aPara.nNewSize = nSize;     // hole die Groesse
        ((_FndBox*)rpFndBox)->GetLines().ForEach( &lcl_CopyLineToDoc, &aPara );
    }
    else
    {
        // erzeuge eine leere Box
        pCpyPara->pDoc->GetNodes().InsBoxen( pCpyPara->pTblNd, pCpyPara->pInsLine,
                        aFindFrm.pNewFrmFmt,
                        (SwTxtFmtColl*)pCpyPara->pDoc->GetDfltTxtFmtColl(),
                        0, pCpyPara->nInsPos );

        if( pCpyPara->bCpyCntnt )
        {
            // dann kopiere mal den Inhalt in diese leere Box
            pBox = pCpyPara->pInsLine->GetTabBoxes()[ pCpyPara->nInsPos ];

            // der Inhalt kopiert wird, dann koennen auch Formeln&Values
            // kopiert werden.
            {
                SfxItemSet aBoxAttrSet( pCpyPara->pDoc->GetAttrPool(),
                                        RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
                aBoxAttrSet.Put( rpFndBox->GetBox()->GetFrmFmt()->GetAttrSet() );
                if( aBoxAttrSet.Count() )
                {
                    const SfxPoolItem* pItem;
                    SvNumberFormatter* pN = pCpyPara->pDoc->GetNumberFormatter( FALSE );
                    if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == aBoxAttrSet.
                        GetItemState( RES_BOXATR_FORMAT, FALSE, &pItem ) )
                    {
                        ULONG nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
                        ULONG nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                        if( nNewIdx != nOldIdx )
                            aBoxAttrSet.Put( SwTblBoxNumFormat( nNewIdx ));
                    }
                    pBox->ClaimFrmFmt()->SetAttr( aBoxAttrSet );
                }
            }
            SwDoc* pFromDoc = rpFndBox->GetBox()->GetFrmFmt()->GetDoc();
            SwNodeRange aCpyRg( *rpFndBox->GetBox()->GetSttNd(), 1,
                        *rpFndBox->GetBox()->GetSttNd()->EndOfSectionNode() );
            SwNodeIndex aInsIdx( *pBox->GetSttNd(), 1 );

            pFromDoc->CopyWithFlyInFly( aCpyRg, aInsIdx, FALSE );
            // den initialen TextNode loeschen
            pCpyPara->pDoc->GetNodes().Delete( aInsIdx, 1 );
        }
        ++pCpyPara->nInsPos;
    }
    return TRUE;
}

BOOL lcl_CopyLineToDoc( const _FndLine*& rpFndLine, void* pPara )
{
    _CpyPara* pCpyPara = (_CpyPara*)pPara;

    // suche das Format in der Liste aller Formate
    _CpyTabFrm aFindFrm( (SwTableBoxFmt*)rpFndLine->GetLine()->GetFrmFmt() );
    USHORT nFndPos;
    if( !pCpyPara->rTabFrmArr.Seek_Entry( aFindFrm, &nFndPos ))
    {
        // es ist noch nicht vorhanden, also kopiere es
        aFindFrm.pNewFrmFmt = (SwTableBoxFmt*)pCpyPara->pDoc->MakeTableLineFmt();
        aFindFrm.pNewFrmFmt->CopyAttrs( *rpFndLine->GetLine()->GetFrmFmt() );
        pCpyPara->rTabFrmArr.Insert( aFindFrm );
    }
    else
        aFindFrm = pCpyPara->rTabFrmArr[ nFndPos ];

    SwTableLine* pNewLine = new SwTableLine( (SwTableLineFmt*)aFindFrm.pNewFrmFmt,
                        rpFndLine->GetBoxes().Count(), pCpyPara->pInsBox );
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

    // berechne die neue Size der Boxen einer Line
    if( rpFndLine->GetBoxes().Count() ==
                    rpFndLine->GetLine()->GetTabBoxes().Count() )
    {
        // hole die Size vom Parent
        const SwFrmFmt* pFmt;

        if( rpFndLine->GetLine()->GetUpper() )
            pFmt = rpFndLine->GetLine()->GetUpper()->GetFrmFmt();
        else
            pFmt = pCpyPara->pTblNd->GetTable().GetFrmFmt();
        aPara.nOldSize = pFmt->GetFrmSize().GetWidth();
    }
    else
        // errechne sie
        for( USHORT n = 0; n < rpFndLine->GetBoxes().Count(); ++n )
            aPara.nOldSize += rpFndLine->GetBoxes()[n]
                        ->GetBox()->GetFrmFmt()->GetFrmSize().GetWidth();

    ((_FndLine*)rpFndLine)->GetBoxes().ForEach( &lcl_CopyBoxToDoc, &aPara );
    return TRUE;
}

BOOL SwTable::CopyHeadlineIntoTable( SwTableNode& rTblNd )
{
    // suche alle Boxen / Lines
    SwSelBoxes aSelBoxes;
    SwTableBox* pBox = GetTabSortBoxes()[ 0 ];
    pBox = GetTblBox( pBox->GetSttNd()->FindStartNode()->GetIndex() + 1 );
    SelLineFromBox( pBox, aSelBoxes, TRUE );

    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( aSelBoxes, &aFndBox );
        ((SwTableLines&)GetTabLines()).ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return FALSE;

    {
        // Tabellen-Formeln in die relative Darstellung umwandeln
        SwTableFmlUpdate aMsgHnt( this );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        GetFrmFmt()->GetDoc()->UpdateTblFlds( &aMsgHnt );
    }

    _CpyTabFrms aCpyFmt;
    _CpyPara aPara( &rTblNd, 1, aCpyFmt, TRUE );
    aPara.nNewSize = aPara.nOldSize = rTblNd.GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
    // dann kopiere mal
    aFndBox.GetLines().ForEach( &lcl_CopyLineToDoc, &aPara );

    return TRUE;
}

BOOL SwTable::MakeCopy( SwDoc* pInsDoc, const SwPosition& rPos,
                        const SwSelBoxes& rSelBoxes, BOOL bCpyNds,
                        BOOL bCpyName ) const
{
    // suche alle Boxen / Lines
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rSelBoxes, &aFndBox );
        ((SwTableLines&)GetTabLines()).ForEach( &_FndLineCopyCol, &aPara );
    }
    if( !aFndBox.GetLines().Count() )
        return FALSE;

    // erst die Poolvorlagen fuer die Tabelle kopieren, damit die dann
    // wirklich kopiert und damit die gueltigen Werte haben.
    SwDoc* pSrcDoc = GetFrmFmt()->GetDoc();
    if( pSrcDoc != pInsDoc )
    {
        pInsDoc->CopyTxtColl( *pSrcDoc->GetTxtCollFromPool( RES_POOLCOLL_TABLE ) );
        pInsDoc->CopyTxtColl( *pSrcDoc->GetTxtCollFromPool( RES_POOLCOLL_TABLE_HDLN ) );
    }

    SwTable* pNewTbl = (SwTable*)pInsDoc->InsertTable( rPos, 1, 1,
                    GetFrmFmt()->GetHoriOrient().GetHoriOrient() );
    if( !pNewTbl )
        return FALSE;

    SwNodeIndex aIdx( rPos.nNode, -1 );
    SwTableNode* pTblNd = aIdx.GetNode().FindTableNode();
    aIdx++;
    ASSERT( pTblNd, "wo ist denn nun der TableNode?" );

    pTblNd->GetTable().SetHeadlineRepeat( IsHeadlineRepeat() );

    if( IS_TYPE( SwDDETable, this ))
    {
        // es wird eine DDE-Tabelle kopiert
        // ist im neuen Dokument ueberhaupt der FeldTyp vorhanden ?
        SwFieldType* pFldType = pInsDoc->InsertFldType(
                                    *((SwDDETable*)this)->GetDDEFldType() );
        ASSERT( pFldType, "unbekannter FieldType" );

        // tauschen am Node den Tabellen-Pointer aus
        pNewTbl = new SwDDETable( *pNewTbl,
                                 (SwDDEFieldType*)pFldType );
        pTblNd->SetNewTable( pNewTbl, FALSE );
    }

    pNewTbl->GetFrmFmt()->CopyAttrs( *GetFrmFmt() );
    pNewTbl->SetTblChgMode( GetTblChgMode() );

    //Vernichten der Frms die bereits angelegt wurden.
    pTblNd->DelFrms();

    {
        // Tabellen-Formeln in die relative Darstellung umwandeln
        SwTableFmlUpdate aMsgHnt( this );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pSrcDoc->UpdateTblFlds( &aMsgHnt );
    }

    SwTblNumFmtMerge aTNFM( *pSrcDoc, *pInsDoc );

    // Namen auch kopieren oder neuen eindeutigen erzeugen
    if( bCpyName )
        pNewTbl->GetFrmFmt()->SetName( GetFrmFmt()->GetName() );

    _CpyTabFrms aCpyFmt;
    _CpyPara aPara( pTblNd, 1, aCpyFmt, bCpyNds );
    aPara.nNewSize = aPara.nOldSize = GetFrmFmt()->GetFrmSize().GetWidth();
    // dann kopiere mal
    aFndBox.GetLines().ForEach( &lcl_CopyLineToDoc, &aPara );

    // dann setze oben und unten noch die "richtigen" Raender:
    {
        _FndLine* pFndLn = aFndBox.GetLines()[ 0 ];
        SwTableLine* pLn = pFndLn->GetLine();
        const SwTableLine* pTmp = pLn;
        USHORT nLnPos = GetTabLines().GetPos( pTmp );
        if( USHRT_MAX != nLnPos && nLnPos )
        {
            // es gibt eine Line davor
            SwCollectTblLineBoxes aLnPara( FALSE, HEADLINE_BORDERCOPY );

            pLn = GetTabLines()[ nLnPos - 1 ];
            pLn->GetTabBoxes().ForEach( &lcl_Box_CollectBox, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( TRUE );
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
            // es gibt eine Line dahinter
            SwCollectTblLineBoxes aLnPara( TRUE, HEADLINE_BORDERCOPY );

            pLn = GetTabLines()[ nLnPos + 1 ];
            pLn->GetTabBoxes().ForEach( &lcl_Box_CollectBox, &aLnPara );

            if( aLnPara.Resize( lcl_GetBoxOffset( aFndBox ),
                                lcl_GetLineWidth( *pFndLn )) )
            {
                aLnPara.SetValues( FALSE );
                pLn = pNewTbl->GetTabLines()[ pNewTbl->GetTabLines().Count()-1 ];
                pLn->GetTabBoxes().ForEach( &lcl_BoxSetSplitBoxFmts, &aLnPara );
            }
        }
    }

    // die initiale Box muss noch geloescht werden
    _DeleteBox( *pNewTbl, pNewTbl->GetTabLines()[
                pNewTbl->GetTabLines().Count() - 1 ]->GetTabBoxes()[0],
                0, FALSE, FALSE );

    // Mal kurz aufraeumen:
    pNewTbl->GCLines();

    pTblNd->MakeFrms( &aIdx );  // erzeuge die Frames neu
    return TRUE;
}



// ---------------------------------------------------------------

// suche ab dieser Line nach der naechsten Box mit Inhalt
SwTableBox* SwTableLine::FindNextBox( const SwTable& rTbl,
                     const SwTableBox* pSrchBox, BOOL bOvrTblLns ) const
{
    const SwTableLine* pLine = this;            // fuer M800
    SwTableBox* pBox;
    USHORT nFndPos;
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
        ASSERT( USHRT_MAX != nFndPos, "Line nicht in der Tabelle" );
        // gibts eine weitere Line
        if( nFndPos+1 >= GetUpper()->GetTabLines().Count() )
            return GetUpper()->GetUpper()->FindNextBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos+1];
    }
    else if( bOvrTblLns )       // ueber die "GrundLines" einer Tabelle ?
    {
        // suche in der Tabelle nach der naechsten Line
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( nFndPos + 1 >= rTbl.GetTabLines().Count() )
            return 0;           // es gibt keine weitere Box mehr

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

// suche ab dieser Line nach der vorherigen Box
SwTableBox* SwTableLine::FindPreviousBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, BOOL bOvrTblLns ) const
{
    const SwTableLine* pLine = this;            // fuer M800
    SwTableBox* pBox;
    USHORT nFndPos;
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
        ASSERT( USHRT_MAX != nFndPos, "Line nicht in der Tabelle" );
        // gibts eine weitere Line
        if( !nFndPos )
            return GetUpper()->GetUpper()->FindPreviousBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos-1];
    }
    else if( bOvrTblLns )       // ueber die "GrundLines" einer Tabelle ?
    {
        // suche in der Tabelle nach der naechsten Line
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( !nFndPos )
            return 0;           // es gibt keine weitere Box mehr

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

// suche ab dieser Line nach der naechsten Box mit Inhalt
SwTableBox* SwTableBox::FindNextBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, BOOL bOvrTblLns ) const
{
    if( !pSrchBox  && !GetTabLines().Count() )
        return (SwTableBox*)this;
    return GetUpper()->FindNextBox( rTbl, pSrchBox ? pSrchBox : this,
                                        bOvrTblLns );

}

// suche ab dieser Line nach der naechsten Box mit Inhalt
SwTableBox* SwTableBox::FindPreviousBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, BOOL bOvrTblLns ) const
{
    if( !pSrchBox && !GetTabLines().Count() )
        return (SwTableBox*)this;
    return GetUpper()->FindPreviousBox( rTbl, pSrchBox ? pSrchBox : this,
                                        bOvrTblLns );
}


BOOL lcl_BoxSetHeadCondColl( const SwTableBox*& rpBox, void* pPara )
{
    // in der HeadLine sind die Absaetze mit BedingtenVorlage anzupassen
    const SwStartNode* pSttNd = rpBox->GetSttNd();
    if( pSttNd )
        pSttNd->CheckSectionCondColl();
    else
        ((SwTableBox*)rpBox)->GetTabLines().ForEach( &lcl_LineSetHeadCondColl, 0 );
    return TRUE;
}

BOOL lcl_LineSetHeadCondColl( const SwTableLine*& rpLine, void* pPara )
{
    ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_BoxSetHeadCondColl, 0 );
    return TRUE;
}

/*  */

#pragma optimize( "", off )

SwTwips lcl_GetDistance( SwTableBox* pBox, BOOL bLeft )
{
    BOOL bFirst = TRUE;
    SwTwips nRet = 0;
    SwTableLine* pLine;
    while( pBox && 0 != ( pLine = pBox->GetUpper() ) )
    {
        USHORT nStt = 0, nPos = pLine->GetTabBoxes().C40_GETPOS( SwTableBox, pBox );

        if( bFirst && !bLeft )
            ++nPos;
        bFirst = FALSE;

        while( nStt < nPos )
            nRet += pLine->GetTabBoxes()[ nStt++ ]->GetFrmFmt()
                            ->GetFrmSize().GetWidth();
        pBox = pLine->GetUpper();
    }
    return nRet;
}

//#pragma optimize( "", on )

BOOL lcl_SetSelBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                         SwTwips nDist, BOOL bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();
        BOOL bGreaterBox;

        if( bCheck )
        {
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_SetSelBoxWidth( pBox->GetTabLines()[ i ], rParam,
                                            nDist, TRUE ))
                    return FALSE;


            // dann noch mal alle "ContentBoxen" sammeln
            if( !rParam.bBigger &&
                ( Abs( nDist + (( rParam.nMode && rParam.bLeft ) ? 0 : nWidth )
                    - rParam.nSide ) < COLFUZZY ) ||
                 ( 0 != ( bGreaterBox = TBLFIX_CHGABS != rParam.nMode &&
                    ( nDist + ( rParam.bLeft ? 0 : nWidth ) ) >= rParam.nSide)) )
            {
                rParam.bAnyBoxFnd = TRUE;
                SwTwips nLowerDiff;
                if( bGreaterBox && TBLFIX_CHGPROP == rParam.nMode )
                {
                    // die "anderen Boxen" wurden angepasst,
                    // also sich um diesen Betrag aendern
                    nLowerDiff = (nDist + ( rParam.bLeft ? 0 : nWidth ) ) - rParam.nSide;
                    nLowerDiff *= rParam.nDiff;
                    nLowerDiff /= rParam.nMaxSize;
                    nLowerDiff = rParam.nDiff - nLowerDiff;
                }
                else
                    nLowerDiff = rParam.nDiff;

                if( nWidth < nLowerDiff || nWidth - nLowerDiff < MINLAY )
                    return FALSE;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_SetSelBoxWidth( pBox->GetTabLines()[ i ], rParam, nDist, FALSE );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;


            if( nLowerDiff ||
                ( Abs( nDist + ( (rParam.nMode && rParam.bLeft) ? 0 : nWidth )
                            - rParam.nSide ) < COLFUZZY ) ||
                 ( 0 != ( bGreaterBox = !nOldLower && TBLFIX_CHGABS != rParam.nMode &&
                    ( nDist + ( rParam.bLeft ? 0 : nWidth ) ) >= rParam.nSide)) )
            {
                // in dieser Spalte ist der Cursor - also verkleinern / vergroessern
                SwFmtFrmSize aNew( rSz );

                if( !nLowerDiff )
                {
                    if( bGreaterBox && TBLFIX_CHGPROP == rParam.nMode )
                    {
                        // die "anderen Boxen" wurden angepasst,
                        // also sich um diesen Betrag aendern
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

        // wenns groesser wird, dann wars das
        if( ( TBLFIX_CHGABS == rParam.nMode || !rParam.bLeft ) &&
                nDist >= rParam.nSide )
            break;
    }
    return TRUE;
}

BOOL lcl_SetOtherBoxWidth( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, BOOL bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwFrmFmt* pFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_SetOtherBoxWidth( pBox->GetTabLines()[ i ],
                                                    rParam, nDist, TRUE ))
                    return FALSE;

            if( rParam.bBigger && ( TBLFIX_CHGABS == rParam.nMode
                    ? Abs( nDist - rParam.nSide ) < COLFUZZY
                    : ( rParam.bLeft ? nDist < rParam.nSide - COLFUZZY
                                     : nDist >= rParam.nSide - COLFUZZY )) )
            {
                rParam.bAnyBoxFnd = TRUE;
                SwTwips nDiff;
                if( TBLFIX_CHGPROP == rParam.nMode )        // Tabelle fix, proport.
                {
                    // relativ berechnen
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nMaxSize;
                }
                else
                    nDiff = rParam.nDiff;

                if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                    return FALSE;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_SetOtherBoxWidth( pBox->GetTabLines()[ i ], rParam,
                                            nDist, FALSE );

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
                    if( TBLFIX_CHGPROP == rParam.nMode )        // Tabelle fix, proport.
                    {
                        // relativ berechnen
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
    return TRUE;
}

/**/

//#pragma optimize( "", off )

BOOL lcl_InsSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                            SwTwips nDist, BOOL bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    USHORT n, nCmp;
    for( n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];
        SwTableBoxFmt* pFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
        const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
        SwTwips nWidth = rSz.GetWidth();

        if( bCheck )
        {
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_InsSelBox( pBox->GetTabLines()[ i ], rParam,
                                            nDist, TRUE ))
                    return FALSE;

            // dann noch mal alle "ContentBoxen" sammeln
            if( Abs( nDist + ( rParam.bLeft ? 0 : nWidth )
                    - rParam.nSide ) < COLFUZZY )
                nCmp = 1;
            else if( nDist + ( rParam.bLeft ? 0 : nWidth/2 ) > rParam.nSide )
                nCmp = 2;
            else
                nCmp = 0;

            if( nCmp )
            {
                rParam.bAnyBoxFnd = TRUE;
                if( pFmt->GetProtect().IsCntntProtected() )
                    return FALSE;

                if( rParam.bSplittBox &&
                    nWidth - rParam.nDiff <= COLFUZZY +
                        ( 567 / 2 /* min. 0,5 cm Platz lassen*/) )
                    return FALSE;

                if( pBox->GetSttNd() )
                    rParam.aBoxes.Insert( pBox );

                break;
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_InsSelBox( pBox->GetTabLines()[ i ], rParam, nDist, FALSE );

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
                // in dieser Spalte ist der Cursor - also verkleinern / vergroessern
                if( 1 == nCmp )
                {
                    if( !rParam.bSplittBox )
                    {
                        // die akt. Box auf
                        SwFmtFrmSize aNew( rSz );
                        aNew.SetWidth( nWidth + rParam.nDiff );
                        rParam.aShareFmts.SetSize( *pBox, aNew );
                    }
                }
                else
                {
                    ASSERT( pBox->GetSttNd(), "Das muss eine EndBox sein!");

                    if( !rParam.bLeft && 3 != nCmp )
                        ++n;

                    ::_InsTblBox( pFmt->GetDoc(), rParam.pTblNd,
                                        pLine, pFmt, pBox, n );

                    SwTableBox* pNewBox = rBoxes[ n ];
                    SwFmtFrmSize aNew( rSz );
                    aNew.SetWidth( rParam.nDiff );
                    rParam.aShareFmts.SetSize( *pNewBox, aNew );

                    // Sonderfall: kein Platz in den anderen Boxen
                    //              aber in der Zelle
                    if( rParam.bSplittBox )
                    {
                        // die akt. Box auf
                        SwFmtFrmSize aNew( rSz );
                        aNew.SetWidth( nWidth - rParam.nDiff );
                        rParam.aShareFmts.SetSize( *pBox, aNew );
                    }

                    // Sonderbehandlung fuer Umrandung die Rechte muss
                    // entfernt werden
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
    return TRUE;
}
//#pragma optimize( "", on )

BOOL lcl_InsOtherBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                                SwTwips nDist, BOOL bCheck )
{
    // Sonderfall: kein Platz in den anderen Boxen aber in der Zelle
    if( rParam.bSplittBox )
        return TRUE;

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    USHORT n;

    // Tabelle fix, proport.
    if( !rParam.nRemainWidth && TBLFIX_CHGPROP == rParam.nMode )
    {
        // dann die richtige Breite suchen, auf die sich die relative
        // Breitenanpassung bezieht.
        SwTwips nTmpDist = nDist;
        for( n = 0; n < rBoxes.Count(); ++n )
        {
            SwTwips nWidth = rBoxes[ n ]->GetFrmFmt()->GetFrmSize().GetWidth();
            if( (nTmpDist + nWidth / 2 ) > rParam.nSide )
            {
                rParam.nRemainWidth = rParam.bLeft
                                        ? USHORT(nTmpDist)
                                        : USHORT(rParam.nTblWidth - nTmpDist);
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
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
                if( !::lcl_InsOtherBox( pBox->GetTabLines()[ i ],
                                                    rParam, nDist, TRUE ))
                    return FALSE;

            if(
                rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TBLFIX_CHGABS != rParam.nMode ||
                                n < rBoxes.Count() &&
                                (nDist + nWidth + rBoxes[ n+1 ]->
                                    GetFrmFmt()->GetFrmSize().GetWidth() / 2)
                                  > rParam.nSide ))
                             : (nDist + nWidth / 2 ) > rParam.nSide
                )
            {
                rParam.bAnyBoxFnd = TRUE;
                SwTwips nDiff;
                if( TBLFIX_CHGPROP == rParam.nMode )        // Tabelle fix, proport.
                {
                    // relativ berechnen
                    nDiff = nWidth;
                    nDiff *= rParam.nDiff;
                    nDiff /= rParam.nRemainWidth;

                    if( nWidth < nDiff || nWidth - nDiff < MINLAY )
                        return FALSE;
                }
                else
                {
                    nDiff = rParam.nDiff;

                    // teste ob die linke oder rechte Box gross genug
                    // ist, um den Platz abzugeben!
                    // es wird davor oder dahinter eine Box eingefuegt!
                    SwTwips nTmpWidth = nWidth;
                    if( rParam.bLeft && pBox->GetUpper()->GetUpper() )
                    {
                        const SwTableBox* pTmpBox = pBox;
                        USHORT nBoxPos = n;
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
                        return FALSE;
                    break;
                }
            }
        }
        else
        {
            SwTwips nLowerDiff = 0, nOldLower = rParam.nLowerDiff;
            for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
            {
                rParam.nLowerDiff = 0;
                lcl_InsOtherBox( pBox->GetTabLines()[ i ], rParam,
                                        nDist, FALSE );

                if( nLowerDiff < rParam.nLowerDiff )
                    nLowerDiff = rParam.nLowerDiff;
            }
            rParam.nLowerDiff = nOldLower;

            if( nLowerDiff ||
                (rParam.bLeft ? ((nDist + nWidth / 2 ) <= rParam.nSide &&
                                (TBLFIX_CHGABS != rParam.nMode ||
                                n < rBoxes.Count() &&
                                (nDist + nWidth + rBoxes[ n+1 ]->
                                    GetFrmFmt()->GetFrmSize().GetWidth() / 2)
                                  > rParam.nSide ))
                              : (nDist + nWidth / 2 ) > rParam.nSide ))
            {
                if( !nLowerDiff )
                {
                    if( TBLFIX_CHGPROP == rParam.nMode )        // Tabelle fix, proport.
                    {
                        // relativ berechnen
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
    return TRUE;
}


// das Ergebnis des Positions Vergleiches
//  POS_BEFORE,             // Box liegt davor
//  POS_BEHIND,             // Box liegt dahinter
//  POS_INSIDE,             // Box liegt vollstaendig in Start/End
//  POS_OUTSIDE,            // Box ueberlappt Start/End vollstaendig
//  POS_EQUAL,              // Box und Start/End sind gleich
//  POS_OVERLAP_BEFORE,     // Box ueberlappt den Start
//  POS_OVERLAP_BEHIND      // Box ueberlappt das Ende

SwComparePosition _CheckBoxInRange( USHORT nStt, USHORT nEnd,
                                    USHORT nBoxStt, USHORT nBoxEnd )
{
// COLFUZZY noch beachten!!
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
    // 1. Schritt die eigene Breite feststellen
    SwTableBoxes& rBoxes = rLine.GetTabBoxes();
    SwTwips nBoxWidth = 0;

    for( USHORT n = rBoxes.Count(); n; )
        nBoxWidth += rBoxes[ --n ]->GetFrmFmt()->GetFrmSize().GetWidth();

    if( COLFUZZY < Abs( nWidth - nBoxWidth ))
    {
        //  sie muessen also angepasst werden
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
                // hat selbst auch Lower, also auch die anpassen
                for( USHORT i = pBox->GetTabLines().Count(); i; )
                    ::lcl_DelSelBox_CorrLowers( *pBox->GetTabLines()[ --i ],
                                                rParam, nDiff  );
            }
        }
    }
}

void lcl_ChgBoxSize( SwTableBox& rBox, CR_SetBoxWidth& rParam,
                    const SwFmtFrmSize& rOldSz,
                    USHORT& rDelWidth, SwTwips nDist )
{
    long nDiff;
    BOOL bSetSize = FALSE;

    switch( rParam.nMode )
    {
    case TBLFIX_CHGABS:     // Tabelle feste Breite, den Nachbar andern
        nDiff = rDelWidth + rParam.nLowerDiff;
        bSetSize = TRUE;
        break;

    case TBLFIX_CHGPROP:    // Tabelle feste Breite, alle Nachbarn aendern
        if( !rParam.nRemainWidth )
        {
            // dann kurz berechnen:
            if( rParam.bLeft )
                rParam.nRemainWidth = USHORT(nDist);
            else
                rParam.nRemainWidth = USHORT(rParam.nTblWidth - nDist);
        }

        // relativ berechnen
        nDiff = rOldSz.GetWidth();
        nDiff *= rDelWidth + rParam.nLowerDiff;
        nDiff /= rParam.nRemainWidth;

        bSetSize = TRUE;
        break;

    case TBLVAR_CHGABS:     // Tabelle variable, alle Nachbarn aendern
        if( COLFUZZY < Abs( rParam.nBoxWidth -
                            ( rDelWidth + rParam.nLowerDiff )))
        {
            nDiff = rDelWidth + rParam.nLowerDiff - rParam.nBoxWidth;
            if( 0 < nDiff )
                rDelWidth -= USHORT(nDiff);
            else
                rDelWidth += USHORT(-nDiff);
            bSetSize = TRUE;
        }
        break;
    }

    if( bSetSize )
    {
        SwFmtFrmSize aNew( rOldSz );
        aNew.SetWidth( aNew.GetWidth() + nDiff );
        rParam.aShareFmts.SetSize( rBox, aNew );

        // dann leider nochmals die Lower anpassen
        for( USHORT i = rBox.GetTabLines().Count(); i; )
            ::lcl_DelSelBox_CorrLowers( *rBox.GetTabLines()[ --i ], rParam,
                                            aNew.GetWidth() );
    }
}

BOOL lcl_DeleteBox_Rekursiv( CR_SetBoxWidth& rParam, SwTableBox& rBox,
                            BOOL bCheck )
{
    BOOL bRet = TRUE;
    if( rBox.GetSttNd() )
    {
        if( bCheck )
        {
            rParam.bAnyBoxFnd = TRUE;
            if( rBox.GetFrmFmt()->GetProtect().IsCntntProtected() )
                bRet = FALSE;
            else
            {
                SwTableBox* pBox = &rBox;
                rParam.aBoxes.Insert( pBox );
            }
        }
        else
            ::_DeleteBox( rParam.pTblNd->GetTable(), &rBox,
                            rParam.pUndo, FALSE, TRUE, &rParam.aShareFmts );
    }
    else
    {
        // die muessen leider alle sequentiel ueber die
        // Contentboxen geloescht werden
        for( USHORT i = rBox.GetTabLines().Count(); i; )
        {
            SwTableLine& rLine = *rBox.GetTabLines()[ --i ];
            for( USHORT n = rLine.GetTabBoxes().Count(); n; )
                if( !::lcl_DeleteBox_Rekursiv( rParam,
                                *rLine.GetTabBoxes()[ --n ], bCheck ))
                    return FALSE;
        }
    }
    return bRet;
}

BOOL lcl_DelSelBox( SwTableLine* pLine, CR_SetBoxWidth& rParam,
                    SwTwips nDist, BOOL bCheck )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    USHORT n, nCntEnd, nBoxChkStt, nBoxChkEnd, nDelWidth = 0;
    if( rParam.bLeft )
    {
        n = rBoxes.Count();
        nCntEnd = 0;
        nBoxChkStt = (USHORT)rParam.nSide;
        nBoxChkEnd = rParam.nSide + rParam.nBoxWidth;
    }
    else
    {
        n = 0;
        nCntEnd = rBoxes.Count();
        nBoxChkStt = rParam.nSide - rParam.nBoxWidth;
        nBoxChkEnd = (USHORT)rParam.nSide;
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
        BOOL bDelBox = FALSE, bChgLowers = FALSE;

        // die Boxenbreite testen und entpsrechend reagieren
        SwComparePosition ePosType = ::_CheckBoxInRange(
                            nBoxChkStt, nBoxChkEnd,
                            USHORT(rParam.bLeft ? nDist - nWidth : nDist),
                            USHORT(rParam.bLeft ? nDist : nDist + nWidth));

        switch( ePosType )
        {
        case POS_BEFORE:
            if( bCheck )
            {
                if( rParam.bLeft )
                    return TRUE;
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
                    return TRUE;
            }
            else if( !rParam.bLeft )
            {
                ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                if( TBLFIX_CHGABS == rParam.nMode )
                    n = nCntEnd;
            }
            break;

        case POS_OUTSIDE:           // Box ueberlappt Start/End vollstaendig
        case POS_INSIDE:            // Box liegt vollstaendig in Start/End
        case POS_EQUAL:             // Box und Start/End sind gleich
            bDelBox = TRUE;
            break;

        case POS_OVERLAP_BEFORE:     // Box ueberlappt den Start
            if( nBoxChkStt <= ( nDist + (rParam.bLeft ? - nWidth / 2
                                                      : nWidth / 2 )))
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = TRUE;
                else
                    bDelBox = TRUE;
            }
            else if( !bCheck && rParam.bLeft )
            {
                if( !pBox->GetSttNd() )
                    bChgLowers = TRUE;
                else
                {
                    ::lcl_ChgBoxSize( *pBox, rParam, rSz, nDelWidth, nDist );
                    if( TBLFIX_CHGABS == rParam.nMode )
                        n = nCntEnd;
                }
            }
            break;

        case POS_OVERLAP_BEHIND:     // Box ueberlappt das Ende
            // JP 10.02.99:
            // generell loeschen oder wie beim OVERLAP_Before nur die, die
            // bis zur Haelfte in die "Loesch-"Box reicht ???
            if( !pBox->GetSttNd() )
                bChgLowers = TRUE;
            else
                bDelBox = TRUE;
            break;
        }

        if( bDelBox )
        {
            nDelWidth += USHORT(nWidth);
            if( bCheck )
            {
                // die letzte/erste Box kann nur bei Tbl-Var geloescht werden,
                // wenn diese so gross ist, wie die Aenderung an der Tabelle
                if( (( TBLVAR_CHGABS != rParam.nMode ||
                        nDelWidth != rParam.nBoxWidth ) &&
                     COLFUZZY > Abs( rParam.bLeft
                                    ? nWidth - nDist
                                    : (nDist + nWidth - rParam.nTblWidth )))
                    || !::lcl_DeleteBox_Rekursiv( rParam, *pBox, bCheck ) )
                    return FALSE;

                if( pFmt->GetProtect().IsCntntProtected() )
                    return FALSE;
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
            BOOL bFirst = TRUE, bCorrLowers = FALSE;
            long nLowerDiff = 0;
            long nOldLower = rParam.nLowerDiff;
            USHORT nOldRemain = rParam.nRemainWidth;

            for( USHORT i = pBox->GetTabLines().Count(); i; )
            {
                rParam.nLowerDiff = nDelWidth + nOldLower;
                rParam.nRemainWidth = nOldRemain;
                SwTableLine* pLine = pBox->GetTabLines()[ --i ];
                if( !::lcl_DelSelBox( pLine, rParam, nDist, bCheck ))
                    return FALSE;

                // gibt es die Box und die darin enthaltenen Lines noch??
                if( n < rBoxes.Count() &&
                    pBox == rBoxes[ rParam.bLeft ? n : n-1 ] &&
                    i < pBox->GetTabLines().Count() &&
                    pLine == pBox->GetTabLines()[ i ] )
                {
                    if( !bFirst && !bCorrLowers &&
                        COLFUZZY < Abs( nLowerDiff - rParam.nLowerDiff ) )
                        bCorrLowers = TRUE;

                    // die groesste "loesch" Breite entscheidet, aber nur wenn
                    // nicht die gesamte Line geloescht wurde
                    if( nLowerDiff < rParam.nLowerDiff )
                        nLowerDiff = rParam.nLowerDiff;

                    bFirst = FALSE;
                }
            }
            rParam.nLowerDiff = nOldLower;
            rParam.nRemainWidth = nOldRemain;

            // wurden alle Boxen geloescht? Dann ist die DelBreite natuerlich
            // die Boxenbreite
            if( !nLowerDiff )
                nLowerDiff = nWidth;

            // DelBreite anpassen!!
            nDelWidth += USHORT(nLowerDiff);

            if( !bCheck )
            {
                // wurde die Box schon entfernt?
                if( n > rBoxes.Count() ||
                    pBox != rBoxes[ ( rParam.bLeft ? n : n-1 ) ] )
                {
                    // dann beim Loeschen nach rechts die Laufvar. anpassen
                    if( !rParam.bLeft )
                        --n, --nCntEnd;
                }
                else
                {
                    // sonst muss die Groesse der Box angepasst werden
                    SwFmtFrmSize aNew( rSz );
                    BOOL bCorrRel = FALSE;

                    if( TBLVAR_CHGABS != rParam.nMode )
                    {
                        switch( ePosType )
                        {
                        case POS_OVERLAP_BEFORE:    // Box ueberlappt den Start
                            if( TBLFIX_CHGPROP == rParam.nMode )
                                bCorrRel = rParam.bLeft;
                            else if( rParam.bLeft ) // TBLFIX_CHGABS
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = TRUE;
                                n = nCntEnd;
                            }
                            break;

                        case POS_OVERLAP_BEHIND:    // Box ueberlappt das Ende
                            if( TBLFIX_CHGPROP == rParam.nMode )
                                bCorrRel = !rParam.bLeft;
                            else if( !rParam.bLeft )    // TBLFIX_CHGABS
                            {
                                nLowerDiff = nLowerDiff - nDelWidth;
                                bCorrLowers = TRUE;
                                n = nCntEnd;
                            }
                            break;

                        default:
                            ASSERT( !pBox, "hier sollte man nie hinkommen" );
                            break;
                        }
                    }

                    if( bCorrRel )
                    {
                        if( !rParam.nRemainWidth )
                        {
                            // dann kurz berechnen:
                            if( rParam.bLeft )
                                rParam.nRemainWidth = USHORT(nDist - nLowerDiff);
                            else
                                rParam.nRemainWidth = USHORT(rParam.nTblWidth - nDist
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
                        // dann leider nochmals die Lower anpassen
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
    return TRUE;
}

// Dummy Funktion fuer die Methode SetColWidth
BOOL lcl_DelOtherBox( SwTableLine* , CR_SetBoxWidth& , SwTwips , BOOL )
{
    return TRUE;
}

/**/

void lcl_AjustLines( SwTableLine* pLine, CR_SetBoxWidth& rParam )
{
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox* pBox = rBoxes[ n ];

        SwFmtFrmSize aSz( pBox->GetFrmFmt()->GetFrmSize() );
        SwTwips nWidth = aSz.GetWidth();
        nWidth *= rParam.nDiff;
        nWidth /= rParam.nMaxSize;
        aSz.SetWidth( nWidth );
        rParam.aShareFmts.SetSize( *pBox, aSz );

        for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
            ::lcl_AjustLines( pBox->GetTabLines()[ i ], rParam );
    }
}

#if !defined( PRODUCT ) || defined( JP_DEBUG )

void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize )
{
    const SwTableBoxes& rBoxes = rLine.GetTabBoxes();

    SwTwips nAktSize = 0;
    // checke doch mal ob die Tabellen korrekte Breiten haben
    for( USHORT n = 0; n < rBoxes.Count(); ++n  )
    {
        const SwTableBox* pBox = rBoxes[ n ];
        SwTwips nBoxW = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        nAktSize += nBoxW;

        for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
            _CheckBoxWidth( *pBox->GetTabLines()[ i ], nBoxW );
    }

    if( Abs( nAktSize - nSize ) > ( COLFUZZY * rBoxes.Count() ) )
    {
        DBG_ERROR( "Boxen der Line zu klein/gross" );
#if defined( WNT ) && defined( JP_DEBUG )
        __asm int 3;
#endif
    }
}

#endif

_FndBox* lcl_SaveInsDelData( CR_SetBoxWidth& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst, SwTwips nDistStt )
{
    // suche alle Boxen / Lines
    SwTable& rTbl = rParam.pTblNd->GetTable();

    if( !rParam.aBoxes.Count() )
    {
        // erstmal die Boxen besorgen !
        if( rParam.bBigger )
            for( USHORT n = 0; n < rTbl.GetTabLines().Count(); ++n )
                ::lcl_DelSelBox( rTbl.GetTabLines()[ n ], rParam, nDistStt, TRUE );
        else
            for( USHORT n = 0; n < rTbl.GetTabLines().Count(); ++n )
                ::lcl_InsSelBox( rTbl.GetTabLines()[ n ], rParam, nDistStt, TRUE );
    }

    // loeschen der gesamten Tabelle verhindern
    if( rParam.bBigger && rParam.aBoxes.Count() ==
        rTbl.GetTabSortBoxes().Count() )
        return 0;

    _FndBox* pFndBox = new _FndBox( 0, 0 );
    if( rParam.bBigger )
        pFndBox->SetTableLines( rParam.aBoxes, rTbl );
    else
    {
        _FndPara aPara( rParam.aBoxes, pFndBox );
        rTbl.GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
        ASSERT( pFndBox->GetLines().Count(), "Wo sind die Boxen" );
        pFndBox->SetTableLines( rTbl );

        if( ppUndo )
            rTmpLst.Insert( &rTbl.GetTabSortBoxes(), 0, rTbl.GetTabSortBoxes().Count() );
    }

    //Lines fuer das Layout-Update herausuchen.
    pFndBox->DelFrms( rTbl );
    pFndBox->SaveChartData( rTbl );

    return pFndBox;
}

BOOL SwTable::SetColWidth( SwTableBox& rAktBox, USHORT eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo )
{
    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    const SwFmtFrmSize& rSz = GetFrmFmt()->GetFrmSize();
    const SvxLRSpaceItem& rLR = GetFrmFmt()->GetLRSpace();

    _FndBox* pFndBox = 0;                   // fuers Einfuegen/Loeschen
    SwTableSortBoxes aTmpLst( 0, 5 );       // fuers Undo
    BOOL bBigger,
        bRet = FALSE,
        bLeft = WH_COL_LEFT == ( eType & 0xff ) ||
                WH_CELL_LEFT == ( eType & 0xff ),
        bInsDel = 0 != (eType & WH_FLAG_INSDEL );
    USHORT n;
    ULONG nBoxIdx = rAktBox.GetSttIdx();

    // bestimme die akt. Kante der Box
    // wird nur fuer die Breitenmanipulation benoetigt!
    SwTwips nDist = ::lcl_GetDistance( &rAktBox, bLeft ), nDistStt = 0;
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
            aParam.nBoxWidth = (USHORT)rAktBox.GetFrmFmt()->GetFrmSize().GetWidth();
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
    case WH_COL_RIGHT:
    case WH_COL_LEFT:
        if( TBLVAR_CHGABS == eTblChgMode )
        {
            if( bInsDel )
                bBigger = !bBigger;

            // erstmal testen, ob ueberhaupt Platz ist
            BOOL bChgLRSpace = TRUE;
            if( bBigger )
            {
                if( GetFrmFmt()->GetDoc()->IsBrowseMode() &&
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
                    // auf der anderen Seite Platz?
                    ( bLeft ? rLR.GetRight() >= nAbsDiff
                            : rLR.GetLeft() >= nAbsDiff ))
                {
                    bRet = TRUE; bLeft = !bLeft;
                }

                if( !bRet )
                {
                    // dann sich selbst rekursiv aufrufen; nur mit
                    // einem anderen Mode -> proprotional
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
                bRet = TRUE;
                for( n = 0; n < aLines.Count(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, TRUE ))
                    {
                        bRet = FALSE;
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
                    if( aParam.bBigger && aParam.aBoxes.Count() ==
                                    aSortCntBoxes.Count() )
                    {
                        // dies gesamte Tabelle soll geloescht werden!!
                        GetFrmFmt()->GetDoc()->DeleteRowCol( aParam.aBoxes );
                        return FALSE;
                    }

                    if( ppUndo )
                        *ppUndo = aParam.CreateUndo(
                                        aParam.bBigger ? UNDO_TABLE_DELBOX
                                                       : UNDO_TABLE_INSCOL );
                }
                else if( ppUndo )
                    *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, TRUE );

                long nFrmWidth = LONG_MAX;
                LockModify();
                SwFmtFrmSize aSz( rSz );
                SvxLRSpaceItem aLR( rLR );
                if( bBigger )
                {
                    // falls die Tabelle keinen Platz zum Wachsen hat, dann
                    // muessen wir welchen schaffen!
                    if( aSz.GetWidth() + nRelDiff > USHRT_MAX )
                    {
                        // dann mal herunterbrechen auf USHRT_MAX / 2
                        CR_SetBoxWidth aTmpPara( 0, aSz.GetWidth() / 2,
                                        0, aSz.GetWidth(), aSz.GetWidth(), aParam.pTblNd );
                        for( USHORT n = 0; n < aLines.Count(); ++n )
                            ::lcl_AjustLines( aLines[ n ], aTmpPara );
                        aSz.SetWidth( aSz.GetWidth() / 2 );
                        aParam.nDiff = nRelDiff /= 2;
                        aParam.nSide /= 2;
                        aParam.nMaxSize /= 2;
                    }

                    if( bLeft )
                        aLR.SetLeft( USHORT( aLR.GetLeft() - nAbsDiff ) );
                    else
                        aLR.SetRight( USHORT( aLR.GetRight() - nAbsDiff ) );
                }
                else if( bLeft )
                    aLR.SetLeft( USHORT( aLR.GetLeft() + nAbsDiff ) );
                else
                    aLR.SetRight( USHORT( aLR.GetRight() + nAbsDiff ) );

                if( bChgLRSpace )
                    GetFrmFmt()->SetAttr( aLR );
                const SwFmtHoriOrient& rHOri = GetFrmFmt()->GetHoriOrient();
                if( HORI_FULL == rHOri.GetHoriOrient() ||
                    (HORI_LEFT == rHOri.GetHoriOrient() && aLR.GetLeft()) ||
                    (HORI_RIGHT == rHOri.GetHoriOrient() && aLR.GetRight()))
                {
                    SwFmtHoriOrient aHOri( rHOri );
                    aHOri.SetHoriOrient( HORI_NONE );
                    GetFrmFmt()->SetAttr( aHOri );

                    // sollte die Tabelle noch auf relativen Werten
                    // (USHRT_MAX) stehen dann muss es jetzt auf absolute
                    // umgerechnet werden. Bug 61494
                    if( GetFrmFmt()->GetDoc()->IsBrowseMode() &&
                        !rSz.GetWidthPercent() )
                    {
                        SwTabFrm* pTabFrm = (SwTabFrm*)SwClientIter(
                                    *GetFrmFmt() ).First( TYPE( SwTabFrm ));
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
                    aSz.SetWidthPercent( ( aSz.GetWidth() * 100 ) /
                        ( aSz.GetWidth() + aLR.GetRight() + aLR.GetLeft()));

                GetFrmFmt()->SetAttr( aSz );
                aParam.nTblWidth = USHORT( aSz.GetWidth() );

                UnlockModify();

                for( n = aLines.Count(); n; )
                {
                    --n;
                    aParam.LoopClear();
                    (*fnSelBox)( aLines[ n ], aParam, nDistStt, FALSE );
                }

                // sollte die Tabelle noch auf relativen Werten
                // (USHRT_MAX) stehen dann muss es jetzt auf absolute
                // umgerechnet werden. Bug 61494
                if( LONG_MAX != nFrmWidth )
                {
                    SwFmtFrmSize aAbsSz( aSz );
                    aAbsSz.SetWidth( nFrmWidth );
                    GetFrmFmt()->SetAttr( aAbsSz );
                }
            }
        }
        else if( bInsDel ||
                ( bLeft ? nDist : Abs( rSz.GetWidth() - nDist ) > COLFUZZY ) )
        {
            bRet = TRUE;
            if( bLeft && TBLFIX_CHGABS == eTblChgMode && !bInsDel )
                aParam.bBigger = !bBigger;

            // erstmal testen, ob ueberhaupt Platz ist
            if( bInsDel )
            {
                if( aParam.bBigger )
                {
                    for( n = 0; n < aLines.Count(); ++n )
                    {
                        aParam.LoopClear();
                        if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, TRUE ))
                        {
                            bRet = FALSE;
                            break;
                        }
                    }
                }
                else
                {
                    if( 0 != ( bRet = bLeft ? nDist
                                            : ( rSz.GetWidth() - nDist )
                        > COLFUZZY ) )
                    {
                        for( n = 0; n < aLines.Count(); ++n )
                        {
                            aParam.LoopClear();
                            if( !(*fnOtherBox)( aLines[ n ], aParam, 0, TRUE ))
                            {
                                bRet = FALSE;
                                break;
                            }
                        }
                        if( bRet && !aParam.bAnyBoxFnd )
                            bRet = FALSE;
                    }

                    if( !bRet && rAktBox.GetFrmFmt()->GetFrmSize().GetWidth()
                        - nRelDiff > COLFUZZY +
                            ( 567 / 2 /* min. 0,5 cm Platz lassen*/) )
                    {
                        // dann den Platz von der akt. Zelle nehmen
                        aParam.bSplittBox = TRUE;
                        // aber das muss auch mal getestet werden!
                        bRet = TRUE;

                        for( n = 0; n < aLines.Count(); ++n )
                        {
                            aParam.LoopClear();
                            if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, TRUE ))
                            {
                                bRet = FALSE;
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
                    if( !(*fnOtherBox)( aLines[ n ], aParam, 0, TRUE ))
                    {
                        bRet = FALSE;
                        break;
                    }
                }
            }
            else
            {
                for( n = 0; n < aLines.Count(); ++n )
                {
                    aParam.LoopClear();
                    if( !(*fnSelBox)( aLines[ n ], aParam, nDistStt, TRUE ))
                    {
                        bRet = FALSE;
                        break;
                    }
                }
            }

            // wenn ja, dann setzen
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
                    *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, TRUE );

                if( bInsDel
                    ? ( TBLFIX_CHGABS == eTblChgMode ? bLeft : bLeft )
                    : ( TBLFIX_CHGABS != eTblChgMode && bLeft ) )
                {
                    for( n = aLines.Count(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnSelBox)( aLines[ n ], aParam, nDistStt, FALSE );
                        (*fnOtherBox)( aLines[ n ], aParam1, nDistStt, FALSE );
                    }
                }
                else
                    for( n = aLines.Count(); n; )
                    {
                        --n;
                        aParam.LoopClear();
                        aParam1.LoopClear();
                        (*fnOtherBox)( aLines[ n ], aParam1, nDistStt, FALSE );
                        (*fnSelBox)( aLines[ n ], aParam, nDistStt, FALSE );
                    }
            }
        }
        break;

    case WH_CELL_RIGHT:
    case WH_CELL_LEFT:
        if( TBLVAR_CHGABS == eTblChgMode )
        {
            // dann sich selbst rekursiv aufrufen; nur mit
            // einem anderen Mode -> Nachbarn
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

            // erstmal testen, ob ueberhaupt Platz ist
            SwTableBox* pBox = &rAktBox;
            SwTableLine* pLine = rAktBox.GetUpper();
            while( pLine->GetUpper() )
            {
                USHORT nPos = pLine->GetTabBoxes().C40_GETPOS( SwTableBox, pBox );
                if( bLeft ? nPos : nPos + 1 != pLine->GetTabBoxes().Count() )
                    break;

                pBox = pLine->GetUpper();
                pLine = pBox->GetUpper();
            }

            if( pLine->GetUpper() )
            {
                // dann muss die Distanz wieder korriegiert werden!
                aParam.nSide -= ::lcl_GetDistance( pLine->GetUpper(), TRUE );

                if( bLeft )
                    aParam.nMaxSize = aParam.nSide;
                else
                    aParam.nMaxSize = pLine->GetUpper()->GetFrmFmt()->
                                    GetFrmSize().GetWidth() - aParam.nSide;
            }

            // erstmal testen, ob ueberhaupt Platz ist
            if( bInsDel )
            {
                if( 0 != ( bRet = bLeft ? nDist
                                : ( rSz.GetWidth() - nDist ) > COLFUZZY ) &&
                    !aParam.bBigger )
                {
                    bRet = (*fnOtherBox)( pLine, aParam, 0, TRUE );
                    if( bRet && !aParam.bAnyBoxFnd )
                        bRet = FALSE;
                }

                if( !bRet && !aParam.bBigger && rAktBox.GetFrmFmt()->
                    GetFrmSize().GetWidth() - nRelDiff > COLFUZZY +
                        ( 567 / 2 /* min. 0,5 cm Platz lassen*/) )
                {
                    // dann den Platz von der akt. Zelle nehmen
                    aParam.bSplittBox = TRUE;
                    bRet = TRUE;
                }
            }
            else
            {
                FN_lcl_SetBoxWidth fnTmp = aParam.bBigger ? fnOtherBox : fnSelBox;
                bRet = (*fnTmp)( pLine, aParam, nDistStt, TRUE );
            }

            // wenn ja, dann setzen
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
                    *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, TRUE );

                if( bInsDel
                    ? ( TBLFIX_CHGABS == eTblChgMode ? (bBigger && bLeft) : bLeft )
                    : ( TBLFIX_CHGABS != eTblChgMode && bLeft ) )
                {
                    (*fnSelBox)( pLine, aParam, nDistStt, FALSE );
                    (*fnOtherBox)( pLine, aParam1, nDistStt, FALSE );
                }
                else
                {
                    (*fnOtherBox)( pLine, aParam1, nDistStt, FALSE );
                    (*fnSelBox)( pLine, aParam, nDistStt, FALSE );
                }
            }
        }
        break;

    }

    if( pFndBox )
    {
        // dann raeume die Struktur aller Lines auf
        GCLines();

        //Layout updaten
        if( !bBigger || pFndBox->AreLinesToRestore( *this ) )
            pFndBox->MakeFrms( *this );
        pFndBox->RestoreChartData( *this );
        delete pFndBox;

        if( ppUndo && *ppUndo )
        {
            aParam.pUndo->SetColWidthParam( nBoxIdx, eTblChgMode, eType,
                                            nAbsDiff, nRelDiff );
            if( !aParam.bBigger )
                aParam.pUndo->SaveNewBoxes( *aParam.pTblNd, aTmpLst );
        }
    }

    if( bRet )
    {
        CHECKBOXWIDTH
    }

    return bRet;
}
#pragma optimize( "", on )

/*  */

SwRowFrm* GetRowFrm( SwTableLine& rLine )
{
    SwClientIter aIter( *rLine.GetFrmFmt() );
    for( SwClient* pFrm = aIter.First( TYPE( SwRowFrm )); pFrm;
            pFrm = aIter.Next() )
        if( ((SwRowFrm*)pFrm)->GetTabLine() == &rLine )
            return (SwRowFrm*)pFrm;
    return 0;
}

_FndBox* lcl_SaveInsDelData( CR_SetLineHeight& rParam, SwUndo** ppUndo,
                                SwTableSortBoxes& rTmpLst )
{
    // suche alle Boxen / Lines
    SwTable& rTbl = rParam.pTblNd->GetTable();

    ASSERT( rParam.aBoxes.Count(), "ohne Boxen ist nichts zu machen!" );

    // loeschen der gesamten Tabelle verhindern
    if( !rParam.bBigger && rParam.aBoxes.Count() ==
        rTbl.GetTabSortBoxes().Count() )
        return 0;

    _FndBox* pFndBox = new _FndBox( 0, 0 );
    if( !rParam.bBigger )
        pFndBox->SetTableLines( rParam.aBoxes, rTbl );
    else
    {
        _FndPara aPara( rParam.aBoxes, pFndBox );
        rTbl.GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
        ASSERT( pFndBox->GetLines().Count(), "Wo sind die Boxen" );
        pFndBox->SetTableLines( rTbl );

        if( ppUndo )
            rTmpLst.Insert( &rTbl.GetTabSortBoxes(), 0, rTbl.GetTabSortBoxes().Count() );
    }

    //Lines fuer das Layout-Update heraussuchen.
    pFndBox->DelFrms( rTbl );
    pFndBox->SaveChartData( rTbl );

    return pFndBox;
}

void SetLineHeight( SwTableLine& rLine, SwTwips nOldHeight, SwTwips nNewHeight,
                    BOOL bMinSize )
{
    SwLayoutFrm* pLineFrm = GetRowFrm( rLine );
    ASSERT( pLineFrm, "wo ist der Frm von der SwTableLine?" );

    SwFrmFmt* pFmt = rLine.ClaimFrmFmt();

    SwTwips nMyNewH, nMyOldH = pLineFrm->Frm().Height();
    if( !nOldHeight )                       // die BaseLine und absolut
        nMyNewH = nMyOldH + nNewHeight;
    else
    {
        // moeglichst genau rechnen
        Fraction aTmp( nMyOldH );
        aTmp *= Fraction( nNewHeight, nOldHeight );
        aTmp += Fraction( 1, 2 );       // ggfs. aufrunden
        nMyNewH = aTmp;
    }

    SwFrmSize eSize = ATT_MIN_SIZE;
    if( !bMinSize &&
        nMyOldH - nMyNewH > CalcRowRstHeight( pLineFrm ) )
        eSize = ATT_FIX_SIZE;

    pFmt->SetAttr( SwFmtFrmSize( eSize, 0, nMyNewH ) );

    // erst alle inneren anpassen
    SwTableBoxes& rBoxes = rLine.GetTabBoxes();
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
    {
        SwTableBox& rBox = *rBoxes[ n ];
        for( USHORT i = 0; i < rBox.GetTabLines().Count(); ++i )
            SetLineHeight( *rBox.GetTabLines()[ i ], nMyOldH, nMyNewH, bMinSize );
    }
}

BOOL lcl_SetSelLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                             SwTwips nDist, BOOL bCheck )
{
    BOOL bRet = TRUE;
    if( !bCheck )
    {
        // Zeilenhoehe einstellen
        SetLineHeight( *pLine, 0, rParam.bBigger ? nDist : -nDist,
                        rParam.bBigger );
    }
    else if( !rParam.bBigger )
    {
        // anhand der alten Size die neue relative errechnen
        SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
        ASSERT( pLineFrm, "wo ist der Frm von der SwTableLine?" );
        SwTwips nRstHeight = CalcRowRstHeight( pLineFrm );
        if( nRstHeight < nDist )
            bRet = FALSE;
    }
    return bRet;
}

BOOL lcl_SetOtherLineHeight( SwTableLine* pLine, CR_SetLineHeight& rParam,
                                SwTwips nDist, BOOL bCheck )
{
    BOOL bRet = TRUE;
    if( bCheck )
    {
        if( rParam.bBigger )
        {
            // anhand der alten Size die neue relative errechnen
            SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
            ASSERT( pLineFrm, "wo ist der Frm von der SwTableLine?" );

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
        // Zeilenhoehe einstellen
        // pLine ist die nachfolgende / vorhergehende -> also anpassen
        if( TBLFIX_CHGPROP == rParam.nMode )
        {
            SwLayoutFrm* pLineFrm = GetRowFrm( *pLine );
            ASSERT( pLineFrm, "wo ist der Frm von der SwTableLine?" );

            // aus der alten Size die neue relative errechnen
            // Wird die selektierte Box groesser ueber den MaxSpace anpassen,
            // sonst ueber die MaxHeight
            if( 1 /*!rParam.bBigger*/ )
            {
                nDist *= pLineFrm->Frm().Height();
                nDist /= rParam.nMaxHeight;
            }
            else
            {
                // aus der alten Size die neue relative errechnen
                nDist *= CalcRowRstHeight( pLineFrm );
                nDist /= rParam.nMaxSpace;
            }
        }
        SetLineHeight( *pLine, 0, rParam.bBigger ? -nDist : nDist,
                        !rParam.bBigger );
    }
    return bRet;
}

BOOL lcl_InsDelSelLine( SwTableLine* pLine, CR_SetLineHeight& rParam,
                            SwTwips nDist, BOOL bCheck )
{
    BOOL bRet = TRUE;
    if( !bCheck )
    {
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwDoc* pDoc = pLine->GetFrmFmt()->GetDoc();
        if( !rParam.bBigger )
        {
            for( USHORT n = rBoxes.Count(); n; )
                ::lcl_SaveUpperLowerBorder( rParam.pTblNd->GetTable(),
                                                    *rBoxes[ --n ],
                                                    rParam.aShareFmts );
            for( n = rBoxes.Count(); n; )
                ::_DeleteBox( rParam.pTblNd->GetTable(),
                                    rBoxes[ --n ], rParam.pUndo, FALSE,
                                    FALSE, &rParam.aShareFmts );
        }
        else
        {
            // Zeile einfuegen
            SwTableLine* pNewLine = new SwTableLine( (SwTableLineFmt*)pLine->GetFrmFmt(),
                                        rBoxes.Count(), pLine->GetUpper() );
            SwTableLines* pLines;
            if( pLine->GetUpper() )
                pLines = &pLine->GetUpper()->GetTabLines();
            else
                pLines = &rParam.pTblNd->GetTable().GetTabLines();
            USHORT nPos = pLines->C40_GETPOS( SwTableLine, pLine );
            if( !rParam.bTop )
                ++nPos;
            pLines->C40_INSERT( SwTableLine, pNewLine, nPos );

            SwFrmFmt* pNewFmt = pNewLine->ClaimFrmFmt();
            pNewFmt->SetAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nDist ) );

            // und noch mal die Anzahl Boxen erzeugen
            SwTableBoxes& rNewBoxes = pNewLine->GetTabBoxes();
            for( USHORT n = 0; n < rBoxes.Count(); ++n )
            {
                SwTwips nWidth = 0;
                SwTableBox* pOld = rBoxes[ n ];
                if( !pOld->GetSttNd() )
                {
                    // keine normale "Content"-Box also auf die 1. naechste
                    // Box zurueckfallen
                    nWidth = pOld->GetFrmFmt()->GetFrmSize().GetWidth();
                    while( !pOld->GetSttNd() )
                        pOld = pOld->GetTabLines()[ 0 ]->GetTabBoxes()[ 0 ];
                }
                ::_InsTblBox( pDoc, rParam.pTblNd, pNewLine,
                                    (SwTableBoxFmt*)pOld->GetFrmFmt(), pOld, n );

                // Sonderbehandlung fuer Umrandung die Obere muss
                // entfernt werden
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
        // Boxen einsammeln!
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for( USHORT n = rBoxes.Count(); n; )
        {
            SwTableBox* pBox = rBoxes[ --n ];
            if( pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                return FALSE;

            if( pBox->GetSttNd() )
                rParam.aBoxes.Insert( pBox );
            else
            {
                for( USHORT i = pBox->GetTabLines().Count(); i; )
                    lcl_InsDelSelLine( pBox->GetTabLines()[ --i ],
                                        rParam, 0, TRUE );
            }
        }
    }
    return bRet;
}

BOOL SwTable::SetRowHeight( SwTableBox& rAktBox, USHORT eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff,SwUndo** ppUndo )
{
    SwTableLine* pLine = rAktBox.GetUpper();

    SwTableLine* pBaseLine = pLine;
    while( pBaseLine->GetUpper() )
        pBaseLine = pBaseLine->GetUpper()->GetUpper();

    _FndBox* pFndBox = 0;                   // fuers Einfuegen/Loeschen
    SwTableSortBoxes aTmpLst( 0, 5 );       // fuers Undo
    BOOL bBigger,
        bRet = FALSE,
        bTop = WH_ROW_TOP == ( eType & 0xff ) ||
                WH_CELL_TOP == ( eType & 0xff ),
        bInsDel = 0 != (eType & WH_FLAG_INSDEL );
    USHORT n, nBaseLinePos = GetTabLines().C40_GETPOS( SwTableLine, pBaseLine );
    ULONG nBoxIdx = rAktBox.GetSttIdx();

    CR_SetLineHeight aParam( eType,
                        (SwTableNode*)rAktBox.GetSttNd()->FindTableNode() );
    bBigger = aParam.bBigger;

    FN_lcl_SetLineHeight fnSelLine, fnOtherLine = lcl_SetOtherLineHeight;
    if( bInsDel )
        fnSelLine = lcl_InsDelSelLine;
    else
        fnSelLine = lcl_SetSelLineHeight;

    SwTableLines* pLines = &aLines;

    // wie kommt man an die Hoehen heran?
    switch( eType & 0xff )
    {
    case WH_CELL_TOP:
    case WH_CELL_BOTTOM:
        if( pLine == pBaseLine )
            break;  // dann geht es nicht!

        // ist eine verschachtelte Line (Box!)
        pLines = &pLine->GetUpper()->GetTabLines();
        nBaseLinePos = pLines->C40_GETPOS( SwTableLine, pLine );
        pBaseLine = pLine;
        // kein break!

    case WH_ROW_TOP:
    case WH_ROW_BOTTOM:
        {
            if( bInsDel && !bBigger )       // um wieviel wird es Hoeher?
            {
                nAbsDiff = GetRowFrm( *pBaseLine )->Frm().Height();
            }

            if( TBLVAR_CHGABS == eTblChgMode )
            {
                // erstmal testen, ob ueberhaupt Platz ist
                if( bBigger )
                {
                    bRet = TRUE;
// was ist mit Top, was ist mit Tabelle im Rahmen oder in Kopf-/Fusszeile
// mit fester Hoehe ??
                    if( !bRet )
                    {
                        // dann sich selbst rekursiv aufrufen; nur mit
                        // einem anderen Mode -> proprotional
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
                                        nAbsDiff, TRUE );

                if( bRet )
                {
                    if( bInsDel )
                    {
                        if( !aParam.aBoxes.Count() )
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, TRUE );

                        pFndBox = ::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst );
                        if( ppUndo )
                            *ppUndo = aParam.CreateUndo(
                                        bBigger ? UNDO_TABLE_INSROW
                                                : UNDO_TABLE_DELBOX );
                    }
                    else if( ppUndo )
                        *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, TRUE );

                    (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                    nAbsDiff, FALSE );
                }
            }
            else
            {
                bRet = TRUE;
                USHORT nStt, nEnd;
                if( bTop )
                    nStt = 0, nEnd = nBaseLinePos;
                else
                    nStt = nBaseLinePos + 1, nEnd = pLines->Count();

                // die akt. Hoehe der Lines besorgen
                if( TBLFIX_CHGPROP == eTblChgMode )
                {
                    for( n = nStt; n < nEnd; ++n )
                    {
                        SwLayoutFrm* pLineFrm = GetRowFrm( *(*pLines)[ n ] );
                        ASSERT( pLineFrm, "wo ist der Frm von der SwTableLine?" );
                        aParam.nMaxSpace += CalcRowRstHeight( pLineFrm );
                        aParam.nMaxHeight += pLineFrm->Frm().Height();
                    }
                    if( bBigger && aParam.nMaxSpace < nAbsDiff )
                        bRet = FALSE;
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
                        bRet = FALSE;
                }

                if( bRet )
                {
                    if( bBigger )
                    {
                        for( n = nStt; n < nEnd; ++n )
                        {
                            if( !(*fnOtherLine)( (*pLines)[ n ], aParam,
                                                    nAbsDiff, TRUE ))
                            {
                                bRet = FALSE;
                                break;
                            }
                        }
                    }
                    else
                        bRet = (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                                nAbsDiff, TRUE );
                }

                if( bRet )
                {
                    // dann mal anpassen
                    if( bInsDel )
                    {
                        if( !aParam.aBoxes.Count() )
                            ::lcl_InsDelSelLine( (*pLines)[ nBaseLinePos ],
                                                    aParam, 0, TRUE );
                        pFndBox = ::lcl_SaveInsDelData( aParam, ppUndo, aTmpLst );
                        if( ppUndo )
                            *ppUndo = aParam.CreateUndo(
                                        bBigger ? UNDO_TABLE_INSROW
                                                : UNDO_TABLE_DELBOX );
                    }
                    else if( ppUndo )
                        *ppUndo = new SwUndoAttrTbl( *aParam.pTblNd, TRUE );

                    CR_SetLineHeight aParam1( aParam );
                    if( TBLFIX_CHGPROP == eTblChgMode && !bBigger &&
                        !aParam.nMaxSpace )
                    {
                        // dann muss der gesamte Platz auf alle Lines
                        // gleichmaessig verteilt werden. Dafuer wird die
                        // Anzahl benoetigt
                        aParam1.nLines = nEnd - nStt;
                    }

                    if( bTop )
                    {
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, FALSE );
                        for( n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, FALSE );
                    }
                    else
                    {
                        for( n = nStt; n < nEnd; ++n )
                            (*fnOtherLine)( (*pLines)[ n ], aParam1,
                                            nAbsDiff, FALSE );
                        (*fnSelLine)( (*pLines)[ nBaseLinePos ], aParam,
                                        nAbsDiff, FALSE );
                    }
                }
                else
                {
                    // dann sich selbst rekursiv aufrufen; nur mit
                    // einem anderen Mode -> proprotional
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
        // dann raeume die Struktur aller Lines auf
        GCLines();

        //Layout updaten
        if( bBigger || pFndBox->AreLinesToRestore( *this ) )
            pFndBox->MakeFrms( *this );
        pFndBox->RestoreChartData( *this );
        delete pFndBox;

        if( ppUndo && *ppUndo )
        {
            aParam.pUndo->SetColWidthParam( nBoxIdx, eTblChgMode, eType,
                                            nAbsDiff, nRelDiff );
            if( bBigger )
                aParam.pUndo->SaveNewBoxes( *aParam.pTblNd, aTmpLst );
        }
    }

    return bRet;
}

/*  */

SwFrmFmt* SwShareBoxFmt::GetFormat( long nWidth ) const
{
    SwFrmFmt *pRet = 0, *pTmp;
    for( USHORT n = aNewFmts.Count(); n; )
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
    USHORT nWhich = rItem.Which();
    SwFrmFmt *pRet = 0, *pTmp;
    const SfxPoolItem& rFrmSz = pOldFmt->GetAttr( RES_FRM_SIZE, FALSE );
    for( USHORT n = aNewFmts.Count(); n; )
        if( SFX_ITEM_SET == ( pTmp = (SwFrmFmt*)aNewFmts[ --n ])->
            GetItemState( nWhich, FALSE, &pItem ) && *pItem == rItem &&
            pTmp->GetAttr( RES_FRM_SIZE, FALSE ) == rFrmSz )
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

FASTBOOL SwShareBoxFmt::RemoveFormat( const SwFrmFmt& rFmt )
{
    // returnt TRUE, wenn geloescht werden kann
    if( pOldFmt == &rFmt )
        return TRUE;

    void* p = (void*)&rFmt;
    USHORT nFnd = aNewFmts.GetPos( p );
    if( USHRT_MAX != nFnd )
        aNewFmts.Remove( nFnd );
    return 0 == aNewFmts.Count();
}

SwShareBoxFmts::~SwShareBoxFmts()
{
}

SwFrmFmt* SwShareBoxFmts::GetFormat( const SwFrmFmt& rFmt, long nWidth ) const
{
    USHORT nPos;
    return Seek_Entry( rFmt, &nPos )
                    ? aShareArr[ nPos ]->GetFormat( nWidth )
                    : 0;
}
SwFrmFmt* SwShareBoxFmts::GetFormat( const SwFrmFmt& rFmt,
                                     const SfxPoolItem& rItem ) const
{
    USHORT nPos;
    return Seek_Entry( rFmt, &nPos )
                    ? aShareArr[ nPos ]->GetFormat( rItem )
                    : 0;
}

void SwShareBoxFmts::AddFormat( const SwFrmFmt& rOld, const SwFrmFmt& rNew )
{
    // wenn das Format nicht geshared ist, braucht es auch nicht in die
    // Liste aufgenommen werden. Denn es gibt keinen 2. der es sucht.
//leider werden auch die CellFrms gefunden
//  if( !rOld.IsLastDepend() )
    {
        USHORT nPos;
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
        pRet->SetAttr( rSz );
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
        pRet->SetAttr( rItem );
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
        pRet->SetAttr( rItem );
        AddFormat( *pLineFmt, *pRet );
    }
}

void SwShareBoxFmts::RemoveFormat( const SwFrmFmt& rFmt )
{
    for( USHORT i = aShareArr.Count(); i; )
        if( aShareArr[ --i ]->RemoveFormat( rFmt ))
            aShareArr.DeleteAndDestroy( i );
}

BOOL SwShareBoxFmts::Seek_Entry( const SwFrmFmt& rFmt, USHORT* pPos ) const
{
    ULONG nIdx = (ULONG)&rFmt;
    register USHORT nO = aShareArr.Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            ULONG nFmt = (ULONG)&aShareArr[ nM ]->GetOldFormat();
            if( nFmt == nIdx )
            {
                if( pPos )
                    *pPos = nM;
                return TRUE;
            }
            else if( nFmt < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pPos )
                    *pPos = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pPos )
        *pPos = nU;
    return FALSE;
}


