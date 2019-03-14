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

#ifndef INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CACHEITEM_HXX
#define INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CACHEITEM_HXX

#include <unordered_map>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <vector>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/mutex.hxx>


namespace filter{
    namespace config{


/** @short  Must be used as first derived base class
            to get a full initialized mutex member,
            which can be used during the ctor runs too!
 */
struct BaseLock
{
    public:

        // must be mutable to be usable in const environments too!
        mutable ::osl::Mutex m_aLock;
};


/** @short  represent an item of a FilterCache
            instance.

    @descr  This class is not threadsafe tp perform
            operations, which use many instances of
            this class! Synchronizations must be done outside.
 */
class CacheItem : public ::comphelper::SequenceAsHashMap
{

    public:


        /** @short  creates an empty item.
          */
        CacheItem();


        /** @short  update only properties, which are given by the
                    specified rItem.

            @descr  Update means: - add new properties
                                  - change existing values

            @param  rUpdateItem
                    another cache item, which contains some special
                    properties, which should by used for updating
                    this one.
         */
        void update(const CacheItem& rUpdateItem);


        /** @short  check, if the given properties exist
                    at this item.

            @descr  All properties are compared in its minimum.
                    E.g: string lists => only the requested items
                    are checked. Additional existing items are ignored.

            @param  lProps
                    contains all properties, which must exist at this item.

            @return sal_True if all given properties exists
                    at this item; sal_False otherwise.
         */
        bool haveProps(const CacheItem& lProps) const;


        /** @short  check, if the given properties don't exist
                    at this item.

            @descr  All properties are compared in its minimum.
                    E.g: string lists => only the requested items
                    are checked. Additional existing items are ignored.

            @param  lProps
                    contains all properties, which should not exists at this item.

            @return sal_False if at least on property exists at this item(!);
                    sal_True otherwise.
         */
        bool dontHaveProps(const CacheItem& lProps) const;


        /** @short  because we know two UIName properties
                    (a list with all locales and the value
                    for the current locale only), we must be sure
                    that the correspond together.

            @param  sActLocale
                    must specify the current office locale.
                    Its needed to address the UIName property inside
                    the list of possible ones.
         */
        void validateUINames(const OUString& sActLocale);


        /** @short  convert this structure to a seq< PropertyValue >
                    and ignore all empty properties!

            @descr  Normally the converter routines of the base class
                    SequenceAsHashMap do this job already.
                    But it doesn't provide a "pack" mechanism to
                    ignore properties with empty (means "void") values.

            @return css::uno::Sequence< css::beans::PropertyValue >
                    as a list of all properties of this cacheitem,
                    where empty properties was removed.
         */
        css::uno::Sequence< css::beans::PropertyValue > getAsPackedPropertyValueList();
};


/** @short  represent an item list of a FilterCache
            instance.
 */
typedef std::unordered_map< OUString,
                            CacheItem > CacheItemList;


/** @short  supports registration of multiple key to
            another string information.

    @descr  E.g. a list of internal type names can be registered
            to an extension. Organization as an hash makes it
            faster than searching inside vectors.

            On the other side e.g. URLPattern can't be really addressed
            by a hash value ... because the use wildcards. But
            there we need key-value pairs too, which can't be provided
            by a pure vector!
 */
typedef std::unordered_map< OUString,
                            std::vector<OUString> > CacheItemRegistration;


/** @short  is used to collect all matching types of an URL
            during type detection.

    @descr  Every type in this list is combined with an information,
            which property matched to the given URL. The user of this
            structure can decide then, if a deep detection should be
            suppressed e.g. if an URLPattern was used.
 */
struct FlatDetectionInfo
{
    // the internal type name
    OUString sType;

    // this type was found by a matching the URL extension
    bool bMatchByExtension;

    // this type was found by a matching URL Pattern
    bool bMatchByPattern;

    // the user selected this type implicit by selecting a corresponding office module
    bool bPreselectedByDocumentService;

    FlatDetectionInfo();
};

typedef ::std::vector< FlatDetectionInfo > FlatDetection;

    } // namespace config
} // namespace filter

#endif // INCLUDED_FILTER_SOURCE_CONFIG_CACHE_CACHEITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
