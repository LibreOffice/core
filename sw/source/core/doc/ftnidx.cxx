/*************************************************************************
 *
 *  $RCSfile: ftnidx.cxx,v $
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

#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif


_SV_IMPL_SORTAR_ALG( _SwFtnIdxs, SwTxtFtnPtr )
BOOL _SwFtnIdxs::Seek_Entry( const SwTxtFtnPtr rSrch, USHORT* pFndPos ) const
{
    ULONG nIdx = _SwTxtFtn_GetIndex( rSrch );
    xub_StrLen nCntIdx = *rSrch->GetStart();

    register USHORT nO = Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            ULONG nFndIdx = _SwTxtFtn_GetIndex( (*this)[ nM ] );
            if( nFndIdx == nIdx && *(*this)[ nM ]->GetStart() == nCntIdx )
            {
                if( pFndPos )
                    *pFndPos = nM;
                return TRUE;
            }
            else if( nFndIdx < nIdx ||
                (nFndIdx == nIdx && *(*this)[ nM ]->GetStart() < nCntIdx ))
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return FALSE;
}


void SwFtnIdxs::UpdateFtn( SwNodeIndex& rStt )
{
    if( !Count() )
        return;

    // besorge erstmal das Nodes-Array ueber den StartIndex der ersten Fussnote
    SwDoc* pDoc = rStt.GetNode().GetDoc();
    if( pDoc->IsInReading() )
        return ;
    SwTxtFtn* pTxtFtn;

    const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
    const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();

    //Fuer normale Fussnoten werden Chapter- und Dokumentweise Nummerierung
    //getrennt behandelt. Fuer Endnoten gibt es nur die Dokumentweise
    //Nummerierung.
    if( FTNNUM_CHAPTER == rFtnInfo.eNum )
    {
        const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
        const SwNode* pCapStt = &pDoc->GetNodes().GetEndOfExtras();
        ULONG nCapEnd = pDoc->GetNodes().GetEndOfContent().GetIndex();
        if( rOutlNds.Count() )
        {
            // suche den Start des Kapitels, in den rStt steht.
            for( USHORT n = 0; n < rOutlNds.Count(); ++n )
                if( rOutlNds[ n ]->GetIndex() > rStt.GetIndex() )
                    break;      // gefunden
                else if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )
                    pCapStt = rOutlNds[ n ];    // Start eines neuen Kapitels
            // dann suche jetzt noch das Ende vom Bereich
            for( ; n < rOutlNds.Count(); ++n )
                if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )
                {
                    nCapEnd = rOutlNds[ n ]->GetIndex();    // Ende des gefundenen Kapitels
                    break;
                }
        }

        USHORT nPos, nFtnNo = 1;
        if( SeekEntry( *pCapStt, &nPos ) && nPos )
        {
            // gehe nach vorne bis der Index nicht mehr gleich ist
            const SwNode* pCmpNd = &rStt.GetNode();
            while( nPos && pCmpNd == &((*this)[ --nPos ]->GetTxtNode()) )
                ;
            ++nPos;
        }

        if( nPos == Count() )       // nichts gefunden
            return;

        if( !rOutlNds.Count() )
            nFtnNo = nPos+1;

        for( ; nPos < Count(); ++nPos )
        {
            pTxtFtn = (*this)[ nPos ];
            if( pTxtFtn->GetTxtNode().GetIndex() >= nCapEnd )
                break;

            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            if( !rFtn.GetNumStr().Len() && !rFtn.IsEndNote() &&
                !SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *pTxtFtn ))
                pTxtFtn->SetNumber( rFtnInfo.nFtnOffset + nFtnNo++,
                                    &rFtn.GetNumStr() );
        }
    }

    SwUpdFtnEndNtAtEnd aNumArr;

    // BOOL, damit hier auch bei Chapter-Einstellung die Endnoten
    // durchlaufen.
    const FASTBOOL bEndNoteOnly = FTNNUM_DOC != rFtnInfo.eNum;

    USHORT nPos, nFtnNo = 1, nEndNo = 1;
    ULONG nUpdNdIdx = rStt.GetIndex();
    for( nPos = 0; nPos < Count(); ++nPos )
    {
        pTxtFtn = (*this)[ nPos ];
        if( nUpdNdIdx <= pTxtFtn->GetTxtNode().GetIndex() )
            break;

        const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
        if( !rFtn.GetNumStr().Len() )
        {
            if( !aNumArr.ChkNumber( *pTxtFtn ) )
            {
                if( pTxtFtn->GetFtn().IsEndNote() )
                    nEndNo++;
                else
                    nFtnNo++;
            }
        }
    }

    // ab nPos bei allen FootNotes die Array-Nummer setzen
    for( ; nPos < Count(); ++nPos )
    {
        pTxtFtn = (*this)[ nPos ];
        const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
        if( !rFtn.GetNumStr().Len() )
        {
            USHORT nSectNo = aNumArr.ChkNumber( *pTxtFtn );
            if( !nSectNo && ( rFtn.IsEndNote() || !bEndNoteOnly ))
                nSectNo = rFtn.IsEndNote()
                            ? rEndInfo.nFtnOffset + nEndNo++
                            : rFtnInfo.nFtnOffset + nFtnNo++;

            if( nSectNo )
            {
                if( rFtn.IsEndNote() )
                    pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
                else
                    pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
            }
        }
    }
    // Pageweise wird vom MA erfuellt !!
}


void SwFtnIdxs::UpdateAllFtn()
{
    if( !Count() )
        return;

    // besorge erstmal das Nodes-Array ueber den StartIndex der
    // ersten Fussnote
    SwDoc* pDoc = (SwDoc*) (*this)[ 0 ]->GetTxtNode().GetDoc();
    SwTxtFtn* pTxtFtn;
    const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
    const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();

    SwUpdFtnEndNtAtEnd aNumArr;

    //Fuer normale Fussnoten werden Chapter- und Dokumentweise Nummerierung
    //getrennt behandelt. Fuer Endnoten gibt es nur die Dokumentweise
    //Nummerierung.
    if( FTNNUM_CHAPTER == rFtnInfo.eNum )
    {
        const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
        USHORT nNo = 1,         // Nummer fuer die Fussnoten
               nFtnIdx = 0;     // Index in das FtnIdx-Array
        for( USHORT n = 0; n < rOutlNds.Count(); ++n )
        {
            if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )
            {
                ULONG nCapStt = rOutlNds[ n ]->GetIndex();  // Start eines neuen Kapitels
                for( ; nFtnIdx < Count(); ++nFtnIdx )
                {
                    pTxtFtn = (*this)[ nFtnIdx ];
                    if( pTxtFtn->GetTxtNode().GetIndex() >= nCapStt )
                        break;

                    // Endnoten nur Dokumentweise
                    const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
                    if( !rFtn.IsEndNote() && !rFtn.GetNumStr().Len() &&
                        !SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *pTxtFtn ))
                        pTxtFtn->SetNumber( rFtnInfo.nFtnOffset + nNo++,
                                            &rFtn.GetNumStr() );
                }
                if( nFtnIdx >= Count() )
                    break;          // ok alles geupdatet
                nNo = 1;
            }
        }

        for( nNo = 1; nFtnIdx < Count(); ++nFtnIdx )
        {
            //Endnoten nur Dokumentweise
            pTxtFtn = (*this)[ nFtnIdx ];
            const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
            if( !rFtn.IsEndNote() && !rFtn.GetNumStr().Len() &&
                !SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *pTxtFtn ))
                pTxtFtn->SetNumber( rFtnInfo.nFtnOffset + nNo++,
                                    &rFtn.GetNumStr() );
        }

    }

    // BOOL, damit hier auch bei Chapter-Einstellung die Endnoten
    // durchlaufen.
    const FASTBOOL bEndNoteOnly = FTNNUM_DOC != rFtnInfo.eNum;
    USHORT nFtnNo = 0, nEndNo = 0;
    for( USHORT nPos = 0; nPos < Count(); ++nPos )
    {
        pTxtFtn = (*this)[ nPos ];
        const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
        if( !rFtn.GetNumStr().Len() )
        {
            USHORT nSectNo = aNumArr.ChkNumber( *pTxtFtn );
            if( !nSectNo && ( rFtn.IsEndNote() || !bEndNoteOnly ))
                nSectNo = rFtn.IsEndNote()
                                ? rEndInfo.nFtnOffset + (++nEndNo)
                                : rFtnInfo.nFtnOffset + (++nFtnNo);

            if( nSectNo )
            {
                if( rFtn.IsEndNote() )
                    pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
                else
                    pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
            }
        }
    }

    if( pDoc->GetRootFrm() && FTNNUM_PAGE == rFtnInfo.eNum )
        pDoc->GetRootFrm()->UpdateFtnNums();
}

void SwFtnIdxs::UpdateFtnInSections()
{
    if( !Count() )
        return;

    // besorge erstmal das Nodes-Array ueber den StartIndex der
    // ersten Fussnote
    SwDoc* pDoc = (SwDoc*) (*this)[ 0 ]->GetTxtNode().GetDoc();
    SwTxtFtn* pTxtFtn;
    SwUpdFtnEndNtAtEnd aNumArr;
    for( USHORT nPos = 0; nPos < Count(); ++nPos )
    {
        pTxtFtn = (*this)[ nPos ];
        const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
        if( !rFtn.GetNumStr().Len() )
        {
            USHORT nSectNo = aNumArr.ChkNumber( *pTxtFtn );
            if( nSectNo )
            {
                if( rFtn.IsEndNote() )
                    pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
                else
                    pTxtFtn->SetNumber( nSectNo, &rFtn.GetNumStr() );
            }
        }
    }
}

SwTxtFtn* SwFtnIdxs::SeekEntry( const SwNodeIndex& rPos, USHORT* pFndPos ) const
{
    ULONG nIdx = rPos.GetIndex();

    register USHORT nO = Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            ULONG nNdIdx = _SwTxtFtn_GetIndex( (*this)[ nM ] );
            if( nNdIdx == nIdx )
            {
                if( pFndPos )
                    *pFndPos = nM;
                return (*this)[ nM ];
            }
            else if( nNdIdx < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return 0;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return 0;
}

/*  */

const SwSectionNode* SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr(
                const SwTxtFtn& rTxtFtn )
{
    USHORT nWh = rTxtFtn.GetFtn().IsEndNote() ? RES_END_AT_TXTEND
                                              : RES_FTN_AT_TXTEND;
    USHORT nVal;
    const SwSectionNode* pNd = rTxtFtn.GetTxtNode().FindSectionNode();
    while( pNd && FTNEND_ATTXTEND_OWNNUMSEQ != ( nVal =
            ((const SwFmtFtnAtTxtEnd&)pNd->GetSection().GetFmt()->
            GetAttr( nWh, TRUE )).GetValue() ) &&
            FTNEND_ATTXTEND_OWNNUMANDFMT != nVal )
        pNd = pNd->FindStartNode()->FindSectionNode();

    return pNd;
}

USHORT SwUpdFtnEndNtAtEnd::GetNumber( const SwTxtFtn& rTxtFtn,
                                    const SwSectionNode& rNd )
{
    USHORT nRet = 0, nWh;
    SvPtrarr* pArr;
    SvUShorts* pNum;
    if( rTxtFtn.GetFtn().IsEndNote() )
    {
        pArr = &aEndSects;
        pNum = &aEndNums;
        nWh = RES_END_AT_TXTEND;
    }
    else
    {
        pArr = &aFtnSects;
        pNum = &aFtnNums;
        nWh = RES_FTN_AT_TXTEND;
    }
    void* pNd = (void*)&rNd;

    for( USHORT n = pArr->Count(); n; )
        if( pArr->GetObject( --n ) == pNd )
        {
            nRet = ++pNum->GetObject( n );
            break;
        }

    if( !nRet )
    {
        pArr->Insert( pNd, pArr->Count() );
        nRet = ((SwFmtFtnEndAtTxtEnd&)rNd.GetSection().GetFmt()->
                                GetAttr( nWh )).GetOffset();
        ++nRet;
        pNum->Insert( nRet, pNum->Count() );
    }
    return nRet;
}

USHORT SwUpdFtnEndNtAtEnd::ChkNumber( const SwTxtFtn& rTxtFtn )
{
    const SwSectionNode* pSectNd = FindSectNdWithEndAttr( rTxtFtn );
    return pSectNd ? GetNumber( rTxtFtn, *pSectNd ) : 0;
}




