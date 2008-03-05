/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptimport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:47:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/ppt/pptimport.hxx"
#include "oox/dump/pptxdumper.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;

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
{
}

PowerPointImport::~PowerPointImport()
{
}

bool PowerPointImport::importDocument() throw()
{
    /*  to activate the PPTX dumper, define the environment variable
        OOO_PPTXDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/pptxdumperconfig.dat. */
    OOX_DUMP_FILE( ::oox::dump::pptx::Dumper );

    OUString aFragmentPath = getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "officeDocument" ) );
    return importFragment( new PresentationFragmentHandler( *this, aFragmentPath ) );
}

bool PowerPointImport::exportDocument() throw()
{
    return false;
}

sal_Int32 PowerPointImport::getSchemeClr( sal_Int32 nColorSchemeToken ) const
{
    sal_Int32 nColor = 0;
    if ( mpActualSlidePersist )
    {
        sal_Bool bColorMapped = sal_False;
        oox::drawingml::ClrMapPtr pClrMapPtr( mpActualSlidePersist->getClrMap() );
        if ( pClrMapPtr )
            bColorMapped = pClrMapPtr->getColorMap( nColorSchemeToken );

        if ( !bColorMapped )    // try masterpage mapping
        {
            SlidePersistPtr pMasterPersist = mpActualSlidePersist->getMasterPersist();
            if ( pMasterPersist )
            {
                pClrMapPtr = pMasterPersist->getClrMap();
                if ( pClrMapPtr )
                    bColorMapped = pClrMapPtr->getColorMap( nColorSchemeToken );
            }
        }
        oox::drawingml::ClrSchemePtr pClrSchemePtr( mpActualSlidePersist->getClrScheme() );
        if ( pClrSchemePtr )
            pClrSchemePtr->getColor( nColorSchemeToken, nColor );
        else
        {
            drawingml::ThemePtr pTheme = mpActualSlidePersist->getTheme();
            if( pTheme )
            {
                pTheme->getClrScheme()->getColor( nColorSchemeToken, nColor );
            }
            else
            {
                OSL_TRACE("OOX: PowerPointImport::mpThemePtr is NULL");
            }
        }
    }
    return nColor;
}

const oox::vml::DrawingPtr PowerPointImport::getDrawings()
{
    oox::vml::DrawingPtr xRet;
    if ( mpActualSlidePersist )
        xRet = mpActualSlidePersist->getDrawing();
    return xRet;
}

OUString PowerPointImport::implGetImplementationName() const
{
    return PowerPointImport_getImplementationName();
}

}}
