/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVX_BULITEM_HXX
#define _SVX_BULITEM_HXX

#include "editeng/editengdllapi.h"


// define ----------------------------------------------------------------

// Styles
#define BS_ABC_BIG          0
#define BS_ABC_SMALL        1
#define BS_ROMAN_BIG        2
#define BS_ROMAN_SMALL      3
#define BS_123              4
#define BS_NONE             5
#define BS_BULLET           6
#define BS_BMP              128

// Justification
#define BJ_HLEFT            0x01
#define BJ_HRIGHT           0x02
#define BJ_HCENTER          0x04
#define BJ_VTOP             0x08
#define BJ_VBOTTOM          0x10
#define BJ_VCENTER          0x20

// Valid-Bits
// First, only the values that are changed by the dialogue ...
#define VALID_FONTCOLOR     0x0001
#define VALID_FONTNAME      0x0002
#define VALID_SYMBOL        0x0004
#define VALID_BITMAP        0x0008
#define VALID_SCALE         0x0010
#define VALID_START         0x0020
#define VALID_STYLE         0x0040
#define VALID_PREVTEXT      0x0080
#define VALID_FOLLOWTEXT    0x0100
#include <svl/poolitem.hxx>
#include <vcl/font.hxx>
#include <svtools/grfmgr.hxx>

// class SvxBulletItem ---------------------------------------------------

class EDITENG_DLLPUBLIC SvxBulletItem : public SfxPoolItem
{
    Font            aFont;
    GraphicObject*  pGraphicObject;
    String          aPrevText;
    String          aFollowText;
    sal_uInt16          nStart;
    sal_uInt16          nStyle;
    long            nWidth;
    sal_uInt16          nScale;
    sal_Unicode     cSymbol;
    sal_uInt8       nJustify;
    sal_uInt16      nValidMask; // Only temporary for GetAttribs / setAttribs,
                                // because of the large Bullets

#ifdef _SVX_BULITEM_CXX
    void    SetDefaultFont_Impl();
    void    SetDefaults_Impl();
#endif

public:
    TYPEINFO();

    SvxBulletItem( sal_uInt16 nWhich = 0 );
    SvxBulletItem( SvStream& rStrm, sal_uInt16 nWhich = 0 );
    SvxBulletItem( const SvxBulletItem& );
    ~SvxBulletItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream & , sal_uInt16 nItemVersion ) const;

    String              GetFullText() const;
    sal_Unicode         GetSymbol() const { return cSymbol; }
    String              GetPrevText() const { return aPrevText; }
    String              GetFollowText() const { return aFollowText; }

    sal_uInt16              GetStart() const { return nStart; }
    long                GetWidth() const { return nWidth; }
    sal_uInt16              GetStyle() const { return nStyle; }
    sal_uInt8                GetJustification() const { return nJustify; }
    Font                GetFont() const { return aFont; }
    sal_uInt16              GetScale() const { return nScale; }

    const GraphicObject& GetGraphicObject() const;
    void                 SetGraphicObject( const GraphicObject& rGraphicObject );

    void                SetSymbol( sal_Unicode c) { cSymbol = c; }
    void                SetPrevText( const String& rStr) { aPrevText = rStr;}
    void                SetFollowText(const String& rStr) { aFollowText=rStr;}

    void                SetStart( sal_uInt16 nNew ) { nStart = nNew; }
    void                SetWidth( long nNew ) { nWidth = nNew; }
    void                SetStyle( sal_uInt16 nNew ) { nStyle = nNew; }
    void                SetJustification( sal_uInt8 nNew ) { nJustify = nNew; }
    void                SetFont( const Font& rNew) { aFont = rNew; }
    void                SetScale( sal_uInt16 nNew ) { nScale = nNew; }

    virtual sal_uInt16      GetVersion(sal_uInt16 nFileVersion) const;
    virtual int         operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    static void         StoreFont( SvStream&, const Font& );
    static Font         CreateFont( SvStream&, sal_uInt16 nVer );

    sal_uInt16&             GetValidMask()                  { return nValidMask;    }
    sal_uInt16              GetValidMask() const            { return nValidMask;    }
    sal_uInt16              IsValid( sal_uInt16 nFlag ) const   { return nValidMask & nFlag; }
    void                SetValid( sal_uInt16 nFlag, sal_Bool bValid )
                        {
                            if ( bValid )
                                nValidMask |= nFlag;
                            else
                                nValidMask &= ~nFlag;
                        }
    void                CopyValidProperties( const SvxBulletItem& rCopyFrom );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
