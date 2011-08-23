/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "oox/ppt/pptimport.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/dump/pptxdumper.hxx"
#include "oox/drawingml/table/tablestylelistfragmenthandler.hxx"
#include "oox/helper/graphichelper.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::lang::XComponent;

namespace oox { namespace ppt {

OUString SAL_CALL PowerPointImport_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.Impress.oox.PowerPointImport" );
}

uno::Sequence< OUString > SAL_CALL PowerPointImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.ooxpptx" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL PowerPointImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new PowerPointImport( rSMgr );
}

PowerPointImport::PowerPointImport( const uno::Reference< lang::XMultiServiceFactory > & rSMgr  )
    : XmlFilterBase( rSMgr )
    , mxChartConv( new ::oox::drawingml::chart::ChartConverter )
{
}

PowerPointImport::~PowerPointImport()
{
}

bool PowerPointImport::importDocument() throw()
{
    /*  to activate the PPTX dumper, define the environment variable
        OOO_PPTXDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/pptxdumper.ini. */
    OOX_DUMP_FILE( ::oox::dump::pptx::Dumper );

    OUString aFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATIONSTYPE( "officeDocument" ) );
    FragmentHandlerRef xPresentationFragmentHandler( new PresentationFragmentHandler( *this, aFragmentPath ) );
    maTableStyleListPath = xPresentationFragmentHandler->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATIONSTYPE( "tableStyles" ) );
    return importFragment( xPresentationFragmentHandler );


}

bool PowerPointImport::exportDocument() throw()
{
    return false;
}

sal_Int32 PowerPointImport::getSchemeColor( sal_Int32 nToken ) const
{
    sal_Int32 nColor = 0;
    if ( mpActualSlidePersist )
    {
        sal_Bool bColorMapped = sal_False;
        oox::drawingml::ClrMapPtr pClrMapPtr( mpActualSlidePersist->getClrMap() );
        if ( pClrMapPtr )
            bColorMapped = pClrMapPtr->getColorMap( nToken );

        if ( !bColorMapped )	// try masterpage mapping
        {
            SlidePersistPtr pMasterPersist = mpActualSlidePersist->getMasterPersist();
            if ( pMasterPersist )
            {
                pClrMapPtr = pMasterPersist->getClrMap();
                if ( pClrMapPtr )
                    bColorMapped = pClrMapPtr->getColorMap( nToken );
            }
        }
        oox::drawingml::ClrSchemePtr pClrSchemePtr( mpActualSlidePersist->getClrScheme() );
        if ( pClrSchemePtr )
            pClrSchemePtr->getColor( nToken, nColor );
        else
        {
            ::oox::drawingml::ThemePtr pTheme = mpActualSlidePersist->getTheme();
            if( pTheme )
            {
                pTheme->getClrScheme().getColor( nToken, nColor );
            }
            else
            {
                OSL_TRACE("OOX: PowerPointImport::mpThemePtr is NULL");
            }
        }
    }
    return nColor;
}

const ::oox::drawingml::Theme* PowerPointImport::getCurrentTheme() const
{
    return mpActualSlidePersist ? mpActualSlidePersist->getTheme().get() : 0;
}

sal_Bool SAL_CALL PowerPointImport::filter( const Sequence< PropertyValue >& rDescriptor ) throw( RuntimeException )
{
    if( XmlFilterBase::filter( rDescriptor ) )
        return true;

    if( isExportFilter() ) {
        Reference< XExporter > xExporter( getGlobalFactory()->createInstance( CREATE_OUSTRING( "com.sun.star.comp.Impress.oox.PowerPointExport" ) ), UNO_QUERY );

        if( xExporter.is() ) {
            Reference< XComponent > xDocument( getModel(), UNO_QUERY );
            Reference< XFilter > xFilter( xExporter, UNO_QUERY );

            if( xFilter.is() ) {
                xExporter->setSourceDocument( xDocument );
                if( xFilter->filter( rDescriptor ) )
                    return true;
            }
        }
    }

    return false;
}

::oox::vml::Drawing* PowerPointImport::getVmlDrawing()
{
    return mpActualSlidePersist ? mpActualSlidePersist->getDrawing() : 0;
}

const oox::drawingml::table::TableStyleListPtr PowerPointImport::getTableStyles()
{
    if ( !mpTableStyleList && maTableStyleListPath.getLength() )
    {
        mpTableStyleList = oox::drawingml::table::TableStyleListPtr( new oox::drawingml::table::TableStyleList() );
        importFragment( new oox::drawingml::table::TableStyleListFragmentHandler(
            *this, maTableStyleListPath, *mpTableStyleList ) );
    }
    return mpTableStyleList;;
}

::oox::drawingml::chart::ChartConverter& PowerPointImport::getChartConverter()
{
    return *mxChartConv;
}

namespace {

class PptGraphicHelper : public GraphicHelper
{
public:
    explicit            PptGraphicHelper( const PowerPointImport& rFilter );
    virtual sal_Int32   getSchemeColor( sal_Int32 nToken ) const;
private:
    const PowerPointImport& mrFilter;
};

PptGraphicHelper::PptGraphicHelper( const PowerPointImport& rFilter ) :
    GraphicHelper( rFilter.getGlobalFactory(), rFilter.getTargetFrame(), rFilter.getStorage() ),
    mrFilter( rFilter )
{
}

sal_Int32 PptGraphicHelper::getSchemeColor( sal_Int32 nToken ) const
{
    return mrFilter.getSchemeColor( nToken );
}

} // namespace

GraphicHelper* PowerPointImport::implCreateGraphicHelper() const
{
    return new PptGraphicHelper( *this );
}

OUString PowerPointImport::implGetImplementationName() const
{
    return PowerPointImport_getImplementationName();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
