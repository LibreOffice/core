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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXF2MTF_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXF2MTF_HXX

#include "dxfreprd.hxx"
#include <vcl/font.hxx>
#include <vcl/lineinfo.hxx>

enum PenStyle { PEN_NULL, PEN_SOLID, PEN_DOT, PEN_DASH, PEN_DASHDOT };
enum BrushStyle { BRUSH_NULL, BRUSH_SOLID, BRUSH_HORZ, BRUSH_VERT,
                  BRUSH_CROSS, BRUSH_DIAGCROSS, BRUSH_UPDIAG, BRUSH_DOWNDIAG,
                  BRUSH_25, BRUSH_50, BRUSH_75 };


class DXF2GDIMetaFile {
private:

    VclPtr<VirtualDevice> pVirDev;
    const DXFRepresentation * pDXF;
    bool bStatus;

    sal_uInt16 OptPointsPerCircle;

    sal_uLong nMinPercent;
    sal_uLong nMaxPercent;
    sal_uLong nLastPercent;
    sal_uLong nMainEntitiesCount;

    long        nBlockColor;
    DXFLineInfo aBlockDXFLineInfo;
    long        nParentLayerColor;
    DXFLineInfo aParentLayerDXFLineInfo;
    Color       aActLineColor;
    Color       aActFillColor;
    vcl::Font   aActFont;

    static sal_uLong CountEntities(const DXFEntities & rEntities);

    Color ConvertColor(sal_uInt8 nColor);

    long GetEntityColor(const DXFBasicEntity & rE);

    DXFLineInfo LTypeToDXFLineInfo(OString const& rLineType);

    DXFLineInfo GetEntityDXFLineInfo(const DXFBasicEntity & rE);

    bool SetLineAttribute(const DXFBasicEntity & rE, sal_uLong nWidth=0);

    bool SetAreaAttribute(const DXFBasicEntity & rE);

    bool SetFontAttribute(const DXFBasicEntity & rE, short nAngle,
                          sal_uInt16 nHeight, double fWidthScale);

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
                      const DXFTransform & rTransform);

public:

    DXF2GDIMetaFile();
    ~DXF2GDIMetaFile();

    bool Convert( const DXFRepresentation & rDXF, GDIMetaFile & rMTF, sal_uInt16 nMinPercent, sal_uInt16 nMaxPercent);

};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
