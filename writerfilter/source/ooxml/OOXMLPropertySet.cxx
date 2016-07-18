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
#include <oox/token/tokens.hxx>

namespace writerfilter {
namespace ooxml
{
using namespace ::std;
using namespace com::sun::star;

OOXMLProperty::OOXMLProperty(Id id, OOXMLValue::Pointer_t pValue,
                                     OOXMLProperty::Type_t eType)
: mId(id), mpValue(pValue), meType(eType)
{
}

OOXMLProperty::OOXMLProperty(const OOXMLProperty & rSprm)
: mId(rSprm.mId), mpValue(rSprm.mpValue), meType(rSprm.meType)
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

    if (mpValue.get() != nullptr)
        pResult = Value::Pointer_t(mpValue->clone());
    else
        pResult = Value::Pointer_t(new OOXMLValue());

    return pResult;
}

writerfilter::Reference<Properties>::Pointer_t OOXMLProperty::getProps()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    if (mpValue.get() != nullptr)
        pResult = mpValue->getProperties();

    return pResult;
}

#ifdef DEBUG_WRITERFILTER
string OOXMLProperty::getName() const
{
    string sResult;

    sResult = (*QNameToString::Instance())(mId);

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

#ifdef DEBUG_WRITERFILTER
string OOXMLProperty::toString() const
{
    string sResult = "(";

    sResult += getName();
    sResult += ", ";
    if (mpValue.get() != nullptr)
        sResult += mpValue->toString();
    else
        sResult +="(null)";
    sResult +=")";

    return sResult;
}
#endif

void OOXMLProperty::resolve(writerfilter::Properties & rProperties)
{
    writerfilter::Properties * pProperties = nullptr;
    pProperties = &rProperties;

    switch (meType)
    {
    case SPRM:
        if (mId != 0x0)
            pProperties->sprm(*this);
        break;
    case ATTRIBUTE:
        pProperties->attribute(mId, *getValue());
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

#ifdef DEBUG_WRITERFILTER
string OOXMLValue::toString() const
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

OOXMLBinaryValue::OOXMLBinaryValue(OOXMLBinaryObjectReference::Pointer_t
                                   pBinaryObj)
: mpBinaryObj(pBinaryObj)
{
}

OOXMLBinaryValue::~OOXMLBinaryValue()
{
}

writerfilter::Reference<BinaryObj>::Pointer_t OOXMLBinaryValue::getBinary()
{
    return mpBinaryObj;
}

#ifdef DEBUG_WRITERFILTER
string OOXMLBinaryValue::toString() const
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

static bool GetBooleanValue(const char *pValue)
{
    return !strcmp(pValue, "true")
           || !strcmp(pValue, "True")
           || !strcmp(pValue, "1")
           || !strcmp(pValue, "on")
           || !strcmp(pValue, "On");
}

OOXMLValue::Pointer_t const & OOXMLBooleanValue::Create(bool bValue)
{
    static OOXMLValue::Pointer_t False(new OOXMLBooleanValue (false));
    static OOXMLValue::Pointer_t True(new OOXMLBooleanValue (true));

    return bValue ? True : False;
}

OOXMLValue::Pointer_t OOXMLBooleanValue::Create(const char *pValue)
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
    uno::Any aResult(mbValue);

    return aResult;
}

#ifdef DEBUG_WRITERFILTER
string OOXMLBooleanValue::toString() const
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

OOXMLStringValue::OOXMLStringValue(const OUString & rStr)
: mStr(rStr)
{
}

OOXMLStringValue::~OOXMLStringValue()
{
}

uno::Any OOXMLStringValue::getAny() const
{
    uno::Any aAny(mStr);

    return aAny;
}

OUString OOXMLStringValue::getString() const
{
    return mStr;
}

#ifdef DEBUG_WRITERFILTER
string OOXMLStringValue::toString() const
{
    return OUStringToOString(mStr, RTL_TEXTENCODING_ASCII_US).getStr();
}
#endif

OOXMLValue * OOXMLStringValue::clone() const
{
    return new OOXMLStringValue(*this);
}

/*
  class OOXMLInputStreamValue
 */
OOXMLInputStreamValue::OOXMLInputStreamValue(uno::Reference<io::XInputStream> const & xInputStream)
: mxInputStream(xInputStream)
{
}

OOXMLInputStreamValue::~OOXMLInputStreamValue()
{
}

uno::Any OOXMLInputStreamValue::getAny() const
{
    uno::Any aAny(mxInputStream);

    return aAny;
}

#ifdef DEBUG_WRITERFILTER
string OOXMLInputStreamValue::toString() const
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
    maType = "OOXMLPropertySet";
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

        if (pProp.get() != nullptr)
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
    if (pProperty.get() != nullptr && pProperty->getId() != 0x0)
    {
        mProperties.push_back(pProperty);
    }
}

void OOXMLPropertySet::add(const OOXMLPropertySet::Pointer_t& pPropertySet)
{
    if (pPropertySet.get() != nullptr)
    {
        OOXMLPropertySet * pSet = pPropertySet.get();

        if (pSet != nullptr)
        {
            mProperties.resize(mProperties.size() + pSet->mProperties.size());
            for (OOXMLProperties_t::iterator aIt = pSet->mProperties.begin();
                 aIt != pSet->mProperties.end(); ++aIt)
                add(*aIt);
        }
    }
}

OOXMLPropertySet * OOXMLPropertySet::clone() const
{
    return new OOXMLPropertySet(*this);
}

#ifdef DEBUG_WRITERFILTER
string OOXMLPropertySet::toString()
{
    string sResult = "[";
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

        if ((*aIt).get() != nullptr)
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

OOXMLPropertySetValue::OOXMLPropertySetValue
(OOXMLPropertySet::Pointer_t pPropertySet)
: mpPropertySet(pPropertySet)
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

#ifdef DEBUG_WRITERFILTER
string OOXMLPropertySetValue::toString() const
{
    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "t:%p, m:%p", this, mpPropertySet.get());

    return "OOXMLPropertySetValue(" + string(sBuffer) + ")";
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
    uno::Any aResult(mnValue);

    return aResult;
}

OOXMLValue * OOXMLIntegerValue::clone() const
{
    return new OOXMLIntegerValue(*this);
}

#ifdef DEBUG_WRITERFILTER
string OOXMLIntegerValue::toString() const
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

OOXMLHexValue::OOXMLHexValue(const char * pValue)
{
    mnValue = rtl_str_toUInt32(pValue, 16);
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

#ifdef DEBUG_WRITERFILTER
string OOXMLHexValue::toString() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "0x%" SAL_PRIxUINT32, mnValue);

    return buffer;
}
#endif

// OOXMLUniversalMeasureValue

OOXMLUniversalMeasureValue::OOXMLUniversalMeasureValue(const char * pValue)
{
    mnValue = rtl_str_toInt32(pValue, 10); // will ignore the trailing 'pt'

    int nLen = strlen(pValue);
    if (nLen > 2 &&
        pValue[nLen-2] == 'p' &&
        pValue[nLen-1] == 't')
    {
        mnValue = mnValue * 20;
    }
}

OOXMLUniversalMeasureValue::~OOXMLUniversalMeasureValue()
{
}

int OOXMLUniversalMeasureValue::getInt() const
{
    return mnValue;
}

OOXMLValue* OOXMLUniversalMeasureValue::clone() const
{
    return new OOXMLUniversalMeasureValue(*this);
}

#ifdef DEBUG_WRITERFILTER
string OOXMLUniversalMeasureValue::toString() const
{
    return OString::number(mnValue).getStr();
}
#endif

/*
  class OOXMLShapeValue
 */


OOXMLShapeValue::OOXMLShapeValue(uno::Reference<drawing::XShape> const & rShape)
: mrShape(rShape)
{
}

OOXMLShapeValue::~OOXMLShapeValue()
{
}

uno::Any OOXMLShapeValue::getAny() const
{
    return uno::Any(mrShape);
}

#ifdef DEBUG_WRITERFILTER
string OOXMLShapeValue::toString() const
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


OOXMLStarMathValue::OOXMLStarMathValue( uno::Reference< embed::XEmbeddedObject > const & c )
: component(c)
{
}

OOXMLStarMathValue::~OOXMLStarMathValue()
{
}

uno::Any OOXMLStarMathValue::getAny() const
{
    return uno::Any(component);
}

#ifdef DEBUG_WRITERFILTER
string OOXMLStarMathValue::toString() const
{
    return "StarMath";
}
#endif

OOXMLValue * OOXMLStarMathValue::clone() const
{
    return new OOXMLStarMathValue( component );
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

    PropertySets_t::iterator it = mPropertySets.begin();
    PropertySets_t::iterator itEnd = mPropertySets.end();

    while (it != itEnd)
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties
            ((*it)->getProperties());

        if (pProperties.get() != nullptr)
            pTable->entry(nPos, pProperties);

        ++nPos;
        ++it;
    }
}

void OOXMLTable::add(const ValuePointer_t& pPropertySet)
{
    if (pPropertySet.get() != nullptr)
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

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
