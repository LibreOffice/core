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
#ifndef _SVX_SHADITEM_HXX
#define _SVX_SHADITEM_HXX

// include ---------------------------------------------------------------

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <bf_svx/svxenum.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif

namespace rtl
{
    class OUString;
}
namespace binfilter {

// class SvxShadowItem ---------------------------------------------------

/*	[Beschreibung]

    Dieses Item beschreibt ein Schattenattribut (Farbe, Breite, Lage).
*/

#define SHADOW_TOP		((USHORT)0)
#define SHADOW_BOTTOM	((USHORT)1)
#define SHADOW_LEFT		((USHORT)2)
#define SHADOW_RIGHT	((USHORT)3)

class SvxShadowItem : public SfxEnumItemInterface
{
    Color 				aShadowColor;
    USHORT 				nWidth;
    SvxShadowLocation 	eLocation;
public:
    TYPEINFO();

    SvxShadowItem( const USHORT nId = ITEMID_SHADOW,
                 const Color *pColor = 0, const USHORT nWidth = 100 /*5pt*/,
                 const SvxShadowLocation eLoc = SVX_SHADOW_NONE );

    inline SvxShadowItem& operator=( const SvxShadowItem& rFmtShadow );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual	sal_Bool        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion ) const;

    const Color& GetColor() const {	return aShadowColor;}
    void SetColor( const Color &rNew ) { aShadowColor = rNew; }

    USHORT GetWidth() const { return nWidth; }
    SvxShadowLocation GetLocation() const { return eLocation; }

    void SetWidth( USHORT nNew ) { nWidth = nNew; }
    void SetLocation( SvxShadowLocation eNew ) { eLocation = eNew; }

        //Breite des Schattens auf der jeweiligen Seite berechnen.
    USHORT CalcShadowSpace( USHORT nShadow ) const;

    virtual USHORT			GetValueCount() const;
    virtual USHORT			GetEnumValue() const;
    virtual void			SetEnumValue( USHORT nNewVal );
};

inline SvxShadowItem &SvxShadowItem::operator=( const SvxShadowItem& rFmtShadow )
{
    aShadowColor = rFmtShadow.aShadowColor;
    nWidth = rFmtShadow.GetWidth();
    eLocation = rFmtShadow.GetLocation();
    return *this;
}

}//end of namespace binfilter
#endif // #ifndef _SVX_SHADITEM_HXX

