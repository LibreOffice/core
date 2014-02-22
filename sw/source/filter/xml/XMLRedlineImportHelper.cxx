/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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


#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::frame::XModel;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XWordCursor;
using ::com::sun::star::lang::XUnoTunnel;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;




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
    
    
    return (pXRange) ? pXRange->GetDoc() : 0;
}









class XTextRangeOrNodeIndexPosition
{
    Reference<XTextRange> xRange;
    SwNodeIndex* pIndex;    

public:
    XTextRangeOrNodeIndexPosition();
    ~XTextRangeOrNodeIndexPosition();

    void Set( Reference<XTextRange> & rRange );
    void Set( SwNodeIndex& rIndex );
    void SetAsNodeIndex( Reference<XTextRange> & rRange );

    void CopyPositionInto(SwPosition& rPos, SwDoc & rDoc);
    SwDoc* GetDoc();

    bool IsValid();
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
    xRange = rRange->getStart();    
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
    (*pIndex)-- ;   
    xRange = NULL;
}

void XTextRangeOrNodeIndexPosition::SetAsNodeIndex(
    Reference<XTextRange> & rRange )
{
    
    SwDoc* pDoc = lcl_GetDocViaTunnel(rRange);

    if (!pDoc)
    {
        OSL_TRACE("SetAsNodeIndex: no SwDoc");
        return;
    }

    
    SwUnoInternalPaM aPaM(*pDoc);
#if OSL_DEBUG_LEVEL > 0
    sal_Bool bSuccess =
#endif
        ::sw::XTextRangeToSwPaM(aPaM, rRange);
    OSL_ENSURE(bSuccess, "illegal range");

    
    Set(aPaM.GetPoint()->nNode);
}

void
XTextRangeOrNodeIndexPosition::CopyPositionInto(SwPosition& rPos, SwDoc & rDoc)
{
    OSL_ENSURE(IsValid(), "Can't get Position");

    
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
        rPos.nNode++;           
        rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), 0 );
    }
}

SwDoc* XTextRangeOrNodeIndexPosition::GetDoc()
{
    OSL_ENSURE(IsValid(), "Can't get Doc");

    return (NULL != pIndex) ? pIndex->GetNodes().GetDoc() : lcl_GetDocViaTunnel(xRange);
}

bool XTextRangeOrNodeIndexPosition::IsValid()
{
    return ( xRange.is() || (pIndex != NULL) );
}


class RedlineInfo
{
public:
    RedlineInfo();
    ~RedlineInfo();

    
    RedlineType_t eType;

    
    OUString sAuthor;               
    OUString sComment;              
    util::DateTime aDateTime;       
    sal_Bool bMergeLastParagraph;   

    

    
    XTextRangeOrNodeIndexPosition aAnchorStart;

    
    XTextRangeOrNodeIndexPosition aAnchorEnd;

    
    SwNodeIndex* pContentIndex;

    
    RedlineInfo* pNextRedline;

    
    bool bNeedsAdjustment;
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
    bNeedsAdjustment( false )
{
}

RedlineInfo::~RedlineInfo()
{
    delete pContentIndex;
    delete pNextRedline;
}


XMLRedlineImportHelper::XMLRedlineImportHelper(
    bool bNoRedlinesPlease,
    const Reference<XPropertySet> & rModel,
    const Reference<XPropertySet> & rImportInfo ) :
        sEmpty(),
        sInsertion( GetXMLToken( XML_INSERTION )),
        sDeletion( GetXMLToken( XML_DELETION )),
        sFormatChange( GetXMLToken( XML_FORMAT_CHANGE )),
        sShowChanges("ShowChanges"),
        sRecordChanges("RecordChanges"),
        sRedlineProtectionKey("RedlineProtectionKey"),
        aRedlineMap(),
        bIgnoreRedlines(bNoRedlinesPlease),
        xModelPropertySet(rModel),
        xImportInfoPropertySet(rImportInfo)
{
    
    bool bHandleShowChanges = true;
    bool bHandleRecordChanges = true;
    bool bHandleProtectionKey = true;
    if ( xImportInfoPropertySet.is() )
    {
        Reference<XPropertySetInfo> xInfo =
            xImportInfoPropertySet->getPropertySetInfo();

        bHandleShowChanges = ! xInfo->hasPropertyByName( sShowChanges );
        bHandleRecordChanges = ! xInfo->hasPropertyByName( sRecordChanges );
        bHandleProtectionKey = ! xInfo->hasPropertyByName( sRedlineProtectionKey );
    }

    
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
    
    RedlineMapType::iterator aFind = aRedlineMap.begin();
    for( ; aRedlineMap.end() != aFind; ++aFind )
    {
        RedlineInfo* pInfo = aFind->second;

        
        
        if( IsReady(pInfo) )
        {
            OSL_FAIL("forgotten RedlineInfo; now inserted");
            InsertIntoDocument( pInfo );
        }
        else
        {
            
            pInfo->bNeedsAdjustment = false;
            if( IsReady(pInfo) )
            {
                OSL_FAIL("RedlineInfo without adjustment; now inserted");
                InsertIntoDocument( pInfo );
            }
            else
            {
                
                
                
                
                OSL_FAIL("incomplete redline (maybe file was corrupt); "
                          "now deleted");
            }
        }
        delete pInfo;
    }
    aRedlineMap.clear();

    
    
    bool bHandleShowChanges = true;
    bool bHandleRecordChanges = true;
    bool bHandleProtectionKey = true;
    if ( xImportInfoPropertySet.is() )
    {
        Reference<XPropertySetInfo> xInfo =
            xImportInfoPropertySet->getPropertySetInfo();

        bHandleShowChanges = ! xInfo->hasPropertyByName( sShowChanges );
        bHandleRecordChanges = ! xInfo->hasPropertyByName( sRecordChanges );
        bHandleProtectionKey = ! xInfo->hasPropertyByName( sRedlineProtectionKey );
    }

    
    try
    {
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
    catch (const uno::RuntimeException &) 
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
    sal_Bool bMergeLastPara)
{
    
    
    
    
    
    

    
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
        
        return;
    }

    
    RedlineInfo* pInfo = new RedlineInfo();

    
    pInfo->eType = eType;
    pInfo->sAuthor = rAuthor;
    pInfo->sComment = rComment;
    pInfo->aDateTime = rDateTime;
    pInfo->bMergeLastParagraph = bMergeLastPara;


    
    if (aRedlineMap.end() == aRedlineMap.find(rId))
    {
        
        aRedlineMap[rId] = pInfo;
    }
    else
    {
        
        
        

        
        RedlineInfo* pInfoChain;
        for( pInfoChain = aRedlineMap[rId];
            NULL != pInfoChain->pNextRedline;
            pInfoChain = pInfoChain->pNextRedline) ; 

        
        pInfoChain->pNextRedline = pInfo;
    }
}

Reference<XTextCursor> XMLRedlineImportHelper::CreateRedlineTextSection(
    Reference<XTextCursor> xOldCursor,
    const OUString& rId)
{
    Reference<XTextCursor> xReturn;

    
    SolarMutexGuard aGuard;

    
    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        
        SwDoc* pDoc = lcl_GetDocViaTunnel(xOldCursor);

        if (!pDoc)
        {
            OSL_TRACE("XMLRedlineImportHelper::CreateRedlineTextSection: "
                "no SwDoc => cannot create section.");
            return 0;
        }

        
        SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool
            (RES_POOLCOLL_STANDARD, false );
        SwStartNode* pRedlineNode = pDoc->GetNodes().MakeTextSection(
            pDoc->GetNodes().GetEndOfRedlines(),
            SwNormalStartNode,
            pColl);

        
        SwNodeIndex aIndex(*pRedlineNode);
        aFind->second->pContentIndex = new SwNodeIndex(aIndex);

        
        SwXText* pXText = new SwXRedlineText(pDoc, aIndex);
        Reference<XText> xText = pXText;  

        
        SwPosition aPos(*pRedlineNode);
        SwXTextCursor *const pXCursor =
            new SwXTextCursor(*pDoc, pXText, CURSOR_REDLINE, aPos);
        pXCursor->GetCursor()->Move(fnMoveForward, fnGoNode);
        
        xReturn = static_cast<text::XWordCursor*>(pXCursor);
    }
    

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
        
        RedlineInfo* pInfo = aFind->second;
        if (bIsOutsideOfParagraph)
        {
            
            if (bStart)
            {
                pInfo->aAnchorStart.SetAsNodeIndex(rRange);
            }
            else
            {
                pInfo->aAnchorEnd.SetAsNodeIndex(rRange);
            }

            
            pInfo->bNeedsAdjustment = true;
        }
        else
        {
            
            if (bStart)
                pInfo->aAnchorStart.Set(rRange);
            else
                pInfo->aAnchorEnd.Set(rRange);
        }

        
        
        
        if (IsReady(pInfo))
        {
            InsertIntoDocument(pInfo);
            aRedlineMap.erase(rId);
            delete pInfo;
        }
    }
    
}

void XMLRedlineImportHelper::AdjustStartNodeCursor(
    const OUString& rId,        
    sal_Bool /*bStart*/,
    Reference<XTextRange> & /*rRange*/)
{
    
    SolarMutexGuard aGuard;

    
    

    RedlineMapType::iterator aFind = aRedlineMap.find(rId);
    if (aRedlineMap.end() != aFind)
    {
        
        RedlineInfo* pInfo = aFind->second;

        pInfo->bNeedsAdjustment = false;

        
        if( IsReady(pInfo) )
        {
            InsertIntoDocument(pInfo);
            aRedlineMap.erase(rId);
            delete pInfo;
        }
    }
    
}


inline bool XMLRedlineImportHelper::IsReady(RedlineInfo* pRedline)
{
    
    
    return ( pRedline->aAnchorEnd.IsValid() &&
             pRedline->aAnchorStart.IsValid() &&
             !pRedline->bNeedsAdjustment );
}

void XMLRedlineImportHelper::InsertIntoDocument(RedlineInfo* pRedlineInfo)
{
    OSL_ENSURE(NULL != pRedlineInfo, "need redline info");
    OSL_ENSURE(IsReady(pRedlineInfo), "redline info not complete yet!");

    
    SolarMutexGuard aGuard;

    
    
    

    
    SwDoc* pDoc = pRedlineInfo->aAnchorStart.GetDoc();

    if (!pDoc)
    {
        OSL_TRACE("XMLRedlineImportHelper::InsertIntoDocument: "
                "no SwDoc => cannot insert redline.");
        return;
    }

    
    SwPaM aPaM(pDoc->GetNodes().GetEndOfContent());
    pRedlineInfo->aAnchorStart.CopyPositionInto(*aPaM.GetPoint(), *pDoc);
    aPaM.SetMark();
    pRedlineInfo->aAnchorEnd.CopyPositionInto(*aPaM.GetPoint(), *pDoc);

    
    if (*aPaM.GetPoint() == *aPaM.GetMark())
    {
        aPaM.DeleteMark();
    }


    
    
    
    
    
    
    if( !aPaM.HasMark() && (pRedlineInfo->pContentIndex == NULL) )
    {
        
        
    }
    else if ( bIgnoreRedlines ||
         !CheckNodesRange( aPaM.GetPoint()->nNode,
                           aPaM.GetMark()->nNode,
                           true ) )
    {
        
        
        if (nsRedlineType_t::REDLINE_DELETE == pRedlineInfo->eType)
        {
            pDoc->DeleteRange(aPaM);
            
            
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
        

        
        SwRedlineData* pRedlineData = ConvertRedline(pRedlineInfo, pDoc);
        SwRangeRedline* pRedline =
            new SwRangeRedline( pRedlineData, *aPaM.GetPoint(), sal_True,
                           !pRedlineInfo->bMergeLastParagraph, sal_False );

        
        if( aPaM.HasMark() )
        {
            pRedline->SetMark();
            *(pRedline->GetMark()) = *aPaM.GetMark();
        }

        
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

        
        pDoc->SetRedlineMode_intern(nsRedlineMode_t::REDLINE_ON);
        pDoc->AppendRedline(pRedline, false);
        pDoc->SetRedlineMode_intern(nsRedlineMode_t::REDLINE_NONE);
    }
}

SwRedlineData* XMLRedlineImportHelper::ConvertRedline(
    RedlineInfo* pRedlineInfo,
    SwDoc* pDoc)
{
    
    
    sal_uInt16 nAuthorId = (NULL == pDoc) ? 0 :
        pDoc->InsertRedlineAuthor( pRedlineInfo->sAuthor );

    
    DateTime aDT( DateTime::EMPTY );
    aDT.SetYear(    pRedlineInfo->aDateTime.Year );
    aDT.SetMonth(   pRedlineInfo->aDateTime.Month );
    aDT.SetDay(     pRedlineInfo->aDateTime.Day );
    aDT.SetHour(    pRedlineInfo->aDateTime.Hours );
    aDT.SetMin(     pRedlineInfo->aDateTime.Minutes );
    aDT.SetSec(     pRedlineInfo->aDateTime.Seconds );
    aDT.SetNanoSec( pRedlineInfo->aDateTime.NanoSeconds );

    
    
    SwRedlineData* pNext = NULL;
    if ( (NULL != pRedlineInfo->pNextRedline) &&
         (nsRedlineType_t::REDLINE_DELETE == pRedlineInfo->eType) &&
         (nsRedlineType_t::REDLINE_INSERT == pRedlineInfo->pNextRedline->eType) )
    {
        pNext = ConvertRedline(pRedlineInfo->pNextRedline, pDoc);
    }

    
    SwRedlineData* pData = new SwRedlineData(pRedlineInfo->eType,
                                             nAuthorId, aDT,
                                             pRedlineInfo->sComment,
                                             pNext, 
                                             NULL); 

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
