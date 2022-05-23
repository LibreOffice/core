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


#include "cacheitem.hxx"
#include "constant.hxx"

#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>


namespace filter::config{

CacheItem::CacheItem()
{
}


void CacheItem::update(const CacheItem& rUpdateItem)
{
    for (auto const& elem : rUpdateItem)
       (*this)[elem.first] = elem.second;
}


void CacheItem::validateUINames(const OUString& sActLocale)
{
    if (sActLocale.isEmpty())
        return;

    // 1) check UINames first
    const_iterator pUINames = find(PROPNAME_UINAMES);
    const_iterator pUIName  = find(PROPNAME_UINAME );

    ::comphelper::SequenceAsHashMap lUINames;
    if (pUINames != end())
        lUINames << pUINames->second;

    OUString sUIName;
    if (pUIName != end())
        pUIName->second >>= sUIName;

    if (!sUIName.isEmpty())
    {
        // 1a) set UIName inside list of UINames for current locale
        lUINames[sActLocale] <<= sUIName;
    }
    else if (!lUINames.empty())
    {
        // 1b) or get it from this list, if it not exist!
        lUINames[sActLocale] >>= sUIName;
    }

    (*this)[PROPNAME_UINAMES] <<= lUINames.getAsConstPropertyValueList();
    (*this)[PROPNAME_UINAME ] <<= sUIName;
}


css::uno::Sequence< css::beans::PropertyValue > CacheItem::getAsPackedPropertyValueList(bool bFinalized, bool bMandatory) const
{
    sal_Int32 c = static_cast<sal_Int32>(size());
    sal_Int32 i = 0;

    css::uno::Sequence< css::beans::PropertyValue > lList(c+2);
    css::beans::PropertyValue*                      pList = lList.getArray();

    for (const_iterator pProp  = begin();
                        pProp != end()  ;
                      ++pProp           )
    {
        const OUString& rName  = pProp->first.maString;
        const css::uno::Any&   rValue = pProp->second;

        if (!rValue.hasValue())
            continue;
        assert (rName != PROPNAME_FINALIZED && rName != PROPNAME_MANDATORY);

        pList[i].Name  = rName ;
        pList[i].Value = rValue;
        ++i;
    }
    pList[i].Name  = PROPNAME_FINALIZED ;
    pList[i].Value <<= bFinalized;
    ++i;
    pList[i].Name  = PROPNAME_MANDATORY ;
    pList[i].Value <<= bMandatory;
    ++i;
    lList.realloc(i);

    return lList;
}


static bool isSubSet(const css::uno::Any& aSubSet,
                  const css::uno::Any& aSet   )
{
    const css::uno::Type& aT1 = aSubSet.getValueType();
    const css::uno::Type& aT2 = aSet.getValueType();

    if (!aT1.equals(aT2))
    {
        return false;
    }

    if (aSubSet.hasValue() && aSet.hasValue())
    {
        css::uno::TypeClass aTypeClass = aT1.getTypeClass();
        switch(aTypeClass)
        {

            case css::uno::TypeClass_BOOLEAN :
            case css::uno::TypeClass_BYTE :
            case css::uno::TypeClass_SHORT :
            case css::uno::TypeClass_UNSIGNED_SHORT :
            case css::uno::TypeClass_LONG :
            case css::uno::TypeClass_UNSIGNED_LONG :
            case css::uno::TypeClass_HYPER :
            case css::uno::TypeClass_UNSIGNED_HYPER :
            case css::uno::TypeClass_FLOAT :
            case css::uno::TypeClass_DOUBLE :
            {
                bool bIs = (aSubSet == aSet);
                return bIs;
            }


            case css::uno::TypeClass_STRING :
                return aSubSet == aSet;
            break;


            case css::uno::TypeClass_STRUCT :
            {
                css::beans::PropertyValue p1;
                css::beans::PropertyValue p2;

                if (
                    (aSubSet >>= p1) &&
                    (aSet    >>= p2)
                   )
                {
                    bool bIs = (p1.Name == p2.Name) && isSubSet(p1.Value, p2.Value);
                    return bIs;
                }

                css::beans::NamedValue n1;
                css::beans::NamedValue n2;

                if (
                    (aSubSet >>= n1) &&
                    (aSet    >>= n2)
                   )
                {
                    bool bIs = (n1.Name == n2.Name) && isSubSet(n1.Value, n2.Value);
                    return bIs;
                }
            }
            break;


            case css::uno::TypeClass_SEQUENCE :
            {
                css::uno::Sequence< OUString > uno_s1;
                css::uno::Sequence< OUString > uno_s2;

                if (
                    (aSubSet >>= uno_s1) &&
                    (aSet    >>= uno_s2)
                   )
                {
                    auto s2Begin = uno_s2.getConstArray();
                    auto s2End = uno_s2.getConstArray() + uno_s2.getLength();

                    for (auto const& elem : uno_s1)
                    {
                        if (::std::find(s2Begin, s2End, elem) == s2End)
                        {
                            return false;
                        }
                    }
                    return true;
                }

                css::uno::Sequence< css::beans::PropertyValue > uno_p1;
                css::uno::Sequence< css::beans::PropertyValue > uno_p2;

                if (
                    (aSubSet >>= uno_p1) &&
                    (aSet    >>= uno_p2)
                   )
                {
                    ::comphelper::SequenceAsHashMap stl_p1(uno_p1);
                    ::comphelper::SequenceAsHashMap stl_p2(uno_p2);

                    for (auto const& elem : stl_p1)
                    {
                        ::comphelper::SequenceAsHashMap::const_iterator it2 = stl_p2.find(elem.first);
                        if (it2 == stl_p2.end())
                        {
                            return false;
                        }
                        if (!isSubSet(elem.second, it2->second))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                css::uno::Sequence< css::beans::NamedValue > uno_n1;
                css::uno::Sequence< css::beans::NamedValue > uno_n2;

                if (
                    (aSubSet >>= uno_n1) &&
                    (aSet    >>= uno_n2)
                   )
                {
                    ::comphelper::SequenceAsHashMap stl_n1(uno_n1);
                    ::comphelper::SequenceAsHashMap stl_n2(uno_n2);

                    for (auto const& elem : stl_n1)
                    {
                        ::comphelper::SequenceAsHashMap::const_iterator it2 = stl_n2.find(elem.first);
                        if (it2 == stl_n2.end())
                        {
                            return false;
                        }
                        if (!isSubSet(elem.second, it2->second))
                        {
                            return false;
                        }
                    }
                    return true;
                }
            }
            break;
            default: break;
        }
    }
    OSL_FAIL("isSubSet() ... this point should not be reached!");
    return false;
}


bool CacheItem::haveProps(o3tl::span< const css::beans::NamedValue > lProps) const
{
    for (auto const& prop : lProps)
    {
        // i) one required property does not exist at this item => return false
        const_iterator pItThis = find(prop.Name);
        if (pItThis == end())
        {
            return false;
        }

        // ii) one item does not have the right value => return false
        if (!isSubSet(prop.Value, pItThis->second))
        {
            return false;
        }
    }

    // this method was not broken before =>
    // the given property set seems to match with our
    // own properties in its minimum => return TRUE
    return true;
}


bool CacheItem::dontHaveProps(o3tl::span< const css::beans::NamedValue > lProps) const
{
    for (auto const& prop : lProps)
    {
        // i) one item does not exist in general
        //    => continue with next one, because
        //    "excluding" means... "don't have it".
        //    And "not exists" matches to "don't have it".
        const_iterator pItThis = find(prop.Name);
        if (pItThis == end())
        {
            continue;
        }

        // ii) one item have the right value => return false
        //     because this item has the requested property...
        //     But we checked for "don't have it" here.
        if (isSubSet(prop.Value, pItThis->second))
        {
            return false;
        }
    }

    // this method was not broken before =>
    // That means: this item has no matching property
    // of the given set. It "don't have" it ... => return true.
    return true;
}

FlatDetectionInfo::FlatDetectionInfo() :
    bMatchByExtension(false), bMatchByPattern(false), bPreselectedByDocumentService(false) {}

} // namespace filter::config

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
