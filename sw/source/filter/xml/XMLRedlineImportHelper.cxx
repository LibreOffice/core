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

#include "XMLRedlineImportHelper.hxx"
#include <unotextcursor.hxx>
#include <unotextrange.hxx>
#include <unocrsr.hxx>
#include "doc.hxx"
#include <tools/datetime.hxx>
#include "poolfmt.hxx"
#include "unoredline.hxx"
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/frame/XModel.hpp>

// for locking SolarMutex: svapp + mutex
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XWordCursor;
using ::com::sun::star::lang::XUnoTunnel;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
// collision with tools/DateTime: use UNO DateTime as util::DateTime
// using util::DateTime;


//
// a few helper functions
//

static SwDoc* lcl_GetDocViaTunnel( Reference<XTextCursor> & rCursor )
{
    Reference<XUnoTunnel> xTunnel( rCursor, UNO_QUERY);
    OSL_ENSURE(xTunnel.is(), "missing XUnoTunnel for XTextCursor");
    OTextCursorHelper *const pXCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xTunnel);
    OSL_ENSURE( pXCursor, "OTextCursorHelper missing" );
    return (pXCursor) ? pXCursor->GetDoc() : 0;
}

static SwDoc* lcl_GetDocViaTunnel( Reference<XTextRange> & rRange )
{
    Reference<XUnoTunnel> xTunnel(rRange, UNO_QUERY);
    OSL_ENSURE(xTunnel.is(), "missing XUnoTunnel for XTextRange");
    SwXTextRange *const pXRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xTunnel);
    // #i115174#: this may be a SvxUnoTextRange
//    OSL_ENSURE( pXRange, "SwXTextRange missing" );
    return (pXRange) ? pXRange->GetDoc() : 0;
}


//
// XTextRangeOrNodeIndexPosition: store a position into the text
// *either* as an XTextRange or as an SwNodeIndex. The reason is that
// we must store either pointers to StartNodes (because redlines may
// start on start nodes) or to a text position, and there appears to
// be no existing type that could do both. Things are complicated by
// the matter that (e.g in section import) we delete a few characters,
// which may cause bookmarks (as used by XTextRange) to be deleted.
//

class XTextRangeOrNodeIndexPosition
{
    Reference<XTextRange> xRange;
    SwNodeIndex* pIndex;    /// pIndex will point to the *previous* node

public:
    XTextRangeOrNodeIndexPosition();
    ~XTextRangeOrNodeIndexPosition();

    void Set( Reference<XTextRange> & rRange );
    void Set( SwNodeIndex& rIndex );
    void SetAsNodeIndex( Reference<XTextRange> & rRange );

    void CopyPositionInto(SwPosition& rPos, SwDoc & rDoc);
    SwDoc* GetDoc();

    sal_Bool IsValid();
};

XTextRangeOrNodeIndexPosition::XTextRangeOrNodeIndexPosition() :
    xRange(NULL),
    pIndex(NULL)
{
}

XTextRangeOrNodeIndexPosition::~XTextRangeOrNodeIndexPosition()
{
    delete pIndex;
}

void XTextRangeOrNodeIndexPosition::Set( Reference<XTextRange> & rRange )
{
    xRange = rRange->getStart();    // set bookmark
    if (NULL != pIndex)
    {
        delete pIndex;
        pIndex = NULL;
    }
}

void XTextRangeOrNodeIndexPosition::Set( SwNodeIndex& rIndex )
{
    if (NULL != pIndex)
        delete pIndex;

    pIndex = new SwNodeIndex(rIndex);
    (*pIndex)-- ;   // previous node!!!
    xRange = NULL;
}

void XTextRangeOrNodeIndexPosition::SetAsNodeIndex(
    Reference<XTextRange> & rRange )
{
    // XTextRange -> XTunnel -> SwXTextRange
    SwDoc* pDoc = lcl_GetDocViaTunnel(rRange);

    if (!pDoc)
    {
        OSL_TRACE("SetAsNodeIndex: no SwDoc");
        return;
    }

    // SwXTextRange -> PaM
    SwUnoInternalPaM aPaM(*pDoc);
#if OSL_DEBUG_LEVEL > 0
    sal_Bool bSuccess =
#endif
        ::sw::XTextRangeToSwPaM(aPaM, rRange);
    OSL_ENSURE(bSuccess, "illegal range");

    // PaM -> Index
    Set(aPaM.GetPoint()->nNode);
}

void
XTextRangeOrNodeIndexPosition::CopyPositionInto(SwPosition& rPos, SwDoc & rDoc)
{
    OSL_ENSURE(IsValid(), "Can't get Position");

    // create PAM from start cursor (if no node index is present)
    if (NULL == pIndex)
    {
        SwUnoInternalPaM aUnoPaM(rDoc);
#if OSL_DEBUG_LEVEL > 0
        sal_Bool bSuccess =
#endif
            ::sw::XTextRangeToSwPaM(aUnoPaM, xRange);
        OSL_ENSURE(bSuccess, "illegal range");

        rPos = *aUnoPaM.GetPoint();
    }
    else
    {
        rPos.nNode = *pIndex;
        rPos.nNode++;           // pIndex points to previous index !!!
        rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), 0 );
    }
}

SwDoc* XTextRangeOrNodeIndexPosition::GetDoc()
{
    OSL_ENSURE(IsValid(), "Can't get Doc");

    return (NULL != pIndex) ? pIndex->GetNodes().GetDoc() : lcl_GetDocViaTunnel(xRange);
}

sal_Bool XTextRangeOrNodeIndexPosition::IsValid()
{
    return ( xRange.is() || (pIndex != NULL) );
}


//
// RedlineInfo: temporary storage for redline data
//

class RedlineInfo
{
public:
    RedlineInfo();
    ~RedlineInfo();

    /// redline type (insert, delete, ...)
    RedlineType_t eType;

    // info fields:
    OUString sAuthor;               /// change author string
    OUString sComment;              /// change comment string
    util::DateTime aDateTime;       /// change DateTime
    sal_Bool bMergeLastParagraph;   /// the SwRedline::IsDelLastPara flag

    // each position can may be either empty, an XTextRange, or an SwNodeIndex

    // start pos of anchor (may be empty)
    XTextRangeOrNodeIndexPosition aAnchorStart;

    // end pos of anchor (may be empty)
    XTextRangeOrNodeIndexPosition aAnchorEnd;

    /// index of content node (maybe NULL)
    SwNodeIndex* pContentIndex;

    /// next redline info (for hierarchical redlines)
    RedlineInfo* pNextRedline;

    /// store whether we expect an adjustment for this redline
    sal_Bool bNeedsAdjustment;
};

RedlineInfo::RedlineInfo() :
    eType(nsRedlineType_t::REDLINE_INSERT),
    sAuthor(),
    sComment(),
    aDateTime(),
    bMergeLastParagraph( sal_False ),
    aAnchorStart(),
    aAnchorEnd(),
    pContentIndex(NULL),
    pNextRedline(NULL),
    bNeedsAdjustment( sal_False )
{
}

RedlineInfo::~RedlineInfo()
{
    delete pContentIndex;
    delete pNextRedline;
}


//
// XMLRedlineImportHelper
//

XMLRedlineImportHelper::XMLRedlineImportHelper(
    sal_Bool bNoRedlinesPlease,
    const Reference<XPropertySet> & rModel,
    const Reference<XPropertySet> & rImportInfo ) :
        sEmpty(),
        sInsertion( GetXMLToken( XML_INSERTION )),
        sDeletion( GetXMLToken( XML_DELETION )),
        sFormatChange( GetXMLToken( XML_FORMAT_CHANGE )),
        sShowChanges(RTL_CONSTASCII_USTRINGPARAM("ShowChanges")),
        sRecordChanges(RTL_CONSTASCII_USTRINGPARAM("RecordChanges")),
        sRedlineProtectionKey(RTL_CONSTASCII_USTRINGPARAM("RedlineProtectionKey")),
        aRedlineMap(),
        bIgnoreRedlines(bNoRedlinesPlease),
        xModelPropertySet(rModel),
        xImportInfoPropertySet(rImportInfo)
{
    // check to see if redline mode is handled outside of component
    sal_Bool bHandleShowChanges = sal_True;
    sal_Bool bHandleRecordChanges = sal_True;
    sal_Bool bHandleProtectionKey = sal_True;
    if ( xImportInfoPropertySet.is() )
    {
        Reference<XPropertySetInfo> xInfo =
            xImportInfoPropertySet->getPropertySetInfo();

        bHandleShowChanges = ! xInfo->hasPropertyByName( sShowChanges );
        bHandleRecordChanges = ! xInfo->hasPropertyByName( sRecordChanges );
        bHandleProtectionKey = ! xInfo->hasPropertyByName( sRedlineProtectionKey );
    }

    // get redline mode
    bShowChanges = *(sal_Bool*)
        ( bHandleShowChanges ? xModelPropertySet : xImportInfoPropertySet )
        ->getPropertyValue( sShowChanges ).getValue();
    bRecordChanges = *(sal_Bool*)
        ( bHandleRecordChanges ? xModelPropertySet : xImportInfoPropertySet )
        ->getPropertyValue( sRecordChanges ).getValue();
    {
        Any aAny = (bHandleProtectionKey  ? xModelPropertySet
                                          : xImportInfoPropertySet )
                        ->getPropertyValue( sRedlineProtectionKey );
        aAny >>= aProtectionKey;
    }

    // set redline mode to "don't record changes"
    if( bHandleRecordChanges )
    {
        Any aAny;
        sal_Bool bTmp = sal_False;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xModelPropertySet->setPropertyValue( sRecordChanges, aAny );
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
            pInfo->bNeedsAdjustment = sal_False;
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
    sal_Bool bHandleShowChanges = sal_True;
    sal_Bool bHandleRecordChanges = sal_True;
    sal_Bool bHandleProtectionKey = sal_True;
    if ( xImportInfoPropertySet.is() )
    {
        Reference<XPropertySetInfo> xInfo =
            xImportInfoPropertySet->getPropertySetInfo();

        bHandleShowChanges = ! xInfo->hasPropertyByName( sShowChanges );
        bHandleRecordChanges = ! xInfo->hasPropertyByName( sRecordChanges );
        bHandleProtectionKey = ! xInfo->hasPropertyByName( sRedlineProtectionKey );
    }

    // set redline mode & key
    Any aAny;

    aAny.setValue( &bShowChanges, ::getBooleanCppuType() );
    if ( bHandleShowChanges )
        xModelPropertySet->setPropertyValue( sShowChanges, aAny );
    else
        xImportInfoPropertySet->setPropertyValue( sShowChanges, aAny );

    aAny.setValue( &bRecordChanges, ::getBooleanCppuType() );
    if ( bHandleRecordChanges )
        xModelPropertySet->setPropertyValue( sRecordChanges, aAny );
    else
        xImportInfoPropertySet->setPropertyValue( sRecordChanges, aAny );

    aAny <<= aProtectionKey;
    if ( bHandleProtectionKey )
        xModelPropertySet->setPropertyValue( sRedlineProtectionKey, aAny );
    else
        xImportInfoPropertySet->setPropertyValue( sRedlineProtectionKey, aAny);
}

void XMLRedlineImportHelper::Add(
    const OUString& rType,
    const OUString& rId,
    const OUString& rAuthor,
    const OUString& rComment,
    const util::DateTime& rDateTime,
    sal_Bool bMergeLastPara)
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
        // (hierarchy sanity checking happens on insertino into the document)

        // find last element
        RedlineInfo* pInfoChain;
        for( pInfoChain = aRedlineMap[rId];
            NULL != pInfoChain->pNextRedline;
            pInfoChain = pInfoChain->pNextRedline) ; // empty loop

        // insert as last element
        pInfoChain->pNextRedline = pInfo;
    }
}

Reference<XTextCursor> XMLRedlineImportHelper::CreateRedlineTextSection(
    Reference<XTextCursor> xOldCursor,
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
            OSL_TRACE("XMLRedlineImportHelper::CreateRedlineTextSection: "
                "no SwDoc => cannot create section.");
            return 0;
        }

        // create text section for redline
        SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool
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
            new SwXTextCursor(*pDoc, pXText, CURSOR_REDLINE, aPos);
        pXCursor->GetCursor()->Move(fnMoveForward, fnGoNode);
        // cast to avoid ambiguity
        xReturn = static_cast<text::XWordCursor*>(pXCursor);
    }
    // else: unknown redline -> Ignore

    return xReturn;
}

void XMLRedlineImportHelper::SetCursor(
    const OUString& rId,
    sal_Bool bStart,
    Reference<XTextRange> & rRange,
    sal_Bool bIsOutsideOfParagraph)
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
            pInfo->bNeedsAdjustment = sal_True;
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
    sal_Bool /*bStart*/,
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

        pInfo->bNeedsAdjustment = sal_False;

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


inline sal_Bool XMLRedlineImportHelper::IsReady(RedlineInfo* pRedline)
{
    // we can insert a redline if we have start & end, and we don't
    // expect adjustments for either of these
    return ( pRedline->aAnchorEnd.IsValid() &&
             pRedline->aAnchorStart.IsValid() &&
             !pRedline->bNeedsAdjustment );
}

void XMLRedlineImportHelper::InsertIntoDocument(RedlineInfo* pRedlineInfo)
{
    OSL_ENSURE(NULL != pRedlineInfo, "need redline info");
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
        OSL_TRACE("XMLRedlineImportHelper::InsertIntoDocument: "
                "no SwDoc => cannot insert redline.");
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
    if( !aPaM.HasMark() && (pRedlineInfo->pContentIndex == NULL) )
    {
        // these redlines have no function, and will thus be ignored (just as
        // in sw3io), so no action here
    }
    else if ( bIgnoreRedlines ||
         !CheckNodesRange( aPaM.GetPoint()->nNode,
                           aPaM.GetMark()->nNode,
                           sal_True ) )
    {
        // ignore redline (e.g. file loaded in insert mode):
        // delete 'deleted' redlines and forget about the whole thing
        if (nsRedlineType_t::REDLINE_DELETE == pRedlineInfo->eType)
        {
            pDoc->DeleteRange(aPaM);
            // And what about the "deleted nodes"?
            // They have to be deleted as well (#i80689)!
            if( bIgnoreRedlines && pRedlineInfo->pContentIndex != NULL )
            {
                SwNodeIndex aIdx( *pRedlineInfo->pContentIndex );
                const SwNode* pEnd = aIdx.GetNode().EndOfSectionNode();
                if( pEnd )
                {
                    SwNodeIndex aEnd( *pEnd, 1 );
                    SwPaM aDel( aIdx, aEnd );
                    pDoc->DeleteRange(aDel);
                }
            }
        }
    }
    else
    {
        // regular file loading: insert redline

        // create redline (using pRedlineData which gets copied in SwRedline())
        SwRedlineData* pRedlineData = ConvertRedline(pRedlineInfo, pDoc);
        SwRedline* pRedline =
            new SwRedline( pRedlineData, *aPaM.GetPoint(), sal_True,
                           !pRedlineInfo->bMergeLastParagraph, sal_False );

        // set mark
        if( aPaM.HasMark() )
        {
            pRedline->SetMark();
            *(pRedline->GetMark()) = *aPaM.GetMark();
        }

        // set content node (if necessary)
        if (NULL != pRedlineInfo->pContentIndex)
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
        pDoc->SetRedlineMode_intern(nsRedlineMode_t::REDLINE_ON);
        pDoc->AppendRedline(pRedline, false);
        pDoc->SetRedlineMode_intern(nsRedlineMode_t::REDLINE_NONE);
    }
}

SwRedlineData* XMLRedlineImportHelper::ConvertRedline(
    RedlineInfo* pRedlineInfo,
    SwDoc* pDoc)
{
    // convert info:
    // 1) Author String -> Author ID (default to zero)
    sal_uInt16 nAuthorId = (NULL == pDoc) ? 0 :
        pDoc->InsertRedlineAuthor( pRedlineInfo->sAuthor );

    // 2) util::DateTime -> DateTime
    DateTime aDT( DateTime::EMPTY );
    aDT.SetYear(    pRedlineInfo->aDateTime.Year );
    aDT.SetMonth(   pRedlineInfo->aDateTime.Month );
    aDT.SetDay(     pRedlineInfo->aDateTime.Day );
    aDT.SetHour(    pRedlineInfo->aDateTime.Hours );
    aDT.SetMin(     pRedlineInfo->aDateTime.Minutes );
    aDT.SetSec(     pRedlineInfo->aDateTime.Seconds );
    aDT.Set100Sec(  pRedlineInfo->aDateTime.HundredthSeconds );

    // 3) recursively convert next redline
    //    ( check presence and sanity of hierarchical redline info )
    SwRedlineData* pNext = NULL;
    if ( (NULL != pRedlineInfo->pNextRedline) &&
         (nsRedlineType_t::REDLINE_DELETE == pRedlineInfo->eType) &&
         (nsRedlineType_t::REDLINE_INSERT == pRedlineInfo->pNextRedline->eType) )
    {
        pNext = ConvertRedline(pRedlineInfo->pNextRedline, pDoc);
    }

    // create redline data
    SwRedlineData* pData = new SwRedlineData(pRedlineInfo->eType,
                                             nAuthorId, aDT,
                                             pRedlineInfo->sComment,
                                             pNext, // next data (if available)
                                             NULL); // no extra data

    return pData;
}


void XMLRedlineImportHelper::SetShowChanges( sal_Bool bShow )
{
    bShowChanges = bShow;
}

void XMLRedlineImportHelper::SetRecordChanges( sal_Bool bRecord )
{
    bRecordChanges = bRecord;
}

void XMLRedlineImportHelper::SetProtectionKey(
    const Sequence<sal_Int8> & rKey )
{
    aProtectionKey = rKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
