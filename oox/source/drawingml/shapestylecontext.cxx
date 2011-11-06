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



#include "oox/drawingml/shapestylecontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// ---------------
// CT_ShapeStyle
// ---------------
ShapeStyleContext::ShapeStyleContext( ContextHandler& rParent, Shape& rShape )
: ContextHandler( rParent )
, mrShape( rShape )
{
}

ShapeStyleContext::~ShapeStyleContext()
{
}

// --------------------------------------------------------------------

void ShapeStyleContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > ShapeStyleContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rxAttributes )
    throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xRet;
    AttributeList aAttribs( rxAttributes );
    switch( aElementToken )
    {
        case A_TOKEN( lnRef ) :     // CT_StyleMatrixReference
        case A_TOKEN( fillRef ) :   // CT_StyleMatrixReference
        case A_TOKEN( effectRef ) : // CT_StyleMatrixReference
        case A_TOKEN( fontRef ) :   // CT_FontReference
        {
            sal_Int32 nToken = getBaseToken( aElementToken );
            ShapeStyleRef& rStyleRef = mrShape.getShapeStyleRefs()[ nToken ];
            rStyleRef.mnThemedIdx = (nToken == XML_fontRef) ? aAttribs.getToken( XML_idx, XML_none ) : aAttribs.getInteger( XML_idx, 0 );
            xRet.set( new ColorContext( *this, rStyleRef.maPhClr ) );
        }
        break;
    }
    return xRet;
}

// --------------------------------------------------------------------

} }

