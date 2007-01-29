/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stdtypes.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-29 15:50:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_STDTYPES_H_
#define __FRAMEWORK_STDTYPES_H_

#include <vector>
#include <queue>
#include <hash_map>

//_________________________________________________________________________________________________________________
// own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
// interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif

//_________________________________________________________________________________________________________________
// other includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#include <comphelper/sequenceasvector.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
    size_t operator()( const sal_Int16& nShort ) const
    {
        return (size_t)nShort;
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
    Basic hash based on a std::hash_map() which provides key=[OUString] and value=[template type] pairs
    It implements some additional funtionality which can be usefull but
    is missing at the normal hash implementation.
*/
template< class TType >
class BaseHash : public ::std::hash_map< ::rtl::OUString                    ,
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
