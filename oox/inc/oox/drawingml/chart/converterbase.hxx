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
 * $Revision: 1.2 $
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

#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace chart2 { class XChartDocument; }
} } }

namespace oox { namespace core {
    class FilterBase;
} }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

class ChartConverter;
struct ConverterData;

class ConverterRoot
{
public:
    explicit            ConverterRoot(
                            ::oox::core::FilterBase& rFilter,
                            ChartConverter& rChartConverter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc );
    virtual             ~ConverterRoot();

    /** Creates an instance for the passed service name, using the passed service factory. */
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        createInstance(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            const ::rtl::OUString& rServiceName );

    /** Creates an instance for the passed service name, using the process service factory. */
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        createInstance( const ::rtl::OUString& rServiceName );

protected:
    /** Returns the filter object of the imported/exported document. */
    ::oox::core::FilterBase& getFilter() const;
    /** Returns the chart converter. */
    ChartConverter&     getChartConverter() const;
    /** Returns the API chart document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
                        getChartDocument() const;

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

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

