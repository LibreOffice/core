/*************************************************************************
 *
 *  $RCSfile: dxfentrd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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

#include <string.h>
#include <dxfentrd.hxx>

//--------------------------DXFBasicEntity--------------------------------------

DXFBasicEntity::DXFBasicEntity(DXFEntityType eThisType)
{
    eType=eThisType;
    pSucc=NULL;
    strcpy(sLayer,"0");
    strcpy(sLineType,"BYLAYER");
    fElevation=0;
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
    switch (rDGR.GetG()) {
        case   8: strcpy(sLayer,rDGR.GetS()); break;
        case   6: strcpy(sLineType,rDGR.GetS()); break;
        case  38: fElevation=rDGR.GetF(); break;
        case  39: fThickness=rDGR.GetF(); break;
        case  62: nColor=rDGR.GetI(); break;
        case  67: nSpace=rDGR.GetI(); break;
        case 210: aExtrusion.fx=rDGR.GetF(); break;
        case 220: aExtrusion.fy=rDGR.GetF(); break;
        case 230: aExtrusion.fz=rDGR.GetF(); break;
    }
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

DXFTextEntity::DXFTextEntity() : DXFBasicEntity(DXF_TEXT)
{
    fHeight=1.0;
    sText[0]=0;
    fRotAngle=0.0;
    fXScale=1.0;
    fOblAngle=0.0;
    strcpy(sStyle,"STANDARD");
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
        case  1: strcpy(sText,rDGR.GetS()); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 42: fOblAngle=rDGR.GetF(); break;
        case  7: strcpy(sStyle,rDGR.GetS()); break;
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
    sName[0]=0;
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
        case  2: strcpy(sName,rDGR.GetS()); break;
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
    sName[0]=0;
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
        case  2: strcpy(sName,rDGR.GetS()); break;
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

DXFAttDefEntity::DXFAttDefEntity() : DXFBasicEntity(DXF_ATTDEF)
{
    fHeight=1.0;
    sDefVal[0]=0;
    sPrompt[0]=0;
    sTagStr[0]=0;
    nAttrFlags=0;
    nFieldLen=0;
    fRotAngle=0.0;
    fXScale=1.0;
    fOblAngle=0.0;
    strcpy(sStyle,"STANDARD");
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
        case  1: strcpy(sDefVal,rDGR.GetS()); break;
        case  3: strcpy(sPrompt,rDGR.GetS()); break;
        case  2: strcpy(sTagStr,rDGR.GetS()); break;
        case 70: nAttrFlags=rDGR.GetI(); break;
        case 73: nFieldLen=rDGR.GetI(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 51: fOblAngle=rDGR.GetF(); break;
        case  7: strcpy(sStyle,rDGR.GetS()); break;
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

DXFAttribEntity::DXFAttribEntity() : DXFBasicEntity(DXF_ATTRIB)
{
    fHeight=1.0;
    sText[0]=0;
    sTagStr[0]=0;
    nAttrFlags=0;
    nFieldLen=0;
    fRotAngle=0.0;
    fXScale=1.0;
    fOblAngle=0.0;
    strcpy(sStyle,"STANDARD");
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
        case  1: strcpy(sText,rDGR.GetS()); break;
        case  2: strcpy(sTagStr,rDGR.GetS()); break;
        case 70: nAttrFlags=rDGR.GetI(); break;
        case 73: nFieldLen=rDGR.GetI(); break;
        case 50: fRotAngle=rDGR.GetF(); break;
        case 41: fXScale=rDGR.GetF(); break;
        case 51: fOblAngle=rDGR.GetF(); break;
        case  7: strcpy(sStyle,rDGR.GetS()); break;
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
    sPseudoBlock[0]=0;
}

void DXFDimensionEntity::EvaluateGroup(DXFGroupReader & rDGR)
{
    switch (rDGR.GetG()) {
        case  2: strcpy(sPseudoBlock,rDGR.GetS()); break;
        default: DXFBasicEntity::EvaluateGroup(rDGR);
    }
}

//---------------------------- DXFEntites --------------------------------------

void DXFEntities::Read(DXFGroupReader & rDGR)
{
    DXFBasicEntity * pE, * * ppSucc;

    ppSucc=&pFirst;
    while (*ppSucc!=NULL) ppSucc=&((*ppSucc)->pSucc);

    while (rDGR.GetG()!=0) rDGR.Read();

    while (strcmp(rDGR.GetS(),"ENDBLK")!=0 &&
           strcmp(rDGR.GetS(),"ENDSEC")!=0 &&
           strcmp(rDGR.GetS(),"EOF")!=0 )
    {

        if      (strcmp(rDGR.GetS(),"LINE"     )==0) pE=new DXFLineEntity;
        else if (strcmp(rDGR.GetS(),"POINT"    )==0) pE=new DXFPointEntity;
        else if (strcmp(rDGR.GetS(),"CIRCLE"   )==0) pE=new DXFCircleEntity;
        else if (strcmp(rDGR.GetS(),"ARC"      )==0) pE=new DXFArcEntity;
        else if (strcmp(rDGR.GetS(),"TRACE"    )==0) pE=new DXFTraceEntity;
        else if (strcmp(rDGR.GetS(),"SOLID"    )==0) pE=new DXFSolidEntity;
        else if (strcmp(rDGR.GetS(),"TEXT"     )==0) pE=new DXFTextEntity;
        else if (strcmp(rDGR.GetS(),"SHAPE"    )==0) pE=new DXFShapeEntity;
        else if (strcmp(rDGR.GetS(),"INSERT"   )==0) pE=new DXFInsertEntity;
        else if (strcmp(rDGR.GetS(),"ATTDEF"   )==0) pE=new DXFAttDefEntity;
        else if (strcmp(rDGR.GetS(),"ATTRIB"   )==0) pE=new DXFAttribEntity;
        else if (strcmp(rDGR.GetS(),"POLYLINE" )==0) pE=new DXFPolyLineEntity;
        else if (strcmp(rDGR.GetS(),"VERTEX"   )==0) pE=new DXFVertexEntity;
        else if (strcmp(rDGR.GetS(),"SEQEND"   )==0) pE=new DXFSeqEndEntity;
        else if (strcmp(rDGR.GetS(),"3DFACE"   )==0) pE=new DXF3DFaceEntity;
        else if (strcmp(rDGR.GetS(),"DIMENSION")==0) pE=new DXFDimensionEntity;
        else {
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

    while (pFirst!=NULL) {
        ptmp=pFirst;
        pFirst=ptmp->pSucc;
        delete ptmp;
    }
}

