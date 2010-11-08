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
#include "oox/ole/vbaproject.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;

namespace oox { namespace ppt {

OUString SAL_CALL PowerPointImport_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.ppt.PowerPointImport" );
}

uno::Sequence< OUString > SAL_CALL PowerPointImport_getSupportedServiceNames() throw()
{
    Sequence< OUString > aSeq( 2 );
    aSeq[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.ImportFilter" );
    aSeq[ 1 ] = CREATE_OUSTRING( "com.sun.star.document.ExportFilter" );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL PowerPointImport_createInstance( const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new PowerPointImport( rxContext ) );
}

PowerPointImport::PowerPointImport( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    XmlFilterBase( rxContext ),
    mxChartConv( new ::oox::drawingml::chart::ChartConverter )
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

    OUString aFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "officeDocument" ) );
    FragmentHandlerRef xPresentationFragmentHandler( new PresentationFragmentHandler( *this, aFragmentPath ) );
    maTableStyleListPath = xPresentationFragmentHandler->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "tableStyles" ) );
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

        if ( !bColorMapped )    // try masterpage mapping
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
    GraphicHelper( rFilter.getComponentContext(), rFilter.getTargetFrame(), rFilter.getStorage() ),
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

::oox::ole::VbaProject* PowerPointImport::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject( getComponentContext(), getModel(), CREATE_OUSTRING( "Impress" ) );
}

OUString PowerPointImport::implGetImplementationName() const
{
    return PowerPointImport_getImplementationName();
}

}}
