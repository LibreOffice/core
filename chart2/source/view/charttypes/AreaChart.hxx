#ifndef _CHART2_AREACHART_HXX
#define _CHART2_AREACHART_HXX

#include "VSeriesPlotter.hxx"

#ifndef _COM_SUN_STAR_CHART2_CURVESTYLE_HPP_
#include <com/sun/star/chart2/CurveStyle.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
class AreaPositionHelper;

class AreaChart : public VSeriesPlotter
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    AreaChart( const ::com::sun::star::uno::Reference<
             ::com::sun::star::chart2::XChartType >& xChartTypeModel
             , bool bCategoryXAxis, bool bNoArea=false
             , PlottingPositionHelper* pPlottingPositionHelper=NULL //takes owner ship
             );
    virtual ~AreaChart();

    //-------------------------------------------------------------------------
    // chart2::XPlotter
    //-------------------------------------------------------------------------

    virtual void SAL_CALL createShapes();
    /*
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemTypeID(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setScales( const ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitScaleData >& rScales ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTransformation( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToLogicTarget, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToFinalPage ) throw (::com::sun::star::uno::RuntimeException);
    */

    virtual void addSeries( VDataSeries* pSeries, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    //-------------------------------------------------------------------------
    // MinimumAndMaximumSupplier
    //-------------------------------------------------------------------------
    virtual double getMaximumZ();

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
private: //methods
    //no default constructor
    AreaChart();

    void impl_createSeriesShapes();
    bool impl_createArea( VDataSeries* pSeries
                , ::com::sun::star::drawing::PolyPolygonShape3D* pSeriesPoly
                , ::com::sun::star::drawing::PolyPolygonShape3D* pPreviousSeriesPoly );
    bool impl_createLine( VDataSeries* pSeries
                , ::com::sun::star::drawing::PolyPolygonShape3D* pSeriesPoly );

    double  getTransformedDepth() const;
    double  getLogicGrounding() const;

private: //member
    bool                                m_bArea;//false -> line or symbol only
    bool                                m_bLine;
    bool                                m_bSymbol;

    //Properties for splines:
    ::com::sun::star::chart2::CurveStyle    m_eCurveStyle;
    sal_Int32                                       m_nCurveResolution;
    sal_Int32                                       m_nSplineOrder;

    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes > m_xSeriesTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes > m_xErrorBarTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes > m_xTextTarget;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
