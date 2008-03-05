/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLPropertySetImpl.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:05:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "OOXMLPropertySetImpl.hxx"
#include <iostream>
#include <resourcemodel/QNameToString.hxx>

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

//#define DEBUG_RESOLVE

namespace writerfilter {
namespace ooxml
{
using namespace ::std;

static ::rtl::OUString strue(RTL_CONSTASCII_USTRINGPARAM("true"));
static ::rtl::OUString sTrue(RTL_CONSTASCII_USTRINGPARAM("True"));
static ::rtl::OUString s1(RTL_CONSTASCII_USTRINGPARAM("1"));
static ::rtl::OUString sOn(RTL_CONSTASCII_USTRINGPARAM("On"));
static ::rtl::OUString son(RTL_CONSTASCII_USTRINGPARAM("on"));

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

#ifdef DEBUG_RESOLVE
    logger("DEBUG", "<resolve>");
    logger("DEBUG", toString());
    logger("DEBUG", "</resolve>");
#endif
}

/*
   class OOXMLValue
*/

OOXMLValue::OOXMLValue()
{
}

OOXMLValue::OOXMLValue(const rtl::OUString & /*rValue*/)
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

::rtl::OUString OOXMLValue::getString() const
{
    return ::rtl::OUString();
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

OOXMLBooleanValue::OOXMLBooleanValue(const rtl::OUString & rValue)
: mbValue(false)
{
    if (strue.compareTo(rValue) == 0
        || sTrue.compareTo(rValue) == 0
        || s1.compareTo(rValue) == 0
        || son.compareTo(rValue) == 0
        || sOn.compareTo(rValue) == 0)
        mbValue = true;
     else
        mbValue = false;
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

OOXMLStringValue::OOXMLStringValue(const rtl::OUString & rStr)
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

rtl::OUString OOXMLStringValue::getString() const
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
    OOXMLProperties_t::iterator aIt = begin();
    while (aIt != end())
    {
        OOXMLProperty::Pointer_t pProp = *aIt;

        if (pProp.get() != NULL)
            pProp->resolve(rHandler);
#ifdef DEBUG_RESOLVE
        else
            logger("DEBUG", "<error>zero-property</error>");
#endif

        aIt++;
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

string OOXMLPropertySetImpl::getType() const
{
    return msType;
}

void OOXMLPropertySetImpl::add(OOXMLProperty::Pointer_t pProperty)
{
#ifdef DEBUG_RESOLVE
    if (pProperty->getId() == 0x0)
        logger("DEBUG", "<error>zero property</error>");
#endif

    if (pProperty.get() != NULL && pProperty->getId() != 0x0)
        mProperties.push_back(pProperty);
}

void OOXMLPropertySetImpl::add(OOXMLPropertySet::Pointer_t pPropertySet)
{
    if (pPropertySet.get() != NULL)
    {
        OOXMLPropertySetImpl * pSet =
            dynamic_cast<OOXMLPropertySetImpl *>(pPropertySet.get());

        if (pSet != NULL)
        {
#ifdef DEBUG_RESOLVE
            logger("DEBUG", "<call class=\"OOXMLPropertySetImpl\" method=\"add\"><me>" + toString()
                 + "</me>" + pSet->toString() + "</call>");
#endif

            mProperties.resize(mProperties.size() + pSet->mProperties.size());
            for (OOXMLProperties_t::iterator aIt = pSet->mProperties.begin();
                 aIt != pSet->mProperties.end(); aIt++)
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

    for (OOXMLProperties_t::iterator aIt = aItBegin; aIt != aItEnd; aIt++)
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

    snprintf(sBuffer, sizeof(sBuffer), "%p", this);

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

OOXMLIntegerValue::OOXMLIntegerValue(const rtl::OUString & rValue)
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

OOXMLHexValue::OOXMLHexValue(const rtl::OUString & rValue)
{
    mnValue = static_cast<sal_uInt32>(rValue.toInt32(16));
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

/*
  class OOXMLListValue
*/
OOXMLListValue::OOXMLListValue()
: OOXMLIntegerValue(0)
{
}

OOXMLListValue::OOXMLListValue(sal_Int32 nValue)
: OOXMLIntegerValue(nValue)
{
}


OOXMLListValue::~OOXMLListValue()
{
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
    int nPos = 0;

    PropertySets_t::iterator it = mPropertySets.begin();
    PropertySets_t::iterator itEnd = mPropertySets.end();

    while (it != itEnd)
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties
            ((*it)->getProperties());

        if (pProperties.get() != NULL)
            rTable.entry(nPos, pProperties);

        ++nPos;
        it++;
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
#ifdef DEBUG_RESOLVE
    logger("DEBUG", "OOXMLPropertySetEntryToString::attribute("
           + (*QNameToString::Instance())(nId) + ")");
#endif

    if (nId == mnId)
        mStr = rValue.getString();
}

const ::rtl::OUString & OOXMLPropertySetEntryToString::getString() const
{
    return mStr;
}

OOXMLPropertySetEntryToInteger::OOXMLPropertySetEntryToInteger(Id nId)
: mnId(nId)
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
