/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: graphicshapecontext.cxx,v $
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

#include <osl/diagnose.h>

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/diagram/diagramfragmenthandler.hxx"
#include "oox/drawingml/table/tablecontext.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/drawingml/transform2dcontext.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/drawingml/chart/chartspacefragment.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"
#include "tokens.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

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
GraphicShapeContext::GraphicShapeContext( ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr )
: ShapeContext( rParent, pMasterShapePtr, pShapePtr )
{
}

Reference< XFastContextHandler > GraphicShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( getToken( aElementToken ) )
    {
    // CT_ShapeProperties
    case XML_xfrm:
        xRet.set( new Transform2DContext( *this, xAttribs, *mpShapePtr ) );
        break;
    case XML_blipFill:
        xRet.set( new BlipFillPropertiesContext( *this, xAttribs, *mpShapePtr->getGraphicProperties() ) );
        break;
    }

    if (getNamespace( aElementToken ) == NMSP_VML && mpShapePtr)
    {
        mpShapePtr->setServiceName("com.sun.star.drawing.CustomShape");
        CustomShapePropertiesPtr pCstmShpProps
            (mpShapePtr->getCustomShapeProperties());

        sal_uInt32 nType = aElementToken & (~ NMSP_MASK);
        OUString sType(GetShapeType(nType));

        if (sType.getLength() > 0)
            pCstmShpProps->setShapePresetType(sType);
    }

    if( !xRet.is() )
        xRet.set( ShapeContext::createFastChildContext( aElementToken, xAttribs ) );

    return xRet;
}

// ====================================================================
// CT_GraphicalObjectFrameContext
GraphicalObjectFrameContext::GraphicalObjectFrameContext( ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr )
: ShapeContext( rParent, pMasterShapePtr, pShapePtr )
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
        xRet.set( new Transform2DContext( *this, xAttribs, *mpShapePtr ) );
        break;
    case XML_graphic:               // CT_GraphicalObject
        xRet.set( this );
        break;

        case XML_graphicData :          // CT_GraphicalObjectData
        {
            OUString sUri( xAttribs->getOptionalValue( XML_uri ) );
            if ( sUri.equalsAscii( "http://schemas.openxmlformats.org/presentationml/2006/ole" ) )
                xRet.set( new PresentationOle2006Context( *this, mpShapePtr ) );
            else if ( sUri.equalsAscii( "http://schemas.openxmlformats.org/drawingml/2006/diagram" ) )
                xRet.set( new DiagramGraphicDataContext( *this, mpShapePtr ) );
            else if ( sUri.equalsAscii( "http://schemas.openxmlformats.org/drawingml/2006/chart" ) )
                xRet.set( new ChartGraphicDataContext( *this, mpShapePtr ) );
            else if ( sUri.compareToAscii( "http://schemas.openxmlformats.org/drawingml/2006/table" ) == 0 )
                xRet.set( new table::TableContext( *this, mpShapePtr ) );
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

PresentationOle2006Context::PresentationOle2006Context( ContextHandler& rParent, ShapePtr pShapePtr )
: ShapeContext( rParent, ShapePtr(), pShapePtr )
{
}

PresentationOle2006Context::~PresentationOle2006Context()
{
    static sal_Int32 nObjectCount = 100;

    XmlFilterBase& rFilter = getFilter();
    const OUString aFragmentPath = getFragmentPathFromRelId( msId );
    if( aFragmentPath.getLength() > 0 ) try
    {
        Reference< ::com::sun::star::io::XInputStream > xInputStream( rFilter.openInputStream( aFragmentPath ), UNO_QUERY_THROW );

        Sequence< sal_Int8 > aData;
        xInputStream->readBytes( aData, 0x7fffffff );
        uno::Reference< lang::XMultiServiceFactory > xMSF( rFilter.getModel(), UNO_QUERY_THROW );
        Reference< com::sun::star::document::XEmbeddedObjectResolver > xEmbeddedResolver( xMSF->createInstance( CREATE_OUSTRING( "com.sun.star.document.ImportEmbeddedObjectResolver" ) ), UNO_QUERY_THROW );

        Reference< com::sun::star::container::XNameAccess > xNA( xEmbeddedResolver, UNO_QUERY_THROW );
        OUString aURL = CREATE_OUSTRING( "Obj" ) + OUString::valueOf( nObjectCount++ );
        Reference < XOutputStream > xOLEStream;
        if( (xNA->getByName( aURL ) >>= xOLEStream) && xOLEStream.is() )
        {
            xOLEStream->writeBytes( aData );
            xOLEStream->closeOutput();

            static const OUString sProtocol = CREATE_OUSTRING( "vnd.sun.star.EmbeddedObject:" );
            OUString aPersistName( xEmbeddedResolver->resolveEmbeddedObjectURL( aURL ) );
            aPersistName = aPersistName.copy( sProtocol.getLength() );

            static const OUString sPersistName = CREATE_OUSTRING( "PersistName" );
            mpShapePtr->getShapeProperties()[ sPersistName ] <<= aPersistName;
        }
        Reference< XComponent > xComp( xEmbeddedResolver, UNO_QUERY_THROW );
        xComp->dispose();
    }
    catch( Exception& )
    {
    }

    // taking care of the representation graphic
    if ( msSpid.getLength() )
    {
        oox::vml::DrawingPtr pDrawingPtr = rFilter.getDrawings();
        if ( pDrawingPtr )
        {
            OUString aGraphicURL( pDrawingPtr->getGraphicUrlById( msSpid ) );
            if ( aGraphicURL.getLength() )
            {
                try
                {
                    Reference< io::XInputStream > xInputStream( rFilter.openInputStream( aGraphicURL ), UNO_QUERY_THROW );
                    Reference< graphic::XGraphicProvider > xGraphicProvider( rFilter.getGlobalFactory()->createInstance( CREATE_OUSTRING( "com.sun.star.graphic.GraphicProvider" ) ), UNO_QUERY_THROW );
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

// ====================================================================

DiagramGraphicDataContext::DiagramGraphicDataContext( ContextHandler& rParent, ShapePtr pShapePtr )
: ShapeContext( rParent, ShapePtr(), pShapePtr )
{
    pShapePtr->setServiceName( "com.sun.star.drawing.GroupShape" );
    pShapePtr->setSubType( 0 );
}



DiagramGraphicDataContext::~DiagramGraphicDataContext()
{

}

DiagramPtr DiagramGraphicDataContext::loadDiagram()
{
    DiagramPtr pDiagram( new Diagram() );
    XmlFilterBase& rFilter = getFilter();

    // data
    OUString sDmPath = getFragmentPathFromRelId( msDm );
    if( sDmPath.getLength() > 0 )
    {
        DiagramDataPtr pData( new DiagramData() );
        pDiagram->setData( pData );
        rFilter.importFragment( new DiagramDataFragmentHandler( rFilter, sDmPath, pData ) );
    }
    // layout
    OUString sLoPath = getFragmentPathFromRelId( msLo );
    if( sLoPath.getLength() > 0 )
    {
        DiagramLayoutPtr pLayout( new DiagramLayout() );
        pDiagram->setLayout( pLayout );
        rFilter.importFragment( new DiagramLayoutFragmentHandler( rFilter, sLoPath, pLayout ) );
    }
    // style
    OUString sQsPath = getFragmentPathFromRelId( msQs );
    if( sQsPath.getLength() > 0 )
    {
        DiagramQStylesPtr pStyles( new DiagramQStyles() );
        pDiagram->setQStyles( pStyles );
        rFilter.importFragment( new DiagramQStylesFragmentHandler( rFilter, sQsPath, pStyles ) );
    }
    // colors
    OUString sCsPath = getFragmentPathFromRelId( msCs );
    if( sCsPath.getLength() > 0 )
    {
        DiagramColorsPtr pColors( new DiagramColors() );
        pDiagram->setColors( pColors );
        rFilter.importFragment( new DiagramColorsFragmentHandler( rFilter, sCsPath, pColors ) ) ;
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
        OSL_TRACE("diagram added shape %s of type %s", OUSTRING_TO_CSTR( mpShapePtr->getName() ),
                  OUSTRING_TO_CSTR( mpShapePtr->getServiceName() ) );
        break;
    }
    default:
        break;
    }

    if( !xRet.is() )
        xRet.set( ShapeContext::createFastChildContext( aElementToken, xAttribs ) );

    return xRet;
}

// ====================================================================

class CreateChartCallback : public CreateShapeCallback
{
public:
    explicit            CreateChartCallback( XmlFilterBase& rFilter, const OUString& rFragmentPath );
    virtual void        onCreateXShape( const Reference< drawing::XShape >& rxShape );

private:
    XmlFilterBase&      mrFilter;
    OUString            maFragmentPath;
};

// --------------------------------------------------------------------

CreateChartCallback::CreateChartCallback( XmlFilterBase& rFilter, const OUString& rFragmentPath ) :
    mrFilter( rFilter ),
    maFragmentPath( rFragmentPath )
{
}

void CreateChartCallback::onCreateXShape( const Reference< drawing::XShape >& rxShape )
{
    OSL_ENSURE( maFragmentPath.getLength() > 0, "CreateChartCallback::onCreateXShape - missing chart fragment" );
    if( maFragmentPath.getLength() > 0 ) try
    {
        // set the chart2 OLE class ID at the OLE shape
        PropertySet aShapeProp( rxShape );
        aShapeProp.setProperty( CREATE_OUSTRING( "CLSID" ), CREATE_OUSTRING( "12dcae26-281f-416f-a234-c3086127382e" ) );

        // get the XModel interface of the embedded object from the OLE shape
        Reference< frame::XModel > xDocModel;
        aShapeProp.getProperty( xDocModel, CREATE_OUSTRING( "Model" ) );

        // load the chart data from the XML fragment
        chart::ChartSpaceModel aModel;
        mrFilter.importFragment( new chart::ChartSpaceFragment( mrFilter, maFragmentPath, aModel ) );

        // convert imported chart model to chart document
        Reference< chart2::XChartDocument > xChartDoc( xDocModel, UNO_QUERY_THROW );
        mrFilter.getChartConverter().convertFromModel( mrFilter, aModel, xChartDoc );
    }
    catch( Exception& )
    {
    }
}

// ====================================================================

ChartGraphicDataContext::ChartGraphicDataContext( ContextHandler& rParent, ShapePtr pShapePtr ) :
    ShapeContext( rParent, ShapePtr(), pShapePtr )
{
    pShapePtr->setServiceName( "com.sun.star.drawing.OLE2Shape" );
}

Reference< XFastContextHandler > ChartGraphicDataContext::createFastChildContext( ::sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
    throw (SAXException, RuntimeException)
{
    if( nElement == (NMSP_CHART | XML_chart) )
    {
        AttributeList aAttribs( rxAttribs );
        OUString aFragmentPath = getFragmentPathFromRelId( aAttribs.getString( NMSP_RELATIONSHIPS | XML_id, OUString() ) );
        CreateShapeCallbackRef xCallback( new CreateChartCallback( getFilter(), aFragmentPath ) );
        mpShapePtr->setCreateShapeCallback( xCallback );
    }
    return 0;
}

// ====================================================================

} }
