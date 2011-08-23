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

#ifndef _SFXVISIBILITYITEM_HXX
#define _SFXVISIBILITYITEM_HXX

#ifndef INCLUDED_SVLDLLAPI_H

#endif

#include <tools/debug.hxx>
#include <bf_svtools/poolitem.hxx>
#include <com/sun/star/frame/status/Visibility.hpp>

namespace binfilter
{

//============================================================================
DBG_NAMEEX_VISIBILITY(SfxVisibilityItem, )

class  SfxVisibilityItem: public SfxPoolItem
{
    ::com::sun::star::frame::status::Visibility m_nValue;

public:
    TYPEINFO();

    SfxVisibilityItem(USHORT which = 0, sal_Bool bVisible = sal_True):
        SfxPoolItem(which)
    { 
        m_nValue.bVisible = bVisible;
        DBG_CTOR(SfxVisibilityItem, 0);
    }

    SfxVisibilityItem(USHORT which, SvStream & rStream);

    SfxVisibilityItem(const SfxVisibilityItem & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(SfxVisibilityItem, 0); }

    virtual ~SfxVisibilityItem() { DBG_DTOR(SfxVisibilityItem, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const ::IntlWrapper * = 0)
        const;

    virtual	bool QueryValue( com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;

    virtual	bool PutValue( const com::sun::star::uno::Any& rVal,
                           BYTE nMemberId = 0 );

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual USHORT GetValueCount() const;

    virtual UniString GetValueTextByVal(BOOL bTheValue) const;

    BOOL GetValue() const { return m_nValue.bVisible; }

    void SetValue(BOOL bVisible) { m_nValue.bVisible = bVisible; }
};

}

#endif // _SFXVISIBILITYITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
