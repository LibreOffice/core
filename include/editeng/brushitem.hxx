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
#ifndef INCLUDED_EDITENG_BRUSHITEM_HXX
#define INCLUDED_EDITENG_BRUSHITEM_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <unotools/securityoptions.hxx>
#include <editeng/editengdllapi.h>
#include <memory>

// class SvxBrushItem ----------------------------------------------------

class Graphic;
class GraphicObject;

constexpr sal_uInt16 BRUSH_GRAPHIC_VERSION = 0x0001;

enum SvxGraphicPosition
{
    GPOS_NONE,
    GPOS_LT, GPOS_MT, GPOS_RT,
    GPOS_LM, GPOS_MM, GPOS_RM,
    GPOS_LB, GPOS_MB, GPOS_RB,
    GPOS_AREA, GPOS_TILED
};

class EDITENG_DLLPUBLIC SvxBrushItem final : public SfxPoolItem
{
    Color               aColor;
    sal_Int32           nShadingValue;
    mutable std::unique_ptr<GraphicObject> xGraphicObject;
    sal_Int8            nGraphicTransparency; //contains a percentage value which is
                                              //copied to the GraphicObject when necessary
    SvtSecurityOptions  maSecOptions;
    OUString            maStrLink;
    OUString            maStrFilter;
    SvxGraphicPosition  eGraphicPos;
    mutable bool        bLoadAgain;

    void        ApplyGraphicTransparency_Impl();

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxBrushItem( sal_uInt16 nWhich );
    SvxBrushItem( const Color& rColor, sal_uInt16 nWhich  );

    SvxBrushItem( const Graphic& rGraphic,
                  SvxGraphicPosition ePos, sal_uInt16 nWhich );
    SvxBrushItem( const GraphicObject& rGraphicObj,
                  SvxGraphicPosition ePos, sal_uInt16 nWhich );
    SvxBrushItem( const OUString& rLink, const OUString& rFilter,
                  SvxGraphicPosition ePos, sal_uInt16 nWhich );
    SvxBrushItem( const SvxBrushItem& );
    SvxBrushItem(SvxBrushItem&&);

    virtual ~SvxBrushItem() override;

public:
    // check if it's used
    bool isUsed() const;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual SvxBrushItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const Color&    GetColor() const                { return aColor; }
    Color&          GetColor()                      { return aColor; }
    void            SetColor( const Color& rCol)    { aColor = rCol; }

    SvxGraphicPosition  GetGraphicPos() const       { return eGraphicPos; }

    sal_Int32               GetShadingValue() const     { return nShadingValue; }
    const Graphic*          GetGraphic(OUString const & referer = OUString()/*TODO*/) const;
    const GraphicObject*    GetGraphicObject(OUString const & referer = OUString()/*TODO*/) const;
    const OUString&         GetGraphicLink() const      { return maStrLink; }
    const OUString&         GetGraphicFilter() const    { return maStrFilter; }

    // get graphic transparency in percent
    sal_Int8 getGraphicTransparency() const { return nGraphicTransparency; }
    void setGraphicTransparency(sal_Int8 nNew);

    void                SetGraphicPos( SvxGraphicPosition eNew );
    void                SetGraphic( const Graphic& rNew );
    void                SetGraphicObject( const GraphicObject& rNewObj );
    void                SetGraphicLink( const OUString& rNew );
    void                SetGraphicFilter( const OUString& rNew );

    static sal_Int8             TransparencyToPercent(sal_Int32 nTrans);

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif // INCLUDED_EDITENG_BRUSHITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
