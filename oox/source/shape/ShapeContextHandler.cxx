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
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>

#include <oox/shape/ShapeContextHandler.hxx>
#include <oox/shape/ShapeDrawingFragmentHandler.hxx>
#include "LockedCanvasContext.hxx"
#include "WordprocessingCanvasContext.hxx"
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
#include <svx/svdogrp.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdobj.hxx>

#include <drawingml/connectorhelper.hxx>

#include <memory>
#include <utility>

using namespace ::com::sun::star;

namespace oox::shape {
using namespace core;
using namespace drawingml;

ShapeContextHandler::ShapeContextHandler(rtl::Reference<ShapeFilterBase> xFilterBase) :
  m_bFullWPGSUpport(false),
  mxShapeFilterBase(std::move(xFilterBase))

{
}

ShapeContextHandler::~ShapeContextHandler()
{
}

uno::Reference<xml::sax::XFastContextHandler> ShapeContextHandler::getLockedCanvasContext(sal_Int32 nElement)
{
    if (!mxLockedCanvasContext.is())
    {
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, msRelationFragmentPath));

        switch (nElement & 0xffff)
        {
            case XML_lockedCanvas:
                mxLockedCanvasContext.set(new LockedCanvasContext(*rFragmentHandler));
                break;
            default:
                break;
        }
    }

    return static_cast<ContextHandler *>(mxLockedCanvasContext.get());
}

/*
 * This method creates new ChartGraphicDataContext Object.
 */
uno::Reference<xml::sax::XFastContextHandler> ShapeContextHandler::getChartShapeContext(sal_Int32 nElement)
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

uno::Reference<xml::sax::XFastContextHandler> ShapeContextHandler::getWpsContext(sal_Int32 nStartElement, sal_Int32 nElement)
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

uno::Reference<xml::sax::XFastContextHandler> ShapeContextHandler::getWpgContext(sal_Int32 nElement)
{
    if (!mxWpgContext.is())
    {
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, msRelationFragmentPath));

        switch (getBaseToken(nElement))
        {
            case XML_wgp:
            {
                mxWpgContext.set(new WpgContext(*rFragmentHandler, oox::drawingml::ShapePtr()));
                mxWpgContext->setFullWPGSupport(m_bFullWPGSUpport);
                break;
            }
            default:
                break;
        }
    }

    return static_cast<ContextHandler *>(mxWpgContext.get());
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

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getDrawingShapeContext()
{
    if (!mxDrawingFragmentHandler.is())
    {
        mpDrawing = std::make_shared<oox::vml::Drawing>( *mxShapeFilterBase, mxDrawPage, oox::vml::VMLDRAWING_WORD );
        mxDrawingFragmentHandler.set
           (new oox::vml::DrawingFragment
            ( *mxShapeFilterBase, msRelationFragmentPath, *mpDrawing ));
    }
    else
    {
        // Reset the handler if fragment path has changed
        OUString sHandlerFragmentPath = mxDrawingFragmentHandler->getFragmentPath();
        if ( msRelationFragmentPath != sHandlerFragmentPath )
        {
            mxDrawingFragmentHandler.clear();
            mxDrawingFragmentHandler.set
               (new oox::vml::DrawingFragment
                ( *mxShapeFilterBase, msRelationFragmentPath, *mpDrawing ));
        }
    }
    return static_cast<ContextHandler *>(mxDrawingFragmentHandler.get());
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getDiagramShapeContext()
{
    if (!mxDiagramShapeContext.is())
    {
        auto pFragmentHandler = std::make_shared<ShapeFragmentHandler>(*mxShapeFilterBase, msRelationFragmentPath);
        mpShape = std::make_shared<Shape>();
        mpShape->setSize(maSize);
        mxDiagramShapeContext.set(new DiagramGraphicDataContext(*pFragmentHandler, mpShape));
    }

    return mxDiagramShapeContext;
}

uno::Reference<xml::sax::XFastContextHandler> ShapeContextHandler::getWordprocessingCanvasContext(sal_Int32 nElement)
{
    if (!mxWordprocessingCanvasContext.is())
    {
        FragmentHandler2Ref rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, msRelationFragmentPath));

        switch (getBaseToken(nElement))
        {
            case XML_wpc:
                mxWordprocessingCanvasContext.set(new WordprocessingCanvasContext(*rFragmentHandler, maSize));
                break;
            default:
                break;
        }
    }

    return static_cast<ContextHandler *>(mxWordprocessingCanvasContext.get());
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getContextHandler(sal_Int32 nElement)
{
    uno::Reference<xml::sax::XFastContextHandler> xResult;
    const sal_uInt32 nStartToken = getStartToken();

    switch (getNamespace( nStartToken ))
    {
        case NMSP_doc:
        case NMSP_vml:
            xResult.set(getDrawingShapeContext());
            break;
        case NMSP_dmlDiagram:
            xResult.set(getDiagramShapeContext());
            break;
        case NMSP_dmlLockedCanvas:
            xResult.set(getLockedCanvasContext(nStartToken));
            break;
        case NMSP_dmlChart:
            xResult.set(getChartShapeContext(nStartToken));
            break;
        case NMSP_wps:
            xResult.set(getWpsContext(nStartToken, nElement));
            break;
        case NMSP_wpg:
            xResult.set(getWpgContext(nStartToken));
            break;
        case NMSP_wpc:
            xResult.set(getWordprocessingCanvasContext(nStartToken));
            break;
        default:
            xResult.set(getGraphicShapeContext(nStartToken));
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
        Element == WPS_TOKEN(wsp) || Element == WPG_TOKEN(wgp) || Element == OOX_TOKEN(dmlPicture, pic)
        || Element == WPC_TOKEN(wpc))
    {
        // Parse the theme relation, if available; the diagram won't have colors without it.
        if (!mpThemePtr && !msRelationFragmentPath.isEmpty())
        {
            // Get Target for Type = "officeDocument" from _rels/.rels file
            // aOfficeDocumentFragmentPath is pointing to "word/document.xml" for docx & to "ppt/presentation.xml" for pptx
            FragmentHandlerRef rFragmentHandlerRef(new ShapeFragmentHandler(*mxShapeFilterBase, u"/"_ustr));
            OUString aOfficeDocumentFragmentPath = rFragmentHandlerRef->getFragmentPathFromFirstTypeFromOfficeDoc( u"officeDocument" );

            // Get the theme DO NOT  use msRelationFragmentPath for getting theme as for a document there is a single theme in document.xml.rels
            // and the same is used by header and footer as well.
            FragmentHandlerRef rFragmentHandler(new ShapeFragmentHandler(*mxShapeFilterBase, aOfficeDocumentFragmentPath));
            OUString aThemeFragmentPath = rFragmentHandler->getFragmentPathFromFirstTypeFromOfficeDoc( u"theme" );

            if (!aThemeFragmentPath.isEmpty())
            {
                mpThemePtr = std::make_shared<Theme>();
                auto pTheme = std::make_shared<model::Theme>();
                mpThemePtr->setTheme(pTheme);
                uno::Reference<xml::sax::XFastSAXSerializable> xDoc(mxShapeFilterBase->importFragment(aThemeFragmentPath), uno::UNO_QUERY_THROW);
                mxShapeFilterBase->importFragment(new ThemeFragmentHandler(*mxShapeFilterBase, aThemeFragmentPath, *mpThemePtr, *pTheme), xDoc);
                mxShapeFilterBase->setCurrentTheme(mpThemePtr);
            }
        }
        else if (mpThemePtr && !mxShapeFilterBase->getCurrentTheme())
        {
            mxShapeFilterBase->setCurrentTheme(mpThemePtr);
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
    bool bTextFrame = xServiceInfo.is() && xServiceInfo->supportsService(u"com.sun.star.text.TextFrame"_ustr);
    bool bTextBox = false;
    if (!bTextFrame)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(mxSavedShape, uno::UNO_QUERY);
        if (xPropertySet.is())
            xPropertySet->getPropertyValue(u"TextBox"_ustr) >>= bTextBox;
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

namespace // helpers for case mxWordprocessingCanvasContext
{
    void lcl_createShapeMap(oox::drawingml::ShapePtr rShapePtr,
                            oox::drawingml::ShapeIdMap& rShapeMap)
{
    std::vector< ShapePtr >& rChildren = rShapePtr->getChildren();
    if (rChildren.empty())
        return;
    for (auto& pIt : rChildren)
    {
        rShapeMap[pIt->getId()] = pIt; // add child itself
        lcl_createShapeMap(pIt, rShapeMap); // and all its descendants
    }
}

} // end anonymous namespace

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
                    OUString aFragmentPath(mxDiagramShapeContext->getFragmentPathFromRelId(extDrawing));
                    oox::drawingml::ShapePtr pShapePtr = std::make_shared<Shape>( "com.sun.star.drawing.GroupShape" );
                    pShapePtr->setDiagramType();
                    mxShapeFilterBase->importFragment(new ShapeDrawingFragmentHandler(*mxShapeFilterBase, aFragmentPath, pShapePtr));
                    pShapePtr->setDiagramDoms(mpShape->getDiagramDoms());
                    pShapePtr->keepDiagramDrawing(*mxShapeFilterBase, aFragmentPath);

                    if (mpShape->getFontRefColorForNodes().isUsed())
                        applyFontRefColor(pShapePtr, mpShape->getFontRefColorForNodes());

                    // migrate IDiagramHelper to new oox::Shape (from mpShape which was loaded
                    // to pShapePtr where the geometry is now constructed)
                    mpShape->migrateDiagramHelperToNewShape(pShapePtr);

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
            ShapePtr pShape = mxLockedCanvasContext->getShape();
            if (pShape)
            {
                basegfx::B2DHomMatrix aMatrix;
                pShape->addShape(*mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());
                xResult = pShape->getXShape();
                mxLockedCanvasContext.clear();
            }
        }
        else if (mxWordprocessingCanvasContext.is())
        {
             // group which represents the drawing canvas
            ShapePtr pShape = mxWordprocessingCanvasContext->getShape();
            if (pShape)
            {
                basegfx::B2DHomMatrix aMatrix;
                pShape->addShape(*mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, pShape->getFillProperties());

                // create a flat map of all shapes in the drawing canvas group.
                oox::drawingml::ShapeIdMap aShapeMap;
                lcl_createShapeMap(pShape, aShapeMap);

                // Traverse aShapeMap and generate edge related properties.
                for (auto& rIt : aShapeMap)
                {
                    if ((rIt.second)->getServiceName() == "com.sun.star.drawing.ConnectorShape")
                    {
                        ConnectorHelper::applyConnections(rIt.second, aShapeMap);

                        if (rIt.second->getConnectorName() == u"bentConnector3"_ustr
                           || rIt.second->getConnectorName() == u"bentConnector4"_ustr
                           || rIt.second->getConnectorName() == u"bentConnector5"_ustr)
                        {
                           ConnectorHelper::applyBentHandleAdjustments(rIt.second);
                        }
                        else if (rIt.second->getConnectorName() == u"curvedConnector3"_ustr
                           || rIt.second->getConnectorName() == u"curvedConnector4"_ustr
                           || rIt.second->getConnectorName() == u"curvedConnector5"_ustr)
                        {
                            ConnectorHelper::applyCurvedHandleAdjustments(rIt.second);
                        }
                        // else use the default path of LibreOffice.
                        // curveConnector2 and bentConnector2 do not have handles.
                    }
                }
                xResult = pShape->getXShape();
                mxWordprocessingCanvasContext.clear();
            }
        }
        //NMSP_dmlChart == getNamespace( mnStartToken ) check is introduced to make sure that
        //mnStartToken is set as NMSP_dmlChart in setStartToken.
        //Only in case it is set then only the below block of code for ChartShapeContext should be executed.
        else if (mxChartShapeContext.is() && (NMSP_dmlChart == getNamespace( getStartToken() )))
        {
            basegfx::B2DHomMatrix aMatrix;
            oox::drawingml::ShapePtr xShapePtr( mxChartShapeContext->getShape());
            // See SwXTextDocument::createInstance(), ODF import uses the same hack.
            xShapePtr->setServiceName(u"com.sun.star.drawing.temporaryForXMLImportOLE2Shape"_ustr);
            xShapePtr->addShape( *mxShapeFilterBase, mpThemePtr.get(), xShapes, aMatrix, xShapePtr->getFillProperties() );
            xResult = xShapePtr->getXShape();
            mxChartShapeContext.clear();
        }
        else if (mxWpsContext.is())
        {
            ShapePtr pShape = mxWpsContext->getShape();
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
            ShapePtr pShape = mxWpgContext->getShape();
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

    if (xResult)
        popStartToken();
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
    assert(mnStartTokenStack.size() && "This stack must not be empty!");
    return mnStartTokenStack.top();
}

void ShapeContextHandler::popStartToken()
{
    if (mnStartTokenStack.size() > 1)
        mnStartTokenStack.pop();
}

void ShapeContextHandler::pushStartToken( sal_Int32 _starttoken )
{
    mnStartTokenStack.push(_starttoken);
}

void ShapeContextHandler::setPosition(const awt::Point& rPosition)
{
    maPosition = rPosition;
}

void ShapeContextHandler::setSize(const awt::Size& rSize)
{
    maSize = rSize;
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

void ShapeContextHandler::applyFontRefColor(const oox::drawingml::ShapePtr& pShape,
                                            const oox::drawingml::Color& rFontRefColor)
{
    pShape->getShapeStyleRefs()[XML_fontRef].maPhClr = rFontRefColor;
    std::vector<oox::drawingml::ShapePtr>& vChildren = pShape->getChildren();
    for (auto const& child : vChildren)
    {
        applyFontRefColor(child, rFontRefColor);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
