/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartlock.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:21:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
