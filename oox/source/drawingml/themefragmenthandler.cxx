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



#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/drawingml/objectdefaultcontext.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/themeelementscontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;

namespace oox {
namespace drawingml {

// ============================================================================

ThemeFragmentHandler::ThemeFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, Theme& rTheme ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    mrTheme( rTheme )
{
}

ThemeFragmentHandler::~ThemeFragmentHandler()
{
}

ContextHandlerRef ThemeFragmentHandler::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    // CT_OfficeStyleSheet
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            switch( nElement )
            {
                case A_TOKEN( theme ):
                    return this;
            }
        break;

        case A_TOKEN( theme ):
            switch( nElement )
            {
                case A_TOKEN( themeElements ):              // CT_BaseStyles
                    return new ThemeElementsContext( *this, mrTheme );
                case A_TOKEN( objectDefaults ):             // CT_ObjectStyleDefaults
                    return new objectDefaultContext( *this, mrTheme );
                case A_TOKEN( extraClrSchemeLst ):          // CT_ColorSchemeList
                    return 0;
                case A_TOKEN( custClrLst ):                 // CustomColorList
                    return 0;
                case A_TOKEN( ext ):                        // CT_OfficeArtExtension
                    return 0;
            }
        break;
    }
    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

