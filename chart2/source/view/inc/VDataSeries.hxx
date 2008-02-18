/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VDataSeries.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 16:02:35 $
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
#ifndef _CHART2_VIEW_DATASERIES_HXX
#define _CHART2_VIEW_DATASERIES_HXX

#include "PropertyMapper.hxx"

#include <vector>
//for auto_ptr
#include <memory>

#ifndef _COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_STACKINGDIRECTION_HPP_
#include <com/sun/star/chart2/StackingDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class VDataSequence
{
public:
    void init( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence >& xModel);
    bool is() const;
    void clear();
    double getValue( sal_Int32 index ) const;
    sal_Int32 detectNumberFormatKey( sal_Int32 index ) const;
    sal_Int32 getLength() const;


    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence >     Model;

    mutable ::com::sun::star::uno::Sequence< double > Doubles;
};

class VDataSeries
{
public:
    VDataSeries( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries >& xDataSeries );
    virtual ~VDataSeries();

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >
        getModel() const;

    void setCategoryXAxis();
    void setParticle( const rtl::OUString& rSeriesParticle );
    void setGlobalSeriesIndex( sal_Int32 nGlobalSeriesIndex );
    void setDiagramReferenceSize( const ::com::sun::star::awt::Size & rDiagramRefSize );

    sal_Int32   getTotalPointCount() const;
    double      getX( sal_Int32 index ) const;
    double      getY( sal_Int32 index ) const;

    double      getY_Min( sal_Int32 index ) const;
    double      getY_Max( sal_Int32 index ) const;
    double      getY_First( sal_Int32 index ) const;
    double      getY_Last( sal_Int32 index ) const;

    double      getMinimumofAllDifferentYValues( sal_Int32 index ) const;
    double      getMaximumofAllDifferentYValues( sal_Int32 index ) const;

    ::com::sun::star::uno::Sequence< double > getAllX() const;
    ::com::sun::star::uno::Sequence< double > getAllY() const;

    bool        hasExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const;
    sal_Int32   getExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const;
    sal_Int32   detectNumberFormatKey( sal_Int32 nPointIndex ) const;

    sal_Int32   getLabelPlacement( sal_Int32 nPointIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType
                        , sal_Int32 nDimensionCount, sal_Bool bSwapXAndY ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getPropertiesOfPoint( sal_Int32 index ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getPropertiesOfSeries() const;

    ::com::sun::star::chart2::Symbol*
                        getSymbolProperties( sal_Int32 index ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getYErrorBarProperties( sal_Int32 index ) const;

    bool hasPointOwnColor( sal_Int32 index ) const;

    ::com::sun::star::chart2::StackingDirection getStackingDirection() const;
    sal_Int32 getAttachedAxisIndex() const;
    void setAttachedAxisIndex( sal_Int32 nAttachedAxisIndex );

    void doSortByXValues();

    void setConnectBars( sal_Bool bConnectBars );
    sal_Bool getConnectBars() const;

    void setGroupBarsPerAxis( sal_Bool bGroupBarsPerAxis );
    sal_Bool getGroupBarsPerAxis() const;

    void setStartingAngle( sal_Int32 nStartingAngle );
    sal_Int32 getStartingAngle() const;

    //this is only temporarily here for area chart:
    ::com::sun::star::drawing::PolyPolygonShape3D       m_aPolyPolygonShape3D;
    sal_Int32   m_nPolygonIndex;
    double m_fLogicMinX;
    double m_fLogicMaxX;
    //
    //this is here for deep stacking:
    double m_fLogicZPos;//from 0 to series count -1
    //

    rtl::OUString       getCID() const;
    rtl::OUString       getSeriesParticle() const;
    rtl::OUString       getPointCID_Stub() const;
    rtl::OUString       getErrorBarsCID() const;
    rtl::OUString       getLabelsCID() const;
    rtl::OUString       getLabelCID_Stub() const;
    rtl::OUString       getDataCurveCID( sal_Int32 nCurveIndex, bool bAverageLine ) const;

    ::com::sun::star::chart2::DataPointLabel*
                        getDataPointLabelIfLabel( sal_Int32 index ) const;
    bool    getTextLabelMultiPropertyLists( sal_Int32 index, tNameSequence*& pPropNames, tAnySequence*& pPropValues ) const;

    rtl::OUString       getDataCurveEquationCID( sal_Int32 nCurveIndex ) const;
    bool    isAttributedDataPoint( sal_Int32 index ) const;

    bool    isVaryColorsByPoint() const;

    void releaseShapes();

private: //methods
    VDataSeries();
    ::com::sun::star::chart2::DataPointLabel*
                        getDataPointLabel( sal_Int32 index ) const;
    void adaptPointCache( sal_Int32 nNewPointIndex ) const;

public: //member
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xGroupShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xLabelsGroupShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xErrorBarsGroupShape;

    //the following group shapes will be created as children of m_xGroupShape on demand
    //they can be used to assure that some parts of a series shape are always in front of others (e.g. symbols in front of lines)
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xFrontSubGroupShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xBackSubGroupShape;

private: //member
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >       m_xDataSeries;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > >   m_aDataSequences;

    //all points given by the model data (here are not only the visible points meant)
    sal_Int32       m_nPointCount;

    VDataSequence   m_aValues_X;
    VDataSequence   m_aValues_Y;
    VDataSequence   m_aValues_Z;

    VDataSequence   m_aValues_Y_Min;
    VDataSequence   m_aValues_Y_Max;
    VDataSequence   m_aValues_Y_First;
    VDataSequence   m_aValues_Y_Last;

    ::com::sun::star::uno::Sequence< sal_Int32 >    m_aAttributedDataPointIndexList;

    ::com::sun::star::chart2::StackingDirection     m_eStackingDirection;

    sal_Int32               m_nAxisIndex;//indicates wether this is attached to a main or secondary axis

    sal_Bool                m_bConnectBars;

    sal_Bool                m_bGroupBarsPerAxis;

    sal_Int32               m_nStartingAngle;

    rtl::OUString           m_aSeriesParticle;
    rtl::OUString           m_aCID;
    rtl::OUString           m_aPointCID_Stub;
    rtl::OUString           m_aLabelCID_Stub;

    sal_Int32               m_nGlobalSeriesIndex;

    //some cached values for data labels as they are very expensive
    mutable ::std::auto_ptr< ::com::sun::star::chart2::DataPointLabel >
                                                    m_apLabel_Series;
    mutable ::std::auto_ptr< tNameSequence >        m_apLabelPropNames_Series;
    mutable ::std::auto_ptr< tAnySequence >         m_apLabelPropValues_Series;
    mutable ::std::auto_ptr< ::com::sun::star::chart2::Symbol >
                                                    m_apSymbolProperties_Series;

    mutable ::std::auto_ptr< ::com::sun::star::chart2::DataPointLabel >
                                                    m_apLabel_AttributedPoint;
    mutable ::std::auto_ptr< tNameSequence >        m_apLabelPropNames_AttributedPoint;
    mutable ::std::auto_ptr< tAnySequence >         m_apLabelPropValues_AttributedPoint;
    mutable ::std::auto_ptr< ::com::sun::star::chart2::Symbol >
                                                    m_apSymbolProperties_AttributedPoint;
    mutable ::std::auto_ptr< ::com::sun::star::chart2::Symbol >
                                                    m_apSymbolProperties_InvisibleSymbolForSelection;
    mutable sal_Int32                               m_nCurrentAttributedPoint;
    ::com::sun::star::awt::Size                     m_aReferenceSize;
    //
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
