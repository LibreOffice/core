/*************************************************************************
 *
 *  $RCSfile: dxf2mtf.hxx,v $
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

#ifndef _DXF2MTF_HXX
#define _DXF2MTF_HXX

#include "dxfreprd.hxx"
#include <vcl/font.hxx>

#ifndef NOOLDSV
#include <vcl/gdiobj.hxx>
#else // NOOLDSV
enum PenStyle { PEN_NULL, PEN_SOLID, PEN_DOT, PEN_DASH, PEN_DASHDOT };
enum BrushStyle { BRUSH_NULL, BRUSH_SOLID, BRUSH_HORZ, BRUSH_VERT,
                  BRUSH_CROSS, BRUSH_DIAGCROSS, BRUSH_UPDIAG, BRUSH_DOWNDIAG,
                  BRUSH_25, BRUSH_50, BRUSH_75,
                  BRUSH_BITMAP };
#endif // NOOLDSV

class DXF2GDIMetaFile {
private:

    VirtualDevice * pVirDev;
    const DXFRepresentation * pDXF;
    BOOL bStatus;

    USHORT OptPointsPerCircle;

    PFilterCallback pCallback;
    void * pCallerData;
    ULONG nMinPercent;
    ULONG nMaxPercent;
    ULONG nLastPercent;
    ULONG nMainEntitiesCount;

    long        nBlockColor;
    PenStyle    eBlockPStyle;
    long        nParentLayerColor;
    PenStyle    eParentLayerPStyle;
    Color       aActLineColor;
    Color       aActFillColor;
    Font        aActFont;

    ULONG CountEntities(const DXFEntities & rEntities);

    void MayCallback(ULONG nMainEntitiesProcessed);

    Color ConvertColor(BYTE nColor);

    long GetEntityColor(const DXFBasicEntity & rE);

    PenStyle LTypeToPStyle(const char * sLineType);

    PenStyle GetEntityPStyle(const DXFBasicEntity & rE);

    BOOL SetLineAttribute(const DXFBasicEntity & rE, ULONG nWidth=0);

    BOOL SetAreaAttribute(const DXFBasicEntity & rE);

    BOOL SetFontAttribute(const DXFBasicEntity & rE, short nAngle,
                          USHORT nHeight, double fWidthScale);

    void DrawLineEntity(const DXFLineEntity & rE, const DXFTransform & rTransform);

    void DrawPointEntity(const DXFPointEntity & rE, const DXFTransform & rTransform);

    void DrawCircleEntity(const DXFCircleEntity & rE, const DXFTransform & rTransform);

    void DrawArcEntity(const DXFArcEntity & rE, const DXFTransform & rTransform);

    void DrawTraceEntity(const DXFTraceEntity & rE, const DXFTransform & rTransform);

    void DrawSolidEntity(const DXFSolidEntity & rE, const DXFTransform & rTransform);

    void DrawTextEntity(const DXFTextEntity & rE, const DXFTransform & rTransform);

    void DrawInsertEntity(const DXFInsertEntity & rE, const DXFTransform & rTransform);

    void DrawAttribEntity(const DXFAttribEntity & rE, const DXFTransform & rTransform);

    void DrawPolyLineEntity(const DXFPolyLineEntity & rE, const DXFTransform & rTransform);

    void Draw3DFaceEntity(const DXF3DFaceEntity & rE, const DXFTransform & rTransform);

    void DrawDimensionEntity(const DXFDimensionEntity & rE, const DXFTransform & rTransform);

    void DrawEntities(const DXFEntities & rEntities,
                      const DXFTransform & rTransform,
                      BOOL bTopEntities);

public:

    DXF2GDIMetaFile();
    ~DXF2GDIMetaFile();

    BOOL Convert(const DXFRepresentation & rDXF, GDIMetaFile & rMTF,
                 PFilterCallback pCallback, void * pCallerData,
                 USHORT nMinPercent, USHORT nMaxPercent);

};


#endif


