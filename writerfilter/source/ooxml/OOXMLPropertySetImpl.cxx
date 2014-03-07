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

#include "OOXMLPropertySetImpl.hxx"
#include <stdio.h>
#include <iostream>
#include <resourcemodel/QNameToString.hxx>
#include <resourcemodel/Protocol.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <ooxml/OOXMLFastTokens.hxx>
#include "ooxmlLoggers.hxx"
#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace ooxml
{
using namespace ::std;

OOXMLProperty::~OOXMLProperty()
{
}

OOXMLPropertySet::~OOXMLPropertySet()
{
}

OOXMLTable::~OOXMLTable()
{
}

OOXMLPropertyImpl::OOXMLPropertyImpl(Id id, OOXMLValue::Pointer_t pValue,
                                     OOXMLPropertyImpl::Type_t eType)
: mId(id), mpValue(pValue), meType(eType)
{
}

OOXMLPropertyImpl::OOXMLPropertyImpl(const OOXMLPropertyImpl & rSprm)
: OOXMLProperty(), mId(rSprm.mId), mpValue(rSprm.mpValue), meType(rSprm.meType)
{
}

OOXMLPropertyImpl::~OOXMLPropertyImpl()
{
}

sal_uInt32 OOXMLPropertyImpl::getId() const
{
    return mId;
}

Value::Pointer_t OOXMLPropertyImpl::getValue()
{
    Value::Pointer_t pResult;

    if (mpValue.get() != NULL)
        pResult = Value::Pointer_t(mpValue->clone());
    else
        pResult = Value::Pointer_t(new OOXMLValue());

    return pResult;
}

writerfilter::Reference<BinaryObj>::Pointer_t OOXMLPropertyImpl::getBinary()
{
    writerfilter::Reference<BinaryObj>::Pointer_t pResult;

    if (mpValue.get() != NULL)
        pResult = mpValue->getBinary();

    return pResult;
}

writerfilter::Reference<Stream>::Pointer_t OOXMLPropertyImpl::getStream()
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    if (mpValue.get() != NULL)
        pResult = mpValue->getStream();

    return pResult;
}

writerfilter::Reference<Properties>::Pointer_t OOXMLPropertyImpl::getProps()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    if (mpValue.get() != NULL)
        pResult = mpValue->getProperties();

    return pResult;
}

string OOXMLPropertyImpl::getName() const
{
    string sResult = (*QNameToString::Instance())(mId);

    if (sResult.length() == 0)
        sResult = (*SprmIdToString::Instance())(mId);

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

string OOXMLPropertyImpl::toString() const
{
    string sResult = "(";

    sResult += getName();
    sResult += ", ";
    if (mpValue.get() != NULL)
        sResult += mpValue->toString();
    else
        sResult +="(null)";
    sResult +=")";

    return sResult;
}

Sprm::Kind OOXMLPropertyImpl::getKind()
{
    return SprmKind(getId());
}

Sprm * OOXMLPropertyImpl::clone()
{
    return new OOXMLPropertyImpl(*this);
}

void OOXMLPropertyImpl::resolve(writerfilter::Properties & rProperties)
{
    writerfilter::Properties * pProperties = NULL;
#ifdef DEBUG_PROTOCOL
    writerfilter::PropertiesProtocol::Pointer_t pProtocol
        (new writerfilter::PropertiesProtocol(&rProperties, debug_logger));
    pProperties = pProtocol.get();
#else
    pProperties = &rProperties;
#endif

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

bool OOXMLValue::getBool() const
{
    return false;
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

writerfilter::Reference<Stream>::Pointer_t OOXMLValue::getStream()
{
    return writerfilter::Reference<Stream>::Pointer_t();
}

writerfilter::Reference<BinaryObj>::Pointer_t OOXMLValue::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t();
}

string OOXMLValue::toString() const
{
    return "OOXMLValue";
}

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

string OOXMLBinaryValue::toString() const
{
    return "BinaryObj";
}

OOXMLValue * OOXMLBinaryValue::clone() const
{
    return new OOXMLBinaryValue(mpBinaryObj);
}

/*
  class OOXMLBooleanValue
*/

OOXMLBooleanValue::OOXMLBooleanValue(bool bValue)
: mbValue(bValue)
{
}

OOXMLBooleanValue::OOXMLBooleanValue(const OUString & rValue)
: mbValue(false)
{
    mbValue = (rValue == "true"
        || rValue == "True"
        || rValue == "1"
        || rValue == "on"
        || rValue == "On");
}

OOXMLBooleanValue::~OOXMLBooleanValue()
{
}

bool OOXMLBooleanValue::getBool() const
{
    return mbValue;
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

string OOXMLBooleanValue::toString() const
{
    return mbValue ? "true" : "false";
}

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

string OOXMLStringValue::toString() const
{
    return OUStringToOString(mStr, RTL_TEXTENCODING_ASCII_US).getStr();
}

OOXMLValue * OOXMLStringValue::clone() const
{
    return new OOXMLStringValue(*this);
}

/*
  class OOXMLInputStreamValue
 */
OOXMLInputStreamValue::OOXMLInputStreamValue(uno::Reference<io::XInputStream> xInputStream)
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

string OOXMLInputStreamValue::toString() const
{
    return "InputStream";
}

OOXMLValue * OOXMLInputStreamValue::clone() const
{
    return new OOXMLInputStreamValue(mxInputStream);
}

/*
  struct OOXMLPropertySetImplCompare
 */

bool OOXMLPropertySetImplCompare::operator()(const OOXMLProperty::Pointer_t x,
                                             const OOXMLProperty::Pointer_t y) const
{
    bool bResult = false;

    if (x.get() == NULL && y.get() != NULL)
        bResult = true;
    else if (x.get() != NULL && y.get() != NULL)
        bResult = x->getId() < y->getId();

    return bResult;
}

/**
   class OOXMLPropertySetImpl
*/

OOXMLPropertySetImpl::OOXMLPropertySetImpl()
: msType("OOXMLPropertySetImpl")
{
}

OOXMLPropertySetImpl::~OOXMLPropertySetImpl()
{
}

void OOXMLPropertySetImpl::resolve(Properties & rHandler)
{
    size_t nIt = 0;

    // The pProp->resolve(rHandler) call below can cause elements to
    // be appended to mProperties. I don't think it can cause elements
    // to be deleted. But let's check with < here just to be safe that
    // the indexing below works.
    while (nIt < mProperties.size())
    {
        OOXMLProperty::Pointer_t pProp = mProperties[nIt];

        if (pProp.get() != NULL)
            pProp->resolve(rHandler);
#ifdef DEBUG_RESOLVE
        else
        {
            debug_logger->startElement("error");
            debug_logger->chars(std::string("zero-property"));
            debug_logger->endElement();
        }
#endif

        ++nIt;
    }
}

OOXMLPropertySetImpl::OOXMLProperties_t::iterator OOXMLPropertySetImpl::begin()
{
    return mProperties.begin();
}

OOXMLPropertySetImpl::OOXMLProperties_t::iterator OOXMLPropertySetImpl::end()
{
    return mProperties.end();
}

OOXMLPropertySetImpl::OOXMLProperties_t::const_iterator
OOXMLPropertySetImpl::begin() const
{
    return mProperties.begin();
}

OOXMLPropertySetImpl::OOXMLProperties_t::const_iterator
OOXMLPropertySetImpl::end() const
{
    return mProperties.end();
}

string OOXMLPropertySetImpl::getType() const
{
    return msType;
}

void OOXMLPropertySetImpl::add(OOXMLProperty::Pointer_t pProperty)
{
#ifdef DEBUG_PROPERTY_SET
    debug_logger->startElement("propertyset.add");
    debug_logger->chars(pProperty->toString());
#endif

    if (pProperty.get() != NULL && pProperty->getId() != 0x0)
    {
        /*
         HACK: Ugly hack. This retarded overdesigned writerfilter thing
         processes attributes in random order (as given by boost::unordered_map
         when iterating it), but StyleSheetTable::lcl_attribute() needs
         to know whether NS_ooxml::LN_CT_Style_type is STYLE_TYPE_TABLE first.
         And all this overdesigned machinery doesn't even give a reasonable
         way to find out if an attribute is there before encountering it
         in random order in lcl_attribute(), so just make sure here that
         the attribute comes first.
        */
        if( pProperty->getId() == NS_ooxml::LN_CT_Style_type )
            mProperties.insert( mProperties.begin(), pProperty );
        else
            mProperties.push_back(pProperty);
    }
#ifdef DEBUG_PROPERTY_SET
    else
    {
        debug_logger->element("warning.property_not_added");
    }

    debug_logger->endElement("propertyset.add");
#endif
}

void OOXMLPropertySetImpl::add(OOXMLPropertySet::Pointer_t pPropertySet)
{
    if (pPropertySet.get() != NULL)
    {
        OOXMLPropertySetImpl * pSet =
            dynamic_cast<OOXMLPropertySetImpl *>(pPropertySet.get());

        if (pSet != NULL)
        {
            mProperties.resize(mProperties.size() + pSet->mProperties.size());
            for (OOXMLProperties_t::iterator aIt = pSet->mProperties.begin();
                 aIt != pSet->mProperties.end(); ++aIt)
                add(*aIt);
        }
    }
}

OOXMLPropertySet * OOXMLPropertySetImpl::clone() const
{
    return new OOXMLPropertySetImpl(*this);
}

void OOXMLPropertySetImpl::setType(const string & rsType)
{
    msType = rsType;
}

string OOXMLPropertySetImpl::toString()
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

        if ((*aIt).get() != NULL)
            sResult += (*aIt)->toString();
        else
            sResult += "0x0";
    }

    sResult += "]";

    return sResult;
}

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

string OOXMLPropertySetValue::toString() const
{
    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "t:%p, m:%p", this, mpPropertySet.get());

    return "OOXMLPropertySetValue(" + string(sBuffer) + ")";
}

OOXMLValue * OOXMLPropertySetValue::clone() const
{
    return new OOXMLPropertySetValue(*this);
}

/*
  class OOXMLIntegerValue
*/

OOXMLIntegerValue::OOXMLIntegerValue(sal_Int32 nValue)
: mnValue(nValue)
{
}

OOXMLIntegerValue::OOXMLIntegerValue(const OUString & rValue)
: mnValue(0)
{
    mnValue = rValue.toInt32();
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

string OOXMLIntegerValue::toString() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%" SAL_PRIdINT32, mnValue);

    return buffer;
}

/*
  class OOXMLHexValue
*/

OOXMLHexValue::OOXMLHexValue(sal_uInt32 nValue)
: mnValue(nValue)
{
}

OOXMLHexValue::OOXMLHexValue(const OUString & rValue)
{
    mnValue = rValue.toUInt32(16);
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

string OOXMLHexValue::toString() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "0x%" SAL_PRIxUINT32, mnValue);

    return buffer;
}

// OOXMLUniversalMeasureValue

OOXMLUniversalMeasureValue::OOXMLUniversalMeasureValue(sal_uInt32 nValue)
    : mnValue(nValue)
{
}

OOXMLUniversalMeasureValue::OOXMLUniversalMeasureValue(const OUString& rValue)
{
    mnValue = rValue.toInt32();
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

string OOXMLUniversalMeasureValue::toString() const
{
    return OString::number(mnValue).getStr();
}

/*
  class OOXMLShapeValue
 */


OOXMLShapeValue::OOXMLShapeValue(uno::Reference<XShape> rShape)
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

string OOXMLShapeValue::toString() const
{
    return "Shape";
}

OOXMLValue * OOXMLShapeValue::clone() const
{
    return new OOXMLShapeValue(mrShape);
}

/*
  class OOXMLStarMathValue
 */


OOXMLStarMathValue::OOXMLStarMathValue( uno::Reference< embed::XEmbeddedObject > c )
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

string OOXMLStarMathValue::toString() const
{
    return "StarMath";
}

OOXMLValue * OOXMLStarMathValue::clone() const
{
    return new OOXMLStarMathValue( component );
}

/*
  class OOXMLTableImpl
 */

OOXMLTableImpl::OOXMLTableImpl()
{
}

OOXMLTableImpl::~OOXMLTableImpl()
{
}

void OOXMLTableImpl::resolve(Table & rTable)
{
#ifdef DEBUG_PROTOCOL
    Table::Pointer_t pTable(new TableProtocol(&rTable, debug_logger));
#else
    Table * pTable = &rTable;
#endif

    int nPos = 0;

    PropertySets_t::iterator it = mPropertySets.begin();
    PropertySets_t::iterator itEnd = mPropertySets.end();

    while (it != itEnd)
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties
            ((*it)->getProperties());

        if (pProperties.get() != NULL)
            pTable->entry(nPos, pProperties);

        ++nPos;
        ++it;
    }
}

void OOXMLTableImpl::add(ValuePointer_t pPropertySet)
{
    if (pPropertySet.get() != NULL)
        mPropertySets.push_back(pPropertySet);
}

string OOXMLTableImpl::getType() const
{
    return "OOXMLTableImpl";
}

OOXMLTable * OOXMLTableImpl::clone() const
{
    return new OOXMLTableImpl(*this);
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

const OUString & OOXMLPropertySetEntryToString::getString() const
{
    return mStr;
}

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

int OOXMLPropertySetEntryToInteger::getValue() const
{
    return mnValue;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
