/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCoordinateSystem.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:20:58 $
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

#include "MinimumAndMaximumSupplier.hxx"
#include "ScaleAutomatism.hxx"

#ifndef _COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XTEXTUALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#endif

#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include "comphelper/implementationreference.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
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
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#include<map>
#include <vector>
#include <boost/shared_ptr.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

class ExplicitCategoriesProvider;

//-----------------------------------------------------------------------------
/**
*/
class VAxisBase;
class VCoordinateSystem
{
public:
    virtual ~VCoordinateSystem();

    static VCoordinateSystem* createCoordinateSystem( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel );

    virtual void SAL_CALL initPlottingTargets(
                  const ::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XShapes >& xLogicTarget
                , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XShapes >& xFinalTarget
                , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
                        throw (::com::sun::star::uno::RuntimeException);

    void setParticle( const rtl::OUString& rCooSysParticle );

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );
    ::com::sun::star::drawing::HomogenMatrix getTransformationSceneToScreen();

    //better performance for big data
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > getCoordinateSystemResolution( const ::com::sun::star::awt::Size& rPageSize
                                    , const ::com::sun::star::awt::Size& rPageResolution );

    ::com::sun::star::chart2::ExplicitScaleData getExplicitScale( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;
    ::com::sun::star::chart2::ExplicitIncrementData getExplicitIncrement( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XTextualDataSequence > getExplicitCategoriesProvider();

    // returns a coplete scale set for a given dimension and index; for example if nDimensionIndex==1 and nAxisIndex==2 you get returned the secondary x axis, main y axis and main z axis
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitScaleData > getExplicitScales( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;
    // returns a coplete increment set for a given dimension and index; for example if nDimensionIndex==1 and nAxisIndex==2 you get returned the secondary x axis, main y axis and main z axis
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitIncrementData > getExplicitIncrements( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;

    void addMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    bool hasMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    void clearMinimumAndMaximumSupplierList();

    void prepareScaleAutomatismForDimensionAndIndex( ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex, sal_Int32 nAxisIndex );

    void setExplicitScaleAndIncrement( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex
        , const ::com::sun::star::chart2::ExplicitScaleData& rExplicitScale
        , const ::com::sun::star::chart2::ExplicitIncrementData& rExplicitIncrement );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem >
        getModel() const;

    virtual void createVAxisList(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier
            , const ::com::sun::star::awt::Size& rFontReferenceSize
            , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels );

    virtual void initVAxisInList();
    virtual void updateScalesAndIncrementsOnAxes();

    virtual void createMaximumAxesLabels();
    virtual void createAxesLabels();
    virtual void updatePositions();
    virtual void createAxesShapes();

    virtual void createGridShapes();

    virtual bool getPropertySwapXAndYAxis() const;

    sal_Int32 getMaximumAxisIndexByDimension( sal_Int32 nDimensionIndex ) const;

    virtual bool needSeriesNamesForAxis() const;
    void setSeriesNamesForAxis( const ::com::sun::star::uno::Sequence< rtl::OUString >& rSeriesNames );

protected: //methods
    VCoordinateSystem( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >
        getAxisByDimension( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex  ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > >
        getGridListFromAxis( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );

    VAxisBase* getVAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    void prepareScaleAutomatism( ScaleAutomatism& rScaleAutomatism, double fMin, double fMax, sal_Int32 nDimIndex, sal_Int32 nAxisIndex );

    sal_Int32 getMaximumIncrementIndexByDimension( sal_Int32 nDimensionIndex ) const;

    rtl::OUString createCIDForAxis( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis >& xAxis
                    , sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    rtl::OUString createCIDForGrid( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis >& xAxis
                    , sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    sal_Int32 getNumberFormatKeyForAxis( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::chart2::XAxis >& xAxis
                     , const ::com::sun::star::uno::Reference<
                     ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

private: //methods
    void impl_adjustDimension( sal_Int32& rDimensionIndex ) const;
    void impl_adjustDimensionAndIndex( sal_Int32& rDimensionIndex, sal_Int32& rAxisIndex ) const;

protected: //member
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem > m_xCooSysModel;

    rtl::OUString m_aCooSysParticle;

    typedef std::pair< sal_Int32, sal_Int32 > tFullAxisIndex; //first index is the dimension, second index is the axis index that indicates wether this is a main or secondary axis

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

    //
    MergedMinimumAndMaximumSupplier m_aMergedMinimumAndMaximumSupplier; //this is used only for autoscaling purpose

    ::com::sun::star::uno::Sequence< rtl::OUString > m_aSeriesNamesForZAxis;

    typedef std::map< tFullAxisIndex, ::boost::shared_ptr< VAxisBase > > tVAxisMap;

    tVAxisMap m_aAxisMap;

private:
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitScaleData >     m_aExplicitScales;
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitIncrementData > m_aExplicitIncrements;

    typedef std::map< tFullAxisIndex, ::com::sun::star::chart2::ExplicitScaleData > tFullExplicitScaleMap;
    typedef std::map< tFullAxisIndex, ::com::sun::star::chart2::ExplicitIncrementData > tFullExplicitIncrementMap;

    tFullExplicitScaleMap       m_aSecondaryExplicitScales;
    tFullExplicitIncrementMap   m_aSecondaryExplicitIncrements;

    comphelper::ImplementationReference< ExplicitCategoriesProvider, ::com::sun::star::chart2::data::XTextualDataSequence >
                                m_aExplicitCategoriesProvider;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
