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

#include "oox/ppt/dgmlayout.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "drawingml/diagram/diagram.hxx"
#include "oox/dump/pptxdumper.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <services.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;
using namespace ::oox::drawingml;

namespace oox { namespace ppt {

OUString SAL_CALL QuickDiagrammingLayout_getImplementationName()
{
    return OUString( "com.sun.star.comp.Impress.oox.QuickDiagrammingLayout" );
}

uno::Sequence< OUString > SAL_CALL QuickDiagrammingLayout_getSupportedServiceNames()
{
    const OUString aServiceName = "com.sun.star.comp.ooxpptx.dgm.layout";
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL QuickDiagrammingLayout_createInstance( const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast<cppu::OWeakObject*>(new QuickDiagrammingLayout( rxContext ));
}

QuickDiagrammingLayout::QuickDiagrammingLayout( const Reference< XComponentContext >& rxContext )
    : XmlFilterBase( rxContext ),
    mpThemePtr(new drawingml::Theme())
{}

bool QuickDiagrammingLayout::importDocument() throw (css::uno::RuntimeException)
{
    Reference<drawing::XShape>  xParentShape(getParentShape(),
                                             UNO_QUERY_THROW);
    Reference<drawing::XShapes> xParentShapes(xParentShape,
                                              UNO_QUERY_THROW);
    Reference<beans::XPropertySet> xPropSet(xParentShape,
                                            UNO_QUERY_THROW);

    // can we grab the theme from the master page?
    Reference<container::XChild> xChild(xParentShape,
                                        UNO_QUERY);
    if( xChild.is() )
    {
        // TODO: cater for diagram shapes inside groups
        Reference<drawing::XMasterPageTarget> xMasterPageTarget(xChild->getParent(),
                                                                UNO_QUERY);
        if( xMasterPageTarget.is() )
        {
            uno::Reference<drawing::XDrawPage> xMasterPage(
                xMasterPageTarget->getMasterPage());

            Reference<beans::XPropertySet> xPropSet2(xMasterPage,
                                                     UNO_QUERY_THROW);
            Reference<xml::dom::XDocument> xThemeFragment;
            xPropSet2->getPropertyValue("PPTTheme") >>= xThemeFragment;

            importFragment(
                new ThemeFragmentHandler(
                    *this, OUString(), *mpThemePtr ),
                Reference<xml::sax::XFastSAXSerializable>(
                    xThemeFragment,
                    UNO_QUERY_THROW));
        }
    }

    Reference<xml::dom::XDocument> xDataModelDom;
    Reference<xml::dom::XDocument> xLayoutDom;
    Reference<xml::dom::XDocument> xQStyleDom;
    Reference<xml::dom::XDocument> xColorStyleDom;

    xPropSet->getPropertyValue("DiagramData") >>= xDataModelDom;
    xPropSet->getPropertyValue("DiagramLayout") >>= xLayoutDom;
    xPropSet->getPropertyValue("DiagramQStyle") >>= xQStyleDom;
    xPropSet->getPropertyValue("DiagramColorStyle") >>= xColorStyleDom;

    oox::drawingml::ShapePtr pShape(
        new oox::drawingml::Shape( "com.sun.star.drawing.DiagramShape" ) );
    drawingml::loadDiagram(pShape,
                           *this,
                           xDataModelDom,
                           xLayoutDom,
                           xQStyleDom,
                           xColorStyleDom);

    // don't add pShape itself, but only its children
    pShape->setXShape(getParentShape());

    const awt::Size& rSize=xParentShape->getSize();
    const awt::Point& rPoint=xParentShape->getPosition();
    const long nScaleFactor=360;
    const awt::Rectangle aRect(nScaleFactor*rPoint.X,
                               nScaleFactor*rPoint.Y,
                               nScaleFactor*rSize.Width,
                               nScaleFactor*rSize.Height);
    basegfx::B2DHomMatrix aMatrix;
    pShape->addChildren( *this,
                         mpThemePtr.get(),
                         xParentShapes,
                         aMatrix,
                         &aRect );

    return true;
}

bool QuickDiagrammingLayout::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* QuickDiagrammingLayout::getCurrentTheme() const
{
    return mpThemePtr.get();
}

const oox::drawingml::table::TableStyleListPtr QuickDiagrammingLayout::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

::oox::vml::Drawing* QuickDiagrammingLayout::getVmlDrawing()
{
    return nullptr;
}

::oox::drawingml::chart::ChartConverter* QuickDiagrammingLayout::getChartConverter()
{
    return nullptr;
}

OUString QuickDiagrammingLayout::getImplementationName() throw (css::uno::RuntimeException, std::exception)
{
    return QuickDiagrammingLayout_getImplementationName();
}

::oox::ole::VbaProject* QuickDiagrammingLayout::implCreateVbaProject() const
{
    return nullptr;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
