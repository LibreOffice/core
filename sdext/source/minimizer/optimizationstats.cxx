 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optimizationstats.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:57:05 $
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

#ifndef OPTIMIZATIONSTATS_HXX
#include "optimizationstats.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#endif


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
