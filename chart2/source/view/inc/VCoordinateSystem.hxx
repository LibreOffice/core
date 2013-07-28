/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _CHART2_VCOORDINATESYSTEM_HXX
#define _CHART2_VCOORDINATESYSTEM_HXX

#include "MinimumAndMaximumSupplier.hxx"
#include "ScaleAutomatism.hxx"
#include "ThreeDHelper.hxx"
#include "ExplicitCategoriesProvider.hxx"
#include "chartview/ExplicitScaleValues.hxx"

#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include "comphelper/implementationreference.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include<map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace chart
{

/**
*/
class VAxisBase;
class VCoordinateSystem
{
public:
    virtual ~VCoordinateSystem();

    static VCoordinateSystem* createCoordinateSystem( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XCoordinateSystem >& xCooSysModel );

    virtual void initPlottingTargets(
                  const ::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XShapes >& xLogicTarget
                , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XShapes >& xFinalTarget
                , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& xFactory
                , ::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XShapes >& xLogicTargetForSeriesBehindAxis )
                        throw (::com::sun::star::uno::RuntimeException);

    void setParticle( const OUString& rCooSysParticle );

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );
    ::com::sun::star::drawing::HomogenMatrix getTransformationSceneToScreen();

    //better performance for big data
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > getCoordinateSystemResolution( const ::com::sun::star::awt::Size& rPageSize
                                    , const ::com::sun::star::awt::Size& rPageResolution );

    ExplicitScaleData getExplicitScale( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;
    ExplicitIncrementData getExplicitIncrement( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;

    void setExplicitCategoriesProvider( ExplicitCategoriesProvider* /*takes ownership*/ );
    ExplicitCategoriesProvider* getExplicitCategoriesProvider();

    // returns a coplete scale set for a given dimension and index; for example if nDimensionIndex==1 and nAxisIndex==2 you get returned the secondary x axis, main y axis and main z axis
    ::std::vector< ExplicitScaleData > getExplicitScales( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;
    // returns a coplete increment set for a given dimension and index; for example if nDimensionIndex==1 and nAxisIndex==2 you get returned the secondary x axis, main y axis and main z axis
    ::std::vector< ExplicitIncrementData > getExplicitIncrements( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;

    void addMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    bool hasMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    void clearMinimumAndMaximumSupplierList();

    void prepareScaleAutomatismForDimensionAndIndex( ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex, sal_Int32 nAxisIndex );

    void setExplicitScaleAndIncrement( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex
        , const ExplicitScaleData& rExplicitScale
        , const ExplicitIncrementData& rExplicitIncrement );

    void set3DWallPositions( CuboidPlanePosition eLeftWallPos, CuboidPlanePosition eBackWallPos, CuboidPlanePosition eBottomPos );

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
    void setSeriesNamesForAxis( const ::com::sun::star::uno::Sequence< OUString >& rSeriesNames );

protected: //methods
    VCoordinateSystem( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >
        getAxisByDimension( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex  ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > >
        getGridListFromAxis( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );

    VAxisBase* getVAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    OUString createCIDForAxis( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis >& xAxis
                    , sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    OUString createCIDForGrid( const ::com::sun::star::uno::Reference<
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

    OUString m_aCooSysParticle;

    typedef std::pair< sal_Int32, sal_Int32 > tFullAxisIndex; //first index is the dimension, second index is the axis index that indicates whether this is a main or secondary axis

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

    CuboidPlanePosition m_eLeftWallPos;
    CuboidPlanePosition m_eBackWallPos;
    CuboidPlanePosition m_eBottomPos;

    //
    MergedMinimumAndMaximumSupplier m_aMergedMinimumAndMaximumSupplier; //this is used only for autoscaling purpose

    ::com::sun::star::uno::Sequence< OUString > m_aSeriesNamesForZAxis;

    typedef std::map< tFullAxisIndex, ::boost::shared_ptr< VAxisBase > > tVAxisMap;

    tVAxisMap m_aAxisMap;

private:
    std::vector< ExplicitScaleData >     m_aExplicitScales;
    std::vector< ExplicitIncrementData > m_aExplicitIncrements;

    typedef std::map< tFullAxisIndex, ExplicitScaleData > tFullExplicitScaleMap;
    typedef std::map< tFullAxisIndex, ExplicitIncrementData > tFullExplicitIncrementMap;

    tFullExplicitScaleMap       m_aSecondaryExplicitScales;
    tFullExplicitIncrementMap   m_aSecondaryExplicitIncrements;

    boost::scoped_ptr< ExplicitCategoriesProvider > m_apExplicitCategoriesProvider;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
