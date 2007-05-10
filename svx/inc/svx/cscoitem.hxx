/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cscoitem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 14:24:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_CSCOITEM_HXX
#define _SVX_CSCOITEM_HXX

// include ---------------------------------------------------------------

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxCharSetColorItem ---------------------------------------------

/*  [Beschreibung]

    PB: wird nur intern im Reader des Writers benoetigt
*/

class SVX_DLLPUBLIC SvxCharSetColorItem : public SvxColorItem
{
    rtl_TextEncoding eFrom;
public:
    TYPEINFO();

    SvxCharSetColorItem( const USHORT nId  );
    SvxCharSetColorItem( const Color& aColor, const rtl_TextEncoding eFrom,
                     const USHORT nId  );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion) const;

    inline rtl_TextEncoding&    GetCharSet() { return eFrom; }
    inline rtl_TextEncoding     GetCharSet() const { return eFrom; }

    inline SvxCharSetColorItem& operator=(const SvxCharSetColorItem& rColor)
    {
        SetValue( rColor.GetValue() );
        eFrom = rColor.GetCharSet();
        return *this;
    }
};

#endif

