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



#include "oox/vml/vmltextbox.hxx"

#include <rtl/ustrbuf.hxx>

namespace oox {
namespace vml {

// ============================================================================

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

TextFontModel::TextFontModel()
{
}

// ============================================================================

TextPortionModel::TextPortionModel( const TextFontModel& rFont, const OUString& rText ) :
    maFont( rFont ),
    maText( rText )
{
}

// ============================================================================

TextBox::TextBox()
{
}

void TextBox::appendPortion( const TextFontModel& rFont, const OUString& rText )
{
    maPortions.push_back( TextPortionModel( rFont, rText ) );
}

const TextFontModel* TextBox::getFirstFont() const
{
    return maPortions.empty() ? 0 : &maPortions.front().maFont;
}

OUString TextBox::getText() const
{
    OUStringBuffer aBuffer;
    for( PortionVector::const_iterator aIt = maPortions.begin(), aEnd = maPortions.end(); aIt != aEnd; ++aIt )
        aBuffer.append( aIt->maText );
    return aBuffer.makeStringAndClear();
}

// ============================================================================

} // namespace vml
} // namespace oox
