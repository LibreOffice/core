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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include "cntfrm.hxx"
#include "doc.hxx"

#include "hintids.hxx"
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <fmtclds.hxx>
#include <fmtfordr.hxx>
#include <frmfmt.hxx>
#include <node.hxx>
#include "frmtool.hxx"
#include "colfrm.hxx"
#include "pagefrm.hxx"
#include "bodyfrm.hxx"   // ColumnFrms jetzt mit BodyFrm
#include "rootfrm.hxx"   // wg. RemoveFtns
#include "sectfrm.hxx"   // wg. FtnAtEnd-Flag

// ftnfrm.cxx:
void lcl_RemoveFtns( SwFtnBossFrm* pBoss, BOOL bPageOnly, BOOL bEndNotes );


/*************************************************************************
|*
|*  SwColumnFrm::SwColumnFrm()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    AMA 30. Oct 98
|*
|*************************************************************************/
SwColumnFrm::SwColumnFrm( SwFrmFmt *pFmt ):
    SwFtnBossFrm( pFmt )
{
    nType = FRMC_COLUMN;
    SwBodyFrm* pColBody = new SwBodyFrm( pFmt->GetDoc()->GetDfltFrmFmt() );
    pColBody->InsertBehind( this, 0 ); // ColumnFrms jetzt mit BodyFrm
    SetMaxFtnHeight( LONG_MAX );
}

SwColumnFrm::~SwColumnFrm()
{
    SwFrmFmt *pFmt = GetFmt();
    SwDoc *pDoc;
    if ( !(pDoc = pFmt->GetDoc())->IsInDtor() && pFmt->IsLastDepend() )
    {
        //Ich bin der einzige, weg mit dem Format.
        //Vorher ummelden, damit die Basisklasse noch klarkommt.
        pDoc->GetDfltFrmFmt()->Add( this );
        pDoc->DelFrmFmt( pFmt );
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::ChgColumns()
|*
|*  Ersterstellung      MA 11. Feb. 93
|*  Letzte Aenderung    MA 12. Oct. 98
|*
|*************************************************************************/

void MA_FASTCALL lcl_RemoveColumns( SwLayoutFrm *pCont, USHORT nCnt )
{
    ASSERT( pCont && pCont->Lower() && pCont->Lower()->IsColumnFrm(),
            "Keine Spalten zu entfernen." );

    SwColumnFrm *pColumn = (SwColumnFrm*)pCont->Lower();
    ::lcl_RemoveFtns( pColumn, TRUE, TRUE );
    while ( pColumn->GetNext() )
    {
        ASSERT( pColumn->GetNext()->IsColumnFrm(),
                "Nachbar von ColFrm kein ColFrm." );
        pColumn = (SwColumnFrm*)pColumn->GetNext();
    }
    for ( USHORT i = 0; i < nCnt; ++i )
    {
        SwColumnFrm *pTmp = (SwColumnFrm*)pColumn->GetPrev();
        pColumn->Cut();
        delete pColumn; //Format wird ggf. im DTor mit vernichtet.
        pColumn = pTmp;
    }
}

SwLayoutFrm * MA_FASTCALL lcl_FindColumns( SwLayoutFrm *pLay, USHORT nCount )
{
    SwFrm *pCol = pLay->Lower();
    if ( pLay->IsPageFrm() )
        pCol = ((SwPageFrm*)pLay)->FindBodyCont()->Lower();

    if ( pCol && pCol->IsColumnFrm() )
    {
        SwFrm *pTmp = pCol;
        USHORT i;
        for ( i = 0; pTmp; pTmp = pTmp->GetNext(), ++i )
            /* do nothing */;
        return i == nCount ? (SwLayoutFrm*)pCol : 0;
    }
    return 0;
}


static BOOL lcl_AddColumns( SwLayoutFrm *pCont, USHORT nCount )
{
    SwDoc *pDoc = pCont->GetFmt()->GetDoc();
    const BOOL bMod = pDoc->IsModified();

    //Format sollen soweit moeglich geshared werden. Wenn es also schon einen
    //Nachbarn mit den selben Spalteneinstellungen gibt, so koennen die
    //Spalten an die selben Formate gehaengt werden.
    //Der Nachbar kann ueber das Format gesucht werden, wer der Owner des Attributes
    //ist, ist allerdings vom Frametyp abhaengig.
    SwLayoutFrm *pAttrOwner = pCont;
    if ( pCont->IsBodyFrm() )
        pAttrOwner = pCont->FindPageFrm();
    SwLayoutFrm *pNeighbourCol = 0;
    SwClientIter aIter( *pAttrOwner->GetFmt() );
    SwLayoutFrm *pNeighbour = (SwLayoutFrm*)aIter.First( TYPE(SwLayoutFrm) );

    USHORT nAdd = 0;
    SwFrm *pCol = pCont->Lower();
    if ( pCol && pCol->IsColumnFrm() )
        for ( nAdd = 1; pCol; pCol = pCol->GetNext(), ++nAdd )
            /* do nothing */;
    while ( pNeighbour )
    {
        if ( 0 != (pNeighbourCol = lcl_FindColumns( pNeighbour, nCount+nAdd )) &&
             pNeighbourCol != pCont )
            break;
        pNeighbourCol = 0;
        pNeighbour = (SwLayoutFrm*)aIter.Next();
    }

    BOOL bRet;
    SwTwips nMax = pCont->IsPageBodyFrm() ?
                   pCont->FindPageFrm()->GetMaxFtnHeight() : LONG_MAX;
    if ( pNeighbourCol )
    {
        bRet = FALSE;
        SwFrm *pTmp = pCont->Lower();
        while ( pTmp )
        {
            pTmp = pTmp->GetNext();
            pNeighbourCol = (SwLayoutFrm*)pNeighbourCol->GetNext();
        }
        for ( USHORT i = 0; i < nCount; ++i )
        {
            SwColumnFrm *pTmpCol = new SwColumnFrm( pNeighbourCol->GetFmt() );
            pTmpCol->SetMaxFtnHeight( nMax );
            pTmpCol->InsertBefore( pCont, NULL );
            pNeighbourCol = (SwLayoutFrm*)pNeighbourCol->GetNext();
        }
    }
    else
    {
        bRet = TRUE;
        for ( USHORT i = 0; i < nCount; ++i )
        {
            SwFrmFmt *pFmt = pDoc->MakeFrmFmt( aEmptyStr, pDoc->GetDfltFrmFmt());
            SwColumnFrm *pTmp = new SwColumnFrm( pFmt );
            pTmp->SetMaxFtnHeight( nMax );
            pTmp->Paste( pCont );
        }
    }

    if ( !bMod )
        pDoc->ResetModified();
    return bRet;
}

/*-----------------21.09.99 15:42-------------------
 * ChgColumns() adds or removes columns from a layoutframe.
 * Normally, a layoutframe with a column attribut of 1 or 0 columns contains
 * no columnframe. However, a sectionframe with "footnotes at the end" needs
 * a columnframe. If the bChgFtn-flag is set, the columnframe will be inserted
 * or remove, if necessary.
 * --------------------------------------------------*/

void SwLayoutFrm::ChgColumns( const SwFmtCol &rOld, const SwFmtCol &rNew,
    const BOOL bChgFtn )
{
    if ( rOld.GetNumCols() <= 1 && rNew.GetNumCols() <= 1 && !bChgFtn )
        return;
    // --> OD 2009-08-12 #i97379#
    // If current lower is a no text frame, then columns are not allowed
    if ( Lower() && Lower()->IsNoTxtFrm() &&
         rNew.GetNumCols() > 1 )
    {
        return;
    }
    // <--

    USHORT nNewNum, nOldNum = 1;
    if( Lower() && Lower()->IsColumnFrm() )
    {
        SwFrm* pCol = Lower();
        while( 0 != (pCol=pCol->GetNext()) )
            ++nOldNum;
    }
    nNewNum = rNew.GetNumCols();
    if( !nNewNum )
        ++nNewNum;
    BOOL bAtEnd;
    if( IsSctFrm() )
        bAtEnd = ((SwSectionFrm*)this)->IsAnyNoteAtEnd();
    else
        bAtEnd = FALSE;

    //Einstellung der Spaltenbreiten ist nur bei neuen Formaten notwendig.
    BOOL bAdjustAttributes = nOldNum != rOld.GetNumCols();

    //Wenn die Spaltenanzahl unterschiedlich ist, wird der Inhalt
    //gesichert und restored.
    SwFrm *pSave = 0;
    if( nOldNum != nNewNum || bChgFtn )
    {
        SwDoc *pDoc = GetFmt()->GetDoc();
        ASSERT( pDoc, "FrmFmt gibt kein Dokument her." );
        // SaveCntnt wuerde auch den Inhalt der Fussnotencontainer aufsaugen
        // und im normalen Textfluss unterbringen.
        if( IsPageBodyFrm() )
            pDoc->GetRootFrm()->RemoveFtns( (SwPageFrm*)GetUpper(), TRUE, FALSE );
        pSave = ::SaveCntnt( this );

        //Wenn Spalten existieren, jetzt aber eine Spaltenanzahl von
        //0 oder eins gewuenscht ist, so werden die Spalten einfach vernichtet.
        if ( nNewNum == 1 && !bAtEnd )
        {
            ::lcl_RemoveColumns( this, nOldNum );
            if ( IsBodyFrm() )
                SetFrmFmt( pDoc->GetDfltFrmFmt() );
            else
                GetFmt()->SetFmtAttr( SwFmtFillOrder() );
            if ( pSave )
                ::RestoreCntnt( pSave, this, 0, true );
            return;
        }
        if ( nOldNum == 1 )
        {
            if ( IsBodyFrm() )
                SetFrmFmt( pDoc->GetColumnContFmt() );
            else
                GetFmt()->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );
            if( !Lower() || !Lower()->IsColumnFrm() )
                --nOldNum;
        }
        if ( nOldNum > nNewNum )
        {
            ::lcl_RemoveColumns( this, nOldNum - nNewNum );
            bAdjustAttributes = TRUE;
        }
        else if( nOldNum < nNewNum )
        {
            USHORT nAdd = nNewNum - nOldNum;
            bAdjustAttributes = lcl_AddColumns( this, nAdd );
        }
    }

    if ( !bAdjustAttributes )
    {
        if ( rOld.GetLineWidth()    != rNew.GetLineWidth() ||
             rOld.GetWishWidth()    != rNew.GetWishWidth() ||
             rOld.IsOrtho()         != rNew.IsOrtho() )
            bAdjustAttributes = TRUE;
        else
        {
            USHORT nCount = Min( rNew.GetColumns().Count(), rOld.GetColumns().Count() );
            for ( USHORT i = 0; i < nCount; ++i )
                if ( !(*rOld.GetColumns()[i] == *rNew.GetColumns()[i]) )
                {
                    bAdjustAttributes = TRUE;
                    break;
                }
        }
    }

    //Sodele, jetzt koennen die Spalten bequem eingestellt werden.
    AdjustColumns( &rNew, bAdjustAttributes );

    //Erst jetzt den Inhalt restaurieren. Ein frueheres Restaurieren wuerde
    //unnuetzte Aktionen beim Einstellen zur Folge haben.
    if ( pSave )
    {
        ASSERT( Lower() && Lower()->IsLayoutFrm() &&
                ((SwLayoutFrm*)Lower())->Lower() &&
                ((SwLayoutFrm*)Lower())->Lower()->IsLayoutFrm(),
                "Gesucht: Spaltenbody (Tod oder Lebend)." );   // ColumnFrms jetzt mit BodyFrm
        ::RestoreCntnt( pSave, (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower(), 0, true );
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::AdjustColumns()
|*
|*  Ersterstellung      MA 19. Jan. 99
|*  Letzte Aenderung    MA 19. Jan. 99
|*
|*************************************************************************/

void SwLayoutFrm::AdjustColumns( const SwFmtCol *pAttr, BOOL bAdjustAttributes )
{
    if( !Lower()->GetNext() )
    {
        Lower()->ChgSize( Prt().SSize() );
        return;
    }

    const BOOL bVert = IsVertical();
    SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;

    //Ist ein Pointer da, oder sollen wir die Attribute einstellen,
    //so stellen wir auf jeden Fall die Spaltenbreiten ein. Andernfalls
    //checken wir, ob eine Einstellung notwendig ist.
    if ( !pAttr )
    {
        pAttr = &GetFmt()->GetCol();
        if ( !bAdjustAttributes )
        {
            long nAvail = (Prt().*fnRect->fnGetWidth)();
            for ( SwLayoutFrm *pCol = (SwLayoutFrm*)Lower();
                  pCol;
                  pCol = (SwLayoutFrm*)pCol->GetNext() )
                nAvail -= (pCol->Frm().*fnRect->fnGetWidth)();
            if ( !nAvail )
                return;
        }
    }

    //Sodele, jetzt koennen die Spalten bequem eingestellt werden.
    //Die Breiten werden mitgezaehlt, damit wir dem letzten den Rest geben
    //koennen.
    SwTwips nAvail = (Prt().*fnRect->fnGetWidth)();
    const BOOL bLine = pAttr->GetLineAdj() != COLADJ_NONE;
    const USHORT nMin = bLine ? USHORT( 20 + ( pAttr->GetLineWidth() / 2) ) : 0;

    const BOOL bR2L = IsRightToLeft();
    SwFrm *pCol = bR2L ? GetLastLower() : Lower();

    // --> FME 2004-07-16 #i27399#
    // bOrtho means we have to adjust the column frames manually. Otherwise
    // we may use the values returned by CalcColWidth:
    const BOOL bOrtho = pAttr->IsOrtho() && pAttr->GetNumCols() > 0;
    long nGutter = 0;
    // <--

    for ( USHORT i = 0; i < pAttr->GetNumCols(); ++i )
    {
        if( !bOrtho )
        {
            const SwTwips nWidth = i == (pAttr->GetNumCols() - 1) ?
                                   nAvail :
                                   pAttr->CalcColWidth( i, USHORT( (Prt().*fnRect->fnGetWidth)() ) );

            const Size aColSz = bVert ?
                                Size( Prt().Width(), nWidth ) :
                                Size( nWidth, Prt().Height() );

            pCol->ChgSize( aColSz );

            // Hierdurch werden die ColumnBodyFrms von Seitenspalten angepasst und
            // ihr bFixHeight-Flag wird gesetzt, damit sie nicht schrumpfen/wachsen.
            // Bei Rahmenspalten hingegen soll das Flag _nicht_ gesetzt werden,
            // da BodyFrms in Rahmenspalten durchaus wachsen/schrumpfen duerfen.
            if( IsBodyFrm() )
                ((SwLayoutFrm*)pCol)->Lower()->ChgSize( aColSz );

            nAvail -= nWidth;
        }

        if ( bOrtho || bAdjustAttributes )
        {
            const SwColumn *pC = pAttr->GetColumns()[i];
            const SwAttrSet* pSet = pCol->GetAttrSet();
            SvxLRSpaceItem aLR( pSet->GetLRSpace() );

            //Damit die Trennlinien Platz finden, muessen sie hier
            //Beruecksichtigung finden. Ueberall wo zwei Spalten aufeinanderstossen
            //wird jeweils rechts bzw. links ein Sicherheitsabstand von 20 plus
            //der halben Penbreite einkalkuliert.
            const USHORT nLeft = pC->GetLeft();
            const USHORT nRight = pC->GetRight();

            aLR.SetLeft ( nLeft );
            aLR.SetRight( nRight );

            if ( bLine )
            {
                if ( i == 0 )
                {
                    aLR.SetRight( Max( nRight, nMin ) );
                }
                else if ( i == pAttr->GetNumCols() - 1 )
                {
                    aLR.SetLeft ( Max( nLeft, nMin ) );
                }
                else
                {
                    aLR.SetLeft ( Max( nLeft,  nMin ) );
                    aLR.SetRight( Max( nRight, nMin ) );
                }
            }

            if ( bAdjustAttributes )
            {
                SvxULSpaceItem aUL( pSet->GetULSpace() );
                aUL.SetUpper( pC->GetUpper());
                aUL.SetLower( pC->GetLower());

                ((SwLayoutFrm*)pCol)->GetFmt()->SetFmtAttr( aLR );
                ((SwLayoutFrm*)pCol)->GetFmt()->SetFmtAttr( aUL );
            }

            nGutter += aLR.GetLeft() + aLR.GetRight();
        }

        pCol = bR2L ? pCol->GetPrev() : pCol->GetNext();
    }

    if( bOrtho )
    {
        long nInnerWidth = ( nAvail - nGutter ) / pAttr->GetNumCols();
        pCol = Lower();
        for( USHORT i = 0; i < pAttr->GetNumCols(); pCol = pCol->GetNext(), ++i )
        {
            SwTwips nWidth;
            if ( i == pAttr->GetNumCols() - 1 )
                nWidth = nAvail;
            else
            {
                SvxLRSpaceItem aLR( pCol->GetAttrSet()->GetLRSpace() );
                nWidth = nInnerWidth + aLR.GetLeft() + aLR.GetRight();
            }
            if( nWidth < 0 )
                nWidth = 0;

            const Size aColSz = bVert ?
                                Size( Prt().Width(), nWidth ) :
                                Size( nWidth, Prt().Height() );

            pCol->ChgSize( aColSz );

            if( IsBodyFrm() )
                ((SwLayoutFrm*)pCol)->Lower()->ChgSize( aColSz );

            nAvail -= nWidth;
        }
    }
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
