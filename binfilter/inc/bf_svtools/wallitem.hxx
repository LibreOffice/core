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
#ifndef _WALLITEM_HXX
#define _WALLITEM_HXX

#ifndef SHL_HXX
#include <tools/shl.hxx>
#endif

// -----------------------------------------------------------------------------------------
// Hilfsklasse, um die Download-Funktionalitaet des SvxBrushItems unterhalb
// des SVX zu benutzen. Der Link wird im Konstruktor von SvxDialogDll gesetzt.
#ifndef _SFX_BRUSHITEMLINK_DECLARED
#define _SFX_BRUSHITEMLINK_DECLARED
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

#ifndef SHL_BRUSHITEM
#define SHL_BRUSHITEM SHL_SHL1
#endif

class Window;
class Graphic;
class String;

namespace binfilter
{
class SfxItemSet;

typedef void* (*CreateSvxBrushTabPage)(Window *pParent, const SfxItemSet &rAttrSet);
typedef USHORT*	(*GetSvxBrushTabPageRanges)();
class SfxBrushItemLink
{
public:
    virtual Graphic GetGraphic( const String& rLink, const String& rFilter) = 0;
    virtual CreateSvxBrushTabPage GetBackgroundTabpageCreateFunc() = 0;
    virtual GetSvxBrushTabPageRanges GetBackgroundTabpageRanges() = 0;
    static SfxBrushItemLink* Get() { return *(SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM);	}
};

}

#endif	// _SFX_BRUSHITEMLINK_DECLARED

#endif // _WALLITEM_HXX

