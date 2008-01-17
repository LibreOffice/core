/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShapeContextHandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "ShapeContextHandler.hxx"
#include "oox/core/fragmenthandler.hxx"

namespace oox { namespace shape {

using namespace ::com::sun::star;
using namespace core;
using namespace drawingml;

ShapeContextHandler::ShapeContextHandler
(uno::Reference< uno::XComponentContext > const & context) :
m_xContext(context)
{
    FragmentHandlerRef rFragmentHandler;
    ShapePtr pMasterShape;
    mpShape.reset(new Shape("com.sun.star.drawing.GraphicObjectShape" ));

    mxGraphicShapeContext.set(new GraphicShapeContext(rFragmentHandler,
                                                      pMasterShape,
                                                      mpShape));

    mpThemePtr.reset(new Theme());
    uno::Reference<lang::XMultiServiceFactory>
        xFactory(context->getServiceManager(), uno::UNO_QUERY_THROW);
    mpFilterBase.reset(new ShapeFilterBase(xFactory));
}

ShapeContextHandler::~ShapeContextHandler()
{
}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL ShapeContextHandler::startFastElement
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxGraphicShapeContext.is())
        mxGraphicShapeContext->startFastElement(Element, Attribs);
}

void SAL_CALL ShapeContextHandler::startUnknownElement
(const ::rtl::OUString & Namespace, const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxGraphicShapeContext.is())
        mxGraphicShapeContext->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL ShapeContextHandler::endFastElement(::sal_Int32 Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxGraphicShapeContext.is())
        mxGraphicShapeContext->endFastElement(Element);
}

void SAL_CALL ShapeContextHandler::endUnknownElement
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxGraphicShapeContext.is())
        mxGraphicShapeContext->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;

    if (mxGraphicShapeContext.is())
        xResult.set(mxGraphicShapeContext->createFastChildContext
                    (Element, Attribs));

    return xResult;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createUnknownChildContext
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxGraphicShapeContext.is())
        return mxGraphicShapeContext->createUnknownChildContext
            (Namespace, Name, Attribs);

    return uno::Reference< xml::sax::XFastContextHandler >();
}

void SAL_CALL ShapeContextHandler::characters(const ::rtl::OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxGraphicShapeContext.is())
        mxGraphicShapeContext->characters(aChars);
}

// ::com::sun::star::xml::sax::XFastShapeContextHandler:
uno::Reference< drawing::XShape > SAL_CALL
ShapeContextHandler::getShape() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xResult;
    std::map< ::rtl::OUString, ShapePtr > aShapeMap;

    if (mpFilterBase.get() != NULL && mxModel.is() &&
        mpThemePtr.get() != NULL && mxShapes.is())
    {
        mpShape->addShape
            (*mpFilterBase, mxModel, mpThemePtr, aShapeMap,
             mxShapes, NULL);

        xResult.set(mpShape->getXShape());
    }

    return xResult;
}

css::uno::Reference< css::drawing::XShapes > SAL_CALL
ShapeContextHandler::getShapes() throw (css::uno::RuntimeException)
{
    return mxShapes;
}

void SAL_CALL ShapeContextHandler::setShapes
(const css::uno::Reference< css::drawing::XShapes > & the_value)
    throw (css::uno::RuntimeException)
{
    mxShapes = the_value;
}

css::uno::Reference< css::frame::XModel > SAL_CALL
ShapeContextHandler::getModel() throw (css::uno::RuntimeException)
{
    return mxModel;
}

void SAL_CALL ShapeContextHandler::setModel
(const css::uno::Reference< css::frame::XModel > & the_value)
    throw (css::uno::RuntimeException)
{
    mxModel = the_value;
}

::rtl::OUString ShapeContextHandler::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ShapeContextHandler_getImplementationName();
}

uno::Sequence< ::rtl::OUString > ShapeContextHandler::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return ShapeContextHandler_getSupportedServiceNames();
}

::rtl::OUString SAL_CALL ShapeContextHandler_getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.oox.ShapeContextHandler"));
}

::sal_Bool SAL_CALL ShapeContextHandler::supportsService
(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSeq = getSupportedServiceNames();

    if (aSeq[0].equals(ServiceName))
        return sal_True;

    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL
ShapeContextHandler_getSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.xml.sax.FastShapeContextHandler"));
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL
ShapeContextHandler_create(
    const uno::Reference< uno::XComponentContext > & context)
        SAL_THROW((uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >
        (new ShapeContextHandler(context));
}

uno::Reference< uno::XInterface > SAL_CALL
ShapeContextHandler_createInstance
( const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
throw( uno::Exception )
{
    uno::Reference<beans::XPropertySet>
        xPropertySet(rSMgr, uno::UNO_QUERY_THROW);
    uno::Any aDefaultContext = xPropertySet->getPropertyValue
        (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")));

    uno::Reference<uno::XComponentContext> xContext;
    aDefaultContext >>= xContext;

    return ShapeContextHandler_create(xContext);
}

}}
