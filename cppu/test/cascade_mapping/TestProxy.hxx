/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestProxy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:46:38 $
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
