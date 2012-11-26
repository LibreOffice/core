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



#include "oox/drawingml/transform2dcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/shape.hxx"

using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;
using ::oox::core::ContextHandler;

namespace oox {
namespace drawingml {

// ============================================================================

/** context to import a CT_Transform2D */
Transform2DContext::Transform2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, Shape& rShape ) throw()
: ContextHandler( rParent )
, mrShape( rShape )
{
    AttributeList aAttributeList( xAttribs );
    mrShape.setRotation( aAttributeList.getInteger( XML_rot, 0 ) ); // 60000ths of a degree Positive angles are clockwise; negative angles are counter-clockwise
    //  TTTT: MirrorX/Y removed
    //mrShape.setFlip( aAttributeList.getBool( XML_flipH, sal_False ), aAttributeList.getBool( XML_flipV, sal_False ) );
}

Reference< XFastContextHandler > Transform2DContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    switch( aElementToken )
    {
    case A_TOKEN( off ):        // horz/vert translation
        mrShape.setPosition( Point( xAttribs->getOptionalValue( XML_x ).toInt32(), xAttribs->getOptionalValue( XML_y ).toInt32() ) );
        break;
    case A_TOKEN( ext ):        // horz/vert size
        mrShape.setSize( Size( xAttribs->getOptionalValue( XML_cx ).toInt32(), xAttribs->getOptionalValue( XML_cy ).toInt32() ) );
        break;
/* todo: what to do?
    case A_TOKEN( chOff ):  // horz/vert translation of children
    case A_TOKEN( chExt ):  // horz/vert size of children
        break;
*/
    }

    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

