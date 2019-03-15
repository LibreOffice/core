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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>

#include "ShapeContextHandler.hxx"
#include "ShapeDrawingFragmentHandler.hxx"
#include "LockedCanvasContext.hxx"
#include "WpsContext.hxx"
#include "WpgContext.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <oox/vml/vmldrawingfragment.hxx>
#include <oox/vml/vmlshape.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <memory>

using namespace ::com::sun::star;

namespace oox { namespace shape {
using namespace core;
using namespace drawingml;

ShapeContextHandler::ShapeContextHandler(uno::Reference< uno::XComponentContext > const & context) :
  mnStartToken(0)
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

uno::Reference<xml::sax::XFastContextHandler> const & ShapeContextHandler::getLockedCanvasContext(sal_Int32 nElement)
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

/*
 * This method creates new ChartGraphicDataContext Object.
 */
uno::Reference<xml::sax::XFastContextHandler> const & ShapeContextHandler::getChartShapeContext(sal_Int32 nElement)
{
    if (!mxChartShapeContext.is())
    {
        switch (nElement & 0xffff)
        {
            case XML_chart:
            {
                std::unique_ptr<ContextHandler2Helper> pFragmentHandler(
                        new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
                mpShape.reset(new Shape("com.sun.star.drawing.OLE2Shape" ));
                mxChartShapeContext.set(new ChartGraphicDataContext(*pFragmentHandler, mpShape, true));
                break;
            }
            default:
                break;
        }
    }

    return mxChartShapeContext;
}

uno::Reference<xml::sax::XFastContextHandler> const & ShapeContextHandler::getWpsContext(sal_Int32 nStartElement, sal_Int32 nElement)
{
    if (!mxWpsContext.is())
    {
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
        ShapePtr pMasterShape;

        uno::Reference<drawing::XShape> xShape;
        // No element happens in case of pretty-printed XML, bodyPr is the case when we are called again after <wps:txbx>.
        if (!nElement || nElement == WPS_TOKEN(bodyPr))
            // Assume that this is just a continuation of the previous shape.
            xShape = mxSavedShape;

        switch (getBaseToken(nStartElement))
        {
            case XML_wsp:
                mxWpsContext.set(new WpsContext(
                                     *rFragmentHandler,
                                     xShape,
                                     pMasterShape,
                                     ShapePtr(
                                         new oox::drawingml::Shape(
                                             "com.sun.star.drawing.CustomShape"))));
                break;
            default:
                break;
        }
    }

    return mxWpsContext;
}

uno::Reference<xml::sax::XFastContextHandler> const & ShapeContextHandler::getWpgContext(sal_Int32 nElement)
{
    if (!mxWpgContext.is())
    {
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
        ShapePtr pMasterShape;

        switch (getBaseToken(nElement))
        {
            case XML_wgp:
                mxWpgContext.set(new WpgContext(*rFragmentHandler));
                break;
            default:
                break;
        }
    }

    return mxWpgContext;
}

uno::Reference<xml::sax::XFastContextHandler> const &
ShapeContextHandler::getGraphicShapeContext(::sal_Int32 Element )
{
    if (! mxGraphicShapeContext.is())
    {
        std::shared_ptr<ContextHandler2Helper> pFragmentHandler
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

uno::Reference<xml::sax::XFastContextHandler> const &
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
    else
    {
        // Reset the handler if fragment path has changed
        OUString sHandlerFragmentPath = dynamic_cast<ContextHandler&>(*mxDrawingFragmentHandler).getFragmentPath();
        if ( msRelationFragmentPath != sHandlerFragmentPath )
        {
            mxDrawingFragmentHandler.clear();
            mxDrawingFragmentHandler.set
              (dynamic_cast<ContextHandler *>
               (new oox::vml::DrawingFragment
                ( *mxFilterBase, msRelationFragmentPath, *mpDrawing )));
        }
    }
    return mxDrawingFragmentHandler;
}

uno::Reference<xml::sax::XFastContextHandler> const &
ShapeContextHandler::getDiagramShapeContext()
{
    if (!mxDiagramShapeContext.is())
    {
        std::shared_ptr<ContextHandler2Helper> pFragmentHandler(new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
        mpShape.reset(new Shape());
        mxDiagramShapeContext.set(new DiagramGraphicDataContext(*pFragmentHandler, mpShape));
    }

    return mxDiagramShapeContext;
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getContextHandler(sal_Int32 nElement)
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
        case NMSP_dmlChart:
            xResult.set(getChartShapeContext(mnStartToken));
            break;
        case NMSP_wps:
            xResult.set(getWpsContext(mnStartToken, nElement));
            break;
        case NMSP_wpg:
            xResult.set(getWpgContext(mnStartToken));
            break;
        default:
            xResult.set(getGraphicShapeContext(mnStartToken));
            break;
    }

    return xResult;
}

// css::xml::sax::XFastContextHandler:
void SAL_CALL ShapeContextHandler::startFastElement
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    mxFilterBase->filter(maMediaDescriptor);

    mpThemePtr.reset(new Theme());

    if (Element == DGM_TOKEN(relIds) || Element == LC_TOKEN(lockedCanvas) || Element == C_TOKEN(chart) ||
        Element == WPS_TOKEN(wsp) || Element == WPG_TOKEN(wgp) || Element == OOX_TOKEN(dmlPicture, pic))
    {
        // Parse the theme relation, if available; the diagram won't have colors without it.
        if (!msRelationFragmentPath.isEmpty())
        {
            // Get Target for Type = "officeDocument" from _rels/.rels file
            // aOfficeDocumentFragmentPath is pointing to "word/document.xml" for docx & to "ppt/presentation.xml" for pptx
            FragmentHandlerRef rFragmentHandlerRef(new ShapeFragmentHandler(*mxFilterBase, "/"));
            OUString aOfficeDocumentFragmentPath = rFragmentHandlerRef->getFragmentPathFromFirstTypeFromOfficeDoc( "officeDocument" );

            // Get the theme DO NOT  use msRelationFragmentPath for getting theme as for a document there is a single theme in document.xml.rels
            // and the same is used by header and footer as well.
            FragmentHandlerRef rFragmentHandler(new ShapeFragmentHandler(*mxFilterBase, aOfficeDocumentFragmentPath));
            OUString aThemeFragmentPath = rFragmentHandler->getFragmentPathFromFirstTypeFromOfficeDoc( "theme" );

            if(!aThemeFragmentPath.isEmpty())
            {
                uno::Reference<xml::sax::XFastSAXSerializable> xDoc(mxFilterBase->importFragment(aThemeFragmentPath), uno::UNO_QUERY_THROW);
                mxFilterBase->importFragment(new ThemeFragmentHandler(*mxFilterBase, aThemeFragmentPath, *mpThemePtr ), xDoc);
                ShapeFilterBase* pShapeFilterBase(dynamic_cast<ShapeFilterBase*>(mxFilterBase.get()));
                if (pShapeFilterBase)
                    pShapeFilterBase->setCurrentTheme(mpThemePtr);
            }
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
{
    if ( getContextHandler() == getDrawingShapeContext() )
        mpDrawing->getShapes().pushMark();

    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL ShapeContextHandler::endFastElement(::sal_Int32 Element)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->endFastElement(Element);
    // In case a textbox is sent, and later we get additional properties for
    // the textbox, then the wps context is not cleared, so do that here.
    if (Element == (NMSP_wps | XML_wsp))
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(mxSavedShape, uno::UNO_QUERY);
        bool bTextFrame = xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.text.TextFrame");
        bool bTextBox = false;
        if (!bTextFrame)
        {
            uno::Reference<beans::XPropertySet> xPropertySet(mxSavedShape, uno::UNO_QUERY);
            if (xPropertySet.is())
                xPropertySet->getPropertyValue("TextBox") >>= bTextBox;
        }
        if (bTextFrame || bTextBox)
            mxWpsContext.clear();
        mxSavedShape.clear();
    }
}

void SAL_CALL ShapeContextHandler::endUnknownElement
(const OUString & Namespace,
 const OUString & Name)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler(getContextHandler(Element));

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
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        return xContextHandler->createUnknownChildContext
            (Namespace, Name, Attribs);

    return uno::Reference< xml::sax::XFastContextHandler >();
}

void SAL_CALL ShapeContextHandler::characters(const OUString & aChars)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->characters(aChars);
}

// css::xml::sax::XFastShapeContextHandler:
uno::Reference< drawing::XShape > SAL_CALL
ShapeContextHandler::getShape()
{
    uno::Reference< drawing::XShape > xResult;
    uno::Reference< drawing::XShapes > xShapes( mxDrawPage, uno::UNO_QUERY );

    if (mxFilterBase.is() && xShapes.is())
    {
        if ( getContextHandler() == getDrawingShapeContext() )
        {
            mpDrawing->finalizeFragmentImport();
            if( std::shared_ptr< vml::ShapeBase > pShape = mpDrawing->getShapes().takeLastShape() )
                xResult = pShape->convertAndInsert( xShapes );
            // Only now remove the recursion mark, because getShape() is called in writerfilter
            // after endFastElement().
            mpDrawing->getShapes().popMark();
        }
        else if (mxDiagramShapeContext.is())
        {
            basegfx::B2DHomMatrix aMatrix;
            if (mpShape->getExtDrawings().empty())
            {
                mpShape->addShape( *mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, mpShape->getFillProperties() );
                xResult = mpShape->getXShape();
            }
            else
            {
                // Prerendered diagram output is available, then use that, and throw away the original result.
                for (auto const& extDrawing : mpShape->getExtDrawings())
                {
                    DiagramGraphicDataContext* pDiagramGraphicDataContext = dynamic_cast<DiagramGraphicDataContext*>(mxDiagramShapeContext.get());
                    if (!pDiagramGraphicDataContext)
                        break;
                    OUString aFragmentPath(pDiagramGraphicDataContext->getFragmentPathFromRelId(extDrawing));
                    oox::drawingml::ShapePtr pShapePtr( new Shape( "com.sun.star.drawing.GroupShape" ) );
                    pShapePtr->setDiagramType();
                    mxFilterBase->importFragment(new ShapeDrawingFragmentHandler(*mxFilterBase, aFragmentPath, pShapePtr));
                    pShapePtr->setDiagramDoms(mpShape->getDiagramDoms());
                    pShapePtr->keepDiagramDrawing(*mxFilterBase, aFragmentPath);

                    pShapePtr->addShape( *mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShapePtr->getFillProperties() );
                    xResult = pShapePtr->getXShape();
                }
                mpShape.reset();
            }
            mxDiagramShapeContext.clear();
        }
        else if (mxLockedCanvasContext.is())
        {
            ShapePtr pShape = dynamic_cast<LockedCanvasContext&>(*mxLockedCanvasContext).getShape();
            if (pShape)
            {
                basegfx::B2DHomMatrix aMatrix;
                pShape->addShape(*mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
                xResult = pShape->getXShape();
                mxLockedCanvasContext.clear();
            }
        }
        //NMSP_dmlChart == getNamespace( mnStartToken ) check is introduced to make sure that
        //mnStartToken is set as NMSP_dmlChart in setStartToken.
        //Only in case it is set then only the below block of code for ChartShapeContext should be executed.
        else if (mxChartShapeContext.is() && (NMSP_dmlChart == getNamespace( mnStartToken )))
        {
            ChartGraphicDataContext* pChartGraphicDataContext = dynamic_cast<ChartGraphicDataContext*>(mxChartShapeContext.get());
            if (pChartGraphicDataContext)
            {
                basegfx::B2DHomMatrix aMatrix;
                oox::drawingml::ShapePtr xShapePtr( pChartGraphicDataContext->getShape());
                // See SwXTextDocument::createInstance(), ODF import uses the same hack.
                xShapePtr->setServiceName("com.sun.star.drawing.temporaryForXMLImportOLE2Shape");
                xShapePtr->addShape( *mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, xShapePtr->getFillProperties() );
                xResult = xShapePtr->getXShape();
            }
            mxChartShapeContext.clear();
        }
        else if (mxWpsContext.is())
        {
            ShapePtr pShape = dynamic_cast<WpsContext&>(*mxWpsContext).getShape();
            if (pShape)
            {
                basegfx::B2DHomMatrix aMatrix;
                pShape->setPosition(maPosition);
                pShape->addShape(*mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
                xResult = pShape->getXShape();
                mxSavedShape = xResult;
                mxWpsContext.clear();
            }
        }
        else if (mxWpgContext.is())
        {
            ShapePtr pShape = dynamic_cast<WpgContext&>(*mxWpgContext).getShape();
            if (pShape)
            {
                basegfx::B2DHomMatrix aMatrix;
                pShape->setPosition(maPosition);
                pShape->addShape(*mxFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
                xResult = pShape->getXShape();
                mxSavedShape = xResult;
            }
        }
        else if (mpShape.get() != nullptr)
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
ShapeContextHandler::getDrawPage()
{
    return mxDrawPage;
}

void SAL_CALL ShapeContextHandler::setDrawPage
(const css::uno::Reference< css::drawing::XDrawPage > & the_value)
{
    mxDrawPage = the_value;
}

css::uno::Reference< css::frame::XModel > SAL_CALL
ShapeContextHandler::getModel()
{
    if( !mxFilterBase.is() )
        throw uno::RuntimeException();
    return mxFilterBase->getModel();
}

void SAL_CALL ShapeContextHandler::setModel
(const css::uno::Reference< css::frame::XModel > & the_value)
{
    if( !mxFilterBase.is() )
        throw uno::RuntimeException();
    uno::Reference<lang::XComponent> xComp(the_value, uno::UNO_QUERY_THROW);
    mxFilterBase->setTargetDocument(xComp);
}

OUString SAL_CALL ShapeContextHandler::getRelationFragmentPath()
{
    return msRelationFragmentPath;
}

void SAL_CALL ShapeContextHandler::setRelationFragmentPath(const OUString & the_value)
{
    msRelationFragmentPath = the_value;
}

::sal_Int32 SAL_CALL ShapeContextHandler::getStartToken()
{
    return mnStartToken;
}

void SAL_CALL ShapeContextHandler::setStartToken( ::sal_Int32 _starttoken )
{
    mnStartToken = _starttoken;
}

awt::Point SAL_CALL ShapeContextHandler::getPosition()
{
    return maPosition;
}

void SAL_CALL ShapeContextHandler::setPosition(const awt::Point& rPosition)
{
    maPosition = rPosition;
}

void SAL_CALL ShapeContextHandler::setDocumentProperties(const uno::Reference<document::XDocumentProperties>& xDocProps)
{
    mxDocumentProperties = xDocProps;
    mxFilterBase->checkDocumentProperties(mxDocumentProperties);
}

uno::Reference<document::XDocumentProperties> SAL_CALL ShapeContextHandler::getDocumentProperties()
{
    return mxDocumentProperties;
}

uno::Sequence<beans::PropertyValue> SAL_CALL ShapeContextHandler::getMediaDescriptor()
{
    return maMediaDescriptor;
}

void SAL_CALL ShapeContextHandler::setMediaDescriptor(const uno::Sequence<beans::PropertyValue>& rMediaDescriptor)
{
    maMediaDescriptor = rMediaDescriptor;
}

OUString ShapeContextHandler::getImplementationName()
{
    return OUString( "com.sun.star.comp.oox.ShapeContextHandler" );
}

uno::Sequence< OUString > ShapeContextHandler::getSupportedServiceNames()
{
    uno::Sequence< OUString > s { "com.sun.star.xml.sax.FastShapeContextHandler" };
    return s;
}

sal_Bool SAL_CALL ShapeContextHandler::supportsService(const OUString & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

}}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_oox_ShapeContextHandler_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new oox::shape::ShapeContextHandler(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
