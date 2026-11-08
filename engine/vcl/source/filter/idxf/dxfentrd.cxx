/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <o3tl/safeint.hxx>

#include <algorithm>
#include <cmath>

#include "dxfentrd.hxx"

//--------------------------DXFBasicEntity--------------------------------------

DXFBasicEntity::DXFBasicEntity(DXFEntityType eThisType)
    : m_sLayer("0"_ostr)
    , m_sLineType("BYLAYER"_ostr)
{
    eType=eThisType;
    pSucc=nullptr;
    fThickness=0;
    nColor=256;
    nTrueColor=-1;
    bInvisible=false;
    fLineTypeScale=1.0;
    nLineWeight=DXF_LINEWEIGHT_BYLAYER;
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
        case 420: nTrueColor=rDGR.GetI() & 0xffffff; break;
        case  60: bInvisible=rDGR.GetI()!=0; break;
        case  48: fLineTypeScale=rDGR.GetF(); break;
        case 370: nLineWeight=rDGR.GetI(); break;
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
    , m_sStyle("STANDARD"_ostr)
{
    fHeight=1.0;
    fRotAngle=0.0;
    fXScale=1.0;
    fOblAngle=0.0;
    nGenFlags=0;
    nHorzJust=0;
    nVertJust=0;
    bHasAlign=false;
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
        case 51: fOblAngle=rDGR.GetF(); break;
        case  7: m_sStyle = rDGR.GetS(); break;
        case 71: nGenFlags=rDGR.GetI(); break;
        case 72: nHorzJust=rDGR.GetI(); break;
        case 73: nVertJust=rDGR.GetI(); break;
        case 11: aAlign.fx=rDGR.GetF(); bHasAlign=true; break;
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
    , m_sStyle("STANDARD"_ostr)
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
    bInEmbeddedObject=false;
}

void DXFAttDefEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    if (bInEmbeddedObject)
        return;
    switch (rDGR.GetG()) {
        case 101: bInEmbeddedObject=true; break;
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
    , m_sStyle("STANDARD"_ostr)
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
    bHasAlign=false;
    bInEmbeddedObject=false;
}

void DXFAttribEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    if (bInEmbeddedObject)
        return;
    switch (rDGR.GetG()) {
        case 101: bInEmbeddedObject=true; break;
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
        case 11: aAlign.fx=rDGR.GetF(); bHasAlign=true; break;
        case 21: aAlign.fy=rDGR.GetF(); break;
        case 31: aAlign.fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFPolyLine-----------------------------------------

DXFPolyLineEntity::DXFPolyLineEntity() : DXFBasicEntity(DXF_POLYLINE)
{
    fElevation=0.0;
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
        case 30: fElevation=rDGR.GetF(); break;
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
    nCount( 0 ),
    bVertexAccepted( false ),
    nFlags( 0 ),
    fConstantWidth( 0.0 ),
    fElevation( 0.0 )
{
}

void DXFLWPolyLineEntity::EvaluateGroup( DXFGroupReader & rDGR )
{
    switch ( rDGR.GetG() )
    {
        case 90 :
        {
            nCount = rDGR.GetI();
            // limit alloc to max reasonable size based on remaining data in stream, a vertex
            // takes at least ten bytes
            if (nCount > 0)
                aVertices.reserve(std::min<sal_uInt64>(nCount, rDGR.remainingSize() / 10));
            else
                nCount = 0;
        }
        break;
        case 70: nFlags = rDGR.GetI(); break;
        case 43: fConstantWidth = rDGR.GetF(); break;
        case 38: fElevation = rDGR.GetF(); break;
        case 10:
        {
            // a new vertex
            bVertexAccepted = aVertices.size() < o3tl::make_unsigned(nCount);
            if (bVertexAccepted)
            {
                DXFPathVertex aVertex;
                aVertex.maPoint.setX(rDGR.GetF());
                aVertex.mfStartWidth = -1.0;
                aVertex.mfEndWidth = -1.0;
                aVertices.push_back(aVertex);
            }
        }
        break;
        case 20:
            if (bVertexAccepted)
                aVertices.back().maPoint.setY(rDGR.GetF());
            break;
        case 40:
            if (bVertexAccepted)
                aVertices.back().mfStartWidth = rDGR.GetF();
            break;
        case 41:
            if (bVertexAccepted)
                aVertices.back().mfEndWidth = rDGR.GetF();
            break;
        case 42:
            if (bVertexAccepted)
                aVertices.back().mfBulge = rDGR.GetF();
            break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFHatchEntity-------------------------------------

namespace
{
// a group of a hatch, its value as a number
struct HatchGroup
{
    sal_uInt16 nCode;
    double fValue;
    tools::Long nValue;
};

// Reads the groups in their order. Reading an expected group that is not the next one does
// not advance and gives the default value.
class HatchReader
{
public:
    explicit HatchReader(const std::vector<HatchGroup>& rGroups)
        : mrGroups(rGroups)
        , mnPos(0)
    {
    }

    bool at(sal_uInt16 nCode) const { return mnPos < mrGroups.size() && mrGroups[mnPos].nCode == nCode; }
    bool atEnd() const { return mnPos >= mrGroups.size(); }
    sal_uInt16 code() const { return mrGroups[mnPos].nCode; }
    void skip() { ++mnPos; }

    double getF(sal_uInt16 nCode, double fDefault = 0.0)
    {
        if (!at(nCode))
            return fDefault;
        return mrGroups[mnPos++].fValue;
    }

    tools::Long getI(sal_uInt16 nCode, tools::Long nDefault = 0)
    {
        if (!at(nCode))
            return nDefault;
        return mrGroups[mnPos++].nValue;
    }

    // a count, at most the number of groups left
    size_t getCount(sal_uInt16 nCode)
    {
        const tools::Long nCount = getI(nCode);
        if (nCount <= 0)
            return 0;
        return std::min(o3tl::make_unsigned(nCount), mrGroups.size() - mnPos);
    }

    basegfx::B2DPoint getPoint(sal_uInt16 nCode)
    {
        const double fX = getF(nCode);
        return basegfx::B2DPoint(fX, getF(nCode + 10));
    }

private:
    const std::vector<HatchGroup>& mrGroups;
    size_t mnPos;
};

// the angle in radians of a counter-clockwise arc edge, or of a clockwise one whose angles
// are stored mirrored
double edgeAngle(double fDegrees, bool bCounterClockwise)
{
    return basegfx::deg2rad(bCounterClockwise ? fDegrees : -fDegrees);
}

// the sweep in radians from the start to the end angle, in degrees, in the edge direction
double edgeSweep(double fStart, double fEnd, bool bCounterClockwise)
{
    double fSweep = std::fmod(fEnd - fStart, 360.0);
    if (fSweep <= 0)
        fSweep += 360.0;
    return basegfx::deg2rad(bCounterClockwise ? fSweep : -fSweep);
}

// the parameter of the ellipse point in the direction fAngle
double ellipseParameter(double fAngle, double fRatio)
{
    if (fRatio == 0)
        return fAngle;
    double fParameter = std::atan2(std::sin(fAngle) / fRatio, std::cos(fAngle));
    // keep the number of turns
    fParameter += std::round((fAngle - fParameter) / (2 * M_PI)) * 2 * M_PI;
    return fParameter;
}

basegfx::B2DPolygon readEdge(HatchReader& rReader, tools::Long nType)
{
    basegfx::B2DPolygon aEdge;
    switch (nType)
    {
        case 1: // line
            aEdge.append(rReader.getPoint(10));
            aEdge.append(rReader.getPoint(11));
            break;
        case 2: // circular arc
        {
            const basegfx::B2DPoint aCenter = rReader.getPoint(10);
            const double fRadius = rReader.getF(40);
            const double fStart = rReader.getF(50);
            const double fEnd = rReader.getF(51);
            const bool bCounterClockwise = rReader.getI(73, 1) != 0;
            dxf::appendEllipseArc(aEdge, aCenter, basegfx::B2DVector(fRadius, 0),
                                  basegfx::B2DVector(0, fRadius),
                                  edgeAngle(fStart, bCounterClockwise),
                                  edgeSweep(fStart, fEnd, bCounterClockwise), true);
            break;
        }
        case 3: // elliptic arc
        {
            const basegfx::B2DPoint aCenter = rReader.getPoint(10);
            const basegfx::B2DPoint aMajorEnd = rReader.getPoint(11);
            const basegfx::B2DVector aMajor(aMajorEnd.getX(), aMajorEnd.getY());
            const double fRatio = rReader.getF(40, 1.0);
            const double fStart = rReader.getF(50);
            const double fEnd = rReader.getF(51);
            const bool bCounterClockwise = rReader.getI(73, 1) != 0;
            const basegfx::B2DVector aMinor(-aMajor.getY() * fRatio, aMajor.getX() * fRatio);
            const double fStartAngle = edgeAngle(fStart, bCounterClockwise);
            const double fEndAngle = fStartAngle + edgeSweep(fStart, fEnd, bCounterClockwise);
            const double fStartParameter = ellipseParameter(fStartAngle, fRatio);
            dxf::appendEllipseArc(aEdge, aCenter, aMajor, aMinor, fStartParameter,
                                  ellipseParameter(fEndAngle, fRatio) - fStartParameter, true);
            break;
        }
        case 4: // spline
        {
            const sal_Int32 nDegree = rReader.getI(94, 3);
            const bool bRational = rReader.getI(73) != 0;
            rReader.getI(74);
            const size_t nKnots = rReader.getCount(95);
            const size_t nControlPoints = rReader.getCount(96);
            std::vector<double> aKnots;
            for (size_t i = 0; i < nKnots && rReader.at(40); ++i)
                aKnots.push_back(rReader.getF(40));
            std::vector<basegfx::B2DPoint> aControlPoints;
            std::vector<double> aWeights;
            for (size_t i = 0; i < nControlPoints && rReader.at(10); ++i)
            {
                aControlPoints.push_back(rReader.getPoint(10));
                if (bRational || rReader.at(42))
                    aWeights.push_back(rReader.getF(42, 1.0));
            }
            std::vector<basegfx::B2DPoint> aFitPoints;
            const size_t nFitPoints = rReader.getCount(97);
            for (size_t i = 0; i < nFitPoints && rReader.at(11); ++i)
                aFitPoints.push_back(rReader.getPoint(11));
            // the tangents of the fit points
            rReader.getPoint(12);
            rReader.getPoint(13);
            aEdge = dxf::createSpline(nDegree, aKnots, aControlPoints, aWeights, aFitPoints);
            break;
        }
    }
    return aEdge;
}

// the points are the same but for the precision of the coordinates
bool isNear(const basegfx::B2DPoint& rA, const basegfx::B2DPoint& rB)
{
    const double fTolerance
        = 1e-6 * std::max({ 1.0, std::fabs(rA.getX()), std::fabs(rA.getY()) });
    return std::fabs(rA.getX() - rB.getX()) <= fTolerance
           && std::fabs(rA.getY() - rB.getY()) <= fTolerance;
}

// Appends an edge to a boundary loop, turning it around if it ends where the loop ends.
void appendEdge(basegfx::B2DPolygon& rLoop, basegfx::B2DPolygon aEdge)
{
    if (aEdge.count() == 0)
        return;
    if (rLoop.count() == 0)
    {
        rLoop = aEdge;
        return;
    }
    const basegfx::B2DPoint aLoopEnd = rLoop.getB2DPoint(rLoop.count() - 1);
    const basegfx::B2DPoint aEdgeStart = aEdge.getB2DPoint(0);
    const basegfx::B2DPoint aEdgeEnd = aEdge.getB2DPoint(aEdge.count() - 1);
    if (!isNear(aEdgeStart, aLoopEnd) && isNear(aEdgeEnd, aLoopEnd))
        aEdge.flip();
    // the joint point once
    if (isNear(aEdge.getB2DPoint(0), aLoopEnd))
    {
        if (aEdge.areControlPointsUsed() && aEdge.isNextControlPointUsed(0))
            rLoop.setNextControlPoint(rLoop.count() - 1, aEdge.getNextControlPoint(0));
        aEdge.remove(0);
    }
    rLoop.append(aEdge);
}
}

DXFHatchEntity::DXFHatchEntity() :
    DXFBasicEntity( DXF_HATCH ),
    fElevation( 0.0 ),
    bSolid( false ),
    bGradient( false ),
    fGradientAngle( 0.0 )
{
}

void DXFHatchEntity::Read(DXFGroupReader & rDGR)
{
    std::vector<HatchGroup> aGroups;
    while (rDGR.Read() != 0)
    {
        DXFBasicEntity::EvaluateGroup(rDGR);
        aGroups.push_back({ rDGR.GetG(), rDGR.GetF(), rDGR.GetI() });
    }

    HatchReader aReader(aGroups);
    // the head, up to the number of boundary paths
    size_t nPaths = 0;
    while (!aReader.atEnd())
    {
        const sal_uInt16 nCode = aReader.code();
        if (nCode == 30)
            fElevation = aReader.getF(30);
        else if (nCode == 70)
            bSolid = aReader.getI(70) != 0;
        else if (nCode == 91)
        {
            nPaths = aReader.getCount(91);
            break;
        }
        else
            aReader.skip();
    }

    for (size_t nPath = 0; nPath < nPaths; ++nPath)
    {
        while (!aReader.atEnd() && !aReader.at(92))
            aReader.skip();
        if (aReader.atEnd())
            break;
        const tools::Long nFlags = aReader.getI(92);
        basegfx::B2DPolygon aLoop;
        if (nFlags & 2)
        {
            // polyline with bulges
            const bool bHasBulges = aReader.getI(72) != 0;
            aReader.getI(73);
            const size_t nVertices = aReader.getCount(93);
            std::vector<DXFPathVertex> aVertices;
            for (size_t i = 0; i < nVertices && aReader.at(10); ++i)
            {
                DXFPathVertex aVertex;
                aVertex.maPoint = aReader.getPoint(10);
                if (bHasBulges || aReader.at(42))
                    aVertex.mfBulge = aReader.getF(42);
                aVertices.push_back(aVertex);
            }
            aLoop = dxf::createPolyline(aVertices, true);
        }
        else
        {
            const size_t nEdges = aReader.getCount(93);
            for (size_t i = 0; i < nEdges; ++i)
            {
                // skip what an edge has beyond the expected groups
                while (!aReader.atEnd() && !aReader.at(72) && !aReader.at(97) && !aReader.at(92))
                    aReader.skip();
                if (!aReader.at(72))
                    break;
                appendEdge(aLoop, readEdge(aReader, aReader.getI(72)));
            }
            if (aLoop.count() > 1 && isNear(aLoop.getB2DPoint(0), aLoop.getB2DPoint(aLoop.count() - 1)))
                dxf::closePolygon(aLoop);
            aLoop.setClosed(true);
        }
        // the source boundary objects
        const size_t nSources = aReader.getCount(97);
        for (size_t i = 0; i < nSources && aReader.at(330); ++i)
            aReader.skip();
        if (aLoop.count() > 1)
            aBoundaries.append(aLoop);
    }

    // the pattern and the gradient
    while (!aReader.atEnd())
    {
        const sal_uInt16 nCode = aReader.code();
        if (nCode == 78)
        {
            const size_t nLines = aReader.getCount(78);
            for (size_t i = 0; i < nLines && aReader.at(53); ++i)
            {
                DXFHatchPatternLine aLine;
                aLine.fAngle = aReader.getF(53);
                aLine.aBase.setX(aReader.getF(43));
                aLine.aBase.setY(aReader.getF(44));
                aLine.aOffset.setX(aReader.getF(45));
                aLine.aOffset.setY(aReader.getF(46));
                const size_t nDashes = aReader.getCount(79);
                for (size_t j = 0; j < nDashes && aReader.at(49); ++j)
                    aLine.aDashes.push_back(aReader.getF(49));
                aPatternLines.push_back(aLine);
            }
        }
        else if (nCode == 450)
            bGradient = aReader.getI(450) != 0;
        else if (nCode == 460)
            fGradientAngle = aReader.getF(460);
        else if (nCode == 421)
            aGradientColors.push_back(aReader.getI(421) & 0xffffff);
        else
            aReader.skip();
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
    nFaceIndex[0]=nFaceIndex[1]=nFaceIndex[2]=nFaceIndex[3]=0;

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
        case 71: nFaceIndex[0]=rDGR.GetI(); break;
        case 72: nFaceIndex[1]=rDGR.GetI(); break;
        case 73: nFaceIndex[2]=rDGR.GetI(); break;
        case 74: nFaceIndex[3]=rDGR.GetI(); break;
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


//--------------------------DXFEllipseEntity------------------------------------

DXFEllipseEntity::DXFEllipseEntity()
    : DXFBasicEntity(DXF_ELLIPSE)
    , aMajor(1.0, 0.0, 0.0)
    , fRatio(1.0)
    , fStart(0.0)
    , fEnd(2 * M_PI)
{
}

void DXFEllipseEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 10: aCenter.fx=rDGR.GetF(); break;
        case 20: aCenter.fy=rDGR.GetF(); break;
        case 30: aCenter.fz=rDGR.GetF(); break;
        case 11: aMajor.fx=rDGR.GetF(); break;
        case 21: aMajor.fy=rDGR.GetF(); break;
        case 31: aMajor.fz=rDGR.GetF(); break;
        case 40: fRatio=rDGR.GetF(); break;
        case 41: fStart=rDGR.GetF(); break;
        case 42: fEnd=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFSplineEntity-------------------------------------

DXFSplineEntity::DXFSplineEntity()
    : DXFBasicEntity(DXF_SPLINE)
    , nFlags(0)
    , nDegree(3)
{
}

void DXFSplineEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case 70: nFlags=rDGR.GetI(); break;
        case 71: nDegree=rDGR.GetI(); break;
        case 40: aKnots.push_back(rDGR.GetF()); break;
        case 41: aWeights.push_back(rDGR.GetF()); break;
        case 10: aControlPoints.emplace_back(rDGR.GetF(), 0.0, 0.0); break;
        case 20: if (!aControlPoints.empty()) aControlPoints.back().fy=rDGR.GetF(); break;
        case 30: if (!aControlPoints.empty()) aControlPoints.back().fz=rDGR.GetF(); break;
        case 11: aFitPoints.emplace_back(rDGR.GetF(), 0.0, 0.0); break;
        case 21: if (!aFitPoints.empty()) aFitPoints.back().fy=rDGR.GetF(); break;
        case 31: if (!aFitPoints.empty()) aFitPoints.back().fz=rDGR.GetF(); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//--------------------------DXFMTextEntity--------------------------------------

DXFMTextEntity::DXFMTextEntity()
    : DXFBasicEntity(DXF_MTEXT)
    , aXDirection(1.0, 0.0, 0.0)
    , bHasXDirection(false)
    , fHeight(1.0)
    , fReferenceWidth(0.0)
    , fLineSpacing(1.0)
    , fRotAngle(0.0)
    , nAttachment(1)
    , m_sStyle("STANDARD"_ostr)
    , bInEmbeddedObject(false)
{
}

void DXFMTextEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    if (bInEmbeddedObject)
        return;
    switch (rDGR.GetG()) {
        case 10: aP0.fx=rDGR.GetF(); break;
        case 20: aP0.fy=rDGR.GetF(); break;
        case 30: aP0.fz=rDGR.GetF(); break;
        case 11: aXDirection.fx=rDGR.GetF(); bHasXDirection=true; break;
        case 21: aXDirection.fy=rDGR.GetF(); bHasXDirection=true; break;
        case 31: aXDirection.fz=rDGR.GetF(); bHasXDirection=true; break;
        case 40: fHeight=rDGR.GetF(); break;
        case 41: fReferenceWidth=rDGR.GetF(); break;
        case 44: fLineSpacing=rDGR.GetF(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 71: nAttachment=rDGR.GetI(); break;
        case  1:
        case  3: m_sText += rDGR.GetS(); break;
        case  7: m_sStyle=rDGR.GetS(); break;
        case 101: bInEmbeddedObject=true; break;
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

//---------------------------- DXFEntities --------------------------------------

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
        else if (rDGR.GetS() == "ELLIPSE"   ) pE=new DXFEllipseEntity;
        else if (rDGR.GetS() == "SPLINE"    ) pE=new DXFSplineEntity;
        else if (rDGR.GetS() == "MTEXT"     ) pE=new DXFMTextEntity;
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
