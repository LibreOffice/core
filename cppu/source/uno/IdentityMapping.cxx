/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IdentityMapping.cxx,v $
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
