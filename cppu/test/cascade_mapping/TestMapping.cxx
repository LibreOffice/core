/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestMapping.cxx,v $
 * $Revision: 1.3 $
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

#include "osl/interlck.h"
#include "uno/dispatcher.h"
#include "uno/mapping.hxx"

#include "TestProxy.hxx"


#ifdef LOG_LIFECYCLE
#define LOG_LIFECYCLE_TestMapping
#endif

#define LOG_LIFECYCLE_TestMapping
#ifdef LOG_LIFECYCLE_TestMapping
#  include <iostream>
#  define LOG_LIFECYCLE_TestMapping_emit(x) x

#else
#  define LOG_LIFECYCLE_TestMapping_emit(x)

#endif


class SAL_DLLPRIVATE TestMapping : public uno_Mapping
{
private:
    oslInterlockedCount   m_nCount;
    uno_ExtEnvironment  * m_pFrom;
    uno_ExtEnvironment  * m_pTo;

public:
    explicit  TestMapping(uno_Environment * pFrom, uno_Environment * pTo);
             ~TestMapping(void);

    void acquire() SAL_THROW(());
    void release() SAL_THROW(());

    void SAL_CALL mapInterface(uno_Interface                    ** ppOut,
                               uno_Interface                     * pUnoI,
                               typelib_InterfaceTypeDescription  * pTypeDescr
        )
        SAL_THROW_EXTERN_C();
};


extern "C" {
static void SAL_CALL s_mapInterface(
    uno_Mapping                       * pMapping,
    void                             ** ppOut,
    void                              * pUnoI,
    typelib_InterfaceTypeDescription  * pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    TestMapping * pTestMapping = static_cast<TestMapping *>(pMapping);
    pTestMapping->mapInterface((uno_Interface **)ppOut, (uno_Interface *)pUnoI, pTypeDescr);
}


static void SAL_CALL s_acquire(uno_Mapping * pMapping) SAL_THROW_EXTERN_C()
{
    TestMapping * pTestMapping = static_cast<TestMapping *>(pMapping);
    pTestMapping->acquire();
}


static void SAL_CALL s_release(uno_Mapping * pMapping) SAL_THROW_EXTERN_C()
{
    TestMapping * pTestMapping = static_cast<TestMapping * >(pMapping);
    pTestMapping->release();
}

static void SAL_CALL s_free(uno_Mapping * pMapping) SAL_THROW_EXTERN_C()
{
    TestMapping * pTestMapping = static_cast<TestMapping * >(pMapping);
    delete pTestMapping;
}
}

TestMapping::TestMapping(uno_Environment * pFrom, uno_Environment * pTo)
    SAL_THROW( () )
        : m_nCount(1),
          m_pFrom(reinterpret_cast<uno_ExtEnvironment *>(pFrom)),
          m_pTo  (reinterpret_cast<uno_ExtEnvironment *>(pTo))
{
    LOG_LIFECYCLE_TestMapping_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestMapping::TestMapping", this));

    m_pFrom->aBase.acquire(&m_pFrom->aBase);
    m_pTo  ->aBase.acquire(&m_pTo  ->aBase);

    uno_Mapping::acquire      = s_acquire;
    uno_Mapping::release      = s_release;
    uno_Mapping::mapInterface = s_mapInterface;
}

TestMapping::~TestMapping(void)
{
    LOG_LIFECYCLE_TestMapping_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestMapping::~TestMapping", this));

    m_pFrom->aBase.release(&m_pFrom->aBase);
    m_pTo  ->aBase.release(&m_pTo  ->aBase);
}


void TestMapping::acquire() SAL_THROW(())
{
    if (osl_incrementInterlockedCount(&m_nCount) == 1)
    {
        uno_Mapping * pMapping = this;

        ::uno_registerMapping(&pMapping, s_free, &m_pFrom->aBase, &m_pTo->aBase, NULL);
    }
}

void TestMapping::release() SAL_THROW(())
{
    if (osl_decrementInterlockedCount(&m_nCount) == 0)
        ::uno_revokeMapping(this);
}


void SAL_CALL TestMapping::mapInterface(
    uno_Interface                    ** ppOut,
    uno_Interface                     * pUnoI,
    typelib_InterfaceTypeDescription  * pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    // get object id of uno interface to be wrapped
    rtl_uString * pOId = 0;
    m_pFrom->getObjectIdentifier(m_pFrom, &pOId, pUnoI);

    OSL_ASSERT(pOId);

    if (*ppOut)
    {
        (*ppOut)->release(*ppOut);
        *ppOut = 0;
    }

    // try to get any known interface from target environment
    m_pTo->getRegisteredInterface(m_pTo, (void **)ppOut, pOId, pTypeDescr);
    if (!*ppOut) // not yet there, register new proxy interface
    {
        // try to publish a new proxy (ref count initially 1)
        TestProxy * pTestProxy = new TestProxy(pUnoI, pOId, pTypeDescr, m_pTo, m_pFrom);

        // proxy may be exchanged during registration
        m_pTo->registerProxyInterface(m_pTo,(void **)&pTestProxy, TestProxy_free, pOId, pTypeDescr);

        *ppOut = pTestProxy;
    }

    rtl_uString_release(pOId);
}


extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_initEnvironment(uno_Environment * /*pEnv*/)
    SAL_THROW_EXTERN_C()
{
}

extern "C" void uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                   uno_Environment  * pFrom,
                                   uno_Environment  * pTo )
{
    *ppMapping = new TestMapping(pFrom, pTo);

    ::uno_registerMapping(ppMapping, s_free, pFrom, pTo, NULL);
}
