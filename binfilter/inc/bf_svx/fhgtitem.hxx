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
#ifndef _SVX_FHGTITEM_HXX
#define _SVX_FHGTITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/poolitem.hxx>
#include <bf_svx/svxids.hrc>

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;
// class SvxFontHeightItem -----------------------------------------------

// Achtung: Twips-Werte

/*	[Beschreibung]

    Dieses Item beschreibt die Font-Hoehe.
*/

#define	FONTHEIGHT_16_VERSION	((USHORT)0x0001)
#define	FONTHEIGHT_UNIT_VERSION	((USHORT)0x0002)

class SvxFontHeightItem : public SfxPoolItem
{
    UINT32	nHeight;
    USHORT	nProp;				// default 100%
    SfxMapUnit ePropUnit;		// Percent, Twip, ...
public:
    TYPEINFO();

    SvxFontHeightItem( const ULONG nSz = 240, const USHORT nPropHeight = 100,
                       const USHORT nId = ITEMID_FONTHEIGHT );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion) const;
    virtual USHORT			 GetVersion( USHORT nItemVersion) const;

    inline SvxFontHeightItem& operator=(const SvxFontHeightItem& rSize)
        {
            SetHeightValue( rSize.GetHeight() );
            SetProp( rSize.GetProp(), ePropUnit );
            return *this;
        }

    void SetHeight( UINT32 nNewHeight, const USHORT nNewProp = 100,
                     SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE );


    UINT32 GetHeight() const { return nHeight; }

    void SetHeightValue( UINT32 nNewHeight )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nHeight = nNewHeight;
        }

    void SetProp( const USHORT nNewProp,
                    SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
            ePropUnit = eUnit;
        }

    USHORT GetProp() const { return nProp; }

    SfxMapUnit GetPropUnit() const { return ePropUnit;  }	// Percent, Twip, ...
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
