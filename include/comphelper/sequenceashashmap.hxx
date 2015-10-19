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

#ifndef INCLUDED_COMPHELPER_SEQUENCEASHASHMAP_HXX
#define INCLUDED_COMPHELPER_SEQUENCEASHASHMAP_HXX

#include <algorithm>
#include <unordered_map>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <comphelper/comphelperdllapi.h>


namespace comphelper{


/** @short  Implements a stl hash map on top of some
            specialized sequence from type PropertyValue
            or NamedValue.

    @descr  That provides the possibility to modify
            such name sequences very easy ...
 */

struct SequenceAsHashMapBase : public std::unordered_map<
    OUString                    ,
    css::uno::Any         ,
    OUStringHash                ,
    ::std::equal_to< OUString > >
{
};

class COMPHELPER_DLLPUBLIC SequenceAsHashMap : public SequenceAsHashMapBase
{

    public:


        /** @short  creates an empty hash map.
         */
        SequenceAsHashMap();


        /** @see    operator<<(const css::uno::Any&)
         */
        SequenceAsHashMap(const css::uno::Any& aSource);


        /** @see    operator<<(const css::uno::Sequence< css::uno::Any >&)
         */
        SequenceAsHashMap(const css::uno::Sequence< css::uno::Any >& lSource);


        /** @see    operator<<(const css::uno::Sequence< css::beans::PropertyValue >&)
         */
        SequenceAsHashMap(const css::uno::Sequence< css::beans::PropertyValue >& lSource);


        /** @see    operator<<(const css::uno::Sequence< css::beans::NamedValue >&)
         */
        SequenceAsHashMap(const css::uno::Sequence< css::beans::NamedValue >& lSource);


        /** @short  not really used but maybe useful :-)
         */
        ~SequenceAsHashMap();


        /** @short  fill this map from the given
                    Any, which of course must contain
                    a suitable sequence of element types
                    "css.beans.PropertyValue" or "css.beans.NamedValue".

            @attention  If the given Any is an empty one
                        (if it's set to VOID), no exception
                        is thrown. In such case this instance will
                        be created as an empty one too!

            @param  aSource
                    contains the new items for this map.

            @throw  A css::lang::IllegalArgumentException
                    is thrown, if the given Any does not contain a suitable sequence ...
                    but not if it's a VOID Any!
         */
        void operator<<(const css::uno::Any& aSource);


        /** @short  fill this map from the given
                    sequence, where every Any must contain
                    an item from type "css.beans.PropertyValue"
                    "css.beans.NamedValue".

            @param  lSource
                    contains the new items for this map.

            @throw  A css::lang::IllegalArgumentException
                    is thrown, if the given Any sequence
                    uses wrong types for its items. VOID Any will be ignored!
         */
        void operator<<(const css::uno::Sequence< css::uno::Any >& lSource);


        /** @short  fill this map from the given
                    PropertyValue sequence.

            @param  lSource
                    contains the new items for this map.
         */
        void operator<<(const css::uno::Sequence< css::beans::PropertyValue >& lSource);


        /** @short  fill this map from the given
                    NamedValue sequence.

            @param  lSource
                    contains the new items for this map.
         */
        void operator<<(const css::uno::Sequence< css::beans::NamedValue >& lSource);


        /** @short  converts this map instance to an
                    PropertyValue sequence.

            @param  lDestination
                    target sequence for converting.
         */
        void operator>>(css::uno::Sequence< css::beans::PropertyValue >& lDestination) const;


        /** @short  converts this map instance to an
                    NamedValue sequence.

            @param  lDestination
                    target sequence for converting.
         */
        void operator>>(css::uno::Sequence< css::beans::NamedValue >& lDestination) const;


        /** @short  return this map instance as an
                    Any, which can be
                    used in const environments only.

            @descr  It's made const to prevent using of the
                    return value directly as an in/out parameter!
                    usage: myMethod(stlDequeAdapter.getAsAnyList());

            @param  bAsPropertyValue
                    switch between using of PropertyValue or NamedValue as
                    value type.

            @return A const Any, which
                    contains all items of this map.
         */
        const css::uno::Any getAsConstAny(bool bAsPropertyValue) const;


        /** @short  return this map instance to as a
                    NamedValue sequence, which can be
                    used in const environments only.

            @descr  It's made const to prevent using of the
                    return value directly as an in/out parameter!
                    usage: myMethod(stlDequeAdapter.getAsNamedValueList());

            @return A const sequence of type NamedValue, which
                    contains all items of this map.
         */
        const css::uno::Sequence< css::beans::NamedValue > getAsConstNamedValueList() const;


        /** @short  return this map instance to as a
                    PropertyValue sequence, which can be
                    used in const environments only.

            @descr  It's made const to prevent using of the
                    return value directly as an in/out parameter!
                    usage: myMethod(stlDequeAdapter.getAsPropertyValueList());

            @return A const sequence of type PropertyValue, which
                    contains all items of this map.
         */
        const css::uno::Sequence< css::beans::PropertyValue > getAsConstPropertyValueList() const;


        /** @short  check if the specified item exists
                    and return its (unpacked!) value or it returns the
                    specified default value otherwise.

            @descr  If a value should be extracted only in case
                    the requested property exists really (without creating
                    of new items as it the index operator of a
                    hash map does!) this method can be used.

            @param  sKey
                    key name of the item.

            @param  aDefault
                    the default value, which is returned
                    if the specified item could not
                    be found.

            @return The (unpacked!) value of the specified property or
                    the given default value otherwise.

            @attention  "unpacked" means the Any content of every iterator->second!
         */
        template< class TValueType >
        TValueType getUnpackedValueOrDefault(const OUString& sKey    ,
                                             const TValueType&      aDefault) const
        {
            const_iterator pIt = find(sKey);
            if (pIt == end())
                return aDefault;

            TValueType aValue = TValueType();
            if (!(pIt->second >>= aValue))
                return aDefault;

            return aValue;
        }


        /** @short  creates a new item with the specified
                    name and value only in case such item name
                    does not already exist.

            @descr  To check if the property already exists only
                    its name is used for compare. Its value isn't
                    checked!

            @param  sKey
                    key name of the property.

            @param  aValue
                    the new (unpacked!) value.
                    Note: This value will be transformed to an Any
                    internally, because only Any values can be
                    part of a PropertyValue or NamedValue structure.

            @return TRUE if this property was added as new item;
                    FALSE if it already exists.
         */
        template< class TValueType >
        bool createItemIfMissing(const OUString& sKey  ,
                                     const TValueType&      aValue)
        {
            if (find(sKey) == end())
            {
                (*this)[sKey] = css::uno::toAny(aValue);
                return true;
            }

            return false;
        }


        /** @short  check if all items of given map
                    exists in these called map also.

            @descr  Every item of the given map must exists
                    with same name and value inside these map.
                    But these map can contain additional items
                    which are not part of the search-map.

            @param  rCheck
                    the map containing all items for checking.

            @return
                    TRUE if all items of Rcheck could be found
                    in these map; FALSE otherwise.
         */
        bool match(const SequenceAsHashMap& rCheck) const;


        /** @short  merge all values from the given map into
                    this one.

            @descr  Existing items will be overwritten ...
                    missing items will be created new ...
                    but non specified items will stay alive !

            @param  rSource
                    the map containing all items for the update.
         */
        void update(const SequenceAsHashMap& rSource);
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_SEQUENCEASHASHMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
