/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_OOX_SOURCE_SHAPE_SHAPECONTEXTHANDLER_HXX
#define INCLUDED_OOX_SOURCE_SHAPE_SHAPECONTEXTHANDLER_HXX

#include <memory>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XFastShapeContextHandler.hpp>
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/core/fragmenthandler2.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "ShapeFilterBase.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

namespace oox { namespace shape {

class ShapeFragmentHandler : public core::FragmentHandler2
{
public:
    typedef std::shared_ptr<ShapeFragmentHandler> Pointer_t;

    explicit ShapeFragmentHandler(core::XmlFilterBase& rFilter,
                                  const OUString& rFragmentPath )
    : FragmentHandler2(rFilter, rFragmentPath)
    {
    }
};

class ShapeContextHandler:
    public ::cppu::WeakImplHelper< css::xml::sax::XFastShapeContextHandler,
                                    css::lang::XServiceInfo >
{
public:
    explicit ShapeContextHandler
    (css::uno::Reference< css::uno::XComponentContext > const & context);

    virtual ~ShapeContextHandler();

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService
    (const OUString & ServiceName) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // css::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    virtual void SAL_CALL startUnknownElement
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    virtual void SAL_CALL endFastElement(::sal_Int32 Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    virtual void SAL_CALL endUnknownElement
    (const OUString & Namespace,
     const OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    virtual void SAL_CALL characters(const OUString & aChars)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    // css::xml::sax::XFastShapeContextHandler:
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getShape()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getDrawPage()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setDrawPage
    (const css::uno::Reference< css::drawing::XDrawPage > & the_value)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setModel
    (const css::uno::Reference< css::frame::XModel > & the_value)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getRelationFragmentPath()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRelationFragmentPath
    (const OUString & the_value)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual ::sal_Int32 SAL_CALL getStartToken() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStartToken( ::sal_Int32 _starttoken ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getPosition() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition(const css::awt::Point& rPosition) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setDocumentProperties(const css::uno::Reference<css::document::XDocumentProperties>& xDocProps)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<css::document::XDocumentProperties> SAL_CALL getDocumentProperties()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getMediaDescriptor()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMediaDescriptor(const css::uno::Sequence<css::beans::PropertyValue>& rMediaDescriptor)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    ShapeContextHandler(ShapeContextHandler &) = delete;
    void operator =(ShapeContextHandler &) = delete;

    ::sal_uInt32 mnStartToken;
    css::awt::Point maPosition;

    drawingml::ShapePtr mpShape;
    std::shared_ptr< vml::Drawing > mpDrawing;

    typedef std::shared_ptr<drawingml::GraphicShapeContext>
    GraphicShapeContextPtr;
    css::uno::Reference<XFastContextHandler> mxDrawingFragmentHandler;
    css::uno::Reference<XFastContextHandler> mxGraphicShapeContext;
    css::uno::Reference<XFastContextHandler> mxDiagramShapeContext;
    css::uno::Reference<XFastContextHandler> mxLockedCanvasContext;
    css::uno::Reference<XFastContextHandler> mxWpsContext;
    css::uno::Reference<css::drawing::XShape> mxSavedShape;
    css::uno::Reference<XFastContextHandler> mxWpgContext;
    css::uno::Reference<XFastContextHandler> mxChartShapeContext;
    css::uno::Reference<css::document::XDocumentProperties> mxDocumentProperties;
    css::uno::Sequence<css::beans::PropertyValue> maMediaDescriptor;

    ::rtl::Reference< core::XmlFilterBase > mxFilterBase;
    drawingml::ThemePtr mpThemePtr;
    css::uno::Reference<css::drawing::XDrawPage> mxDrawPage;
    OUString msRelationFragmentPath;

    css::uno::Reference<XFastContextHandler> const & getGraphicShapeContext(::sal_Int32 Element);
    css::uno::Reference<XFastContextHandler> const & getChartShapeContext(::sal_Int32 Element);
    css::uno::Reference<XFastContextHandler> const & getDrawingShapeContext();
    css::uno::Reference<XFastContextHandler> const & getDiagramShapeContext();
    css::uno::Reference<XFastContextHandler> const & getLockedCanvasContext(sal_Int32 nElement);
    css::uno::Reference<XFastContextHandler> const & getWpsContext(sal_Int32 nStartElement, sal_Int32 nElement);
    css::uno::Reference<XFastContextHandler> const & getWpgContext(sal_Int32 nElement);
    css::uno::Reference<XFastContextHandler> getContextHandler(sal_Int32 nElement = 0);
};

}}

#endif // INCLUDED_OOX_SOURCE_SHAPE_SHAPECONTEXTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
