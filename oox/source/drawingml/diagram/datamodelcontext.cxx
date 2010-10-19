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

#include "datamodelcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace oox { namespace drawingml {



// CT_CxnList
class CxnListContext
    : public ContextHandler
{
public:
    CxnListContext( ContextHandler& rParent,
                    dgm::Connections & aConnections )
        : ContextHandler( rParent )
        , mrConnections( aConnections )
        {
        }
    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case NMSP_DIAGRAM|XML_cxn:
            {
                mrConnections.push_back( dgm::Connection() );
                dgm::Connection& rConnection=mrConnections.back();

                const sal_Int32 nType = xAttribs->getOptionalValueToken( XML_type, XML_parOf );
                rConnection.mnType = nType;
                rConnection.msModelId = xAttribs->getOptionalValue( XML_modelId );
                rConnection.msSourceId = xAttribs->getOptionalValue( XML_srcId );
                rConnection.msDestId  = xAttribs->getOptionalValue( XML_destId );
                rConnection.msPresId  = xAttribs->getOptionalValue( XML_presId );
                rConnection.msSibTransId  = xAttribs->getOptionalValue( XML_sibTransId );
                rConnection.msParTransId  = xAttribs->getOptionalValue( XML_parTransId );
                const AttributeList attribs( xAttribs );
                rConnection.mnSourceOrder = attribs.getInteger( XML_srcOrd, 0 );
                rConnection.mnDestOrder = attribs.getInteger( XML_destOrd, 0 );

                // skip CT_extLst
                return xRet;
            }
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::Connections& mrConnections;
};


// CT_presLayoutVars
class PresLayoutVarsContext
    : public ContextHandler
{
public:
    PresLayoutVarsContext( ContextHandler& rParent,
                           dgm::Point & rPoint ) :
        ContextHandler( rParent ),
        mrPoint( rPoint )
    {
    }
    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;
            AttributeList aAttribs( xAttribs );

            switch( aElementToken )
            {
                // TODO
            case NMSP_DIAGRAM|XML_animLvl:
            case NMSP_DIAGRAM|XML_animOne:
                break;
            case NMSP_DIAGRAM|XML_bulletEnabled:
                mrPoint.mbBulletEnabled = aAttribs.getBool( XML_val, false );
                break;
            case NMSP_DIAGRAM|XML_chMax:
                mrPoint.mnMaxChildren = aAttribs.getInteger( XML_val, -1 );
                break;
            case NMSP_DIAGRAM|XML_chPref:
                mrPoint.mnPreferredChildren = aAttribs.getInteger( XML_val, -1 );
                break;
            case NMSP_DIAGRAM|XML_dir:
                mrPoint.mnDirection = aAttribs.getToken( XML_val, XML_norm );
                break;
            case NMSP_DIAGRAM|XML_hierBranch:
                mrPoint.mnHierarchyBranch = aAttribs.getToken( XML_val, XML_std );
                break;
            case NMSP_DIAGRAM|XML_orgChart:
                mrPoint.mbOrgChartEnabled = aAttribs.getBool( XML_val, false );
                break;
            case NMSP_DIAGRAM|XML_resizeHandles:
                mrPoint.mnResizeHandles = aAttribs.getToken( XML_val, XML_rel );
                break;
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::Point& mrPoint;
};


// CT_prSet
class PropertiesContext
    : public ContextHandler
{
public:
    PropertiesContext( ContextHandler& rParent,
                       dgm::Point & rPoint,
                       const Reference< XFastAttributeList >& xAttribs ) :
        ContextHandler( rParent ),
        mrPoint( rPoint )
    {
        OUString aEmptyStr;
        AttributeList aAttribs( xAttribs );

        mrPoint.msColorTransformCategoryId = aAttribs.getString( XML_csCatId, aEmptyStr );
        mrPoint.msColorTransformTypeId = aAttribs.getString( XML_csTypeId, aEmptyStr );
        mrPoint.msLayoutCategoryId = aAttribs.getString( XML_loCatId, aEmptyStr );
        mrPoint.msLayoutTypeId = aAttribs.getString( XML_loTypeId, aEmptyStr );
        mrPoint.msPlaceholderText = aAttribs.getString( XML_phldrT, aEmptyStr );
        mrPoint.msPresentationAssociationId = aAttribs.getString( XML_presAssocID, aEmptyStr );
        mrPoint.msPresentationLayoutName = aAttribs.getString( XML_presName, aEmptyStr );
        mrPoint.msPresentationLayoutStyleLabel = aAttribs.getString( XML_presStyleLbl, aEmptyStr );
        mrPoint.msQuickStyleCategoryId = aAttribs.getString( XML_qsCatId, aEmptyStr );
        mrPoint.msQuickStyleTypeId = aAttribs.getString( XML_qsTypeId, aEmptyStr );

        mrPoint.mnCustomAngle = aAttribs.getInteger( XML_custAng, -1 );
        mrPoint.mnPercentageNeighbourWidth = aAttribs.getInteger( XML_custLinFactNeighborX, -1 );
        mrPoint.mnPercentageNeighbourHeight = aAttribs.getInteger( XML_custLinFactNeighborY, -1 );
        mrPoint.mnPercentageOwnWidth = aAttribs.getInteger( XML_custLinFactX, -1 );
        mrPoint.mnPercentageOwnHeight = aAttribs.getInteger( XML_custLinFactY, -1 );
        mrPoint.mnIncludeAngleScale = aAttribs.getInteger( XML_custRadScaleInc, -1 );
        mrPoint.mnRadiusScale = aAttribs.getInteger( XML_custRadScaleRad, -1 );
        mrPoint.mnWidthScale = aAttribs.getInteger( XML_custScaleX, -1 );
        mrPoint.mnHeightScale = aAttribs.getInteger( XML_custScaleY, -1 );
        mrPoint.mnWidthOverride = aAttribs.getInteger( XML_custSzX, -1 );
        mrPoint.mnHeightOverride = aAttribs.getInteger( XML_custSzY, -1 );
        mrPoint.mnLayoutStyleCount = aAttribs.getInteger( XML_presStyleCnt, -1 );
        mrPoint.mnLayoutStyleIndex = aAttribs.getInteger( XML_presStyleIdx, -1 );

        mrPoint.mbCoherent3DOffset = aAttribs.getBool( XML_coherent3DOff, false );
        mrPoint.mbCustomHorizontalFlip = aAttribs.getBool( XML_custFlipHor, false );
        mrPoint.mbCustomVerticalFlip = aAttribs.getBool( XML_custFlipVert, false );
        mrPoint.mbCustomText = aAttribs.getBool( XML_custT, false );
        mrPoint.mbIsPlaceholder = aAttribs.getBool( XML_phldr, false );
    }
    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case NMSP_DIAGRAM|XML_presLayoutVars:
            {
                xRet.set( new PresLayoutVarsContext( *this, mrPoint ) );
                break;
            }
            case NMSP_DIAGRAM|XML_style:
            {
                // TODO
                // skip CT_shapeStyle
                return xRet;
            }
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::Point& mrPoint;
};


// CL_Pt
class PtContext
    : public ContextHandler
{
public:
    PtContext( ContextHandler& rParent,
               const Reference< XFastAttributeList >& xAttribs,
               dgm::Point & rPoint):
        ContextHandler( rParent ),
        mrPoint( rPoint )
    {
        mrPoint.msModelId = xAttribs->getOptionalValue( XML_modelId );

        // the default type is XML_node
        const sal_Int32 nType  = xAttribs->getOptionalValueToken( XML_type, XML_node );
        mrPoint.mnType = nType;

        // ignore the cxnId unless it is this type. See 5.15.3.1.3 in Primer
        if( ( nType == XML_parTrans ) || ( nType == XML_sibTrans ) )
            mrPoint.msCnxId = xAttribs->getOptionalValue( XML_cxnId );
    }


    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case NMSP_DIAGRAM|XML_extLst:
                return xRet;
            case NMSP_DIAGRAM|XML_prSet:
                OSL_TRACE( "diagram property set for point");
                xRet = new PropertiesContext( *this, mrPoint, xAttribs );
                break;
            case NMSP_DIAGRAM|XML_spPr:
                OSL_TRACE( "shape props for point");
                if( !mrPoint.mpShape )
                    mrPoint.mpShape.reset( new Shape() );
                xRet = new ShapePropertiesContext( *this, *(mrPoint.mpShape) );
                break;
            case NMSP_DIAGRAM|XML_t:
            {
                OSL_TRACE( "shape text body for point");
                TextBodyPtr xTextBody( new TextBody );
                if( !mrPoint.mpShape )
                    mrPoint.mpShape.reset( new Shape() );
                mrPoint.mpShape->setTextBody( xTextBody );
                xRet = new TextBodyContext( *this, *xTextBody );
                break;
            }
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::Point& mrPoint;
};



// CT_PtList
class PtListContext
    : public ContextHandler
{
public:
    PtListContext( ContextHandler& rParent,  dgm::Points& rPoints) :
        ContextHandler( rParent ),
        mrPoints( rPoints )
    {}
    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case NMSP_DIAGRAM|XML_pt:
            {
                // CT_Pt
                mrPoints.push_back( dgm::Point() );
                xRet.set( new PtContext( *this, xAttribs, mrPoints.back() ) );
                break;
            }
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::Points& mrPoints;
};

// CT_BackgroundFormatting
class BackgroundFormattingContext
    : public ContextHandler
{
public:
    BackgroundFormattingContext( ContextHandler& rParent, DiagramDataPtr & pModel )
        : ContextHandler( rParent )
        , mpDataModel( pModel )
        {
            OSL_ENSURE( pModel, "the data model MUST NOT be NULL" );
        }

    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case NMSP_DRAWINGML|XML_blipFill:
            case NMSP_DRAWINGML|XML_gradFill:
            case NMSP_DRAWINGML|XML_grpFill:
            case NMSP_DRAWINGML|XML_noFill:
            case NMSP_DRAWINGML|XML_pattFill:
            case NMSP_DRAWINGML|XML_solidFill:
                // EG_FillProperties
                xRet.set( FillPropertiesContext::createFillContext(
                    *this, aElementToken, xAttribs, *mpDataModel->getFillProperties() ) );
                break;
            case NMSP_DRAWINGML|XML_effectDag:
            case NMSP_DRAWINGML|XML_effectLst:
                // TODO
                // EG_EffectProperties
                break;
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }
private:
    DiagramDataPtr mpDataModel;
};



DataModelContext::DataModelContext( ContextHandler& rParent,
                                    const DiagramDataPtr & pDataModel )
    : ContextHandler( rParent )
    , mpDataModel( pDataModel )
{
    OSL_ENSURE( pDataModel, "Data Model must not be NULL" );
}


DataModelContext::~DataModelContext()
{
    // some debug
    mpDataModel->dump();
}


Reference< XFastContextHandler > SAL_CALL
DataModelContext::createFastChildContext( ::sal_Int32 aElement,
                                          const Reference< XFastAttributeList >& /*xAttribs*/ )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case NMSP_DIAGRAM|XML_cxnLst:
        // CT_CxnList
        xRet.set( new CxnListContext( *this, mpDataModel->getConnections() ) );
        break;
    case NMSP_DIAGRAM|XML_ptLst:
        // CT_PtList
        xRet.set( new PtListContext( *this, mpDataModel->getPoints() ) );
        break;
    case NMSP_DIAGRAM|XML_bg:
        // CT_BackgroundFormatting
        xRet.set( new BackgroundFormattingContext( *this, mpDataModel ) );
        break;
    case NMSP_DIAGRAM|XML_whole:
        // CT_WholeE2oFormatting
        // TODO
        return xRet;
    case NMSP_DIAGRAM|XML_extLst:
        return xRet;
    default:
        break;
    }

    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
