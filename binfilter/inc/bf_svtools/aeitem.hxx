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
#ifndef _AEITEM_HXX
#define _AEITEM_HXX

#include "bf_svtools/svtdllapi.h"

#include <bf_svtools/poolitem.hxx>
#include <bf_svtools/eitem.hxx>

namespace binfilter {

class SfxAllEnumValueArr;
class SvUShorts;

class  SfxAllEnumItem: public SfxEnumItem
{
    SfxAllEnumValueArr*		pValues;
    SvUShorts*				pDisabledValues;

protected:
    USHORT					_GetPosByValue( USHORT nValue ) const;

public:
    TYPEINFO();
                            SfxAllEnumItem();
                            SfxAllEnumItem( USHORT nWhich);
                            SfxAllEnumItem( USHORT nWhich, SvStream &rStream );
                            SfxAllEnumItem( const SfxAllEnumItem & );
                            ~SfxAllEnumItem();

    void                    InsertValue( USHORT nValue );

    USHORT					GetPosByValue( USHORT nValue ) const;

    virtual USHORT			GetValueCount() const;
    virtual USHORT			GetValueByPos( USHORT nPos ) const;
    virtual XubString		GetValueTextByPos( USHORT nPos ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVersion) const;
    virtual BOOL			IsEnabled( USHORT ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
