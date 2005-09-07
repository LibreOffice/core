/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AxisItemConverter.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:20:55 $
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
#ifndef CHART_AXISITEMCONVERTER_HXX
#define CHART_AXISITEMCONVERTER_HXX

#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#include "ItemConverter.hxx"
#include "chartview/NumberFormatterWrapper.hxx"

#include <vector>
#include <memory>

class SdrModel;

namespace chart
{
namespace wrapper
{

class AxisItemConverter : public ::comphelper::ItemConverter
{
public:
    AxisItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        NumberFormatterWrapper * pNumFormatter,
        ::com::sun::star::chart2::ExplicitScaleData * pScale = NULL,
        ::com::sun::star::chart2::ExplicitIncrementData * pIncrement = NULL,
        double * pExplicitOrigin = NULL,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    virtual ~AxisItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

protected:
    virtual const USHORT * GetWhichPairs() const;
    virtual bool GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const;

    virtual void FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

private:
    ::std::vector< ItemConverter * >               m_aConverters;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XAxis >  m_xAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XBoundedCoordinateSystem >
                                                   m_xCoordinateSystem;
    sal_Int32                                      m_nDimension;
    NumberFormatterWrapper *                       m_pNumberFormatterWrapper;

    ::com::sun::star::chart2::ExplicitScaleData *       m_pExplicitScale;
    ::com::sun::star::chart2::ExplicitIncrementData *   m_pExplicitIncrement;
    double *                                                m_pExplicitOrigin;
};

} //  namespace wrapper
} //  namespace chart

// CHART_AXISITEMCONVERTER_HXX
#endif
