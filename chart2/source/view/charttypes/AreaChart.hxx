#ifndef _CHART2_AREACHART_HXX
#define _CHART2_AREACHART_HXX

#include "VSeriesPlotter.hxx"

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
    AreaChart( sal_Int32 nDimension, sal_Bool bArea, sal_Bool bLine, sal_Bool bSymbol );
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

    bool                                m_bArea;//false -> line or symbol only
    bool                                m_bLine;
    bool                                m_bSymbol;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
