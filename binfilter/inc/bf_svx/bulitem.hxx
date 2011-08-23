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
namespace binfilter {

// include ---------------------------------------------------------------

// define ----------------------------------------------------------------

// Styles
#define BS_ABC_BIG			0
#define BS_ABC_SMALL		1
#define BS_ROMAN_BIG		2
#define BS_ROMAN_SMALL		3
#define BS_123				4
#define BS_NONE				5
#define BS_BULLET			6
#define BS_BMP				128

// Justification
#define BJ_HLEFT			0x01
#define BJ_HRIGHT			0x02
#define BJ_HCENTER			0x04
#define BJ_VTOP				0x08
#define BJ_VBOTTOM  		0x10
#define BJ_VCENTER			0x20

// Valid-Bits
// Erstmal nur die Werte, die vom Dialog geaendert werden...
#define VALID_FONTCOLOR		0x0001
#define VALID_FONTNAME		0x0002
#define VALID_SYMBOL		0x0004
#define VALID_BITMAP		0x0008
#define VALID_SCALE			0x0010
#define VALID_START			0x0020
#define VALID_STYLE			0x0040
#define VALID_PREVTEXT		0x0080
#define VALID_FOLLOWTEXT	0x0100
}//end of namespace binfilter
#include <bf_svtools/poolitem.hxx>
#include <vcl/font.hxx>
#include <bf_goodies/graphicobject.hxx>

// class SvxBulletItem ---------------------------------------------------
namespace binfilter {
class SvxBulletItem : public SfxPoolItem
{
    Font	        aFont;
    BfGraphicObject*  pGraphicObject;
    String	        aPrevText;
    String	        aFollowText;
    USHORT      	nStart;
    USHORT	        nStyle;
    long  	        nWidth;
    USHORT	        nScale;
    sal_Unicode	    cSymbol;
    BYTE	        nJustify;
    USHORT	        nValidMask;	// Nur temporaer fuer GetAttribs/SetAttribs, wegen des grossen Bullets

#ifdef _SVX_BULITEM_CXX
    void	SetDefaultFont_Impl();
    void	SetDefaults_Impl();
#endif

public:
    TYPEINFO();

    SvxBulletItem( USHORT nWhich = 0 );
    SvxBulletItem( SvStream& rStrm, USHORT nWhich = 0 );
    SvxBulletItem( const SvxBulletItem& );
    ~SvxBulletItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream&, USHORT nVersion ) const;
    virtual SvStream&       Store( SvStream & , USHORT nItemVersion ) const;

    sal_Unicode	        GetSymbol() const { return cSymbol; }
    String	            GetPrevText() const { return aPrevText; }
    String	            GetFollowText() const { return aFollowText; }

    USHORT				GetStart() const { return nStart; }
    long  				GetWidth() const { return nWidth; }
    USHORT  			GetStyle() const { return nStyle; }
    BYTE				GetJustification() const { return nJustify; }
    Font				GetFont() const { return aFont; }
    USHORT				GetScale() const { return nScale; }



    void	            SetSymbol( sal_Unicode c) { cSymbol = c; }
    void	            SetPrevText( const String& rStr) { aPrevText = rStr;}
    void	            SetFollowText(const String& rStr) { aFollowText=rStr;}

    void				SetStart( USHORT nNew ) { nStart = nNew; }
    void				SetWidth( long nNew ) { nWidth = nNew; }
    void				SetStyle( USHORT nNew ) { nStyle = nNew; }
    void				SetJustification( BYTE nNew ) { nJustify = nNew; }
    void				SetFont( const Font& rNew) { aFont = rNew; }
    void				SetScale( USHORT nNew ) { nScale = nNew; }

    virtual USHORT		GetVersion(USHORT nFileVersion) const;
    virtual int			operator==( const SfxPoolItem& ) const;

    static void			StoreFont( SvStream&, const Font& );
    static Font			CreateFont( SvStream&, USHORT nVer );

    USHORT&				GetValidMask() 					{ return nValidMask;	}
    USHORT				GetValidMask() const 			{ return nValidMask;	}
    USHORT				IsValid( USHORT nFlag ) const	{ return nValidMask & nFlag; }
    void				SetValid( USHORT nFlag, BOOL bValid )
                        {
                            if ( bValid )
                                nValidMask |= nFlag;
                            else
                                nValidMask &= ~nFlag;
                        }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
