/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lckbitem.hxx,v $
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
                            SfxLockBytesItem( USHORT nWhich,
                                              SvLockBytes *pLockBytes );
                            SfxLockBytesItem( USHORT nWhich, SvStream & );
                            SfxLockBytesItem( const SfxLockBytesItem& );
                            ~SfxLockBytesItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nItemVersion) const;
    virtual SvStream&       Store(SvStream &, USHORT nItemVersion ) const;

    SvLockBytes*            GetValue() const { return _xVal; }

    virtual BOOL            PutValue  ( const com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );
    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
};

#endif

