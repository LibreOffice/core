/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mutexholder.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:00:13 $
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

#ifndef __MUTEXHOLDER_HXX_
#define __MUTEXHOLDER_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

class SotMutexHolder
{
    ::osl::Mutex m_aMutex;
    sal_Int32    m_nRefCount;

    public:
    SotMutexHolder() : m_nRefCount( 0 ) {}

    void AddRef()
    {
        m_nRefCount++;
    }

    void ReleaseRef()
    {
        if ( !--m_nRefCount )
            delete this;
    }

    ::osl::Mutex& GetMutex() { return m_aMutex; }
};

class SotMutexHolderRef
{
    SotMutexHolder* m_pHolder;

public:
    SotMutexHolderRef()
    : m_pHolder( NULL )
    {}

    SotMutexHolderRef( SotMutexHolder* pHolder )
    : m_pHolder( pHolder )
    {
        if ( m_pHolder )
            m_pHolder->AddRef();
    }

    SotMutexHolderRef( const SotMutexHolderRef& rRef )
    : m_pHolder( rRef.m_pHolder )
    {
        if ( m_pHolder )
            m_pHolder->AddRef();
    }

    ~SotMutexHolderRef()
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();
    }

    SotMutexHolderRef& operator =( const SotMutexHolderRef& rRef )
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();

        m_pHolder = rRef.m_pHolder;

        if ( m_pHolder )
            m_pHolder->AddRef();

        return *this;
    }

    SotMutexHolderRef& operator =( SotMutexHolder* pHolder )
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();

        m_pHolder = pHolder;

        if ( m_pHolder )
            m_pHolder->AddRef();
        return *this;
    }

    SotMutexHolder* operator ->() const
    {
        return m_pHolder;
    }

    SotMutexHolder& operator *() const
    {
        return *m_pHolder;
    }

    operator SotMutexHolder*() const
    {
        return m_pHolder;
    }

    sal_Bool Is() const
    {
        return m_pHolder != NULL;
    }
};

#endif

