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



#include "oox/drawingml/table/tablestylelistfragmenthandler.hxx"
#include "oox/drawingml/table/tablestylecontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace drawingml {
namespace table {

// ============================================================================

TableStyleListFragmentHandler::TableStyleListFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, TableStyleList& rTableStyleList ):
FragmentHandler2( rFilter, rFragmentPath ),
mrTableStyleList( rTableStyleList )
{
}

TableStyleListFragmentHandler::~TableStyleListFragmentHandler()
{
}

// CT_TableStyleList
Reference< XFastContextHandler > TableStyleListFragmentHandler::createFastChildContext(
    sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case A_TOKEN( tblStyleLst ):    // CT_TableStyleList
            mrTableStyleList.getDefaultStyleId() = xAttribs->getOptionalValue( XML_def );
            break;
        case A_TOKEN( tblStyle ):       // CT_TableStyle
            std::vector< TableStyle >& rTableStyles = mrTableStyleList.getTableStyles();
            rTableStyles.resize( rTableStyles.size() + 1 );
            xRet = new TableStyleContext( *this, xAttribs, rTableStyles.back() );
            break;
    }
    if ( !xRet.is() )
        xRet = getFastContextHandler();
    return xRet;
}

// ============================================================================

} // namespace table
} // namespace drawingml
} // namespace oox

