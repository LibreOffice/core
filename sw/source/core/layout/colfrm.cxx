/*************************************************************************
 *
 *  $RCSfile: colfrm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:22 $
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

#include <hintids.hxx>
#include "cntfrm.hxx"
#include "doc.hxx"

#include "hintids.hxx"

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
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
    nType = FRM_COLUMN;
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


BOOL MA_FASTCALL lcl_AddColumns( SwLayoutFrm *pCont, USHORT nCount )
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
            SwColumnFrm *pTmp = new SwColumnFrm( pNeighbourCol->GetFmt() );
            pTmp->SetMaxFtnHeight( nMax );
            pTmp->InsertBefore( pCont, NULL );
            pTmp->bVarHeight = FALSE;
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
                GetFmt()->SetAttr( SwFmtFillOrder() );
            if ( pSave )
                ::RestoreCntnt( pSave, this, 0 );
            return;
        }
        if ( nOldNum == 1 )
        {
            if ( IsBodyFrm() )
                SetFrmFmt( pDoc->GetColumnContFmt() );
            else
                GetFmt()->SetAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );
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
            bAdjustAttributes = ::lcl_AddColumns( this, nAdd );
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
        ::RestoreCntnt( pSave, (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower(), 0 );
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

    //Ist ein Pointer da, oder sollen wir die Attribute einstellen,
    //so stellen wir auf jeden Fall die Spaltenbreiten ein. Andernfalls
    //checken wir, ob eine Einstellung notwendig ist.
    if ( !pAttr )
    {
        pAttr = &GetFmt()->GetCol();
        if ( !bAdjustAttributes )
        {
            ;
            long nAvail = Prt().Width();
            for ( SwLayoutFrm *pCol = (SwLayoutFrm*)Lower();
                  pCol;
                  pCol = (SwLayoutFrm*)pCol->GetNext() )
                nAvail -= pCol->Frm().Width();
            if ( !nAvail )
                return;
        }
    }

    //Sodele, jetzt koennen die Spalten bequem eingestellt werden.
    //Die Breiten werden mitgezaehlt, damit wir dem letzten den Rest geben
    //koennen.
    SwTwips nAvail = Prt().Width();
    const BOOL bLine = pAttr->GetLineAdj() != COLADJ_NONE;
    USHORT nMin = 0;
    if ( bLine )
        nMin = USHORT(20 + (pAttr->GetLineWidth() / 2));
    SwFrm *pCol = Lower();
    for ( USHORT i = 0; i < pAttr->GetNumCols(); pCol = pCol->GetNext(), ++i )
    {
        const SwTwips nWidth = i == (pAttr->GetNumCols() - 1) ?
                                nAvail :
                                pAttr->CalcColWidth( i, USHORT(Prt().Width()) );
        const Size aColSz( nWidth, Prt().Height() );
        pCol->ChgSize( aColSz );

        // Hierdurch werden die ColumnBodyFrms von Seitenspalten angepasst und
        // ihr bFixHeight-Flag wird gesetzt, damit sie nicht schrumpfen/wachsen.
        // Bei Rahmenspalten hingegen soll das Flag _nicht_ gesetzt werden,
        // da BodyFrms in Rahmenspalten durchaus wachsen/schrumpfen duerfen.
        if( IsBodyFrm() )
            ((SwLayoutFrm*)pCol)->Lower()->ChgSize( aColSz );

        nAvail -= nWidth;

        if ( bAdjustAttributes )
        {
            SwColumn *pC = pAttr->GetColumns()[i];
            SwAttrSet* pSet = pCol->GetAttrSet();
            SvxLRSpaceItem aLR( pSet->GetLRSpace() );
            SvxULSpaceItem aUL( pSet->GetULSpace() );

            //Damit die Trennlinien Platz finden, muessen sie hier
            //Beruecksichtigung finden. Ueberall wo zwei Spalten aufeinanderstossen
            //wird jeweils rechts bzw. links ein Sicherheitsabstand von 20 plus
            //der halben Penbreite einkalkuliert.
            if ( bLine )
            {
                if ( i == 0 )
                {   aLR.SetLeft ( pC->GetLeft() );
                    aLR.SetRight( Max(pC->GetRight(), nMin) );
                }
                else if ( i == (pAttr->GetNumCols() - 1) )
                {   aLR.SetLeft ( Max(pC->GetLeft(), nMin) );
                    aLR.SetRight( pC->GetRight() );
                }
                else
                {   aLR.SetLeft ( Max(pC->GetLeft(),  nMin) );
                    aLR.SetRight( Max(pC->GetRight(), nMin) );
                }
            }
            else
            {
                aLR.SetLeft ( pC->GetLeft() );
                aLR.SetRight( pC->GetRight());
            }
            aUL.SetUpper( pC->GetUpper());
            aUL.SetLower( pC->GetLower());

            ((SwLayoutFrm*)pCol)->GetFmt()->SetAttr( aLR );
            ((SwLayoutFrm*)pCol)->GetFmt()->SetAttr( aUL );
        }
    }
}





