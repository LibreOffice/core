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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <pagedesc.hxx>
#include "poolfmt.hxx"
#include <redline.hxx>
#include <section.hxx>
#include <unoprnms.hxx>

#include <errhdl.hxx>

#include <unoobj.hxx>
#include <unocoll.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>
#include <unoobj.hxx>
#include <unoredline.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <rtl/uuid.h>

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/text/XTextTable.hpp>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

using rtl::OUString;
using rtl::OUStringBuffer;

SwXRedlineText::SwXRedlineText(SwDoc* pDoc, SwNodeIndex aIndex) :
    SwXText(pDoc, CURSOR_REDLINE),
    aNodeIndex(aIndex)
{
}

const SwStartNode* SwXRedlineText::GetStartNode() const
{
    return aNodeIndex.GetNode().GetStartNode();
}

Any SwXRedlineText::queryInterface( const Type& rType )
    throw(RuntimeException)
{
    Any aRet;

    if (::getCppuType((Reference<XEnumerationAccess> *)0) == rType)
    {
        Reference<XEnumerationAccess> aAccess = this;
        aRet <<= aAccess;
    }
    else
    {
        // delegate to SwXText and OWeakObject
        aRet = SwXText::queryInterface(rType);
        if(!aRet.hasValue())
        {
            aRet = OWeakObject::queryInterface(rType);
        }
    }

    return aRet;
}

Sequence<Type> SwXRedlineText::getTypes()
    throw(RuntimeException)
{
    // SwXText::getTypes()
    uno::Sequence<uno::Type> aTypes = SwXText::getTypes();

    // add XEnumerationAccess
    sal_Int32 nLength = aTypes.getLength();
    aTypes.realloc(nLength + 1);
    aTypes[nLength] = ::getCppuType((Reference<XEnumerationAccess> *)0);

    return aTypes;
}

Sequence<sal_Int8> SwXRedlineText::getImplementationId()
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

Reference<XTextCursor> SwXRedlineText::createTextCursor(void)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    SwPosition aPos(aNodeIndex);
    SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, CURSOR_REDLINE,
                                             GetDoc());
    SwUnoCrsr* pUnoCursor = pCrsr->GetCrsr();
    pUnoCursor->Move(fnMoveForward, fnGoNode);

    // #101929# prevent a newly created text cursor from running inside a table
    // because table cells have their own XText.
    // Patterned after SwXTextFrame::createTextCursor(void).

    // skip all tables at the beginning
    SwTableNode* pTableNode = pUnoCursor->GetNode()->FindTableNode();
    SwCntntNode* pContentNode = NULL;
    while( pTableNode != NULL )
    {
        pUnoCursor->GetPoint()->nNode = *(pTableNode->EndOfSectionNode());
        pContentNode = GetDoc()->GetNodes().GoNext(&pUnoCursor->GetPoint()->nNode);
        pTableNode = pContentNode->FindTableNode();
    }
    if( pContentNode != NULL )
        pUnoCursor->GetPoint()->nContent.Assign( pContentNode, 0 );
    if( pUnoCursor->GetNode()->FindSttNodeByType( SwNormalStartNode ) != 
                                                               GetStartNode() )
    {
        // We have gone too far and have left our own redline. This means that
        // no content node outside of a table could be found, and therefore we
        // except.
        uno::RuntimeException aExcept;
        aExcept.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( 
            "No content node found that is inside this change section "
            "but outside of a table" ) );
        throw aExcept;
    }

    return (XWordCursor*)pCrsr;
}

Reference<XTextCursor> SwXRedlineText::createTextCursorByRange(
    const Reference<XTextRange> & aTextRange)
        throw( RuntimeException )
{
    Reference<XTextCursor> xCursor = createTextCursor();
    xCursor->gotoRange(aTextRange->getStart(), sal_False);
    xCursor->gotoRange(aTextRange->getEnd(), sal_True);
    return xCursor;
}

Reference<XEnumeration> SwXRedlineText::createEnumeration(void)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwPaM aPam(aNodeIndex);
    aPam.Move(fnMoveForward, fnGoNode);
    return new SwXParagraphEnumeration(this, *aPam.Start(), CURSOR_REDLINE);
}

uno::Type SwXRedlineText::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

sal_Bool SwXRedlineText::hasElements(  ) throw(RuntimeException)
{
    return sal_True; 	// we always have a content index
}

SwXRedlinePortion::SwXRedlinePortion(	const SwRedline* pRed,
                        const SwUnoCrsr* pPortionCrsr,
                        Reference< XText >  xParent, BOOL bStart) :
    SwXTextPortion(pPortionCrsr, xParent, bStart ? PORTION_REDLINE_START : PORTION_REDLINE_END),
    pRedline(pRed)
{
    SetCollapsed(!pRedline->HasMark());
}

SwXRedlinePortion::~SwXRedlinePortion()
{
}

util::DateTime lcl_DateTimeToUno(const DateTime& rDT)
{
    util::DateTime aRetDT;
    aRetDT.Year = rDT.GetYear();
    aRetDT.Month= rDT.GetMonth();
    aRetDT.Day  	= rDT.GetDay();
    aRetDT.Hours	= rDT.GetHour();
    aRetDT.Minutes = rDT.GetMin();
    aRetDT.Seconds = rDT.GetSec();
    aRetDT.HundredthSeconds = rDT.Get100Sec();
    return aRetDT;
}

OUString lcl_RedlineTypeToOUString(SwRedlineType eType)
{
    OUString sRet;
    switch(eType & REDLINE_NO_FLAG_MASK)
    {
        case REDLINE_INSERT: sRet = C2U("Insert"); break;
        case REDLINE_DELETE: sRet = C2U("Delete"); break;
        case REDLINE_FORMAT: sRet = C2U("Format"); break;
        case REDLINE_TABLE:  sRet = C2U("TextTable"); break;
        case REDLINE_FMTCOLL:sRet = C2U("Style"); break;
    }
    return sRet;
}

Sequence<PropertyValue> lcl_GetSuccessorProperties(const SwRedline& rRedline)
{
    Sequence<PropertyValue> aValues(4);

    const SwRedlineData* pNext = rRedline.GetRedlineData().Next();
    if(pNext)
    {
        PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_AUTHOR));
        // GetAuthorString(n) walks the SwRedlineData* chain;
        // here we always need element 1
        pValues[0].Value <<= OUString(rRedline.GetAuthorString(1));
        pValues[1].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_DATE_TIME));
        pValues[1].Value <<= lcl_DateTimeToUno(pNext->GetTimeStamp());
        pValues[2].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_COMMENT));
        pValues[2].Value <<= OUString(pNext->GetComment());
        pValues[3].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_TYPE));
        pValues[3].Value <<= lcl_RedlineTypeToOUString(pNext->GetType());
    }
    return aValues;
}

Any SwXRedlinePortion::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Validate();
    Any aRet;
    if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_TEXT)))
    {
        SwNodeIndex* pNodeIdx = pRedline->GetContentIdx();
        if(pNodeIdx )
        {
            if ( 1 < ( pNodeIdx->GetNode().EndOfSectionIndex() - pNodeIdx->GetNode().GetIndex() ) )
            {
                SwUnoCrsr* pUnoCrsr = GetCrsr();
                Reference<XText> xRet = new SwXRedlineText(pUnoCrsr->GetDoc(), *pNodeIdx);
                aRet <<= xRet;
            }
            else
                DBG_ASSERT(0, "Empty section in redline portion! (end node immediately follows start node)");
        }
    }
    else
    {
        aRet = GetPropertyValue( rPropertyName, *pRedline);
        if(!aRet.hasValue() &&
           ! rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_SUCCESSOR_DATA)))
            aRet = SwXTextPortion::getPropertyValue(rPropertyName);
    }
    return aRet;
}

void SwXRedlinePortion::Validate() throw( RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    //search for the redline
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    const SwRedlineTbl& rRedTbl = pDoc->GetRedlineTbl();
    sal_Bool bFound = FALSE;
    for(USHORT nRed = 0; nRed < rRedTbl.Count() && !bFound; nRed++)
        bFound = pRedline == rRedTbl[nRed];
    if(!bFound)
        throw RuntimeException();
}

uno::Sequence< sal_Int8 > SAL_CALL SwXRedlinePortion::getImplementationId(  ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

Any  SwXRedlinePortion::GetPropertyValue( const OUString& rPropertyName, const SwRedline& rRedline ) throw()
{
    Any aRet;
    if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_AUTHOR)))
        aRet <<= OUString(rRedline.GetAuthorString());
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_DATE_TIME)))
    {
        aRet <<= lcl_DateTimeToUno(rRedline.GetTimeStamp());
    }
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_COMMENT)))
        aRet <<= OUString(rRedline.GetComment());
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_TYPE)))
    {
        aRet <<= lcl_RedlineTypeToOUString(rRedline.GetType());
    }
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_SUCCESSOR_DATA)))
    {
        if(rRedline.GetRedlineData().Next())
            aRet <<= lcl_GetSuccessorProperties(rRedline);
    }
    else if (rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_IDENTIFIER)))
    {
        OUStringBuffer sBuf;
        sBuf.append((sal_Int64)&rRedline);
        aRet <<= sBuf.makeStringAndClear();
    }
    else if (rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_IS_IN_HEADER_FOOTER)))
    {
        sal_Bool bRet =
            rRedline.GetDoc()->IsInHeaderFooter( rRedline.GetPoint()->nNode );
        aRet.setValue(&bRet, ::getBooleanCppuType());
    }
    else if (rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_MERGE_LAST_PARA)))
    {
        sal_Bool bRet = !rRedline.IsDelLastPara();
        aRet.setValue( &bRet, ::getBooleanCppuType() );
    }
    return aRet;
}

Sequence< PropertyValue > SwXRedlinePortion::CreateRedlineProperties(
    const SwRedline& rRedline, sal_Bool bIsStart ) throw()
{
    Sequence< PropertyValue > aRet(11);
    const SwRedlineData* pNext = rRedline.GetRedlineData().Next();
    PropertyValue* pRet = aRet.getArray();

    OUStringBuffer sRedlineIdBuf;
    sRedlineIdBuf.append((sal_Int64)&rRedline);

    sal_Int32 nPropIdx  = 0;
    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_AUTHOR));
    pRet[nPropIdx++].Value <<= OUString(rRedline.GetAuthorString());
    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_DATE_TIME));
    pRet[nPropIdx++].Value <<= lcl_DateTimeToUno(rRedline.GetTimeStamp());
    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_COMMENT));
    pRet[nPropIdx++].Value <<= OUString(rRedline.GetComment());
    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_TYPE));
    pRet[nPropIdx++].Value <<= lcl_RedlineTypeToOUString(rRedline.GetType());
    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_IDENTIFIER));
    pRet[nPropIdx++].Value <<= sRedlineIdBuf.makeStringAndClear();
    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_IS_COLLAPSED));
    sal_Bool bTmp = !rRedline.HasMark();
    pRet[nPropIdx++].Value.setValue(&bTmp, ::getBooleanCppuType()) ;

    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_IS_START));
    pRet[nPropIdx++].Value.setValue(&bIsStart, ::getBooleanCppuType()) ;

    bTmp = !rRedline.IsDelLastPara();
    pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_MERGE_LAST_PARA));
    pRet[nPropIdx++].Value.setValue(&bTmp, ::getBooleanCppuType()) ;

    SwNodeIndex* pNodeIdx = rRedline.GetContentIdx();
    if(pNodeIdx )
    {
        if ( 1 < ( pNodeIdx->GetNode().EndOfSectionIndex() - pNodeIdx->GetNode().GetIndex() ) )
        {
            Reference<XText> xRet = new SwXRedlineText(rRedline.GetDoc(), *pNodeIdx);
            pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_TEXT));
            pRet[nPropIdx++].Value <<= xRet;
        }
        else
            DBG_ASSERT(0, "Empty section in redline portion! (end node immediately follows start node)");
    }
    if(pNext)
    {
        pRet[nPropIdx].Name = C2U(SW_PROP_NAME_STR(UNO_NAME_REDLINE_SUCCESSOR_DATA));
        pRet[nPropIdx++].Value <<= lcl_GetSuccessorProperties(rRedline);
    }
    aRet.realloc(nPropIdx);
    return aRet;
}

TYPEINIT1(SwXRedline, SwClient);
SwXRedline::SwXRedline(SwRedline& rRedline, SwDoc& rDoc) :
    pDoc(&rDoc),
    SwXText(&rDoc, CURSOR_REDLINE),
    pRedline(&rRedline)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXRedline::~SwXRedline()
{
}

Reference< XPropertySetInfo > SwXRedline::getPropertySetInfo(  ) throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  xRef =
        SfxItemPropertySet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_REDLINE)).getPropertySetInfo();
    return xRef;
}

void SwXRedline::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
        WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw RuntimeException();
    if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_AUTHOR)))
    {
        DBG_ERROR("currently not available");
    }
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_DATE_TIME)))
    {
        DBG_ERROR("currently not available");
    }
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_COMMENT)))
    {
        OUString sTmp; aValue >>= sTmp;
        pRedline->SetComment(sTmp);
    }
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_TYPE)))
    {
        DBG_ERROR("currently not available");
        OUString sTmp; aValue >>= sTmp;
        if(!sTmp.getLength())
            throw IllegalArgumentException();
    }
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_SUCCESSOR_DATA)))
    {
        DBG_ERROR("currently not available");
    }
    else
    {
        throw IllegalArgumentException();
    }
}

Any SwXRedline::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw RuntimeException();
    Any aRet;
    BOOL bStart = rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_START));
    if(bStart ||
        rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_END)))
    {
        Reference<XInterface> xRet;
        SwNode* pNode = pRedline->GetNode();
        if(!bStart && pRedline->HasMark())
            pNode = pRedline->GetNode(FALSE);
        switch(pNode->GetNodeType())
        {
            case ND_SECTIONNODE:
            {
                SwSectionNode* pSectNode = pNode->GetSectionNode();
                DBG_ASSERT(pSectNode, "No section node!");
                xRet = SwXTextSections::GetObject( *pSectNode->GetSection().GetFmt() );
            }
            break;
            case ND_TABLENODE :
            {
                SwTableNode* pTblNode = pNode->GetTableNode();
                DBG_ASSERT(pTblNode, "No table node!");
                SwTable& rTbl = pTblNode->GetTable();
                SwFrmFmt* pTblFmt = rTbl.GetFrmFmt();
                xRet = SwXTextTables::GetObject( *pTblFmt );
            }
            break;
            case ND_TEXTNODE :
            {
                SwPosition* pPoint = 0;
                if(bStart || !pRedline->HasMark())
                    pPoint = pRedline->GetPoint();
                else
                    pPoint = pRedline->GetMark();
                SwPaM aTmp(*pPoint);
                Reference<XText> xTmpParent;
                Reference<XTextRange>xRange =
                    SwXTextRange::CreateTextRangeFromPosition( pDoc, *pPoint, 0 );
                xRet = xRange.get();
            }
            break;
            default:
                DBG_ERROR("illegal node type");
        }
        aRet <<= xRet;
    }
    else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_REDLINE_TEXT)))
    {
        SwNodeIndex* pNodeIdx = pRedline->GetContentIdx();
        if( pNodeIdx )
        {
            if ( 1 < ( pNodeIdx->GetNode().EndOfSectionIndex() - pNodeIdx->GetNode().GetIndex() ) )
            {
                Reference<XText> xRet = new SwXRedlineText(pDoc, *pNodeIdx);
                aRet <<= xRet;
            }
            else
                DBG_ASSERT(0, "Empty section in redline portion! (end node immediately follows start node)");
        }
    }
    else
        aRet = SwXRedlinePortion::GetPropertyValue(rPropertyName, *pRedline);
    return aRet;
}

void SwXRedline::addPropertyChangeListener(
    const OUString& aPropertyName,
    const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXRedline::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXRedline::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXRedline::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXRedline::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
      {
        pDoc = 0;
        pRedline = 0;
    }
}

Reference< XEnumeration >  SwXRedline::createEnumeration(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XEnumeration > xRet;
    if(!pDoc)
        throw RuntimeException();

    SwNodeIndex* pNodeIndex = pRedline->GetContentIdx();
    if(pNodeIndex)
    {
        SwPaM aPam(*pNodeIndex);
        aPam.Move(fnMoveForward, fnGoNode);
        xRet = new SwXParagraphEnumeration(this, *aPam.Start(), CURSOR_REDLINE);
    }
    return xRet;
}

uno::Type SwXRedline::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

sal_Bool SwXRedline::hasElements(  ) throw(RuntimeException)
{
    if(!pDoc)
        throw RuntimeException();
    return 0 != pRedline->GetContentIdx();
}

Reference< XTextCursor >  SwXRedline::createTextCursor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw RuntimeException();

    uno::Reference< XTextCursor >  	xRet;
    SwNodeIndex* pNodeIndex = pRedline->GetContentIdx();
    if(pNodeIndex)
    {
        SwPosition aPos(*pNodeIndex);
        SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, CURSOR_REDLINE, pDoc);
        SwUnoCrsr* pUnoCrsr = pCrsr->GetCrsr();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);

        //steht hier eine Tabelle?
        SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
        SwCntntNode* pCont = 0;
        while( pTblNode )
        {
            pUnoCrsr->GetPoint()->nNode = *pTblNode->EndOfSectionNode();
            pCont = GetDoc()->GetNodes().GoNext(&pUnoCrsr->GetPoint()->nNode);
            pTblNode = pCont->FindTableNode();
        }
        if(pCont)
            pUnoCrsr->GetPoint()->nContent.Assign(pCont, 0);
        xRet =  (XWordCursor*)pCrsr;
    }
    else
    {
        throw RuntimeException();
    }
    return xRet;
}

Reference< XTextCursor >  SwXRedline::createTextCursorByRange(
    const Reference< XTextRange > & aTextPosition)
        throw( RuntimeException )
{
    throw RuntimeException();
    return Reference< XTextCursor >  ();
}

Any SwXRedline::queryInterface( const uno::Type& rType )
    throw(RuntimeException)
{
    Any aRet = SwXText::queryInterface(rType);
    if(!aRet.hasValue())
    {
        aRet = SwXRedlineBaseClass::queryInterface(rType);
    }
    return aRet;
}

Sequence<Type> SwXRedline::getTypes()
    throw(RuntimeException)
{
    uno::Sequence<uno::Type> aTypes = SwXText::getTypes();
    uno::Sequence<uno::Type> aBaseTypes = SwXRedlineBaseClass::getTypes();
    const uno::Type* pBaseTypes = aBaseTypes.getConstArray();
    sal_Int32 nCurType = aTypes.getLength();
    aTypes.realloc(aTypes.getLength() + aBaseTypes.getLength());
    uno::Type* pTypes = aTypes.getArray();
    for(sal_Int32 nType = 0; nType < aBaseTypes.getLength(); nType++)
        pTypes[nCurType++] = pBaseTypes[nType];
    return aTypes;
}

Sequence<sal_Int8> SwXRedline::getImplementationId()
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
