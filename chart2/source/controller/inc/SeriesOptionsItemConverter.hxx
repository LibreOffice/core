/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SeriesOptionsItemConverter.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:54:03 $
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
#ifndef CHART_SERIESOPTIONSITEMCONVERTER_HXX
#define CHART_SERIESOPTIONSITEMCONVERTER_HXX

#include "ItemConverter.hxx"


#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif

namespace chart
{
namespace wrapper
{

class SeriesOptionsItemConverter :
        public ::comphelper::ItemConverter
{
public:
    SeriesOptionsItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool );
    virtual ~SeriesOptionsItemConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >  m_xChartModel;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext>   m_xCC;

    bool m_bAttachToMainAxis;
    bool m_bSupportingOverlapAndGapWidthProperties;
    bool m_bSupportingBarConnectors;

    sal_Int32 m_nBarOverlap;
    sal_Int32 m_nGapWidth;
    sal_Bool  m_bConnectBars;

    bool m_bSupportingAxisSideBySide;
    bool m_bGroupBarsPerAxis;
    bool m_bAllSeriesAttachedToSameAxis;
    sal_Int32 m_nAllSeriesAxisIndex;

    bool m_bSupportingStartingAngle;
    sal_Int32 m_nStartingAngle;

    bool m_bClockwise;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem > m_xCooSys;
};

} //  namespace wrapper
} //  namespace chart

// CHART_SERIESOPTIONSITEMCONVERTER_HXX
#endif
