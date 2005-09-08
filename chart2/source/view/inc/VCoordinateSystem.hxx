/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCoordinateSystem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:45:07 $
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
#ifndef _CHART2_VCOORDINATESYSTEM_HXX
#define _CHART2_VCOORDINATESYSTEM_HXX

#ifndef _COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XGRID_HPP_
#include <com/sun/star/chart2/XGrid.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
// #ifndef _COM_SUN_STAR_CHART2_XBOUNDEDCOORDINATESYSTEMCONTAINER_HPP_
// #include <com/sun/star/chart2/XBoundedCoordinateSystemContainer.hpp>
// #endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class MinimumAndMaximumSupplier;
class NumberFormatterWrapper;

class VCoordinateSystem
{
public:
    virtual ~VCoordinateSystem();

    static VCoordinateSystem* createCoordinateSystem( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XBoundedCoordinateSystem >& xCooSysModel );

    virtual void SAL_CALL initPlottingTargets(
                  const ::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XShapes >& xLogicTarget
                , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XShapes >& xFinalTarget
                , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
                        throw (::com::sun::star::uno::RuntimeException);
    void setOrigin( double* fCoordinateOrigin );
    void addAxis( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );
    void addGrid( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XGrid >& xGrid );
    void doAutoScale( MinimumAndMaximumSupplier* pMinMaxSupplier );

    void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );

    const ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitScaleData >& getExplicitScales() const {return m_aExplicitScales;}
    const ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitIncrementData >& getExplicitIncrements() const {return m_aExplicitIncrements;}
    double getOriginByDimension( sal_Int32 nDim ) const { return m_fCoordinateOrigin[nDim]; }

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XBoundedCoordinateSystem >
        getModel() const;

    virtual void createGridShapes();
    virtual void createAxesShapes( const ::com::sun::star::awt::Size& rReferenceSize, NumberFormatterWrapper* pNumberFormatterWrapper );

protected: //methods
    VCoordinateSystem( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XBoundedCoordinateSystem >& xCooSys );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XGrid > >&
        getGridListByDimension( sal_Int32 nDim );
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >
        getAxisByDimension( sal_Int32 nDim  ) const;

protected: //member
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XBoundedCoordinateSystem > m_xCooSysModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > m_xAxis0;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > m_xAxis1;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > m_xAxis2;

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XGrid > > m_xGridList0;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XGrid > > m_xGridList1;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XGrid > > m_xGridList2;

    double m_fCoordinateOrigin[3];

    ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitScaleData >     m_aExplicitScales;
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitIncrementData > m_aExplicitIncrements;

    //
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                m_xLogicTargetForGrids;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                m_xLogicTargetForAxes;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                m_xFinalTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>       m_xShapeFactory;
    ::com::sun::star::drawing::HomogenMatrix                            m_aMatrixSceneToScreen;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
