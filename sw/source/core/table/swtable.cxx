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

#include <ctype.h>
#include <float.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/colritem.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/boxitem.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <docary.hxx>
#include <frame.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <tabcol.hxx>
#include <tabfrm.hxx>
#include <cellfrm.hxx>
#include <rowfrm.hxx>
#include <swserv.hxx>
#include <expfld.hxx>
#include <mdiexp.hxx>
#include <cellatr.hxx>
#include <txatbase.hxx>
#include <htmltbl.hxx>
#include <swtblfmt.hxx>
#include <ndindex.hxx>
#include <tblrwcl.hxx>
#include <shellres.hxx>
#include <viewsh.hxx>
#include <redline.hxx>
#include <list>
#include <calbck.hxx>

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

using namespace com::sun::star;

TYPEINIT1( SwTable, SwClient );
TYPEINIT1( SwTableBox, SwClient );
TYPEINIT1( SwTableLine, SwClient );
TYPEINIT1( SwTableFmt, SwFrmFmt );
TYPEINIT1( SwTableBoxFmt, SwFrmFmt );
TYPEINIT1( SwTableLineFmt, SwFrmFmt );

#define COLFUZZY 20

void ChgTextToNum( SwTableBox& rBox, const OUString& rTxt, const Color* pCol,
                    bool bChgAlign, sal_uLong nNdPos );

class SwTableBox_Impl
{
    Color *mpUserColor, *mpNumFmtColor;
    long mnRowSpan;
    bool mbDummyFlag;

    static void SetNewCol( Color** ppCol, const Color* pNewCol );
public:
    SwTableBox_Impl() : mpUserColor(0), mpNumFmtColor(0), mnRowSpan(1),
        mbDummyFlag( false ) {}
    ~SwTableBox_Impl() { delete mpUserColor; delete mpNumFmtColor; }

    const Color* GetSaveUserColor() const       { return mpUserColor; }
    const Color* GetSaveNumFmtColor() const     { return mpNumFmtColor; }
    void SetSaveUserColor(const Color* p )      { SetNewCol( &mpUserColor, p ); }
    void SetSaveNumFmtColor( const Color* p )   { SetNewCol( &mpNumFmtColor, p ); }
    long getRowSpan() const { return mnRowSpan; }
    void setRowSpan( long nNewRowSpan ) { mnRowSpan = nNewRowSpan; }
    bool getDummyFlag() const { return mbDummyFlag; }
    void setDummyFlag( bool bDummy ) { mbDummyFlag = bDummy; }
};

inline const Color* SwTableBox::GetSaveUserColor() const
{
    return pImpl ? pImpl->GetSaveUserColor() : 0;
}

inline const Color* SwTableBox::GetSaveNumFmtColor() const
{
    return pImpl ? pImpl->GetSaveNumFmtColor() : 0;
}

inline void SwTableBox::SetSaveUserColor(const Color* p )
{
    if( pImpl )
        pImpl->SetSaveUserColor( p );
    else if( p )
        ( pImpl = new SwTableBox_Impl ) ->SetSaveUserColor( p );
}

inline void SwTableBox::SetSaveNumFmtColor( const Color* p )
{
    if( pImpl )
        pImpl->SetSaveNumFmtColor( p );
    else if( p )
        ( pImpl = new SwTableBox_Impl )->SetSaveNumFmtColor( p );
}

long SwTableBox::getRowSpan() const
{
    return pImpl ? pImpl->getRowSpan() : 1;
}

void SwTableBox::setRowSpan( long nNewRowSpan )
{
    if( !pImpl )
    {
        if( nNewRowSpan == 1 )
            return;
        pImpl = new SwTableBox_Impl();
    }
    pImpl->setRowSpan( nNewRowSpan );
}

bool SwTableBox::getDummyFlag() const
{
    return pImpl && pImpl->getDummyFlag();
}

void SwTableBox::setDummyFlag( bool bDummy )
{
    if( !pImpl )
    {
        if( !bDummy )
            return;
        pImpl = new SwTableBox_Impl();
    }
    pImpl->setDummyFlag( bDummy );
}

//JP 15.09.98: Bug 55741 - Keep tabs (front and rear)
static OUString& lcl_TabToBlankAtSttEnd( OUString& rTxt )
{
    sal_Unicode c;
    sal_Int32 n;

    for( n = 0; n < rTxt.getLength() && ' ' >= ( c = rTxt[n] ); ++n )
        if( '\x9' == c )
            rTxt = rTxt.replaceAt( n, 1, " " );
    for( n = rTxt.getLength(); n && ' ' >= ( c = rTxt[--n] ); )
        if( '\x9' == c )
            rTxt = rTxt.replaceAt( n, 1, " " );
    return rTxt;
}

static OUString& lcl_DelTabsAtSttEnd( OUString& rTxt )
{
    sal_Unicode c;
    sal_Int32 n;
    OUStringBuffer sBuff(rTxt);

    for( n = 0; n < sBuff.getLength() && ' ' >= ( c = sBuff[ n ]); ++n )
    {
        if( '\x9' == c )
            sBuff.remove( n--, 1 );
    }
    for( n = sBuff.getLength(); n && ' ' >= ( c = sBuff[ --n ]); )
    {
        if( '\x9' == c )
            sBuff.remove( n, 1 );
    }
    rTxt = sBuff.makeStringAndClear();
    return rTxt;
}

void _InsTblBox( SwDoc* pDoc, SwTableNode* pTblNd,
                        SwTableLine* pLine, SwTableBoxFmt* pBoxFrmFmt,
                        SwTableBox* pBox,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt )
{
    OSL_ENSURE( pBox->GetSttNd(), "Box with no start node" );
    SwNodeIndex aIdx( *pBox->GetSttNd(), +1 );
    SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = pDoc->GetNodes().GoNext( &aIdx );
    OSL_ENSURE( pCNd, "Box with no content node" );

    if( pCNd->IsTxtNode() )
    {
        if( pBox->GetSaveNumFmtColor() && pCNd->GetpSwAttrSet() )
        {
            SwAttrSet aAttrSet( *pCNd->GetpSwAttrSet() );
            if( pBox->GetSaveUserColor() )
                aAttrSet.Put( SvxColorItem( *pBox->GetSaveUserColor(), RES_CHRATR_COLOR ));
            else
                aAttrSet.ClearItem( RES_CHRATR_COLOR );
            pDoc->GetNodes().InsBoxen( pTblNd, pLine, pBoxFrmFmt,
                                    static_cast<SwTxtNode*>(pCNd)->GetTxtColl(),
                                    &aAttrSet, nInsPos, nCnt );
        }
        else
            pDoc->GetNodes().InsBoxen( pTblNd, pLine, pBoxFrmFmt,
                                    static_cast<SwTxtNode*>(pCNd)->GetTxtColl(),
                                    pCNd->GetpSwAttrSet(),
                                    nInsPos, nCnt );
    }
    else
        pDoc->GetNodes().InsBoxen( pTblNd, pLine, pBoxFrmFmt,
                pDoc->GetDfltTxtFmtColl(), 0,
                nInsPos, nCnt );

    long nRowSpan = pBox->getRowSpan();
    if( nRowSpan != 1 )
    {
        SwTableBoxes& rTblBoxes = pLine->GetTabBoxes();
        for( sal_uInt16 i = 0; i < nCnt; ++i )
        {
            pBox = rTblBoxes[ i + nInsPos ];
            pBox->setRowSpan( nRowSpan );
        }
    }
}

SwTable::SwTable( SwTableFmt* pFmt )
    : SwClient( pFmt ),
    pHTMLLayout( 0 ),
    pTableNode( 0 ),
    nGrfsThatResize( 0 ),
    nRowsToRepeat( 1 ),
    bModifyLocked( false ),
    bNewModel( true )
{
    // default value set in the options
    eTblChgMode = (TblChgMode)GetTblChgDefaultMode();
}

SwTable::SwTable( const SwTable& rTable )
    : SwClient( rTable.GetFrmFmt() ),
    pHTMLLayout( 0 ),
    pTableNode( 0 ),
    eTblChgMode( rTable.eTblChgMode ),
    nGrfsThatResize( 0 ),
    nRowsToRepeat( rTable.GetRowsToRepeat() ),
    bModifyLocked( false ),
    bNewModel( rTable.bNewModel )
{
}

void DelBoxNode( SwTableSortBoxes& rSortCntBoxes )
{
    for (size_t n = 0; n < rSortCntBoxes.size(); ++n)
    {
        rSortCntBoxes[ n ]->pSttNd = 0;
    }
}

SwTable::~SwTable()
{
    if( refObj.Is() )
    {
        SwDoc* pDoc = GetFrmFmt()->GetDoc();
        if( !pDoc->IsInDtor() )         // then remove from the list
            pDoc->getIDocumentLinksAdministration().GetLinkManager().RemoveServer( &refObj );

        refObj->Closed();
    }

    // the table can be deleted if it's the last client of the FrameFormat
    SwTableFmt* pFmt = static_cast<SwTableFmt*>(GetFrmFmt());
    pFmt->Remove( this );               // remove

    if( !pFmt->HasWriterListeners() )
        pFmt->GetDoc()->DelTblFrmFmt( pFmt );   // and delete

    // Delete the pointers from the SortArray of the boxes. The objects
    // are preserved and are deleted by the lines/boxes arrays dtor.
    // Note: unfortunately not enough, pointers to the StartNode of the
    // section need deletion.
    DelBoxNode(m_TabSortContentBoxes);
    m_TabSortContentBoxes.clear();
    delete pHTMLLayout;
}

namespace
{

template<class T>
inline T lcl_MulDiv64(sal_uInt64 nA, sal_uInt64 nM, sal_uInt64 nD)
{
    return static_cast<T>((nA*nM)/nD);
}

}

static void FmtInArr( std::vector<SwFmt*>& rFmtArr, SwFmt* pBoxFmt )
{
    std::vector<SwFmt*>::const_iterator it = std::find( rFmtArr.begin(), rFmtArr.end(), pBoxFmt );
    if ( it == rFmtArr.end() )
        rFmtArr.push_back( pBoxFmt );
}

static void lcl_ModifyBoxes( SwTableBoxes &rBoxes, const long nOld,
                         const long nNew, std::vector<SwFmt*>& rFmtArr );

static void lcl_ModifyLines( SwTableLines &rLines, const long nOld,
                         const long nNew, std::vector<SwFmt*>& rFmtArr, const bool bCheckSum )
{
    for ( size_t i = 0; i < rLines.size(); ++i )
        ::lcl_ModifyBoxes( rLines[i]->GetTabBoxes(), nOld, nNew, rFmtArr );
    if( bCheckSum )
    {
        for( size_t i = 0; i < rFmtArr.size(); ++i )
        {
            SwFmt* pFmt = rFmtArr[i];
            const SwTwips nBox = lcl_MulDiv64<SwTwips>(pFmt->GetFrmSize().GetWidth(), nNew, nOld);
            SwFmtFrmSize aNewBox( ATT_VAR_SIZE, nBox, 0 );
            pFmt->LockModify();
            pFmt->SetFmtAttr( aNewBox );
            pFmt->UnlockModify();
        }
    }
}

static void lcl_ModifyBoxes( SwTableBoxes &rBoxes, const long nOld,
                         const long nNew, std::vector<SwFmt*>& rFmtArr )
{
    sal_uInt64 nSum = 0; // To avoid rounding errors we summarize all box widths
    sal_uInt64 nOriginalSum = 0; // Sum of original widths
    for ( size_t i = 0; i < rBoxes.size(); ++i )
    {
        SwTableBox &rBox = *rBoxes[i];
        if ( !rBox.GetTabLines().empty() )
        {
            // For SubTables the rounding problem will not be solved :-(
            ::lcl_ModifyLines( rBox.GetTabLines(), nOld, nNew, rFmtArr, false );
        }
        // Adjust the box
        SwFrmFmt *pFmt = rBox.GetFrmFmt();
        sal_uInt64 nBox = pFmt->GetFrmSize().GetWidth();
        nOriginalSum += nBox;
        nBox *= nNew;
        nBox /= nOld;
        const sal_uInt64 nWishedSum = lcl_MulDiv64<sal_uInt64>(nOriginalSum, nNew, nOld) - nSum;
        if( nWishedSum > 0 )
        {
            if( nBox == nWishedSum )
                FmtInArr( rFmtArr, pFmt );
            else
            {
                nBox = nWishedSum;
                pFmt = rBox.ClaimFrmFmt();
                SwFmtFrmSize aNewBox( ATT_VAR_SIZE, static_cast< SwTwips >(nBox), 0 );
                pFmt->LockModify();
                pFmt->SetFmtAttr( aNewBox );
                pFmt->UnlockModify();
            }
        }
        else {
            OSL_FAIL( "Rounding error" );
        }
        nSum += nBox;
    }
}

void SwTable::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    // catch SSize changes, to adjust the lines/boxes
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    const SwFmtFrmSize* pNewSize = 0, *pOldSize = 0;

    if( RES_ATTRSET_CHG == nWhich )
    {
        if (pOld && pNew && SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState(
            RES_FRM_SIZE, false, reinterpret_cast<const SfxPoolItem**>(&pNewSize)))
        {
            pOldSize = &static_cast<const SwAttrSetChg*>(pOld)->GetChgSet()->GetFrmSize();
        }
    }
    else if( RES_FRM_SIZE == nWhich )
    {
        pOldSize = static_cast<const SwFmtFrmSize*>(pOld);
        pNewSize = static_cast<const SwFmtFrmSize*>(pNew);
    }
    else
        CheckRegistration( pOld, pNew );

    if (pOldSize && pNewSize && !IsModifyLocked())
        AdjustWidths( pOldSize->GetWidth(), pNewSize->GetWidth() );
}

void SwTable::AdjustWidths( const long nOld, const long nNew )
{
    std::vector<SwFmt*> aFmtArr;
    aFmtArr.reserve( aLines[0]->GetTabBoxes().size() );
    ::lcl_ModifyLines( aLines, nOld, nNew, aFmtArr, true );
}

static void lcl_RefreshHidden( SwTabCols &rToFill, size_t nPos )
{
    for ( size_t i = 0; i < rToFill.Count(); ++i )
    {
        if ( std::abs(static_cast<long>(nPos) - rToFill[i]) <= COLFUZZY )
        {
            rToFill.SetHidden( i, false );
            break;
        }
    }
}

static void lcl_SortedTabColInsert( SwTabCols &rToFill, const SwTableBox *pBox,
                   const SwFrmFmt *pTabFmt, const bool bHidden,
                   const bool bRefreshHidden )
{
    const long nWish = pTabFmt->GetFrmSize().GetWidth();
    OSL_ENSURE(nWish, "weird <= 0 width frmfrm");

    // The value for the left edge of the box is calculated from the
    // widths of the previous boxes.
    long nPos = 0;
    long nLeftMin = 0;
    long nRightMax = 0;
    if (nWish != 0) //fdo#33012 0 width frmfmt
    {
        SwTwips nSum = 0;
        const SwTableBox  *pCur  = pBox;
        const SwTableLine *pLine = pBox->GetUpper();
        const long nAct  = rToFill.GetRight() - rToFill.GetLeft();  // +1 why?

        while ( pLine )
        {
            const SwTableBoxes &rBoxes = pLine->GetTabBoxes();
            for ( size_t i = 0; i < rBoxes.size(); ++i )
            {
                const SwTwips nWidth = rBoxes[i]->GetFrmFmt()->GetFrmSize().GetWidth();
                nSum += nWidth;
                const long nTmp = lcl_MulDiv64<long>(nSum, nAct, nWish);

                if (rBoxes[i] != pCur)
                {
                    if ( pLine == pBox->GetUpper() || 0 == nLeftMin )
                        nLeftMin = nTmp - nPos;
                    nPos = nTmp;
                }
                else
                {
                    nSum -= nWidth;
                    if ( 0 == nRightMax )
                        nRightMax = nTmp - nPos;
                    break;
                }
            }
            pCur  = pLine->GetUpper();
            pLine = pCur ? pCur->GetUpper() : 0;
        }
    }

    bool bInsert = !bRefreshHidden;
    for ( size_t j = 0; bInsert && (j < rToFill.Count()); ++j )
    {
        long nCmp = rToFill[j];
        if ( (nPos >= ((nCmp >= COLFUZZY) ? nCmp - COLFUZZY : nCmp)) &&
             (nPos <= (nCmp + COLFUZZY)) )
        {
            bInsert = false;        // Already has it.
        }
        else if ( nPos < nCmp )
        {
            bInsert = false;
            rToFill.Insert( nPos, bHidden, j );
        }
    }
    if ( bInsert )
        rToFill.Insert( nPos, bHidden, rToFill.Count() );
    else if ( bRefreshHidden )
        ::lcl_RefreshHidden( rToFill, nPos );

    if ( bHidden && !bRefreshHidden )
    {
        // calculate minimum/maximum values for the existing entries:
        nLeftMin = nPos - nLeftMin;
        nRightMax = nPos + nRightMax;

        // check if nPos is entry:
        bool bFoundPos = false;
        bool bFoundMax = false;
        for ( size_t j = 0; !(bFoundPos && bFoundMax ) && j < rToFill.Count(); ++j )
        {
            SwTabColsEntry& rEntry = rToFill.GetEntry( j );
            long nCmp = rToFill[j];

            if ( (nPos >= ((nCmp >= COLFUZZY) ? nCmp - COLFUZZY : nCmp)) &&
                 (nPos <= (nCmp + COLFUZZY)) )
            {
                // check if nLeftMin is > old minimum for entry nPos:
                const long nOldMin = rEntry.nMin;
                if ( nLeftMin > nOldMin )
                    rEntry.nMin = nLeftMin;
                // check if nRightMin is < old maximum for entry nPos:
                const long nOldMax = rEntry.nMax;
                if ( nRightMax < nOldMax )
                    rEntry.nMax = nRightMax;

                bFoundPos = true;
            }
            else if ( (nRightMax >= ((nCmp >= COLFUZZY) ? nCmp - COLFUZZY : nCmp)) &&
                      (nRightMax <= (nCmp + COLFUZZY)) )
            {
                // check if nPos is > old minimum for entry nRightMax:
                const long nOldMin = rEntry.nMin;
                if ( nPos > nOldMin )
                    rEntry.nMin = nPos;

                bFoundMax = true;
            }
        }
    }
}

static void lcl_ProcessBoxGet( const SwTableBox *pBox, SwTabCols &rToFill,
                        const SwFrmFmt *pTabFmt, bool bRefreshHidden )
{
    if ( !pBox->GetTabLines().empty() )
    {
        const SwTableLines &rLines = pBox->GetTabLines();
        for ( size_t i = 0; i < rLines.size(); ++i )
        {
            const SwTableBoxes &rBoxes = rLines[i]->GetTabBoxes();
            for ( size_t j = 0; j < rBoxes.size(); ++j )
                ::lcl_ProcessBoxGet( rBoxes[j], rToFill, pTabFmt, bRefreshHidden);
        }
    }
    else
        ::lcl_SortedTabColInsert( rToFill, pBox, pTabFmt, false, bRefreshHidden );
}

static void lcl_ProcessLineGet( const SwTableLine *pLine, SwTabCols &rToFill,
                         const SwFrmFmt *pTabFmt )
{
    for ( size_t i = 0; i < pLine->GetTabBoxes().size(); ++i )
    {
        const SwTableBox *pBox = pLine->GetTabBoxes()[i];
        if ( pBox->GetSttNd() )
            ::lcl_SortedTabColInsert( rToFill, pBox, pTabFmt, true, false );
        else
            for ( size_t j = 0; j < pBox->GetTabLines().size(); ++j )
                ::lcl_ProcessLineGet( pBox->GetTabLines()[j], rToFill, pTabFmt );
    }
}

void SwTable::GetTabCols( SwTabCols &rToFill, const SwTableBox *pStart,
              bool bRefreshHidden, bool bCurRowOnly ) const
{
    // Optimization: if bHidden is set, we only update the Hidden Array.
    if ( bRefreshHidden )
    {
        // remove corrections
        for ( size_t i = 0; i < rToFill.Count(); ++i )
        {
            SwTabColsEntry& rEntry = rToFill.GetEntry( i );
            rEntry.nPos -= rToFill.GetLeft();
            rEntry.nMin -= rToFill.GetLeft();
            rEntry.nMax -= rToFill.GetLeft();
        }

        // All are hidden, so add the visible ones.
        for ( size_t i = 0; i < rToFill.Count(); ++i )
            rToFill.SetHidden( i, true );
    }
    else
    {
        rToFill.Remove( 0, rToFill.Count() );
    }

    // Insertion cases:
    // 1. All boxes which are inferior to Line which is superior to the Start,
    //    as well as their inferior boxes if present.
    // 2. Starting from the Line, the superior box plus its neighbours; but no inferiors.
    // 3. Apply 2. to the Line superior to the chain of boxes,
    //    until the Line's superior is not a box but the table.
    // Only those boxes are inserted that don't contain further rows. The insertion
    // function takes care to avoid duplicates. In order to achieve this, we work
    // with some degree of fuzzyness (to avoid rounding errors).
    // Only the left edge of the boxes are inserted.
    // Finally, the first entry is removed again, because it's already
    // covered by the border.
    // 4. Scan the table again and insert _all_ boxes, this time as hidden.

    const SwFrmFmt *pTabFmt = GetFrmFmt();

    // 1.
    const SwTableBoxes &rBoxes = pStart->GetUpper()->GetTabBoxes();

    for ( size_t i = 0; i < rBoxes.size(); ++i )
        ::lcl_ProcessBoxGet( rBoxes[i], rToFill, pTabFmt, bRefreshHidden );

    // 2. and 3.
    const SwTableLine *pLine = pStart->GetUpper()->GetUpper() ?
                                pStart->GetUpper()->GetUpper()->GetUpper() : 0;
    while ( pLine )
    {
        const SwTableBoxes &rBoxes2 = pLine->GetTabBoxes();
        for ( size_t k = 0; k < rBoxes2.size(); ++k )
            ::lcl_SortedTabColInsert( rToFill, rBoxes2[k],
                                      pTabFmt, false, bRefreshHidden );
        pLine = pLine->GetUpper() ? pLine->GetUpper()->GetUpper() : 0;
    }

    if ( !bRefreshHidden )
    {
        // 4.
        if ( !bCurRowOnly )
        {
            for ( size_t i = 0; i < aLines.size(); ++i )
                ::lcl_ProcessLineGet( aLines[i], rToFill, pTabFmt );
        }

        rToFill.Remove( 0, 1 );
    }

    // Now the coordinates are relative to the left table border - i.e.
    // relative to SwTabCols.nLeft. However, they are expected
    // relative to the left document border, i.e. SwTabCols.nLeftMin.
    // So all values need to be extended by nLeft.
    for ( size_t i = 0; i < rToFill.Count(); ++i )
    {
        SwTabColsEntry& rEntry = rToFill.GetEntry( i );
        rEntry.nPos += rToFill.GetLeft();
        rEntry.nMin += rToFill.GetLeft();
        rEntry.nMax += rToFill.GetLeft();
    }
}

// Structure for parameter passing
struct Parm
{
    const SwTabCols &rNew;
    const SwTabCols &rOld;
    long nNewWish,
         nOldWish;
    std::deque<SwTableBox*> aBoxArr;
    SwShareBoxFmts aShareFmts;

    Parm( const SwTabCols &rN, const SwTabCols &rO )
        : rNew( rN ), rOld( rO ), nNewWish(0), nOldWish(0)
    {}
};

static void lcl_ProcessBoxSet( SwTableBox *pBox, Parm &rParm );

static void lcl_ProcessLine( SwTableLine *pLine, Parm &rParm )
{
    SwTableBoxes &rBoxes = pLine->GetTabBoxes();
    for ( size_t i = rBoxes.size(); i > 0; )
    {
        --i;
        ::lcl_ProcessBoxSet( rBoxes[i], rParm );
    }
}

static void lcl_ProcessBoxSet( SwTableBox *pBox, Parm &rParm )
{
    if ( !pBox->GetTabLines().empty() )
    {
        SwTableLines &rLines = pBox->GetTabLines();
        for ( size_t i = rLines.size(); i > 0; )
        {
            --i;
            lcl_ProcessLine( rLines[i], rParm );
        }
    }
    else
    {
        // Search the old TabCols for the current position (calculate from
        // left and right edge). Adjust the box if the values differ from
        // the new TabCols. If the adjusted edge has no neighbour we also
        // adjust all superior boxes.

        const long nOldAct = rParm.rOld.GetRight() -
                             rParm.rOld.GetLeft(); // +1 why?

        // The value for the left edge of the box is calculated from the
        // widths of the previous boxes plus the left edge.
        long nLeft = rParm.rOld.GetLeft();
        const  SwTableBox  *pCur  = pBox;
        const  SwTableLine *pLine = pBox->GetUpper();

        while ( pLine )
        {
            const SwTableBoxes &rBoxes = pLine->GetTabBoxes();
            for ( size_t i = 0; (i < rBoxes.size()) && (rBoxes[i] != pCur); ++i)
            {
                nLeft += lcl_MulDiv64<long>(
                    rBoxes[i]->GetFrmFmt()->GetFrmSize().GetWidth(),
                    nOldAct, rParm.nOldWish);
            }
            pCur  = pLine->GetUpper();
            pLine = pCur ? pCur->GetUpper() : 0;
        }
        long nLeftDiff = 0;
        long nRightDiff = 0;
        if ( nLeft != rParm.rOld.GetLeft() ) // There are still boxes before this.
        {
            // Right edge is left edge plus width.
            const long nWidth = lcl_MulDiv64<long>(
                pBox->GetFrmFmt()->GetFrmSize().GetWidth(),
                nOldAct, rParm.nOldWish);
            const long nRight = nLeft + nWidth;
            size_t nLeftPos  = 0;
            size_t nRightPos = 0;
            bool bFoundLeftPos = false;
            bool bFoundRightPos = false;
            for ( size_t i = 0; i < rParm.rOld.Count(); ++i )
            {
                if ( nLeft >= (rParm.rOld[i] - COLFUZZY) &&
                     nLeft <= (rParm.rOld[i] + COLFUZZY) )
                {
                    nLeftPos = i;
                    bFoundLeftPos = true;
                }
                else if ( nRight >= (rParm.rOld[i] - COLFUZZY) &&
                          nRight <= (rParm.rOld[i] + COLFUZZY) )
                {
                    nRightPos = i;
                    bFoundRightPos = true;
                }
            }
            nLeftDiff = bFoundLeftPos ?
                rParm.rOld[nLeftPos] - rParm.rNew[nLeftPos] : 0;
            nRightDiff= bFoundRightPos ?
                rParm.rNew[nRightPos] - rParm.rOld[nRightPos] : 0;
        }
        else    // The first box.
        {
            nLeftDiff = rParm.rOld.GetLeft() - rParm.rNew.GetLeft();
            if ( rParm.rOld.Count() )
            {
                // Calculate the difference to the edge touching the first box.
                const long nWidth = lcl_MulDiv64<long>(
                    pBox->GetFrmFmt()->GetFrmSize().GetWidth(),
                    nOldAct, rParm.nOldWish);
                const long nTmp = nWidth + rParm.rOld.GetLeft();
                for ( size_t i = 0; i < rParm.rOld.Count(); ++i )
                {
                    if ( nTmp >= (rParm.rOld[i] - COLFUZZY) &&
                         nTmp <= (rParm.rOld[i] + COLFUZZY) )
                    {
                        nRightDiff = rParm.rNew[i] - rParm.rOld[i];
                        break;
                    }
                }
            }
        }

        if( pBox->getRowSpan() == 1 )
        {
            SwTableBoxes& rTblBoxes = pBox->GetUpper()->GetTabBoxes();
            const sal_uInt16 nPos = rTblBoxes.GetPos( pBox );
            if( nPos && rTblBoxes[ nPos - 1 ]->getRowSpan() != 1 )
                nLeftDiff = 0;
            if( nPos + 1 < (sal_uInt16)rTblBoxes.size() &&
                rTblBoxes[ nPos + 1 ]->getRowSpan() != 1 )
                nRightDiff = 0;
        }
        else
            nLeftDiff = nRightDiff = 0;

        if ( nLeftDiff || nRightDiff )
        {
            // The difference is the actual difference amount. For stretched
            // tables, it does not make sense to adjust the attributes of the
            // boxes by this amount. The difference amount needs to be converted
            // accordingly.
            long nTmp = rParm.rNew.GetRight() - rParm.rNew.GetLeft(); // +1 why?
            nLeftDiff *= rParm.nNewWish;
            nLeftDiff /= nTmp;
            nRightDiff *= rParm.nNewWish;
            nRightDiff /= nTmp;
            long nDiff = nLeftDiff + nRightDiff;

            // Adjust the box and all superiors by the difference amount.
            while ( pBox )
            {
                SwFmtFrmSize aFmtFrmSize( pBox->GetFrmFmt()->GetFrmSize() );
                aFmtFrmSize.SetWidth( aFmtFrmSize.GetWidth() + nDiff );
                if ( aFmtFrmSize.GetWidth() < 0 )
                    aFmtFrmSize.SetWidth( -aFmtFrmSize.GetWidth() );
                rParm.aShareFmts.SetSize( *pBox, aFmtFrmSize );

                // The outer cells of the last row are responsible to adjust a surrounding cell.
                // Last line check:
                if ( pBox->GetUpper()->GetUpper() &&
                     pBox->GetUpper() != pBox->GetUpper()->GetUpper()->GetTabLines().back())
                {
                    pBox = 0;
                }
                else
                {
                    // Middle cell check:
                    if ( pBox != pBox->GetUpper()->GetTabBoxes().front() )
                        nDiff = nRightDiff;

                    if ( pBox != pBox->GetUpper()->GetTabBoxes().back() )
                        nDiff -= nRightDiff;

                    pBox = nDiff ? pBox->GetUpper()->GetUpper() : 0;
                }
            }
        }
    }
}

static void lcl_ProcessBoxPtr( SwTableBox *pBox, std::deque<SwTableBox*> &rBoxArr,
                           bool bBefore )
{
    if ( !pBox->GetTabLines().empty() )
    {
        const SwTableLines &rLines = pBox->GetTabLines();
        for ( size_t i = 0; i < rLines.size(); ++i )
        {
            const SwTableBoxes &rBoxes = rLines[i]->GetTabBoxes();
            for ( size_t j = 0; j < rBoxes.size(); ++j )
                ::lcl_ProcessBoxPtr( rBoxes[j], rBoxArr, bBefore );
        }
    }
    else if ( bBefore )
        rBoxArr.push_front( pBox );
    else
        rBoxArr.push_back( pBox );
}

static void lcl_AdjustBox( SwTableBox *pBox, const long nDiff, Parm &rParm );

static void lcl_AdjustLines( SwTableLines &rLines, const long nDiff, Parm &rParm )
{
    for ( size_t i = 0; i < rLines.size(); ++i )
    {
        SwTableBox *pBox = rLines[i]->GetTabBoxes()
                                [rLines[i]->GetTabBoxes().size()-1];
        lcl_AdjustBox( pBox, nDiff, rParm );
    }
}

static void lcl_AdjustBox( SwTableBox *pBox, const long nDiff, Parm &rParm )
{
    if ( !pBox->GetTabLines().empty() )
        ::lcl_AdjustLines( pBox->GetTabLines(), nDiff, rParm );

    // Adjust the size of the box.
    SwFmtFrmSize aFmtFrmSize( pBox->GetFrmFmt()->GetFrmSize() );
    aFmtFrmSize.SetWidth( aFmtFrmSize.GetWidth() + nDiff );

    rParm.aShareFmts.SetSize( *pBox, aFmtFrmSize );
}

void SwTable::SetTabCols( const SwTabCols &rNew, const SwTabCols &rOld,
                          const SwTableBox *pStart, bool bCurRowOnly )
{
    CHECK_TABLE( *this )

    SetHTMLTableLayout( 0 );    // delete HTML-Layout

    // FME: Made rOld const. The caller is responsible for passing correct
    // values of rOld. Therefore we do not have to call GetTabCols anymore:
    //GetTabCols( rOld, pStart );

    Parm aParm( rNew, rOld );

    OSL_ENSURE( rOld.Count() == rNew.Count(), "Columnanzahl veraendert.");

    // Convert the edges. We need to adjust the size of the table and some boxes.
    // For the size adjustment, we must not make use of the Modify, since that'd
    // adjust all boxes, which we really don't want.
    SwFrmFmt *pFmt = GetFrmFmt();
    aParm.nOldWish = aParm.nNewWish = pFmt->GetFrmSize().GetWidth();
    if ( (rOld.GetLeft() != rNew.GetLeft()) ||
         (rOld.GetRight()!= rNew.GetRight()) )
    {
        LockModify();
        {
            SvxLRSpaceItem aLR( pFmt->GetLRSpace() );
            SvxShadowItem aSh( pFmt->GetShadow() );

            SwTwips nShRight = aSh.CalcShadowSpace( SvxShadowItemSide::RIGHT );
            SwTwips nShLeft = aSh.CalcShadowSpace( SvxShadowItemSide::LEFT );

            aLR.SetLeft ( rNew.GetLeft() - nShLeft );
            aLR.SetRight( rNew.GetRightMax() - rNew.GetRight() - nShRight );
            pFmt->SetFmtAttr( aLR );

            // The alignment of the table needs to be adjusted accordingly.
            // This is done by preserving the exact positions that have been
            // set by the user.
            SwFmtHoriOrient aOri( pFmt->GetHoriOrient() );
            if(text::HoriOrientation::NONE != aOri.GetHoriOrient())
            {
                const bool bLeftDist = rNew.GetLeft() != nShLeft;
                const bool bRightDist = rNew.GetRight() + nShRight != rNew.GetRightMax();
                if(!bLeftDist && !bRightDist)
                    aOri.SetHoriOrient( text::HoriOrientation::FULL );
                else if(!bRightDist && rNew.GetLeft() > nShLeft )
                    aOri.SetHoriOrient( text::HoriOrientation::RIGHT );
                else if(!bLeftDist && rNew.GetRight() + nShRight < rNew.GetRightMax())
                    aOri.SetHoriOrient( text::HoriOrientation::LEFT );
                else
                    aOri.SetHoriOrient( text::HoriOrientation::LEFT_AND_WIDTH );
            }
            pFmt->SetFmtAttr( aOri );
        }
        const long nAct = rOld.GetRight() - rOld.GetLeft(); // +1 why?
        long nTabDiff = 0;

        if ( rOld.GetLeft() != rNew.GetLeft() )
        {
            nTabDiff = rOld.GetLeft() - rNew.GetLeft();
            nTabDiff *= aParm.nOldWish;
            nTabDiff /= nAct;
        }
        if ( rOld.GetRight() != rNew.GetRight() )
        {
            long nDiff = rNew.GetRight() - rOld.GetRight();
            nDiff *= aParm.nOldWish;
            nDiff /= nAct;
            nTabDiff += nDiff;
            if( !IsNewModel() )
                ::lcl_AdjustLines( GetTabLines(), nDiff, aParm );
        }

        // Adjust the size of the table, watch out for stretched tables.
        if ( nTabDiff )
        {
            aParm.nNewWish += nTabDiff;
            if ( aParm.nNewWish < 0 )
                aParm.nNewWish = USHRT_MAX; // Oops! Have to roll back.
            SwFmtFrmSize aSz( pFmt->GetFrmSize() );
            if ( aSz.GetWidth() != aParm.nNewWish )
            {
                aSz.SetWidth( aParm.nNewWish );
                aSz.SetWidthPercent( 0 );
                pFmt->SetFmtAttr( aSz );
            }
        }
        UnlockModify();
    }

    if( IsNewModel() )
        NewSetTabCols( aParm, rNew, rOld, pStart, bCurRowOnly );
    else
    {
        if ( bCurRowOnly )
        {
            // To adjust the current row, we need to process all its boxes,
            // similar to the filling of the TabCols (see GetTabCols()).
            // Unfortunately we again have to take care to adjust the boxes
            // from back to front, respectively from outer to inner.
            // The best way to achieve this is probably to track the boxes
            // in a PtrArray.
            const SwTableBoxes &rBoxes = pStart->GetUpper()->GetTabBoxes();
            for ( size_t i = 0; i < rBoxes.size(); ++i )
                ::lcl_ProcessBoxPtr( rBoxes[i], aParm.aBoxArr, false );

            const SwTableLine *pLine = pStart->GetUpper()->GetUpper() ?
                                    pStart->GetUpper()->GetUpper()->GetUpper() : 0;
            const SwTableBox  *pExcl = pStart->GetUpper()->GetUpper();
            while ( pLine )
            {
                const SwTableBoxes &rBoxes2 = pLine->GetTabBoxes();
                bool bBefore = true;
                for ( size_t i = 0; i < rBoxes2.size(); ++i )
                {
                    if ( rBoxes2[i] != pExcl )
                        ::lcl_ProcessBoxPtr( rBoxes2[i], aParm.aBoxArr, bBefore );
                    else
                        bBefore = false;
                }
                pExcl = pLine->GetUpper();
                pLine = pLine->GetUpper() ? pLine->GetUpper()->GetUpper() : 0;
            }
            // After we've inserted a bunch of boxes (hopefully all and in
            // correct order), we just need to process them in reverse order.
            for ( int j = aParm.aBoxArr.size()-1; j >= 0; --j )
            {
                SwTableBox *pBox = aParm.aBoxArr[j];
                ::lcl_ProcessBoxSet( pBox, aParm );
            }
        }
        else
        {
            // Adjusting the entire table is 'easy'. All boxes without lines are
            // adjusted, as are their superiors. Of course we need to process
            // in reverse order to prevent fooling ourselves!
            SwTableLines &rLines = GetTabLines();
            for ( size_t i = rLines.size(); i > 0; )
            {
                --i;
                ::lcl_ProcessLine( rLines[i], aParm );
            }
        }
    }

#ifdef DBG_UTIL
    {
        // do some checking for correct table widths
        SwTwips nSize = GetFrmFmt()->GetFrmSize().GetWidth();
        for (size_t n = 0; n < aLines.size(); ++n)
        {
            _CheckBoxWidth( *aLines[ n ], nSize );
        }
    }
#endif
}

typedef std::pair<sal_uInt16, sal_uInt16> ColChange;
typedef std::list< ColChange > ChangeList;

static void lcl_AdjustWidthsInLine( SwTableLine* pLine, ChangeList& rOldNew,
    Parm& rParm, sal_uInt16 nColFuzzy )
{
    ChangeList::iterator pCurr = rOldNew.begin();
    if( pCurr == rOldNew.end() )
        return;
    const size_t nCount = pLine->GetTabBoxes().size();
    SwTwips nBorder = 0;
    SwTwips nRest = 0;
    for( size_t i = 0; i < nCount; ++i )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[i];
        SwTwips nWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        SwTwips nNewWidth = nWidth - nRest;
        nRest = 0;
        nBorder += nWidth;
        if( pCurr != rOldNew.end() && nBorder + nColFuzzy >= pCurr->first )
        {
            nBorder -= nColFuzzy;
            while( pCurr != rOldNew.end() && nBorder > pCurr->first )
                ++pCurr;
            if( pCurr != rOldNew.end() )
            {
                nBorder += nColFuzzy;
                if( nBorder + nColFuzzy >= pCurr->first )
                {
                    if( pCurr->second == pCurr->first )
                        nRest = 0;
                    else
                        nRest = pCurr->second - nBorder;
                    nNewWidth += nRest;
                    ++pCurr;
                }
            }
        }
        if( nNewWidth != nWidth )
        {
            if( nNewWidth < 0 )
            {
                nRest += 1 - nNewWidth;
                nNewWidth = 1;
            }
            SwFmtFrmSize aFmtFrmSize( pBox->GetFrmFmt()->GetFrmSize() );
            aFmtFrmSize.SetWidth( nNewWidth );
            rParm.aShareFmts.SetSize( *pBox, aFmtFrmSize );
        }
    }
}

static void lcl_CalcNewWidths( std::list<sal_uInt16> &rSpanPos, ChangeList& rChanges,
    SwTableLine* pLine, long nWish, long nWidth, bool bTop )
{
    if( rChanges.empty() )
    {
        rSpanPos.clear();
        return;
    }
    if( rSpanPos.empty() )
    {
        rChanges.clear();
        return;
    }
    std::list<sal_uInt16> aNewSpanPos;
    ChangeList aNewChanges;
    ChangeList::iterator pCurr = rChanges.begin();
    aNewChanges.push_back( *pCurr ); // Nullposition
    std::list<sal_uInt16>::iterator pSpan = rSpanPos.begin();
    sal_uInt16 nCurr = 0;
    SwTwips nOrgSum = 0;
    bool bRowSpan = false;
    sal_uInt16 nRowSpanCount = 0;
    const size_t nCount = pLine->GetTabBoxes().size();
    for( size_t nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox];
        SwTwips nCurrWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        const long nRowSpan = pBox->getRowSpan();
        const bool bCurrRowSpan = bTop ? nRowSpan < 0 :
            ( nRowSpan > 1 || nRowSpan < -1 );
        if( bRowSpan || bCurrRowSpan )
            aNewSpanPos.push_back( nRowSpanCount );
        bRowSpan = bCurrRowSpan;
        nOrgSum += nCurrWidth;
        const sal_uInt16 nPos = lcl_MulDiv64<sal_uInt16>(
            lcl_MulDiv64<sal_uInt64>(nOrgSum, nWidth, nWish),
            nWish, nWidth);
        while( pCurr != rChanges.end() && pCurr->first < nPos )
        {
            ++nCurr;
            ++pCurr;
        }
        bool bNew = true;
        if( pCurr != rChanges.end() && pCurr->first <= nPos &&
            pCurr->first != pCurr->second )
        {
            while( pSpan != rSpanPos.end() && *pSpan < nCurr )
                ++pSpan;
            if( pSpan != rSpanPos.end() && *pSpan == nCurr )
            {
                aNewChanges.push_back( *pCurr );
                ++nRowSpanCount;
                bNew = false;
            }
        }
        if( bNew )
        {
            ColChange aTmp( nPos, nPos );
            aNewChanges.push_back( aTmp );
            ++nRowSpanCount;
        }
    }

    pCurr = aNewChanges.begin();
    ChangeList::iterator pLast = pCurr;
    ChangeList::iterator pLeftMove = pCurr;
    while( pCurr != aNewChanges.end() )
    {
        if( pLeftMove == pCurr )
        {
            while( ++pLeftMove != aNewChanges.end() && pLeftMove->first <= pLeftMove->second )
                ;
        }
        if( pCurr->second == pCurr->first )
        {
            if( pLeftMove != aNewChanges.end() && pCurr->second > pLeftMove->second )
            {
                if( pLeftMove->first == pLast->first )
                    pCurr->second = pLeftMove->second;
                else
                {
                    pCurr->second = lcl_MulDiv64<sal_uInt16>(
                        pCurr->first - pLast->first,
                        pLeftMove->second - pLast->second,
                        pLeftMove->first - pLast->first) + pLast->second;
                }
            }
            pLast = pCurr;
            ++pCurr;
        }
        else if( pCurr->second > pCurr->first )
        {
            pLast = pCurr;
            ++pCurr;
            ChangeList::iterator pNext = pCurr;
            while( pNext != pLeftMove && pNext->second == pNext->first &&
                pNext->second < pLast->second )
                ++pNext;
            while( pCurr != pNext )
            {
                if( pNext == aNewChanges.end() || pNext->first == pLast->first )
                    pCurr->second = pLast->second;
                else
                {
                    pCurr->second = lcl_MulDiv64<sal_uInt16>(
                        pCurr->first - pLast->first,
                        pNext->second - pLast->second,
                        pNext->first - pLast->first) + pLast->second;
                }
                ++pCurr;
            }
            pLast = pCurr;
        }
        else
        {
            pLast = pCurr;
            ++pCurr;
        }
    }

    rChanges.clear();
    ChangeList::iterator pCopy = aNewChanges.begin();
    while( pCopy != aNewChanges.end() )
        rChanges.push_back( *pCopy++ );
    rSpanPos.clear();
    std::list<sal_uInt16>::iterator pSpCopy = aNewSpanPos.begin();
    while( pSpCopy != aNewSpanPos.end() )
        rSpanPos.push_back( *pSpCopy++ );
}

void SwTable::NewSetTabCols( Parm &rParm, const SwTabCols &rNew,
    const SwTabCols &rOld, const SwTableBox *pStart, bool bCurRowOnly )
{
#if OSL_DEBUG_LEVEL > 1
    static int nCallCount = 0;
    ++nCallCount;
#endif
    // First step: evaluate which lines have been moved/which widths changed
    ChangeList aOldNew;
    const long nNewWidth = rParm.rNew.GetRight() - rParm.rNew.GetLeft();
    const long nOldWidth = rParm.rOld.GetRight() - rParm.rOld.GetLeft();
    if( nNewWidth < 1 || nOldWidth < 1 )
        return;
    for( size_t i = 0; i <= rOld.Count(); ++i )
    {
        long nNewPos;
        long nOldPos;
        if( i == rOld.Count() )
        {
            nOldPos = rParm.rOld.GetRight() - rParm.rOld.GetLeft();
            nNewPos = rParm.rNew.GetRight() - rParm.rNew.GetLeft();
        }
        else
        {
            nOldPos = rOld[i] - rParm.rOld.GetLeft();
            nNewPos = rNew[i] - rParm.rNew.GetLeft();
        }
        nNewPos = lcl_MulDiv64<long>(nNewPos, rParm.nNewWish, nNewWidth);
        nOldPos = lcl_MulDiv64<long>(nOldPos, rParm.nOldWish, nOldWidth);
        if( nOldPos != nNewPos && nNewPos > 0 && nOldPos > 0 )
        {
            ColChange aChg( (sal_uInt16)nOldPos, (sal_uInt16)nNewPos );
            aOldNew.push_back( aChg );
        }
    }
    // Finished first step
    int nCount = aOldNew.size();
    if( !nCount )
        return; // no change, nothing to do
    SwTableLines &rLines = GetTabLines();
    if( bCurRowOnly )
    {
        const SwTableLine* pCurrLine = pStart->GetUpper();
        sal_uInt16 nCurr = rLines.GetPos( pCurrLine );
        if( nCurr >= USHRT_MAX )
            return;

        ColChange aChg( 0, 0 );
        aOldNew.push_front( aChg );
        std::list<sal_uInt16> aRowSpanPos;
        if( nCurr )
        {
            ChangeList aCopy;
            ChangeList::iterator pCop = aOldNew.begin();
            sal_uInt16 nPos = 0;
            while( pCop != aOldNew.end() )
            {
                aCopy.push_back( *pCop );
                ++pCop;
                aRowSpanPos.push_back( nPos++ );
            }
            lcl_CalcNewWidths( aRowSpanPos, aCopy, rLines[nCurr],
                rParm.nOldWish, nOldWidth, true );
            bool bGoOn = !aRowSpanPos.empty();
            sal_uInt16 j = nCurr;
            while( bGoOn )
            {
                lcl_CalcNewWidths( aRowSpanPos, aCopy, rLines[--j],
                    rParm.nOldWish, nOldWidth, true );
                lcl_AdjustWidthsInLine( rLines[j], aCopy, rParm, 0 );
                bGoOn = !aRowSpanPos.empty() && j > 0;
            }
            aRowSpanPos.clear();
        }
        if( nCurr+1 < (sal_uInt16)rLines.size() )
        {
            ChangeList aCopy;
            ChangeList::iterator pCop = aOldNew.begin();
            sal_uInt16 nPos = 0;
            while( pCop != aOldNew.end() )
            {
                aCopy.push_back( *pCop );
                ++pCop;
                aRowSpanPos.push_back( nPos++ );
            }
            lcl_CalcNewWidths( aRowSpanPos, aCopy, rLines[nCurr],
                rParm.nOldWish, nOldWidth, false );
            bool bGoOn = !aRowSpanPos.empty();
            sal_uInt16 j = nCurr;
            while( bGoOn )
            {
                lcl_CalcNewWidths( aRowSpanPos, aCopy, rLines[++j],
                    rParm.nOldWish, nOldWidth, false );
                lcl_AdjustWidthsInLine( rLines[j], aCopy, rParm, 0 );
                bGoOn = !aRowSpanPos.empty() && j+1 < (sal_uInt16)rLines.size();
            }
        }
        ::lcl_AdjustWidthsInLine( rLines[nCurr], aOldNew, rParm, COLFUZZY );
    }
    else
    {
        for( size_t i = 0; i < rLines.size(); ++i )
            ::lcl_AdjustWidthsInLine( rLines[i], aOldNew, rParm, COLFUZZY );
    }
    CHECK_TABLE( *this )
}

// return the pointer of the box specified.
static bool lcl_IsValidRowName( const OUString& rStr )
{
    bool bIsValid = true;
    sal_Int32 nLen = rStr.getLength();
    for( sal_Int32 i = 0;  i < nLen && bIsValid; ++i )
    {
        const sal_Unicode cChar = rStr[i];
        if (cChar < '0' || cChar > '9')
            bIsValid = false;
    }
    return bIsValid;
}

// #i80314#
// add 3rd parameter and its handling
sal_uInt16 SwTable::_GetBoxNum( OUString& rStr, bool bFirstPart,
                            const bool bPerformValidCheck )
{
    sal_uInt16 nRet = 0;
    if( bFirstPart )   // true == column; false == row
    {
        sal_Int32 nPos = 0;
        // the first one uses letters for addressing!
        bool bFirst = true;
        while (nPos<rStr.getLength())
        {
            sal_Unicode cChar = rStr[nPos];
            if ((cChar<'A' || cChar>'Z') && (cChar<'a' || cChar>'z'))
                break;
            if( (cChar -= 'A') >= 26 )
                cChar -= 'a' - '[';
            if( bFirst )
                bFirst = false;
            else
                ++nRet;
            nRet = nRet * 52 + cChar;
            ++nPos;
        }
        rStr = rStr.copy( nPos );      // Remove char from String
    }
    else
    {
        const sal_Int32 nPos = rStr.indexOf( "." );
        if ( nPos<0 )
        {
            nRet = 0;
            if ( !bPerformValidCheck || lcl_IsValidRowName( rStr ) )
            {
                nRet = static_cast<sal_uInt16>(rStr.toInt32());
            }
            rStr.clear();
        }
        else
        {
            nRet = 0;
            const OUString aTxt( rStr.copy( 0, nPos ) );
            if ( !bPerformValidCheck || lcl_IsValidRowName( aTxt ) )
            {
                nRet = static_cast<sal_uInt16>(aTxt.toInt32());
            }
            rStr = rStr.copy( nPos+1 );
        }
    }
    return nRet;
}

// #i80314#
// add 2nd parameter and its handling
const SwTableBox* SwTable::GetTblBox( const OUString& rName,
                                      const bool bPerformValidCheck ) const
{
    const SwTableBox* pBox = 0;
    const SwTableLine* pLine;
    const SwTableLines* pLines;

    sal_uInt16 nLine, nBox;
    OUString aNm( rName );
    while( !aNm.isEmpty() )
    {
        nBox = SwTable::_GetBoxNum( aNm, 0 == pBox, bPerformValidCheck );
        // first box ?
        if( !pBox )
            pLines = &GetTabLines();
        else
        {
            pLines = &pBox->GetTabLines();
            if( nBox )
                --nBox;
        }

        nLine = SwTable::_GetBoxNum( aNm, false, bPerformValidCheck );

        // determine line
        if( !nLine || nLine > pLines->size() )
            return 0;
        pLine = (*pLines)[ nLine-1 ];

        // determine box
        const SwTableBoxes* pBoxes = &pLine->GetTabBoxes();
        if( nBox >= pBoxes->size() )
            return 0;
        pBox = (*pBoxes)[ nBox ];
    }

    // check if the box found has any contents
    if( pBox && !pBox->GetSttNd() )
    {
        OSL_FAIL( "Box without content, looking for the next one!" );
        // "drop this" until the first box
        while( !pBox->GetTabLines().empty() )
            pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
    }
    return pBox;
}

SwTableBox* SwTable::GetTblBox( sal_uLong nSttIdx )
{
    // For optimizations, don't always process the entire SortArray.
    // Converting text to table, tries certain conditions
    // to ask for a table box of a table that is not yet having a format
    if(!GetFrmFmt())
        return 0;
    SwTableBox* pRet = 0;
    SwNodes& rNds = GetFrmFmt()->GetDoc()->GetNodes();
    sal_uLong nIndex = nSttIdx + 1;
    SwCntntNode* pCNd = 0;
    SwTableNode* pTblNd = 0;

    while ( nIndex < rNds.Count() )
    {
        pTblNd = rNds[ nIndex ]->GetTableNode();
        if ( pTblNd )
            break;

        pCNd = rNds[ nIndex ]->GetCntntNode();
        if ( pCNd )
            break;

        ++nIndex;
    }

    if ( pCNd || pTblNd )
    {
        SwModify* pModify = pCNd;
        // #144862# Better handling of table in table
        if ( pTblNd && pTblNd->GetTable().GetFrmFmt() )
            pModify = pTblNd->GetTable().GetFrmFmt();

        SwFrm* pFrm = pModify ? SwIterator<SwFrm,SwModify>(*pModify).First() : nullptr;
        while ( pFrm && !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        if ( pFrm )
            pRet = const_cast<SwTableBox*>(static_cast<SwCellFrm*>(pFrm)->GetTabBox());
    }

    // In case the layout doesn't exist yet or anything else goes wrong.
    if ( !pRet )
    {
        for (size_t n = m_TabSortContentBoxes.size(); n; )
        {
            if (m_TabSortContentBoxes[ --n ]->GetSttIdx() == nSttIdx)
            {
                return m_TabSortContentBoxes[ n ];
            }
        }
    }
    return pRet;
}

bool SwTable::IsTblComplex() const
{
    // Returns true for complex tables, i.e. tables that contain nestings,
    // like containing boxes not part of the first line, e.g. results of
    // splits/merges which lead to more complex structures.
    for (size_t n = 0; n < m_TabSortContentBoxes.size(); ++n)
    {
        if (m_TabSortContentBoxes[ n ]->GetUpper()->GetUpper())
        {
            return true;
        }
    }
    return false;
}

SwTableLine::SwTableLine( SwTableLineFmt *pFmt, sal_uInt16 nBoxes,
                            SwTableBox *pUp )
    : SwClient( pFmt ),
    aBoxes(),
    pUpper( pUp )
{
    aBoxes.reserve( nBoxes );
}

SwTableLine::~SwTableLine()
{
    for (size_t i = 0; i < aBoxes.size(); ++i)
    {
        delete aBoxes[i];
    }
    // the TabelleLine can be deleted if it's the last client of the FrameFormat
    SwModify* pMod = GetFrmFmt();
    pMod->Remove( this );               // remove,
    if( !pMod->HasWriterListeners() )
        delete pMod;    // and delete
}

SwFrmFmt* SwTableLine::ClaimFrmFmt()
{
    // This method makes sure that this object is an exclusive SwTableLine client
    // of an SwTableLineFmt object
    // If other SwTableLine objects currently listen to the same SwTableLineFmt as
    // this one, something needs to be done
    SwTableLineFmt *pRet = static_cast<SwTableLineFmt*>(GetFrmFmt());
    SwIterator<SwTableLine,SwFmt> aIter( *pRet );
    for( SwTableLine* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        if ( pLast != this )
        {
            // found another SwTableLine that is a client of the current Fmt
            // create a new Fmt as a copy and use it for this object
            SwTableLineFmt *pNewFmt = pRet->GetDoc()->MakeTableLineFmt();
            *pNewFmt = *pRet;

            // register SwRowFrms that know me as clients at the new Fmt
            SwIterator<SwRowFrm,SwFmt> aFrmIter( *pRet );
            for( SwRowFrm* pFrm = aFrmIter.First(); pFrm; pFrm = aFrmIter.Next() )
                if( pFrm->GetTabLine() == this )
                    pFrm->RegisterToFormat( *pNewFmt );

            // register myself
            pNewFmt->Add( this );
            pRet = pNewFmt;
            break;
        }
    }

    return pRet;
}

void SwTableLine::ChgFrmFmt( SwTableLineFmt *pNewFmt )
{
    SwFrmFmt *pOld = GetFrmFmt();
    SwIterator<SwRowFrm,SwFmt> aIter( *pOld );

    // First, re-register the Frms.
    for( SwRowFrm* pRow = aIter.First(); pRow; pRow = aIter.Next() )
    {
        if( pRow->GetTabLine() == this )
        {
            pRow->RegisterToFormat( *pNewFmt );

            pRow->InvalidateSize();
            pRow->_InvalidatePrt();
            pRow->SetCompletePaint();
            pRow->ReinitializeFrmSizeAttrFlags();

            // #i35063#
            // consider 'split row allowed' attribute
            SwTabFrm* pTab = pRow->FindTabFrm();
            bool bInFollowFlowRow = false;
            const bool bInFirstNonHeadlineRow = pTab->IsFollow() &&
                                                pRow == pTab->GetFirstNonHeadlineRow();
            if ( bInFirstNonHeadlineRow ||
                 !pRow->GetNext() ||
                 ( bInFollowFlowRow = pRow->IsInFollowFlowRow() ) ||
                 0 != pRow->IsInSplitTableRow() )
            {
                if ( bInFirstNonHeadlineRow || bInFollowFlowRow )
                    pTab = pTab->FindMaster();

                pTab->SetRemoveFollowFlowLinePending( true );
                pTab->InvalidatePos();
            }
        }
    }

    // Now, re-register self.
    pNewFmt->Add( this );

    if ( !pOld->HasWriterListeners() )
        delete pOld;
}

SwTwips SwTableLine::GetTableLineHeight( bool& bLayoutAvailable ) const
{
    SwTwips nRet = 0;
    bLayoutAvailable = false;
    SwIterator<SwRowFrm,SwFmt> aIter( *GetFrmFmt() );
    // A row could appear several times in headers/footers so only one chain of master/follow tables
    // will be accepted...
    const SwTabFrm* pChain = NULL; // My chain
    for( SwRowFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        if( pLast->GetTabLine() == this )
        {
            const SwTabFrm* pTab = pLast->FindTabFrm();
            bLayoutAvailable = ( pTab && pTab->IsVertical() ) ?
                               ( 0 < pTab->Frm().Height() ) :
                               ( 0 < pTab->Frm().Width() );

            // The first one defines the chain, if a chain is defined, only members of the chain
            // will be added.
            if (pTab && (!pChain || pChain->IsAnFollow( pTab ) || pTab->IsAnFollow(pChain)))
            {
                pChain = pTab; // defines my chain (even it is already)
                if( pTab->IsVertical() )
                    nRet += pLast->Frm().Width();
                else
                    nRet += pLast->Frm().Height();
                // Optimization, if there are no master/follows in my chain, nothing more to add
                if( !pTab->HasFollow() && !pTab->IsFollow() )
                    break;
                // This is not an optimization, this is necessary to avoid double additions of
                // repeating rows
                if( pTab->IsInHeadline(*pLast) )
                    break;
            }
        }
    }
    return nRet;
}

SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, sal_uInt16 nLines, SwTableLine *pUp )
    : SwClient( 0 ),
    aLines(),
    pSttNd( 0 ),
    pUpper( pUp ),
    pImpl( 0 )
{
    aLines.reserve( nLines );
    CheckBoxFmt( pFmt )->Add( this );
}

SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, const SwNodeIndex &rIdx,
                        SwTableLine *pUp )
    : SwClient( 0 ),
    aLines(),
    pUpper( pUp ),
    pImpl( 0 )
{
    CheckBoxFmt( pFmt )->Add( this );

    pSttNd = rIdx.GetNode().GetStartNode();

    // insert into the table
    const SwTableNode* pTblNd = pSttNd->FindTableNode();
    OSL_ENSURE( pTblNd, "In which table is that box?" );
    SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
                                GetTabSortBoxes();
    SwTableBox* p = this;   // error: &this
    rSrtArr.insert( p );        // insert
}

SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, const SwStartNode& rSttNd, SwTableLine *pUp ) :
    SwClient( 0 ),
    aLines(),
    pSttNd( &rSttNd ),
    pUpper( pUp ),
    pImpl( 0 )
{
    CheckBoxFmt( pFmt )->Add( this );

    // insert into the table
    const SwTableNode* pTblNd = pSttNd->FindTableNode();
    OSL_ENSURE( pTblNd, "In which table is the box?" );
    SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
                                GetTabSortBoxes();
    SwTableBox* p = this;   // error: &this
    rSrtArr.insert( p );        // insert
}

void SwTableBox::RemoveFromTable()
{
    if (pSttNd) // box containing contents?
    {
        // remove from table
        const SwTableNode* pTblNd = pSttNd->FindTableNode();
        assert(pTblNd && "In which table is that box?");
        SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
                                    GetTabSortBoxes();
        SwTableBox *p = this;   // error: &this
        rSrtArr.erase( p );        // remove
        pSttNd = 0; // clear it so this is only run once
    }
}

SwTableBox::~SwTableBox()
{
    if (!GetFrmFmt()->GetDoc()->IsInDtor())
    {
        RemoveFromTable();
    }

    // the TabelleBox can be deleted if it's the last client of the FrameFormat
    SwModify* pMod = GetFrmFmt();
    pMod->Remove( this );               // remove,
    if( !pMod->HasWriterListeners() )
        delete pMod;    // and delete

    delete pImpl;
}

SwTableBoxFmt* SwTableBox::CheckBoxFmt( SwTableBoxFmt* pFmt )
{
    // We might need to create a new format here, because the box must be
    // added to the format solely if pFmt has a value or formular.
    if( SfxItemState::SET == pFmt->GetItemState( RES_BOXATR_VALUE, false ) ||
        SfxItemState::SET == pFmt->GetItemState( RES_BOXATR_FORMULA, false ) )
    {
        SwTableBox* pOther = SwIterator<SwTableBox,SwFmt>( *pFmt ).First();
        if( pOther )
        {
            SwTableBoxFmt* pNewFmt = pFmt->GetDoc()->MakeTableBoxFmt();
            pNewFmt->LockModify();
            *pNewFmt = *pFmt;

            // Remove values and formulars
            pNewFmt->ResetFmtAttr( RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
            pNewFmt->UnlockModify();

            pFmt = pNewFmt;
        }
    }
    return pFmt;
}

SwFrmFmt* SwTableBox::ClaimFrmFmt()
{
    // This method makes sure that this object is an exclusive SwTableBox client
    // of an SwTableBoxFmt object
    // If other SwTableBox objects currently listen to the same SwTableBoxFmt as
    // this one, something needs to be done
    SwTableBoxFmt *pRet = static_cast<SwTableBoxFmt*>(GetFrmFmt());
    SwIterator<SwTableBox,SwFmt> aIter( *pRet );
    for( SwTableBox* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        if ( pLast != this )
        {
            // Found another SwTableBox object
            // create a new Fmt as a copy and assign me to it
            // don't copy values and formulas
            SwTableBoxFmt* pNewFmt = pRet->GetDoc()->MakeTableBoxFmt();
            pNewFmt->LockModify();
            *pNewFmt = *pRet;
            pNewFmt->ResetFmtAttr( RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
            pNewFmt->UnlockModify();

            // re-register SwCellFrm objects that know me
            SwIterator<SwCellFrm,SwFmt> aFrmIter( *pRet );
            for( SwCellFrm* pCell = aFrmIter.First(); pCell; pCell = aFrmIter.Next() )
                if( pCell->GetTabBox() == this )
                    pCell->RegisterToFormat( *pNewFmt );

            // re-register myself
            pNewFmt->Add( this );
            pRet = pNewFmt;
            break;
        }
    }
    return pRet;
}

void SwTableBox::ChgFrmFmt( SwTableBoxFmt* pNewFmt )
{
    SwFrmFmt *pOld = GetFrmFmt();
    SwIterator<SwCellFrm,SwFmt> aIter( *pOld );

    // First, re-register the Frms.
    for( SwCellFrm* pCell = aIter.First(); pCell; pCell = aIter.Next() )
    {
        if( pCell->GetTabBox() == this )
        {
            pCell->RegisterToFormat( *pNewFmt );
            pCell->InvalidateSize();
            pCell->_InvalidatePrt();
            pCell->SetCompletePaint();
            pCell->SetDerivedVert( false );
            pCell->CheckDirChange();

            // #i47489#
            // make sure that the row will be formatted, in order
            // to have the correct Get(Top|Bottom)MarginForLowers values
            // set at the row.
            const SwTabFrm* pTab = pCell->FindTabFrm();
            if ( pTab && pTab->IsCollapsingBorders() )
            {
                SwFrm* pRow = pCell->GetUpper();
                pRow->_InvalidateSize();
                pRow->_InvalidatePrt();
            }
        }
    }

    // Now, re-register self.
    pNewFmt->Add( this );

    if( !pOld->HasWriterListeners() )
        delete pOld;
}

// Return the name of this box. This is determined dynamically
// resulting from the position in the lines/boxes/tables.
void sw_GetTblBoxColStr( sal_uInt16 nCol, OUString& rNm )
{
    const sal_uInt16 coDiff = 52;   // 'A'-'Z' 'a' - 'z'

    do {
        const sal_uInt16 nCalc = nCol % coDiff;
        if( nCalc >= 26 )
            rNm = OUString( sal_Unicode('a' - 26 + nCalc ) ) + rNm;
        else
            rNm = OUString( sal_Unicode('A' + nCalc ) ) + rNm;

        if( 0 == (nCol = nCol - nCalc) )
            break;
        nCol /= coDiff;
        --nCol;
    } while( true );
}

Point SwTableBox::GetCoordinates() const
{
    if( !pSttNd )       // box without content?
    {
        // search for the next first box?
        return Point( 0, 0 );
    }

    const SwTable& rTbl = pSttNd->FindTableNode()->GetTable();
    sal_uInt16 nX, nY;
    const SwTableBox* pBox = this;
    do {
        const SwTableBoxes* pBoxes = &pBox->GetUpper()->GetTabBoxes();
        const SwTableLine* pLine = pBox->GetUpper();
        // at the first level?
        const SwTableLines* pLines = pLine->GetUpper()
                ? &pLine->GetUpper()->GetTabLines() : &rTbl.GetTabLines();

        nY = pLines->GetPos( pLine ) + 1 ;
        nX = pBoxes->GetPos( pBox ) + 1 ;
        pBox = pLine->GetUpper();
    } while( pBox );
    return Point( nX, nY );
}

OUString SwTableBox::GetName() const
{
    if( !pSttNd )       // box without content?
    {
        // search for the next first box?
        return OUString();
    }

    const SwTable& rTbl = pSttNd->FindTableNode()->GetTable();
    sal_uInt16 nPos;
    OUString sNm, sTmp;
    const SwTableBox* pBox = this;
    do {
        const SwTableBoxes* pBoxes = &pBox->GetUpper()->GetTabBoxes();
        const SwTableLine* pLine = pBox->GetUpper();
        // at the first level?
        const SwTableLines* pLines = pLine->GetUpper()
                ? &pLine->GetUpper()->GetTabLines() : &rTbl.GetTabLines();

        sTmp = OUString::number( nPos = pLines->GetPos( pLine ) + 1 );
        if( !sNm.isEmpty() )
            sNm = sTmp + "." + sNm;
        else
            sNm = sTmp;

        sTmp = OUString::number(( nPos = pBoxes->GetPos( pBox )) + 1 );
        if( 0 != ( pBox = pLine->GetUpper()) )
            sNm = sTmp + "." + sNm;
        else
            sw_GetTblBoxColStr( nPos, sNm );

    } while( pBox );
    return sNm;
}

bool SwTableBox::IsInHeadline( const SwTable* pTbl ) const
{
    if( !GetUpper() )           // should only happen upon merge.
        return false;

    if( !pTbl )
        pTbl = &pSttNd->FindTableNode()->GetTable();

    const SwTableLine* pLine = GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    // Headerline?
    return pTbl->GetTabLines()[ 0 ] == pLine;
}

sal_uLong SwTableBox::GetSttIdx() const
{
    return pSttNd ? pSttNd->GetIndex() : 0;
}

    // retrieve information from the client
bool SwTable::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
        case RES_AUTOFMT_DOCNODE:
        {
            const SwTableNode* pNode = GetTableNode();
            if (pNode && &pNode->GetNodes() == static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes)
            {
                if (!m_TabSortContentBoxes.empty())
                {
                    SwNodeIndex aIdx( *m_TabSortContentBoxes[0]->GetSttNd() );
                    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode =
                                    GetFrmFmt()->GetDoc()->GetNodes().GoNext( &aIdx );
                }
                return false;
            }
            break;
        }
        case RES_FINDNEARESTNODE:
            if( GetFrmFmt() && static_cast<const SwFmtPageDesc&>(GetFrmFmt()->GetFmtAttr(
                RES_PAGEDESC )).GetPageDesc() &&
                !m_TabSortContentBoxes.empty() &&
                m_TabSortContentBoxes[0]->GetSttNd()->GetNodes().IsDocNodes() )
                static_cast<SwFindNearestNode&>(rInfo).CheckNode( *
                    m_TabSortContentBoxes[0]->GetSttNd()->FindTableNode() );
            break;

        case RES_CONTENT_VISIBLE:
            static_cast<SwPtrMsgPoolItem&>(rInfo).pObject = SwIterator<SwFrm,SwFmt>( *GetFrmFmt() ).First();
            return false;
    }
    return true;
}

SwTable * SwTable::FindTable( SwFrmFmt const*const pFmt )
{
    return (pFmt)
        ? SwIterator<SwTable,SwFmt>(*pFmt).First()
        : nullptr;
}

SwTableNode* SwTable::GetTableNode() const
{
    return !GetTabSortBoxes().empty() ?
           const_cast<SwTableNode*>(GetTabSortBoxes()[ 0 ]->GetSttNd()->FindTableNode()) :
           pTableNode;
}

void SwTable::SetRefObject( SwServerObject* pObj )
{
    if( refObj.Is() )
        refObj->Closed();

    refObj = pObj;
}

void SwTable::SetHTMLTableLayout( SwHTMLTableLayout *p )
{
    delete pHTMLLayout;
    pHTMLLayout = p;
}

void ChgTextToNum( SwTableBox& rBox, const OUString& rTxt, const Color* pCol,
                    bool bChgAlign )
{
    sal_uLong nNdPos = rBox.IsValidNumTxtNd( true );
    ChgTextToNum( rBox,rTxt,pCol,bChgAlign,nNdPos);
}
void ChgTextToNum( SwTableBox& rBox, const OUString& rTxt, const Color* pCol,
                    bool bChgAlign,sal_uLong nNdPos )
{

    if( ULONG_MAX != nNdPos )
    {
        SwDoc* pDoc = rBox.GetFrmFmt()->GetDoc();
        SwTxtNode* pTNd = pDoc->GetNodes()[ nNdPos ]->GetTxtNode();
        const SfxPoolItem* pItem;

        // assign adjustment
        if( bChgAlign )
        {
            pItem = &pTNd->SwCntntNode::GetAttr( RES_PARATR_ADJUST );
            SvxAdjust eAdjust = static_cast<const SvxAdjustItem*>(pItem)->GetAdjust();
            if( SVX_ADJUST_LEFT == eAdjust || SVX_ADJUST_BLOCK == eAdjust )
            {
                SvxAdjustItem aAdjust( *static_cast<const SvxAdjustItem*>(pItem) );
                aAdjust.SetAdjust( SVX_ADJUST_RIGHT );
                pTNd->SetAttr( aAdjust );
            }
        }

        // assign color or save "user color"
        if( !pTNd->GetpSwAttrSet() || SfxItemState::SET != pTNd->GetpSwAttrSet()->
            GetItemState( RES_CHRATR_COLOR, false, &pItem ))
            pItem = 0;

        const Color* pOldNumFmtColor = rBox.GetSaveNumFmtColor();
        const Color* pNewUserColor = pItem ? &static_cast<const SvxColorItem*>(pItem)->GetValue() : 0;

        if( ( pNewUserColor && pOldNumFmtColor &&
                *pNewUserColor == *pOldNumFmtColor ) ||
            ( !pNewUserColor && !pOldNumFmtColor ))
        {
            // Keep the user color, set updated values, delete old NumFmtColor if needed
            if( pCol )
                // if needed, set the color
                pTNd->SetAttr( SvxColorItem( *pCol, RES_CHRATR_COLOR ));
            else if( pItem )
            {
                pNewUserColor = rBox.GetSaveUserColor();
                if( pNewUserColor )
                    pTNd->SetAttr( SvxColorItem( *pNewUserColor, RES_CHRATR_COLOR ));
                else
                    pTNd->ResetAttr( RES_CHRATR_COLOR );
            }
        }
        else
        {
            // Save user color, set NumFormat color if needed, but never reset the color
            rBox.SetSaveUserColor( pNewUserColor );

            if( pCol )
                // if needed, set the color
                pTNd->SetAttr( SvxColorItem( *pCol, RES_CHRATR_COLOR ));

        }
        rBox.SetSaveNumFmtColor( pCol );

        if( pTNd->GetTxt() != rTxt )
        {
            // Exchange text. Bugfix to keep Tabs (front and back!)
            const OUString& rOrig = pTNd->GetTxt();
            sal_Int32 n;

            for( n = 0; n < rOrig.getLength() && '\x9' == rOrig[n]; ++n )
                ;
            for( ; n < rOrig.getLength() && '\x01' == rOrig[n]; ++n )
                ;
            SwIndex aIdx( pTNd, n );
            for( n = rOrig.getLength(); n && '\x9' == rOrig[--n]; )
                ;
            n -= aIdx.GetIndex() - 1;

            // Reset DontExpand-Flags before exchange, to retrigger expansion
            {
                SwIndex aResetIdx( aIdx, n );
                pTNd->DontExpandFmt( aResetIdx, false, false );
            }

            if( !pDoc->getIDocumentRedlineAccess().IsIgnoreRedline() && !pDoc->getIDocumentRedlineAccess().GetRedlineTbl().empty() )
            {
                SwPaM aTemp(*pTNd, 0, *pTNd, rOrig.getLength());
                pDoc->getIDocumentRedlineAccess().DeleteRedline(aTemp, true, USHRT_MAX);
            }

            pTNd->EraseText( aIdx, n, SwInsertFlags::EMPTYEXPAND );
            pTNd->InsertText( rTxt, aIdx, SwInsertFlags::EMPTYEXPAND );

            if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
            {
                SwPaM aTemp(*pTNd, 0, *pTNd, rTxt.getLength());
                pDoc->getIDocumentRedlineAccess().AppendRedline(new SwRangeRedline(nsRedlineType_t::REDLINE_INSERT, aTemp), true);
            }
        }

        // assign vertical orientation
        if( bChgAlign &&
            ( SfxItemState::SET != rBox.GetFrmFmt()->GetItemState(
                RES_VERT_ORIENT, true, &pItem ) ||
                text::VertOrientation::TOP == static_cast<const SwFmtVertOrient*>(pItem)->GetVertOrient() ))
        {
            rBox.GetFrmFmt()->SetFmtAttr( SwFmtVertOrient( 0, text::VertOrientation::BOTTOM ));
        }
    }
}

void ChgNumToText( SwTableBox& rBox, sal_uLong nFmt )
{
    sal_uLong nNdPos = rBox.IsValidNumTxtNd( false );
    if( ULONG_MAX != nNdPos )
    {
        SwDoc* pDoc = rBox.GetFrmFmt()->GetDoc();
        SwTxtNode* pTNd = pDoc->GetNodes()[ nNdPos ]->GetTxtNode();
        bool bChgAlign = pDoc->IsInsTblAlignNum();
        const SfxPoolItem* pItem;

        Color* pCol = 0;
        if( css::util::NumberFormat::TEXT != static_cast<sal_Int16>(nFmt) )
        {
            // special text format:
            OUString sTmp;
            const OUString sTxt( pTNd->GetTxt() );
            pDoc->GetNumberFormatter()->GetOutputString( sTxt, nFmt, sTmp, &pCol );
            if( sTxt != sTmp )
            {
                // exchange text
                SwIndex aIdx( pTNd, sTxt.getLength() );
                // Reset DontExpand-Flags before exchange, to retrigger expansion
                pTNd->DontExpandFmt( aIdx, false, false );
                aIdx = 0;
                pTNd->EraseText( aIdx, SAL_MAX_INT32, SwInsertFlags::EMPTYEXPAND );
                pTNd->InsertText( sTmp, aIdx, SwInsertFlags::EMPTYEXPAND );
            }
        }

        const SfxItemSet* pAttrSet = pTNd->GetpSwAttrSet();

        // assign adjustment
        if( bChgAlign && pAttrSet && SfxItemState::SET == pAttrSet->GetItemState(
            RES_PARATR_ADJUST, false, &pItem ) &&
                SVX_ADJUST_RIGHT == static_cast<const SvxAdjustItem*>(pItem)->GetAdjust() )
        {
            pTNd->SetAttr( SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ) );
        }

        // assign color or save "user color"
        if( !pAttrSet || SfxItemState::SET != pAttrSet->
            GetItemState( RES_CHRATR_COLOR, false, &pItem ))
            pItem = 0;

        const Color* pOldNumFmtColor = rBox.GetSaveNumFmtColor();
        const Color* pNewUserColor = pItem ? &static_cast<const SvxColorItem*>(pItem)->GetValue() : 0;

        if( ( pNewUserColor && pOldNumFmtColor &&
                *pNewUserColor == *pOldNumFmtColor ) ||
            ( !pNewUserColor && !pOldNumFmtColor ))
        {
            // Keep the user color, set updated values, delete old NumFmtColor if needed
            if( pCol )
                // if needed, set the color
                pTNd->SetAttr( SvxColorItem( *pCol, RES_CHRATR_COLOR ));
            else if( pItem )
            {
                pNewUserColor = rBox.GetSaveUserColor();
                if( pNewUserColor )
                    pTNd->SetAttr( SvxColorItem( *pNewUserColor, RES_CHRATR_COLOR ));
                else
                    pTNd->ResetAttr( RES_CHRATR_COLOR );
            }
        }
        else
        {
            // Save user color, set NumFormat color if needed, but never reset the color
            rBox.SetSaveUserColor( pNewUserColor );

            if( pCol )
                // if needed, set the color
                pTNd->SetAttr( SvxColorItem( *pCol, RES_CHRATR_COLOR ));

        }
        rBox.SetSaveNumFmtColor( pCol );

        // assign vertical orientation
        if( bChgAlign &&
            SfxItemState::SET == rBox.GetFrmFmt()->GetItemState(
            RES_VERT_ORIENT, false, &pItem ) &&
            text::VertOrientation::BOTTOM == static_cast<const SwFmtVertOrient*>(pItem)->GetVertOrient() )
        {
            rBox.GetFrmFmt()->SetFmtAttr( SwFmtVertOrient( 0, text::VertOrientation::TOP ));
        }
    }
}

// for detection of modifications (mainly TableBoxAttribute)
void SwTableBoxFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !IsModifyLocked() && !IsInDocDTOR() )
    {
        const SwTblBoxNumFormat *pNewFmt = 0;
        const SwTblBoxFormula *pNewFml = 0;
        const SwTblBoxValue *pNewVal = 0;
        sal_uLong nOldFmt = css::util::NumberFormat::TEXT;

        switch( pNew ? pNew->Which() : 0 )
        {
            case RES_ATTRSET_CHG:
            {
                const SfxItemSet& rSet = *static_cast<const SwAttrSetChg*>(pNew)->GetChgSet();
                if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMAT,
                                    false, reinterpret_cast<const SfxPoolItem**>(&pNewFmt) ) )
                    nOldFmt = static_cast<const SwTblBoxNumFormat&>(static_cast<const SwAttrSetChg*>(pOld)->
                            GetChgSet()->Get( RES_BOXATR_FORMAT )).GetValue();
                rSet.GetItemState( RES_BOXATR_FORMULA, false,
                                    reinterpret_cast<const SfxPoolItem**>(&pNewFml) );
                rSet.GetItemState( RES_BOXATR_VALUE, false,
                                    reinterpret_cast<const SfxPoolItem**>(&pNewVal) );
                break;
            }
            case RES_BOXATR_FORMAT:
                pNewFmt = static_cast<const SwTblBoxNumFormat*>(pNew);
                nOldFmt = static_cast<const SwTblBoxNumFormat*>(pOld)->GetValue();
                break;
            case RES_BOXATR_FORMULA:
                pNewFml = static_cast<const SwTblBoxFormula*>(pNew);
                break;
            case RES_BOXATR_VALUE:
                pNewVal = static_cast<const SwTblBoxValue*>(pNew);
                break;
        }

        // something changed and some BoxAttribut remained in the set!
        if( pNewFmt || pNewFml || pNewVal )
        {
            GetDoc()->getIDocumentFieldsAccess().SetFieldsDirty(true, NULL, 0);

            if( SfxItemState::SET == GetItemState( RES_BOXATR_FORMAT, false ) ||
                SfxItemState::SET == GetItemState( RES_BOXATR_VALUE, false ) ||
                SfxItemState::SET == GetItemState( RES_BOXATR_FORMULA, false ) )
            {
                // fetch the box
                SwIterator<SwTableBox,SwFmt> aIter( *this );
                SwTableBox* pBox = aIter.First();
                if( pBox )
                {
                    OSL_ENSURE( !aIter.Next(), "zeor or more than one box at format" );

                    sal_uLong nNewFmt;
                    if( pNewFmt )
                    {
                        nNewFmt = pNewFmt->GetValue();
                        // new formatting
                        // is it newer or has the current been removed?
                        if( SfxItemState::SET != GetItemState( RES_BOXATR_VALUE, false ))
                            pNewFmt = 0;
                    }
                    else
                    {
                        // fetch the current Item
                        GetItemState( RES_BOXATR_FORMAT, false,
                                            reinterpret_cast<const SfxPoolItem**>(&pNewFmt) );
                        nOldFmt = GetTblBoxNumFmt().GetValue();
                        nNewFmt = pNewFmt ? pNewFmt->GetValue() : nOldFmt;
                    }

                    // is it newer or has the current been removed?
                    if( pNewVal )
                    {
                        if( css::util::NumberFormat::TEXT != static_cast<sal_Int16>(nNewFmt) )
                        {
                            if( SfxItemState::SET == GetItemState( RES_BOXATR_VALUE, false ))
                                nOldFmt = css::util::NumberFormat::TEXT;
                            else
                                nNewFmt = css::util::NumberFormat::TEXT;
                        }
                        else if( css::util::NumberFormat::TEXT == static_cast<sal_Int16>(nNewFmt) )
                            nOldFmt = 0;
                    }

                    // Logic:
                    // Value change: -> "simulate" a format change!
                    // Format change:
                    // Text -> !Text or format change:
                    //          - align right for horizontal alignment, if LEFT or JUSTIFIED
                    //          - align bottom for vertical alignment, if TOP is set, or default
                    //          - replace text (color? negative numbers RED?)
                    // !Text -> Text:
                    //          - align left for horizontal alignment, if RIGHT
                    //          - align top for vertical alignment, if BOTTOM is set
                    SvNumberFormatter* pNumFmtr = GetDoc()->GetNumberFormatter();
                    bool bNewIsTxtFmt = pNumFmtr->IsTextFormat( nNewFmt ) ||
                                        css::util::NumberFormat::TEXT == static_cast<sal_Int16>(nNewFmt);

                    if( (!bNewIsTxtFmt && nOldFmt != nNewFmt) || pNewFml )
                    {
                        bool bChgTxt = true;
                        double fVal = 0;
                        if( !pNewVal && SfxItemState::SET != GetItemState(
                            RES_BOXATR_VALUE, false, reinterpret_cast<const SfxPoolItem**>(&pNewVal) ))
                        {
                            // so far, no value has been set, so try to evaluate the content
                            sal_uLong nNdPos = pBox->IsValidNumTxtNd( true );
                            if( ULONG_MAX != nNdPos )
                            {
                                sal_uInt32 nTmpFmtIdx = nNewFmt;
                                OUString aTxt( GetDoc()->GetNodes()[ nNdPos ]
                                                ->GetTxtNode()->GetRedlineTxt());
                                if( aTxt.isEmpty() )
                                    bChgTxt = false;
                                else
                                {
                                    // Keep Tabs
                                    lcl_TabToBlankAtSttEnd( aTxt );

                                    // JP 22.04.98: Bug 49659 -
                                    //  Special casing for percent
                                    bool bIsNumFmt = false;
                                    if( css::util::NumberFormat::PERCENT ==
                                        pNumFmtr->GetType( nNewFmt ))
                                    {
                                        sal_uInt32 nTmpFmt = 0;
                                        if( pNumFmtr->IsNumberFormat(
                                                    aTxt, nTmpFmt, fVal ))
                                        {
                                            if( css::util::NumberFormat::NUMBER ==
                                                pNumFmtr->GetType( nTmpFmt ))
                                                aTxt += "%";

                                            bIsNumFmt = pNumFmtr->IsNumberFormat(
                                                        aTxt, nTmpFmtIdx, fVal );
                                        }
                                    }
                                    else
                                        bIsNumFmt = pNumFmtr->IsNumberFormat(
                                                        aTxt, nTmpFmtIdx, fVal );

                                    if( bIsNumFmt )
                                    {
                                        // directly assign value - without Modify
                                        bool bIsLockMod = IsModifyLocked();
                                        LockModify();
                                        SetFmtAttr( SwTblBoxValue( fVal ));
                                        if( !bIsLockMod )
                                            UnlockModify();
                                    }
                                }
                            }
                        }
                        else
                            fVal = pNewVal->GetValue();

                        // format contents with the new value assigned and write to paragraph
                        Color* pCol = 0;
                        OUString sNewTxt;
                        if( DBL_MAX == fVal )
                        {
                            sNewTxt = SwViewShell::GetShellRes()->aCalc_Error;
                        }
                        else
                        {
                            pNumFmtr->GetOutputString( fVal, nNewFmt, sNewTxt, &pCol );

                            if( !bChgTxt )
                            {
                                sNewTxt.clear();
                            }
                        }

                        // across all boxes
                        ChgTextToNum( *pBox, sNewTxt, pCol,
                                        GetDoc()->IsInsTblAlignNum() );

                    }
                    else if( bNewIsTxtFmt && nOldFmt != nNewFmt )
                    {
                        ChgNumToText( *pBox, nNewFmt );
                    }
                }
            }
        }
    }
    // call base class
    SwFrmFmt::Modify( pOld, pNew );
}

bool SwTableBoxFmt::supportsFullDrawingLayerFillAttributeSet() const
{
    return false;
}

bool SwTableFmt::supportsFullDrawingLayerFillAttributeSet() const
{
    return false;
}

bool SwTableLineFmt::supportsFullDrawingLayerFillAttributeSet() const
{
    return false;
}

bool SwTableBox::HasNumCntnt( double& rNum, sal_uInt32& rFmtIndex,
                            bool& rIsEmptyTxtNd ) const
{
    bool bRet = false;
    sal_uLong nNdPos = IsValidNumTxtNd( true );
    if( ULONG_MAX != nNdPos )
    {
        OUString aTxt( pSttNd->GetNodes()[ nNdPos ]->GetTxtNode()->GetRedlineTxt() );
        // Keep Tabs
        lcl_TabToBlankAtSttEnd( aTxt );
        rIsEmptyTxtNd = aTxt.isEmpty();
        SvNumberFormatter* pNumFmtr = GetFrmFmt()->GetDoc()->GetNumberFormatter();

        const SfxPoolItem* pItem;
        if( SfxItemState::SET == GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT, false, &pItem ))
        {
            rFmtIndex = static_cast<const SwTblBoxNumFormat*>(pItem)->GetValue();
            // Special casing for percent
            if( !rIsEmptyTxtNd && css::util::NumberFormat::PERCENT == pNumFmtr->GetType( rFmtIndex ))
            {
                sal_uInt32 nTmpFmt = 0;
                if( pNumFmtr->IsNumberFormat( aTxt, nTmpFmt, rNum ) &&
                    css::util::NumberFormat::NUMBER == pNumFmtr->GetType( nTmpFmt ))
                    aTxt += "%";
            }
        }
        else
            rFmtIndex = 0;

        bRet = pNumFmtr->IsNumberFormat( aTxt, rFmtIndex, rNum );
    }
    else
        rIsEmptyTxtNd = false;
    return bRet;
}

bool SwTableBox::IsNumberChanged() const
{
    bool bRet = true;

    if( SfxItemState::SET == GetFrmFmt()->GetItemState( RES_BOXATR_FORMULA, false ))
    {
        const SwTblBoxNumFormat *pNumFmt;
        const SwTblBoxValue *pValue;

        if( SfxItemState::SET != GetFrmFmt()->GetItemState( RES_BOXATR_VALUE, false,
            reinterpret_cast<const SfxPoolItem**>(&pValue) ))
            pValue = 0;
        if( SfxItemState::SET != GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT, false,
            reinterpret_cast<const SfxPoolItem**>(&pNumFmt) ))
            pNumFmt = 0;

        sal_uLong nNdPos;
        if( pNumFmt && pValue && ULONG_MAX != ( nNdPos = IsValidNumTxtNd( true ) ) )
        {
            OUString sNewTxt, sOldTxt( pSttNd->GetNodes()[ nNdPos ]->
                                    GetTxtNode()->GetRedlineTxt() );
            lcl_DelTabsAtSttEnd( sOldTxt );

            Color* pCol = 0;
            GetFrmFmt()->GetDoc()->GetNumberFormatter()->GetOutputString(
                pValue->GetValue(), pNumFmt->GetValue(), sNewTxt, &pCol );

            bRet = sNewTxt != sOldTxt ||
                    !( ( !pCol && !GetSaveNumFmtColor() ) ||
                       ( pCol && GetSaveNumFmtColor() &&
                        *pCol == *GetSaveNumFmtColor() ));
        }
    }
    return bRet;
}

sal_uLong SwTableBox::IsValidNumTxtNd( bool bCheckAttr ) const
{
    sal_uLong nPos = ULONG_MAX;
    if( pSttNd )
    {
        SwNodeIndex aIdx( *pSttNd );
        sal_uLong nIndex = aIdx.GetIndex();
        const sal_uLong nIndexEnd = pSttNd->GetNodes()[ nIndex ]->EndOfSectionIndex();
        const SwTxtNode *pTextNode = 0;
        while( ++nIndex < nIndexEnd )
        {
            const SwNode* pNode = pSttNd->GetNodes()[nIndex];
            if( pNode->IsTableNode() )
            {
                pTextNode = 0;
                break;
            }
            if( pNode->IsTxtNode() )
            {
                if( pTextNode )
                {
                    pTextNode = 0;
                    break;
                }
                else
                {
                    pTextNode = pNode->GetTxtNode();
                    nPos = nIndex;
                }
            }
        }
        if( pTextNode )
        {
            if( bCheckAttr )
            {
                const SwpHints* pHts = pTextNode->GetpSwpHints();
                // do some tests if there's only text in the node!
                // Flys/fields/...
                if( pHts )
                {
                    sal_Int32 nNextSetField = 0;
                    for( size_t n = 0; n < pHts->Count(); ++n )
                    {
                        const SwTxtAttr* pAttr = (*pHts)[ n ];
                        if( RES_TXTATR_NOEND_BEGIN <= pAttr->Which() )
                        {
                            if ( (pAttr->GetStart() == nNextSetField)
                                 && (pAttr->Which() == RES_TXTATR_FIELD))
                            {
                                // #i104949# hideous hack for report builder:
                                // it inserts hidden variable-set fields at
                                // the beginning of para in cell, but they
                                // should not turn cell into text cell
                                const SwField* pField = pAttr->GetFmtFld().GetField();
                                if (pField &&
                                    (pField->GetTypeId() == TYP_SETFLD) &&
                                    (0 != (static_cast<SwSetExpField const*>
                                           (pField)->GetSubType() &
                                        nsSwExtendedSubType::SUB_INVISIBLE)))
                                {
                                    nNextSetField = pAttr->GetStart() + 1;
                                    continue;
                                }
                            }
                            nPos = ULONG_MAX;
                            break;
                        }
                    }
                }
            }
        }
        else
            nPos = ULONG_MAX;
    }
    return nPos;
}

// is this a Formula box or one with numeric content (AutoSum)
sal_uInt16 SwTableBox::IsFormulaOrValueBox() const
{
    sal_uInt16 nWhich = 0;
    const SwTxtNode* pTNd;
    SwFrmFmt* pFmt = GetFrmFmt();
    if( SfxItemState::SET == pFmt->GetItemState( RES_BOXATR_FORMULA, false ))
        nWhich = RES_BOXATR_FORMULA;
    else if( SfxItemState::SET == pFmt->GetItemState( RES_BOXATR_VALUE, false ) &&
            !pFmt->GetDoc()->GetNumberFormatter()->IsTextFormat(
                pFmt->GetTblBoxNumFmt().GetValue() ))
        nWhich = RES_BOXATR_VALUE;
    else if( pSttNd && pSttNd->GetIndex() + 2 == pSttNd->EndOfSectionIndex()
            && 0 != ( pTNd = pSttNd->GetNodes()[ pSttNd->GetIndex() + 1 ]
            ->GetTxtNode() ) && pTNd->GetTxt().isEmpty())
        nWhich = USHRT_MAX;

    return nWhich;
}

void SwTableBox::ActualiseValueBox()
{
    const SfxPoolItem *pFmtItem, *pValItem;
    SwFrmFmt* pFmt = GetFrmFmt();
    if( SfxItemState::SET == pFmt->GetItemState( RES_BOXATR_FORMAT, true, &pFmtItem )
        && SfxItemState::SET == pFmt->GetItemState( RES_BOXATR_VALUE, true, &pValItem ))
    {
        const sal_uLong nFmtId = static_cast<const SwTblBoxNumFormat*>(pFmtItem)->GetValue();
        sal_uLong nNdPos = ULONG_MAX;
        SvNumberFormatter* pNumFmtr = pFmt->GetDoc()->GetNumberFormatter();

        if( !pNumFmtr->IsTextFormat( nFmtId ) &&
            ULONG_MAX != (nNdPos = IsValidNumTxtNd( true )) )
        {
            double fVal = static_cast<const SwTblBoxValue*>(pValItem)->GetValue();
            Color* pCol = 0;
            OUString sNewTxt;
            pNumFmtr->GetOutputString( fVal, nFmtId, sNewTxt, &pCol );

            const OUString& rTxt = pSttNd->GetNodes()[ nNdPos ]->GetTxtNode()->GetTxt();
            if( rTxt != sNewTxt )
                ChgTextToNum( *this, sNewTxt, pCol, false ,nNdPos);
        }
    }
}

void SwTableBox_Impl::SetNewCol( Color** ppCol, const Color* pNewCol )
{
    if( *ppCol != pNewCol )
    {
        delete *ppCol;
        if( pNewCol )
            *ppCol = new Color( *pNewCol );
        else
            *ppCol = 0;
    }
}

struct SwTableCellInfo::Impl
{
    const SwTable * m_pTable;
    const SwCellFrm * m_pCellFrm;
    const SwTabFrm * m_pTabFrm;
    typedef ::std::set<const SwTableBox *> TableBoxes_t;
    TableBoxes_t m_HandledTableBoxes;

public:
    Impl()
        : m_pTable(NULL), m_pCellFrm(NULL), m_pTabFrm(NULL)
    {
    }

    ~Impl() {}

    void setTable(const SwTable * pTable)
    {
        m_pTable = pTable;
        SwFrmFmt * pFrmFmt = m_pTable->GetFrmFmt();
        m_pTabFrm = SwIterator<SwTabFrm,SwFmt>(*pFrmFmt).First();
        if (m_pTabFrm && m_pTabFrm->IsFollow())
            m_pTabFrm = m_pTabFrm->FindMaster(true);
    }

    const SwCellFrm * getCellFrm() const { return m_pCellFrm; }

    const SwFrm * getNextFrmInTable(const SwFrm * pFrm);
    const SwCellFrm * getNextCellFrm(const SwFrm * pFrm);
    const SwCellFrm * getNextTableBoxsCellFrm(const SwFrm * pFrm);
    bool getNext();
};

const SwFrm * SwTableCellInfo::Impl::getNextFrmInTable(const SwFrm * pFrm)
{
    const SwFrm * pResult = NULL;

    if (((! pFrm->IsTabFrm()) || pFrm == m_pTabFrm) && pFrm->GetLower())
        pResult = pFrm->GetLower();
    else if (pFrm->GetNext())
        pResult = pFrm->GetNext();
    else
    {
        while (pFrm->GetUpper() != NULL)
        {
            pFrm = pFrm->GetUpper();

            if (pFrm->IsTabFrm())
            {
                m_pTabFrm = static_cast<const SwTabFrm *>(pFrm)->GetFollow();
                pResult = m_pTabFrm;
                break;
            }
            else if (pFrm->GetNext())
            {
                pResult = pFrm->GetNext();
                break;
            }
        }
    }

    return pResult;
}

const SwCellFrm * SwTableCellInfo::Impl::getNextCellFrm(const SwFrm * pFrm)
{
    const SwCellFrm * pResult = NULL;

    while ((pFrm = getNextFrmInTable(pFrm)) != NULL)
    {
        if (pFrm->IsCellFrm())
        {
            pResult = static_cast<const SwCellFrm *>(pFrm);
            break;
        }
    }

    return pResult;
}

const SwCellFrm * SwTableCellInfo::Impl::getNextTableBoxsCellFrm(const SwFrm * pFrm)
{
    const SwCellFrm * pResult = NULL;

    while ((pFrm = getNextCellFrm(pFrm)) != NULL)
    {
        const SwCellFrm * pCellFrm = static_cast<const SwCellFrm *>(pFrm);
        const SwTableBox * pTabBox = pCellFrm->GetTabBox();
        TableBoxes_t::const_iterator aIt = m_HandledTableBoxes.find(pTabBox);

        if (aIt == m_HandledTableBoxes.end())
        {
            pResult = pCellFrm;
            m_HandledTableBoxes.insert(pTabBox);
            break;
        }
    }

    return pResult;
}

const SwCellFrm * SwTableCellInfo::getCellFrm() const
{
    return m_pImpl->getCellFrm();
}

bool SwTableCellInfo::Impl::getNext()
{
    if (m_pCellFrm == NULL)
    {
        if (m_pTabFrm != NULL)
            m_pCellFrm = Impl::getNextTableBoxsCellFrm(m_pTabFrm);
    }
    else
        m_pCellFrm = Impl::getNextTableBoxsCellFrm(m_pCellFrm);

    return m_pCellFrm != NULL;
}

SwTableCellInfo::SwTableCellInfo(const SwTable * pTable)
{
    m_pImpl.reset(new Impl());
    m_pImpl->setTable(pTable);
}

SwTableCellInfo::~SwTableCellInfo()
{
}

bool SwTableCellInfo::getNext()
{
    return m_pImpl->getNext();
}

SwRect SwTableCellInfo::getRect() const
{
    SwRect aRet;

    if (getCellFrm() != NULL)
        aRet = getCellFrm()->Frm();

    return aRet;
}

const SwTableBox * SwTableCellInfo::getTableBox() const
{
    const SwTableBox * pRet = NULL;

    if (getCellFrm() != NULL)
        pRet = getCellFrm()->GetTabBox();

    return pRet;
}

void SwTable::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

bool SwTable::HasLayout() const
{
    const SwFrmFmt* pFrmFmt = GetFrmFmt();
    //a table in a clipboard document doesn't have any layout information
    return pFrmFmt && SwIterator<SwTabFrm,SwFmt>(*pFrmFmt).First();
}

void SwTableLine::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

void SwTableBox::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

// free's any remaining child objects
SwTableLines::~SwTableLines()
{
    for ( const_iterator it = begin(); it != end(); ++it )
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
