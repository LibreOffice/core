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

#include "svl/svldllapi.h"
#include <tools/debug.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/frame/status/Visibility.hpp>

//============================================================================
DBG_NAMEEX_VISIBILITY(SfxVisibilityItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC SfxVisibilityItem: public SfxPoolItem
{
    ::com::sun::star::frame::status::Visibility m_nValue;

public:
    TYPEINFO();

    SfxVisibilityItem(sal_uInt16 which = 0, sal_Bool bVisible = sal_True):
        SfxPoolItem(which)
    {
        m_nValue.bVisible = bVisible;
        DBG_CTOR(SfxVisibilityItem, 0);
    }

    SfxVisibilityItem(sal_uInt16 which, SvStream & rStream);

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
                                                const IntlWrapper * = 0)
        const;

    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const;

    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal,
                           sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual sal_uInt16 GetValueCount() const;

    virtual UniString GetValueTextByVal(sal_Bool bTheValue) const;

    sal_Bool GetValue() const { return m_nValue.bVisible; }

    void SetValue(sal_Bool bVisible) { m_nValue.bVisible = bVisible; }
};

#endif // _SFXVISIBILITYITEM_HXX
