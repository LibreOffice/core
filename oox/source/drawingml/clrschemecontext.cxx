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



#include "oox/drawingml/clrschemecontext.hxx"
#include "oox/core/xmlfilterbase.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

static void setClrMap( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes,
            ClrMap& rClrMap, sal_Int32 nToken )
{
    if ( xAttributes->hasAttribute( nToken ) )
    {
        sal_Int32 nMappedToken = xAttributes->getOptionalValueToken( nToken, 0 );
        rClrMap.setColorMap( nToken, nMappedToken );
    }
}

clrMapContext::clrMapContext( ContextHandler& rParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, ClrMap& rClrMap )
: ContextHandler( rParent )
{
    setClrMap( xAttributes, rClrMap, XML_bg1 );
    setClrMap( xAttributes, rClrMap, XML_tx1 );
    setClrMap( xAttributes, rClrMap, XML_bg2 );
    setClrMap( xAttributes, rClrMap, XML_tx2 );
    setClrMap( xAttributes, rClrMap, XML_accent1 );
    setClrMap( xAttributes, rClrMap, XML_accent2 );
    setClrMap( xAttributes, rClrMap, XML_accent3 );
    setClrMap( xAttributes, rClrMap, XML_accent4 );
    setClrMap( xAttributes, rClrMap, XML_accent5 );
    setClrMap( xAttributes, rClrMap, XML_accent6 );
    setClrMap( xAttributes, rClrMap, XML_hlink );
    setClrMap( xAttributes, rClrMap, XML_folHlink );
}

clrSchemeColorContext::clrSchemeColorContext( ContextHandler& rParent, ClrScheme& rClrScheme, sal_Int32 nColorToken ) :
    ColorContext( rParent, *this ),
    mrClrScheme( rClrScheme ),
    mnColorToken( nColorToken )
{
}

clrSchemeColorContext::~clrSchemeColorContext()
{
    mrClrScheme.setColor( mnColorToken, getColor( getFilter().getGraphicHelper() ) );
}

clrSchemeContext::clrSchemeContext( ContextHandler& rParent, ClrScheme& rClrScheme ) :
    ContextHandler( rParent ),
    mrClrScheme( rClrScheme )
{
}

Reference< XFastContextHandler > clrSchemeContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    switch( nElement )
    {
        case A_TOKEN( dk1 ):
        case A_TOKEN( lt1 ):
        case A_TOKEN( dk2 ):
        case A_TOKEN( lt2 ):
        case A_TOKEN( accent1 ):
        case A_TOKEN( accent2 ):
        case A_TOKEN( accent3 ):
        case A_TOKEN( accent4 ):
        case A_TOKEN( accent5 ):
        case A_TOKEN( accent6 ):
        case A_TOKEN( hlink ):
        case A_TOKEN( folHlink ):
            return new clrSchemeColorContext( *this, mrClrScheme, getBaseToken( nElement ) );
    }
    return 0;
}

} }
