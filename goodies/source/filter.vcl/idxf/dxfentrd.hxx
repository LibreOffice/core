/*************************************************************************
 *
 *  $RCSfile: dxfentrd.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DXFENTRD_HXX
#define _DXFENTRD_HXX

#ifndef _DXFGRPRD_HXX
#include <dxfgrprd.hxx>
#endif

#ifndef _DXFVEC_HXX
#include <dxfvec.hxx>
#endif

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
    DXF_DIMENSION
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


