/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/simple/CntOUString.hxx>
#include <item/base/ItemControlBlock.hxx>
#include <item/base/ItemAdministrator.hxx>
#include <libxml/xmlwriter.h>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemAdministrator& CntOUString::CntOUString_Data::getItemAdministrator() const
    {
        static ItemAdministrator_set aItemAdministrator_set(
            // hand over localized lambda call to construct a new instance of Item
            []()
            {
                return new CntOUString_Data(rtl::OUString());
            },
            // hand over localized lambda call to clone an Item
            [](const ItemData& rRef)
            {
                const CntOUString_Data& rData(static_cast<const CntOUString_Data&>(rRef));
                return new CntOUString_Data(rData.getValue());
            },
            // hand over localized lambda operator< to have a sorted set
            [](ItemData* A, ItemData* B)
            {
                return static_cast<CntOUString_Data*>(A)->getValue() < static_cast<CntOUString_Data*>(B)->getValue();
            });

        return aItemAdministrator_set;
    }

    CntOUString::CntOUString_Data::CntOUString_Data(const rtl::OUString& rValue)
    :   ItemData(),
        m_aValue(rValue)
    {
    }

    bool CntOUString::CntOUString_Data::operator==(const ItemData& rRef) const
    {
        return ItemData::operator==(rRef) || // ptr-compare
            getValue() == static_cast<const CntOUString_Data&>(rRef).getValue(); // content compare
    }

    CntOUString::CntOUString(ItemControlBlock& rItemControlBlock, const rtl::OUString& rValue)
    :   ItemBuffered(rItemControlBlock)
    {
        setItemData(new CntOUString_Data(rValue));
    }

    bool CntOUString::operator==(const ItemBase& rRef) const
    {
        return ItemBase::operator==(rRef) || // ptr-compare
            getValue() == static_cast<const CntOUString&>(rRef).getValue();
    }

    const rtl::OUString& CntOUString::getValue() const
    {
        return static_cast<CntOUString_Data&>(getItemData()).getValue();
    }

    void CntOUString::setValue(const rtl::OUString& rValue)
    {
        setItemData(new CntOUString_Data(rValue));
    }

    bool CntOUString::getPresentation(
        SfxItemPresentation,
        MapUnit,
        MapUnit,
        rtl::OUString& rText,
        const IntlWrapper&) const
    {
        rText = getValue();
        return true;
    }

    bool CntOUString::queryValue(css::uno::Any& rVal, sal_uInt8) const
    {
        rVal <<= getValue();
        return true;
    }

    bool CntOUString::putAnyValue(const css::uno::Any& rVal, sal_uInt8)
    {
        rtl::OUString aTheValue;

        if(rVal >>= aTheValue)
        {
            setValue(aTheValue);
            return true;
        }

        assert(false && "CntOUString::putAnyValue - Wrong type!");
        return false;
    }

    void CntOUString::dumpAsXml(xmlTextWriterPtr pWriter) const
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("CntOUString"));
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(getValue().toUtf8().getStr()));
        xmlTextWriterEndElement(pWriter);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
