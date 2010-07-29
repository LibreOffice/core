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
#ifndef _AEITEM_HXX
#define _AEITEM_HXX

#include "svl/svldllapi.h"
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>

class SfxAllEnumValueArr;
class SvUShorts;

class SVL_DLLPUBLIC SfxAllEnumItem: public SfxEnumItem
{
    SfxAllEnumValueArr*     pValues;
    SvUShorts*              pDisabledValues;

protected:
    sal_uInt16                  _GetPosByValue( sal_uInt16 nValue ) const;

public:
    TYPEINFO();
                            SfxAllEnumItem();
                            SfxAllEnumItem( sal_uInt16 nWhich);
                            SfxAllEnumItem( sal_uInt16 nWhich, sal_uInt16 nVal );
                            SfxAllEnumItem( sal_uInt16 nWhich, sal_uInt16 nVal, const XubString &rText );
                            SfxAllEnumItem( sal_uInt16 nWhich, SvStream &rStream );
                            SfxAllEnumItem( const SfxAllEnumItem & );
                            ~SfxAllEnumItem();

    void                    InsertValue( sal_uInt16 nValue );
    void                    InsertValue( sal_uInt16 nValue, const XubString &rText );
    void                    RemoveValue( sal_uInt16 nValue );
    void                    RemoveAllValues();

    sal_uInt16                  GetPosByValue( sal_uInt16 nValue ) const;

    virtual sal_uInt16          GetValueCount() const;
    virtual sal_uInt16          GetValueByPos( sal_uInt16 nPos ) const;
    virtual XubString       GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nVersion) const;
    virtual sal_Bool            IsEnabled( sal_uInt16 ) const;
    void                    DisableValue( sal_uInt16 );
};

#endif
