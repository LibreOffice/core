/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


