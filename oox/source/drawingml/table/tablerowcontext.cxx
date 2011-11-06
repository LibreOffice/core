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



#include <osl/diagnose.h>

#include "oox/drawingml/table/tablerowcontext.hxx"
#include "oox/drawingml/table/tablecellcontext.hxx"
#include "oox/drawingml/table/tablerow.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableRowContext::TableRowContext( ContextHandler& rParent, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs, TableRow& rTableRow )
: ContextHandler( rParent )
, mrTableRow( rTableRow )
{
    rTableRow.setHeight( xAttribs->getOptionalValue( XML_h ).toInt32() );
}

TableRowContext::~TableRowContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableRowContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case A_TOKEN( tc ):         // CT_TableCell
        {
            std::vector< TableCell >& rvTableCells = mrTableRow.getTableCells();
            rvTableCells.resize( rvTableCells.size() + 1 );
            xRet.set( new TableCellContext( *this, xAttribs, rvTableCells.back() ) );
        }
        break;
    case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
    default:
        break;
    }
    if( !xRet.is() )
    {
        uno::Reference< XFastContextHandler > xTmp( this );
        xRet.set( xTmp );
    }
    return xRet;
}

} } }
