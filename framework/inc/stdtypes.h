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

#ifndef __FRAMEWORK_STDTYPES_H_
#define __FRAMEWORK_STDTYPES_H_

#include <vector>
#include <queue>
#include <boost/unordered_map.hpp>

#include <general.h>

#ifndef __COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif

#include <comphelper/sequenceasvector.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

/**
    Own hash functions used for stl-structures ... e.g. hash tables/maps ...
*/
struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

struct ShortHashCode
{
    size_t operator()( const ::sal_Int16& nShort ) const
    {
        return (size_t)nShort;
    }
};

struct Int32HashCode
{
    size_t operator()( const ::sal_Int32& nValue ) const
    {
        return (size_t)nValue;
    }
};

struct KeyEventHashCode
{
    size_t operator()( const css::awt::KeyEvent& aEvent ) const
    {
        return (size_t)(aEvent.KeyCode  +
                        //aEvent.KeyChar  +
                        //aEvent.KeyFunc  +
                        aEvent.Modifiers);
    }
};

struct KeyEventEqualsFunc
{
    bool operator()(const css::awt::KeyEvent aKey1,
                    const css::awt::KeyEvent aKey2) const
    {
        return (
                (aKey1.KeyCode   == aKey2.KeyCode  ) &&
                //(aKey1.KeyChar   == aKey2.KeyChar  ) &&
                //(aKey1.KeyFunc   == aKey2.KeyFunc  ) &&
                (aKey1.Modifiers == aKey2.Modifiers)
               );
    }
};

//_________________________________________________________________________________________________________________

/**
    Basic string list based on a std::vector()
    It implements some additional funtionality which can be usefull but
    is missing at the normal vector implementation.
*/
class OUStringList : public ::comphelper::SequenceAsVector< ::rtl::OUString >
{
    public:

        // insert given element as the first one into the vector
        void push_front( const ::rtl::OUString& sElement )
        {
            insert( begin(), sElement );
        }

        // search for given element
        iterator find( const ::rtl::OUString& sElement )
        {
            return ::std::find(begin(), end(), sElement);
        }

        const_iterator findConst( const ::rtl::OUString& sElement ) const
        {
            return ::std::find(begin(), end(), sElement);
        }

        // the only way to free used memory realy!
        void free()
        {
            OUStringList().swap( *this );
        }
};

//_________________________________________________________________________________________________________________

/**
    Basic string queue based on a std::queue()
    It implements some additional funtionality which can be usefull but
    is missing at the normal std implementation.
*/
typedef ::std::queue< ::rtl::OUString > OUStringQueue;

//_________________________________________________________________________________________________________________

/**
    Basic hash based on a boost::unordered_map() which provides key=[OUString] and value=[template type] pairs
    It implements some additional funtionality which can be usefull but
    is missing at the normal hash implementation.
*/
template< class TType >
class BaseHash : public ::boost::unordered_map< ::rtl::OUString                    ,
                                         TType                              ,
                                         OUStringHashCode                   ,
                                         ::std::equal_to< ::rtl::OUString > >
{
    public:

        // the only way to free used memory realy!
        void free()
        {
            BaseHash().swap( *this );
        }
};

//_________________________________________________________________________________________________________________

/**
    Basic OUString hash.
    Key and values are OUStrings.
*/
typedef BaseHash< ::rtl::OUString > OUStringHash;

//_________________________________________________________________________________________________________________

/**
    It can be used to map names (e.g. of properties) to her corresponding handles.
    Our helper class OPropertySetHelper works optimized with handles but sometimes we have only a property name.
    Mapping between these two parts of a property should be done in the fastest way :-)
*/
typedef BaseHash< sal_Int32 > NameToHandleHash;

//_________________________________________________________________________________________________________________

/**
    Sometimes we need this template to implement listener container ...
    and we need it at different positions ...
    So it's better to declare it one times only!
*/
typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<  ::rtl::OUString                    ,
                                                        OUStringHashCode                   ,
                                                        ::std::equal_to< ::rtl::OUString > >    ListenerHash;

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_STDTYPES_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
