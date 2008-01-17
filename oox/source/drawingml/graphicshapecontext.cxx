/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphicshapecontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:51 $
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

#include <osl/diagnose.h>

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/diagram/diagramfragmenthandler.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "tokens.hxx"
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/io/XStream.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/document/XEmbeddedObjectResolver.hpp"
#include <com/sun/star/graphic/XGraphicProvider.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// ====================================================================
// CT_Picture
GraphicShapeContext::GraphicShapeContext( const FragmentHandlerRef& xHandler, ShapePtr pMasterShapePtr, ShapePtr pShapePtr )
: ShapeContext( xHandler, pMasterShapePtr, pShapePtr )
{
}

Reference< XFastContextHandler > GraphicShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken &(~NMSP_MASK) )
    {
    // CT_ShapeProperties
    case XML_xfrm:
        xRet.set( new Transform2DContext( getHandler(), xAttribs, *(mpShapePtr.get()) ) );
        break;
    case XML_blipFill:
        xRet.set( new BlipFillPropertiesContext( getHandler(), xAttribs, *(mpShapePtr->getGraphicProperties().get()) ) );
        break;
    }
    if( !xRet.is() )
        xRet.set( ShapeContext::createFastChildContext( aElementToken, xAttribs ) );

    return xRet;
}

// ====================================================================
// CT_GraphicalObjectFrameContext
GraphicalObjectFrameContext::GraphicalObjectFrameContext( const FragmentHandlerRef& xHandler, ShapePtr pMasterShapePtr, ShapePtr pShapePtr )
: ShapeContext( xHandler, pMasterShapePtr, pShapePtr )
{
}

Reference< XFastContextHandler > GraphicalObjectFrameContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken &(~NMSP_MASK) )
    {
    // CT_ShapeProperties
    case XML_nvGraphicFramePr:      // CT_GraphicalObjectFrameNonVisual
        break;
    case XML_xfrm:                  // CT_Transform2D
        xRet.set( new Transform2DContext( getHandler(), xAttribs, *(mpShapePtr.get()) ) );
        break;
    case XML_graphic:               // CT_GraphicalObject
        xRet.set( this );
        break;

        case XML_graphicData :          // CT_GraphicalObjectData
        {
            rtl::OUString sUri( xAttribs->getOptionalValue( XML_uri ) );
            if ( sUri.compareToAscii( "http://schemas.openxmlformats.org/presentationml/2006/ole" ) == 0 )
                xRet.set( new PresentationOle2006Context( mxHandler, mpShapePtr ) );
            else if ( sUri.compareToAscii( "http://schemas.openxmlformats.org/drawingml/2006/diagram" ) == 0 )
                xRet.set( new DiagramGraphicDataContext( mxHandler, mpShapePtr ) );
            else if ( sUri.compareToAscii( "http://schemas.openxmlformats.org/drawingml/2006/table" ) == 0 )
                // TODO deal with tables too.
                xRet.set( this );
            else
            {
                OSL_TRACE( "OOX: Ignore graphicsData of %s", OUSTRING_TO_CSTR( sUri ) );
                return xRet;
            }
        }
        break;
    }
    if( !xRet.is() )
        xRet.set( ShapeContext::createFastChildContext( aElementToken, xAttribs ) );

    return xRet;
}

// ====================================================================

PresentationOle2006Context::PresentationOle2006Context( const FragmentHandlerRef& xHandler, ShapePtr pShapePtr )
: ShapeContext( xHandler, ShapePtr(), pShapePtr )
{
}

PresentationOle2006Context::~PresentationOle2006Context()
{
    XmlFilterRef xFilter = getHandler()->getFilter();
    const OUString aFragmentPath = getHandler()->getFragmentPathFromRelId( msId );
    if( aFragmentPath.getLength() > 0 )
    {
        Reference< ::com::sun::star::io::XInputStream > xInputStream( xFilter->openInputStream( aFragmentPath ), UNO_QUERY_THROW );

        Sequence< sal_Int8 > aData;
        xInputStream->readBytes( aData, 0x7fffffff );
        uno::Reference< lang::XMultiServiceFactory > xMSF( xFilter->getModel(), UNO_QUERY );
        Reference< com::sun::star::document::XEmbeddedObjectResolver > xEmbeddedResolver( xMSF->createInstance( OUString::createFromAscii( "com.sun.star.document.ImportEmbeddedObjectResolver" ) ), UNO_QUERY );

        if ( xEmbeddedResolver.is() )
        {
            Reference< com::sun::star::container::XNameAccess > xNA( xEmbeddedResolver, UNO_QUERY );
            if( xNA.is() )
            {
                Reference < XOutputStream > xOLEStream;
                OUString aURL = CREATE_OUSTRING( "Obj12345678" );
                Any aAny( xNA->getByName( aURL ) );
                aAny >>= xOLEStream;
                if ( xOLEStream.is() )
                {
                    xOLEStream->writeBytes( aData );
                    xOLEStream->closeOutput();

                    const OUString sProtocol = CREATE_OUSTRING( "vnd.sun.star.EmbeddedObject:" );
                    rtl::OUString aPersistName( xEmbeddedResolver->resolveEmbeddedObjectURL( aURL ) );
                    aPersistName = aPersistName.copy( sProtocol.getLength() );

                    static const OUString sPersistName = CREATE_OUSTRING( "PersistName" );
                    mpShapePtr->getShapeProperties()[ sPersistName ] <<= aPersistName;
                }
            }
            Reference< XComponent > xComp( xEmbeddedResolver, UNO_QUERY );
            xComp->dispose();
        }
    }

    // taking care of the representation graphic
    if ( msSpid.getLength() )
    {
        oox::vml::DrawingPtr pDrawingPtr = xFilter->getDrawings();
        if ( pDrawingPtr.get() )
        {
            rtl::OUString aGraphicURL( pDrawingPtr->getGraphicUrlById( msSpid ) );
            if ( aGraphicURL.getLength() )
            {
                try
                {
                    uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
                    Reference< io::XInputStream > xInputStream( xFilter->openInputStream( aGraphicURL ), UNO_QUERY_THROW );
                    Reference< graphic::XGraphicProvider > xGraphicProvider( xMSF->createInstance( OUString::createFromAscii( "com.sun.star.graphic.GraphicProvider" ) ), UNO_QUERY_THROW );
                    if ( xInputStream.is() && xGraphicProvider.is() )
                    {
                        Sequence< PropertyValue > aArgs( 1 );
                        const OUString sInputStream = CREATE_OUSTRING( "InputStream" );
                        aArgs[ 0 ].Name = sInputStream;
                        aArgs[ 0 ].Value <<= xInputStream;
                        Reference< graphic::XGraphic > xGraphic = xGraphicProvider->queryGraphic( aArgs );
                        if ( xGraphic.is() )
                        {
                            static const OUString sEmptyGraphicURL;
                            static const OUString sGraphicURL = CREATE_OUSTRING( "GraphicURL" );
                            mpShapePtr->getShapeProperties()[ sGraphicURL ] <<= sEmptyGraphicURL;

                            static const OUString sThumbnailGraphic = CREATE_OUSTRING( "Graphic" );
                            mpShapePtr->getShapeProperties()[ sThumbnailGraphic ] <<= xGraphic;
                        }
                    }
                }
                catch( Exception& )
                {
                }
            }
        }
    }
}

Reference< XFastContextHandler > PresentationOle2006Context::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken &(~NMSP_MASK) )
    {
        case XML_oleObj:
        {
            msSpid = xAttribs->getOptionalValue( XML_spid );
            msName = xAttribs->getOptionalValue( XML_name );
            msId = xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_id );
            mnWidth = GetCoordinate( xAttribs->getOptionalValue( XML_imgW ) );
            mnHeight = GetCoordinate( xAttribs->getOptionalValue( XML_imgH ) );
            msProgId = xAttribs->getOptionalValue( XML_progId );
        }
        break;

            case XML_embed:
            {
                mnFollowColorSchemeToken = xAttribs->getOptionalValueToken( XML_followColorScheme, XML_full );
            }
            break;
    }
    if( !xRet.is() )
        xRet.set( ShapeContext::createFastChildContext( aElementToken, xAttribs ) );

    return xRet;
}

DiagramGraphicDataContext::DiagramGraphicDataContext( const ::oox::core::FragmentHandlerRef& xHandler, ShapePtr pShapePtr )
: ShapeContext( xHandler, ShapePtr(), pShapePtr )
{
}



DiagramGraphicDataContext::~DiagramGraphicDataContext()
{

}

DiagramPtr DiagramGraphicDataContext::loadDiagram()
{
    DiagramPtr pDiagram( new Diagram() );
    const oox::core::XmlFilterRef& xFilter( getHandler()->getFilter() );

    // data
    OUString sDmPath = getHandler()->getFragmentPathFromRelId( msDm );
    if( sDmPath.getLength() > 0 )
    {
        DiagramDataPtr pData( new DiagramData() );
        pDiagram->setData( pData );
        xFilter->importFragment( new DiagramDataFragmentHandler( xFilter, sDmPath, pData ) );
    }
    // layout
    OUString sLoPath = getHandler()->getFragmentPathFromRelId( msLo );
    if( sLoPath.getLength() > 0 )
    {
        DiagramLayoutPtr pLayout( new DiagramLayout() );
        pDiagram->setLayout( pLayout );
        xFilter->importFragment( new DiagramLayoutFragmentHandler( xFilter, sLoPath, pLayout ) );
    }
    // style
    OUString sQsPath = getHandler()->getFragmentPathFromRelId( msQs );
    if( sQsPath.getLength() > 0 )
    {
        DiagramQStylesPtr pStyles( new DiagramQStyles() );
        pDiagram->setQStyles( pStyles );
        xFilter->importFragment( new DiagramQStylesFragmentHandler( xFilter, sQsPath, pStyles ) );
    }
    // colors
    OUString sCsPath = getHandler()->getFragmentPathFromRelId( msCs );
    if( sCsPath.getLength() > 0 )
    {
        DiagramColorsPtr pColors( new DiagramColors() );
        pDiagram->setColors( pColors );
        xFilter->importFragment( new DiagramColorsFragmentHandler( xFilter, sCsPath, pColors ) ) ;
    }

    return pDiagram;
}


Reference< XFastContextHandler > DiagramGraphicDataContext::createFastChildContext( ::sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_DIAGRAM|XML_relIds:
    {
        msDm = xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_dm );
        msLo = xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_lo );
        msQs = xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_qs );
        msCs = xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_cs );
        DiagramPtr pDiagram = loadDiagram();
        pDiagram->addTo( mpShapePtr );
        break;
    }
    default:
        break;
    }

    if( !xRet.is() )
        xRet.set( ShapeContext::createFastChildContext( aElementToken, xAttribs ) );

    return xRet;
}

} }
