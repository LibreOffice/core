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

#include <sal/config.h>

#include <optional>
#include <map>
#include <string_view>

#include "dxfreprd.hxx"
#include "dxfmtext.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/font.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>

// the properties of TEXT and ATTRIB entities
struct DXFSingleLineText
{
    const OString& maText;
    const OString& maStyle;
    DXFVector maPoint;
    DXFVector maAlignPoint;
    bool mbHasAlignPoint;
    double mfHeight;
    double mfWidthFactor;
    double mfRotation;
    double mfOblique;
    // 0 left, 1 center, 2 right, 3 aligned, 4 middle, 5 fit
    tools::Long mnHorizontal;
    // 0 baseline, 1 bottom, 2 middle, 3 top
    tools::Long mnVertical;
};

class DXF2GDIMetaFile {
private:

    VclPtr<VirtualDevice> pVirDev;
    // for measuring text, never recorded
    ScopedVclPtr<VirtualDevice> mpMeasureDev;
    // the cap height of fonts relative to their size
    std::map<std::pair<OUString, bool>, double> maCapHeightRatios;
    const DXFRepresentation * pDXF;

    // The entities are walked twice: first only to measure the extents of what
    // is drawn, then to record the metafile.
    bool mbMeasure;
    basegfx::B2DRange maExtents;
    basegfx::B2DRange maPointExtents;
    // projected drawing coordinates -> metafile coordinates
    basegfx::B2DHomMatrix maDeviceTransform;
    double mfDeviceScale;
    // the amount of output in the current pass and the nesting of inserts
    sal_uInt64 mnWork;
    sal_uInt64 mnVisits;
    sal_Int32 mnInsertDepth;

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

    // the colour of the entity if it had the colour index nColor and the true colour nTrueColor
    std::optional<Color> ResolveColor(const DXFBasicEntity & rE, tools::Long nColor,
                                      tools::Long nTrueColor) const;

    // the character format of a text style
    DXFTextFormat GetStyleFormat(std::string_view rStyle) const;

    void DrawSingleLineText(const DXFBasicEntity & rE, const DXFSingleLineText & rText,
                            const DXFTransform & rTransform);

    static vcl::Font MakeFont(const DXFTextFormat & rFormat, double fSize);

    double GetCapHeightRatio(const DXFTextFormat & rFormat);

    // the width of a text in drawing units
    double MeasureText(std::u16string_view aText, const DXFTextFormat & rFormat);

    // Outputs text with its baseline starting at (fX, fY) of the text space, which
    // rTextTransform maps to drawing coordinates.
    void OutTextRun(const DXFBasicEntity & rE, const DXFTransform & rTextTransform, double fX,
                    double fY, const OUString & rText, const DXFTextFormat & rFormat,
                    double fWidth);

    DXFLineInfo LTypeToDXFLineInfo(std::string_view rLineType) const;

    DXFLineInfo GetEntityDXFLineInfo(const DXFBasicEntity & rE);

    tools::Long GetEntityLineWeight(const DXFBasicEntity & rE) const;

    // the line style of the entity in metafile units
    LineInfo GetEntityLineInfo(const DXFBasicEntity & rE, const DXFTransform & rTransform);

    bool SetLineAttribute(const DXFBasicEntity & rE);

    bool SetAreaAttribute(const DXFBasicEntity & rE);

    // false when the pass has done too much already
    bool AddWork(sal_uInt64 nWork);
    bool AddVisit();
    bool IsOverBudget() const;

    // Output of projected drawing coordinates, or only measuring them in the first pass
    void OutPolyLine(const basegfx::B2DPolygon & rPolygon, const LineInfo & rLineInfo);
    void OutPolygon(const basegfx::B2DPolyPolygon & rPolyPolygon);
    void OutPoint(const basegfx::B2DPoint & rPoint);

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

    void DrawMTextEntity(const DXFMTextEntity & rE, const DXFTransform & rTransform);

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
