/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveItemConverter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:53:51 $
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
#ifndef CHART_REGRESSIONCURVE_ITEMCONVERTER_HXX
#define CHART_REGRESSIONCURVE_ITEMCONVERTER_HXX

#include "ItemConverter.hxx"

#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vector>
#include <boost/shared_ptr.hpp>

class SdrModel;

namespace chart
{
namespace wrapper
{

class RegressionCurveItemConverter :
        public ::comphelper::ItemConverter
{
public:
    RegressionCurveItemConverter(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & rPropertySet,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCurveCnt,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory );
    virtual ~RegressionCurveItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

protected:
    virtual const USHORT * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

private:
    ::boost::shared_ptr< ItemConverter >  m_spGraphicConverter;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer >  m_xCurveContainer;
};

} //  namespace wrapper
} //  namespace chart

// CHART_REGRESSIONCURVE_ITEMCONVERTER_HXX
#endif
