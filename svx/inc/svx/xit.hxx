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

#ifndef _SVX_XIT_HXX
#define _SVX_XIT_HXX

#include <svl/stritem.hxx>
#include <svx/xdef.hxx>
#include "svx/svxdllapi.h"

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

typedef sal_Bool (*SvxCompareValueFunc)( const NameOrIndex* p1, const NameOrIndex* p2 );

//-------------------
// class NameOrIndex
//-------------------
class SVX_DLLPUBLIC NameOrIndex : public SfxStringItem
{
    sal_Int32    nPalIndex;

protected:
    void    Detach()    { nPalIndex = -1; }

public:
            TYPEINFO();
            NameOrIndex() { nPalIndex = -1; }
            NameOrIndex(sal_uInt16 nWhich, sal_Int32 nIndex);
            NameOrIndex(sal_uInt16 nWhich,
                        const String& rName= String());
            NameOrIndex(sal_uInt16 nWhich, SvStream& rIn);
            NameOrIndex(const NameOrIndex& rNameOrIndex);
           ~NameOrIndex() {};

    virtual int          operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual SvStream&    Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

            String       GetName() const              { return GetValue();   }
            void         SetName(const String& rName) { SetValue(rName);     }
            sal_Int32        GetIndex() const             { return nPalIndex;    }
            void         SetIndex(sal_Int32 nIndex)        { nPalIndex = nIndex;  }
            sal_Bool         IsIndex() const          { return (nPalIndex >= 0); }

    /** this static checks if the given NameOrIndex item has a unique name for its value.
        The returned String is a unique name for an item with this value in both given pools.
        Argument pPool2 can be null.
        If returned string equals NameOrIndex->GetName(), the name was already unique.
    */
    static String CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, const SfxItemPool* pPool2, SvxCompareValueFunc pCompareValueFunc, sal_uInt16 nPrefixResId, XPropertyList* pDefaults = NULL );
};

#endif
