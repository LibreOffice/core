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

#ifndef _DXFENTRD_HXX
#define _DXFENTRD_HXX

#include <dxfgrprd.hxx>
#include <dxfvec.hxx>

#include <deque>

typedef std::deque< Point > DXFPointArray;

//------------------------------------------------------------------------------
//------------------------- Art eines Entity -----------------------------------
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
//---------------------- Basisklasse fuer ein Entity ---------------------------
//------------------------------------------------------------------------------

class DXFBasicEntity {

public:

    DXFBasicEntity * pSucc;
        // Zeiger auf naechstes Entity (in der Liste DXFEntities.pFirst)

    DXFEntityType eType;
        // Art des Entitys (Linie oder Kreis oder was)

    // Eigenschaftenm, die alle Entities besitzen, jeweils
    // durch den Gruppencode kommentiert:
    char sLayer[DXF_MAX_STRING_LEN+1];    //  8
    char sLineType[DXF_MAX_STRING_LEN+1]; //  6
    double fElevation;                    // 38
    double fThickness;                    // 39
    long nColor;                          // 62
    long nSpace;                          // 67
    DXFVector aExtrusion;                 // 210,220,230

protected:

    DXFBasicEntity(DXFEntityType eThisType);
        // Konstruktoren der Entities initialiseren immer mit Defaultwerten.

public:

    virtual ~DXFBasicEntity();
    virtual void Read(DXFGroupReader & rDGR);
        // Liest die Prameter ein, bis zur naechten 0-Gruppe

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
        // Diese Methode wird durch Read() fuer jeden Parameter (bzw. fuer jede
        // Gruppe) aufgerufen.
        // Sofern der Gruppencode dem Entity bekannt ist, wird der entsprechende
        // Parameter geholt.

};

//------------------------------------------------------------------------------
//---------------- die verschiedenen Arten von Entyties ------------------------
//------------------------------------------------------------------------------

//--------------------------Line------------------------------------------------

class DXFLineEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31

    DXFLineEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Point-----------------------------------------------

class DXFPointEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30

    DXFPointEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Circle----------------------------------------------

class DXFCircleEntity : public DXFBasicEntity {

public:

    DXFVector aP0;  // 10,20,30
    double fRadius; // 40

    DXFCircleEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Arc-------------------------------------------------

class DXFArcEntity : public DXFBasicEntity {

public:

    DXFVector aP0;  // 10,20,30
    double fRadius; // 40
    double fStart;  // 50
    double fEnd;    // 51

    DXFArcEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Trace-----------------------------------------------

class DXFTraceEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31
    DXFVector aP2; // 12,22,32
    DXFVector aP3; // 13,23,33

    DXFTraceEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Solid-----------------------------------------------

class DXFSolidEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31
    DXFVector aP2; // 12,22,32
    DXFVector aP3; // 13,23,33

    DXFSolidEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Text------------------------------------------------

class DXFTextEntity : public DXFBasicEntity {

public:

    DXFVector aP0;                     // 10,20,30
    double fHeight;                    // 40
    char sText[DXF_MAX_STRING_LEN+1];  //  1
    double fRotAngle;                  // 50
    double fXScale;                    // 41
    double fOblAngle;                  // 42
    char sStyle[DXF_MAX_STRING_LEN+1]; //  7
    long nGenFlags;                    // 71
    long nHorzJust;                    // 72
    long nVertJust;                    // 73
    DXFVector aAlign;                  // 11,21,31

    DXFTextEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Shape-----------------------------------------------

class DXFShapeEntity : public DXFBasicEntity {

public:

    DXFVector aP0;                    // 10,20,30
    double fSize;                     // 40
    char sName[DXF_MAX_STRING_LEN+1]; //  2
    double fRotAngle;                 // 50
    double fXScale;                   // 41
    double fOblAngle;                 // 51

    DXFShapeEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Insert----------------------------------------------

class DXFInsertEntity : public DXFBasicEntity {

public:

    long nAttrFlag;                   // 66
    char sName[DXF_MAX_STRING_LEN+1]; //  2
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

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------AttDef----------------------------------------------

class DXFAttDefEntity : public DXFBasicEntity {

public:

    DXFVector aP0;                      // 10,20,30
    double fHeight;                     // 40
    char sDefVal[DXF_MAX_STRING_LEN+1]; //  1
    char sPrompt[DXF_MAX_STRING_LEN+1]; //  3
    char sTagStr[DXF_MAX_STRING_LEN+1]; //  2
    long nAttrFlags;                    // 70
    long nFieldLen;                     // 73
    double fRotAngle;                   // 50
    double fXScale;                     // 41
    double fOblAngle;                   // 51
    char sStyle[DXF_MAX_STRING_LEN+1];  //  7
    long nGenFlags;                     // 71
    long nHorzJust;                     // 72
    long nVertJust;                     // 74
    DXFVector aAlign;                   // 11,21,31

    DXFAttDefEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Attrib----------------------------------------------

class DXFAttribEntity : public DXFBasicEntity {

public:

    DXFVector aP0;                      // 10,20,30
    double fHeight;                     // 40
    char sText[DXF_MAX_STRING_LEN+1];   //  1
    char sTagStr[DXF_MAX_STRING_LEN+1]; //  2
    long nAttrFlags;                    // 70
    long nFieldLen;                     // 73
    double fRotAngle;                   // 50
    double fXScale;                     // 41
    double fOblAngle;                   // 51
    char sStyle[DXF_MAX_STRING_LEN+1];  //  7
    long nGenFlags;                     // 71
    long nHorzJust;                     // 72
    long nVertJust;                     // 74
    DXFVector aAlign;                   // 11,21,31

    DXFAttribEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------PolyLine--------------------------------------------

class DXFPolyLineEntity : public DXFBasicEntity {

public:

    double fElevation; // 30
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

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

class DXFLWPolyLineEntity : public DXFBasicEntity
{
        sal_Int32	nIndex;

    public :
    
        sal_Int32	nCount;			// 90
        sal_Int32	nFlags;			// 70	1 = closed, 128 = plinegen
        double		fConstantWidth;	// 43	(optional - default: 0, not used if fStartWidth and/or fEndWidth is used)
        double		fStartWidth;	// 40
        double		fEndWidth;		// 41

        DXFVector*	pP;

        DXFLWPolyLineEntity();
        ~DXFLWPolyLineEntity();

    protected :
        
        virtual void EvaluateGroup( DXFGroupReader & rDGR );

};

//-------------------------- Hatch ---------------------------------------------

struct DXFEdgeType
{
    sal_Int32 nEdgeType;

    virtual ~DXFEdgeType(){};
    virtual sal_Bool EvaluateGroup( DXFGroupReader & /*rDGR*/ ){ return sal_True; };

    protected :

        DXFEdgeType( sal_Int32 EdgeType ):nEdgeType(EdgeType){};
};
struct DXFEdgeTypeLine : public DXFEdgeType
{
    DXFVector aStartPoint;				// 10,20
    DXFVector aEndPoint;				// 11,21
    DXFEdgeTypeLine();
    virtual ~DXFEdgeTypeLine();
    virtual sal_Bool EvaluateGroup( DXFGroupReader & rDGR );
};
struct DXFEdgeTypeCircularArc : public DXFEdgeType
{
    DXFVector aCenter;					// 10,20
    double	  fRadius;					// 40
    double	  fStartAngle;				// 50
    double	  fEndAngle;				// 51
    sal_Int32 nIsCounterClockwiseFlag;	// 73
    DXFEdgeTypeCircularArc();
    virtual ~DXFEdgeTypeCircularArc();
    virtual sal_Bool EvaluateGroup( DXFGroupReader & rDGR );
};
struct DXFEdgeTypeEllipticalArc : public DXFEdgeType
{
    DXFVector aCenter;					// 10,20
    DXFVector aEndPoint;				// 11,21
    double	  fLength;					// 40
    double	  fStartAngle;				// 50
    double	  fEndAngle;				// 51
    sal_Int32 nIsCounterClockwiseFlag;	// 73

    DXFEdgeTypeEllipticalArc();
    virtual ~DXFEdgeTypeEllipticalArc();
    virtual sal_Bool EvaluateGroup( DXFGroupReader & rDGR );
};
struct DXFEdgeTypeSpline : public DXFEdgeType
{
    sal_Int32 nDegree;					// 94
    sal_Int32 nRational;				// 73
    sal_Int32 nPeriodic;				// 74
    sal_Int32 nKnotCount;				// 75
    sal_Int32 nControlCount;			// 76

    DXFEdgeTypeSpline();
    virtual ~DXFEdgeTypeSpline();
    virtual sal_Bool EvaluateGroup( DXFGroupReader & rDGR );
};

typedef std::deque< DXFEdgeType* > DXFEdgeTypeArray;

struct DXFBoundaryPathData
{
    sal_Int32			nFlags;					// 92
    sal_Int32			nHasBulgeFlag;			// 72
    sal_Int32			nIsClosedFlag;			// 73
    sal_Int32			nPointCount;			// 93
    double				fBulge;					// 42
    sal_Int32			nSourceBoundaryObjects;	// 97
    sal_Int32			nEdgeCount;				// 93

    sal_Bool			bIsPolyLine;
    sal_Int32			nPointIndex;

    DXFVector*			pP;
    DXFEdgeTypeArray	aEdges;

    DXFBoundaryPathData();
    ~DXFBoundaryPathData();

    sal_Bool EvaluateGroup( DXFGroupReader & rDGR );
};

class DXFHatchEntity : public DXFBasicEntity
{
        sal_Bool	bIsInBoundaryPathContext;
        sal_Int32	nCurrentBoundaryPathIndex;

    public :

        DXFVector	aElevationPoint;
        sal_Int32	nFlags;							// 70 (solid fill = 1, pattern fill = 0)
        sal_Int32	nAssociativityFlag;				// 71 (assoiciative = 1, non-associative = 0)
        sal_Int32	nBoundaryPathCount;				// 91
        sal_Int32	nHatchStyle;					// 75 (odd parity = 0, outmost area = 1, entire area = 2 )
        sal_Int32	nHatchPatternType;				// 76 (user defined = 0, predefined = 1, custom = 2)
        double		fHatchPatternAngle;				// 52 (pattern fill only)
        double		fHatchPatternScale;				// 41 (pattern fill only:scale or spacing)
        sal_Int32	nHatchDoubleFlag;				// 77 (pattern fill only:double = 1, not double = 0)
        sal_Int32	nHatchPatternDefinitionLines;	// 78 
        double		fPixelSize;						// 47
        sal_Int32	nNumberOfSeedPoints;			// 98

        DXFBoundaryPathData* pBoundaryPathData;

        DXFHatchEntity();
        ~DXFHatchEntity();

    protected :
        
        virtual void EvaluateGroup( DXFGroupReader & rDGR );
};


//--------------------------Vertex----------------------------------------------

class DXFVertexEntity : public DXFBasicEntity {

public:

    DXFVector aP0;     // 10,20,30
    double fSWidth;    // 40 (Wenn <0.0, dann gilt DXFPolyLine::fSWidth)
    double fEWidth;    // 41 (Wenn <0.0, dann gilt DXFPolyLine::fEWidth)
    double fBulge;     // 42
    long nFlags;       // 70
    double fCFTDir;    // 50

    DXFVertexEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------SeqEnd----------------------------------------------

class DXFSeqEndEntity : public DXFBasicEntity {

public:

    DXFSeqEndEntity();
};

//--------------------------3DFace----------------------------------------------

class DXF3DFaceEntity : public DXFBasicEntity {

public:

    DXFVector aP0; // 10,20,30
    DXFVector aP1; // 11,21,31
    DXFVector aP2; // 12,22,32
    DXFVector aP3; // 13,23,33
    long nIEFlags; // 70

    DXF3DFaceEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//--------------------------Dimension-------------------------------------------

class DXFDimensionEntity : public DXFBasicEntity {

public:

    char sPseudoBlock[DXF_MAX_STRING_LEN+1]; //  2

    DXFDimensionEntity();

protected:

    virtual void EvaluateGroup(DXFGroupReader & rDGR);
};

//------------------------------------------------------------------------------
//----------- Eine Menge von Entities lesen und repraesentieren ----------------
//------------------------------------------------------------------------------

class DXFEntities {

public:

    DXFEntities();
    ~DXFEntities();

    DXFBasicEntity * pFirst; // Liste von Entities, READ ONLY!

    void Read(DXFGroupReader & rDGR);
        // Liest Entitis per rGDR aus einer DXF-Datei bis zu
        // einem ENDBLK, ENDSEC oder EOF (der Gruppe 0).
        // (Alle unbekannten Dinge werden uebersprungen)

    void Clear();
        // Loescht alle Entities
};

//------------------------------------------------------------------------------
//--------------------------------- inlines ------------------------------------
//------------------------------------------------------------------------------

inline DXFEntities::DXFEntities()
{
    pFirst=NULL;
}


inline DXFEntities::~DXFEntities()
{
    Clear();
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
