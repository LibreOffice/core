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

#include <editeng/editengdllapi.h>
#include <svl/poolitem.hxx>
#include <svtools/grfmgr.hxx>
#include <vcl/font.hxx>


// Styles
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

class EDITENG_DLLPUBLIC SvxBulletItem : public SfxPoolItem
{
    vcl::Font       aFont;
    GraphicObject*  pGraphicObject;
    OUString        aPrevText;
    OUString        aFollowText;
    sal_uInt16      nStart;
    SvxBulletStyle  nStyle;
    long            nWidth;
    sal_uInt16      nScale;
    sal_Unicode     cSymbol;

    void    SetDefaultFont_Impl();
    void    SetDefaults_Impl();

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxBulletItem( sal_uInt16 nWhich = 0 );
    explicit SvxBulletItem( SvStream& rStrm, sal_uInt16 nWhich = 0 );
    SvxBulletItem( const SvxBulletItem& );
    virtual ~SvxBulletItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nVersion ) const override;
    virtual SvStream&       Store( SvStream & , sal_uInt16 nItemVersion ) const override;

    OUString            GetFullText() const;
    sal_Unicode         GetSymbol() const { return cSymbol; }
    OUString            GetPrevText() const { return aPrevText; }
    OUString            GetFollowText() const { return aFollowText; }

    sal_uInt16          GetStart() const { return nStart; }
    long                GetWidth() const { return nWidth; }
    SvxBulletStyle      GetStyle() const { return nStyle; }
    vcl::Font           GetFont() const { return aFont; }
    sal_uInt16          GetScale() const { return nScale; }

    const GraphicObject& GetGraphicObject() const;
    void                 SetGraphicObject( const GraphicObject& rGraphicObject );

    void                SetSymbol( sal_Unicode c) { cSymbol = c; }
    void                SetPrevText( const OUString& rStr) { aPrevText = rStr;}
    void                SetFollowText(const OUString& rStr) { aFollowText=rStr;}

    void                SetStart( sal_uInt16 nNew ) { nStart = nNew; }
    void                SetWidth( long nNew ) { nWidth = nNew; }
    void                SetStyle( SvxBulletStyle nNew ) { nStyle = nNew; }
    void                SetFont( const vcl::Font& rNew) { aFont = rNew; }
    void                SetScale( sal_uInt16 nNew ) { nScale = nNew; }

    virtual sal_uInt16  GetVersion(sal_uInt16 nFileVersion) const override;
    virtual bool        operator==( const SfxPoolItem& ) const override;
    virtual bool        GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    static void         StoreFont( SvStream&, const vcl::Font& );
    static vcl::Font    CreateFont( SvStream&, sal_uInt16 nVer );

    void                CopyValidProperties( const SvxBulletItem& rCopyFrom );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
