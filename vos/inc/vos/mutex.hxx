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

#ifndef _VOS_MUTEX_HXX_
#define _VOS_MUTEX_HXX_

#   include <vos/types.hxx>
#   include <vos/object.hxx>
#   include <osl/mutex.h>

namespace vos
{


/** IMutex interface

    @author  Bernd Hofner
    @version 1.0
*/

class IMutex
{
public:

    /// Blocks if Mutex is already in use
    virtual void SAL_CALL acquire()= 0;

    // Tries to get the mutex without blocking.
    virtual sal_Bool SAL_CALL tryToAcquire()= 0;

    /// releases the mutex.
    virtual void SAL_CALL release()= 0;

protected:
    IMutex() { }
    virtual ~IMutex() { }

};

// ----------------------------------------------------------

/** OMutex

    @author  Bernd Hofner
    @version 1.0
*/

class OMutex : public OObject, public IMutex
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OMutex, vos));

public:
    static IMutex& SAL_CALL getGlobalMutex();

    /// Creates mutex
    OMutex();
    /// Implicitly destroys mutex
    virtual ~OMutex();

    /// Blocks if Mutex is already in use
    virtual void SAL_CALL acquire();

    /** Tries to get the mutex without blocking.
        @return True if mutex could be obtained, otherwise False
    */
    virtual sal_Bool SAL_CALL tryToAcquire();

    /// releases the mutex.
    virtual void SAL_CALL release();

protected:
    oslMutex    m_Impl;

private:
    // disable copy/assignment
    OMutex(const OMutex&);
    OMutex& SAL_CALL operator= (const OMutex&);
};

// *********************************************************************************

/** OGuard

    @author  Bernd Hofner
    @version 1.0
*/

class OGuard
{
    OGuard( const OGuard& );
    const OGuard& operator = ( const OGuard& );
public:
    /** Acquires mutex
        @param pMutex pointer to mutex which is to be acquired  */
    OGuard(IMutex* pMutex)
        : m_rMutex( *pMutex )
    {   // only for compatible reasons
        m_rMutex.acquire();
    }
    OGuard(IMutex & rMutex)
        : m_rMutex( rMutex )
    {
        m_rMutex.acquire();
    }

    /** Releases mutex. */
    virtual ~OGuard()
    {
        m_rMutex.release();
    }

protected:
    IMutex& m_rMutex;
};

/** A guard that can release the mutex with the clear method.

    @author  Bernd Hofner
    @version 1.0
*/
class OClearableGuard
{
    OClearableGuard( const OClearableGuard& );
    const OClearableGuard& operator = ( const OClearableGuard& );
public:
    /** Acquires mutex
        @param pMutex pointer to mutex which is to be acquired  */
    OClearableGuard(IMutex & rMutex)
        : m_pMutex( &rMutex )
    {
        m_pMutex->acquire();
    }

    /** Releases mutex. */
    virtual ~OClearableGuard()
    {
        if( m_pMutex )
            m_pMutex->release();
    }

    /** Releases mutex. */
    void SAL_CALL clear()
    {
        if( m_pMutex )
        {
            m_pMutex->release();
            m_pMutex = NULL;
        }
    }
protected:
    IMutex* m_pMutex;
};

}


#endif  //_VOS_MUTEX_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
