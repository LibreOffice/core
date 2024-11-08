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

#include <sal/config.h>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/reflection/XIdlField.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

namespace
{
uno::Any jsonToUnoAny(const boost::property_tree::ptree& aTree)
{
    uno::Any aAny;
    uno::Any aValue;
    sal_Int32 nFields;
    uno::Reference<reflection::XIdlField> aField;
    boost::property_tree::ptree aNodeNull, aNodeValue, aNodeField;
    const std::string aType = aTree.get<std::string>("type", "");
    const std::string aValueStr = aTree.get<std::string>("value", "");
    uno::Sequence<uno::Reference<reflection::XIdlField>> aFields;
    uno::Reference<reflection::XIdlClass> xIdlClass
        = css::reflection::theCoreReflection::get(comphelper::getProcessComponentContext())
              ->forName(OUString::fromUtf8(aType));
    if (xIdlClass.is())
    {
        uno::TypeClass aTypeClass = xIdlClass->getTypeClass();
        xIdlClass->createObject(aAny);
        aFields = xIdlClass->getFields();
        nFields = aFields.getLength();
        aNodeValue = aTree.get_child("value", aNodeNull);
        if (nFields > 0 && aNodeValue != aNodeNull)
        {
            for (sal_Int32 itField = 0; itField < nFields; ++itField)
            {
                aField = aFields[itField];
                aNodeField = aNodeValue.get_child(aField->getName().toUtf8().getStr(), aNodeNull);
                if (aNodeField != aNodeNull)
                {
                    aValue = jsonToUnoAny(aNodeField);
                    aField->set(aAny, aValue);
                }
            }
        }
        else if (!aValueStr.empty())
        {
            if (aTypeClass == uno::TypeClass_VOID)
                aAny.clear();
            else if (aTypeClass == uno::TypeClass_BYTE)
                aAny <<= static_cast<sal_Int8>(o3tl::toInt32(aValueStr));
            else if (aTypeClass == uno::TypeClass_BOOLEAN)
                aAny <<= rtl_str_toBoolean(aValueStr.c_str());
            else if (aTypeClass == uno::TypeClass_SHORT)
                aAny <<= static_cast<sal_Int16>(o3tl::toInt32(aValueStr));
            else if (aTypeClass == uno::TypeClass_UNSIGNED_SHORT)
                aAny <<= static_cast<sal_uInt16>(o3tl::toUInt32(aValueStr));
            else if (aTypeClass == uno::TypeClass_LONG)
                aAny <<= o3tl::toInt32(aValueStr);
            else if (aTypeClass == uno::TypeClass_UNSIGNED_LONG)
                aAny <<= static_cast<sal_uInt32>(o3tl::toInt32(aValueStr));
            else if (aTypeClass == uno::TypeClass_FLOAT)
                aAny <<= rtl_str_toFloat(aValueStr.c_str());
            else if (aTypeClass == uno::TypeClass_DOUBLE)
                aAny <<= o3tl::toDouble(aValueStr);
            else if (aTypeClass == uno::TypeClass_STRING)
                aAny <<= OUString::fromUtf8(aValueStr);
        }
    }
    return aAny;
}
}

namespace comphelper{

SequenceAsHashMap::SequenceAsHashMap()
{
}

SequenceAsHashMap::SequenceAsHashMap(const css::uno::Any& aSource)
{
    (*this) << aSource;
}


SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::uno::Any >& lSource)
{
    (*this) << lSource;
}

SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
{
    (*this) << lSource;
}

SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::beans::NamedValue >& lSource)
{
    (*this) << lSource;
}

void SequenceAsHashMap::operator<<(const css::uno::Any& aSource)
{
    // An empty Any reset this instance!
    if (!aSource.hasValue())
    {
        clear();
        return;
    }

    css::uno::Sequence< css::beans::NamedValue > lN;
    if (aSource >>= lN)
    {
        (*this) << lN;
        return;
    }

    css::uno::Sequence< css::beans::PropertyValue > lP;
    if (aSource >>= lP)
    {
        (*this) << lP;
        return;
    }

    throw css::lang::IllegalArgumentException(
        u"Any contains wrong type."_ustr, css::uno::Reference<css::uno::XInterface>(),
        -1);
}


void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::uno::Any >& lSource)
{
    sal_Int32 c = lSource.getLength();
    sal_Int32 i = 0;

    m_aMap.reserve(c);
    for (i=0; i<c; ++i)
    {
        css::beans::PropertyValue lP;
        if (lSource[i] >>= lP)
        {
            if (
                (lP.Name.isEmpty()) ||
                (!lP.Value.hasValue())
               )
                throw css::lang::IllegalArgumentException(
                    u"PropertyValue struct contains no useful information."_ustr,
                    css::uno::Reference<css::uno::XInterface>(), -1);
            (*this)[lP.Name] = lP.Value;
            continue;
        }

        css::beans::NamedValue lN;
        if (lSource[i] >>= lN)
        {
            if (
                (lN.Name.isEmpty()) ||
                (!lN.Value.hasValue())
               )
                throw css::lang::IllegalArgumentException(
                    u"NamedValue struct contains no useful information."_ustr,
                    css::uno::Reference<css::uno::XInterface>(), -1);
            (*this)[lN.Name] = lN.Value;
            continue;
        }

        // ignore VOID Any ... but reject wrong filled ones!
        if (lSource[i].hasValue())
            throw css::lang::IllegalArgumentException(
                u"Any contains wrong type."_ustr,
                css::uno::Reference<css::uno::XInterface>(), -1);
    }
}

void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
{
    clear();

    m_aMap.reserve(lSource.getLength());
    for (auto& rSource : lSource)
        (*this)[rSource.Name] = rSource.Value;
}

void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::beans::NamedValue >& lSource)
{
    clear();

    m_aMap.reserve(lSource.getLength());
    for (auto& rSource : lSource)
        (*this)[rSource.Name] = rSource.Value;
}

void SequenceAsHashMap::operator>>(css::uno::Sequence< css::beans::PropertyValue >& lDestination) const
{
    sal_Int32 c = static_cast<sal_Int32>(size());
    lDestination.realloc(c);
    css::beans::PropertyValue* pDestination = lDestination.getArray();

    sal_Int32 i = 0;
    for (const_iterator pThis  = begin();
                        pThis != end()  ;
                      ++pThis           )
    {
        pDestination[i].Name  = pThis->first.maString;
        pDestination[i].Value = pThis->second;
        ++i;
    }
}

void SequenceAsHashMap::operator>>(css::uno::Sequence< css::beans::NamedValue >& lDestination) const
{
    sal_Int32 c = static_cast<sal_Int32>(size());
    lDestination.realloc(c);
    css::beans::NamedValue* pDestination = lDestination.getArray();

    sal_Int32 i = 0;
    for (const_iterator pThis  = begin();
                        pThis != end()  ;
                      ++pThis           )
    {
        pDestination[i].Name  = pThis->first.maString;
        pDestination[i].Value = pThis->second;
        ++i;
    }
}

css::uno::Any SequenceAsHashMap::getAsConstAny(bool bAsPropertyValueList) const
{
    css::uno::Any aDestination;
    if (bAsPropertyValueList)
        aDestination <<= getAsConstPropertyValueList();
    else
        aDestination <<= getAsConstNamedValueList();
    return aDestination;
}

css::uno::Sequence< css::beans::NamedValue > SequenceAsHashMap::getAsConstNamedValueList() const
{
    css::uno::Sequence< css::beans::NamedValue > lReturn;
    (*this) >> lReturn;
    return lReturn;
}

css::uno::Sequence< css::beans::PropertyValue > SequenceAsHashMap::getAsConstPropertyValueList() const
{
    css::uno::Sequence< css::beans::PropertyValue > lReturn;
    (*this) >> lReturn;
    return lReturn;
}

bool SequenceAsHashMap::match(const SequenceAsHashMap& rCheck) const
{
    for (auto const& elem : rCheck)
    {
        const OUString& sCheckName  = elem.first.maString;
        const css::uno::Any&   aCheckValue = elem.second;
        const_iterator         pFound      = find(sCheckName);

        if (pFound == end())
            return false;

        const css::uno::Any& aFoundValue = pFound->second;
        if (aFoundValue != aCheckValue)
            return false;
    }

    return true;
}

void SequenceAsHashMap::update(const SequenceAsHashMap& rUpdate)
{
    m_aMap.reserve(std::max(size(), rUpdate.size()));
    for (auto const& elem : rUpdate.m_aMap)
    {
        m_aMap[elem.first] = elem.second;
    }
}

static std::vector<css::beans::PropertyValue> JsonToPropertyValues(const boost::property_tree::ptree& aTree)
{
    std::vector<beans::PropertyValue> aArguments;
    boost::property_tree::ptree aNodeNull, aNodeValue;
    for (const auto& rPair : aTree)
    {
        const std::string aType = rPair.second.get<std::string>("type", "");
        const std::string aValueStr = rPair.second.get<std::string>("value", "");

        beans::PropertyValue aValue;
        aValue.Name = OUString::fromUtf8(rPair.first);
        if (aType == "string")
            aValue.Value <<= OUString::fromUtf8(aValueStr);
        else if (aType == "boolean")
            aValue.Value <<= rtl_str_toBoolean(aValueStr.c_str());
        else if (aType == "float")
            aValue.Value <<= rtl_str_toFloat(aValueStr.c_str());
        else if (aType == "long")
            aValue.Value <<= o3tl::toInt32(aValueStr);
        else if (aType == "short")
            aValue.Value <<= sal_Int16(o3tl::toInt32(aValueStr));
        else if (aType == "unsigned short")
            aValue.Value <<= sal_uInt16(o3tl::toUInt32(aValueStr));
        else if (aType == "int64")
            aValue.Value <<= o3tl::toInt64(aValueStr);
        else if (aType == "int32")
            aValue.Value <<= o3tl::toInt32(aValueStr);
        else if (aType == "int16")
            aValue.Value <<= sal_Int16(o3tl::toInt32(aValueStr));
        else if (aType == "uint64")
            aValue.Value <<= rtl_str_toUInt64(aValueStr.c_str(), 10);
        else if (aType == "uint32")
            aValue.Value <<= o3tl::toUInt32(aValueStr);
        else if (aType == "uint16")
            aValue.Value <<= sal_uInt16(o3tl::toUInt32(aValueStr));
        else if (aType == "[]byte")
        {
            aNodeValue = rPair.second.get_child("value", aNodeNull);
            if (aNodeValue != aNodeNull && aNodeValue.size() == 0)
            {
                uno::Sequence<sal_Int8> aSeqByte(reinterpret_cast<const sal_Int8*>(aValueStr.c_str()),
                                                 aValueStr.size());
                aValue.Value <<= aSeqByte;
            }
        }
        else if (aType == "any")
        {
            aNodeValue = rPair.second.get_child("value", aNodeNull);
            if (aNodeValue != aNodeNull && !aNodeValue.empty())
            {
                aValue.Value = jsonToUnoAny(aNodeValue);
            }
        }
        else if (aType == "[]any")
        {
            aNodeValue = rPair.second.get_child("value", aNodeNull);
            if (aNodeValue != aNodeNull && !aNodeValue.empty())
            {
                uno::Sequence<uno::Any> aSeq(aNodeValue.size());
                std::transform(aNodeValue.begin(), aNodeValue.end(), aSeq.getArray(),
                               [](const auto& rSeqPair) { return jsonToUnoAny(rSeqPair.second); });
                aValue.Value <<= aSeq;
            }
        }
        else if (aType == "[]com.sun.star.beans.PropertyValue")
        {
            aNodeValue = rPair.second.get_child("value", aNodeNull);
            aValue.Value <<= comphelper::containerToSequence(JsonToPropertyValues(aNodeValue));
        }
        else if (aType == "[][]com.sun.star.beans.PropertyValue")
        {
            aNodeValue = rPair.second.get_child("value", aNodeNull);
            std::vector<uno::Sequence<beans::PropertyValue>> aSeqs;
            for (const auto& rItem : aNodeValue)
            {
                aSeqs.push_back(comphelper::containerToSequence(JsonToPropertyValues(rItem.second)));
            }
            aValue.Value <<= comphelper::containerToSequence(aSeqs);
        }
        else
            SAL_WARN("comphelper", "JsonToPropertyValues: unhandled type '" << aType << "'");
        aArguments.push_back(aValue);
    }
    return aArguments;
}

std::vector<css::beans::PropertyValue> JsonToPropertyValues(std::string_view rJson)
{
    boost::property_tree::ptree aTree;
    std::stringstream aStream((std::string(rJson)));
    boost::property_tree::read_json(aStream, aTree);
    return JsonToPropertyValues(aTree);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
