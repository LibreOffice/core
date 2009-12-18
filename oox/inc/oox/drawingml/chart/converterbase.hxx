/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: converterbase.hxx,v $
 *
 * $Revision: 1.4.6.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_CHART_CONVERTERBASE_HXX
#define OOX_DRAWINGML_CHART_CONVERTERBASE_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"
#include "oox/drawingml/chart/objectformatter.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace awt { struct Size; }
    namespace chart2 { class XChartDocument; }
    namespace drawing { class XShape; }
} } }

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

class ChartConverter;
class ObjectFormatter;
struct ChartSpaceModel;
struct ConverterData;

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
                        createInstance( const ::rtl::OUString& rServiceName ) const;

protected:
    /** Returns the filter object of the imported/exported document. */
    ::oox::core::XmlFilterBase& getFilter() const;
    /** Returns the chart converter. */
    ChartConverter&     getChartConverter() const;
    /** Returns the API chart document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
                        getChartDocument() const;
    /** Returns the position and size of the chart shape in 1/100 mm. */
    const ::com::sun::star::awt::Size& getChartSize() const;
    /** Returns the object formatter. */
    ObjectFormatter&    getFormatter() const;

private:
    ::boost::shared_ptr< ConverterData > mxData;
};

// ============================================================================

template< typename ModelType >
class ConverterBase : public ConverterRoot
{
public:
    inline const ModelType& getModel() const { return mrModel; }

protected:
    inline explicit     ConverterBase( const ConverterRoot& rParent, ModelType& rModel ) :
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

    /** Tries to set the position from the contained OOXML layout model.
        Returns true, if a manual position could be calculated. */
    bool                convertFromModel( PropertySet& rPropSet );
    /** Tries to set the position from the contained OOXML layout model.
        Returns true, if a manual position could be calculated. */
    bool                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            double fRotationAngle );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

