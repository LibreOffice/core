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

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>

#include <pagedesc.hxx>
#include "poolfmt.hxx"
#include <redline.hxx>
#include <section.hxx>
#include <unoprnms.hxx>
#include <unomid.h>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unoparagraph.hxx>
#include <unocoll.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>
#include <unoport.hxx>
#include <unoredline.hxx>
#include <doc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docary.hxx>

using namespace ::com::sun::star;

SwXRedlineText::SwXRedlineText(SwDoc* _pDoc, SwNodeIndex aIndex) :
    SwXText(_pDoc, CursorType::Redline),
    aNodeIndex(aIndex)
{
}

const SwStartNode* SwXRedlineText::GetStartNode() const
{
    return aNodeIndex.GetNode().GetStartNode();
}

uno::Any SwXRedlineText::queryInterface( const uno::Type& rType )
{
    uno::Any aRet;

    if (cppu::UnoType<container::XEnumerationAccess>::get()== rType)
    {
        uno::Reference<container::XEnumerationAccess> aAccess = this;
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

uno::Sequence<uno::Type> SwXRedlineText::getTypes()
{
    // SwXText::getTypes()
    uno::Sequence<uno::Type> aTypes = SwXText::getTypes();

    // add container::XEnumerationAccess
    sal_Int32 nLength = aTypes.getLength();
    aTypes.realloc(nLength + 1);
    aTypes[nLength] = cppu::UnoType<container::XEnumerationAccess>::get();

    return aTypes;
}

uno::Sequence<sal_Int8> SwXRedlineText::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Reference<text::XTextCursor> SwXRedlineText::createTextCursor()
{
    SolarMutexGuard aGuard;

    SwPosition aPos(aNodeIndex);
    SwXTextCursor *const pXCursor =
        new SwXTextCursor(*GetDoc(), this, CursorType::Redline, aPos);
    auto& rUnoCursor(pXCursor->GetCursor());
    rUnoCursor.Move(fnMoveForward, GoInNode);

    // #101929# prevent a newly created text cursor from running inside a table
    // because table cells have their own XText.
    // Patterned after SwXTextFrame::createTextCursor().

    // skip all tables at the beginning
    SwTableNode* pTableNode = rUnoCursor.GetNode().FindTableNode();
    SwContentNode* pContentNode = nullptr;
    bool bTable = pTableNode != nullptr;
    while( pTableNode != nullptr )
    {
        rUnoCursor.GetPoint()->nNode = *(pTableNode->EndOfSectionNode());
        pContentNode = GetDoc()->GetNodes().GoNext(&rUnoCursor.GetPoint()->nNode);
        pTableNode = pContentNode->FindTableNode();
    }
    if( pContentNode != nullptr )
        rUnoCursor.GetPoint()->nContent.Assign( pContentNode, 0 );
    if( bTable && rUnoCursor.GetNode().FindSttNodeByType( SwNormalStartNode )
                                                            != GetStartNode() )
    {
        // We have gone too far and have left our own redline. This means that
        // no content node outside of a table could be found, and therefore we
        // except.
        uno::RuntimeException aExcept;
        aExcept.Message =
            "No content node found that is inside this change section "
            "but outside of a table";
        throw aExcept;
    }

    return static_cast<text::XWordCursor*>(pXCursor);
}

uno::Reference<text::XTextCursor> SwXRedlineText::createTextCursorByRange(
    const uno::Reference<text::XTextRange> & aTextRange)
{
    uno::Reference<text::XTextCursor> xCursor = createTextCursor();
    xCursor->gotoRange(aTextRange->getStart(), false);
    xCursor->gotoRange(aTextRange->getEnd(), true);
    return xCursor;
}

uno::Reference<container::XEnumeration> SwXRedlineText::createEnumeration()
{
    SolarMutexGuard aGuard;
    SwPaM aPam(aNodeIndex);
    aPam.Move(fnMoveForward, GoInNode);
    auto pUnoCursor(GetDoc()->CreateUnoCursor(*aPam.Start()));
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CursorType::Redline);
}

uno::Type SwXRedlineText::getElementType(  )
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SwXRedlineText::hasElements(  )
{
    return true;    // we always have a content index
}

SwXRedlinePortion::SwXRedlinePortion(SwRangeRedline const& rRedline,
        SwUnoCursor const*const pPortionCursor,
        uno::Reference< text::XText > const& xParent, bool const bStart)
    : SwXTextPortion(pPortionCursor, xParent,
            (bStart) ? PORTION_REDLINE_START : PORTION_REDLINE_END)
    , m_rRedline(rRedline)
{
    SetCollapsed(!m_rRedline.HasMark());
}

SwXRedlinePortion::~SwXRedlinePortion()
{
}

OUString SwRedlineTypeToOUString(RedlineType_t eType)
{
    OUString sRet;
    switch(eType & nsRedlineType_t::REDLINE_NO_FLAG_MASK)
    {
        case nsRedlineType_t::REDLINE_INSERT: sRet = "Insert"; break;
        case nsRedlineType_t::REDLINE_DELETE: sRet = "Delete"; break;
        case nsRedlineType_t::REDLINE_FORMAT: sRet = "Format"; break;
        case nsRedlineType_t::REDLINE_PARAGRAPH_FORMAT: sRet = "ParagraphFormat"; break;
        case nsRedlineType_t::REDLINE_TABLE:  sRet = "TextTable"; break;
        case nsRedlineType_t::REDLINE_FMTCOLL:sRet = "Style"; break;
    }
    return sRet;
}

static uno::Sequence<beans::PropertyValue> lcl_GetSuccessorProperties(const SwRangeRedline& rRedline)
{
    uno::Sequence<beans::PropertyValue> aValues(4);

    const SwRedlineData* pNext = rRedline.GetRedlineData().Next();
    if(pNext)
    {
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = UNO_NAME_REDLINE_AUTHOR;
        // GetAuthorString(n) walks the SwRedlineData* chain;
        // here we always need element 1
        pValues[0].Value <<= rRedline.GetAuthorString(1);
        pValues[1].Name = UNO_NAME_REDLINE_DATE_TIME;
        pValues[1].Value <<= pNext->GetTimeStamp().GetUNODateTime();
        pValues[2].Name = UNO_NAME_REDLINE_COMMENT;
        pValues[2].Value <<= pNext->GetComment();
        pValues[3].Name = UNO_NAME_REDLINE_TYPE;
        pValues[3].Value <<= SwRedlineTypeToOUString(pNext->GetType());
    }
    return aValues;
}

uno::Any SwXRedlinePortion::getPropertyValue( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    Validate();
    uno::Any aRet;
    if(rPropertyName == UNO_NAME_REDLINE_TEXT)
    {
        SwNodeIndex* pNodeIdx = m_rRedline.GetContentIdx();
        if(pNodeIdx )
        {
            if ( 1 < ( pNodeIdx->GetNode().EndOfSectionIndex() - pNodeIdx->GetNode().GetIndex() ) )
            {
                SwUnoCursor& rUnoCursor = GetCursor();
                uno::Reference<text::XText> xRet = new SwXRedlineText(rUnoCursor.GetDoc(), *pNodeIdx);
                aRet <<= xRet;
            }
            else {
                OSL_FAIL("Empty section in redline portion! (end node immediately follows start node)");
            }
        }
    }
    else
    {
        aRet = GetPropertyValue(rPropertyName, m_rRedline);
        if(!aRet.hasValue() &&
           rPropertyName != UNO_NAME_REDLINE_SUCCESSOR_DATA)
            aRet = SwXTextPortion::getPropertyValue(rPropertyName);
    }
    return aRet;
}

void SwXRedlinePortion::Validate()
{
    SwUnoCursor& rUnoCursor = GetCursor();
    //search for the redline
    SwDoc* pDoc = rUnoCursor.GetDoc();
    const SwRedlineTable& rRedTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    bool bFound = false;
    for(size_t nRed = 0; nRed < rRedTable.size() && !bFound; nRed++)
        bFound = &m_rRedline == rRedTable[nRed];
    if(!bFound)
        throw uno::RuntimeException();
}

uno::Sequence< sal_Int8 > SAL_CALL SwXRedlinePortion::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Any  SwXRedlinePortion::GetPropertyValue( const OUString& rPropertyName, const SwRangeRedline& rRedline )
{
    uno::Any aRet;
    if(rPropertyName == UNO_NAME_REDLINE_AUTHOR)
        aRet <<= rRedline.GetAuthorString();
    else if(rPropertyName == UNO_NAME_REDLINE_DATE_TIME)
    {
        aRet <<= rRedline.GetTimeStamp().GetUNODateTime();
    }
    else if(rPropertyName == UNO_NAME_REDLINE_COMMENT)
        aRet <<= rRedline.GetComment();
    else if(rPropertyName == UNO_NAME_REDLINE_DESCRIPTION)
        aRet <<= const_cast<SwRangeRedline&>(rRedline).GetDescr();
    else if(rPropertyName == UNO_NAME_REDLINE_TYPE)
    {
        aRet <<= SwRedlineTypeToOUString(rRedline.GetType());
    }
    else if(rPropertyName == UNO_NAME_REDLINE_SUCCESSOR_DATA)
    {
        if(rRedline.GetRedlineData().Next())
            aRet <<= lcl_GetSuccessorProperties(rRedline);
    }
    else if (rPropertyName == UNO_NAME_REDLINE_IDENTIFIER)
    {
        aRet <<= OUString::number(
            sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(&rRedline) ) );
    }
    else if (rPropertyName == UNO_NAME_IS_IN_HEADER_FOOTER)
    {
        aRet <<= rRedline.GetDoc()->IsInHeaderFooter( rRedline.GetPoint()->nNode );
    }
    else if (rPropertyName == UNO_NAME_MERGE_LAST_PARA)
    {
        aRet <<= !rRedline.IsDelLastPara();
    }
    return aRet;
}

uno::Sequence< beans::PropertyValue > SwXRedlinePortion::CreateRedlineProperties(
    const SwRangeRedline& rRedline, bool bIsStart )
{
    uno::Sequence< beans::PropertyValue > aRet(12);
    const SwRedlineData* pNext = rRedline.GetRedlineData().Next();
    beans::PropertyValue* pRet = aRet.getArray();

    sal_Int32 nPropIdx  = 0;
    pRet[nPropIdx].Name = UNO_NAME_REDLINE_AUTHOR;
    pRet[nPropIdx++].Value <<= rRedline.GetAuthorString();
    pRet[nPropIdx].Name = UNO_NAME_REDLINE_DATE_TIME;
    pRet[nPropIdx++].Value <<= rRedline.GetTimeStamp().GetUNODateTime();
    pRet[nPropIdx].Name = UNO_NAME_REDLINE_COMMENT;
    pRet[nPropIdx++].Value <<= rRedline.GetComment();
    pRet[nPropIdx].Name = UNO_NAME_REDLINE_DESCRIPTION;
    pRet[nPropIdx++].Value <<= const_cast<SwRangeRedline&>(rRedline).GetDescr();
    pRet[nPropIdx].Name = UNO_NAME_REDLINE_TYPE;
    pRet[nPropIdx++].Value <<= SwRedlineTypeToOUString(rRedline.GetType());
    pRet[nPropIdx].Name = UNO_NAME_REDLINE_IDENTIFIER;
    pRet[nPropIdx++].Value <<= OUString::number(
        sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(&rRedline) ) );
    pRet[nPropIdx].Name = UNO_NAME_IS_COLLAPSED;
    pRet[nPropIdx++].Value <<= !rRedline.HasMark();

    pRet[nPropIdx].Name = UNO_NAME_IS_START;
    pRet[nPropIdx++].Value <<= bIsStart;

    pRet[nPropIdx].Name = UNO_NAME_MERGE_LAST_PARA;
    pRet[nPropIdx++].Value <<= !rRedline.IsDelLastPara();

    SwNodeIndex* pNodeIdx = rRedline.GetContentIdx();
    if(pNodeIdx )
    {
        if ( 1 < ( pNodeIdx->GetNode().EndOfSectionIndex() - pNodeIdx->GetNode().GetIndex() ) )
        {
            uno::Reference<text::XText> xRet = new SwXRedlineText(rRedline.GetDoc(), *pNodeIdx);
            pRet[nPropIdx].Name = UNO_NAME_REDLINE_TEXT;
            pRet[nPropIdx++].Value <<= xRet;
        }
        else {
            OSL_FAIL("Empty section in redline portion! (end node immediately follows start node)");
        }
    }
    if(pNext)
    {
        pRet[nPropIdx].Name = UNO_NAME_REDLINE_SUCCESSOR_DATA;
        pRet[nPropIdx++].Value <<= lcl_GetSuccessorProperties(rRedline);
    }
    aRet.realloc(nPropIdx);
    return aRet;
}

SwXRedline::SwXRedline(SwRangeRedline& rRedline, SwDoc& rDoc) :
    SwXText(&rDoc, CursorType::Redline),
    pDoc(&rDoc),
    pRedline(&rRedline)
{
    pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXRedline::~SwXRedline()
{
}

uno::Reference< beans::XPropertySetInfo > SwXRedline::getPropertySetInfo(  )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef =
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_REDLINE)->getPropertySetInfo();
    return xRef;
}

void SwXRedline::setPropertyValue( const OUString& rPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(rPropertyName == UNO_NAME_REDLINE_AUTHOR)
    {
        OSL_FAIL("currently not available");
    }
    else if(rPropertyName == UNO_NAME_REDLINE_DATE_TIME)
    {
        OSL_FAIL("currently not available");
    }
    else if(rPropertyName == UNO_NAME_REDLINE_COMMENT)
    {
        OUString sTmp; aValue >>= sTmp;
        pRedline->SetComment(sTmp);
    }
    else if(rPropertyName == UNO_NAME_REDLINE_DESCRIPTION)
    {
        SAL_WARN("sw.uno", "SwXRedline::setPropertyValue: can't set Description");
    }
    else if(rPropertyName == UNO_NAME_REDLINE_TYPE)
    {
        OSL_FAIL("currently not available");
        OUString sTmp; aValue >>= sTmp;
        if(sTmp.isEmpty())
            throw lang::IllegalArgumentException();
    }
    else if(rPropertyName == UNO_NAME_REDLINE_SUCCESSOR_DATA)
    {
        OSL_FAIL("currently not available");
    }
    else
    {
        throw lang::IllegalArgumentException();
    }
}

uno::Any SwXRedline::getPropertyValue( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    uno::Any aRet;
    bool bStart = rPropertyName == UNO_NAME_REDLINE_START;
    if(bStart ||
        rPropertyName == UNO_NAME_REDLINE_END)
    {
        uno::Reference<XInterface> xRet;
        SwNode* pNode = &pRedline->GetNode();
        if(!bStart && pRedline->HasMark())
            pNode = &pRedline->GetNode(false);
        switch(pNode->GetNodeType())
        {
            case SwNodeType::Section:
            {
                SwSectionNode* pSectNode = pNode->GetSectionNode();
                OSL_ENSURE(pSectNode, "No section node!");
                xRet = SwXTextSections::GetObject( *pSectNode->GetSection().GetFormat() );
            }
            break;
            case SwNodeType::Table :
            {
                SwTableNode* pTableNode = pNode->GetTableNode();
                OSL_ENSURE(pTableNode, "No table node!");
                SwTable& rTable = pTableNode->GetTable();
                SwFrameFormat* pTableFormat = rTable.GetFrameFormat();
                xRet = SwXTextTables::GetObject( *pTableFormat );
            }
            break;
            case SwNodeType::Text :
            {
                SwPosition* pPoint = nullptr;
                if(bStart || !pRedline->HasMark())
                    pPoint = pRedline->GetPoint();
                else
                    pPoint = pRedline->GetMark();
                const uno::Reference<text::XTextRange> xRange =
                    SwXTextRange::CreateXTextRange(*pDoc, *pPoint, nullptr);
                xRet = xRange.get();
            }
            break;
            default:
                OSL_FAIL("illegal node type");
        }
        aRet <<= xRet;
    }
    else if(rPropertyName == UNO_NAME_REDLINE_TEXT)
    {
        SwNodeIndex* pNodeIdx = pRedline->GetContentIdx();
        if( pNodeIdx )
        {
            if ( 1 < ( pNodeIdx->GetNode().EndOfSectionIndex() - pNodeIdx->GetNode().GetIndex() ) )
            {
                uno::Reference<text::XText> xRet = new SwXRedlineText(pDoc, *pNodeIdx);
                aRet <<= xRet;
            }
            else {
                OSL_FAIL("Empty section in redline portion! (end node immediately follows start node)");
            }
        }
    }
    else
        aRet = SwXRedlinePortion::GetPropertyValue(rPropertyName, *pRedline);
    return aRet;
}

void SwXRedline::addPropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
}

void SwXRedline::removePropertyChangeListener(
    const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
}

void SwXRedline::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
}

void SwXRedline::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
}

void SwXRedline::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
      {
        pDoc = nullptr;
        pRedline = nullptr;
    }
}

uno::Reference< container::XEnumeration >  SwXRedline::createEnumeration()
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration > xRet;
    if(!pDoc)
        throw uno::RuntimeException();

    SwNodeIndex* pNodeIndex = pRedline->GetContentIdx();
    if(!pNodeIndex)
        return nullptr;
    SwPaM aPam(*pNodeIndex);
    aPam.Move(fnMoveForward, GoInNode);
    auto pUnoCursor(GetDoc()->CreateUnoCursor(*aPam.Start()));
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CursorType::Redline);
}

uno::Type SwXRedline::getElementType(  )
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SwXRedline::hasElements(  )
{
    if(!pDoc)
        throw uno::RuntimeException();
    return nullptr != pRedline->GetContentIdx();
}

uno::Reference< text::XTextCursor >  SwXRedline::createTextCursor()
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();

    uno::Reference< text::XTextCursor >     xRet;
    SwNodeIndex* pNodeIndex = pRedline->GetContentIdx();
    if(pNodeIndex)
    {
        SwPosition aPos(*pNodeIndex);
        SwXTextCursor *const pXCursor =
            new SwXTextCursor(*pDoc, this, CursorType::Redline, aPos);
        auto& rUnoCursor(pXCursor->GetCursor());
        rUnoCursor.Move(fnMoveForward, GoInNode);

        // is here a table?
        SwTableNode* pTableNode = rUnoCursor.GetNode().FindTableNode();
        SwContentNode* pCont = nullptr;
        while( pTableNode )
        {
            rUnoCursor.GetPoint()->nNode = *pTableNode->EndOfSectionNode();
            pCont = GetDoc()->GetNodes().GoNext(&rUnoCursor.GetPoint()->nNode);
            pTableNode = pCont->FindTableNode();
        }
        if(pCont)
            rUnoCursor.GetPoint()->nContent.Assign(pCont, 0);
        xRet = static_cast<text::XWordCursor*>(pXCursor);
    }
    else
    {
        throw uno::RuntimeException();
    }
    return xRet;
}

uno::Reference< text::XTextCursor >  SwXRedline::createTextCursorByRange(
    const uno::Reference< text::XTextRange > & /*aTextPosition*/)
{
    throw uno::RuntimeException();
}

uno::Any SwXRedline::queryInterface( const uno::Type& rType )
{
    uno::Any aRet = SwXText::queryInterface(rType);
    if(!aRet.hasValue())
    {
        aRet = SwXRedlineBaseClass::queryInterface(rType);
    }
    return aRet;
}

uno::Sequence<uno::Type> SwXRedline::getTypes()
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

uno::Sequence<sal_Int8> SwXRedline::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
