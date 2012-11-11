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

#include <doc.hxx>
#include <cntfrm.hxx>       // OSL_ENSURE(in ~SwTxtFtn()
#include <pagefrm.hxx>      // RemoveFtn()
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <swfont.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <ftnfrm.hxx>
#include <ndindex.hxx>
#include <fmtftntx.hxx>
#include <section.hxx>
#include <switerator.hxx>

namespace {
    /// Get a sorted list of the used footnote reference numbers.
    /// @param[in]  rDoc     The active document.
    /// @param[in]  pExclude A footnote whose reference number should be excluded from the set.
    /// @param[out] rInvalid  A returned list of all items that had an invalid reference number.
    /// @returns The set of used reference numbers.
    static std::set<sal_uInt16> lcl_GetUsedFtnRefNumbers(SwDoc &rDoc,
                                                         SwTxtFtn *pExclude,
                                                         std::vector<SwTxtFtn*> &rInvalid)
    {
        std::set<sal_uInt16> aArr;
        SwFtnIdxs& ftnIdxs = rDoc.GetFtnIdxs();

        rInvalid.clear();

        for( sal_uInt16 n = 0; n < ftnIdxs.size(); ++n )
        {
            SwTxtFtn* pTxtFtn = ftnIdxs[ n ];
            if ( pTxtFtn != pExclude )
            {
                if ( USHRT_MAX == pTxtFtn->GetSeqRefNo() )
                {
                    rInvalid.push_back(pTxtFtn);
                }
                else
                {
                    aArr.insert( pTxtFtn->GetSeqRefNo() );
                }
            }
        }
        return aArr;
    }

    /// Check whether a requested reference number is available.
    /// @param[in] rUsedNums Set of used reference numbers.
    /// @param[in] requested The requested reference number.
    /// @returns true if the number is available, false if not.
    static bool lcl_IsRefNumAvailable(std::set<sal_uInt16> &rUsedNums,
                                         sal_uInt16 requested)
    {
        if ( USHRT_MAX == requested )
            return false;  // Invalid sequence number.
        if ( rUsedNums.count(requested) )
            return false;  // Number already used.
        return true;
    }

    /// Get the first few unused sequential reference numbers.
    /// @param[in] rUsedNums   The set of used sequential reference numbers.
    /// @param[in] numRequired The number of reference number required.
    /// @returns The lowest unused sequential reference numbers.
    static std::vector<sal_uInt16> lcl_GetUnusedSeqRefNums(std::set<sal_uInt16> &rUsedNums,
                                                           size_t numRequired)
    {
        std::vector<sal_uInt16> unusedNums;
        sal_uInt16 newNum = 0;
        std::set<sal_uInt16>::iterator it;
        //Start by using numbers from gaps in rUsedNums
        for( it = rUsedNums.begin(); it != rUsedNums.end(); ++it )
        {
            while ( newNum < *it )
            {
                unusedNums.push_back( newNum++ );
                if ( unusedNums.size() >= numRequired )
                    return unusedNums;
            }
            newNum++;
        }
        //Filled in all gaps. Fill the rest of the list with new numbers.
        while ( unusedNums.size() < numRequired )
        {
            unusedNums.push_back( newNum++ );
        }

        return unusedNums;
    }

}

/*************************************************************************
|*
|*    class SwFmtFtn
|*
*************************************************************************/


SwFmtFtn::SwFmtFtn( bool bEndNote )
    : SfxPoolItem( RES_TXTATR_FTN ),
    pTxtAttr( 0 ),
    nNumber( 0 ),
    m_bEndNote( bEndNote )
{
}


int SwFmtFtn::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return nNumber  == ((SwFmtFtn&)rAttr).nNumber &&
           aNumber  == ((SwFmtFtn&)rAttr).aNumber &&
           m_bEndNote == ((SwFmtFtn&)rAttr).m_bEndNote;
}


SfxPoolItem* SwFmtFtn::Clone( SfxItemPool* ) const
{
    SwFmtFtn* pNew  = new SwFmtFtn;
    pNew->aNumber   = aNumber;
    pNew->nNumber   = nNumber;
    pNew->m_bEndNote = m_bEndNote;
    return pNew;
}

void SwFmtFtn::SetEndNote( bool b )
{
    if ( b != m_bEndNote )
    {
        if ( GetTxtFtn() )
        {
            GetTxtFtn()->DelFrms(0);
        }
        m_bEndNote = b;
    }
}

SwFmtFtn::~SwFmtFtn()
{
}


void SwFmtFtn::GetFtnText( XubString& rStr ) const
{
    if( pTxtAttr->GetStartNode() )
    {
        SwNodeIndex aIdx( *pTxtAttr->GetStartNode(), 1 );
        SwCntntNode* pCNd = aIdx.GetNode().GetTxtNode();
        if( !pCNd )
            pCNd = aIdx.GetNodes().GoNext( &aIdx );

        if( pCNd->IsTxtNode() )
            rStr = ((SwTxtNode*)pCNd)->GetExpandTxt();
    }
}

    // returnt den anzuzeigenden String der Fuss-/Endnote
XubString SwFmtFtn::GetViewNumStr( const SwDoc& rDoc, sal_Bool bInclStrings ) const
{
    XubString sRet( GetNumStr() );
    if( !sRet.Len() )
    {
        // dann ist die Nummer von Interesse, also ueber die Info diese
        // besorgen.
        sal_Bool bMakeNum = sal_True;
        const SwSectionNode* pSectNd = pTxtAttr
                    ? SwUpdFtnEndNtAtEnd::FindSectNdWithEndAttr( *pTxtAttr )
                    : 0;

        if( pSectNd )
        {
            const SwFmtFtnEndAtTxtEnd& rFtnEnd = (SwFmtFtnEndAtTxtEnd&)
                pSectNd->GetSection().GetFmt()->GetFmtAttr(
                                IsEndNote() ?
                                static_cast<sal_uInt16>(RES_END_AT_TXTEND) :
                                static_cast<sal_uInt16>(RES_FTN_AT_TXTEND) );

            if( FTNEND_ATTXTEND_OWNNUMANDFMT == rFtnEnd.GetValue() )
            {
                bMakeNum = sal_False;
                sRet = rFtnEnd.GetSwNumType().GetNumStr( GetNumber() );
                if( bInclStrings )
                {
                    sRet.Insert( rFtnEnd.GetPrefix(), 0 );
                    sRet += rFtnEnd.GetSuffix();
                }
            }
        }

        if( bMakeNum )
        {
            const SwEndNoteInfo* pInfo;
            if( IsEndNote() )
                pInfo = &rDoc.GetEndNoteInfo();
            else
                pInfo = &rDoc.GetFtnInfo();
            sRet = pInfo->aFmt.GetNumStr( GetNumber() );
            if( bInclStrings )
            {
                sRet.Insert( pInfo->GetPrefix(), 0 );
                sRet += pInfo->GetSuffix();
            }
        }
    }
    return sRet;
}

/*************************************************************************
 *                      class SwTxt/FmtFnt
 *************************************************************************/

SwTxtFtn::SwTxtFtn( SwFmtFtn& rAttr, xub_StrLen nStartPos )
    : SwTxtAttr( rAttr, nStartPos )
    , m_pStartNode( 0 )
    , m_pTxtNode( 0 )
    , m_nSeqNo( USHRT_MAX )
{
    rAttr.pTxtAttr = this;
    SetHasDummyChar(true);
}


SwTxtFtn::~SwTxtFtn()
{
    SetStartNode( 0 );
}



void SwTxtFtn::SetStartNode( const SwNodeIndex *pNewNode, sal_Bool bDelNode )
{
    if( pNewNode )
    {
        if ( !m_pStartNode )
        {
            m_pStartNode = new SwNodeIndex( *pNewNode );
        }
        else
        {
            *m_pStartNode = *pNewNode;
        }
    }
    else if ( m_pStartNode )
    {
        // Zwei Dinge muessen erledigt werden:
        // 1) Die Fussnoten muessen bei ihren Seiten abgemeldet werden
        // 2) Die Fussnoten-Sektion in den Inserts muss geloescht werden.
        SwDoc* pDoc;
        if ( m_pTxtNode )
        {
            pDoc = m_pTxtNode->GetDoc();
        }
        else
        {
            //JP 27.01.97: der sw3-Reader setzt einen StartNode aber das
            //              Attribut ist noch nicht im TextNode verankert.
            //              Wird es geloescht (z.B. bei Datei einfuegen mit
            //              Ftn in einen Rahmen), muss auch der Inhalt
            //              geloescht werden
            pDoc = m_pStartNode->GetNodes().GetDoc();
        }

        // Wir duerfen die Fussnotennodes nicht loeschen
        // und brauchen die Fussnotenframes nicht loeschen, wenn
        // wir im ~SwDoc() stehen.
        if( !pDoc->IsInDtor() )
        {
            if( bDelNode )
            {
                // 1) Die Section fuer die Fussnote wird beseitigt
                // Es kann sein, dass die Inserts schon geloescht wurden.
                pDoc->DeleteSection( &m_pStartNode->GetNode() );
            }
            else
                // Werden die Nodes nicht geloescht mussen sie bei den Seiten
                // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
                // stehen (Undo loescht sie nicht!)
                DelFrms( 0 );
        }
        DELETEZ( m_pStartNode );

        // loesche die Fussnote noch aus dem Array am Dokument
        for( sal_uInt16 n = 0; n < pDoc->GetFtnIdxs().size(); ++n )
            if( this == pDoc->GetFtnIdxs()[n] )
            {
                pDoc->GetFtnIdxs().erase( pDoc->GetFtnIdxs().begin() + n );
                // gibt noch weitere Fussnoten
                if( !pDoc->IsInDtor() && n < pDoc->GetFtnIdxs().size() )
                {
                    SwNodeIndex aTmp( pDoc->GetFtnIdxs()[n]->GetTxtNode() );
                    pDoc->GetFtnIdxs().UpdateFtn( aTmp );
                }
                break;
            }
    }
}


void SwTxtFtn::SetNumber( const sal_uInt16 nNewNum, const XubString* pStr )
{
    SwFmtFtn& rFtn = (SwFmtFtn&)GetFtn();
    if( pStr && pStr->Len() )
        rFtn.aNumber = *pStr;
    else
    {
        rFtn.nNumber = nNewNum;
        rFtn.aNumber = aEmptyStr;
    }

    OSL_ENSURE( m_pTxtNode, "SwTxtFtn: where is my TxtNode?" );
    SwNodes &rNodes = m_pTxtNode->GetDoc()->GetNodes();
    m_pTxtNode->ModifyNotification( 0, &rFtn );
    if ( m_pStartNode )
    {
        // must iterate over all TxtNodes because of footnotes on other pages
        SwNode* pNd;
        sal_uLong nSttIdx = m_pStartNode->GetIndex() + 1;
        sal_uLong nEndIdx = m_pStartNode->GetNode().EndOfSectionIndex();
        for( ; nSttIdx < nEndIdx; ++nSttIdx )
        {
            // Es koennen ja auch Grafiken in der Fussnote stehen ...
            if( ( pNd = rNodes[ nSttIdx ] )->IsTxtNode() )
                ((SwTxtNode*)pNd)->ModifyNotification( 0, &rFtn );
        }
    }
}

// Die Fussnoten duplizieren
void SwTxtFtn::CopyFtn(SwTxtFtn & rDest, SwTxtNode & rDestNode) const
{
    if (m_pStartNode && !rDest.GetStartNode())
    {
        // dest missing node section? create it here!
        // (happens in SwTxtNode::CopyText if pDest == this)
        rDest.MakeNewTextSection( rDestNode.GetNodes() );
    }
    if (m_pStartNode && rDest.GetStartNode())
    {
        // footnotes not necessarily in same document!
        SwDoc *const pDstDoc = rDestNode.GetDoc();
        SwNodes &rDstNodes = pDstDoc->GetNodes();

        // copy only the content of the section
        SwNodeRange aRg( *m_pStartNode, 1,
                    *m_pStartNode->GetNode().EndOfSectionNode() );

        // insert at the end of rDest, i.e., the nodes are appended.
        // nDestLen contains number of CntntNodes in rDest _before_ copy.
        SwNodeIndex aStart( *(rDest.GetStartNode()) );
        SwNodeIndex aEnd( *aStart.GetNode().EndOfSectionNode() );
        sal_uLong  nDestLen = aEnd.GetIndex() - aStart.GetIndex() - 1;

        m_pTxtNode->GetDoc()->CopyWithFlyInFly( aRg, 0, aEnd, sal_True );

        // in case the destination section was not empty, delete the old nodes
        // before:   Src: SxxxE,  Dst: SnE
        // now:      Src: SxxxE,  Dst: SnxxxE
        // after:    Src: SxxxE,  Dst: SxxxE
        ++aStart;
        rDstNodes.Delete( aStart, nDestLen );
    }

    // also copy user defined number string
    if( GetFtn().aNumber.Len() )
    {
        const_cast<SwFmtFtn &>(rDest.GetFtn()).aNumber = GetFtn().aNumber;
    }
}


    // lege eine neue leere TextSection fuer diese Fussnote an
void SwTxtFtn::MakeNewTextSection( SwNodes& rNodes )
{
    if ( m_pStartNode )
        return;

    // Nun verpassen wir dem TxtNode noch die Fussnotenvorlage.
    SwTxtFmtColl *pFmtColl;
    const SwEndNoteInfo* pInfo;
    sal_uInt16 nPoolId;

    if( GetFtn().IsEndNote() )
    {
        pInfo = &rNodes.GetDoc()->GetEndNoteInfo();
        nPoolId = RES_POOLCOLL_ENDNOTE;
    }
    else
    {
        pInfo = &rNodes.GetDoc()->GetFtnInfo();
        nPoolId = RES_POOLCOLL_FOOTNOTE;
    }

    if( 0 == (pFmtColl = pInfo->GetFtnTxtColl() ) )
        pFmtColl = rNodes.GetDoc()->GetTxtCollFromPool( nPoolId );

    SwStartNode* pSttNd = rNodes.MakeTextSection( SwNodeIndex( rNodes.GetEndOfInserts() ),
                                        SwFootnoteStartNode, pFmtColl );
    m_pStartNode = new SwNodeIndex( *pSttNd );
}


void SwTxtFtn::DelFrms( const SwFrm* pSib )
{
    // delete the FtnFrames from the pages
    OSL_ENSURE( m_pTxtNode, "SwTxtFtn: where is my TxtNode?" );
    if ( !m_pTxtNode )
        return;

    const SwRootFrm* pRoot = pSib ? pSib->getRootFrm() : 0;
    sal_Bool bFrmFnd = sal_False;
    {
        SwIterator<SwCntntFrm,SwTxtNode> aIter( *m_pTxtNode );
        for( SwCntntFrm* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
        {
            if( pRoot != pFnd->getRootFrm() && pRoot )
                continue;
            SwPageFrm* pPage = pFnd->FindPageFrm();
            if( pPage )
            {
                pPage->RemoveFtn( pFnd, this );
                bFrmFnd = sal_True;
            }
        }
    }
    //JP 13.05.97: falls das Layout vorm loeschen der Fussnoten entfernt
    //              wird, sollte man das ueber die Fussnote selbst tun
    if ( !bFrmFnd && m_pStartNode )
    {
        SwNodeIndex aIdx( *m_pStartNode );
        SwCntntNode* pCNd = m_pTxtNode->GetNodes().GoNext( &aIdx );
        if( pCNd )
        {
            SwIterator<SwCntntFrm,SwCntntNode> aIter( *pCNd );
            for( SwCntntFrm* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
            {
                if( pRoot != pFnd->getRootFrm() && pRoot )
                    continue;
                SwPageFrm* pPage = pFnd->FindPageFrm();

                SwFrm *pFrm = pFnd->GetUpper();
                while ( pFrm && !pFrm->IsFtnFrm() )
                    pFrm = pFrm->GetUpper();

                SwFtnFrm *pFtn = (SwFtnFrm*)pFrm;
                while ( pFtn && pFtn->GetMaster() )
                    pFtn = pFtn->GetMaster();
                OSL_ENSURE( pFtn->GetAttr() == this, "Ftn mismatch error." );

                while ( pFtn )
                {
                    SwFtnFrm *pFoll = pFtn->GetFollow();
                    pFtn->Cut();
                    delete pFtn;
                    pFtn = pFoll;
                }

                // #i20556# During hiding of a section, the connection
                // to the layout is already lost. pPage may be 0:
                if ( pPage )
                    pPage->UpdateFtnNum();
            }
        }
    }
}

/// Set the sequence number for the current footnote.
/// @returns The new sequence number or USHRT_MAX if invalid.
sal_uInt16 SwTxtFtn::SetSeqRefNo()
{
    if( !m_pTxtNode )
        return USHRT_MAX;

    SwDoc* pDoc = m_pTxtNode->GetDoc();
    if( pDoc->IsInReading() )
        return USHRT_MAX;

    std::vector<SwTxtFtn*> badRefNums;
    std::set<sal_uInt16> aUsedNums = ::lcl_GetUsedFtnRefNumbers(*pDoc, this, badRefNums);
    if ( ::lcl_IsRefNumAvailable(aUsedNums, m_nSeqNo) )
        return m_nSeqNo;
    std::vector<sal_uInt16> unused = ::lcl_GetUnusedSeqRefNums(aUsedNums, 1);
    return m_nSeqNo = unused[0];
}

/// Set a unique sequential reference number for every footnote in the document.
/// @param[in] rDoc The document to be processed.
void SwTxtFtn::SetUniqueSeqRefNo( SwDoc& rDoc )
{
    std::vector<SwTxtFtn*> badRefNums;
    std::set<sal_uInt16> aUsedNums = ::lcl_GetUsedFtnRefNumbers(rDoc, NULL, badRefNums);
    std::vector<sal_uInt16> unused = ::lcl_GetUnusedSeqRefNums(aUsedNums, badRefNums.size());

    for (size_t i = 0; i < badRefNums.size(); ++i)
    {
        badRefNums[i]->m_nSeqNo = unused[i];
    }
}

void SwTxtFtn::CheckCondColl()
{
//FEATURE::CONDCOLL
    if( GetStartNode() )
        ((SwStartNode&)GetStartNode()->GetNode()).CheckSectionCondColl();
//FEATURE::CONDCOLL
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
