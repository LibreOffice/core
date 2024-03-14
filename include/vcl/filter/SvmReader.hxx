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

#include <vcl/gdimtf.hxx>

class SvStream;

class SvmReader
{
private:
    SvStream& mrStream;

    void ReadColor(::Color& rColor);

public:
    VCL_DLLPUBLIC SvmReader(SvStream& rIStm);

    VCL_DLLPUBLIC SvStream& Read(GDIMetaFile& rMetaFile, ImplMetaReadData* pData = nullptr);
    rtl::Reference<MetaAction> MetaActionHandler(ImplMetaReadData* pData);
    rtl::Reference<MetaAction> LineColorHandler();
    rtl::Reference<MetaAction> FillColorHandler();
    rtl::Reference<MetaAction> RectHandler();
    rtl::Reference<MetaAction> PointHandler();
    rtl::Reference<MetaAction> PixelHandler();
    rtl::Reference<MetaAction> LineHandler();
    rtl::Reference<MetaAction> RoundRectHandler();
    rtl::Reference<MetaAction> EllipseHandler();
    rtl::Reference<MetaAction> ArcHandler();
    rtl::Reference<MetaAction> PieHandler();
    rtl::Reference<MetaAction> ChordHandler();
    rtl::Reference<MetaAction> PolyLineHandler();
    rtl::Reference<MetaAction> PolygonHandler();
    rtl::Reference<MetaAction> PolyPolygonHandler();
    rtl::Reference<MetaAction> TextHandler(const ImplMetaReadData* pData);
    rtl::Reference<MetaAction> TextArrayHandler(const ImplMetaReadData* pData);
    rtl::Reference<MetaAction> StretchTextHandler(const ImplMetaReadData* pData);
    rtl::Reference<MetaAction> TextRectHandler(const ImplMetaReadData* pData);
    rtl::Reference<MetaAction> TextLineHandler();
    rtl::Reference<MetaAction> BmpHandler();
    rtl::Reference<MetaAction> BmpScaleHandler();
    rtl::Reference<MetaAction> BmpScalePartHandler();
    rtl::Reference<MetaAction> BmpExHandler();
    rtl::Reference<MetaAction> BmpExScaleHandler();
    rtl::Reference<MetaAction> BmpExScalePartHandler();
    rtl::Reference<MetaAction> MaskHandler();
    rtl::Reference<MetaAction> MaskScaleHandler();
    rtl::Reference<MetaAction> MaskScalePartHandler();
    rtl::Reference<MetaAction> GradientHandler();
    rtl::Reference<MetaAction> GradientExHandler();
    rtl::Reference<MetaAction> HatchHandler();
    rtl::Reference<MetaAction> WallpaperHandler();
    rtl::Reference<MetaAction> ClipRegionHandler();
    rtl::Reference<MetaAction> ISectRectClipRegionHandler();
    rtl::Reference<MetaAction> ISectRegionClipRegionHandler();
    rtl::Reference<MetaAction> MoveClipRegionHandler();
    rtl::Reference<MetaAction> TextColorHandler();
    rtl::Reference<MetaAction> TextFillColorHandler();
    rtl::Reference<MetaAction> TextLineColorHandler();
    rtl::Reference<MetaAction> OverlineColorHandler();
    rtl::Reference<MetaAction> TextAlignHandler();
    rtl::Reference<MetaAction> MapModeHandler();
    rtl::Reference<MetaAction> FontHandler(ImplMetaReadData* pData);
    rtl::Reference<MetaAction> PushHandler();
    rtl::Reference<MetaAction> PopHandler();
    rtl::Reference<MetaAction> RasterOpHandler();
    rtl::Reference<MetaAction> TransparentHandler();
    rtl::Reference<MetaAction> FloatTransparentHandler(ImplMetaReadData* pData);
    rtl::Reference<MetaAction> EPSHandler();
    rtl::Reference<MetaAction> RefPointHandler();
    rtl::Reference<MetaAction> CommentHandler();
    rtl::Reference<MetaAction> LayoutModeHandler();
    rtl::Reference<MetaAction> TextLanguageHandler();
    static rtl::Reference<MetaAction> DefaultHandler();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
