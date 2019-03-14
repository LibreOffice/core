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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXFENTRD_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXFENTRD_HXX

#include "dxfgrprd.hxx"
#include "dxfvec.hxx"

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
    DXF_HATCH
};

// base class of an entity

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
    long nColor;                          // 62
    long nSpace;                          // 67
    DXFVector aExtrusion;                 // 210,220,230

protected:

    DXFBasicEntity(DXFEntityType eThisType);
        // always initialize the constructors of entities with default values

public:

    virtual ~DXFBasicEntity();
    void Read(DXFGroupReader & rDGR);
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
    long nGenFlags;                    // 71
    long nHorzJust;                    // 72
    long nVertJust;                    // 73
    DXFVector aAlign;                  // 11,21,31

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

    long nAttrFlag;                   // 66
    OString m_sName;                  //  2
    DXFVector aP0;                    // 10,20,30
    double fXScale;                   // 41
    double fYScale;                   // 42
    double fZScale;                   // 43
    double fRotAngle;                 // 50
    long nColCount;                   // 70
    long nRowCount;                   // 71
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
    long nAttrFlags;                    // 70
    long nFieldLen;                     // 73
    double fRotAngle;                   // 50
    double fXScale;                     // 41
    double fOblAngle;                   // 51
    OString m_sStyle;                   //  7
    long nGenFlags;                     // 71
    long nHorzJust;                     // 72
    long nVertJust;                     // 74
    DXFVector aAlign;                   // 11,21,31

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
    long nAttrFlags;                    // 70
    long nFieldLen;                     // 73
    double fRotAngle;                   // 50
    double fXScale;                     // 41
    double fOblAngle;                   // 51
    OString m_sStyle;                   //  7
    long nGenFlags;                     // 71
    long nHorzJust;                     // 72
    long nVertJust;                     // 74
    DXFVector aAlign;                   // 11,21,31

    DXFAttribEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFPolyLineEntity : public DXFBasicEntity {

public:

    long nFlags;       // 70
    double fSWidth;    // 40
    double fEWidth;    // 41
    long nMeshMCount;  // 71
    long nMeshNCount;  // 72
    long nMDensity;    // 73
    long nNDensity;    // 74
    long nCSSType;     // 75

    DXFPolyLineEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR) override;
};

class DXFLWPolyLineEntity : public DXFBasicEntity
{
        sal_Int32   nIndex;
        sal_Int32   nCount;         // 90

    public:

        sal_Int32   nFlags;         // 70   1 = closed, 128 = plinegen
        double      fConstantWidth; // 43   (optional - default: 0, not used if fStartWidth and/or fEndWidth is used)
        double      fStartWidth;    // 40
        double      fEndWidth;      // 41

        std::vector<DXFVector>  aP;

        DXFLWPolyLineEntity();

    protected:

        virtual void EvaluateGroup( DXFGroupReader & rDGR ) override;

};

struct DXFEdgeType
{
    sal_Int32 nEdgeType;

    virtual ~DXFEdgeType(){};
    virtual bool EvaluateGroup( DXFGroupReader & /*rDGR*/ ){ return true; };

    protected:

        DXFEdgeType( sal_Int32 EdgeType ):nEdgeType(EdgeType){};
};

struct DXFEdgeTypeLine : public DXFEdgeType
{
    DXFVector aStartPoint;              // 10,20
    DXFVector aEndPoint;                // 11,21
    DXFEdgeTypeLine();
    virtual bool EvaluateGroup( DXFGroupReader & rDGR ) override;
};

struct DXFEdgeTypeCircularArc : public DXFEdgeType
{
    DXFVector aCenter;                  // 10,20
    double    fRadius;                  // 40
    double    fStartAngle;              // 50
    double    fEndAngle;                // 51
    sal_Int32 nIsCounterClockwiseFlag;  // 73
    DXFEdgeTypeCircularArc();
    virtual bool EvaluateGroup( DXFGroupReader & rDGR ) override;
};

struct DXFEdgeTypeEllipticalArc : public DXFEdgeType
{
    DXFVector aCenter;                  // 10,20
    DXFVector aEndPoint;                // 11,21
    double    fLength;                  // 40
    double    fStartAngle;              // 50
    double    fEndAngle;                // 51
    sal_Int32 nIsCounterClockwiseFlag;  // 73

    DXFEdgeTypeEllipticalArc();
    virtual bool EvaluateGroup( DXFGroupReader & rDGR ) override;
};

struct DXFEdgeTypeSpline : public DXFEdgeType
{
    sal_Int32 nDegree;                  // 94
    sal_Int32 nRational;                // 73
    sal_Int32 nPeriodic;                // 74
    sal_Int32 nKnotCount;               // 75
    sal_Int32 nControlCount;            // 76

    DXFEdgeTypeSpline();
    virtual bool EvaluateGroup( DXFGroupReader & rDGR ) override;
};

struct DXFBoundaryPathData
{
private:
    sal_Int32           nPointCount;            // 93
public:
    sal_Int32           nFlags;                 // 92
    sal_Int32           nHasBulgeFlag;          // 72
    sal_Int32           nIsClosedFlag;          // 73
    double              fBulge;                 // 42
    sal_Int32           nSourceBoundaryObjects; // 97
    sal_Int32           nEdgeCount;             // 93

    bool                bIsPolyLine;
    sal_Int32           nPointIndex;

    std::vector<DXFVector> aP;
    std::vector<std::unique_ptr<DXFEdgeType>> aEdges;

    DXFBoundaryPathData();
    ~DXFBoundaryPathData();

    bool EvaluateGroup( DXFGroupReader & rDGR );
};

class DXFHatchEntity : public DXFBasicEntity
{
        bool    bIsInBoundaryPathContext;
        sal_Int32   nCurrentBoundaryPathIndex;

    public:

        sal_Int32   nFlags;                         // 70 (solid fill = 1, pattern fill = 0)
        sal_Int32   nAssociativityFlag;             // 71 (associative = 1, non-associative = 0)
        sal_Int32   nBoundaryPathCount;             // 91
        sal_Int32   nHatchStyle;                    // 75 (odd parity = 0, outmost area = 1, entire area = 2 )
        sal_Int32   nHatchPatternType;              // 76 (user defined = 0, predefined = 1, custom = 2)
        double      fHatchPatternAngle;             // 52 (pattern fill only)
        double      fHatchPatternScale;             // 41 (pattern fill only:scale or spacing)
        sal_Int32   nHatchDoubleFlag;               // 77 (pattern fill only:double = 1, not double = 0)
        sal_Int32   nHatchPatternDefinitionLines;   // 78
        double      fPixelSize;                     // 47
        sal_Int32   nNumberOfSeedPoints;            // 98

        std::unique_ptr<DXFBoundaryPathData[]> pBoundaryPathData;

        DXFHatchEntity();

    protected:

        virtual void EvaluateGroup( DXFGroupReader & rDGR ) override;
};

class DXFVertexEntity : public DXFBasicEntity {

public:

    DXFVector aP0;     // 10,20,30
    double fSWidth;    // 40 (if <0.0, then one has DXFPolyLine::fSWidth)
    double fEWidth;    // 41 (if <0.0, then one has DXFPolyLine::fEWidth)
    double fBulge;     // 42
    long nFlags;       // 70
    double fCFTDir;    // 50

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
    long nIEFlags; // 70

    DXF3DFaceEntity();

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

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
