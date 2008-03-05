/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShapeContextHandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:52:58 $
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
#include "oox/vml/drawingfragmenthandler.hxx"

namespace oox { namespace shape {

using namespace ::com::sun::star;
using namespace core;
using namespace drawingml;

ShapeContextHandler::ShapeContextHandler
(uno::Reference< uno::XComponentContext > const & context) :
m_xContext(context)
{
    try
    {
        uno::Reference<lang::XMultiServiceFactory>
            xFactory(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        mxFilterBase.set( new ShapeFilterBase(xFactory) );
    }
    catch( uno::Exception& )
    {
    }
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
    static const ::rtl::OUString sInputStream
        (RTL_CONSTASCII_USTRINGPARAM ("InputStream"));

    uno::Sequence<beans::PropertyValue> aSeq(1);
    aSeq[0].Name = sInputStream;
    aSeq[0].Value <<= mxInputStream;
    mxFilterBase->filter(aSeq);

//    FragmentHandlerRef rFragmentHandler
//        (new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));

    ShapePtr pMasterShape;
    mpDrawing.reset( new oox::vml::Drawing() );
    mpShape.reset(new Shape("com.sun.star.drawing.GraphicObjectShape" ));


    mxDrawingFragmentHandler.set
        (new oox::vml::DrawingFragmentHandler( *mxFilterBase, msRelationFragmentPath, mpDrawing ));

    mpThemePtr.reset(new Theme());

    if (mxDrawingFragmentHandler.is())
        mxDrawingFragmentHandler->startFastElement(Element, Attribs);
}

void SAL_CALL ShapeContextHandler::startUnknownElement
(const ::rtl::OUString & Namespace, const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxDrawingFragmentHandler.is())
        mxDrawingFragmentHandler->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL ShapeContextHandler::endFastElement(::sal_Int32 Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxDrawingFragmentHandler.is())
        mxDrawingFragmentHandler->endFastElement(Element);
}

void SAL_CALL ShapeContextHandler::endUnknownElement
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxDrawingFragmentHandler.is())
        mxDrawingFragmentHandler->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;

    if (mxDrawingFragmentHandler.is())
        xResult.set(mxDrawingFragmentHandler->createFastChildContext
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
    if (mxDrawingFragmentHandler.is())
        return mxDrawingFragmentHandler->createUnknownChildContext
            (Namespace, Name, Attribs);

    return uno::Reference< xml::sax::XFastContextHandler >();
}

void SAL_CALL ShapeContextHandler::characters(const ::rtl::OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if (mxDrawingFragmentHandler.is())
        mxDrawingFragmentHandler->characters(aChars);
}

// ::com::sun::star::xml::sax::XFastShapeContextHandler:
uno::Reference< drawing::XShape > SAL_CALL
ShapeContextHandler::getShape() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xResult;

    if (mxFilterBase.is() && mxShapes.is())
    {
        std::vector< oox::vml::ShapePtr >& rShapes = mpDrawing->getShapes();
        if ( rShapes.size() )
        {
            rShapes[ 0 ]->addShape( *mxFilterBase, mxShapes );

//          mpShape->addShape(*mxFilterBase, mpThemePtr, mxShapes);

            uno::Reference<drawing::XShape> xShape(mpShape->getXShape());
            awt::Point aPoint(xShape->getPosition());
            awt::Size aSize(xShape->getSize());
            aSize.Width = 1000;
            aSize.Height = 1000;
            xShape->setSize(aSize);
            xResult.set(xShape);
        }
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
    if( !mxFilterBase.is() )
        throw uno::RuntimeException();
    return mxFilterBase->getModel();
}

void SAL_CALL ShapeContextHandler::setModel
(const css::uno::Reference< css::frame::XModel > & the_value)
    throw (css::uno::RuntimeException)
{
    if( !mxFilterBase.is() )
        throw uno::RuntimeException();
    uno::Reference<lang::XComponent> xComp(the_value, uno::UNO_QUERY_THROW);
    mxFilterBase->setTargetDocument(xComp);
}

uno::Reference< io::XInputStream > SAL_CALL
ShapeContextHandler::getInputStream() throw (uno::RuntimeException)
{
    return mxInputStream;
}

void SAL_CALL ShapeContextHandler::setInputStream
(const uno::Reference< io::XInputStream > & the_value)
    throw (uno::RuntimeException)
{
    mxInputStream = the_value;
}

::rtl::OUString SAL_CALL ShapeContextHandler::getRelationFragmentPath()
    throw (uno::RuntimeException)
{
    return msRelationFragmentPath;
}

void SAL_CALL ShapeContextHandler::setRelationFragmentPath
(const ::rtl::OUString & the_value)
    throw (uno::RuntimeException)
{
    msRelationFragmentPath = the_value;
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
