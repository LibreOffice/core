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


#include <string.h>
#include "dxfentrd.hxx"

//--------------------------DXFBasicEntity--------------------------------------

DXFBasicEntity::DXFBasicEntity(DXFEntityType eThisType)
    : m_sLayer("0")
    , m_sLineType("BYLAYER")
{
    eType=eThisType;
    pSucc=nullptr;
    fThickness=0;
    nColor=256;
    nSpace=0;
    aExtrusion.fx=0.0;
    aExtrusion.fy=0.0;
    aExtrusion.fz=1.0;
}

void DXFBasicEntity::Read(DXFGroupReader & rDGR)
{
    while (rDGR.Read()!=0) EvaluateGroup(rDGR);
}

void DXFBasicEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG())
    {
        case   8: m_sLayer = rDGR.GetS(); break;
        case   6: m_sLineType = rDGR.GetS(); break;
        case  39: fThickness=rDGR.GetF(); break;
        case  62: nColor=rDGR.GetI(); break;
        case  67: nSpace=rDGR.GetI(); break;
        case 210: aExtrusion.fx=rDGR.GetF(); break;
        case 220: aExtrusion.fy=rDGR.GetF(); break;
        case 230: aExtrusion.fz=rDGR.GetF(); break;
    }
}

DXFBasicEntity::~DXFBasicEntity()
{
}

//--------------------------DXFLineEntity---------------------------------------

DXFLineEntity::DXFLineEntity() : DXFBasicEntity(DXF_LINE)
{
}

void DXFLineEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 11: aP1.fx=rDGR.GetF(); break;
        case 21: aP1.fy=rDGR.GetF(); break;
        case 31: aP1.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFPointEntity--------------------------------------

DXFPointEntity::DXFPointEntity() : DXFBasicEntity(DXF_POINT)
{
}

void DXFPointEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFCircleEntity-------------------------------------

DXFCircleEntity::DXFCircleEntity() : DXFBasicEntity(DXF_CIRCLE)
{
    fRadius=1.0;
}

void DXFCircleEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 40: fRadius=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFArcEntity----------------------------------------

DXFArcEntity::DXFArcEntity() : DXFBasicEntity(DXF_ARC)
{
    fRadius=1.0;
    fStart=0;
    fEnd=360.0;
}

void DXFArcEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 40: fRadius=rDGR.GetF(); break;
        case 50: fStart=rDGR.GetF(); break;
        case 51: fEnd=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFTraceEntity--------------------------------------

DXFTraceEntity::DXFTraceEntity() : DXFBasicEntity(DXF_TRACE)
{
}

void DXFTraceEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 11: aP1.fx=rDGR.GetF(); break;
        case 21: aP1.fy=rDGR.GetF(); break;
        case 31: aP1.fz=rDGR.GetF(); break;
        case 12: aP2.fx=rDGR.GetF(); break;
        case 22: aP2.fy=rDGR.GetF(); break;
        case 32: aP2.fz=rDGR.GetF(); break;
        case 13: aP3.fx=rDGR.GetF(); break;
        case 23: aP3.fy=rDGR.GetF(); break;
        case 33: aP3.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFSolidEntity--------------------------------------

DXFSolidEntity::DXFSolidEntity() : DXFBasicEntity(DXF_SOLID)
{
}

void DXFSolidEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 11: aP1.fx=rDGR.GetF(); break;
        case 21: aP1.fy=rDGR.GetF(); break;
        case 31: aP1.fz=rDGR.GetF(); break;
        case 12: aP2.fx=rDGR.GetF(); break;
        case 22: aP2.fy=rDGR.GetF(); break;
        case 32: aP2.fz=rDGR.GetF(); break;
        case 13: aP3.fx=rDGR.GetF(); break;
        case 23: aP3.fy=rDGR.GetF(); break;
        case 33: aP3.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFTextEntity---------------------------------------

DXFTextEntity::DXFTextEntity()
    : DXFBasicEntity(DXF_TEXT)
    , m_sStyle("STANDARD")
{
    fHeight=1.0;
    fRotAngle=0.0;
    fXScale=1.0;
    fOblAngle=0.0;
    nGenFlags=0;
    nHorzJust=0;
    nVertJust=0;
}

void DXFTextEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 40: fHeight=rDGR.GetF(); break;
        case  1: m_sText = rDGR.GetS(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 42: fOblAngle=rDGR.GetF(); break;
        case  7: m_sStyle = rDGR.GetS(); break;
        case 71: nGenFlags=rDGR.GetI(); break;
        case 72: nHorzJust=rDGR.GetI(); break;
        case 73: nVertJust=rDGR.GetI(); break;
        case 11: aAlign.fx=rDGR.GetF(); break;
        case 21: aAlign.fy=rDGR.GetF(); break;
        case 31: aAlign.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFShapeEntity--------------------------------------

DXFShapeEntity::DXFShapeEntity() : DXFBasicEntity(DXF_SHAPE)
{
    fSize=1.0;
    fRotAngle=0;
    fXScale=1.0;
    fOblAngle=0;
}

void DXFShapeEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 40: fSize=rDGR.GetF(); break;
        case  2: m_sName = rDGR.GetS(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 51: fOblAngle=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFInsertEntity-------------------------------------

DXFInsertEntity::DXFInsertEntity() : DXFBasicEntity(DXF_INSERT)
{
    nAttrFlag=0;
    fXScale=1.0;
    fYScale=1.0;
    fZScale=1.0;
    fRotAngle=0.0;
    nColCount=1;
    nRowCount=1;
    fColSpace=0.0;
    fRowSpace=0.0;
}

void DXFInsertEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 66: nAttrFlag=rDGR.GetI(); break;
        case  2: m_sName = rDGR.GetS(); break;
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 42: fYScale=rDGR.GetF(); break;
        case 43: fZScale=rDGR.GetF(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 70: nColCount=rDGR.GetI(); break;
        case 71: nRowCount=rDGR.GetI(); break;
        case 44: fColSpace=rDGR.GetF(); break;
        case 45: fRowSpace=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFAttDefEntity-------------------------------------

DXFAttDefEntity::DXFAttDefEntity()
    : DXFBasicEntity(DXF_ATTDEF)
    , m_sStyle("STANDARD")
{
    fHeight=1.0;
    nAttrFlags=0;
    nFieldLen=0;
    fRotAngle=0.0;
    fXScale=1.0;
    fOblAngle=0.0;
    nGenFlags=0;
    nHorzJust=0;
    nVertJust=0;
}

void DXFAttDefEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 40: fHeight=rDGR.GetF(); break;
        case  1: m_sDefVal = rDGR.GetS(); break;
        case  3: m_sPrompt = rDGR.GetS(); break;
        case  2: m_sTagStr = rDGR.GetS(); break;
        case 70: nAttrFlags=rDGR.GetI(); break;
        case 73: nFieldLen=rDGR.GetI(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 51: fOblAngle=rDGR.GetF(); break;
        case  7: m_sStyle = rDGR.GetS(); break;
        case 71: nGenFlags=rDGR.GetI(); break;
        case 72: nHorzJust=rDGR.GetI(); break;
        case 74: nVertJust=rDGR.GetI(); break;
        case 11: aAlign.fx=rDGR.GetF(); break;
        case 21: aAlign.fy=rDGR.GetF(); break;
        case 31: aAlign.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFAttribEntity-------------------------------------

DXFAttribEntity::DXFAttribEntity()
    : DXFBasicEntity(DXF_ATTRIB)
    , m_sStyle("STANDARD")
{
    fHeight=1.0;
    nAttrFlags=0;
    nFieldLen=0;
    fRotAngle=0.0;
    fXScale=1.0;
    fOblAngle=0.0;
    nGenFlags=0;
    nHorzJust=0;
    nVertJust=0;
}

void DXFAttribEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 40: fHeight=rDGR.GetF(); break;
        case  1: m_sText = rDGR.GetS(); break;
        case  2: m_sTagStr = rDGR.GetS(); break;
        case 70: nAttrFlags=rDGR.GetI(); break;
        case 73: nFieldLen=rDGR.GetI(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 51: fOblAngle=rDGR.GetF(); break;
        case  7: m_sStyle = rDGR.GetS(); break;
        case 71: nGenFlags=rDGR.GetI(); break;
        case 72: nHorzJust=rDGR.GetI(); break;
        case 74: nVertJust=rDGR.GetI(); break;
        case 11: aAlign.fx=rDGR.GetF(); break;
        case 21: aAlign.fy=rDGR.GetF(); break;
        case 31: aAlign.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFPolyLine-----------------------------------------

DXFPolyLineEntity::DXFPolyLineEntity() : DXFBasicEntity(DXF_POLYLINE)
{
    nFlags=0;
    fSWidth=0.0;
    fEWidth=0.0;
    nMeshMCount=0;
    nMeshNCount=0;
    nMDensity=0;
    nNDensity=0;
    nCSSType=0;
}

void DXFPolyLineEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 70: nFlags=rDGR.GetI(); break;
        case 40: fSWidth=rDGR.GetF(); break;
        case 41: fEWidth=rDGR.GetF(); break;
        case 71: nMeshMCount=rDGR.GetI(); break;
        case 72: nMeshNCount=rDGR.GetI(); break;
        case 73: nMDensity=rDGR.GetI(); break;
        case 74: nNDensity=rDGR.GetI(); break;
        case 75: nCSSType=rDGR.GetI(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFLWPolyLine---------------------------------------

DXFLWPolyLineEntity::DXFLWPolyLineEntity() :
    DXFBasicEntity( DXF_LWPOLYLINE ),
    nIndex( 0 ),
    nCount( 0 ),
    nFlags( 0 ),
    fConstantWidth( 0.0 ),
    fStartWidth( 0.0 ),
    fEndWidth( 0.0 )
{
}

void DXFLWPolyLineEntity::EvaluateGroup( DXFGroupReader & rDGR )
{
    switch ( rDGR.GetG() )
    {
        case 90 :
        {
            nCount = rDGR.GetI();
            // limit alloc to max reasonable size based on remaining data in stream
            if (nCount > 0 && static_cast<sal_uInt32>(nCount) <= rDGR.remainingSize())
                aP.reserve(nCount);
            else
                nCount = 0;
        }
        break;
        case 70: nFlags = rDGR.GetI(); break;
        case 43: fConstantWidth = rDGR.GetF(); break;
        case 40: fStartWidth = rDGR.GetF(); break;
        case 41: fEndWidth = rDGR.GetF(); break;
        case 10:
        {
            if (nIndex < nCount)
            {
                aP.resize(nIndex+1);
                aP[nIndex].fx = rDGR.GetF();
            }
        }
        break;
        case 20:
        {
            if (nIndex < nCount)
            {
                aP.resize(nIndex+1);
                aP[nIndex].fy = rDGR.GetF();
                ++nIndex;
            }
        }
        break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFHatchEntity-------------------------------------

DXFEdgeTypeLine::DXFEdgeTypeLine() :
    DXFEdgeType( 1 )
{

}

bool DXFEdgeTypeLine::EvaluateGroup( DXFGroupReader & rDGR )
{
    bool bExecutingGroupCode = true;
    switch ( rDGR.GetG() )
    {
        case 10 : aStartPoint.fx = rDGR.GetF(); break;
        case 20 : aStartPoint.fy = rDGR.GetF(); break;
        case 11 : aEndPoint.fx = rDGR.GetF(); break;
        case 21 : aEndPoint.fy = rDGR.GetF(); break;
        default : bExecutingGroupCode = false; break;
    }
    return  bExecutingGroupCode;
}

DXFEdgeTypeCircularArc::DXFEdgeTypeCircularArc() :
    DXFEdgeType( 2 ),
    fRadius( 0.0 ),
    fStartAngle( 0.0 ),
    fEndAngle( 0.0 ),
    nIsCounterClockwiseFlag( 0 )
{
}

bool DXFEdgeTypeCircularArc::EvaluateGroup( DXFGroupReader & rDGR )
{
    bool bExecutingGroupCode = true;
    switch ( rDGR.GetG() )
    {
        case 10 : aCenter.fx = rDGR.GetF(); break;
        case 20 : aCenter.fy = rDGR.GetF(); break;
        case 40 : fRadius = rDGR.GetF(); break;
        case 50 : fStartAngle = rDGR.GetF(); break;
        case 51 : fEndAngle = rDGR.GetF(); break;
        case 73 : nIsCounterClockwiseFlag = rDGR.GetI(); break;
        default : bExecutingGroupCode = false; break;
    }
    return  bExecutingGroupCode;
}

DXFEdgeTypeEllipticalArc::DXFEdgeTypeEllipticalArc() :
    DXFEdgeType( 3 ),
    fLength( 0.0 ),
    fStartAngle( 0.0 ),
    fEndAngle( 0.0 ),
    nIsCounterClockwiseFlag( 0 )
{
}

bool DXFEdgeTypeEllipticalArc::EvaluateGroup( DXFGroupReader & rDGR )
{
    bool bExecutingGroupCode = true;
    switch( rDGR.GetG() )
    {
        case 10 : aCenter.fx = rDGR.GetF(); break;
        case 20 : aCenter.fy = rDGR.GetF(); break;
        case 11 : aEndPoint.fx = rDGR.GetF(); break;
        case 21 : aEndPoint.fy = rDGR.GetF(); break;
        case 40 : fLength = rDGR.GetF(); break;
        case 50 : fStartAngle = rDGR.GetF(); break;
        case 51 : fEndAngle = rDGR.GetF(); break;
        case 73 : nIsCounterClockwiseFlag = rDGR.GetI(); break;
        default : bExecutingGroupCode = false; break;
    }
    return  bExecutingGroupCode;
}

DXFEdgeTypeSpline::DXFEdgeTypeSpline() :
    DXFEdgeType( 4 ),
    nDegree( 0 ),
    nRational( 0 ),
    nPeriodic( 0 ),
    nKnotCount( 0 ),
    nControlCount( 0 )
{
}

bool DXFEdgeTypeSpline::EvaluateGroup( DXFGroupReader & rDGR )
{
    bool bExecutingGroupCode = true;
    switch ( rDGR.GetG() )
    {
        case 94 : nDegree = rDGR.GetI(); break;
        case 73 : nRational = rDGR.GetI(); break;
        case 74 : nPeriodic = rDGR.GetI(); break;
        case 95 : nKnotCount = rDGR.GetI(); break;
        case 96 : nControlCount = rDGR.GetI(); break;
        default : bExecutingGroupCode = false; break;
    }
    return  bExecutingGroupCode;
}

DXFBoundaryPathData::DXFBoundaryPathData() :
    nPointCount( 0 ),
    nFlags( 0 ),
    nHasBulgeFlag( 0 ),
    nIsClosedFlag( 0 ),
    fBulge( 0.0 ),
    nSourceBoundaryObjects( 0 ),
    nEdgeCount( 0 ),
    bIsPolyLine( true ),
    nPointIndex( 0 )
{
}

DXFBoundaryPathData::~DXFBoundaryPathData()
{
}

bool DXFBoundaryPathData::EvaluateGroup( DXFGroupReader & rDGR )
{
    bool bExecutingGroupCode = true;
    if ( bIsPolyLine )
    {
        switch( rDGR.GetG() )
        {
            case 92 :
            {
                nFlags = rDGR.GetI();
                if ( ( nFlags & 2 ) == 0 )
                    bIsPolyLine = false;
            }
            break;
            case 93 :
            {
                nPointCount = rDGR.GetI();
                // limit alloc to max reasonable size based on remaining data in stream
                if (nPointCount > 0 && static_cast<sal_uInt32>(nPointCount) <= rDGR.remainingSize())
                    aP.reserve(nPointCount);
                else
                    nPointCount = 0;
            }
            break;
            case 72 : nHasBulgeFlag = rDGR.GetI(); break;
            case 73 : nIsClosedFlag = rDGR.GetI(); break;
            case 97 : nSourceBoundaryObjects = rDGR.GetI(); break;
            case 42 : fBulge = rDGR.GetF(); break;
            case 10:
            {
                if (nPointIndex < nPointCount)
                {
                    aP.resize(nPointIndex+1);
                    aP[nPointIndex].fx = rDGR.GetF();
                }
            }
            break;
            case 20:
            {
                if (nPointIndex < nPointCount)
                {
                    aP.resize(nPointIndex+1);
                    aP[nPointIndex].fy = rDGR.GetF();
                    ++nPointIndex;
                }
            }
            break;

            default : bExecutingGroupCode = false; break;
        }
    }
    else
    {
        if ( rDGR.GetG() == 93 )
            nEdgeCount = rDGR.GetI();
        else if ( rDGR.GetG() == 72 )
        {
            sal_Int32 nEdgeType = rDGR.GetI();
            switch( nEdgeType )
            {
                case 1 : aEdges.emplace_back( new DXFEdgeTypeLine() ); break;
                case 2 : aEdges.emplace_back( new DXFEdgeTypeCircularArc() ); break;
                case 3 : aEdges.emplace_back( new DXFEdgeTypeEllipticalArc() ); break;
                case 4 : aEdges.emplace_back( new DXFEdgeTypeSpline() ); break;
            }
        }
        else if ( !aEdges.empty() )
            aEdges.back()->EvaluateGroup( rDGR );
        else
            bExecutingGroupCode = false;
    }
    return bExecutingGroupCode;
}

DXFHatchEntity::DXFHatchEntity() :
    DXFBasicEntity( DXF_HATCH ),
    bIsInBoundaryPathContext( false ),
    nCurrentBoundaryPathIndex( -1 ),
    nFlags( 0 ),
    nAssociativityFlag( 0 ),
    nBoundaryPathCount( 0 ),
    nHatchStyle( 0 ),
    nHatchPatternType( 0 ),
    fHatchPatternAngle( 0.0 ),
    fHatchPatternScale( 1.0 ),
    nHatchDoubleFlag( 0 ),
    nHatchPatternDefinitionLines( 0 ),
    fPixelSize( 1.0 ),
    nNumberOfSeedPoints( 0 )
{
}

void DXFHatchEntity::EvaluateGroup( DXFGroupReader & rDGR )
{
    switch ( rDGR.GetG() )
    {
//      case 10 : aElevationPoint.fx = rDGR.GetF(); break;
//      case 20 : aElevationPoint.fy = rDGR.GetF(); break;
//      case 30 : aElevationPoint.fz = rDGR.GetF(); break;
        case 70 : nFlags = rDGR.GetI(); break;
        case 71 : nAssociativityFlag = rDGR.GetI(); break;
        case 91 :
        {
            bIsInBoundaryPathContext = true;
            nBoundaryPathCount = rDGR.GetI();
            // limit alloc to max reasonable size based on remaining data in stream
            if (nBoundaryPathCount > 0 && static_cast<sal_uInt32>(nBoundaryPathCount) <= rDGR.remainingSize())
                pBoundaryPathData.reset( new DXFBoundaryPathData[ nBoundaryPathCount ] );
            else
                nBoundaryPathCount = 0;
        }
        break;
        case 75 :
        {
            nHatchStyle = rDGR.GetI();
            bIsInBoundaryPathContext = false;
        }
        break;
        case 76 : nHatchPatternType = rDGR.GetI(); break;
        case 52 : fHatchPatternAngle = rDGR.GetF(); break;
        case 41 : fHatchPatternScale = rDGR.GetF(); break;
        case 77 : nHatchDoubleFlag = rDGR.GetI(); break;
        case 78 : nHatchPatternDefinitionLines = rDGR.GetI(); break;
        case 47 : fPixelSize = rDGR.GetF(); break;
        case 98 : nNumberOfSeedPoints = rDGR.GetI(); break;

        case 92:
            nCurrentBoundaryPathIndex++;
            [[fallthrough]];
        default:
        {
            bool bExecutingGroupCode = false;
            if ( bIsInBoundaryPathContext )
            {
                if ( ( nCurrentBoundaryPathIndex >= 0 ) &&
                    ( nCurrentBoundaryPathIndex < nBoundaryPathCount ) )
                    bExecutingGroupCode = pBoundaryPathData[ nCurrentBoundaryPathIndex ].EvaluateGroup( rDGR );
            }
            if ( !bExecutingGroupCode )
                DXFBasicEntity::EvaluateGroup(rDGR);
        }
        break;
    }
}

//--------------------------DXFVertexEntity-------------------------------------

DXFVertexEntity::DXFVertexEntity() : DXFBasicEntity(DXF_VERTEX)
{
    fSWidth=-1.0;
    fEWidth=-1.0;
    fBulge=0.0;
    nFlags=0;
    fCFTDir=0.0;

}

void DXFVertexEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 40: fSWidth=rDGR.GetF(); break;
        case 41: fEWidth=rDGR.GetF(); break;
        case 42: fBulge=rDGR.GetF(); break;
        case 70: nFlags=rDGR.GetI(); break;
        case 50: fCFTDir=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFSeqEndEntity-------------------------------------

DXFSeqEndEntity::DXFSeqEndEntity() : DXFBasicEntity(DXF_SEQEND)
{
}

//--------------------------DXF3DFace-------------------------------------------

DXF3DFaceEntity::DXF3DFaceEntity() : DXFBasicEntity(DXF_3DFACE)
{
    nIEFlags=0;
}

void DXF3DFaceEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 11: aP1.fx=rDGR.GetF(); break;
        case 21: aP1.fy=rDGR.GetF(); break;
        case 31: aP1.fz=rDGR.GetF(); break;
        case 12: aP2.fx=rDGR.GetF(); break;
        case 22: aP2.fy=rDGR.GetF(); break;
        case 32: aP2.fz=rDGR.GetF(); break;
        case 13: aP3.fx=rDGR.GetF(); break;
        case 23: aP3.fy=rDGR.GetF(); break;
        case 33: aP3.fz=rDGR.GetF(); break;
        case 70: nIEFlags=rDGR.GetI(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}


//--------------------------DXFDimensionEntity----------------------------------

DXFDimensionEntity::DXFDimensionEntity() : DXFBasicEntity(DXF_DIMENSION)
{
}

void DXFDimensionEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case  2: m_sPseudoBlock = rDGR.GetS(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//---------------------------- DXFEntites --------------------------------------

void DXFEntities::Read(DXFGroupReader & rDGR)
{
    DXFBasicEntity * pE, * * ppSucc;

    ppSucc=&pFirst;
    while (*ppSucc!=nullptr) ppSucc=&((*ppSucc)->pSucc);

    while (rDGR.GetG()!=0) rDGR.Read();

    while (rDGR.GetS()!="ENDBLK" &&
           rDGR.GetS()!="ENDSEC" &&
           rDGR.GetS()!="EOF" )
    {

        if      (rDGR.GetS() == "LINE"      ) pE=new DXFLineEntity;
        else if (rDGR.GetS() == "POINT"     ) pE=new DXFPointEntity;
        else if (rDGR.GetS() == "CIRCLE"    ) pE=new DXFCircleEntity;
        else if (rDGR.GetS() == "ARC"       ) pE=new DXFArcEntity;
        else if (rDGR.GetS() == "TRACE"     ) pE=new DXFTraceEntity;
        else if (rDGR.GetS() == "SOLID"     ) pE=new DXFSolidEntity;
        else if (rDGR.GetS() == "TEXT"      ) pE=new DXFTextEntity;
        else if (rDGR.GetS() == "SHAPE"     ) pE=new DXFShapeEntity;
        else if (rDGR.GetS() == "INSERT"    ) pE=new DXFInsertEntity;
        else if (rDGR.GetS() == "ATTDEF"    ) pE=new DXFAttDefEntity;
        else if (rDGR.GetS() == "ATTRIB"    ) pE=new DXFAttribEntity;
        else if (rDGR.GetS() == "POLYLINE"  ) pE=new DXFPolyLineEntity;
        else if (rDGR.GetS() == "LWPOLYLINE") pE=new DXFLWPolyLineEntity;
        else if (rDGR.GetS() == "VERTEX"    ) pE=new DXFVertexEntity;
        else if (rDGR.GetS() == "SEQEND"    ) pE=new DXFSeqEndEntity;
        else if (rDGR.GetS() == "3DFACE"    ) pE=new DXF3DFaceEntity;
        else if (rDGR.GetS() == "DIMENSION" ) pE=new DXFDimensionEntity;
        else if (rDGR.GetS() == "HATCH"     ) pE=new DXFHatchEntity;
        else
        {
            do {
                rDGR.Read();
            } while (rDGR.GetG()!=0);
            continue;
        }
        *ppSucc=pE;
        ppSucc=&(pE->pSucc);
        pE->Read(rDGR);
    }
}

void DXFEntities::Clear()
{
    DXFBasicEntity * ptmp;

    while (pFirst!=nullptr) {
        ptmp=pFirst;
        pFirst=ptmp->pSucc;
        delete ptmp;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
