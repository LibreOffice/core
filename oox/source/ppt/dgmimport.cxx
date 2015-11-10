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

#include "oox/ppt/dgmimport.hxx"
#include "oox/drawingml/theme.hxx"
#include "drawingml/diagram/diagram.hxx"
#include "oox/dump/pptxdumper.hxx"

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <services.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;

namespace oox { namespace ppt {

OUString SAL_CALL QuickDiagrammingImport_getImplementationName()
{
    return OUString( "com.sun.star.comp.Impress.oox.QuickDiagrammingImport" );
}

uno::Sequence< OUString > SAL_CALL QuickDiagrammingImport_getSupportedServiceNames()
{
    const OUString aServiceName = "com.sun.star.comp.ooxpptx.dgm.import";
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL QuickDiagrammingImport_createInstance( const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast<cppu::OWeakObject*>(new QuickDiagrammingImport( rxContext ));
}

QuickDiagrammingImport::QuickDiagrammingImport( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : XmlFilterBase( rxContext )
{}

bool QuickDiagrammingImport::importDocument() throw (css::uno::RuntimeException, std::exception)
{
    /*  to activate the PPTX dumper, define the environment variable
        OOO_PPTXDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/pptxdumper.ini. */
    OOX_DUMP_FILE( ::oox::dump::pptx::Dumper );

    OUString aFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "diagramLayout" );

    Reference<drawing::XShapes> xParentShape(getParentShape(),
                                             UNO_QUERY_THROW);
    oox::drawingml::ShapePtr pShape(
        new oox::drawingml::Shape( "com.sun.star.drawing.DiagramShape" ) );
    drawingml::loadDiagram(pShape,
                           *this,
                           "",
                           aFragmentPath,
                           "",
                           "");
    oox::drawingml::ThemePtr pTheme(
        new oox::drawingml::Theme());
    basegfx::B2DHomMatrix aMatrix;
    pShape->addShape( *this,
                      pTheme.get(),
                      xParentShape,
                      aMatrix, pShape->getFillProperties() );

    return true;
}

bool QuickDiagrammingImport::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* QuickDiagrammingImport::getCurrentTheme() const
{
    // TODO
    return nullptr;
}

const oox::drawingml::table::TableStyleListPtr QuickDiagrammingImport::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

oox::vml::Drawing* QuickDiagrammingImport::getVmlDrawing()
{
    return nullptr;
}

oox::drawingml::chart::ChartConverter* QuickDiagrammingImport::getChartConverter()
{
    return nullptr;
}

OUString QuickDiagrammingImport::getImplementationName() throw (css::uno::RuntimeException, std::exception)
{
    return QuickDiagrammingImport_getImplementationName();
}

::oox::ole::VbaProject* QuickDiagrammingImport::implCreateVbaProject() const
{
    return nullptr;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
