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
#include <svl/metitem.hxx>
#include <libxml/xmlwriter.h>
#include <boost/property_tree/ptree.hpp>




SfxPoolItem* SfxByteItem::CreateDefault()
{
    return new SfxByteItem();
};

SfxPoolItem* SfxInt16Item::CreateDefault()
{
    return new SfxInt16Item();
};

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


boost::property_tree::ptree SfxInt16Item::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = SfxPoolItem::dumpAsJSON();
    aTree.put("state", GetValue());
    return aTree;
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

SfxInt16Item* SfxInt16Item::Clone(SfxItemPool *) const
{
    return new SfxInt16Item(*this);
}

SfxPoolItem* SfxUInt16Item::CreateDefault()
{
    return new SfxUInt16Item();
};

void SfxUInt16Item::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxUInt16Item"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

boost::property_tree::ptree SfxUInt16Item::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = SfxPoolItem::dumpAsJSON();
    aTree.put("state", GetValue());
    return aTree;
}




SfxPoolItem* SfxInt32Item::CreateDefault()
{
    return new SfxInt32Item();
};

void SfxInt32Item::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxInt32Item"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

boost::property_tree::ptree SfxInt32Item::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = SfxPoolItem::dumpAsJSON();
    aTree.put("state", GetValue());
    return aTree;
}




SfxPoolItem* SfxUInt32Item::CreateDefault()
{
    return new SfxUInt32Item();
};

void SfxUInt32Item::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxUInt32Item"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

boost::property_tree::ptree SfxUInt32Item::dumpAsJSON() const
{
    boost::property_tree::ptree aTree = SfxPoolItem::dumpAsJSON();
    aTree.put("state", GetValue());
    return aTree;
}

SfxMetricItem::SfxMetricItem(sal_uInt16 which, sal_uInt32 nValue):
    SfxInt32Item(which, nValue)
{
}

// virtual
void SfxMetricItem::ScaleMetrics(tools::Long nMult, tools::Long nDiv)
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
