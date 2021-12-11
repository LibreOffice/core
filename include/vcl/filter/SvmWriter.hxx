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

#include <memory>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

class SvStream;

class VCL_DLLPUBLIC SvmWriter
{
private:
    SvStream& mrStream;

    void WriteColor(::Color aColor);

public:
    SvmWriter(SvStream& rIStm);

    SvStream& Write(const GDIMetaFile& rMetaFile);
    static BitmapChecksum GetChecksum(const GDIMetaFile& rMetaFile);
    void MetaActionHandler(MetaAction* pAction, ImplMetaWriteData* pData);
    void ActionHandler(const MetaAction* pAction);
    void PixelHandler(const MetaPixelAction* pAction);
    void PointHandler(const MetaPointAction* pAction);
    void LineHandler(const MetaLineAction* pAction);
    void RectHandler(const MetaRectAction* pAction);
    void RoundRectHandler(const MetaRoundRectAction* pAction);
    void EllipseHandler(const MetaEllipseAction* pAction);
    void ArcHandler(const MetaArcAction* pAction);
    void PieHandler(const MetaPieAction* pAction);
    void ChordHandler(const MetaChordAction* pAction);
    void PolyLineHandler(const MetaPolyLineAction* pAction);
    void PolygonHandler(const MetaPolygonAction* pAction);
    void PolyPolygonHandler(const MetaPolyPolygonAction* pAction);
    void TextHandler(const MetaTextAction* pAction, const ImplMetaWriteData* pData);
    void TextArrayHandler(const MetaTextArrayAction* pAction, const ImplMetaWriteData* pData);
    void StretchTextHandler(const MetaStretchTextAction* pAction, const ImplMetaWriteData* pData);
    void TextRectHandler(const MetaTextRectAction* pAction, const ImplMetaWriteData* pData);
    void TextLineHandler(const MetaTextLineAction* pAction);
    void BmpHandler(const MetaBmpAction* pAction);
    void BmpScaleHandler(const MetaBmpScaleAction* pAction);
    void BmpScalePartHandler(const MetaBmpScalePartAction* pAction);
    void BmpExHandler(const MetaBmpExAction* pAction);
    void BmpExScaleHandler(const MetaBmpExScaleAction* pAction);
    void BmpExScalePartHandler(const MetaBmpExScalePartAction* pAction);
    void MaskHandler(const MetaMaskAction* pAction);
    void MaskScaleHandler(const MetaMaskScaleAction* pAction);
    void MaskScalePartHandler(const MetaMaskScalePartAction* pAction);
    void GradientHandler(const MetaGradientAction* pAction);
    void GradientExHandler(const MetaGradientExAction* pAction);
    void HatchHandler(const MetaHatchAction* pAction);
    void WallpaperHandler(const MetaWallpaperAction* pAction);
    void ClipRegionHandler(const MetaClipRegionAction* pAction);
    void ISectRectClipRegionHandler(const MetaISectRectClipRegionAction* pAction);
    void ISectRegionClipRegionHandler(const MetaISectRegionClipRegionAction* pAction);
    void MoveClipRegionHandler(const MetaMoveClipRegionAction* pAction);
    void LineColorHandler(const MetaLineColorAction* pAction);
    void FillColorHandler(const MetaFillColorAction* pAction);
    void TextColorHandler(const MetaTextColorAction* pAction);
    void TextFillColorHandler(const MetaTextFillColorAction* pAction);
    void TextLineColorHandler(const MetaTextLineColorAction* pAction);
    void OverlineColorHandler(const MetaOverlineColorAction* pAction);
    void TextAlignHandler(const MetaTextAlignAction* pAction);
    void MapModeHandler(const MetaMapModeAction* pAction);
    void FontHandler(const MetaFontAction* pAction, ImplMetaWriteData* pData);
    void PushHandler(const MetaPushAction* pAction);
    void PopHandler(const MetaPopAction* pAction);
    void RasterOpHandler(const MetaRasterOpAction* pAction);
    void TransparentHandler(const MetaTransparentAction* pAction);
    void FloatTransparentHandler(const MetaFloatTransparentAction* pAction);
    void EPSHandler(const MetaEPSAction* pAction);
    void RefPointHandler(const MetaRefPointAction* pAction);
    void CommentHandler(const MetaCommentAction* pAction);
    void LayoutModeHandler(const MetaLayoutModeAction* pAction);
    void TextLanguageHandler(const MetaTextLanguageAction* pAction);
    void LinearGradientHandler(const MetaLinearGradientAction* pAction);
    void ComplexGradientHandler(const MetaComplexGradientAction* pAction);
};
