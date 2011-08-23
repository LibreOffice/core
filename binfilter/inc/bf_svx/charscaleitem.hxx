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
#ifndef _SVX_CHARSCALEITEM_HXX
#define _SVX_CHARSCALEITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXENUMITEM_HXX
#include <bf_svtools/intitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
namespace binfilter {

// class SvxCharScaleItem ----------------------------------------------

/* [Description]

    This item defines a character scaling factor as percent value.
    A value of 100 is identical to 100% and means normal width
    A value of 50 is identical to 50% and means 1/2 width.

*/

class SvxCharScaleWidthItem : public SfxUInt16Item
{
public:
    TYPEINFO();

    SvxCharScaleWidthItem( sal_uInt16 nValue = 100,
                            const sal_uInt16 nId = ITEMID_CHARSCALE_W );

    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT) const;
    virtual SvStream&		Store( SvStream& , USHORT nItemVersion ) const;
    virtual USHORT			GetVersion( USHORT nFileVersion ) const;


    virtual sal_Bool PutValue( const ::com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId );
    virtual sal_Bool QueryValue( ::com::sun::star::uno::Any& rVal,
                                BYTE nMemberId ) const;

    inline SvxCharScaleWidthItem& operator=(const SvxCharScaleWidthItem& rItem )
    {
        SetValue( rItem.GetValue() );
        return *this;
    }

};

}//end of namespace binfilter
#endif

