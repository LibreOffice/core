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
#ifndef OOX_SHAPE_SHAPE_CONTEXT_HANDLER_HXX
#define OOX_SHAPE_SHAPE_CONTEXT_HANDLER_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XFastShapeContextHandler.hpp>
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/core/fragmenthandler2.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "ShapeFilterBase.hxx"

namespace oox { namespace shape {

class ShapeFragmentHandler : public core::FragmentHandler2
{
public:
    typedef boost::shared_ptr<ShapeFragmentHandler> Pointer_t;

    explicit ShapeFragmentHandler(core::XmlFilterBase& rFilter,
                                  const OUString& rFragmentPath )
    : FragmentHandler2(rFilter, rFragmentPath)
    {
    }
};

class ShapeContextHandler:
    public ::cppu::WeakImplHelper1<
        css::xml::sax::XFastShapeContextHandler>
{
public:
    explicit ShapeContextHandler
    (css::uno::Reference< css::uno::XComponentContext > const & context);

    virtual ~ShapeContextHandler();

    // ::com::sun::star::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL supportsService
    (const OUString & ServiceName) throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL startUnknownElement
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL endFastElement(::sal_Int32 Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL endUnknownElement
    (const OUString & Namespace,
     const OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL characters(const OUString & aChars)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    // ::com::sun::star::xml::sax::XFastShapeContextHandler:
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getShape()
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getDrawPage()
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setDrawPage
    (const css::uno::Reference< css::drawing::XDrawPage > & the_value)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel()
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setModel
    (const css::uno::Reference< css::frame::XModel > & the_value)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
    getInputStream() throw (css::uno::RuntimeException);

    virtual void SAL_CALL setInputStream
    (const css::uno::Reference< css::io::XInputStream > & the_value)
        throw (css::uno::RuntimeException);

    virtual OUString SAL_CALL getRelationFragmentPath()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRelationFragmentPath
    (const OUString & the_value)
        throw (css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getStartToken() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStartToken( ::sal_Int32 _starttoken ) throw (::com::sun::star::uno::RuntimeException);

private:
    ShapeContextHandler(ShapeContextHandler &); // not defined
    void operator =(ShapeContextHandler &); // not defined

    ::sal_uInt32 mnStartToken;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    drawingml::ShapePtr mpShape;
    ::boost::shared_ptr< vml::Drawing > mpDrawing;

    typedef boost::shared_ptr<drawingml::GraphicShapeContext>
    GraphicShapeContextPtr;
    css::uno::Reference<XFastContextHandler> mxDrawingFragmentHandler;
    css::uno::Reference<XFastContextHandler> mxGraphicShapeContext;
    css::uno::Reference<XFastContextHandler> mxDiagramShapeContext;
    css::uno::Reference<XFastContextHandler> mxLockedCanvasContext;
    css::uno::Reference<XFastContextHandler> mxChartShapeContext;

    core::XmlFilterRef mxFilterBase;
    drawingml::ThemePtr mpThemePtr;
    css::uno::Reference<css::drawing::XDrawPage> mxDrawPage;
    css::uno::Reference<css::io::XInputStream> mxInputStream;
    OUString msRelationFragmentPath;

    css::uno::Reference<XFastContextHandler> getGraphicShapeContext(::sal_Int32 Element);
    css::uno::Reference<XFastContextHandler> getChartShapeContext(::sal_Int32 Element);
    css::uno::Reference<XFastContextHandler> getDrawingShapeContext();
    css::uno::Reference<XFastContextHandler> getDiagramShapeContext();
    css::uno::Reference<XFastContextHandler> getLockedCanvasContext(sal_Int32 nElement);
    css::uno::Reference<XFastContextHandler> getContextHandler();
};

}}

#endif // OOX_SHAPE_SHAPE_CONTEXT_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
