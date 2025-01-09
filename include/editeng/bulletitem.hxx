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
#ifndef INCLUDED_EDITENG_BULLETITEM_HXX
#define INCLUDED_EDITENG_BULLETITEM_HXX

#include <memory>
#include <editeng/editengdllapi.h>
#include <svl/poolitem.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/font.hxx>


/**
 * these must match the values in css::style::NumberingType
 */
enum class SvxBulletStyle
{
    ABC_BIG          = 0,
    ABC_SMALL        = 1,
    ROMAN_BIG        = 2,
    ROMAN_SMALL      = 3,
    N123             = 4,
    NONE             = 5,
    BULLET           = 6,
    BMP              = 128
};

// class SvxBulletItem ---------------------------------------------------

class EDITENG_DLLPUBLIC SvxBulletItem final : public SfxPoolItem
{
    vcl::Font       aFont;
    std::unique_ptr<GraphicObject>
                    pGraphicObject;
    OUString        aPrevText;
    OUString        aFollowText;
    sal_uInt16      nStart;
    SvxBulletStyle  nStyle;
    tools::Long            nWidth;
    sal_uInt16      nScale;
    sal_Unicode     cSymbol;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxBulletItem)
    explicit SvxBulletItem( sal_uInt16 nWhich );
    SvxBulletItem( const SvxBulletItem& );
    virtual ~SvxBulletItem() override;

    virtual SvxBulletItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    OUString            GetFullText() const;

    tools::Long                GetWidth() const { return nWidth; }
    const vcl::Font&    GetFont() const { return aFont; }

    const GraphicObject& GetGraphicObject() const;
    void                 SetGraphicObject( const GraphicObject& rGraphicObject );

    void                SetSymbol( sal_Unicode c) { cSymbol = c; }

    void                SetStart( sal_uInt16 nNew ) { nStart = nNew; }
    void                SetWidth( tools::Long nNew ) { nWidth = nNew; }
    void                SetStyle( SvxBulletStyle nNew ) { nStyle = nNew; }
    void                SetFont( const vcl::Font& rNew) { aFont = rNew; }
    void                SetScale( sal_uInt16 nNew ) { nScale = nNew; }

    virtual bool        operator==( const SfxPoolItem& ) const override;
    virtual bool        GetPresentation( SfxItemPresentation ePres,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper& ) const override;

    void                CopyValidProperties( const SvxBulletItem& rCopyFrom );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
