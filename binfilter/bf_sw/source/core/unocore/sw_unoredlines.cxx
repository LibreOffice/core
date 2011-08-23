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

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _CALBCK_HXX
#include "calbck.hxx"
#endif

#ifndef _UNOREDLINES_HXX
#include <unoredlines.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOREDLINE_HXX
#include <unoredline.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#include "poolfmt.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

/*-- 11.01.01 15:28:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlines::SwXRedlines(SwDoc* pDoc) :
    SwUnoCollection(pDoc)
{
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlines::~SwXRedlines()
{
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXRedlines::getCount(  ) throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count();
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXRedlines::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    Any aRet;
    if(rRedTbl.Count() > nIndex && nIndex >= 0)
    {
        Reference <XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject((USHORT)nIndex), *GetDoc() );
        aRet <<= xRet;
    }
    else
        throw IndexOutOfBoundsException();
    return aRet;
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XEnumeration >  SwXRedlines::createEnumeration(void)
    throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return Reference< XEnumeration >(new SwXRedlineEnumeration(*GetDoc()));
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Type SwXRedlines::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((Reference<XPropertySet>*)0);
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXRedlines::hasElements(  ) throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count() > 0;
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXRedlines::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXRedlines");
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL SwXRedlines::supportsService(const ::rtl::OUString& ServiceName)
    throw( RuntimeException )
{
    DBG_ERROR("not implemented");
    return FALSE;
}
/*-- 11.01.01 15:28:57---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXRedlines::getSupportedServiceNames(void)
    throw( RuntimeException )
{
    DBG_ERROR("not implemented");
    return Sequence< OUString >();
}
/*-- 11.01.01 15:28:57---------------------------------------------------

  -----------------------------------------------------------------------*/
XPropertySet* 	SwXRedlines::GetObject( SwRedline& rRedline, SwDoc& rDoc )
{
    SwPageDesc* pStdDesc = rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
    SwClientIter aIter(*pStdDesc);
    SwXRedline* pxRedline = (SwXRedline*)aIter.First( TYPE( SwXRedline ));
    while(pxRedline)
    {
        if(pxRedline->GetRedline() == &rRedline)
            break;
        pxRedline = (SwXRedline*)aIter.Next();
    }
    if( !pxRedline )
        pxRedline = new SwXRedline(rRedline, rDoc);
    return pxRedline;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlineEnumeration::SwXRedlineEnumeration(SwDoc& rDoc) :
    pDoc(&rDoc),
    nCurrentIndex(0)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlineEnumeration::~SwXRedlineEnumeration()
{
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL SwXRedlineEnumeration::hasMoreElements(void) throw( RuntimeException )
{
    if(!pDoc)
        throw RuntimeException();
    return pDoc->GetRedlineTbl().Count() > nCurrentIndex;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXRedlineEnumeration::nextElement(void)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    if(!pDoc)
        throw RuntimeException();
    const SwRedlineTbl& rRedTbl = pDoc->GetRedlineTbl();
    if(!(rRedTbl.Count() > nCurrentIndex))
        throw NoSuchElementException();
    Reference <XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject(nCurrentIndex++), *pDoc );
    Any aRet;
    aRet <<= xRet;
    return aRet;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
rtl::OUString SwXRedlineEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXRedlineEnumeration");
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL SwXRedlineEnumeration::supportsService(const ::rtl::OUString& ServiceName) throw( RuntimeException )
{
    return FALSE;
}
/*-- 12.01.01 15:06:11---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXRedlineEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    return Sequence< OUString >();
}
/*-- 12.01.01 15:06:11---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedlineEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        pDoc = 0;
}
}
