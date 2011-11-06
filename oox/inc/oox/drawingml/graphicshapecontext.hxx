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



#ifndef OOX_DRAWINGML_GRAPHICSHAPECONTEXT_HXX
#define OOX_DRAWINGML_GRAPHICSHAPECONTEXT_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/diagram/diagram.hxx"

namespace oox { namespace vml { struct OleObjectInfo; } }

namespace oox { namespace drawingml {

class GraphicShapeContext : public ShapeContext
{
public:
    GraphicShapeContext( ::oox::core::ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

};

// ====================================================================

class GraphicalObjectFrameContext : public ShapeContext
{
public:
    GraphicalObjectFrameContext( ::oox::core::ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr, bool bEmbedShapesInChart );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    bool                mbEmbedShapesInChart;
};

// ====================================================================

class OleObjectGraphicDataContext : public ShapeContext
{
public:
    OleObjectGraphicDataContext( ::oox::core::ContextHandler& rParent, ShapePtr pShapePtr );
    ~OleObjectGraphicDataContext();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    ::oox::vml::OleObjectInfo& mrOleObjectInfo;
};

// ====================================================================

class DiagramGraphicDataContext
    : public ShapeContext
{
public:
    DiagramGraphicDataContext( ::oox::core::ContextHandler& rParent, ShapePtr pShapePtr );
    virtual ~DiagramGraphicDataContext();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    DiagramPtr loadDiagram();

    ::rtl::OUString msDm;
    ::rtl::OUString msLo;
    ::rtl::OUString msQs;
    ::rtl::OUString msCs;
};

// ====================================================================

class ChartGraphicDataContext : public ShapeContext
{
public:
    explicit            ChartGraphicDataContext(
                            ::oox::core::ContextHandler& rParent,
                            const ShapePtr& rxShape, bool bEmbedShapes );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    ChartShapeInfo&     mrChartShapeInfo;
};

// ====================================================================

} }

#endif  //  OOX_DRAWINGML_GRAPHICSHAPECONTEXT_HXX
