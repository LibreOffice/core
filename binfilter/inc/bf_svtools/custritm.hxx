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

#ifndef _SVTOOLS_CUSTRITM_HXX
#define _SVTOOLS_CUSTRITM_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

namespace binfilter {

//============================================================================
DBG_NAMEEX_VISIBILITY(CntUnencodedStringItem, )

class  CntUnencodedStringItem: public SfxPoolItem
{
    XubString m_aValue;

public:
    TYPEINFO();

    CntUnencodedStringItem(USHORT which = 0): SfxPoolItem(which)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(USHORT which, const XubString & rTheValue):
        SfxPoolItem(which), m_aValue(rTheValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(const CntUnencodedStringItem & rItem):
        SfxPoolItem(rItem), m_aValue(rItem.m_aValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    virtual ~CntUnencodedStringItem() { DBG_DTOR(CntUnencodedStringItem, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual int Compare(SfxPoolItem const & rWith,
        ::IntlWrapper const & rIntlWrapper) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const ::IntlWrapper * = 0)
        const;

    virtual	BOOL QueryValue(::com::sun::star::uno::Any& rVal,
                            BYTE nMemberId = 0) const;

    virtual	BOOL PutValue(const ::com::sun::star::uno::Any& rVal,
                          BYTE nMemberId = 0);

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    const XubString & GetValue() const { return m_aValue; }

    inline void SetValue(const XubString & rTheValue);
};

inline void CntUnencodedStringItem::SetValue(const XubString & rTheValue)
{
    DBG_ASSERT(GetRefCount() == 0,
               "CntUnencodedStringItem::SetValue(): Pooled item");
    m_aValue = rTheValue;
}

}

#endif //  _SVTOOLS_CUSTRITM_HXX

