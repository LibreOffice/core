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



#include "oox/drawingml/textbodyproperties.hxx"
#include <com/sun/star/text/WritingMode.hpp>
#include "oox/token/tokens.hxx"

namespace oox {
namespace drawingml {

// ============================================================================

TextBodyProperties::TextBodyProperties()
{
}

void TextBodyProperties::pushToPropMap( PropertyMap& rPropMap ) const
{
    rPropMap.insert( maPropertyMap.begin(), maPropertyMap.end() );

    // #160799# fake different vertical text modes by top-bottom writing mode
    if( moVert.get( XML_horz ) != XML_horz )
        rPropMap[ PROP_TextWritingMode ] <<= ::com::sun::star::text::WritingMode_TB_RL;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

