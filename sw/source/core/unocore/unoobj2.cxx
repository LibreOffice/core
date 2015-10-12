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

#include <sal/config.h>

#include <utility>

#include <rtl/ustrbuf.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <hints.hxx>
#include <IMark.hxx>
#include <bookmrk.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <textboxhelper.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <unocrsr.hxx>
#include <swundo.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <ftnidx.hxx>
#include <sfx2/linkmgr.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <swerror.h>
#include <swtblfmt.hxx>
#include <docsh.hxx>
#include <docstyle.hxx>
#include <charfmt.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include <edimp.hxx>
#include <fchrfmt.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doctxm.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <unoparaframeenum.hxx>
#include <unofootnote.hxx>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <unoparagraph.hxx>
#include <unomap.hxx>
#include <unoport.hxx>
#include <unocrsrhelper.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <unotbl.hxx>
#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unostyle.hxx>
#include <fmtanchr.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/unolingu.hxx>
#include <svtools/ctrltool.hxx>
#include <flypos.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtcntnt.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <fmtflcnt.hxx>
#include <editeng/brushitem.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <crsskip.hxx>
#include <vector>
#include <sortedobjs.hxx>
#include <sortopt.hxx>
#include <algorithm>
#include <iterator>
#include <calbck.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

namespace sw {

uno::Sequence< OUString >
GetSupportedServiceNamesImpl(
        size_t const nServices, char const*const pServices[])
{
    uno::Sequence< OUString > ret(nServices);
    for (size_t i = 0; i < nServices; ++i)
    {
        ret[i] = OUString::createFromAscii(pServices[i]);
    }
    return ret;
}

} // namespace sw

namespace sw {

void DeepCopyPaM(SwPaM const & rSource, SwPaM & rTarget)
{
    rTarget = rSource;

    if (rSource.GetNext() != &rSource)
    {
        SwPaM *pPam = const_cast<SwPaM*>(rSource.GetNext());
        do
        {
            // create new PaM
            SwPaM *const pNew = new SwPaM(*pPam, nullptr);
            // insert into ring
            pNew->MoveTo(&rTarget);
            pPam = pPam->GetNext();
        }
        while (pPam != &rSource);
    }
}

} // namespace sw

struct FrameClientSortListLess
{
    bool operator() (FrameClientSortListEntry const& r1,
                     FrameClientSortListEntry const& r2) const
    {
        return  (r1.nIndex <  r2.nIndex)
            || ((r1.nIndex == r2.nIndex) && (r1.nOrder < r2.nOrder));
    }
};

namespace
{
    void lcl_CollectFrameAtNodeWithLayout(SwDoc* pDoc, const SwContentFrm* pCFrm,
            FrameClientSortList_t& rFrames,
            const sal_uInt16 nAnchorType)
    {
        auto pObjs = pCFrm->GetDrawObjs();
        if(!pObjs)
            return;
        const auto aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);
        for(const auto pAnchoredObj : *pObjs)
        {
            SwFrameFormat& rFormat = pAnchoredObj->GetFrameFormat();
            // Filter out textboxes, which are not interesting at an UNO level.
            if(aTextBoxes.find(&rFormat) != aTextBoxes.end())
                continue;
            if(rFormat.GetAnchor().GetAnchorId() == nAnchorType)
            {
                const auto nIdx =
                    rFormat.GetAnchor().GetContentAnchor()->nContent.GetIndex();
                const auto nOrder = rFormat.GetAnchor().GetOrder();
                FrameClientSortListEntry entry(nIdx, nOrder, new sw::FrameClient(&rFormat));
                rFrames.push_back(entry);
            }
        }
    }
}


void CollectFrameAtNode( const SwNodeIndex& rIdx,
                         FrameClientSortList_t& rFrames,
                         const bool bAtCharAnchoredObjs )
{
    // _bAtCharAnchoredObjs:
    // <true>: at-character anchored objects are collected
    // <false>: at-paragraph anchored objects are collected

    // search all borders, images, and OLEs that are connected to the paragraph
    SwDoc* pDoc = rIdx.GetNode().GetDoc();

    const auto nChkType = static_cast< sal_uInt16 >((bAtCharAnchoredObjs)
            ? FLY_AT_CHAR : FLY_AT_PARA);
    const SwContentFrm* pCFrm;
    const SwContentNode* pCNd;
    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() &&
        0 != (pCNd = rIdx.GetNode().GetContentNode()) &&
        0 != (pCFrm = pCNd->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout())) )
    {
        lcl_CollectFrameAtNodeWithLayout(pDoc, pCFrm, rFrames, nChkType);
    }
    else
    {
        const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
        const size_t nSize = rFormats.size();
        for ( size_t i = 0; i < nSize; i++)
        {
            const SwFrameFormat* pFormat = rFormats[ i ];
            const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
            const SwPosition* pAnchorPos;
            if( rAnchor.GetAnchorId() == nChkType &&
                0 != (pAnchorPos = rAnchor.GetContentAnchor()) &&
                    pAnchorPos->nNode == rIdx )
            {

                // OD 2004-05-07 #i28701# - determine insert position for
                // sorted <rFrameArr>
                const sal_Int32 nIndex = pAnchorPos->nContent.GetIndex();
                sal_uInt32 nOrder = rAnchor.GetOrder();

                FrameClientSortListEntry entry(nIndex, nOrder, new sw::FrameClient(const_cast<SwFrameFormat*>(pFormat)));
                rFrames.push_back(entry);
            }
        }
        ::std::sort(rFrames.begin(), rFrames.end(), FrameClientSortListLess());
    }
}

UnoActionContext::UnoActionContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    SwRootFrm *const pRootFrm = m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    if (pRootFrm)
    {
        pRootFrm->StartAllAction();
    }
}

UnoActionContext::~UnoActionContext()
{
    // Doc may already have been removed here
    if (m_pDoc)
    {
        SwRootFrm *const pRootFrm = m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
        if (pRootFrm)
        {
            pRootFrm->EndAllAction();
        }
    }
}

static void lcl_RemoveImpl(SwDoc *const pDoc)
{
    assert(pDoc);
    SwRootFrm *const pRootFrm = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    if (pRootFrm)
    {
        pRootFrm->UnoRemoveAllActions();
    }
}

UnoActionRemoveContext::UnoActionRemoveContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    lcl_RemoveImpl(m_pDoc);
}

static SwDoc * lcl_IsNewStyleTable(SwUnoTableCrsr const& rCursor)
{
    SwTableNode *const pTableNode = rCursor.GetNode().FindTableNode();
    return (pTableNode && !pTableNode->GetTable().IsNewModel())
        ? rCursor.GetDoc()
        : nullptr;
}

UnoActionRemoveContext::UnoActionRemoveContext(SwUnoTableCrsr const& rCursor)
    : m_pDoc(lcl_IsNewStyleTable(rCursor))
{
    // this insanity is only necessary for old-style tables
    // because SwRootFrm::MakeTableCrsrs() creates the table cursor for these
    if (m_pDoc)
    {
        lcl_RemoveImpl(m_pDoc);
    }
}

UnoActionRemoveContext::~UnoActionRemoveContext()
{
    if (m_pDoc)
    {
        SwRootFrm *const pRootFrm = m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
        if (pRootFrm)
        {
            pRootFrm->UnoRestoreAllActions();
        }
    }
}

void ClientModify(SwClient* pClient, const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( static_cast<void*>(pClient->GetRegisteredIn()) == static_cast<const SwPtrMsgPoolItem *>(pOld)->pObject )
            pClient->GetRegisteredIn()->Remove(pClient);
        break;

    case RES_FMT_CHG:
        // Is the move to the new one finished and will the old one be deleted?
        if( static_cast<const SwFormatChg*>(pNew)->pChangedFormat == pClient->GetRegisteredIn() &&
            static_cast<const SwFormatChg*>(pOld)->pChangedFormat->IsFormatInDTOR() )
            static_cast<SwModify*>(pClient->GetRegisteredIn())->Remove(pClient);
        break;
    }
}

void SwUnoCursorHelper::SetCrsrAttr(SwPaM & rPam,
        const SfxItemSet& rSet,
        const SetAttrMode nAttrMode, const bool bTableMode)
{
    const SetAttrMode nFlags = nAttrMode | SetAttrMode::APICALL;
    SwDoc* pDoc = rPam.GetDoc();
    //StartEndAction
    UnoActionContext aAction(pDoc);
    if (rPam.GetNext() != &rPam)    // Ring of Cursors
    {
        pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSATTR, NULL);

        for(SwPaM& rCurrent : rPam.GetRingContainer())
        {
            if (rCurrent.HasMark() &&
                ( (bTableMode) ||
                  (*rCurrent.GetPoint() != *rCurrent.GetMark()) ))
            {
                pDoc->getIDocumentContentOperations().InsertItemSet(rCurrent, rSet, nFlags);
            }
        }

        pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSATTR, NULL);
    }
    else
    {
        pDoc->getIDocumentContentOperations().InsertItemSet( rPam, rSet, nFlags );
    }

    if( rSet.GetItemState( RES_PARATR_OUTLINELEVEL, false ) >= SfxItemState::DEFAULT )
    {
        SwTextNode * pTmpNode = rPam.GetNode().GetTextNode();
        if ( pTmpNode )
        {
            rPam.GetDoc()->GetNodes().UpdateOutlineNode( *pTmpNode );
        }
    }
}

// #i63870#
// split third parameter <bCurrentAttrOnly> into new parameters <bOnlyTextAttr>
// and <bGetFromChrFormat> to get better control about resulting <SfxItemSet>
void SwUnoCursorHelper::GetCrsrAttr(SwPaM & rPam,
        SfxItemSet & rSet, const bool bOnlyTextAttr, const bool bGetFromChrFormat)
{
    static const sal_uLong nMaxLookup = 1000;
    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;
    for(SwPaM& rCurrent : rPam.GetRingContainer())
    {
        SwPosition const & rStart( *rCurrent.Start() );
        SwPosition const & rEnd( *rCurrent.End() );
        const sal_uLong nSttNd = rStart.nNode.GetIndex();
        const sal_uLong nEndNd = rEnd  .nNode.GetIndex();

        if (nEndNd - nSttNd >= nMaxLookup)
        {
            rSet.ClearItem();
            rSet.InvalidateAllItems();
            return;// uno::Any();
        }

        // the first node inserts the values into the get set
        // all other nodes merge their values into the get set
        for (sal_uLong n = nSttNd; n <= nEndNd; ++n)
        {
            SwNode *const pNd = rPam.GetDoc()->GetNodes()[ n ];
            switch (pNd->GetNodeType())
            {
                case ND_TEXTNODE:
                {
                    const sal_Int32 nStart = (n == nSttNd)
                        ? rStart.nContent.GetIndex() : 0;
                    const sal_Int32 nEnd   = (n == nEndNd)
                        ? rEnd.nContent.GetIndex()
                        : pNd->GetTextNode()->GetText().getLength();
                    pNd->GetTextNode()->GetAttr(*pSet, nStart, nEnd, bOnlyTextAttr, bGetFromChrFormat);
                }
                break;

                case ND_GRFNODE:
                case ND_OLENODE:
                    static_cast<SwContentNode*>(pNd)->GetAttr( *pSet );
                break;

                default:
                    continue; // skip this node
            }

            if (pSet != &rSet)
            {
                rSet.MergeValues( aSet );
            }
            else
            {
                pSet = &aSet;
            }

            if (aSet.Count())
            {
                aSet.ClearItem();
            }
        }
    }
}

struct SwXParagraphEnumerationImpl SAL_FINAL : public SwXParagraphEnumeration
{
    uno::Reference< text::XText > const m_xParentText;
    const CursorType m_eCursorType;
    /// Start node of the cell _or_ table the enumeration belongs to.
    /// Used to restrict the movement of the UNO cursor to the cell and its
    /// embedded tables.
    SwStartNode const*const m_pOwnStartNode;
    SwTable const*const m_pOwnTable;
    const sal_uLong m_nEndIndex;
    sal_Int32 m_nFirstParaStart;
    sal_Int32 m_nLastParaEnd;
    bool m_bFirstParagraph;
    uno::Reference< text::XTextContent > m_xNextPara;
    sw::UnoCursorPointer m_pCrsr;

    SwXParagraphEnumerationImpl(
            uno::Reference< text::XText > const& xParent,
            ::std::shared_ptr<SwUnoCrsr> pCursor,
            const CursorType eType,
            SwStartNode const*const pStartNode, SwTable const*const pTable)
        : m_xParentText( xParent )
        , m_eCursorType( eType )
        // remember table and start node for later travelling
        // (used in export of tables in tables)
        , m_pOwnStartNode( pStartNode )
        // for import of tables in tables we have to remember the actual
        // table and start node of the current position in the enumeration.
        , m_pOwnTable( pTable )
        , m_nEndIndex( pCursor->End()->nNode.GetIndex() )
        , m_nFirstParaStart( -1 )
        , m_nLastParaEnd( -1 )
        , m_bFirstParagraph( true )
        , m_pCrsr(pCursor)
    {
        OSL_ENSURE(m_xParentText.is(), "SwXParagraphEnumeration: no parent?");
        OSL_ENSURE(   !((CURSOR_SELECTION_IN_TABLE == eType) ||
                        (CURSOR_TBLTEXT == eType))
                   || (m_pOwnTable && m_pOwnStartNode),
            "SwXParagraphEnumeration: table type but no start node or table?");

        if ((CURSOR_SELECTION == m_eCursorType) ||
            (CURSOR_SELECTION_IN_TABLE == m_eCursorType))
        {
            SwUnoCrsr & rCursor = GetCursor();
            rCursor.Normalize();
            m_nFirstParaStart = rCursor.GetPoint()->nContent.GetIndex();
            m_nLastParaEnd = rCursor.GetMark()->nContent.GetIndex();
            rCursor.DeleteMark();
        }
    }

    virtual ~SwXParagraphEnumerationImpl()
        { m_pCrsr.reset(nullptr); }
    virtual void SAL_CALL release() throw () override
    {
        SolarMutexGuard g;
        OWeakObject::release();
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return OUString("SwXParagraphEnumeration"); }
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName) throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return cppu::supportsService(this, rServiceName); };
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return {"com.sun.star.text.ParagraphEnumeration"}; };

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    SwUnoCrsr& GetCursor()
        { return *m_pCrsr; }
    uno::Reference< text::XTextContent > NextElement_Impl()
        throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
};

SwXParagraphEnumeration* SwXParagraphEnumeration::Create(
    uno::Reference< text::XText > const& xParent,
    ::std::shared_ptr<SwUnoCrsr> pCursor,
    const CursorType eType,
    SwStartNode const*const pStartNode,
    SwTable const*const pTable)
{
    return new SwXParagraphEnumerationImpl(xParent, pCursor, eType, pStartNode, pTable);
}

sal_Bool SAL_CALL
SwXParagraphEnumerationImpl::hasMoreElements() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return m_bFirstParagraph || m_xNextPara.is();
}

//!! compare to SwShellTableCrsr::FillRects() in viscrs.cxx
static SwTableNode *
lcl_FindTopLevelTable(
        SwTableNode *const pTableNode, SwTable const*const pOwnTable)
{
    // find top-most table in current context (section) level

    SwTableNode * pLast = pTableNode;
    for (SwTableNode* pTmp = pLast;
         pTmp != NULL  &&  &pTmp->GetTable() != pOwnTable;  /* we must not go up higher than the own table! */
         pTmp = pTmp->StartOfSectionNode()->FindTableNode() )
    {
        pLast = pTmp;
    }
    return pLast;
}

static bool
lcl_CursorIsInSection(
        SwUnoCrsr const*const pUnoCrsr, SwStartNode const*const pOwnStartNode)
{
    // returns true if the cursor is in the section (or in a sub section!)
    // represented by pOwnStartNode

    bool bRes = true;
    if (pUnoCrsr && pOwnStartNode)
    {
        const SwEndNode * pOwnEndNode = pOwnStartNode->EndOfSectionNode();
        bRes = pOwnStartNode->GetIndex() <= pUnoCrsr->Start()->nNode.GetIndex() &&
               pUnoCrsr->End()->nNode.GetIndex() <= pOwnEndNode->GetIndex();
    }
    return bRes;
}

uno::Reference< text::XTextContent >
SwXParagraphEnumerationImpl::NextElement_Impl() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SwUnoCrsr& rUnoCrsr = GetCursor();

    // check for exceeding selections
    if (!m_bFirstParagraph &&
        ((CURSOR_SELECTION == m_eCursorType) ||
         (CURSOR_SELECTION_IN_TABLE == m_eCursorType)))
    {
        SwPosition* pStart = rUnoCrsr.Start();
        auto aNewCrsr(rUnoCrsr.GetDoc()->CreateUnoCrsr(*pStart));
        // one may also go into tables here
        if ((CURSOR_TBLTEXT != m_eCursorType) &&
            (CURSOR_SELECTION_IN_TABLE != m_eCursorType))
        {
            aNewCrsr->SetRemainInSection( false );
        }

        // os 2005-01-14: This part is only necessary to detect movements out
        // of a selection; if there is no selection we don't have to care
        SwTableNode *const pTableNode = aNewCrsr->GetNode().FindTableNode();
        if (((CURSOR_TBLTEXT != m_eCursorType) &&
            (CURSOR_SELECTION_IN_TABLE != m_eCursorType)) && pTableNode)
        {
            aNewCrsr->GetPoint()->nNode = pTableNode->EndOfSectionIndex();
            aNewCrsr->Move(fnMoveForward, fnGoNode);
        }
        else
        {
            aNewCrsr->MovePara(fnParaNext, fnParaStart);
        }
        if (m_nEndIndex < aNewCrsr->Start()->nNode.GetIndex())
        {
            return 0;
        }
    }

    bool bInTable = false;
    if (!m_bFirstParagraph)
    {
        rUnoCrsr.SetRemainInSection( false );
        // what to do if already in a table?
        SwTableNode * pTableNode = rUnoCrsr.GetNode().FindTableNode();
        pTableNode = lcl_FindTopLevelTable( pTableNode, m_pOwnTable );
        if (pTableNode && (&pTableNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table: go to end
            rUnoCrsr.GetPoint()->nNode = pTableNode->EndOfSectionIndex();
            if (!rUnoCrsr.Move(fnMoveForward, fnGoNode))
            {
                return 0;
            }
            bInTable = true;
        }
    }

    uno::Reference< text::XTextContent >  xRef;
    // the cursor must remain in the current section or a subsection
    // before AND after the movement...
    if (lcl_CursorIsInSection( &rUnoCrsr, m_pOwnStartNode ) &&
        (m_bFirstParagraph || bInTable ||
        (rUnoCrsr.MovePara(fnParaNext, fnParaStart) &&
            lcl_CursorIsInSection( &rUnoCrsr, m_pOwnStartNode ))))
    {
        SwPosition* pStart = rUnoCrsr.Start();
        const sal_Int32 nFirstContent =
            (m_bFirstParagraph) ? m_nFirstParaStart : -1;
        const sal_Int32 nLastContent =
            (m_nEndIndex == pStart->nNode.GetIndex()) ? m_nLastParaEnd : -1;

        // position in a table, or in a simple paragraph?
        SwTableNode * pTableNode = rUnoCrsr.GetNode().FindTableNode();
        pTableNode = lcl_FindTopLevelTable( pTableNode, m_pOwnTable );
        if (/*CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType && */
            pTableNode && (&pTableNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table
            SwFrameFormat* pTableFormat =
                static_cast<SwFrameFormat*>(pTableNode->GetTable().GetFrameFormat());
            xRef = SwXTextTable::CreateXTextTable(pTableFormat);
        }
        else
        {
            text::XText *const pText = m_xParentText.get();
            xRef = SwXParagraph::CreateXParagraph(*rUnoCrsr.GetDoc(),
                pStart->nNode.GetNode().GetTextNode(),
                static_cast<SwXText*>(pText), nFirstContent, nLastContent);
        }
    }

    return xRef;
}

uno::Any SAL_CALL SwXParagraphEnumerationImpl::nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_bFirstParagraph)
    {
        m_xNextPara = NextElement_Impl();
        m_bFirstParagraph = false;
    }
    const uno::Reference< text::XTextContent > xRef = m_xNextPara;
    if (!xRef.is())
    {
        throw container::NoSuchElementException();
    }
    m_xNextPara = NextElement_Impl();

    uno::Any aRet;
    aRet <<= xRef;
    return aRet;
}

class SwXTextRange::Impl
    : public SwClient
{
public:
    const SfxItemPropertySet &  m_rPropSet;
    const enum RangePosition    m_eRangePosition;
    SwDoc &                     m_rDoc;
    uno::Reference<text::XText> m_xParentText;
    SwDepend            m_ObjectDepend; // register at format of table or frame
    ::sw::mark::IMark * m_pMark;

    Impl(   SwDoc & rDoc, const enum RangePosition eRange,
            SwFrameFormat *const pTableFormat = 0,
            const uno::Reference< text::XText > & xParent = 0)
        : SwClient()
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
        , m_eRangePosition(eRange)
        , m_rDoc(rDoc)
        , m_xParentText(xParent)
        , m_ObjectDepend(this, pTableFormat)
        , m_pMark(0)
    {
    }

    virtual ~Impl()
    {
        // Impl owns the bookmark; delete it here: SolarMutex is locked
        Invalidate();
    }

    void Invalidate()
    {
        if (m_pMark)
        {
            m_rDoc.getIDocumentMarkAccess()->deleteMark(m_pMark);
            m_pMark = 0;
        }
    }

    const ::sw::mark::IMark * GetBookmark() const { return m_pMark; }

protected:
    // SwClient
    virtual void    Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew) override;
};

void SwXTextRange::Impl::Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    const bool bAlreadyRegistered = 0 != GetRegisteredIn();
    ClientModify(this, pOld, pNew);
    if (m_ObjectDepend.GetRegisteredIn())
    {
        ClientModify(&m_ObjectDepend, pOld, pNew);
        // if the depend was removed then the range must be removed too
        if (!m_ObjectDepend.GetRegisteredIn() && GetRegisteredIn())
        {
            GetRegisteredIn()->Remove(this);
        }
        // or if the range has been removed but the depend is still
        // connected then the depend must be removed
        else if (bAlreadyRegistered && !GetRegisteredIn() &&
                    m_ObjectDepend.GetRegisteredIn())
        {
            m_ObjectDepend.GetRegisteredIn()
                ->Remove(& m_ObjectDepend);
        }
    }
    if (!GetRegisteredIn())
    {
        m_pMark = 0;
    }
}

SwXTextRange::SwXTextRange(SwPaM& rPam,
        const uno::Reference< text::XText > & xParent,
        const enum RangePosition eRange)
    : m_pImpl( new SwXTextRange::Impl(*rPam.GetDoc(), eRange, 0, xParent) )
{
    SetPositions(rPam);
}

SwXTextRange::SwXTextRange(SwFrameFormat& rTableFormat)
    : m_pImpl(
        new SwXTextRange::Impl(*rTableFormat.GetDoc(), RANGE_IS_TABLE, &rTableFormat) )
{
    SwTable *const pTable = SwTable::FindTable( &rTableFormat );
    SwTableNode *const pTableNode = pTable->GetTableNode();
    SwPosition aPosition( *pTableNode );
    SwPaM aPam( aPosition );

    SetPositions( aPam );
}

SwXTextRange::~SwXTextRange()
{
}

const SwDoc& SwXTextRange::GetDoc() const
{
    return m_pImpl->m_rDoc;
}

SwDoc& SwXTextRange::GetDoc()
{
    return m_pImpl->m_rDoc;
}

void SwXTextRange::Invalidate()
{
    m_pImpl->Invalidate();
}

void SwXTextRange::SetPositions(const SwPaM& rPam)
{
    m_pImpl->Invalidate();
    IDocumentMarkAccess* const pMA = m_pImpl->m_rDoc.getIDocumentMarkAccess();
    m_pImpl->m_pMark = pMA->makeMark(rPam, OUString(),
                IDocumentMarkAccess::MarkType::UNO_BOOKMARK);
    m_pImpl->m_pMark->Add(m_pImpl.get());
}

void SwXTextRange::DeleteAndInsert(
        const OUString& rText, const bool bForceExpandHints)
throw (uno::RuntimeException)
{
    if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // setString on table not allowed
        throw uno::RuntimeException();
    }

    const SwPosition aPos(GetDoc().GetNodes().GetEndOfContent());
    SwCursor aCursor(aPos, 0, false);
    if (GetPositions(aCursor))
    {
        UnoActionContext aAction(& m_pImpl->m_rDoc);
        m_pImpl->m_rDoc.GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);
        if (aCursor.HasMark())
        {
            m_pImpl->m_rDoc.getIDocumentContentOperations().DeleteAndJoin(aCursor);
        }

        if (!rText.isEmpty())
        {
            SwUnoCursorHelper::DocInsertStringSplitCR(
                    m_pImpl->m_rDoc, aCursor, rText, bForceExpandHints);

            SwUnoCursorHelper::SelectPam(aCursor, true);
            aCursor.Left(rText.getLength(), CRSR_SKIP_CHARS, false, false);
        }
        SetPositions(aCursor);
        m_pImpl->m_rDoc.GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
    }
}

namespace
{
    class theSwXTextRangeUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextRangeUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextRange::getUnoTunnelId()
{
    return theSwXTextRangeUnoTunnelId::get().getSeq();
}

// XUnoTunnel
sal_Int64 SAL_CALL
SwXTextRange::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException, std::exception)
{
    return ::sw::UnoTunnelImpl<SwXTextRange>(rId, this);
}

OUString SAL_CALL
SwXTextRange::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("SwXTextRange");
}

static char const*const g_ServicesTextRange[] =
{
    "com.sun.star.text.TextRange",
    "com.sun.star.style.CharacterProperties",
    "com.sun.star.style.CharacterPropertiesAsian",
    "com.sun.star.style.CharacterPropertiesComplex",
    "com.sun.star.style.ParagraphProperties",
    "com.sun.star.style.ParagraphPropertiesAsian",
    "com.sun.star.style.ParagraphPropertiesComplex",
};

static const size_t g_nServicesTextRange(
    sizeof(g_ServicesTextRange)/sizeof(g_ServicesTextRange[0]));

sal_Bool SAL_CALL SwXTextRange::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesTextRange, g_ServicesTextRange);
}

uno::Reference< text::XText > SAL_CALL
SwXTextRange::getText() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_xParentText.is())
    {
        if (m_pImpl->m_eRangePosition == RANGE_IS_TABLE &&
            m_pImpl->m_ObjectDepend.GetRegisteredIn())
        {
            SwFrameFormat const*const pTableFormat = static_cast<SwFrameFormat const*>(
                    m_pImpl->m_ObjectDepend.GetRegisteredIn());
            SwTable const*const pTable = SwTable::FindTable( pTableFormat );
            SwTableNode const*const pTableNode = pTable->GetTableNode();
            const SwPosition aPosition( *pTableNode );
            m_pImpl->m_xParentText =
                ::sw::CreateParentXText(m_pImpl->m_rDoc, aPosition);
        }
    }
    OSL_ENSURE(m_pImpl->m_xParentText.is(), "SwXTextRange::getText: no text");
    return m_pImpl->m_xParentText;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getStart() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XTextRange >  xRet;
    ::sw::mark::IMark const * const pBkmk = m_pImpl->GetBookmark();
    if (!m_pImpl->m_xParentText.is())
    {
        getText();
    }
    if(pBkmk)
    {
        SwPaM aPam(pBkmk->GetMarkStart());
        xRet = new SwXTextRange(aPam, m_pImpl->m_xParentText);
    }
    else if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // start and end are this, if it's a table
        xRet = this;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getEnd() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XTextRange >  xRet;
    ::sw::mark::IMark const * const pBkmk = m_pImpl->GetBookmark();
    if (!m_pImpl->m_xParentText.is())
    {
        getText();
    }
    if(pBkmk)
    {
        SwPaM aPam(pBkmk->GetMarkEnd());
        xRet = new SwXTextRange(aPam, m_pImpl->m_xParentText);
    }
    else if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // start and end are this, if it's a table
        xRet = this;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return xRet;
}

OUString SAL_CALL SwXTextRange::getString() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString sRet;
    // for tables there is no bookmark, thus also no text
    // one could export the table as ASCII here maybe?
    SwPaM aPaM(GetDoc().GetNodes());
    if (GetPositions(aPaM) && aPaM.HasMark())
    {
        SwUnoCursorHelper::GetTextFromPam(aPaM, sRet);
    }
    return sRet;
}

void SAL_CALL SwXTextRange::setString(const OUString& rString)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    DeleteAndInsert(rString, false);
}

bool SwXTextRange::GetPositions(SwPaM& rToFill) const
{
    ::sw::mark::IMark const * const pBkmk = m_pImpl->GetBookmark();
    if(pBkmk)
    {
        *rToFill.GetPoint() = pBkmk->GetMarkPos();
        if(pBkmk->IsExpanded())
        {
            rToFill.SetMark();
            *rToFill.GetMark() = pBkmk->GetOtherMarkPos();
        }
        else
        {
            rToFill.DeleteMark();
        }
        return true;
    }
    return false;
}

namespace sw {

bool XTextRangeToSwPaM( SwUnoInternalPaM & rToFill,
        const uno::Reference< text::XTextRange > & xTextRange)
{
    bool bRet = false;

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    SwXTextPortion* pPortion = 0;
    SwXText* pText = 0;
    SwXParagraph* pPara = 0;
    if(xRangeTunnel.is())
    {
        pRange  = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
        pPortion=
            ::sw::UnoTunnelGetImplementation<SwXTextPortion>(xRangeTunnel);
        pText   = ::sw::UnoTunnelGetImplementation<SwXText>(xRangeTunnel);
        pPara   = ::sw::UnoTunnelGetImplementation<SwXParagraph>(xRangeTunnel);
    }

    // if it's a text then create a temporary cursor there and re-use
    // the pCursor variable
    // #i108489#: Reference in outside scope to keep cursor alive
    uno::Reference< text::XTextCursor > xTextCursor;
    if (pText)
    {
        xTextCursor.set( pText->CreateCursor() );
        xTextCursor->gotoEnd(sal_True);
        const uno::Reference<lang::XUnoTunnel> xCrsrTunnel(
                xTextCursor, uno::UNO_QUERY);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xCrsrTunnel);
    }
    if(pRange && &pRange->GetDoc() == rToFill.GetDoc())
    {
        bRet = pRange->GetPositions(rToFill);
    }
    else
    {
        if (pPara)
        {
            bRet = pPara->SelectPaM(rToFill);
        }
        else
        {
            SwDoc* const pDoc = (pCursor) ? pCursor->GetDoc()
                : ((pPortion) ? pPortion->GetCursor().GetDoc() : 0);
            const SwPaM* const pUnoCrsr = (pCursor) ? pCursor->GetPaM()
                : ((pPortion) ? &pPortion->GetCursor() : 0);
            if (pUnoCrsr && pDoc == rToFill.GetDoc())
            {
                OSL_ENSURE(!pUnoCrsr->IsMultiSelection(),
                        "what to do about rings?");
                bRet = true;
                *rToFill.GetPoint() = *pUnoCrsr->GetPoint();
                if (pUnoCrsr->HasMark())
                {
                    rToFill.SetMark();
                    *rToFill.GetMark() = *pUnoCrsr->GetMark();
                }
                else
                    rToFill.DeleteMark();
            }
        }
    }
    return bRet;
}

static bool
lcl_IsStartNodeInFormat(const bool bHeader, SwStartNode *const pSttNode,
    SwFrameFormat const*const pFrameFormat, SwFrameFormat*& rpFormat)
{
    bool bRet = false;
    const SfxItemSet& rSet = pFrameFormat->GetAttrSet();
    const SfxPoolItem* pItem;
    if (SfxItemState::SET == rSet.GetItemState(
            static_cast<sal_uInt16>(bHeader ? RES_HEADER : RES_FOOTER),
            true, &pItem))
    {
        SfxPoolItem *const pItemNonConst(const_cast<SfxPoolItem *>(pItem));
        SwFrameFormat *const pHeadFootFormat = (bHeader) ?
            static_cast<SwFormatHeader*>(pItemNonConst)->GetHeaderFormat() :
            static_cast<SwFormatFooter*>(pItemNonConst)->GetFooterFormat();
        if (pHeadFootFormat)
        {
            const SwFormatContent& rFlyContent = pHeadFootFormat->GetContent();
            const SwNode& rNode = rFlyContent.GetContentIdx()->GetNode();
            SwStartNode const*const pCurSttNode = rNode.FindSttNodeByType(
                (bHeader) ? SwHeaderStartNode : SwFooterStartNode);
            if (pCurSttNode && (pCurSttNode == pSttNode))
            {
                rpFormat = pHeadFootFormat;
                bRet = true;
            }
        }
    }
    return bRet;
}

} // namespace sw

uno::Reference< text::XTextRange >
SwXTextRange::CreateXTextRange(
    SwDoc & rDoc, const SwPosition& rPos, const SwPosition *const pMark)
{
    const uno::Reference<text::XText> xParentText(
            ::sw::CreateParentXText(rDoc, rPos));
    const auto pNewCrsr(rDoc.CreateUnoCrsr(rPos));
    if(pMark)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    const bool isCell( dynamic_cast<SwXCell*>(xParentText.get()) );
    const uno::Reference< text::XTextRange > xRet(
        new SwXTextRange(*pNewCrsr, xParentText,
            isCell ? RANGE_IN_CELL : RANGE_IN_TEXT) );
    return xRet;
}

namespace sw {

uno::Reference< text::XText >
CreateParentXText(SwDoc & rDoc, const SwPosition& rPos)
{
    uno::Reference< text::XText > xParentText;
    SwStartNode* pSttNode = rPos.nNode.GetNode().StartOfSectionNode();
    while(pSttNode && pSttNode->IsSectionNode())
    {
        pSttNode = pSttNode->StartOfSectionNode();
    }
    SwStartNodeType eType = pSttNode ? pSttNode->GetStartNodeType() : SwNormalStartNode;
    switch(eType)
    {
        case SwTableBoxStartNode:
        {
            SwTableNode const*const pTableNode = pSttNode->FindTableNode();
            SwFrameFormat *const pTableFormat =
                static_cast<SwFrameFormat*>(pTableNode->GetTable().GetFrameFormat());
            SwTableBox *const  pBox = pSttNode->GetTableBox();

            xParentText = (pBox)
                ? SwXCell::CreateXCell( pTableFormat, pBox )
                : new SwXCell( pTableFormat, *pSttNode );
        }
        break;
        case SwFlyStartNode:
        {
            SwFrameFormat *const pFormat = pSttNode->GetFlyFormat();
            if (0 != pFormat)
            {
                xParentText.set(SwXTextFrame::CreateXTextFrame(rDoc, pFormat),
                        uno::UNO_QUERY);
            }
        }
        break;
        case SwHeaderStartNode:
        case SwFooterStartNode:
        {
            const bool bHeader = (SwHeaderStartNode == eType);
            const size_t nPDescCount = rDoc.GetPageDescCnt();
            for(size_t i = 0; i < nPDescCount; i++)
            {
                const SwPageDesc& rDesc = rDoc.GetPageDesc( i );
                const SwFrameFormat* pFrameFormatMaster = &rDesc.GetMaster();
                const SwFrameFormat* pFrameFormatLeft = &rDesc.GetLeft();

                SwFrameFormat* pHeadFootFormat = 0;
                if (!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrameFormatMaster,
                            pHeadFootFormat))
                {
                    lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrameFormatLeft,
                            pHeadFootFormat);
                }

                if (pHeadFootFormat)
                {
                    xParentText = SwXHeadFootText::CreateXHeadFootText(
                            *pHeadFootFormat, bHeader);
                }
            }
        }
        break;
        case SwFootnoteStartNode:
        {
            const size_t nFootnoteCnt = rDoc.GetFootnoteIdxs().size();
            uno::Reference< text::XFootnote >  xRef;
            for (size_t n = 0; n < nFootnoteCnt; ++n )
            {
                const SwTextFootnote* pTextFootnote = rDoc.GetFootnoteIdxs()[ n ];
                const SwFormatFootnote& rFootnote = pTextFootnote->GetFootnote();
                pTextFootnote = rFootnote.GetTextFootnote();
#if OSL_DEBUG_LEVEL > 1
                const SwStartNode* pTmpSttNode =
                        pTextFootnote->GetStartNode()->GetNode().
                                FindSttNodeByType(SwFootnoteStartNode);
                (void)pTmpSttNode;
#endif

                if (pSttNode == pTextFootnote->GetStartNode()->GetNode().
                                    FindSttNodeByType(SwFootnoteStartNode))
                {
                    xParentText.set(SwXFootnote::CreateXFootnote(rDoc,
                            &const_cast<SwFormatFootnote&>(rFootnote)), uno::UNO_QUERY);
                    break;
                }
            }
        }
        break;
        default:
        {
            // then it is the body text
            const uno::Reference<frame::XModel> xModel =
                rDoc.GetDocShell()->GetBaseModel();
            const uno::Reference< text::XTextDocument > xDoc(
                xModel, uno::UNO_QUERY);
            xParentText = xDoc->getText();
        }
    }
    OSL_ENSURE(xParentText.is(), "no parent text?");
    return xParentText;
}

} // namespace sw

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextRange::createContentEnumeration(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if ( rServiceName != "com.sun.star.text.TextContent" )
    {
        throw uno::RuntimeException();
    }

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    const SwPosition aPos(GetDoc().GetNodes().GetEndOfContent());
    const auto pNewCrsr(m_pImpl->m_rDoc.CreateUnoCrsr(aPos));
    if (!GetPositions(*pNewCrsr))
    {
        throw uno::RuntimeException();
    }

    return SwXParaFrameEnumeration::Create(*pNewCrsr, PARAFRAME_PORTION_TEXTRANGE);
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextRange::createEnumeration() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    const SwPosition aPos(GetDoc().GetNodes().GetEndOfContent());
    auto pNewCrsr(m_pImpl->m_rDoc.CreateUnoCrsr(aPos));
    if (!GetPositions(*pNewCrsr))
    {
        throw uno::RuntimeException();
    }
    if (!m_pImpl->m_xParentText.is())
    {
        getText();
    }

    const CursorType eSetType = (RANGE_IN_CELL == m_pImpl->m_eRangePosition)
            ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    return SwXParagraphEnumeration::Create(m_pImpl->m_xParentText, pNewCrsr, eSetType);
}

uno::Type SAL_CALL SwXTextRange::getElementType() throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXTextRange::hasElements() throw (uno::RuntimeException, std::exception)
{
    return sal_True;
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getAvailableServiceNames() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextContent";
    return aRet;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextRange::getPropertySetInfo() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    static uno::Reference< beans::XPropertySetInfo > xRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xRef;
}

void SAL_CALL
SwXTextRange::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    SwUnoCursorHelper::SetPropertyValue(aPaM, m_pImpl->m_rPropSet,
            rPropertyName, rValue);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyValue(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL
SwXTextRange::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextRange::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextRange::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextRange::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXTextRange::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState SAL_CALL
SwXTextRange::getPropertyState(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyState(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextRange::getPropertyStates(const uno::Sequence< OUString >& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyStates(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL SwXTextRange::setPropertyToDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    SwUnoCursorHelper::SetPropertyToDefault(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyDefault(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL
SwXTextRange::makeRedline(
    const OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc().GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwUnoCursorHelper::makeRedline( aPaM, rRedlineType, rRedlineProperties );
}

struct SwXTextRangesImpl SAL_FINAL : public SwXTextRanges
{

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return OUString("SwXTextRanges"); };
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName) throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return cppu::supportsService(this, rServiceName); };
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return { "com.sun.star.text.TextRanges" }; };

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return cppu::UnoType<text::XTextRange>::get(); };
    virtual sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return getCount() > 0; };
    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    explicit SwXTextRangesImpl(SwPaM *const pPaM)
    {
        if (pPaM)
        {
            m_pUnoCursor.reset(pPaM->GetDoc()->CreateUnoCrsr(*pPaM->GetPoint()));
            ::sw::DeepCopyPaM(*pPaM, *GetCursor());
        }
        MakeRanges();
    }
    virtual void SAL_CALL release() throw () override
    {
        SolarMutexGuard g;
        OWeakObject::release();
    }
    virtual SwUnoCrsr* GetCursor() override
        { return &(*m_pUnoCursor); };
    void MakeRanges();
    ::std::vector< uno::Reference< text::XTextRange > > m_Ranges;
    sw::UnoCursorPointer m_pUnoCursor;
};

void SwXTextRangesImpl::MakeRanges()
{
    if (GetCursor())
    {
        for(SwPaM& rTmpCursor : GetCursor()->GetRingContainer())
        {
            const uno::Reference< text::XTextRange > xRange(
                    SwXTextRange::CreateXTextRange(
                        *rTmpCursor.GetDoc(),
                        *rTmpCursor.GetPoint(), rTmpCursor.GetMark()));
            if (xRange.is())
            {
                m_Ranges.push_back(xRange);
            }
        }
    }
}

SwXTextRanges* SwXTextRanges::Create(SwPaM *const pPaM)
    { return new SwXTextRangesImpl(pPaM); }

namespace
{
    class theSwXTextRangesUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextRangesUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextRanges::getUnoTunnelId()
    { return theSwXTextRangesUnoTunnelId::get().getSeq(); }

sal_Int64 SAL_CALL
SwXTextRangesImpl::getSomething(const uno::Sequence< sal_Int8 >& rId)
    throw (uno::RuntimeException, std::exception)
{
    return ::sw::UnoTunnelImpl<SwXTextRanges>(rId, this);
}

/*
 *  Text positions
 * Up to the first access to a text position, only a SwCursor is stored.
 * Afterwards, an array with uno::Reference<XTextPosition> will be created.
 */

sal_Int32 SAL_CALL SwXTextRangesImpl::getCount()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return static_cast<sal_Int32>(m_Ranges.size());
}

uno::Any SAL_CALL SwXTextRangesImpl::getByIndex(sal_Int32 nIndex) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ((nIndex < 0) || (static_cast<size_t>(nIndex) >= m_Ranges.size()))
        throw lang::IndexOutOfBoundsException();
    uno::Any ret;
    ret <<= (m_Ranges.at(nIndex));
    return ret;
}

void SwUnoCursorHelper::SetString(SwCursor & rCursor, const OUString& rString)
{
    // Start/EndAction
    SwDoc *const pDoc = rCursor.GetDoc();
    UnoActionContext aAction(pDoc);
    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);
    if (rCursor.HasMark())
    {
        pDoc->getIDocumentContentOperations().DeleteAndJoin(rCursor);
    }
    if (!rString.isEmpty())
    {
        const bool bSuccess( SwUnoCursorHelper::DocInsertStringSplitCR(
                    *pDoc, rCursor, rString, false ) );
        OSL_ENSURE( bSuccess, "DocInsertStringSplitCR" );
        (void) bSuccess;
        SwUnoCursorHelper::SelectPam(rCursor, true);
        rCursor.Left(rString.getLength(), CRSR_SKIP_CHARS, false, false);
    }
    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
}

struct SwXParaFrameEnumerationImpl SAL_FINAL : public SwXParaFrameEnumeration
{
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return OUString("SwXParaFrameEnumeration"); };
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return cppu::supportsService(this, rServiceName); };
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        { return {"com.sun.star.util.ContentEnumeration"}; };

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    SwXParaFrameEnumerationImpl(const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode, SwFrameFormat* const pFormat = nullptr);
    virtual void SAL_CALL release() throw () override
    {
        SolarMutexGuard g;
        OWeakObject::release();
    }
    SwUnoCrsr& GetCursor()
        { return *m_pUnoCursor; }
    void PurgeFrameClients()
    {
        if(!m_pUnoCursor)
        {
            m_vFrames.clear();
            m_xNextObject = nullptr;
        }
        else
        {
            // removing orphaned SwDepends
            const auto iter = std::remove_if(m_vFrames.begin(), m_vFrames.end(),
                    [] (std::shared_ptr<sw::FrameClient>& rEntry) -> bool { return !rEntry->GetRegisteredIn(); });
            m_vFrames.erase(iter, m_vFrames.end());
        }
    }
    void FillFrame();
    bool CreateNextObject();
    uno::Reference< text::XTextContent > m_xNextObject;
    FrameClientList_t m_vFrames;
    ::sw::UnoCursorPointer m_pUnoCursor;
};



SwXParaFrameEnumeration* SwXParaFrameEnumeration::Create(const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode, SwFrameFormat* const pFormat)
    { return new SwXParaFrameEnumerationImpl(rPaM, eParaFrameMode, pFormat); }

SwXParaFrameEnumerationImpl::SwXParaFrameEnumerationImpl(
        const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode,
        SwFrameFormat* const pFormat)
    : m_pUnoCursor(rPaM.GetDoc()->CreateUnoCrsr(*rPaM.GetPoint()))
{
    if (rPaM.HasMark())
    {
        GetCursor().SetMark();
        *GetCursor().GetMark() = *rPaM.GetMark();
    }
    if (PARAFRAME_PORTION_PARAGRAPH == eParaFrameMode)
    {
        FrameClientSortList_t vFrames;
        ::CollectFrameAtNode(rPaM.GetPoint()->nNode, vFrames, false);
        ::std::transform(vFrames.begin(), vFrames.end(),
            ::std::back_inserter(m_vFrames),
            [] (const FrameClientSortListEntry& rEntry) { return rEntry.pFrameClient; });
    }
    else if (pFormat)
    {
        m_vFrames.push_back(std::shared_ptr<sw::FrameClient>(new sw::FrameClient(pFormat)));
    }
    else if ((PARAFRAME_PORTION_CHAR == eParaFrameMode) ||
             (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode))
    {
        if (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode)
        {
            //get all frames that are bound at paragraph or at character
            for(const auto& pFlyFrm : rPaM.GetDoc()->GetAllFlyFormats(&GetCursor(), false, true))
            {
                const auto pFrameFormat = const_cast<SwFrameFormat*>(&pFlyFrm->GetFormat());
                m_vFrames.push_back(std::shared_ptr<sw::FrameClient>(new sw::FrameClient(pFrameFormat)));
            }
        }
        FillFrame();
    }
}

// Search for a FLYCNT text attribute at the cursor point and fill the frame
// into the array
void SwXParaFrameEnumerationImpl::FillFrame()
{
    if(!m_pUnoCursor->GetNode().IsTextNode())
        return;
    // search for objects at the cursor - anchored at/as char
    const auto pTextAttr = m_pUnoCursor->GetNode().GetTextNode()->GetTextAttrForCharAt(
            m_pUnoCursor->GetPoint()->nContent.GetIndex(), RES_TXTATR_FLYCNT);
    if(!pTextAttr)
        return;
    const SwFormatFlyCnt& rFlyCnt = pTextAttr->GetFlyCnt();
    SwFrameFormat* const pFrameFormat = rFlyCnt.GetFrameFormat();
    m_vFrames.push_back(std::shared_ptr<sw::FrameClient>(new sw::FrameClient(pFrameFormat)));
}

bool SwXParaFrameEnumerationImpl::CreateNextObject()
{
    if (m_vFrames.empty())
        return false;

    SwFrameFormat* const pFormat = static_cast<SwFrameFormat*>(
            m_vFrames.front()->GetRegisteredIn());
    m_vFrames.pop_front();
    // the format should be valid here, otherwise the client
    // would have been removed by PurgeFrameClients
    // check for a shape first
    SwDrawContact* const pContact = SwIterator<SwDrawContact,SwFormat>( *pFormat ).First();
    if (pContact)
    {
        SdrObject* const pSdr = pContact->GetMaster();
        if (pSdr)
            m_xNextObject.set(pSdr->getUnoShape(), uno::UNO_QUERY);
    }
    else
    {
        const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
        OSL_ENSURE(pIdx, "where is the index?");
        SwNode const*const pNd =
            m_pUnoCursor->GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];

        if (!pNd->IsNoTextNode())
        {
            m_xNextObject.set(SwXTextFrame::CreateXTextFrame(
                        *pFormat->GetDoc(), pFormat));
        }
        else if (pNd->IsGrfNode())
        {
            m_xNextObject.set(SwXTextGraphicObject::CreateXTextGraphicObject(
                        *pFormat->GetDoc(), pFormat));
        }
        else
        {
            assert(pNd->IsOLENode());
            m_xNextObject.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                        *pFormat->GetDoc(), pFormat));
        }
    }
    return m_xNextObject.is();
}

sal_Bool SAL_CALL
SwXParaFrameEnumerationImpl::hasMoreElements() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    PurgeFrameClients();
    return m_xNextObject.is() || CreateNextObject();
}

uno::Any SAL_CALL SwXParaFrameEnumerationImpl::nextElement()
throw (container::NoSuchElementException,
        lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    PurgeFrameClients();
    if (!m_xNextObject.is() && !m_vFrames.empty())
        CreateNextObject();
    if (!m_xNextObject.is())
        throw container::NoSuchElementException();
    uno::Any aRet;
    aRet <<= m_xNextObject;
    m_xNextObject = nullptr;
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
