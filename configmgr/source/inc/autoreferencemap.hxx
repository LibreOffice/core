/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autoreferencemap.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:15:08 $
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

#ifndef CONFIGMGR_AUTOREFERENCEMAP_HXX
#define CONFIGMGR_AUTOREFERENCEMAP_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    using ::rtl::OUString;

//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare = std::less<Key> >
    class AutoReferenceMap
    {
    public:
        typedef rtl::Reference<Object>         Ref;
        typedef std::map<Key,Ref,KeyCompare>   Map;

        typedef Object      object_type;
        typedef Key         key_type;
        typedef KeyCompare  key_compare;

        typedef typename Map::value_type value_type;
    public:
        AutoReferenceMap()  {}
        ~AutoReferenceMap() {}

        Map copy() const
        {
            return m_aMap;
        }
        void swap(Map & _rOtherData)
        {
            m_aMap.swap( _rOtherData );
        }
        void swap(AutoReferenceMap & _rOther)
        {
            this->swap( _rOther.m_aMap );
        }


        bool has(Key const & _aKey) const;
        Ref get(Key const & _aKey) const;

        Ref insert(Key const & _aKey, Ref const & _anEntry);
        Ref remove(Key const & _aKey);

    private:
        Ref internalGet(Key const & _aKey) const
        {
            typename Map::const_iterator it = m_aMap.find(_aKey);

            return it != m_aMap.end() ? it->second : Ref();
        }

        Ref internalAdd(Key const & _aKey, Ref const & _aNewRef)
        {
            return m_aMap[_aKey] = _aNewRef;
        }

        void internalDrop(Key const & _aKey)
        {
            m_aMap.erase(_aKey);
        }
    private:
        Map                 m_aMap;
    };
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    bool AutoReferenceMap<Key,Object,KeyCompare>::has(Key const & _aKey) const
    {
        return internalGet(_aKey).is();
    }
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::get(Key const & _aKey) const
    {
        return internalGet(_aKey);
    }
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::insert(Key const & _aKey, Ref const & _anEntry)
    {
        Ref aRef = internalAdd(_aKey,_anEntry);
        return aRef;

    }
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::remove(Key const & _aKey)
    {
        Ref aRef = internalGet(_aKey);
        internalDrop(_aKey);
        return aRef;
    }
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

