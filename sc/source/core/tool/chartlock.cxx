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

#include <vcl/svapp.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

#include <chartlock.hxx>
#include <document.hxx>
#include <drwlayer.hxx>

#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;

#define SC_CHARTLOCKTIMEOUT 660

namespace
{

std::vector< WeakReference< frame::XModel > > lcl_getAllLivingCharts( ScDocument* pDoc )
{
    std::vector< WeakReference< frame::XModel > > aRet;
    if( !pDoc )
        return aRet;
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    if (!pDrawLayer)
        return aRet;

    for (SCTAB nTab=0; nTab<=pDoc->GetMaxTableNumber(); nTab++)
    {
        if (pDoc->HasTable(nTab))
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");

            SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if( ScDocument::IsChart( pObject ) )
                {
                    uno::Reference< embed::XEmbeddedObject > xIPObj = static_cast<SdrOle2Obj*>(pObject)->GetObjRef();
                    uno::Reference< embed::XComponentSupplier > xCompSupp( xIPObj, uno::UNO_QUERY );
                    if( xCompSupp.is())
                    {
                        Reference< frame::XModel > xModel( xCompSupp->getComponent(), uno::UNO_QUERY );
                        if( xModel.is() )
                            aRet.emplace_back(xModel );
                    }
                }
                pObject = aIter.Next();
            }
        }
    }
    return aRet;
}

}//end anonymous namespace

// ScChartLockGuard
ScChartLockGuard::ScChartLockGuard( ScDocument* pDoc ) :
    maChartModels( lcl_getAllLivingCharts( pDoc ) )
{
    for( const auto& rxChartModel : maChartModels )
    {
        try
        {
            Reference< frame::XModel > xModel( rxChartModel );
            if( xModel.is())
                xModel->lockControllers();
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Unexpected exception in ScChartLockGuard");
        }
    }
}

ScChartLockGuard::~ScChartLockGuard()
{
    for( const auto& rxChartModel : maChartModels )
    {
        try
        {
            Reference< frame::XModel > xModel( rxChartModel );
            if( xModel.is())
                xModel->unlockControllers();
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Unexpected exception in ScChartLockGuard");
        }
    }
}

void ScChartLockGuard::AlsoLockThisChart( const Reference< frame::XModel >& xModel )
{
    if(!xModel.is())
        return;

    WeakReference< frame::XModel > xWeakModel(xModel);

    std::vector< WeakReference< frame::XModel > >::iterator aFindIter(
            ::std::find( maChartModels.begin(), maChartModels.end(), xWeakModel ) );

    if( aFindIter == maChartModels.end() )
    {
        try
        {
            xModel->lockControllers();
            maChartModels.emplace_back(xModel );
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Unexpected exception in ScChartLockGuard");
        }
    }
}

// ScTemporaryChartLock
ScTemporaryChartLock::ScTemporaryChartLock( ScDocument* pDocP ) :
    mpDoc( pDocP )
{
    maTimer.SetTimeout( SC_CHARTLOCKTIMEOUT );
    maTimer.SetInvokeHandler( LINK( this, ScTemporaryChartLock, TimeoutHdl ) );
}

ScTemporaryChartLock::~ScTemporaryChartLock()
{
    mpDoc = nullptr;
    StopLocking();
}

void ScTemporaryChartLock::StartOrContinueLocking()
{
    if (!mapScChartLockGuard)
        mapScChartLockGuard.reset( new ScChartLockGuard(mpDoc) );
    maTimer.Start();
}

void ScTemporaryChartLock::StopLocking()
{
    maTimer.Stop();
    mapScChartLockGuard.reset();
}

void ScTemporaryChartLock::AlsoLockThisChart( const Reference< frame::XModel >& xModel )
{
    if (mapScChartLockGuard)
        mapScChartLockGuard->AlsoLockThisChart( xModel );
}

IMPL_LINK_NOARG(ScTemporaryChartLock, TimeoutHdl, Timer *, void)
{
    mapScChartLockGuard.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
