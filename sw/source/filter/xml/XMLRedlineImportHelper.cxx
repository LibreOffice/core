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

#include "XMLRedlineImportHelper.hxx"
#include <unotextcursor.hxx>
#include <unotextrange.hxx>
#include <unocrsr.hxx>
#include "doc.hxx"
#include <IDocumentContentOperations.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <tools/datetime.hxx>
#include "poolfmt.hxx"
#include "unoredline.hxx"
#include <o3tl/any.hxx>
#include <xmloff/xmltoken.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XWordCursor;
using ::com::sun::star::lang::XUnoTunnel;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
// collision with tools/DateTime: use UNO DateTime as util::DateTime
// using util::DateTime;

// a few helper functions
static SwDoc* lcl_GetDocViaTunnel( Reference<XTextCursor> const & rCursor )
{
    Reference<XUnoTunnel> xTunnel( rCursor, UNO_QUERY);
    OSL_ENSURE(xTunnel.is(), "missing XUnoTunnel for XTextCursor");
    OTextCursorHelper *const pXCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xTunnel);
    OSL_ENSURE( pXCursor, "OTextCursorHelper missing" );
    return (pXCursor) ? pXCursor->GetDoc() : nullptr;
}

static SwDoc* lcl_GetDocViaTunnel( Reference<XTextRange> const & rRange )
{
    Reference<XUnoTunnel> xTunnel(rRange, UNO_QUERY);
    OSL_ENSURE(xTunnel.is(), "missing XUnoTunnel for XTextRange");
    SwXTextRange *const pXRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xTunnel);
    // #i115174#: this may be a SvxUnoTextRange
    // OSL_ENSURE( pXRange, "SwXTextRange missing" );
    return (pXRange) ? &pXRange->GetDoc() : nullptr;
}

// XTextRangeOrNodeIndexPosition: store a position into the text
// *either* as an XTextRange or as an SwNodeIndex. The reason is that
// we must store either pointers to StartNodes (because redlines may
// start on start nodes) or to a text position, and there appears to
// be no existing type that could do both. Things are complicated by
// the matter that (e.g in section import) we delete a few characters,
// which may cause bookmarks (as used by XTextRange) to be deleted.

class XTextRangeOrNodeIndexPosition
{
    Reference<XTextRange> xRange;
    std::unique_ptr<SwNodeIndex> pIndex;    // pIndex will point to the *previous* node

public:
    XTextRangeOrNodeIndexPosition();

    void Set( Reference<XTextRange> & rRange );
    void Set( SwNodeIndex& rIndex );
    void SetAsNodeIndex( Reference<XTextRange> & rRange );

    void CopyPositionInto(SwPosition& rPos, SwDoc & rDoc);
    SwDoc* GetDoc();

    bool IsValid();
};

XTextRangeOrNodeIndexPosition::XTextRangeOrNodeIndexPosition() :
    xRange(nullptr),
    pIndex(nullptr)
{
}

void XTextRangeOrNodeIndexPosition::Set( Reference<XTextRange> & rRange )
{
    xRange = rRange->getStart();    // set bookmark
    pIndex.reset();
}

void XTextRangeOrNodeIndexPosition::Set( SwNodeIndex& rIndex )
{
    pIndex.reset( new SwNodeIndex(rIndex) );
    (*pIndex)-- ;   // previous node!!!
    xRange = nullptr;
}

void XTextRangeOrNodeIndexPosition::SetAsNodeIndex(
    Reference<XTextRange> & rRange )
{
    // XTextRange -> XTunnel -> SwXTextRange
    SwDoc* pDoc = lcl_GetDocViaTunnel(rRange);

    if (!pDoc)
    {
        SAL_WARN("sw", "no SwDoc");
        return;
    }

    // SwXTextRange -> PaM
    SwUnoInternalPaM aPaM(*pDoc);
    bool bSuccess = ::sw::XTextRangeToSwPaM(aPaM, rRange);
    OSL_ENSURE(bSuccess, "illegal range");

    // PaM -> Index
    Set(aPaM.GetPoint()->nNode);
}

void
XTextRangeOrNodeIndexPosition::CopyPositionInto(SwPosition& rPos, SwDoc & rDoc)
{
    OSL_ENSURE(IsValid(), "Can't get Position");

    // create PAM from start cursor (if no node index is present)
    if (nullptr == pIndex)
    {
        SwUnoInternalPaM aUnoPaM(rDoc);
        bool bSuccess = ::sw::XTextRangeToSwPaM(aUnoPaM, xRange);
        OSL_ENSURE(bSuccess, "illegal range");

        rPos = *aUnoPaM.GetPoint();
    }
    else
    {
        rPos.nNode = *pIndex;
        rPos.nNode++;           // pIndex points to previous index !!!
        rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(), 0 );
    }
}

SwDoc* XTextRangeOrNodeIndexPosition::GetDoc()
{
    OSL_ENSURE(IsValid(), "Can't get Doc");

    return (nullptr != pIndex) ? pIndex->GetNodes().GetDoc() : lcl_GetDocViaTunnel(xRange);
}

bool XTextRangeOrNodeIndexPosition::IsValid()
{
    return ( xRange.is() || (pIndex != nullptr) );
}

// RedlineInfo: temporary storage for redline data
class RedlineInfo
{
public:
    RedlineInfo();
    ~RedlineInfo();

    // redline type (insert, delete, ...)
    RedlineType_t eType;

    // info fields:
    OUString sAuthor;               // change author string
    OUString sComment;              // change comment string
    util::DateTime aDateTime;       // change DateTime
    bool bMergeLastParagraph;   // the SwRangeRedline::IsDelLastPara flag

    // each position can may be either empty, an XTextRange, or an SwNodeIndex

    // start pos of anchor (may be empty)
    XTextRangeOrNodeIndexPosition aAnchorStart;

    // end pos of anchor (may be empty)
    XTextRangeOrNodeIndexPosition aAnchorEnd;

    // index of content node (maybe NULL)
    SwNodeIndex* pContentIndex;

    // next redline info (for hierarchical redlines)
    RedlineInfo* pNextRedline;

    // store whether we expect an adjustment for this redline
    bool bNeedsAdjustment;
};

RedlineInfo::RedlineInfo() :
    eType(nsRedlineType_t::REDLINE_INSERT),
    sAuthor(),
    sComment(),
    aDateTime(),
    bMergeLastParagraph( false ),
    aAnchorStart(),
    aAnchorEnd(),
    pContentIndex(nullptr),
    pNextRedline(nullptr),
    bNeedsAdjustment( false )
{
}

RedlineInfo::~RedlineInfo()
{
    delete pContentIndex;
    delete pNextRedline;
}

static const char g_sShowChanges[] = "ShowChanges";
static const char g_sRecordChanges[] = "RecordChanges";
static const char g_sRedlineProtectionKey[] = "RedlineProtectionKey";

XMLRedlineImportHelper::XMLRedlineImportHelper(
    bool bNoRedlinesPlease,
    const Reference<XPropertySet> & rModel,
    const Reference<XPropertySet> & rImportInfo ) :
        sInsertion( GetXMLToken( XML_INSERTION )),
        sDeletion( GetXMLToken( XML_DELETION )),
        sFormatChange( GetXMLToken( XML_FORMAT_CHANGE )),
        aRedlineMap(),
        bIgnoreRedlines(bNoRedlinesPlease),
        xModelPropertySet(rModel),
        xImportInfoPropertySet(rImportInfo)
{
    // check to see if redline mode is handled outside of component
    bool bHandleShowChanges = true;
    bool bHandleRecordChanges = true;
    bool bHandleProtectionKey = true;
    if ( xImportInfoPropertySet.is() )
    {
        Reference<XPropertySetInfo> xInfo =
            xImportInfoPropertySet->getPropertySetInfo();

        bHandleShowChanges = ! xInfo->hasPropertyByName( g_sShowChanges );
        bHandleRecordChanges = ! xInfo->hasPropertyByName( g_sRecordChanges );
        bHandleProtectionKey = ! xInfo->hasPropertyByName( g_sRedlineProtectionKey );
    }

    // get redline mode
    bShowChanges = *o3tl::doAccess<bool>(
        ( bHandleShowChanges ? xModelPropertySet : xImportInfoPropertySet )
        ->getPropertyValue( g_sShowChanges ));
    bRecordChanges = *o3tl::doAccess<bool>(
        ( bHandleRecordChanges ? xModelPropertySet : xImportInfoPropertySet )
        ->getPropertyValue( g_sRecordChanges ));
    {
        Any aAny = (bHandleProtectionKey  ? xModelPropertySet
                                          : xImportInfoPropertySet )
                        ->getPropertyValue( g_sRedlineProtectionKey );
        aAny >>= aProtectionKey;
    }

    // set redline mode to "don't record changes"
    if( bHandleRecordChanges )
    {
        xModelPropertySet->setPropertyValue( g_sRecordChanges, makeAny(false) );
    }
}

XMLRedlineImportHelper::~XMLRedlineImportHelper()
{
    // delete all left over (and obviously incomplete) RedlineInfos (and map)
    RedlineMapType::iterator aFind = aRedlineMap.begin();
    for( ; aRedlineMap.end() != aFind; ++aFind )
    {
        RedlineInfo* pInfo = aFind->second;

        // left-over redlines. Insert them if possible (but assert),
        // and delete the incomplete ones. Finally, delete it.
        if( IsReady(pInfo) )
        {
            OSL_FAIL("forgotten RedlineInfo; now inserted");
            InsertIntoDocument( pInfo );
        }
        else
        {
            // try if only the adjustment was missing
            pInfo->bNeedsAdjustment = false;
            if( IsReady(pInfo) )
            {
                OSL_FAIL("RedlineInfo without adjustment; now inserted");
                InsertIntoDocument( pInfo );
            }
            else
            {
                // this situation occurs if redlines aren't closed
                // (i.e. end without start, or start without
                // end). This may well be a problem in the file,
                // rather than the code.
                OSL_FAIL("incomplete redline (maybe file was corrupt); "
                          "now deleted");
            }
        }
        delete pInfo;
    }
    aRedlineMap.clear();

    // set redline mode, either to info property set, or directly to
    // the document
    bool bHandleShowChanges = true;
    bool bHandleRecordChanges = true;
    bool bHandleProtectionKey = true;
    if ( xImportInfoPropertySet.is() )
    {
        Reference<XPropertySetInfo> xInfo =
            xImportInfoPropertySet->getPropertySetInfo();

        bHandleShowChanges = ! xInfo->hasPropertyByName( g_sShowChanges );
        bHandleRecordChanges = ! xInfo->hasPropertyByName( g_sRecordChanges );
        bHandleProtectionKey = ! xInfo->hasPropertyByName( g_sRedlineProtectionKey );
    }

    // set redline mode & key
    try
    {
        Any aAny;

        aAny <<= bShowChanges;
        if ( bHandleShowChanges )
            xModelPropertySet->setPropertyValue( g_sShowChanges, aAny );
        else
            xImportInfoPropertySet->setPropertyValue( g_sShowChanges, aAny );

        aAny <<= bRecordChanges;
        if ( bHandleRecordChanges )
            xModelPropertySet->setPropertyValue( g_sRecordChanges, aAny );
        else
            xImportInfoPropertySet->setPropertyValue( g_sRecordChanges, aAny );

        aAny <<= aProtectionKey;
        if ( bHandleProtectionKey )
            xModelPropertySet->setPropertyValue( g_sRedlineProtectionKey, aAny );
        else
            xImportInfoPropertySet->setPropertyValue( g_sRedlineProtectionKey, aAny);
    }
    catch (const uno::RuntimeException &) // fdo#65882
    {
        SAL_WARN( "sw", "potentially benign ordering issue during shutdown" );
    }
}

void XMLRedlineImportHelper::Add(
    const OUString& rType,
    const OUString& rId,
    const OUString& rAuthor,
    const OUString& rComment,
    const util::DateTime& rDateTime,
    bool bMergeLastPara)
{
    // we need to do the following:
    // 1) parse type string
    // 2) create RedlineInfo and fill it with data
    // 3) check for existing redline with same ID
    // 3a) insert redline into map
    // 3b) attach to existing redline

    // ad 1)
    RedlineType_t eType;
    if (rType.equals(sInsertion))
    {
        eType = nsRedlineType_t::REDLINE_INSERT;
    }
    else if (rType.equals(sDeletion))
    {
        eType = nsRedlineType_t::REDLINE_DELETE;
    }
    else if (rType.equals(sFormatChange))
    {
        eType = nsRedlineType_t::REDLINE_FORMAT;
    }
    else
    {
        // no proper type found: early out!
        return;
    }

    // ad 2) create a new RedlineInfo
    RedlineInfo* pInfo = new RedlineInfo();

    // fill entries
    pInfo->eType = eType;
    pInfo->sAuthor = rAuthor;
    pInfo->sComment = rComment;
    pInfo->aDateTime = rDateTime;
    pInfo->bMergeLastParagraph = bMergeLastPara;

    // ad 3)
    if (aRedlineMap.end() == aRedlineMap.find(rId))
    {
        // 3a) insert into map
        aRedlineMap[rId] = pInfo;
    }
    else
    {
        // 3b) we already have a redline with this name: hierarchical redlines
        // insert pInfo as last element in the chain.
        // (hierarchy sanity checking happens on inserting into the document)

        // find last element
        RedlineInfo* pInfoChain;
        for( pInfoChain = aRedlineMap[rId];
            nullptr != pInfoChain->pNextRedline;
            pInfoChain = pInfoChain->pNextRedline) ; // empty loop

        // insert as last element
        pInfoChain->pNextRedline = pInfo;
    }
}

Reference<XTextCursor> XMLRedlineImportHelper::CreateRedlineTextSection(
    Reference<XTextCursor> const & xOldCursor,
    const OUString& rId)
{
    Reference<XTextCursor> xReturn;

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    // get RedlineInfo
    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        // get document from old cursor (via tunnel)
        SwDoc* pDoc = lcl_GetDocViaTunnel(xOldCursor);

        if (!pDoc)
        {
            SAL_WARN("sw", "no SwDoc => cannot create section.");
            return nullptr;
        }

        // create text section for redline
        SwTextFormatColl *pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool
            (RES_POOLCOLL_STANDARD, false );
        SwStartNode* pRedlineNode = pDoc->GetNodes().MakeTextSection(
            pDoc->GetNodes().GetEndOfRedlines(),
            SwNormalStartNode,
            pColl);

        // remember node-index in RedlineInfo
        SwNodeIndex aIndex(*pRedlineNode);
        aFind->second->pContentIndex = new SwNodeIndex(aIndex);

        // create XText for document
        SwXText* pXText = new SwXRedlineText(pDoc, aIndex);
        Reference<XText> xText = pXText;  // keep Reference until end of method

        // create (UNO-) cursor
        SwPosition aPos(*pRedlineNode);
        SwXTextCursor *const pXCursor =
            new SwXTextCursor(*pDoc, pXText, CursorType::Redline, aPos);
        pXCursor->GetCursor().Move(fnMoveForward, GoInNode);
        // cast to avoid ambiguity
        xReturn = static_cast<text::XWordCursor*>(pXCursor);
    }
    // else: unknown redline -> Ignore

    return xReturn;
}

void XMLRedlineImportHelper::SetCursor(
    const OUString& rId,
    bool bStart,
    Reference<XTextRange> & rRange,
    bool bIsOutsideOfParagraph)
{
    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        // RedlineInfo found; now set Cursor
        RedlineInfo* pInfo = aFind->second;
        if (bIsOutsideOfParagraph)
        {
            // outside of paragraph: remember SwNodeIndex
            if (bStart)
            {
                pInfo->aAnchorStart.SetAsNodeIndex(rRange);
            }
            else
            {
                pInfo->aAnchorEnd.SetAsNodeIndex(rRange);
            }

            // also remember that we expect an adjustment for this redline
            pInfo->bNeedsAdjustment = true;
        }
        else
        {
            // inside of a paragraph: use regular XTextRanges (bookmarks)
            if (bStart)
                pInfo->aAnchorStart.Set(rRange);
            else
                pInfo->aAnchorEnd.Set(rRange);
        }

        // if this Cursor was the last missing info, we insert the
        // node into the document
        // then we can remove the entry from the map and destroy the object
        if (IsReady(pInfo))
        {
            InsertIntoDocument(pInfo);
            aRedlineMap.erase(rId);
            delete pInfo;
        }
    }
    // else: unknown Id -> ignore
}

void XMLRedlineImportHelper::AdjustStartNodeCursor(
    const OUString& rId,        /// ID used in RedlineAdd() call
    bool /*bStart*/,
    Reference<XTextRange> & /*rRange*/)
{
    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    // start + end nodes are treated the same. For either it's
    // necessary that the target node already exists.

    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        // RedlineInfo found; now set Cursor
        RedlineInfo* pInfo = aFind->second;

        pInfo->bNeedsAdjustment = false;

        // if now ready, insert into document
        if( IsReady(pInfo) )
        {
            InsertIntoDocument(pInfo);
            aRedlineMap.erase(rId);
            delete pInfo;
        }
    }
    // else: can't find redline -> ignore
}

inline bool XMLRedlineImportHelper::IsReady(RedlineInfo* pRedline)
{
    // we can insert a redline if we have start & end, and we don't
    // expect adjustments for either of these
    return ( pRedline->aAnchorEnd.IsValid() &&
             pRedline->aAnchorStart.IsValid() &&
             !pRedline->bNeedsAdjustment );
}

void XMLRedlineImportHelper::InsertIntoDocument(RedlineInfo* pRedlineInfo)
{
    OSL_ENSURE(nullptr != pRedlineInfo, "need redline info");
    OSL_ENSURE(IsReady(pRedlineInfo), "redline info not complete yet!");

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    // Insert the Redline as described by pRedlineInfo into the
    // document.  If we are in insert mode, don't insert any redlines
    // (and delete 'deleted' inline redlines)

    // get the document (from one of the positions)
    SwDoc* pDoc = pRedlineInfo->aAnchorStart.GetDoc();

    if (!pDoc)
    {
        SAL_WARN("sw", "no SwDoc => cannot insert redline.");
        return;
    }

    // now create the PaM for the redline
    SwPaM aPaM(pDoc->GetNodes().GetEndOfContent());
    pRedlineInfo->aAnchorStart.CopyPositionInto(*aPaM.GetPoint(), *pDoc);
    aPaM.SetMark();
    pRedlineInfo->aAnchorEnd.CopyPositionInto(*aPaM.GetPoint(), *pDoc);

    // collapse PaM if (start == end)
    if (*aPaM.GetPoint() == *aPaM.GetMark())
    {
        aPaM.DeleteMark();
    }

    // cover three cases:
    // 1) empty redlines (no range, no content)
    // 2) check for:
    //    a) bIgnoreRedline (e.g. insert mode)
    //    b) illegal PaM range (CheckNodesRange())
    // 3) normal case: insert redline
    if( !aPaM.HasMark() && (pRedlineInfo->pContentIndex == nullptr) )
    {
        // these redlines have no function, and will thus be ignored (just as
        // in sw3io), so no action here
    }
    else if ( bIgnoreRedlines ||
         !CheckNodesRange( aPaM.GetPoint()->nNode,
                           aPaM.GetMark()->nNode,
                           true ) )
    {
        // ignore redline (e.g. file loaded in insert mode):
        // delete 'deleted' redlines and forget about the whole thing
        if (nsRedlineType_t::REDLINE_DELETE == pRedlineInfo->eType)
        {
            pDoc->getIDocumentContentOperations().DeleteRange(aPaM);
            // And what about the "deleted nodes"?
            // They have to be deleted as well (#i80689)!
            if( bIgnoreRedlines && pRedlineInfo->pContentIndex != nullptr )
            {
                SwNodeIndex aIdx( *pRedlineInfo->pContentIndex );
                const SwNode* pEnd = aIdx.GetNode().EndOfSectionNode();
                if( pEnd )
                {
                    SwNodeIndex aEnd( *pEnd, 1 );
                    SwPaM aDel( aIdx, aEnd );
                    pDoc->getIDocumentContentOperations().DeleteRange(aDel);
                }
            }
        }
    }
    else
    {
        // regular file loading: insert redline

        // create redline (using pRedlineData which gets copied in SwRangeRedline())
        SwRedlineData* pRedlineData = ConvertRedline(pRedlineInfo, pDoc);
        SwRangeRedline* pRedline =
            new SwRangeRedline( pRedlineData, *aPaM.GetPoint(),
                           !pRedlineInfo->bMergeLastParagraph );

        // set mark
        if( aPaM.HasMark() )
        {
            pRedline->SetMark();
            *(pRedline->GetMark()) = *aPaM.GetMark();
        }

        // set content node (if necessary)
        if (nullptr != pRedlineInfo->pContentIndex)
        {
            sal_uLong nPoint = aPaM.GetPoint()->nNode.GetIndex();
            if( nPoint < pRedlineInfo->pContentIndex->GetIndex() ||
                nPoint > pRedlineInfo->pContentIndex->GetNode().EndOfSectionIndex() )
                pRedline->SetContentIdx(pRedlineInfo->pContentIndex);
#if OSL_DEBUG_LEVEL > 1
            else
                OSL_FAIL( "Recursive change tracking" );
#endif
        }

        // set redline mode (without doing the associated book-keeping)
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(RedlineFlags::On);
        pDoc->getIDocumentRedlineAccess().AppendRedline(pRedline, false);
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(RedlineFlags::NONE);
    }
}

SwRedlineData* XMLRedlineImportHelper::ConvertRedline(
    RedlineInfo* pRedlineInfo,
    SwDoc* pDoc)
{
    // convert info:
    // 1) Author String -> Author ID (default to zero)
    sal_uInt16 nAuthorId = (nullptr == pDoc) ? 0 :
        pDoc->getIDocumentRedlineAccess().InsertRedlineAuthor( pRedlineInfo->sAuthor );

    // 2) util::DateTime -> DateTime
    DateTime aDT( DateTime::EMPTY );
    aDT.SetYear(    pRedlineInfo->aDateTime.Year );
    aDT.SetMonth(   pRedlineInfo->aDateTime.Month );
    aDT.SetDay(     pRedlineInfo->aDateTime.Day );
    aDT.SetHour(    pRedlineInfo->aDateTime.Hours );
    aDT.SetMin(     pRedlineInfo->aDateTime.Minutes );
    aDT.SetSec(     pRedlineInfo->aDateTime.Seconds );
    aDT.SetNanoSec( pRedlineInfo->aDateTime.NanoSeconds );

    // 3) recursively convert next redline
    //    ( check presence and sanity of hierarchical redline info )
    SwRedlineData* pNext = nullptr;
    if ( (nullptr != pRedlineInfo->pNextRedline) &&
         (nsRedlineType_t::REDLINE_DELETE == pRedlineInfo->eType) &&
         (nsRedlineType_t::REDLINE_INSERT == pRedlineInfo->pNextRedline->eType) )
    {
        pNext = ConvertRedline(pRedlineInfo->pNextRedline, pDoc);
    }

    // create redline data
    SwRedlineData* pData = new SwRedlineData(pRedlineInfo->eType,
                                             nAuthorId, aDT,
                                             pRedlineInfo->sComment,
                                             pNext); // next data (if available)

    return pData;
}

void XMLRedlineImportHelper::SetShowChanges( bool bShow )
{
    bShowChanges = bShow;
}

void XMLRedlineImportHelper::SetRecordChanges( bool bRecord )
{
    bRecordChanges = bRecord;
}

void XMLRedlineImportHelper::SetProtectionKey(
    const Sequence<sal_Int8> & rKey )
{
    aProtectionKey = rKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
