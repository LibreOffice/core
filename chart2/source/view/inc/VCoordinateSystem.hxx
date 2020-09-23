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
#pragma once

#include "MinimumAndMaximumSupplier.hxx"
#include <ThreeDHelper.hxx>
#include <chartview/ExplicitScaleValues.hxx>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/uno/Sequence.h>

#include <map>
#include <memory>
#include <vector>

namespace chart { class ExplicitCategoriesProvider; }
namespace chart { class ScaleAutomatism; }
namespace com::sun::star::awt { struct Rectangle; }
namespace com::sun::star::awt { struct Size; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::chart2 { class XAxis; }
namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::chart2 { class XCoordinateSystem; }
namespace com::sun::star::drawing { class XShapes; }
namespace com::sun::star::lang { class XMultiServiceFactory; }


namespace chart
{

class VAxisBase;

class VCoordinateSystem
{
public:
    virtual ~VCoordinateSystem();

    static std::unique_ptr<VCoordinateSystem> createCoordinateSystem( const css::uno::Reference<
                                css::chart2::XCoordinateSystem >& xCooSysModel );

    /// @throws css::uno::RuntimeException
    void initPlottingTargets(
                  const css::uno::Reference< css::drawing::XShapes >& xLogicTarget
                , const css::uno::Reference< css::drawing::XShapes >& xFinalTarget
                , const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory
                , css::uno::Reference< css::drawing::XShapes >& xLogicTargetForSeriesBehindAxis );

    void setParticle( const OUString& rCooSysParticle );

    void setTransformationSceneToScreen( const css::drawing::HomogenMatrix& rMatrix );
    const css::drawing::HomogenMatrix& getTransformationSceneToScreen() const { return m_aMatrixSceneToScreen;}

    //better performance for big data
    virtual css::uno::Sequence< sal_Int32 > getCoordinateSystemResolution( const css::awt::Size& rPageSize
                                    , const css::awt::Size& rPageResolution );

    ExplicitScaleData getExplicitScale( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;
    ExplicitIncrementData getExplicitIncrement( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;

    void setExplicitCategoriesProvider( ExplicitCategoriesProvider* /*takes ownership*/ );
    ExplicitCategoriesProvider* getExplicitCategoriesProvider();

    // returns a complete scale set for a given dimension and index; for example if nDimensionIndex==1 and nAxisIndex==2 you get returned the secondary x axis, main y axis and main z axis
    std::vector< ExplicitScaleData > getExplicitScales( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;
    // returns a complete increment set for a given dimension and index; for example if nDimensionIndex==1 and nAxisIndex==2 you get returned the secondary x axis, main y axis and main z axis
    std::vector< ExplicitIncrementData > getExplicitIncrements( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex ) const;

    void addMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    bool hasMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    void clearMinimumAndMaximumSupplierList();

    /**
     * It sets the scaling parameters for the passed `ScaleAutomatism` object.
     * Especially it sets the `m_fValueMinimum` and the `m_fValueMaximum`
     * parameters (see `ScaleAutomatism::expandValueRange`).
     * The value to be assigned to these two parameters is retrieved by
     * invoking the `getMinimum` and `getMaximum` methods of the minimum-maximum
     * supplier object that belongs to the given coordinate system.
     * The minimum-maximum supplier object is set in the
     * `SeriesPlotterContainer::initializeCooSysAndSeriesPlotter` method to the
     * series plotter which is based on the coordinate system (see notes for
     * the method). For instance for a pie chart the `m_fValueMinimum` and the
     * `m_fValueMaximum` parameters are initialized by the `PieChart::getMinimum`
     * and `PieChart::getMaximum` methods.
     */
    void prepareAutomaticAxisScaling( ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex, sal_Int32 nAxisIndex );

    void setExplicitScaleAndIncrement( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex
        , const ExplicitScaleData& rExplicitScale
        , const ExplicitIncrementData& rExplicitIncrement );

    void set3DWallPositions( CuboidPlanePosition eLeftWallPos, CuboidPlanePosition eBackWallPos, CuboidPlanePosition eBottomPos );

    const css::uno::Reference< css::chart2::XCoordinateSystem >&
        getModel() const { return m_xCooSysModel;}

    /**
     * Create "view" axis objects 'VAxis' from the coordinate system model.
     */
    virtual void createVAxisList(
            const css::uno::Reference< css::chart2::XChartDocument> & xChartDoc
            , const css::awt::Size& rFontReferenceSize
            , const css::awt::Rectangle& rMaximumSpaceForLabels
            , bool bLimitSpaceForLabels );

    virtual void initVAxisInList();
    virtual void updateScalesAndIncrementsOnAxes();

    void createMaximumAxesLabels();
    void createAxesLabels();
    void updatePositions();
    void createAxesShapes();

    virtual void createGridShapes();

    bool getPropertySwapXAndYAxis() const;

    sal_Int32 getMaximumAxisIndexByDimension( sal_Int32 nDimensionIndex ) const;

    bool needSeriesNamesForAxis() const;
    void setSeriesNamesForAxis( const css::uno::Sequence< OUString >& rSeriesNames );

protected: //methods
    VCoordinateSystem( const css::uno::Reference<
        css::chart2::XCoordinateSystem >& xCooSys );

    css::uno::Reference< css::chart2::XAxis >
        getAxisByDimension( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex  ) const;
    static css::uno::Sequence< css::uno::Reference< css::beans::XPropertySet > >
        getGridListFromAxis( const css::uno::Reference< css::chart2::XAxis >& xAxis );

    VAxisBase* getVAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    OUString createCIDForAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    OUString createCIDForGrid( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    sal_Int32 getNumberFormatKeyForAxis( const css::uno::Reference<
                     css::chart2::XAxis >& xAxis
                     , const css::uno::Reference<
                     css::chart2::XChartDocument>& xChartDoc);

private: //methods
    static void impl_adjustDimension( sal_Int32& rDimensionIndex );
    void impl_adjustDimensionAndIndex( sal_Int32& rDimensionIndex, sal_Int32& rAxisIndex ) const;

protected: //member
    css::uno::Reference< css::chart2::XCoordinateSystem > m_xCooSysModel;

    OUString m_aCooSysParticle;

    typedef std::pair< sal_Int32, sal_Int32 > tFullAxisIndex; //first index is the dimension, second index is the axis index that indicates whether this is a main or secondary axis

    css::uno::Reference< css::drawing::XShapes >                m_xLogicTargetForGrids;
    css::uno::Reference< css::drawing::XShapes >                m_xLogicTargetForAxes;
    css::uno::Reference< css::drawing::XShapes >                m_xFinalTarget;
    css::uno::Reference< css::lang::XMultiServiceFactory>       m_xShapeFactory;
    css::drawing::HomogenMatrix                            m_aMatrixSceneToScreen;

    CuboidPlanePosition m_eLeftWallPos;
    CuboidPlanePosition m_eBackWallPos;
    CuboidPlanePosition m_eBottomPos;

    /**
     * Collection of min-max suppliers which are basically different chart
     * types present in the same coordinate system.  This is used only for
     * auto-scaling purposes.
     */
    MergedMinimumAndMaximumSupplier m_aMergedMinMaxSupplier;

    css::uno::Sequence< OUString > m_aSeriesNamesForZAxis;

    typedef std::map< tFullAxisIndex, std::shared_ptr< VAxisBase > > tVAxisMap;

    tVAxisMap m_aAxisMap;

private:
    std::vector< ExplicitScaleData >     m_aExplicitScales;
    std::vector< ExplicitIncrementData > m_aExplicitIncrements;

    typedef std::map< tFullAxisIndex, ExplicitScaleData > tFullExplicitScaleMap;
    typedef std::map< tFullAxisIndex, ExplicitIncrementData > tFullExplicitIncrementMap;

    tFullExplicitScaleMap       m_aSecondaryExplicitScales;
    tFullExplicitIncrementMap   m_aSecondaryExplicitIncrements;

    std::unique_ptr< ExplicitCategoriesProvider > m_apExplicitCategoriesProvider;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
