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

protected:
    void WriteColor(::Color aColor);

public:
    SvmWriter(SvStream& rIStm);

    SvStream& Write(const GDIMetaFile& rMetaFile);
    static BitmapChecksum GetChecksum(const GDIMetaFile& rMetaFile);
    void MetaActionHandler(MetaAction* pAction, ImplMetaWriteData* pData);
    void ActionHandler(MetaAction* pAction);
    void PixelHandler(MetaPixelAction* pAction);
    void PointHandler(MetaPointAction* pAction);
    void LineHandler(MetaLineAction* pAction);
    void RectHandler(MetaRectAction* pAction);
    void RoundRectHandler(MetaRoundRectAction* pAction);
    void EllipseHandler(MetaEllipseAction* pAction);
    void ArcHandler(MetaArcAction* pAction);
    void PieHandler(MetaPieAction* pAction);
    void ChordHandler(MetaChordAction* pAction);
    void PolyLineHandler(MetaPolyLineAction* pAction);
    void PolygonHandler(MetaPolygonAction* pAction);
    void PolyPolygonHandler(MetaPolyPolygonAction* pAction);
    void TextHandler(MetaTextAction* pAction, ImplMetaWriteData* pData);
    void TextArrayHandler(MetaTextArrayAction* pAction, ImplMetaWriteData* pData);
    void StretchTextHandler(MetaStretchTextAction* pAction, ImplMetaWriteData* pData);
    void TextRectHandler(MetaTextRectAction* pAction, ImplMetaWriteData* pData);
    void TextLineHandler(MetaTextLineAction* pAction);
    void BmpHandler(MetaBmpAction* pAction);
    void BmpScaleHandler(MetaBmpScaleAction* pAction);
    void BmpScalePartHandler(MetaBmpScalePartAction* pAction);
    void BmpExHandler(MetaBmpExAction* pAction);
    void BmpExScaleHandler(MetaBmpExScaleAction* pAction);
    void BmpExScalePartHandler(MetaBmpExScalePartAction* pAction);
    void MaskHandler(MetaMaskAction* pAction);
    void MaskScaleHandler(MetaMaskScaleAction* pAction);
    void MaskScalePartHandler(MetaMaskScalePartAction* pAction);
    void GradientHandler(MetaGradientAction* pAction);
    void GradientExHandler(MetaGradientExAction* pAction);
    void HatchHandler(MetaHatchAction* pAction);
    void WallpaperHandler(MetaWallpaperAction* pAction);
    void ClipRegionHandler(MetaClipRegionAction* pAction);
    void ISectRectClipRegionHandler(MetaISectRectClipRegionAction* pAction);
    void ISectRegionClipRegionHandler(MetaISectRegionClipRegionAction* pAction);
    void MoveClipRegionHandler(MetaMoveClipRegionAction* pAction);
    void LineColorHandler(MetaLineColorAction* pAction);
    void FillColorHandler(MetaFillColorAction* pAction);
    void TextColorHandler(MetaTextColorAction* pAction);
    void TextFillColorHandler(MetaTextFillColorAction* pAction);
    void TextLineColorHandler(MetaTextLineColorAction* pAction);
    void OverlineColorHandler(MetaOverlineColorAction* pAction);
    void TextAlignHandler(MetaTextAlignAction* pAction);
    void MapModeHandler(MetaMapModeAction* pAction);
    void FontHandler(MetaFontAction* pAction, ImplMetaWriteData* pData);
    void PushHandler(MetaPushAction* pAction);
    void PopHandler(MetaPopAction* pAction);
    void RasterOpHandler(MetaRasterOpAction* pAction);
    void TransparentHandler(MetaTransparentAction* pAction);
    void FloatTransparentHandler(MetaFloatTransparentAction* pAction);
    void EPSHandler(MetaEPSAction* pAction);
    void RefPointHandler(MetaRefPointAction* pAction);
    void CommentHandler(MetaCommentAction* pAction);
    void LayoutModeHandler(MetaLayoutModeAction* pAction);
    void TextLanguageHandler(MetaTextLanguageAction* pAction);
};