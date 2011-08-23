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

#ifndef _SVXE3DITEM_HXX
#define _SVXE3DITEM_HXX

#include <bf_svtools/poolitem.hxx>
#include <bf_goodies/vector3d.hxx>
class SvStream;
namespace binfilter {

#ifndef _SVXVECT3DITEM_HXX
#define _SVXVECT3DITEM_HXX


DBG_NAMEEX(SvxVector3DItem)//STRIP008

class SvxVector3DItem: public SfxPoolItem
{
    Vector3D				aVal;

public:
                            TYPEINFO();
                            SvxVector3DItem();
                            SvxVector3DItem( USHORT nWhich, const Vector3D& rVal );
                            SvxVector3DItem( const SvxVector3DItem& );
                            ~SvxVector3DItem() {
                                 DBG_DTOR(SvxVector3DItem, 0); }

    virtual int				operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVersion) const;
    virtual SvStream&		Store(SvStream &, USHORT nItemVersion ) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const Vector3D&			GetValue() const { return aVal; }
            void			SetValue( const Vector3D& rNewVal ) {
                                 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
                                 aVal = rNewVal;
                             }

    virtual USHORT GetVersion (USHORT nFileFormatVersion) const;
};

#endif

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
