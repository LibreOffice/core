/*************************************************************************
 *
 *  $RCSfile: mutexholder.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 17:49:00 $
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

