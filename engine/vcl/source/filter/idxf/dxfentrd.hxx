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

#pragma once

#include "dxfgrprd.hxx"
#include "dxfvec.hxx"
#include "dxfgeom.hxx"
#include <tools/long.hxx>

#include <memory>
#include <vector>

enum DXFEntityType {
    DXF_LINE,
    DXF_POINT,
    DXF_CIRCLE,
    DXF_ARC,
    DXF_TRACE,
    DXF_SOLID,
    DXF_TEXT,
    DXF_SHAPE,
    DXF_INSERT,
    DXF_ATTDEF,
    DXF_ATTRIB,
    DXF_POLYLINE,
    DXF_VERTEX,
    DXF_SEQEND,
    DXF_3DFACE,
    DXF_DIMENSION,
    DXF_LWPOLYLINE,
    DXF_HATCH,
    DXF_ELLIPSE,
    DXF_SPLINE,
    DXF_MTEXT,
    DXF_LEADER,
    DXF_IMAGE
};

// base class of an entity

// special line weights
constexpr tools::Long DXF_LINEWEIGHT_BYLAYER = -1;
constexpr tools::Long DXF_LINEWEIGHT_BYBLOCK = -2;
constexpr tools::Long DXF_LINEWEIGHT_DEFAULT = -3;

class DXFBasicEntity {

public:

    DXFBasicEntity * pSucc;
        // pointer to next entity (in the list of DXFEntities.pFirst)

    DXFEntityType eType;
        // entity kind (line or circle or what)

    // properties that all entities have, each
    // commented with group codes:
    OString m_sLayer;                     //  8
    OString m_sLineType;                  //  6
    double fThickness;                    // 39
    tools::Long nColor;                          // 62
    tools::Long nTrueColor;                      // 420, 0xRRGGBB or -1
    bool bInvisible;                             // 60
    double fLineTypeScale;                       // 48
    tools::Long nLineWeight;                     // 370, 1/100 mm or one of the DXF_LINEWEIGHT_* values
    tools::Long nSpace;                          // 67
    DXFVector aExtrusion;                 // 210,220,230

protected:

    DXFBasicEntity(DXFEntityType eThisType);
        // always initialize the constructors of entities with default values

public:

    virtual ~DXFBasicEntity();
    virtual void Read(DXFGroupReader & rDGR);
        // Reads a parameter till the next 0-group

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
        // This method will be called by Read() for every parameter (respectively
        // for every group).
        // As far as the group code of the entity is known, the corresponding
        // parameter is fetched.

};


// the different kinds of entities

class DXFLineEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31

    DXFLineEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFPointEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30

    DXFPointEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFCircleEntity : public DXFBasicEntity {

public:

    DXFVector aP0;  // 10,20,30
    double fRadius; // 40

    DXFCircleEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFArcEntity : public DXFBasicEntity {

public:

    DXFVector aP0;  // 10,20,30
    double fRadius; // 40
    double fStart;  // 50
    double fEnd;    // 51

    DXFArcEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFTraceEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31
    DXFVector aP2; // 12,22,32
    DXFVector aP3; // 13,23,33

    DXFTraceEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFSolidEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31
    DXFVector aP2; // 12,22,32
    DXFVector aP3; // 13,23,33

    DXFSolidEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFTextEntity : public DXFBasicEntity {

public:

    DXFVector aP0;                     // 10,20,30
    double fHeight;                    // 40
    OString m_sText;                   //  1
    double fRotAngle;                  // 50
    double fXScale;                    // 41
    double fOblAngle;                  // 42
    OString m_sStyle;                  //  7
    tools::Long nGenFlags;                    // 71
    tools::Long nHorzJust;                    // 72
    tools::Long nVertJust;                    // 73
    DXFVector aAlign;                  // 11,21,31
    bool bHasAlign;

    DXFTextEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFShapeEntity : public DXFBasicEntity {

    DXFVector aP0;                    // 10,20,30
    double fSize;                     // 40
    OString m_sName;                  //  2
    double fRotAngle;                 // 50
    double fXScale;                   // 41
    double fOblAngle;                 // 51

public:

    DXFShapeEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFInsertEntity : public DXFBasicEntity {

public:

    tools::Long nAttrFlag;                   // 66
    OString m_sName;                  //  2
    DXFVector aP0;                    // 10,20,30
    double fXScale;                   // 41
    double fYScale;                   // 42
    double fZScale;                   // 43
    double fRotAngle;                 // 50
    tools::Long nColCount;                   // 70
    tools::Long nRowCount;                   // 71
    double fColSpace;                 // 44
    double fRowSpace;                 // 45

    DXFInsertEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFAttDefEntity : public DXFBasicEntity {

    DXFVector aP0;                      // 10,20,30
    double fHeight;                     // 40
    OString m_sDefVal;                  //  1
    OString m_sPrompt;                  //  3
    OString m_sTagStr;                  //  2
    tools::Long nAttrFlags;                    // 70
    tools::Long nFieldLen;                     // 73
    double fRotAngle;                   // 50
    double fXScale;                     // 41
    double fOblAngle;                   // 51
    OString m_sStyle;                   //  7
    tools::Long nGenFlags;                     // 71
    tools::Long nHorzJust;                     // 72
    tools::Long nVertJust;                     // 74
    DXFVector aAlign;                   // 11,21,31
    // the groups after 101 describe the embedded multiline text, not the attribute
    bool bInEmbeddedObject;

public:

    DXFAttDefEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFAttribEntity : public DXFBasicEntity {

public:

    DXFVector aP0;                      // 10,20,30
    double fHeight;                     // 40
    OString m_sText;                    //  1
    OString m_sTagStr;                  //  2
    tools::Long nAttrFlags;                    // 70
    tools::Long nFieldLen;                     // 73
    double fRotAngle;                   // 50
    double fXScale;                     // 41
    double fOblAngle;                   // 51
    OString m_sStyle;                   //  7
    tools::Long nGenFlags;                     // 71
    tools::Long nHorzJust;                     // 72
    tools::Long nVertJust;                     // 74
    DXFVector aAlign;                   // 11,21,31
    bool bHasAlign;

    DXFAttribEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;

private:
    // the groups after 101 describe the embedded multiline text, not the attribute
    bool bInEmbeddedObject;
};

class DXFPolyLineEntity : public DXFBasicEntity {

public:

    double fElevation; // 30
    tools::Long nFlags;       // 70
    double fSWidth;    // 40
    double fEWidth;    // 41
    tools::Long nMeshMCount;  // 71
    tools::Long nMeshNCount;  // 72
    tools::Long nMDensity;    // 73
    tools::Long nNDensity;    // 74
    tools::Long nCSSType;     // 75

    DXFPolyLineEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFLWPolyLineEntity : public DXFBasicEntity
{
        sal_Int32   nCount;         // 90
        // the groups of a vertex beyond the count are ignored
        bool        bVertexAccepted;

    public:

        sal_Int32   nFlags;         // 70   1 = closed, 128 = plinegen
        double      fConstantWidth; // 43   used by the vertices without their own widths
        double      fElevation;     // 38

        // 10, 20, and the 40, 41 widths (-1 if not given) and 42 bulge of the segment
        // starting there
        std::vector<DXFPathVertex> aVertices;

        DXFLWPolyLineEntity();

    protected:

        virtual void EvaluateGroup( DXFGroupReader & rDGR ) override;

};

// a line family of a hatch pattern, already rotated and scaled
struct DXFHatchPatternLine
{
    double fAngle = 0.0;                // 53, degrees
    basegfx::B2DPoint aBase;            // 43, 44
    basegfx::B2DVector aOffset;         // 45, 46, from one line to the next
    std::vector<double> aDashes;        // 49, positive for dashes, negative for gaps
};

class DXFHatchEntity : public DXFBasicEntity
{
    public:

        double      fElevation;         // 30
        bool        bSolid;             // 70
        // the boundary paths in the entity coordinate system, filled with the even-odd rule
        basegfx::B2DPolyPolygon aBoundaries;
        std::vector<DXFHatchPatternLine> aPatternLines;
        bool        bGradient;          // 450
        double      fGradientAngle;     // 460, radians
        std::vector<sal_Int32> aGradientColors; // 421, 0xRRGGBB

        DXFHatchEntity();

        // the groups use the same codes for different values in their sections, read them
        // in their order
        virtual void Read(DXFGroupReader & rDGR) override;
};

class DXFVertexEntity : public DXFBasicEntity {

public:

    DXFVector aP0;     // 10,20,30
    double fSWidth;    // 40 (if <0.0, then one has DXFPolyLine::fSWidth)
    double fEWidth;    // 41 (if <0.0, then one has DXFPolyLine::fEWidth)
    double fBulge;     // 42
    tools::Long nFlags;       // 70
    double fCFTDir;    // 50
    tools::Long nFaceIndex[4]; // 71-74, polyface mesh faces

    DXFVertexEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFSeqEndEntity : public DXFBasicEntity {

public:

    DXFSeqEndEntity();
};

class DXF3DFaceEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31
    DXFVector aP2; // 12,22,32
    DXFVector aP3; // 13,23,33
    tools::Long nIEFlags; // 70

    DXF3DFaceEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

//--------------------------Ellipse---------------------------------------------

// in world coordinates, the extrusion only orients the minor axis
class DXFEllipseEntity : public DXFBasicEntity {
public:
    DXFVector aCenter;   // 10,20,30
    DXFVector aMajor;    // 11,21,31, end of the major axis relative to the centre
    double fRatio;       // 40, minor to major axis
    double fStart;       // 41, parameter in radians
    double fEnd;         // 42

    DXFEllipseEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

//--------------------------Spline----------------------------------------------

// in world coordinates
class DXFSplineEntity : public DXFBasicEntity {
public:
    tools::Long nFlags;                  // 70, 1 = closed
    tools::Long nDegree;                 // 71
    std::vector<double> aKnots;          // 40
    std::vector<double> aWeights;        // 41
    std::vector<DXFVector> aControlPoints; // 10,20,30
    std::vector<DXFVector> aFitPoints;     // 11,21,31

    DXFSplineEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

//--------------------------MText-----------------------------------------------

// multiline text with inline formatting, the insertion point is in world coordinates
class DXFMTextEntity : public DXFBasicEntity {
public:
    DXFVector aP0;                 // 10,20,30
    DXFVector aXDirection;         // 11,21,31, wins over the rotation
    bool bHasXDirection;
    double fHeight;                // 40
    double fReferenceWidth;        // 41, for word wrapping
    double fLineSpacing;           // 44
    double fRotAngle;              // 50, degrees
    tools::Long nAttachment;       // 71, 1-9: top/middle/bottom x left/center/right
    OString m_sText;               // 3 chunks then 1
    OString m_sStyle;              // 7

    DXFMTextEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;

private:
    // the groups after 101 describe an embedded object, not the text
    bool bInEmbeddedObject;
};

//--------------------------Leader----------------------------------------------

// in world coordinates
class DXFLeaderEntity : public DXFBasicEntity {
public:
    OString m_sDimStyle;               // 3
    bool bArrow;                       // 71
    bool bSpline;                      // 72
    std::vector<DXFVector> aVertices;  // 10,20,30

    DXFLeaderEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

//--------------------------Image and Wipeout-----------------------------------

// A raster image is drawn as its frame, a wipeout covers what is below it. In world coordinates.
class DXFImageEntity : public DXFBasicEntity {
public:
    bool bWipeout;
    DXFVector aInsert;                 // 10,20,30, the outer corner of the first pixel
    DXFVector aU;                      // 11,21,31, one pixel along the width
    DXFVector aV;                      // 12,22,32, one pixel along the height
    double fWidth;                     // 13, in pixels
    double fHeight;                    // 23
    tools::Long nClipType;             // 71, 1 rectangle, 2 polygon
    std::vector<basegfx::B2DPoint> aClipVertices; // 14,24, in pixels from the top left

    explicit DXFImageEntity(bool bIsWipeout);

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFDimensionEntity : public DXFBasicEntity {

public:

    OString m_sPseudoBlock;                  //  2

    DXFDimensionEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};


// read and represent the set of entities
class DXFEntities {

public:

    DXFEntities()
        : pFirst(nullptr)
        , mbBeingDrawn(false)
    {
    }

    ~DXFEntities()
    {
        Clear();
    }

    DXFBasicEntity * pFirst; // list of entities, READ ONLY!
    mutable bool mbBeingDrawn; // guard for loop in entity parsing

    void Read(DXFGroupReader & rDGR);
        // read entities by rGDR of a DXF file until a
        // ENDBLK, ENDSEC or EOF (of group 0).
        // (all unknown thing will be skipped)

    void Clear();
        // deletes all entities
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
