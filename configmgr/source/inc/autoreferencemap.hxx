/*************************************************************************
 *
 *  $RCSfile: autoreferencemap.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-12 14:10:33 $
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

#ifndef CONFIGMGR_AUTOREFERENCEMAP_HXX
#define CONFIGMGR_AUTOREFERENCEMAP_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
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

        void swap(Map & _rOtherData)
        {
            m_aMap.swap( _rOtherData );
        }
        void swap(AutoReferenceMap & _rOther)
        {
            this->swap( _rOther.m_aMap );
        }


        bool has(Key const & _aKey) const;
        Ref getExisting(Key const & _aKey) const;
        Ref getOrCreate(Key const & _aKey);
        Ref remove(Key const & _aKey);

        osl::Mutex & mutex() const { return m_aMutex; }
    private:
        Ref internalGet(Key const & _aKey) const
        {
            typename Map::const_iterator it = m_aMap.find(_aKey);

            return it != m_aMap.end() ? it->second : Ref();
        }

        Ref internalNew(Key const & _aKey)
        {
            Ref aNewRef(new Object());

            return m_aMap[_aKey] = aNewRef;
        }

        void internalDrop(Key const & _aKey)
        {
            m_aMap.erase(_aKey);
        }
    private:
        mutable osl::Mutex  m_aMutex;
        Map                 m_aMap;
    };
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    bool AutoReferenceMap<Key,Object,KeyCompare>::has(Key const & _aKey) const
    {
        osl::MutexGuard aGuard(m_aMutex);
        return internalGet(_aKey).is();
    }
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::getExisting(Key const & _aKey) const
    {
        osl::MutexGuard aGuard(m_aMutex);
        return internalGet(_aKey);
    }
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::getOrCreate(Key const & _aKey)
    {
        osl::MutexGuard aGuard(m_aMutex);
        Ref aRef = internalGet(_aKey);
        if (!aRef.is())
            aRef = internalNew(_aKey);
        return aRef;

    }
//-----------------------------------------------------------------------------

    template < class Key, class Object, class KeyCompare >
    rtl::Reference<Object> AutoReferenceMap<Key,Object,KeyCompare>::remove(Key const & _aKey)
    {
        osl::MutexGuard aGuard(m_aMutex);
        Ref aRef = internalGet(_aKey);
        internalDrop(_aKey);
        return aRef;
    }
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

