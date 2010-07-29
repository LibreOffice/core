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
#ifndef _LCKBITEM_HXX
#define _LCKBITEM_HXX

#include "svl/svldllapi.h"
#include <tools/solar.h>
#include <tools/rtti.hxx>
#include <tools/stream.hxx>
#include <svl/poolitem.hxx>

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxLockBytesItem : public SfxPoolItem
{
    SvLockBytesRef          _xVal;

public:
                            TYPEINFO();
                            SfxLockBytesItem();
                            SfxLockBytesItem( sal_uInt16 nWhich,
                                              SvLockBytes *pLockBytes );
                            SfxLockBytesItem( sal_uInt16 nWhich, SvStream & );
                            SfxLockBytesItem( const SfxLockBytesItem& );
                            ~SfxLockBytesItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nItemVersion) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;

    SvLockBytes*            GetValue() const { return _xVal; }

    virtual sal_Bool            PutValue  ( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 );
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
};

#endif

