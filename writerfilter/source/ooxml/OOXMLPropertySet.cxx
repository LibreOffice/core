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

OOXMLProperty::OOXMLProperty(Id id, OOXMLValue::Pointer_t pValue,
                             OOXMLProperty::Type_t eType)
    : mId(id), mpValue(std::move(pValue)), meType(eType)
{
}

OOXMLProperty::~OOXMLProperty()
{
}

sal_uInt32 OOXMLProperty::getId() const
{
    return mId;
}

Value::Pointer_t OOXMLProperty::getValue()
{
    Value::Pointer_t pResult;

    if (mpValue)
        pResult = Value::Pointer_t(mpValue->clone());
    else
        pResult = Value::Pointer_t(new OOXMLValue());

    return pResult;
}

writerfilter::Reference<Properties>::Pointer_t OOXMLProperty::getProps()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    if (mpValue)
        pResult = mpValue->getProperties();

    return pResult;
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
    if (mpValue)
        sResult += mpValue->toString();
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

OOXMLValue::OOXMLValue()
{
}

OOXMLValue::~OOXMLValue()
{
}

int OOXMLValue::getInt() const
{
    return 0;
}

OUString OOXMLValue::getString() const
{
    return OUString();
}

uno::Any OOXMLValue::getAny() const
{
    return uno::Any();
}

writerfilter::Reference<Properties>::Pointer_t OOXMLValue::getProperties()
{
    return writerfilter::Reference<Properties>::Pointer_t();
}

writerfilter::Reference<BinaryObj>::Pointer_t OOXMLValue::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t();
}

#ifdef DBG_UTIL
std::string OOXMLValue::toString() const
{
    return "OOXMLValue";
}
#endif

OOXMLValue * OOXMLValue::clone() const
{
    return new OOXMLValue(*this);
}

/*
  class OOXMLBinaryValue
 */

OOXMLBinaryValue::OOXMLBinaryValue(OOXMLBinaryObjectReference::Pointer_t pBinaryObj)
: mpBinaryObj(std::move(pBinaryObj))
{
}

OOXMLBinaryValue::~OOXMLBinaryValue()
{
}

writerfilter::Reference<BinaryObj>::Pointer_t OOXMLBinaryValue::getBinary()
{
    return mpBinaryObj;
}

#ifdef DBG_UTIL
std::string OOXMLBinaryValue::toString() const
{
    return "BinaryObj";
}
#endif

OOXMLValue * OOXMLBinaryValue::clone() const
{
    return new OOXMLBinaryValue(mpBinaryObj);
}

/*
  class OOXMLBooleanValue
*/

bool GetBooleanValue(std::string_view pValue)
{
    return pValue == "true"
           || pValue == "True"
           || pValue == "1"
           || pValue == "on"
           || pValue == "On";
}

OOXMLValue::Pointer_t const & OOXMLBooleanValue::Create(bool bValue)
{
    static OOXMLValue::Pointer_t False(new OOXMLBooleanValue (false));
    static OOXMLValue::Pointer_t True(new OOXMLBooleanValue (true));

    return bValue ? True : False;
}

OOXMLValue::Pointer_t const & OOXMLBooleanValue::Create(std::string_view pValue)
{
    return Create (GetBooleanValue(pValue));
}

OOXMLBooleanValue::OOXMLBooleanValue(bool bValue)
: mbValue(bValue)
{
}

OOXMLBooleanValue::~OOXMLBooleanValue()
{
}

int OOXMLBooleanValue::getInt() const
{
    return mbValue ? 1 : 0;
}

uno::Any OOXMLBooleanValue::getAny() const
{
    return uno::Any(mbValue);
}

#ifdef DBG_UTIL
std::string OOXMLBooleanValue::toString() const
{
    return mbValue ? "true" : "false";
}
#endif

OOXMLValue * OOXMLBooleanValue::clone() const
{
    return new OOXMLBooleanValue(*this);
}

/*
  class OOXMLStringValue
*/

OOXMLStringValue::OOXMLStringValue(OUString sStr)
: mStr(std::move(sStr))
{
}

OOXMLStringValue::~OOXMLStringValue()
{
}

uno::Any OOXMLStringValue::getAny() const
{
    return uno::Any(mStr);
}

OUString OOXMLStringValue::getString() const
{
    return mStr;
}

#ifdef DBG_UTIL
std::string OOXMLStringValue::toString() const
{
    return std::string(OUStringToOString(mStr, RTL_TEXTENCODING_ASCII_US));
}
#endif

OOXMLValue * OOXMLStringValue::clone() const
{
    return new OOXMLStringValue(*this);
}

/*
  class OOXMLInputStreamValue
 */
OOXMLInputStreamValue::OOXMLInputStreamValue(uno::Reference<io::XInputStream> xInputStream)
: mxInputStream(std::move(xInputStream))
{
}

OOXMLInputStreamValue::~OOXMLInputStreamValue()
{
}

uno::Any OOXMLInputStreamValue::getAny() const
{
    return uno::Any(mxInputStream);
}

#ifdef DBG_UTIL
std::string OOXMLInputStreamValue::toString() const
{
    return "InputStream";
}
#endif

OOXMLValue * OOXMLInputStreamValue::clone() const
{
    return new OOXMLInputStreamValue(mxInputStream);
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

void OOXMLPropertySet::add(Id id, const OOXMLValue::Pointer_t& pValue, OOXMLProperty::Type_t eType)
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

/*
  class OOXMLPropertySetValue
*/

OOXMLPropertySetValue::OOXMLPropertySetValue(OOXMLPropertySet::Pointer_t pPropertySet)
    : mpPropertySet(std::move(pPropertySet))
{
}

OOXMLPropertySetValue::~OOXMLPropertySetValue()
{
}

writerfilter::Reference<Properties>::Pointer_t OOXMLPropertySetValue::getProperties()
{
    return writerfilter::Reference<Properties>::Pointer_t
        (mpPropertySet->clone());
}

#ifdef DBG_UTIL
std::string OOXMLPropertySetValue::toString() const
{
    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "t:%p, m:%p", this, mpPropertySet.get());

    return "OOXMLPropertySetValue(" + std::string(sBuffer) + ")";
}
#endif

OOXMLValue * OOXMLPropertySetValue::clone() const
{
    return new OOXMLPropertySetValue(*this);
}

/*
  class OOXMLIntegerValue
*/

OOXMLValue::Pointer_t OOXMLIntegerValue::Create(sal_Int32 nValue)
{
    static OOXMLValue::Pointer_t Zero(new OOXMLIntegerValue (0));
    static OOXMLValue::Pointer_t One(new OOXMLIntegerValue (1));
    static OOXMLValue::Pointer_t Two(new OOXMLIntegerValue (2));
    static OOXMLValue::Pointer_t Three(new OOXMLIntegerValue (3));
    static OOXMLValue::Pointer_t Four(new OOXMLIntegerValue (4));
    static OOXMLValue::Pointer_t Five(new OOXMLIntegerValue (5));
    static OOXMLValue::Pointer_t Six(new OOXMLIntegerValue (6));
    static OOXMLValue::Pointer_t Seven(new OOXMLIntegerValue (7));
    static OOXMLValue::Pointer_t Eight(new OOXMLIntegerValue (8));
    static OOXMLValue::Pointer_t Nine(new OOXMLIntegerValue (9));

    switch (nValue) {
    case 0: return Zero;
    case 1: return One;
    case 2: return Two;
    case 3: return Three;
    case 4: return Four;
    case 5: return Five;
    case 6: return Six;
    case 7: return Seven;
    case 8: return Eight;
    case 9: return Nine;
    default: break;
    }

    OOXMLValue::Pointer_t value(new OOXMLIntegerValue(nValue));

    return value;
}

OOXMLIntegerValue::OOXMLIntegerValue(sal_Int32 nValue)
: mnValue(nValue)
{
}

OOXMLIntegerValue::~OOXMLIntegerValue()
{
}

int OOXMLIntegerValue::getInt() const
{
    return mnValue;
}

uno::Any OOXMLIntegerValue::getAny() const
{
    return uno::Any(mnValue);
}

OOXMLValue * OOXMLIntegerValue::clone() const
{
    return new OOXMLIntegerValue(*this);
}

#ifdef DBG_UTIL
std::string OOXMLIntegerValue::toString() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%" SAL_PRIdINT32, mnValue);

    return buffer;
}
#endif

/*
  class OOXMLHexValue
*/

OOXMLHexValue::OOXMLHexValue(sal_uInt32 nValue)
: mnValue(nValue)
{
}

OOXMLHexValue::OOXMLHexValue(std::string_view pValue)
: mnValue(o3tl::toUInt32(pValue, 16))
{
}

OOXMLHexValue::~OOXMLHexValue()
{
}

int OOXMLHexValue::getInt() const
{
    return mnValue;
}

OOXMLValue * OOXMLHexValue::clone() const
{
    return new OOXMLHexValue(*this);
}

#ifdef DBG_UTIL
std::string OOXMLHexValue::toString() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "0x%" SAL_PRIxUINT32, mnValue);

    return buffer;
}
#endif

/*
  class OOXMLHexColorValue
*/
OOXMLHexColorValue::OOXMLHexColorValue(std::string_view pValue)
    : OOXMLHexValue(sal_uInt32(COL_AUTO))
{
    if (pValue == "auto")
        return;

    mnValue = o3tl::toUInt32(pValue, 16);

    // Convert hash-encoded values (like #FF0080)
    const sal_Int32 nLen = pValue.size();
    if ( !mnValue && nLen > 1 && pValue[0] == '#' )
    {
        sal_Int32 nColor(COL_AUTO);
        // Word appears to require strict 6 digit length, else it ignores it
        if ( nLen == 7 )
        {
            const OUString sHashColor(pValue.data(), nLen, RTL_TEXTENCODING_ASCII_US);
            sax::Converter::convertColor( nColor, sHashColor );
        }
        mnValue = nColor;
    }
}

// OOXMLUniversalMeasureValue
// ECMA-376 5th ed. Part 1 , 22.9.2.15
OOXMLUniversalMeasureValue::OOXMLUniversalMeasureValue(std::string_view pValue, sal_uInt32 npPt)
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

    mnValue = std::round(val);
}

OOXMLUniversalMeasureValue::~OOXMLUniversalMeasureValue()
{
}

int OOXMLUniversalMeasureValue::getInt() const
{
    return mnValue;
}

#ifdef DBG_UTIL
std::string OOXMLUniversalMeasureValue::toString() const
{
    return std::string(OString::number(mnValue));
}
#endif

// OOXMLMeasurementOrPercentValue
// ECMA-376 5th ed. Part 1 , 17.18.107; 17.18.11
OOXMLMeasurementOrPercentValue::OOXMLMeasurementOrPercentValue(std::string_view pValue)
{
    double val = o3tl::toDouble(pValue); // will ignore the trailing unit

    int nLen = pValue.size();
    if (nLen > 1 &&
        pValue[nLen - 1] == '%')
    {
        mnValue = static_cast<int>(val * 50);
    }
    else
    {
        mnValue = OOXMLTwipsMeasureValue(pValue).getInt();
    }
}

int OOXMLMeasurementOrPercentValue::getInt() const
{
    return mnValue;
}

#ifdef DBG_UTIL
std::string OOXMLMeasurementOrPercentValue::toString() const
{
    return std::string(OString::number(mnValue));
}
#endif

/*
  class OOXMLShapeValue
 */


OOXMLShapeValue::OOXMLShapeValue(uno::Reference<drawing::XShape> xShape)
: mrShape(std::move(xShape))
{
}

OOXMLShapeValue::~OOXMLShapeValue()
{
}

uno::Any OOXMLShapeValue::getAny() const
{
    return uno::Any(mrShape);
}

#ifdef DBG_UTIL
std::string OOXMLShapeValue::toString() const
{
    return "Shape";
}
#endif

OOXMLValue * OOXMLShapeValue::clone() const
{
    return new OOXMLShapeValue(mrShape);
}

/*
  class OOXMLStarMathValue
 */


OOXMLStarMathValue::OOXMLStarMathValue( uno::Reference< embed::XEmbeddedObject > c )
: m_component(std::move(c))
{
}

OOXMLStarMathValue::~OOXMLStarMathValue()
{
}

uno::Any OOXMLStarMathValue::getAny() const
{
    return uno::Any(m_component);
}

#ifdef DBG_UTIL
std::string OOXMLStarMathValue::toString() const
{
    return "StarMath";
}
#endif

OOXMLValue * OOXMLStarMathValue::clone() const
{
    return new OOXMLStarMathValue( m_component );
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
            (rPropSet->getProperties());

        if (pProperties)
            pTable->entry(nPos, pProperties);

        ++nPos;
    }
}

void OOXMLTable::add(const ValuePointer_t& pPropertySet)
{
    if (pPropertySet)
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

void OOXMLPropertySetEntryToString::attribute(Id nId, Value & rValue)
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

void OOXMLPropertySetEntryToInteger::attribute(Id nId, Value & rValue)
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

void OOXMLPropertySetEntryToBool::attribute(Id nId, Value & rValue)
{
    if (nId == mnId)
        mValue = (rValue.getInt() != 0);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
