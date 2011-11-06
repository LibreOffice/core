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

#include "oox/drawingml/table/tablestylecontext.hxx"
#include "oox/drawingml/table/tablebackgroundstylecontext.hxx"
#include "oox/drawingml/table/tablepartstylecontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableStyleContext::TableStyleContext( ContextHandler& rParent,
    const Reference< XFastAttributeList >& xAttribs, TableStyle& rTableStyle )
: ContextHandler( rParent )
, mrTableStyle( rTableStyle )
{
    mrTableStyle.getStyleId() = xAttribs->getOptionalValue( XML_styleId );
    mrTableStyle.getStyleName() = xAttribs->getOptionalValue( XML_styleName );
}

TableStyleContext::~TableStyleContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableStyleContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& /* xAttribs */ )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
        case A_TOKEN( tblBg ):      // CT_TableBackgroundStyle
            xRet = new TableBackgroundStyleContext( *this, mrTableStyle );
            break;
        case A_TOKEN( wholeTbl ):   // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getWholeTbl() );
            break;
        case A_TOKEN( band1H ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand1H() );
            break;
        case A_TOKEN( band2H ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand2H() );
            break;
        case A_TOKEN( band1V ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand1V() );
            break;
        case A_TOKEN( band2V ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand2V() );
            break;
        case A_TOKEN( lastCol ):    // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getLastCol() );
            break;
        case A_TOKEN( firstCol ):   // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getFirstCol() );
            break;
        case A_TOKEN( lastRow ):    // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getLastRow() );
            break;
        case A_TOKEN( seCell ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getSeCell() );
            break;
        case A_TOKEN( swCell ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getSwCell() );
            break;
        case A_TOKEN( firstRow ):   // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getFirstRow() );
            break;
        case A_TOKEN( neCell ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getNeCell() );
            break;
        case A_TOKEN( nwCell ):     // CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getNwCell() );
            break;
        case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
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
