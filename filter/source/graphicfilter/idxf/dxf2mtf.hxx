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

#ifndef _DXF2MTF_HXX
#define _DXF2MTF_HXX

#include "dxfreprd.hxx"
#include <vcl/font.hxx>
#include <vcl/lineinfo.hxx>

// MT: NOOLDSV, someone should change the code...
enum PenStyle { PEN_NULL, PEN_SOLID, PEN_DOT, PEN_DASH, PEN_DASHDOT };
enum BrushStyle { BRUSH_NULL, BRUSH_SOLID, BRUSH_HORZ, BRUSH_VERT,
                  BRUSH_CROSS, BRUSH_DIAGCROSS, BRUSH_UPDIAG, BRUSH_DOWNDIAG,
                  BRUSH_25, BRUSH_50, BRUSH_75,
                  BRUSH_BITMAP };


class DXF2GDIMetaFile {
private:

    VirtualDevice * pVirDev;
    const DXFRepresentation * pDXF;
    BOOL bStatus;

    USHORT OptPointsPerCircle;

    ULONG nMinPercent;
    ULONG nMaxPercent;
    ULONG nLastPercent;
    ULONG nMainEntitiesCount;

    long		nBlockColor;
    DXFLineInfo	aBlockDXFLineInfo;
    long		nParentLayerColor;
    DXFLineInfo	aParentLayerDXFLineInfo;
    Color		aActLineColor;
    Color		aActFillColor;
    Font		aActFont;

    ULONG CountEntities(const DXFEntities & rEntities);

    void MayCallback(ULONG nMainEntitiesProcessed);

    Color ConvertColor(BYTE nColor);

    long GetEntityColor(const DXFBasicEntity & rE);

    DXFLineInfo LTypeToDXFLineInfo(const char * sLineType);

    DXFLineInfo GetEntityDXFLineInfo(const DXFBasicEntity & rE);

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

    void DrawLWPolyLineEntity( const DXFLWPolyLineEntity & rE, const DXFTransform & rTransform );

    void DrawHatchEntity( const DXFHatchEntity & rE, const DXFTransform & rTransform );

    void DrawEntities(const DXFEntities & rEntities,
                      const DXFTransform & rTransform,
                      BOOL bTopEntities);

public:

    DXF2GDIMetaFile();
    ~DXF2GDIMetaFile();

    BOOL Convert( const DXFRepresentation & rDXF, GDIMetaFile & rMTF, USHORT nMinPercent, USHORT nMaxPercent);

};


#endif


