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


#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <ftninfo.hxx>
#include <doc.hxx>
#include <ftnidx.hxx>
#include <ndtxt.hxx>
#include <ndindex.hxx>
#include <section.hxx>
#include <fmtftntx.hxx>
#include <rootfrm.hxx>


_SV_IMPL_SORTAR_ALG( _SwFtnIdxs, SwTxtFtnPtr )
sal_Bool _SwFtnIdxs::Seek_Entry( const SwTxtFtnPtr rSrch, sal_uInt16* pFndPos ) const
{
    sal_uLong nIdx = _SwTxtFtn_GetIndex( rSrch );
    xub_StrLen nCntIdx = *rSrch->GetStart();

    sal_uInt16 nO = Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            sal_uLong nFndIdx = _SwTxtFtn_GetIndex( (*this)[ nM ] );
            if( nFndIdx == nIdx && *(*this)[ nM ]->GetStart() == nCntIdx )
            {
                if( pFndPos )
                    *pFndPos = nM;
                return sal_True;
            }
            else if( nFndIdx < nIdx ||
                (nFndIdx == nIdx && *(*this)[ nM ]->GetStart() < nCntIdx ))
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return sal_False;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return sal_False;
}


void SwFtnIdxs::UpdateFtn( const SwNodeIndex& rStt )
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
        sal_uLong nCapEnd = pDoc->GetNodes().GetEndOfContent().GetIndex();
        if( rOutlNds.Count() )
        {
            // suche den Start des Kapitels, in den rStt steht.
            sal_uInt16 n;

            for( n = 0; n < rOutlNds.Count(); ++n )
                if( rOutlNds[ n ]->GetIndex() > rStt.GetIndex() )
                    break;      // gefunden
                //else if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )  //#outline level,zhaojianwei
                else if ( rOutlNds[ n ]->GetTxtNode()->GetAttrOutlineLevel() == 1 )   //<-end,zhaojianwei
                    pCapStt = rOutlNds[ n ];    // Start eines neuen Kapitels
            // dann suche jetzt noch das Ende vom Bereich
            for( ; n < rOutlNds.Count(); ++n )
                //if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )//#outline level,zhaojianwei
                if ( rOutlNds[ n ]->GetTxtNode()->GetAttrOutlineLevel() == 1 )//<-end,zhaojianwei
                {
                    nCapEnd = rOutlNds[ n ]->GetIndex();    // Ende des gefundenen Kapitels
                    break;
                }
        }

        sal_uInt16 nPos, nFtnNo = 1;
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

    // sal_Bool, damit hier auch bei Chapter-Einstellung die Endnoten
    // durchlaufen.
    const sal_Bool bEndNoteOnly = FTNNUM_DOC != rFtnInfo.eNum;

    sal_uInt16 nPos, nFtnNo = 1, nEndNo = 1;
    sal_uLong nUpdNdIdx = rStt.GetIndex();
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
            sal_uInt16 nSectNo = aNumArr.ChkNumber( *pTxtFtn );
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
        sal_uInt16 nNo = 1,         // Nummer fuer die Fussnoten
               nFtnIdx = 0;     // Index in das FtnIdx-Array
        for( sal_uInt16 n = 0; n < rOutlNds.Count(); ++n )
        {
            //if( !rOutlNds[ n ]->GetTxtNode()->GetTxtColl()->GetOutlineLevel() )//#outline level,zhaojianwei
            if ( rOutlNds[ n ]->GetTxtNode()->GetAttrOutlineLevel() == 1 )//<-end,zhaojianwei
            {
                sal_uLong nCapStt = rOutlNds[ n ]->GetIndex();  // Start eines neuen Kapitels
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

    // sal_Bool, damit hier auch bei Chapter-Einstellung die Endnoten
    // durchlaufen.
    const sal_Bool bEndNoteOnly = FTNNUM_DOC != rFtnInfo.eNum;
    sal_uInt16 nFtnNo = 0, nEndNo = 0;
    for( sal_uInt16 nPos = 0; nPos < Count(); ++nPos )
    {
        pTxtFtn = (*this)[ nPos ];
        const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
        if( !rFtn.GetNumStr().Len() )
        {
            sal_uInt16 nSectNo = aNumArr.ChkNumber( *pTxtFtn );
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

SwTxtFtn* SwFtnIdxs::SeekEntry( const SwNodeIndex& rPos, sal_uInt16* pFndPos ) const
{
    sal_uLong nIdx = rPos.GetIndex();

    sal_uInt16 nO = Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            sal_uLong nNdIdx = _SwTxtFtn_GetIndex( (*this)[ nM ] );
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
    sal_uInt16 nWh = static_cast<sal_uInt16>( rTxtFtn.GetFtn().IsEndNote() ?
                        RES_END_AT_TXTEND : RES_FTN_AT_TXTEND );
    sal_uInt16 nVal;
    const SwSectionNode* pNd = rTxtFtn.GetTxtNode().FindSectionNode();
    while( pNd && FTNEND_ATTXTEND_OWNNUMSEQ != ( nVal =
            ((const SwFmtFtnAtTxtEnd&)pNd->GetSection().GetFmt()->
            GetFmtAttr( nWh, sal_True )).GetValue() ) &&
            FTNEND_ATTXTEND_OWNNUMANDFMT != nVal )
        pNd = pNd->StartOfSectionNode()->FindSectionNode();

    return pNd;
}

sal_uInt16 SwUpdFtnEndNtAtEnd::GetNumber( const SwTxtFtn& rTxtFtn,
                                    const SwSectionNode& rNd )
{
    sal_uInt16 nRet = 0, nWh;
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

    for( sal_uInt16 n = pArr->Count(); n; )
        if( pArr->GetObject( --n ) == pNd )
        {
            nRet = ++pNum->GetObject( n );
            break;
        }

    if( !nRet )
    {
        pArr->Insert( pNd, pArr->Count() );
        nRet = ((SwFmtFtnEndAtTxtEnd&)rNd.GetSection().GetFmt()->
                                GetFmtAttr( nWh )).GetOffset();
        ++nRet;
        pNum->Insert( nRet, pNum->Count() );
    }
    return nRet;
}

sal_uInt16 SwUpdFtnEndNtAtEnd::ChkNumber( const SwTxtFtn& rTxtFtn )
{
    const SwSectionNode* pSectNd = FindSectNdWithEndAttr( rTxtFtn );
    return pSectNd ? GetNumber( rTxtFtn, *pSectNd ) : 0;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
