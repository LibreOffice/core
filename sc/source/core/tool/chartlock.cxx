/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            DBG_ERROR("Unexpected exception in ScChartLockGuard");
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
            DBG_ERROR("Unexpected exception in ScChartLockGuard");
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
            DBG_ERROR("Unexpected exception in ScChartLockGuard");
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
