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

#include "oox/ppt/dgmlayout.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/dump/pptxdumper.hxx"

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/container/XChild.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;
using namespace ::oox::drawingml;

namespace oox { namespace ppt {

OUString SAL_CALL QuickDiagrammingLayout_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.Impress.oox.QuickDiagrammingLayout" );
}

uno::Sequence< OUString > SAL_CALL QuickDiagrammingLayout_getSupportedServiceNames() throw()
{
    const OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.ooxpptx.dgm.layout" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL QuickDiagrammingLayout_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new QuickDiagrammingLayout( rSMgr );
}

QuickDiagrammingLayout::QuickDiagrammingLayout( const uno::Reference< lang::XMultiServiceFactory > & rSMgr  )
    : XmlFilterBase( rSMgr ),
    mpThemePtr(new drawingml::Theme())
{}

bool QuickDiagrammingLayout::importDocument() throw()
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
            xPropSet2->getPropertyValue(
                CREATE_OUSTRING("PPTTheme")) >>= xThemeFragment;

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

    xPropSet->getPropertyValue(
        CREATE_OUSTRING("DiagramData")) >>= xDataModelDom;
    xPropSet->getPropertyValue(
        CREATE_OUSTRING("DiagramLayout")) >>= xLayoutDom;
    xPropSet->getPropertyValue(
        CREATE_OUSTRING("DiagramQStyle")) >>= xQStyleDom;
    xPropSet->getPropertyValue(
        CREATE_OUSTRING("DiagramColorStyle")) >>= xColorStyleDom;

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

    pShape->addChildren( *this,
                         mpThemePtr.get(),
                         xParentShapes,
                         &aRect,
                         NULL );

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

sal_Int32 QuickDiagrammingLayout::getSchemeClr( sal_Int32 nColorSchemeToken ) const
{
    sal_Int32 nColor = 0;
    if( mpThemePtr )
        mpThemePtr->getClrScheme().getColor( nColorSchemeToken,
                                             nColor );
    return nColor;
}

const oox::drawingml::table::TableStyleListPtr QuickDiagrammingLayout::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

::oox::vml::Drawing* QuickDiagrammingLayout::getVmlDrawing()
{
    return 0;
}

::oox::drawingml::chart::ChartConverter* QuickDiagrammingLayout::getChartConverter()
{
    return 0;
}

OUString QuickDiagrammingLayout::implGetImplementationName() const
{
    return QuickDiagrammingLayout_getImplementationName();
}

}}
