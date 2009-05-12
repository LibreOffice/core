/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: autoobject.hxx,v $
 * $Revision: 1.4 $
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

#ifndef CONFIGMGR_AUTOOBJECT_HXX
#define CONFIGMGR_AUTOOBJECT_HXX

#include "sal/config.h"

#include "boost/utility.hpp"

#include "utility.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    template < class Object >
    class AutoObject: private boost::noncopyable
    {
    public:
        AutoObject() : m_pObject(NULL) {}
        AutoObject(Object * _obj) : m_pObject(_obj) {}
        ~AutoObject() { delete m_pObject; }

        bool is()   const;
        Object * get()   const;
        Object * getOrCreate();
    private:
        Object * internalCreate();
    private:
        Object *  m_pObject;
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

