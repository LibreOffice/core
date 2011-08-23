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
#ifndef _SVX_COLRITEM_HXX
#define _SVX_COLRITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;

// class SvxColorItem ----------------------------------------------------

/*	[Beschreibung]

    Dieses Item beschreibt eine Farbe.
*/

#define VERSION_USEAUTOCOLOR	1

class SvxColorItem : public SfxPoolItem
{
private:
    Color					mColor;

public:
    TYPEINFO();

    SvxColorItem( const USHORT nId = ITEMID_COLOR );
    SvxColorItem( const Color& aColor, const USHORT nId = ITEMID_COLOR );
    SvxColorItem( SvStream& rStrm, const USHORT nId = ITEMID_COLOR );
    SvxColorItem( const SvxColorItem& rCopy );
    ~SvxColorItem();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual	sal_Bool        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual USHORT			GetVersion( USHORT nFileVersion ) const;


    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion) const;

    inline SvxColorItem& operator=(const SvxColorItem& rColor)
    {
        SetValue( rColor.GetValue() );
        return *this;
    }

    const	Color&			GetValue() const
                            {
                                return mColor;
                            }
            void			SetValue( const Color& rNewCol );

};

}//end of namespace binfilter
#endif

