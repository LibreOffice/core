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



#include "IdentityMapping.hxx"

#include "uno/mapping.h"
#include "uno/environment.hxx"

#include "osl/interlck.h"


using namespace ::com::sun::star;

struct IdentityMapping : public uno_Mapping
{
    sal_Int32        m_nRef;
    uno::Environment m_env;

    IdentityMapping(uno::Environment const & rEnv);
};

extern "C"
{

static void SAL_CALL s_free(uno_Mapping * pMapping) SAL_THROW(())
{
    delete static_cast<IdentityMapping *>(pMapping);
}

static void SAL_CALL s_acquire(uno_Mapping * pMapping) SAL_THROW(())
{
    static rtl::OUString s_purpose;

    if (1 == ::osl_incrementInterlockedCount(&static_cast<IdentityMapping *>(pMapping)->m_nRef))
    {
        uno_registerMapping(
            &pMapping,
            s_free,
            static_cast<IdentityMapping *>(pMapping)->m_env.get(),
            static_cast<IdentityMapping *>(pMapping)->m_env.get(),
            s_purpose.pData);
    }
}

static void SAL_CALL s_release(uno_Mapping * pMapping) SAL_THROW(())
{
    if (!::osl_decrementInterlockedCount(&static_cast<IdentityMapping *>(pMapping )->m_nRef))
        uno_revokeMapping(pMapping);
}

static void SAL_CALL s_mapInterface(uno_Mapping                       * pMapping,
                                    void                             ** ppOut,
                                    void                              * pInterface,
                                    struct _typelib_InterfaceTypeDescription * /*pInterfaceTypeDescr*/)
    SAL_THROW(())
{
    *ppOut = pInterface;

    if (pInterface)
    {
        IdentityMapping * that = static_cast<IdentityMapping *>(pMapping);

        (that->m_env.get()->pExtEnv->acquireInterface)(that->m_env.get()->pExtEnv, pInterface);
    }
}
}


IdentityMapping::IdentityMapping(uno::Environment const & rEnv)
    : m_nRef(0),
      m_env(rEnv)
{
    uno_Mapping::acquire        = s_acquire;
    uno_Mapping::release        = s_release;
    uno_Mapping::mapInterface   = s_mapInterface;
}


uno_Mapping * createIdentityMapping(uno::Environment const & rEnv) SAL_THROW(())
{
    return new IdentityMapping(rEnv);
}
