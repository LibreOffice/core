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


#include <svl/intitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <osl/diagnose.h>
#include <tools/bigint.hxx>
#include <tools/stream.hxx>
#include <svl/metitem.hxx>
#include <libxml/xmlwriter.h>


//  class SfxByteItem


SfxPoolItem* SfxByteItem::CreateDefault()
{
    return new SfxByteItem();
};

// virtual
SfxPoolItem * SfxByteItem::Create(SvStream & rStream, sal_uInt16) const
{
    short nValue = 0;
    rStream.ReadInt16( nValue );
    return new SfxByteItem(Which(), sal_uInt8(nValue));
}

SfxPoolItem* SfxInt16Item::CreateDefault()
{
    return new SfxInt16Item();
};

SfxInt16Item::SfxInt16Item(sal_uInt16 which, SvStream & rStream):
    SfxPoolItem(which)
{
    short nTheValue = 0;
    rStream.ReadInt16( nTheValue );
    m_nValue = nTheValue;
}

// virtual
bool SfxInt16Item::operator ==(const SfxPoolItem & rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return m_nValue == static_cast< const SfxInt16Item * >(&rItem)->
                        m_nValue;
}

// virtual
bool SfxInt16Item::GetPresentation(SfxItemPresentation,
                                                  MapUnit, MapUnit,
                                                  OUString & rText,
                                                  const IntlWrapper&) const
{
    rText = OUString::number(m_nValue);
    return true;
}


// virtual
bool SfxInt16Item::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    sal_Int16 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

// virtual
bool SfxInt16Item::PutValue(const css::uno::Any& rVal, sal_uInt8 )
{
    sal_Int16 nValue = sal_Int16();
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return true;
    }

    OSL_FAIL( "SfxInt16Item::PutValue - Wrong type!" );
    return false;
}

// virtual
SfxPoolItem * SfxInt16Item::Create(SvStream & rStream, sal_uInt16) const
{
    return new SfxInt16Item(Which(), rStream);
}

// virtual
SvStream & SfxInt16Item::Store(SvStream & rStream, sal_uInt16) const
{
    rStream.WriteInt16( short(m_nValue) );
    return rStream;
}

SfxPoolItem * SfxInt16Item::Clone(SfxItemPool *) const
{
    return new SfxInt16Item(*this);
}

//  class SfxUInt16Item
SfxPoolItem* SfxUInt16Item::CreateDefault()
{
    return new SfxUInt16Item();
};

void SfxUInt16Item::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SfxUInt16Item"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}


//  class SfxInt32Item


SfxPoolItem* SfxInt32Item::CreateDefault()
{
    return new SfxInt32Item();
};


//  class SfxUInt32Item


SfxPoolItem* SfxUInt32Item::CreateDefault()
{
    return new SfxUInt32Item();
};


SfxMetricItem::SfxMetricItem(sal_uInt16 which, sal_uInt32 nValue):
    SfxInt32Item(which, nValue)
{
}

// virtual
void SfxMetricItem::ScaleMetrics(long nMult, long nDiv)
{
    BigInt aTheValue(GetValue());
    aTheValue *= nMult;
    aTheValue += nDiv / 2;
    aTheValue /= nDiv;
    SetValue(aTheValue);
}

// virtual
bool SfxMetricItem::HasMetrics() const
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
