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

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/safeint.hxx>
#include <svl/listener.hxx>
#include <svx/svdobj.hxx>
#include <utility>
#include <vcl/svapp.hxx>

#include <anchoredobject.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <IMark.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <textboxhelper.hxx>
#include <ndtxt.hxx>
#include <unocrsr.hxx>
#include <unotextcursor.hxx>
#include <swundo.hxx>
#include <rootfrm.hxx>
#include <ftnidx.hxx>
#include <pam.hxx>
#include <swtblfmt.hxx>
#include <docsh.hxx>
#include <pagedesc.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <flyfrms.hxx>
#include <unoparaframeenum.hxx>
#include <unofootnote.hxx>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <unoparagraph.hxx>
#include <unomap.hxx>
#include <unoport.hxx>
#include <unocrsrhelper.hxx>
#include <unotbl.hxx>
#include <fmtanchr.hxx>
#include <flypos.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtcntnt.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <fmtflcnt.hxx>
#include <vector>
#include <sortedobjs.hxx>
#include <frameformats.hxx>
#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;

namespace sw {

void DeepCopyPaM(SwPaM const & rSource, SwPaM & rTarget)
{
    rTarget = rSource;

    if (rSource.GetNext() == &rSource)
        return;

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

} // namespace sw

namespace {

struct FrameClientSortListLess
{
    bool operator() (FrameClientSortListEntry const& r1,
                     FrameClientSortListEntry const& r2) const
    {
        return  (r1.nIndex <  r2.nIndex)
            || ((r1.nIndex == r2.nIndex) && (r1.nOrder < r2.nOrder));
    }
};

    void lcl_CollectFrameAtNodeWithLayout(const SwContentFrame* pCFrame,
            FrameClientSortList_t& rFrames,
            const RndStdIds nAnchorType)
    {
        auto pObjs = pCFrame->GetDrawObjs();
        if(!pObjs)
            return;
        for(const auto pAnchoredObj : *pObjs)
        {
            SwFrameFormat* pFormat = pAnchoredObj->GetFrameFormat();
            // Filter out textboxes, which are not interesting at a UNO level.
            if(SwTextBoxHelper::isTextBox(pFormat, RES_FLYFRMFMT))
                continue;

            if (nAnchorType == RndStdIds::FLY_AT_PARA)
            {
                auto pFlyAtContentFrame = dynamic_cast<SwFlyAtContentFrame*>(pAnchoredObj);
                if (pFlyAtContentFrame && pFlyAtContentFrame->IsFollow())
                {
                    // We're collecting frame formats, ignore non-master fly frames to prevent
                    // duplication.
                    continue;
                }
            }

            if(pFormat->GetAnchor().GetAnchorId() == nAnchorType)
            {
                const sal_Int32 nIdx = pFormat->GetAnchor().GetAnchorContentOffset();
                const auto nOrder = pFormat->GetAnchor().GetOrder();
                rFrames.emplace_back(nIdx, nOrder, std::make_unique<sw::FrameClient>(pFormat));
            }
        }
    }
}


void CollectFrameAtNode( const SwNode& rNd,
                         FrameClientSortList_t& rFrames,
                         const bool bAtCharAnchoredObjs )
{
    // _bAtCharAnchoredObjs:
    // <true>: at-character anchored objects are collected
    // <false>: at-paragraph anchored objects are collected

    // search all borders, images, and OLEs that are connected to the paragraph
    const SwDoc& rDoc = rNd.GetDoc();

    const auto nChkType = bAtCharAnchoredObjs ? RndStdIds::FLY_AT_CHAR : RndStdIds::FLY_AT_PARA;
    const SwContentFrame* pCFrame;
    const SwContentNode* pCNd;
    if( rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() &&
        nullptr != (pCNd = rNd.GetContentNode()) &&
        nullptr != (pCFrame = pCNd->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout())) )
    {
        lcl_CollectFrameAtNodeWithLayout(pCFrame, rFrames, nChkType);
    }
    else
    {
        for(sw::SpzFrameFormat* pSpz: *rDoc.GetSpzFrameFormats())
        {
            const SwFormatAnchor& rAnchor = pSpz->GetAnchor();
            const SwNode* pAnchorNode;
            if( rAnchor.GetAnchorId() == nChkType &&
                nullptr != (pAnchorNode = rAnchor.GetAnchorNode()) &&
                    *pAnchorNode == rNd )
            {

                // OD 2004-05-07 #i28701# - determine insert position for
                // sorted <rFrameArr>
                const sal_Int32 nIndex = rAnchor.GetAnchorContentOffset();
                sal_uInt32 nOrder = rAnchor.GetOrder();

                rFrames.emplace_back(nIndex, nOrder, std::make_unique<sw::FrameClient>(pSpz));
            }
        }
        std::sort(rFrames.begin(), rFrames.end(), FrameClientSortListLess());
    }
}

UnoActionContext::UnoActionContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    SwRootFrame *const pRootFrame = m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    if (pRootFrame)
    {
        pRootFrame->StartAllAction();
    }
}

UnoActionContext::~UnoActionContext() COVERITY_NOEXCEPT_FALSE
{
    // Doc may already have been removed here
    if (m_pDoc)
    {
        SwRootFrame *const pRootFrame = m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
        if (pRootFrame)
        {
            pRootFrame->EndAllAction();
        }
    }
}

static void lcl_RemoveImpl(SwDoc *const pDoc)
{
    assert(pDoc);
    SwRootFrame *const pRootFrame = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    if (pRootFrame)
    {
        pRootFrame->UnoRemoveAllActions();
    }
}

UnoActionRemoveContext::UnoActionRemoveContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    lcl_RemoveImpl(m_pDoc);
}

static SwDoc * lcl_IsNewStyleTable(SwUnoTableCursor const& rCursor)
{
    SwTableNode *const pTableNode = rCursor.GetPointNode().FindTableNode();
    return (pTableNode && !pTableNode->GetTable().IsNewModel())
        ? &rCursor.GetDoc()
        : nullptr;
}

UnoActionRemoveContext::UnoActionRemoveContext(SwUnoTableCursor const& rCursor)
    : m_pDoc(lcl_IsNewStyleTable(rCursor))
{
    // this insanity is only necessary for old-style tables
    // because SwRootFrame::MakeTableCursors() creates the table cursor for these
    if (m_pDoc)
    {
        lcl_RemoveImpl(m_pDoc);
    }
}

UnoActionRemoveContext::~UnoActionRemoveContext() COVERITY_NOEXCEPT_FALSE
{
    if (m_pDoc)
    {
        SwRootFrame *const pRootFrame = m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
        if (pRootFrame)
        {
            pRootFrame->UnoRestoreAllActions();
        }
    }
}

void SwUnoCursorHelper::SetCursorAttr(SwPaM & rPam,
        const SfxItemSet& rSet,
        const SetAttrMode nAttrMode, const bool bTableMode)
{
    const SetAttrMode nFlags = nAttrMode | SetAttrMode::APICALL;
    SwDoc& rDoc = rPam.GetDoc();
    //StartEndAction
    UnoActionContext aAction(&rDoc);
    if (rPam.GetNext() != &rPam)    // Ring of Cursors
    {
        rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::INSATTR, nullptr);

        for(SwPaM& rCurrent : rPam.GetRingContainer())
        {
            if (rCurrent.HasMark() &&
                ( bTableMode ||
                  (*rCurrent.GetPoint() != *rCurrent.GetMark()) ))
            {
                rDoc.getIDocumentContentOperations().InsertItemSet(rCurrent, rSet, nFlags);
            }
        }

        rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::INSATTR, nullptr);
    }
    else
    {
        rDoc.getIDocumentContentOperations().InsertItemSet( rPam, rSet, nFlags );
    }

    if( rSet.GetItemState( RES_PARATR_OUTLINELEVEL, false ) >= SfxItemState::DEFAULT )
    {
        SwTextNode * pTmpNode = rPam.GetPointNode().GetTextNode();
        if ( pTmpNode )
        {
            rPam.GetDoc().GetNodes().UpdateOutlineNode( *pTmpNode );
        }
    }
}

// #i63870#
// split third parameter <bCurrentAttrOnly> into new parameters <bOnlyTextAttr>
// and <bGetFromChrFormat> to get better control about resulting <SfxItemSet>
void SwUnoCursorHelper::GetCursorAttr(SwPaM & rPam,
        SfxItemSet & rSet, const bool bOnlyTextAttr, const bool bGetFromChrFormat)
{
    static const sal_uLong nMaxLookup = 1000;
    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;
    for(SwPaM& rCurrent : rPam.GetRingContainer())
    {
        SwPosition const & rStart( *rCurrent.Start() );
        SwPosition const & rEnd( *rCurrent.End() );
        const SwNodeOffset nSttNd = rStart.GetNodeIndex();
        const SwNodeOffset nEndNd = rEnd  .GetNodeIndex();

        if (nEndNd - nSttNd >= SwNodeOffset(nMaxLookup))
        {
            rSet.ClearItem();
            return;// uno::Any();
        }

        // the first node inserts the values into the get set
        // all other nodes merge their values into the get set
        for (SwNodeOffset n = nSttNd; n <= nEndNd; ++n)
        {
            SwNode *const pNd = rPam.GetDoc().GetNodes()[ n ];
            switch (pNd->GetNodeType())
            {
                case SwNodeType::Text:
                {
                    const sal_Int32 nStart = (n == nSttNd)
                        ? rStart.GetContentIndex() : 0;
                    const sal_Int32 nEnd   = (n == nEndNd)
                        ? rEnd.GetContentIndex()
                        : pNd->GetTextNode()->GetText().getLength();
                    pNd->GetTextNode()->GetParaAttr(*pSet, nStart, nEnd, bOnlyTextAttr, bGetFromChrFormat);
                }
                break;

                case SwNodeType::Grf:
                case SwNodeType::Ole:
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

namespace {

struct SwXParagraphEnumerationImpl final : public SwXParagraphEnumeration
{
    uno::Reference< text::XText > const m_xParentText;
    const CursorType m_eCursorType;
    /// Start node of the cell _or_ table the enumeration belongs to.
    /// Used to restrict the movement of the UNO cursor to the cell and its
    /// embedded tables.
    SwStartNode const*const m_pOwnStartNode;
    SwTable const*const m_pOwnTable;
    const SwNodeOffset m_nEndIndex;
    sal_Int32 m_nFirstParaStart;
    sal_Int32 m_nLastParaEnd;
    bool m_bFirstParagraph;
    uno::Reference< text::XTextContent > m_xNextPara;
    sw::UnoCursorPointer m_pCursor;

    SwXParagraphEnumerationImpl(
            uno::Reference< text::XText > xParent,
            const std::shared_ptr<SwUnoCursor>& pCursor,
            const CursorType eType,
            SwStartNode const*const pStartNode, SwTable const*const pTable)
        : m_xParentText(std::move( xParent ))
        , m_eCursorType( eType )
        // remember table and start node for later travelling
        // (used in export of tables in tables)
        , m_pOwnStartNode( pStartNode )
        // for import of tables in tables we have to remember the actual
        // table and start node of the current position in the enumeration.
        , m_pOwnTable( pTable )
        , m_nEndIndex( pCursor->End()->GetNodeIndex() )
        , m_nFirstParaStart( -1 )
        , m_nLastParaEnd( -1 )
        , m_bFirstParagraph( true )
        , m_pCursor(pCursor)
    {
        OSL_ENSURE(m_xParentText.is(), "SwXParagraphEnumeration: no parent?");
        assert( !((CursorType::SelectionInTable == eType)
                      || (CursorType::TableText == eType))
            || (m_pOwnTable && m_pOwnStartNode));

        if ((CursorType::Selection == m_eCursorType) ||
            (CursorType::SelectionInTable == m_eCursorType))
        {
            SwUnoCursor & rCursor = GetCursor();
            rCursor.Normalize();
            m_nFirstParaStart = rCursor.GetPoint()->GetContentIndex();
            m_nLastParaEnd = rCursor.GetMark()->GetContentIndex();
            rCursor.DeleteMark();
        }
    }

    virtual ~SwXParagraphEnumerationImpl() override
        { m_pCursor.reset(nullptr); }
    virtual void SAL_CALL release() noexcept override
    {
        SolarMutexGuard g;
        OWeakObject::release();
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
        { return "SwXParagraphEnumeration"; }
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName) override
        { return cppu::supportsService(this, rServiceName); };
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override
        { return {"com.sun.star.text.ParagraphEnumeration"}; };

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

    SwUnoCursor& GetCursor()
        { return *m_pCursor; }
    /// @throws container::NoSuchElementException
    /// @throws lang::WrappedTargetException
    /// @throws uno::RuntimeException
    uno::Reference< text::XTextContent > NextElement_Impl();

    /**
     * Determines if the last element in the enumeration should be ignored or
     * not.
     */
    bool IgnoreLastElement(SwUnoCursor& rCursor, bool bMovedFromTable);
};

}

rtl::Reference<SwXParagraphEnumeration> SwXParagraphEnumeration::Create(
    uno::Reference< text::XText > const& xParent,
    const std::shared_ptr<SwUnoCursor>& pCursor,
    const CursorType eType,
    SwTableBox const*const pTableBox)
{
    SwStartNode const* pStartNode(nullptr);
    SwTable const* pTable(nullptr);
    assert((eType == CursorType::TableText) == (pTableBox != nullptr));
    switch (eType)
    {
        case CursorType::TableText:
        {
            pStartNode = pTableBox->GetSttNd();
            pTable = & pStartNode->FindTableNode()->GetTable();
            break;
        }
        case CursorType::SelectionInTable:
        {
            SwTableNode const*const pTableNode(
                pCursor->GetPoint()->GetNode().FindTableNode());
            pStartNode = pTableNode;
            pTable = & pTableNode->GetTable();
            break;
        }
        default:
            break;
    }
    return new SwXParagraphEnumerationImpl(xParent, pCursor, eType, pStartNode, pTable);
}

sal_Bool SAL_CALL
SwXParagraphEnumerationImpl::hasMoreElements()
{
    SolarMutexGuard aGuard;
    return m_bFirstParagraph || m_xNextPara.is();
}

//!! compare to SwShellTableCursor::FillRects() in viscrs.cxx
static SwTableNode *
lcl_FindTopLevelTable(
        SwTableNode *const pTableNode, SwTable const*const pOwnTable)
{
    // find top-most table in current context (section) level

    SwTableNode * pLast = pTableNode;
    for (SwTableNode* pTmp = pLast;
         pTmp != nullptr  &&  &pTmp->GetTable() != pOwnTable;  /* we must not go up higher than the own table! */
         pTmp = pTmp->StartOfSectionNode()->FindTableNode() )
    {
        pLast = pTmp;
    }
    return pLast;
}

static bool
lcl_CursorIsInSection(
        SwUnoCursor const*const pUnoCursor, SwStartNode const*const pOwnStartNode)
{
    // returns true if the cursor is in the section (or in a sub section!)
    // represented by pOwnStartNode

    bool bRes = true;
    if (pUnoCursor && pOwnStartNode)
    {
        const SwEndNode * pOwnEndNode = pOwnStartNode->EndOfSectionNode();
        bRes = pOwnStartNode->GetIndex() <= pUnoCursor->Start()->GetNodeIndex() &&
               pUnoCursor->End()->GetNodeIndex() <= pOwnEndNode->GetIndex();
    }
    return bRes;
}

bool SwXParagraphEnumerationImpl::IgnoreLastElement(SwUnoCursor& rCursor, bool bMovedFromTable)
{
    // Ignore the last element of a selection enumeration if this is a stub
    // paragraph (directly after table, selection ends at paragraph start).

    if (rCursor.Start()->GetNodeIndex() != m_nEndIndex)
        return false;

    if (m_eCursorType != CursorType::Selection)
        return false;

    if (!bMovedFromTable)
        return false;

    return m_nLastParaEnd == 0;
}

uno::Reference< text::XTextContent >
SwXParagraphEnumerationImpl::NextElement_Impl()
{
    SwUnoCursor& rUnoCursor = GetCursor();

    // check for exceeding selections
    if (!m_bFirstParagraph &&
        ((CursorType::Selection == m_eCursorType) ||
         (CursorType::SelectionInTable == m_eCursorType)))
    {
        SwPosition* pStart = rUnoCursor.Start();
        auto aNewCursor(rUnoCursor.GetDoc().CreateUnoCursor(*pStart));
        // one may also go into tables here
        if (CursorType::SelectionInTable != m_eCursorType)
        {
            aNewCursor->SetRemainInSection( false );
        }

        // os 2005-01-14: This part is only necessary to detect movements out
        // of a selection; if there is no selection we don't have to care
        SwTableNode *const pTableNode = aNewCursor->GetPointNode().FindTableNode();
        bool bMovedFromTable = false;
        if (CursorType::SelectionInTable != m_eCursorType && pTableNode)
        {
            aNewCursor->GetPoint()->Assign( pTableNode->EndOfSectionIndex() );
            aNewCursor->Move(fnMoveForward, GoInNode);
            bMovedFromTable = true;
        }
        else
        {
            aNewCursor->MovePara(GoNextPara, fnParaStart);
        }
        if (m_nEndIndex < aNewCursor->Start()->GetNodeIndex())
        {
            return nullptr;
        }

        if (IgnoreLastElement(*aNewCursor, bMovedFromTable))
        {
            return nullptr;
        }
    }

    bool bInTable = false;
    if (!m_bFirstParagraph)
    {
        rUnoCursor.SetRemainInSection( false );
        // what to do if already in a table?
        SwTableNode * pTableNode = rUnoCursor.GetPointNode().FindTableNode();
        pTableNode = lcl_FindTopLevelTable( pTableNode, m_pOwnTable );
        if (pTableNode && (&pTableNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table: go to end
            rUnoCursor.GetPoint()->Assign( pTableNode->EndOfSectionIndex() );
            if (!rUnoCursor.Move(fnMoveForward, GoInNode))
            {
                return nullptr;
            }
            bInTable = true;
        }
    }

    uno::Reference< text::XTextContent >  xRef;
    // the cursor must remain in the current section or a subsection
    // before AND after the movement...
    if (lcl_CursorIsInSection( &rUnoCursor, m_pOwnStartNode ) &&
        (m_bFirstParagraph || bInTable ||
        (rUnoCursor.MovePara(GoNextPara, fnParaStart) &&
            lcl_CursorIsInSection( &rUnoCursor, m_pOwnStartNode ))))
    {
        if (m_eCursorType == CursorType::Selection || m_eCursorType == CursorType::SelectionInTable)
        {
            // This is a selection, check if the cursor would go past the end
            // of the selection.
            if (rUnoCursor.Start()->GetNodeIndex() > m_nEndIndex)
                return nullptr;
        }

        SwPosition* pStart = rUnoCursor.Start();
        const sal_Int32 nFirstContent =
            m_bFirstParagraph ? m_nFirstParaStart : -1;
        const sal_Int32 nLastContent =
            (m_nEndIndex == pStart->GetNodeIndex()) ? m_nLastParaEnd : -1;

        // position in a table, or in a simple paragraph?
        SwTableNode * pTableNode = rUnoCursor.GetPointNode().FindTableNode();
        pTableNode = lcl_FindTopLevelTable( pTableNode, m_pOwnTable );
        if (/*CursorType::TableText != eCursorType && CursorType::SelectionInTable != eCursorType && */
            pTableNode && (&pTableNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table
            SwFrameFormat* pTableFormat = pTableNode->GetTable().GetFrameFormat();
            xRef = SwXTextTable::CreateXTextTable(pTableFormat);
        }
        else
        {
            text::XText *const pText = m_xParentText.get();
            xRef = SwXParagraph::CreateXParagraph(rUnoCursor.GetDoc(),
                pStart->GetNode().GetTextNode(),
                static_cast<SwXText*>(pText), nFirstContent, nLastContent);
        }
    }

    return xRef;
}

uno::Any SAL_CALL SwXParagraphEnumerationImpl::nextElement()
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
    : public SvtListener
{
public:
    const SfxItemPropertySet& m_rPropSet;
    const enum RangePosition m_eRangePosition;
    SwDoc& m_rDoc;
    uno::Reference<text::XText> m_xParentText;
    const SwFrameFormat* m_pTableOrSectionFormat;
    const ::sw::mark::IMark* m_pMark;

    Impl(SwDoc& rDoc, const enum RangePosition eRange,
            SwFrameFormat* const pTableOrSectionFormat,
            uno::Reference<text::XText> xParent = nullptr)
        : m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
        , m_eRangePosition(eRange)
        , m_rDoc(rDoc)
        , m_xParentText(std::move(xParent))
        , m_pTableOrSectionFormat(pTableOrSectionFormat)
        , m_pMark(nullptr)
    {
        if (m_pTableOrSectionFormat)
        {
            assert(m_eRangePosition == RANGE_IS_TABLE || m_eRangePosition == RANGE_IS_SECTION);
            StartListening(pTableOrSectionFormat->GetNotifier());
        }
        else
        {
            assert(m_eRangePosition != RANGE_IS_TABLE && m_eRangePosition != RANGE_IS_SECTION);
        }
    }

    virtual ~Impl() override
    {
        // Impl owns the bookmark; delete it here: SolarMutex is locked
        Invalidate();
    }

    void Invalidate()
    {
        if (m_pMark)
        {
            m_rDoc.getIDocumentMarkAccess()->deleteMark(m_pMark);
            m_pMark = nullptr;
        }
        m_pTableOrSectionFormat = nullptr;
        EndListeningAll();
    }

    const ::sw::mark::IMark* GetBookmark() const { return m_pMark; }
    void SetMark(::sw::mark::IMark& rMark)
    {
        EndListeningAll();
        m_pTableOrSectionFormat = nullptr;
        m_pMark = &rMark;
        StartListening(rMark.GetNotifier());
    }

protected:
    virtual void Notify(const SfxHint&) override;
};

void SwXTextRange::Impl::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        EndListeningAll();
        m_pTableOrSectionFormat = nullptr;
        m_pMark = nullptr;
    }
}

SwXTextRange::SwXTextRange(SwPaM const & rPam,
        const uno::Reference< text::XText > & xParent,
        const enum RangePosition eRange)
    : m_pImpl( new SwXTextRange::Impl(rPam.GetDoc(), eRange, nullptr, xParent) )
{
    SetPositions(rPam);
}

SwXTextRange::SwXTextRange(SwTableFormat& rTableFormat)
    : m_pImpl(
        new SwXTextRange::Impl(*rTableFormat.GetDoc(), RANGE_IS_TABLE, &rTableFormat) )
{
    SwTable *const pTable = SwTable::FindTable( &rTableFormat );
    SwTableNode *const pTableNode = pTable->GetTableNode();
    SwPaM aPam( *pTableNode );

    SetPositions( aPam );
}

SwXTextRange::SwXTextRange(SwSectionFormat& rSectionFormat)
    : m_pImpl(
        new SwXTextRange::Impl(*rSectionFormat.GetDoc(), RANGE_IS_SECTION, &rSectionFormat) )
{
    // no SetPositions here for now
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
    auto pMark = pMA->makeMark(rPam, OUString(), IDocumentMarkAccess::MarkType::UNO_BOOKMARK, sw::mark::InsertMode::New);
    if (pMark)
        m_pImpl->SetMark(*pMark);
}

static void DeleteTable(SwDoc & rDoc, SwTable& rTable)
{
    SwSelBoxes aSelBoxes;
    for (auto& rBox : rTable.GetTabSortBoxes())
    {
        aSelBoxes.insert(rBox);
    }
    // note: if the table is the content in the section, this will create
    // a new text node - that's desirable here
    rDoc.DeleteRowCol(aSelBoxes, SwDoc::RowColMode::DeleteProtected);
}

void SwXTextRange::DeleteAndInsert(
        std::u16string_view aText, ::sw::DeleteAndInsertMode const eMode)
{
    if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // setString on table not allowed
        throw uno::RuntimeException("not possible for table");
    }

    const SwPosition aPos(GetDoc().GetNodes().GetEndOfContent());
    SwCursor aCursor(aPos, nullptr);

    UnoActionContext aAction(& m_pImpl->m_rDoc);

    if (RANGE_IS_SECTION == m_pImpl->m_eRangePosition)
    {
        SwSectionNode const* pSectionNode = m_pImpl->m_pTableOrSectionFormat ?
            static_cast<SwSectionFormat const*>(m_pImpl->m_pTableOrSectionFormat)->GetSectionNode() :
            nullptr;
        if (!pSectionNode)
        {
            throw uno::RuntimeException("disposed?");
        }
        m_pImpl->m_rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);
        SwNodeIndex const start(*pSectionNode);
        SwNodeIndex const end(*start.GetNode().EndOfSectionNode());

        // delete tables at start
        for (SwNodeIndex i = start; i < end; )
        {
            SwNode & rNode(i.GetNode());
            if (rNode.IsSectionNode())
            {
                ++i;
                continue;
            }
            else if (SwTableNode *const pTableNode = rNode.GetTableNode())
            {
                DeleteTable(m_pImpl->m_rDoc, pTableNode->GetTable());
                // where does that leave index? presumably behind?
            }
            else
            {
                assert(rNode.IsTextNode());
                break;
            }
        }
        // delete tables at end
        for (SwNodeIndex i = end; start <= i; )
        {
            --i;
            SwNode & rNode(i.GetNode());
            if (rNode.IsEndNode())
            {
                if (SwTableNode *const pTableNode = rNode.StartOfSectionNode()->GetTableNode())
                {
                    DeleteTable(m_pImpl->m_rDoc, pTableNode->GetTable());
                }
                else
                {
                    assert(rNode.StartOfSectionNode()->IsSectionNode());
                    continue;
                }
            }
            else
            {
                assert(rNode.IsTextNode());
                break;
            }
        }
        // now there should be a text node at the start and end of it!
        aCursor.GetPoint()->Assign(start);
        aCursor.Move( fnMoveForward, GoInContent );
        assert(aCursor.GetPoint()->GetNode() <= end.GetNode());
        aCursor.SetMark();
        aCursor.GetPoint()->Assign(end);
        aCursor.Move( fnMoveBackward, GoInContent );
        assert(start <= aCursor.GetPoint()->GetNode());
    }
    else
    {
        if (!GetPositions(aCursor))
            return;
        m_pImpl->m_rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);
    }

    if (aCursor.HasMark())
    {
        m_pImpl->m_rDoc.getIDocumentContentOperations().DeleteAndJoin(aCursor,
            (!aText.empty() || eMode & ::sw::DeleteAndInsertMode::ForceReplace) ? SwDeleteFlags::ArtificialSelection : SwDeleteFlags::Default);
    }

    if (!aText.empty())
    {
        SwUnoCursorHelper::DocInsertStringSplitCR(
            m_pImpl->m_rDoc, aCursor, aText, bool(eMode & ::sw::DeleteAndInsertMode::ForceExpandHints));

        SwUnoCursorHelper::SelectPam(aCursor, true);
        aCursor.Left(aText.size());
    }
    SetPositions(aCursor);
    m_pImpl->m_rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, nullptr);
}

OUString SAL_CALL
SwXTextRange::getImplementationName()
{
    return "SwXTextRange";
}

sal_Bool SAL_CALL SwXTextRange::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getSupportedServiceNames()
{
    return {
        "com.sun.star.text.TextRange",
        "com.sun.star.style.CharacterProperties",
        "com.sun.star.style.CharacterPropertiesAsian",
        "com.sun.star.style.CharacterPropertiesComplex",
        "com.sun.star.style.ParagraphProperties",
        "com.sun.star.style.ParagraphPropertiesAsian",
        "com.sun.star.style.ParagraphPropertiesComplex"
    };
}

uno::Reference< text::XText > SAL_CALL
SwXTextRange::getText()
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_xParentText.is() && m_pImpl->m_pTableOrSectionFormat)
    {
        std::optional<SwPosition> oPosition;
        if (m_pImpl->m_eRangePosition == RANGE_IS_TABLE)
        {
            SwTable const*const pTable = SwTable::FindTable( m_pImpl->m_pTableOrSectionFormat );
            SwTableNode const*const pTableNode = pTable->GetTableNode();
            oPosition.emplace(*pTableNode);
        }
        else
        {
            assert(m_pImpl->m_eRangePosition == RANGE_IS_SECTION);
            auto const pSectFormat(static_cast<SwSectionFormat const*>(m_pImpl->m_pTableOrSectionFormat));
            oPosition.emplace(pSectFormat->GetContent().GetContentIdx()->GetNode());
        }
        m_pImpl->m_xParentText =
            ::sw::CreateParentXText(m_pImpl->m_rDoc, *oPosition);
    }
    OSL_ENSURE(m_pImpl->m_xParentText.is(), "SwXTextRange::getText: no text");
    return m_pImpl->m_xParentText;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getStart()
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
    else if (RANGE_IS_SECTION == m_pImpl->m_eRangePosition
            && m_pImpl->m_pTableOrSectionFormat)
    {
        auto const pSectFormat(static_cast<SwSectionFormat const*>(m_pImpl->m_pTableOrSectionFormat));
        SwPaM aPaM(*pSectFormat->GetContent().GetContentIdx());
        aPaM.Move( fnMoveForward, GoInContent );
        assert(aPaM.GetPoint()->GetNode() < *pSectFormat->GetContent().GetContentIdx()->GetNode().EndOfSectionNode());
        xRet = new SwXTextRange(aPaM, m_pImpl->m_xParentText);
    }
    else
    {
        throw uno::RuntimeException("disposed?");
    }
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getEnd()
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
    else if (RANGE_IS_SECTION == m_pImpl->m_eRangePosition
            && m_pImpl->m_pTableOrSectionFormat)
    {
        auto const pSectFormat(static_cast<SwSectionFormat const*>(m_pImpl->m_pTableOrSectionFormat));
        SwPaM aPaM(*pSectFormat->GetContent().GetContentIdx()->GetNode().EndOfSectionNode());
        aPaM.Move( fnMoveBackward, GoInContent );
        assert(*pSectFormat->GetContent().GetContentIdx() < aPaM.GetPoint()->GetNode());
        xRet = new SwXTextRange(aPaM, m_pImpl->m_xParentText);
    }
    else
    {
        throw uno::RuntimeException("disposed?");
    }
    return xRet;
}

OUString SAL_CALL SwXTextRange::getString()
{
    SolarMutexGuard aGuard;

    OUString sRet;
    // for tables there is no bookmark, thus also no text
    // one could export the table as ASCII here maybe?
    SwPaM aPaM(GetDoc().GetNodes());
    if (GetPositions(aPaM, sw::TextRangeMode::AllowNonTextNode) && aPaM.HasMark())
    {
        SwUnoCursorHelper::GetTextFromPam(aPaM, sRet);
    }
    return sRet;
}

void SAL_CALL SwXTextRange::setString(const OUString& rString)
{
    SolarMutexGuard aGuard;

    DeleteAndInsert(rString, ::sw::DeleteAndInsertMode::Default);
}

bool SwXTextRange::GetPositions(SwPaM& rToFill, ::sw::TextRangeMode const eMode) const
{
    if (RANGE_IS_SECTION == m_pImpl->m_eRangePosition)
    {
        if (auto const pSectFormat = static_cast<SwSectionFormat const*>(m_pImpl->m_pTableOrSectionFormat))
        {
            if (eMode == ::sw::TextRangeMode::AllowNonTextNode)
            {
                SwNodeIndex const*const pSectionNode(pSectFormat->GetContent().GetContentIdx());
                assert(pSectionNode);
                assert(pSectionNode->GetNodes().IsDocNodes());
                rToFill.GetPoint()->Assign( pSectionNode->GetNode(), SwNodeOffset(1) );
                rToFill.SetMark();
                rToFill.GetMark()->Assign( *pSectionNode->GetNode().EndOfSectionNode(), SwNodeOffset(-1) );
                if (const SwContentNode* pCNd = rToFill.GetMark()->GetContentNode())
                    rToFill.GetMark()->AssignEndIndex(*pCNd);
                return true;
            }
            else
            {
                SwPaM aPaM(*pSectFormat->GetContent().GetContentIdx());
                aPaM.Move(fnMoveForward, GoInContent);
                assert(aPaM.GetPoint()->GetNode() < *pSectFormat->GetContent().GetContentIdx()->GetNode().EndOfSectionNode());
                aPaM.SetMark();
                *aPaM.GetPoint() = SwPosition(*pSectFormat->GetContent().GetContentIdx()->GetNode().EndOfSectionNode());
                aPaM.Move(fnMoveBackward, GoInContent);
                assert(*pSectFormat->GetContent().GetContentIdx() < aPaM.GetPoint()->GetNode());
                // tdf#149555 if there is no table involved, only nested
                // sections, then PaM is valid
                if (aPaM.GetPoint()->GetNode().FindTableNode()
                    == aPaM.GetMark()->GetNode().FindTableNode())
                {
                    rToFill = aPaM;
                    return true;
                }
            }
        }
    }
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
        const uno::Reference<text::XTextRange> & xTextRange,
        ::sw::TextRangeMode const eMode)
{
    bool bRet = false;

    SwXHeadFootText* pHeadText
        = eMode == TextRangeMode::AllowTableNode ? dynamic_cast<SwXHeadFootText*>(xTextRange.get()) : nullptr;

    // if it's a text then create a temporary cursor there and re-use
    // the pCursor variable
    // #i108489#: Reference in outside scope to keep cursor alive
    rtl::Reference< SwXTextCursor > xTextCursor;
    OTextCursorHelper* pCursor;
    if (pHeadText)
    {
        // if it is a header / footer text, and eMode == TextRangeMode::AllowTableNode
        // then set the cursor to the beginning of the text
        // if it is started with a table then set into the table
        xTextCursor = pHeadText->CreateTextCursor(true);
        xTextCursor->gotoEnd(true);
        pCursor = xTextCursor.get();
        pCursor->GetPaM()->Normalize();
    }
    else if (SwXText* pText = dynamic_cast<SwXText*>(xTextRange.get()))
    {
        xTextCursor = pText->createXTextCursor();
        xTextCursor->gotoEnd(true);
        pCursor = xTextCursor.get();
    }
    else
    {
        pCursor = dynamic_cast<OTextCursorHelper*>(xTextRange.get());
    }

    SwXTextRange* pRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    if(pRange && &pRange->GetDoc() == &rToFill.GetDoc())
    {
        bRet = pRange->GetPositions(rToFill, eMode);
    }
    else if (SwXParagraph* pPara = dynamic_cast<SwXParagraph*>(xTextRange.get()))
    {
        bRet = pPara->SelectPaM(rToFill);
    }
    else
    {
        SwDoc* pDoc = nullptr;
        const SwPaM* pUnoCursor = nullptr;
        if (pCursor)
        {
            pDoc = pCursor->GetDoc();
            pUnoCursor = pCursor->GetPaM();
        }
        else if (SwXTextPortion* pPortion = dynamic_cast<SwXTextPortion*>(xTextRange.get()))
        {
            pDoc = &pPortion->GetCursor().GetDoc();
            pUnoCursor = &pPortion->GetCursor();
        }
        if (pUnoCursor && pDoc == &rToFill.GetDoc())
        {
            OSL_ENSURE(!pUnoCursor->IsMultiSelection(),
                    "what to do about rings?");
            bRet = true;
            *rToFill.GetPoint() = *pUnoCursor->GetPoint();
            if (pUnoCursor->HasMark())
            {
                rToFill.SetMark();
                *rToFill.GetMark() = *pUnoCursor->GetMark();
            }
            else
                rToFill.DeleteMark();
        }
    }
    return bRet;
}

static bool
lcl_IsStartNodeInFormat(const bool bHeader, SwStartNode const *const pSttNode,
    SwFrameFormat const*const pFrameFormat, SwFrameFormat*& rpFormat)
{
    bool bRet = false;
    const SfxItemSet& rSet = pFrameFormat->GetAttrSet();
    const SfxPoolItem* pItem;
    if (SfxItemState::SET == rSet.GetItemState(
            bHeader ? sal_uInt16(RES_HEADER) : sal_uInt16(RES_FOOTER),
            true, &pItem))
    {
        SfxPoolItem *const pItemNonConst(const_cast<SfxPoolItem *>(pItem));
        SwFrameFormat *const pHeadFootFormat = bHeader ?
            static_cast<SwFormatHeader*>(pItemNonConst)->GetHeaderFormat() :
            static_cast<SwFormatFooter*>(pItemNonConst)->GetFooterFormat();
        if (pHeadFootFormat)
        {
            const SwFormatContent& rFlyContent = pHeadFootFormat->GetContent();
            // tdf#146248 avoid Undo crash at shared first page
            if ( !rFlyContent.GetContentIdx() )
                return false;
            const SwNode& rNode = rFlyContent.GetContentIdx()->GetNode();
            SwStartNode const*const pCurSttNode = rNode.FindSttNodeByType(
                bHeader ? SwHeaderStartNode : SwFooterStartNode);
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

rtl::Reference< SwXTextRange >
SwXTextRange::CreateXTextRange(
    SwDoc & rDoc, const SwPosition& rPos, const SwPosition *const pMark)
{
    const uno::Reference<text::XText> xParentText(
            ::sw::CreateParentXText(rDoc, rPos));
    const auto pNewCursor(rDoc.CreateUnoCursor(rPos));
    if(pMark)
    {
        pNewCursor->SetMark();
        *pNewCursor->GetMark() = *pMark;
    }
    const bool isCell( dynamic_cast<SwXCell*>(xParentText.get()) );
    return new SwXTextRange(*pNewCursor, xParentText,
            isCell ? RANGE_IN_CELL : RANGE_IN_TEXT);
}

namespace sw {

css::uno::Reference< SwXText >
CreateParentXText(SwDoc & rDoc, const SwPosition& rPos)
{
    css::uno::Reference< SwXText > xParentText;
    SwStartNode* pSttNode = rPos.GetNode().StartOfSectionNode();
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
                pTableNode->GetTable().GetFrameFormat();
            SwTableBox *const  pBox = pSttNode->GetTableBox();

            xParentText = pBox
                ? SwXCell::CreateXCell( pTableFormat, pBox )
                : new SwXCell( pTableFormat, *pSttNode );
        }
        break;
        case SwFlyStartNode:
        {
            SwFrameFormat *const pFormat = pSttNode->GetFlyFormat();
            if (nullptr != pFormat)
            {
                xParentText = SwXTextFrame::CreateXTextFrame(rDoc, pFormat);
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
                const SwFrameFormat* pFrameFormatFirstMaster = &rDesc.GetFirstMaster();
                const SwFrameFormat* pFrameFormatFirstLeft = &rDesc.GetFirstLeft();

                SwFrameFormat* pHeadFootFormat = nullptr;
                if (!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrameFormatMaster,
                          pHeadFootFormat))
                {
                    if (!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrameFormatLeft,
                             pHeadFootFormat))
                    {
                        if (!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrameFormatFirstMaster,
                             pHeadFootFormat))
                        {
                             lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrameFormatFirstLeft, pHeadFootFormat);
                        }
                    }
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
            for (size_t n = 0; n < nFootnoteCnt; ++n )
            {
                const SwTextFootnote* pTextFootnote = rDoc.GetFootnoteIdxs()[ n ];
                const SwFormatFootnote& rFormatFootnote = pTextFootnote->GetFootnote();
                assert(pTextFootnote == rFormatFootnote.GetTextFootnote());
                assert(&pTextFootnote->GetStartNode()->GetNode() == pTextFootnote->GetStartNode()->GetNode().
                                    FindSttNodeByType(SwFootnoteStartNode));

                if (pSttNode == &pTextFootnote->GetStartNode()->GetNode())
                {
                    xParentText = SwXFootnote::CreateXFootnote(rDoc,
                            &const_cast<SwFormatFootnote&>(rFormatFootnote));
                    break;
                }
            }
        }
        break;
        default:
        {
            if (SwDocShell *const pDocSh = rDoc.GetDocShell())
            {
                // then it is the body text
                const uno::Reference<frame::XModel> xModel = pDocSh->GetBaseModel();
                const uno::Reference< text::XTextDocument > xDoc(
                    xModel, uno::UNO_QUERY);
                xParentText = dynamic_cast<SwXText*>(xDoc->getText().get());
            }
        }
    }
    OSL_ENSURE(xParentText.is(), "no parent text?");
    return xParentText;
}

} // namespace sw

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextRange::createContentEnumeration(const OUString& rServiceName)
{
    SolarMutexGuard g;

    if ( rServiceName != "com.sun.star.text.TextContent" )
    {
        throw uno::RuntimeException("unsupported service");
    }

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
    }
    const SwPosition aPos(GetDoc().GetNodes().GetEndOfContent());
    const auto pNewCursor(m_pImpl->m_rDoc.CreateUnoCursor(aPos));
    if (!GetPositions(*pNewCursor))
    {
        throw uno::RuntimeException("range has no positions");
    }

    return SwXParaFrameEnumeration::Create(*pNewCursor, PARAFRAME_PORTION_TEXTRANGE);
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextRange::createEnumeration()
{
    SolarMutexGuard g;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
    }
    const SwPosition aPos(GetDoc().GetNodes().GetEndOfContent());
    auto pNewCursor(m_pImpl->m_rDoc.CreateUnoCursor(aPos));
    if (!GetPositions(*pNewCursor))
    {
        throw uno::RuntimeException("range has no positions");
    }
    if (!m_pImpl->m_xParentText.is())
    {
        getText();
    }

    const CursorType eSetType = (RANGE_IN_CELL == m_pImpl->m_eRangePosition)
            ? CursorType::SelectionInTable : CursorType::Selection;
    return SwXParagraphEnumeration::Create(m_pImpl->m_xParentText, pNewCursor, eSetType);
}

uno::Type SAL_CALL SwXTextRange::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXTextRange::hasElements()
{
    return true;
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getAvailableServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.text.TextContent" };
    return aRet;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextRange::getPropertySetInfo()
{
    SolarMutexGuard aGuard;

    static uno::Reference< beans::XPropertySetInfo > xRef =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xRef;
}

void SAL_CALL
SwXTextRange::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    SwUnoCursorHelper::SetPropertyValue(aPaM, m_pImpl->m_rPropSet,
            rPropertyName, rValue);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
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
{
    OSL_FAIL("SwXTextRange::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXTextRange::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXTextRange::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXTextRange::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState SAL_CALL
SwXTextRange::getPropertyState(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyState(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextRange::getPropertyStates(const uno::Sequence< OUString >& rPropertyName)
{
    SolarMutexGuard g;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyStates(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL SwXTextRange::setPropertyToDefault(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
    }
    SwPaM aPaM(GetDoc().GetNodes());
    GetPositions(aPaM);
    SwUnoCursorHelper::SetPropertyToDefault(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyDefault(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
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
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException("range has no mark (table?)");
    }
    SwPaM aPaM(GetDoc().GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwUnoCursorHelper::makeRedline( aPaM, rRedlineType, rRedlineProperties );
}

namespace {

struct SwXTextRangesImpl final : public SwXTextRanges
{

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
        { return "SwXTextRanges"; };
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName) override
        { return cppu::supportsService(this, rServiceName); };
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override
        { return { "com.sun.star.text.TextRanges" }; };

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override
        { return cppu::UnoType<text::XTextRange>::get(); };
    virtual sal_Bool SAL_CALL hasElements() override
        { return getCount() > 0; };
    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    explicit SwXTextRangesImpl(SwPaM *const pPaM)
    {
        if (pPaM)
        {
            m_pUnoCursor.reset(pPaM->GetDoc().CreateUnoCursor(*pPaM->GetPoint()));
            ::sw::DeepCopyPaM(*pPaM, *GetCursor());
        }
        MakeRanges();
    }
    virtual void SAL_CALL release() noexcept override
    {
        SolarMutexGuard g;
        OWeakObject::release();
    }
    virtual SwUnoCursor* GetCursor() override
        { return &(*m_pUnoCursor); };
    void MakeRanges();
    std::vector< rtl::Reference<SwXTextRange> > m_Ranges;
    sw::UnoCursorPointer m_pUnoCursor;
};

}

void SwXTextRangesImpl::MakeRanges()
{
    if (!GetCursor())
        return;

    for(SwPaM& rTmpCursor : GetCursor()->GetRingContainer())
    {
        const rtl::Reference<SwXTextRange> xRange(
                SwXTextRange::CreateXTextRange(
                    rTmpCursor.GetDoc(),
                    *rTmpCursor.GetPoint(), rTmpCursor.GetMark()));
        if (xRange.is())
        {
            m_Ranges.push_back(xRange);
        }
    }
}

rtl::Reference<SwXTextRanges> SwXTextRanges::Create(SwPaM *const pPaM)
    { return new SwXTextRangesImpl(pPaM); }

/*
 *  Text positions
 * Up to the first access to a text position, only a SwCursor is stored.
 * Afterwards, an array with uno::Reference<XTextPosition> will be created.
 */

sal_Int32 SAL_CALL SwXTextRangesImpl::getCount()
{
    SolarMutexGuard aGuard;
    return static_cast<sal_Int32>(m_Ranges.size());
}

uno::Any SAL_CALL SwXTextRangesImpl::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if ((nIndex < 0) || (o3tl::make_unsigned(nIndex) >= m_Ranges.size()))
        throw lang::IndexOutOfBoundsException();
    uno::Any ret(uno::Reference<text::XTextRange>(m_Ranges.at(nIndex)));
    return ret;
}

void SwUnoCursorHelper::SetString(SwCursor & rCursor, std::u16string_view aString)
{
    // Start/EndAction
    SwDoc& rDoc = rCursor.GetDoc();
    UnoActionContext aAction(&rDoc);
    rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);
    if (rCursor.HasMark())
    {
        rDoc.getIDocumentContentOperations().DeleteAndJoin(rCursor);
    }
    if (!aString.empty())
    {
        const bool bSuccess( SwUnoCursorHelper::DocInsertStringSplitCR(
                    rDoc, rCursor, aString, false ) );
        OSL_ENSURE( bSuccess, "DocInsertStringSplitCR" );
        SwUnoCursorHelper::SelectPam(rCursor, true);
        rCursor.Left(aString.size());
    }
    rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, nullptr);
}

namespace {

struct SwXParaFrameEnumerationImpl final : public SwXParaFrameEnumeration
{
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
        { return "SwXParaFrameEnumeration"; };
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override
        { return cppu::supportsService(this, rServiceName); };
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override
        { return {"com.sun.star.util.ContentEnumeration"}; };

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

    SwXParaFrameEnumerationImpl(const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode, SwFrameFormat* const pFormat);
    virtual void SAL_CALL release() noexcept override
    {
        SolarMutexGuard g;
        OWeakObject::release();
    }
    SwUnoCursor& GetCursor()
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
            // removing orphaned Clients
            std::erase_if(m_vFrames,
                    [] (std::unique_ptr<sw::FrameClient>& rEntry) -> bool { return !rEntry->GetRegisteredIn(); });
        }
    }
    void FillFrame();
    bool CreateNextObject();
    uno::Reference< text::XTextContent > m_xNextObject;
    std::deque< std::unique_ptr<sw::FrameClient> > m_vFrames;
    ::sw::UnoCursorPointer m_pUnoCursor;
};

}

rtl::Reference<SwXParaFrameEnumeration> SwXParaFrameEnumeration::Create(const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode, SwFrameFormat* const pFormat)
    { return new SwXParaFrameEnumerationImpl(rPaM, eParaFrameMode, pFormat); }

SwXParaFrameEnumerationImpl::SwXParaFrameEnumerationImpl(
        const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode,
        SwFrameFormat* const pFormat)
    : m_pUnoCursor(rPaM.GetDoc().CreateUnoCursor(*rPaM.GetPoint()))
{
    if (rPaM.HasMark())
    {
        GetCursor().SetMark();
        *GetCursor().GetMark() = *rPaM.GetMark();
    }
    if (PARAFRAME_PORTION_PARAGRAPH == eParaFrameMode)
    {
        FrameClientSortList_t vFrames;
        ::CollectFrameAtNode(rPaM.GetPoint()->GetNode(), vFrames, false);
        std::transform(vFrames.begin(), vFrames.end(),
            std::back_inserter(m_vFrames),
            [] (FrameClientSortListEntry& rEntry) { return std::move(rEntry.pFrameClient); });
    }
    else if (pFormat)
    {
        m_vFrames.push_back(std::make_unique<sw::FrameClient>(pFormat));
    }
    else if ((PARAFRAME_PORTION_CHAR == eParaFrameMode) ||
             (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode))
    {
        if (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode)
        {
            //get all frames that are bound at paragraph or at character
            for(const SwPosFlyFrame& rFlyFrame : rPaM.GetDoc().GetAllFlyFormats(&GetCursor(), false, true))
            {
                const auto pFrameFormat = const_cast<SwFrameFormat*>(&rFlyFrame.GetFormat());
                m_vFrames.push_back(std::make_unique<sw::FrameClient>(pFrameFormat));
            }
        }
        FillFrame();
    }
}

// Search for a FLYCNT text attribute at the cursor point and fill the frame
// into the array
void SwXParaFrameEnumerationImpl::FillFrame()
{
    if(!m_pUnoCursor->GetPointNode().IsTextNode())
        return;
    // search for objects at the cursor - anchored at/as char
    const auto pTextAttr = m_pUnoCursor->GetPointNode().GetTextNode()->GetTextAttrForCharAt(
            m_pUnoCursor->GetPoint()->GetContentIndex(), RES_TXTATR_FLYCNT);
    if(!pTextAttr)
        return;
    const SwFormatFlyCnt& rFlyCnt = pTextAttr->GetFlyCnt();
    SwFrameFormat* const pFrameFormat = rFlyCnt.GetFrameFormat();
    m_vFrames.push_back(std::make_unique<sw::FrameClient>(pFrameFormat));
}

bool SwXParaFrameEnumerationImpl::CreateNextObject()
{
    if (m_vFrames.empty())
        return false;

    m_xNextObject.set(FrameClientToXTextContent(m_vFrames.front().get()));
    m_vFrames.pop_front();
    return m_xNextObject.is();
}

uno::Reference<text::XTextContent> FrameClientToXTextContent(sw::FrameClient* pClient)
{
    assert(pClient);

    uno::Reference<text::XTextContent> xRet;
    SwFrameFormat* const pFormat = static_cast<SwFrameFormat*>(pClient->GetRegisteredIn());
    // the format should be valid here, otherwise the client
    // would have been removed by PurgeFrameClients
    // check for a shape first
    if(pFormat->Which() == RES_DRAWFRMFMT)
    {
        SdrObject* pObject(nullptr);
        pFormat->CallSwClientNotify(sw::FindSdrObjectHint(pObject));
        if(pObject)
            xRet.set(pObject->getUnoShape(), uno::UNO_QUERY);
    }
    else
    {
        const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
        OSL_ENSURE(pIdx, "where is the index?");
        SwNode const* const pNd = pIdx->GetNodes()[pIdx->GetIndex() + 1];

        if (!pNd->IsNoTextNode())
        {
            xRet = static_cast<SwXFrame*>(SwXTextFrame::CreateXTextFrame(
                        *pFormat->GetDoc(), pFormat).get());
        }
        else if (pNd->IsGrfNode())
        {
            xRet.set(SwXTextGraphicObject::CreateXTextGraphicObject(
                        *pFormat->GetDoc(), pFormat));
        }
        else
        {
            assert(pNd->IsOLENode());
            xRet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                        *pFormat->GetDoc(), pFormat));
        }
    }
    return xRet;
}

sal_Bool SAL_CALL
SwXParaFrameEnumerationImpl::hasMoreElements()
{
    SolarMutexGuard aGuard;
    PurgeFrameClients();
    return m_xNextObject.is() || CreateNextObject();
}

uno::Any SAL_CALL SwXParaFrameEnumerationImpl::nextElement()
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
