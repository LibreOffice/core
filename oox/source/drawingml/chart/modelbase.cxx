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



#include "oox/drawingml/chart/modelbase.hxx"

#include "oox/helper/attributelist.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

using ::rtl::OUString;

// ============================================================================

NumberFormat::NumberFormat() :
    mbSourceLinked( true )
{
}

void NumberFormat::setAttributes( const AttributeList& rAttribs )
{
    maFormatCode = rAttribs.getString( XML_formatCode, OUString() );
    // default is 'false', not 'true' as specified
    mbSourceLinked = rAttribs.getBool( XML_sourceLinked, false );
}

// ============================================================================

LayoutModel::LayoutModel() :
    mfX( 0.0 ),
    mfY( 0.0 ),
    mfW( 0.0 ),
    mfH( 0.0 ),
    mnXMode( XML_factor ),
    mnYMode( XML_factor ),
    mnWMode( XML_factor ),
    mnHMode( XML_factor ),
    mnTarget( XML_outer ),
    mbAutoLayout( true )
{
}

LayoutModel::~LayoutModel()
{
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox
