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



#include "ShapeFilterBase.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/ole/vbaproject.hxx"

namespace oox {
namespace shape {

using namespace ::com::sun::star;

ShapeFilterBase::ShapeFilterBase( const uno::Reference< uno::XComponentContext >& rxContext ) throw( uno::RuntimeException ) :
    XmlFilterBase( rxContext ),
    mxChartConv( new ::oox::drawingml::chart::ChartConverter )
{
}

ShapeFilterBase::~ShapeFilterBase()
{
}

const ::oox::drawingml::Theme* ShapeFilterBase::getCurrentTheme() const
{
    return 0;
}

::oox::vml::Drawing* ShapeFilterBase::getVmlDrawing()
{
    return 0;
}

const ::oox::drawingml::table::TableStyleListPtr ShapeFilterBase::getTableStyles()
{
    return ::oox::drawingml::table::TableStyleListPtr();
}

::oox::drawingml::chart::ChartConverter& ShapeFilterBase::getChartConverter()
{
    return *mxChartConv;
}

::oox::ole::VbaProject* ShapeFilterBase::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject( getComponentContext(), getModel(), CREATE_OUSTRING( "Writer" ) );
}

::rtl::OUString ShapeFilterBase::implGetImplementationName() const
{
    return ::rtl::OUString();
}

}
}
