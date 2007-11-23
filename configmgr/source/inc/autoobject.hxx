/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autoobject.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:14:51 $
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

#ifndef CONFIGMGR_AUTOOBJECT_HXX
#define CONFIGMGR_AUTOOBJECT_HXX

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    using ::rtl::OUString;

//-----------------------------------------------------------------------------
    template < class Object >
    class AutoObject : Noncopyable
    {
    public:
        typedef Object      object_type;
        typedef Object *    Ptr;
    public:
        AutoObject() : m_pObject(NULL) {}
        AutoObject(Object * _obj) : m_pObject(_obj) {}
        ~AutoObject() { delete m_pObject; }

        bool is()   const;
        Ptr get()   const;
        Ptr getOrCreate();
    private:
        Ptr internalCreate();
    private:
        Ptr  m_pObject;
    };
//-----------------------------------------------------------------------------

    template < class Object >
    inline
    Object * AutoObject<Object>::get() const
    {
        return m_pObject;
    }
//-----------------------------------------------------------------------------

    template < class Object >
    inline
    bool AutoObject<Object>::is() const
    {
        return get() != NULL;
    }
//-----------------------------------------------------------------------------

    template < class Object >
    Object * AutoObject<Object>::getOrCreate()
    {
        Object * p = get();
        return p ? p : internalCreate();
    }
//-----------------------------------------------------------------------------

    template < class Object >
    Object * AutoObject<Object>::internalCreate()
    {
        if (m_pObject == NULL)
            m_pObject = new Object();
        return m_pObject;

    }
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

