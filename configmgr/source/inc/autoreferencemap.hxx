/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: autoreferencemap.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_AUTOREFERENCEMAP_HXX
#define CONFIGMGR_AUTOREFERENCEMAP_HXX

#include <rtl/ref.hxx>
#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare = std::less<Key> >
    class AutoReferenceMap
    {
    public:
        typedef std::map<Key,rtl::Reference<Object>,KeyCompare>   Map;
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
        rtl::Reference<Object> get(Key const & _aKey) const;

        rtl::Reference<Object> insert(Key const & _aKey, rtl::Reference<Object> const & _anEntry);
        rtl::Reference<Object> remove(Key const & _aKey);

    private:
        rtl::Reference<Object> internalGet(Key const & _aKey) const
        {
            typename Map::const_iterator it = m_aMap.find(_aKey);

            return it != m_aMap.end() ? it->second : rtl::Reference<Object>();
        }

        rtl::Reference<Object> internalAdd(Key const & _aKey, rtl::Reference<Object> const & _aNewRef)
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
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::insert(Key const & _aKey, rtl::Reference<Object> const & _anEntry)
    {
        rtl::Reference<Object> aRef = internalAdd(_aKey,_anEntry);
        return aRef;

    }
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::remove(Key const & _aKey)
    {
        rtl::Reference<Object> aRef = internalGet(_aKey);
        internalDrop(_aKey);
        return aRef;
    }
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

