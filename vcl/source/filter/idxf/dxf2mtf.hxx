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

#pragma once

#include <sal/config.h>

#include <optional>
#include <string_view>

#include "dxfreprd.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/font.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>

class DXF2GDIMetaFile {
private:

    VclPtr<VirtualDevice> pVirDev;
    const DXFRepresentation * pDXF;

    // The entities are walked twice: first only to measure the extents of what
    // is drawn, then to record the metafile.
    bool mbMeasure;
    basegfx::B2DRange maExtents;
    // projected drawing coordinates -> metafile coordinates
    basegfx::B2DHomMatrix maDeviceTransform;
    double mfDeviceScale;

    // the colour and line type of the innermost insert, for BYBLOCK
    std::optional<Color> moBlockColor;
    DXFLineInfo aBlockDXFLineInfo;
    tools::Long mnBlockLineWeight;
    // the layer of the innermost insert, for the entities on layer 0 of the block
    const DXFLayer * mpParentLayer;
    Color       aActLineColor;
    Color       aActFillColor;
    vcl::Font   aActFont;

    void ResetState();

    Color ConvertColor(tools::Long nColor) const;

    const DXFLayer * GetEntityLayer(const DXFBasicEntity & rE) const;

    // no colour if the entity is not visible
    std::optional<Color> GetEntityColor(const DXFBasicEntity & rE) const;

    DXFLineInfo LTypeToDXFLineInfo(std::string_view rLineType) const;

    DXFLineInfo GetEntityDXFLineInfo(const DXFBasicEntity & rE);

    tools::Long GetEntityLineWeight(const DXFBasicEntity & rE) const;

    // the line style of the entity in metafile units
    LineInfo GetEntityLineInfo(const DXFBasicEntity & rE, const DXFTransform & rTransform);

    bool SetLineAttribute(const DXFBasicEntity & rE);

    bool SetAreaAttribute(const DXFBasicEntity & rE);

    bool SetFontAttribute(const DXFBasicEntity & rE, Degree10 nOrientation,
                          sal_Int32 nHeight);

    // Output of projected drawing coordinates, or only measuring them in the first pass
    void OutPolyLine(const basegfx::B2DPolygon & rPolygon, const LineInfo & rLineInfo);
    void OutPolygon(const basegfx::B2DPolyPolygon & rPolyPolygon);
    void OutPoint(const basegfx::B2DPoint & rPoint);
    // rTextTransform maps text space, where the baseline runs along x and the
    // text height is 1, to drawing coordinates
    void OutText(const DXFBasicEntity & rE, const DXFTransform & rTextTransform,
                 std::string_view rText);
    // the sides of an entity with a thickness
    void OutThickness(const basegfx::B2DPolygon & rBase, const DXFTransform & rTransform,
                      double fThickness, bool bFilled);

    void DrawLineEntity(const DXFLineEntity & rE, const DXFTransform & rTransform);

    void DrawPointEntity(const DXFPointEntity & rE, const DXFTransform & rTransform);

    void DrawCircleEntity(const DXFCircleEntity & rE, const DXFTransform & rTransform);

    void DrawArcEntity(const DXFArcEntity & rE, const DXFTransform & rTransform);

    void DrawTraceEntity(const DXFTraceEntity & rE, const DXFTransform & rTransform);

    void DrawSolidEntity(const DXFSolidEntity & rE, const DXFTransform & rTransform);

    void DrawTextEntity(const DXFTextEntity & rE, const DXFTransform & rTransform);

    void DrawInsertEntity(const DXFInsertEntity & rE, const DXFTransform & rTransform);

    void DrawAttribEntity(const DXFAttribEntity & rE, const DXFTransform & rTransform);

    // metafile units per drawing unit
    double GetDeviceScale(const DXFTransform & rTransform) const;

    void Draw2DPolyLine(const DXFBasicEntity & rE, const std::vector<DXFPathVertex> & rVertices,
                        bool bClosed, double fElevation, const DXFTransform & rTransform);

    void DrawPolyLineEntity(const DXFPolyLineEntity & rE, const DXFTransform & rTransform);

    void Draw3DFaceEntity(const DXF3DFaceEntity & rE, const DXFTransform & rTransform);

    void DrawEllipseEntity(const DXFEllipseEntity & rE, const DXFTransform & rTransform);

    void DrawSplineEntity(const DXFSplineEntity & rE, const DXFTransform & rTransform);

    void DrawDimensionEntity(const DXFDimensionEntity & rE, const DXFTransform & rTransform);

    void DrawLWPolyLineEntity( const DXFLWPolyLineEntity & rE, const DXFTransform & rTransform );

    void DrawHatchEntity( const DXFHatchEntity & rE, const DXFTransform & rTransform );

    void DrawBlock(const DXFBlock & rBlock, const DXFBasicEntity & rE,
                   const DXFTransform & rTransform);

    void DrawEntities(const DXFEntities & rEntities,
                      const DXFTransform & rTransform);

public:

    DXF2GDIMetaFile();
    ~DXF2GDIMetaFile();

    bool Convert( const DXFRepresentation & rDXF, GDIMetaFile & rMTF );

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
