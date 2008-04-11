/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sequenceashashmap.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#define _COMPHELPER_SEQUENCEASHASHMAP_HXX_

//_______________________________________________
// includes

#ifndef INCLUDED_HASH_MAP
#include <hash_map>
#define INCLUDED_HASH_MAP
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#ifndef _COM_SUN_STAR_BEANS_IllegalTypeException_HPP_
#include <com/sun/star/beans/IllegalTypeException.hpp>
#endif
#include "comphelper/comphelperdllapi.h"

// see method dbg_dumpToFile() below!
#if OSL_DEBUG_LEVEL > 1
    #ifndef _RTL_USTRBUF_HXX_
    #include <rtl/ustrbuf.hxx>
    #endif

    #include <stdio.h>
#endif

//_______________________________________________
// namespace

namespace comphelper{

//_______________________________________________
// definitions

/** @short  Implements a stl hash map on top of some
            specialized sequence from type PropertyValue
            or NamedValue.

    @descr  That provides the possibility to modify
            such name sequences very easy ...
 */

struct SequenceAsHashMapBase : public ::std::hash_map<
    ::rtl::OUString                    ,
    ::com::sun::star::uno::Any         ,
    ::rtl::OUStringHash                ,
    ::std::equal_to< ::rtl::OUString > >
{
};

class COMPHELPER_DLLPUBLIC SequenceAsHashMap : public SequenceAsHashMapBase
{
    //-------------------------------------------
    public:

        //---------------------------------------
        /** @short  creates an empty hash map.
         */
        SequenceAsHashMap();

        //---------------------------------------
        /** @see    operator<<(const ::com::sun::star::uno::Any&)
         */
        SequenceAsHashMap(const ::com::sun::star::uno::Any& aSource);

        //---------------------------------------
        /** @see    operator<<(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >&)
         */
        SequenceAsHashMap(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& lSource);

        //---------------------------------------
        /** @see    operator<<(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&)
         */
        SequenceAsHashMap(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lSource);

        //---------------------------------------
        /** @see    operator<<(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >&)
         */
        SequenceAsHashMap(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& lSource);

        //---------------------------------------
        /** @short  not realy used but maybe usefull :-)
         */
        ~SequenceAsHashMap();

        //---------------------------------------
        /** @short  fill this map from the given
                    any, which of course must contain
                    a suitable sequence of element types
                    "css.beans.PropertyValue" or "css.beans.NamedValue".

            @attention  If the given Any is an empty one
                        (if its set to VOID), no exception
                        is thrown. In such case this instance will
                        be created as an empty one too!

            @param  aSource
                    contains the new items for this map.

            @throw  An <type scope="com::sun::star::beans">IllegalTypeException</type>
                    is thrown, if the given any does not contain a suitable sequence ...
                    but not if its a VOID Any!
         */
        void operator<<(const ::com::sun::star::uno::Any& aSource);

        //---------------------------------------
        /** @short  fill this map from the given
                    sequence, where every Any must contain
                    an item from type "css.beans.PropertyValue"
                    "css.beans.NamedValue".

            @param  lSource
                    contains the new items for this map.

            @throw  An <type scope="com::sun::star::beans">IllegalTypeException</type>
                    is thrown, if the given any sequence
                    uses wrong types for its items. VOID Any will be ignored!
         */
        void operator<<(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& lSource);

        //---------------------------------------
        /** @short  fill this map from the given
                    PropertyValue sequence.

            @param  lSource
                    contains the new items for this map.
         */
        void operator<<(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lSource);

        //---------------------------------------
        /** @short  fill this map from the given
                    NamedValue sequence.

            @param  lSource
                    contains the new items for this map.
         */
        void operator<<(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& lSource);

        //---------------------------------------
        /** @short  converts this map instance to an
                    PropertyValue sequence.

            @param  lDestination
                    target sequence for converting.
         */
        void operator>>(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDestination) const;

        //---------------------------------------
        /** @short  converts this map instance to an
                    NamedValue sequence.

            @param  lDestination
                    target sequence for converting.
         */
        void operator>>(::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& lDestination) const;

        //---------------------------------------
        /** @short  return this map instance as an
                    Any, which can be
                    used in const environments only.

            @descr  Its made const to prevent using of the
                    return value directly as an in/out parameter!
                    usage: myMethod(stlDequeAdapter.getAsAnyList());

            @param  bAsPropertyValue
                    switch between using of PropertyValue or NamedValue as
                    value type.

            @return A const Any, which
                    contains all items of this map.
         */
        const ::com::sun::star::uno::Any getAsConstAny(::sal_Bool bAsPropertyValue) const;

        //---------------------------------------
        /** @short  return this map instance as a
                    sequence< Any >, which can be
                    used in const environments only.

            @descr  Its made const to prevent using of the
                    return value directly as an in/out parameter!
                    usage: myMethod(stlDequeAdapter.getAsAnyList());

            @param  bAsPropertyValue
                    switch between using of PropertyValue or NamedValue as
                    value type.

            @return A const sequence which elements of Any, which
                    contains all items of this map.
         */
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > getAsConstAnyList(::sal_Bool bAsPropertyValue) const;

        //---------------------------------------
        /** @short  return this map instance to as a
                    NamedValue sequence, which can be
                    used in const environments only.

            @descr  Its made const to prevent using of the
                    return value directly as an in/out parameter!
                    usage: myMethod(stlDequeAdapter.getAsNamedValueList());

            @return A const sequence of type NamedValue, which
                    contains all items of this map.
         */
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > getAsConstNamedValueList() const;

        //---------------------------------------
        /** @short  return this map instance to as a
                    PropertyValue sequence, which can be
                    used in const environments only.

            @descr  Its made const to prevent using of the
                    return value directly as an in/out parameter!
                    usage: myMethod(stlDequeAdapter.getAsPropertyValueList());

            @return A const sequence of type PropertyValue, which
                    contains all items of this map.
         */
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > getAsConstPropertyValueList() const;

        //---------------------------------------
        /** @short  check if the specified item exists
                    and return its (unpacked!) value or it returns the
                    specified default value otherwhise.

            @descr  If a value should be extracted only in case
                    the requsted property exists realy (without creating
                    of new items as it the index operator of a
                    has_map does!) this method can be used.

            @param  sKey
                    key name of the item.

            @param  aDefault
                    the default value, which is returned
                    if the specified item could not
                    be found.

            @return The (unpacked!) value of the specified property or
                    the given default value otherwhise.

            @attention  "unpacked" means the Any content of every iterator->second!
         */
        template< class TValueType >
        TValueType getUnpackedValueOrDefault(const ::rtl::OUString& sKey    ,
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

        //---------------------------------------
        /** @short  creates a new item with the specified
                    name and value only in case such item name
                    does not already exist.

            @descr  To check if the property already exists only
                    her name is used for compare. Its value isnt
                    checked!

            @param  sKey
                    key name of the property.

            @param  aValue
                    the new (unpacked!) value.
                    Note: This value will be transformed to an Any
                    internaly, because only Any values can be
                    part of a PropertyValue or NamedValue structure.

            @return TRUE if this property was added as new item;
                    FALSE if it already exists.
         */
        template< class TValueType >
        sal_Bool createItemIfMissing(const ::rtl::OUString& sKey  ,
                                     const TValueType&      aValue)
        {
            if (find(sKey) == end())
            {
                (*this)[sKey] = ::com::sun::star::uno::makeAny(aValue);
                return sal_True;
            }

            return sal_False;
        }

        //---------------------------------------
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
        sal_Bool match(const SequenceAsHashMap& rCheck) const;

        //---------------------------------------
        /** @short  merge all values from the given map into
                    this one.

            @descr  Existing items will be overwritten ...
                    missing items will be created new ...
                    but non specified items will stay alive !

            @param  rSource
                    the map containing all items for the update.
         */
        void update(const SequenceAsHashMap& rSource);

        //---------------------------------------
        /** @short  can be used to generate a file dump of
                    the current content of this instance.

            @descr  Because the content of STL container
                    cant be analyzed easy, such dump function
                    seem to be usefull.
                    Of course its available in debug versions
                    only.

            @param  pFileName
                    a system file name.
                    (doesnt matter if relativ or absolute)

            @param  pComment
                    used to mark the dump inside the same log file.
                    Can be usefull to analyze changes of this
                    hash map due to the parts of an operation.
         */
        #if OSL_DEBUG_LEVEL > 1
        void dbg_dumpToFile(const char* pFileName, const char* pComment) const;
        #endif
};

} // namespace comphelper

#endif // _COMPHELPER_SEQUENCEASHASHMAP_HXX_
