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

#ifndef _SFXILSTITEM_HXX
#define _SFXILSTITEM_HXX

#ifndef INCLUDED_SVLDLLAPI_H

#endif

#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace binfilter
{
class SvULongs;

class  SfxIntegerListItem : public SfxPoolItem
{
    ::com::sun::star::uno::Sequence < sal_Int32 > m_aList;

public:
    TYPEINFO();

    SfxIntegerListItem();
    SfxIntegerListItem( const SfxIntegerListItem& rItem );
    ~SfxIntegerListItem();

    ::com::sun::star::uno::Sequence < sal_Int32 > GetSequence()
    { return m_aList; }
    ::com::sun::star::uno::Sequence < sal_Int32 > GetConstSequence() const
    { return SAL_CONST_CAST(SfxIntegerListItem *, this)->GetSequence(); }

    virtual int 			operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual BOOL            PutValue  ( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
};

}

#endif // _SFXINTITEM_HXX

