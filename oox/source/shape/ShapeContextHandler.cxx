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

#include <oox/shape/ShapeContextHandler.hxx>
#include "ShapeDrawingFragmentHandler.hxx"
#include "LockedCanvasContext.hxx"
#include "WpsContext.hxx"
#include "WpgContext.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <oox/vml/vmldrawingfragment.hxx>
#include <oox/vml/vmlshape.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include <oox/shape/ShapeFilterBase.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <memory>
#include <sal/log.hxx>

using namespace ::com::sun::star;

namespace oox::shape {
using namespace core;
using namespace drawingml;

ShapeContextHandler::ShapeContextHandler(const rtl::Reference<ShapeFilterBase>& xFilterBase) :
  mnStartToken(0),
  mbShouldWPGbeUsed(false),
  mxShapeFilterBase(xFilterBase)

{
}

ShapeContextHandler::~ShapeContextHandler()
{
}

uno::Reference<xml::sax::XFastContextHandler> const & ShapeContextHandler::getLockedCanvasContext(sal_Int32 nElement)
{
    if (!mxLockedCanvasContext.is())
    {
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, msRelationFragmentPath));

        switch (nElement & 0xffff)
        {
            case XML_lockedCanvas:
                mxLockedCanvasContext.set(static_cast<oox::core::ContextHandler*>(new LockedCanvasContext(*rFragmentHandler)));
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
                        new ShapeFragmentHandler(*mxShapeFilterBase, msRelationFragmentPath));
                mpShape = std::make_shared<Shape>("com.sun.star.drawing.OLE2Shape" );
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
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, msRelationFragmentPath));
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
                                     std::make_shared<oox::drawingml::Shape>(
                                             "com.sun.star.drawing.CustomShape")));
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
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, msRelationFragmentPath));

        switch (getBaseToken(nElement))
        {
            case XML_wgp:
                mxWpgContext.set(static_cast<oox::core::ContextHandler*>(new WpgContext(
                    *rFragmentHandler, oox::drawingml::ShapePtr(), mbShouldWPGbeUsed)));
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
        auto pFragmentHandler = std::make_shared<ShapeFragmentHandler>(*mxShapeFilterBase, msRelationFragmentPath);
        ShapePtr pMasterShape;

        switch (Element & 0xffff)
        {
            case XML_graphic:
                mpShape = std::make_shared<Shape>("com.sun.star.drawing.GraphicObjectShape" );
                mxGraphicShapeContext.set
                (new GraphicalObjectFrameContext(*pFragmentHandler, pMasterShape, mpShape, true));
                break;
            case XML_pic:
                mpShape = std::make_shared<Shape>("com.sun.star.drawing.GraphicObjectShape" );
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
        mpDrawing = std::make_shared<oox::vml::Drawing>( *mxShapeFilterBase, mxDrawPage, oox::vml::VMLDRAWING_WORD );
        mxDrawingFragmentHandler.set
          (static_cast<ContextHandler *>
           (new oox::vml::DrawingFragment
            ( *mxShapeFilterBase, msRelationFragmentPath, *mpDrawing )));
    }
    else
    {
        // Reset the handler if fragment path has changed
        OUString sHandlerFragmentPath = dynamic_cast<ContextHandler&>(*mxDrawingFragmentHandler).getFragmentPath();
        if ( msRelationFragmentPath != sHandlerFragmentPath )
        {
            mxDrawingFragmentHandler.clear();
            mxDrawingFragmentHandler.set
              (static_cast<ContextHandler *>
               (new oox::vml::DrawingFragment
                ( *mxShapeFilterBase, msRelationFragmentPath, *mpDrawing )));
        }
    }
    return mxDrawingFragmentHandler;
}

uno::Reference<xml::sax::XFastContextHandler> const &
ShapeContextHandler::getDiagramShapeContext()
{
    if (!mxDiagramShapeContext.is())
    {
        auto pFragmentHandler = std::make_shared<ShapeFragmentHandler>(*mxShapeFilterBase, msRelationFragmentPath);
        mpShape = std::make_shared<Shape>();
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
    mxShapeFilterBase->filter(maMediaDescriptor);

    if (Element == DGM_TOKEN(relIds) || Element == LC_TOKEN(lockedCanvas) || Element == C_TOKEN(chart) ||
        Element == WPS_TOKEN(wsp) || Element == WPG_TOKEN(wgp) || Element == OOX_TOKEN(dmlPicture, pic))
    {
        // Parse the theme relation, if available; the diagram won't have colors without it.
        if (!mpThemePtr && !msRelationFragmentPath.isEmpty())
        {
            mpThemePtr = std::make_shared<Theme>();
            // Get Target for Type = "officeDocument" from _rels/.rels file
            // aOfficeDocumentFragmentPath is pointing to "word/document.xml" for docx & to "ppt/presentation.xml" for pptx
            FragmentHandlerRef rFragmentHandlerRef(new ShapeFragmentHandler(*mxShapeFilterBase, "/"));
            OUString aOfficeDocumentFragmentPath = rFragmentHandlerRef->getFragmentPathFromFirstTypeFromOfficeDoc( u"officeDocument" );

            // Get the theme DO NOT  use msRelationFragmentPath for getting theme as for a document there is a single theme in document.xml.rels
            // and the same is used by header and footer as well.
            FragmentHandlerRef rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, aOfficeDocumentFragmentPath));
            OUString aThemeFragmentPath = rFragmentHandler->getFragmentPathFromFirstTypeFromOfficeDoc( u"theme" );

            if(!aThemeFragmentPath.isEmpty())
            {
                uno::Reference<xml::sax::XFastSAXSerializable> xDoc(mxShapeFilterBase->importFragment(aThemeFragmentPath), uno::UNO_QUERY_THROW);
                mxShapeFilterBase->importFragment(new ThemeFragmentHandler(*mxShapeFilterBase, aThemeFragmentPath, *mpThemePtr ), xDoc);
                mxShapeFilterBase->setCurrentTheme(mpThemePtr);
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
    if (Element != (NMSP_wps | XML_wsp))
        return;

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

uno::Reference< drawing::XShape >
ShapeContextHandler::getShape()
{
    uno::Reference< drawing::XShape > xResult;
    uno::Reference< drawing::XShapes > xShapes = mxDrawPage;

    if (mxShapeFilterBase && xShapes.is())
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
                mpShape->addShape( *mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, mpShape->getFillProperties() );
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
                    oox::drawingml::ShapePtr pShapePtr = std::make_shared<Shape>( "com.sun.star.drawing.GroupShape" );
                    pShapePtr->setDiagramType();
                    mxShapeFilterBase->importFragment(new ShapeDrawingFragmentHandler(*mxShapeFilterBase, aFragmentPath, pShapePtr));
                    pShapePtr->setDiagramDoms(mpShape->getDiagramDoms());
                    pShapePtr->keepDiagramDrawing(*mxShapeFilterBase, aFragmentPath);

                    if (!mpShape->getChildren().empty())
                    {
                        // first child is diagram background - we want to keep it, as drawingML fallback doesn't contain it
                        auto& aChildren = pShapePtr->getChildren();
                        ShapePtr pBackground = mpShape->getChildren().front();
                        aChildren.insert(aChildren.begin(), pBackground);
                    }

                    pShapePtr->addShape( *mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShapePtr->getFillProperties() );
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
                pShape->addShape(*mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
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
                xShapePtr->addShape( *mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, xShapePtr->getFillProperties() );
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
                pShape->addShape(*mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
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
                pShape->addShape(*mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
                xResult = pShape->getXShape();
                mxSavedShape = xResult;
                mxWpgContext.clear();
            }
        }
        else if (mpShape)
        {
            basegfx::B2DHomMatrix aTransformation;

            if (maPosition.X != 0 || maPosition.Y != 0)
            {
                // We got a position from writerfilter/, store that in the shape, otherwise the
                // position won't be set.
                mpShape->setWps(true);
                mpShape->setPosition(maPosition);
            }

            mpShape->addShape(*mxShapeFilterBase, mpThemePtr.get(), xShapes, aTransformation, mpShape->getFillProperties() );
            xResult.set(mpShape->getXShape());
            mxGraphicShapeContext.clear( );
        }
    }

    return xResult;
}

void ShapeContextHandler::setDrawPage(const css::uno::Reference< css::drawing::XDrawPage > & the_value)
{
    mxDrawPage = the_value;
}

void ShapeContextHandler::setModel(const css::uno::Reference< css::frame::XModel > & the_value)
{
    if( !mxShapeFilterBase.is() )
        throw uno::RuntimeException();
    uno::Reference<lang::XComponent> xComp(the_value, uno::UNO_QUERY_THROW);
    mxShapeFilterBase->setTargetDocument(xComp);
}

void ShapeContextHandler::setRelationFragmentPath(const OUString & the_value)
{
    msRelationFragmentPath = the_value;
}

sal_Int32 ShapeContextHandler::getStartToken() const
{
    return mnStartToken;
}

void ShapeContextHandler::setStartToken( sal_Int32 _starttoken )
{
    mnStartToken = _starttoken;
}

void ShapeContextHandler::setPosition(const awt::Point& rPosition)
{
    maPosition = rPosition;
}

void ShapeContextHandler::setDocumentProperties(const uno::Reference<document::XDocumentProperties>& xDocProps)
{
    mxDocumentProperties = xDocProps;
    mxShapeFilterBase->checkDocumentProperties(mxDocumentProperties);
}

void ShapeContextHandler::setMediaDescriptor(const uno::Sequence<beans::PropertyValue>& rMediaDescriptor)
{
    maMediaDescriptor = rMediaDescriptor;
}

void ShapeContextHandler::setGraphicMapper(css::uno::Reference<css::graphic::XGraphicMapper> const & rxGraphicMapper)
{
    mxShapeFilterBase->setGraphicMapper(rxGraphicMapper);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
