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

#include "OOXMLPropertySet.hxx"
#include <stdio.h>
#include <iostream>
#include <ooxml/QNameToString.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <sax/tools/converter.hxx>
#include <tools/color.hxx>
#include <o3tl/string_view.hxx>
#include <utility>

namespace writerfilter::ooxml
{
using namespace com::sun::star;

OOXMLProperty::OOXMLProperty(Id id, const OOXMLValue& pValue,
                             OOXMLProperty::Type_t eType)
    : mId(id), maValue(pValue), meType(eType)
{
}

OOXMLProperty::~OOXMLProperty()
{
}

sal_uInt32 OOXMLProperty::getId() const
{
    return mId;
}

writerfilter::Reference<Properties>::Pointer_t OOXMLProperty::getProps()
{
    if (maValue.hasValue())
        return maValue.getProperties();
    return nullptr;
}

#ifdef DBG_UTIL
std::string OOXMLProperty::getName() const
{
    std::string sResult(QNameToString(mId));

    if (sResult.length() == 0)
        sResult = fastTokenToId(mId);

    if (sResult.length() == 0)
    {
        static char sBuffer[256];

        snprintf(sBuffer, sizeof(sBuffer), "%" SAL_PRIxUINT32, mId);
        sResult = sBuffer;
    }

    return sResult;
}
#endif

#ifdef DBG_UTIL
std::string OOXMLProperty::toString() const
{
    std::string sResult = "(";

    sResult += getName();
    sResult += ", ";
    if (maValue.hasValue())
        sResult += maValue.toString();
    else
        sResult +="(null)";
    sResult +=")";

    return sResult;
}
#endif

void OOXMLProperty::resolve(writerfilter::Properties & rProperties)
{
    switch (meType)
    {
    case SPRM:
        if (mId != 0x0)
            rProperties.sprm(*this);
        break;
    case ATTRIBUTE:
        rProperties.attribute(mId, *getValue());
        break;
    }
}

/*
   class OOXMLValue
*/

OOXMLValue::OOXMLValue(VariantType aData) : maData(std::move(aData))
{
}

OOXMLValue::~OOXMLValue()
{
}

int OOXMLValue::getInt() const
{
    assert(maData.index() != 0 && "this OOXMLValue is empty");
    switch(maData.index())
    {
        case 1: return std::get<bool>(maData) ? 1 : 0; // bool
        case 2: return std::get<2>(maData); // int
        case 3: return std::get<3>(maData); // universal measure
        case 4: return std::get<4>(maData); // measure or percent
        case 5: return std::get<5>(maData); // hex
    }
    return 0;
}

OUString OOXMLValue::getString() const
{
    assert(maData.index() != 0 && "this OOXMLValue is empty");
    if (std::holds_alternative<OUString>(maData))
        return std::get<OUString>(maData);
    return OUString();
}

uno::Any OOXMLValue::getAny() const
{
    assert(maData.index() != 0 && "this OOXMLValue is empty");
    switch(maData.index())
    {
        case  1: return uno::Any(std::get<bool>(maData));
        case  2: return uno::Any(std::get<2>(maData)); // int
        case  6: return uno::Any(std::get<OUString>(maData));
        case  9: return uno::Any(std::get<uno::Reference<io::XInputStream>>(maData));
        case 10: return uno::Any(std::get<uno::Reference<drawing::XShape>>(maData));
        case 11: return uno::Any(std::get<uno::Reference<embed::XEmbeddedObject>>(maData));
        default: break;
    }
    return uno::Any();
}

writerfilter::Reference<Properties>::Pointer_t OOXMLValue::getProperties() const
{
    assert(maData.index() != 0 && "this OOXMLValue is empty");
    if (std::holds_alternative<tools::SvRef<OOXMLPropertySet>>(maData))
        return std::get<tools::SvRef<OOXMLPropertySet>>(maData).get();
    return writerfilter::Reference<Properties>::Pointer_t();
}

writerfilter::Reference<BinaryObj>::Pointer_t OOXMLValue::getBinary() const
{
    assert(maData.index() != 0 && "this OOXMLValue is empty");
    if (std::holds_alternative<writerfilter::Reference<BinaryObj>::Pointer_t>(maData))
        return std::get<writerfilter::Reference<BinaryObj>::Pointer_t>(maData);
    return writerfilter::Reference<BinaryObj>::Pointer_t();
}

#ifdef DBG_UTIL
std::string OOXMLValue::toString() const
{
    switch(maData.index())
    {
        case 0: return "empty";
        case 1: return std::get<bool>(maData) ? "true" : "false";
        case 2: // int
        {
            auto i = std::get<2>(maData);
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "%" SAL_PRIdINT32, i);
            return buffer;
        }
        case 3: // universal measure
            return std::string(OString::number(std::get<3>(maData)));
        case 4: // measure or percent
            return std::string(OString::number(std::get<4>(maData)));
        case 5: // hex
        {
            auto i = std::get<5>(maData);
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "0x%" SAL_PRIxUINT32, i);
            return buffer;
        }
        case 6: // OUString
            return std::string(OUStringToOString(std::get<OUString>(maData), RTL_TEXTENCODING_ASCII_US));
        case 7: // Any
            return "Any";
            break;
        case 8: // PropertySet
        {
            auto pPropertySet = std::get<tools::SvRef<OOXMLPropertySet>>(maData);
            char sBuffer[256];
            snprintf(sBuffer, sizeof(sBuffer), "t:%p, m:%p", this, pPropertySet.get());
            return "OOXMLPropertySet(" + std::string(sBuffer) + ")";
        }
        case 9:
            return "BinaryObj";
        case 10:
            return "InputStream";
        case 11:
            return "Shape";
        case 12:
            return "StarMath";
        default:
            assert(false);
    }
    return "";
}
#endif

OOXMLValue* OOXMLValue::clone() const
{
    return new OOXMLValue(maData);
}


// static
OOXMLValue OOXMLValue::createBoolean(bool bValue)
{
    return OOXMLValue(VariantType(std::in_place_type<bool>, bValue));
}

// static
OOXMLValue OOXMLValue::createBoolean(std::string_view bValue)
{
    return OOXMLValue(VariantType(std::in_place_type<bool>, GetBooleanValue(bValue)));
}

// static
OOXMLValue OOXMLValue::createInteger(int aValue)
{
    return OOXMLValue(VariantType(std::in_place_index_t<2>(), aValue));
}

// static
OOXMLValue OOXMLValue::createHex(sal_uInt32 aValue)
{
    return OOXMLValue(VariantType(std::in_place_index_t<5>(), aValue));
}

// static
OOXMLValue OOXMLValue::createHex(std::string_view pValue)
{
    return OOXMLValue(VariantType(std::in_place_index_t<5>(), o3tl::toUInt32(pValue, 16)));
}

// static
OOXMLValue OOXMLValue::createHexColor(std::string_view pValue)
{
    sal_uInt32 aValue(COL_AUTO);
    if (pValue == "auto")
        return OOXMLValue(VariantType(std::in_place_index_t<5>(), aValue));

    aValue = o3tl::toUInt32(pValue, 16);

    // Convert hash-encoded values (like #FF0080)
    const sal_Int32 nLen = pValue.size();
    if ( !aValue && nLen > 1 && pValue[0] == '#' )
    {
        Color aColor(COL_AUTO);
        // Word appears to require strict 6 digit length, else it ignores it
        if ( nLen == 7 )
        {
            const OUString sHashColor(pValue.data(), nLen, RTL_TEXTENCODING_ASCII_US);
            sax::Converter::convertColor(aColor, sHashColor);
        }
        aValue = sal_uInt32(aColor);
    }
    return OOXMLValue(VariantType(std::in_place_index_t<5>(), aValue));
}

// static
OOXMLValue OOXMLValue::createString(const OUString& bValue)
{
    return OOXMLValue(VariantType(std::in_place_type<OUString>, bValue));
}

// static
OOXMLValue OOXMLValue::createShape(const css::uno::Reference<css::drawing::XShape>& xShape)
{
    return OOXMLValue(VariantType(std::in_place_type<css::uno::Reference<css::drawing::XShape>>, xShape));
}

// static
OOXMLValue OOXMLValue::createBinary(const OOXMLBinaryObjectReference::Pointer_t& pBinaryObj)
{
    return OOXMLValue(VariantType(std::in_place_type<OOXMLBinaryObjectReference::Pointer_t>, pBinaryObj));
}

// static
OOXMLValue OOXMLValue::createInputStream(const uno::Reference<io::XInputStream>& rValue)
{
    return OOXMLValue(VariantType(std::in_place_type<uno::Reference<io::XInputStream>>, rValue));
}

// static
OOXMLValue OOXMLValue::createPropertySet(const tools::SvRef<OOXMLPropertySet>& rValue)
{
    return OOXMLValue(VariantType(std::in_place_type<tools::SvRef<OOXMLPropertySet>>, rValue));
}

// static
OOXMLValue OOXMLValue::createStarMath(const uno::Reference< embed::XEmbeddedObject >& rValue)
{
    return OOXMLValue(VariantType(std::in_place_type<uno::Reference< embed::XEmbeddedObject >>, rValue));
}

bool GetBooleanValue(std::string_view pValue)
{
    return pValue == "true"
           || pValue == "True"
           || pValue == "1"
           || pValue == "on"
           || pValue == "On";
}


/**
   class OOXMLPropertySet
*/

OOXMLPropertySet::OOXMLPropertySet()
{
}

OOXMLPropertySet::~OOXMLPropertySet()
{
}

void OOXMLPropertySet::resolve(Properties & rHandler)
{
    // The pProp->resolve(rHandler) call below can cause elements to
    // be appended to mProperties. I don't think it can cause elements
    // to be deleted. But let's check with < here just to be safe that
    // the indexing below works.
    for (size_t nIt = 0; nIt < mProperties.size(); ++nIt)
    {
        OOXMLProperty::Pointer_t pProp = mProperties[nIt];

        if (pProp)
            pProp->resolve(rHandler);
    }
}

OOXMLPropertySet::OOXMLProperties_t::iterator OOXMLPropertySet::begin()
{
    return mProperties.begin();
}

OOXMLPropertySet::OOXMLProperties_t::iterator OOXMLPropertySet::end()
{
    return mProperties.end();
}

OOXMLPropertySet::OOXMLProperties_t::const_iterator
OOXMLPropertySet::begin() const
{
    return mProperties.begin();
}

OOXMLPropertySet::OOXMLProperties_t::const_iterator
OOXMLPropertySet::end() const
{
    return mProperties.end();
}

void OOXMLPropertySet::add(const OOXMLProperty::Pointer_t& pProperty)
{
    if (pProperty && pProperty->getId() != 0x0)
    {
        mProperties.push_back(pProperty);
    }
}

void OOXMLPropertySet::add(Id id, const OOXMLValue& pValue, OOXMLProperty::Type_t eType)
{
    OOXMLProperty::Pointer_t pProperty(new OOXMLProperty(id, pValue, eType));
    add(pProperty);
}

void OOXMLPropertySet::add(const OOXMLPropertySet::Pointer_t& pPropertySet)
{
    const OOXMLPropertySet * pSet = pPropertySet.get();

    if (pSet != nullptr)
    {
        mProperties.insert( mProperties.end(), pSet->mProperties.begin(), pSet->mProperties.end() );
    }
}

OOXMLPropertySet * OOXMLPropertySet::clone() const
{
    return new OOXMLPropertySet(*this);
}

#ifdef DBG_UTIL
std::string OOXMLPropertySet::toString()
{
    std::string sResult = "[";
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%p", this);
    sResult += sBuffer;
    sResult += ":";

    OOXMLProperties_t::iterator aItBegin = begin();
    OOXMLProperties_t::iterator aItEnd = end();

    for (OOXMLProperties_t::iterator aIt = aItBegin; aIt != aItEnd; ++aIt)
    {
        if (aIt != aItBegin)
            sResult += ", ";

        if (*aIt)
            sResult += (*aIt)->toString();
        else
            sResult += "0x0";
    }

    sResult += "]";

    return sResult;
}
#endif

// OOXMLUniversalMeasureValue
// ECMA-376 5th ed. Part 1 , 22.9.2.15
// static
OOXMLValue OOXMLValue::createUniversalMeasure(std::string_view pValue, sal_uInt32 npPt)
{
    double val = o3tl::toDouble(pValue); // will ignore the trailing unit

    if (pValue.ends_with("pt"))
    {
        val *= npPt;
    }
    else if (pValue.ends_with("cm"))
    {
        val = o3tl::convert(val, o3tl::Length::cm, o3tl::Length::pt) * npPt;
    }
    else if (pValue.ends_with("mm"))
    {
        val = o3tl::convert(val, o3tl::Length::mm, o3tl::Length::pt) * npPt;
    }
    else if (pValue.ends_with("in"))
    {
        val = o3tl::convert(val, o3tl::Length::in, o3tl::Length::pt) * npPt;
    }
    else if (pValue.ends_with("pc") || pValue.ends_with("pi"))
    {
        val = o3tl::convert(val, o3tl::Length::pc, o3tl::Length::pt) * npPt;
    }

    int nValue = std::round(val);
    return OOXMLValue(VariantType(std::in_place_index_t<3>(), nValue));
}

// static
OOXMLValue OOXMLValue::createEmuMeasure(std::string_view pValue)
{
    return createUniversalMeasure(pValue, 12700); // 12,700 English Metric Units (EMU) = 1 pt
}

// static
OOXMLValue OOXMLValue::createTwipsMeasure(std::string_view pValue)
{
    return createUniversalMeasure(pValue, 20);
}

// static
OOXMLValue OOXMLValue::createHpsMeasure(std::string_view pValue)
{
    return createUniversalMeasure(pValue, 2);
}

// OOXMLMeasurementOrPercentValue
// ECMA-376 5th ed. Part 1 , 17.18.107; 17.18.11
// static
OOXMLValue OOXMLValue::createMeasurementOrPercent(std::string_view pValue)
{
    double val = o3tl::toDouble(pValue); // will ignore the trailing unit

    int nValue;
    int nLen = pValue.size();
    if (nLen > 1 &&
        pValue[nLen - 1] == '%')
    {
        nValue = static_cast<int>(val * 50);
    }
    else
    {
        nValue = createTwipsMeasure(pValue).getInt();
    }
    return OOXMLValue(VariantType(std::in_place_index_t<4>(), nValue));
}


/*
  class OOXMLTableImpl
 */

OOXMLTable::OOXMLTable()
{
}

OOXMLTable::~OOXMLTable()
{
}


void OOXMLTable::resolve(Table & rTable)
{
    Table * pTable = &rTable;

    int nPos = 0;

    for (const auto& rPropSet : mPropertySets)
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties
            (rPropSet.getProperties());

        if (pProperties)
            pTable->entry(nPos, std::move(pProperties));

        ++nPos;
    }
}

void OOXMLTable::add(const OOXMLValue& pPropertySet)
{
    if (pPropertySet.hasValue())
        mPropertySets.push_back(pPropertySet);
}

OOXMLTable * OOXMLTable::clone() const
{
    return new OOXMLTable(*this);
}

/*
  class: OOXMLPropertySetEntryToString
*/

OOXMLPropertySetEntryToString::OOXMLPropertySetEntryToString(Id nId)
: mnId(nId)
{
}

OOXMLPropertySetEntryToString::~OOXMLPropertySetEntryToString()
{
}

void OOXMLPropertySetEntryToString::sprm(Sprm & /*rSprm*/)
{
}

void OOXMLPropertySetEntryToString::attribute(Id nId, const Value & rValue)
{
    if (nId == mnId)
        mStr = rValue.getString();
}

/*
  class: OOXMLPropertySetEntryToInteger
*/

OOXMLPropertySetEntryToInteger::OOXMLPropertySetEntryToInteger(Id nId)
: mnId(nId), mnValue(0)
{
}

OOXMLPropertySetEntryToInteger::~OOXMLPropertySetEntryToInteger()
{
}

void OOXMLPropertySetEntryToInteger::sprm(Sprm & /*rSprm*/)
{
}

void OOXMLPropertySetEntryToInteger::attribute(Id nId, const Value & rValue)
{
    if (nId == mnId)
        mnValue = rValue.getInt();
}

/*
  class: OOXMLPropertySetEntryToBool
*/

OOXMLPropertySetEntryToBool::OOXMLPropertySetEntryToBool(Id nId)
    : mnId(nId), mValue(false)
{}

OOXMLPropertySetEntryToBool::~OOXMLPropertySetEntryToBool() {}

void OOXMLPropertySetEntryToBool::sprm(Sprm & /*rSprm*/) {}

void OOXMLPropertySetEntryToBool::attribute(Id nId, const Value & rValue)
{
    if (nId == mnId)
        mValue = (rValue.getInt() != 0);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
