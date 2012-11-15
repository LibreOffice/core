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
#ifndef _WALLITEM_HXX
#define _WALLITEM_HXX

#include <tools/shl.hxx>

// -----------------------------------------------------------------------------------------
// Hilfsklasse, um die Download-Funktionalitaet des SvxBrushItems unterhalb
// des SVX zu benutzen. Der Link wird im Konstruktor von SvxDialogDll gesetzt.
#ifndef _SFX_BRUSHITEMLINK_DECLARED
#define _SFX_BRUSHITEMLINK_DECLARED
#include <vcl/graph.hxx>

#ifndef SHL_BRUSHITEM
#define SHL_BRUSHITEM SHL_SHL1
#endif

class Window;
class SfxItemSet;

typedef void* (*CreateSvxBrushTabPage)(Window *pParent, const SfxItemSet &rAttrSet);
typedef sal_uInt16* (*GetSvxBrushTabPageRanges)();

class Graphic;
class String;
class SfxBrushItemLink
{
public:
    virtual ~SfxBrushItemLink() = 0;
    virtual Graphic GetGraphic( const String& rLink, const String& rFilter) = 0;
    virtual CreateSvxBrushTabPage GetBackgroundTabpageCreateFunc() = 0;
    virtual GetSvxBrushTabPageRanges GetBackgroundTabpageRanges() = 0;
    static SfxBrushItemLink* Get() { return *(SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM); }
    //static void Set( SfxBrushItemLink* pLink );
};
#endif  // _SFX_BRUSHITEMLINK_DECLARED

#endif // _WALLITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
