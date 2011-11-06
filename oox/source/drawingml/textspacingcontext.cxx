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



#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/textspacing.hxx"
#include "textspacingcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;


namespace oox { namespace drawingml {

    TextSpacingContext::TextSpacingContext( ContextHandler& rParent, TextSpacing & aSpacing )
        : ContextHandler( rParent )
        , maSpacing( aSpacing )
    {
        maSpacing.bHasValue = sal_True;
    }

    void TextSpacingContext::endFastElement( sal_Int32 /*nElement*/ )
        throw ( SAXException, RuntimeException )
    {
    }

    Reference< XFastContextHandler > TextSpacingContext::createFastChildContext( ::sal_Int32 aElement,
            const Reference< XFastAttributeList >& xAttribs )
        throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;
        switch( aElement )
        {
        case A_TOKEN( spcPct ):
            maSpacing.nUnit = TextSpacing::PERCENT;
            maSpacing.nValue = GetPercent( xAttribs->getValue( XML_val ) );
            break;
        case A_TOKEN( spcPts ):
            maSpacing.nUnit = TextSpacing::POINTS;
            maSpacing.nValue = GetTextSpacingPoint( xAttribs->getValue( XML_val ) );
            break;
        default:
            break;
        }
        if ( !xRet.is() )
            xRet.set( this );
        return xRet;
    }


} }
