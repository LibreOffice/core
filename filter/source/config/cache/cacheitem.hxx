/*************************************************************************
 *
 *  $RCSfile: cacheitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 15:07:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FILTER_CONFIG_CACHEITEM_HXX_
#define __FILTER_CONFIG_CACHEITEM_HXX_

//_______________________________________________
// includes

#include <hash_map>
#include <deque>

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#include <comphelper/sequenceasvector.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

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

            @return TRUE if all given properties exists
                    at this item; FALSE otherwhise.
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

            @return FALSE if at least on property exists at this item(!);
                    TRUE otherwhise.
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

            @return TRUE if all given properties dont exists
                    at this item; FALSE otherwhise.
         */
        sal_Bool excludeProps(const CacheItem& lProps) const;

        //---------------------------------------

        /** @short  because we know two UIName properties
                    (a list with all locales and the value
                    for the current locale only), we must be shure
                    that the correspond together.

            @param  sActLocale
                    must specify the current office locale.
                    Its needed to adress the UIName property inside
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
typedef ::std::hash_map< ::rtl::OUString                    ,
                         CacheItem                          ,
                         ::rtl::OUStringHash                ,
                         ::std::equal_to< ::rtl::OUString > > CacheItemList;

//_______________________________________________

/** @short  supports registration of multiple key to
            another string information.

    @descr  E.g. a list of internal type names can be registered
            to an extension. Organization as an hash makes it
            faster then searching inside vectors.

            On the other side e.g. URLPattern cant be realy adressed
            by a hash value ... because the use wildcards. But
            there we need key-value pairs too, which cant be provided
            by a pur vector!
 */
typedef ::std::hash_map< ::rtl::OUString                    ,
                         OUStringList                       ,
                         ::rtl::OUStringHash                ,
                         ::std::equal_to< ::rtl::OUString > > CacheItemRegistration;

//_______________________________________________

/** @short  is used to collect all matching types of an URL
            during type detection.

    @descr  Every type in this list is combined with an information,
            which property matched to the given URL. The user of this
            structure can decide then, if a deep detection should be
            supressed e.g. if an URLPattern was used.

            Structure info: NamedValue.Name  = <internal type name>
                            NamedValue.Value = [boolean]bMatchByURLPattern
 */
typedef ::std::vector< css::beans::NamedValue > FlatDetection;

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_CACHEITEM_HXX_
