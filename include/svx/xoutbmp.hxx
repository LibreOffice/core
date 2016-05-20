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

#ifndef INCLUDED_SVX_XOUTBMP_HXX
#define INCLUDED_SVX_XOUTBMP_HXX

#include <vcl/graph.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>

enum class XOutFlags {
    NONE                 = 0x00000000,
    MirrorHorz           = 0x00000001,
    MirrorVert           = 0x00000010,
    ContourHorz          = 0x00000001,
    ContourVert          = 0x00000002,
    ContourEdgeDetect    = 0x00000004,
    DontAddExtension     = 0x00000008,
    DontExpandFilename   = 0x00010000,
    UseGifIfPossible     = 0x00020000,
    UseGifIfSensible     = 0x00040000,
    UseNativeIfPossible  = 0x00080000,
};
namespace o3tl {
    template<> struct typed_flags<XOutFlags> : is_typed_flags<XOutFlags, 0x000f001f> {};
}

class GraphicFilter;
class VirtualDevice;
class INetURLObject;
namespace tools {
    class Polygon;
}

class SVX_DLLPUBLIC XOutBitmap
{
public:

    static GraphicFilter* pGrfFilter;

    static Graphic      MirrorGraphic( const Graphic& rGraphic, const BmpMirrorFlags nMirrorFlags );
    static Animation    MirrorAnimation( const Animation& rAnimation, bool bHMirr, bool bVMirr );
    static sal_uInt16   WriteGraphic( const Graphic& rGraphic, OUString& rFileName,
                                      const OUString& rFilterName, const XOutFlags nFlags = XOutFlags::NONE,
                                      const Size* pMtfSize_100TH_MM = nullptr );
    static bool         GraphicToBase64(const Graphic& rGraphic, OUString& rOUString);

    static sal_uInt16   ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                       GraphicFilter& rFilter, const sal_uInt16 nFormat,
                                       const css::uno::Sequence< css::beans::PropertyValue >* pFilterData = nullptr );

    static Bitmap       DetectEdges( const Bitmap& rBmp, const sal_uInt8 cThreshold );

    static tools::Polygon GetCountour( const Bitmap& rBmp, const XOutFlags nContourFlags,
                                       const sal_uInt8 cEdgeDetectThreshold = 50,
                                       const Rectangle* pWorkRect = nullptr );
};

SVX_DLLPUBLIC bool DitherBitmap( Bitmap& rBitmap );

#endif // INCLUDED_SVX_XOUTBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
