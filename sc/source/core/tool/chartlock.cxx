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
#include "precompiled_sc.hxx"

#include <vcl/svapp.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

#include "chartlock.hxx"
#include "document.hxx"
#include "drwlayer.hxx"

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;

#define SC_CHARTLOCKTIMEOUT 660

// ====================================================================

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
            DBG_ASSERT(pPage,"Page ?");

            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if( pDoc->IsChart( pObject ) )
                {
                    uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                    uno::Reference< embed::XComponentSupplier > xCompSupp( xIPObj, uno::UNO_QUERY );
                    if( xCompSupp.is())
                    {
                        Reference< frame::XModel > xModel( xCompSupp->getComponent(), uno::UNO_QUERY );
                        if( xModel.is() )
                            aRet.push_back( xModel );
                    }
                }
                pObject = aIter.Next();
            }
        }
    }
    return aRet;
}

}//end anonymous namespace

// === ScChartLockGuard ======================================

ScChartLockGuard::ScChartLockGuard( ScDocument* pDoc ) :
    maChartModels( lcl_getAllLivingCharts( pDoc ) )
{
    std::vector< WeakReference< frame::XModel > >::const_iterator aIter = maChartModels.begin();
    const std::vector< WeakReference< frame::XModel > >::const_iterator aEnd = maChartModels.end();
    for( ; aIter != aEnd; ++aIter )
    {
        try
        {
            Reference< frame::XModel > xModel( *aIter );
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
    std::vector< WeakReference< frame::XModel > >::const_iterator aIter = maChartModels.begin();
    const std::vector< WeakReference< frame::XModel > >::const_iterator aEnd = maChartModels.end();
    for( ; aIter != aEnd; ++aIter )
    {
        try
        {
            Reference< frame::XModel > xModel( *aIter );
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
            maChartModels.push_back( xModel );
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Unexpected exception in ScChartLockGuard");
        }
    }
}

// === ScTemporaryChartLock ======================================

ScTemporaryChartLock::ScTemporaryChartLock( ScDocument* pDocP ) :
    mpDoc( pDocP )
{
    maTimer.SetTimeout( SC_CHARTLOCKTIMEOUT );
    maTimer.SetTimeoutHdl( LINK( this, ScTemporaryChartLock, TimeoutHdl ) );
}


ScTemporaryChartLock::~ScTemporaryChartLock()
{
    mpDoc = 0;
    StopLocking();
}

void ScTemporaryChartLock::StartOrContinueLocking()
{
    if(!mapScChartLockGuard.get())
        mapScChartLockGuard = std::auto_ptr< ScChartLockGuard >( new ScChartLockGuard(mpDoc) );
    maTimer.Start();
}

void ScTemporaryChartLock::StopLocking()
{
    maTimer.Stop();
    mapScChartLockGuard.reset();
}

void ScTemporaryChartLock::AlsoLockThisChart( const Reference< frame::XModel >& xModel )
{
    if(mapScChartLockGuard.get())
        mapScChartLockGuard->AlsoLockThisChart( xModel );
}

IMPL_LINK( ScTemporaryChartLock, TimeoutHdl, Timer*, EMPTYARG )
{
    mapScChartLockGuard.reset();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
