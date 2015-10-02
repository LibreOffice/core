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
#include "macros.hxx"
#include "constant.hxx"

#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>


namespace filter{
    namespace config{

CacheItem::CacheItem()
    : SequenceAsHashMap()
{
}



void CacheItem::update(const CacheItem& rUpdateItem)
{
    for(const_iterator pItUpdate  = rUpdateItem.begin();
                       pItUpdate != rUpdateItem.end()  ;
                     ++pItUpdate                       )
    {
        iterator pItThis = this->find(pItUpdate->first);
        if (pItThis == this->end())
            (*this)[pItUpdate->first] = pItUpdate->second; // add new prop
        else
            pItThis->second = pItUpdate->second; // change value of existing prop
    }
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
    else if (lUINames.size()>0)
    {
        // 1b) or get it from this list, if it not exist!
        lUINames[sActLocale] >>= sUIName;
    }

    (*this)[PROPNAME_UINAMES] <<= lUINames.getAsConstPropertyValueList();
    (*this)[PROPNAME_UINAME ] <<= sUIName;
}



css::uno::Sequence< css::beans::PropertyValue > CacheItem::getAsPackedPropertyValueList()
{
    sal_Int32 c = (sal_Int32)size();
    sal_Int32 i = 0;

    css::uno::Sequence< css::beans::PropertyValue > lList(c);
    css::beans::PropertyValue*                      pList = lList.getArray();

    for (const_iterator pProp  = begin();
                        pProp != end()  ;
                      ++pProp           )
    {
        const OUString& rName  = pProp->first;
        const css::uno::Any&   rValue = pProp->second;

        if (!rValue.hasValue())
            continue;

        pList[i].Name  = rName ;
        pList[i].Value = rValue;
        ++i;
    }
    lList.realloc(i);

    return lList;
}



bool isSubSet(const css::uno::Any& aSubSet,
                  const css::uno::Any& aSet   )
{
    css::uno::Type aT1 = aSubSet.getValueType();
    css::uno::Type aT2 = aSet.getValueType();

    if (!aT1.equals(aT2))
    {
        _FILTER_CONFIG_LOG_("isSubSet() ... types of any values are different => return FALSE\n")
        return false;
    }

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
            _FILTER_CONFIG_LOG_1_("isSubSet() ... check for atomic types => return %s\n", bIs ? "TRUE" : "FALSE")
            return bIs;
        }


        case css::uno::TypeClass_STRING :
        {
            OUString v1;
            OUString v2;

            if (
                (aSubSet >>= v1) &&
                (aSet    >>= v2)
               )
            {
                bool bIs = (v1.equals(v2));
                _FILTER_CONFIG_LOG_1_("isSubSet() ... check for string types => return %s\n", bIs ? "TRUE" : "FALSE")
                return bIs;
            }
        }
        break;


        case css::uno::TypeClass_ANY :
        {
            css::uno::Any v1;
            css::uno::Any v2;

            if (
                (aSubSet >>= v1) &&
                (aSet    >>= v2)
               )
            {
                bool bIs = (isSubSet(v1, v2));
                _FILTER_CONFIG_LOG_1_("isSubSet() ... check for packed any types => return %s\n", bIs ? "TRUE" : "FALSE")
                return bIs;
            }
        }
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
                bool bIs = (
                                (p1.Name.equals(p2.Name)     ) &&
                                (isSubSet(p1.Value, p2.Value))
                               );
                _FILTER_CONFIG_LOG_1_("isSubSet() ... check for structured types [PropertyValue] => return %s\n", bIs ? "TRUE" : "FALSE")
                return bIs;
            }

            css::beans::NamedValue n1;
            css::beans::NamedValue n2;

            if (
                (aSubSet >>= n1) &&
                (aSet    >>= n2)
               )
            {
                bool bIs = (
                                (n1.Name.equals(n2.Name)     ) &&
                                (isSubSet(n1.Value, n2.Value))
                               );
                _FILTER_CONFIG_LOG_1_("isSubSet() ... check for structured types [NamedValue] => return %s\n", bIs ? "TRUE" : "FALSE")
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
                OUStringList stl_s1(comphelper::sequenceToContainer<OUStringList>(uno_s1));
                OUStringList stl_s2(comphelper::sequenceToContainer<OUStringList>(uno_s2));

                for (OUStringList::const_iterator it1  = stl_s1.begin();
                                                  it1 != stl_s1.end()  ;
                                                ++it1                  )
                {
                    if (::std::find(stl_s2.begin(), stl_s2.end(), *it1) == stl_s2.end())
                    {
                        _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [OUString] ... didn't found \"%s\" => return FALSE\n", _FILTER_CONFIG_TO_ASCII_(*it1))
                        return false;
                    }
                    _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [OUString] ... found \"%s\" => continue loop\n", _FILTER_CONFIG_TO_ASCII_(*it1))
                }
                _FILTER_CONFIG_LOG_("isSubSet() ... check for list types [OUString] => return TRUE\n")
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

                for (::comphelper::SequenceAsHashMap::const_iterator it1  = stl_p1.begin();
                                                                     it1 != stl_p1.end()  ;
                                                                   ++it1                  )
                {
                    ::comphelper::SequenceAsHashMap::const_iterator it2 = stl_p2.find(it1->first);
                    if (it2 == stl_p2.end())
                    {
                        _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [PropertyValue] ... didn't found \"%s\" => return FALSE\n", _FILTER_CONFIG_TO_ASCII_(it1->first))
                        return false;
                    }
                    if (!isSubSet(it1->second, it2->second))
                    {
                        _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [PropertyValue] ... found \"%s\" but has different value => return FALSE\n", _FILTER_CONFIG_TO_ASCII_(it1->first))
                        return false;
                    }
                    _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [PropertyValue] ... found \"%s\" with right value => continue loop\n", _FILTER_CONFIG_TO_ASCII_(it1->first))
                }
                _FILTER_CONFIG_LOG_("isSubSet() ... check for list types [PropertyValue] => return TRUE\n")
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

                for (::comphelper::SequenceAsHashMap::const_iterator it1  = stl_n1.begin();
                                                                     it1 != stl_n1.end()  ;
                                                                   ++it1                  )
                {
                    ::comphelper::SequenceAsHashMap::const_iterator it2 = stl_n2.find(it1->first);
                    if (it2 == stl_n2.end())
                    {
                        _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [NamedValue] ... didn't found \"%s\" => return FALSE\n", _FILTER_CONFIG_TO_ASCII_(it1->first))
                        return false;
                    }
                    if (!isSubSet(it1->second, it2->second))
                    {
                        _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [NamedValue] ... found \"%s\" but has different value => return FALSE\n", _FILTER_CONFIG_TO_ASCII_(it1->first))
                        return false;
                    }
                    _FILTER_CONFIG_LOG_1_("isSubSet() ... check for list types [NamedValue] ... found \"%s\" with right value => continue loop\n", _FILTER_CONFIG_TO_ASCII_(it1->first))
                }
                _FILTER_CONFIG_LOG_("isSubSet() ... check for list types [NamedValue] => return TRUE\n")
                return true;
            }
        }
        break;
        default: break;
    }

    OSL_FAIL("isSubSet() ... this point should not be reached!");
    return false;
}



bool CacheItem::haveProps(const CacheItem& lProps) const
{
    for (const_iterator pIt  = lProps.begin();
                        pIt != lProps.end()  ;
                      ++pIt                  )
    {
        // i) one required property does not exist at this item => return false
        const_iterator pItThis = this->find(pIt->first);
        if (pItThis == this->end())
        {
            _FILTER_CONFIG_LOG_1_("CacheItem::haveProps() ... didn't found \"%s\" => return FALSE\n", _FILTER_CONFIG_TO_ASCII_(pIt->first))
            return false;
        }

        // ii) one item does not have the right value => return false
        if (!isSubSet(pIt->second, pItThis->second))
        {
            _FILTER_CONFIG_LOG_1_("CacheItem::haveProps() ... item \"%s\" has different value => return FALSE\n", _FILTER_CONFIG_TO_ASCII_(pIt->first))
            return false;
        }
    }

    // this method was not breaked before =>
    // the given property set seems to match with our
    // own properties in its minimum => return TRUE
    _FILTER_CONFIG_LOG_("CacheItem::haveProps() ... => return TRUE\n")
    return true;
}



bool CacheItem::dontHaveProps(const CacheItem& lProps) const
{
    for (const_iterator pIt  = lProps.begin();
                        pIt != lProps.end()  ;
                      ++pIt                  )
    {
        // i) one item does not exist in general
        //    => continue with next one, because
        //    "excluding" means... "don't have it".
        //    And "not exists" matches to "don't have it".
        const_iterator pItThis = this->find(pIt->first);
        if (pItThis == this->end())
        {
            _FILTER_CONFIG_LOG_1_("CacheItem::dontHaveProps() ... not found \"%s\" => continue loop!\n", _FILTER_CONFIG_TO_ASCII_(pIt->first))
            continue;
        }

        // ii) one item have the right value => return false
        //     because this item has the requested property...
        //     But we checked for "don't have it" here.
        if (isSubSet(pIt->second, pItThis->second))
        {
            _FILTER_CONFIG_LOG_1_("CacheItem::dontHaveProps() ... item \"%s\" has same value => return FALSE!\n", _FILTER_CONFIG_TO_ASCII_(pIt->first))
            return false;
        }
    }

    // this method was not breaked before =>
    // That means: this item has no matching property
    // of the given set. It "don't have" it ... => return true.
    _FILTER_CONFIG_LOG_("CacheItem::dontHaveProps() ... => return TRUE\n")
    return true;
}

FlatDetectionInfo::FlatDetectionInfo() :
    bMatchByExtension(false), bMatchByPattern(false), bPreselectedByDocumentService(false) {}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
