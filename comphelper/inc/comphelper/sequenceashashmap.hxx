/*************************************************************************
 *
 *  $RCSfile: sequenceashashmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 12:45:49 $
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

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#define _COMPHELPER_SEQUENCEASHASHMAP_HXX_

//_______________________________________________
// includes

#include <hash_map>
#include <algorithm>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_IllegalTypeException_HPP_
#include <com/sun/star/beans/IllegalTypeException.hpp>
#endif

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

typedef ::std::hash_map< ::rtl::OUString                    ,
                         ::com::sun::star::uno::Any         ,
                         ::rtl::OUStringHash                ,
                         ::std::equal_to< ::rtl::OUString > > SequenceAsHashMapBase;

class SequenceAsHashMap : public SequenceAsHashMapBase
{
    //-------------------------------------------
    public:

        //---------------------------------------
        /** @short  creates an empty hash map.
         */
        SequenceAsHashMap();

        //---------------------------------------
        /** @short  creates a new map from the given
                    any, which must contain a suitable
                    sequence of element types
                    "css.beans.PropertyValue" or
                    "css.beans.NamedValue".

            @attention  If the given Any is an empty one
                        (if its set to VOID), no exception
                        is thrown. In such case this instance will
                        be created as an empty one too!

            @param  aSource
                    contains the new items for this map.

            @throw  An <type scope="com::sun::star::beans">IllegalTypeException</type>
                    is thrown, if the given any
                    does not contain a suitable sequence.
         */
        SequenceAsHashMap(const ::com::sun::star::uno::Any& aSource);

        //---------------------------------------
        /** @short  creates a new map from the given
                    PropertyValue sequence.

            @attention  Only the struct elements "Name" and
                        "Value" are used here. The items "Handle"
                        and "State" will be ignored and cant be
                        restored later!

            @param  lSource
                    contains the new items for this map.
         */
        SequenceAsHashMap(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lSource);

        //---------------------------------------
        /** @short  creates a new map from the given
                    NamedValue sequence.

            @param  lSource
                    contains the new items for this map.
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
                    is thrown, if the given any
                    does not contain a suitable sequence.
         */
        void operator<<(const ::com::sun::star::uno::Any& aSource);

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

            TValueType aValue;
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
