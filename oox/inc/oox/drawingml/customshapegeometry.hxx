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



#ifndef OOX_DRAWINGML_CUSTOMSHAPEGEOMETRY_HXX
#define OOX_DRAWINGML_CUSTOMSHAPEGEOMETRY_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include "oox/helper/propertymap.hxx"
#include "oox/core/contexthandler.hxx"
#include "oox/drawingml/shape.hxx"

namespace oox { namespace drawingml {


// ---------------------------------------------------------------------
// CT_CustomGeometry2D
class CustomShapeGeometryContext : public ::oox::core::ContextHandler
{
public:
    CustomShapeGeometryContext( ::oox::core::ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, CustomShapeProperties& rCustomShapeProperties );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    CustomShapeProperties& mrCustomShapeProperties;
};

// ---------------------------------------------------------------------
// CT_PresetGeometry2D
class PresetShapeGeometryContext : public ::oox::core::ContextHandler
{
public:
    PresetShapeGeometryContext( ::oox::core::ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, CustomShapeProperties& rCustomShapeProperties );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    CustomShapeProperties& mrCustomShapeProperties;
};

// ---------------------------------------------------------------------
// CT_PresetTextShape
class PresetTextShapeContext : public ::oox::core::ContextHandler
{
public:
    PresetTextShapeContext( ::oox::core::ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, CustomShapeProperties& rCustomShapeProperties );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    CustomShapeProperties& mrCustomShapeProperties;
};

} }

#endif // OOX_DRAWINGML_CUSTOMSHAPEGEOMETRY_HXX
