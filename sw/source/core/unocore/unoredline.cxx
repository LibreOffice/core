/*************************************************************************
 *
 *  $RCSfile: unoredline.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2001-01-02 15:11:40 $
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
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
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

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

/* -----------------------------19.12.00 11:36--------------------------------

 ---------------------------------------------------------------------------*/
SwXRedlinePortion::SwXRedlinePortion(   const SwRedline* pRed,
                        const SwUnoCrsr* pPortionCrsr,
                        Reference< XText >  xParent, BOOL bStart) :
    SwXTextPortion(pPortionCrsr, xParent, bStart ? PORTION_REDLINE_START : PORTION_REDLINE_END),
    SwXText(pPortionCrsr->GetDoc(), CURSOR_REDLINE),
    pRedline(pRed)
{
    SetCollapsed(!pRedline->HasMark());
}
/*-- 19.12.00 11:37:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlinePortion::~SwXRedlinePortion()
{
}
/*-- 19.12.00 11:37:24---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXRedlinePortion::queryInterface( const uno::Type& rType ) throw(RuntimeException)
{
    Any aRet = SwXTextPortion::queryInterface(rType);
    if(!aRet.hasValue() &&
        IsCollapsed())
    {
        aRet = SwXText::queryInterface(rType);
        if(!aRet.hasValue())
        {
            if(rType == ::getCppuType((Reference<XEnumerationAccess>*)0))
            {
                Reference<XEnumerationAccess> xTmp = this;
                aRet <<= xTmp;
            }
            else if(rType == ::getCppuType((Reference<XElementAccess>*)0))
            {
                Reference<XElementAccess> xTmp = this;
                aRet <<= xTmp;
            }
        }
    }
    return aRet;
}
/*-- 19.12.00 11:37:25---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< Type > SwXRedlinePortion::getTypes(  ) throw(RuntimeException)
{
    uno::Sequence< uno::Type > aTypes = SwXTextPortion::getTypes();
    if(!IsCollapsed())
        return aTypes;
    uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    long nIndex = aTypes.getLength();
    aTypes.realloc(aTypes.getLength() + aTextTypes.getLength() + 1);
    uno::Type* pTypes = aTypes.getArray();
    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    for(int i = 0; i < aTextTypes.getLength(); i++)
        pTypes[nIndex++] = pTextTypes[i];
    pTypes[nIndex++] = ::getCppuType((Reference<XEnumerationAccess>*)0);
    return aTypes;
}
/*-- 19.12.00 11:37:25---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XEnumeration >  SwXRedlinePortion::createEnumeration(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Reference< XEnumeration > xRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    Validate();

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
uno::Type SwXRedlinePortion::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/* -----------------------------19.12.00 12:34--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXRedlinePortion::hasElements(  ) throw(RuntimeException)
{
    return TRUE;
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
Any SwXRedlinePortion::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    Validate();

    Any aRet;
    if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_AUTHOR       .pName, UNO_NAME_REDLINE_AUTHOR.nNameLen))
        aRet <<= OUString(pRedline->GetAuthorString( pRedline->GetAuthor()));
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_DATE_TIME.pName, UNO_NAME_REDLINE_DATE_TIME.nNameLen))
    {
        aRet <<= lcl_DateTimeToUno(pRedline->GetTimeStamp());
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_COMMENT  .pName, UNO_NAME_REDLINE_COMMENT  .nNameLen))
        aRet <<= OUString(pRedline->GetComment());
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_TYPE        .pName, UNO_NAME_REDLINE_TYPE     .nNameLen))
    {
        aRet <<= lcl_RedlineTypeToOUString(pRedline->GetType());
    }
    else if(rPropertyName.equalsAsciiL(UNO_NAME_REDLINE_SUCCESSOR_DATA.pName, UNO_NAME_REDLINE_SUCCESSOR_DATA.nNameLen))
    {
        const SwRedlineData* pNext = pRedline->GetRedlineData().Next();
        if(pNext)
        {
            Sequence<PropertyValue> aValues(4);
            PropertyValue* pValues = aValues.getArray();
            pValues[0].Name = C2U(UNO_NAME_REDLINE_AUTHOR);
            pValues[0].Value <<= OUString(pRedline->GetAuthorString( pNext->GetAuthor()));
            pValues[1].Name = C2U(UNO_NAME_REDLINE_DATE_TIME);
            pValues[1].Value <<= lcl_DateTimeToUno(pNext->GetTimeStamp());
            pValues[2].Name = C2U(UNO_NAME_REDLINE_COMMENT);
            pValues[2].Value <<= OUString(pNext->GetComment());
            pValues[3].Name = C2U(UNO_NAME_REDLINE_TYPE);
            pValues[3].Value <<= lcl_RedlineTypeToOUString(pNext->GetType());
            aRet <<= aValues;
        }
    }
    else
        aRet = SwXTextPortion::getPropertyValue(rPropertyName);
    return aRet;
}
/* -----------------------------19.12.00 15:11--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XTextCursor >  SwXRedlinePortion::createTextCursor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    Validate();

    uno::Reference< XTextCursor >   xRet;
    SwNodeIndex* pNodeIndex = pRedline->GetContentIdx();
    if(pNodeIndex)
    {
        SwPosition aPos(*pNodeIndex);
        SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, CURSOR_REDLINE, pUnoCrsr->GetDoc());
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
Reference< XTextCursor >  SwXRedlinePortion::createTextCursorByRange(const Reference< XTextRange > & aTextPosition)
    throw( RuntimeException )
{
    throw RuntimeException();
    return Reference< XTextCursor >  ();
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

