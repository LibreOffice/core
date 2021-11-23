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
#include <editeng/formatbreakitem.hxx>
#include <comphelper/classids.hxx>
#include <docsh.hxx>
#include <ndole.hxx>
#include <txttxmrk.hxx>
#include <fmtpdsc.hxx>
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
#include <UndoSection.hxx>
#include <swundo.hxx>
#include <mdiexp.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <expfld.hxx>
#include <mvsave.hxx>
#include <node2lay.hxx>
#include <SwStyleNameMapper.hxx>
#include <breakit.hxx>
#include <calbck.hxx>
#include <ToxTextGenerator.hxx>
#include <ToxTabStopTokenHandler.hxx>
#include <frameformats.hxx>
#include <tools/datetimeutils.hxx>
#include <tools/globname.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>

#include <memory>

using namespace ::com::sun::star;

template<typename T, typename... Args> static
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
MakeSwTOXSortTabBase(SwRootFrame const*const pLayout, Args&& ... args)
{
    std::unique_ptr<T> pRet(new T(std::forward<Args>(args)...));
    pRet->InitText(pLayout); // ensure it's expanded with the layout
    return pRet;
}

void SwDoc::GetTOIKeys(SwTOIKeyType eTyp, std::vector<OUString>& rArr,
        SwRootFrame const& rLayout) const
{
    rArr.clear();

    // Look up all Primary and Secondary via the Pool
    for (const SfxPoolItem* pPoolItem : GetAttrPool().GetItemSurrogates(RES_TXTATR_TOXMARK))
    {
        const SwTOXMark* pItem = dynamic_cast<const SwTOXMark*>(pPoolItem);
        if( !pItem )
            continue;
        const SwTOXType* pTOXType = pItem->GetTOXType();
        if ( !pTOXType || pTOXType->GetType()!=TOX_INDEX )
            continue;
        const SwTextTOXMark* pMark = pItem->GetTextTOXMark();
        if ( pMark && pMark->GetpTextNd() &&
             pMark->GetpTextNd()->GetNodes().IsDocNodes() &&
             (!rLayout.IsHideRedlines()
                || !sw::IsMarkHintHidden(rLayout, *pMark->GetpTextNd(), *pMark)))
        {
            const OUString sStr = TOI_PRIMARY == eTyp
                ? pItem->GetPrimaryKey()
                : pItem->GetSecondaryKey();

            if( !sStr.isEmpty() )
                rArr.push_back( sStr );
        }
    }
}

/// Get current table of contents Mark.
sal_uInt16 SwDoc::GetCurTOXMark( const SwPosition& rPos,
                                SwTOXMarks& rArr )
{
    // search on Position rPos for all SwTOXMarks
    SwTextNode *const pTextNd = rPos.nNode.GetNode().GetTextNode();
    if( !pTextNd || !pTextNd->GetpSwpHints() )
        return 0;

    const SwpHints & rHts = *pTextNd->GetpSwpHints();
    sal_Int32 nSttIdx;
    const sal_Int32 *pEndIdx;

    const sal_Int32 nCurrentPos = rPos.nContent.GetIndex();

    for( size_t n = 0; n < rHts.Count(); ++n )
    {
        const SwTextAttr* pHt = rHts.Get(n);
        if( RES_TXTATR_TOXMARK != pHt->Which() )
            continue;
        if( ( nSttIdx = pHt->GetStart() ) < nCurrentPos )
        {
            // also check the end
            pEndIdx = pHt->End();
            if( nullptr == pEndIdx || *pEndIdx <= nCurrentPos )
                continue;       // keep searching
        }
        else if( nSttIdx > nCurrentPos )
            // If Hint's Start is greater than rPos, break, because
            // the attributes are sorted by Start!
            break;

        SwTOXMark* pTMark = const_cast<SwTOXMark*>(&pHt->GetTOXMark());
        rArr.push_back( pTMark );
    }
    return rArr.size();
}

/// Delete table of contents Mark
void SwDoc::DeleteTOXMark( const SwTOXMark* pTOXMark )
{
    const SwTextTOXMark* pTextTOXMark = pTOXMark->GetTextTOXMark();
    assert(pTextTOXMark);

    SwTextNode& rTextNd = const_cast<SwTextNode&>(pTextTOXMark->GetTextNode());
    assert(rTextNd.GetpSwpHints());

    if (pTextTOXMark->HasDummyChar())
    {
        // tdf#106377 don't use SwUndoResetAttr, it uses NOTXTATRCHR
        SwPaM tmp(rTextNd, pTextTOXMark->GetStart(),
                  rTextNd, pTextTOXMark->GetStart()+1);
        assert(rTextNd.GetText()[pTextTOXMark->GetStart()] == CH_TXTATR_INWORD);
        getIDocumentContentOperations().DeleteRange(tmp);
    }
    else
    {
        std::unique_ptr<SwRegHistory> aRHst;
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            // save attributes for Undo
            SwUndoResetAttr* pUndo = new SwUndoResetAttr(
                SwPosition( rTextNd, SwIndex( &rTextNd, pTextTOXMark->GetStart() ) ),
                RES_TXTATR_TOXMARK );
            GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pUndo) );

            aRHst.reset(new SwRegHistory(rTextNd, &pUndo->GetHistory()));
            rTextNd.GetpSwpHints()->Register(aRHst.get());
        }

        rTextNd.DeleteAttribute( const_cast<SwTextTOXMark*>(pTextTOXMark) );

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            if( rTextNd.GetpSwpHints() )
                rTextNd.GetpSwpHints()->DeRegister();
        }
    }

    getIDocumentState().SetModified();
}

namespace {

/// Travel between table of content Marks
class CompareNodeContent
{
    SwNodeOffset m_nNode;
    sal_Int32 m_nContent;
public:
    CompareNodeContent( SwNodeOffset nNd, sal_Int32 nCnt )
        : m_nNode( nNd ), m_nContent( nCnt ) {}

    bool operator==( const CompareNodeContent& rCmp ) const
        { return m_nNode == rCmp.m_nNode && m_nContent == rCmp.m_nContent; }
    bool operator!=( const CompareNodeContent& rCmp ) const
        { return m_nNode != rCmp.m_nNode || m_nContent != rCmp.m_nContent; }
    bool operator< ( const CompareNodeContent& rCmp ) const
        { return m_nNode < rCmp.m_nNode ||
            ( m_nNode == rCmp.m_nNode && m_nContent < rCmp.m_nContent); }
    bool operator<=( const CompareNodeContent& rCmp ) const
        { return m_nNode < rCmp.m_nNode ||
            ( m_nNode == rCmp.m_nNode && m_nContent <= rCmp.m_nContent); }
    bool operator> ( const CompareNodeContent& rCmp ) const
        { return m_nNode > rCmp.m_nNode ||
            ( m_nNode == rCmp.m_nNode && m_nContent > rCmp.m_nContent); }
    bool operator>=( const CompareNodeContent& rCmp ) const
        { return m_nNode > rCmp.m_nNode ||
            ( m_nNode == rCmp.m_nNode && m_nContent >= rCmp.m_nContent); }
};

}

const SwTOXMark& SwDoc::GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                    SwTOXSearch eDir, bool bInReadOnly )
{
    const SwTextTOXMark* pMark = rCurTOXMark.GetTextTOXMark();

    CompareNodeContent aAbsIdx(pMark ? pMark->GetpTextNd()->GetIndex() : SwNodeOffset(0), pMark ? pMark->GetStart() : 0);
    CompareNodeContent aPrevPos( SwNodeOffset(0), 0 );
    CompareNodeContent aNextPos( NODE_OFFSET_MAX, SAL_MAX_INT32 );
    CompareNodeContent aMax( SwNodeOffset(0), 0 );
    CompareNodeContent aMin( NODE_OFFSET_MAX, SAL_MAX_INT32 );

    const SwTOXMark*    pNew    = nullptr;
    const SwTOXMark*    pMax    = &rCurTOXMark;
    const SwTOXMark*    pMin    = &rCurTOXMark;

    const SwTOXType* pType = rCurTOXMark.GetTOXType();
    SwTOXMarks aMarks;
    pType->CollectTextMarks(aMarks);

    for(SwTOXMark* pTOXMark : aMarks)
    {
        if ( pTOXMark == &rCurTOXMark )
            continue;

        pMark = pTOXMark->GetTextTOXMark();
        if (!pMark)
            continue;

        SwTextNode const*const pTOXSrc = pMark->GetpTextNd();
        if (!pTOXSrc)
            continue;

        Point aPt;
        std::pair<Point, bool> const tmp(aPt, false);
        const SwContentFrame* pCFrame = pTOXSrc->getLayoutFrame(
                getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, &tmp);
        if (!pCFrame)
            continue;

        if ( bInReadOnly || !pCFrame->IsProtected() )
        {
            CompareNodeContent aAbsNew( pTOXSrc->GetIndex(), pMark->GetStart() );
            switch( eDir )
            {
            // The following (a bit more complicated) statements make it
            // possible to also travel across Entries on the same (!)
            // position. If someone has time, please feel free to optimize.
            case TOX_SAME_PRV:
                if (pTOXMark->GetText(nullptr) != rCurTOXMark.GetText(nullptr))
                    break;
                [[fallthrough]];
            case TOX_PRV:
                if ( (aAbsNew < aAbsIdx && aAbsNew > aPrevPos) ||
                     (aAbsIdx == aAbsNew &&
                      (reinterpret_cast<sal_uLong>(&rCurTOXMark) > reinterpret_cast<sal_uLong>(pTOXMark) &&
                       (!pNew || aPrevPos < aAbsIdx || reinterpret_cast<sal_uLong>(pNew) < reinterpret_cast<sal_uLong>(pTOXMark) ) )) ||
                     (aPrevPos == aAbsNew && aAbsIdx != aAbsNew &&
                      reinterpret_cast<sal_uLong>(pTOXMark) > reinterpret_cast<sal_uLong>(pNew)) )
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
                if (pTOXMark->GetText(nullptr) != rCurTOXMark.GetText(nullptr))
                    break;
                [[fallthrough]];
            case TOX_NXT:
                if ( (aAbsNew > aAbsIdx && aAbsNew < aNextPos) ||
                     (aAbsIdx == aAbsNew &&
                      (reinterpret_cast<sal_uLong>(&rCurTOXMark) < reinterpret_cast<sal_uLong>(pTOXMark) &&
                       (!pNew || aNextPos > aAbsIdx || reinterpret_cast<sal_uLong>(pNew) > reinterpret_cast<sal_uLong>(pTOXMark)) )) ||
                     (aNextPos == aAbsNew && aAbsIdx != aAbsNew &&
                      reinterpret_cast<sal_uLong>(pTOXMark) < reinterpret_cast<sal_uLong>(pNew)) )
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

SwTOXBaseSection* SwDoc::InsertTableOf( const SwPosition& rPos,
                                        const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet,
                                        bool bExpand,
                                        SwRootFrame const*const pLayout)
{
    SwPaM aPam( rPos );
    return InsertTableOf( aPam, rTOX, pSet, bExpand, pLayout );
}

SwTOXBaseSection* SwDoc::InsertTableOf( const SwPaM& aPam,
                                        const SwTOXBase& rTOX,
                                        const SfxItemSet* pSet,
                                        bool bExpand,
                                        SwRootFrame const*const pLayout )
{
    assert(!bExpand || pLayout != nullptr);
    GetIDocumentUndoRedo().StartUndo( SwUndoId::INSTOX, nullptr );

    OUString sSectNm = GetUniqueTOXBaseName( *rTOX.GetTOXType(), rTOX.GetTOXName() );
    SwSectionData aSectionData( SectionType::ToxContent, sSectNm );

    std::tuple<SwTOXBase const*, sw::RedlineMode, sw::FieldmarkMode> const tmp(
        &rTOX,
        pLayout && pLayout->IsHideRedlines()
            ? sw::RedlineMode::Hidden
            : sw::RedlineMode::Shown,
        pLayout ? pLayout->GetFieldmarkMode() : sw::FieldmarkMode::ShowBoth);
    SwTOXBaseSection *const pNewSection = dynamic_cast<SwTOXBaseSection *>(
        InsertSwSection(aPam, aSectionData, & tmp, pSet, false));
    if (pNewSection)
    {
        SwSectionNode *const pSectNd = pNewSection->GetFormat()->GetSectionNode();
        pNewSection->SetTOXName(sSectNm); // rTOX may have had no name...

        if( bExpand )
        {
            // add value for 2nd parameter = true to
            // indicate, that a creation of a new table of content has to be performed.
            // Value of 1st parameter = default value.
            pNewSection->Update( nullptr, pLayout, true );
        }
        else if( rTOX.GetTitle().getLength()==1 && IsInReading() )
        // insert title of TOX
        {
            // then insert the headline section
            SwNodeIndex aIdx( *pSectNd, +1 );

            SwTextNode* pHeadNd = GetNodes().MakeTextNode( aIdx,
                            getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ) );

            SwSectionData headerData( SectionType::ToxHeader, pNewSection->GetTOXName()+"_Head" );

            SwNodeIndex aStt( *pHeadNd ); --aIdx;
            SwSectionFormat* pSectFormat = MakeSectionFormat();
            GetNodes().InsertTextSection(
                    aStt, *pSectFormat, headerData, nullptr, &aIdx, true, false);
        }
    }

    GetIDocumentUndoRedo().EndUndo( SwUndoId::INSTOX, nullptr );

    return pNewSection;
}

void SwDoc::InsertTableOf( SwNodeOffset nSttNd, SwNodeOffset nEndNd,
                                                const SwTOXBase& rTOX,
                                                const SfxItemSet* pSet )
{
    // check for recursive TOX
    SwNode* pNd = GetNodes()[ nSttNd ];
    SwSectionNode* pSectNd = pNd->FindSectionNode();
    while( pSectNd )
    {
        SectionType eT = pSectNd->GetSection().GetType();
        if( SectionType::ToxHeader == eT || SectionType::ToxContent == eT )
            return;
        pSectNd = pSectNd->StartOfSectionNode()->FindSectionNode();
    }

    const OUString sSectNm = GetUniqueTOXBaseName(*rTOX.GetTOXType(), rTOX.GetTOXName());

    SwSectionData aSectionData( SectionType::ToxContent, sSectNm );

    SwNodeIndex aStt( GetNodes(), nSttNd ), aEnd( GetNodes(), nEndNd );
    SwSectionFormat* pFormat = MakeSectionFormat();
    if(pSet)
        pFormat->SetFormatAttr(*pSet);

    SwSectionNode *const pNewSectionNode =
        GetNodes().InsertTextSection(aStt, *pFormat, aSectionData, &rTOX, &aEnd);
    if (!pNewSectionNode)
    {
        DelSectionFormat( pFormat );
        return;
    }

    SwTOXBaseSection *const pNewSection(
        dynamic_cast<SwTOXBaseSection*>(& pNewSectionNode->GetSection()));
    if (pNewSection)
        pNewSection->SetTOXName(sSectNm); // rTOX may have had no name...
}

/// Get current table of contents
SwTOXBase* SwDoc::GetCurTOX( const SwPosition& rPos )
{
    SwNode& rNd = rPos.nNode.GetNode();
    SwSectionNode* pSectNd = rNd.FindSectionNode();
    while( pSectNd )
    {
        SectionType eT = pSectNd->GetSection().GetType();
        if( SectionType::ToxContent == eT )
        {
            assert( dynamic_cast< const SwTOXBaseSection *>( &pSectNd->GetSection()) &&
                    "no TOXBaseSection!" );
            SwTOXBaseSection& rTOXSect = static_cast<SwTOXBaseSection&>(
                                                pSectNd->GetSection());
            return &rTOXSect;
        }
        pSectNd = pSectNd->StartOfSectionNode()->FindSectionNode();
    }
    return nullptr;
}

const SwAttrSet& SwDoc::GetTOXBaseAttrSet(const SwTOXBase& rTOXBase)
{
    assert( dynamic_cast<const SwTOXBaseSection*>( &rTOXBase) && "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = static_cast<const SwTOXBaseSection&>(rTOXBase);
    SwSectionFormat const * pFormat = rTOXSect.GetFormat();
    OSL_ENSURE( pFormat, "invalid TOXBaseSection!" );
    return pFormat->GetAttrSet();
}

const SwTOXBase* SwDoc::GetDefaultTOXBase( TOXTypes eTyp, bool bCreate )
{
    std::unique_ptr<SwTOXBase>* prBase = nullptr;
    switch(eTyp)
    {
    case  TOX_CONTENT:          prBase = &mpDefTOXBases->pContBase; break;
    case  TOX_INDEX:            prBase = &mpDefTOXBases->pIdxBase;  break;
    case  TOX_USER:             prBase = &mpDefTOXBases->pUserBase; break;
    case  TOX_TABLES:           prBase = &mpDefTOXBases->pTableBase;  break;
    case  TOX_OBJECTS:          prBase = &mpDefTOXBases->pObjBase;  break;
    case  TOX_ILLUSTRATIONS:    prBase = &mpDefTOXBases->pIllBase;  break;
    case  TOX_AUTHORITIES:      prBase = &mpDefTOXBases->pAuthBase; break;
    case  TOX_BIBLIOGRAPHY:      prBase = &mpDefTOXBases->pBiblioBase; break;
    case  TOX_CITATION: /** TODO */break;
    }
    if (!prBase)
        return nullptr;
    if(!(*prBase) && bCreate)
    {
        SwForm aForm(eTyp);
        const SwTOXType* pType = GetTOXType(eTyp, 0);
        prBase->reset(new SwTOXBase(pType, aForm, SwTOXElement::NONE, pType->GetTypeName()));
    }
    return prBase->get();
}

void    SwDoc::SetDefaultTOXBase(const SwTOXBase& rBase)
{
    std::unique_ptr<SwTOXBase>* prBase = nullptr;
    switch(rBase.GetType())
    {
    case  TOX_CONTENT:          prBase = &mpDefTOXBases->pContBase; break;
    case  TOX_INDEX:            prBase = &mpDefTOXBases->pIdxBase;  break;
    case  TOX_USER:             prBase = &mpDefTOXBases->pUserBase; break;
    case  TOX_TABLES:           prBase = &mpDefTOXBases->pTableBase;  break;
    case  TOX_OBJECTS:          prBase = &mpDefTOXBases->pObjBase;  break;
    case  TOX_ILLUSTRATIONS:    prBase = &mpDefTOXBases->pIllBase;  break;
    case  TOX_AUTHORITIES:      prBase = &mpDefTOXBases->pAuthBase; break;
    case  TOX_BIBLIOGRAPHY:      prBase = &mpDefTOXBases->pBiblioBase; break;
    case  TOX_CITATION: /** TODO */break;
    }
    if (!prBase)
        return;
    prBase->reset(new SwTOXBase(rBase));
}

/// Delete table of contents
bool SwDoc::DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes )
{
    // We only delete the TOX, not the Nodes
    bool bRet = false;
    assert( dynamic_cast<const SwTOXBaseSection*>( &rTOXBase) && "no TOXBaseSection!" );

    const SwTOXBaseSection& rTOXSect = static_cast<const SwTOXBaseSection&>(rTOXBase);
    SwSectionFormat const * pFormat = rTOXSect.GetFormat();
    /* Save the start node of the TOX' section. */
    SwSectionNode const * pMyNode = pFormat ? pFormat->GetSectionNode() : nullptr;
    if (pMyNode)
    {
        GetIDocumentUndoRedo().StartUndo( SwUndoId::CLEARTOXRANGE, nullptr );

        /* Save start node of section's surrounding. */
        SwNode const * pStartNd = pMyNode->StartOfSectionNode();

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
                getIDocumentContentOperations().AppendTextNode(aInsPos);

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
            pFormat->GetChildSections( aArr, SectionSort::Not, false );
            for( const auto pSect : aArr )
            {
                if( SectionType::ToxHeader == pSect->GetType() )
                {
                    DelSectionFormat( pSect->GetFormat(), bDelNodes );
                }
            }
        }

        DelSectionFormat( const_cast<SwSectionFormat *>(pFormat), bDelNodes );

        GetIDocumentUndoRedo().EndUndo( SwUndoId::CLEARTOXRANGE, nullptr );
        bRet = true;
    }

    return bRet;
}

/// Manage table of content types
sal_uInt16 SwDoc::GetTOXTypeCount(TOXTypes eTyp) const
{
    sal_uInt16 nCnt = 0;
    for( auto const & pTOXType : *mpTOXTypes )
        if( eTyp == pTOXType->GetType() )
            ++nCnt;
    return nCnt;
}

const SwTOXType* SwDoc::GetTOXType( TOXTypes eTyp, sal_uInt16 nId ) const
{
    sal_uInt16 nCnt = 0;
    for( auto const & pTOXType : *mpTOXTypes )
        if( eTyp == pTOXType->GetType() && nCnt++ == nId )
            return pTOXType.get();
    return nullptr;
}

const SwTOXType* SwDoc::InsertTOXType( const SwTOXType& rTyp )
{
    SwTOXType * pNew = new SwTOXType(rTyp);
    mpTOXTypes->emplace_back( pNew );
    return pNew;
}

OUString SwDoc::GetUniqueTOXBaseName( const SwTOXType& rType,
                                      const OUString& sChkStr ) const
{
    if( IsInMailMerge())
    {
        OUString newName = "MailMergeTOX"
            + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
            + OUString::number( mpSectionFormatTable->size() + 1 );
        if( !sChkStr.isEmpty())
            newName += sChkStr;
        return newName;
    }

    bool bUseChkStr = !sChkStr.isEmpty();
    const OUString& aName( rType.GetTypeName() );
    const sal_Int32 nNmLen = aName.getLength();

    SwSectionFormats::size_type nNum = 0;
    const SwSectionFormats::size_type nFlagSize = ( mpSectionFormatTable->size() / 8 ) +2;
    std::unique_ptr<sal_uInt8[]> pSetFlags(new sal_uInt8[ nFlagSize ]);
    memset( pSetFlags.get(), 0, nFlagSize );

    for( auto pSectionFormat : *mpSectionFormatTable )
    {
        const SwSectionNode *pSectNd = pSectionFormat->GetSectionNode();
        if ( !pSectNd )
            continue;

        const SwSection& rSect = pSectNd->GetSection();
        if (rSect.GetType()==SectionType::ToxContent)
        {
            const OUString& rNm = rSect.GetSectionName();
            if ( rNm.startsWith(aName) )
            {
                // Calculate number and set the Flag
                nNum = rNm.copy( nNmLen ).toInt32();
                if( nNum-- && nNum < mpSectionFormatTable->size() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if ( bUseChkStr && sChkStr==rNm )
                bUseChkStr = false;
        }
    }

    if( !bUseChkStr )
    {
        // All Numbers have been flagged accordingly, so get the right Number
        nNum = mpSectionFormatTable->size();
        for( SwSectionFormats::size_type n = 0; n < nFlagSize; ++n )
        {
            sal_uInt8 nTmp = pSetFlags[ n ];
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
    if ( bUseChkStr )
        return sChkStr;
    return aName + OUString::number( ++nNum );
}

bool SwDoc::SetTOXBaseName(const SwTOXBase& rTOXBase, const OUString& rName)
{
    assert( dynamic_cast<const SwTOXBaseSection*>( &rTOXBase) && "no TOXBaseSection!" );
    SwTOXBaseSection* pTOX = const_cast<SwTOXBaseSection*>(static_cast<const SwTOXBaseSection*>(&rTOXBase));

    if (GetUniqueTOXBaseName(*rTOXBase.GetTOXType(), rName) == rName)
    {
        pTOX->SetTOXName(rName);
        pTOX->SetSectionName(rName);
        getIDocumentState().SetModified();
        return true;
    }
    return false;
}

static const SwTextNode* lcl_FindChapterNode( const SwNode& rNd,
        SwRootFrame const*const pLayout, sal_uInt8 const nLvl = 0 )
{
    const SwNode* pNd = &rNd;
    if( pNd->GetNodes().GetEndOfExtras().GetIndex() > pNd->GetIndex() )
    {
        // then find the "Anchor" (Body) position
        Point aPt;
        SwNode2Layout aNode2Layout( *pNd, pNd->GetIndex() );
        const SwFrame* pFrame = aNode2Layout.GetFrame( &aPt );

        if( pFrame )
        {
            SwPosition aPos( *pNd );
            pNd = GetBodyTextNode( pNd->GetDoc(), aPos, *pFrame );
            OSL_ENSURE( pNd, "Where's the paragraph?" );
        }
    }
    return pNd ? pNd->FindOutlineNodeOfLevel(nLvl, pLayout) : nullptr;
}

static bool IsHeadingContained(const SwTextNode* pOwnChapterNode, const SwNode& rNd)
{
    const SwNode* pNd = &rNd;
    const SwOutlineNodes& rONds = pNd->GetNodes().GetOutLineNds();
    bool bIsHeadingContained = false;
    if (!rONds.empty())
    {
        bool bCheckFirst = false;
        SwOutlineNodes::size_type nPos;

        if (!rONds.Seek_Entry(const_cast<SwNode*>(pNd), &nPos))
        {
            if (nPos == 0)
                bCheckFirst = true;
            else
                nPos--;
        }

        if (bCheckFirst)
        {
            const SwContentNode* pCNd = pNd->GetContentNode();
            const SwTextNode* pTNd = rONds[nPos]->GetTextNode();

            Point aPt(0, 0);
            std::pair<Point, bool> const tmp(aPt, false);
            const SwFrame *pFrame
                = pTNd->getLayoutFrame(pTNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
                                       nullptr, &tmp),
                *pMyFrame
                = pCNd ? pCNd->getLayoutFrame(
                      pCNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, &tmp)
                       : nullptr;
            const SwPageFrame* pPgFrame = pFrame ? pFrame->FindPageFrame() : nullptr;
            // Check if the one asking doesn't precede the page
            bIsHeadingContained
                = pPgFrame && pMyFrame
                  && pPgFrame->getFrameArea().Top() <= pMyFrame->getFrameArea().Top();
        }
        else
        {
            if (nPos < rONds.size())
            {
                while (pOwnChapterNode->GetAttrOutlineLevel()
                       < rONds[nPos]->GetTextNode()->GetAttrOutlineLevel())
                    nPos--;
                bIsHeadingContained = pOwnChapterNode == rONds[nPos]->GetTextNode();
            }
        }
    }
    return bIsHeadingContained;
}

// Table of contents class
SwTOXBaseSection::SwTOXBaseSection(SwTOXBase const& rBase, SwSectionFormat & rFormat)
    : SwTOXBase( rBase )
    , SwSection( SectionType::ToxContent, OUString(), rFormat )
{
    SetProtect( rBase.IsProtected() );
    SetSectionName( GetTOXName() );
}

SwTOXBaseSection::~SwTOXBaseSection()
{
}

bool SwTOXBaseSection::SetPosAtStartEnd( SwPosition& rPos ) const
{
    bool bRet = false;
    const SwSectionNode* pSectNd = GetFormat()->GetSectionNode();
    if( pSectNd )
    {
        rPos.nNode = *pSectNd;
        SwContentNode* pCNd = pSectNd->GetDoc().GetNodes().GoNext( &rPos.nNode );
        rPos.nContent.Assign( pCNd, 0 );
        bRet = true;
    }
    return bRet;
}

/// Collect table of contents content
void SwTOXBaseSection::Update(const SfxItemSet* pAttr,
                              SwRootFrame const*const pLayout,
                              const bool        _bNewTOX)
{
    if (!GetFormat())
        return;
    SwSectionNode const*const pSectNd(GetFormat()->GetSectionNode());
    if (nullptr == pSectNd ||
        !pSectNd->GetNodes().IsDocNodes() ||
        IsHiddenFlag() ||
        (pLayout->HasMergedParas() && pSectNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden))
    {
        return;
    }

    if ( !mbKeepExpression )
    {
        maMSTOCExpression.clear();
    }

    SwDoc& rDoc = const_cast<SwDoc&>(pSectNd->GetDoc());

    if (pAttr && GetFormat())
        rDoc.ChgFormat(*GetFormat(), *pAttr);

    // determine default page description, which will be used by the content nodes,
    // if no appropriate one is found.
    const SwPageDesc* pDefaultPageDesc;
    {
        pDefaultPageDesc =
            pSectNd->GetSection().GetFormat()->GetPageDesc().GetPageDesc();
        if ( !_bNewTOX && !pDefaultPageDesc )
        {
            // determine page description of table-of-content
            SwNodeOffset nPgDescNdIdx = pSectNd->GetIndex() + 1;
            SwNodeOffset* pPgDescNdIdx = &nPgDescNdIdx;
            pDefaultPageDesc = pSectNd->FindPageDesc( pPgDescNdIdx );
            if ( nPgDescNdIdx < pSectNd->GetIndex() )
            {
                pDefaultPageDesc = nullptr;
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
            const SwContentNode* pNdAfterTOX = pSectNd->GetNodes().GoNext( &aIdx );
            const SwAttrSet& aNdAttrSet = pNdAfterTOX->GetSwAttrSet();
            const SvxBreak eBreak = aNdAttrSet.GetBreak().GetBreak();
            if ( eBreak != SvxBreak::PageBefore && eBreak != SvxBreak::PageBoth )
            {
                pDefaultPageDesc = pNdAfterTOX->FindPageDesc();
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
                SwNodes::GoPrevious( &aIdx )->FindPageDesc();

        }
        if ( !pDefaultPageDesc )
        {
            // determine default page description
            pDefaultPageDesc = &rDoc.GetPageDesc( 0 );
        }
    }

    rDoc.getIDocumentState().SetModified();

    // get current Language
    SwTOXInternational aIntl(  GetLanguage(),
                               TOX_INDEX == GetTOXType()->GetType() ?
                               GetOptions() : SwTOIOptions::NONE,
                               GetSortAlgorithm() );

    m_aSortArr.clear();

    // find the first layout node for this TOX, if it only find the content
    // in his own chapter
    const SwTextNode* pOwnChapterNode = IsFromChapter()
        ? ::lcl_FindChapterNode( *pSectNd, pLayout, pSectNd->FindSectionNode()->GetSectionLevel() + 1 )
        : nullptr;

    SwNode2LayoutSaveUpperFrames aN2L(*pSectNd);
    const_cast<SwSectionNode*>(pSectNd)->DelFrames();

    // This would be a good time to update the Numbering
    rDoc.UpdateNumRule();

    if( GetCreateType() & SwTOXElement::Mark )
        UpdateMarks( aIntl, pOwnChapterNode, pLayout );

    if( GetCreateType() & SwTOXElement::OutlineLevel )
        UpdateOutline( pOwnChapterNode, pLayout );

    if( GetCreateType() & SwTOXElement::Template )
        UpdateTemplate( pOwnChapterNode, pLayout );

    if( GetCreateType() & SwTOXElement::Ole ||
            TOX_OBJECTS == SwTOXBase::GetType())
        UpdateContent( SwTOXElement::Ole, pOwnChapterNode, pLayout );

    if( GetCreateType() & SwTOXElement::Table ||
            (TOX_TABLES == SwTOXBase::GetType() && IsFromObjectNames()) )
        UpdateTable( pOwnChapterNode, pLayout );

    if( GetCreateType() & SwTOXElement::Graphic ||
        (TOX_ILLUSTRATIONS == SwTOXBase::GetType() && IsFromObjectNames()))
        UpdateContent( SwTOXElement::Graphic, pOwnChapterNode, pLayout );

    if( !GetSequenceName().isEmpty() && !IsFromObjectNames() &&
        (TOX_TABLES == SwTOXBase::GetType() ||
         TOX_ILLUSTRATIONS == SwTOXBase::GetType() ) )
        UpdateSequence( pOwnChapterNode, pLayout );

    if( GetCreateType() & SwTOXElement::Frame )
        UpdateContent( SwTOXElement::Frame, pOwnChapterNode, pLayout );

    if(TOX_AUTHORITIES == SwTOXBase::GetType())
        UpdateAuthorities( aIntl, pLayout );

    // Insert AlphaDelimiters if needed (just for keywords)
    if( TOX_INDEX == SwTOXBase::GetType() &&
        ( GetOptions() & SwTOIOptions::AlphaDelimiter ) )
        InsertAlphaDelimiter( aIntl );

    // remove old content an insert one empty textnode (to hold the layout!)
    SwTextNode* pFirstEmptyNd;

    SwUndoUpdateIndex * pUndo(nullptr);
    {
        rDoc.getIDocumentRedlineAccess().DeleteRedline( *pSectNd, true, RedlineType::Any );

        SwNodeIndex aSttIdx( *pSectNd, +1 );
        SwNodeIndex aEndIdx( *pSectNd->EndOfSectionNode() );
        pFirstEmptyNd = rDoc.GetNodes().MakeTextNode( aEndIdx,
                        rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ) );

        {
            // Task 70995 - save and restore PageDesc and Break Attributes
            SwNodeIndex aNxtIdx( aSttIdx );
            const SwContentNode* pCNd = aNxtIdx.GetNode().GetContentNode();
            if( !pCNd )
                pCNd = rDoc.GetNodes().GoNext( &aNxtIdx );
            assert(pCNd != pFirstEmptyNd);
            assert(pCNd->GetIndex() < pFirstEmptyNd->GetIndex());
            if( pCNd->HasSwAttrSet() )
            {
                SfxItemSet aBrkSet( rDoc.GetAttrPool(), aBreakSetRange );
                aBrkSet.Put( *pCNd->GetpSwAttrSet() );
                if( aBrkSet.Count() )
                    pFirstEmptyNd->SetAttr( aBrkSet );
            }
        }

        if (rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            // note: this will first append a SwUndoDelSection from the ctor...
            pUndo = new SwUndoUpdateIndex(*this);
            // tdf#123313 insert Undo *after* all CrossRefBookmark Undos have
            // been inserted by the Update*() functions
            rDoc.GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndoUpdateIndex>(pUndo));
        }
        else
        {
            --aEndIdx;
            SwPosition aPos( aEndIdx, SwIndex( pFirstEmptyNd, 0 ));
            SwDoc::CorrAbs( aSttIdx, aEndIdx, aPos, true );

            // delete flys in whole range including start node which requires
            // giving the node before start node as Mark parameter, hence -1.
            // (flys must be deleted because the anchor nodes are removed)
            DelFlyInRange( SwNodeIndex(aSttIdx, -1), aEndIdx );

            rDoc.GetNodes().Delete( aSttIdx, aEndIdx.GetIndex() - aSttIdx.GetIndex() );
        }
    }

    // insert title of TOX
    if ( !GetTitle().isEmpty() )
    {
        // then insert the headline section
        SwNodeIndex aIdx( *pSectNd, +1 );

        SwTextNode* pHeadNd = rDoc.GetNodes().MakeTextNode( aIdx,
                                GetTextFormatColl( FORM_TITLE ) );
        pHeadNd->InsertText( GetTitle(), SwIndex( pHeadNd ) );

        SwSectionData headerData( SectionType::ToxHeader, GetTOXName()+"_Head" );

        SwNodeIndex aStt( *pHeadNd ); --aIdx;
        SwSectionFormat* pSectFormat = rDoc.MakeSectionFormat();
        rDoc.GetNodes().InsertTextSection(
                aStt, *pSectFormat, headerData, nullptr, &aIdx, true, false);

        if (pUndo)
        {
            pUndo->TitleSectionInserted(*pSectFormat);
        }
    }

    // Sort the List of all TOC Marks and TOC Sections
    std::vector<SwTextFormatColl*> aCollArr( GetTOXForm().GetFormMax(), nullptr );
    std::unordered_map<OUString, int> markURLs;
    SwNodeIndex aInsPos( *pFirstEmptyNd, 1 );
    for( size_t nCnt = 0; nCnt < m_aSortArr.size(); ++nCnt )
    {
        ::SetProgressState( 0, rDoc.GetDocShell() );

        // Put the Text into the TOC
        sal_uInt16 nLvl = m_aSortArr[ nCnt ]->GetLevel();
        SwTextFormatColl* pColl = aCollArr[ nLvl ];
        if( !pColl )
        {
            pColl = GetTextFormatColl( nLvl );
            aCollArr[ nLvl ] = pColl;
        }

        // Generate: Set dynamic TabStops
        SwTextNode* pTOXNd = rDoc.GetNodes().MakeTextNode( aInsPos , pColl );
        m_aSortArr[ nCnt ]->pTOXNd = pTOXNd;

        // Generate: Evaluate Form and insert the place holder for the
        // page number. If it is a TOX_INDEX and the SwForm IsCommaSeparated()
        // then a range of entries must be generated into one paragraph
        size_t nRange = 1;
        if(TOX_INDEX == SwTOXBase::GetType() &&
                GetTOXForm().IsCommaSeparated() &&
                m_aSortArr[nCnt]->GetType() == TOX_SORT_INDEX)
        {
            const SwTOXMark& rMark = m_aSortArr[nCnt]->pTextMark->GetTOXMark();
            const OUString& sPrimKey = rMark.GetPrimaryKey();
            const OUString& sSecKey = rMark.GetSecondaryKey();
            const SwTOXMark* pNextMark = nullptr;
            while(m_aSortArr.size() > (nCnt + nRange) &&
                    m_aSortArr[nCnt + nRange]->GetType() == TOX_SORT_INDEX )
            {
                pNextMark = &(m_aSortArr[nCnt + nRange]->pTextMark->GetTOXMark());
                if( !pNextMark ||
                    pNextMark->GetPrimaryKey() != sPrimKey ||
                    pNextMark->GetSecondaryKey() != sSecKey)
                    break;
                nRange++;
            }
        }
        // pass node index of table-of-content section and default page description
        // to method <GenerateText(..)>.
        ::SetProgressState( 0, rDoc.GetDocShell() );

        std::shared_ptr<sw::ToxTabStopTokenHandler> tabStopTokenHandler =
                std::make_shared<sw::DefaultToxTabStopTokenHandler>(
                        pSectNd->GetIndex(), *pDefaultPageDesc, GetTOXForm().IsRelTabPos(),
                        rDoc.GetDocumentSettingManager().get(DocumentSettingId::TABS_RELATIVE_TO_INDENT) ?
                                sw::DefaultToxTabStopTokenHandler::TABSTOPS_RELATIVE_TO_INDENT :
                                sw::DefaultToxTabStopTokenHandler::TABSTOPS_RELATIVE_TO_PAGE);
        sw::ToxTextGenerator ttgn(GetTOXForm(), tabStopTokenHandler);
        ttgn.GenerateText(GetFormat()->GetDoc(), markURLs, m_aSortArr, nCnt, nRange, pLayout);
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
        SwDoc::CorrAbs( aInsPos, aEndIdx, *aCorPam.GetPoint(), true );

        // Task 70995 - save and restore PageDesc and Break Attributes
        if( pFirstEmptyNd->HasSwAttrSet() )
        {
            if( !GetTitle().isEmpty() )
                aEndIdx = *pSectNd;
            else
                aEndIdx = *pFirstEmptyNd;
            SwContentNode* pCNd = rDoc.GetNodes().GoNext( &aEndIdx );
            if( pCNd ) // Robust against defect documents, e.g. i60336
                pCNd->SetAttr( *pFirstEmptyNd->GetpSwAttrSet() );
        }
    }

    // now create the new Frames
    SwNodeOffset nIdx = pSectNd->GetIndex();
    // don't delete if index is empty
    if(nIdx + SwNodeOffset(2) < pSectNd->EndOfSectionIndex())
        rDoc.GetNodes().Delete( aInsPos );

    aN2L.RestoreUpperFrames( rDoc.GetNodes(), nIdx, nIdx + 1 );
    o3tl::sorted_vector<SwRootFrame*> aAllLayouts = rDoc.GetAllLayouts();
    for ( const auto& rpLayout : aAllLayouts )
    {
        SwFrame::CheckPageDescs( static_cast<SwPageFrame*>(rpLayout->Lower()) );
    }

    SetProtect( SwTOXBase::IsProtected() );
}

void SwTOXBaseSection::InsertAlphaDelimiter( const SwTOXInternational& rIntl )
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    OUString sLastDeli;
    size_t i = 0;
    while( i < m_aSortArr.size() )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        sal_uInt16 nLevel = m_aSortArr[i]->GetLevel();

        // Skip AlphaDelimiter
        if( nLevel == FORM_ALPHA_DELIMITER )
            continue;

        const OUString sDeli = rIntl.GetIndexKey( m_aSortArr[i]->GetText(),
                                   m_aSortArr[i]->GetLocale() );

        // Do we already have a Delimiter?
        if( !sDeli.isEmpty() && sLastDeli != sDeli )
        {
            // We skip all that are less than a small Blank (these are special characters)
            if( ' ' <= sDeli[0] )
            {
                std::unique_ptr<SwTOXCustom> pCst(
                        MakeSwTOXSortTabBase<SwTOXCustom>(nullptr,
                                TextAndReading(sDeli, OUString()),
                                                     FORM_ALPHA_DELIMITER,
                                                     rIntl, m_aSortArr[i]->GetLocale() ));
                m_aSortArr.insert( m_aSortArr.begin() + i, std::move(pCst));
                i++;
            }
            sLastDeli = sDeli;
        }

        // Skip until we get to the same or a lower Level
        do {
            i++;
        } while (i < m_aSortArr.size() && m_aSortArr[i]->GetLevel() > nLevel);
    }
}

/// Evaluate Template
SwTextFormatColl* SwTOXBaseSection::GetTextFormatColl( sal_uInt16 nLevel )
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    const OUString& rName = GetTOXForm().GetTemplate( nLevel );
    SwTextFormatColl* pColl = !rName.isEmpty() ? pDoc->FindTextFormatCollByName(rName) :nullptr;
    if( !pColl )
    {
        sal_uInt16 nPoolFormat = 0;
        const TOXTypes eMyType = SwTOXBase::GetType();
        switch( eMyType )
        {
        case TOX_INDEX:         nPoolFormat = RES_POOLCOLL_TOX_IDXH;       break;
        case TOX_USER:
            if( nLevel < 6 )
                nPoolFormat = RES_POOLCOLL_TOX_USERH;
            else
                nPoolFormat = RES_POOLCOLL_TOX_USER6 - 6;
            break;
        case TOX_ILLUSTRATIONS: nPoolFormat = RES_POOLCOLL_TOX_ILLUSH;     break;
        case TOX_OBJECTS:       nPoolFormat = RES_POOLCOLL_TOX_OBJECTH;    break;
        case TOX_TABLES:        nPoolFormat = RES_POOLCOLL_TOX_TABLESH;    break;
        case TOX_AUTHORITIES:
        case TOX_BIBLIOGRAPHY:
            nPoolFormat = RES_POOLCOLL_TOX_AUTHORITIESH; break;
        case  TOX_CITATION: /** TODO */break;
        case TOX_CONTENT:
            // There's a jump in the ContentArea!
            if( nLevel < 6 )
                nPoolFormat = RES_POOLCOLL_TOX_CNTNTH;
            else
                nPoolFormat = RES_POOLCOLL_TOX_CNTNT6 - 6;
            break;
        }

        if(eMyType == TOX_AUTHORITIES && nLevel)
            nPoolFormat = nPoolFormat + 1;
        else if(eMyType == TOX_INDEX && nLevel)
        {
            // pool: Level 1,2,3, Delimiter
            // SwForm: Delimiter, Level 1,2,3
            nPoolFormat += 1 == nLevel ? nLevel + 3 : nLevel - 1;
        }
        else
            nPoolFormat = nPoolFormat + nLevel;
        pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nPoolFormat );
    }
    return pColl;
}

void SwTOXBaseSection::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if (auto pFindHint = dynamic_cast<const sw::FindContentFrameHint*>(&rHint))
    {
        if(pFindHint->m_rpContentFrame)
            return;
        auto pSectFormat = GetFormat();
        if(!pSectFormat)
            return;
        const SwSectionNode* pSectNd = pSectFormat->GetSectionNode();
        if(!pSectNd)
            return;
        SwNodeIndex aIdx(*pSectNd, 1);
        SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
        if(!pCNd)
            pCNd = pFindHint->m_rDoc.GetNodes().GoNext(&aIdx);
        if(!pCNd)
            return;
        if(pCNd->EndOfSectionIndex() >= pSectNd->EndOfSectionIndex())
            return;
        pFindHint->m_rpContentFrame = pCNd->getLayoutFrame(&pFindHint->m_rLayout);
    } else
        SwTOXBase::SwClientNotify(rModify, rHint);
}

/// Create from Marks
void SwTOXBaseSection::UpdateMarks(const SwTOXInternational& rIntl,
        const SwTextNode* pOwnChapterNode,
        SwRootFrame const*const pLayout)
{
    const auto pType = static_cast<SwTOXType*>(SwTOXBase::GetRegisteredIn());
    auto pShell = GetFormat()->GetDoc()->GetDocShell();
    const TOXTypes eTOXTyp = GetTOXType()->GetType();
    std::vector<std::reference_wrapper<SwTextTOXMark>> vMarks;
    pType->CollectTextTOXMarksForLayout(vMarks, pLayout);
    for(auto& rMark: vMarks)
    {
        ::SetProgressState(0, pShell);
        auto& rNode = rMark.get().GetTextNode();
        if(IsFromChapter() && !IsHeadingContained(pOwnChapterNode, rNode))
            continue;
        auto rTOXMark = rMark.get().GetTOXMark();
        if(TOX_INDEX == eTOXTyp)
        {
            // index entry mark
            assert(g_pBreakIt);
            lang::Locale aLocale = g_pBreakIt->GetLocale(rNode.GetLang(rMark.get().GetStart()));
            InsertSorted(MakeSwTOXSortTabBase<SwTOXIndex>(pLayout, rNode, &rMark.get(), GetOptions(), FORM_ENTRY, rIntl, aLocale));
            if(GetOptions() & SwTOIOptions::KeyAsEntry && !rTOXMark.GetPrimaryKey().isEmpty())
            {
                InsertSorted(MakeSwTOXSortTabBase<SwTOXIndex>(pLayout, rNode, &rMark.get(), GetOptions(), FORM_PRIMARY_KEY, rIntl, aLocale));
                if (!rTOXMark.GetSecondaryKey().isEmpty())
                {
                    InsertSorted(MakeSwTOXSortTabBase<SwTOXIndex>(pLayout, rNode, &rMark.get(), GetOptions(), FORM_SECONDARY_KEY, rIntl, aLocale));
                }
            }
        }
        else if(TOX_USER == eTOXTyp || rTOXMark.GetLevel() <= GetLevel())
        {   // table of content mark, also used for user marks
            InsertSorted(MakeSwTOXSortTabBase<SwTOXContent>(pLayout, rNode, &rMark.get(), rIntl));
        }
    }
}

/// Generate table of contents from outline
void SwTOXBaseSection::UpdateOutline( const SwTextNode* pOwnChapterNode,
        SwRootFrame const*const pLayout)
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();

    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    for( auto pOutlineNode : rOutlNds )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );
        SwTextNode* pTextNd = pOutlineNode->GetTextNode();
        if( pTextNd && pTextNd->Len() && pTextNd->HasWriterListeners() &&
            o3tl::make_unsigned( pTextNd->GetAttrOutlineLevel()) <= GetLevel() &&
            pTextNd->getLayoutFrame(pLayout) &&
           !pTextNd->IsHiddenByParaField() &&
           !pTextNd->HasHiddenCharAttribute( true ) &&
           (!pLayout || !pLayout->HasMergedParas()
                || static_cast<SwTextFrame*>(pTextNd->getLayoutFrame(pLayout))->GetTextNodeForParaProps() == pTextNd) &&
            ( !IsFromChapter() || IsHeadingContained(pOwnChapterNode, *pTextNd) ))
        {
            InsertSorted(MakeSwTOXSortTabBase<SwTOXPara>(pLayout, *pTextNd, SwTOXElement::OutlineLevel));
        }
    }
}

/// Generate table of contents from template areas
void SwTOXBaseSection::UpdateTemplate(const SwTextNode* pOwnChapterNode,
        SwRootFrame const*const pLayout)
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        const OUString sTmpStyleNames = GetStyleNames(i);
        if (sTmpStyleNames.isEmpty())
            continue;

        sal_Int32 nIndex = 0;
        while (nIndex >= 0)
        {
            SwTextFormatColl* pColl = pDoc->FindTextFormatCollByName(
                                    sTmpStyleNames.getToken( 0, TOX_STYLE_DELIMITER, nIndex ));
            //TODO: no outline Collections in content indexes if OutlineLevels are already included
            if( !pColl ||
                ( TOX_CONTENT == SwTOXBase::GetType() &&
                  GetCreateType() & SwTOXElement::OutlineLevel &&
                    pColl->IsAssignedToListLevelOfOutlineStyle()) )
                continue;

            SwIterator<SwTextNode,SwFormatColl> aIter( *pColl );
            for( SwTextNode* pTextNd = aIter.First(); pTextNd; pTextNd = aIter.Next() )
            {
                ::SetProgressState( 0, pDoc->GetDocShell() );

                if (pTextNd->GetText().getLength() &&
                    pTextNd->getLayoutFrame(pLayout) &&
                    pTextNd->GetNodes().IsDocNodes() &&
                    (!pLayout || !pLayout->HasMergedParas()
                        || static_cast<SwTextFrame*>(pTextNd->getLayoutFrame(pLayout))->GetTextNodeForParaProps() == pTextNd) &&
                    (!IsFromChapter() || IsHeadingContained(pOwnChapterNode, *pTextNd)))
                {
                    InsertSorted(MakeSwTOXSortTabBase<SwTOXPara>(pLayout, *pTextNd, SwTOXElement::Template, i + 1));
                }
            }
        }
    }
}

/// Generate content from sequence fields
void SwTOXBaseSection::UpdateSequence(const SwTextNode* pOwnChapterNode,
        SwRootFrame const*const pLayout)
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    SwFieldType* pSeqField = pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::SetExp, GetSequenceName(), false);
    if(!pSeqField)
        return;

    std::vector<SwFormatField*> vFields;
    pSeqField->GatherFields(vFields);
    for(auto pFormatField: vFields)
    {
        const SwTextField* pTextField = pFormatField->GetTextField();
        SwTextNode& rTextNode = pTextField->GetTextNode();
        ::SetProgressState( 0, pDoc->GetDocShell() );

        if (rTextNode.GetText().getLength() &&
            rTextNode.getLayoutFrame(pLayout) &&
            ( !IsFromChapter() || IsHeadingContained(pOwnChapterNode, rTextNode))
            && (!pLayout || !pLayout->IsHideRedlines()
                || !sw::IsFieldDeletedInModel(pDoc->getIDocumentRedlineAccess(), *pTextField)))
        {
            const SwSetExpField& rSeqField = dynamic_cast<const SwSetExpField&>(*(pFormatField->GetField()));
            const OUString sName = GetSequenceName()
                + OUStringChar(cSequenceMarkSeparator)
                + OUString::number( rSeqField.GetSeqNumber() );
            std::unique_ptr<SwTOXPara> pNew(new SwTOXPara( rTextNode, SwTOXElement::Sequence, 1, sName ));
            // set indexes if the number or the reference text are to be displayed
            if( GetCaptionDisplay() == CAPTION_TEXT )
            {
                pNew->SetStartIndex(
                    SwGetExpField::GetReferenceTextPos( *pFormatField, *pDoc ));
            }
            else if(GetCaptionDisplay() == CAPTION_NUMBER)
            {
                pNew->SetEndIndex(pTextField->GetStart() + 1);
            }
            pNew->InitText(pLayout);
            InsertSorted(std::move(pNew));
        }
    }
}

void SwTOXBaseSection::UpdateAuthorities(const SwTOXInternational& rIntl,
        SwRootFrame const*const pLayout)
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    SwFieldType* pAuthField = pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::TableOfAuthorities, OUString(), false);
    if(!pAuthField)
        return;

    std::vector<SwFormatField*> vFields;
    pAuthField->GatherFields(vFields);
    for(auto pFormatField: vFields)
    {
        const auto pTextField = pFormatField->GetTextField();
        const SwTextNode& rTextNode = pFormatField->GetTextField()->GetTextNode();
        ::SetProgressState( 0, pDoc->GetDocShell() );

        if (rTextNode.GetText().getLength() &&
            rTextNode.getLayoutFrame(pLayout) &&
            (!pLayout || !pLayout->IsHideRedlines()
                || !sw::IsFieldDeletedInModel(pDoc->getIDocumentRedlineAccess(), *pTextField)))
        {
            //#106485# the body node has to be used!
            SwContentFrame *const pFrame = rTextNode.getLayoutFrame(pLayout);
            SwPosition aFieldPos(rTextNode);
            const SwTextNode* pTextNode = nullptr;
            if(pFrame && !pFrame->IsInDocBody())
                pTextNode = GetBodyTextNode( *pDoc, aFieldPos, *pFrame );
            if(!pTextNode)
                pTextNode = &rTextNode;

            InsertSorted(MakeSwTOXSortTabBase<SwTOXAuthority>(pLayout, *pTextNode, *pFormatField, rIntl));
        }
    }
}

static SwTOOElements lcl_IsSOObject( const SvGlobalName& rFactoryNm )
{
    static const struct SoObjType {
        SwTOOElements nFlag;
        // GlobalNameId
        struct {
            sal_uInt32 n1;
            sal_uInt16 n2, n3;
            sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
        } aGlNmIds[4];
    } aArr[] = {
        { SwTOOElements::Math,
          { {SO3_SM_CLASSID_60},{SO3_SM_CLASSID_50},
            {SO3_SM_CLASSID_40},{SO3_SM_CLASSID_30} } },
        { SwTOOElements::Chart,
          { {SO3_SCH_CLASSID_60},{SO3_SCH_CLASSID_50},
            {SO3_SCH_CLASSID_40},{SO3_SCH_CLASSID_30} } },
        { SwTOOElements::Calc,
          { {SO3_SC_CLASSID_60},{SO3_SC_CLASSID_50},
            {SO3_SC_CLASSID_40},{SO3_SC_CLASSID_30} } },
        { SwTOOElements::DrawImpress,
          { {SO3_SIMPRESS_CLASSID_60},{SO3_SIMPRESS_CLASSID_50},
            {SO3_SIMPRESS_CLASSID_40},{SO3_SIMPRESS_CLASSID_30} } },
        { SwTOOElements::DrawImpress,
          { {SO3_SDRAW_CLASSID_60},{SO3_SDRAW_CLASSID_50} } }
    };

    for( SoObjType const & rArr : aArr )
        for (auto & rId : rArr.aGlNmIds)
        {
            if( !rId.n1 )
                break;
            SvGlobalName aGlbNm( rId.n1, rId.n2, rId.n3,
                        rId.b8, rId.b9, rId.b10, rId.b11,
                        rId.b12, rId.b13, rId.b14, rId.b15 );
            if( rFactoryNm == aGlbNm )
            {
                return rArr.nFlag;
            }
        }

    return SwTOOElements::NONE;
}

void SwTOXBaseSection::UpdateContent( SwTOXElement eMyType,
        const SwTextNode* pOwnChapterNode,
        SwRootFrame const*const pLayout)
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();
    // on the 1st Node of the 1st Section
    SwNodeOffset nIdx = rNds.GetEndOfAutotext().StartOfSectionIndex() + SwNodeOffset(2),
         nEndIdx = rNds.GetEndOfAutotext().GetIndex();

    while( nIdx < nEndIdx )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        SwNode* pNd = rNds[ nIdx ];
        SwContentNode* pCNd = nullptr;
        switch( eMyType )
        {
        case SwTOXElement::Frame:
            if( !pNd->IsNoTextNode() )
            {
                pCNd = pNd->GetContentNode();
                if( !pCNd )
                {
                    SwNodeIndex aTmp( *pNd );
                    pCNd = rNds.GoNext( &aTmp );
                }
            }
            break;
        case SwTOXElement::Graphic:
            if( pNd->IsGrfNode() )
                pCNd = static_cast<SwContentNode*>(pNd);
            break;
        case SwTOXElement::Ole:
            if( pNd->IsOLENode() )
            {
                bool bInclude = true;
                if(TOX_OBJECTS == SwTOXBase::GetType())
                {
                    SwOLENode* pOLENode = pNd->GetOLENode();
                    SwTOOElements nMyOLEOptions = GetOLEOptions();
                    SwOLEObj& rOLEObj = pOLENode->GetOLEObj();

                    if( rOLEObj.IsOleRef() )    // Not yet loaded
                    {
                        SvGlobalName aTmpName( rOLEObj.GetOleRef()->getClassID() );
                        SwTOOElements nObj = ::lcl_IsSOObject( aTmpName );
                        bInclude = ( (nMyOLEOptions & SwTOOElements::Other) && SwTOOElements::NONE == nObj )
                                   || (nMyOLEOptions & nObj);
                    }
                    else
                    {
                        OSL_FAIL("OLE Object no loaded?");
                        bInclude = false;
                    }
                }

                if(bInclude)
                    pCNd = static_cast<SwContentNode*>(pNd);
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
                const SwTextNode* pOutlNd = ::lcl_FindChapterNode( *pCNd,
                                                pLayout, MAXLEVEL - 1);
                if( pOutlNd )
                {
                    if( pOutlNd->GetTextColl()->IsAssignedToListLevelOfOutlineStyle())
                    {
                        nSetLevel = pOutlNd->GetTextColl()->GetAttrOutlineLevel();
                    }
                }
            }

            if (pCNd->getLayoutFrame(pLayout)
                && (!pLayout || !pLayout->HasMergedParas()
                    || pCNd->GetRedlineMergeFlag() != SwNode::Merge::Hidden)
                && ( !IsFromChapter() || IsHeadingContained(pOwnChapterNode, *pCNd)))
            {
                std::unique_ptr<SwTOXPara> pNew( MakeSwTOXSortTabBase<SwTOXPara>(
                        pLayout, *pCNd, eMyType,
                            ( USHRT_MAX != nSetLevel )
                            ? o3tl::narrowing<sal_uInt16>(nSetLevel)
                            : FORM_ALPHA_DELIMITER ) );
                InsertSorted( std::move(pNew) );
            }
        }

        nIdx = pNd->StartOfSectionNode()->EndOfSectionIndex() + SwNodeOffset(2);  // 2 == End/Start Node
    }
}

/// Collect table entries
void SwTOXBaseSection::UpdateTable(const SwTextNode* pOwnChapterNode,
        SwRootFrame const*const pLayout)
{
    SwDoc* pDoc = GetFormat()->GetDoc();
    SwNodes& rNds = pDoc->GetNodes();
    const SwFrameFormats& rArr = *pDoc->GetTableFrameFormats();

    for( auto pFrameFormat : rArr )
    {
        ::SetProgressState( 0, pDoc->GetDocShell() );

        SwTable* pTmpTable = SwTable::FindTable( pFrameFormat );
        SwTableBox* pFBox;
        if( pTmpTable && nullptr != (pFBox = pTmpTable->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() && pFBox->GetSttNd()->GetNodes().IsDocNodes() )
        {
            const SwTableNode* pTableNd = pFBox->GetSttNd()->FindTableNode();
            SwNodeIndex aContentIdx( *pTableNd, 1 );

            SwContentNode* pCNd;
            while( nullptr != ( pCNd = rNds.GoNext( &aContentIdx ) ) &&
                aContentIdx.GetIndex() < pTableNd->EndOfSectionIndex() )
            {
                if (pCNd->getLayoutFrame(pLayout)
                    && (!pLayout || !pLayout->HasMergedParas()
                        || pCNd->GetRedlineMergeFlag() != SwNode::Merge::Hidden)
                    && (!IsFromChapter() || IsHeadingContained(pOwnChapterNode, *pCNd)))
                {
                    std::unique_ptr<SwTOXTable> pNew(new SwTOXTable( *pCNd ));
                    if( IsLevelFromChapter() && TOX_TABLES != SwTOXBase::GetType())
                    {
                        const SwTextNode* pOutlNd =
                            ::lcl_FindChapterNode(*pCNd, pLayout, MAXLEVEL - 1);
                        if( pOutlNd )
                        {
                            if( pOutlNd->GetTextColl()->IsAssignedToListLevelOfOutlineStyle())
                            {
                                const int nTmp = pOutlNd->GetTextColl()->GetAttrOutlineLevel();
                                pNew->SetLevel(o3tl::narrowing<sal_uInt16>(nTmp));
                            }
                        }
                    }
                    pNew->InitText(pLayout);
                    InsertSorted(std::move(pNew));
                    break;
                }
            }
        }
    }
}

/// Calculate PageNumber and insert after formatting
void SwTOXBaseSection::UpdatePageNum()
{
    if( m_aSortArr.empty() )
        return ;

    // Insert the current PageNumber into the TOC
    SwPageFrame*  pCurrentPage    = nullptr;
    sal_uInt16      nPage       = 0;
    SwDoc* pDoc = GetFormat()->GetDoc();

    SwTOXInternational aIntl( GetLanguage(),
                              TOX_INDEX == GetTOXType()->GetType() ?
                              GetOptions() : SwTOIOptions::NONE,
                              GetSortAlgorithm() );

    for( size_t nCnt = 0; nCnt < m_aSortArr.size(); ++nCnt )
    {
        // Loop over all SourceNodes

        // process run in lines
        size_t nRange = 0;
        if(GetTOXForm().IsCommaSeparated() &&
                m_aSortArr[nCnt]->GetType() == TOX_SORT_INDEX)
        {
            const SwTOXMark& rMark = m_aSortArr[nCnt]->pTextMark->GetTOXMark();
            const OUString& sPrimKey = rMark.GetPrimaryKey();
            const OUString& sSecKey = rMark.GetSecondaryKey();
            const SwTOXMark* pNextMark = nullptr;
            while(m_aSortArr.size() > (nCnt + nRange)&&
                    m_aSortArr[nCnt + nRange]->GetType() == TOX_SORT_INDEX &&
                    nullptr != (pNextMark = &(m_aSortArr[nCnt + nRange]->pTextMark->GetTOXMark())) &&
                    pNextMark->GetPrimaryKey() == sPrimKey &&
                    pNextMark->GetSecondaryKey() == sSecKey)
                nRange++;
        }
        else
            nRange = 1;

        for(size_t nRunInEntry = nCnt; nRunInEntry < nCnt + nRange; ++nRunInEntry)
        {
            std::vector<sal_uInt16> aNums; // the PageNumber
            std::vector<SwPageDesc*> aDescs; // The PageDescriptors matching the PageNumbers
            std::vector<sal_uInt16> aMainNums; // contains page numbers of main entries
            SwTOXSortTabBase* pSortBase = m_aSortArr[nRunInEntry].get();
            size_t nSize = pSortBase->aTOXSources.size();
            for (size_t j = 0; j < nSize; ++j)
            {
                ::SetProgressState( 0, pDoc->GetDocShell() );

                SwTOXSource& rTOXSource = pSortBase->aTOXSources[j];
                if( rTOXSource.pNd )
                {
                    SwContentFrame* pFrame = rTOXSource.pNd->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
                    OSL_ENSURE( pFrame || pDoc->IsUpdateTOX(), "TOX, no Frame found");
                    if( !pFrame )
                        continue;
                    if( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->HasFollow() )
                    {
                        // find the right one
                        SwTextFrame* pNext;
                        TextFrameIndex const nPos(static_cast<SwTextFrame*>(pFrame)
                            ->MapModelToView(static_cast<SwTextNode const*>(rTOXSource.pNd),
                                rTOXSource.nPos));
                        for (;;)
                        {
                            pNext = static_cast<SwTextFrame*>(pFrame->GetFollow());
                            if (!pNext || nPos < pNext->GetOffset())
                                break;
                            pFrame = pNext;
                        }
                    }

                    SwPageFrame*  pTmpPage = pFrame->FindPageFrame();
                    if( pTmpPage != pCurrentPage )
                    {
                        nPage       = pTmpPage->GetVirtPageNum();
                        pCurrentPage    = pTmpPage;
                    }

                    // Insert as sorted
                    std::vector<sal_uInt16>::size_type i;
                    for( i = 0; i < aNums.size() && aNums[i] < nPage; ++i )
                        ;

                    if( i >= aNums.size() || aNums[ i ] != nPage )
                    {
                        aNums.insert(aNums.begin() + i, nPage);
                        aDescs.insert(aDescs.begin() + i, pCurrentPage->GetPageDesc() );
                    }
                    // is it a main entry?
                    if(TOX_SORT_INDEX == pSortBase->GetType() &&
                        rTOXSource.bMainEntry)
                    {
                        aMainNums.push_back(nPage);
                    }
                }
            }
            // Insert the PageNumber into the TOC TextNode
            const SwTOXSortTabBase* pBase = m_aSortArr[ nCnt ].get();
            if(pBase->pTOXNd)
            {
                const SwTextNode* pTextNd = pBase->pTOXNd->GetTextNode();
                OSL_ENSURE( pTextNd, "no TextNode, wrong TOC" );

                UpdatePageNum_( const_cast<SwTextNode*>(pTextNd), aNums, aDescs, &aMainNums,
                                aIntl );
            }
        }
    }
    // Delete the mapping array after setting the right PageNumber
    m_aSortArr.clear();
}

/// Replace the PageNumber place holders. Search for the page no. in the array
/// of main entry page numbers.
static bool lcl_HasMainEntry( const std::vector<sal_uInt16>* pMainEntryNums, sal_uInt16 nToFind )
{
    if (!pMainEntryNums)
        return false;

    for( auto nMainEntry : *pMainEntryNums )
        if (nToFind == nMainEntry)
            return true;
    return false;
}

void SwTOXBaseSection::UpdatePageNum_( SwTextNode* pNd,
                                    const std::vector<sal_uInt16>& rNums,
                                    const std::vector<SwPageDesc*>& rDescs,
                                    const std::vector<sal_uInt16>* pMainEntryNums,
                                    const SwTOXInternational& rIntl )
{
    // collect starts end ends of main entry character style
    std::optional< std::vector<sal_uInt16> > xCharStyleIdx;
    if (pMainEntryNums)
        xCharStyleIdx.emplace();

    OUString sSrchStr
        = OUStringChar(C_NUM_REPL) + SwTOXMark::S_PAGE_DELI + OUStringChar(C_NUM_REPL);
    sal_Int32 nStartPos = pNd->GetText().indexOf(sSrchStr);
    sSrchStr = OUStringChar(C_NUM_REPL) + OUStringChar(C_END_PAGE_NUM);
    sal_Int32 nEndPos = pNd->GetText().indexOf(sSrchStr);

    if (-1 == nEndPos || rNums.empty())
        return;

    if (-1 == nStartPos || nStartPos > nEndPos)
        nStartPos = nEndPos;

    sal_uInt16 nOld = rNums[0],
           nBeg = nOld,
           nCount  = 0;
    OUString aNumStr( rDescs[0]->GetNumType().GetNumStr( nBeg ) );
    if( xCharStyleIdx && lcl_HasMainEntry( pMainEntryNums, nBeg ))
    {
        xCharStyleIdx->push_back( 0 );
    }

    // Delete place holder
    SwIndex aPos(pNd, nStartPos);
    SwCharFormat* pPageNoCharFormat = nullptr;
    SwpHints* pHints = pNd->GetpSwpHints();
    if(pHints)
        for(size_t nHintIdx = 0; nHintIdx < pHints->Count(); ++nHintIdx)
        {
            const SwTextAttr* pAttr = pHints->Get(nHintIdx);
            const sal_Int32 nTmpEnd = pAttr->End() ? *pAttr->End() : 0;
            if( nStartPos >= pAttr->GetStart() &&
                (nStartPos + 2) <= nTmpEnd &&
                pAttr->Which() == RES_TXTATR_CHARFMT)
            {
                pPageNoCharFormat = pAttr->GetCharFormat().GetCharFormat();
                break;
            }
        }
    pNd->EraseText(aPos, nEndPos - nStartPos + 2);

    std::vector<sal_uInt16>::size_type i;
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
                (GetOptions() & (SwTOIOptions::FF|SwTOIOptions::Dash)))
                nCount++;
            else
            {
                // Flush for the following old values
                if(GetOptions() & SwTOIOptions::FF)
                {
                    if ( nCount >= 1 )
                        aNumStr += rIntl.GetFollowingText( nCount > 1 );
                }
                else if (nCount) //#58127# If nCount == 0, then the only PageNumber is already in aNumStr!
                {
                    if (nCount == 1 )
                        aNumStr += SwTOXMark::S_PAGE_DELI;
                    else
                        aNumStr += "-";

                    aNumStr += aType.GetNumStr( nBeg + nCount );
                }

                // Create new String
                nBeg     = rNums[i];
                aNumStr += SwTOXMark::S_PAGE_DELI;
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
            aNumStr += aType.GetNumStr( rNums[i] );
            if (i+1 != rNums.size())
                aNumStr += SwTOXMark::S_PAGE_DELI;
        }
    }
    // Flush when ending and the following old values
    if( TOX_INDEX == SwTOXBase::GetType() )
    {
        if(GetOptions() & SwTOIOptions::FF)
        {
            if( nCount >= 1 )
                aNumStr += rIntl.GetFollowingText( nCount > 1 );
        }
        else
        {
            if(nCount >= 2)
                aNumStr += "-";
            else if(nCount == 1)
                aNumStr += SwTOXMark::S_PAGE_DELI;
            //#58127# If nCount == 0, then the only PageNumber is already in aNumStr!
            if(nCount)
                aNumStr += rDescs[i-1]->GetNumType().GetNumStr( nBeg+nCount );
        }
    }
    pNd->InsertText( aNumStr, aPos, SwInsertFlags::EMPTYEXPAND | SwInsertFlags::FORCEHINTEXPAND );
    if(pPageNoCharFormat)
    {
        SwFormatCharFormat aCharFormat( pPageNoCharFormat );
        pNd->InsertItem(aCharFormat, nStartPos, nStartPos + aNumStr.getLength(), SetAttrMode::DONTEXPAND);
    }

    // The main entries should get their character style
    if (!xCharStyleIdx || xCharStyleIdx->empty() || GetMainEntryCharStyle().isEmpty())
        return;

    // eventually the last index must me appended
    if (xCharStyleIdx->size()&0x01)
        xCharStyleIdx->push_back(aNumStr.getLength());

    // search by name
    SwDoc& rDoc = pNd->GetDoc();
    sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( GetMainEntryCharStyle(), SwGetPoolIdFromName::ChrFmt );
    SwCharFormat* pCharFormat = nullptr;
    if(USHRT_MAX != nPoolId)
        pCharFormat = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool(nPoolId);
    else
        pCharFormat = rDoc.FindCharFormatByName( GetMainEntryCharStyle() );
    if(!pCharFormat)
        pCharFormat = rDoc.MakeCharFormat(GetMainEntryCharStyle(), nullptr);

    // find the page numbers in aNumStr and set the character style
    sal_Int32 nOffset = pNd->GetText().getLength() - aNumStr.getLength();
    SwFormatCharFormat aCharFormat(pCharFormat);
    for (size_t j = 0; j < xCharStyleIdx->size(); j += 2)
    {
        sal_Int32 nStartIdx = (*xCharStyleIdx)[j] + nOffset;
        sal_Int32 nEndIdx   = (*xCharStyleIdx)[j + 1]  + nOffset;
        pNd->InsertItem(aCharFormat, nStartIdx, nEndIdx, SetAttrMode::DONTEXPAND);
    }
}

void SwTOXBaseSection::InsertSorted(std::unique_ptr<SwTOXSortTabBase> pNew)
{
    Range aRange(0, m_aSortArr.size());
    if( TOX_INDEX == SwTOXBase::GetType() && pNew->pTextMark )
    {
        const SwTOXMark& rMark = pNew->pTextMark->GetTOXMark();
        // Evaluate Key
        // Calculate the range where to insert
        if( !(GetOptions() & SwTOIOptions::KeyAsEntry) &&
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
        for(short i = static_cast<short>(aRange.Min()); i < static_cast<short>(aRange.Max()); ++i)
        {
            SwTOXSortTabBase* pOld = m_aSortArr[i].get();
            if (pOld->equivalent(*pNew))
            {
                if (pOld->sort_lt(*pNew))
                {
                    return;
                }
                else
                {
                    // remove the old content
                    m_aSortArr.erase( m_aSortArr.begin() + i );
                    aRange.Max()--;
                    break;
                }
            }
        }
    }

    // find position and insert
    tools::Long i;

    for( i = aRange.Min(); i < aRange.Max(); ++i)
    {   // Only check for same level
        SwTOXSortTabBase* pOld = m_aSortArr[i].get();
        if (pOld->equivalent(*pNew))
        {
            if(TOX_AUTHORITIES != SwTOXBase::GetType())
            {
                // Own entry for double entries or keywords
                if( pOld->GetType() == TOX_SORT_CUSTOM &&
                    SwTOXSortTabBase::GetOptions() & SwTOIOptions::KeyAsEntry)
                    continue;

                if(!(SwTOXSortTabBase::GetOptions() & SwTOIOptions::SameEntry))
                {   // Own entry
                    m_aSortArr.insert(m_aSortArr.begin() + i, std::move(pNew));
                    return;
                }
                // If the own entry is already present, add it to the references list
                pOld->aTOXSources.push_back(pNew->aTOXSources[0]);

                return;
            }
#if OSL_DEBUG_LEVEL > 0
            else
                OSL_FAIL("Bibliography entries cannot be found here");
#endif
        }
        if (pNew->sort_lt(*pOld))
            break;
    }
    // Skip SubLevel
    while( TOX_INDEX == SwTOXBase::GetType() && i < aRange.Max() &&
          m_aSortArr[i]->GetLevel() > pNew->GetLevel() )
        i++;

    // Insert at position i
    m_aSortArr.insert(m_aSortArr.begin()+i, std::move(pNew));
}

/// Find Key Range and insert if possible
Range SwTOXBaseSection::GetKeyRange(const OUString& rStr, const OUString& rStrReading,
                                    const SwTOXSortTabBase& rNew,
                                    sal_uInt16 nLevel, const Range& rRange )
{
    const SwTOXInternational& rIntl = *rNew.pTOXIntl;
    TextAndReading aToCompare(rStr, rStrReading);

    if( SwTOIOptions::InitialCaps & GetOptions() )
    {
        aToCompare.sText = rIntl.ToUpper( aToCompare.sText, 0 )
                         + aToCompare.sText.subView(1);
    }

    OSL_ENSURE(rRange.Min() >= 0 && rRange.Max() >= 0, "Min Max < 0");

    const tools::Long nMin = rRange.Min();
    const tools::Long nMax = rRange.Max();

    tools::Long i;

    for( i = nMin; i < nMax; ++i)
    {
        SwTOXSortTabBase* pBase = m_aSortArr[i].get();

        if( rIntl.IsEqual( pBase->GetText(), pBase->GetLocale(),
                           aToCompare, rNew.GetLocale() )  &&
                    pBase->GetLevel() == nLevel )
            break;
    }
    if(i == nMax)
    {   // If not already present, create and insert
        std::unique_ptr<SwTOXCustom> pKey(MakeSwTOXSortTabBase<SwTOXCustom>(
                    nullptr, aToCompare, nLevel, rIntl, rNew.GetLocale() ));
        for(i = nMin; i < nMax; ++i)
        {
            if (nLevel == m_aSortArr[i]->GetLevel() && pKey->sort_lt(*m_aSortArr[i]))
                break;
        }
        m_aSortArr.insert(m_aSortArr.begin() + i, std::move(pKey));
    }
    const tools::Long nStart = i+1;
    const tools::Long nEnd   = m_aSortArr.size();

    // Find end of range
    for(i = nStart; i < nEnd; ++i)
    {
        if(m_aSortArr[i]->GetLevel() <= nLevel)
        {
            return Range(nStart, i);
        }
    }
    return Range(nStart, nEnd);
}

bool SwTOXBase::IsTOXBaseInReadonly() const
{
    const SwTOXBaseSection *pSect = dynamic_cast<const SwTOXBaseSection*>(this);
    if (!pSect || !pSect->GetFormat())
        return false;

    const SwSectionNode* pSectNode = pSect->GetFormat()->GetSectionNode();
    if (!pSectNode)
        return false;

    const SwDocShell* pDocSh = pSectNode->GetDoc().GetDocShell();
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
    if(pSect && pSect->GetFormat())
        return &pSect->GetFormat()->GetAttrSet();
    return nullptr;
}

void SwTOXBase::SetAttrSet( const SfxItemSet& rSet )
{
    SwTOXBaseSection *pSect = dynamic_cast<SwTOXBaseSection*>(this);
    if( pSect && pSect->GetFormat() )
        pSect->GetFormat()->SetFormatAttr( rSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
