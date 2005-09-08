/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wallitem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:15:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
class SfxItemSet;

typedef void* (*CreateSvxBrushTabPage)(Window *pParent, const SfxItemSet &rAttrSet);
typedef USHORT* (*GetSvxBrushTabPageRanges)();

class Graphic;
class String;
class SfxBrushItemLink
{
public:
    virtual Graphic GetGraphic( const String& rLink, const String& rFilter) = 0;
    virtual CreateSvxBrushTabPage GetBackgroundTabpageCreateFunc() = 0;
    virtual GetSvxBrushTabPageRanges GetBackgroundTabpageRanges() = 0;
    static SfxBrushItemLink* Get() { return *(SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM); }
    static void Set( SfxBrushItemLink* pLink );
};
#endif  // _SFX_BRUSHITEMLINK_DECLARED

#endif // _WALLITEM_HXX

