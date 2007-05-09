/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Proxy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:36:47 $
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
