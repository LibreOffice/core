/*************************************************************************
 *
 *  $RCSfile: findtxt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-22 14:11:32 $
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

#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif

#ifndef _FLDBAS_HXX //autogen
#include <fldbas.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif

String& lcl_CleanStr( const SwTxtNode& rNd, xub_StrLen nStart,
                        xub_StrLen& rEnde, SvULongs& rArr, String& rRet )
{
    rRet = rNd.GetTxt();
    if( rArr.Count() )
        rArr.Remove( 0, rArr.Count() );

    const SwpHints *pHts = rNd.GetpSwpHints();
    if( pHts )
    {
        SvULongs aReplaced;

        for( USHORT n = 0; n < pHts->Count(); ++n )
        {
            const SwTxtAttr *pHt = (*pHts)[n];

            if( pHt->GetEnd() )         // nur Attribute ohne Ende
                continue;

            register xub_StrLen nStt = *pHt->GetStart();
            if(  nStt < nStart )
                continue;

            const xub_StrLen nAkt = nStt - rArr.Count();

            //JP 17.05.00: Task 75806 ask for ">=" and not for ">"
            if( nAkt >= rEnde )         // uebers Ende hinaus =
                break;                  // das wars

            switch( pHt->Which() )
            {
            case RES_TXTATR_FLYCNT:
            case RES_TXTATR_FTN:
            case RES_TXTATR_FIELD:
            case RES_TXTATR_REFMARK:
            case RES_TXTATR_TOXMARK:
                {
// JP 06.05.98: mit Bug 50100 werden sie als Trenner erwuenscht und nicht
//              mehr zum Wort dazu gehoerend.
// MA 23.06.98: mit Bug 51215 sollen sie konsequenterweise auch am
//              Satzanfang und -ende ignoriert werden wenn sie Leer sind.
//              Dazu werden sie schlicht entfernt. Fuer den Anfang entfernen
//              wir sie einfach.
//              Fuer das Ende merken wir uns die Ersetzungen und entferenen
//              hinterher alle am Stringende (koenten ja 'normale' 0x7f drinstehen
                    BOOL bEmpty = RES_TXTATR_FIELD != pHt->Which() ||
                        !((SwTxtFld*)pHt)->GetFld().GetFld()->Expand().Len();
                    if ( bEmpty && nStart == nAkt )
                    {
                        rArr.Insert( nAkt, rArr.Count() );
                        --rEnde;
                        rRet.Erase( nAkt, 1 );
                    }
                    else
                    {
                        if ( bEmpty )
                            aReplaced.Insert( nAkt, aReplaced.Count() );
                        rRet.SetChar( nAkt, '\x7f' );
                    }
                }
                break;

                case RES_TXTATR_HARDBLANK:
                    rRet.SetChar( nAkt, ((SwTxtHardBlank*)pHt)->GetChar() );
                    break;
                default:
                    {
                        rArr.Insert( nAkt, rArr.Count() );
                        --rEnde;
                        rRet.Erase( nAkt, 1 );
                    }
                    break;
            }
        }
        for( USHORT i = aReplaced.Count(); i; )
        {
            const xub_StrLen nTmp = aReplaced[ --i ];
            if( nTmp == rRet.Len()-1 )
            {
                rRet.Erase( nTmp );
                rArr.Insert( nTmp, rArr.Count() );
                --rEnde;
            }
        }
    }

    return rRet;
}



BYTE SwPaM::Find( const utl::SearchParam& rParam, utl::TextSearch& rSTxt,
                    SwMoveFn fnMove, const SwPaM * pRegion,
                    FASTBOOL bInReadOnly )
{
    if( !rParam.GetSrchStr().Len() )
        return FALSE;

    SwPaM* pPam = MakeRegion( fnMove, pRegion );
    FASTBOOL bSrchForward = fnMove == fnMoveForward;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    SwIndex& rCntntIdx = pPam->GetPoint()->nContent;

    // Wenn am Anfang/Ende, aus dem Node moven
    // beim leeren Node nicht weiter
    if( bSrchForward
        ? ( rCntntIdx.GetIndex() == pPam->GetCntntNode()->Len() &&
            rCntntIdx.GetIndex() )
        : !rCntntIdx.GetIndex() && pPam->GetCntntNode()->Len() )
    {
        if( !(*fnMove->fnNds)( &rNdIdx, FALSE ))
        {
            delete pPam;
            return FALSE;
        }
        SwCntntNode *pNd = rNdIdx.GetNode().GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        rCntntIdx.Assign( pNd, nTmpPos );
    }

    /*
     * Ist bFound == TRUE, dann wurde der String gefunden und in
     * nStart und nEnde steht der gefundenen String
     */
    BOOL bFound = FALSE;
    /*
     * StartPostion im Text oder Anfangsposition
     */
    FASTBOOL bFirst = TRUE;
    SwCntntNode * pNode;
    String sCleanStr;
    SvULongs aFltArr;

    xub_StrLen nStart, nEnde, nTxtLen;
    const SwNode* pSttNd = &rNdIdx.GetNode();
    xub_StrLen nSttCnt = rCntntIdx.GetIndex();

    while( 0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ))
    {
        if( pNode->IsTxtNode() )
        {
            nTxtLen = ((SwTxtNode*)pNode)->GetTxt().Len();
            if( rNdIdx == pPam->GetMark()->nNode )
                nEnde = pPam->GetMark()->nContent.GetIndex();
            else
                nEnde = bSrchForward ? nTxtLen : 0;
            nStart = rCntntIdx.GetIndex();

            if( bSrchForward )
                lcl_CleanStr( *(SwTxtNode*)pNode, nStart, nEnde,
                                aFltArr, sCleanStr );
            else
                lcl_CleanStr( *(SwTxtNode*)pNode, nEnde, nStart,
                                aFltArr, sCleanStr );

            if( (rSTxt.*fnMove->fnSearch)( sCleanStr, &nStart, &nEnde, 0 ))
            {
                // setze den Bereich richtig
                *GetPoint() = *pPam->GetPoint();
                SetMark();
                // Start und Ende wieder korrigieren !!
                if( aFltArr.Count() )
                {
                    xub_StrLen n, nNew;
                    // bei Rueckwaertssuche die Positionen temp. vertauschen
                    if( !bSrchForward ) { n = nStart; nStart = nEnde; nEnde = n; }

                    for( n = 0, nNew = nStart;
                        n < aFltArr.Count() && aFltArr[ n ] <= nStart;
                        ++n, ++nNew )
                        ;
                    nStart = nNew;
                    for( n = 0, nNew = nEnde;
                        n < aFltArr.Count() && aFltArr[ n ] <= nEnde;
                        ++n, ++nNew )
                        ;
                    nEnde = nNew;

                    // bei Rueckwaertssuche die Positionen temp. vertauschen
                    if( !bSrchForward ) { n = nStart; nStart = nEnde; nEnde = n; }
                }
                GetMark()->nContent = nStart;       // Startposition setzen

                // kein Bereich selektiert und am Anfng/Ende ? ueber den
                // Absatz selektieren
                if( (!nStart && !((USHORT)(nEnde+1))) ||    // fuer 0 und -1 !
                    ( nStart > nEnde ))
                {
                    // nicht von der Start Position entfernt
                    if( pSttNd == &rNdIdx.GetNode() && nSttCnt == nStart
                        && ( !bSrchForward || nStart || nTxtLen ))
                        continue;

                    GetPoint()->nContent = nStart;
                    if( ( !nStart || nStart == nTxtLen ) &&
                         !Move( fnMoveForward, fnGoCntnt ) )
                        continue;
                }
                else
                {
                    GetPoint()->nContent = nEnde;
                    if( !Move( fnMoveForward, fnGoCntnt ) )
                        GetPoint()->nContent = nTxtLen;
                }
                if( utl::SearchParam::SRCH_REGEXP == rParam.GetSrchType() &&
                    1 < Abs( (int)(GetPoint()->nNode.GetIndex() - GetMark()->nNode.GetIndex())))
                    // Fehler: es koennen maximal 2 Nodes selektiert werden !!
                    continue;

                if( !bSrchForward )         // rueckwaerts Suche?
                    Exchange();             // Point und Mark tauschen
                bFound = TRUE;
                break;
            }
        }
    }
    delete pPam;
    return bFound;
}


// Parameter fuers Suchen und Ersetzen von Text
struct SwFindParaText : public SwFindParas
{
    const utl::SearchParam& rParam;
    SwCursor& rCursor;
    utl::TextSearch aSTxt;
    BOOL bReplace;

    SwFindParaText( const utl::SearchParam& rPara, int bRepl, SwCursor& rCrsr )
        : rCursor( rCrsr ), bReplace( bRepl ), rParam( rPara ),
        aSTxt( rPara, LANGUAGE_SYSTEM )
    {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, FASTBOOL bInReadOnly );
    virtual int IsReplaceMode() const;
};


int SwFindParaText::Find( SwPaM* pCrsr, SwMoveFn fnMove,
                            const SwPaM* pRegion, FASTBOOL bInReadOnly )
{
    if( bInReadOnly && bReplace )
        bInReadOnly = FALSE;

    BOOL bFnd = (BOOL)pCrsr->Find( rParam, aSTxt, fnMove, pRegion, bInReadOnly );
    // kein Bereich ??
    if( bFnd && *pCrsr->GetMark() == *pCrsr->GetPoint() )
        return FIND_NOT_FOUND;

    if( bFnd && bReplace )          // String ersetzen ??
    {
        // Replace-Methode vom SwDoc benutzen
        int bRegExp = utl::SearchParam::SRCH_REGEXP == rParam.GetSrchType();
        SwIndex& rSttCntIdx = pCrsr->Start()->nContent;
        xub_StrLen nSttCnt = rSttCntIdx.GetIndex();
        // damit die Region auch verschoben wird, in den Shell-Cursr-Ring
        // mit aufnehmen !!
        Ring *pPrev;
        if( bRegExp )
        {
            pPrev = pRegion->GetPrev();
            ((Ring*)pRegion)->MoveRingTo( &rCursor );
        }

        rCursor.GetDoc()->Replace( *pCrsr, rParam.GetReplaceStr(), bRegExp );
        rCursor.SaveTblBoxCntnt( pCrsr->GetPoint() );

        if( bRegExp )
        {
            // und die Region wieder herausnehmen:
            Ring *p, *pNext = (Ring*)pRegion;
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo( (Ring*)pRegion );
            } while( p != pPrev );
        }
        rSttCntIdx = nSttCnt;
        return FIND_NO_RING;
    }
    return bFnd ? FIND_FOUND : FIND_NOT_FOUND;
}


int SwFindParaText::IsReplaceMode() const
{
    return bReplace;
}


ULONG SwCursor::Find( const utl::SearchParam& rParam,
                        SwDocPositions nStart, SwDocPositions nEnde,
                        FindRanges eFndRngs, int bReplace )
{
    // OLE-Benachrichtigung abschalten !!
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    BOOL bSttUndo = pDoc->DoesUndo() && bReplace;
    if( bSttUndo )
        pDoc->StartUndo( UNDO_REPLACE );

    if( rParam.IsSrchInSelection() )
        eFndRngs = (FindRanges)(eFndRngs | FND_IN_SEL);
    SwFindParaText aSwFindParaText( rParam, bReplace, *this );
    ULONG nRet = FindAll( aSwFindParaText, nStart, nEnde, eFndRngs );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->SetModified();

    if( bSttUndo )
        pDoc->EndUndo( UNDO_REPLACE );

    return nRet;
}



