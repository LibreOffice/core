/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mutex.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:00:15 $
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

#ifndef _VOS_MUTEX_HXX_
#define _VOS_MUTEX_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _OSL_MUTEX_H_
#   include <osl/mutex.h>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif


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


#ifdef _USE_NAMESPACE
}
#endif


#endif  //_VOS_MUTEX_HXX_


