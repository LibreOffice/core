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

#include "oox/drawingml/table/tablepartstylecontext.hxx"
#include "oox/drawingml/table/tablestyletextstylecontext.hxx"
#include "oox/drawingml/table/tablestylecellstylecontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TablePartStyleContext::TablePartStyleContext( ContextHandler& rParent, TableStylePart& rTableStylePart )
: ContextHandler( rParent )
, mrTableStylePart( rTableStylePart )
{
}

TablePartStyleContext::~TablePartStyleContext()
{
}

// CT_TablePartStyle
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TablePartStyleContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
        case A_TOKEN( tcTxStyle ):  // CT_TableStyleTextStyle
            xRet.set( new TableStyleTextStyleContext( *this, xAttribs, mrTableStylePart ) );
            break;
        case A_TOKEN( tcStyle ):    // CT_TableStyleCellStyle
            xRet.set( new TableStyleCellStyleContext( *this,  mrTableStylePart ) );
            break;
    }
    if( !xRet.is() )
    {
        uno::Reference<XFastContextHandler> xTmp(this);
        xRet.set( xTmp );
    }
    return xRet;
}

} } }
