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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <vcl/svapp.hxx>
#include <osl/diagnose.h>

#include <unoredlines.hxx>
#include <unoredline.hxx>
#include <unomid.h>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <doc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docary.hxx>
#include <redline.hxx>

using namespace ::com::sun::star;

SwXRedlines::SwXRedlines(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXRedlines::~SwXRedlines()
{
}

sal_Int32 SwXRedlines::getCount(  )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTable& rRedTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    return rRedTable.size();
}

uno::Any SwXRedlines::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTable& rRedTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    if ((rRedTable.size() <= static_cast<size_t>(nIndex)) || (nIndex < 0))
        throw lang::IndexOutOfBoundsException();

    uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTable[nIndex], *GetDoc() );
    return uno::Any(xRet);
}

uno::Reference< container::XEnumeration >  SwXRedlines::createEnumeration()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >(new SwXRedlineEnumeration(*GetDoc()));
}

uno::Type SwXRedlines::getElementType(  )
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SwXRedlines::hasElements(  )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    const SwRedlineTable& rRedTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    return !rRedTable.empty();
}

OUString SwXRedlines::getImplementationName()
{
    return OUString("SwXRedlines");
}

sal_Bool SwXRedlines::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SwXRedlines::getSupportedServiceNames()
{
    OSL_FAIL("not implemented");
    return uno::Sequence< OUString >();
}

beans::XPropertySet* SwXRedlines::GetObject( SwRangeRedline& rRedline, SwDoc& rDoc )
{
    SwXRedline* pXRedline(nullptr);
    sw::FindRedlineHint aHint(rRedline, &pXRedline);
    rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier().Broadcast(aHint);
    return pXRedline ? pXRedline : new SwXRedline(rRedline, rDoc);
}

SwXRedlineEnumeration::SwXRedlineEnumeration(SwDoc& rDoc) :
    pDoc(&rDoc),
    nCurrentIndex(0)
{
    StartListening(pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier());
}

SwXRedlineEnumeration::~SwXRedlineEnumeration()
{
}

sal_Bool SwXRedlineEnumeration::hasMoreElements()
{
    if(!pDoc)
        throw uno::RuntimeException();
    return pDoc->getIDocumentRedlineAccess().GetRedlineTable().size() > nCurrentIndex;
}

uno::Any SwXRedlineEnumeration::nextElement()
{
    if(!pDoc)
        throw uno::RuntimeException();
    const SwRedlineTable& rRedTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    if( rRedTable.size() <= nCurrentIndex )
        throw container::NoSuchElementException();
    uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTable[nCurrentIndex++], *pDoc );
    uno::Any aRet;
    aRet <<= xRet;
    return aRet;
}

OUString SwXRedlineEnumeration::getImplementationName()
{
    return OUString("SwXRedlineEnumeration");
}

sal_Bool SwXRedlineEnumeration::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SwXRedlineEnumeration::getSupportedServiceNames()
{
    return uno::Sequence< OUString >();
}

void SwXRedlineEnumeration::Notify( const SfxHint& rHint )
{
    if(rHint.GetId() == SfxHintId::Dying)
        pDoc = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
