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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/debug.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include <unoredlines.hxx>
#include <unoredline.hxx>
#include <unomid.h>
#include <pagedesc.hxx>
#include "poolfmt.hxx"
#include <doc.hxx>
#include <docary.hxx>
#include <redline.hxx>


using namespace ::com::sun::star;
using ::rtl::OUString;

/*-- 11.01.01 15:28:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlines::SwXRedlines(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXRedlines::~SwXRedlines()
{
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXRedlines::getCount(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count();
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXRedlines::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    uno::Any aRet;
    if(rRedTbl.Count() > nIndex && nIndex >= 0)
    {
        uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject((sal_uInt16)nIndex), *GetDoc() );
        aRet <<= xRet;
    }
    else
        throw lang::IndexOutOfBoundsException();
    return aRet;
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXRedlines::createEnumeration(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >(new SwXRedlineEnumeration(*GetDoc()));
}
/*-- 11.01.01 15:28:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXRedlines::getElementType(  ) throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<beans::XPropertySet>*)0);
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXRedlines::hasElements(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count() > 0;
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXRedlines::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXRedlines");
}
/*-- 11.01.01 15:28:56---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXRedlines::supportsService(const rtl::OUString& /*ServiceName*/)
    throw( uno::RuntimeException )
{
    DBG_ERROR("not implemented");
    return sal_False;
}
/*-- 11.01.01 15:28:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXRedlines::getSupportedServiceNames(void)
    throw( uno::RuntimeException )
{
    DBG_ERROR("not implemented");
    return uno::Sequence< OUString >();
}
/*-- 11.01.01 15:28:57---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::XPropertySet*    SwXRedlines::GetObject( SwRedline& rRedline, SwDoc& rDoc )
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
sal_Bool SwXRedlineEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    if(!pDoc)
        throw uno::RuntimeException();
    return pDoc->GetRedlineTbl().Count() > nCurrentIndex;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXRedlineEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    if(!pDoc)
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = pDoc->GetRedlineTbl();
    if(!(rRedTbl.Count() > nCurrentIndex))
        throw container::NoSuchElementException();
    uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject(nCurrentIndex++), *pDoc );
    uno::Any aRet;
    aRet <<= xRet;
    return aRet;
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
rtl::OUString SwXRedlineEnumeration::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXRedlineEnumeration");
}
/*-- 12.01.01 15:06:10---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXRedlineEnumeration::supportsService(const rtl::OUString& /*ServiceName*/) throw( uno::RuntimeException )
{
    return sal_False;
}
/*-- 12.01.01 15:06:11---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXRedlineEnumeration::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    return uno::Sequence< OUString >();
}
/*-- 12.01.01 15:06:11---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXRedlineEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        pDoc = 0;
}
