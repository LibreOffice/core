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
#ifndef INCLUDED_CHART2_SOURCE_INC_CHARTVIEW_EXPLICITVALUEPROVIDER_HXX
#define INCLUDED_CHART2_SOURCE_INC_CHARTVIEW_EXPLICITVALUEPROVIDER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <chartview/chartviewdllapi.hxx>

#include <memory>

namespace chart { class ChartModel; }
namespace chart { struct ExplicitIncrementData; }
namespace chart { struct ExplicitScaleData; }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XAxis; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XChartDocument; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XCoordinateSystem; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XDataSeries; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XDiagram; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XInterface; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Sequence; } } } }
namespace com { namespace sun { namespace star { namespace util { class XNumberFormatsSupplier; } } } }

namespace chart
{

class DrawModelWrapper;
class OOO_DLLPUBLIC_CHARTVIEW ExplicitValueProvider
{
public:
    /** Gives calculated scale and increment values for a given xAxis in the current view.
        In contrast to the model data these explicit values are always complete as missing auto properties are calculated.
        If the given Axis could not be found or for another reason no correct output can be given false is returned.
     */
    virtual bool getExplicitValuesForAxis(
        css::uno::Reference< css::chart2::XAxis > xAxis
        , ExplicitScaleData&  rExplicitScale
        , ExplicitIncrementData& rExplicitIncrement )=0;

    /** for rotated objects the shape size and position differs from the visible rectangle
        if bSnapRect is set to true you get the resulting visible position (left-top) and size
    */
    virtual css::awt::Rectangle
        getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect=false )=0;

    virtual css::awt::Rectangle getDiagramRectangleExcludingAxes()=0;

    virtual css::uno::Reference< css::drawing::XShape >
        getShapeForCID( const OUString& rObjectCID )=0;

    virtual std::shared_ptr< DrawModelWrapper > getDrawModelWrapper() = 0;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();

    static css::awt::Rectangle
        AddSubtractAxisTitleSizes(
                ChartModel& rModel
            , const css::uno::Reference< css::uno::XInterface >& xChartView
            , const css::awt::Rectangle& rPositionAndSize, bool bSubtract );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
              const css::uno::Reference< css::chart2::XAxis >& xAxis
            , const css::uno::Reference< css::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem
            , const css::uno::Reference< css::chart2::XChartDocument>& xChartDoc);

    static sal_Int32 getExplicitNumberFormatKeyForDataLabel(
            const css::uno::Reference< css::beans::XPropertySet >& xSeriesOrPointProp
            , const css::uno::Reference< css::chart2::XDataSeries >& xSeries
            , sal_Int32 nPointIndex /*-1 for whole series*/
            , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static sal_Int32 getExplicitPercentageNumberFormatKeyForDataLabel(
            const css::uno::Reference< css::beans::XPropertySet >& xSeriesOrPointProp
            , const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

protected:
    ~ExplicitValueProvider() {}
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
