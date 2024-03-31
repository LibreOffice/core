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
using namespace ::com::sun::star::uno;

namespace oox::drawingml {

namespace {

// CT_CxnList
class CxnListContext
    : public ContextHandler2
{
public:
    CxnListContext( ContextHandler2Helper const & rParent,
                    svx::diagram::Connections & aConnections )
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
                    svx::diagram::Connection& rConnection=mrConnection.back();

                    rConnection.mnXMLType = static_cast<svx::diagram::TypeConstant>(rAttribs.getToken( XML_type, XML_parOf ));
                    rConnection.msModelId = rAttribs.getStringDefaulted( XML_modelId );
                    rConnection.msSourceId = rAttribs.getStringDefaulted( XML_srcId );
                    rConnection.msDestId  = rAttribs.getStringDefaulted( XML_destId );
                    rConnection.msPresId  = rAttribs.getStringDefaulted( XML_presId );
                    rConnection.msSibTransId  = rAttribs.getStringDefaulted( XML_sibTransId );
                    rConnection.msParTransId  = rAttribs.getStringDefaulted( XML_parTransId );
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
    svx::diagram::Connections& mrConnection;
};

// CT_presLayoutVars
class PresLayoutVarsContext
    : public ContextHandler2
{
public:
    PresLayoutVarsContext( ContextHandler2Helper const & rParent,
                           svx::diagram::Point & rPoint ) :
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
            {
                // need to convert from oox::OptValue to std::optional since 1st is not available in svx
                const std::optional< sal_Int32 > aOptVal(rAttribs.getToken( XML_val ));
                if(aOptVal.has_value())
                    mrPoint.moHierarchyBranch = aOptVal.value();
                break;
            }
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
    svx::diagram::Point& mrPoint;
};

// CT_prSet
class PropertiesContext
    : public ContextHandler2
{
public:
    PropertiesContext( ContextHandler2Helper const & rParent,
                       svx::diagram::Point & rPoint,
                       const AttributeList& rAttribs ) :
        ContextHandler2( rParent ),
        mrPoint( rPoint )
    {
        mrPoint.msColorTransformCategoryId = rAttribs.getStringDefaulted( XML_csCatId);
        mrPoint.msColorTransformTypeId = rAttribs.getStringDefaulted( XML_csTypeId);
        mrPoint.msLayoutCategoryId = rAttribs.getStringDefaulted( XML_loCatId);
        mrPoint.msLayoutTypeId = rAttribs.getStringDefaulted( XML_loTypeId);
        mrPoint.msPlaceholderText = rAttribs.getStringDefaulted( XML_phldrT);
        mrPoint.msPresentationAssociationId = rAttribs.getStringDefaulted( XML_presAssocID);
        mrPoint.msPresentationLayoutName = rAttribs.getStringDefaulted( XML_presName);
        mrPoint.msPresentationLayoutStyleLabel = rAttribs.getStringDefaulted( XML_presStyleLbl);
        mrPoint.msQuickStyleCategoryId = rAttribs.getStringDefaulted( XML_qsCatId);
        mrPoint.msQuickStyleTypeId = rAttribs.getStringDefaulted( XML_qsTypeId);

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
    svx::diagram::Point& mrPoint;
};

// CL_Pt
class PtContext
    : public ContextHandler2
{
public:
    PtContext( ContextHandler2Helper const& rParent,
               const AttributeList& rAttribs,
               svx::diagram::Point& rPoint,
               DiagramData& rDiagramData):
        ContextHandler2( rParent ),
        mrPoint( rPoint ),
        mrDiagramData( rDiagramData )
    {
        mrPoint.msModelId = rAttribs.getStringDefaulted( XML_modelId );

        // the default type is XML_node
        const sal_Int32 nType  = rAttribs.getToken( XML_type, XML_node );
        mrPoint.mnXMLType = static_cast<svx::diagram::TypeConstant>(nType);

        // ignore the cxnId unless it is this type. See 5.15.3.1.3 in Primer
        if( ( nType == XML_parTrans ) || ( nType == XML_sibTrans ) )
            mrPoint.msCnxId = rAttribs.getStringDefaulted( XML_cxnId );
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
            {
                Shape* pShape(mrDiagramData.getOrCreateAssociatedShape(mrPoint, true));
                return new ShapePropertiesContext( *this, *pShape );
            }
            case DGM_TOKEN( t ):
            {
                Shape* pShape(mrDiagramData.getOrCreateAssociatedShape(mrPoint, true));
                TextBodyPtr xTextBody = std::make_shared<TextBody>();
                pShape->setTextBody( xTextBody );
                return new TextBodyContext( *this, *xTextBody );
            }
            default:
                break;
            }
            return this;
        }

private:
    svx::diagram::Point& mrPoint;
    DiagramData& mrDiagramData;
};

// CT_PtList
class PtListContext
    : public ContextHandler2
{
public:
    PtListContext( ContextHandler2Helper const & rParent,  svx::diagram::Points& rPoints, DiagramData& rDiagramData) :
        ContextHandler2( rParent ),
        mrPoints( rPoints ),
        mrDiagramData( rDiagramData )
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
                return new PtContext( *this, rAttribs, mrPoints.back(), mrDiagramData );
            }
            default:
                break;
            }
            return this;
        }

private:
    svx::diagram::Points& mrPoints;
    DiagramData& mrDiagramData;
};

// CT_BackgroundFormatting
class BackgroundFormattingContext
    : public ContextHandler2
{
public:
    BackgroundFormattingContext( ContextHandler2Helper const & rParent, OoxDiagramDataPtr const& pModel )
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
                return FillPropertiesContext::createFillContext(*this, aElementToken, rAttribs, *mpDataModel->getBackgroundShapeFillProperties(), nullptr);
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
    OoxDiagramDataPtr mpDataModel;
};

}

DataModelContext::DataModelContext( ContextHandler2Helper const& rParent,
                                    const OoxDiagramDataPtr& pDataModel )
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
        return new PtListContext( *this, mpDataModel->getPoints(), *mpDataModel );
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
        mpDataModel->getExtDrawings().push_back( rAttribs.getStringDefaulted( XML_relId ) );
        break;
    default:
        break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
