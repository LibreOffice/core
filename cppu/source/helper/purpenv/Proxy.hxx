/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Proxy.hxx,v $
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

#ifndef INCLUDED_Proxy_hxx
#define INCLUDED_Proxy_hxx

#include "osl/interlck.h"

#include "uno/environment.hxx"
#include "uno/mapping.hxx"
#include "uno/dispatcher.h"

#include "cppu/helper/purpenv/Mapping.hxx"


namespace cssu = com::sun::star::uno;


class SAL_DLLPRIVATE Proxy : public uno_Interface
{
    oslInterlockedCount                 m_nRef;

    cssu::Environment                   m_from;
    cssu::Environment                   m_to;

    cssu::Mapping                       m_from_to;
    cssu::Mapping                       m_to_from;

    // mapping information
    uno_Interface                    *  m_pUnoI; // wrapped interface
    typelib_InterfaceTypeDescription *  m_pTypeDescr;
    rtl::OUString                       m_aOId;

    cppu::helper::purpenv::ProbeFun   * m_probeFun;
    void                              * m_pProbeContext;

public:
    explicit Proxy(cssu::Mapping                    const & to_from,
                   uno_Environment                        * pTo,
                   uno_Environment                        * pFrom,
                   uno_Interface                          * pUnoI,
                   typelib_InterfaceTypeDescription       * pTypeDescr,
                   rtl::OUString                    const & rOId,
                   cppu::helper::purpenv::ProbeFun        * probeFun,
                   void                                   * pProbeContext)
        SAL_THROW( () );
    ~Proxy(void);

    void acquire(void);
    void release(void);

    void dispatch(
        typelib_TypeDescriptionReference  * pReturnTypeRef,
        typelib_MethodParameter           * pParams,
        sal_Int32                           nParams,
        typelib_TypeDescription     const * pMemberType,
        void                              * pReturn,
        void                              * pArgs[],
        uno_Any                          ** ppException );

};

extern "C" SAL_DLLPRIVATE void SAL_CALL Proxy_free(uno_ExtEnvironment * pEnv, void * pProxy) SAL_THROW_EXTERN_C();

#endif
