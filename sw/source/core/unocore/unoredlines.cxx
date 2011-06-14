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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <unoredlines.hxx>
#include <unoredline.hxx>
#include <unomid.h>
#include <pagedesc.hxx>
#include "poolfmt.hxx"
#include <doc.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <switerator.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

SwXRedlines::SwXRedlines(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXRedlines::~SwXRedlines()
{
}

sal_Int32 SwXRedlines::getCount(  ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count();
}

uno::Any SwXRedlines::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

uno::Reference< container::XEnumeration >  SwXRedlines::createEnumeration(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >(new SwXRedlineEnumeration(*GetDoc()));
}

uno::Type SwXRedlines::getElementType(  ) throw(uno::RuntimeException)
{
    return ::getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SwXRedlines::hasElements(  ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
    return rRedTbl.Count() > 0;
}

OUString SwXRedlines::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXRedlines");
}

sal_Bool SwXRedlines::supportsService(const rtl::OUString& /*ServiceName*/)
    throw( uno::RuntimeException )
{
    OSL_FAIL("not implemented");
    return sal_False;
}

uno::Sequence< OUString > SwXRedlines::getSupportedServiceNames(void)
    throw( uno::RuntimeException )
{
    OSL_FAIL("not implemented");
    return uno::Sequence< OUString >();
}

beans::XPropertySet*    SwXRedlines::GetObject( SwRedline& rRedline, SwDoc& rDoc )
{
    SwPageDesc* pStdDesc = rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
    SwIterator<SwXRedline,SwPageDesc> aIter(*pStdDesc);
    SwXRedline* pxRedline = aIter.First();
    while(pxRedline)
    {
        if(pxRedline->GetRedline() == &rRedline)
            break;
        pxRedline = aIter.Next();
    }
    if( !pxRedline )
        pxRedline = new SwXRedline(rRedline, rDoc);
    return pxRedline;
}

SwXRedlineEnumeration::SwXRedlineEnumeration(SwDoc& rDoc) :
    pDoc(&rDoc),
    nCurrentIndex(0)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXRedlineEnumeration::~SwXRedlineEnumeration()
{
}

sal_Bool SwXRedlineEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    if(!pDoc)
        throw uno::RuntimeException();
    return pDoc->GetRedlineTbl().Count() > nCurrentIndex;
}

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

rtl::OUString SwXRedlineEnumeration::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXRedlineEnumeration");
}

sal_Bool SwXRedlineEnumeration::supportsService(const rtl::OUString& /*ServiceName*/) throw( uno::RuntimeException )
{
    return sal_False;
}

uno::Sequence< OUString > SwXRedlineEnumeration::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    return uno::Sequence< OUString >();
}

void SwXRedlineEnumeration::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        pDoc = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
