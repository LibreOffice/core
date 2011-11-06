/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_SHAPE_SHAPEFILTERBASE_HXX
#define OOX_SHAPE_SHAPEFILTERBASE_HXX

#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include "oox/vml/vmldrawing.hxx"
#include "oox/drawingml/table/tablestylelist.hxx"
#include "oox/core/xmlfilterbase.hxx"

namespace oox {
namespace shape {

// ============================================================================

class ShapeFilterBase : public core::XmlFilterBase
{
public:
    typedef boost::shared_ptr<ShapeFilterBase> Pointer_t;

    explicit            ShapeFilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual             ~ShapeFilterBase();

    /** Has to be implemented by each filter, returns the current theme. */
    virtual const ::oox::drawingml::Theme* getCurrentTheme() const;

    /** Has to be implemented by each filter to return the collection of VML shapes. */
    virtual ::oox::vml::Drawing* getVmlDrawing();

    /** Has to be implemented by each filter to return TableStyles. */
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles();

    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter();

    virtual bool importDocument() { return true; }
    virtual bool exportDocument() { return true; }

private:
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    virtual rtl::OUString implGetImplementationName() const;

    ::boost::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;
};

// ============================================================================

} // namespace shape
} // namespace oox

#endif

