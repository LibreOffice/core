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
#include <sal/log.hxx>

#include <utility>

#include <unoport.hxx>
#include <IMark.hxx>
#include <crossrefbookmark.hxx>
#include <annotationmark.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <txatbase.hxx>
#include <txtatr.hxx>
#include <ndhints.hxx>
#include <ndtxt.hxx>
#include <unocrsr.hxx>
#include <docary.hxx>
#include <textboxhelper.hxx>
#include <tox.hxx>
#include <unomid.h>
#include <unoparaframeenum.hxx>
#include <unocrsrhelper.hxx>
#include <unorefmark.hxx>
#include <unobookmark.hxx>
#include <unoredline.hxx>
#include <unofield.hxx>
#include <unometa.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>
#include <fmtmeta.hxx>
#include <fmtanchr.hxx>
#include <fmtrfmrk.hxx>
#include <frmfmt.hxx>
#include <fmtflcnt.hxx>
#include <unoidx.hxx>
#include <unocoll.hxx>
#include <redline.hxx>
#include <docufld.hxx>
#include <txtfld.hxx>
#include <txtannotationfld.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/string.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <algorithm>
#include <memory>
#include <set>
#include <stack>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::std;

typedef std::pair< TextRangeList_t * const, SwTextAttr const * const > PortionList_t;
typedef std::stack< PortionList_t > PortionStack_t;

static void lcl_CreatePortions(
    TextRangeList_t & i_rPortions,
    uno::Reference< text::XText > const& i_xParentText,
    SwUnoCursor* pUnoCursor,
    FrameClientSortList_t & i_rFrames,
    const sal_Int32 i_nStartPos, const sal_Int32 i_nEndPos );

namespace
{
    enum class BkmType {
        Start, End, StartEnd
    };

    struct SwXBookmarkPortion_Impl
    {
        Reference<XTextContent>     xBookmark;
        BkmType const               nBkmType;
        const SwPosition            aPosition;

        SwXBookmarkPortion_Impl(uno::Reference<text::XTextContent> const& xMark,
                const BkmType nType, SwPosition const& rPosition)
        : xBookmark ( xMark )
        , nBkmType  ( nType )
        , aPosition ( rPosition )
        {
        }
        sal_Int32 getIndex ()
        {
            return aPosition.nContent.GetIndex();
        }
    };
    typedef std::shared_ptr < SwXBookmarkPortion_Impl > SwXBookmarkPortion_ImplSharedPtr;
    struct BookmarkCompareStruct
    {
        bool operator () ( const SwXBookmarkPortion_ImplSharedPtr &r1,
                           const SwXBookmarkPortion_ImplSharedPtr &r2 ) const
        {
            // #i16896# for bookmark portions at the same position, the start should
            // always precede the end. Hence compare positions, and use bookmark type
            // as tie-breaker for same position.
            // return ( r1->nIndex   == r2->nIndex )
            //   ? ( r1->nBkmType <  r2->nBkmType )
            //   : ( r1->nIndex   <  r2->nIndex );

            // Note that the above code does not correctly handle
            // the case when one bookmark ends, and another begins in the same
            // position. When this occurs, the above code will return the
            // start of the 2nd bookmark BEFORE the end of the first bookmark
            // See bug #i58438# for more details. The below code is correct and
            // fixes both #i58438 and #i16896#
            return r1->aPosition < r2->aPosition;
        }
    };
    typedef std::multiset < SwXBookmarkPortion_ImplSharedPtr, BookmarkCompareStruct > SwXBookmarkPortion_ImplList;

    /// Inserts pBkmk to rBkmArr in case it starts or ends at nOwnNode
    void lcl_FillBookmark(sw::mark::IMark* const pBkmk, const SwNodeIndex& nOwnNode, SwDoc& rDoc, SwXBookmarkPortion_ImplList& rBkmArr)
    {
        bool const hasOther = pBkmk->IsExpanded();

        const SwPosition& rStartPos = pBkmk->GetMarkStart();
        if(rStartPos.nNode == nOwnNode)
        {
            // #i109272#: cross reference marks: need special handling!
            ::sw::mark::CrossRefBookmark *const pCrossRefMark(dynamic_cast< ::sw::mark::CrossRefBookmark*>(pBkmk));
            BkmType const nType = (hasOther || pCrossRefMark)
                ? BkmType::Start : BkmType::StartEnd;
            rBkmArr.insert(std::make_shared<SwXBookmarkPortion_Impl>(
                        SwXBookmark::CreateXBookmark(rDoc, pBkmk),
                        nType, rStartPos));
        }

        const SwPosition& rEndPos = pBkmk->GetMarkEnd();
        if(rEndPos.nNode == nOwnNode)
        {
            unique_ptr<SwPosition> pCrossRefEndPos;
            const SwPosition* pEndPos = nullptr;
            ::sw::mark::CrossRefBookmark *const pCrossRefMark(dynamic_cast< ::sw::mark::CrossRefBookmark*>(pBkmk));
            if(hasOther)
            {
                pEndPos = &rEndPos;
            }
            else if (pCrossRefMark)
            {
                // Crossrefbookmarks only remember the start position but have to span the whole paragraph
                pCrossRefEndPos = std::make_unique<SwPosition>(rEndPos);
                pCrossRefEndPos->nContent = pCrossRefEndPos->nNode.GetNode().GetTextNode()->Len();
                pEndPos = pCrossRefEndPos.get();
            }
            if(pEndPos)
            {
                rBkmArr.insert(std::make_shared<SwXBookmarkPortion_Impl>(
                            SwXBookmark::CreateXBookmark(rDoc, pBkmk),
                            BkmType::End, *pEndPos));
            }
        }
    }

    void lcl_FillBookmarkArray(SwDoc& rDoc, SwUnoCursor& rUnoCursor, SwXBookmarkPortion_ImplList& rBkmArr)
    {
        IDocumentMarkAccess* const pMarkAccess = rDoc.getIDocumentMarkAccess();
        if(!pMarkAccess->getBookmarksCount())
            return;

        const SwNodeIndex nOwnNode = rUnoCursor.GetPoint()->nNode;
        SwTextNode* pTextNode = nOwnNode.GetNode().GetTextNode();
        if (!pTextNode)
        {
            // no need to consider marks starting after aEndOfPara
            SwPosition aEndOfPara(*rUnoCursor.GetPoint());
            aEndOfPara.nContent = aEndOfPara.nNode.GetNode().GetTextNode()->Len();
            const IDocumentMarkAccess::const_iterator_t pCandidatesEnd = upper_bound(
                pMarkAccess->getBookmarksBegin(),
                pMarkAccess->getBookmarksEnd(),
                aEndOfPara,
                sw::mark::CompareIMarkStartsAfter()); // finds the first that starts after

            // search for all bookmarks that start or end in this paragraph
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getBookmarksBegin();
                ppMark != pCandidatesEnd;
                ++ppMark)
            {
                ::sw::mark::IMark* const pBkmk = ppMark->get();
                lcl_FillBookmark(pBkmk, nOwnNode, rDoc, rBkmArr);
            }
        }
        else
        {
            // A text node already knows its marks via its SwIndexes.
            std::set<sw::mark::IMark*> aSeenMarks;
            for (const SwIndex* pIndex = pTextNode->GetFirstIndex(); pIndex; pIndex = pIndex->GetNext())
            {
                // Need a non-cost mark here, as we'll create an UNO wrapper around it.
                sw::mark::IMark* pBkmk = const_cast<sw::mark::IMark*>(pIndex->GetMark());
                if (!pBkmk)
                    continue;
                IDocumentMarkAccess::MarkType eType = IDocumentMarkAccess::GetType(*pBkmk);
                // These are the types stored in the container otherwise accessible via getBookmarks*()
                if (eType != IDocumentMarkAccess::MarkType::BOOKMARK && eType != IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK &&
                    eType != IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK)
                    continue;
                // Only handle bookmarks once, if they start and end at this node as well.
                if (aSeenMarks.find(pBkmk) != aSeenMarks.end())
                    continue;
                aSeenMarks.insert(pBkmk);
                lcl_FillBookmark(pBkmk, nOwnNode, rDoc, rBkmArr);
            }
        }
    }

    class theSwXTextPortionEnumerationUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextPortionEnumerationUnoTunnelId > {};
    struct SwAnnotationStartPortion_Impl
    {

        uno::Reference< text::XTextField > mxAnnotationField;
        const SwPosition maPosition;

        SwAnnotationStartPortion_Impl(
            uno::Reference< text::XTextField > const& xAnnotationField,
            SwPosition const& rPosition)
        : mxAnnotationField ( xAnnotationField )
        , maPosition ( rPosition )
        {
        }

        sal_Int32 getIndex ()
        {
            return maPosition.nContent.GetIndex();
        }
    };
    typedef std::shared_ptr < SwAnnotationStartPortion_Impl > SwAnnotationStartPortion_ImplSharedPtr;
    struct AnnotationStartCompareStruct
    {
        bool operator () ( const SwAnnotationStartPortion_ImplSharedPtr &r1,
                           const SwAnnotationStartPortion_ImplSharedPtr &r2 )
            const
        {
            return r1->maPosition < r2->maPosition;
        }
    };
    typedef std::multiset < SwAnnotationStartPortion_ImplSharedPtr, AnnotationStartCompareStruct > SwAnnotationStartPortion_ImplList;

    void lcl_FillAnnotationStartArray(
        SwDoc& rDoc,
        SwUnoCursor& rUnoCursor,
        SwAnnotationStartPortion_ImplList& rAnnotationStartArr )
    {
        IDocumentMarkAccess* const pMarkAccess = rDoc.getIDocumentMarkAccess();
        if ( pMarkAccess->getAnnotationMarksCount() == 0 )
        {
            return;
        }

        // no need to consider annotation marks starting after aEndOfPara
        SwPosition aEndOfPara(*rUnoCursor.GetPoint());
        aEndOfPara.nContent = aEndOfPara.nNode.GetNode().GetTextNode()->Len();
        const IDocumentMarkAccess::const_iterator_t pCandidatesEnd = upper_bound(
            pMarkAccess->getAnnotationMarksBegin(),
            pMarkAccess->getAnnotationMarksEnd(),
            aEndOfPara,
            sw::mark::CompareIMarkStartsAfter()); // finds the first that starts after

        // search for all annotation marks that have its start position in this paragraph
        const SwNodeIndex nOwnNode = rUnoCursor.GetPoint()->nNode;
        for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAnnotationMarksBegin();
             ppMark != pCandidatesEnd;
             ++ppMark )
        {
            ::sw::mark::AnnotationMark* const pAnnotationMark =
                dynamic_cast< ::sw::mark::AnnotationMark* >(ppMark->get());

            if (!pAnnotationMark)
                continue;

            const SwPosition& rStartPos = pAnnotationMark->GetMarkStart();
            if (rStartPos.nNode != nOwnNode)
                continue;

            const SwFormatField* pAnnotationFormatField = pAnnotationMark->GetAnnotationFormatField();
            if (!pAnnotationFormatField)
            {
                SAL_WARN("sw.core", "missing annotation format field");
                continue;
            }

            rAnnotationStartArr.insert(
                std::make_shared<SwAnnotationStartPortion_Impl>(
                        SwXTextField::CreateXTextField(&rDoc,
                            pAnnotationFormatField),
                        rStartPos));
        }
    }
}

const uno::Sequence< sal_Int8 > & SwXTextPortionEnumeration::getUnoTunnelId()
{
    return theSwXTextPortionEnumerationUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXTextPortionEnumeration::getSomething(
        const uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ) );
    }
    return 0;
}

OUString SwXTextPortionEnumeration::getImplementationName()
{
    return OUString("SwXTextPortionEnumeration");
}

sal_Bool
SwXTextPortionEnumeration::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextPortionEnumeration::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.text.TextPortionEnumeration" };
    return aRet;
}

SwXTextPortionEnumeration::SwXTextPortionEnumeration(
        SwPaM& rParaCursor,
        uno::Reference< XText > const & xParentText,
        const sal_Int32 nStart,
        const sal_Int32 nEnd )
    : m_Portions()
{
    m_pUnoCursor = rParaCursor.GetDoc()->CreateUnoCursor(*rParaCursor.GetPoint());

    OSL_ENSURE(nEnd == -1 || (nStart <= nEnd &&
        nEnd <= m_pUnoCursor->Start()->nNode.GetNode().GetTextNode()->GetText().getLength()),
            "start or end value invalid!");

    // find all frames, graphics and OLEs that are bound AT character in para
    FrameClientSortList_t frames;
    ::CollectFrameAtNode(m_pUnoCursor->GetPoint()->nNode, frames, true);
    lcl_CreatePortions(m_Portions, xParentText, &*m_pUnoCursor, frames, nStart, nEnd);
}

SwXTextPortionEnumeration::SwXTextPortionEnumeration(
        SwPaM& rParaCursor,
        TextRangeList_t const & rPortions )
    : m_Portions( rPortions )
{
    m_pUnoCursor = rParaCursor.GetDoc()->CreateUnoCursor(*rParaCursor.GetPoint());
}

SwXTextPortionEnumeration::~SwXTextPortionEnumeration()
{
    SolarMutexGuard aGuard;
    m_pUnoCursor.reset(nullptr);
}

sal_Bool SwXTextPortionEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;

    return !m_Portions.empty();
}

uno::Any SwXTextPortionEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if (m_Portions.empty())
        throw container::NoSuchElementException();

    Any any;
    any <<= m_Portions.front();
    m_Portions.pop_front();
    return any;
}

static void
lcl_FillFieldMarkArray(std::deque<sal_Int32> & rFieldMarks, SwUnoCursor const & rUnoCursor,
        const sal_Int32 i_nStartPos)
{
    const SwTextNode * const pTextNode =
        rUnoCursor.GetPoint()->nNode.GetNode().GetTextNode();
    if (!pTextNode) return;

    const sal_Unicode fld[] = {
        CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND, CH_TXT_ATR_FORMELEMENT, 0 };
    sal_Int32 pos = std::max(static_cast<sal_Int32>(0), i_nStartPos);
    while ((pos = ::comphelper::string::indexOfAny(pTextNode->GetText(), fld, pos)) != -1)
    {
        rFieldMarks.push_back(pos);
        ++pos;
    }
}

static uno::Reference<text::XTextRange>
lcl_ExportFieldMark(
        uno::Reference< text::XText > const & i_xParentText,
        SwUnoCursor * const pUnoCursor,
        const SwTextNode * const pTextNode )
{
    uno::Reference<text::XTextRange> xRef;
    SwDoc* pDoc = pUnoCursor->GetDoc();
    // maybe it's a good idea to add a special hint to the hints array and rely on the hint segmentation....
    const sal_Int32 start = pUnoCursor->Start()->nContent.GetIndex();
    OSL_ENSURE(pUnoCursor->End()->nContent.GetIndex() == start,
               "hmm --- why is this different");

    pUnoCursor->Right(1);
    if ( *pUnoCursor->GetMark() == *pUnoCursor->GetPoint() )
    {
        OSL_FAIL("cannot move cursor?");
        return nullptr;
    }

    const sal_Unicode Char = pTextNode->GetText()[start];
    if (CH_TXT_ATR_FIELDSTART == Char)
    {
        ::sw::mark::IFieldmark* pFieldmark = nullptr;
        if (pDoc)
        {
            pFieldmark = pDoc->getIDocumentMarkAccess()->
                getFieldmarkFor(*pUnoCursor->GetMark());
        }
        SwXTextPortion* pPortion = new SwXTextPortion(
            pUnoCursor, i_xParentText, PORTION_FIELD_START);
        xRef = pPortion;
        if (pFieldmark && pDoc)
        {
            pPortion->SetBookmark(
                SwXFieldmark::CreateXFieldmark(*pDoc, pFieldmark));
        }
    }
    else if (CH_TXT_ATR_FIELDEND == Char)
    {
        ::sw::mark::IFieldmark* pFieldmark = nullptr;
        if (pDoc)
        {
            pFieldmark = pDoc->getIDocumentMarkAccess()->
                getFieldmarkFor(*pUnoCursor->GetMark());
        }
        SwXTextPortion* pPortion = new SwXTextPortion(
            pUnoCursor, i_xParentText, PORTION_FIELD_END);
        xRef = pPortion;
        if (pFieldmark && pDoc)
        {
            pPortion->SetBookmark(
                SwXFieldmark::CreateXFieldmark(*pDoc, pFieldmark));
        }
    }
    else if (CH_TXT_ATR_FORMELEMENT == Char)
    {
        ::sw::mark::IFieldmark* pFieldmark = nullptr;
        if (pDoc)
        {
            pFieldmark = pDoc->getIDocumentMarkAccess()->getFieldmarkFor(*pUnoCursor->GetMark());
        }
        SwXTextPortion* pPortion = new SwXTextPortion(
            pUnoCursor, i_xParentText, PORTION_FIELD_START_END);
        xRef = pPortion;
        if (pFieldmark && pDoc)
        {
            pPortion->SetBookmark(
                SwXFieldmark::CreateXFieldmark(*pDoc, pFieldmark));
        }
    }
    else
    {
        OSL_FAIL("no fieldmark found?");
    }
    return xRef;
}

static Reference<XTextRange>
lcl_CreateRefMarkPortion(
    Reference<XText> const& xParent,
    const SwUnoCursor * const pUnoCursor,
    const SwTextAttr & rAttr, const bool bEnd)
{
    SwDoc* pDoc = pUnoCursor->GetDoc();
    SwFormatRefMark& rRefMark = const_cast<SwFormatRefMark&>(
            static_cast<const SwFormatRefMark&>(rAttr.GetAttr()));
    Reference<XTextContent> xContent;
    if (!xContent.is())
    {
        xContent = SwXReferenceMark::CreateXReferenceMark(*pDoc, &rRefMark);
    }

    SwXTextPortion* pPortion = nullptr;
    if (!bEnd)
    {
        pPortion = new SwXTextPortion(pUnoCursor, xParent, PORTION_REFMARK_START);
        pPortion->SetRefMark(xContent);
        pPortion->SetCollapsed(rAttr.End() == nullptr);
    }
    else
    {
        pPortion = new SwXTextPortion(pUnoCursor, xParent, PORTION_REFMARK_END);
        pPortion->SetRefMark(xContent);
    }
    return pPortion;
}

static void
lcl_InsertRubyPortion(
    TextRangeList_t & rPortions,
    Reference<XText> const& xParent,
    const SwUnoCursor * const pUnoCursor,
    const SwTextAttr & rAttr, const bool bEnd)
{
    SwXTextPortion* pPortion = new SwXTextPortion(pUnoCursor,
            static_txtattr_cast<const SwTextRuby&>(rAttr), xParent, bEnd);
    rPortions.emplace_back(pPortion);
    pPortion->SetCollapsed(rAttr.End() == nullptr);
}

static Reference<XTextRange>
lcl_CreateTOXMarkPortion(
    Reference<XText> const& xParent,
    const SwUnoCursor * const pUnoCursor,
    SwTextAttr & rAttr, const bool bEnd)
{
    SwDoc* pDoc = pUnoCursor->GetDoc();
    SwTOXMark & rTOXMark = static_cast<SwTOXMark&>(rAttr.GetAttr());

    const Reference<XTextContent> xContent(
        SwXDocumentIndexMark::CreateXDocumentIndexMark(*pDoc, & rTOXMark),
        uno::UNO_QUERY);

    SwXTextPortion* pPortion = nullptr;
    if (!bEnd)
    {
        pPortion = new SwXTextPortion(pUnoCursor, xParent, PORTION_TOXMARK_START);
        pPortion->SetTOXMark(xContent);
        pPortion->SetCollapsed(rAttr.GetEnd() == nullptr);
    }
    else
    {
        pPortion = new SwXTextPortion(pUnoCursor, xParent, PORTION_TOXMARK_END);
        pPortion->SetTOXMark(xContent);
    }
    return pPortion;
}

static uno::Reference<text::XTextRange>
lcl_CreateMetaPortion(
    uno::Reference<text::XText> const& xParent,
    const SwUnoCursor * const pUnoCursor,
    SwTextAttr & rAttr, std::unique_ptr<TextRangeList_t const> && pPortions)
{
    const uno::Reference<rdf::XMetadatable> xMeta( SwXMeta::CreateXMeta(
            *static_cast<SwFormatMeta &>(rAttr.GetAttr()).GetMeta(),
            xParent, std::move(pPortions)));
    SwXTextPortion * pPortion(nullptr);
    if (RES_TXTATR_META == rAttr.Which())
    {
        const uno::Reference<text::XTextContent> xContent(xMeta,
                uno::UNO_QUERY);
        pPortion = new SwXTextPortion(pUnoCursor, xParent, PORTION_META);
        pPortion->SetMeta(xContent);
    }
    else
    {
        const uno::Reference<text::XTextField> xField(xMeta, uno::UNO_QUERY);
        pPortion = new SwXTextPortion(pUnoCursor, xParent, PORTION_FIELD);
        pPortion->SetTextField(xField);
    }
    return pPortion;
}

/**
 * Exports all bookmarks from rBkmArr into rPortions that have the same start
 * or end position as nIndex.
 *
 * @param rBkmArr the array of bookmarks. If bOnlyFrameStarts is true, then
 * this is only read, otherwise consumed entries are removed.
 *
 * @param rFramePositions the list of positions where there is an at-char /
 * anchored frame.
 *
 * @param bOnlyFrameStarts If true: export only the start of the bookmarks
 * which cover an at-char anchored frame. If false: export the end of the same
 * bookmarks and everything else.
 */
static void lcl_ExportBookmark(
    TextRangeList_t & rPortions,
    Reference<XText> const& xParent,
    const SwUnoCursor * const pUnoCursor,
    SwXBookmarkPortion_ImplList& rBkmArr,
    const sal_Int32 nIndex,
    const std::set<sal_Int32>& rFramePositions,
    bool bOnlyFrameStarts)
{
    for ( SwXBookmarkPortion_ImplList::iterator aIter = rBkmArr.begin(), aEnd = rBkmArr.end(); aIter != aEnd; )
    {
        const SwXBookmarkPortion_ImplSharedPtr& pPtr = *aIter;
        if ( nIndex > pPtr->getIndex() )
        {
            if (bOnlyFrameStarts)
                ++aIter;
            else
                aIter = rBkmArr.erase(aIter);
            continue;
        }
        if ( nIndex < pPtr->getIndex() )
            break;

        if ((BkmType::Start == pPtr->nBkmType && bOnlyFrameStarts) ||
            (BkmType::StartEnd == pPtr->nBkmType))
        {
            bool bFrameStart = rFramePositions.find(nIndex) != rFramePositions.end();
            bool bEnd = pPtr->nBkmType == BkmType::StartEnd && bFrameStart && !bOnlyFrameStarts;
            if (pPtr->nBkmType == BkmType::Start || bFrameStart || !bOnlyFrameStarts)
            {
                // At this we create a text portion, due to one of these
                // reasons:
                // - this is the real start of a non-collapsed bookmark
                // - this is the real position of a collapsed bookmark
                // - this is the start or end (depending on bOnlyFrameStarts)
                //   of a collapsed bookmark at the same position as an at-char
                //   anchored frame
                SwXTextPortion* pPortion =
                    new SwXTextPortion(pUnoCursor, xParent, bEnd ? PORTION_BOOKMARK_END : PORTION_BOOKMARK_START);
                rPortions.emplace_back(pPortion);
                pPortion->SetBookmark(pPtr->xBookmark);
                pPortion->SetCollapsed( BkmType::StartEnd == pPtr->nBkmType && !bFrameStart );
            }
        }
        else if (BkmType::End == pPtr->nBkmType && !bOnlyFrameStarts)
        {
            SwXTextPortion* pPortion =
                new SwXTextPortion(pUnoCursor, xParent, PORTION_BOOKMARK_END);
            rPortions.emplace_back(pPortion);
            pPortion->SetBookmark(pPtr->xBookmark);
        }

        // next bookmark
        if (bOnlyFrameStarts)
            ++aIter;
        else
            aIter = rBkmArr.erase(aIter);
    }
}

static void lcl_ExportSoftPageBreak(
    TextRangeList_t & rPortions,
    Reference<XText> const& xParent,
    const SwUnoCursor * const pUnoCursor,
    SwSoftPageBreakList& rBreakArr,
    const sal_Int32 nIndex)
{
    for ( SwSoftPageBreakList::iterator aIter = rBreakArr.begin(),
          aEnd = rBreakArr.end();
          aIter != aEnd; )
    {
        if ( nIndex > *aIter )
        {
            aIter = rBreakArr.erase(aIter);
            continue;
        }
        if ( nIndex < *aIter )
            break;

        rPortions.push_back(
            new SwXTextPortion(pUnoCursor, xParent, PORTION_SOFT_PAGEBREAK) );
        aIter = rBreakArr.erase(aIter);
    }
}

struct SwXRedlinePortion_Impl
{
    const SwRangeRedline*    m_pRedline;
    const bool          m_bStart;

    SwXRedlinePortion_Impl ( const SwRangeRedline* pRed, const bool bIsStart )
    : m_pRedline(pRed)
    , m_bStart(bIsStart)
    {
    }

    sal_Int32 getRealIndex ()
    {
        return m_bStart ? m_pRedline->Start()->nContent.GetIndex()
                        : m_pRedline->End()  ->nContent.GetIndex();
    }
};

typedef std::shared_ptr < SwXRedlinePortion_Impl >
    SwXRedlinePortion_ImplSharedPtr;

struct RedlineCompareStruct
{
    static const SwPosition& getPosition ( const SwXRedlinePortion_ImplSharedPtr &r )
    {
        return *(r->m_bStart ? r->m_pRedline->Start() : r->m_pRedline->End());
    }

    bool operator () ( const SwXRedlinePortion_ImplSharedPtr &r1,
                       const SwXRedlinePortion_ImplSharedPtr &r2 ) const
    {
        return getPosition ( r1 ) < getPosition ( r2 );
    }
};

typedef std::multiset < SwXRedlinePortion_ImplSharedPtr, RedlineCompareStruct >
SwXRedlinePortion_ImplList;

static Reference<XTextRange>
lcl_ExportHints(
    PortionStack_t & rPortionStack,
    const Reference<XText> & xParent,
    SwUnoCursor * const pUnoCursor,
    SwpHints const * const pHints,
    const sal_Int32 i_nStartPos,
    const sal_Int32 i_nEndPos,
    const sal_Int32 nCurrentIndex,
    const bool bRightMoveForbidden,
    bool & o_rbCursorMoved,
    sal_Int32 & o_rNextAttrPosition)
{
    // if the attribute has a dummy character, then xRef is set (except META)
    // otherwise, the portion for the attribute is inserted into rPortions!
    Reference<XTextRange> xRef;
    SwDoc* pDoc = pUnoCursor->GetDoc();
    //search for special text attributes - first some ends
    size_t nEndIndex = 0;
    sal_Int32 nNextEnd = 0;
    while(nEndIndex < pHints->Count() &&
        (!pHints->GetSortedByEnd(nEndIndex)->GetEnd() ||
        nCurrentIndex >= (nNextEnd = (*pHints->GetSortedByEnd(nEndIndex)->GetEnd()))))
    {
        if(pHints->GetSortedByEnd(nEndIndex)->GetEnd())
        {
            SwTextAttr * const pAttr = pHints->GetSortedByEnd(nEndIndex);
            if (nNextEnd == nCurrentIndex)
            {
                const sal_uInt16 nWhich( pAttr->Which() );
                switch (nWhich)
                {
                    case RES_TXTATR_TOXMARK:
                    {
                        Reference<XTextRange> xTmp = lcl_CreateTOXMarkPortion(
                                xParent, pUnoCursor, *pAttr, true);
                        rPortionStack.top().first->push_back(xTmp);
                    }
                    break;
                    case RES_TXTATR_REFMARK:
                    {
                        Reference<XTextRange> xTmp = lcl_CreateRefMarkPortion(
                                xParent, pUnoCursor, *pAttr, true);
                        rPortionStack.top().first->push_back(xTmp);
                    }
                    break;
                    case RES_TXTATR_CJK_RUBY:
                       //#i91534# GetEnd() == 0 mixes the order of ruby start/end
                        if( *pAttr->GetEnd() == pAttr->GetStart())
                        {
                            lcl_InsertRubyPortion( *rPortionStack.top().first,
                                    xParent, pUnoCursor, *pAttr, false);
                        }
                        lcl_InsertRubyPortion( *rPortionStack.top().first,
                                xParent, pUnoCursor, *pAttr, true);
                    break;
                    case RES_TXTATR_META:
                    case RES_TXTATR_METAFIELD:
                    {
                        OSL_ENSURE(pAttr->GetStart() != *pAttr->GetEnd(),
                                "empty meta?");
                        if ((i_nStartPos > 0) &&
                            (pAttr->GetStart() < i_nStartPos))
                        {
                            // force skip pAttr and rest of attribute ends
                            // at nCurrentIndex
                            // because they are not contained in the meta pAttr
                            // and the meta pAttr itself is outside selection!
                            // (necessary for SwXMeta::createEnumeration)
                            if (pAttr->GetStart() + 1 == i_nStartPos)
                            {
                                nEndIndex = pHints->Count() - 1;
                            }
                            break;
                        }
                        PortionList_t Top = rPortionStack.top();
                        if (Top.second != pAttr)
                        {
                            OSL_FAIL("ExportHints: stack error" );
                        }
                        else
                        {
                            std::unique_ptr<const TextRangeList_t>
                                pCurrentPortions(Top.first);
                            rPortionStack.pop();
                            const uno::Reference<text::XTextRange> xPortion(
                                lcl_CreateMetaPortion(xParent, pUnoCursor,
                                                      *pAttr, std::move(pCurrentPortions)));
                            rPortionStack.top().first->push_back(xPortion);
                        }
                    }
                    break;
                }
            }
        }
        nEndIndex++;
    }

    // then some starts
    size_t nStartIndex = 0;
    sal_Int32 nNextStart = 0;
    while(nStartIndex < pHints->Count() &&
        nCurrentIndex >= (nNextStart = pHints->Get(nStartIndex)->GetStart()))
    {
        SwTextAttr * const pAttr = pHints->Get(nStartIndex);
        sal_uInt16 nAttrWhich = pAttr->Which();
        if (nNextStart == nCurrentIndex)
        {
            switch( nAttrWhich )
            {
                case RES_TXTATR_FIELD:
                   if(!bRightMoveForbidden)
                    {
                        pUnoCursor->Right(1);
                        if( *pUnoCursor->GetMark() == *pUnoCursor->GetPoint() )
                            break;
                        SwXTextPortion* pPortion;
                        xRef = pPortion =
                            new SwXTextPortion(
                                pUnoCursor, xParent, PORTION_FIELD);
                        Reference<XTextField> const xField =
                            SwXTextField::CreateXTextField(pDoc,
                                    &pAttr->GetFormatField());
                        pPortion->SetTextField(xField);
                    }
                    break;

                case RES_TXTATR_ANNOTATION:
                    if(!bRightMoveForbidden)
                    {
                        pUnoCursor->Right(1);
                        if( *pUnoCursor->GetMark() == *pUnoCursor->GetPoint() )
                            break;

                        const SwTextAnnotationField* pTextAnnotationField = dynamic_cast<const SwTextAnnotationField*>( pAttr );
                        ::sw::mark::IMark* pAnnotationMark = pTextAnnotationField ? pTextAnnotationField->GetAnnotationMark() : nullptr;
                        if ( pAnnotationMark != nullptr )
                        {
                            SwXTextPortion* pPortion = new SwXTextPortion( pUnoCursor, xParent, PORTION_ANNOTATION_END );
                            pPortion->SetBookmark(SwXBookmark::CreateXBookmark(
                                        *pDoc, pAnnotationMark));
                            xRef = pPortion;
                        }
                        else
                        {
                            SwXTextPortion* pPortion = new SwXTextPortion( pUnoCursor, xParent, PORTION_ANNOTATION );
                            Reference<XTextField> xField =
                                SwXTextField::CreateXTextField(pDoc,
                                        &pAttr->GetFormatField());
                            pPortion->SetTextField(xField);
                            xRef = pPortion;
                        }
                    }
                    break;

                case RES_TXTATR_INPUTFIELD:
                    if(!bRightMoveForbidden)
                    {

                        pUnoCursor->Right(
                            pAttr->GetFormatField().GetField()->ExpandField(true, nullptr).getLength() + 2 );
                        if( *pUnoCursor->GetMark() == *pUnoCursor->GetPoint() )
                            break;
                        SwXTextPortion* pPortion =
                            new SwXTextPortion( pUnoCursor, xParent, PORTION_FIELD);
                        xRef = pPortion;
                        Reference<XTextField> xField =
                            SwXTextField::CreateXTextField(pDoc,
                                    &pAttr->GetFormatField());
                        pPortion->SetTextField(xField);
                    }
                    break;

                case RES_TXTATR_FLYCNT:
                    if(!bRightMoveForbidden)
                    {
                        pUnoCursor->Right(1);
                        if( *pUnoCursor->GetMark() == *pUnoCursor->GetPoint() )
                            break; // Robust #i81708# content in covered cells

                        // Do not expose inline anchored textboxes.
                        if (SwTextBoxHelper::isTextBox(pAttr->GetFlyCnt().GetFrameFormat(), RES_FLYFRMFMT))
                            break;

                        pUnoCursor->Exchange();
                        xRef = new SwXTextPortion( pUnoCursor, xParent, PORTION_FRAME);
                    }
                    break;

                case RES_TXTATR_FTN:
                    {
                        if(!bRightMoveForbidden)
                        {
                            pUnoCursor->Right(1);
                            if( *pUnoCursor->GetMark() == *pUnoCursor->GetPoint() )
                                break;
                            SwXTextPortion* pPortion;
                            xRef = pPortion = new SwXTextPortion(
                                pUnoCursor, xParent, PORTION_FOOTNOTE);
                            Reference<XFootnote> xContent =
                                SwXFootnotes::GetObject(*pDoc, pAttr->GetFootnote());
                            pPortion->SetFootnote(xContent);
                        }
                    }
                    break;

                case RES_TXTATR_TOXMARK:
                case RES_TXTATR_REFMARK:
                {
                    bool bIsPoint = !(pAttr->GetEnd());
                    if (!bRightMoveForbidden || !bIsPoint)
                    {
                        if (bIsPoint)
                        {
                            pUnoCursor->Right(1);
                        }
                        Reference<XTextRange> xTmp =
                                (RES_TXTATR_REFMARK == nAttrWhich)
                            ? lcl_CreateRefMarkPortion(
                                xParent, pUnoCursor, *pAttr, false)
                            : lcl_CreateTOXMarkPortion(
                                xParent, pUnoCursor, *pAttr, false);
                        if (bIsPoint) // consume CH_TXTATR!
                        {
                            pUnoCursor->Normalize(false);
                            pUnoCursor->DeleteMark();
                            xRef = xTmp;
                        }
                        else // just insert it
                        {
                            rPortionStack.top().first->push_back(xTmp);
                        }
                    }
                }
                break;
                case RES_TXTATR_CJK_RUBY:
                    //#i91534# GetEnd() == 0 mixes the order of ruby start/end
                    if(pAttr->GetEnd() && (*pAttr->GetEnd() != pAttr->GetStart()))
                    {
                        lcl_InsertRubyPortion( *rPortionStack.top().first,
                            xParent, pUnoCursor, *pAttr, false);
                    }
                break;
                case RES_TXTATR_META:
                case RES_TXTATR_METAFIELD:
                    if (pAttr->GetStart() != *pAttr->GetEnd())
                    {
                        if (!bRightMoveForbidden)
                        {
                            pUnoCursor->Right(1);
                            o_rbCursorMoved = true;
                            // only if the end is included in selection!
                            if ((i_nEndPos < 0) ||
                                (*pAttr->GetEnd() <= i_nEndPos))
                            {
                                rPortionStack.push( std::make_pair(
                                        new TextRangeList_t, pAttr ));
                            }
                        }
                    }
                break;
                case RES_TXTATR_AUTOFMT:
                case RES_TXTATR_INETFMT:
                case RES_TXTATR_CHARFMT:
                break; // these are handled as properties of a "Text" portion
                default:
                    OSL_FAIL("unknown attribute");
                break;
            }
        }
        nStartIndex++;
    }

    if (xRef.is()) // implies that we have moved the cursor
    {
        o_rbCursorMoved = true;
    }
    if (!o_rbCursorMoved)
    {
        // search for attribute changes behind the current cursor position
        // break up at frames, bookmarks, redlines

        nStartIndex = 0;
        nNextStart = 0;
        while(nStartIndex < pHints->Count() &&
            nCurrentIndex >= (nNextStart = pHints->Get(nStartIndex)->GetStart()))
            nStartIndex++;

        nEndIndex = 0;
        nNextEnd = 0;
        while(nEndIndex < pHints->Count() &&
            nCurrentIndex >= (nNextEnd = (*pHints->GetSortedByEnd(nEndIndex)->GetAnyEnd())))
            nEndIndex++;

        sal_Int32 nNextPos =
            ((nNextStart > nCurrentIndex) && (nNextStart < nNextEnd))
            ?   nNextStart  :   nNextEnd;
        if (nNextPos > nCurrentIndex)
        {
            o_rNextAttrPosition = nNextPos;
        }
    }
    return xRef;
}

static void lcl_MoveCursor( SwUnoCursor * const pUnoCursor,
    const sal_Int32 nCurrentIndex,
    const sal_Int32 nNextFrameIndex,
    const sal_Int32 nNextPortionIndex,
    const sal_Int32 nNextAttrIndex,
    const sal_Int32 nNextMarkIndex,
    const sal_Int32 nEndPos )
{
    sal_Int32 nMovePos = pUnoCursor->GetContentNode()->Len();

    if ((nEndPos >= 0) && (nEndPos < nMovePos))
    {
        nMovePos = nEndPos;
    }

    if ((nNextFrameIndex >= 0) && (nNextFrameIndex < nMovePos))
    {
        nMovePos = nNextFrameIndex;
    }

    if ((nNextPortionIndex >= 0) && (nNextPortionIndex < nMovePos))
    {
        nMovePos = nNextPortionIndex;
    }

    if ((nNextAttrIndex >= 0) && (nNextAttrIndex < nMovePos))
    {
        nMovePos = nNextAttrIndex;
    }

    if ((nNextMarkIndex >= 0) && (nNextMarkIndex < nMovePos))
    {
        nMovePos = nNextMarkIndex;
    }

    if (nMovePos > nCurrentIndex)
    {
        pUnoCursor->GetPoint()->nContent = nMovePos;
    }
}

static void lcl_FillRedlineArray(
    SwDoc const & rDoc,
    SwUnoCursor const & rUnoCursor,
    SwXRedlinePortion_ImplList& rRedArr )
{
    const SwRedlineTable& rRedTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
    const size_t nRedTableCount = rRedTable.size();

    if ( nRedTableCount > 0 )
    {
        const SwPosition* pStart = rUnoCursor.GetPoint();
        const SwNodeIndex nOwnNode = pStart->nNode;

        for(size_t nRed = 0; nRed < nRedTableCount; ++nRed)
        {
            const SwRangeRedline* pRedline = rRedTable[nRed];
            const SwPosition* pRedStart = pRedline->Start();
            const SwNodeIndex nRedNode = pRedStart->nNode;
            if ( nOwnNode == nRedNode )
                rRedArr.insert( std::make_shared<SwXRedlinePortion_Impl>(
                    pRedline, true ) );
            if( pRedline->HasMark() && pRedline->End()->nNode == nOwnNode )
                rRedArr.insert( std::make_shared<SwXRedlinePortion_Impl>(
                    pRedline, false ) );
       }
    }
}

static void lcl_FillSoftPageBreakArray(
    SwUnoCursor const & rUnoCursor,
    SwSoftPageBreakList& rBreakArr )
{
    const SwTextNode *pTextNode =
        rUnoCursor.GetPoint()->nNode.GetNode().GetTextNode();
    if( pTextNode )
        pTextNode->fillSoftPageBreakList( rBreakArr );
}

static void lcl_ExportRedline(
    TextRangeList_t & rPortions,
    Reference<XText> const& xParent,
    const SwUnoCursor * const pUnoCursor,
    SwXRedlinePortion_ImplList& rRedlineArr,
    const sal_Int32 nIndex)
{

    // We want this loop to iterate over all red lines in this
    // array. We will only insert the ones with index matches
    for ( SwXRedlinePortion_ImplList::iterator aIter = rRedlineArr.begin(), aEnd = rRedlineArr.end();
          aIter != aEnd; )
    {
        SwXRedlinePortion_ImplSharedPtr pPtr = *aIter;
        sal_Int32 nRealIndex = pPtr->getRealIndex();
        // If there are elements before nIndex, remove them
        if ( nIndex > nRealIndex )
            aIter = rRedlineArr.erase(aIter);
        // If the elements match, and them to the list
        else if ( nIndex == nRealIndex )
        {
            rPortions.push_back( new SwXRedlinePortion(
                        *pPtr->m_pRedline, pUnoCursor, xParent, pPtr->m_bStart));
            aIter = rRedlineArr.erase(aIter);
        }
        // If we've iterated past nIndex, exit the loop
        else
            break;
    }
}

static void lcl_ExportBkmAndRedline(
    TextRangeList_t & rPortions,
    Reference<XText> const & xParent,
    const SwUnoCursor * const pUnoCursor,
    SwXBookmarkPortion_ImplList& rBkmArr,
    SwXRedlinePortion_ImplList& rRedlineArr,
    SwSoftPageBreakList& rBreakArr,
    const sal_Int32 nIndex,
    const std::set<sal_Int32>& rFramePositions,
    bool bOnlyFrameBookmarkStarts)
{
    if (!rBkmArr.empty())
        lcl_ExportBookmark(rPortions, xParent, pUnoCursor, rBkmArr, nIndex, rFramePositions,
                           bOnlyFrameBookmarkStarts);

    if (bOnlyFrameBookmarkStarts)
        // Only exporting the start of some collapsed bookmarks: no export of
        // other arrays.
        return;

    if (!rRedlineArr.empty())
        lcl_ExportRedline(rPortions, xParent, pUnoCursor, rRedlineArr, nIndex);

    if (!rBreakArr.empty())
        lcl_ExportSoftPageBreak(rPortions, xParent, pUnoCursor, rBreakArr, nIndex);
}

static void lcl_ExportAnnotationStarts(
    TextRangeList_t & rPortions,
    Reference<XText> const & xParent,
    const SwUnoCursor * const pUnoCursor,
    SwAnnotationStartPortion_ImplList& rAnnotationStartArr,
    const sal_Int32 nIndex)
{
    for ( SwAnnotationStartPortion_ImplList::iterator aIter = rAnnotationStartArr.begin(), aEnd = rAnnotationStartArr.end();
          aIter != aEnd; )
    {
        SwAnnotationStartPortion_ImplSharedPtr pPtr = *aIter;
        if ( nIndex > pPtr->getIndex() )
        {
            aIter = rAnnotationStartArr.erase(aIter);
            continue;
        }
        if ( pPtr->getIndex() > nIndex )
        {
            break;
        }

        SwXTextPortion* pPortion =
            new SwXTextPortion( pUnoCursor, xParent, PORTION_ANNOTATION );
        pPortion->SetTextField( pPtr->mxAnnotationField );
        rPortions.emplace_back(pPortion);

        aIter = rAnnotationStartArr.erase(aIter);
    }
}

/// Fills character positions from rFrames into rFramePositions.
static void lcl_ExtractFramePositions(FrameClientSortList_t& rFrames, sal_Int32 nCurrentIndex,
                                      std::set<sal_Int32>& rFramePositions)
{
    for (const auto& rFrame : rFrames)
    {
        if (rFrame.nIndex < nCurrentIndex)
            continue;

        if (rFrame.nIndex > nCurrentIndex)
            break;

        const SwModify* pFrame = rFrame.pFrameClient->GetRegisteredIn();
        if (!pFrame)
            continue;

        auto& rFormat = *static_cast<SwFrameFormat*>(const_cast<SwModify*>(pFrame));
        const SwFormatAnchor& rAnchor = rFormat.GetAnchor();
        const SwPosition* pPosition = rAnchor.GetContentAnchor();
        if (!pPosition)
            continue;

        rFramePositions.insert(pPosition->nContent.GetIndex());
    }
}

/**
 * Exports at-char anchored frames.
 *
 * @param i_rFrames the frames for this paragraph, frames at <= i_nCurrentIndex
 * are removed from the container.
 */
static sal_Int32 lcl_ExportFrames(
    TextRangeList_t & rPortions,
    Reference<XText> const & i_xParent,
    SwUnoCursor const * const i_pUnoCursor,
    FrameClientSortList_t & i_rFrames,
    sal_Int32 const i_nCurrentIndex)
{
    // Ignore frames which are not exported, as we are exporting a selection
    // and they are anchored before the start of the selection.
    while (!i_rFrames.empty() && i_rFrames.front().nIndex < i_nCurrentIndex)
        i_rFrames.pop_front();

    // find first Frame in (sorted) i_rFrames at current position
    while (!i_rFrames.empty() && (i_rFrames.front().nIndex == i_nCurrentIndex))
    // do not check for i_nEnd here; this is done implicitly by lcl_MoveCursor
    {
        const SwModify * const pFrame =
            i_rFrames.front().pFrameClient->GetRegisteredIn();
        if (pFrame) // Frame could be disposed
        {
            SwXTextPortion* pPortion = new SwXTextPortion(i_pUnoCursor, i_xParent,
                *static_cast<SwFrameFormat*>( const_cast<SwModify*>( pFrame ) ) );
            rPortions.emplace_back(pPortion);
        }
        i_rFrames.pop_front();
    }

    return !i_rFrames.empty() ? i_rFrames.front().nIndex : -1;
}

static sal_Int32 lcl_GetNextIndex(
    SwXBookmarkPortion_ImplList const & rBkmArr,
    SwXRedlinePortion_ImplList const & rRedlineArr,
    SwSoftPageBreakList const & rBreakArr )
{
    sal_Int32 nRet = -1;
    if(!rBkmArr.empty())
    {
        SwXBookmarkPortion_ImplSharedPtr pPtr = *rBkmArr.begin();
        nRet = pPtr->getIndex();
    }
    if(!rRedlineArr.empty())
    {
        SwXRedlinePortion_ImplSharedPtr pPtr = *rRedlineArr.begin();
        sal_Int32 nTmp = pPtr->getRealIndex();
        if(nRet < 0 || nTmp < nRet)
            nRet = nTmp;
    }
    if(!rBreakArr.empty())
    {
        if(nRet < 0 || *rBreakArr.begin() < nRet)
            nRet = *rBreakArr.begin();
    }
    return nRet;
};

static void lcl_CreatePortions(
        TextRangeList_t & i_rPortions,
        uno::Reference< text::XText > const & i_xParentText,
        SwUnoCursor * const pUnoCursor,
        FrameClientSortList_t & i_rFrames,
        const sal_Int32 i_nStartPos,
        const sal_Int32 i_nEndPos )
{
    if (!pUnoCursor)
        return;

    // set the start if a selection should be exported
    if ((i_nStartPos > 0) &&
        (pUnoCursor->Start()->nContent.GetIndex() != i_nStartPos))
    {
        pUnoCursor->DeleteMark();
        OSL_ENSURE(pUnoCursor->Start()->nNode.GetNode().GetTextNode() &&
            (i_nStartPos <= pUnoCursor->Start()->nNode.GetNode().GetTextNode()->
                        GetText().getLength()), "Incorrect start position" );
        // ??? should this be i_nStartPos - current position ?
        pUnoCursor->Right(i_nStartPos);
    }

    SwDoc * const pDoc = pUnoCursor->GetDoc();

    std::deque<sal_Int32> FieldMarks;
    lcl_FillFieldMarkArray(FieldMarks, *pUnoCursor, i_nStartPos);

    SwXBookmarkPortion_ImplList Bookmarks;
    lcl_FillBookmarkArray(*pDoc, *pUnoCursor, Bookmarks);

    SwXRedlinePortion_ImplList Redlines;
    lcl_FillRedlineArray(*pDoc, *pUnoCursor, Redlines);

    SwSoftPageBreakList SoftPageBreaks;
    lcl_FillSoftPageBreakArray(*pUnoCursor, SoftPageBreaks);

    SwAnnotationStartPortion_ImplList AnnotationStarts;
    lcl_FillAnnotationStartArray( *pDoc, *pUnoCursor, AnnotationStarts );

    PortionStack_t PortionStack;
    PortionStack.push( PortionList_t(&i_rPortions, nullptr) );

    bool bAtEnd( false );
    while (!bAtEnd) // every iteration consumes at least current character!
    {
        if (pUnoCursor->HasMark())
        {
            pUnoCursor->Normalize(false);
            pUnoCursor->DeleteMark();
        }

        SwTextNode * const pTextNode = pUnoCursor->GetNode().GetTextNode();
        if (!pTextNode)
        {
            OSL_FAIL("lcl_CreatePortions: no TextNode - what now ?");
            return;
        }

        SwpHints * const pHints = pTextNode->GetpSwpHints();
        const sal_Int32 nCurrentIndex =
            pUnoCursor->GetPoint()->nContent.GetIndex();
        // this contains the portion which consumes the character in the
        // text at nCurrentIndex; i.e. it must be set _once_ per iteration
        uno::Reference< XTextRange > xRef;

        SwUnoCursorHelper::SelectPam(*pUnoCursor, true); // set mark

        // First remember the frame positions.
        std::set<sal_Int32> aFramePositions;
        lcl_ExtractFramePositions(i_rFrames, nCurrentIndex, aFramePositions);

        // Then export start of collapsed bookmarks which "cover" at-char
        // anchored frames.
        lcl_ExportBkmAndRedline( *PortionStack.top().first, i_xParentText,
            pUnoCursor, Bookmarks, Redlines, SoftPageBreaks, nCurrentIndex, aFramePositions, /*bOnlyFrameBookmarkStarts=*/true );

        const sal_Int32 nFirstFrameIndex =
            lcl_ExportFrames( *PortionStack.top().first,
                i_xParentText, pUnoCursor, i_rFrames, nCurrentIndex);

        // Export ends of the previously started collapsed bookmarks + all
        // other bookmarks, redlines, etc.
        lcl_ExportBkmAndRedline( *PortionStack.top().first, i_xParentText,
            pUnoCursor, Bookmarks, Redlines, SoftPageBreaks, nCurrentIndex, aFramePositions, /*bOnlyFrameBookmarkStarts=*/false );

        lcl_ExportAnnotationStarts(
            *PortionStack.top().first,
            i_xParentText,
            pUnoCursor,
            AnnotationStarts,
            nCurrentIndex );

        bool bCursorMoved( false );
        sal_Int32 nNextAttrIndex = -1;
        // #111716# the cursor must not move right at the
        //          end position of a selection!
        bAtEnd = ((i_nEndPos >= 0) && (nCurrentIndex >= i_nEndPos))
              || (nCurrentIndex >= pTextNode->Len());
        if (pHints)
        {
            // N.B.: side-effects nNextAttrIndex, bCursorMoved; may move cursor
            xRef = lcl_ExportHints(PortionStack, i_xParentText, pUnoCursor,
                        pHints, i_nStartPos, i_nEndPos, nCurrentIndex, bAtEnd,
                        bCursorMoved, nNextAttrIndex);
            if (PortionStack.empty())
            {
                OSL_FAIL("CreatePortions: stack underflow");
                return;
            }
        }

        if (!xRef.is() && !bCursorMoved)
        {
            if (!bAtEnd &&
                !FieldMarks.empty() && (FieldMarks.front() == nCurrentIndex))
            {
                // moves cursor
                xRef = lcl_ExportFieldMark(i_xParentText, pUnoCursor, pTextNode);
                FieldMarks.pop_front();
            }
        }
        else
        {
            OSL_ENSURE(FieldMarks.empty() ||
                   (FieldMarks.front() != nCurrentIndex),
                   "fieldmark and hint with CH_TXTATR at same pos?");
        }

        if (!bAtEnd && !xRef.is() && !bCursorMoved)
        {
            const sal_Int32 nNextPortionIndex =
                lcl_GetNextIndex(Bookmarks, Redlines, SoftPageBreaks);

            sal_Int32 nNextMarkIndex = ( !FieldMarks.empty() ? FieldMarks.front() : -1 );
            if ( !AnnotationStarts.empty()
                 && ( nNextMarkIndex == -1
                      || (*AnnotationStarts.begin())->getIndex() < nNextMarkIndex ) )
            {
                nNextMarkIndex = (*AnnotationStarts.begin())->getIndex();
            }

            lcl_MoveCursor(
                pUnoCursor,
                nCurrentIndex,
                nFirstFrameIndex,
                nNextPortionIndex,
                nNextAttrIndex,
                nNextMarkIndex,
                i_nEndPos );

            xRef = new SwXTextPortion(pUnoCursor, i_xParentText, PORTION_TEXT);
        }
        else if (bAtEnd && !xRef.is() && !pTextNode->Len())
        {
            // special case: for an empty paragraph, we better put out a
            // text portion because there may be a hyperlink attribute
            xRef = new SwXTextPortion(pUnoCursor, i_xParentText, PORTION_TEXT);
        }

        if (xRef.is())
        {
            PortionStack.top().first->push_back(xRef);
        }
    }

    OSL_ENSURE((PortionStack.size() == 1) && !PortionStack.top().second,
            "CreatePortions: stack error" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
