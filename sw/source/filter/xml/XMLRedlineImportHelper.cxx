/*************************************************************************
 *
 *  $RCSfile: XMLRedlineImportHelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2001-01-10 21:01:48 $
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



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XWordCursor;
using ::com::sun::star::lang::XUnoTunnel;
// collision with tools/DateTime: use UNO DateTime as util::DateTime
// using ::com::sun::star::util::DateTime;


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

    /// start pos of anchor (may be empty)
    Reference<XTextRange> xAnchorStartPos;
    /// end pos of anchor (may be empty)
    Reference<XTextRange> xAnchorEndPos;

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
    xAnchorStartPos(),
    xAnchorEndPos(),
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

XMLRedlineImportHelper::XMLRedlineImportHelper() :
    sEmpty(),
    sInsertion(RTL_CONSTASCII_USTRINGPARAM(sXML_insertion)),
    sDeletion(RTL_CONSTASCII_USTRINGPARAM(sXML_deletion)),
    sFormatChange(RTL_CONSTASCII_USTRINGPARAM(sXML_format_change)),
    aRedlineMap()
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
        Reference<XUnoTunnel> xTunnel( xOldCursor, UNO_QUERY);
        DBG_ASSERT( xTunnel.is(), "missing XUnoTunnel for Cursor" );
        SwXTextCursor* pOldCursor = (SwXTextCursor*)xTunnel->getSomething(
                                            SwXTextCursor::getUnoTunnelId());
        DBG_ASSERT( NULL != pOldCursor, "SwXTextCursor missing" );
        SwDoc* pDoc = pOldCursor->GetDoc();

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
    Reference<XTextRange> & rRange)
{
    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        // RedlineInfo found; now set Cursor
        RedlineInfo* pInfo = aFind->second;
        if (bStart)
        {
            pInfo->xAnchorStartPos = rRange;
        }
        else
        {
            pInfo->xAnchorEndPos = rRange;
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

inline sal_Bool XMLRedlineImportHelper::IsReady(RedlineInfo* pRedline)
{
    return pRedline->xAnchorEndPos.is() && pRedline->xAnchorStartPos.is();
}

void XMLRedlineImportHelper::InsertIntoDocument(RedlineInfo* pRedlineInfo)
{
    DBG_ASSERT(NULL != pRedlineInfo, "need redline info");
    DBG_ASSERT(IsReady(pRedlineInfo), "redline info not complete yet!");

    // get the document (from the XTextRange implementation object)
    Reference<XUnoTunnel> xTunnel(pRedlineInfo->xAnchorStartPos, UNO_QUERY);
    DBG_ASSERT(xTunnel.is(), "Can't tunnel -> can't get document");
    SwXTextRange *pRange = (SwXTextRange*)xTunnel->getSomething(
        SwXTextRange::getUnoTunnelId());
    SwDoc *pDoc = pRange->GetDoc();

    // create PAM from start+end cursors
    SwUnoInternalPaM aStartPaM(*pDoc);
    sal_Bool bSuccess =
        SwXTextRange::XTextRangeToSwPaM( aStartPaM,
                                         pRedlineInfo->xAnchorStartPos);
    DBG_ASSERT(bSuccess, "illegal range");
    SwUnoInternalPaM aEndPaM(*pDoc);
    bSuccess =
        SwXTextRange::XTextRangeToSwPaM( aEndPaM,
                                         pRedlineInfo->xAnchorEndPos);
    DBG_ASSERT(bSuccess, "illegal range");

    // now create the PaM for the redline
    SwPaM aPaM(*aStartPaM.GetPoint(), *aEndPaM.GetPoint());

    // create redline (using redline data, which gets copied in SwRedline())
    SwRedlineData* pRedlineData = ConvertRedline(pRedlineInfo, pDoc);
    SwRedline* pRedline = new SwRedline(*pRedlineData, aPaM);
    delete pRedlineData;

    // set content node (if necessary)
    if (NULL != pRedlineInfo->pContentIndex)
    {
        pRedline->SetContentIdx(pRedlineInfo->pContentIndex);
    }

// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
// HACK: DO NOT COMMIT!
    pDoc->SetRedlineMode(REDLINE_ON);
    pDoc->AppendRedline(pRedline);
    pDoc->SetRedlineMode(REDLINE_IGNORE);

// instead of:
    // and insert into document
    // pDoc->AppendRedline(pRedline);
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
