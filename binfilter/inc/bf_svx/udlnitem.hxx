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
#ifndef _SVX_UDLNITEM_HXX
#define _SVX_UDLNITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;

// class SvxUnderlineItem ------------------------------------------------

/*	[Beschreibung]

    Dieses Item beschreibt, ob und wie unterstrichen ist.
*/

class SvxUnderlineItem : public SfxEnumItem
{
    Color mColor;
public:
    TYPEINFO();

    SvxUnderlineItem( const FontUnderline eSt = UNDERLINE_NONE,
                      const USHORT nId = ITEMID_UNDERLINE );

    // "pure virtual Methoden" vom SfxPoolItem

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&		Store(SvStream &, USHORT nItemVersion) const;
    virtual USHORT          GetValueCount() const;

    virtual	sal_Bool        QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual	sal_Bool		PutValue( const ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );

    // MS VC4.0 kommt durcheinander
    void			 		SetValue( USHORT nNewVal )
                                {SfxEnumItem::SetValue(nNewVal); }
    virtual int 			HasBoolValue() const;
    virtual BOOL			GetBoolValue() const;

    virtual int 			 operator==( const SfxPoolItem& ) const;

    inline SvxUnderlineItem& operator=(const SvxUnderlineItem& rUnderline)
        {
            SetValue( rUnderline.GetValue() );
            SetColor( rUnderline.GetColor() );
            return *this;
        }

    // enum cast
    FontUnderline 			GetUnderline() const
                                { return (FontUnderline)GetValue(); }
    void 					SetUnderline ( FontUnderline eNew )
                                { SetValue((USHORT) eNew); }

    const Color&			GetColor() const				{ return mColor; }
    void 					SetColor( const Color& rCol )	{ mColor = rCol; }
};

}//end of namespace binfilter
#endif // #ifndef _SVX_UDLNITEM_HXX

