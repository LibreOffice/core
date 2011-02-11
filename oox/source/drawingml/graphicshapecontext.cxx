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

#include "oox/drawingml/graphicshapecontext.hxx"
#include <osl/diagnose.h>

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/drawingml/diagram/diagramfragmenthandler.hxx"
#include "oox/drawingml/table/tablecontext.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/transform2dcontext.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

namespace oox {
namespace drawingml {

// ============================================================================
// CT_Picture

GraphicShapeContext::GraphicShapeContext( ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr )
: ShapeContext( rParent, pMasterShapePtr, pShapePtr )
{
}

Reference< XFastContextHandler > GraphicShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( getBaseToken( aElementToken ) )
    {
    // CT_ShapeProperties
    case XML_xfrm:
        xRet.set( new Transform2DContext( *this, xAttribs, *mpShapePtr ) );
        break;
    case XML_blipFill:
        xRet.set( new BlipFillContext( *this, xAttribs, mpShapePtr->getGraphicProperties().maBlipProps ) );
        break;
    }

    if ((getNamespace( aElementToken ) == NMSP_vml) && mpShapePtr)
    {
        mpShapePtr->setServiceName("com.sun.star.drawing.CustomShape");
        CustomShapePropertiesPtr pCstmShpProps
            (mpShapePtr->getCustomShapeProperties());

        sal_uInt32 nType = getBaseToken( aElementToken );
        OUString sType(GetShapeType(nType));

        if (sType.getLength() > 0)
            pCstmShpProps->setShapePresetType(sType);
    }

    if( !xRet.is() )
        xRet.set( ShapeContext::createFastChildContext( aElementToken, xAttribs ) );

    return xRet;
}

// ============================================================================
// CT_GraphicalObjectFrameContext

GraphicalObjectFrameContext::GraphicalObjectFrameContext( ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr, bool bEmbedShapesInChart ) :
    ShapeContext( rParent, pMasterShapePtr, pShapePtr ),
    mbEmbedShapesInChart( bEmbedShapesInChart )
{
}

Reference< XFastContextHandler > GraphicalObjectFrameContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( getBaseToken( aElementToken ) )
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
                xRet.set( new OleObjectGraphicDataContext( *this, mpShapePtr ) );
            else if ( sUri.equalsAscii( "http://schemas.openxmlformats.org/drawingml/2006/diagram" ) )
                xRet.set( new DiagramGraphicDataContext( *this, mpShapePtr ) );
            else if ( sUri.equalsAscii( "http://schemas.openxmlformats.org/drawingml/2006/chart" ) )
                xRet.set( new ChartGraphicDataContext( *this, mpShapePtr, mbEmbedShapesInChart ) );
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

// ============================================================================

OleObjectGraphicDataContext::OleObjectGraphicDataContext( ContextHandler& rParent, ShapePtr xShape ) :
    ShapeContext( rParent, ShapePtr(), xShape ),
    mrOleObjectInfo( xShape->setOleObjectType() )
{
}

OleObjectGraphicDataContext::~OleObjectGraphicDataContext()
{
    /*  Register the OLE shape at the VML drawing, this prevents that the
        related VML shape converts the OLE object by itself. */
    if( mrOleObjectInfo.maShapeId.getLength() > 0 )
        if( ::oox::vml::Drawing* pVmlDrawing = getFilter().getVmlDrawing() )
            pVmlDrawing->registerOleObject( mrOleObjectInfo );
}

Reference< XFastContextHandler > OleObjectGraphicDataContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    AttributeList aAttribs( xAttribs );

    switch( nElement )
    {
        case PPT_TOKEN( oleObj ):
        {
            mrOleObjectInfo.maShapeId = aAttribs.getXString( XML_spid, OUString() );
            const Relation* pRelation = getRelations().getRelationFromRelId( aAttribs.getString( R_TOKEN( id ), OUString() ) );
            OSL_ENSURE( pRelation, "OleObjectGraphicDataContext::createFastChildContext - missing relation for OLE object" );
            if( pRelation )
            {
                mrOleObjectInfo.mbLinked = pRelation->mbExternal;
                if( pRelation->mbExternal )
                {
                    mrOleObjectInfo.maTargetLink = getFilter().getAbsoluteUrl( pRelation->maTarget );
                }
                else
                {
                    OUString aFragmentPath = getFragmentPathFromRelation( *pRelation );
                    if( aFragmentPath.getLength() > 0 )
                        getFilter().importBinaryData( mrOleObjectInfo.maEmbeddedData, aFragmentPath );
                }
            }
            mrOleObjectInfo.maName = aAttribs.getXString( XML_name, OUString() );
            mrOleObjectInfo.maProgId = aAttribs.getXString( XML_progId, OUString() );
            mrOleObjectInfo.mbShowAsIcon = aAttribs.getBool( XML_showAsIcon, false );
            xRet.set( this );
        }
        break;

        case PPT_TOKEN( embed ):
            OSL_ENSURE( !mrOleObjectInfo.mbLinked, "OleObjectGraphicDataContext::createFastChildContext - unexpected child element" );
        break;

        case PPT_TOKEN( link ):
            OSL_ENSURE( mrOleObjectInfo.mbLinked, "OleObjectGraphicDataContext::createFastChildContext - unexpected child element" );
            mrOleObjectInfo.mbAutoUpdate = aAttribs.getBool( XML_updateAutomatic, false );
        break;
    }
    return xRet;
}

// ============================================================================

DiagramGraphicDataContext::DiagramGraphicDataContext( ContextHandler& rParent, ShapePtr pShapePtr )
: ShapeContext( rParent, ShapePtr(), pShapePtr )
{
    pShapePtr->setDiagramType();
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
    case DGM_TOKEN( relIds ):
    {
        msDm = xAttribs->getOptionalValue( R_TOKEN( dm ) );
        msLo = xAttribs->getOptionalValue( R_TOKEN( lo ) );
        msQs = xAttribs->getOptionalValue( R_TOKEN( qs ) );
        msCs = xAttribs->getOptionalValue( R_TOKEN( cs ) );
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

// ============================================================================

ChartGraphicDataContext::ChartGraphicDataContext( ContextHandler& rParent, const ShapePtr& rxShape, bool bEmbedShapes ) :
    ShapeContext( rParent, ShapePtr(), rxShape ),
    mrChartShapeInfo( rxShape->setChartType( bEmbedShapes ) )
{
}

Reference< XFastContextHandler > ChartGraphicDataContext::createFastChildContext( ::sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
    throw (SAXException, RuntimeException)
{
    if( nElement == C_TOKEN( chart ) )
    {
        AttributeList aAttribs( rxAttribs );
        mrChartShapeInfo.maFragmentPath = getFragmentPathFromRelId( aAttribs.getString( R_TOKEN( id ), OUString() ) );
    }
    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

