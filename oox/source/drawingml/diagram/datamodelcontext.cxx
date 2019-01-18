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

#include "datamodelcontext.hxx"
#include <oox/helper/attributelist.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/textbodycontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

namespace oox { namespace drawingml {

// CT_CxnList
class CxnListContext
    : public ContextHandler2
{
public:
    CxnListContext( ContextHandler2Helper const & rParent,
                    dgm::Connections & aConnections )
        : ContextHandler2( rParent )
        , mrConnection( aConnections )
        {
        }

    virtual ContextHandlerRef
    onCreateContext( sal_Int32 aElementToken,
                     const AttributeList& rAttribs ) override
        {
            switch( aElementToken )
            {
                case DGM_TOKEN( cxn ):
                {
                    mrConnection.emplace_back( );
                    dgm::Connection& rConnection=mrConnection.back();

                    rConnection.mnType = rAttribs.getToken( XML_type, XML_parOf );
                    rConnection.msModelId = rAttribs.getString( XML_modelId ).get();
                    rConnection.msSourceId = rAttribs.getString( XML_srcId ).get();
                    rConnection.msDestId  = rAttribs.getString( XML_destId ).get();
                    rConnection.msPresId  = rAttribs.getString( XML_presId ).get();
                    rConnection.msSibTransId  = rAttribs.getString( XML_sibTransId ).get();
                    rConnection.msParTransId  = rAttribs.getString( XML_parTransId ).get();
                    rConnection.mnSourceOrder = rAttribs.getInteger( XML_srcOrd, 0 );
                    rConnection.mnDestOrder = rAttribs.getInteger( XML_destOrd, 0 );

                    // skip CT_extLst
                    return nullptr;
                }
                default:
                    break;
            }

            return this;
        }
private:
    dgm::Connections& mrConnection;
};

// CT_presLayoutVars
class PresLayoutVarsContext
    : public ContextHandler2
{
public:
    PresLayoutVarsContext( ContextHandler2Helper const & rParent,
                           dgm::Point & rPoint ) :
        ContextHandler2( rParent ),
        mrPoint( rPoint )
    {
    }
    virtual ContextHandlerRef
    onCreateContext( sal_Int32 aElementToken,
                     const AttributeList& rAttribs ) override
    {
        switch( aElementToken )
        {
            // TODO
            case DGM_TOKEN( animLvl ):
            case DGM_TOKEN( animOne ):
                break;
            case DGM_TOKEN( bulletEnabled ):
                mrPoint.mbBulletEnabled = rAttribs.getBool( XML_val, false );
                break;
            case DGM_TOKEN( chMax ):
                mrPoint.mnMaxChildren = rAttribs.getInteger( XML_val, -1 );
                break;
            case DGM_TOKEN( chPref ):
                mrPoint.mnPreferredChildren = rAttribs.getInteger( XML_val, -1 );
                break;
            case DGM_TOKEN( dir ):
                mrPoint.mnDirection = rAttribs.getToken( XML_val, XML_norm );
                break;
            case DGM_TOKEN( hierBranch ):
                mrPoint.moHierarchyBranch = rAttribs.getToken( XML_val );
                break;
            case DGM_TOKEN( orgChart ):
                mrPoint.mbOrgChartEnabled = rAttribs.getBool( XML_val, false );
                break;
            case DGM_TOKEN( resizeHandles ):
                mrPoint.mnResizeHandles = rAttribs.getToken( XML_val, XML_rel );
                break;
            default:
                break;
        }

        return this;
    }

private:
    dgm::Point& mrPoint;
};

// CT_prSet
class PropertiesContext
    : public ContextHandler2
{
public:
    PropertiesContext( ContextHandler2Helper const & rParent,
                       dgm::Point & rPoint,
                       const AttributeList& rAttribs ) :
        ContextHandler2( rParent ),
        mrPoint( rPoint )
    {
        mrPoint.msColorTransformCategoryId = rAttribs.getString( XML_csCatId, "" );
        mrPoint.msColorTransformTypeId = rAttribs.getString( XML_csTypeId, "" );
        mrPoint.msLayoutCategoryId = rAttribs.getString( XML_loCatId, "" );
        mrPoint.msLayoutTypeId = rAttribs.getString( XML_loTypeId, "" );
        mrPoint.msPlaceholderText = rAttribs.getString( XML_phldrT, "" );
        mrPoint.msPresentationAssociationId = rAttribs.getString( XML_presAssocID, "" );
        mrPoint.msPresentationLayoutName = rAttribs.getString( XML_presName, "" );
        mrPoint.msPresentationLayoutStyleLabel = rAttribs.getString( XML_presStyleLbl, "" );
        mrPoint.msQuickStyleCategoryId = rAttribs.getString( XML_qsCatId, "" );
        mrPoint.msQuickStyleTypeId = rAttribs.getString( XML_qsTypeId, "" );

        mrPoint.mnCustomAngle = rAttribs.getInteger( XML_custAng, -1 );
        mrPoint.mnPercentageNeighbourWidth = rAttribs.getInteger( XML_custLinFactNeighborX, -1 );
        mrPoint.mnPercentageNeighbourHeight = rAttribs.getInteger( XML_custLinFactNeighborY, -1 );
        mrPoint.mnPercentageOwnWidth = rAttribs.getInteger( XML_custLinFactX, -1 );
        mrPoint.mnPercentageOwnHeight = rAttribs.getInteger( XML_custLinFactY, -1 );
        mrPoint.mnIncludeAngleScale = rAttribs.getInteger( XML_custRadScaleInc, -1 );
        mrPoint.mnRadiusScale = rAttribs.getInteger( XML_custRadScaleRad, -1 );
        mrPoint.mnWidthScale = rAttribs.getInteger( XML_custScaleX, -1 );
        mrPoint.mnHeightScale = rAttribs.getInteger( XML_custScaleY, -1 );
        mrPoint.mnWidthOverride = rAttribs.getInteger( XML_custSzX, -1 );
        mrPoint.mnHeightOverride = rAttribs.getInteger( XML_custSzY, -1 );
        mrPoint.mnLayoutStyleCount = rAttribs.getInteger( XML_presStyleCnt, -1 );
        mrPoint.mnLayoutStyleIndex = rAttribs.getInteger( XML_presStyleIdx, -1 );

        mrPoint.mbCoherent3DOffset = rAttribs.getBool( XML_coherent3DOff, false );
        mrPoint.mbCustomHorizontalFlip = rAttribs.getBool( XML_custFlipHor, false );
        mrPoint.mbCustomVerticalFlip = rAttribs.getBool( XML_custFlipVert, false );
        mrPoint.mbCustomText = rAttribs.getBool( XML_custT, false );
        mrPoint.mbIsPlaceholder = rAttribs.getBool( XML_phldr, false );
    }

    virtual ContextHandlerRef
    onCreateContext( sal_Int32 aElementToken,
                     const AttributeList& ) override
        {
            switch( aElementToken )
            {
            case DGM_TOKEN( presLayoutVars ):
                return new PresLayoutVarsContext( *this, mrPoint );
            case DGM_TOKEN( style ):
                // skip CT_shapeStyle
                return nullptr;
            default:
                break;
            }
            return this;
        }

private:
    dgm::Point& mrPoint;
};

// CL_Pt
class PtContext
    : public ContextHandler2
{
public:
    PtContext( ContextHandler2Helper const & rParent,
               const AttributeList& rAttribs,
               dgm::Point & rPoint):
        ContextHandler2( rParent ),
        mrPoint( rPoint )
    {
        mrPoint.msModelId = rAttribs.getString( XML_modelId ).get();

        // the default type is XML_node
        const sal_Int32 nType  = rAttribs.getToken( XML_type, XML_node );
        mrPoint.mnType = nType;

        // ignore the cxnId unless it is this type. See 5.15.3.1.3 in Primer
        if( ( nType == XML_parTrans ) || ( nType == XML_sibTrans ) )
            mrPoint.msCnxId = rAttribs.getString( XML_cxnId ).get();
    }

    virtual ContextHandlerRef
    onCreateContext( sal_Int32 aElementToken,
                     const AttributeList& rAttribs ) override
        {
            switch( aElementToken )
            {
            case DGM_TOKEN( extLst ):
                return nullptr;
            case DGM_TOKEN( prSet ):
                return new PropertiesContext( *this, mrPoint, rAttribs );
            case DGM_TOKEN( spPr ):
                if( !mrPoint.mpShape )
                    mrPoint.mpShape.reset( new Shape() );
                return new ShapePropertiesContext( *this, *(mrPoint.mpShape) );
            case DGM_TOKEN( t ):
            {
                TextBodyPtr xTextBody( new TextBody );
                if( !mrPoint.mpShape )
                    mrPoint.mpShape.reset( new Shape() );
                mrPoint.mpShape->setTextBody( xTextBody );
                return new TextBodyContext( *this, *xTextBody );
            }
            default:
                break;
            }
            return this;
        }

private:
    dgm::Point& mrPoint;
};

// CT_PtList
class PtListContext
    : public ContextHandler2
{
public:
    PtListContext( ContextHandler2Helper const & rParent,  dgm::Points& rPoints) :
        ContextHandler2( rParent ),
        mrPoints( rPoints )
    {}
    virtual ContextHandlerRef
    onCreateContext( sal_Int32 aElementToken,
                     const AttributeList& rAttribs ) override
        {
            switch( aElementToken )
            {
            case DGM_TOKEN( pt ):
            {
                // CT_Pt
                mrPoints.emplace_back( );
                return new PtContext( *this, rAttribs, mrPoints.back() );
            }
            default:
                break;
            }
            return this;
        }

private:
    dgm::Points& mrPoints;
};

// CT_BackgroundFormatting
class BackgroundFormattingContext
    : public ContextHandler2
{
public:
    BackgroundFormattingContext( ContextHandler2Helper const & rParent, DiagramDataPtr const & pModel )
        : ContextHandler2( rParent )
        , mpDataModel( pModel )
        {
            assert( pModel && "the data model MUST NOT be NULL" );
        }

    virtual ContextHandlerRef
    onCreateContext( sal_Int32 aElementToken,
                     const AttributeList& rAttribs ) override
        {
            switch( aElementToken )
            {
            case A_TOKEN( blipFill ):
            case A_TOKEN( gradFill ):
            case A_TOKEN( grpFill ):
            case A_TOKEN( noFill ):
            case A_TOKEN( pattFill ):
            case A_TOKEN( solidFill ):
                // EG_FillProperties
                return FillPropertiesContext::createFillContext(
                    *this, aElementToken, rAttribs, *mpDataModel->getFillProperties() );
            case A_TOKEN( effectDag ):
            case A_TOKEN( effectLst ):
                // TODO
                // EG_EffectProperties
                break;
            default:
                break;
            }
            return this;
        }
private:
    DiagramDataPtr mpDataModel;
};

DataModelContext::DataModelContext( ContextHandler2Helper const & rParent,
                                    const DiagramDataPtr & pDataModel )
    : ContextHandler2( rParent )
    , mpDataModel( pDataModel )
{
    assert( pDataModel && "Data Model must not be NULL" );
}

DataModelContext::~DataModelContext()
{
    // some debug
    mpDataModel->dump();
}

ContextHandlerRef
DataModelContext::onCreateContext( ::sal_Int32 aElement,
                                   const AttributeList& rAttribs )
{
    switch( aElement )
    {
    case DGM_TOKEN( cxnLst ):
        // CT_CxnList
        return new CxnListContext( *this, mpDataModel->getConnections() );
    case DGM_TOKEN( ptLst ):
        // CT_PtList
        return new PtListContext( *this, mpDataModel->getPoints() );
    case DGM_TOKEN( bg ):
        // CT_BackgroundFormatting
        return new BackgroundFormattingContext( *this, mpDataModel );
    case DGM_TOKEN( whole ):
        // CT_WholeE2oFormatting
        // TODO
        return nullptr;
    case DGM_TOKEN( extLst ):
    case A_TOKEN( ext ):
        break;
    case DSP_TOKEN( dataModelExt ):
        mpDataModel->getExtDrawings().push_back( rAttribs.getString( XML_relId ).get() );
        break;
    default:
        break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
