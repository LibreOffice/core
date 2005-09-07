/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatisticsHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:46:42 $
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
#ifndef CHART2_STATISTICSHELPER_HXX
#define CHART2_STATISTICSHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace chart
{

class StatisticsHelper
{
public:
    /** Calculates 1/n * sum (x_i - x_mean)^2.

        @see http://mathworld.wolfram.com/Variance.html

        @param bUnbiasedEstimator
            If true, 1/(n-1) * sum (x_i - x_mean)^2 is returned.
     */
    static double getVariance( const ::com::sun::star::uno::Sequence< double > & rData,
        bool bUnbiasedEstimator = false );

    // square root of the variance
    static double getStandardDeviation( const ::com::sun::star::uno::Sequence< double > & rData );

    // also called "Standard deviation of the mean (SDOM)"
    static double getStandardError( const ::com::sun::star::uno::Sequence< double > & rData );

private:
    // not implemented
    StatisticsHelper();
};

} //  namespace chart

// CHART2_STATISTICSHELPER_HXX
#endif
