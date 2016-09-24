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

#include "ChartViewHelper.hxx"
#include "macros.hxx"
#include "servicenames.hxx"

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

namespace chart
{
using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

void ChartViewHelper::setViewToDirtyState( const uno::Reference< frame::XModel >& xChartModel )
{
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFact( xChartModel, uno::UNO_QUERY );
        if( xFact.is() )
        {
            Reference< util::XModifyListener > xModifyListener(
                    xFact->createInstance( CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );
            if( xModifyListener.is() )
            {
                lang::EventObject aEvent( Reference< lang::XComponent >( xChartModel, uno::UNO_QUERY ) );
                xModifyListener->modified( aEvent );
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
