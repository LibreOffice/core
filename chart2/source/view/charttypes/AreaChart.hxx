#ifndef _CHART2_AREACHART_HXX
#define _CHART2_AREACHART_HXX

#include "VSeriesPlotter.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_CURVESTYLE_HPP_
#include <drafts/com/sun/star/chart2/CurveStyle.hpp>
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
             ::drafts::com::sun::star::chart2::XChartType >& xChartTypeModel
             , bool bCategoryXAxis, bool bNoArea=false );
    virtual ~AreaChart();

    //-------------------------------------------------------------------------
    // chart2::XPlotter
    //-------------------------------------------------------------------------

    virtual void SAL_CALL createShapes();
    /*
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemTypeID(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setScales( const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::chart2::ExplicitScaleData >& rScales ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTransformation( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTransformation >& xTransformationToLogicTarget, const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTransformation >& xTransformationToFinalPage ) throw (::com::sun::star::uno::RuntimeException);
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

private: //member
    AreaPositionHelper*                 m_pPosHelper;

    bool                                m_bCategoryXAxis;//true->xvalues are indices (this would not be necessary if series for category chart wouldn't have x-values)

    bool                                m_bArea;//false -> line or symbol only
    bool                                m_bLine;
    bool                                m_bSymbol;

    //Properties for splines:
    ::drafts::com::sun::star::chart2::CurveStyle    m_eCurveStyle;
    sal_Int32                                       m_nCurveResolution;
    sal_Int32                                       m_nSplineOrder;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
