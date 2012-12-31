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
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/dump/pptxdumper.hxx"

#include <com/sun/star/drawing/XShape.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;

namespace oox { namespace ppt {

OUString SAL_CALL QuickDiagrammingImport_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.Impress.oox.QuickDiagrammingImport" );
}

uno::Sequence< OUString > SAL_CALL QuickDiagrammingImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.ooxpptx.dgm.import" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL QuickDiagrammingImport_createInstance( const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return (cppu::OWeakObject*)new QuickDiagrammingImport( rxContext );
}

QuickDiagrammingImport::QuickDiagrammingImport( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
    : XmlFilterBase( rxContext )
{}

bool QuickDiagrammingImport::importDocument() throw()
{
    /*  to activate the PPTX dumper, define the environment variable
        OOO_PPTXDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/pptxdumper.ini. */
    OOX_DUMP_FILE( ::oox::dump::pptx::Dumper );

    OUString aEmpty;
    OUString aFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "diagramLayout" ) );

    Reference<drawing::XShapes> xParentShape(getParentShape(),
                                             UNO_QUERY_THROW);
    oox::drawingml::ShapePtr pShape(
        new oox::drawingml::Shape( "com.sun.star.drawing.DiagramShape" ) );
    drawingml::loadDiagram(pShape,
                           *this,
                           aEmpty,
                           aFragmentPath,
                           aEmpty,
                           aEmpty);
    oox::drawingml::ThemePtr pTheme(
        new oox::drawingml::Theme());
    basegfx::B2DHomMatrix aMatrix;
    pShape->addShape( *this,
                      pTheme.get(),
                      xParentShape,
                      aMatrix );

    return true;
}

bool QuickDiagrammingImport::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* QuickDiagrammingImport::getCurrentTheme() const
{
    // TODO
    return 0;
}

sal_Int32 QuickDiagrammingImport::getSchemeClr( sal_Int32 /*nColorSchemeToken*/ ) const
{
    // TODO
    return 0;
}

const oox::drawingml::table::TableStyleListPtr QuickDiagrammingImport::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

oox::vml::Drawing* QuickDiagrammingImport::getVmlDrawing()
{
    return 0;
}

oox::drawingml::chart::ChartConverter* QuickDiagrammingImport::getChartConverter()
{
    return 0;
}

OUString QuickDiagrammingImport::implGetImplementationName() const
{
    return QuickDiagrammingImport_getImplementationName();
}

::oox::ole::VbaProject* QuickDiagrammingImport::implCreateVbaProject() const
{
    return 0;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
