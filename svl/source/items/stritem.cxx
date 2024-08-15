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

#include <svl/stritem.hxx>
#include <libxml/xmlwriter.h>
#include <com/sun/star/uno/Any.hxx>
#include <osl/diagnose.h>
#include <unotools/intlwrapper.hxx>

// virtual
bool SfxStringItem::GetPresentation(SfxItemPresentation, MapUnit,
                                        MapUnit, OUString & rText,
                                        const IntlWrapper&) const
{
    rText = m_aValue;
    return true;
}

// virtual
bool SfxStringItem::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    rVal <<= m_aValue;
    return true;
}

// virtual
bool SfxStringItem::PutValue(const css::uno::Any& rVal,
                                         sal_uInt8)
{
    OUString aTheValue;
    if (rVal >>= aTheValue)
    {
        m_aValue = aTheValue;
        return true;
    }
    OSL_FAIL("SfxStringItem::PutValue(): Wrong type");
    return false;
}

// virtual
bool SfxStringItem::operator ==(const SfxPoolItem & rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return m_aValue
            == static_cast< const SfxStringItem * >(&rItem)->
                m_aValue;
}

// virtual
SfxStringItem* SfxStringItem::Clone(SfxItemPool *) const
{
    return new SfxStringItem(*this);
}

void SfxStringItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxStringItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(GetValue().toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

SfxPoolItem* SfxStringItem::CreateDefault()
{
    return new SfxStringItem();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
