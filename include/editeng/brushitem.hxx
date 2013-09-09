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
#ifndef _SVX_BRSHITEM_HXX
#define _SVX_BRSHITEM_HXX

#include <svl/poolitem.hxx>
#include <vcl/wall.hxx>
#include <tools/link.hxx>
#include <editeng/editengdllapi.h>

// class SvxBrushItem ----------------------------------------------------

class Graphic;
class GraphicObject;
class CntWallpaperItem;

#define BRUSH_GRAPHIC_VERSION   ((sal_uInt16)0x0001)

enum SvxGraphicPosition
{
    GPOS_NONE,
    GPOS_LT, GPOS_MT, GPOS_RT,
    GPOS_LM, GPOS_MM, GPOS_RM,
    GPOS_LB, GPOS_MB, GPOS_RB,
    GPOS_AREA, GPOS_TILED
};

#define PARA_DEST_PARA  0
#define PARA_DEST_CHAR  1

class SvxBrushItem_Impl;
class EDITENG_DLLPUBLIC SvxBrushItem : public SfxPoolItem
{
    Color               aColor;
    sal_Int32           nShadingValue;
    SvxBrushItem_Impl*  pImpl;
    OUString            maStrLink;
    OUString            maStrFilter;
    SvxGraphicPosition  eGraphicPos;
    sal_Bool            bLoadAgain;

    void        ApplyGraphicTransparency_Impl();
    DECL_STATIC_LINK( SvxBrushItem, DoneHdl_Impl, void *);
    // wird nur von Create benutzt
    SvxBrushItem( SvStream& rStrm,
                  sal_uInt16 nVersion, sal_uInt16 nWhich  );

public:
    TYPEINFO();

    explicit SvxBrushItem( sal_uInt16 nWhich );
    SvxBrushItem( const Color& rColor, sal_uInt16 nWhich  );

    SvxBrushItem( const Graphic& rGraphic,
                  SvxGraphicPosition ePos, sal_uInt16 nWhich );
    SvxBrushItem( const GraphicObject& rGraphicObj,
                  SvxGraphicPosition ePos, sal_uInt16 nWhich );
    SvxBrushItem( const OUString& rLink, const OUString& rFilter,
                  SvxGraphicPosition ePos, sal_uInt16 nWhich );
    SvxBrushItem( const SvxBrushItem& );
    SvxBrushItem( const CntWallpaperItem&, sal_uInt16 nWhich );

    ~SvxBrushItem();

public:

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 nVersion ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const;

    const Color&    GetColor() const                { return aColor; }
    Color&          GetColor()                      { return aColor; }
    void            SetColor( const Color& rCol)    { aColor = rCol; }

    void                SetDoneLink( const Link& rLink );

    SvxGraphicPosition  GetGraphicPos() const       { return eGraphicPos; }

    void                PurgeMedium() const;

    sal_uInt32              GetShadingValue() const     { return nShadingValue; }
    const Graphic*          GetGraphic() const;
    const GraphicObject*    GetGraphicObject() const;
    OUString                GetGraphicLink() const      { return maStrLink; }
    OUString                GetGraphicFilter() const    { return maStrFilter; }

    void                SetShadingValue( const sal_uInt32 nNew );
    void                SetGraphicPos( SvxGraphicPosition eNew );
    void                SetGraphic( const Graphic& rNew );
    void                SetGraphicObject( const GraphicObject& rNewObj );
    void                SetGraphicLink( const OUString& rNew );
    void                SetGraphicFilter( const OUString& rNew );

    SvxBrushItem&       operator=( const SvxBrushItem& rItem);

    static SvxGraphicPosition   WallpaperStyle2GraphicPos( WallpaperStyle eStyle );
    static WallpaperStyle       GraphicPos2WallpaperStyle( SvxGraphicPosition ePos );
    static sal_Int8             TransparencyToPercent(sal_Int32 nTrans);
};

#endif // #ifndef _SVX_BRSHITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
