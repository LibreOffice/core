/*************************************************************************
 *
 *  $RCSfile: autoobject.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-28 08:41:26 $
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

#ifndef CONFIGMGR_AUTOOBJECT_HXX
#define CONFIGMGR_AUTOOBJECT_HXX

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
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

        osl::Mutex & mutex() const { return m_aMutex; }
    private:
        Ptr internalCreate();
    private:
        mutable osl::Mutex  m_aMutex;
        Ptr  m_pObject;
    };
//-----------------------------------------------------------------------------

    template < class Object >
    inline
    Object * AutoObject<Object>::get() const
    {
        // osl::MutexGuard aGuard(m_aMutex);
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
        osl::MutexGuard aGuard(m_aMutex);
        if (m_pObject == NULL)
            m_pObject = new Object();
        return m_pObject;

    }
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

