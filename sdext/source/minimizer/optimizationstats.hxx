/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optimizationstats.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:57:14 $
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
#define OPTIMIZATIONSTATS_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _PPPOPTIMIZER_TOKEN_HXX
#include "pppoptimizertoken.hxx"
#endif
#include <map>

class OptimizationStats
{
    struct Compare
    {
        bool operator()( const PPPOptimizerTokenEnum s1, const PPPOptimizerTokenEnum s2 ) const
        {
            return s1 < s2;
        }
    };
    std::map < PPPOptimizerTokenEnum, com::sun::star::uno::Any, Compare > maStats;

    public :

        OptimizationStats();
        ~OptimizationStats(){};

        void SetStatusValue( const PPPOptimizerTokenEnum eStat, const com::sun::star::uno::Any& rStatValue );
        const com::sun::star::uno::Any* GetStatusValue( const PPPOptimizerTokenEnum eStat ) const;

        void InitializeStatusValuesFromDocument( com::sun::star::uno::Reference< com::sun::star::frame::XModel > rxModel );
        void InitializeStatusValues( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rOptimizationStats );

        com::sun::star::beans::PropertyValues GetStatusSequence();
};


#endif // OPTIMIZATIONSTATS_HXX
