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

#include <com/sun/star/io/TempFile.hpp>
#include "oox/drawingml/graphicshapecontext.hxx"

#include <osl/diagnose.h>

#include <drawingml/embeddedwavaudiofile.hxx>
#include "drawingml/fillpropertiesgroupcontext.hxx"
#include "drawingml/graphicproperties.hxx"
#include "drawingml/customshapeproperties.hxx"
#include "drawingml/diagram/diagram.hxx"
#include "drawingml/table/tablecontext.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "drawingml/transform2dcontext.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/ppt/pptshapegroupcontext.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

static uno::Reference<io::XInputStream>
lcl_GetMediaStream(const OUString& rStream, const oox::core::XmlFilterBase& rFilter)
{
    if (rStream.isEmpty())
        return nullptr;

    Reference< XInputStream > xInStrm( rFilter.openInputStream(rStream), UNO_SET_THROW );
    return xInStrm;
}

static OUString lcl_GetMediaReference(const OUString& rStream)
{
    return rStream.isEmpty() ? OUString() : "vnd.sun.star.Package:" + rStream;
}

namespace oox {
namespace drawingml {

// CT_Picture

GraphicShapeContext::GraphicShapeContext( ContextHandler2Helper& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr )
: ShapeContext( rParent, pMasterShapePtr, pShapePtr )
{
}

ContextHandlerRef GraphicShapeContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( getBaseToken( aElementToken ) )
    {
    // CT_ShapeProperties
    case XML_xfrm:
        return new Transform2DContext( *this, rAttribs, *mpShapePtr );
    case XML_blipFill:
        return new BlipFillContext( *this, rAttribs, mpShapePtr->getGraphicProperties().maBlipProps );
    case XML_wavAudioFile:
        {
            OUString const path(getEmbeddedWAVAudioFile(getRelations(), rAttribs));
            mpShapePtr->getGraphicProperties().m_xMediaStream =
                lcl_GetMediaStream(path, getFilter());
            mpShapePtr->getGraphicProperties().m_sMediaPackageURL =
                lcl_GetMediaReference(path);
        }
        break;
    case XML_audioFile:
    case XML_videoFile:
        {
            OUString rPath = getRelations().getFragmentPathFromRelId(
                    rAttribs.getString(R_TOKEN(link)).get() );
            mpShapePtr->getGraphicProperties().m_xMediaStream =
                lcl_GetMediaStream(rPath, getFilter());
            mpShapePtr->getGraphicProperties().m_sMediaPackageURL =
                lcl_GetMediaReference(rPath);
        }
        break;
    }

    if ((getNamespace( aElementToken ) == NMSP_vml) && mpShapePtr)
    {
        mpShapePtr->setServiceName("com.sun.star.drawing.CustomShape");
        CustomShapePropertiesPtr pCstmShpProps
            (mpShapePtr->getCustomShapeProperties());

        pCstmShpProps->setShapePresetType( getBaseToken( aElementToken ) );
    }

    return ShapeContext::onCreateContext( aElementToken, rAttribs );
}

// CT_GraphicalObjectFrameContext

GraphicalObjectFrameContext::GraphicalObjectFrameContext( ContextHandler2Helper& rParent, ShapePtr pMasterShapePtr, ShapePtr pShapePtr, bool bEmbedShapesInChart ) :
    ShapeContext( rParent, pMasterShapePtr, pShapePtr ),
    mbEmbedShapesInChart( bEmbedShapesInChart ),
    mpParent(&rParent)
{
}

ContextHandlerRef GraphicalObjectFrameContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( getBaseToken( aElementToken ) )
    {
    // CT_ShapeProperties
    case XML_nvGraphicFramePr:      // CT_GraphicalObjectFrameNonVisual
        break;
    case XML_xfrm:                  // CT_Transform2D
        return new Transform2DContext( *this, rAttribs, *mpShapePtr );
    case XML_graphic:               // CT_GraphicalObject
        return this;

        case XML_graphicData :          // CT_GraphicalObjectData
        {
            OUString sUri( rAttribs.getString( XML_uri ).get() );
            if ( sUri == "http://schemas.openxmlformats.org/presentationml/2006/ole" ||
                    sUri == "http://purl.oclc.org/ooxml/presentationml/ole" )
                return new OleObjectGraphicDataContext( *this, mpShapePtr );
            else if ( sUri == "http://schemas.openxmlformats.org/drawingml/2006/diagram" ||
                    sUri == "http://purl.oclc.org/ooxml/drawingml/diagram" )
                return new DiagramGraphicDataContext( *this, mpShapePtr );
            else if ( sUri == "http://schemas.openxmlformats.org/drawingml/2006/chart" ||
                    sUri == "http://purl.oclc.org/ooxml/drawingml/chart" )
                return new ChartGraphicDataContext( *this, mpShapePtr, mbEmbedShapesInChart );
            else if ( sUri == "http://schemas.openxmlformats.org/drawingml/2006/table" ||
                    sUri == "http://purl.oclc.org/ooxml/drawingml/table" )
                return new table::TableContext( *this, mpShapePtr );
            else
            {
                SAL_WARN("oox", "OOX: Ignore graphicsData of :" << sUri );
                return nullptr;
            }
        }
        break;
    }

    return ShapeContext::onCreateContext( aElementToken, rAttribs );
}

void GraphicalObjectFrameContext::onEndElement()
{
    if( getCurrentElement() == PPT_TOKEN( graphicFrame ) && mpParent )
    {
        oox::ppt::PPTShapeGroupContext* pParent = dynamic_cast<oox::ppt::PPTShapeGroupContext*>(mpParent);
        if( pParent )
            pParent->importExtDrawings();
    }
}

OleObjectGraphicDataContext::OleObjectGraphicDataContext( ContextHandler2Helper& rParent, ShapePtr xShape ) :
    ShapeContext( rParent, ShapePtr(), xShape ),
    mrOleObjectInfo( xShape->setOleObjectType() )
{
}

OleObjectGraphicDataContext::~OleObjectGraphicDataContext()
{
    /*  Register the OLE shape at the VML drawing, this prevents that the
        related VML shape converts the OLE object by itself. */
    if( !mrOleObjectInfo.maShapeId.isEmpty() )
        if( ::oox::vml::Drawing* pVmlDrawing = getFilter().getVmlDrawing() )
            pVmlDrawing->registerOleObject( mrOleObjectInfo );
}

ContextHandlerRef OleObjectGraphicDataContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case PPT_TOKEN( oleObj ):
        {
            mrOleObjectInfo.maShapeId = rAttribs.getXString( XML_spid, OUString() );
            const Relation* pRelation = getRelations().getRelationFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
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
                    if( !aFragmentPath.isEmpty() )
                        getFilter().importBinaryData( mrOleObjectInfo.maEmbeddedData, aFragmentPath );
                }
            }
            mrOleObjectInfo.maName = rAttribs.getXString( XML_name, OUString() );
            mrOleObjectInfo.maProgId = rAttribs.getXString( XML_progId, OUString() );
            mrOleObjectInfo.mbShowAsIcon = rAttribs.getBool( XML_showAsIcon, false );
            return this;
        }
        break;

        case PPT_TOKEN( embed ):
            OSL_ENSURE( !mrOleObjectInfo.mbLinked, "OleObjectGraphicDataContext::createFastChildContext - unexpected child element" );
        break;

        case PPT_TOKEN( link ):
            OSL_ENSURE( mrOleObjectInfo.mbLinked, "OleObjectGraphicDataContext::createFastChildContext - unexpected child element" );
            mrOleObjectInfo.mbAutoUpdate = rAttribs.getBool( XML_updateAutomatic, false );
        break;
        case PPT_TOKEN( pic ):
            return new GraphicShapeContext( *this, mpMasterShapePtr, mpShapePtr );
        break;
    }
    return nullptr;
}

DiagramGraphicDataContext::DiagramGraphicDataContext( ContextHandler2Helper& rParent, ShapePtr pShapePtr )
: ShapeContext( rParent, ShapePtr(), pShapePtr )
{
    pShapePtr->setDiagramType();
}

DiagramGraphicDataContext::~DiagramGraphicDataContext()
{
}

ContextHandlerRef DiagramGraphicDataContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case DGM_TOKEN( relIds ):
    {
        msDm = rAttribs.getString( R_TOKEN( dm ) ).get();
        msLo = rAttribs.getString( R_TOKEN( lo ) ).get();
        msQs = rAttribs.getString( R_TOKEN( qs ) ).get();
        msCs = rAttribs.getString( R_TOKEN( cs ) ).get();
        loadDiagram(mpShapePtr,
                    getFilter(),
                    getFragmentPathFromRelId( msDm ),
                    getFragmentPathFromRelId( msLo ),
                    getFragmentPathFromRelId( msQs ),
                    getFragmentPathFromRelId( msCs ));
        SAL_INFO("oox.drawingml", OSL_THIS_FUNC
                 << "diagram added shape " << mpShapePtr->getName()
                 << " of type " << mpShapePtr->getServiceName()
                 << ", size (" << mpShapePtr->getPosition().X
                 << "," << mpShapePtr->getPosition().Y
                 << "," << mpShapePtr->getSize().Width
                 << "," << mpShapePtr->getSize().Height
                 <<")");

        // No DrawingML fallback, need to warn the user at the end.
        if (mpShapePtr->getExtDrawings().empty())
            getFilter().setMissingExtDrawing();

        break;
    }
    default:
        break;
    }

    return ShapeContext::onCreateContext( aElementToken, rAttribs );
}

ChartGraphicDataContext::ChartGraphicDataContext( ContextHandler2Helper& rParent, const ShapePtr& rxShape, bool bEmbedShapes ) :
    ShapeContext( rParent, ShapePtr(), rxShape ),
    mrChartShapeInfo( rxShape->setChartType( bEmbedShapes ) )
{
}

ContextHandlerRef ChartGraphicDataContext::onCreateContext( ::sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( nElement == C_TOKEN( chart ) )
    {
        mrChartShapeInfo.maFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    }
    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
