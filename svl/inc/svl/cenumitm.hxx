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

#ifndef _SVTOOLS_CENUMITM_HXX
#define _SVTOOLS_CENUMITM_HXX

#include "svl/svldllapi.h"
#include <tools/debug.hxx>
#include <svl/poolitem.hxx>

//============================================================================
DBG_NAMEEX(SfxEnumItemInterface)

class SVL_DLLPUBLIC SfxEnumItemInterface: public SfxPoolItem
{
protected:
    SfxEnumItemInterface(sal_uInt16 which): SfxPoolItem(which) {}

    SfxEnumItemInterface(const SfxEnumItemInterface & rItem):
        SfxPoolItem(rItem) {}

public:
    TYPEINFO();

    virtual int operator ==(const SfxPoolItem & rItem) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual sal_Bool QueryValue(com::sun::star::uno::Any & rVal, sal_uInt8 = 0) const;

    virtual sal_Bool PutValue(const com::sun::star::uno::Any & rVal, sal_uInt8 = 0);

    virtual sal_uInt16 GetValueCount() const = 0;

    virtual XubString GetValueTextByPos(sal_uInt16 nPos) const;

    virtual sal_uInt16 GetValueByPos(sal_uInt16 nPos) const;

    /// Return the position of some value within this enumeration.
    ///
    /// @descr  This method is implemented using GetValueCount() and
    /// GetValueByPos().  Derived classes may replace this with a more
    /// efficient implementation.
    ///
    /// @param nValue  Some value.
    ///
    /// @return  The position of nValue within this enumeration, or USHRT_MAX
    /// if not included.
    virtual sal_uInt16 GetPosByValue(sal_uInt16 nValue) const;

    virtual sal_Bool IsEnabled(sal_uInt16 nValue) const;

    virtual sal_uInt16 GetEnumValue() const = 0;

    virtual void SetEnumValue(sal_uInt16 nValue) = 0;

    virtual int HasBoolValue() const;

    virtual sal_Bool GetBoolValue() const;

    virtual void SetBoolValue(sal_Bool bValue);
};

//============================================================================
DBG_NAMEEX(CntEnumItem)

class SVL_DLLPUBLIC CntEnumItem: public SfxEnumItemInterface
{
    sal_uInt16 m_nValue;

protected:
    CntEnumItem(sal_uInt16 which = 0, sal_uInt16 nTheValue = 0):
        SfxEnumItemInterface(which), m_nValue(nTheValue) {}

    CntEnumItem(sal_uInt16 which, SvStream & rStream);

    CntEnumItem(const CntEnumItem & rItem):
        SfxEnumItemInterface(rItem), m_nValue(rItem.m_nValue) {}

public:
    TYPEINFO();

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual sal_uInt16 GetEnumValue() const;

    virtual void SetEnumValue(sal_uInt16 nTheValue);

    sal_uInt16 GetValue() const { return m_nValue; }

    inline void SetValue(sal_uInt16 nTheValue);
};

inline void CntEnumItem::SetValue(sal_uInt16 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntEnumItem::SetValue(): Pooled item");
    m_nValue = nTheValue;
}

//============================================================================
DBG_NAMEEX(CntBoolItem)

class SVL_DLLPUBLIC CntBoolItem: public SfxPoolItem
{
    sal_Bool m_bValue;

public:
    TYPEINFO();

    CntBoolItem(sal_uInt16 which = 0, sal_Bool bTheValue = sal_False):
        SfxPoolItem(which), m_bValue(bTheValue) {}

    CntBoolItem(sal_uInt16 nWhich, SvStream & rStream);

    CntBoolItem(const CntBoolItem & rItem):
        SfxPoolItem(rItem), m_bValue(rItem.m_bValue) {}

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                UniString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual sal_Bool QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8 = 0) const;

    virtual sal_Bool PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8 = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual sal_uInt16 GetValueCount() const;

    virtual UniString GetValueTextByVal(sal_Bool bTheValue) const;

    sal_Bool GetValue() const { return m_bValue; }

    void SetValue(sal_Bool bTheValue) { m_bValue = bTheValue; }
};

#endif // _SVTOOLS_CENUMITM_HXX

