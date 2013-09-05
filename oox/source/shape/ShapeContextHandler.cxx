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

#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>

#include "ShapeContextHandler.hxx"
#include "ShapeDrawingFragmentHandler.hxx"
#include "LockedCanvasContext.hxx"
#include "oox/vml/vmldrawingfragment.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"

namespace oox { namespace shape {

using namespace ::com::sun::star;
using namespace core;
using namespace drawingml;

OUString SAL_CALL ShapeContextHandler_getImplementationName()
{
    return OUString( "com.sun.star.comp.oox.ShapeContextHandler" );
}

uno::Sequence< OUString > SAL_CALL
ShapeContextHandler_getSupportedServiceNames()
{
    uno::Sequence< OUString > s(1);
    s[0] = "com.sun.star.xml.sax.FastShapeContextHandler";
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL
ShapeContextHandler_createInstance( const uno::Reference< uno::XComponentContext > & context)
        SAL_THROW((uno::Exception))
{
    return static_cast< ::cppu::OWeakObject* >( new ShapeContextHandler(context) );
}


ShapeContextHandler::ShapeContextHandler
(uno::Reference< uno::XComponentContext > const & context) :
mnStartToken(0), m_xContext(context)
{
    try
    {
        mxFilterBase.set( new ShapeFilterBase(context) );
    }
    catch( uno::Exception& )
    {
    }
}

ShapeContextHandler::~ShapeContextHandler()
{
}

uno::Reference<xml::sax::XFastContextHandler> ShapeContextHandler::getLockedCanvasContext(sal_Int32 nElement)
{
    if (!mxLockedCanvasContext.is())
    {
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
        ShapePtr pMasterShape;

        switch (nElement & 0xffff)
        {
            case XML_lockedCanvas:
                mxLockedCanvasContext.set(new LockedCanvasContext(*rFragmentHandler));
                break;
            default:
                break;
        }
    }

    return mxLockedCanvasContext;
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getGraphicShapeContext(::sal_Int32 Element )
{
    if (! mxGraphicShapeContext.is())
    {
        boost::shared_ptr<ContextHandler2Helper> pFragmentHandler
            (new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
        ShapePtr pMasterShape;

        switch (Element & 0xffff)
        {
            case XML_graphic:
                mpShape.reset(new Shape("com.sun.star.drawing.GraphicObjectShape" ));
                mxGraphicShapeContext.set
                (new GraphicalObjectFrameContext(*pFragmentHandler, pMasterShape, mpShape, true));
                break;
            case XML_pic:
                mpShape.reset(new Shape("com.sun.star.drawing.GraphicObjectShape" ));
                mxGraphicShapeContext.set
                (new GraphicShapeContext(*pFragmentHandler, pMasterShape, mpShape));
                break;
            default:
                break;
        }
    }

    return mxGraphicShapeContext;
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getDrawingShapeContext()
{
    if (!mxDrawingFragmentHandler.is())
    {
        mpDrawing.reset( new oox::vml::Drawing( *mxFilterBase, mxDrawPage, oox::vml::VMLDRAWING_WORD ) );
        mxDrawingFragmentHandler.set
          (dynamic_cast<ContextHandler *>
           (new oox::vml::DrawingFragment
            ( *mxFilterBase, msRelationFragmentPath, *mpDrawing )));
    }

    return mxDrawingFragmentHandler;
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getDiagramShapeContext()
{
    if (!mxDiagramShapeContext.is())
    {
        boost::shared_ptr<ContextHandler2Helper> pFragmentHandler(new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
        mpShape.reset(new Shape());
        mxDiagramShapeContext.set(new DiagramGraphicDataContext(*pFragmentHandler, mpShape));
    }

    return mxDiagramShapeContext;
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getContextHandler()
{
    uno::Reference<xml::sax::XFastContextHandler> xResult;

    switch (getNamespace( mnStartToken ))
    {
        case NMSP_doc:
        case NMSP_vml:
            xResult.set(getDrawingShapeContext());
            break;
        case NMSP_dmlDiagram:
            xResult.set(getDiagramShapeContext());
            break;
        case NMSP_dmlLockedCanvas:
            xResult.set(getLockedCanvasContext(mnStartToken));
            break;
        default:
            xResult.set(getGraphicShapeContext(mnStartToken));
            break;
    }

    return xResult;
}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL ShapeContextHandler::startFastElement
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    static const OUString sInputStream
        ("InputStream");

    uno::Sequence<beans::PropertyValue> aSeq(1);
    aSeq[0].Name = sInputStream;
    aSeq[0].Value <<= mxInputStream;
    mxFilterBase->filter(aSeq);

    mpThemePtr.reset(new Theme());

    if (Element == DGM_TOKEN(relIds) || Element == LC_TOKEN(lockedCanvas))
    {
        // Parse the theme relation, if available; the diagram won't have colors without it.
        if (!msRelationFragmentPath.isEmpty())
        {
            FragmentHandlerRef rFragmentHandler(new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
            OUString aThemeFragmentPath = rFragmentHandler->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "theme" ) );
            uno::Reference<xml::sax::XFastSAXSerializable> xDoc(mxFilterBase->importFragment(aThemeFragmentPath), uno::UNO_QUERY_THROW);
            mxFilterBase->importFragment(new ThemeFragmentHandler(*mxFilterBase, aThemeFragmentPath, *mpThemePtr ), xDoc);
            ShapeFilterBase* pShapeFilterBase(dynamic_cast<ShapeFilterBase*>(mxFilterBase.get()));
            if (pShapeFilterBase)
                pShapeFilterBase->setCurrentTheme(mpThemePtr);
        }

        createFastChildContext(Element, Attribs);
    }

    // Entering VML block (startFastElement() is called for the outermost tag),
    // handle possible recursion.
    if ( getContextHandler() == getDrawingShapeContext() )
        mpDrawing->getShapes().pushMark();

    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->startFastElement(Element, Attribs);
}

void SAL_CALL ShapeContextHandler::startUnknownElement
(const OUString & Namespace, const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    if ( getContextHandler() == getDrawingShapeContext() )
        mpDrawing->getShapes().pushMark();

    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL ShapeContextHandler::endFastElement(::sal_Int32 Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->endFastElement(Element);
}

void SAL_CALL ShapeContextHandler::endUnknownElement
(const OUString & Namespace,
 const OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xResult.set(xContextHandler->createFastChildContext
                    (Element, Attribs));

    return xResult;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createUnknownChildContext
(const OUString & Namespace,
 const OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        return xContextHandler->createUnknownChildContext
            (Namespace, Name, Attribs);

    return uno::Reference< xml::sax::XFastContextHandler >();
}

void SAL_CALL ShapeContextHandler::characters(const OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->characters(aChars);
}

// ::com::sun::star::xml::sax::XFastShapeContextHandler:
uno::Reference< drawing::XShape > SAL_CALL
ShapeContextHandler::getShape() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xResult;
    uno::Reference< drawing::XShapes > xShapes( mxDrawPage, uno::UNO_QUERY );

    if (mxFilterBase.is() && xShapes.is())
    {
        if ( getContextHandler() == getDrawingShapeContext() )
        {
            mpDrawing->finalizeFragmentImport();
            if( boost::shared_ptr< vml::ShapeBase > pShape = mpDrawing->getShapes().takeLastShape() )
                xResult = pShape->convertAndInsert( xShapes );
            // Only now remove the recursion mark, because getShape() is called in writerfilter
            // after endFastElement().
            mpDrawing->getShapes().popMark();
        }
        else if (mxDiagramShapeContext.is())
        {
            basegfx::B2DHomMatrix aMatrix;
            if (mpShape->getExtDrawings().size() == 0)
            {
                mpShape->addShape( *mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, mpShape->getFillProperties() );
                xResult = mpShape->getXShape();
            }
            else
            {
                // Prerendered diagram output is available, then use that, and throw away the original result.
                for (std::vector<OUString>::const_iterator aIt = mpShape->getExtDrawings().begin(); aIt != mpShape->getExtDrawings().end(); ++aIt)
                {
                    DiagramGraphicDataContext* pDiagramGraphicDataContext = dynamic_cast<DiagramGraphicDataContext*>(mxDiagramShapeContext.get());
                    OUString aFragmentPath(pDiagramGraphicDataContext->getFragmentPathFromRelId(*aIt));
                    oox::drawingml::ShapePtr pShapePtr( new Shape( "com.sun.star.drawing.GroupShape" ) );
                    mxFilterBase->importFragment(new ShapeDrawingFragmentHandler(*mxFilterBase, aFragmentPath, pShapePtr));
                    pShapePtr->addShape( *mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShapePtr->getFillProperties() );
                    xResult = pShapePtr->getXShape();
                }
                mpShape.reset((Shape*)0);
            }
            mxDiagramShapeContext.clear();
        }
        else if (mxLockedCanvasContext.is())
        {
            ShapePtr pShape = dynamic_cast<LockedCanvasContext*>(mxLockedCanvasContext.get())->getShape();
            if (pShape)
            {
                basegfx::B2DHomMatrix aMatrix;
                pShape->addShape(*mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
                xResult = pShape->getXShape();
                mxLockedCanvasContext.clear();
            }
        }
        else if (mpShape.get() != NULL)
        {
            basegfx::B2DHomMatrix aTransformation;
            mpShape->addShape(*mxFilterBase, mpThemePtr.get(), xShapes, aTransformation, mpShape->getFillProperties() );
            xResult.set(mpShape->getXShape());
            mxGraphicShapeContext.clear( );
        }
    }

    return xResult;
}

css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
ShapeContextHandler::getDrawPage() throw (css::uno::RuntimeException)
{
    return mxDrawPage;
}

void SAL_CALL ShapeContextHandler::setDrawPage
(const css::uno::Reference< css::drawing::XDrawPage > & the_value)
    throw (css::uno::RuntimeException)
{
    mxDrawPage = the_value;
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

OUString SAL_CALL ShapeContextHandler::getRelationFragmentPath()
    throw (uno::RuntimeException)
{
    return msRelationFragmentPath;
}

void SAL_CALL ShapeContextHandler::setRelationFragmentPath
(const OUString & the_value)
    throw (uno::RuntimeException)
{
    msRelationFragmentPath = the_value;
}

::sal_Int32 SAL_CALL ShapeContextHandler::getStartToken() throw (::com::sun::star::uno::RuntimeException)
{
    return mnStartToken;
}

void SAL_CALL ShapeContextHandler::setStartToken( ::sal_Int32 _starttoken ) throw (::com::sun::star::uno::RuntimeException)
{
    mnStartToken = _starttoken;


}

OUString ShapeContextHandler::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ShapeContextHandler_getImplementationName();
}

uno::Sequence< OUString > ShapeContextHandler::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return ShapeContextHandler_getSupportedServiceNames();
}

::sal_Bool SAL_CALL ShapeContextHandler::supportsService
(const OUString & ServiceName) throw (css::uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq = getSupportedServiceNames();

    if (aSeq[0].equals(ServiceName))
        return sal_True;

    return sal_False;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
