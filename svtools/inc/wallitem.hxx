/*************************************************************************
 *
 *  $RCSfile: wallitem.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
// -----------------------------------------------------------------------------------------

#ifndef _SFX_BRUSHITEMLINK

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _SV_WALL_HXX
#include <vcl/wall.hxx>
#endif

#include "poolitem.hxx"

class SvStream;
class Graphic;

DBG_NAMEEX(SfxWallpaperItem);

class SfxWallpaperItem : public SfxPoolItem
{
private:
    Wallpaper               _aWallpaper;
    XubString               _aURL;
    XubString               _aFilter;
    USHORT                  _nFlags;
    BOOL                    IsDownloaded() const;
    void                    Download();
public:
                            TYPEINFO();

                            SfxWallpaperItem( USHORT nWhich );
                            SfxWallpaperItem( USHORT nWhich, SvStream& rStream, USHORT nVersion );
                            SfxWallpaperItem( const SfxWallpaperItem& rCpy );
                            SfxWallpaperItem( USHORT nWhich, const Wallpaper& );
                            ~SfxWallpaperItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Create( SvStream&, USHORT nItemVersion ) const;
    virtual SvStream&       Store( SvStream&, USHORT nItemVersion ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal,
                                         BYTE nMemberId = 0 ) const;
    virtual BOOL            PutValue  ( const com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );

    void                    SetBitmapURL( const XubString& rURL, const XubString& rFilter );
    void                    SetGraphic( const Graphic& rGraphic, BOOL bKeepLink=FALSE );
    void                    SetColor( const Color& rColor ) { _aWallpaper.SetColor(rColor); }
    void                    SetStyle( WallpaperStyle eStyle ) { _aWallpaper.SetStyle(eStyle); }

    const Wallpaper&        GetWallpaper( BOOL bNeedsBitmap = TRUE ) const;
    const XubString&        GetBitmapURL() const { return _aURL; }
    const XubString&        GetBitmapFilter() const { return _aFilter; }
};

class WallpaperLoader_Impl;
class CntWallpaperItem;

class WallpaperLoader : public SvRefBase
{
    WallpaperLoader_Impl    *pImp;

public:

                        WallpaperLoader( const CntWallpaperItem& rItem);
                        ~WallpaperLoader();

    void                RequestData( const Link& rLink );
    const Wallpaper&    GetWallpaper() const;
    void                Done();
    void                SetBitmap( SvStream* pStream );
};

SV_DECL_REF( WallpaperLoader );
SV_IMPL_REF( WallpaperLoader );

#endif // _SFX_BRUSHITEM_LINK


////////////////////////////////////////////////////////////////////////////////
/*
    $Log: not supported by cvs2svn $
    Revision 1.15  2000/09/18 14:13:18  willem.vandorp
    OpenOffice header added.

    Revision 1.14  2000/08/31 13:41:16  willem.vandorp
    Header and footer replaced

    Revision 1.13  2000/03/22 11:47:48  kso
    Removed: SmartUno leftovers.

    Revision 1.12  2000/03/21 11:39:32  kso
    Added: [Put|QueryValue( ... com::sun::star::uno::Any ... )

    Revision 1.11  2000/02/09 16:16:37  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.10  1999/08/19 14:23:00  dv
    no include cntwall.hxx needed

    Revision 1.9  1999/08/18 23:33:43  hjs
    includes

    Revision 1.8  1999/08/18 09:17:45  dv
    #66082# The WallpaperLoader is now constructed with a CntWallpaperItem

    Revision 1.7  1999/06/25 09:02:25  dv
    #63380# WallpaperLoader neu

    Revision 1.6  1999/06/15 12:29:07  hjs
    once again...

    Revision 1.4  1998/08/13 15:17:28  DV
    #52764# Put/Query Value neu

      Rev 1.3   13 Aug 1998 17:17:28   DV
   #52764# Put/Query Value neu

      Rev 1.2   03 Jul 1998 09:42:08   OV
   Impl.

      Rev 1.1   01 Jul 1998 09:28:02   OV
   Erweiterungen, Umstellungen

      Rev 1.0   30 Jun 1998 10:50:56   OV
   Neu: SfxWallpaperItem

*/

#endif // _WALLITEM_HXX

