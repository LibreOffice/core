/*************************************************************************
 *
 *  $RCSfile: tabfrm.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-13 14:36:37 $
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

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "viewsh.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "viewimp.hxx"
#include "swtable.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "dcontact.hxx"
#include "viewopt.hxx"
#include "hints.hxx"

#include <ftnidx.hxx>

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "cellfrm.hxx"
#include "flyfrms.hxx"
#include "txtfrm.hxx"       //HasFtn()
#include "htmltbl.hxx"
#include "frmsh.hxx"
#include "sectfrm.hxx"  //SwSectionFrm

/*************************************************************************
|*
|*  SwTabFrm::SwTabFrm(), ~SwTabFrm()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
SwTabFrm::SwTabFrm( SwTable &rTab ):
    SwLayoutFrm( rTab.GetFrmFmt() ),
    SwFlowFrm( (SwFrm&)*this ),
    pTable( &rTab )
{
    bComplete = bCalcLowers = bONECalcLowers = bLowersFormatted = bLockBackMove =
    bResizeHTMLTable = FALSE;
    bFixHeight = FALSE;     //Nicht nochmal auf die Importfilter hereinfallen.
    nType = FRM_TAB;

    //Gleich die Zeilen erzeugen und einfuegen.
    const SwTableLines &rLines = rTab.GetTabLines();
    SwFrm *pPrev = 0;
    for ( USHORT i = 0; i < rLines.Count(); ++i )
    {
        SwRowFrm *pNew = new SwRowFrm( *rLines[i] );
        if( pNew->Lower() )
        {
            pNew->InsertBehind( this, pPrev );
            pPrev = pNew;
        }
        else
            delete pNew;
    }
}

SwTabFrm::SwTabFrm( SwTabFrm &rTab ) :
    SwLayoutFrm( rTab.GetFmt() ),
    SwFlowFrm( (SwFrm&)*this ),
    pTable( rTab.GetTable() )
{
    bIsFollow = TRUE;
    bLockJoin = bComplete = bONECalcLowers = bCalcLowers = bLowersFormatted = bLockBackMove =
    bResizeHTMLTable = FALSE;
    bFixHeight = FALSE;     //Nicht nochmal auf die Importfilter hereinfallen.
    nType = FRM_TAB;

    SetFollow( rTab.GetFollow() );
    rTab.SetFollow( this );
}

SwTabFrm::~SwTabFrm()
{
}

/*************************************************************************
|*
|*  SwTabFrm::JoinAndDelFollows()
|*
|*  Ersterstellung      MA 30. May. 96
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
void SwTabFrm::JoinAndDelFollows()
{
    SwTabFrm *pFoll = GetFollow();
    if ( pFoll->HasFollow() )
        pFoll->JoinAndDelFollows();
    pFoll->Cut();
    SetFollow( pFoll->GetFollow() );
    delete pFoll;
}

/*************************************************************************
|*
|*  SwTabFrm::RegistFlys()
|*
|*  Ersterstellung      MA 08. Jul. 93
|*  Letzte Aenderung    MA 27. Jan. 99
|*
|*************************************************************************/
void SwTabFrm::RegistFlys()
{
    ASSERT( Lower() && Lower()->IsRowFrm(), "Keine Zeilen." );

    SwPageFrm *pPage = FindPageFrm();
    if ( pPage )
    {
        SwRowFrm *pRow = (SwRowFrm*)Lower();
        do
        {   pRow->RegistFlys( pPage );
            pRow = (SwRowFrm*)pRow->GetNext();
        } while ( pRow );
    }
}

/*************************************************************************
|*
|*  SwTabFrm::Split(), Join()
|*
|*  Ersterstellung      MA 03. Jun. 93
|*  Letzte Aenderung    MA 03. Sep. 96
|*
|*************************************************************************/
SwTwips SwTabFrm::Split( const SwTwips nCutPos )
{
    ASSERT( nCutPos >= Frm().Top() && nCutPos <= Frm().Bottom(),
            "SplitLine ausserhalb der Tabelle." );

    //Um die Positionen der Zellen mit der CutPos zu vergleichen muessen sie
    //ausgehend von der Tabelle nacheinander berechnet werden. Sie koennen
    //wg. Positionsaenderungen der Tabelle durchaus ungueltig sein.

    SwFrm *pRow = Lower();
    if( !pRow )
        return 0;
    SwTwips nRowPos = Frm().Top() + Prt().Top() + pRow->Frm().Height();
    const BOOL bRepeat  = GetTable()->IsHeadlineRepeat();
    pRow = pRow->GetNext();
    if( pRow && bRepeat )
    {
        nRowPos += pRow->Frm().Height();
        pRow = pRow->GetNext();
    }
    // No break before the first row and, in case of repeated headlines,
    // before the the second row.
    if( !pRow )
        return 0;

    while ( pRow && nCutPos > (nRowPos + pRow->Frm().Height() - 1) )
    {
        nRowPos += pRow->Frm().Height();
        pRow = pRow->GetNext();
    }

    if ( !pRow )
    {
#ifdef DEBUG
        ASSERT( FALSE, "Tablesplit out of rows?" );
#endif
        pRow = Lower();
        while ( pRow && pRow->GetNext() )
            pRow = pRow->GetNext();
    }

    //Wenn es bereits einen Follow gibt so geht's dort hinein andernfalls
    //muss eben einer erzeugt werden.
    FASTBOOL bNewFollow;
    SwTabFrm *pFoll;
    if ( GetFollow() )
    {
        pFoll = GetFollow();
        bNewFollow = FALSE;
    }
    else
    {
        bNewFollow = TRUE;
        pFoll = new SwTabFrm( *this );
        pFoll->InsertBehind( GetUpper(), this );

        if( bRepeat )
        {   //Ueberschrift wiederholen.
            ASSERT( GetTable()->GetTabLines()[0], "Table ohne Zeilen?" );
            bDontCreateObjects = TRUE;              //frmtool
            SwRowFrm *pHeadline = new SwRowFrm(
                                    *GetTable()->GetTabLines()[0] );
            bDontCreateObjects = FALSE;
            pHeadline->InsertBefore( pFoll, 0 );
        }
    }
    SwTwips nRet = 0;
    SwFrm *pNxt;

    //Optimierung beim neuen Follow braucht's kein Paste und dann kann
    //das Optimierte Insert verwendet werden (nur dann treten gluecklicher weise
    //auch groessere Mengen von Rows auf).
    if ( bNewFollow )
    {
        SwFrm *pPrv = GetTable()->IsHeadlineRepeat() ? pFoll->Lower() : 0;
        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nRet += pRow->Frm().Height();
            pRow->Remove();
            pRow->InsertBehind( pFoll, pPrv );
            pRow->_InvalidateAll();
            pPrv = pRow;
            pRow = pNxt;
        }
    }
    else
    {
        SwFrm *pPrv = pFoll->Lower();
        if ( pPrv && GetTable()->IsHeadlineRepeat() )
            pPrv = pPrv->GetNext();
        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nRet += pRow->Frm().Height();
            pRow->Remove();
            pRow->Paste( pFoll, pPrv );
            pRow = pNxt;
        }
    }
    ASSERT( !bNewFollow || !pFoll->Frm().Height(), "Dont care about Performance");
    Shrink( nRet, pHeight );
    return nRet;
}

SwTwips SwTabFrm::Join()
{
    SwTabFrm *pFoll = GetFollow();
    SwTwips nHeight = 0;    //Gesamthoehe der eingefuegten Zeilen als Return.

    if ( !pFoll->IsJoinLocked() )
    {
        pFoll->Cut();   //Erst ausschneiden um unuetze Benachrichtigungen zu
                        //minimieren.

        SwFrm *pRow = pFoll->Lower(),
              *pNxt;

        if ( pRow && GetTable()->IsHeadlineRepeat() )
            pRow = pRow->GetNext();

        SwFrm *pPrv = Lower();
        while ( pPrv && pPrv->GetNext() )
            pPrv = pPrv->GetNext();
        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nHeight += pRow->Frm().Height();
            pRow->Remove();
            pRow->_InvalidateAll();
            pRow->InsertBehind( this, pPrv );
            pPrv = pRow;
            pRow = pNxt;
        }
        SetFollow( pFoll->GetFollow() );
        delete pFoll;
        Grow( nHeight, pHeight );
    }
    return nHeight;
}

/*************************************************************************
|*
|*  SwTabFrm::MakeAll()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 10. Apr. 97
|*
|*************************************************************************/
void MA_FASTCALL SwInvalidatePositions( SwFrm *pFrm, long nBottom )
{
    do
    {   pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        if( pFrm->IsLayoutFrm() )
        {
            if ( ((SwLayoutFrm*)pFrm)->Lower() )
                ::SwInvalidatePositions( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
        }
        else
            pFrm->Prepare( PREP_ADJUST_FRM );
        pFrm = pFrm->GetNext();
    } while ( pFrm && pFrm->Frm().Top() < nBottom );
}

BOOL MA_FASTCALL lcl_CalcLowers( SwLayoutFrm *pLay, long nBottom )
{
    BOOL bRet = FALSE;
    SwCntntFrm *pCnt = pLay->ContainsCntnt();
    while ( pCnt && pLay->GetUpper()->IsAnLower( pCnt ) )
    {
        bRet |= !pCnt->IsValid();
        pCnt->CalcFlys( FALSE );
        pCnt->Calc();
        pCnt->GetUpper()->Calc();
        if ( pCnt->Frm().Top() > nBottom )
            break;
        pCnt = pCnt->GetNextCntntFrm();
    }
    return bRet;
}

BOOL MA_FASTCALL lcl_InnerCalcLayout( SwFrm *pFrm, long nBottom )
{
    BOOL bRet = FALSE;
    do
    {
        if( pFrm->IsLayoutFrm() )
        {
            bRet |= !pFrm->IsValid();
            pFrm->Calc();
            if( ((SwLayoutFrm*)pFrm)->Lower() )
                bRet |= lcl_InnerCalcLayout( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
        }
        pFrm = pFrm->GetNext();
    } while ( pFrm && pFrm->Frm().Top() < nBottom );
    return bRet;
}

void MA_FASTCALL lcl_CalcLayout( SwLayoutFrm *pLay, long nBottom )
{
    BOOL bCheck = TRUE;
    do
    {
        while( lcl_InnerCalcLayout( pLay, nBottom ) )
            bCheck = TRUE;
        if( bCheck )
        {
            bCheck = FALSE;
            if( lcl_CalcLowers( pLay, nBottom ) )
                continue;
        }
        break;
    } while( TRUE );
}

void MA_FASTCALL lcl_FirstTabCalc( SwTabFrm *pTab )
{
    //Ersteinmal koennen wir die Strukturen auf die richtige Groesse
    //bringen.
    if ( !pTab->IsFollow() && !pTab->GetTable()->IsTblComplex() )
    {
        SwLayoutFrm *pRow = (SwLayoutFrm*)pTab->Lower();
        do
        {
            SwLayoutFrm *pCell = (SwLayoutFrm*)pRow->Lower();
            SwFrm *pCnt = pCell->Lower();
            pCnt->Calc();
            const long nCellHeight = pCell->Frm().Height();
            const long nCellY      = pCell->Frm().Top()-1;
            const long nCntHeight  = pCnt->Frm().Height();
            const long nCntY       = pCnt->Frm().Top()-1;
            if ( 0 != (pCell = (SwLayoutFrm*)pCell->GetNext()) )
                do
                {   pCell->Frm().SSize().Height() =
                    pCell->Prt().SSize().Height() = nCellHeight;
                    pCell->Frm().Pos().Y() = nCellY;
                    pCell->_InvalidateAll();

                    pCnt = pCell->Lower();
                    pCnt->Frm().SSize().Height() =
                    pCnt->Prt().SSize().Height() = nCntHeight;
                    pCnt->Frm().Pos().Y() = nCntY;
                    pCnt->_InvalidateAll();

                    pCell = (SwLayoutFrm*)pCell->GetNext();
                } while ( pCell );

            pRow = (SwLayoutFrm*)pRow->GetNext();

        } while ( pRow );
    }
    //MA 28. Nov. 95: einen weiteren habe ich noch: Es braucht
    //nur soweit formatiert werden, wie Platz vorhanden ist.
    SwFrm *pUp = pTab->GetUpper();
    long nBottom = pUp->Prt().Bottom() + pUp->Frm().Top();
    if ( pTab->GetFmt()->GetDoc()->IsBrowseMode() )
        nBottom += pUp->Grow( LONG_MAX, pHeight, TRUE );
    lcl_CalcLowers( (SwLayoutFrm*)pTab->Lower(), nBottom );
}

void MA_FASTCALL lcl_Recalc( SwTabFrm *pTab,
                             SwLayoutFrm *pFirstRow,
                             SwLayNotify &rNotify )
{
    if ( pTab->Lower() )
    {
        const SwTwips nOldHeight = pTab->Frm().Height();
        const SwTwips nOldWidth  = pTab->Frm().Width();
        if ( !pFirstRow )
        {
            pFirstRow = (SwLayoutFrm*)pTab->Lower();
            rNotify.SetLowersComplete( TRUE );
        }
        ::SwInvalidatePositions( pFirstRow, LONG_MAX );
        ::lcl_CalcLayout( pFirstRow, LONG_MAX );
        if ( nOldHeight < pTab->Frm().Height() )
            rNotify.AddHeightOfst( pTab->Frm().Height() - nOldHeight );
        else if ( nOldHeight > pTab->Frm().Height() )
            rNotify.SubtractHeightOfst( nOldHeight - pTab->Frm().Height() );
        if ( nOldWidth < pTab->Frm().Width() )
            rNotify.AddWidthOfst( pTab->Frm().Width() - nOldWidth );
        else if ( nOldWidth > pTab->Frm().Width() )
            rNotify.SubtractWidthOfst( nOldWidth - pTab->Frm().Width() );
    }
}

#define KEEPTAB ( !GetFollow() && !IsFollow() )

void SwTabFrm::MakeAll()
{
    if ( IsJoinLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;

    LockJoin(); //Ich lass mich nicht unterwegs vernichten.
    SwLayNotify aNotify( this );    //uebernimmt im DTor die Benachrichtigung

    //Wenn mein direkter Nachbar gleichzeitig mein Follow ist
    //verleibe ich mir das Teil ein.
    if ( GetNext() && GetNext() == GetFollow() )
        aNotify.AddHeightOfst( Join() );

    if ( bResizeHTMLTable ) //Optimiertes Zusammenspiel mit Grow/Shrink des Inhaltes
    {
        bResizeHTMLTable = FALSE;
        SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
        if ( pLayout )
            bCalcLowers = pLayout->Resize(
                            pLayout->GetBrowseWidthByTabFrm( *this ), FALSE );
    }


    BOOL bMakePage  = TRUE;     //solange TRUE kann eine neue Seite
                                //angelegt werden (genau einmal)
    BOOL bMovedBwd  = FALSE;    //Wird TRUE wenn der Frame zurueckfliesst
    BOOL bMovedFwd  = FALSE;    //solange FALSE kann der Frm zurueck-
                                //fliessen (solange, bis er einmal
                                //vorwaerts ge'moved wurde).
    BOOL bSplit     = FALSE;    //Wird TRUE wenn der Frm gesplittet wurde.
    BOOL bFtnsInDoc = 0 != GetFmt()->GetDoc()->GetFtnIdxs().Count();
    BOOL bMoveable;
    const BOOL bRepeat  = GetTable()->IsHeadlineRepeat();
    const BOOL bFly     = IsInFly();

    SwBorderAttrAccess  *pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs *pAttrs = pAccess->Get();

    const BOOL bKeep = IsKeep( *pAttrs );
    const BOOL bDontSplit = !IsFollow() && !GetFmt()->GetLayoutSplit().GetValue();

    if ( bDontSplit )
        while ( GetFollow() )
            aNotify.AddHeightOfst( Join() );

    //Einen Frischling moven wir gleich schon einmal vorwaerts...
    if ( !Frm().Top() && IsFollow() )
    {
        SwFrm *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
        {
            if ( !MoveFwd( bMakePage, FALSE ) )
                bMakePage = FALSE;
            bMovedFwd = TRUE;
        }
    }

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( TRUE == (bMoveable = IsMoveable()) )
            if ( CheckMoveFwd( bMakePage, bKeep && KEEPTAB, bMovedBwd ) )
            {
                bMovedFwd = TRUE;
                bCalcLowers = TRUE;
            }

        Point aOldPos( Frm().Pos() );
        MakePos();
        if ( aOldPos != Frm().Pos() )
        {
            if ( aOldPos.Y() != Frm().Top() )
            {
                SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
                if( pLayout )
                {
                    delete pAccess;
                    bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrm( *this ), FALSE );
                    pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                    pAttrs = pAccess->Get();
                }

                bValidPrtArea = FALSE;
                aNotify.SetLowersComplete( FALSE );
            }
            SwFrm *pPre;
            if ( bKeep || (0 != (pPre = FindPrev()) &&
                           pPre->GetAttrSet()->GetKeep().GetValue()) )
            {
                bCalcLowers = TRUE;
            }
        }

        //Wir muessen die Hoehe der ersten Zeile kennen, denn nur wenn diese
        //kleiner wird muss ggf. der Master angestossen werden um noetigenfalls
        //die Zeile aufzunehmen.
        long n1StLineHeight = 0;
        if ( IsFollow() )
        {
            SwFrm *pFrm = Lower();
            if ( bRepeat && pFrm )
                pFrm = pFrm->GetNext();
            if ( pFrm )
                n1StLineHeight = pFrm->Frm().Height();
        }

        if ( !bValidSize || !bValidPrtArea )
        {
            const BOOL bOptLower = Frm().Height() == 0;

            const long nOldPrtWidth = Prt().Width();
            const long nOldFrmWidth = Frm().Width();
            const Point aOldPrtPos  = Prt().Pos();
            Format( pAttrs );

            SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
            if ( /*!bOptLower &&*/ pLayout &&
                 (Prt().Width() != nOldPrtWidth ||
                  Frm().Width() != nOldFrmWidth) )
            {
                delete pAccess;
                bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrm( *this ), FALSE );
//                  GetFmt()->GetDoc()->GetDocShell()->IsReadOnly() ? FALSE : TRUE );
                pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                pAttrs = pAccess->Get();
            }
            if ( !bOptLower && aOldPrtPos != Prt().Pos() )
            {
                aNotify.SetLowersComplete( FALSE );
            }

            if ( bOptLower )
            {
                //MA 24. May. 95: Optimierungsversuch!
                //Ganz nigel nagel neu das Teil. Damit wir nicht n-fach
                //MakeAll'en formatieren wir flugs den Inhalt.
                //Das erste Format mussten wir allerdings abwarten, damit
                //die Breiten Stimmen!
                //MA: Fix, Kein Calc wenn evtl. noch Seitengebunde Flys
                //an den Cntnt haengen (siehe frmtool.cxx, ~SwCntntNotify).
                SwDoc *pDoc = GetFmt()->GetDoc();
                if ( !pDoc->GetSpzFrmFmts()->Count() ||
                     pDoc->IsLoaded() || pDoc->IsNewDoc() )
                {
                    //MA 28. Nov. 95: Und wieder ein Trick, gleich mal sehen
                    //ob ein Rueckfluss lohnt.
                    if ( bMoveable && !GetPrev() )
                    {
                        GetLeaf( MAKEPAGE_NONE, FALSE ); //setzt das BackMoveJump
                        if ( SwFlowFrm::IsMoveBwdJump() )
                        {
                            BOOL bDummy;
                            SwFtnBossFrm *pOldBoss = bFtnsInDoc ?
                                FindFtnBossFrm( TRUE ) : 0;
                            const FASTBOOL bOldPrev = GetPrev() != 0;
                            if ( MoveBwd( bDummy ) )
                            {
                                bMovedBwd = TRUE;
                                if ( bFtnsInDoc )
                                    MoveLowerFtns( 0, pOldBoss, 0, TRUE );

                                long nOldTop = Frm().Top();
                                MakePos();
                                if( nOldTop != Frm().Top() )
                                {
                                    SwHTMLTableLayout *pLayout =
                                        GetTable()->GetHTMLTableLayout();
                                    if( pLayout )
                                    {
                                        delete pAccess;
                                        bCalcLowers |= pLayout->Resize(
                                            pLayout->GetBrowseWidthByTabFrm(
                                                            *this ), FALSE );
                                        pAccess= new SwBorderAttrAccess(
                                                    SwFrm::GetCache(), this );
                                        pAttrs = pAccess->Get();
                                    }
                                }

                                if ( bOldPrev != (0 != GetPrev()) )
                                {
                                    //Abstand nicht vergessen!
                                    bValidPrtArea = FALSE;
                                    Format( pAttrs );
                                }
                                if ( bKeep && KEEPTAB )
                                {
                                    SwFrm *pNxt = FindNextCnt();
                                    // FindNextCnt geht ggf. in einen Bereich
                                    // hinein, in eine Tabelle allerdings auch
                                    if( pNxt && pNxt->IsInTab() )
                                        pNxt = pNxt->FindTabFrm();
                                    if ( pNxt )
                                    {
                                        pNxt->Calc();
                                        if ( !GetNext() )
                                            bValidPos = FALSE;
                                    }
                                }
                            }
                        }
                    }
                    ::lcl_FirstTabCalc( this );
                    bValidSize = bValidPrtArea = FALSE;
                    Format( pAttrs );
                    aNotify.SetLowersComplete( TRUE );
                }
            }
        }

        //Wenn ich der erste einer Kette bin koennte ich mal sehen ob
        //ich zurueckfliessen kann (wenn ich mich ueberhaupt bewegen soll).
        //Damit es keine Oszillation gibt, darf ich nicht gerade vorwaerts
        //geflosssen sein.
        if ( !GetIndPrev() && !bMovedFwd && (bMoveable || bFly) )
        {
            //Bei Follows muss der Master benachrichtigt
            //werden. Der Follow muss nur dann Moven, wenn er leere Blaetter
            //ueberspringen muss.
            if ( IsFollow() )
            {
                //Nur wenn die Hoehe der ersten Zeile kleiner geworder ist.
                SwFrm *pFrm = Lower();
                if ( bRepeat && pFrm )
                    pFrm = pFrm->GetNext();
                if ( pFrm && n1StLineHeight > pFrm->Frm().Height() )
                {
                    SwTabFrm *pMaster = (SwTabFrm*)FindMaster();
                    BOOL bDummy;
                    if ( ShouldBwdMoved( pMaster->GetUpper(), FALSE, bDummy ) )
                        pMaster->InvalidatePos();
                }
            }
            SwFtnBossFrm *pOldBoss = bFtnsInDoc ? FindFtnBossFrm( TRUE ) : 0;
            BOOL bReformat;
            if ( MoveBwd( bReformat ) )
            {
                bMovedBwd = TRUE;
                aNotify.SetLowersComplete( FALSE );
                if ( bFtnsInDoc )
                    MoveLowerFtns( 0, pOldBoss, 0, TRUE );
                if ( bReformat || bKeep )
                {
                    long nOldTop = Frm().Top();
                    MakePos();
                    if ( nOldTop != Frm().Top() )
                    {
                        SwHTMLTableLayout *pLayout =
                            GetTable()->GetHTMLTableLayout();
                        if( pLayout )
                        {
                            delete pAccess;
                            bCalcLowers |= pLayout->Resize(
                                pLayout->GetBrowseWidthByTabFrm( *this ),
                                FALSE );
                            pAccess= new SwBorderAttrAccess(
                                        SwFrm::GetCache(), this );
                            pAttrs = pAccess->Get();
                        }

                        bValidPrtArea = FALSE;
                        Format( pAttrs );
                    }
                    ::lcl_Recalc( this, 0, aNotify );
                    bLowersFormatted = TRUE;
                    if ( bKeep && KEEPTAB )
                    {
                        SwFrm *pNxt = FindNextCnt();
                        if( pNxt && pNxt->IsInTab() )
                            pNxt = pNxt->FindTabFrm();
                        if ( pNxt )
                        {
                            pNxt->Calc();
                            if ( !GetNext() )
                                bValidPos = FALSE;
                        }
                    }
                }
            }
        }

        //Wieder ein Wert ungueltig? - dann nochmal das ganze...
        if ( !bValidPos || !bValidSize || !bValidPrtArea )
            continue;

        //Fertig?
        if ( GetUpper()->Prt().Bottom()+GetUpper()->Frm().Top() >=
             Frm().Bottom() )
        {
            //Wenn ich zur Unterkante des Upper noch Raum habe, so kann ich
            //wenigstens probehalber eine weitere Zeile meines Follows
            //aufnehmen.
            if ( !bSplit && GetFollow() )
            {
                BOOL bDummy;
                if ( GetFollow()->ShouldBwdMoved( GetUpper(), FALSE, bDummy ) )
                {
                    SwFrm *pTmp = GetUpper();
                    SwTwips nDeadLine = pTmp->Prt().Bottom() + pTmp->Frm().Top();
                    if ( GetFmt()->GetDoc()->IsBrowseMode() )
                        nDeadLine += pTmp->Grow( LONG_MAX, pHeight, TRUE );
                    if ( Frm().Bottom() < nDeadLine )
                    {
                        SwFrm *pRow = GetFollow()->Lower();
                        if ( bRepeat )
                            pRow = pRow->GetNext();
                        const SwTwips nOld = Frm().Height();

                        const BOOL bMoveFtns = bFtnsInDoc && pRow &&
                                               !GetFollow()->IsJoinLocked();

                        SwFtnBossFrm *pOldBoss;
                        if ( bMoveFtns )
                            pOldBoss = pRow->FindFtnBossFrm( TRUE );

                        //fix(8680): Row kann 0 werden.
                        if ( !pRow || !pRow->GetNext() )
                            //Der Follow wird leer und damit ueberfluessig.
                            aNotify.AddHeightOfst( Join() );
                        else
                        {
                            pRow->Cut();
                            pRow->Paste( this );
                            aNotify.AddHeightOfst( pRow->Frm().Height() );
                        }
                        //Die Fussnoten verschieben!
                        if ( pRow && bMoveFtns )
                            if ( ((SwLayoutFrm*)pRow)->MoveLowerFtns(
                                 0, pOldBoss, FindFtnBossFrm( TRUE ), TRUE ) )
                                GetUpper()->Calc();

                        if ( pRow && nOld != Frm().Height() )
                            ::lcl_Recalc( this, (SwLayoutFrm*)pRow, aNotify );
                        continue;
                    }
                }
            }
            else if ( bKeep && KEEPTAB )
            {
                SwFrm *pNxt = FindNextCnt();
                if( pNxt && pNxt->IsInTab() )
                    pNxt = pNxt->FindTabFrm();
                if ( pNxt )
                    pNxt->Calc();
            }
            if ( IsValid() )
            {
                if ( bCalcLowers )
                {
                    ::lcl_Recalc( this, 0, aNotify );
                    bLowersFormatted = TRUE;
                    bCalcLowers = FALSE;
                }
                else if ( bONECalcLowers )
                {
                    lcl_CalcLayout( (SwLayoutFrm*)Lower(), LONG_MAX );
                    bONECalcLowers = FALSE;
                }
            }
            continue;
        }

        //Ich passe nicht mehr in meinen Uebergeordneten, also ist es jetzt
        //an der Zeit moeglichst konstruktive Veranderungen vorzunehmen

        //Wenn ich den uebergeordneten Frm nicht verlassen darf, habe
        //ich ein Problem; Frei nach Artur Dent tun wir das einzige das man
        //mit einen nicht loesbaren Problem tun kann: wir ignorieren es - und
        //zwar mit aller Kraft.
        if ( !bMoveable )
        {
            if ( bCalcLowers && IsValid() )
            {
                lcl_Recalc( this, 0, aNotify );
                bLowersFormatted = TRUE;
                bCalcLowers = FALSE;
            }
            else if ( bONECalcLowers )
            {
                lcl_CalcLayout( (SwLayoutFrm*)Lower(), LONG_MAX );
                bONECalcLowers = FALSE;
            }
            continue;
        }

        //Der erste Versuch muss natuerlich das Aufspalten der Tabelle sein.
        //Das funktioniert natuerlich nur dann, wenn die Tabelle mehr als eine
        //Zeile enthaelt und wenn die Unterkante des Upper unter der ersten
        //Zeile liegt.
        SwFrm *pIndPrev = GetIndPrev();
        if ( Lower()->GetNext() && (!bDontSplit || !pIndPrev) )
        {
            //Damit der Schatten nicht extra herausgerechnet werden muss,
            //lassen wir das Spiel gleich wenn es ein HeadlineRepeat gibt und
            //nur noch eine nicht Headline Zeile vorhanden ist.
            if ( !bRepeat || Lower()->GetNext()->GetNext() )
            {
                SwTwips nDeadLine = GetUpper()->Prt().Bottom() +
                                    GetUpper()->Frm().Top();
                if( IsInSct() )
                    nDeadLine += GetUpper()->Grow( LONG_MAX, pHeight, TRUE );

                //Zunaechst einmal sollten wir fuer Stabilitaet sorgen,
                //denn andernfalls koennen wir nicht hinreichend zuverlaessig
                //splitten.
                ::lcl_CalcLayout( (SwLayoutFrm*)Lower(), nDeadLine );
                bLowersFormatted = TRUE;
                aNotify.SetLowersComplete( TRUE );
                if( Frm().Bottom() < nDeadLine )
                    continue;

                //Position unter der ersten Zeile ermitteln. Wenn Headlines im
                //Spiel sind, ist es die Pos unter der ersten nicht-Headline
                //Zeile.
                SwTwips nBreakLine = Lower()->Frm().Height();
                if ( bRepeat )
                    nBreakLine += Lower()->GetNext()->Frm().Height();
                nBreakLine += Frm().Top() + Prt().Top();
                if ( nBreakLine <= nDeadLine || !pIndPrev )
                {
                    aNotify.SubtractHeightOfst( Split( nDeadLine ) );
                    if ( aNotify.GetHeightOfst() < 0 )
                        aNotify.ResetHeightOfst();
                    aNotify.SetLowersComplete( FALSE );
                    bSplit = TRUE;
                    //Damit es nicht zu Oszillationen kommt, muss der
                    //Follow gleich gueltig gemacht werden.
                    if ( GetFollow() )
                    {
                        if ( !StackHack::IsLocked() )
                        {
                            StackHack aHack;
                            GetFollow()->MakeAll();
                            ((SwTabFrm*)GetFollow())->SetLowersFormatted(FALSE);
                            ::lcl_CalcLayout((SwLayoutFrm*)GetFollow()->Lower(),
                                GetFollow()->GetUpper()->Frm().Bottom() );
                            if ( !GetFollow()->GetFollow() )
                            {
                                SwFrm *pNxt = ((SwFrm*)GetFollow())->FindNext();
                                if ( pNxt )
                                    pNxt->Calc();
                            }
                        }
                        else if ( GetFollow() == GetNext() )
                            ((SwTabFrm*)GetFollow())->MoveFwd( TRUE, FALSE );
                        ViewShell *pSh;
                        if ( 0 != (pSh = GetShell()) )
                            pSh->Imp()->ResetScroll();
                    }
                    continue;
                }
            }
        }

        if( IsInSct() && bMovedFwd && bMakePage && GetUpper()->IsColBodyFrm() &&
            GetUpper()->GetUpper()->GetUpper()->IsSctFrm() &&
            ( GetUpper()->GetUpper()->GetPrev() || GetIndPrev() ) &&
            ((SwSectionFrm*)GetUpper()->GetUpper()->GetUpper())->MoveAllowed(this) )
            bMovedFwd = FALSE;

        //Mal sehen ob ich irgenwo Platz finde...
        if ( !bMovedFwd && !MoveFwd( bMakePage, FALSE ) )
            bMakePage = FALSE;
        bMovedFwd = bCalcLowers = TRUE;
        aNotify.SetLowersComplete( FALSE );
        if ( IsFollow() )
        {   //Um Oszillationen zu vermeiden sollte kein ungueltiger Master
            //zurueckbleiben.
            SwTabFrm *pTab = FindMaster();
            if ( pTab->GetUpper() )
                pTab->GetUpper()->Calc();
            pTab->Calc();
            pTab->SetLowersFormatted( FALSE );
        }

        //Wenn mein direkter Nachbar jetzt gleichzeitig mein Follow ist
        //verleibe ich mir das Teil ein.
        if ( GetNext() && GetNext() == GetFollow() )
            aNotify.AddHeightOfst( Join() );

        if ( bMovedBwd && GetUpper() )
            //Beim zurueckfliessen wurde der Upper angeregt sich vollstaendig
            //zu Painten, dass koennen wir uns jetzt nach dem hin und her
            //fliessen sparen.
            GetUpper()->ResetCompletePaint();

        if ( bCalcLowers && IsValid() )
        {
            ::lcl_Recalc( this, 0, aNotify );
            bLowersFormatted = TRUE;
            bCalcLowers = FALSE;
        }

    } //while ( !bValidPos || !bValidSize || !bValidPrtArea )

    //Wenn mein direkter Vorgaenger jetzt mein Master ist, so kann er mich
    //bei der nachstbesten Gelegenheit vernichten.
    if ( IsFollow() )
    {
        SwFrm *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
            pPre->InvalidatePos();
    }

    bCalcLowers = bONECalcLowers = FALSE;
    delete pAccess;
    UnlockJoin();
    if ( bMovedFwd || bMovedBwd )
        aNotify.SetInvaKeep();
}

/*************************************************************************
|*
|*  SwTabFrm::CalcFlyOffsets()
|*
|*  Beschreibung:       Berechnet die Offsets, die durch FlyFrames
|*                      entstehen.
|*  Ersterstellung      MA/MIB 14. Apr. 99
|*  Letzte Aenderung
|*
|*************************************************************************/
BOOL SwTabFrm::CalcFlyOffsets( SwTwips& rUpper,
                               long& rLeftOffset,
                               long& rRightOffset ) const
{
    BOOL bInvalidatePrtArea = FALSE;
    const SwPageFrm *pPage = FindPageFrm();
    const SwFlyFrm* pMyFly = FindFlyFrm();
    if ( pPage->GetSortedObjs() )
    {
        long nPrtPos = Frm().Top() + rUpper;
        SwRect aRect( Frm() );
        if ( Prt().Top() - rUpper > 0 )
            aRect.SSize().Height() -= Prt().Top() - rUpper;
        for ( USHORT i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
        {
            SdrObject *pObj = (*pPage->GetSortedObjs())[i];
            if ( pObj->IsWriterFlyFrame() )
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                const SwRect aFlyRect = pFly->AddSpacesToFrm();
                if ( WEIT_WECH != pFly->Frm().Top() &&
                     pFly->IsFlyAtCntFrm() && aFlyRect.IsOver( aRect ) &&
                     pFly->GetAnchor()->Frm().Bottom() < Frm().Top() &&
                     !IsAnLower( pFly ) && !pFly->IsAnLower( this ) &&
                     ( !pMyFly || pMyFly->IsAnLower( pFly ) ) &&
                     pPage->GetPhyPageNum() >=
                     pFly->GetAnchor()->FindPageFrm()->GetPhyPageNum() )
                {
                    const SwFmtSurround   &rSur = pFly->GetFmt()->GetSurround();
                    const SwFmtHoriOrient &rHori= pFly->GetFmt()->GetHoriOrient();
                    if ( SURROUND_NONE == rSur.GetSurround() )
                    {
                        nPrtPos = Max( nPrtPos, aFlyRect.Bottom() + 1l );
                        bInvalidatePrtArea = TRUE;
                    }
                    if ( (SURROUND_RIGHT    == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         HORI_LEFT == rHori.GetHoriOrient() )
                    {
                        //Der Rahmen kann auch noch einem anderen Rahmen
                        //ausgewichen sein.
                        const long nWidth = aFlyRect.Width() +
                                    aFlyRect.Left() - pFly->GetAnchor()->Frm().Left();
                        rLeftOffset = Max( rLeftOffset, nWidth );
                        bInvalidatePrtArea = TRUE;
                    }
                    if ( (SURROUND_LEFT     == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         HORI_RIGHT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = aFlyRect.Width() +
                                    pFly->GetAnchor()->Frm().Right() - aFlyRect.Right();
                        rRightOffset = Max( rRightOffset, nWidth );
                        bInvalidatePrtArea = TRUE;
                    }
                }
            }
        }
        rUpper = nPrtPos - Frm().Top();
    }

    return bInvalidatePrtArea;
}

/*************************************************************************
|*
|*  SwTabFrm::Format()
|*
|*  Beschreibung:       "Formatiert" den Frame; Frm und PrtArea
|*                      Die Fixsize wird hier nicht eingestellt.
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 18. Jun. 97
|*
|*************************************************************************/
void SwTabFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "TabFrm::Format, pAttrs ist 0." );

    //FixSize einstellen
    if ( !bValidSize )
        aFrm.Width( GetUpper()->Prt().Width() );

    //VarSize ist immer die Hoehe.
    //Fuer den oberen/unteren Rand gelten die selben Regeln wie fuer
    //cntfrms (sie MakePrtArea() von diesen).

    SwTwips nUpper = CalcUpperSpace( pAttrs );

    //Wir wollen Rahmen ausweichen. Zwei Moeglichkeiten:
    //1. Es gibt Rahmen mit SurroundNone, diesen wird vollsaendig ausgewichen
    //2. Es gibt Rahmen mit Umlauf nur rechts bzw. nur links und diese sind
    //   rechts bzw. links ausgerichtet, diese geben ein Minimum fuer die
    //   Raender vor.
    long nTmpRight = -1000000,
         nLeftOffset  = 0;
    if( CalcFlyOffsets( nUpper, nLeftOffset, nTmpRight ) )
        bValidPrtArea = FALSE;
    long nRightOffset = Max( 0L, nTmpRight );

    SwTwips nLower = pAttrs->CalcBottomLine();

    if ( !bValidPrtArea )
    {   bValidPrtArea = TRUE;

        const SwTwips nOldHeight = Prt().Height();

        //Die Breite der PrtArea wird vom FrmFmt vorgegeben, die Raender
        //sind entsprechend einzustellen.
        //Mindestraender werden von Umrandung und Schatten vorgegeben.
        //Die Rander werden so eingestellt, dass die PrtArea nach dem
        //angegebenen Adjustment im Frm ausgerichtet wird.
        //Wenn das Adjustment 0 ist, so werden die Rander anhand des
        //Randattributes eingestellt.

        const SwTwips nMax = aFrm.Width();
        SwTwips nLeft  = pAttrs->CalcLeftLine();
        SwTwips nRight = pAttrs->CalcRightLine();

        //Die Breite ist evtl. eine Prozentangabe. Wenn die Tabelle irgendwo
        //'drinsteckt bezieht sie sich auf die Umgebung. Ist es der Body, so
        //bezieht sie sich in der BrowseView auf die Bildschirmbreite.
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        const SwTwips nWish = CalcRel( rSz, TRUE );

        BOOL bCheckBrowseWidth = FALSE;

        switch ( GetFmt()->GetHoriOrient().GetHoriOrient() )
        {
            case HORI_LEFT:
                {
                    const SwTwips nTmp = nMax-nLeftOffset-nRightOffset -
                                         (nWish + nLeft);
                    nLeft  += nLeftOffset;
                    nRight += nRightOffset;
                    nRight -= nLeftOffset;
                    if ( nTmp > nRight )
                        nRight = nTmp;
                    nRight = Max( nRight, 0L );
                }
                break;
            case HORI_RIGHT:
                {
                    const SwTwips nTmp = nMax-nLeftOffset-nRightOffset -
                                         (nWish + nRight);
                    nRight += nRightOffset;
                    nLeft  += nLeftOffset;
                    nLeft  -= nRightOffset;
                    if ( nTmp > nLeft )
                        nLeft = nTmp;
                    nLeft = Max( nLeft, 0L );
                }
                break;
            case HORI_CENTER:
                {
                    const SwTwips nTmp = (nMax-nLeftOffset-nRightOffset - nWish) / 2;
                    nLeft += nLeftOffset;
                    nRight+= nRightOffset;
                    if ( nTmp > nLeft )
                        nLeft = nTmp;
                    if ( nTmp > nRight )
                        nRight = nTmp;
                    nRight= Max( nRight, 0L );
                    nLeft = Max( nLeft, 0L );
                }
                break;
            case HORI_FULL:
                    //Das Teil dehnt sich ueber die gesamte Breite aus.
                    //Nur die fuer die Umrandung benoetigten Freiraeume
                    //werden beruecksichtigt.
                    //Die Attributwerte von LRSpace werden bewusst missachtet!
                    bCheckBrowseWidth = TRUE;
                    nLeft  += nLeftOffset;
                    nRight += nRightOffset;
                break;
            case HORI_NONE:
                {
                    //Die Raender werden vom Randattribut bestimmt.
                    nLeft = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        if( nLeft < 0 )
                            nLeft = 0;
                        nLeft += nLeftOffset;
                    }
                    nRight = pAttrs->CalcRight();
                    if( nRightOffset )
                    {
                        if( nRight < 0 )
                            nRight = 0;
                        nRight += nRightOffset;
                    }
                    if ( !pAttrs->GetLRSpace().GetRight() )
                        nRight = Max( nRight, nMax - (nWish + nLeft + nRight));
                }
                break;
            case HORI_LEFT_AND_WIDTH:
                    //Linker Rand und die Breite zaehlen (Word-Spezialitaet)
                    bCheckBrowseWidth = TRUE;
                    nLeft = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        if( nLeft < 0 )
                            nLeft = 0;
                        nLeft += nLeftOffset;
                    }
                    nRight = Max( nMax - nLeft - nWish, nTmpRight );
                break;
            default:
                ASSERT( FALSE, "Ungueltige orientation fuer Table." );
        }
        Prt().Top( nUpper );
        Prt().Height( aFrm.Height() - (nUpper + nLower) );
        if ( (nMax - MINLAY) < (nLeft + nRight) )
        {   //Wenn die Raender garnicht passen lasse ich sie lieber gleich weg.
            Prt().Left( 0 );
            Prt().Width( nMax );
        }
        else
        {
            Prt().Left( nLeft );
            Prt().Width( nMax - (nLeft + nRight) );
        }

        ViewShell *pSh;
        if ( bCheckBrowseWidth && GetFmt()->GetDoc()->IsBrowseMode() &&
             GetUpper()->IsPageBodyFrm() &&  // nur PageBodyFrms, nicht etwa ColBodyFrms
             0 != (pSh = GetShell()) && pSh->VisArea().Width() )
        {
            //Nicht ueber die Kante des sichbaren Bereiches hinausragen.
            //Die Seite kann breiter sein, weil es Objekte mit "ueberbreite"
            //geben kann (RootFrm::ImplCalcBrowseWidth())
            const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
            long nWidth = pSh->VisArea().Width() - 2 * aBorder.Width();
            nWidth -= Prt().Left();
            nWidth -= pAttrs->CalcRightLine();
            Prt().Width( Min( nWidth, Prt().Width() ) );
        }

        if ( nOldHeight != Prt().Height() )
            bValidSize = FALSE;
    }

    if ( !bValidSize )
    {
        bValidSize = TRUE;

        //Die Groesse wird durch den Inhalt plus den Raendern bestimmt.
        SwTwips nRemaining = 0, nDiff;
        SwFrm *pFrm = pLower;
        while ( pFrm )
        {   nRemaining += pFrm->Frm().Height();
            pFrm = pFrm->GetNext();
        }
        //Jetzt noch die Raender addieren
        nRemaining += nUpper + nLower;

        nDiff = Frm().Height() - nRemaining;
        if ( nDiff > 0 )
            Shrink( nDiff, pHeight );
        else if ( nDiff < 0 )
            Grow( -nDiff, pHeight );
    }
}
/*************************************************************************
|*
|*  SwTabFrm::GrowFrm()
|*
|*  Ersterstellung      MA 12. Mar. 93
|*  Letzte Aenderung    MA 23. Sep. 96
|*
|*************************************************************************/
SwTwips SwTabFrm::GrowFrm( SwTwips nDist, const SzPtr pDirection,
                           BOOL bTst, BOOL bInfo )
{
    //Tabellen sind immer in der Breite fix
    if( pDirection == pWidth )
        return 0;

    if ( Frm().SSize().Height() > 0 && nDist > (LONG_MAX - Frm().Height()) )
        nDist = LONG_MAX - Frm().Height();

    //Tabelle waechst immer (sie kann ja ggf. aufgespalten werden).
    if ( !bTst )
    {
        if ( GetUpper() )
        {
            //Der Upper wird nur soweit wie notwendig gegrowed. In nReal wird erstmal
            //die bereits zur Verfuegung stehende Strecke bereitgestellt.
            SwTwips nReal = GetUpper()->Prt().SSize().*pDirection;
            SwFrm *pFrm = GetUpper()->Lower();
            while ( pFrm )
            {   nReal -= pFrm->Frm().Height();
                pFrm = pFrm->GetNext();
            }

            Frm().SSize().Height() += nDist;

            if ( nReal < nDist )
                GetUpper()->Grow( nDist - (nReal > 0 ? nReal : 0),
                                                pDirection, bTst, bInfo );
        }
        else
        {
            ASSERT( !this, "Table without Upper" );
            Frm().SSize().Height() += nDist;
        }

        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            if ( GetNext()->IsCntntFrm() )
                GetNext()->InvalidatePage( pPage );
        }
        _InvalidateAll();
        InvalidatePage( pPage );
        SetComplete();

        const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            SetCompletePaint();
    }
    return nDist;
}
/*************************************************************************
|*
|*    SwTabFrm::Modify()
|*
|*    Ersterstellung    MA 14. Mar. 93
|*    Letzte Aenderung  MA 06. Dec. 96
|*
|*************************************************************************/
void SwTabFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BYTE nInvFlags = 0;
    BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();

    if( bAttrSetChg )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( TRUE )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrm *pPage = FindPageFrm();
        InvalidatePage( pPage );
//      if ( nInvFlags & 0x01 )
//          SetCompletePaint();
        if ( nInvFlags & 0x02 )
            _InvalidatePrt();
        if ( nInvFlags & 0x40 )
            _InvalidatePos();
        SwFrm *pTmp;
        if ( 0 != (pTmp = GetIndNext()) )
        {
            if ( nInvFlags & 0x04 )
            {
                pTmp->_InvalidatePrt();
                if ( pTmp->IsCntntFrm() )
                    pTmp->InvalidatePage( pPage );
            }
            if ( nInvFlags & 0x10 )
                pTmp->SetCompletePaint();
        }
        if ( nInvFlags & 0x08 && 0 != (pTmp = GetPrev()) )
        {
            pTmp->_InvalidatePrt();
            if ( pTmp->IsCntntFrm() )
                pTmp->InvalidatePage( pPage );
        }
        if ( nInvFlags & 0x20  )
        {
            if ( pPage && pPage->GetUpper() && !IsFollow() )
                ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
        }
        if ( nInvFlags & 0x80 )
            InvalidateNextPos();
    }
}

void SwTabFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
                            BYTE &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    BOOL bClear = TRUE;
    const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_TBLHEADLINECHG:
            //Es wird getoggelt.
            if ( IsFollow() )
            {
                if ( GetTable()->IsHeadlineRepeat() )
                {
                    bDontCreateObjects = TRUE;          //frmtool
                    SwFrm *pRow = new SwRowFrm( *GetTable()->GetTabLines()[0] );
                    bDontCreateObjects = FALSE;
                    pRow->Paste( this, Lower() );
                }
                else if ( Lower() )
                {
                    SwFrm *pLow = Lower();
                    pLow->Cut();
                    delete pLow;
                }
            }
            else if ( !HasFollow() )
                rInvFlags |= 0x02;
            break;

        case RES_FRM_SIZE:
        case RES_HORI_ORIENT:
            rInvFlags |= 0x22;
            break;

        case RES_PAGEDESC:                      //Attributaenderung (an/aus)
            if ( IsInDocBody() )
            {
                rInvFlags |= 0x40;
                SwPageFrm *pPage = FindPageFrm();
                if ( !GetPrev() )
                    CheckPageDescs( pPage );
                if ( pPage && GetFmt()->GetPageDesc().GetNumOffset() )
                    ((SwRootFrm*)pPage->GetUpper())->SetVirtPageNum( TRUE );
                SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
                GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
            }
            break;

        case RES_BREAK:
            rInvFlags |= 0xC0;
            break;

        case RES_LAYOUT_SPLIT:
            if ( !IsFollow() )
                rInvFlags |= 0x40;
            break;

        case RES_UL_SPACE:
            rInvFlags |= 0x1C;
            /* kein Break hier */

        default:
            bClear = FALSE;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

/*************************************************************************
|*
|*    SwTabFrm::GetInfo()
|*
|*    Ersterstellung    MA 06. Dec. 96
|*    Letzte Aenderung  MA 26. Jun. 98
|*
|*************************************************************************/
BOOL SwTabFrm::GetInfo( SfxPoolItem &rHnt ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() )
    {
        SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
        const SwPageFrm *pPage = FindPageFrm();
        if ( pPage  )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                //Das sollte er sein (kann allenfalls temporaer anders sein,
                //                    sollte uns das beunruhigen?)
                rInfo.SetInfo( pPage, this );
                return FALSE;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                //Das koennte er sein.
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return TRUE;
}

/*************************************************************************
|*
|*    SwTabFrm::FindLastCntnt()
|*
|*    Ersterstellung    MA 13. Apr. 93
|*    Letzte Aenderung  MA 15. May. 98
|*
|*************************************************************************/
SwCntntFrm *SwTabFrm::FindLastCntnt()
{
    SwFrm *pRet = pLower;
    while ( pRet && !pRet->IsCntntFrm() )
    {
        SwFrm *pOld = pRet;
        while ( pRet->GetNext() )
            pRet = pRet->GetNext();
        if ( pRet->GetLower() )
            pRet = pRet->GetLower();
        if ( pRet == pOld )
        {   // Wenn am Ende der letzten Zelle ein spaltiger Bereich steht,
            // der eine leere letzte Spalte hat, muessen wir noch die anderen
            // Spalten abklappern, dies erledigt SwSectionFrm::FindLastCntnt
            if( pRet->IsColBodyFrm() )
            {
#ifndef PRODUCT
                SwSectionFrm* pSect = pRet->FindSctFrm();
                ASSERT( pSect, "Wo kommt denn die Spalte her?")
                ASSERT( IsAnLower( pSect ), "Gespaltene Zelle?" );
#endif
                return pRet->FindSctFrm()->FindLastCntnt();
            }
            return 0;   //Hier geht es nicht weiter. Inkonsistenter Zustand
                        //der Tabelle (z.B. Undo TextToTable).
        }
    }
//  ASSERT( pRet && pRet->IsCntntFrm(), "Letzter Lower von Tab kein Cnt." );
    if ( pRet ) //#50235#
        while ( pRet->GetNext() )
            pRet = pRet->GetNext();
    if( pRet->IsSctFrm() )
        pRet = ((SwSectionFrm*)pRet)->FindLastCntnt();
    ASSERT( pRet && pRet->IsCntntFrm(), "Letzter Lower von Tab kein Cnt." );
    return (SwCntntFrm*)pRet;
}

/*************************************************************************
|*
|*  SwTabFrm::GetLeaf()
|*
|*  Ersterstellung      MA 19. Mar. 93
|*  Letzte Aenderung    MA 25. Apr. 95
|*
|*************************************************************************/
SwLayoutFrm *SwTabFrm::GetLeaf( MakePageType eMakePage, BOOL bFwd )
{
    SwLayoutFrm *pRet;
    if ( bFwd )
    {
        pRet = GetNextLeaf( eMakePage );
        while ( IsAnLower( pRet ) )
            pRet = pRet->GetNextLeaf( eMakePage );
    }
    else
        pRet = GetPrevLeaf();
    if ( pRet )
        pRet->Calc();
    return pRet;
}

/*************************************************************************
|*
|*  SwTabFrm::ShouldBwdMoved()
|*
|*  Beschreibung        Returnwert sagt ob der Frm verschoben werden sollte
|*  Ersterstellung      MA 10. Jul. 95
|*  Letzte Aenderung    MA 04. Mar. 97
|*
|*************************************************************************/
BOOL SwTabFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL bHead, BOOL &rReformat )
{
    rReformat = FALSE;
    if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()) )
    {
        //Das zurueckfliessen von Frm's ist leider etwas Zeitintensiv.
        //Der haufigste Fall ist der, dass dort wo der Frm hinfliessen
        //moechte die FixSize die gleiche ist, die der Frm selbst hat.
        //In diesem Fall kann einfach geprueft werden, ob der Frm genug
        //Platz fuer seine VarSize findet, ist dies nicht der Fall kann
        //gleich auf das Verschieben verzichtet werden.
        //Die Pruefung, ob der Frm genug Platz findet fuehrt er selbst
        //durch, dabei wird beruecksichtigt, dass er sich moeglicherweise
        //aufspalten kann.
        //Wenn jedoch die FixSize eine andere ist oder Flys im Spiel sind
        //(an der alten oder neuen Position) hat alle Prueferei keinen Sinn
        //der Frm muss dann halt Probehalber verschoben werden (Wenn ueberhaupt
        //etwas Platz zur Verfuegung steht).

        //Die FixSize der Umgebungen in denen Tabellen herumlungern ist immer
        //Die Breite.

        SwPageFrm *pOldPage = FindPageFrm(),
                  *pNewPage = pNewUpper->FindPageFrm();
        BOOL bMoveAnyway = FALSE;
        SwTwips nSpace = 0;

        if ( !SwFlowFrm::IsMoveBwdJump() &&
             Abs(pNewUpper->Prt().Width() - GetUpper()->Prt().Width()) < 2 )
        {
            if ( FALSE == (bMoveAnyway = BwdMoveNecessary( pOldPage, Frm() ) > 1) )
            {
                //Das Rechteck, in dem ich landen wuerde berechenen.
                SwRect aRect( pNewUpper->Prt() );
                aRect.Pos() += pNewUpper->Frm().Pos();
                const SwFrm *pPrevFrm = pNewUpper->Lower();
                while ( pPrevFrm )
                {
                    aRect.Top( pPrevFrm->Frm().Bottom() );
                    pPrevFrm = pPrevFrm->GetNext();
                }
                bMoveAnyway = BwdMoveNecessary( pNewPage, aRect) > 1;
                nSpace = aRect.Height();
                if ( GetFmt()->GetDoc()->IsBrowseMode() )
                    nSpace += pNewUpper->Grow( LONG_MAX, pHeight, TRUE );
            }
        }
        else if ( SwFlowFrm::IsMoveBwdJump() || !bLockBackMove )
            bMoveAnyway = TRUE;

        if ( bMoveAnyway )
            return rReformat = TRUE;
        else if ( !bLockBackMove )
        {   const BOOL bRepeat = GetTable()->IsHeadlineRepeat();
            SwTwips nHeight = bRepeat && Lower()->GetNext() ?
                    Lower()->GetNext()->Frm().Height() : Lower()->Frm().Height();
            if ( bHead && bRepeat && Lower()->GetNext() )
                nHeight += Lower()->Frm().Height();
            return nHeight <= nSpace;
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|*  SwTabFrm::Cut()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 09. Sep. 98
|*
|*************************************************************************/
void SwTabFrm::Cut()
{
    ASSERT( GetUpper(), "Cut ohne Upper()." );

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetNext();
    if( pFrm )
    {   //Der alte Nachfolger hat evtl. einen Abstand zum Vorgaenger
        //berechnet der ist jetzt wo er der erste wird obsolete
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        if ( pFrm->IsCntntFrm() )
            pFrm->InvalidatePage( pPage );
        if( IsInSct() && !GetPrev() )
        {
            SwSectionFrm* pSct = FindSctFrm();
            if( !pSct->IsFollow() )
            {
                pSct->_InvalidatePrt();
                pSct->InvalidatePage( pPage );
            }
        }
    }
    else
    {
        InvalidateNextPos();
        //Einer muss die Retusche uebernehmen: Vorgaenger oder Upper
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            pFrm->_InvalidatePos();
            if ( pFrm->IsCntntFrm() )
                pFrm->InvalidatePage( pPage );
        }
        //Wenn ich der einzige FlowFrm in meinem Upper bin (war), so muss
        //er die Retouche uebernehmen.
        //Ausserdem kann eine Leerseite entstanden sein.
        else
        {   SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
            if( IsInSct() )
            {
                SwSectionFrm* pSct = FindSctFrm();
                if( !pSct->IsFollow() )
                {
                    pSct->_InvalidatePrt();
                    pSct->InvalidatePage( pPage );
                }
            }
        }
    }

    //Erst removen, dann Upper Shrinken.
    SwLayoutFrm *pUp = GetUpper();
    Remove();
    if ( pUp )
    {
        ASSERT( !pUp->IsFtnFrm(), "Tabelle in Fussnote." );
        SwSectionFrm *pSct;
        if( !pUp->Lower() && pUp->IsInSct() &&
            !(pSct = pUp->FindSctFrm())->ContainsCntnt() )
        {
            if ( pUp->GetUpper() )
            {
                pSct->DelEmpty( FALSE );
                pSct->_InvalidateSize();
            }
        }
        else if ( Frm().Height() )
            pUp->Shrink( Frm().Height(), pHeight );
    }

    if ( pPage && !IsFollow() && pPage->GetUpper() )
        ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
}

/*************************************************************************
|*
|*  SwTabFrm::Paste()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 09. Sep. 98
|*
|*************************************************************************/
void SwTabFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    ASSERT( pParent, "Kein Parent fuer Paste." );
    ASSERT( pParent->IsLayoutFrm(), "Parent ist CntntFrm." );
    ASSERT( pParent != this, "Bin selbst der Parent." );
    ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
    ASSERT( !GetPrev() && !GetNext() && !GetUpper(),
            "Bin noch irgendwo angemeldet." );

    //In den Baum einhaengen.
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    _InvalidateAll();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );

    if ( GetNext() )
    {
        GetNext()->_InvalidatePos();
        GetNext()->_InvalidatePrt();
        if ( GetNext()->IsCntntFrm() )
            GetNext()->InvalidatePage( pPage );
    }

    if ( Frm().Height() )
        pParent->Grow( Frm().Height(), pHeight );

    if ( Frm().Width() != pParent->Prt().Width() )
        Prepare( PREP_FIXSIZE_CHG );

    if ( GetPrev() )
    {
        if ( !IsFollow() )
        {
            GetPrev()->InvalidateSize();
            if ( GetPrev()->IsCntntFrm() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
    else if ( GetNext() )
        //Bei CntntFrm's gilt es den Abstand zum Vorgaenger/Nachfolger
        //zu beachten. Faelle (beide treten immer gleichzeitig auf):
        //a) Der Cntnt wird der erste einer Kette
        //b) Der neue Nachfolger war vorher der erste einer Kette
        GetNext()->_InvalidatePrt();

    if ( pPage && !IsFollow() )
    {
        if ( pPage->GetUpper() )
            ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();

        if ( !GetPrev() )//Mindestens fuer HTML mit Tabelle am Anfang notwendig.
        {
            const SwPageDesc *pDesc = GetFmt()->GetPageDesc().GetPageDesc();
            if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
                 (!pDesc && pPage->GetPageDesc() != &GetFmt()->GetDoc()->GetPageDesc(0)) )
                CheckPageDescs( pPage, TRUE );
        }
    }
}

/*************************************************************************
|*
|*  SwRowFrm::SwRowFrm(), ~SwRowFrm()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
SwRowFrm::SwRowFrm( const SwTableLine &rLine ):
    SwLayoutFrm( rLine.GetFrmFmt() ),
    pTabLine( &rLine )
{
    nType = FRM_ROW;

    //Gleich die Boxen erzeugen und einfuegen.
    const SwTableBoxes &rBoxes = rLine.GetTabBoxes();
    SwFrm *pPrev = 0;
    for ( USHORT i = 0; i < rBoxes.Count(); ++i )
    {
        SwCellFrm *pNew = new SwCellFrm( *rBoxes[i] );
        pNew->InsertBehind( this, pPrev );
        pNew->bVarHeight = FALSE;
        pPrev = pNew;
    }
}

SwRowFrm::~SwRowFrm()
{
    SwModify* pMod = GetFmt();
    if( pMod )
    {
        pMod->Remove( this );           // austragen,
        if( !pMod->GetDepends() )
            delete pMod;                // und loeschen
    }
}

/*************************************************************************
|*
|*  SwRowFrm::RegistFlys()
|*
|*  Ersterstellung      MA 08. Jul. 93
|*  Letzte Aenderung    MA 08. Jul. 93
|*
|*************************************************************************/
void SwRowFrm::RegistFlys( SwPageFrm *pPage )
{
    ::RegistFlys( pPage ? pPage : FindPageFrm(), this );
}

/*************************************************************************
|*
|*    SwRowFrm::Modify()
|*
|*    Ersterstellung    MA 12. Nov. 97
|*    Letzte Aenderung  MA 12. Nov. 97
|*
|*************************************************************************/
void SwRowFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

    if( bAttrSetChg )
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_FRM_SIZE, FALSE, &pItem);
    else if ( RES_FRM_SIZE == pNew->Which() )
        pItem = pNew;

    if ( pItem )
    {
        SwTabFrm *pTab = FindTabFrm();
        if ( pTab && pTab->IsFollow() &&
             (!GetPrev() ||
              (pTab->GetTable()->IsHeadlineRepeat() && !GetPrev()->GetPrev())))
        {
            pTab->FindMaster()->InvalidatePos();
        }
    }

    SwLayoutFrm::Modify( pOld, pNew );
}



/*************************************************************************
|*
|*  SwRowFrm::MakeAll()
|*
|*  Ersterstellung      MA 01. Mar. 94
|*  Letzte Aenderung    MA 01. Mar. 94
|*
|*************************************************************************/
void SwRowFrm::MakeAll()
{
    if ( !GetNext() )
        bValidSize = FALSE;
    SwLayoutFrm::MakeAll();
}

/*************************************************************************
|*
|*  SwRowFrm::Format()
|*
|*  Ersterstellung      MA 13. Mar. 93
|*  Letzte Aenderung    MA 20. Jun. 96
|*
|*************************************************************************/
long MA_FASTCALL CalcHeightWidthFlys( const SwFrm *pFrm )
{
    long nHeight = 0;
    const SwFrm* pTmp = pFrm->IsSctFrm() ?
            ((SwSectionFrm*)pFrm)->ContainsCntnt() : pFrm;
    while( pTmp )
    {
        if ( pTmp->GetDrawObjs() )
        {
            for ( USHORT i = 0; i < pTmp->GetDrawObjs()->Count(); ++i )
            {
                const SdrObject *pO = (*pTmp->GetDrawObjs())[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                    if ( !pFly->IsFlyInCntFrm() && pFly->Frm().Top()!=WEIT_WECH )
//                       pFrm->GetValidPosFlag() && pFrm->GetValidSizeFlag() )
                    {
                        const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
                        if( !rSz.GetHeightPercent() )
                            nHeight = Max( nHeight, pFly->Frm().Height() +
                                pFly->GetCurRelPos().Y() + pTmp->Frm().Top()
                                - pFrm->Frm().Top() - pFrm->Frm().Height() );
                    }
                }
            }
        }
        if( !pFrm->IsSctFrm() )
            break;
        pTmp = pTmp->FindNextCnt();
        if( !((SwSectionFrm*)pFrm)->IsAnLower( pTmp ) )
            break;
    }
    return nHeight;
}

SwTwips MA_FASTCALL lcl_CalcMinRowHeight( SwLayoutFrm *pRow );

SwTwips MA_FASTCALL lcl_CalcMinCellHeight( SwLayoutFrm *pCell,
                                  const SwBorderAttrs *pAttrs = 0 )
{
    SwTwips nHeight = 0;
    SwFrm *pLow = pCell->Lower();
    if ( pLow )
    {
        long nFlyAdd = 0;
        while ( pLow )
        {
            if( pLow->IsCntntFrm() || pLow->IsSctFrm() )
            {
                nHeight += pLow->Frm().Height();
                nFlyAdd = Max( 0L, nFlyAdd - pLow->Frm().Height() );
                nFlyAdd = Max( nFlyAdd, ::CalcHeightWidthFlys( pLow ) );
            }
            else
                nHeight += ::lcl_CalcMinRowHeight( (SwLayoutFrm*)pLow );

            pLow = pLow->GetNext();
        }
        if ( nFlyAdd )
            nHeight += nFlyAdd;
    }
    //Der Border will natuerlich auch mitspielen, er kann leider nicht
    //aus PrtArea und Frm errechnet werden, da diese in beliebiger
    //Kombination ungueltig sein koennen.
    if ( pAttrs )
        nHeight += pAttrs->CalcTop() + pAttrs->CalcBottom();
    else
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        nHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();
    }
    return nHeight;
}

SwTwips MA_FASTCALL lcl_CalcMinRowHeight( SwLayoutFrm *pRow )
{
    if ( pRow->HasFixSize( pHeight ) )
        return pRow->Frm().Height();

    SwTwips nHeight = 0;
    SwLayoutFrm *pLow = (SwLayoutFrm*)pRow->Lower();
    while ( pLow )
    {
        SwTwips nTmp = ::lcl_CalcMinCellHeight( pLow );
        if ( nTmp > nHeight )
            nHeight = nTmp;
        pLow = (SwLayoutFrm*)pLow->GetNext();
    }
    const SwFmtFrmSize &rSz = pRow->GetFmt()->GetFrmSize();
    if ( rSz.GetSizeType() == ATT_MIN_SIZE )
        nHeight = Max( nHeight, rSz.GetHeight() );
    return nHeight;
}

void SwRowFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "SwRowFrm::Format ohne Attrs." );

    const BOOL bFix = bFixHeight;

    if ( !bValidPrtArea )
    {
        //RowFrms haben keine Umrandung usw. also entspricht die PrtArea immer
        //dem Frm.
        bValidPrtArea = TRUE;
        aPrt.Left( 0 );
        aPrt.Top( 0 );
        aPrt.Width ( aFrm.Width() );
        aPrt.Height( aFrm.Height() );
    }

    while ( !bValidSize )
    {
        bValidSize = TRUE;

#ifndef PRODUCT
        if ( HasFixSize( pHeight ) )
        {
            const SwFmtFrmSize &rFrmSize = GetFmt()->GetFrmSize();
            ASSERT( rFrmSize.GetSize().Height() > 0, "Hat ihn" );
        }
#endif
        const SwTwips nDiff = Frm().Height() - (HasFixSize( pHeight ) ?
                                                    pAttrs->GetSize().Height() :
                                                    ::lcl_CalcMinRowHeight( this ));
        if ( nDiff )
        {
            bFixHeight = FALSE;
            if ( nDiff > 0 )
                Shrink( nDiff, pHeight, FALSE, TRUE );
            else if ( nDiff < 0 )
                Grow( -nDiff, pHeight );
            bFixHeight = bFix;
        }
    }
    if ( !GetNext() )
    {
        //Der letzte fuellt den verbleibenden Raum im Upper aus.
        SwTwips nDiff = GetUpper()->Prt().Height();
        SwFrm *pSibling = GetUpper()->Lower();
        do
        {   nDiff -= pSibling->Frm().Height();
            pSibling = pSibling->GetNext();
        } while ( pSibling );
        if ( nDiff > 0 )
        {
            bFixHeight = FALSE;
            Grow( nDiff, pHeight );
            bFixHeight = bFix;
            bValidSize = TRUE;
        }
    }
}

/*************************************************************************
|*
|*  SwRowFrm::AdjustCells()
|*
|*  Ersterstellung      MA 10. Aug. 93
|*  Letzte Aenderung    MA 16. Dec. 96
|*
|*************************************************************************/
void SwRowFrm::AdjustCells( const SwTwips nHeight, const BOOL bHeight )
{
    SwFrm *pFrm = Lower();
    if ( bHeight )
    {
        while ( pFrm )
        {   if ( pFrm->Frm().Height() != nHeight )
            {
                pFrm->Frm().Height( nHeight );
                pFrm->_InvalidatePrt();
            }
            pFrm = pFrm->GetNext();
        }
    }
    else
    {   while ( pFrm )
        {
            pFrm->_InvalidateAll();
            pFrm = pFrm->GetNext();
        }
    }
    InvalidatePage();
}

/*************************************************************************
|*
|*  SwRowFrm::Cut()
|*
|*  Ersterstellung      MA 12. Nov. 97
|*  Letzte Aenderung    MA 12. Nov. 97
|*
|*************************************************************************/
void SwRowFrm::Cut()
{
    SwTabFrm *pTab = FindTabFrm();
    if ( pTab && pTab->IsFollow() &&
         (!GetPrev() ||
          (pTab->GetTable()->IsHeadlineRepeat() && !GetPrev()->GetPrev())))
    {
        pTab->FindMaster()->InvalidatePos();
    }
    SwLayoutFrm::Cut();
}

/*************************************************************************
|*
|*  SwRowFrm::GrowFrm()
|*
|*  Ersterstellung      MA 15. Mar. 93
|*  Letzte Aenderung    MA 05. May. 94
|*
|*************************************************************************/
SwTwips SwRowFrm::GrowFrm( SwTwips nDist, const SzPtr pDirection,
                           BOOL bTst, BOOL bInfo )
{
    const SwTwips nReal = SwLayoutFrm::GrowFrm( nDist, pDirection, bTst, bInfo);

    //Hoehe der Zellen auf den neuesten Stand bringen.
    if ( !bTst )
    {
        AdjustCells( Prt().Height() + nReal,
                                      pDirection == pHeight ? TRUE : FALSE );
        if ( nReal )
            SetCompletePaint();
    }
    return nReal;
}
/*************************************************************************
|*
|*  SwRowFrm::ShrinkFrm()
|*
|*  Ersterstellung      MA 15. Mar. 93
|*  Letzte Aenderung    MA 20. Jun. 96
|*
|*************************************************************************/
SwTwips SwRowFrm::ShrinkFrm( SwTwips nDist, const SzPtr pDirection,
                             BOOL bTst, BOOL bInfo )
{
    if ( HasFixSize( pDirection ) )
    {
        AdjustCells( Prt().Height(), pDirection == pHeight ? TRUE : FALSE);
        return 0L;
    }

    //bInfo wird ggf. vom SwRowFrm::Format auf TRUE gesetzt, hier muss dann
    //entsprechend reagiert werden
    const BOOL bShrinkAnyway = bInfo;

    //Nur soweit Shrinken, wie es der Inhalt der groessten Zelle zulaesst.
    SwTwips nRealDist = nDist;
    if ( pDirection == pHeight )
    {
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        SwTwips nMinHeight = rSz.GetSizeType() == ATT_MIN_SIZE ? rSz.GetHeight() : 0;
        SwLayoutFrm *pCell = (SwLayoutFrm*)Lower();
        if ( nMinHeight < Frm().Height() )
        {
            SwLayoutFrm *pCell = (SwLayoutFrm*)Lower();
            while ( pCell )
            {
                SwTwips nAct = ::lcl_CalcMinCellHeight( pCell );
                if ( nAct > nMinHeight )
                    nMinHeight = nAct;
                if ( nMinHeight >= Frm().Height() )
                    break;
                pCell = (SwLayoutFrm*)pCell->GetNext();
            }
        }
        if ( (Frm().Height() - nRealDist) < nMinHeight )
            nRealDist = Frm().Height() - nMinHeight;
    }
    if ( nRealDist < 0 )
        nRealDist = 0;

    SwTwips nReal = nRealDist;
    if ( nReal )
    {
        if ( !bTst )
            Frm().SSize().*pDirection -= nReal;

        SwTwips nTmp = GetUpper()->Shrink( nReal, pDirection, bTst );
        if ( !bShrinkAnyway && !GetNext() && nTmp != nReal )
        {
            //Der letzte bekommt den Rest im Upper und nimmt deshalb
            //ggf. Ruecksichten (sonst: Endlosschleife)
            if ( !bTst )
                Frm().SSize().*pDirection += nReal - nTmp;
            nReal = nTmp;
        }
    }

    //Geeignet invalidieren und die Hoehe der Zellen auf den neuesten
    //Stand bringen.
    if ( !bTst )
    {
        if ( nReal )
        {
            if ( GetNext() )
                GetNext()->_InvalidatePos();
            _InvalidateAll();
            SetCompletePaint();

            SwTabFrm *pTab = FindTabFrm();
            if ( pTab->IsFollow() &&
                 (!GetPrev() ||
                  (pTab->GetTable()->IsHeadlineRepeat() && !GetPrev()->GetPrev())))
            {
                pTab->FindMaster()->InvalidatePos();
            }
        }
        AdjustCells( Prt().Height() - nReal,
                                         pDirection == pHeight ? TRUE : FALSE );
    }
    return nReal;
}

/*************************************************************************
|*
|*  SwCellFrm::SwCellFrm(), ~SwCellFrm()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
SwCellFrm::SwCellFrm( const SwTableBox &rBox ) :
    SwLayoutFrm( rBox.GetFrmFmt() ),
    pTabBox( &rBox )
{
    nType = FRM_CELL;

    //Wenn ein StartIdx vorhanden ist, so werden CntntFrms in der Zelle
    //angelegt, andernfalls muessen Rows vorhanden sein und diese werden
    //angelegt.
    if ( rBox.GetSttIdx() )
    {
        ULONG nIndex = rBox.GetSttIdx();
        ::_InsertCnt( this, rBox.GetFrmFmt()->GetDoc(), ++nIndex );
    }
    else
    {   const SwTableLines &rLines = rBox.GetTabLines();
        SwFrm *pPrev = 0;
        for ( USHORT i = 0; i < rLines.Count(); ++i )
        {
            SwRowFrm *pNew = new SwRowFrm( *rLines[i] );
            pNew->InsertBehind( this, pPrev );
            pPrev = pNew;
        }
    }
}

SwCellFrm::~SwCellFrm()
{
    SwModify* pMod = GetFmt();
    if( pMod )
    {
        pMod->Remove( this );           // austragen,
        if( !pMod->GetDepends() )
            delete pMod;                // und loeschen
    }
}

/*************************************************************************
|*
|*  SwCellFrm::Format()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 29. Jan. 98
|*
|*************************************************************************/
BOOL lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, BOOL bInva )
{
    BOOL bRet = FALSE;
    SwFrm *pFrm = pLay->Lower();
    while ( pFrm )
    {
        if ( pFrm->Frm().Top() != lYStart )
        {
            bRet = TRUE;
            const long lDiff = lYStart - pFrm->Frm().Top();
            pFrm->Frm().Pos().Y() = lYStart;
            pFrm->SetCompletePaint();
            if ( !pFrm->GetNext() )
                pFrm->SetRetouche();
            if( bInva )
                pFrm->Prepare( PREP_POS_CHGD );
            if ( pFrm->IsLayoutFrm() && ((SwLayoutFrm*)pFrm)->Lower() )
                lcl_ArrangeLowers( (SwLayoutFrm*)pFrm,
                    ((SwLayoutFrm*)pFrm)->Lower()->Frm().Top()+lDiff, bInva );
            if ( pFrm->GetDrawObjs() )
            {
                for ( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
                {
                    SdrObject *pO = (*pFrm->GetDrawObjs())[i];
                    if ( pO->IsWriterFlyFrame() )
                    {
                        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                        if ( WEIT_WECH != pFly->Frm().Top() )
                            pFly->Frm().Pos().Y() += lDiff;
                        if ( pFly->IsFlyInCntFrm() )
                            ((SwFlyInCntFrm*)pFly)->AddRefOfst( lDiff );
                        else if( pFly->IsAutoPos() )
                            ((SwFlyAtCntFrm*)pFly)->AddLastCharY( lDiff );
                        if ( ::lcl_ArrangeLowers( pFly, pFly->Frm().Top() +
                                pFly->Prt().Top(), bInva ) )
                            pFly->SetCompletePaint();
                    }
                    else
                        pO->SetAnchorPos( pFrm->Frm().Pos() );
                }
            }
        }
        if( !pFrm->IsColumnFrm() ) // Spalten in Bereichen sind nebeneinander,
            lYStart += pFrm->Frm().Height();  // nicht untereinander!
        pFrm = pFrm->GetNext();
    }
    return bRet;
}

void SwCellFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "CellFrm::Format, pAttrs ist 0." );

    if ( !bValidPrtArea )
    {
        bValidPrtArea = TRUE;

        //Position einstellen.
        aPrt.Left( pAttrs->CalcLeft( this ) );
        aPrt.Top(  pAttrs->CalcTop() );

        //Sizes einstellen; die Groesse gibt der umgebende Frm vor, die
        //die Raender werden einfach abgezogen.
        aPrt.Width ( aFrm.Width() - (aPrt.Left() + pAttrs->CalcRight()) );
        aPrt.Height( aFrm.Height()- (aPrt.Top()  + pAttrs->CalcBottom()));
    }

    long nRemaining = ::lcl_CalcMinCellHeight( this, pAttrs );
    if ( !bValidSize )
    {
        bValidSize = TRUE;

        //Die VarSize der CellFrms ist immer die Breite.
        //Tatsaechlich ist die Breite jedoch nicht Variabel, sie wird durch das
        //Format vorgegeben. Dieser Vorgegebene Wert muss aber nun wiederum
        //nicht der tatsaechlichen Breite entsprechen. Die Breite wird auf
        //Basis des Attributes errechnet, der Wert im Attribut passt zu dem
        //gewuenschten Wert des TabFrms. Anpassungen die dort vorgenommen
        //wurden werden hier Proportional beruecksichtigt.
        //Wenn die Celle keinen Nachbarn mehr hat beruecksichtigt sie nicht
        //die Attribute, sonder greift sich einfach den Rest des
        //Uppers.
        SwTwips nWidth;
        if ( GetNext() )
        {
            const SwTabFrm *pTab = FindTabFrm();
            SwTwips nWish = pTab->GetFmt()->GetFrmSize().GetWidth();
            nWidth = pAttrs->GetSize().Width();

            ASSERT( nWish, "Tabelle ohne Breite?" );
            ASSERT( nWidth <= nWish, "Zelle breiter als Tabelle." );
            ASSERT( nWidth > 0, "Box without width" );

            if ( nWish != pTab->Prt().Width() )
            {
                nWidth *= pTab->Prt().Width();
                nWidth /= nWish;
            }
        }
        else
        {
            ASSERT( pAttrs->GetSize().Width() > 0, "Box without width" );
            nWidth = GetUpper()->Prt().Width();
            SwFrm *pPre = GetUpper()->Lower();
            while ( pPre != this )
            {   nWidth -= pPre->Frm().Width();
                pPre = pPre->GetNext();
            }
        }
        const long nDiff = nWidth - Frm().Width();
        Frm().Width( nWidth );
        Prt().Width( Prt().Width() + nDiff );

        //Jetzt die Hoehe einstellen, sie wird vom Inhalt und den Raendern
        //bestimmt.
        const long nDiffHeight = nRemaining - Frm().Height();
        if ( nDiffHeight )
        {
            if ( nDiffHeight > 0 )
            {
                //Wieder validieren wenn kein Wachstum stattgefunden hat.
                //Invalidiert wird durch AdjustCells von der Row.
                if ( !Grow( nDiffHeight, pHeight ) )
                    bValidSize = bValidPrtArea = TRUE;
            }
            else
            {
                //Nur dann invalidiert lassen, wenn tatsaechlich
                //geshrinkt wurde; das kann abgelehnt werden, weil alle
                //nebeneinanderliegenden Zellen gleichgross sein muessen.
                if ( !Shrink( -nDiffHeight, pHeight ) )
                    bValidSize = bValidPrtArea = TRUE;
            }
        }
    }
    const SwFmtVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();
    if ( VERT_NONE != rOri.GetVertOrient() )
    {
        if ( !Lower()->IsCntntFrm() && !Lower()->IsSctFrm() )
        {
            //ASSERT fuer HTML-Import!
            ASSERT( !this, "VAlign an Zelle ohne Inhalt" );
            return;
        }
        BOOL bVertDir = TRUE;
        //Keine Ausrichtung wenn Rahmen mit Umlauf in die Zelle ragen.
        SwPageFrm *pPg = FindPageFrm();
        if ( pPg->GetSortedObjs() )
        {
            SwRect aRect( Prt() ); aRect += Frm().Pos();
            for ( USHORT i = 0; i < pPg->GetSortedObjs()->Count(); ++i )
            {
                const SdrObject *pObj = (*pPg->GetSortedObjs())[i];
                SwRect aTmp( pObj->GetBoundRect() );
                if ( aTmp.IsOver( aRect ) )
                {
                    SdrObjUserCall *pUserCall;
                    const SwFmtSurround &rSur = ((SwContact*)
                               (pUserCall=GetUserCall(pObj)))->GetFmt()->GetSurround();
                    if ( SURROUND_THROUGHT != rSur.GetSurround() )
                    {
                        const SwFrm *pAnch;
                        if ( pObj->IsWriterFlyFrame() )
                        {
                            const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                            if ( pFly->IsAnLower( this ) )
                                continue;
                            pAnch = pFly->GetAnchor();
                        }
                        else
                            pAnch = ((SwDrawContact*)pUserCall)->GetAnchor();
                        if ( !IsAnLower( pAnch ) )
                        {
                            bVertDir = FALSE;
                            break;
                        }
                    }
                }
            }
        }
        if ( (bVertDir &&
                (nRemaining -= (pAttrs->CalcTop() + pAttrs->CalcBottom())) < Prt().Height()) ||
                Lower()->Frm().Top() > Frm().Top()+Prt().Top() )
        {
            long lTopOfst = 0,
                    nDiff = Prt().Height() - nRemaining;
            if ( nDiff >= 0 )
            {
                if ( bVertDir )
                {
                    switch ( rOri.GetVertOrient() )
                    {
                        case VERT_CENTER:   lTopOfst = nDiff / 2; break;
                        case VERT_BOTTOM:   lTopOfst = nDiff;     break;
                    };
                }
                if ( lcl_ArrangeLowers( this, Frm().Top()+Prt().Top()+lTopOfst,
                      !bVertDir ) )
                    SetCompletePaint();
            }
        }
    }
    else
    {
        //Ist noch eine alte Ausrichtung beruecksichtigt worden?
        if ( Lower()->IsCntntFrm() )
        {
            const long lYStart = Frm().Top()+Prt().Top();
            lcl_ArrangeLowers( this, lYStart, TRUE );
        }
    }
}

/*************************************************************************
|*
|*    SwCellFrm::Modify()
|*
|*    Ersterstellung    MA 20. Dec. 96
|*    Letzte Aenderung  MA 20. Dec. 96
|*
|*************************************************************************/
void SwCellFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

    if( bAttrSetChg )
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_VERT_ORIENT, FALSE, &pItem);
    else if ( RES_VERT_ORIENT == pNew->Which() )
        pItem = pNew;

    if ( pItem )
    {
        BOOL bInva = TRUE;
        if ( VERT_NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient() &&
             Lower()->IsCntntFrm() )
        {
            const long lYStart = Frm().Top()+Prt().Top();
            bInva = lcl_ArrangeLowers( this, lYStart, FALSE );
        }
        if ( bInva )
        {
            SetCompletePaint();
            InvalidatePrt();
        }
    }

    SwLayoutFrm::Modify( pOld, pNew );
}

