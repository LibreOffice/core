/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestProxy.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:46:27 $
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

#include "uno/mapping.hxx"
#include "cppu/EnvDcp.hxx"

#include "../mapping_tester/Mapping.tester.hxx"

#include "TestProxy.hxx"


#ifdef LOG_LIFECYCLE
#define LOG_LIFECYCLE_TestProxy
#endif

#define LOG_LIFECYCLE_TestProxy
#ifdef LOG_LIFECYCLE_TestProxy
#  include <iostream>
#  define LOG_LIFECYCLE_TestProxy_emit(x) x

#else
#  define LOG_LIFECYCLE_TestProxy_emit(x)

#endif


using namespace com::sun::star;

extern "C" void SAL_CALL TestProxy_free(uno_ExtEnvironment * /*pEnv*/, void * pObject)
    SAL_THROW_EXTERN_C()
{
    TestProxy * pTestProxy = reinterpret_cast<TestProxy *>(pObject);
    delete pTestProxy;
}


extern "C" {
static void SAL_CALL s_acquire(uno_Interface * pUnoI) SAL_THROW_EXTERN_C()
{
    TestProxy * pTestProxy = static_cast<TestProxy *>(pUnoI);
    pTestProxy->acquire();
}

static void SAL_CALL s_release(uno_Interface * pUnoI) SAL_THROW_EXTERN_C()
{
    TestProxy * pTestProxy = static_cast<TestProxy *>(pUnoI);
    pTestProxy->release();
}

static void SAL_CALL s_dispatch(uno_Interface                 * pUnoI,
                                typelib_TypeDescription const * pMemberType,
                                void                          * pReturn,
                                void                          * pArgs[],
                                uno_Any                      ** ppException)
    SAL_THROW_EXTERN_C()
{
    TestProxy * pThis = static_cast<TestProxy *>(pUnoI);
    pThis->dispatch(pMemberType, pReturn, pArgs, ppException);
}
}

TestProxy::TestProxy(uno_Interface                          * pObject,
                     rtl::OUString                    const & oid,
                     typelib_InterfaceTypeDescription       * pTypeDescr,
                     uno_ExtEnvironment                     * pFrom_extEnv,
                     uno_ExtEnvironment                     * pTo_extEnv)
    : m_nCount     (1),
      m_from_envDcp(pFrom_extEnv->aBase.pTypeName),
      m_to_envDcp  (pTo_extEnv  ->aBase.pTypeName),
      m_oid        (oid),
      m_pTypeDescr (pTypeDescr)
{
    LOG_LIFECYCLE_TestProxy_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestProxy::TestProxy", this));

    // uno_Interface
    uno_Interface::acquire     = s_acquire;
    uno_Interface::release     = s_release;
    uno_Interface::pDispatcher = s_dispatch;

    m_theObject = pObject;
    m_theObject->acquire(m_theObject);

    typelib_typedescription_acquire(&pTypeDescr->aBase);

    m_pFrom_extEnv = pFrom_extEnv;
    m_pFrom_extEnv->aBase.acquire(&m_pFrom_extEnv->aBase);

    m_pTo_extEnv = pTo_extEnv;
    m_pTo_extEnv->aBase.acquire(&m_pTo_extEnv->aBase);

    m_pTo_extEnv->registerInterface(m_pTo_extEnv, reinterpret_cast<void **>(&pObject), oid.pData, pTypeDescr);
}

TestProxy::~TestProxy(void)
{
    LOG_LIFECYCLE_TestProxy_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestProxy::~TestProxy", this));

    m_pTo_extEnv->revokeInterface(m_pTo_extEnv, reinterpret_cast<void *>(m_theObject));

    typelib_typedescription_release(&m_pTypeDescr->aBase);
    m_theObject->release(m_theObject);
    m_pFrom_extEnv->aBase.release(&m_pFrom_extEnv->aBase);
    m_pTo_extEnv  ->aBase.release(&m_pTo_extEnv->aBase);
}

void TestProxy::acquire() SAL_THROW(())
{
    if (osl_incrementInterlockedCount(&m_nCount) == 1)
    {
        uno_Interface * pThis = this;
        m_pTo_extEnv->registerProxyInterface(m_pTo_extEnv,
                                             (void **)&pThis,
                                             TestProxy_free,
                                             m_oid.pData,
                                             m_pTypeDescr);
        OSL_ASSERT(pThis == this);
    }
}

void TestProxy::release() SAL_THROW(())
{
    if (osl_decrementInterlockedCount(&m_nCount) == 0)
    {
        m_pFrom_extEnv->revokeInterface(m_pFrom_extEnv, this);
    }
}

void TestProxy::dispatch(typelib_TypeDescription const * pMemberType,
                     void                          * pReturn,
                     void                          * pArgs[],
                     uno_Any                      ** ppException)
{
    {
        rtl::OUString arrow(RTL_CONSTASCII_USTRINGPARAM("-->"));

        if (!g_custom.getLength())
            g_custom += m_from_envDcp;

        g_custom += arrow;
        g_custom += m_to_envDcp;

        m_theObject->pDispatcher(m_theObject, pMemberType, pReturn, pArgs, ppException);

         uno_Any * any = (uno_Any *)pReturn;

        void * pout = NULL;

        uno::Mapping mapping(m_to_envDcp, m_from_envDcp);
        mapping.mapInterface(&pout, any->pReserved, any->pType);

        ((uno_Interface *)any->pReserved)->release((uno_Interface*)any->pReserved);
        any->pReserved = pout;
    }
}

