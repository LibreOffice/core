/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVL_INTITEM_HXX
#define INCLUDED_SVL_INTITEM_HXX

#include <svl/svldllapi.h>
#include <svl/cintitem.hxx>
#include <boost/property_tree/ptree_fwd.hpp>


class SVL_DLLPUBLIC SfxByteItem: public CntByteItem
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SfxByteItem)
    explicit SfxByteItem(sal_uInt16 which = 0, sal_uInt8 nValue = 0):
        CntByteItem(which, nValue) {}

    virtual SfxByteItem* Clone(SfxItemPool * = nullptr) const override
    { return new SfxByteItem(*this); }
};

class SVL_DLLPUBLIC SfxInt16Item: public SfxPoolItem
{
    sal_Int16 m_nValue;

public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SfxInt16Item)
    explicit SfxInt16Item(sal_uInt16 which = 0, sal_Int16 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    {}

    virtual bool operator ==(const SfxPoolItem & rItem) const override;
    virtual bool supportsHashCode() const override { return true; }
    virtual size_t hashCode() const override { return m_nValue; }

    virtual bool GetPresentation(SfxItemPresentation,
                                                MapUnit, MapUnit,
                                                OUString & rText,
                                                const IntlWrapper&)
        const override;

    virtual boost::property_tree::ptree dumpAsJSON() const override;

    virtual bool QueryValue( css::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const override;

    virtual bool PutValue( const css::uno::Any& rVal,
                           sal_uInt8 nMemberId ) override;

    virtual SfxInt16Item* Clone(SfxItemPool * = nullptr) const override;

    sal_Int16 GetValue() const { return m_nValue; }

    void SetValue(sal_Int16 nTheValue) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nValue = nTheValue; }
};


class SVL_DLLPUBLIC SfxUInt16Item: public CntUInt16Item
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SfxUInt16Item)
    explicit SfxUInt16Item(sal_uInt16 which = 0, sal_uInt16 nValue = 0):
        CntUInt16Item(which, nValue) {}

    virtual SfxUInt16Item* Clone(SfxItemPool * = nullptr) const override
    { return new SfxUInt16Item(*this); }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};


class SVL_DLLPUBLIC SfxInt32Item: public CntInt32Item
{
public:
    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(SfxInt32Item)
    explicit SfxInt32Item(sal_uInt16 which = 0, sal_Int32 nValue = 0):
        CntInt32Item(which, nValue) {}

    virtual SfxInt32Item* Clone(SfxItemPool * = nullptr) const override
    { return new SfxInt32Item(*this); }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};


class SVL_DLLPUBLIC SfxUInt32Item: public CntUInt32Item
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SfxUInt32Item)
    explicit SfxUInt32Item(sal_uInt16 which = 0, sal_uInt32 nValue = 0):
        CntUInt32Item(which, nValue) {}

    virtual SfxUInt32Item* Clone(SfxItemPool * = nullptr) const override
    { return new SfxUInt32Item(*this); }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

#endif // INCLUDED_SVL_INTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
