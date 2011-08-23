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
#ifndef _SVX_EMPHITEM_HXX
#define _SVX_EMPTITEM_HXX

// include ---------------------------------------------------------------

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <bf_svtools/intitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;
// class SvxEmphasisMarkItem ----------------------------------------------

/* [Beschreibung]

    Dieses Item beschreibt die Font-Betonung.
*/

class SvxEmphasisMarkItem : public SfxUInt16Item
{
public:
    TYPEINFO();

    SvxEmphasisMarkItem(  const FontEmphasisMark eVal = EMPHASISMARK_NONE,
                          const USHORT nId = ITEMID_EMPHASISMARK );

    // "pure virtual Methoden" vom SfxPoolItem + SfxEnumItem

    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT) const;
    virtual SvStream&		Store(SvStream &, USHORT nItemVersion) const;
    virtual USHORT			GetVersion( USHORT nFileVersion ) const;

    virtual	sal_Bool        QueryValue( ::com::sun::star::uno::Any& rVal,
                                            BYTE nMemberId = 0 ) const;
    virtual	sal_Bool		PutValue( const ::com::sun::star::uno::Any& rVal,
                                            BYTE nMemberId = 0 );

    inline SvxEmphasisMarkItem& operator=(const SvxEmphasisMarkItem& rItem )
    {
        SetValue( rItem.GetValue() );
        return *this;
    }

    // enum cast
    FontEmphasisMark		GetEmphasisMark() const
                                { return (FontEmphasisMark)GetValue(); }
    void					SetEmphasisMark( FontEmphasisMark eNew )
                                { SetValue( (USHORT)eNew ); }
};

}//end of namespace binfilter
#endif // #ifndef _SVX_EMPHITEM_HXX

