/*************************************************************************
 *
 *  $RCSfile: unoredline.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-08 14:14:56 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#include "poolfmt.hxx"
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOREDLINE_HXX
#include <unoredline.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLE_HPP_
#include <com/sun/star/text/XTextTable.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;


/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwXRedlineText::SwXRedlineText(SwDoc* pDoc, SwNodeIndex aIndex) :
    SwXText(pDoc, CURSOR_REDLINE),
    aNodeIndex(aIndex)
{
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
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
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
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
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
Sequence<sal_Int8> SwXRedlineText::getImplementationId()
    throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    return aId;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
Reference<XTextCursor> SwXRedlineText::createTextCursor(void)
    throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwPosition aPos(aNodeIndex);
    SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, CURSOR_REDLINE,
                                             GetDoc());
    SwUnoCrsr* pUnoCursor = pCrsr->GetCrsr();
    pUnoCursor->Move(fnMoveForward, fnGoNode);
    return (XWordCursor*)pCrsr;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
Reference<XTextCursor> SwXRedlineText::createTextCursorByRange(
    const Reference<XTextRange> & aTextRange)
        throw( RuntimeException )
{
    Reference<XTextCursor> xCursor = createTextCursor();
    xCursor->gotoRange(aTextRange->getStart(), sal_False);
    xCursor->gotoRange(aTextRange->getEnd(), sal_True);
    return xCursor;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
Reference<XEnumeration> SwXRedlineText::createEnumeration(void)
    throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwPaM aPam(aNodeIndex);
    aPam.Move(fnMoveForward, fnGoNode);
    return new SwXParagraphEnumeration(this, *aPam.Start(), CURSOR_REDLINE);
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
uno::Type SwXRedlineText::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXRedlineText::hasElements(  ) throw(RuntimeException)
{
    return sal_True;    // we always have a content index
}
/* -----------------------------19.12.00 11:36--------------------------------

 ---------------------------------------------------------------------------*/
SwXRedlinePortion::SwXRedlinePortion(   const SwRedline* pRed,
                        const SwUnoCrsr* pPortionCrsr,
                        Reference< XText >  xParent, BOOL bStart) :
    SwXTextPortion(pPortionCrsr, xParent, bStart ? PORTION_REDLINE_START : PORTION_REDLINE_END),
//  SwXText(pPortionCrsr->GetDoc(), CURSOR_REDLINE),
//  SwXRedlineText(pPortionCrsr->GetDoc(), *pRed->GetContentIdx()),
    pRedline(pRed)
{
    SetCollapsed(!pRedline->HasMark());
}
/*-- 19.12.00 11:37:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlinePortion::~SwXRedlinePortion()
{
}
/* -----------------------------19.12.00 11:46--------------------------------

 ---------------------------------------------------------------------------*/
util::DateTime lcl_DateTimeToUno(const DateTime& rDT)
{
    util::DateTime aRetDT;
    aRetDT.Year = rDT.GetYear();
    aRetDT.Month= rDT.GetMonth();
    aRetDT.Day      = rDT.GetDay();
    aRetDT.Hours    = rDT.GetHour();
    aRetDT.Minutes = rDT.GetMin();
    aRetDT.Seconds = rDT.GetSec();
    aRetDT.HundredthSeconds = rDT.Get100Sec();
    return aRetDT;
}
// ---------------------------------------------------------------------------
DateTime lcl_DateTimeFromUno(const util::DateTime& rDT)
{
    DateTime aRetDT;
    aRetDT.SetYear(rDT.Year);
    aRetDT.SetMonth(rDT.Month);
    aRetDT.SetDay(rDT.Day);
    aRetDT.SetHour(rDT.Hours);
    aRetDT.SetMin(rDT.Minutes);
    aRetDT.SetSec(rDT.Seconds);
    aRetDT.Set100Sec(rDT.HundredthSeconds);
    return aRetDT;
}
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
SwRedlineType  lcl_OUStringToRedlineType(const OUString& rType)
{
    SwRedlineType eType = REDLINE_INSERT;
    if(!rType.compareToAscii("Delete"))
        eType = REDLINE_DELETE;
    else if(!rType.compareToAscii("Format"))
        eType = REDLINE_FORMAT;
    else if(!rType.compareToAscii("TextTable"))
        eType = REDLINE_TABLE;
    else if(!rType.compareToAscii("Style"))
        eType = REDLINE_FMTCOLL;
//  else if(!rType.compareToAscii("Insert"))
    return eType;
}
// ---------------------------------------------------------------------------
Sequence<PropertyValue> lcl_GetSuccessorProperties(const SwRedline& rRedline)
{
    Sequence<PropertyValue> aValues(4);

    const SwRedlineData* pNext = rRedline.GetRedlineData().Next();
    if(pNext)
    {
        PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = C2U(UNO_NAME_REDLINE_AUTHOR);
        // GetAuthorString(n) walks the SwRedlineData* chain;
        // here we always need element 1
        pValues[0].Value <<= OUString(rRedline.GetAuthorString(1));
        pValues[1].Name = C2U(UNO_NAME_REDLINE_DATE_TIME);
        pValues[1].Value <<= lcl_DateTimeToUno(pNext->GetTimeStamp());
        pValues[2].Name = C2U(UNO_NAME_REDLINE_COMMENT);
        pValues[2].Value <<= OUString(pNext->GetComment());
        pValues[3].Name = C2U(UNO_NAME_REDLINE_TYPE);
        pValues[3].Value <<= lcl_RedlineTypeToOUString(pNext->GetType());
    }
    return aValues;
}
// ---------------------------------------------------------------------------
Any SwXRedlinePortion::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Validate();
    Any aRet;
    if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_TEXT.pName, UNO_NAME_REDLINE_TEXT.nNameLen))
    {
        SwNodeIndex* pNodeIdx = pRedline->GetContentIdx();
        if(pNodeIdx)
        {
            SwUnoCrsr* pUnoCrsr = GetCrsr();
            Reference<XText> xRet = new SwXRedlineText(pUnoCrsr->GetDoc(), *pNodeIdx);
            aRet <<= xRet;
        }
    }
    else
    {
        aRet = GetPropertyValue( rPropertyName, *pRedline);
        if(!aRet.hasValue() &&
           ! rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_SUCCESSOR_DATA.pName,
                                     UNO_NAME_REDLINE_SUCCESSOR_DATA.nNameLen))
            aRet = SwXTextPortion::getPropertyValue(rPropertyName);
    }
    return aRet;
}
/* -----------------------------19.12.00 15:16--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------21.03.00 15:39--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXRedlinePortion::getImplementationId(  ) throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId( 16 );
    static BOOL bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    return aId;
}
/* -----------------------------11.01.01 16:39--------------------------------

 ---------------------------------------------------------------------------*/
Any  SwXRedlinePortion::GetPropertyValue( const OUString& rPropertyName, const SwRedline& rRedline ) throw()
{
    Any aRet;
    if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_AUTHOR       .pName, UNO_NAME_REDLINE_AUTHOR.nNameLen))
        aRet <<= OUString(rRedline.GetAuthorString());
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_DATE_TIME.pName, UNO_NAME_REDLINE_DATE_TIME.nNameLen))
    {
        aRet <<= lcl_DateTimeToUno(rRedline.GetTimeStamp());
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_COMMENT  .pName, UNO_NAME_REDLINE_COMMENT  .nNameLen))
        aRet <<= OUString(rRedline.GetComment());
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_TYPE        .pName, UNO_NAME_REDLINE_TYPE     .nNameLen))
    {
        aRet <<= lcl_RedlineTypeToOUString(rRedline.GetType());
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_SUCCESSOR_DATA.pName, UNO_NAME_REDLINE_SUCCESSOR_DATA.nNameLen))
    {
        if(rRedline.GetRedlineData().Next())
            aRet <<= lcl_GetSuccessorProperties(rRedline);
    }
    else if (rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_IDENTIFIER.pName,
                                        UNO_NAME_REDLINE_IDENTIFIER.nNameLen))
    {
        OUStringBuffer sBuf;
        sBuf.append((sal_Int64)&rRedline);
        aRet <<= sBuf.makeStringAndClear();
    }
    else if (rPropertyName.equalsAsciiL(UNO_NAME_IS_IN_HEADER_FOOTER.pName,
                                        UNO_NAME_IS_IN_HEADER_FOOTER.nNameLen))
    {
        sal_Bool bRet =
            rRedline.GetDoc()->IsInHeaderFooter( rRedline.GetPoint()->nNode );
        aRet.setValue(&bRet, ::getBooleanCppuType());
    }
    return aRet;
}
/* -----------------------------11.01.01 11:22--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< PropertyValue > SwXRedlinePortion::CreateRedlineProperties( const SwRedline& rRedline ) throw()
{
    Sequence< PropertyValue > aRet;
    const SwRedlineData* pNext = rRedline.GetRedlineData().Next();
    aRet.realloc( pNext ? 6 : 5 );
    PropertyValue* pRet = aRet.getArray();

    OUStringBuffer sRedlineIdBuf;
    sRedlineIdBuf.append((sal_Int64)&rRedline);

    pRet[0].Name = C2U(UNO_NAME_REDLINE_AUTHOR.pName);
    pRet[0].Value <<= OUString(rRedline.GetAuthorString());
    pRet[1].Name = C2U(UNO_NAME_REDLINE_DATE_TIME.pName);
    pRet[1].Value <<= lcl_DateTimeToUno(rRedline.GetTimeStamp());
    pRet[2].Name = C2U(UNO_NAME_REDLINE_COMMENT  .pName);
    pRet[2].Value <<= OUString(rRedline.GetComment());
    pRet[3].Name = C2U(UNO_NAME_REDLINE_TYPE        .pName);
    pRet[3].Value <<= lcl_RedlineTypeToOUString(rRedline.GetType());
    pRet[4].Name = C2U(UNO_NAME_REDLINE_IDENTIFIER.pName);
    pRet[4].Value <<= sRedlineIdBuf.makeStringAndClear();
    if(pNext)
    {
        pRet[5].Name = C2U(UNO_NAME_REDLINE_SUCCESSOR_DATA.pName);
        pRet[5].Value <<= lcl_GetSuccessorProperties(rRedline);
    }
    return aRet;
}
/*-- 11.01.01 17:06:07---------------------------------------------------

  -----------------------------------------------------------------------*/
TYPEINIT1(SwXRedline, SwClient);
SwXRedline::SwXRedline(SwRedline& rRedline, SwDoc& rDoc) :
    pDoc(&rDoc),
    SwXText(&rDoc, CURSOR_REDLINE),
    pRedline(&rRedline)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}
/*-- 11.01.01 17:06:08---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedline::~SwXRedline()
{
}
/*-- 11.01.01 17:06:08---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo > SwXRedline::getPropertySetInfo(  ) throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  xRef =
        SfxItemPropertySet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_REDLINE)).getPropertySetInfo();
    return xRef;
}
/*-- 11.01.01 17:06:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedline::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
        WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw RuntimeException();
    if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_AUTHOR       .pName, UNO_NAME_REDLINE_AUTHOR.nNameLen))
    {
        DBG_ERROR("currently not available")
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_DATE_TIME.pName, UNO_NAME_REDLINE_DATE_TIME.nNameLen))
    {
        DBG_ERROR("currently not available")
//      util::DateTime aDT;
//      if(aValue >>= aDT)
//              pRedline->SetTimeStamp(lcl_DateTimeFromUno(aDT));
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_COMMENT  .pName, UNO_NAME_REDLINE_COMMENT  .nNameLen))
    {
        OUString sTmp; aValue >>= sTmp;
        pRedline->SetComment(sTmp);
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_TYPE        .pName, UNO_NAME_REDLINE_TYPE     .nNameLen))
    {
        DBG_ERROR("currently not available")
        OUString sTmp; aValue >>= sTmp;
        if(!sTmp.getLength())
            throw IllegalArgumentException();
//      pRedline->SetType(lcl_OUStringToRedlineType(sTmp));
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_SUCCESSOR_DATA.pName, UNO_NAME_REDLINE_SUCCESSOR_DATA.nNameLen))
    {
        DBG_ERROR("currently not available")
/*      SwRedlineData* pNext = pRedline->GetRedlineData().Next();
        Sequence<PropertyValue> aValues;
        if(!(aValue =>> aValues) || !pNext)
            throw IllegalArgumentException();

        const PropertyValue* pValues = aValues.getConstArray();
        for(sal_Int32 nValue = 0; nValue < aValues.getLength(); nValue++)
        {
            if(pValues[nValue].Name.equalsAscii(UNO_NAME_REDLINE_AUTHOR.pName)
            {
                DBG_ERROR("currently not available")
            }
            else if(pValues[nValue].Name.equalsAscii(UNO_NAME_REDLINE_DATE_TIME.pName))
            {
                util::DateTime aDT;
                if(pValues[nValue].Value >>= aDT)
                    pNext->SetTimeStamp(lcl_DateTimeFromUno(aDT));
            }
            else if(pValues[nValue].Name.equalsAscii(UNO_NAME_REDLINE_COMMENT.pName))
            {
                OUString sTmp; pValues[nValue].Value >>= sTmp;
                pNext->SetComment(sTmp);
            }
            else if(pValues[nValue].Name.equalsAscii(UNO_NAME_REDLINE_TYPE.pName))
            {
                OUString sTmp; pValues[nValue].Value >>= sTmp;
                pNext->SetType(lcl_OUStringToRedlineType(sTmp);
            }
        }
*/  }
    else
    {
        throw IllegalArgumentException();
    }
}
/*-- 11.01.01 17:06:08---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXRedline::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw RuntimeException();
    Any aRet;
    BOOL bStart = rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_START     .pName, UNO_NAME_REDLINE_START.nNameLen);
    if(bStart ||
        rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_END     .pName, UNO_NAME_REDLINE_END.nNameLen))
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
                DBG_ASSERT(pSectNode, "No section node!")
                xRet = SwXTextSections::GetObject( *pSectNode->GetSection().GetFmt() );
            }
            break;
            case ND_TABLENODE :
            {
                SwTableNode* pTblNode = pNode->GetTableNode();
                DBG_ASSERT(pTblNode, "No table node!")
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
//              Reference< XTextRange > xRange = SwXTextRange::createTextRangeFromPaM(aTmp);
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
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_TEXT.pName, UNO_NAME_REDLINE_TEXT.nNameLen))
    {
        SwNodeIndex* pNodeIdx = pRedline->GetContentIdx();
        if(pNodeIdx)
        {
            Reference<XText> xRet = new SwXRedlineText(pDoc, *pNodeIdx);
            aRet <<= xRet;
        }
    }
    else
        aRet = SwXRedlinePortion::GetPropertyValue(rPropertyName, *pRedline);
    return aRet;
}
/*-- 11.01.01 17:06:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedline::addPropertyChangeListener(
    const OUString& aPropertyName,
    const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 11.01.01 17:06:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedline::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 11.01.01 17:06:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedline::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 11.01.01 17:06:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedline::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 11.01.01 17:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedline::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
      {
        pDoc = 0;
        pRedline = 0;
    }
}
/*-- 19.12.00 11:37:25---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XEnumeration >  SwXRedline::createEnumeration(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/* -----------------------------19.12.00 12:34--------------------------------

 ---------------------------------------------------------------------------*/
uno::Type SwXRedline::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/* -----------------------------19.12.00 12:34--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXRedline::hasElements(  ) throw(RuntimeException)
{
    if(!pDoc)
        throw RuntimeException();
    return 0 != pRedline->GetContentIdx();
}
/* -----------------------------19.12.00 15:11--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XTextCursor >  SwXRedline::createTextCursor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw RuntimeException();

    uno::Reference< XTextCursor >   xRet;
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
/* -----------------------------19.12.00 15:11--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XTextCursor >  SwXRedline::createTextCursorByRange(
    const Reference< XTextRange > & aTextPosition)
        throw( RuntimeException )
{
    throw RuntimeException();
    return Reference< XTextCursor >  ();
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
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
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
Sequence<Type> SwXRedline::getTypes()
    throw(RuntimeException)
{
    uno::Sequence<uno::Type> aTypes = SwXText::getTypes();
    uno::Sequence<uno::Type> aBaseTypes = SwXRedlineBaseClass::getTypes();
    const uno::Type* pBaseTypes = aBaseTypes.getConstArray();
    aTypes.realloc(aTypes.getLength() + aBaseTypes.getLength());
    uno::Type* pTypes = aTypes.getArray();
    sal_Int32 nCurType = aTypes.getLength();
    for(sal_Int32 nType = 0; nType < aBaseTypes.getLength(); nType++)
        pTypes[nCurType++] = pBaseTypes[nType];
    return aTypes;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
Sequence<sal_Int8> SwXRedline::getImplementationId()
    throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    return aId;
}

