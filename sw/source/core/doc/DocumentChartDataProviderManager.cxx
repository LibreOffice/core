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

#include <DocumentChartDataProviderManager.hxx>

#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <vcl/svapp.hxx>
#include <swtable.hxx>
#include <unochart.hxx>
#include <frmfmt.hxx>
#include <ndole.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>


using namespace com::sun::star;
using namespace css::uno;

namespace sw {

DocumentChartDataProviderManager::DocumentChartDataProviderManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc ),
                                                                                        maChartDataProviderImplRef(),
                                                                                        mpChartControllerHelper( 0 )
{

}

SwChartDataProvider * DocumentChartDataProviderManager::GetChartDataProvider( bool bCreate ) const
{
    // since there must be only one instance of this object per document
    // we need a mutex here
    SolarMutexGuard aGuard;

    if (bCreate && !maChartDataProviderImplRef.is())
    {
        maChartDataProviderImplRef = new SwChartDataProvider( & m_rDoc );
    }
    return maChartDataProviderImplRef.get();
}

void DocumentChartDataProviderManager::CreateChartInternalDataProviders( const SwTable *pTable )
{
    if (pTable)
    {
        OUString aName( pTable->GetFrameFormat()->GetName() );
        SwOLENode *pONd;
        SwStartNode *pStNd;
        SwNodeIndex aIdx( *m_rDoc.GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
        while (0 != (pStNd = aIdx.GetNode().GetStartNode()))
        {
            ++aIdx;
            if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
                aName == pONd->GetChartTableName() /* OLE node is chart? */ &&
                0 != (pONd->getLayoutFrm( m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout() )) /* chart frame is not hidden */ )
            {
                uno::Reference < embed::XEmbeddedObject > xIP = pONd->GetOLEObj().GetOleRef();
                if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
                {
                    uno::Reference< chart2::XChartDocument > xChart( xIP->getComponent(), UNO_QUERY );
                    if (xChart.is())
                        xChart->createInternalDataProvider( sal_True );

                    // there may be more than one chart for each table thus we need to continue the loop...
                }
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
    }
}

SwChartLockController_Helper & DocumentChartDataProviderManager::GetChartControllerHelper()
{
    if (!mpChartControllerHelper)
    {
        mpChartControllerHelper = new SwChartLockController_Helper( & m_rDoc );
    }
    return *mpChartControllerHelper;
}

DocumentChartDataProviderManager::~DocumentChartDataProviderManager()
{
    // clean up chart related structures...
    // Note: the chart data provider gets already disposed in ~SwDocShell
    // since all UNO API related functionality requires an existing SwDocShell
    // this assures that dispose gets called if there is need for it.
    maChartDataProviderImplRef.clear();
    delete mpChartControllerHelper;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
