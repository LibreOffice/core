/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:56:16 $
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

#ifndef _SVX_XIT_HXX
#define _SVX_XIT_HXX

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _XDEF_HXX
#include <svx/xdef.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

/************************************************************************/

class XColorTable;
class XDashTable;
class XLineEndTable;
class XHatchTable;
class XBitmapTable;
class XGradientTable;
class SfxItemPool;
class NameOrIndex;
class XPropertyList;

typedef BOOL (*SvxCompareValueFunc)( const NameOrIndex* p1, const NameOrIndex* p2 );

//-------------------
// class NameOrIndex
//-------------------
class SVX_DLLPUBLIC NameOrIndex : public SfxStringItem
{
    long    nPalIndex;

protected:
    void    Detach()    { nPalIndex = -1; }

public:
            TYPEINFO();
            NameOrIndex() { nPalIndex = -1; }
            NameOrIndex(USHORT nWhich, long nIndex);
            NameOrIndex(USHORT nWhich,
                        const String& rName= String());
            NameOrIndex(USHORT nWhich, SvStream& rIn);
            NameOrIndex(const NameOrIndex& rNameOrIndex);
           ~NameOrIndex() {};

    virtual int          operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, USHORT nVer) const;
    virtual SvStream&    Store(SvStream& rOut, USHORT nItemVersion ) const;

            String       GetName() const              { return GetValue();   }
            void         SetName(const String& rName) { SetValue(rName);     }
            long         GetIndex() const             { return nPalIndex;    }
            void         SetIndex(long nIndex)        { nPalIndex = nIndex;  }
            BOOL         IsIndex() const          { return (nPalIndex >= 0); }

    /** this static checks if the given NameOrIndex item has a unique name for its value.
        The returned String is a unique name for an item with this value in both given pools.
        Argument pPool2 can be null.
        If returned string equals NameOrIndex->GetName(), the name was already unique.
    */
    static String CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, const SfxItemPool* pPool2, SvxCompareValueFunc pCompareValueFunc, USHORT nPrefixResId, XPropertyList* pDefaults = NULL );
};

#endif
