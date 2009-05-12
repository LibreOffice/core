 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: optimizationstats.cxx,v $
 *
 * $Revision: 1.3 $
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
#include "precompiled_sdext.hxx"

#include "optimizationstats.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;

// -----------------------------------------------------------------------------

OptimizationStats::OptimizationStats()
{
}

// -----------------------------------------------------------------------------

void OptimizationStats::SetStatusValue( const PPPOptimizerTokenEnum eStat, const uno::Any& rStatValue )
{
    maStats[ eStat ] = rStatValue;
}

// -----------------------------------------------------------------------------

const uno::Any* OptimizationStats::GetStatusValue( const PPPOptimizerTokenEnum eStat ) const
{
    std::map< PPPOptimizerTokenEnum, uno::Any, Compare >::const_iterator aIter( maStats.find( eStat ) );
    return aIter != maStats.end() ? &((*aIter).second) : NULL;
}

// -----------------------------------------------------------------------------

com::sun::star::beans::PropertyValues OptimizationStats::GetStatusSequence()
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

// -----------------------------------------------------------------------------

void OptimizationStats::InitializeStatusValues( const uno::Sequence< PropertyValue >& rOptimizationStats )
{
    for( int i = 0; i < rOptimizationStats.getLength(); i++ )
        rOptimizationStats[ i ].Value >>= maStats[ TKGet( rOptimizationStats[ i ].Name ) ];
}

// -----------------------------------------------------------------------------

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
