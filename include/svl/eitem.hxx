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

#ifndef INCLUDED_SVL_EITEM_HXX
#define INCLUDED_SVL_EITEM_HXX

#include <svl/svldllapi.h>
#include <svl/cenumitm.hxx>
#include <tools/stream.hxx>


template<typename EnumT>
class SAL_DLLPUBLIC_RTTI SfxEnumItem : public SfxEnumItemInterface
{
    EnumT m_nValue;

protected:
    explicit SfxEnumItem(sal_uInt16 const nWhich, EnumT const nValue)
        : SfxEnumItemInterface(nWhich)
        , m_nValue(nValue)
    { }

    SfxEnumItem(const SfxEnumItem &) = default;

    SfxEnumItem(sal_uInt16 const nWhich, SvStream & rStream)
        : SfxEnumItemInterface(nWhich)
    {
        sal_uInt16 nTmp = 0;
        rStream.ReadUInt16( nTmp );
        m_nValue = static_cast<EnumT>(nTmp);
    }

public:

    EnumT GetValue() const { return m_nValue; }

    void SetValue(EnumT nTheValue)
    {
        assert(GetRefCount() == 0 && "SfxEnumItem::SetValue(): Pooled item");
        m_nValue = nTheValue;
    }

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const override
    {
        rStream.WriteUInt16( static_cast<sal_uInt16>(m_nValue) );
        return rStream;
    }

    virtual sal_uInt16 GetEnumValue() const override
    {
        return static_cast<sal_uInt16>(GetValue());
    }

    virtual void SetEnumValue(sal_uInt16 nTheValue) override
    {
        SetValue(static_cast<EnumT>(nTheValue));
    }

    virtual bool operator==(SfxPoolItem const & other) const override
    {
        return SfxEnumItemInterface::operator==(other) &&
               m_nValue == static_cast<const SfxEnumItem<EnumT> &>(other).m_nValue;
    }
};

class SVL_DLLPUBLIC SfxBoolItem
    : public SfxPoolItem
{
    bool m_bValue;

public:
    static SfxPoolItem* CreateDefault();

    explicit SfxBoolItem(sal_uInt16 const nWhich = 0, bool const bValue = false)
        : SfxPoolItem(nWhich)
        , m_bValue(bValue)
    { }

    SfxBoolItem(sal_uInt16 nWhich, SvStream & rStream);

    bool GetValue() const { return m_bValue; }

    void SetValue(bool const bTheValue) { m_bValue = bTheValue; }

    // SfxPoolItem
    virtual bool operator ==(const SfxPoolItem & rItem) const override;

    virtual bool GetPresentation(SfxItemPresentation,
                                 MapUnit, MapUnit,
                                 OUString & rText,
                                 const IntlWrapper&)
        const override;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 = 0) const override;

    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8) override;


    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
        override;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const override;

    virtual SfxPoolItem * Clone(SfxItemPool * = nullptr) const override;

    virtual OUString GetValueTextByVal(bool bTheValue) const;
};

#endif // INCLUDED_SVL_EITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
