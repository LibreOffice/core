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

#ifndef OOX_DRAWINGML_CHART_CONVERTERBASE_HXX
#define OOX_DRAWINGML_CHART_CONVERTERBASE_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"
#include "oox/drawingml/chart/objectformatter.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace awt { struct Size; }
    namespace chart2 { class XChartDocument; }
    namespace chart2 { class XTitle; }
    namespace drawing { class XShape; }
} } }

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox {
namespace drawingml {
namespace chart {

class ChartConverter;
struct ChartSpaceModel;
struct ConverterData;

// ============================================================================

const sal_Int32 API_PRIM_AXESSET = 0;
const sal_Int32 API_SECN_AXESSET = 1;

const sal_Int32 API_X_AXIS = 0;
const sal_Int32 API_Y_AXIS = 1;
const sal_Int32 API_Z_AXIS = 2;

// ============================================================================

class ConverterRoot
{
public:
    explicit            ConverterRoot(
                            ::oox::core::XmlFilterBase& rFilter,
                            ChartConverter& rChartConverter,
                            const ChartSpaceModel& rChartModel,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc,
                            const ::com::sun::star::awt::Size& rChartSize );
    virtual             ~ConverterRoot();

    /** Creates an instance for the passed service name, using the process service factory. */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        createInstance( const OUString& rServiceName ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        getComponentContext() const;

protected:
    /** Returns the filter object of the imported/exported document. */
    ::oox::core::XmlFilterBase& getFilter() const;
    /** Returns the chart converter. */
    ChartConverter*     getChartConverter() const;
    /** Returns the API chart document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
                        getChartDocument() const;
    /** Returns the position and size of the chart shape in 1/100 mm. */
    const ::com::sun::star::awt::Size& getChartSize() const;
    /** Returns the object formatter. */
    ObjectFormatter&    getFormatter() const;

    /** Registers a title object and its layout data, needed for conversion of
        the title position using the old Chart1 API. */
    void                registerTitleLayout(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTitle >& rxTitle,
                            const ModelRef< LayoutModel >& rxLayout, ObjectType eObjType,
                            sal_Int32 nMainIdx = -1, sal_Int32 nSubIdx = -1 );
    /** Converts the positions of the main title and all axis titles. */
    void                convertTitlePositions();

private:
    ::boost::shared_ptr< ConverterData > mxData;
};

// ============================================================================

/** Base class of all converter classes. Holds a reference to a model structure
    of the specified type.
 */
template< typename ModelType >
class ConverterBase : public ConverterRoot
{
public:
    const ModelType& getModel() const { return mrModel; }

protected:
    explicit            ConverterBase( const ConverterRoot& rParent, ModelType& rModel ) :
                            ConverterRoot( rParent ), mrModel( rModel ) {}
    virtual             ~ConverterBase() {}

protected:
    ModelType&          mrModel;
};

// ============================================================================

/** A layout converter calculates positions and sizes for various chart objects.
 */
class LayoutConverter : public ConverterBase< LayoutModel >
{
public:
    explicit            LayoutConverter( const ConverterRoot& rParent, LayoutModel& rModel );
    virtual             ~LayoutConverter();

    /** Tries to calculate the absolute position and size from the contained
        OOXML layout model. Returns true, if returned rectangle is valid. */
    bool                calcAbsRectangle( ::com::sun::star::awt::Rectangle& orRect ) const;

    /** Tries to set the position and size from the contained OOXML layout model.
        Returns true, if a manual position and size could be calculated. */
    bool                convertFromModel( PropertySet& rPropSet );

    /** Tries to set the position from the contained OOXML layout model.
        Returns true, if a manual position could be calculated. */
    bool                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            double fRotationAngle );
    bool getAutoLayout(){return mrModel.mbAutoLayout;}
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
