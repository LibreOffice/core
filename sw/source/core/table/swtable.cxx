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


#include <ctype.h>
#include <float.h>
#include <hintids.hxx>
#include <hints.hxx>    // fuer SwAttrSetChg
#include <editeng/lrspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/adjitem.hxx>
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
#include <docary.hxx>   // fuer RedlineTbl()
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
#include <switerator.hxx>

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

SV_IMPL_REF( SwServerObject )

#define COLFUZZY 20

void ChgTextToNum( SwTableBox& rBox, const String& rTxt, const Color* pCol,
                    sal_Bool bChgAlign,sal_uLong nNdPos );
//----------------------------------

class SwTableBox_Impl
{
    Color *mpUserColor, *mpNumFmtColor;
    long mnRowSpan;
    bool mbDummyFlag;

    void SetNewCol( Color** ppCol, const Color* pNewCol );
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

// ----------- Inlines -----------------------------

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
    return pImpl ? pImpl->getDummyFlag() : false;
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
String& lcl_TabToBlankAtSttEnd( String& rTxt )
{
    sal_Unicode c;
    xub_StrLen n;

    for( n = 0; n < rTxt.Len() && ' ' >= ( c = rTxt.GetChar( n )); ++n )
        if( '\x9' == c )
            rTxt.SetChar( n, ' ' );
    for( n = rTxt.Len(); n && ' ' >= ( c = rTxt.GetChar( --n )); )
        if( '\x9' == c )
            rTxt.SetChar( n, ' ' );
    return rTxt;
}

String& lcl_DelTabsAtSttEnd( String& rTxt )
{
    sal_Unicode c;
    xub_StrLen n;

    for( n = 0; n < rTxt.Len() && ' ' >= ( c = rTxt.GetChar( n )); ++n )
        if( '\x9' == c )
            rTxt.Erase( n--, 1 );
    for( n = rTxt.Len(); n && ' ' >= ( c = rTxt.GetChar( --n )); )
        if( '\x9' == c )
            rTxt.Erase( n, 1 );
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
                                    ((SwTxtNode*)pCNd)->GetTxtColl(),
                                    &aAttrSet, nInsPos, nCnt );
        }
        else
            pDoc->GetNodes().InsBoxen( pTblNd, pLine, pBoxFrmFmt,
                                    ((SwTxtNode*)pCNd)->GetTxtColl(),
                                    pCNd->GetpSwAttrSet(),
                                    nInsPos, nCnt );
    }
    else
        pDoc->GetNodes().InsBoxen( pTblNd, pLine, pBoxFrmFmt,
                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl(), 0,
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

/*************************************************************************
|*
|*  SwTable::SwTable()
|*
|*************************************************************************/
SwTable::SwTable( SwTableFmt* pFmt )
    : SwClient( pFmt ),
    pHTMLLayout( 0 ),
    pTableNode( 0 ),
    nGrfsThatResize( 0 ),
    nRowsToRepeat( 1 ),
    bModifyLocked( sal_False ),
    bNewModel( sal_True )
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
    bModifyLocked( sal_False ),
    bNewModel( rTable.bNewModel )
{
}

void DelBoxNode( SwTableSortBoxes& rSortCntBoxes )
{
    for( sal_uInt16 n = 0; n < rSortCntBoxes.size(); ++n )
        rSortCntBoxes[ n ]->pSttNd = 0;
}

SwTable::~SwTable()
{
    if( refObj.Is() )
    {
        SwDoc* pDoc = GetFrmFmt()->GetDoc();
        if( !pDoc->IsInDtor() )         // then remove from the list
            pDoc->GetLinkManager().RemoveServer( &refObj );

        refObj->Closed();
    }

    // ist die Tabelle der letzte Client im FrameFormat, kann dieses
    // geloescht werden
    SwTableFmt* pFmt = (SwTableFmt*)GetFrmFmt();
    pFmt->Remove( this );               // austragen,

    if( !pFmt->GetDepends() )
        pFmt->GetDoc()->DelTblFrmFmt( pFmt );   // und loeschen

    // Loesche die Pointer aus dem SortArray der Boxen, die
    // Objecte bleiben erhalten und werden vom DTOR der Lines/Boxes
    // Arrays geloescht.
    //JP: reicht leider nicht, es muessen die Pointer auf den StartNode
    //  der Section geloescht werden
    DelBoxNode( aSortCntBoxes );
    aSortCntBoxes.clear();
    delete pHTMLLayout;
}

/*************************************************************************
|*
|*  SwTable::Modify()
|*
|*************************************************************************/
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
    for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
        ::lcl_ModifyBoxes( rLines[i]->GetTabBoxes(), nOld, nNew, rFmtArr );
    if( bCheckSum )
    {
        for( sal_uInt16 i = 0; i < rFmtArr.size(); ++i )
        {
            SwFmt* pFmt = rFmtArr[i];
            sal_uInt64 nBox = pFmt->GetFrmSize().GetWidth();
            nBox *= nNew;
            nBox /= nOld;
            SwFmtFrmSize aNewBox( ATT_VAR_SIZE, SwTwips(nBox), 0 );
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
    for ( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
    {
        SwTableBox &rBox = *rBoxes[i];
        if ( !rBox.GetTabLines().empty() )
        {
            // For SubTables the rounding problem will not be solved :-(
            ::lcl_ModifyLines( rBox.GetTabLines(), nOld, nNew, rFmtArr, false );
        }
        //Die Box anpassen
        SwFrmFmt *pFmt = rBox.GetFrmFmt();
        sal_uInt64 nBox = pFmt->GetFrmSize().GetWidth();
        nOriginalSum += nBox;
        nBox *= nNew;
        nBox /= nOld;
        sal_uInt64 nWishedSum = nOriginalSum;
        nWishedSum *= nNew;
        nWishedSum /= nOld;
        nWishedSum -= nSum;
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
    // fange SSize Aenderungen ab, um die Lines/Boxen anzupassen
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    const SwFmtFrmSize* pNewSize = 0, *pOldSize = 0;

    if( RES_ATTRSET_CHG == nWhich )
    {
        if( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
            RES_FRM_SIZE, sal_False, (const SfxPoolItem**)&pNewSize ))
            pOldSize = &((SwAttrSetChg*)pOld)->GetChgSet()->GetFrmSize();
    }
    else if( RES_FRM_SIZE == nWhich )
    {
        pOldSize = (const SwFmtFrmSize*)pOld;
        pNewSize = (const SwFmtFrmSize*)pNew;
    }
    else
        CheckRegistration( pOld, pNew );

    if( pOldSize || pNewSize )
    {
        if ( !IsModifyLocked() )
        {
            OSL_ENSURE( pOldSize && pOldSize->Which() == RES_FRM_SIZE &&
                    pNewSize && pNewSize->Which() == RES_FRM_SIZE,
                    "Kein Old oder New fuer FmtFrmSize-Modify der SwTable." );
            AdjustWidths( pOldSize->GetWidth(), pNewSize->GetWidth() );
        }
    }
}

void SwTable::AdjustWidths( const long nOld, const long nNew )
{
    std::vector<SwFmt*> aFmtArr;
    aFmtArr.reserve( aLines[0]->GetTabBoxes().size() );
    ::lcl_ModifyLines( aLines, nOld, nNew, aFmtArr, true );
}

/*************************************************************************
|*
|*  SwTable::GetTabCols()
|*
|*************************************************************************/
void lcl_RefreshHidden( SwTabCols &rToFill, sal_uInt16 nPos )
{
    for ( sal_uInt16 i = 0; i < rToFill.Count(); ++i )
    {
        if ( Abs((long)(nPos - rToFill[i])) <= COLFUZZY )
        {
            rToFill.SetHidden( i, sal_False );
            break;
        }
    }
}

void lcl_SortedTabColInsert( SwTabCols &rToFill, const SwTableBox *pBox,
                   const SwFrmFmt *pTabFmt, const sal_Bool bHidden,
                   const bool bRefreshHidden )
{
    const long nWish = pTabFmt->GetFrmSize().GetWidth();
    OSL_ENSURE(nWish, "weird <= 0 width frmfrm");
    const long nAct  = rToFill.GetRight() - rToFill.GetLeft();  // +1 why?

    //Der Wert fuer die linke Kante der Box errechnet sich aus den
    //Breiten der vorhergehenden Boxen.
    sal_uInt16 nPos = 0;
    sal_uInt16 nSum = 0;
    sal_uInt16 nLeftMin = 0;
    sal_uInt16 nRightMax = 0;
    const SwTableBox  *pCur  = pBox;
    const SwTableLine *pLine = pBox->GetUpper();
    while ( pLine )
    {
        const SwTableBoxes &rBoxes = pLine->GetTabBoxes();
        for ( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
        {
            SwTwips nWidth = rBoxes[i]->GetFrmFmt()->GetFrmSize().GetWidth();
            nSum = (sal_uInt16)(nSum + nWidth);
            sal_uInt64 nTmp = nSum;
            nTmp *= nAct;

            if (nWish == 0) //fdo#33012 0 width frmfmt
                continue;

            nTmp /= nWish;
            if (rBoxes[i] != pCur)
            {
                if ( pLine == pBox->GetUpper() || 0 == nLeftMin )
                    nLeftMin = (sal_uInt16)(nTmp - nPos);
                nPos = (sal_uInt16)nTmp;
            }
            else
            {
                nSum = (sal_uInt16)(nSum - nWidth);
                if ( 0 == nRightMax )
                    nRightMax = (sal_uInt16)(nTmp - nPos);
                break;
            }
        }
        pCur  = pLine->GetUpper();
        pLine = pCur ? pCur->GetUpper() : 0;
    }

    sal_Bool bInsert = !bRefreshHidden;
    for ( sal_uInt16 j = 0; bInsert && (j < rToFill.Count()); ++j )
    {
        long nCmp = rToFill[j];
        if ( (nPos >= ((nCmp >= COLFUZZY) ? nCmp - COLFUZZY : nCmp)) &&
             (nPos <= (nCmp + COLFUZZY)) )
        {
            bInsert = sal_False;        //Hat ihn schon.
        }
        else if ( nPos < nCmp )
        {
            bInsert = sal_False;
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
        for ( sal_uInt16 j = 0; !(bFoundPos && bFoundMax ) && j < rToFill.Count(); ++j )
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

void lcl_ProcessBoxGet( const SwTableBox *pBox, SwTabCols &rToFill,
                        const SwFrmFmt *pTabFmt, bool bRefreshHidden )
{
    if ( !pBox->GetTabLines().empty() )
    {
        const SwTableLines &rLines = pBox->GetTabLines();
        for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
        {   const SwTableBoxes &rBoxes = rLines[i]->GetTabBoxes();
            for ( sal_uInt16 j = 0; j < rBoxes.size(); ++j )
                ::lcl_ProcessBoxGet( rBoxes[j], rToFill, pTabFmt, bRefreshHidden);
        }
    }
    else
        ::lcl_SortedTabColInsert( rToFill, pBox, pTabFmt, sal_False, bRefreshHidden );
}

void lcl_ProcessLineGet( const SwTableLine *pLine, SwTabCols &rToFill,
                         const SwFrmFmt *pTabFmt )
{
    for ( sal_uInt16 i = 0; i < pLine->GetTabBoxes().size(); ++i )
    {
        const SwTableBox *pBox = pLine->GetTabBoxes()[i];
        if ( pBox->GetSttNd() )
            ::lcl_SortedTabColInsert( rToFill, pBox, pTabFmt, sal_True, sal_False );
        else
            for ( sal_uInt16 j = 0; j < pBox->GetTabLines().size(); ++j )
                ::lcl_ProcessLineGet( pBox->GetTabLines()[j], rToFill, pTabFmt );
    }
}

void SwTable::GetTabCols( SwTabCols &rToFill, const SwTableBox *pStart,
              sal_Bool bRefreshHidden, sal_Bool bCurRowOnly ) const
{
    //MA 30. Nov. 95: Opt: wenn bHidden gesetzt ist, wird nur das Hidden
    //Array aktualisiert.
    if ( bRefreshHidden )
    {
        // remove corrections
        sal_uInt16 i;
        for ( i = 0; i < rToFill.Count(); ++i )
        {
            SwTabColsEntry& rEntry = rToFill.GetEntry( i );
            rEntry.nPos -= rToFill.GetLeft();
            rEntry.nMin -= rToFill.GetLeft();
            rEntry.nMax -= rToFill.GetLeft();
        }

        //Alle sind hidden, dann die sichtbaren eintragen.
        for ( i = 0; i < rToFill.Count(); ++i )
            rToFill.SetHidden( i, sal_True );
    }
    else
    {
        rToFill.Remove( 0, rToFill.Count() );
    }

    //Eingetragen werden:
    //1. Alle Boxen unterhalb der dem Start uebergeordneten Line sowie
    //   deren untergeordnete Boxen falls vorhanden.
    //2. Ausgehend von der Line die uebergeordnete Box sowie deren Nachbarn;
    //   nicht aber deren untergeordnete.
    //3. Mit der der Boxenkette uebergeordneten Line wieder wie 2. bis einer
    //   Line keine Box (sondern die Table) uebergeordnet ist.
    //Es werden nur diejenigen Boxen eingetragen, die keine weiteren Zeilen
    //enhalten. Die eintragende Funktion sorgt dafuer, dass keine doppelten
    //eingetragen werden. Um dies zu gewaehrleisten wird mit einer gewissen
    //Unschaerfe gearbeitet (um Rundungsfehler auszuschalten).
    //Es werden nur die linken Kanten der Boxen eingetragen.
    //Am Schluss wird der Erste wieder ausgetragen denn er ist bereits vom
    //Rand abgedeckt.

    //4. Nochmalige abscannen der Tabelle und eintragen _aller_ Boxen,
    //   jetzt aber als Hidden.

    const SwFrmFmt *pTabFmt = GetFrmFmt();

    //1.
    const SwTableBoxes &rBoxes = pStart->GetUpper()->GetTabBoxes();

    sal_uInt16 i;
    for ( i = 0; i < rBoxes.size(); ++i )
        ::lcl_ProcessBoxGet( rBoxes[i], rToFill, pTabFmt, bRefreshHidden );

    //2. und 3.
    const SwTableLine *pLine = pStart->GetUpper()->GetUpper() ?
                                pStart->GetUpper()->GetUpper()->GetUpper() : 0;
    while ( pLine )
    {
        const SwTableBoxes &rBoxes2 = pLine->GetTabBoxes();
        for ( sal_uInt16 k = 0; k < rBoxes2.size(); ++k )
            ::lcl_SortedTabColInsert( rToFill, rBoxes2[k],
                                      pTabFmt, sal_False, bRefreshHidden );
        pLine = pLine->GetUpper() ? pLine->GetUpper()->GetUpper() : 0;
    }

    if ( !bRefreshHidden )
    {
        //4.
        if ( !bCurRowOnly )
        {
            for ( i = 0; i < aLines.size(); ++i )
                ::lcl_ProcessLineGet( aLines[i], rToFill, pTabFmt );
        }

        rToFill.Remove( 0, 1 );
    }

    //Die Koordinaten sind jetzt relativ zum linken Rand der Tabelle - also
    //relativ zum nLeft vom SwTabCols. Die Werte werden aber relativ zum
    //linken Rand - also nLeftMin vom SwTabCols - erwartet.
    //Alle Werte muessen also um nLeft erweitert werden.
    for ( i = 0; i < rToFill.Count(); ++i )
    {
        SwTabColsEntry& rEntry = rToFill.GetEntry( i );
        rEntry.nPos += rToFill.GetLeft();
        rEntry.nMin += rToFill.GetLeft();
        rEntry.nMax += rToFill.GetLeft();
    }
}

/*************************************************************************
|*
|*  SwTable::SetTabCols()
|*
|*************************************************************************/
//Struktur zur Parameteruebergabe
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

void lcl_ProcessBoxSet( SwTableBox *pBox, Parm &rParm );

void lcl_ProcessLine( SwTableLine *pLine, Parm &rParm )
{
    SwTableBoxes &rBoxes = pLine->GetTabBoxes();
    for ( int i = rBoxes.size()-1; i >= 0; --i )
        ::lcl_ProcessBoxSet( rBoxes[ static_cast< sal_uInt16 >(i) ], rParm );
}

void lcl_ProcessBoxSet( SwTableBox *pBox, Parm &rParm )
{
    if ( !pBox->GetTabLines().empty() )
    {   SwTableLines &rLines = pBox->GetTabLines();
        for ( int i = rLines.size()-1; i >= 0; --i )
            lcl_ProcessLine( rLines[ static_cast< sal_uInt16 >(i) ], rParm );
    }
    else
    {
        //Aktuelle Position (linke und rechte Kante berechnen) und im
        //alten TabCols suchen. Im neuen TabCols die Werte vergleichen und
        //wenn es Unterschiede gibt die Box entsprechend anpassen.
        //Wenn an der veraenderten Kante kein Nachbar existiert werden auch
        //alle uebergeordneten Boxen angepasst.

        const long nOldAct = rParm.rOld.GetRight() -
                             rParm.rOld.GetLeft(); // +1 why?

        //Der Wert fuer die linke Kante der Box errechnet sich aus den
        //Breiten der vorhergehenden Boxen plus dem linken Rand
        long nLeft = rParm.rOld.GetLeft();
        const  SwTableBox  *pCur  = pBox;
        const  SwTableLine *pLine = pBox->GetUpper();

        while ( pLine )
        {   const SwTableBoxes &rBoxes = pLine->GetTabBoxes();
            for ( sal_uInt16 i = 0; (i < rBoxes.size()) && (rBoxes[i] != pCur); ++i)
            {
                sal_uInt64 nWidth = rBoxes[i]->GetFrmFmt()->
                                        GetFrmSize().GetWidth();
                nWidth *= nOldAct;
                nWidth /= rParm.nOldWish;
                nLeft += (sal_uInt16)nWidth;
            }
            pCur  = pLine->GetUpper();
            pLine = pCur ? pCur->GetUpper() : 0;
        }
        long nLeftDiff;
        long nRightDiff = 0;
        if ( nLeft != rParm.rOld.GetLeft() ) //Es gibt noch Boxen davor.
        {
            //Rechte Kante ist linke Kante plus Breite.
            sal_uInt64 nWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
            nWidth *= nOldAct;
            nWidth /= rParm.nOldWish;
            long nRight = nLeft + (long)nWidth;
            sal_uInt16 nLeftPos  = USHRT_MAX,
                   nRightPos = USHRT_MAX;
            for ( sal_uInt16 i = 0; i < rParm.rOld.Count(); ++i )
            {
                if ( nLeft >= (rParm.rOld[i] - COLFUZZY) &&
                     nLeft <= (rParm.rOld[i] + COLFUZZY) )
                    nLeftPos = i;
                else if ( nRight >= (rParm.rOld[i] - COLFUZZY) &&
                          nRight <= (rParm.rOld[i] + COLFUZZY) )
                    nRightPos = i;
            }
            nLeftDiff = nLeftPos != USHRT_MAX ?
                    (int)rParm.rOld[nLeftPos] - (int)rParm.rNew[nLeftPos] : 0;
            nRightDiff= nRightPos!= USHRT_MAX ?
                    (int)rParm.rNew[nRightPos] - (int)rParm.rOld[nRightPos] : 0;
        }
        else    //Die erste Box.
        {
            nLeftDiff = (long)rParm.rOld.GetLeft() - (long)rParm.rNew.GetLeft();
            if ( rParm.rOld.Count() )
            {
                //Differnz zu der Kante berechnen, von der die erste Box
                //beruehrt wird.
                sal_uInt64 nWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
                nWidth *= nOldAct;
                nWidth /= rParm.nOldWish;
                long nTmp = (long)nWidth;
                nTmp += rParm.rOld.GetLeft();
                sal_uInt16 nLeftPos = USHRT_MAX;
                for ( sal_uInt16 i = 0; i < rParm.rOld.Count() &&
                                    nLeftPos == USHRT_MAX; ++i )
                {
                    if ( nTmp >= (rParm.rOld[i] - COLFUZZY) &&
                         nTmp <= (rParm.rOld[i] + COLFUZZY) )
                        nLeftPos = i;
                }
                if ( nLeftPos != USHRT_MAX )
                    nRightDiff = (long)rParm.rNew[nLeftPos] -
                                 (long)rParm.rOld[nLeftPos];
            }
        }

        if( pBox->getRowSpan() == 1 )
        {
            SwTableBoxes& rTblBoxes = pBox->GetUpper()->GetTabBoxes();
            sal_uInt16 nPos = rTblBoxes.GetPos( pBox );
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
            //Die Differenz ist der tatsaechliche Differenzbetrag; die
            //Attribute der Boxen um diesen Betrag anzupassen macht keinen
            //Sinn wenn die Tabelle gestrecht ist. Der Differenzbetrag muss
            //entsprechend umgerechnet werden.
            long nTmp = rParm.rNew.GetRight() - rParm.rNew.GetLeft(); // +1 why?
            nLeftDiff *= rParm.nNewWish;
            nLeftDiff /= nTmp;
            nRightDiff *= rParm.nNewWish;
            nRightDiff /= nTmp;
            long nDiff = nLeftDiff + nRightDiff;

            //Box und alle uebergeordneten um den Differenzbetrag anpassen.
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

void lcl_ProcessBoxPtr( SwTableBox *pBox, std::deque<SwTableBox*> &rBoxArr,
                           sal_Bool bBefore )
{
    if ( !pBox->GetTabLines().empty() )
    {
        const SwTableLines &rLines = pBox->GetTabLines();
        for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
        {
            const SwTableBoxes &rBoxes = rLines[i]->GetTabBoxes();
            for ( sal_uInt16 j = 0; j < rBoxes.size(); ++j )
                ::lcl_ProcessBoxPtr( rBoxes[j], rBoxArr, bBefore );
        }
    }
    else if ( bBefore )
        rBoxArr.push_front( pBox );
    else
        rBoxArr.push_back( pBox );
}

void lcl_AdjustBox( SwTableBox *pBox, const long nDiff, Parm &rParm );

void lcl_AdjustLines( SwTableLines &rLines, const long nDiff, Parm &rParm )
{
    for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
    {
        SwTableBox *pBox = rLines[i]->GetTabBoxes()
                                [rLines[i]->GetTabBoxes().size()-1];
        lcl_AdjustBox( pBox, nDiff, rParm );
    }
}

void lcl_AdjustBox( SwTableBox *pBox, const long nDiff, Parm &rParm )
{
    if ( !pBox->GetTabLines().empty() )
        ::lcl_AdjustLines( pBox->GetTabLines(), nDiff, rParm );

    //Groesse der Box anpassen.
    SwFmtFrmSize aFmtFrmSize( pBox->GetFrmFmt()->GetFrmSize() );
    aFmtFrmSize.SetWidth( aFmtFrmSize.GetWidth() + nDiff );

    rParm.aShareFmts.SetSize( *pBox, aFmtFrmSize );
}

void SwTable::SetTabCols( const SwTabCols &rNew, const SwTabCols &rOld,
                          const SwTableBox *pStart, sal_Bool bCurRowOnly )
{
    CHECK_TABLE( *this )

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    // FME: Made rOld const. The caller is responsible for passing correct
    // values of rOld. Therefore we do not have to call GetTabCols anymore:
    //GetTabCols( rOld, pStart );

    Parm aParm( rNew, rOld );

    OSL_ENSURE( rOld.Count() == rNew.Count(), "Columnanzahl veraendert.");

    //Raender verarbeiten. Groesse der Tabelle und ein paar Boxen mussen
    //angepasst werden. Bei der Groesseneinstellung darf allerdings das
    //Modify nicht verarbeitet werden - dieses wuerde alle Boxen anpassen
    //und das koennen wir ueberhaupt nicht gebrauchen.
    SwFrmFmt *pFmt = GetFrmFmt();
    aParm.nOldWish = aParm.nNewWish = pFmt->GetFrmSize().GetWidth();
    if ( (rOld.GetLeft() != rNew.GetLeft()) ||
         (rOld.GetRight()!= rNew.GetRight()) )
    {
        LockModify();
        {
            SvxLRSpaceItem aLR( pFmt->GetLRSpace() );
            SvxShadowItem aSh( pFmt->GetShadow() );

            SwTwips nShRight = aSh.CalcShadowSpace( SHADOW_RIGHT );
            SwTwips nShLeft = aSh.CalcShadowSpace( SHADOW_LEFT );

            aLR.SetLeft ( rNew.GetLeft() - nShLeft );
            aLR.SetRight( rNew.GetRightMax() - rNew.GetRight() - nShRight );
            pFmt->SetFmtAttr( aLR );

            //Die Ausrichtung der Tabelle muss entsprechend angepasst werden,
            //das geschieht so, dass die Tabelle genauso stehenbleibt wie der
            //Anwender sie gerade hingezuppelt hat.
            SwFmtHoriOrient aOri( pFmt->GetHoriOrient() );
            if(text::HoriOrientation::NONE != aOri.GetHoriOrient())
            {
                const sal_Bool bLeftDist = rNew.GetLeft() != nShLeft;
                const sal_Bool bRightDist = rNew.GetRight() + nShRight != rNew.GetRightMax();
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

        //Groesse der Tabelle anpassen. Es muss beachtet werden, das die
        //Tabelle gestrecht sein kann.
        if ( nTabDiff )
        {
            aParm.nNewWish += nTabDiff;
            if ( aParm.nNewWish < 0 )
                aParm.nNewWish = USHRT_MAX; //Uuups! Eine Rolle rueckwaerts.
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
            //Um die aktuelle Zeile anzupassen muessen wir analog zu dem
            //Verfahren zum fuellen der TabCols (siehe GetTabCols()) die
            //Boxen der aktuellen Zeile abklappern.
            //Leider muessen wir auch hier dafuer sorgen, dass die Boxen von
            //hinten nach vorne bzw. von innen nach aussen veraendert werden.
            //Der beste Weg hierzu scheint mir darin zu liegen die
            //entsprechenden Boxen in einem PtrArray vorzumerken.

            const SwTableBoxes &rBoxes = pStart->GetUpper()->GetTabBoxes();
            for ( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
                ::lcl_ProcessBoxPtr( rBoxes[i], aParm.aBoxArr, sal_False );

            const SwTableLine *pLine = pStart->GetUpper()->GetUpper() ?
                                    pStart->GetUpper()->GetUpper()->GetUpper() : 0;
            const SwTableBox  *pExcl = pStart->GetUpper()->GetUpper();
            while ( pLine )
            {
                const SwTableBoxes &rBoxes2 = pLine->GetTabBoxes();
                sal_Bool bBefore = sal_True;
                for ( sal_uInt16 i = 0; i < rBoxes2.size(); ++i )
                {
                    if ( rBoxes2[i] != pExcl )
                        ::lcl_ProcessBoxPtr( rBoxes2[i], aParm.aBoxArr, bBefore );
                    else
                        bBefore = sal_False;
                }
                pExcl = pLine->GetUpper();
                pLine = pLine->GetUpper() ? pLine->GetUpper()->GetUpper() : 0;
            }
            //Nachdem wir haufenweise Boxen (hoffentlich alle und in der richtigen
            //Reihenfolge) eingetragen haben, brauchen diese nur noch rueckwaerts
            //verarbeitet zu werden.
            for ( int j = aParm.aBoxArr.size()-1; j >= 0; --j )
            {
                SwTableBox *pBox = aParm.aBoxArr[j];
                ::lcl_ProcessBoxSet( pBox, aParm );
            }
        }
        else
        {   //Die gesamte Tabelle anzupassen ist 'einfach'.
            //Es werden alle Boxen, die keine Lines mehr enthalten angepasst.
            //Diese Boxen passen alle uebergeordneten Boxen entsprechend mit an.
            //Um uns nicht selbst hereinzulegen muss natuerlich rueckwaerst
            //gearbeitet werden!
            SwTableLines &rLines = GetTabLines();
            for ( int i = rLines.size()-1; i >= 0; --i )
                ::lcl_ProcessLine( rLines[ static_cast< sal_uInt16 >(i) ], aParm );
        }
    }

#ifdef DBG_UTIL
    {
// steht im tblrwcl.cxx
extern void _CheckBoxWidth( const SwTableLine&, SwTwips );
        // checke doch mal ob die Tabellen korrekte Breiten haben
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
    sal_uInt16 nCount = pLine->GetTabBoxes().size();
    sal_uInt16 i = 0;
    SwTwips nBorder = 0;
    SwTwips nRest = 0;
    while( i < nCount )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[i++];
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
    sal_uInt16 nOrgSum = 0;
    bool bRowSpan = false;
    sal_uInt16 nRowSpanCount = 0;
    sal_uInt16 nCount = pLine->GetTabBoxes().size();
    for( sal_uInt16 nCurrBox = 0; nCurrBox < nCount; ++nCurrBox )
    {
        SwTableBox* pBox = pLine->GetTabBoxes()[nCurrBox];
        SwTwips nCurrWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        const long nRowSpan = pBox->getRowSpan();
        const bool bCurrRowSpan = bTop ? nRowSpan < 0 :
            ( nRowSpan > 1 || nRowSpan < -1 );
        if( bRowSpan || bCurrRowSpan )
            aNewSpanPos.push_back( nRowSpanCount );
        bRowSpan = bCurrRowSpan;
        nOrgSum = (sal_uInt16)(nOrgSum + nCurrWidth);
        sal_uInt64 nSum = nOrgSum;
        nSum *= nWidth;
        nSum /= nWish;
        nSum *= nWish;
        nSum /= nWidth;
        sal_uInt16 nPos = (sal_uInt16)nSum;
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
                    sal_uInt64 nTmp = pCurr->first - pLast->first;
                    nTmp *= pLeftMove->second - pLast->second;
                    nTmp /= pLeftMove->first - pLast->first;
                    nTmp += pLast->second;
                    pCurr->second = (sal_uInt16)nTmp;
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
                    sal_uInt64 nTmp = pCurr->first - pLast->first;
                    nTmp *= pNext->second - pLast->second;
                    nTmp /= pNext->first - pLast->first;
                    nTmp += pLast->second;
                    pCurr->second = (sal_uInt16)nTmp;
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
    const SwTabCols &rOld, const SwTableBox *pStart, sal_Bool bCurRowOnly )
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
    for( sal_uInt16 i = 0; i <= rOld.Count(); ++i )
    {
        sal_uInt64 nNewPos;
        sal_uInt64 nOldPos;
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
        nNewPos *= rParm.nNewWish;
        nNewPos /= nNewWidth;
        nOldPos *= rParm.nOldWish;
        nOldPos /= nOldWidth;
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
            };
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
            };
        }
        ::lcl_AdjustWidthsInLine( rLines[nCurr], aOldNew, rParm, COLFUZZY );
    }
    else for( sal_uInt16 i = 0; i < rLines.size(); ++i )
        ::lcl_AdjustWidthsInLine( rLines[i], aOldNew, rParm, COLFUZZY );
    CHECK_TABLE( *this )
}


/*************************************************************************
|*
|*  const SwTableBox* SwTable::GetTblBox( const Strn?ng& rName ) const
|*      gebe den Pointer auf die benannte Box zurueck.
|*
|*************************************************************************/

sal_Bool IsValidRowName( const String& rStr )
{
    sal_Bool bIsValid = sal_True;
    xub_StrLen nLen = rStr.Len();
    for (xub_StrLen i = 0;  i < nLen && bIsValid;  ++i)
    {
        const sal_Unicode cChar = rStr.GetChar(i);
        if (cChar < '0' || cChar > '9')
            bIsValid = sal_False;
    }
    return bIsValid;
}

// #i80314#
// add 3rd parameter and its handling
sal_uInt16 SwTable::_GetBoxNum( String& rStr, sal_Bool bFirstPart,
                            const bool bPerformValidCheck )
{
    sal_uInt16 nRet = 0;
    xub_StrLen nPos = 0;
    if( bFirstPart )   // sal_True == column; sal_False == row
    {
        // die 1. ist mit Buchstaben addressiert!
        sal_Unicode cChar;
        sal_Bool bFirst = sal_True;
        while( 0 != ( cChar = rStr.GetChar( nPos )) &&
               ( (cChar >= 'A' && cChar <= 'Z') ||
                 (cChar >= 'a' && cChar <= 'z') ) )
        {
            if( (cChar -= 'A') >= 26 )
                cChar -= 'a' - '[';
            if( bFirst )
                bFirst = sal_False;
            else
                ++nRet;
            nRet = nRet * 52 + cChar;
            ++nPos;
        }
        rStr.Erase( 0, nPos );      // Zeichen aus dem String loeschen
    }
    else if( STRING_NOTFOUND == ( nPos = rStr.Search( aDotStr ) ))
    {
        nRet = 0;
        if ( !bPerformValidCheck || IsValidRowName( rStr ) )
        {
            nRet = static_cast<sal_uInt16>(rStr.ToInt32());
        }
        rStr.Erase();
    }
    else
    {
        nRet = 0;
        String aTxt( rStr.Copy( 0, nPos ) );
        if ( !bPerformValidCheck || IsValidRowName( aTxt ) )
        {
            nRet = static_cast<sal_uInt16>(aTxt.ToInt32());
        }
        rStr.Erase( 0, nPos+1 );
    }
    return nRet;
}

// #i80314#
// add 2nd parameter and its handling
const SwTableBox* SwTable::GetTblBox( const String& rName,
                                      const bool bPerformValidCheck ) const
{
    const SwTableBox* pBox = 0;
    const SwTableLine* pLine;
    const SwTableLines* pLines;
    const SwTableBoxes* pBoxes;

    sal_uInt16 nLine, nBox;
    String aNm( rName );
    while( aNm.Len() )
    {
        nBox = SwTable::_GetBoxNum( aNm, 0 == pBox, bPerformValidCheck );
        // erste Box ?
        if( !pBox )
            pLines = &GetTabLines();
        else
        {
            pLines = &pBox->GetTabLines();
            if( nBox )
                --nBox;
        }

        nLine = SwTable::_GetBoxNum( aNm, sal_False, bPerformValidCheck );

        // bestimme die Line
        if( !nLine || nLine > pLines->size() )
            return 0;
        pLine = (*pLines)[ nLine-1 ];

        // bestimme die Box
        pBoxes = &pLine->GetTabBoxes();
        if( nBox >= pBoxes->size() )
            return 0;
        pBox = (*pBoxes)[ nBox ];
    }

    // abpruefen, ob die gefundene Box auch wirklich eine Inhaltstragende
    // Box ist ??
    if( pBox && !pBox->GetSttNd() )
    {
        OSL_FAIL( "Box ohne Inhalt, suche die naechste !!" );
        // "herunterfallen lassen" bis zur ersten Box
        while( !pBox->GetTabLines().empty() )
            pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
    }
    return pBox;
}

SwTableBox* SwTable::GetTblBox( sal_uLong nSttIdx )
{
    //MA: Zur Optimierung nicht immer umstaendlich das ganze SortArray abhuenern.
    //OS: #102675# converting text to table tries und certain conditions
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

        SwFrm* pFrm = SwIterator<SwFrm,SwModify>::FirstElement( *pModify );
        while ( pFrm && !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        if ( pFrm )
            pRet = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
    }

    //Falls es das Layout noch nicht gibt oder sonstwie etwas schieft geht.
    if ( !pRet )
    {
        for( sal_uInt16 n = aSortCntBoxes.size(); n; )
            if( aSortCntBoxes[ --n ]->GetSttIdx() == nSttIdx )
                return aSortCntBoxes[ n ];
    }
    return pRet;
}

sal_Bool SwTable::IsTblComplex() const
{
    // returnt sal_True wenn sich in der Tabelle Verschachtelungen befinden
    // steht eine Box nicht in der obersten Line, da wurde gesplittet/
    // gemergt und die Struktur ist komplexer.
    for( sal_uInt16 n = 0; n < aSortCntBoxes.size(); ++n )
        if( aSortCntBoxes[ n ]->GetUpper()->GetUpper() )
            return sal_True;
    return sal_False;
}



/*************************************************************************
|*
|*  SwTableLine::SwTableLine()
|*
|*************************************************************************/
SwTableLine::SwTableLine( SwTableLineFmt *pFmt, sal_uInt16 nBoxes,
                            SwTableBox *pUp )
    : SwClient( pFmt ),
    aBoxes(),
    pUpper( pUp )
{
    aBoxes.reserve( (sal_uInt8)nBoxes );
}

SwTableLine::~SwTableLine()
{
    for (size_t i = 0; i < aBoxes.size(); ++i)
    {
        delete aBoxes[i];
    }
    // ist die TabelleLine der letzte Client im FrameFormat, kann dieses
    // geloescht werden
    SwModify* pMod = GetFrmFmt();
    pMod->Remove( this );               // austragen,
    if( !pMod->GetDepends() )
        delete pMod;    // und loeschen
}

/*************************************************************************
|*
|*  SwTableLine::ClaimFrmFmt(), ChgFrmFmt()
|*
|*************************************************************************/
SwFrmFmt* SwTableLine::ClaimFrmFmt()
{
    // This method makes sure that this object is an exclusive SwTableLine client
    // of an SwTableLineFmt object
    // If other SwTableLine objects currently listen to the same SwTableLineFmt as
    // this one, something needs to be done
    SwTableLineFmt *pRet = (SwTableLineFmt*)GetFrmFmt();
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

    //Erstmal die Frms ummelden.
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
                 0 != ( bInFollowFlowRow = pRow->IsInFollowFlowRow() ) ||
                 0 != pRow->IsInSplitTableRow() )
            {
                if ( bInFirstNonHeadlineRow || bInFollowFlowRow )
                    pTab = pTab->FindMaster();

                pTab->SetRemoveFollowFlowLinePending( sal_True );
                pTab->InvalidatePos();
            }
        }
    }

    //Jetzt noch mich selbst ummelden.
    pNewFmt->Add( this );

    if ( !pOld->GetDepends() )
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
            if( !pChain || pChain->IsAnFollow( pTab ) || pTab->IsAnFollow( pChain ) )
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

/*************************************************************************
|*
|*  SwTableBox::SwTableBox()
|*
|*************************************************************************/
SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, sal_uInt16 nLines, SwTableLine *pUp )
    : SwClient( 0 ),
    aLines(),
    pSttNd( 0 ),
    pUpper( pUp ),
    pImpl( 0 )
{
    aLines.reserve( (sal_uInt8)nLines );
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

    // an der Table eintragen
    const SwTableNode* pTblNd = pSttNd->FindTableNode();
    OSL_ENSURE( pTblNd, "in welcher Tabelle steht denn die Box?" );
    SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
                                GetTabSortBoxes();
    SwTableBox* p = this;   // error: &this
    rSrtArr.insert( p );        // eintragen
}

SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, const SwStartNode& rSttNd, SwTableLine *pUp ) :
    SwClient( 0 ),
    aLines(),
    pSttNd( &rSttNd ),
    pUpper( pUp ),
    pImpl( 0 )
{
    CheckBoxFmt( pFmt )->Add( this );

    // an der Table eintragen
    const SwTableNode* pTblNd = pSttNd->FindTableNode();
    OSL_ENSURE( pTblNd, "In which table is the box?" );
    SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
                                GetTabSortBoxes();
    SwTableBox* p = this;   // error: &this
    rSrtArr.insert( p );        // eintragen
}

SwTableBox::~SwTableBox()
{
    // Inhaltstragende Box ?
    if( !GetFrmFmt()->GetDoc()->IsInDtor() && pSttNd )
    {
        // an der Table austragen
        const SwTableNode* pTblNd = pSttNd->FindTableNode();
        OSL_ENSURE( pTblNd, "in welcher Tabelle steht denn die Box?" );
        SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
                                    GetTabSortBoxes();
        SwTableBox *p = this;   // error: &this
        rSrtArr.erase( p );        // austragen
    }

    // ist die TabelleBox der letzte Client im FrameFormat, kann dieses
    // geloescht werden
    SwModify* pMod = GetFrmFmt();
    pMod->Remove( this );               // austragen,
    if( !pMod->GetDepends() )
        delete pMod;    // und loeschen

    delete pImpl;
}

SwTableBoxFmt* SwTableBox::CheckBoxFmt( SwTableBoxFmt* pFmt )
{
    // sollte das Format eine Formel oder einen Value tragen, dann muss die
    // Box alleine am Format haengen. Ggfs. muss ein neues angelegt werden.
    if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_VALUE, sal_False ) ||
        SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMULA, sal_False ) )
    {
        SwTableBox* pOther = SwIterator<SwTableBox,SwFmt>::FirstElement( *pFmt );
        if( pOther )
        {
            SwTableBoxFmt* pNewFmt = pFmt->GetDoc()->MakeTableBoxFmt();
            pNewFmt->LockModify();
            *pNewFmt = *pFmt;

            // Values und Formeln entfernen
            pNewFmt->ResetFmtAttr( RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
            pNewFmt->UnlockModify();

            pFmt = pNewFmt;
        }
    }
    return pFmt;
}

/*************************************************************************
|*
|*  SwTableBox::ClaimFrmFmt(), ChgFrmFmt()
|*
|*************************************************************************/
SwFrmFmt* SwTableBox::ClaimFrmFmt()
{
    // This method makes sure that this object is an exclusive SwTableBox client
    // of an SwTableBoxFmt object
    // If other SwTableBox objects currently listen to the same SwTableBoxFmt as
    // this one, something needs to be done
    SwTableBoxFmt *pRet = (SwTableBoxFmt*)GetFrmFmt();
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

    //Erstmal die Frms ummelden.
    for( SwCellFrm* pCell = aIter.First(); pCell; pCell = aIter.Next() )
    {
        if( pCell->GetTabBox() == this )
        {
            pCell->RegisterToFormat( *pNewFmt );
            pCell->InvalidateSize();
            pCell->_InvalidatePrt();
            pCell->SetCompletePaint();
            pCell->SetDerivedVert( sal_False );
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

    //Jetzt noch mich selbst ummelden.
    pNewFmt->Add( this );

    if( !pOld->GetDepends() )
        delete pOld;
}

/*************************************************************************
|*
|*  String SwTableBox::GetName() const
|*      gebe den Namen dieser Box zurueck. Dieser wird dynamisch bestimmt
|*      und ergibt sich aus der Position in den Lines/Boxen/Tabelle
|*
|*************************************************************************/
void lcl_GetTblBoxColStr( sal_uInt16 nCol, String& rNm )
{
    const sal_uInt16 coDiff = 52;   // 'A'-'Z' 'a' - 'z'
    sal_uInt16 nCalc;

    do {
        nCalc = nCol % coDiff;
        if( nCalc >= 26 )
            rNm.Insert( sal_Unicode('a' - 26 + nCalc ), 0 );
        else
            rNm.Insert( sal_Unicode('A' + nCalc ), 0 );

        if( 0 == (nCol = nCol - nCalc) )
            break;
        nCol /= coDiff;
        --nCol;
    } while( 1 );
}

String SwTableBox::GetName() const
{
    if( !pSttNd )       // keine Content Box ??
    {
        // die naechste erste Box suchen ??
        return aEmptyStr;
    }

    const SwTable& rTbl = pSttNd->FindTableNode()->GetTable();
    sal_uInt16 nPos;
    String sNm, sTmp;
    const SwTableBox* pBox = this;
    do {
        const SwTableBoxes* pBoxes = &pBox->GetUpper()->GetTabBoxes();
        const SwTableLine* pLine = pBox->GetUpper();
        // auf oberstere Ebene ?
        const SwTableLines* pLines = pLine->GetUpper()
                ? &pLine->GetUpper()->GetTabLines() : &rTbl.GetTabLines();

        sTmp = String::CreateFromInt32( nPos = pLines->GetPos( pLine ) + 1 );
        if( sNm.Len() )
            sNm.Insert( aDotStr, 0 ).Insert( sTmp, 0 );
        else
            sNm = sTmp;

        sTmp = String::CreateFromInt32(( nPos = pBoxes->GetPos( pBox )) + 1 );
        if( 0 != ( pBox = pLine->GetUpper()) )
            sNm.Insert( aDotStr, 0 ).Insert( sTmp, 0 );
        else
            ::lcl_GetTblBoxColStr( nPos, sNm );

    } while( pBox );
    return sNm;
}

sal_Bool SwTableBox::IsInHeadline( const SwTable* pTbl ) const
{
    if( !GetUpper() )           // sollte nur beim Merge vorkommen.
        return sal_False;

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

    // erfrage vom Client Informationen
sal_Bool SwTable::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_AUTOFMT_DOCNODE:
    {
        const SwTableNode* pTblNode = GetTableNode();
        if( pTblNode && &pTblNode->GetNodes() == ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        {
            if ( !aSortCntBoxes.empty() )
            {
                  SwNodeIndex aIdx( *aSortCntBoxes[ 0 ]->GetSttNd() );
                ((SwAutoFmtGetDocNode&)rInfo).pCntntNode =
                                GetFrmFmt()->GetDoc()->GetNodes().GoNext( &aIdx );
            }
            return sal_False;
        }
        break;
    }
    case RES_FINDNEARESTNODE:
        if( GetFrmFmt() && ((SwFmtPageDesc&)GetFrmFmt()->GetFmtAttr(
            RES_PAGEDESC )).GetPageDesc() &&
            !aSortCntBoxes.empty() &&
            aSortCntBoxes[ 0 ]->GetSttNd()->GetNodes().IsDocNodes() )
            ((SwFindNearestNode&)rInfo).CheckNode( *
                aSortCntBoxes[ 0 ]->GetSttNd()->FindTableNode() );
        break;

    case RES_CONTENT_VISIBLE:
        {
            ((SwPtrMsgPoolItem&)rInfo).pObject = SwIterator<SwFrm,SwFmt>::FirstElement( *GetFrmFmt() );
        }
        return sal_False;
    }
    return sal_True;
}

SwTable * SwTable::FindTable( SwFrmFmt const*const pFmt )
{
    return (pFmt)
        ? SwIterator<SwTable,SwFmt>::FirstElement(*pFmt)
        : 0;
}

SwTableNode* SwTable::GetTableNode() const
{
    return !GetTabSortBoxes().empty() ?
           (SwTableNode*)GetTabSortBoxes()[ 0 ]->GetSttNd()->FindTableNode() :
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

void ChgTextToNum( SwTableBox& rBox, const String& rTxt, const Color* pCol,
                    sal_Bool bChgAlign )
{
    sal_uLong nNdPos = rBox.IsValidNumTxtNd( sal_True );
    ChgTextToNum( rBox,rTxt,pCol,bChgAlign,nNdPos);
}
void ChgTextToNum( SwTableBox& rBox, const String& rTxt, const Color* pCol,
                    sal_Bool bChgAlign,sal_uLong nNdPos )
{

    if( ULONG_MAX != nNdPos )
    {
        SwDoc* pDoc = rBox.GetFrmFmt()->GetDoc();
        SwTxtNode* pTNd = pDoc->GetNodes()[ nNdPos ]->GetTxtNode();
        const SfxPoolItem* pItem;

        // Ausrichtung umsetzen
        if( bChgAlign )
        {
            pItem = &pTNd->SwCntntNode::GetAttr( RES_PARATR_ADJUST );
            SvxAdjust eAdjust = ((SvxAdjustItem*)pItem)->GetAdjust();
            if( SVX_ADJUST_LEFT == eAdjust || SVX_ADJUST_BLOCK == eAdjust )
            {
                SvxAdjustItem aAdjust( *(SvxAdjustItem*)pItem );
                aAdjust.SetAdjust( SVX_ADJUST_RIGHT );
                pTNd->SetAttr( aAdjust );
            }
        }

        // Farbe umsetzen oder "Benutzer Farbe" sichern
        if( !pTNd->GetpSwAttrSet() || SFX_ITEM_SET != pTNd->GetpSwAttrSet()->
            GetItemState( RES_CHRATR_COLOR, sal_False, &pItem ))
            pItem = 0;

        const Color* pOldNumFmtColor = rBox.GetSaveNumFmtColor();
        const Color* pNewUserColor = pItem ? &((SvxColorItem*)pItem)->GetValue() : 0;

        if( ( pNewUserColor && pOldNumFmtColor &&
                *pNewUserColor == *pOldNumFmtColor ) ||
            ( !pNewUserColor && !pOldNumFmtColor ))
        {
            // User Color nicht veraendern aktuellen Werte setzen
            // ggfs. die alte NumFmtColor loeschen
            if( pCol )
                // ggfs. die Farbe setzen
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
            // User Color merken, ggfs. die NumFormat Color setzen, aber
            // nie die Farbe zurueck setzen
            rBox.SetSaveUserColor( pNewUserColor );

            if( pCol )
                // ggfs. die Farbe setzen
                pTNd->SetAttr( SvxColorItem( *pCol, RES_CHRATR_COLOR ));

        }
        rBox.SetSaveNumFmtColor( pCol );

        if( pTNd->GetTxt() != rTxt )
        {
            // Text austauschen
            //JP 15.09.98: Bug 55741 - Tabs beibehalten (vorne und hinten!)
            const String& rOrig = pTNd->GetTxt();
            xub_StrLen n;

            for( n = 0; n < rOrig.Len() && '\x9' == rOrig.GetChar( n ); ++n )
                ;
            for( ; n < rOrig.Len() && '\x01' == rOrig.GetChar( n ); ++n )
                ;
            SwIndex aIdx( pTNd, n );
            for( n = rOrig.Len(); n && '\x9' == rOrig.GetChar( --n ); )
                ;
            n -= aIdx.GetIndex() - 1;

            //JP 06.04.99: Bug 64321 - DontExpand-Flags vorm Austauschen
            //             zuruecksetzen, damit sie wieder aufgespannt werden
            {
                SwIndex aResetIdx( aIdx, n );
                pTNd->DontExpandFmt( aResetIdx, sal_False, sal_False );
            }

            if( !pDoc->IsIgnoreRedline() && !pDoc->GetRedlineTbl().empty() )
            {
                SwPaM aTemp(*pTNd, 0, *pTNd, rOrig.Len());
                pDoc->DeleteRedline(aTemp, true, USHRT_MAX);
            }

            pTNd->EraseText( aIdx, n,
                    IDocumentContentOperations::INS_EMPTYEXPAND );
            pTNd->InsertText( rTxt, aIdx,
                    IDocumentContentOperations::INS_EMPTYEXPAND );

            if( pDoc->IsRedlineOn() )
            {
                SwPaM aTemp(*pTNd, 0, *pTNd, rTxt.Len());
                pDoc->AppendRedline(new SwRedline(nsRedlineType_t::REDLINE_INSERT, aTemp), true);
            }
        }

        // vertikale Ausrichtung umsetzen
        if( bChgAlign &&
            ( SFX_ITEM_SET != rBox.GetFrmFmt()->GetItemState(
                RES_VERT_ORIENT, sal_True, &pItem ) ||
                text::VertOrientation::TOP == ((SwFmtVertOrient*)pItem)->GetVertOrient() ))
        {
            rBox.GetFrmFmt()->SetFmtAttr( SwFmtVertOrient( 0, text::VertOrientation::BOTTOM ));
        }
    }
}

void ChgNumToText( SwTableBox& rBox, sal_uLong nFmt )
{
    sal_uLong nNdPos = rBox.IsValidNumTxtNd( sal_False );
    if( ULONG_MAX != nNdPos )
    {
        SwDoc* pDoc = rBox.GetFrmFmt()->GetDoc();
        SwTxtNode* pTNd = pDoc->GetNodes()[ nNdPos ]->GetTxtNode();
        sal_Bool bChgAlign = pDoc->IsInsTblAlignNum();
        const SfxPoolItem* pItem;

        Color* pCol = 0;
        if( NUMBERFORMAT_TEXT != nFmt )
        {
            // speziellen Textformat:
            String sTmp, sTxt( pTNd->GetTxt() );
            pDoc->GetNumberFormatter()->GetOutputString( sTxt, nFmt, sTmp, &pCol );
            if( sTxt != sTmp )
            {
                // Text austauschen
                SwIndex aIdx( pTNd, sTxt.Len() );
                //JP 06.04.99: Bug 64321 - DontExpand-Flags vorm Austauschen
                //             zuruecksetzen, damit sie wieder aufgespannt werden
                pTNd->DontExpandFmt( aIdx, sal_False, sal_False );
                aIdx = 0;
                pTNd->EraseText( aIdx, STRING_LEN,
                        IDocumentContentOperations::INS_EMPTYEXPAND );
                pTNd->InsertText( sTmp, aIdx,
                        IDocumentContentOperations::INS_EMPTYEXPAND );
            }
        }

        const SfxItemSet* pAttrSet = pTNd->GetpSwAttrSet();

        // Ausrichtung umsetzen
        if( bChgAlign && pAttrSet && SFX_ITEM_SET == pAttrSet->GetItemState(
            RES_PARATR_ADJUST, sal_False, &pItem ) &&
                SVX_ADJUST_RIGHT == ((SvxAdjustItem*)pItem)->GetAdjust() )
        {
            pTNd->SetAttr( SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ) );
        }

        // Farbe umsetzen oder "Benutzer Farbe" sichern
        if( !pAttrSet || SFX_ITEM_SET != pAttrSet->
            GetItemState( RES_CHRATR_COLOR, sal_False, &pItem ))
            pItem = 0;

        const Color* pOldNumFmtColor = rBox.GetSaveNumFmtColor();
        const Color* pNewUserColor = pItem ? &((SvxColorItem*)pItem)->GetValue() : 0;

        if( ( pNewUserColor && pOldNumFmtColor &&
                *pNewUserColor == *pOldNumFmtColor ) ||
            ( !pNewUserColor && !pOldNumFmtColor ))
        {
            // User Color nicht veraendern aktuellen Werte setzen
            // ggfs. die alte NumFmtColor loeschen
            if( pCol )
                // ggfs. die Farbe setzen
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
            // User Color merken, ggfs. die NumFormat Color setzen, aber
            // nie die Farbe zurueck setzen
            rBox.SetSaveUserColor( pNewUserColor );

            if( pCol )
                // ggfs. die Farbe setzen
                pTNd->SetAttr( SvxColorItem( *pCol, RES_CHRATR_COLOR ));

        }
        rBox.SetSaveNumFmtColor( pCol );


        // vertikale Ausrichtung umsetzen
        if( bChgAlign &&
            SFX_ITEM_SET == rBox.GetFrmFmt()->GetItemState(
            RES_VERT_ORIENT, sal_False, &pItem ) &&
            text::VertOrientation::BOTTOM == ((SwFmtVertOrient*)pItem)->GetVertOrient() )
        {
            rBox.GetFrmFmt()->SetFmtAttr( SwFmtVertOrient( 0, text::VertOrientation::TOP ));
        }
    }
}

// zum Erkennen von Veraenderungen (haupts. TableBoxAttribute)
void SwTableBoxFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !IsModifyLocked() && !IsInDocDTOR() )
    {
        const SwTblBoxNumFormat *pNewFmt = 0;
        const SwTblBoxFormula *pNewFml = 0;
        const SwTblBoxValue *pNewVal = 0;
        sal_uLong nOldFmt = NUMBERFORMAT_TEXT;

        switch( pNew ? pNew->Which() : 0 )
        {
        case RES_ATTRSET_CHG:
            {
                const SfxItemSet& rSet = *((SwAttrSetChg*)pNew)->GetChgSet();
                if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMAT,
                                    sal_False, (const SfxPoolItem**)&pNewFmt ) )
                    nOldFmt = ((SwTblBoxNumFormat&)((SwAttrSetChg*)pOld)->
                            GetChgSet()->Get( RES_BOXATR_FORMAT )).GetValue();
                rSet.GetItemState( RES_BOXATR_FORMULA, sal_False,
                                    (const SfxPoolItem**)&pNewFml );
                rSet.GetItemState( RES_BOXATR_VALUE, sal_False,
                                    (const SfxPoolItem**)&pNewVal );
            }
            break;

        case RES_BOXATR_FORMAT:
            pNewFmt = (SwTblBoxNumFormat*)pNew;
            nOldFmt = ((SwTblBoxNumFormat*)pOld)->GetValue();
            break;
        case RES_BOXATR_FORMULA:
            pNewFml = (SwTblBoxFormula*)pNew;
            break;
        case RES_BOXATR_VALUE:
            pNewVal = (SwTblBoxValue*)pNew;
            break;
        }

        // es hat sich etwas getan und im Set ist noch irgendein BoxAttribut
        // vorhanden!
        if( pNewFmt || pNewFml || pNewVal )
        {
            GetDoc()->SetFieldsDirty(true, NULL, 0);

            if( SFX_ITEM_SET == GetItemState( RES_BOXATR_FORMAT, sal_False ) ||
                SFX_ITEM_SET == GetItemState( RES_BOXATR_VALUE, sal_False ) ||
                SFX_ITEM_SET == GetItemState( RES_BOXATR_FORMULA, sal_False ) )
            {
                // die Box holen
                SwIterator<SwTableBox,SwFmt> aIter( *this );
                SwTableBox* pBox = aIter.First();
                if( pBox )
                {
                    OSL_ENSURE( !aIter.Next(), "keine Box oder mehrere am Format" );

                    sal_uLong nNewFmt;
                    if( pNewFmt )
                    {
                        nNewFmt = pNewFmt->GetValue();
                        // neu Formatieren
                        // ist es neuer oder wurde der akt. entfernt?
                        if( SFX_ITEM_SET != GetItemState( RES_BOXATR_VALUE, sal_False ))
                            pNewFmt = 0;
                    }
                    else
                    {
                        // das akt. Item besorgen
                        GetItemState( RES_BOXATR_FORMAT, sal_False,
                                            (const SfxPoolItem**)&pNewFmt );
                        nOldFmt = GetTblBoxNumFmt().GetValue();
                        nNewFmt = pNewFmt ? pNewFmt->GetValue() : nOldFmt;
                    }

                    // ist es neuer oder wurde der akt. entfernt?
                    if( pNewVal )
                    {
                        if( NUMBERFORMAT_TEXT != nNewFmt )
                        {
                            if( SFX_ITEM_SET == GetItemState(
                                                RES_BOXATR_VALUE, sal_False ))
                                nOldFmt = NUMBERFORMAT_TEXT;
                            else
                                nNewFmt = NUMBERFORMAT_TEXT;
                        }
                        else if( NUMBERFORMAT_TEXT == nNewFmt )
                            nOldFmt = 0;
                    }

                    // Logik:
                    // ValueAenderung:  -> "simuliere" eine FormatAenderung!
                    // FormatAenderung:
                    // Text -> !Text oder FormatAenderung:
                    //          - Ausrichtung auf RECHTS, wenn LINKS oder Blocksatz
                    //          - vertikale Ausrichtung auf UNTEN wenn OBEN oder nicht
                    //              gesetzt ist.
                    //          - Text ersetzen (Farbe?? neg. Zahlen ROT??)
                    // !Text -> Text:
                    //          - Ausrichtung auf LINKS, wenn RECHTS
                    //          - vertikale Ausrichtung auf OEBN, wenn UNTEN gesetzt ist

                    SvNumberFormatter* pNumFmtr = GetDoc()->GetNumberFormatter();
                    sal_Bool bNewIsTxtFmt = pNumFmtr->IsTextFormat( nNewFmt ) ||
                                        NUMBERFORMAT_TEXT == nNewFmt;

                    if( (!bNewIsTxtFmt && nOldFmt != nNewFmt) || pNewFml )
                    {
                        sal_Bool bChgTxt = sal_True;
                        double fVal = 0;
                        if( !pNewVal && SFX_ITEM_SET != GetItemState(
                            RES_BOXATR_VALUE, sal_False, (const SfxPoolItem**)&pNewVal ))
                        {
                            // es wurde noch nie ein Wert gesetzt, dann versuche
                            // doch mal den Inhalt auszuwerten
                            sal_uLong nNdPos = pBox->IsValidNumTxtNd( sal_True );
                            if( ULONG_MAX != nNdPos )
                            {
                                sal_uInt32 nTmpFmtIdx = nNewFmt;
                                String aTxt( GetDoc()->GetNodes()[ nNdPos ]
                                                ->GetTxtNode()->GetRedlineTxt());
                                if( !aTxt.Len() )
                                    bChgTxt = sal_False;
                                else
                                {
                                    //JP 15.09.98: Bug 55741 - Tabs beibehalten
                                    lcl_TabToBlankAtSttEnd( aTxt );

                                    // JP 22.04.98: Bug 49659 -
                                    //          Sonderbehandlung fuer Prozent
                                    sal_Bool bIsNumFmt = sal_False;
                                    if( NUMBERFORMAT_PERCENT ==
                                        pNumFmtr->GetType( nNewFmt ))
                                    {
                                        sal_uInt32 nTmpFmt = 0;
                                        if( pNumFmtr->IsNumberFormat(
                                                    aTxt, nTmpFmt, fVal ))
                                        {
                                            if( NUMBERFORMAT_NUMBER ==
                                                pNumFmtr->GetType( nTmpFmt ))
                                                aTxt += '%';

                                            bIsNumFmt = pNumFmtr->IsNumberFormat(
                                                        aTxt, nTmpFmtIdx, fVal );
                                        }
                                    }
                                    else
                                        bIsNumFmt = pNumFmtr->IsNumberFormat(
                                                        aTxt, nTmpFmtIdx, fVal );

                                    if( bIsNumFmt )
                                    {
                                        // dann setze den Value direkt in den Set -
                                        // ohne Modify
                                        int bIsLockMod = IsModifyLocked();
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

                        // den Inhalt mit dem neuen Wert Formtieren und in den Absatz
                        // schbreiben
                        Color* pCol = 0;
                        String sNewTxt;
                        if( DBL_MAX == fVal )
                            sNewTxt = ViewShell::GetShellRes()->aCalc_Error;
                        else
                        {
                            pNumFmtr->GetOutputString( fVal, nNewFmt, sNewTxt, &pCol );

                            if( !bChgTxt )
                                sNewTxt.Erase();
                        }

                        // ueber alle Boxen
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
    // Und die Basis-Klasse rufen
    SwFrmFmt::Modify( pOld, pNew );
}

sal_Bool SwTableBox::HasNumCntnt( double& rNum, sal_uInt32& rFmtIndex,
                            sal_Bool& rIsEmptyTxtNd ) const
{
    sal_Bool bRet = sal_False;
    sal_uLong nNdPos = IsValidNumTxtNd( sal_True );
    if( ULONG_MAX != nNdPos )
    {
        String aTxt( pSttNd->GetNodes()[ nNdPos ]->GetTxtNode()->
                            GetRedlineTxt() );
        //JP 15.09.98: Bug 55741 - Tabs beibehalten
        lcl_TabToBlankAtSttEnd( aTxt );
        rIsEmptyTxtNd = 0 == aTxt.Len();
        SvNumberFormatter* pNumFmtr = GetFrmFmt()->GetDoc()->GetNumberFormatter();

        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT,
                sal_False, &pItem ))
        {
            rFmtIndex = ((SwTblBoxNumFormat*)pItem)->GetValue();
            // JP 22.04.98: Bug 49659 - Sonderbehandlung fuer Prozent
            if( !rIsEmptyTxtNd &&
                NUMBERFORMAT_PERCENT == pNumFmtr->GetType( rFmtIndex ))
            {
                sal_uInt32 nTmpFmt = 0;
                if( pNumFmtr->IsNumberFormat( aTxt, nTmpFmt, rNum ) &&
                    NUMBERFORMAT_NUMBER == pNumFmtr->GetType( nTmpFmt ))
                    aTxt += '%';
            }
        }
        else
            rFmtIndex = 0;

        bRet = pNumFmtr->IsNumberFormat( aTxt, rFmtIndex, rNum );
    }
    else
        rIsEmptyTxtNd = sal_False;
    return bRet;
}

sal_Bool SwTableBox::IsNumberChanged() const
{
    sal_Bool bRet = sal_True;

    if( SFX_ITEM_SET == GetFrmFmt()->GetItemState( RES_BOXATR_FORMULA, sal_False ))
    {
        const SwTblBoxNumFormat *pNumFmt;
        const SwTblBoxValue *pValue;

        if( SFX_ITEM_SET != GetFrmFmt()->GetItemState( RES_BOXATR_VALUE, sal_False,
            (const SfxPoolItem**)&pValue ))
            pValue = 0;
        if( SFX_ITEM_SET != GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT, sal_False,
            (const SfxPoolItem**)&pNumFmt ))
            pNumFmt = 0;

        sal_uLong nNdPos;
        if( pNumFmt && pValue &&
            ULONG_MAX != ( nNdPos = IsValidNumTxtNd( sal_True ) ) )
        {
            String sNewTxt, sOldTxt( pSttNd->GetNodes()[ nNdPos ]->
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

sal_uLong SwTableBox::IsValidNumTxtNd( sal_Bool bCheckAttr ) const
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
                const String& rTxt = pTextNode->GetTxt();
                // dann teste doch mal, ob das wirklich nur Text im Node steht!
                // Flys/Felder/..
                if( pHts )
                {
                    xub_StrLen nNextSetField = 0;
                    for( sal_uInt16 n = 0; n < pHts->Count(); ++n )
                    {
                        const SwTxtAttr* pAttr = (*pHts)[ n ];
                        if( RES_TXTATR_NOEND_BEGIN <= pAttr->Which() ||
                            *pAttr->GetStart() ||
                            *pAttr->GetAnyEnd() < rTxt.Len() )
                        {
                            if ((*pAttr->GetStart() == nNextSetField) &&
                                (pAttr->Which() == RES_TXTATR_FIELD))
                            {
                                // #i104949# hideous hack for report builder:
                                // it inserts hidden variable-set fields at
                                // the beginning of para in cell, but they
                                // should not turn cell into text cell
                                const SwField* pField = pAttr->GetFld().GetFld();
                                if (pField &&
                                    (pField->GetTypeId() == TYP_SETFLD) &&
                                    (0 != (static_cast<SwSetExpField const*>
                                           (pField)->GetSubType() &
                                        nsSwExtendedSubType::SUB_INVISIBLE)))
                                {
                                    nNextSetField = *pAttr->GetStart() + 1;
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

// ist das eine FormelBox oder eine Box mit numerischen Inhalt (AutoSum)
sal_uInt16 SwTableBox::IsFormulaOrValueBox() const
{
    sal_uInt16 nWhich = 0;
    const SwTxtNode* pTNd;
    SwFrmFmt* pFmt = GetFrmFmt();
    if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMULA, sal_False ))
        nWhich = RES_BOXATR_FORMULA;
    else if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_VALUE, sal_False ) &&
            !pFmt->GetDoc()->GetNumberFormatter()->IsTextFormat(
                pFmt->GetTblBoxNumFmt().GetValue() ))
        nWhich = RES_BOXATR_VALUE;
    else if( pSttNd && pSttNd->GetIndex() + 2 == pSttNd->EndOfSectionIndex()
            && 0 != ( pTNd = pSttNd->GetNodes()[ pSttNd->GetIndex() + 1 ]
            ->GetTxtNode() ) && !pTNd->GetTxt().Len() )
        nWhich = USHRT_MAX;

    return nWhich;
}

void SwTableBox::ActualiseValueBox()
{
    const SfxPoolItem *pFmtItem, *pValItem;
    SwFrmFmt* pFmt = GetFrmFmt();
    if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMAT, sal_True, &pFmtItem )
        && SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_VALUE, sal_True, &pValItem ))
    {
        const sal_uLong nFmtId = ((SwTblBoxNumFormat*)pFmtItem)->GetValue();
        sal_uLong nNdPos = ULONG_MAX;
        SvNumberFormatter* pNumFmtr = pFmt->GetDoc()->GetNumberFormatter();

        if( !pNumFmtr->IsTextFormat( nFmtId ) &&
            ULONG_MAX != (nNdPos = IsValidNumTxtNd( sal_True )) )
        {
            double fVal = ((SwTblBoxValue*)pValItem)->GetValue();
            Color* pCol = 0;
            String sNewTxt;
            pNumFmtr->GetOutputString( fVal, nFmtId, sNewTxt, &pCol );

            const String& rTxt = pSttNd->GetNodes()[ nNdPos ]->GetTxtNode()->GetTxt();
            if( rTxt != sNewTxt )
                ChgTextToNum( *this, sNewTxt, pCol, sal_False ,nNdPos);
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

    void setTable(const SwTable * pTable) {
        m_pTable = pTable;
        SwFrmFmt * pFrmFmt = m_pTable->GetFrmFmt();
        m_pTabFrm = SwIterator<SwTabFrm,SwFmt>::FirstElement(*pFrmFmt);
        if (m_pTabFrm->IsFollow())
            m_pTabFrm = m_pTabFrm->FindMaster(true);
    }
    const SwTable * getTable() const { return m_pTable; }

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
    return pFrmFmt && SwIterator<SwTabFrm,SwFmt>::FirstElement(*pFrmFmt);
}

void SwTableLine::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

void SwTableBox::RegisterToFormat( SwFmt& rFmt )
{
    rFmt.Add( this );
}

void SwTableBox::ForgetFrmFmt()
{
    if ( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove(this);
}

// free's any remaining child objects
SwTableLines::~SwTableLines()
{
    for ( const_iterator it = begin(); it != end(); ++it )
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
