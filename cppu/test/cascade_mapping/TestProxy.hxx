/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestProxy.hxx,v $
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

#ifndef INCLUDED_TestProxy_hxx
#define INCLUDED_TestProxy_hxx

#include "osl/interlck.h"
#include "uno/dispatcher.h"
#include "uno/environment.hxx"


extern "C" void SAL_CALL TestProxy_free(uno_ExtEnvironment * pEnv, void * pObject) SAL_THROW_EXTERN_C();


class SAL_DLLPRIVATE TestProxy : public uno_Interface
{
private:
    uno_Interface                    * m_theObject;
    uno_ExtEnvironment               * m_pFrom_extEnv;
    uno_ExtEnvironment               * m_pTo_extEnv;
    oslInterlockedCount                m_nCount;
    rtl::OUString                      m_from_envDcp;
    rtl::OUString                      m_to_envDcp;
    rtl::OUString                      m_oid;
    typelib_InterfaceTypeDescription * m_pTypeDescr;

public:
    explicit TestProxy(uno_Interface                          * pObject,
                       rtl::OUString                    const & oid,
                       typelib_InterfaceTypeDescription       * pTypeDescr,
                       uno_ExtEnvironment                     * pFrom_env,
                       uno_ExtEnvironment                     * pExtEnvironment);
    ~TestProxy(void);


    void acquire() SAL_THROW(());
    void release() SAL_THROW(());

    void dispatch(typelib_TypeDescription const  * pMemberType,
                  void                           * pReturn,
                  void                           * pArgs[],
                  uno_Any                       ** ppException);
};


#endif
