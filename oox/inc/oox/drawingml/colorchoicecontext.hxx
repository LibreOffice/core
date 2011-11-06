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



#ifndef OOX_DRAWINGML_COLORCHOICECONTEXT_HXX
#define OOX_DRAWINGML_COLORCHOICECONTEXT_HXX

#include "oox/core/contexthandler.hxx"

namespace oox {
namespace drawingml {

class Color;

// ============================================================================

/** Context handler for the different color value elements (a:scrgbClr,
    a:srgbClr, a:hslClr, a:sysClr, a:schemeClr, a:prstClr). */
class ColorValueContext : public ::oox::core::ContextHandler
{
public:
    explicit            ColorValueContext( ::oox::core::ContextHandler& rParent, Color& rColor );

    virtual void SAL_CALL startFastElement(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    Color&              mrColor;
};

// ============================================================================

/** Context handler for elements that *contain* a color value element
    (a:scrgbClr, a:srgbClr, a:hslClr, a:sysClr, a:schemeClr, a:prstClr). */
class ColorContext : public ::oox::core::ContextHandler
{
public:
    explicit            ColorContext( ::oox::core::ContextHandler& rParent, Color& rColor );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    Color&              mrColor;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

