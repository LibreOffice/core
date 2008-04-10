/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: aeitem.hxx,v $
 * $Revision: 1.3 $
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

#include "svtools/svtdllapi.h"
#include <svtools/poolitem.hxx>
#include <svtools/eitem.hxx>

class SfxAllEnumValueArr;
class SvUShorts;

class SVT_DLLPUBLIC SfxAllEnumItem: public SfxEnumItem
{
    SfxAllEnumValueArr*     pValues;
    SvUShorts*              pDisabledValues;

protected:
    USHORT                  _GetPosByValue( USHORT nValue ) const;

public:
    TYPEINFO();
                            SfxAllEnumItem();
                            SfxAllEnumItem( USHORT nWhich);
                            SfxAllEnumItem( USHORT nWhich, USHORT nVal );
                            SfxAllEnumItem( USHORT nWhich, USHORT nVal, const XubString &rText );
                            SfxAllEnumItem( USHORT nWhich, SvStream &rStream );
                            SfxAllEnumItem( const SfxAllEnumItem & );
                            ~SfxAllEnumItem();

    void                    InsertValue( USHORT nValue );
    void                    InsertValue( USHORT nValue, const XubString &rText );
    void                    RemoveValue( USHORT nValue );
    void                    RemoveAllValues();

    USHORT                  GetPosByValue( USHORT nValue ) const;

    virtual USHORT          GetValueCount() const;
    virtual USHORT          GetValueByPos( USHORT nPos ) const;
    virtual XubString       GetValueTextByPos( USHORT nPos ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVersion) const;
    virtual BOOL            IsEnabled( USHORT ) const;
    void                    DisableValue( USHORT );
};

#endif
