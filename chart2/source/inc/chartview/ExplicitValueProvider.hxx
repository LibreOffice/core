/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExplicitValueProvider.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:53:20 $
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
#ifndef _CHART2_EXPLICITVALUEPROVIDER_HXX
#define _CHART2_EXPLICITVALUEPROVIDER_HXX

#include <boost/shared_ptr.hpp>

#ifndef _COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class DrawModelWrapper;
class ExplicitValueProvider
{
public:
    /** Gives calculated scale and increment values for a given xAxis in the current view.
        In contrast to the model data these explicit values are always complete as misssing auto properties are calculated.
        If the given Axis could not be found or for another reason no correct output can be given false is returned.
     */
    virtual sal_Bool getExplicitValuesForAxis(
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > xAxis
        , ::com::sun::star::chart2::ExplicitScaleData&  rExplicitScale
        , ::com::sun::star::chart2::ExplicitIncrementData& rExplicitIncrement )=0;

    /** for rotated objects the shape size and position differs from the visible rectangle
        if bSnapRect is set to true you get the resulting visible position (left-top) and size
    */
    virtual ::com::sun::star::awt::Rectangle
        getRectangleOfObject( const rtl::OUString& rObjectCID, bool bSnapRect=false )=0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        getShapeForCID( const rtl::OUString& rObjectCID )=0;

    virtual ::boost::shared_ptr< DrawModelWrapper > getDrawModelWrapper() = 0;

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ExplicitValueProvider* getExplicitValueProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xChartView );

    static ::com::sun::star::awt::Rectangle
        calculateDiagramPositionAndSizeInclusiveTitle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel
            , const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface >& xChartView
            , const ::com::sun::star::awt::Rectangle& rExclusivePositionAndSize );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
              const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem
            , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

    static sal_Int32 getPercentNumberFormat( const ::com::sun::star::uno::Reference<
                ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

    static sal_Int32 getExplicitNumberFormatKeyForLabel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesOrPointProp
            , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries
            , sal_Int32 nPointIndex /*-1 for whole series*/
            , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAttachedAxisProps );

    static sal_Int32 getExplicitPercentageNumberFormatKeyForLabel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesOrPointProp
            , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
