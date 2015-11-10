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


#include "optimizationstats.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;



OptimizationStats::OptimizationStats()
{
}



void OptimizationStats::SetStatusValue( const PPPOptimizerTokenEnum eStat, const uno::Any& rStatValue )
{
    maStats[ eStat ] = rStatValue;
}



const uno::Any* OptimizationStats::GetStatusValue( const PPPOptimizerTokenEnum eStat ) const
{
    std::map< PPPOptimizerTokenEnum, uno::Any, Compare >::const_iterator aIter( maStats.find( eStat ) );
    return aIter != maStats.end() ? &((*aIter).second) : nullptr;
}



css::beans::PropertyValues OptimizationStats::GetStatusSequence()
{
    int i = 0;
    uno::Sequence< PropertyValue > aStatsSequence( maStats.size() );
    std::map< PPPOptimizerTokenEnum, uno::Any, Compare >::iterator aIter( maStats.begin() );
    while( aIter != maStats.end() )
    {
        aStatsSequence[ i ].Name = TKGet( (*aIter).first );
        aStatsSequence[ i++ ].Value <<= (*aIter++).second;
    }
    return aStatsSequence;
}



void OptimizationStats::InitializeStatusValues( const uno::Sequence< PropertyValue >& rOptimizationStats )
{
    for( int i = 0; i < rOptimizationStats.getLength(); i++ )
        rOptimizationStats[ i ].Value >>= maStats[ TKGet( rOptimizationStats[ i ].Name ) ];
}



void OptimizationStats::InitializeStatusValuesFromDocument( Reference< XModel > rxModel )
{
    try
    {
        Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
        Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        SetStatusValue( TK_Pages, Any( awt::Size( 0, xDrawPages->getCount() ) ) );
    }
    catch ( Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
