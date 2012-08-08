/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FILTER_CONFIG_CACHEITEM_HXX_
#define __FILTER_CONFIG_CACHEITEM_HXX_

#include <boost/unordered_map.hpp>
#include <deque>
#include <list>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/mutex.hxx>


namespace filter{
    namespace config{

namespace css = ::com::sun::star;


/** @short  Must be used as first derived base class
            to get a full initialized mutex member,
            which can be used during the ctor runs too!
 */
struct BaseLock
{
    public:

        // must be mutable to be useable in const environments too!
        mutable ::osl::Mutex m_aLock;
};


typedef ::comphelper::SequenceAsVector< ::rtl::OUString > OUStringList;

//_______________________________________________

/** @short  represent an item of a FilterCache
            instance.

    @descr  This class is not threadsafe tp perform
            operations, which use many instances of
            this class! Synchronizations must be done outside.
 */
class CacheItem : public ::comphelper::SequenceAsHashMap
{
    //-------------------------------------------
    public:

        //---------------------------------------
        /** @short  creates an empty item.
          */
        CacheItem();

        //---------------------------------------

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

        //---------------------------------------

        /** @short  check, if the given properties exists
                    at this item.

            @descr  All properties are compared in its minimum.
                    E.g: string lists => only the requested items
                    are checked. Additional existing items are ignored.

            @param  lProps
                    contains all properties, which must exist at this item.

            @return sal_True if all given properties exists
                    at this item; sal_False otherwhise.
         */
        sal_Bool haveProps(const CacheItem& lProps) const;

        //---------------------------------------

        /** @short  check, if the given properties dont exists
                    at this item.

            @descr  All properties are compared in its minimum.
                    E.g: string lists => only the requested items
                    are checked. Additional existing items are ignored.

            @param  lProps
                    contains all properties, which should not exists at this item.

            @return sal_False if at least on property exists at this item(!);
                    sal_True otherwhise.
         */
        sal_Bool dontHaveProps(const CacheItem& lProps) const;

        //---------------------------------------

        /** @short  check, if the given properties dont exists
                    at this item.

            @descr  All properties are compared in its minimum.
                    E.g: string lists => only the specified items
                    are checked. Additional existing items are ignored.

            @param  lProps
                    contains all properties, which should be checked.

            @return sal_True if all given properties dont exists
                    at this item; sal_False otherwhise.
         */
        sal_Bool excludeProps(const CacheItem& lProps) const;

        //---------------------------------------

        /** @short  because we know two UIName properties
                    (a list with all locales and the value
                    for the current locale only), we must be shure
                    that the correspond together.

            @param  sActLocale
                    must specify the current office locale.
                    Its needed to address the UIName property inside
                    the list of possible ones.
         */
        void validateUINames(const ::rtl::OUString& sActLocale);

        //---------------------------------------

        /** @short  convert this structure to a seq< PropertyValue >
                    and ignore all empty properties!

            @descr  Normaly the converter routines of the base class
                    SequenceAsHashMap do this job already.
                    But it doesnt provide a "pack" mechanism to
                    ignore properties with empty (means "void") values.

            @return css::uno::Sequence< css::beans::PropertyValue >
                    as a list of all properties of this cacheitem,
                    where empty properties was removed.
         */
        css::uno::Sequence< css::beans::PropertyValue > getAsPackedPropertyValueList();
};

//_______________________________________________

/** @short  represent an item list of a FilterCache
            instance.
 */
typedef ::boost::unordered_map< ::rtl::OUString                    ,
                         CacheItem                          ,
                         ::rtl::OUStringHash                ,
                         ::std::equal_to< ::rtl::OUString > > CacheItemList;

//_______________________________________________

/** @short  supports registration of multiple key to
            another string information.

    @descr  E.g. a list of internal type names can be registered
            to an extension. Organization as an hash makes it
            faster than searching inside vectors.

            On the other side e.g. URLPattern cant be really addressed
            by a hash value ... because the use wildcards. But
            there we need key-value pairs too, which cant be provided
            by a pure vector!
 */
typedef ::boost::unordered_map< ::rtl::OUString                    ,
                         OUStringList                       ,
                         ::rtl::OUStringHash                ,
                         ::std::equal_to< ::rtl::OUString > > CacheItemRegistration;

//_______________________________________________

/** @short  is used to collect all matching types of an URL
            during type detection.

    @descr  Every type in this list is combined with an information,
            which property matched to the given URL. The user of this
            structure can decide then, if a deep detection should be
            suppressed e.g. if an URLPattern was used.
 */
struct  FlatDetectionInfo
{
    // the internal type name
    ::rtl::OUString sType;

    // this type was found by a matching the URL extension
    sal_Bool bMatchByExtension;

    // this type was found by a matching URL Pattern
    sal_Bool bMatchByPattern;

    // the user selected this type implicit by selecting a corresponding office module
    sal_Bool bPreselectedByDocumentService;

    FlatDetectionInfo()
        : sType                        (::rtl::OUString())
        , bMatchByExtension            (sal_False        )
        , bMatchByPattern              (sal_False        )
        , bPreselectedByDocumentService(sal_False        )
    {}
};

typedef ::std::list< FlatDetectionInfo > FlatDetection;

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_CACHEITEM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
