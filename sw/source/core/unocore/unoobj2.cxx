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
#include <tools/cachestr.hxx>
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
#include <unofield.hxx>
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
#include <unoidx.hxx>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <fmtflcnt.hxx>
#include <editeng/brshitem.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <crsskip.hxx>
#include <vector>
#include <sortedobjs.hxx>
#include <sortopt.hxx>
#include <algorithm>
#include <iterator>
#include <boost/bind.hpp>
#include <switerator.hxx>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

namespace sw {

sal_Bool SupportsServiceImpl(
        size_t const nServices, char const*const pServices[],
        ::rtl::OUString const & rServiceName)
{
    for (size_t i = 0; i < nServices; ++i)
    {
        if (rServiceName.equalsAscii(pServices[i]))
        {
            return sal_True;
        }
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString >
GetSupportedServiceNamesImpl(
        size_t const nServices, char const*const pServices[])
{
    uno::Sequence< ::rtl::OUString > ret(nServices);
    for (size_t i = 0; i < nServices; ++i)
    {
        ret[i] = rtl::OUString::createFromAscii(pServices[i]);
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
        SwPaM *pPam = static_cast<SwPaM *>(rSource.GetNext());
        do
        {
            // create new PaM
            SwPaM *const pNew = new SwPaM(*pPam);
            // insert into ring
            pNew->MoveTo(&rTarget);
            pPam = static_cast<SwPaM *>(pPam->GetNext());
        }
        while (pPam != &rSource);
    }
}

} // namespace sw

struct FrameDependSortListLess
{
    bool operator() (FrameDependSortListEntry const& r1,
                     FrameDependSortListEntry const& r2) const
    {
        return  (r1.nIndex <  r2.nIndex)
            || ((r1.nIndex == r2.nIndex) && (r1.nOrder < r2.nOrder));
    }
};

// OD 2004-05-07 #i28701# - adjust 4th parameter
void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                         FrameDependSortList_t & rFrames,
                         const bool _bAtCharAnchoredObjs )
{
    // _bAtCharAnchoredObjs:
    // <sal_True>: at-character anchored objects are collected
    // <sal_False>: at-paragraph anchored objects are collected

    // alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz
    // gebunden sind
    SwDoc* pDoc = rIdx.GetNode().GetDoc();

    sal_uInt16 nChkType = static_cast< sal_uInt16 >((_bAtCharAnchoredObjs)
            ? FLY_AT_CHAR : FLY_AT_PARA);
    const SwCntntFrm* pCFrm;
    const SwCntntNode* pCNd;
    if( pDoc->GetCurrentViewShell() &&  //swmod 071108//swmod 071225
        0 != (pCNd = rIdx.GetNode().GetCntntNode()) &&
        0 != (pCFrm = pCNd->getLayoutFrm( pDoc->GetCurrentLayout())) )
    {
        const SwSortedObjs *pObjs = pCFrm->GetDrawObjs();
        if( pObjs )
            for( sal_uInt16 i = 0; i < pObjs->Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                if ( rFmt.GetAnchor().GetAnchorId() == nChkType )
                {
                    // create SwDepend and insert into array
                    SwDepend* pNewDepend = new SwDepend( &rClnt, &rFmt );
                    xub_StrLen idx =
                        rFmt.GetAnchor().GetCntntAnchor()->nContent.GetIndex();
                    sal_uInt32 nOrder = rFmt.GetAnchor().GetOrder();

                    // OD 2004-05-07 #i28701# - sorting no longer needed,
                    // because list <SwSortedObjs> is already sorted.
                    FrameDependSortListEntry entry(idx, nOrder, pNewDepend);
                    rFrames.push_back(entry);
                }
            }
    }
    else
    {
        const SwFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
        sal_uInt16 nSize = rFmts.size();
        for ( sal_uInt16 i = 0; i < nSize; i++)
        {
            const SwFrmFmt* pFmt = rFmts[ i ];
            const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
            const SwPosition* pAnchorPos;
            if( rAnchor.GetAnchorId() == nChkType &&
                0 != (pAnchorPos = rAnchor.GetCntntAnchor()) &&
                    pAnchorPos->nNode == rIdx )
            {
                //jetzt einen SwDepend anlegen und in das Array einfuegen
                SwDepend* pNewDepend = new SwDepend( &rClnt, (SwFrmFmt*)pFmt);

                // OD 2004-05-07 #i28701# - determine insert position for
                // sorted <rFrameArr>
                xub_StrLen nIndex = pAnchorPos->nContent.GetIndex();
                sal_uInt32 nOrder = rAnchor.GetOrder();

                FrameDependSortListEntry entry(nIndex, nOrder, pNewDepend);
                rFrames.push_back(entry);
            }
        }
        ::std::sort(rFrames.begin(), rFrames.end(), FrameDependSortListLess());
    }
}

/****************************************************************************
    ActionContext
****************************************************************************/
UnoActionContext::UnoActionContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    SwRootFrm *const pRootFrm = m_pDoc->GetCurrentLayout();
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
        SwRootFrm *const pRootFrm = m_pDoc->GetCurrentLayout();
        if (pRootFrm)
        {
            pRootFrm->EndAllAction();
        }
    }
}

/****************************************************************************
    ActionRemoveContext
****************************************************************************/
UnoActionRemoveContext::UnoActionRemoveContext(SwDoc *const pDoc)
    : m_pDoc(pDoc)
{
    SwRootFrm *const pRootFrm = m_pDoc->GetCurrentLayout();
    if (pRootFrm)
    {
        pRootFrm->UnoRemoveAllActions();
    }
}

UnoActionRemoveContext::~UnoActionRemoveContext()
{
    SwRootFrm *const pRootFrm = m_pDoc->GetCurrentLayout();
    if (pRootFrm)
    {
        pRootFrm->UnoRestoreAllActions();
    }
}

void ClientModify(SwClient* pClient, const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)pClient->GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)pClient->GetRegisteredIn())->Remove(pClient);
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == pClient->GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            ((SwModify*)pClient->GetRegisteredIn())->Remove(pClient);
        break;
    }
}

void SwUnoCursorHelper::SetCrsrAttr(SwPaM & rPam,
        const SfxItemSet& rSet,
        const SetAttrMode nAttrMode, const bool bTableMode)
{
    const SetAttrMode nFlags = nAttrMode | nsSetAttrMode::SETATTR_APICALL;
    SwDoc* pDoc = rPam.GetDoc();
    //StartEndAction
    UnoActionContext aAction(pDoc);
    if (rPam.GetNext() != &rPam)    // Ring of Cursors
    {
        pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSATTR, NULL);

        SwPaM *pCurrent = &rPam;
        do
        {
            if (pCurrent->HasMark() &&
                ( (bTableMode) ||
                  (*pCurrent->GetPoint() != *pCurrent->GetMark()) ))
            {
                pDoc->InsertItemSet(*pCurrent, rSet, nFlags);
            }
            pCurrent= static_cast<SwPaM *>(pCurrent->GetNext());
        } while (pCurrent != &rPam);

        pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSATTR, NULL);
    }
    else
    {
//          if( !HasSelection() )
//              UpdateAttr();
        pDoc->InsertItemSet( rPam, rSet, nFlags );
    }
    //#outline level,add by zhaojianwei
    if( rSet.GetItemState( RES_PARATR_OUTLINELEVEL, false ) >= SFX_ITEM_AVAILABLE )
    {
        SwTxtNode * pTmpNode = rPam.GetNode()->GetTxtNode();
        if ( pTmpNode )
        {
            rPam.GetDoc()->GetNodes().UpdateOutlineNode( *pTmpNode );
        }
    }
    //<-end,zhaojianwei
}

// #i63870#
// split third parameter <bCurrentAttrOnly> into new parameters <bOnlyTxtAttr>
// and <bGetFromChrFmt> to get better control about resulting <SfxItemSet>
void SwUnoCursorHelper::GetCrsrAttr(SwPaM & rPam,
        SfxItemSet & rSet, const bool bOnlyTxtAttr, const bool bGetFromChrFmt)
{
    static const sal_uInt16 nMaxLookup = 1000;
    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;
    SwPaM *pCurrent = & rPam;
    do
    {
        SwPosition const & rStart( *pCurrent->Start() );
        SwPosition const & rEnd( *pCurrent->End() );
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
                    const xub_StrLen nStart = (n == nSttNd)
                        ? rStart.nContent.GetIndex() : 0;
                    const xub_StrLen nEnd   = (n == nEndNd)
                        ? rEnd.nContent.GetIndex()
                        : static_cast<SwTxtNode*>(pNd)->GetTxt().Len();
                    static_cast<SwTxtNode*>(pNd)->GetAttr(
                        *pSet, nStart, nEnd, bOnlyTxtAttr, bGetFromChrFmt);
                }
                break;
                case ND_GRFNODE:
                case ND_OLENODE:
                    static_cast<SwCntntNode*>(pNd)->GetAttr( *pSet );
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
        pCurrent= static_cast<SwPaM *>(pCurrent->GetNext());
    } while ( pCurrent != &rPam );
}

/******************************************************************
 * SwXParagraphEnumeration
 ******************************************************************/
class SwXParagraphEnumeration::Impl
    : public SwClient
{

public:

    uno::Reference< text::XText > const     m_xParentText;
    const CursorType        m_eCursorType;
    /// Start node of the cell _or_ table the enumeration belongs to.
    /// Used to restrict the movement of the UNO cursor to the cell and its
    /// embedded tables.
    SwStartNode const*const m_pOwnStartNode;
    SwTable const*const     m_pOwnTable;
    const sal_uLong             m_nEndIndex;
    sal_Int32               m_nFirstParaStart;
    sal_Int32               m_nLastParaEnd;
    bool                    m_bFirstParagraph;
    uno::Reference< text::XTextContent >    m_xNextPara;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    Impl(   uno::Reference< text::XText > const& xParent,
            ::std::auto_ptr<SwUnoCrsr> pCursor,
            const CursorType eType,
            SwStartNode const*const pStartNode, SwTable const*const pTable)
        : SwClient( pCursor.release() )
        , m_xParentText( xParent )
        , m_eCursorType( eType )
        // remember table and start node for later travelling
        // (used in export of tables in tables)
        , m_pOwnStartNode( pStartNode )
        // for import of tables in tables we have to remember the actual
        // table and start node of the current position in the enumeration.
        , m_pOwnTable( pTable )
        , m_nEndIndex( GetCursor()->End()->nNode.GetIndex() )
        , m_nFirstParaStart( -1 )
        , m_nLastParaEnd( -1 )
        , m_bFirstParagraph( true )
    {
        OSL_ENSURE(m_xParentText.is(), "SwXParagraphEnumeration: no parent?");
        OSL_ENSURE(GetRegisteredIn(),  "SwXParagraphEnumeration: no cursor?");
        OSL_ENSURE(   !((CURSOR_SELECTION_IN_TABLE == eType) ||
                        (CURSOR_TBLTEXT == eType))
                   || (m_pOwnTable && m_pOwnStartNode),
            "SwXParagraphEnumeration: table type but no start node or table?");

        if ((CURSOR_SELECTION == m_eCursorType) ||
            (CURSOR_SELECTION_IN_TABLE == m_eCursorType))
        {
            SwUnoCrsr & rCursor = *GetCursor();
            rCursor.Normalize();
            m_nFirstParaStart = rCursor.GetPoint()->nContent.GetIndex();
            m_nLastParaEnd = rCursor.GetMark()->nContent.GetIndex();
            rCursor.DeleteMark();
        }
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    ~Impl() {
        // Impl owns the cursor; delete it here: SolarMutex is locked
        delete GetRegisteredIn();
    }

    SwUnoCrsr * GetCursor() {
        return static_cast<SwUnoCrsr*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }

    uno::Reference< text::XTextContent > NextElement_Impl()
        throw (container::NoSuchElementException, lang::WrappedTargetException,
                uno::RuntimeException);
protected:
    // SwClient
    virtual void Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew);

};

void SwXParagraphEnumeration::Impl::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
SwXParagraphEnumeration::SwXParagraphEnumeration(
        uno::Reference< text::XText > const& xParent,
        ::std::auto_ptr<SwUnoCrsr> pCursor,
        const CursorType eType,
        SwStartNode const*const pStartNode, SwTable const*const pTable)
    : m_pImpl( new SwXParagraphEnumeration::Impl(xParent, pCursor, eType,
                    pStartNode, pTable) )
{
}
SAL_WNODEPRECATED_DECLARATIONS_POP

SwXParagraphEnumeration::~SwXParagraphEnumeration()
{
}

OUString SAL_CALL
SwXParagraphEnumeration::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXParagraphEnumeration");
}

static char const*const g_ServicesParagraphEnum[] =
{
    "com.sun.star.text.ParagraphEnumeration",
};

static const size_t g_nServicesParagraphEnum(
    sizeof(g_ServicesParagraphEnum)/sizeof(g_ServicesParagraphEnum[0]));

sal_Bool SAL_CALL
SwXParagraphEnumeration::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesParagraphEnum, g_ServicesParagraphEnum, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXParagraphEnumeration::getSupportedServiceNames()
throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesParagraphEnum, g_ServicesParagraphEnum);
}

sal_Bool SAL_CALL
SwXParagraphEnumeration::hasMoreElements() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return (m_pImpl->m_bFirstParagraph) ? sal_True : m_pImpl->m_xNextPara.is();
}

//!! compare to SwShellTableCrsr::FillRects() in viscrs.cxx
static SwTableNode *
lcl_FindTopLevelTable(
        SwTableNode *const pTblNode, SwTable const*const pOwnTable)
{
    // find top-most table in current context (section) level

    SwTableNode * pLast = pTblNode;
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
SwXParagraphEnumeration::Impl::NextElement_Impl()
throw (container::NoSuchElementException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SwUnoCrsr *const pUnoCrsr = GetCursor();
    if (!pUnoCrsr)
    {
        throw uno::RuntimeException();
    }

    // check for exceeding selections
    if (!m_bFirstParagraph &&
        ((CURSOR_SELECTION == m_eCursorType) ||
         (CURSOR_SELECTION_IN_TABLE == m_eCursorType)))
    {
        SwPosition* pStart = pUnoCrsr->Start();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        const ::std::auto_ptr<SwUnoCrsr> aNewCrsr(
            pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, false) );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        // one may also go into tables here
        if ((CURSOR_TBLTEXT != m_eCursorType) &&
            (CURSOR_SELECTION_IN_TABLE != m_eCursorType))
        {
            aNewCrsr->SetRemainInSection( sal_False );
        }

        // os 2005-01-14: This part is only necessary to detect movements out
        // of a selection; if there is no selection we don't have to care
        SwTableNode *const pTblNode = aNewCrsr->GetNode()->FindTableNode();
        if (((CURSOR_TBLTEXT != m_eCursorType) &&
            (CURSOR_SELECTION_IN_TABLE != m_eCursorType)) && pTblNode)
        {
            aNewCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
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

    sal_Bool bInTable = sal_False;
    if (!m_bFirstParagraph)
    {
        pUnoCrsr->SetRemainInSection( sal_False );
        // what to do if already in a table?
        SwTableNode * pTblNode = pUnoCrsr->GetNode()->FindTableNode();
        pTblNode = lcl_FindTopLevelTable( pTblNode, m_pOwnTable );
        if (pTblNode && (&pTblNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table: go to end
            pUnoCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
            if (!pUnoCrsr->Move(fnMoveForward, fnGoNode))
            {
                return 0;
            }
            bInTable = sal_True;
        }
    }

    uno::Reference< text::XTextContent >  xRef;
    // the cursor must remain in the current section or a subsection
    // before AND after the movement...
    if (lcl_CursorIsInSection( pUnoCrsr, m_pOwnStartNode ) &&
        (m_bFirstParagraph || bInTable ||
        (pUnoCrsr->MovePara(fnParaNext, fnParaStart) &&
            lcl_CursorIsInSection( pUnoCrsr, m_pOwnStartNode ))))
    {
        SwPosition* pStart = pUnoCrsr->Start();
        const sal_Int32 nFirstContent =
            (m_bFirstParagraph) ? m_nFirstParaStart : -1;
        const sal_Int32 nLastContent =
            (m_nEndIndex == pStart->nNode.GetIndex()) ? m_nLastParaEnd : -1;

        // position in a table, or in a simple paragraph?
        SwTableNode * pTblNode = pUnoCrsr->GetNode()->FindTableNode();
        pTblNode = lcl_FindTopLevelTable( pTblNode, m_pOwnTable );
        if (/*CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType && */
            pTblNode && (&pTblNode->GetTable() != m_pOwnTable))
        {
            // this is a foreign table
            SwFrmFmt* pTableFmt =
                static_cast<SwFrmFmt*>(pTblNode->GetTable().GetFrmFmt());
            text::XTextTable *const pTable =
                SwXTextTables::GetObject( *pTableFmt );
            xRef = static_cast<text::XTextContent*>(
                    static_cast<SwXTextTable*>(pTable));
        }
        else
        {
            text::XText *const pText = m_xParentText.get();
            xRef = SwXParagraph::CreateXParagraph(*pUnoCrsr->GetDoc(),
                *pStart->nNode.GetNode().GetTxtNode(),
                static_cast<SwXText*>(pText), nFirstContent, nLastContent);
        }
    }

    return xRef;
}

uno::Any SAL_CALL SwXParagraphEnumeration::nextElement()
throw (container::NoSuchElementException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (m_pImpl->m_bFirstParagraph)
    {
        m_pImpl->m_xNextPara = m_pImpl->NextElement_Impl();
        m_pImpl->m_bFirstParagraph = false;
    }
    const uno::Reference< text::XTextContent > xRef = m_pImpl->m_xNextPara;
    if (!xRef.is())
    {
        throw container::NoSuchElementException();
    }
    m_pImpl->m_xNextPara = m_pImpl->NextElement_Impl();

    uno::Any aRet;
    aRet <<= xRef;
    return aRet;
}

/******************************************************************
 * SwXTextRange
 ******************************************************************/
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
            SwFrmFmt *const pTblFmt = 0,
            const uno::Reference< text::XText > & xParent = 0)
        : SwClient()
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
        , m_eRangePosition(eRange)
        , m_rDoc(rDoc)
        , m_xParentText(xParent)
        , m_ObjectDepend(this, pTblFmt)
        , m_pMark(0)
    {
    }

    ~Impl()
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
    virtual void    Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew);

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
            const_cast<SwModify*>(GetRegisteredIn())->Remove(this);
        }
        // or if the range has been removed but the depend ist still
        // connected then the depend must be removed
        else if (bAlreadyRegistered && !GetRegisteredIn() &&
                    m_ObjectDepend.GetRegisteredIn())
        {
            const_cast<SwModify*>(m_ObjectDepend.GetRegisteredIn())
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

SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt)
    : m_pImpl(
        new SwXTextRange::Impl(*rTblFmt.GetDoc(), RANGE_IS_TABLE, &rTblFmt) )
{
    SwTable *const pTable = SwTable::FindTable( &rTblFmt );
    SwTableNode *const pTblNode = pTable->GetTableNode();
    SwPosition aPosition( *pTblNode );
    SwPaM aPam( aPosition );

    SetPositions( aPam );
}

SwXTextRange::~SwXTextRange()
{
}

const SwDoc * SwXTextRange::GetDoc() const
{
    return & m_pImpl->m_rDoc;
}

SwDoc * SwXTextRange::GetDoc()
{
    return & m_pImpl->m_rDoc;
}

void SwXTextRange::Invalidate()
{
    m_pImpl->Invalidate();
}

void SwXTextRange::SetPositions(const SwPaM& rPam)
{
    m_pImpl->Invalidate();
    IDocumentMarkAccess* const pMA = m_pImpl->m_rDoc.getIDocumentMarkAccess();
    m_pImpl->m_pMark = pMA->makeMark(rPam, ::rtl::OUString(),
                IDocumentMarkAccess::UNO_BOOKMARK);
    m_pImpl->m_pMark->Add(m_pImpl.get());
}

void SwXTextRange::DeleteAndInsert(
        const ::rtl::OUString& rText, const bool bForceExpandHints)
throw (uno::RuntimeException)
{
    if (RANGE_IS_TABLE == m_pImpl->m_eRangePosition)
    {
        // setString on table not allowed
        throw uno::RuntimeException();
    }

    const SwPosition aPos(GetDoc()->GetNodes().GetEndOfContent());
    SwCursor aCursor(aPos, 0, false);
    if (GetPositions(aCursor))
    {
        UnoActionContext aAction(& m_pImpl->m_rDoc);
        m_pImpl->m_rDoc.GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);
        if (aCursor.HasMark())
        {
            m_pImpl->m_rDoc.DeleteAndJoin(aCursor);
        }

        if (!rText.isEmpty())
        {
            SwUnoCursorHelper::DocInsertStringSplitCR(
                    m_pImpl->m_rDoc, aCursor, rText, bForceExpandHints);

            SwUnoCursorHelper::SelectPam(aCursor, true);
            aCursor.Left(rText.getLength(), CRSR_SKIP_CHARS, sal_False, sal_False);
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
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXTextRange>(rId, this);
}

OUString SAL_CALL
SwXTextRange::getImplementationName() throw (uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SwXTextRange"));
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
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesTextRange, g_ServicesTextRange, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesTextRange, g_ServicesTextRange);
}

uno::Reference< text::XText > SAL_CALL
SwXTextRange::getText() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_xParentText.is())
    {
        if (m_pImpl->m_eRangePosition == RANGE_IS_TABLE &&
            m_pImpl->m_ObjectDepend.GetRegisteredIn())
        {
            SwFrmFmt const*const pTblFmt = static_cast<SwFrmFmt const*>(
                    m_pImpl->m_ObjectDepend.GetRegisteredIn());
            SwTable const*const pTable = SwTable::FindTable( pTblFmt );
            SwTableNode const*const pTblNode = pTable->GetTableNode();
            const SwPosition aPosition( *pTblNode );
            m_pImpl->m_xParentText =
                ::sw::CreateParentXText(m_pImpl->m_rDoc, aPosition);
        }
    }
    OSL_ENSURE(m_pImpl->m_xParentText.is(), "SwXTextRange::getText: no text");
    return m_pImpl->m_xParentText;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getStart() throw (uno::RuntimeException)
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
        // start and end are this, if its a table
        xRet = this;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextRange::getEnd() throw (uno::RuntimeException)
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
        // start and end are this, if its a table
        xRet = this;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return xRet;
}

OUString SAL_CALL SwXTextRange::getString() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    OUString sRet;
    // for tables there is no bookmark, thus also no text
    // one could export the table as ASCII here maybe?
    SwPaM aPaM(GetDoc()->GetNodes());
    if (GetPositions(aPaM) && aPaM.HasMark())
    {
        SwUnoCursorHelper::GetTextFromPam(aPaM, sRet);
    }
    return sRet;
}

void SAL_CALL SwXTextRange::setString(const OUString& rString)
throw (uno::RuntimeException)
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
    if(pRange && pRange->GetDoc() == rToFill.GetDoc())
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
                : ((pPortion) ? pPortion->GetCursor()->GetDoc() : 0);
            const SwPaM* const pUnoCrsr = (pCursor) ? pCursor->GetPaM()
                : ((pPortion) ? pPortion->GetCursor() : 0);
            if (pUnoCrsr && pDoc == rToFill.GetDoc())
            {
                OSL_ENSURE((SwPaM*)pUnoCrsr->GetNext() == pUnoCrsr,
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
    SwFrmFmt const*const pFrmFmt, SwFrmFmt*& rpFormat)
{
    bool bRet = false;
    const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
    const SfxPoolItem* pItem;
    if (SFX_ITEM_SET == rSet.GetItemState(
            static_cast<sal_uInt16>(bHeader ? RES_HEADER : RES_FOOTER),
            sal_True, &pItem))
    {
        SfxPoolItem *const pItemNonConst(const_cast<SfxPoolItem *>(pItem));
        SwFrmFmt *const pHeadFootFmt = (bHeader) ?
            static_cast<SwFmtHeader*>(pItemNonConst)->GetHeaderFmt() :
            static_cast<SwFmtFooter*>(pItemNonConst)->GetFooterFmt();
        if (pHeadFootFmt)
        {
            const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
            const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
            SwStartNode const*const pCurSttNode = rNode.FindSttNodeByType(
                (bHeader) ? SwHeaderStartNode : SwFooterStartNode);
            if (pCurSttNode && (pCurSttNode == pSttNode))
            {
                rpFormat = pHeadFootFmt;
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
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
            rDoc.CreateUnoCrsr(rPos, false));
    SAL_WNODEPRECATED_DECLARATIONS_POP
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
    SwStartNodeType eType = pSttNode->GetStartNodeType();
    switch(eType)
    {
        case SwTableBoxStartNode:
        {
            SwTableNode const*const pTblNode = pSttNode->FindTableNode();
            SwFrmFmt *const pTableFmt =
                static_cast<SwFrmFmt*>(pTblNode->GetTable().GetFrmFmt());
            SwTableBox *const  pBox = pSttNode->GetTblBox();

            xParentText = (pBox)
                ? SwXCell::CreateXCell( pTableFmt, pBox )
                : new SwXCell( pTableFmt, *pSttNode );
        }
        break;
        case SwFlyStartNode:
        {
            SwFrmFmt *const pFmt = pSttNode->GetFlyFmt();
            if (0 != pFmt)
            {
                SwXTextFrame* pFrame = SwIterator<SwXTextFrame,SwFmt>::FirstElement( *pFmt );
                xParentText = pFrame ? pFrame : new SwXTextFrame( *pFmt );
            }
        }
        break;
        case SwHeaderStartNode:
        case SwFooterStartNode:
        {
            const bool bHeader = (SwHeaderStartNode == eType);
            const sal_uInt16 nPDescCount = rDoc.GetPageDescCnt();
            for(sal_uInt16 i = 0; i < nPDescCount; i++)
            {
                const SwPageDesc& rDesc = rDoc.GetPageDesc( i );
                const SwFrmFmt* pFrmFmtMaster = &rDesc.GetMaster();
                const SwFrmFmt* pFrmFmtLeft = &rDesc.GetLeft();

                SwFrmFmt* pHeadFootFmt = 0;
                if (!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtMaster,
                            pHeadFootFmt))
                {
                    lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtLeft,
                            pHeadFootFmt);
                }

                if (pHeadFootFmt)
                {
                    xParentText = SwXHeadFootText::CreateXHeadFootText(
                            *pHeadFootFmt, bHeader);
                }
            }
        }
        break;
        case SwFootnoteStartNode:
        {
            const sal_uInt16 nFtnCnt = rDoc.GetFtnIdxs().size();
            uno::Reference< text::XFootnote >  xRef;
            for (sal_uInt16 n = 0; n < nFtnCnt; ++n )
            {
                const SwTxtFtn* pTxtFtn = rDoc.GetFtnIdxs()[ n ];
                const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
                pTxtFtn = rFtn.GetTxtFtn();
#if OSL_DEBUG_LEVEL > 1
                const SwStartNode* pTmpSttNode =
                        pTxtFtn->GetStartNode()->GetNode().
                                FindSttNodeByType(SwFootnoteStartNode);
                (void)pTmpSttNode;
#endif

                if (pSttNode == pTxtFtn->GetStartNode()->GetNode().
                                    FindSttNodeByType(SwFootnoteStartNode))
                {
                    xParentText = SwXFootnote::CreateXFootnote(rDoc, rFtn);
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
throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if ( rServiceName != "com.sun.star.text.TextContent" )
    {
        throw uno::RuntimeException();
    }

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    const SwPosition aPos(GetDoc()->GetNodes().GetEndOfContent());
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
            m_pImpl->m_rDoc.CreateUnoCrsr(aPos, false));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (!GetPositions(*pNewCrsr))
    {
        throw uno::RuntimeException();
    }

    const uno::Reference< container::XEnumeration > xRet =
        new SwXParaFrameEnumeration(*pNewCrsr, PARAFRAME_PORTION_TEXTRANGE);
    return xRet;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXTextRange::createEnumeration() throw (uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    const SwPosition aPos(GetDoc()->GetNodes().GetEndOfContent());
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
            m_pImpl->m_rDoc.CreateUnoCrsr(aPos, false));
    SAL_WNODEPRECATED_DECLARATIONS_POP
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
    const uno::Reference< container::XEnumeration > xRet =
        new SwXParagraphEnumeration(m_pImpl->m_xParentText, pNewCrsr, eSetType);
    return xRet;
}

uno::Type SAL_CALL SwXTextRange::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

sal_Bool SAL_CALL SwXTextRange::hasElements() throw (uno::RuntimeException)
{
    return sal_True;
}

uno::Sequence< OUString > SAL_CALL
SwXTextRange::getAvailableServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextContent"));
    return aRet;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextRange::getPropertySetInfo() throw (uno::RuntimeException)
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
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    SwUnoCursorHelper::SetPropertyValue(aPaM, m_pImpl->m_rPropSet,
            rPropertyName, rValue);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyValue(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL
SwXTextRange::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXTextRange::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXTextRange::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXTextRange::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXTextRange::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_FAIL("SwXTextRange::removeVetoableChangeListener(): not implemented");
}

beans::PropertyState SAL_CALL
SwXTextRange::getPropertyState(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyState(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextRange::getPropertyStates(const uno::Sequence< OUString >& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard g;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyStates(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL SwXTextRange::setPropertyToDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    SwUnoCursorHelper::SetPropertyToDefault(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

uno::Any SAL_CALL
SwXTextRange::getPropertyDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    GetPositions(aPaM);
    return SwUnoCursorHelper::GetPropertyDefault(aPaM, m_pImpl->m_rPropSet,
            rPropertyName);
}

void SAL_CALL
SwXTextRange::makeRedline(
    const ::rtl::OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!GetDoc() || !m_pImpl->GetBookmark())
    {
        throw uno::RuntimeException();
    }
    SwPaM aPaM(GetDoc()->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwUnoCursorHelper::makeRedline( aPaM, rRedlineType, rRedlineProperties );
}

/******************************************************************
 * SwXTextRanges
 ******************************************************************/
class SwXTextRanges::Impl
    : public SwClient
{

public:

    ::std::vector< uno::Reference< text::XTextRange > > m_Ranges;

    Impl(SwPaM *const pPaM)
        : SwClient( (pPaM)
            ? pPaM->GetDoc()->CreateUnoCrsr(*pPaM->GetPoint())
            : 0 )
    {
        if (pPaM)
        {
            ::sw::DeepCopyPaM(*pPaM, *GetCursor());
        }
        MakeRanges();
    }

    ~Impl() {
        // Impl owns the cursor; delete it here: SolarMutex is locked
        delete GetRegisteredIn();
    }

    SwUnoCrsr * GetCursor() {
        return static_cast<SwUnoCrsr*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }

    void MakeRanges();
protected:
    // SwClient
    virtual void Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew);

};

void SwXTextRanges::Impl::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

void SwXTextRanges::Impl::MakeRanges()
{
    SwUnoCrsr *const pCursor = GetCursor();
    if (pCursor)
    {
        SwPaM *pTmpCursor = pCursor;
        do {
            const uno::Reference< text::XTextRange > xRange(
                    SwXTextRange::CreateXTextRange(
                        *pTmpCursor->GetDoc(),
                        *pTmpCursor->GetPoint(), pTmpCursor->GetMark()));
            if (xRange.is())
            {
                m_Ranges.push_back(xRange);
            }
            pTmpCursor = static_cast<SwPaM*>(pTmpCursor->GetNext());
        }
        while (pTmpCursor != pCursor);
    }
}

const SwUnoCrsr* SwXTextRanges::GetCursor() const
{
    return m_pImpl->GetCursor();
}

SwXTextRanges::SwXTextRanges(SwPaM *const pPaM)
    : m_pImpl( new SwXTextRanges::Impl(pPaM) )
{
}

SwXTextRanges::~SwXTextRanges()
{
}

namespace
{
    class theSwXTextRangesUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextRangesUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextRanges::getUnoTunnelId()
{
    return theSwXTextRangesUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXTextRanges::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXTextRanges>(rId, this);
}

/****************************************************************************
 *  Text positions
 *  Bis zum ersten Zugriff auf eine TextPosition wird ein SwCursor gehalten,
 * danach wird ein Array mit uno::Reference< XTextPosition >  angelegt
 *
****************************************************************************/
OUString SAL_CALL
SwXTextRanges::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXTextRanges");
}

static char const*const g_ServicesTextRanges[] =
{
    "com.sun.star.text.TextRanges",
};

static const size_t g_nServicesTextRanges(
    sizeof(g_ServicesTextRanges)/sizeof(g_ServicesTextRanges[0]));

sal_Bool SAL_CALL SwXTextRanges::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesTextRanges, g_ServicesTextRanges, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextRanges::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesTextRanges, g_ServicesTextRanges);
}

sal_Int32 SAL_CALL SwXTextRanges::getCount() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return static_cast<sal_Int32>(m_pImpl->m_Ranges.size());
}

uno::Any SAL_CALL SwXTextRanges::getByIndex(sal_Int32 nIndex)
throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ((nIndex < 0) ||
        (static_cast<size_t>(nIndex) >= m_pImpl->m_Ranges.size()))
    {
        throw lang::IndexOutOfBoundsException();
    }
    uno::Any ret;
    ret <<= (m_pImpl->m_Ranges.at(nIndex));
    return ret;
}

uno::Type SAL_CALL
SwXTextRanges::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

sal_Bool SAL_CALL SwXTextRanges::hasElements() throw (uno::RuntimeException)
{
    // no mutex necessary: getCount() does locking
    return getCount() > 0;
}

void SwUnoCursorHelper::SetString(SwCursor & rCursor, const OUString& rString)
{
    // Start/EndAction
    SwDoc *const pDoc = rCursor.GetDoc();
    UnoActionContext aAction(pDoc);
    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);
    if (rCursor.HasMark())
    {
        pDoc->DeleteAndJoin(rCursor);
    }
    if (!rString.isEmpty())
    {
        String aText(rString);
        const bool bSuccess( SwUnoCursorHelper::DocInsertStringSplitCR(
                    *pDoc, rCursor, aText, false ) );
        OSL_ENSURE( bSuccess, "DocInsertStringSplitCR" );
        (void) bSuccess;
        SwUnoCursorHelper::SelectPam(rCursor, true);
        rCursor.Left(rString.getLength(), CRSR_SKIP_CHARS, sal_False, sal_False);
    }
    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
}

/******************************************************************
 * SwXParaFrameEnumeration
 ******************************************************************/
class SwXParaFrameEnumeration::Impl
    : public SwClient
{

public:

    // created by hasMoreElements
    uno::Reference< text::XTextContent > m_xNextObject;
    FrameDependList_t m_Frames;

    Impl(SwPaM const & rPaM)
        : SwClient(rPaM.GetDoc()->CreateUnoCrsr(*rPaM.GetPoint(), false))
    {
        if (rPaM.HasMark())
        {
            GetCursor()->SetMark();
            *GetCursor()->GetMark() = *rPaM.GetMark();
        }
    }

    ~Impl() {
        // Impl owns the cursor; delete it here: SolarMutex is locked
        delete GetRegisteredIn();
    }

    SwUnoCrsr * GetCursor() {
        return static_cast<SwUnoCrsr*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }
protected:
    // SwClient
    virtual void Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew);

};

struct InvalidFrameDepend {
    bool operator() (::boost::shared_ptr<SwDepend> const & rEntry)
    { return !rEntry->GetRegisteredIn(); }
};

void SwXParaFrameEnumeration::Impl::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_Frames.clear();
        m_xNextObject = 0;
    }
    else
    {
        // check if any frame went away...
        FrameDependList_t::iterator const iter =
            ::std::remove_if(m_Frames.begin(), m_Frames.end(),
                    InvalidFrameDepend());
        m_Frames.erase(iter, m_Frames.end());
    }
}

static sal_Bool
lcl_CreateNextObject(SwUnoCrsr& i_rUnoCrsr,
        uno::Reference<text::XTextContent> & o_rNextObject,
        FrameDependList_t & i_rFrames)
{
    if (!i_rFrames.size())
        return sal_False;

    SwFrmFmt *const pFormat = static_cast<SwFrmFmt*>(const_cast<SwModify*>(
                i_rFrames.front()->GetRegisteredIn()));
    i_rFrames.pop_front();
    // the format should be valid here, otherwise the client
    // would have been removed in ::Modify
    // check for a shape first
    SwDrawContact* const pContact = SwIterator<SwDrawContact,SwFmt>::FirstElement( *pFormat );
    if (pContact)
    {
        SdrObject * const pSdr = pContact->GetMaster();
        if (pSdr)
        {
            o_rNextObject.set(pSdr->getUnoShape(), uno::UNO_QUERY);
        }
    }
    else
    {
        const SwNodeIndex* pIdx = pFormat->GetCntnt().GetCntntIdx();
        OSL_ENSURE(pIdx, "where is the index?");
        SwNode const*const pNd =
            i_rUnoCrsr.GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];

        const FlyCntType eType = (!pNd->IsNoTxtNode()) ? FLYCNTTYPE_FRM
            : ( (pNd->IsGrfNode()) ? FLYCNTTYPE_GRF : FLYCNTTYPE_OLE );

        const uno::Reference< container::XNamed >  xFrame =
            SwXFrames::GetObject(*pFormat, eType);
        o_rNextObject.set(xFrame, uno::UNO_QUERY);
    }

    return o_rNextObject.is();
}

/* ---------------------------------------------------------------------------
    Description: Search for a FLYCNT text attribute at the cursor point
                and fill the frame into the array
 ---------------------------------------------------------------------------*/
static void
lcl_FillFrame(SwClient & rEnum, SwUnoCrsr& rUnoCrsr,
        FrameDependList_t & rFrames)
{
    // search for objects at the cursor - anchored at/as char
    SwTxtAttr const*const pTxtAttr =
        rUnoCrsr.GetNode()->GetTxtNode()->GetTxtAttrForCharAt(
            rUnoCrsr.GetPoint()->nContent.GetIndex(), RES_TXTATR_FLYCNT);
    if (pTxtAttr)
    {
        const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
        SwFrmFmt * const  pFrmFmt = rFlyCnt.GetFrmFmt();
        SwDepend * const pNewDepend = new SwDepend(&rEnum, pFrmFmt);
        rFrames.push_back( ::boost::shared_ptr<SwDepend>(pNewDepend) );
    }
}

SwXParaFrameEnumeration::SwXParaFrameEnumeration(
        const SwPaM& rPaM, const enum ParaFrameMode eParaFrameMode,
        SwFrmFmt *const pFmt)
    : m_pImpl( new SwXParaFrameEnumeration::Impl(rPaM) )
{
    if (PARAFRAME_PORTION_PARAGRAPH == eParaFrameMode)
    {
        FrameDependSortList_t frames;
        ::CollectFrameAtNode(*m_pImpl.get(), rPaM.GetPoint()->nNode,
                frames, false);
        ::std::transform(frames.begin(), frames.end(),
            ::std::back_inserter(m_pImpl->m_Frames),
            ::boost::bind(&FrameDependSortListEntry::pFrameDepend, _1));
    }
    else if (pFmt)
    {
        // create SwDepend for frame and insert into array
        SwDepend *const pNewDepend = new SwDepend(m_pImpl.get(), pFmt);
        m_pImpl->m_Frames.push_back(::boost::shared_ptr<SwDepend>(pNewDepend));
    }
    else if ((PARAFRAME_PORTION_CHAR == eParaFrameMode) ||
             (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode))
    {
        if (PARAFRAME_PORTION_TEXTRANGE == eParaFrameMode)
        {
            SwPosFlyFrms aFlyFrms;
            //get all frames that are bound at paragraph or at character
            rPaM.GetDoc()->GetAllFlyFmts(aFlyFrms, m_pImpl->GetCursor(), false, true);
            for(SwPosFlyFrms::iterator it = aFlyFrms.begin(); it != aFlyFrms.end(); ++it)
            {
                SwPosFlyFrm* pPosFly = *it;
                SwFrmFmt *const pFrmFmt =
                    const_cast<SwFrmFmt*>(&pPosFly->GetFmt());
                // create SwDepend for frame and insert into array
                SwDepend *const pNewDepend =
                    new SwDepend(m_pImpl.get(), pFrmFmt);
                m_pImpl->m_Frames.push_back(
                        ::boost::shared_ptr<SwDepend>(pNewDepend) );
            }
        }
        lcl_FillFrame(*m_pImpl.get(), *m_pImpl->GetCursor(), m_pImpl->m_Frames);
    }
}

SwXParaFrameEnumeration::~SwXParaFrameEnumeration()
{
}

sal_Bool SAL_CALL
SwXParaFrameEnumeration::hasMoreElements() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetCursor())
        throw uno::RuntimeException();

    return (m_pImpl->m_xNextObject.is())
        ? sal_True
        : lcl_CreateNextObject(*m_pImpl->GetCursor(),
            m_pImpl->m_xNextObject, m_pImpl->m_Frames);
}

uno::Any SAL_CALL SwXParaFrameEnumeration::nextElement()
throw (container::NoSuchElementException,
        lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->GetCursor())
    {
        throw uno::RuntimeException();
    }

    if (!m_pImpl->m_xNextObject.is() && m_pImpl->m_Frames.size())
    {
        lcl_CreateNextObject(*m_pImpl->GetCursor(),
                m_pImpl->m_xNextObject, m_pImpl->m_Frames);
    }
    if (!m_pImpl->m_xNextObject.is())
    {
        throw container::NoSuchElementException();
    }
    uno::Any aRet;
    aRet <<= m_pImpl->m_xNextObject;
    m_pImpl->m_xNextObject = 0;
    return aRet;
}

OUString SAL_CALL
SwXParaFrameEnumeration::getImplementationName() throw (uno::RuntimeException)
{
    return C2U("SwXParaFrameEnumeration");
}

static char const*const g_ServicesParaFrameEnum[] =
{
    "com.sun.star.util.ContentEnumeration",
};

static const size_t g_nServicesParaFrameEnum(
    sizeof(g_ServicesParaFrameEnum)/sizeof(g_ServicesParaFrameEnum[0]));

sal_Bool SAL_CALL
SwXParaFrameEnumeration::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesParaFrameEnum, g_ServicesParaFrameEnum, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXParaFrameEnumeration::getSupportedServiceNames()
throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesParaFrameEnum, g_ServicesParaFrameEnum);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
