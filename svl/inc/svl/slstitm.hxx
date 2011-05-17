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
#ifndef _SFXSLSTITM_HXX
#define _SFXSLSTITM_HXX

#include "svl/svldllapi.h"
#include <tools/rtti.hxx>
#include <tools/list.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Sequence.h>

class SfxImpStringList;

class SVL_DLLPUBLIC SfxStringListItem : public SfxPoolItem
{
protected:
    SfxImpStringList*   pImp;

public:
    TYPEINFO();

    SfxStringListItem();
    SfxStringListItem( sal_uInt16 nWhich, const List* pList=NULL );
    SfxStringListItem( sal_uInt16 nWhich, SvStream& rStream );
    SfxStringListItem( const SfxStringListItem& rItem );
    ~SfxStringListItem();

    List *                  GetList();

    const List *            GetList() const
    { return SAL_CONST_CAST(SfxStringListItem *, this)->GetList(); }

#ifndef TF_POOLABLE
    virtual int             IsPoolable() const;
#endif

    // String-Separator: \n
    virtual void            SetString( const XubString& );
    virtual XubString       GetString();

    void                    SetStringList( const com::sun::star::uno::Sequence< rtl::OUString >& rList );
    void                    GetStringList( com::sun::star::uno::Sequence< rtl::OUString >& rList ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream &, sal_uInt16 nItemVersion ) const;
    void                    Sort( sal_Bool bAscending = sal_True, List* pParallelList = 0 );

    virtual bool            PutValue  ( const com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 );
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 ) const;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
