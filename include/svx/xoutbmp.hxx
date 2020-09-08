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
#include <vcl/errcode.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/salctype.hxx>


enum class XOutFlags {
    NONE                 = 0x00000000,
    MirrorHorz           = 0x00000001,
    MirrorVert           = 0x00000010,
    DontAddExtension     = 0x00000008,
    DontExpandFilename   = 0x00010000,
    UseGifIfPossible     = 0x00020000,
    UseGifIfSensible     = 0x00040000,
    UseNativeIfPossible  = 0x00080000,
};
namespace o3tl {
    template<> struct typed_flags<XOutFlags> : is_typed_flags<XOutFlags, 0x000f0019> {};
}

class GraphicFilter;
class VirtualDevice;
class INetURLObject;
namespace tools {
    class Polygon;
}

class SVXCORE_DLLPUBLIC XOutBitmap
{
public:

    static Graphic      MirrorGraphic( const Graphic& rGraphic, const BmpMirrorFlags nMirrorFlags );
    static Animation    MirrorAnimation( const Animation& rAnimation, bool bHMirr, bool bVMirr );
    static ErrCode      WriteGraphic( const Graphic& rGraphic, OUString& rFileName,
                                      const OUString& rFilterName, const XOutFlags nFlags,
                                      const Size* pMtfSize_100TH_MM = nullptr,
                                      const css::uno::Sequence< css::beans::PropertyValue >* pFilterData = nullptr);
    static bool GraphicToBase64(const Graphic& rGraphic, OUString& rOUString,
                                bool bAddPrefix = true,
                                ConvertDataFormat aTargetFormat = ConvertDataFormat::Unknown);

    static ErrCode      ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                       GraphicFilter& rFilter, const sal_uInt16 nFormat,
                                       const css::uno::Sequence< css::beans::PropertyValue >* pFilterData );
};

#endif // INCLUDED_SVX_XOUTBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
