/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef OOX_SHAPE_SHAPE_CONTEXT_HANDLER_HXX
#define OOX_SHAPE_SHAPE_CONTEXT_HANDLER_HXX

#include <boost/shared_ptr.hpp>
#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/xml/sax/XFastShapeContextHandler.hpp"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "ShapeFilterBase.hxx"

namespace css = ::com::sun::star;

namespace oox { namespace shape {

// component and service helper functions:
::rtl::OUString SAL_CALL ShapeContextHandler_getImplementationName();

css::uno::Sequence< ::rtl::OUString > SAL_CALL
ShapeContextHandler_getSupportedServiceNames();

css::uno::Reference< css::uno::XInterface > SAL_CALL
ShapeContextHandler_create
( css::uno::Reference< css::uno::XComponentContext > const & context );

css::uno::Reference< css::uno::XInterface > SAL_CALL
ShapeContextHandler_createInstance
( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
throw( css::uno::Exception );

class ShapeFragmentHandler : public core::FragmentHandler
{
public:
    typedef boost::shared_ptr<ShapeFragmentHandler> Pointer_t;

    explicit ShapeFragmentHandler(core::XmlFilterBase& rFilter,
                                  const ::rtl::OUString& rFragmentPath )
    : FragmentHandler(rFilter, rFragmentPath)
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
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL supportsService
    (const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL startUnknownElement
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL endFastElement(::sal_Int32 Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL endUnknownElement
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const ::rtl::OUString & Namespace,
     const ::rtl::OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

    virtual void SAL_CALL characters(const ::rtl::OUString & aChars)
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

    virtual ::rtl::OUString SAL_CALL getRelationFragmentPath()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRelationFragmentPath
    (const ::rtl::OUString & the_value)
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

    core::XmlFilterRef mxFilterBase;
    drawingml::ThemePtr mpThemePtr;
    css::uno::Reference<css::drawing::XDrawPage> mxDrawPage;
    css::uno::Reference<css::io::XInputStream> mxInputStream;
    ::rtl::OUString msRelationFragmentPath;

    css::uno::Reference<XFastContextHandler> getGraphicShapeContext(::sal_Int32 Element);
    css::uno::Reference<XFastContextHandler> getDrawingShapeContext();
    css::uno::Reference<XFastContextHandler> getContextHandler();
};

}}

#endif // OOX_SHAPE_SHAPE_CONTEXT_HANDLER_HXX
