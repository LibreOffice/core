/*************************************************************************
 *
 *  $RCSfile: XMLRedlineImportHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:42:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop


#ifndef _XMLREDLINEIMPORTHELPER_HXX
#include "XMLRedlineImportHelper.hxx"
#endif

#ifndef _UNOOBJ_HXX
#include "unoobj.hxx"
#endif

#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif

#ifndef _DOC_HXX
#include "doc.hxx"
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#ifndef _POOLFMT_HXX
#include "poolfmt.hxx"
#endif

#ifndef _UNOREDLINE_HXX
#include "unoredline.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XWORDCURSOR_HPP_
#include <com/sun/star/text/XWordCursor.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextDocument;
using ::com::sun::star::text::XWordCursor;
using ::com::sun::star::lang::XUnoTunnel;
// collision with tools/DateTime: use UNO DateTime as util::DateTime
// using ::com::sun::star::util::DateTime;


//
// a few helper functions
//

SwDoc* lcl_GetDocViaTunnel( Reference<XTextCursor> & rCursor )
{
    Reference<XUnoTunnel> xTunnel( rCursor, UNO_QUERY);
    DBG_ASSERT( xTunnel.is(), "missing XUnoTunnel for Cursor" );
    SwXTextCursor* pSwXCursor =
        (SwXTextCursor*)xTunnel->getSomething(SwXTextCursor::getUnoTunnelId());
    DBG_ASSERT( NULL != pSwXCursor, "SwXTextCursor missing" );
    return pSwXCursor->GetDoc();
}

SwDoc* lcl_GetDocViaTunnel( Reference<XTextRange> & rRange )
{
    Reference<XUnoTunnel> xTunnel(rRange, UNO_QUERY);
    DBG_ASSERT(xTunnel.is(), "Can't tunnel XTextRange");
    SwXTextRange *pRange =
        (SwXTextRange*)xTunnel->getSomething(SwXTextRange::getUnoTunnelId());
    DBG_ASSERT( NULL != pRange, "SwXTextRange missing" );
    return pRange->GetDoc();
}


//
// XTextRangeOrNodeIndexPosition: store a position into the text
// *either* as an XTextRange or as an SwNodeIndex. The reason is that
// me must store either pointers to StartNodes (because redlines may
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

    void CopyPositionInto(SwPosition& rPos);
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

    // SwXTextRange -> PaM
    SwUnoInternalPaM aPaM(*pDoc);
    sal_Bool bSuccess = SwXTextRange::XTextRangeToSwPaM( aPaM, rRange);
    DBG_ASSERT(bSuccess, "illegal range");

    // PaM -> Index
    Set(aPaM.GetPoint()->nNode);
}

void XTextRangeOrNodeIndexPosition::CopyPositionInto(SwPosition& rPos)
{
    DBG_ASSERT(IsValid(), "Can't get Position");

    // create PAM from start cursor (if no node index is present)
    if (NULL == pIndex)
    {
        SwUnoInternalPaM aUnoPaM(*GetDoc());
        sal_Bool bSuccess = SwXTextRange::XTextRangeToSwPaM(aUnoPaM, xRange);
        DBG_ASSERT(bSuccess, "illegal range");

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
    DBG_ASSERT(IsValid(), "Can't get Doc");

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
    enum SwRedlineType eType;

    // info fields:
    OUString sAuthor;               /// change author string
    OUString sComment;              /// change comment string
    util::DateTime aDateTime;       /// change DateTime

    // each position can may be either empty, an XTextRange, or an SwNodeIndex

    // start pos of anchor (may be empty)
    XTextRangeOrNodeIndexPosition aAnchorStart;

    // end pos of anchor (may be empty)
    XTextRangeOrNodeIndexPosition aAnchorEnd;

    /// index of content node (maybe NULL)
    SwNodeIndex* pContentIndex;

    /// next redline info (for hierarchical redlines)
    RedlineInfo* pNextRedline;
};

RedlineInfo::RedlineInfo() :
    eType(REDLINE_INSERT),
    sAuthor(),
    sComment(),
    aDateTime(),
    aAnchorStart(),
    aAnchorEnd(),
    pContentIndex(NULL),
    pNextRedline(NULL)
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
    sal_Bool bNoRedlinesPlease) :
        sEmpty(),
        sInsertion(RTL_CONSTASCII_USTRINGPARAM(sXML_insertion)),
        sDeletion(RTL_CONSTASCII_USTRINGPARAM(sXML_deletion)),
        sFormatChange(RTL_CONSTASCII_USTRINGPARAM(sXML_format_change)),
        aRedlineMap(),
        bIgnoreRedlines(bNoRedlinesPlease),
        pSaveDoc(NULL)
{
}

XMLRedlineImportHelper::~XMLRedlineImportHelper()
{
    // delete all left over (and obviously incomplete) RedlineInfos (and map)
    RedlineMapType::iterator aFind = aRedlineMap.begin();
    if (aRedlineMap.end() != aFind)
    {
        // get RedlineInfo* and delete; the RedlineInfo should not be complete
        RedlineInfo* pInfo = aFind->second;
        DBG_ASSERT(! IsReady(pInfo), "forgotten RedlineInfo (now deleted)");
        delete pInfo;
    }
    aRedlineMap.clear();

    // set redline mode; first set bogus redline mode with
    // SetRedlineMode_intern(), so that the subsequent
    // SetRedlineMode() is forced to update the data structures
    if (NULL != pSaveDoc)
    {
        // TODO: get "real" Redline mode from the saved document
        // sal_uInt16 nRedlineMode = REDLINE_ON | REDLINE_SHOW_MASK;
        sal_uInt16 nRedlineMode = REDLINE_NONE | REDLINE_SHOW_MASK;
        pSaveDoc->SetRedlineMode_intern(~nRedlineMode);
        pSaveDoc->SetRedlineMode(nRedlineMode);
    }
}

void XMLRedlineImportHelper::Add(
    const OUString& rType,
    const OUString& rId,
    const OUString& rAuthor,
    const OUString& rComment,
    const util::DateTime& rDateTime)
{
    // we need to do the following:
    // 1) parse type string
    // 2) create RedlineInfo and fill it with data
    // 3) check for existing redline with same ID
    // 3a) insert redline into map
    // 3b) attach to existing redline

    // ad 1)
    enum SwRedlineType eType;
    if (rType.equals(sInsertion))
    {
        eType = REDLINE_INSERT;
    }
    else if (rType.equals(sDeletion))
    {
        eType = REDLINE_DELETE;
    }
    else if (rType.equals(sFormatChange))
    {
        eType = REDLINE_FORMAT;
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
        for(RedlineInfo* pInfoChain = aRedlineMap[rId];
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

    // get RedlineInfo
    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        // get document from old cursor (via tunnel)
        SwDoc* pDoc = lcl_GetDocViaTunnel(xOldCursor);

        // create text section for redline
        SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
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
        SwXTextCursor* pCursor =
            new SwXTextCursor(pXText, aPos, CURSOR_REDLINE, pDoc);
        pCursor->GetCrsr()->Move(fnMoveForward, fnGoNode);

        xReturn = (XWordCursor*)pCursor;    // cast to avoid ambigiouty
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
                pInfo->aAnchorStart.SetAsNodeIndex(rRange);
            else
                pInfo->aAnchorEnd.SetAsNodeIndex(rRange);
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
    sal_Bool bStart,
    Reference<XTextRange> & rRange)
{
    DBG_ASSERT(bStart,"End nodes not supported. Can't happen anyway, can it?");
    if (!bStart)
        return;

    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        // RedlineInfo found; now set Cursor
        RedlineInfo* pInfo = aFind->second;

        DBG_ASSERT(pInfo->aAnchorStart.IsValid(),
                   "AdjustStartNodeCursor may only be called after SetCursor");
        if (!pInfo->aAnchorStart.IsValid())
            return;

        SwDoc* pDoc = pInfo->aAnchorStart.GetDoc();


        // OK, we have the redline. Now find the proper start node for
        // the range and do the necessary sanity checking. If
        // successful, set the start node as the new redline start.
        SwUnoInternalPaM aUnoPaM(*pDoc);
        sal_Bool bSuccess = SwXTextRange::XTextRangeToSwPaM(aUnoPaM, rRange);
        DBG_ASSERT(bSuccess, "illegal range");

        // adjustment only supported for tables and sections
        SwNode& rNode = aUnoPaM.GetPoint()->nNode.GetNode();
        SwNode* pTblNode = rNode.FindTableNode();
        SwNode* pSctnNode = rNode.FindSectionNode();
        if ((NULL != pTblNode) || (NULL != pSctnNode))
        {
            // find the closest
            SwNode* pStartNode = NULL;
            if (pTblNode == NULL)
            {
                pStartNode = pSctnNode;
            }
            else if (pSctnNode == NULL)
            {
                pStartNode = pTblNode;
            }
            else
            {
                pStartNode = (pSctnNode->GetIndex() > pTblNode->GetIndex()) ?
                    pSctnNode : pTblNode;
            }

            // pStartNode is our start node candidate. Now check for distance
            // between previous start node
            // ...skip for now

            SwNodeIndex aIndex(pStartNode->GetDoc()->GetNodes(),
                               pStartNode->GetIndex());
            pInfo->aAnchorStart.Set(aIndex);
        }
        // else: we are neither inside a table nor a section -> ignore


    }
    // else: can't find redline -> ignore
}


inline sal_Bool XMLRedlineImportHelper::IsReady(RedlineInfo* pRedline)
{
    return ( pRedline->aAnchorEnd.IsValid() &&
             pRedline->aAnchorStart.IsValid()   );
}

void XMLRedlineImportHelper::InsertIntoDocument(RedlineInfo* pRedlineInfo)
{
    DBG_ASSERT(NULL != pRedlineInfo, "need redline info");
    DBG_ASSERT(IsReady(pRedlineInfo), "redline info not complete yet!");

    // Insert the Redline as described by pRedlineInfo into the
    // document.  If we are in insert mode, don't insert any redlines
    // (and delete 'deleted' inline redlines)

    // get the document (from one of the positions)
    SwDoc* pDoc = pRedlineInfo->aAnchorStart.GetDoc();

    // now create the PaM for the redline
    SwPaM aPaM(pDoc->GetNodes().GetEndOfContent());
    pRedlineInfo->aAnchorStart.CopyPositionInto(*aPaM.GetPoint());
    aPaM.SetMark();
    pRedlineInfo->aAnchorEnd.CopyPositionInto(*aPaM.GetPoint());

    // collapse PaM if (start == end)
    if (*aPaM.GetPoint() == *aPaM.GetMark())
    {
        aPaM.DeleteMark();
    }

    // check for:
    // a) bIgnoreRedline (e.g. insert mode)
    // b) illegal PaM range (CheckNodesRange())
    if ( bIgnoreRedlines ||
         !CheckNodesRange( aPaM.GetPoint()->nNode,
                           aPaM.GetMark()->nNode,
                           sal_True ) )
    {
        // ignore redline (e.g. file loaded in insert mode):
        // delete 'deleted' redlines and forget about the whole thing
        if (REDLINE_DELETE == pRedlineInfo->eType)
        {
            pDoc->Delete(aPaM);
        }
    }
    else
    {
        // regular file loading: insert redline

        // create redline (using pRedlineData which gets copied in SwRedline())
        SwRedlineData* pRedlineData = ConvertRedline(pRedlineInfo, pDoc);
        SwRedline* pRedline = new SwRedline(*pRedlineData, aPaM);
        delete pRedlineData;

        // set content node (if necessary)
        if (NULL != pRedlineInfo->pContentIndex)
        {
            pRedline->SetContentIdx(pRedlineInfo->pContentIndex);
        }

        // set redline mode (without doing the associated book-keeping)
        pDoc->SetRedlineMode_intern(REDLINE_ON);
        pDoc->AppendRedline(pRedline);
        pDoc->SetRedlineMode_intern(REDLINE_NONE);

        // also: save document
        pSaveDoc = pDoc;
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
    DateTime aDT;
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
         (REDLINE_DELETE == pRedlineInfo->eType) &&
         (REDLINE_INSERT == pRedlineInfo->pNextRedline->eType) )
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
