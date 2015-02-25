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

#include <limits.h>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <editeng/langitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <comphelper/classids.hxx>
#include <docsh.hxx>
#include <ndole.hxx>
#include <txttxmrk.hxx>
#include <fmtinfmt.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <pagedesc.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pagefrm.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <doctxm.hxx>
#include <txmsrt.hxx>
#include <rolbck.hxx>
#include <poolfmt.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <UndoAttribute.hxx>
#include <swundo.hxx>
#include <mdiexp.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <expfld.hxx>
#include <chpfld.hxx>
#include <mvsave.hxx>
#include <node2lay.hxx>
#include <SwStyleNameMapper.hxx>
#include <breakit.hxx>
#include <editsh.hxx>
#include <scriptinfo.hxx>
#include <switerator.hxx>
#include <ToxTextGenerator.hxx>
#include <ToxTabStopTokenHandler.hxx>
#include <tools/datetimeutils.hxx>

#include <boost/make_shared.hpp>

using namespace ::com::sun::star;

TYPEINIT2( SwTOXBaseSection, SwTOXBase, SwSection );    // for RTTI

sal_uInt16 SwDoc::GetTOIKeys( SwTOIKeyType eTyp, std::vector<OUString>& rArr ) const
{
    rArr.clear();

    // Look up all Primary and Secondary via the Pool
    const sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_TOXMARK );
    for( sal_uInt32 i = 0; i < nMaxItems; ++i )
    {
        const SwTOXMark* pItem = static_cast<const SwTOXMark*>(GetAttrPool().GetItem2( RES_TXTATR_TOXMARK, i ));
        if( !pItem )
            continue;
        const SwTOXType* pTOXType = pItem->GetTOXType();
        if ( !pTOXType || pTOXType->GetType()!=TOX_INDEX )
            continue;
        const SwTxtTOXMark* pMark = pItem->GetTxtTOXMark();
        if ( pMark && pMark->GetpTxtNd() &&
             pMark->GetpTxtNd()->GetNodes().IsDocNodes() )
        {
            const OUString sStr = TOI_PRIMARY == eTyp
                ? pItem->GetPrimaryKey()
                : pItem->GetSecondaryKey();

            if( !sStr.isEmpty() )
                rArr.push_back( sStr );
        }
    }

    return rArr.size();
}

/// Get current table of contents Mark.
sal_uInt16 SwDoc::GetCurTOXMark( const SwPosition& rPos,
                                SwTOXMarks& rArr ) const
{
    // search on Position rPos for all SwTOXMarks
    SwTxtNode *const pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( !pTxtNd || !pTxtNd->GetpSwpHints() )
        return 0;

    const SwpHints & rHts = *pTxtNd->GetpSwpHints();
    sal_Int32 nSttIdx;
    const sal_Int32 *pEndIdx;

    const sal_Int32 nAktPos = rPos.nContent.GetIndex();

    for( size_t n = 0; n < rHts.Count(); ++n )
    {
        const SwTxtAttr* pHt = rHts[n];
        if( RES_TXTATR_TOXMARK != pHt->Which() )
            continue;
        if( ( nSttIdx = pHt->GetStart() ) < nAktPos )
        {
            // also check the end
            if( 0 == ( pEndIdx = pHt->End() ) ||
                *pEndIdx <= nAktPos )
                continue;       // keep searching
        }
        else if( nSttIdx > nAktPos )
            // If Hint's Start is greater than rPos, break, because
            // the attributes are sorted by Start!
            break;

        SwTOXMark* pTMark = (SwTOXMark*) &pHt->GetTOXMark();
        rArr.push_back( pTMark );
    }
    return rArr.size();
}

/// Delete table of contents Mark
void SwDoc::DeleteTOXMark( const SwTOXMark* pTOXMark )
{
    const SwTxtTOXMark* pTxtTOXMark = pTOXMark->GetTxtTOXMark();
    OSL_ENSURE( pTxtTOXMark, "No TxtTOXMark, cannot be deleted" );

    SwTxtNode& rTxtNd = const_cast<SwTxtNode&>(pTxtTOXMark->GetTxtNode());
    OSL_ENSURE( rTxtNd.GetpSwpHints(), "cannot be deleted" );

    std::unique_ptr<SwRegHistory> aRHst;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // save attributes for Undo
        SwUndoResetAttr* pUndo = new SwUndoResetAttr(
            SwPosition( rTxtNd, SwIndex( &rTxtNd, pTxtTOXMark->GetStart() ) ),
            RES_TXTATR_TOXMARK );
        GetIDocumentUndoRedo().AppendUndo( pUndo );

        aRHst.reset(new SwRegHistory(rTxtNd, &pUndo->GetHistory()));
        rTxtNd.GetpSwpHints()->Register(aRHst.get());
    }

    rTxtNd.DeleteAttribute( const_cast<SwTxtTOXMark*>(pTxtTOXMark) );

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        if( rTxtNd.GetpSwpHints() )
            rTxtNd.GetpSwpHints()->DeRegister();
    }
    getIDocumentState().SetModified();
}

/// Travel between table of content Marks
class CompareNodeCntnt
{
    sal_uLong nNode;
    sal_Int32 nCntnt;
public:
    CompareNodeCntnt( sal_uLong nNd, sal_Int32 nCnt )
        : nNode( nNd ), nCntnt( nCnt ) {}

    bool operator==( const CompareNodeCntnt& rCmp ) const
        { return nNode == rCmp.nNode && nCntnt == rCmp.nCntnt; }
    bool operator!=( const CompareNodeCntnt& rCmp ) const
        { return nNode != rCmp.nNode || nCntnt != rCmp.nCntnt; }
    bool operator< ( const CompareNodeCntnt& rCmp ) const
        { return nNode < rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt < rCmp.nCntnt); }
    bool operator<=( const CompareNodeCntnt& rCmp ) const
        { return nNode < rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt <= rCmp.nCntnt); }
    bool operator> ( const CompareNodeCntnt& rCmp ) const
        { return nNode > rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt > rCmp.nCntnt); }
    bool operator>=( const CompareNodeCntnt& rCmp ) const
        { return nNode > rCmp.nNode ||
            ( nNode == rCmp.nNode && nCntnt >= rCmp.nCntnt); }
};

const SwTOXMark& SwDoc::GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                    SwTOXSearch eDir, bool bInReadOnly )
{
    const SwTxtTOXMark* pMark = rCurTOXMark.GetTxtTOXMark();
    OSL_ENSURE(pMark, "pMark==0 invalid TxtTOXMark");

    const SwTxtNode *pTOXSrc = pMark->GetpTxtNd();

    CompareNodeCntnt aAbsIdx( pTOXSrc->GetIndex(), pMark->GetStart() );
    CompareNodeCntnt aPrevPos( 0, 0 );
    CompareNodeCntnt aNextPos( ULONG_MAX, SAL_MAX_INT32 );
    CompareNodeCntnt aMax( 0, 0 );
    CompareNodeCntnt aMin( ULONG_MAX, SAL_MAX_INT32 );

    const SwTOXMark*    pNew    = 0;
    const SwTOXMark*    pMax    = &rCurTOXMark;
    const SwTOXMark*    pMin    = &rCurTOXMark;

    const SwTOXType* pType = rCurTOXMark.GetTOXType();
    SwTOXMarks aMarks;
    SwTOXMark::InsertTOXMarks( aMarks, *pType );

    for( size_t nMark=0; nMark<aMarks.size(); ++nMark )
    {
        const SwTOXMark* pTOXMark = aMarks[nMark];
        if ( pTOXMark == &rCurTOXMark )
            continue;

        pMark = pTOXMark->GetTxtTOXMark();
        if (!pMark)
            continue;

        pTOXSrc = pMark->GetpTxtNd();
        if (!pTOXSrc)
            continue;

        Point aPt;
        const SwCntntFrm* pCFrm = pTOXSrc->getLayoutFrm( getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
        if (!pCFrm)
            continue;

        if ( bInReadOnly || !pCFrm->IsProtected() )
        {
            CompareNodeCntnt aAbsNew( pTOXSrc->GetIndex(), pMark->GetStart() );
            switch( eDir )
            {
            // The following (a bit more complicated) statements make it
            // possible to also travel across Entries on the same (!)
            // position. If someone has time, please feel free to optimize.
            case TOX_SAME_PRV:
                if( pTOXMark->GetText() != rCurTOXMark.GetText() )
                    break;
            /* no break here */
            case TOX_PRV:
                if ( (aAbsNew < aAbsIdx && aAbsNew > aPrevPos) ||
                     (aAbsIdx == aAbsNew &&
                      (sal_uLong(&rCurTOXMark) > sal_uLong(pTOXMark) &&
                       (!pNew ||
                        (pNew && (aPrevPos < aAbsIdx ||
                                  sal_uLong(pNew) < sal_uLong(pTOXMark)))))) ||
                     (aPrevPos == aAbsNew && aAbsIdx != aAbsNew &&
                      sal_uLong(pTOXMark) > sal_uLong(pNew)) )
                {
                    pNew = pTOXMark;
                    aPrevPos = aAbsNew;
                    if ( aAbsNew >= aMax )
                    {
                        aMax = aAbsNew;
                        pMax = pTOXMark;
                    }
                }
                break;

            case TOX_SAME_NXT:
                if( pTOXMark->GetText() != rCurTOXMark.GetText() )
                    break;
            /* no break here */
            case TOX_NXT:
                if ( (aAbsNew > aAbsIdx && aAbsNew < aNextPos) ||
                     (aAbsIdx == aAbsNew &&
                      (sal_uLong(&rCurTOXMark) < sal_uLong(pTOXMark) &&
                       (!pNew ||
                        (pNew && (aNextPos > aAbsIdx ||
                                  sal_uLong(pNew) > sal_uLong(pTOXMark)))))) ||
                     (aNextPos == aAbsNew && aAbsIdx != aAbsNew &&
                      sal_uLong(pTOXMark) < sal_uLong(pNew)) )
                {
                    pNew = pTOXMark;
                    aNextPos = aAbsNew;
                    if ( aAbsNew <= aMin )
                    {
                        aMin = aAbsNew;
                        pMin = pTOXMark;
                    }
                }
                break;
            }
        }
    }

    // We couldn't find a successor
    // Use minimum or maximum
    if(!pNew)
    {
        switch(eDir)
        {
        case TOX_PRV:
        case TOX_SAME_PRV:
            pNew = pMax;
            break;
        case TOX_NXT:
        case TOX_SAME_NXT:
            pNew = pMin;
            break;
        default:
            pNew = &rCurTOXMark;
        }
    }
    return *pNew;
}

const SwTOXBaseSection* SwDoc::InsertTableOf( const SwPosition& rPos,
                                                const SwTOXBase& rTOX,
                                                const SfxItemSet* pSet,
                                                bool bExpand )
{
    GetIDocumentUndoRedo().StartUndo( UNDO_INSTOX, NULL );

    OUString sSectNm = GetUniqueTOXBaseName( *rTOX.GetTOXType(), rTOX.GetTOXName() );
    SwPaM aPam( rPos );
    SwSectionData aSectionData( TOX_CONTENT_SECTION, sSectNm );
    SwTOXBaseSection *const pNewSection = dynamic_cast<SwTOXBaseSection *>(
        InsertSwSection( aPam, aSectionData, & rTOX, pSet, false ));
    if (pNewSection)
    {
        SwSectionNode *const pSectNd = pNewSection->GetFmt()->GetSectionNode();
        pNewSection->SetTOXName(sSectNm); // rTOX may have had no name...

        if( bExpand )
        {
            // add value for 2nd parameter = true to
            // indicate, that a creation of a new table of content has to be performed.
            // Value of 1st parameter = default value.
            pNewSection->Update( 0, true );
        }
        else if( rTOX.GetTitle().getLength()==1 && IsInReading() )
        // insert title of TOX
        {
            // then insert the headline section
            SwNodeIndex aIdx( *pSectNd, +1 );

            SwTxtNode* pHeadNd = GetNodes().MakeTxtNode( aIdx,
                            getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

            OUString sNm( pNewSection->GetTOXName() );
            // ??Resource
            sNm += "_Head";

            SwSectionData headerData( TOX_HEADER_SECTION, sNm );

            SwNodeIndex aStt( *pHeadNd ); aIdx--;
            SwSectionFmt* pSectFmt = MakeSectionFmt( 0 );
            GetNodes().InsertTextSection(
                    aStt, *pSectFmt, headerData, 0, &aIdx, true, false);
        }
    }

    GetIDocumentUndoRedo().EndUndo( UNDO_INSTOX, NULL );

    return pNewSection;
}

const SwTOXBaseSection* SwDoc::InsertTableOf( sal_uLong nSttNd, sal_uLong nEndNd,
                                                const SwTOXBase& rTOX,
                                                const SfxItemSet* pSet )
{
    // check for recursiv TOX
    SwNode* pNd = GetNodes()[ nSttNd ];
    SwSectionNode* pSectNd = pNd->FindSectionNode();
    while( pSectNd )
    {
        SectionType eT = pSectNd->GetSection().GetType();
        if( TOX_HEADER_SECTION == eT || TOX_CONTENT_SECTION == eT )
            return 0;
        pSectNd = pSectNd->StartOfSectionNode()->FindSectionNode();
    }

    OUString sSectNm = GetUniqueTOXBaseName(*rTOX.GetTOXType(), rTOX.GetTOXName());

    SwSectionData aSectionData( TOX_CONTENT_SECTION, sSectNm );

    SwNodeIndex aStt( GetNodes(), nSttNd ), aEnd( GetNodes(), nEndNd );
    SwSectionFmt* pFmt = MakeSectionFmt( 0 );
    if(pSet)
        pFmt->SetFmtAttr(*pSet);

    SwSectionNode *const pNewSectionNode =
        GetNodes().InsertTextSection(aStt, *pFmt, aSectionData, &rTOX, &aEnd);
    if (!pNewSectionNode)
    {
        DelSectionFmt( pFmt );
        return 0;
    }

    SwTOXBaseSection *const pNewSection(
        dynamic_cast<SwTOXBaseSection*>(& pNewSectionNode->GetSection()));
    if (pNewSection)
        pNewSection->SetTOXName(sSectNm); // rTOX may have had no name...
    return pNewSection;
}

/// Get current table of contents
const SwTOXBase* SwDoc::GetCurTOX( const SwPosition& rPos ) const
{
    const SwNode& rNd = rPos.nNode.GetNode();
    const SwSectionNode* pSectNd = rNd.FindSectionNode();
    while( pSectNd )
    {
        SectionType eT = pSectNd->GetSection().GetType();
        if( TOX_CONTENT_SECTION == eT )
        {
            OSL_ENSURE( pSectNd->GetSection().ISA( SwTOXBaseSection ),
                    "no TOXBaseSection!" );
            const SwTOXBaseSection& rTOXSect = static_cast<const SwTOXBaseSection&>(
                                                pSectNd->GetSection());
            return &rTOXSect;
        }
        pSectNd = pSectNd->StartOfSectionNode()->FindSectionNode();
    }
    return 0;
}

const SwAttrSet& SwDoc::GetTOXBaseAttrSet(const SwTOXBase& rTOXBase) const
{
    OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = static_cast<const SwTOXBaseSection&>(rTOXBase);
    SwSectionFmt* pFmt = rTOXSect.GetFmt();
    OSL_ENSURE( pFmt, "invalid TOXBaseSection!" );
    return pFmt->GetAttrSet();
}

const SwTOXBase* SwDoc::GetDefaultTOXBase( TOXTypes eTyp, bool bCreate )
{
    SwTOXBase** prBase = 0;
    switch(eTyp)
    {
    case  TOX_CONTENT:          prBase = &mpDefTOXBases->pContBase; break;
    case  TOX_INDEX:            prBase = &mpDefTOXBases->pIdxBase;  break;
    case  TOX_USER:             prBase = &mpDefTOXBases->pUserBase; break;
    case  TOX_TABLES:           prBase = &mpDefTOXBases->pTblBase;  break;
    case  TOX_OBJECTS:          prBase = &mpDefTOXBases->pObjBase;  break;
    case  TOX_ILLUSTRATIONS:    prBase = &mpDefTOXBases->pIllBase;  break;
    case  TOX_AUTHORITIES:      prBase = &mpDefTOXBases->pAuthBase; break;
    case  TOX_BIBLIOGRAPHY:      prBase = &mpDefTOXBases->pBiblioBase; break;
    case  TOX_CITATION: /** TODO */break;
    }
    if (!prBase)
        return NULL;
    if(!(*prBase) && bCreate)
    {
        SwForm aForm(eTyp);
        const SwTOXType* pType = GetTOXType(eTyp, 0);
        (*prBase) = new SwTOXBase(pType, aForm, 0, pType->GetTypeName());
    }
    return (*prBase);
}

void    SwDoc::SetDefaultTOXBase(const SwTOXBase& rBase)
{
    SwTOXBase** prBase = 0;
    switch(rBase.GetType())
    {
    case  TOX_CONTENT:          prBase = &mpDefTOXBases->pContBase; break;
    case  TOX_INDEX:            prBase = &mpDefTOXBases->pIdxBase;  break;
    case  TOX_USER:             prBase = &mpDefTOXBases->pUserBase; break;
    case  TOX_TABLES:           prBase = &mpDefTOXBases->pTblBase;  break;
    case  TOX_OBJECTS:          prBase = &mpDefTOXBases->pObjBase;  break;
    case  TOX_ILLUSTRATIONS:    prBase = &mpDefTOXBases->pIllBase;  break;
    case  TOX_AUTHORITIES:      prBase = &mpDefTOXBases->pAuthBase; break;
    case  TOX_BIBLIOGRAPHY:      prBase = &mpDefTOXBases->pBiblioBase; break;
    case  TOX_CITATION: /** TODO */break;
    }
    if (!prBase)
        return;
    delete (*prBase);
    (*prBase) = new SwTOXBase(rBase);
}

/// Delete table of contents
bool SwDoc::DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes )
{
    // We only delete the TOX, not the Nodes
    bool bRet = false;
    OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );

    const SwTOXBaseSection& rTOXSect = static_cast<const SwTOXBaseSection&>(rTOXBase);
    SwSectionFmt* pFmt = rTOXSect.GetFmt();
    if( pFmt )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_CLEARTOXRANGE, NULL );

        /* Save the start node of the TOX' section. */
        SwSectionNode * pMyNode = pFmt->GetSectionNode();
        /* Save start node of section's surrounding. */
        SwNode * pStartNd = pMyNode->StartOfSectionNode();

        /* Look for the point where to move the cursors in the area to
           delete to. This is done by first searching forward from the
           end of the TOX' section. If no content node is found behind
           the TOX one is searched before it. If this is not
           successful, too, insert new text node behind the end of
           the TOX' section. The cursors from the TOX' section will be
           moved to the content node found or the new text node. */

        /* Set PaM to end of TOX' section and search following content node.
           aSearchPam will contain the point where to move the cursors
           to. */
        SwPaM aSearchPam(*pMyNode->EndOfSectionNode());
        SwPosition aEndPos(*pStartNd->EndOfSectionNode());
        if (! aSearchPam.Move() /* no content node found */
            || *aSearchPam.GetPoint() >= aEndPos /* content node found
                                                    outside surrounding */
            )
        {
            /* Set PaM to beginning of TOX' section and search previous
               content node */
            SwPaM aTmpPam(*pMyNode);
            aSearchPam = aTmpPam;
            SwPosition aStartPos(*pStartNd);

            if ( ! aSearchPam.Move(fnMoveBackward) /* no content node found */
                 || *aSearchPam.GetPoint() <= aStartPos  /* content node
                                                            found outside
                                                            surrounding */
                 )
            {
                /* There is no content node in the surrounding of
                   TOX'. Append text node behind TOX' section. */

                SwPosition aInsPos(*pMyNode->EndOfSectionNode());
                getIDocumentContentOperations().AppendTxtNode(aInsPos);

                SwPaM aTmpPam1(aInsPos);
                aSearchPam = aTmpPam1;
            }
        }

        /* PaM containing the TOX. */
        SwPaM aPam(*pMyNode->EndOfSectionNode(), *pMyNode);

        /* Move cursors contained in TOX to the above calculated point. */
        PaMCorrAbs(aPam, *aSearchPam.GetPoint());

        if( !bDelNodes )
        {
            SwSections aArr( 0 );
            sal_uInt16 nCnt = pFmt->GetChildSections( aArr, SORTSECT_NOT, false );
            for( sal_uInt16 n = 0; n < nCnt; ++n )
            {
                SwSection* pSect = aArr[ n ];
                if( TOX_HEADER_SECTION == pSect->GetType() )
                {
                    DelSectionFmt( pSect->GetFmt(), bDelNodes );
                }
            }
        }

        DelSectionFmt( pFmt, bDelNodes );

        GetIDocumentUndoRedo().EndUndo( UNDO_CLEARTOXRANGE, NULL );
        bRet = true;
    }

    return bRet;
}

/// Manage table of content types
sal_uInt16 SwDoc::GetTOXTypeCount(TOXTypes eTyp) const
{
    sal_uInt16 nCnt = 0;
    for( sal_uInt16 n = 0; n < mpTOXTypes->size(); ++n )
        if( eTyp == (*mpTOXTypes)[n]->GetType() )
            ++nCnt;
    return nCnt;
}

const SwTOXType* SwDoc::GetTOXType( TOXTypes eTyp, sal_uInt16 nId ) const
{
    sal_uInt16 nCnt = 0;
    for( sal_uInt16 n = 0; n < mpTOXTypes->size(); ++n )
        if( eTyp == (*mpTOXTypes)[n]->GetType() && nCnt++ == nId )
            return (*mpTOXTypes)[n];
    return 0;
}

const SwTOXType* SwDoc::InsertTOXType( const SwTOXType& rTyp )
{
    SwTOXType * pNew = new SwTOXType( rTyp );
    mpTOXTypes->push_back( pNew );
    return pNew;
}

OUString SwDoc::GetUniqueTOXBaseName( const SwTOXType& rType,
                                      const OUString& sChkStr ) const
{
    if( IsInMailMerge())
    {
        OUString newName = "MailMergeTOX"
            + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
            + OUString::number( mpSectionFmtTbl->size() + 1 );
        if( !sChkStr.isEmpty())
            newName += sChkStr;
        return newName;
    }

    bool bUseChkStr = !sChkStr.isEmpty();
    const OUString aName( rType.GetTypeName() );
    const sal_Int32 nNmLen = aName.getLength();

    sal_uInt16 nNum = 0;
    const sal_uInt16 nFlagSize = ( mpSectionFmtTbl->size() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    for( sal_uInt16 n = 0; n < mpSectionFmtTbl->size(); ++n )
    {
        const SwSectionNode *pSectNd = (*mpSectionFmtTbl)[ n ]->GetSectionNode( false );
        if ( !pSectNd )
            continue;

        const SwSection& rSect = pSectNd->GetSection();
        if (rSect.GetType()==TOX_CONTENT_SECTION)
        {
            const OUString rNm = rSect.GetSectionName();
            if ( aName.startsWith(rNm) )
            {
                // Calculate number and set the Flag
                nNum = (sal_uInt16)rNm.copy( nNmLen ).toInt32();
                if( nNum-- && nNum < mpSectionFmtTbl->size() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if ( bUseChkStr && sChkStr==rNm )
                bUseChkStr = false;
        }
    }

    if( !bUseChkStr )
    {
        // All Numbers have been flagged accordingly, so get the right Number
        nNum = mpSectionFmtTbl->size();
        for( sal_uInt16 n = 0; n < nFlagSize; ++n )
        {
            sal_uInt16 nTmp = pSetFlags[ n ];
            if( nTmp != 0xff )
            {
                // so get the Number
                nNum = n * 8;
                while( nTmp & 1 )
                {
                    ++nNum;
                    nTmp >>= 1;
                }
                break;
            }
        }
    }
    delete [] pSetFlags;
    if ( bUseChkStr )
        return sChkStr;
    return aName + OUString::number( ++nNum );
}

bool SwDoc::SetTOXBaseName(const SwTOXBase& rTOXBase, const OUString& rName)
{
    OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ),
                    "no TOXBaseSection!" );
    SwTOXBaseSection* pTOX = const_cast<SwTOXBaseSection*>(static_cast<const SwTOXBaseSection*>(&rTOXBase));

    OUString sTmp = GetUniqueTOXBaseName(*rTOXBase.GetTOXType(), rName);
    bool bRet = sTmp == rName;
    if(bRet)
    {
        pTOX->SetTOXName(rName);
        pTOX->SetSectionName(rName);
        getIDocumentState().SetModified();
    }
    return bRet;
}

static const SwTxtNode* lcl_FindChapterNode( const SwNode& rNd, sal_uInt8 nLvl = 0 )
{
    const SwNode* pNd = &rNd;
    if( pNd->GetNodes().GetEndOfExtras().GetIndex() > pNd->GetIndex() )
    {
        // then find the "Anchor" (Body) position
        Point aPt;
        SwNode2Layout aNode2Layout( *pNd, pNd->GetIndex() );
        const SwFrm* pFrm = aNode2Layout.GetFrm( &aPt, 0, false );

        if( pFrm )
        {
            SwPosition aPos( *pNd );
            pNd = GetBodyTxtNode( *pNd->GetDoc(), aPos, *pFrm );
            OSL_ENSURE( pNd, "Where's the paragraph?" );
        }
    }
    return pNd ? pNd->FindOutlineNodeOfLevel( nLvl ) : 0;
}

// Table of contents class
SwTOXBaseSection::SwTOXBaseSection(SwTOXBase const& rBase, SwSectionFmt & rFmt)
    : SwTOXBase( rBase )
    , SwSection( TOX_CONTENT_SECTION, OUString(), rFmt )
{
    SetProtect( rBase.IsProtected() );
    SetSectionName( GetTOXName() );
}

SwTOXBaseSection::~SwTOXBaseSection()
{
    for (SwTOXSortTabBases::const_iterator it = aSortArr.begin(); it != aSortArr.end(); ++it)
        delete *it;
}

bool SwTOXBaseSection::SetPosAtStartEnd( SwPosition& rPos, bool bAtStart ) const
{
    bool bRet = false;
    const SwSectionNode* pSectNd = GetFmt()->GetSectionNode();
    if( pSectNd )
    {
        SwCntntNode* pCNd;
        sal_Int32 nC = 0;
        if( bAtStart )
        {
            rPos.nNode = *pSectNd;
            pCNd = pSectNd->GetDoc()->GetNodes().GoNext( &rPos.nNode );
        }
        else
        {
            rPos.nNode = *pSectNd->EndOfSectionNode();
            pCNd = pSectNd->GetDoc()->GetNodes().GoPrevious( &rPos.nNode );
            if( pCNd ) nC = pCNd->Len();
        }
        rPos.nContent.Assign( pCNd, nC );
        bRet = true;
    }
    return bRet;
}

/// Collect table of contents content
void SwTOXBaseSection::Update(const SfxItemSet* pAttr,
                              const bool        _bNewTOX )
{
    const SwSectionNode* pSectNd;
    if( !SwTOXBase::GetRegisteredIn()->GetDepends() ||
        !GetFmt() || 0 == (pSectNd = GetFmt()->GetSectionNode() ) ||
        !pSectNd->GetNodes().IsDocNodes() ||
        IsHiddenFlag() )
    {
        return;
    }

    if ( !mbKeepExpression )
    {
        maMSTOCExpression.clear();
    }

    SwDoc* pDoc = (SwDoc*)pSectNd->GetDoc();

    assert(pDoc); //Where is the document?

    if (pAttr && GetFmt())
        pDoc->ChgFmt(*GetFmt(), *pAttr);

    // determine default page description, which will be used by the content nodes,
    // if no appropriate one is found.
    const SwPageDesc* pDefaultPageDesc;
    {
        pDefaultPageDesc =
            pSectNd->GetSection().GetFmt()->GetPageDesc().GetPageDesc();
        if ( !_bNewTOX && !pDefaultPageDesc )
        {
            // determine page description of table-of-content
            sal_uInt32 nPgDescNdIdx = pSectNd->GetIndex() + 1;
            sal_uInt32* pPgDescNdIdx = &nPgDescNdIdx;
            pDefaultPageDesc = pSectNd->FindPageDesc( false, pPgDescNdIdx );
            if ( nPgDescNdIdx < pSectNd->GetIndex() )
            {
                pDefaultPageDesc = 0;
            }
        }
        // consider end node of content section in the node array.
        if ( !pDefaultPageDesc &&
             ( pSectNd->EndOfSectionNode()->GetIndex() <
                 (pSectNd->GetNodes().GetEndOfContent().GetIndex() - 1) )
           )
        {
            // determine page description of content after table-of-content
            SwNodeIndex aIdx( *(pSectNd->EndOfSectionNode()) );
            const SwCntntNode* pNdAfterTOX = pSectNd->GetNodes().GoNext( &aIdx );
            const SwAttrSet& aNdAttrSet = pNdAfterTOX->GetSwAttrSet();
            const SvxBreak eBreak = aNdAttrSet.GetBreak().GetBreak();
            if ( !( eBreak == SVX_BREAK_PAGE_BEFORE ||
                    eBreak == SVX_BREAK_PAGE_BOTH )
               )
            {
                pDefaultPageDesc = pNdAfterTOX->FindPageDesc( false );
            }
        }
        // consider start node of content section in the node array.
        if ( !pDefaultPageDesc &&
             ( pSectNd->GetIndex() >
                 (pSectNd->GetNodes().GetEndOfContent().StartOfSectionIndex() + 1) )
           )
        {
            // determine page description of content before table-of-content
            SwNodeIndex aIdx( *pSectNd );
            pDefaultPageDesc =
                pSectNd->GetNodes().GoPrevious( &aIdx )->FindPageDesc( false );

        }
        if ( !pDefaultPageDesc )
        {
            // determine default page description
            pDefaultPageDesc = &pDoc->GetPageDesc( 0 );
        }
    }

    pDoc->getIDocumentState().SetModified();

    // get current Language
    SwTOXInternational aIntl(  GetLanguage(),
                               TOX_INDEX == GetTOXType()->GetType() ?
                               GetOptions() : 0,
                               GetSortAlgorithm() );

    for (SwTOXSortTabBases::const_iterator it = aSortArr.begin(); it != aSortArr.end(); ++it)
        delete *it;
    aSortArr.clear();

    // find the first layout node for this TOX, if it only find the content
    // in his own chapter
    const SwTxtNode* pOwnChapterNode = IsFromChapter()
            ? ::lcl_FindChapterNode( *pSectNd, 0 )
            : 0;

    SwNode2Layout aN2L( *pSectNd );
    ((SwSectionNode*)pSectNd)->DelFrms();

    // remove old content an insert one empty textnode (to hold the layout!)
    SwTxtNode* pFirstEmptyNd;
    {
        pDoc->getIDocumentRedlineAccess().DeleteRedline( *pSectNd, true, USHRT_MAX );

        SwNodeIndex aSttIdx( *pSectNd, +1 );
        SwNodeIndex aEndIdx( *pSectNd->EndOfSectionNode() );
        pFirstEmptyNd = pDoc->GetNodes().MakeTxtNode( aEndIdx,
                        pDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_TEXT ) );

        {
            // Task 70995 - save and restore PageDesc and Break Attributes
            SwNodeIndex aNxtIdx( aSttIdx );
            const SwCntntNode* pCNd = aNxtIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = pDoc->GetNodes().GoNext( &aNxtIdx );
            if( pCNd->HasSwAttrSet() )
            {
                SfxItemSet aBrkSet( pDoc->GetAttrPool(), aBreakSetRange );
                aBrkSet.Put( *pCNd->GetpSwAttrSet() );
                if( aBrkSet.Count() )
                    pFirstEmptyNd->SetAttr( aBrkSet );
            }
        }
        aEndIdx--;
        SwPosition aPos( aEndIdx, SwIndex( pFirstEmptyNd, 0 ));
        pDoc->CorrAbs( aSttIdx, aEndIdx, aPos, true );

        // delete flys in whole range including start node which requires
        // giving the node before start node as Mark parameter, hence -1.
        // (flys must be deleted because the anchor nodes are removed)
        DelFlyInRange( SwNodeIndex(aSttIdx, -1), aEndIdx );

        pDoc->GetNodes().Delete( aSttIdx, aEndIdx.GetIndex() - aSttIdx.GetIndex() );
    }

    // insert title of TOX
    if ( !GetTitle().isEmpty() )
    {
        // then insert the headline section
        SwNodeIndex aIdx( *pSectNd, +1 );

        SwTxtNode* pHeadNd = pDoc->GetNodes().MakeTxtNode( aIdx,
                                GetTxtFmtColl( FORM_TITLE ) );
        pHeadNd->InsertText( GetTitle(), SwIndex( pHeadNd ) );

        OUString sNm( GetTOXName() );
        // ??Resource
        sNm += "_Head";

        SwSectionData headerData( TOX_HEADER_SECTION, sNm );

        SwNodeIndex aStt( *pHeadNd ); aIdx--;
        SwSectionFmt* pSectFmt = pDoc->MakeSectionFmt( 0 );
        pDoc->GetNodes().InsertTextSection(
                aStt, *pSectFmt, headerData, 0, &aIdx, true, false);
    }

    // This would be a good time to update the Numbering
    pDoc->UpdateNumRule();

    if( GetCreateType() & nsSwTOXElement::TOX_MARK )
        UpdateMarks( aIntl, pOwnChapterNode );

    if( GetCreateType() & nsSwTOXElement::TOX_OUTLINELEVEL )
        UpdateOutline( pOwnChapterNode );

    if( GetCreateType() & nsSwTOXElement::TOX_TEMPLATE )
        UpdateTemplate( pOwnChapterNode );

    if( GetCreateType() & nsSwTOXElement::TOX_OLE ||
            TOX_OBJECTS == SwTOXBase::GetType())
        UpdateCntnt( nsSwTOXElement::TOX_OLE, pOwnChapterNode );

    if( GetCreateType() & nsSwTOXElement::TOX_TABLE ||
            (TOX_TABLES == SwTOXBase::GetType() && IsFromObjectNames()) )
        UpdateTable( pOwnChapterNode );

    if( GetCreateType() & nsSwTOXElement::TOX_GRAPHIC ||
        (TOX_ILLUSTRATIONS == SwTOXBase::GetType() && IsFromObjectNames()))
        UpdateCntnt( nsSwTOXElement::TOX_GRAPHIC, pOwnChapterNode );

    if( !GetSequenceName().isEmpty() && !IsFromObjectNames() &&
        (TOX_TABLES == SwTOXBase::GetType() ||
         TOX_ILLUSTRATIONS == SwTOXBase::GetType() ) )
        UpdateSequence( pOwnChapterNode );

    if( GetCreateType() & nsSwTOXElement::TOX_FRAME )
        UpdateCntnt( nsSwTOXElement::TOX_FRAME, pOwnChapterNode );

    if(TOX_AUTHORITIES == SwTOXBase::GetType())
        UpdateAuthorities( aIntl );

    // Insert AlphaDelimitters if needed (just for keywords)
    if( TOX_INDEX == SwTOXBase::GetType() &&
        ( GetOptions() & nsSwTOIOptions::TOI_ALPHA_DELIMITTER ) )
        InsertAlphaDelimitter( aIntl );

    // Sort the List of all TOC Marks and TOC Sections
    std::vector<SwTxtFmtColl*> aCollArr( GetTOXForm().GetFormMax(), 0 );
    SwNodeIndex aInsPos( *pFirstEmptyNd, 1 );
    for( sal_uInt16 nCnt = 0; nCnt < aSortArr.size(); ++nCnt )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        // Put the Text into the TOC
        sal_uInt16 nLvl = aSortArr[ nCnt ]->GetLevel();
        SwTxtFmtColl* pColl = aCollArr[ nLvl ];
        if( !pColl )
        {
            pColl = GetTxtFmtColl( nLvl );
            aCollArr[ nLvl ] = pColl;
        }

        // Generate: Set dynamic TabStops
        SwTxtNode* pTOXNd = pDoc->GetNodes().MakeTxtNode( aInsPos , pColl );
        aSortArr[ nCnt ]->pTOXNd = pTOXNd;

        // Generate: Evaluate Form and insert the place holder for the
        // page number. If it is a TOX_INDEX and the SwForm IsCommaSeparated()
        // then a range of entries must be generated into one paragraph
        size_t nRange = 1;
        if(TOX_INDEX == SwTOXBase::GetType() &&
                GetTOXForm().IsCommaSeparated() &&
                aSortArr[nCnt]->GetType() == TOX_SORT_INDEX)
        {
            const SwTOXMark& rMark = aSortArr[nCnt]->pTxtMark->GetTOXMark();
            const OUString sPrimKey = rMark.GetPrimaryKey();
            const OUString sSecKey = rMark.GetSecondaryKey();
            const SwTOXMark* pNextMark = 0;
            while(aSortArr.size() > (nCnt + nRange)&&
                    aSortArr[nCnt + nRange]->GetType() == TOX_SORT_INDEX &&
                    0 != (pNextMark = &(aSortArr[nCnt + nRange]->pTxtMark->GetTOXMark())) &&
                    pNextMark->GetPrimaryKey() == sPrimKey &&
                    pNextMark->GetSecondaryKey() == sSecKey)
                nRange++;
        }
        // pass node index of table-of-content section and default page description
        // to method <GenerateText(..)>.
        ::SetProgressState( 0, pDoc->GetDocShell() );

        boost::shared_ptr<sw::ToxTabStopTokenHandler> tabStopTokenHandler =
                boost::make_shared<sw::DefaultToxTabStopTokenHandler>(
                        pSectNd->GetIndex(), *pDefaultPageDesc, GetTOXForm().IsRelTabPos(),
                        pDoc->GetDocumentSettingManager().get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT) ?
                                sw::DefaultToxTabStopTokenHandler::TABSTOPS_RELATIVE_TO_INDENT :
                                sw::DefaultToxTabStopTokenHandler::TABSTOPS_RELATIVE_TO_PAGE);
        sw::ToxTextGenerator ttgn(GetTOXForm(), tabStopTokenHandler);
        ttgn.GenerateText((SwDoc*) GetFmt()->GetDoc(), aSortArr, nCnt, nRange);
        nCnt += nRange - 1;
    }

    // delete the first dummy node and remove all Cursor into the previous node
    aInsPos = *pFirstEmptyNd;
    {
        SwPaM aCorPam( *pFirstEmptyNd );
        aCorPam.GetPoint()->nContent.Assign( pFirstEmptyNd, 0 );
        if( !aCorPam.Move( fnMoveForward ) )
            aCorPam.Move( fnMoveBackward );
        SwNodeIndex aEndIdx( aInsPos, 1 );
        pDoc->CorrAbs( aInsPos, aEndIdx, *aCorPam.GetPoint(), true );

        // Task 70995 - save and restore PageDesc and Break Attributes
        if( pFirstEmptyNd->HasSwAttrSet() )
        {
            if( !GetTitle().isEmpty() )
                aEndIdx = *pSectNd;
            else
                aEndIdx = *pFirstEmptyNd;
            SwCntntNode* pCNd = pDoc->GetNodes().GoNext( &aEndIdx );
            if( pCNd ) // Robust against defect documents, e.g. i60336
                pCNd->SetAttr( *pFirstEmptyNd->GetpSwAttrSet() );
        }
    }

    // now create the new Frames
    sal_uLong nIdx = pSectNd->GetIndex();
    // don't delete if index is empty
    if(nIdx + 2 < pSectNd->EndOfSectionIndex())
        pDoc->GetNodes().Delete( aInsPos, 1 );

    aN2L.RestoreUpperFrms( pDoc->GetNodes(), nIdx, nIdx + 1 );
    std::set<SwRootFrm*> aAllLayouts = pDoc->GetAllLayouts();
    for ( std::set<SwRootFrm*>::iterator pLayoutIter = aAllLayouts.begin(); pLayoutIter != aAllLayouts.end(); ++pLayoutIter)
    {
        SwFrm::CheckPageDescs( static_cast<SwPageFrm*>((*pLayoutIter)->Lower()) );
    }

    SetProtect( SwTOXBase::IsProtected() );
}

void SwTOXBaseSection::InsertAlphaDelimitter( const SwTOXInternational& rIntl )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    OUString sDeli, sLastDeli;
    sal_uInt16  i = 0;
    while( i < aSortArr.size() )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        sal_uInt16 nLevel = aSortArr[i]->GetLevel();

        // Skip AlphaDelimitter
        if( nLevel == FORM_ALPHA_DELIMITTER )
            continue;

        sDeli = rIntl.GetIndexKey( aSortArr[i]->GetTxt(),
                                   aSortArr[i]->GetLocale() );

        // Do we already have a Delimitter?
        if( !sDeli.isEmpty() && sLastDeli != sDeli )
        {
            // We skip all that are less than a small Blank (these are special characters)
            if( ' ' <= sDeli[0] )
            {
                SwTOXCustom* pCst = new SwTOXCustom( TextAndReading(sDeli, OUString()),
                                                     FORM_ALPHA_DELIMITTER,
                                                     rIntl, aSortArr[i]->GetLocale() );
                aSortArr.insert( aSortArr.begin() + i, pCst);
                i++;
            }
            sLastDeli = sDeli;
        }

        // Skip until we get to the same or a lower Level
        do {
            i++;
        } while (i < aSortArr.size() && aSortArr[i]->GetLevel() > nLevel);
    }
}

/// Evaluate Template
SwTxtFmtColl* SwTOXBaseSection::GetTxtFmtColl( sal_uInt16 nLevel )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    const OUString& rName = GetTOXForm().GetTemplate( nLevel );
    SwTxtFmtColl* pColl = !rName.isEmpty() ? pDoc->FindTxtFmtCollByName(rName) :0;
    if( !pColl )
    {
        sal_uInt16 nPoolFmt = 0;
        const TOXTypes eMyType = SwTOXBase::GetType();
        switch( eMyType )
        {
        case TOX_INDEX:         nPoolFmt = RES_POOLCOLL_TOX_IDXH;       break;
        case TOX_USER:
            if( nLevel < 6 )
                nPoolFmt = RES_POOLCOLL_TOX_USERH;
            else
                nPoolFmt = RES_POOLCOLL_TOX_USER6 - 6;
            break;
        case TOX_ILLUSTRATIONS: nPoolFmt = RES_POOLCOLL_TOX_ILLUSH;     break;
        case TOX_OBJECTS:       nPoolFmt = RES_POOLCOLL_TOX_OBJECTH;    break;
        case TOX_TABLES:        nPoolFmt = RES_POOLCOLL_TOX_TABLESH;    break;
        case TOX_AUTHORITIES:
        case TOX_BIBLIOGRAPHY:
            nPoolFmt = RES_POOLCOLL_TOX_AUTHORITIESH; break;
        case  TOX_CITATION: /** TODO */break;
        case TOX_CONTENT:
            // There's a jump in the ContentArea!
            if( nLevel < 6 )
                nPoolFmt = RES_POOLCOLL_TOX_CNTNTH;
            else
                nPoolFmt = RES_POOLCOLL_TOX_CNTNT6 - 6;
            break;
        }

        if(eMyType == TOX_AUTHORITIES && nLevel)
            nPoolFmt = nPoolFmt + 1;
        else if(eMyType == TOX_INDEX && nLevel)
        {
            // pool: Level 1,2,3, Delimiter
            // SwForm: Delimiter, Level 1,2,3
            nPoolFmt += 1 == nLevel ? nLevel + 3 : nLevel - 1;
        }
        else
            nPoolFmt = nPoolFmt + nLevel;
        pColl = pDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool( nPoolFmt );
    }
    return pColl;
}

/// Create from Marks
void SwTOXBaseSection::UpdateMarks( const SwTOXInternational& rIntl,
                                    const SwTxtNode* pOwnChapterNode )
{
    const SwTOXType* pType = static_cast<SwTOXType*>( SwTOXBase::GetRegisteredIn() );
    if( !pType->GetDepends() )
        return;

    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    TOXTypes eTOXTyp = GetTOXType()->GetType();
    SwIterator<SwTOXMark,SwTOXType> aIter( *pType );

    SwTxtTOXMark* pTxtMark;
    SwTOXMark* pMark;
    for( pMark = aIter.First(); pMark; pMark = aIter.Next() )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        if( pMark->GetTOXType()->GetType() == eTOXTyp &&
            0 != ( pTxtMark = pMark->GetTxtTOXMark() ) )
        {
            const SwTxtNode* pTOXSrc = pTxtMark->GetpTxtNd();
            // Only insert TOXMarks from the Doc, not from the
            // UNDO.

            // If selected use marks from the same chapter only
            if( pTOXSrc->GetNodes().IsDocNodes() &&
                pTOXSrc->GetTxt().getLength() && pTOXSrc->GetDepends() &&
                pTOXSrc->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() ) &&
               (!IsFromChapter() || ::lcl_FindChapterNode( *pTOXSrc, 0 ) == pOwnChapterNode ) &&
               !pTOXSrc->HasHiddenParaField() &&
               !SwScriptInfo::IsInHiddenRange( *pTOXSrc, pTxtMark->GetStart() ) )
            {
                SwTOXSortTabBase* pBase = 0;
                if(TOX_INDEX == eTOXTyp)
                {
                    // index entry mark
                    lang::Locale aLocale;
                    if ( g_pBreakIt->GetBreakIter().is() )
                    {
                        aLocale = g_pBreakIt->GetLocale(
                                        pTOXSrc->GetLang( pTxtMark->GetStart() ) );
                    }

                    pBase = new SwTOXIndex( *pTOXSrc, pTxtMark,
                                            GetOptions(), FORM_ENTRY, rIntl, aLocale );
                    InsertSorted(pBase);
                    if(GetOptions() & nsSwTOIOptions::TOI_KEY_AS_ENTRY &&
                        !pTxtMark->GetTOXMark().GetPrimaryKey().isEmpty())
                    {
                        pBase = new SwTOXIndex( *pTOXSrc, pTxtMark,
                                                GetOptions(), FORM_PRIMARY_KEY, rIntl, aLocale );
                        InsertSorted(pBase);
                        if (!pTxtMark->GetTOXMark().GetSecondaryKey().isEmpty())
                        {
                            pBase = new SwTOXIndex( *pTOXSrc, pTxtMark,
                                                    GetOptions(), FORM_SECONDARY_KEY, rIntl, aLocale );
                            InsertSorted(pBase);
                        }
                    }
                }
                else if( TOX_USER == eTOXTyp ||
                    pMark->GetLevel() <= GetLevel())
                {   // table of content mark
                    // also used for user marks
                    pBase = new SwTOXContent( *pTOXSrc, pTxtMark, rIntl );
                    InsertSorted(pBase);
                }
            }
        }
    }
}

/// Generate table of contents from outline
void SwTOXBaseSection::UpdateOutline( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();

    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    for( sal_uInt16 n = 0; n < rOutlNds.size(); ++n )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );
        SwTxtNode* pTxtNd = rOutlNds[ n ]->GetTxtNode();
        if( pTxtNd && pTxtNd->Len() && pTxtNd->GetDepends() &&
            sal_uInt16( pTxtNd->GetAttrOutlineLevel()) <= GetLevel() &&
            pTxtNd->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() ) &&
           !pTxtNd->HasHiddenParaField() &&
           !pTxtNd->HasHiddenCharAttribute( true ) &&
            ( !IsFromChapter() ||
               ::lcl_FindChapterNode( *pTxtNd, 0 ) == pOwnChapterNode ))
        {
            SwTOXPara * pNew = new SwTOXPara( *pTxtNd, nsSwTOXElement::TOX_OUTLINELEVEL );
            InsertSorted( pNew );
        }
    }
}

/// Generate table of contents from template areas
void SwTOXBaseSection::UpdateTemplate( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        OUString sTmpStyleNames = GetStyleNames(i);
        sal_uInt16 nTokenCount = comphelper::string::getTokenCount(sTmpStyleNames, TOX_STYLE_DELIMITER);
        for( sal_uInt16 nStyle = 0; nStyle < nTokenCount; ++nStyle )
        {
            SwTxtFmtColl* pColl = pDoc->FindTxtFmtCollByName(
                                    sTmpStyleNames.getToken( nStyle,
                                                    TOX_STYLE_DELIMITER ));
            //TODO: no outline Collections in content indexes if OutlineLevels are already included
            if( !pColl ||
                ( TOX_CONTENT == SwTOXBase::GetType() &&
                  GetCreateType() & nsSwTOXElement::TOX_OUTLINELEVEL &&
                    pColl->IsAssignedToListLevelOfOutlineStyle()) )
                  continue;

            SwIterator<SwTxtNode,SwFmtColl> aIter( *pColl );
            for( SwTxtNode* pTxtNd = aIter.First(); pTxtNd; pTxtNd = aIter.Next() )
            {
                ::SetProgressState( 0, pDoc->GetDocShell() );

                if (pTxtNd->GetTxt().getLength() &&
                    pTxtNd->getLayoutFrm(pDoc->getIDocumentLayoutAccess().GetCurrentLayout()) &&
                    pTxtNd->GetNodes().IsDocNodes() &&
                    ( !IsFromChapter() || pOwnChapterNode ==
                        ::lcl_FindChapterNode( *pTxtNd, 0 ) ) )
                {
                    SwTOXPara * pNew = new SwTOXPara( *pTxtNd, nsSwTOXElement::TOX_TEMPLATE, i + 1 );
                    InsertSorted(pNew);
                }
            }
        }
    }
}

/// Generate content from sequence fields
void SwTOXBaseSection::UpdateSequence( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwFieldType* pSeqFld = pDoc->getIDocumentFieldsAccess().GetFldType(RES_SETEXPFLD, GetSequenceName(), false);
    if(!pSeqFld)
        return;

    SwIterator<SwFmtFld,SwFieldType> aIter( *pSeqFld );
    for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
    {
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if(!pTxtFld)
            continue;
        const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
        ::SetProgressState( 0, pDoc->GetDocShell() );

        if (rTxtNode.GetTxt().getLength() &&
            rTxtNode.getLayoutFrm(pDoc->getIDocumentLayoutAccess().GetCurrentLayout()) &&
            rTxtNode.GetNodes().IsDocNodes() &&
            ( !IsFromChapter() ||
                ::lcl_FindChapterNode( rTxtNode, 0 ) == pOwnChapterNode ) )
        {
            const SwSetExpField& rSeqField = dynamic_cast<const SwSetExpField&>(*(pFmtFld->GetField()));
            OUString sName = GetSequenceName();
            sName += OUString( cSequenceMarkSeparator );
            sName += OUString::number( rSeqField.GetSeqNumber() );
            SwTOXPara * pNew = new SwTOXPara( rTxtNode, nsSwTOXElement::TOX_SEQUENCE, 1, sName );
            // set indexes if the number or the reference text are to be displayed
            if( GetCaptionDisplay() == CAPTION_TEXT )
            {
                pNew->SetStartIndex(
                    SwGetExpField::GetReferenceTextPos( *pFmtFld, *pDoc ));
            }
            else if(GetCaptionDisplay() == CAPTION_NUMBER)
            {
                pNew->SetEndIndex(pTxtFld->GetStart() + 1);
            }
            InsertSorted(pNew);
        }
    }
}

void SwTOXBaseSection::UpdateAuthorities( const SwTOXInternational& rIntl )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwFieldType* pAuthFld = pDoc->getIDocumentFieldsAccess().GetFldType(RES_AUTHORITY, OUString(), false);
    if(!pAuthFld)
        return;

    SwIterator<SwFmtFld,SwFieldType> aIter( *pAuthFld );
    for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
    {
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        // undo
        if(!pTxtFld)
            continue;
        const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
        ::SetProgressState( 0, pDoc->GetDocShell() );

        if (rTxtNode.GetTxt().getLength() &&
            rTxtNode.getLayoutFrm(pDoc->getIDocumentLayoutAccess().GetCurrentLayout()) &&
            rTxtNode.GetNodes().IsDocNodes() )
        {
            //#106485# the body node has to be used!
            SwCntntFrm *pFrm = rTxtNode.getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
            SwPosition aFldPos(rTxtNode);
            const SwTxtNode* pTxtNode = 0;
            if(pFrm && !pFrm->IsInDocBody())
                pTxtNode = GetBodyTxtNode( *pDoc, aFldPos, *pFrm );
            if(!pTxtNode)
                pTxtNode = &rTxtNode;
            SwTOXAuthority* pNew = new SwTOXAuthority( *pTxtNode, *pFmtFld, rIntl );

            InsertSorted(pNew);
        }
    }
}

static long lcl_IsSOObject( const SvGlobalName& rFactoryNm )
{
    static struct _SoObjType {
        long nFlag;
        // GlobalNameId
        struct _GlobalNameIds {
            sal_uInt32 n1;
            sal_uInt16 n2, n3;
            sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
        } aGlNmIds[4];
    } aArr[] = {
        { nsSwTOOElements::TOO_MATH,
          { {SO3_SM_CLASSID_60},{SO3_SM_CLASSID_50},
            {SO3_SM_CLASSID_40},{SO3_SM_CLASSID_30} } },
        { nsSwTOOElements::TOO_CHART,
          { {SO3_SCH_CLASSID_60},{SO3_SCH_CLASSID_50},
            {SO3_SCH_CLASSID_40},{SO3_SCH_CLASSID_30} } },
        { nsSwTOOElements::TOO_CALC,
          { {SO3_SC_CLASSID_60},{SO3_SC_CLASSID_50},
            {SO3_SC_CLASSID_40},{SO3_SC_CLASSID_30} } },
        { nsSwTOOElements::TOO_DRAW_IMPRESS,
          { {SO3_SIMPRESS_CLASSID_60},{SO3_SIMPRESS_CLASSID_50},
            {SO3_SIMPRESS_CLASSID_40},{SO3_SIMPRESS_CLASSID_30} } },
        { nsSwTOOElements::TOO_DRAW_IMPRESS,
          { {SO3_SDRAW_CLASSID_60},{SO3_SDRAW_CLASSID_50}}},
        { 0,{{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0} } }
    };

    long nRet = 0;
    for( const _SoObjType* pArr = aArr; !nRet && pArr->nFlag; ++pArr )
        for ( int n = 0; n < 4; ++n )
        {
            const _SoObjType::_GlobalNameIds& rId = pArr->aGlNmIds[ n ];
            if( !rId.n1 )
                break;
            SvGlobalName aGlbNm( rId.n1, rId.n2, rId.n3,
                        rId.b8, rId.b9, rId.b10, rId.b11,
                        rId.b12, rId.b13, rId.b14, rId.b15 );
            if( rFactoryNm == aGlbNm )
            {
                nRet = pArr->nFlag;
                break;
            }
        }

    return nRet;
}

void SwTOXBaseSection::UpdateCntnt( SwTOXElement eMyType,
                                    const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();
    // on the 1st Node of the 1st Section
    sal_uLong nIdx = rNds.GetEndOfAutotext().StartOfSectionIndex() + 2,
         nEndIdx = rNds.GetEndOfAutotext().GetIndex();

    while( nIdx < nEndIdx )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        SwNode* pNd = rNds[ nIdx ];
        SwCntntNode* pCNd = 0;
        switch( eMyType )
        {
        case nsSwTOXElement::TOX_FRAME:
            if( !pNd->IsNoTxtNode() )
            {
                pCNd = pNd->GetCntntNode();
                if( !pCNd )
                {
                    SwNodeIndex aTmp( *pNd );
                    pCNd = rNds.GoNext( &aTmp );
                }
            }
            break;
        case nsSwTOXElement::TOX_GRAPHIC:
            if( pNd->IsGrfNode() )
                pCNd = static_cast<SwCntntNode*>(pNd);
            break;
        case nsSwTOXElement::TOX_OLE:
            if( pNd->IsOLENode() )
            {
                bool bInclude = true;
                if(TOX_OBJECTS == SwTOXBase::GetType())
                {
                    SwOLENode* pOLENode = pNd->GetOLENode();
                    long nMyOLEOptions = GetOLEOptions();
                    SwOLEObj& rOLEObj = pOLENode->GetOLEObj();

                    if( rOLEObj.IsOleRef() )    // Not yet loaded
                    {
                        SvGlobalName aTmpName = SvGlobalName( rOLEObj.GetOleRef()->getClassID() );
                        long nObj = ::lcl_IsSOObject( aTmpName );
                        bInclude = ( (nMyOLEOptions & nsSwTOOElements::TOO_OTHER) && 0 == nObj)
                                                    || (0 != (nMyOLEOptions & nObj));
                    }
                    else
                    {
                        OSL_FAIL("OLE Object no loaded?");
                        bInclude = false;
                    }
                }

                if(bInclude)
                    pCNd = static_cast<SwCntntNode*>(pNd);
            }
            break;
        default: break;
        }

        if( pCNd )
        {
            // find node in body text
            int nSetLevel = USHRT_MAX;

            //#111105# tables of tables|illustrations|objects don't support hierarchies
            if( IsLevelFromChapter() &&
                    TOX_TABLES != SwTOXBase::GetType() &&
                    TOX_ILLUSTRATIONS != SwTOXBase::GetType() &&
                    TOX_OBJECTS != SwTOXBase::GetType() )
            {
                const SwTxtNode* pOutlNd = ::lcl_FindChapterNode( *pCNd,
                                                        MAXLEVEL - 1 );
                if( pOutlNd )
                {
                    if( pOutlNd->GetTxtColl()->IsAssignedToListLevelOfOutlineStyle())
                    {
                        nSetLevel = pOutlNd->GetTxtColl()->GetAttrOutlineLevel();
                    }
                }
            }

            if( pCNd->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() ) && ( !IsFromChapter() ||
                    ::lcl_FindChapterNode( *pCNd, 0 ) == pOwnChapterNode ))
            {
                SwTOXPara * pNew = new SwTOXPara( *pCNd, eMyType,
                            ( USHRT_MAX != nSetLevel )
                            ? static_cast<sal_uInt16>(nSetLevel)
                            : FORM_ALPHA_DELIMITTER );
                InsertSorted( pNew );
            }
        }

        nIdx = pNd->StartOfSectionNode()->EndOfSectionIndex() + 2;  // 2 == End/Start Node
    }
}

/// Collect table entries
void SwTOXBaseSection::UpdateTable( const SwTxtNode* pOwnChapterNode )
{
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();
    const SwFrmFmts& rArr = *pDoc->GetTblFrmFmts();

    for( sal_uInt16 n = 0; n < rArr.size(); ++n )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        SwTable* pTmpTbl = SwTable::FindTable( rArr[ n ] );
        SwTableBox* pFBox;
        if( pTmpTbl && 0 != (pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() && pFBox->GetSttNd()->GetNodes().IsDocNodes() )
        {
            const SwTableNode* pTblNd = pFBox->GetSttNd()->FindTableNode();
            SwNodeIndex aCntntIdx( *pTblNd, 1 );

            SwCntntNode* pCNd;
            while( 0 != ( pCNd = rNds.GoNext( &aCntntIdx ) ) &&
                aCntntIdx.GetIndex() < pTblNd->EndOfSectionIndex() )
            {
                if( pCNd->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() ) && (!IsFromChapter() ||
                    ::lcl_FindChapterNode( *pCNd, 0 ) == pOwnChapterNode ))
                {
                    SwTOXTable * pNew = new SwTOXTable( *pCNd );
                    if( IsLevelFromChapter() && TOX_TABLES != SwTOXBase::GetType())
                    {
                        const SwTxtNode* pOutlNd =
                            ::lcl_FindChapterNode( *pCNd, MAXLEVEL - 1 );
                        if( pOutlNd )
                        {
                            if( pOutlNd->GetTxtColl()->IsAssignedToListLevelOfOutlineStyle())
                            {
                                const int nTmp = pOutlNd->GetTxtColl()->GetAttrOutlineLevel();
                                pNew->SetLevel(static_cast<sal_uInt16>(nTmp));
                            }
                        }
                    }
                    InsertSorted(pNew);
                    break;
                }
            }
        }
    }
}

/// Calculate PageNumber and insert after formatting
void SwTOXBaseSection::UpdatePageNum()
{
    if( aSortArr.empty() )
        return ;

    // Insert the current PageNumber into the TOC
    SwPageFrm*  pAktPage    = 0;
    sal_uInt16      nPage       = 0;
    SwDoc* pDoc = (SwDoc*)GetFmt()->GetDoc();

    SwTOXInternational aIntl( GetLanguage(),
                              TOX_INDEX == GetTOXType()->GetType() ?
                              GetOptions() : 0,
                              GetSortAlgorithm() );

    for( size_t nCnt = 0; nCnt < aSortArr.size(); ++nCnt )
    {
        // Loop over all SourceNodes
        std::vector<sal_uInt16> aNums; // the PageNumber
        std::vector<SwPageDesc*> aDescs;        // The PageDescriptors matching the PageNumbers
        std::vector<sal_uInt16> *pMainNums = 0; // contains page numbers of main entries

        // process run in lines
        sal_uInt16 nRange = 0;
        if(GetTOXForm().IsCommaSeparated() &&
                aSortArr[nCnt]->GetType() == TOX_SORT_INDEX)
        {
            const SwTOXMark& rMark = aSortArr[nCnt]->pTxtMark->GetTOXMark();
            const OUString sPrimKey = rMark.GetPrimaryKey();
            const OUString sSecKey = rMark.GetSecondaryKey();
            const SwTOXMark* pNextMark = 0;
            while(aSortArr.size() > (nCnt + nRange)&&
                    aSortArr[nCnt + nRange]->GetType() == TOX_SORT_INDEX &&
                    0 != (pNextMark = &(aSortArr[nCnt + nRange]->pTxtMark->GetTOXMark())) &&
                    pNextMark->GetPrimaryKey() == sPrimKey &&
                    pNextMark->GetSecondaryKey() == sSecKey)
                nRange++;
        }
        else
            nRange = 1;

        for(sal_uInt16 nRunInEntry = nCnt; nRunInEntry < nCnt + nRange; nRunInEntry++)
        {
            SwTOXSortTabBase* pSortBase = aSortArr[nRunInEntry];
            size_t nSize = pSortBase->aTOXSources.size();
            for (size_t j = 0; j < nSize; ++j)
            {
                ::SetProgressState( 0, pDoc->GetDocShell() );

                SwTOXSource& rTOXSource = pSortBase->aTOXSources[j];
                if( rTOXSource.pNd )
                {
                    SwCntntFrm* pFrm = rTOXSource.pNd->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
                    OSL_ENSURE( pFrm || pDoc->IsUpdateTOX(), "TOX, no Frame found");
                    if( !pFrm )
                        continue;
                    if( pFrm->IsTxtFrm() && static_cast<SwTxtFrm*>(pFrm)->HasFollow() )
                    {
                        // find the right one
                        SwTxtFrm* pNext = static_cast<SwTxtFrm*>(pFrm);
                        while( 0 != ( pNext = static_cast<SwTxtFrm*>(pFrm->GetFollow()) )
                                && rTOXSource.nPos >= pNext->GetOfst() )
                            pFrm = pNext;
                    }

                    SwPageFrm*  pTmpPage = pFrm->FindPageFrm();
                    if( pTmpPage != pAktPage )
                    {
                        nPage       = pTmpPage->GetVirtPageNum();
                        pAktPage    = pTmpPage;
                    }

                    // Insert as sorted
                    sal_uInt16 i;
                    for( i = 0; i < aNums.size() && aNums[i] < nPage; ++i )
                        ;

                    if( i >= aNums.size() || aNums[ i ] != nPage )
                    {
                        aNums.insert(aNums.begin() + i, nPage);
                        aDescs.insert(aDescs.begin() + i, pAktPage->GetPageDesc() );
                    }
                    // is it a main entry?
                    if(TOX_SORT_INDEX == pSortBase->GetType() &&
                        rTOXSource.bMainEntry)
                    {
                        if(!pMainNums)
                            pMainNums = new std::vector<sal_uInt16>;
                        pMainNums->push_back(nPage);
                    }
                }
            }
            // Insert the PageNumber into the TOC TextNode
            const SwTOXSortTabBase* pBase = aSortArr[ nCnt ];
            if(pBase->pTOXNd)
            {
                const SwTxtNode* pTxtNd = pBase->pTOXNd->GetTxtNode();
                OSL_ENSURE( pTxtNd, "no TextNode, wrong TOC" );

                _UpdatePageNum( (SwTxtNode*)pTxtNd, aNums, aDescs, pMainNums,
                                aIntl );
            }
            DELETEZ(pMainNums);
            aNums.clear();
        }
    }
    // Delete the mapping array after setting the right PageNumber
    for (SwTOXSortTabBases::const_iterator it = aSortArr.begin(); it != aSortArr.end(); ++it)
        delete *it;
    aSortArr.clear();
}

/// Replace the PageNumber place holders. Search for the page no. in the array
/// of main entry page numbers.
static bool lcl_HasMainEntry( const std::vector<sal_uInt16>* pMainEntryNums, sal_uInt16 nToFind )
{
    for(sal_uInt16 i = 0; pMainEntryNums && i < pMainEntryNums->size(); ++i)
        if(nToFind == (*pMainEntryNums)[i])
            return true;
    return false;
}

void SwTOXBaseSection::_UpdatePageNum( SwTxtNode* pNd,
                                    const std::vector<sal_uInt16>& rNums,
                                    const std::vector<SwPageDesc*>& rDescs,
                                    const std::vector<sal_uInt16>* pMainEntryNums,
                                    const SwTOXInternational& rIntl )
{
    // collect starts end ends of main entry character style
    boost::scoped_ptr< std::vector<sal_uInt16> > xCharStyleIdx(pMainEntryNums ? new std::vector<sal_uInt16> : 0);

    OUString sSrchStr = OUStringBuffer().append(C_NUM_REPL).
        append(S_PAGE_DELI).append(C_NUM_REPL).makeStringAndClear();
    sal_Int32 nStartPos = pNd->GetTxt().indexOf(sSrchStr);
    sSrchStr = OUStringBuffer().append(C_NUM_REPL).
        append(C_END_PAGE_NUM).makeStringAndClear();
    sal_Int32 nEndPos = pNd->GetTxt().indexOf(sSrchStr);
    sal_uInt16 i;

    if (-1 == nEndPos || rNums.empty())
        return;

    if (-1 == nStartPos || nStartPos > nEndPos)
        nStartPos = nEndPos;

    sal_uInt16 nOld = rNums[0],
           nBeg = nOld,
           nCount  = 0;
    OUString aNumStr( SvxNumberType( rDescs[0]->GetNumType() ).
                    GetNumStr( nBeg ) );
    if( xCharStyleIdx && lcl_HasMainEntry( pMainEntryNums, nBeg ))
    {
        sal_uInt16 nTemp = 0;
        xCharStyleIdx->push_back( nTemp );
    }

    // Delete place holder
    SwIndex aPos(pNd, nStartPos);
    SwCharFmt* pPageNoCharFmt = 0;
    SwpHints* pHints = pNd->GetpSwpHints();
    if(pHints)
        for(size_t nHintIdx = 0; nHintIdx < pHints->GetStartCount(); ++nHintIdx)
        {
            SwTxtAttr* pAttr = pHints->GetStart(nHintIdx);
            const sal_Int32 nTmpEnd = pAttr->End() ? *pAttr->End() : 0;
            if( nStartPos >= pAttr->GetStart() &&
                (nStartPos + 2) <= nTmpEnd &&
                pAttr->Which() == RES_TXTATR_CHARFMT)
            {
                pPageNoCharFmt = pAttr->GetCharFmt().GetCharFmt();
                break;
            }
        }
    pNd->EraseText(aPos, nEndPos - nStartPos + 2);

    for( i = 1; i < rNums.size(); ++i)
    {
        SvxNumberType aType( rDescs[i]->GetNumType() );
        if( TOX_INDEX == SwTOXBase::GetType() )
        {   // Summarize for the following
            // Add up all following
            // break up if main entry starts or ends and
            // insert a char style index
            bool bMainEntryChanges = lcl_HasMainEntry(pMainEntryNums, nOld)
                    != lcl_HasMainEntry(pMainEntryNums, rNums[i]);

            if(nOld == rNums[i]-1 && !bMainEntryChanges &&
                0 != (GetOptions() & (nsSwTOIOptions::TOI_FF|nsSwTOIOptions::TOI_DASH)))
                nCount++;
            else
            {
                // Flush for the following old values
                if(GetOptions() & nsSwTOIOptions::TOI_FF)
                {
                    if ( nCount >= 1 )
                        aNumStr += rIntl.GetFollowingText( nCount > 1 );
                }
                else
                {
                    if(nCount >= 2 )
                        aNumStr += "-";
                    else if(nCount == 1 )
                        aNumStr += S_PAGE_DELI;
                    //#58127# If nCount == 0, then the only PageNumber is already in aNumStr!
                    if(nCount)
                        aNumStr += aType.GetNumStr( nBeg + nCount );
                }

                // Create new String
                nBeg     = rNums[i];
                aNumStr += S_PAGE_DELI;
                //the change of the character style must apply after sPageDeli is appended
                if (xCharStyleIdx && bMainEntryChanges)
                {
                    xCharStyleIdx->push_back(aNumStr.getLength());
                }
                aNumStr += aType.GetNumStr( nBeg );
                nCount   = 0;
            }
            nOld = rNums[i];
        }
        else
        {   // Insert all Numbers
            aNumStr += aType.GetNumStr( sal_uInt16(rNums[i]) );
            if(i != (rNums.size()-1))
                aNumStr += S_PAGE_DELI;
        }
    }
    // Flush when ending and the following old values
    if( TOX_INDEX == SwTOXBase::GetType() )
    {
        if(GetOptions() & nsSwTOIOptions::TOI_FF)
        {
            if( nCount >= 1 )
                aNumStr += rIntl.GetFollowingText( nCount > 1 );
        }
        else
        {
            if(nCount >= 2)
                aNumStr += "-";
            else if(nCount == 1)
                aNumStr += S_PAGE_DELI;
            //#58127# If nCount == 0, then the only PageNumber is already in aNumStr!
            if(nCount)
                aNumStr += SvxNumberType( rDescs[i-1]->GetNumType() ).GetNumStr( nBeg+nCount );
        }
    }
    pNd->InsertText( aNumStr, aPos,
           static_cast<IDocumentContentOperations::InsertFlags>(
               IDocumentContentOperations::INS_EMPTYEXPAND |
               IDocumentContentOperations::INS_FORCEHINTEXPAND) );
    if(pPageNoCharFmt)
    {
        SwFmtCharFmt aCharFmt( pPageNoCharFmt );
        pNd->InsertItem(aCharFmt, nStartPos, nStartPos + aNumStr.getLength(), nsSetAttrMode::SETATTR_DONTEXPAND);
    }

    // The main entries should get their character style
    if (xCharStyleIdx && !xCharStyleIdx->empty() && !GetMainEntryCharStyle().isEmpty())
    {
        // eventually the last index must me appended
        if (xCharStyleIdx->size()&0x01)
            xCharStyleIdx->push_back(aNumStr.getLength());

        // search by name
        SwDoc* pDoc = pNd->GetDoc();
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( GetMainEntryCharStyle(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
        SwCharFmt* pCharFmt = 0;
        if(USHRT_MAX != nPoolId)
            pCharFmt = pDoc->getIDocumentStylePoolAccess().GetCharFmtFromPool(nPoolId);
        else
            pCharFmt = pDoc->FindCharFmtByName( GetMainEntryCharStyle() );
        if(!pCharFmt)
            pCharFmt = pDoc->MakeCharFmt(GetMainEntryCharStyle(), 0);

        // find the page numbers in aNumStr and set the character style
        sal_Int32 nOffset = pNd->GetTxt().getLength() - aNumStr.getLength();
        SwFmtCharFmt aCharFmt(pCharFmt);
        for (sal_uInt16 j = 0; j < xCharStyleIdx->size(); j += 2)
        {
            sal_Int32 nStartIdx = (*xCharStyleIdx)[j] + nOffset;
            sal_Int32 nEndIdx   = (*xCharStyleIdx)[j + 1]  + nOffset;
            pNd->InsertItem(aCharFmt, nStartIdx, nEndIdx, nsSetAttrMode::SETATTR_DONTEXPAND);
        }

    }
}

void SwTOXBaseSection::InsertSorted(SwTOXSortTabBase* pNew)
{
    Range aRange(0, aSortArr.size());
    if( TOX_INDEX == SwTOXBase::GetType() && pNew->pTxtMark )
    {
        const SwTOXMark& rMark = pNew->pTxtMark->GetTOXMark();
        // Evaluate Key
        // Calculate the range where to insert
        if( 0 == (GetOptions() & nsSwTOIOptions::TOI_KEY_AS_ENTRY) &&
            !rMark.GetPrimaryKey().isEmpty() )
        {
            aRange = GetKeyRange( rMark.GetPrimaryKey(),
                                  rMark.GetPrimaryKeyReading(),
                                  *pNew, FORM_PRIMARY_KEY, aRange );

            if( !rMark.GetSecondaryKey().isEmpty() )
                aRange = GetKeyRange( rMark.GetSecondaryKey(),
                                      rMark.GetSecondaryKeyReading(),
                                      *pNew, FORM_SECONDARY_KEY, aRange );
        }
    }
    // Search for identical entries and remove the trailing one
    if(TOX_AUTHORITIES == SwTOXBase::GetType())
    {
        for(short i = (short)aRange.Min(); i < (short)aRange.Max(); ++i)
        {
            SwTOXSortTabBase* pOld = aSortArr[i];
            if(*pOld == *pNew)
            {
                if(*pOld < *pNew)
                {
                    delete pNew;
                    return;
                }
                else
                {
                    // remove the old content
                    delete aSortArr[i];
                    aSortArr.erase( aSortArr.begin() + i );
                    aRange.Max()--;
                    break;
                }
            }
        }
    }

    // find position and insert
    short i;

    for( i = (short)aRange.Min(); i < (short)aRange.Max(); ++i)
    {   // Only check for same level
        SwTOXSortTabBase* pOld = aSortArr[i];
        if(*pOld == *pNew)
        {
            if(TOX_AUTHORITIES != SwTOXBase::GetType())
            {
                // Own entry for double entries or keywords
                if( pOld->GetType() == TOX_SORT_CUSTOM &&
                    pNew->GetOptions() & nsSwTOIOptions::TOI_KEY_AS_ENTRY)
                    continue;

                if(!(pNew->GetOptions() & nsSwTOIOptions::TOI_SAME_ENTRY))
                {   // Own entry
                    aSortArr.insert(aSortArr.begin() + i, pNew);
                    return;
                }
                // If the own entry is already present, add it to the references list
                pOld->aTOXSources.push_back(pNew->aTOXSources[0]);

                delete pNew;
                return;
            }
#if OSL_DEBUG_LEVEL > 0
            else
                OSL_FAIL("Bibliography entries cannot be found here");
#endif
        }
        if(*pNew < *pOld)
            break;
    }
    // Skip SubLevel
    while( TOX_INDEX == SwTOXBase::GetType() && i < aRange.Max() &&
          aSortArr[i]->GetLevel() > pNew->GetLevel() )
        i++;

    // Insert at position i
    aSortArr.insert(aSortArr.begin()+i, pNew);
}

/// Find Key Range and insert if possible
Range SwTOXBaseSection::GetKeyRange(const OUString& rStr, const OUString& rStrReading,
                                    const SwTOXSortTabBase& rNew,
                                    sal_uInt16 nLevel, const Range& rRange )
{
    const SwTOXInternational& rIntl = *rNew.pTOXIntl;
    TextAndReading aToCompare(rStr, rStrReading);

    if( 0 != (nsSwTOIOptions::TOI_INITIAL_CAPS & GetOptions()) )
    {
        aToCompare.sText = rIntl.ToUpper( aToCompare.sText, 0 )
                         + aToCompare.sText.copy(1);
    }

    OSL_ENSURE(rRange.Min() >= 0 && rRange.Max() >= 0, "Min Max < 0");

    const sal_uInt16 nMin = (sal_uInt16)rRange.Min();
    const sal_uInt16 nMax = (sal_uInt16)rRange.Max();

    sal_uInt16 i;

    for( i = nMin; i < nMax; ++i)
    {
        SwTOXSortTabBase* pBase = aSortArr[i];

        if( rIntl.IsEqual( pBase->GetTxt(), pBase->GetLocale(),
                           aToCompare, rNew.GetLocale() )  &&
                    pBase->GetLevel() == nLevel )
            break;
    }
    if(i == nMax)
    {   // If not already present, create and insert
        SwTOXCustom* pKey = new SwTOXCustom( aToCompare, nLevel, rIntl,
                                             rNew.GetLocale() );
        for(i = nMin; i < nMax; ++i)
        {
            if(nLevel == aSortArr[i]->GetLevel() &&  *pKey < *(aSortArr[i]))
                break;
        }
        aSortArr.insert(aSortArr.begin() + i, pKey);
    }
    sal_uInt16 nStart = i+1;
    sal_uInt16 nEnd   = aSortArr.size();

    // Find end of range
    for(i = nStart; i < aSortArr.size(); ++i)
    {
        if(aSortArr[i]->GetLevel() <= nLevel)
        {   nEnd = i;
            break;
        }
    }
    return Range(nStart, nEnd);
}

bool SwTOXBase::IsTOXBaseInReadonly() const
{
    const SwTOXBaseSection *pSect = dynamic_cast<const SwTOXBaseSection*>(this);
    if (!pSect || !pSect->GetFmt())
        return false;

    const SwSectionNode* pSectNode = pSect->GetFmt()->GetSectionNode();
    if (!pSectNode)
        return false;

    const SwDocShell* pDocSh = pSectNode->GetDoc()->GetDocShell();
    if (!pDocSh)
        return false;

    if (pDocSh->IsReadOnly())
        return true;

    pSectNode = pSectNode->StartOfSectionNode()->FindSectionNode();
    if (!pSectNode)
        return false;

    return pSectNode->GetSection().IsProtectFlag();
}

const SfxItemSet* SwTOXBase::GetAttrSet() const
{
    const SwTOXBaseSection *pSect = dynamic_cast<const SwTOXBaseSection*>(this);
    if(pSect && pSect->GetFmt())
        return &pSect->GetFmt()->GetAttrSet();
    return 0;
}

void SwTOXBase::SetAttrSet( const SfxItemSet& rSet )
{
    const SwTOXBaseSection *pSect = dynamic_cast<const SwTOXBaseSection*>(this);
    if( pSect && pSect->GetFmt() )
        pSect->GetFmt()->SetFmtAttr( rSet );
}

bool SwTOXBase::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_CONTENT_VISIBLE:
        {
            const SwTOXBaseSection *pSect = dynamic_cast<const SwTOXBaseSection*>(this);
            if( pSect && pSect->GetFmt() )
                pSect->GetFmt()->GetInfo( rInfo );
        }
        return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
