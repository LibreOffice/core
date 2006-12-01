/*************************************************************************
 *
 *  $RCSfile: IdentityMapping.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:25:52 $
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
