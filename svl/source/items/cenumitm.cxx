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

#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>
#include <svl/cenumitm.hxx>
#include <svl/eitem.hxx>

#include <comphelper/extract.hxx>
#include <libxml/xmlwriter.h>
#include <sal/log.hxx>


// virtual
bool SfxEnumItemInterface::operator ==(const SfxPoolItem & rItem) const
{
    SAL_WARN_IF(!SfxPoolItem::operator ==(rItem), "svl.items", "unequal type, with ID/pos " << Which() );
    return GetEnumValue()
               == static_cast< const SfxEnumItemInterface * >(&rItem)->
                      GetEnumValue();
}

// virtual
bool SfxEnumItemInterface::GetPresentation(SfxItemPresentation, MapUnit,
                                      MapUnit, OUString & rText,
                                      const IntlWrapper&) const
{
    rText = OUString::number( GetEnumValue() );
    return true;
}

// virtual
bool SfxEnumItemInterface::QueryValue(css::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= sal_Int32(GetEnumValue());
    return true;
}

// virtual
bool SfxEnumItemInterface::PutValue(const css::uno::Any& rVal,
                                    sal_uInt8)
{
    sal_Int32 nTheValue = 0;

    if ( ::cppu::enum2int( nTheValue, rVal ) )
    {
        SetEnumValue(sal_uInt16(nTheValue));
        return true;
    }
    SAL_WARN("svl.items", "SfxEnumItemInterface::PutValue(): Wrong type");
    return false;
}

// virtual
bool SfxEnumItemInterface::HasBoolValue() const
{
    return false;
}

// virtual
bool SfxEnumItemInterface::GetBoolValue() const
{
    return false;
}

// virtual
void SfxEnumItemInterface::SetBoolValue(bool)
{}

SfxPoolItem* SfxBoolItem::CreateDefault()
{
    return new SfxBoolItem();
}

SfxBoolItem::SfxBoolItem(sal_uInt16 const nWhich, SvStream & rStream)
    : SfxPoolItem(nWhich)
{
    bool tmp = false;
    rStream.ReadCharAsBool( tmp );
    m_bValue = tmp;
}

// virtual
bool SfxBoolItem::operator ==(const SfxPoolItem & rItem) const
{
    assert(dynamic_cast<const SfxBoolItem*>(&rItem) != nullptr);
    return m_bValue == static_cast< SfxBoolItem const * >(&rItem)->m_bValue;
}

// virtual
bool SfxBoolItem::GetPresentation(SfxItemPresentation,
                                                 MapUnit, MapUnit,
                                                 OUString & rText,
                                                 const IntlWrapper&) const
{
    rText = GetValueTextByVal(m_bValue);
    return true;
}

void SfxBoolItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SfxBoolItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(GetValueTextByVal(m_bValue).toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

// virtual
bool SfxBoolItem::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    rVal <<= m_bValue;
    return true;
}

// virtual
bool SfxBoolItem::PutValue(const css::uno::Any& rVal, sal_uInt8)
{
    bool bTheValue = bool();
    if (rVal >>= bTheValue)
    {
        m_bValue = bTheValue;
        return true;
    }
    SAL_WARN("svl.items", "SfxBoolItem::PutValue(): Wrong type");
    return false;
}

// virtual
SfxPoolItem * SfxBoolItem::Create(SvStream & rStream, sal_uInt16) const
{
    return new SfxBoolItem(Which(), rStream);
}

// virtual
SvStream & SfxBoolItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream.WriteBool( m_bValue ); // not bool for serialization!
    return rStream;
}

// virtual
SfxPoolItem * SfxBoolItem::Clone(SfxItemPool *) const
{
    return new SfxBoolItem(*this);
}

// virtual
OUString SfxBoolItem::GetValueTextByVal(bool bTheValue) const
{
    return bTheValue ?  OUString("TRUE") : OUString("FALSE");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
