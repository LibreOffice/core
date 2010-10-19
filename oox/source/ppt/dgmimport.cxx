/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pptimport.cxx,v $
 * $Revision: 1.8.6.1 $
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

#include "oox/ppt/dgmimport.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/dump/pptxdumper.hxx"

#include <com/sun/star/drawing/XShape.hpp>

using ::rtl::OUString;
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

uno::Reference< uno::XInterface > SAL_CALL QuickDiagrammingImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new QuickDiagrammingImport( rSMgr );
}

QuickDiagrammingImport::QuickDiagrammingImport( const uno::Reference< lang::XMultiServiceFactory > & rSMgr  )
    : XmlFilterBase( rSMgr )
{}

bool QuickDiagrammingImport::importDocument() throw()
{
    /*  to activate the PPTX dumper, define the environment variable
        OOO_PPTXDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/pptxdumper.ini. */
    OOX_DUMP_FILE( ::oox::dump::pptx::Dumper );

    OUString aEmpty;
    OUString aFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATIONSTYPE( "diagramLayout" ) );

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
    pShape->addShape( *this,
                      pTheme.get(),
                      xParentShape,
                      NULL,
                      NULL );

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

}}
