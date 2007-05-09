/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helper_purpenv_Mapping.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:37:17 $
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

#include "cppu/helper/purpenv/Mapping.hxx"

#include "Proxy.hxx"

#include "osl/interlck.h"
#include "uno/environment.hxx"
#include "uno/dispatcher.h"
#include "typelib/typedescription.h"


#ifdef debug
# define LOG_LIFECYCLE_cppu_helper_purpenv_Mapping
#endif

#ifdef LOG_LIFECYCLE_cppu_helper_purpenv_Mapping
#  include <iostream>
#  define LOG_LIFECYCLE_cppu_helper_purpenv_Mapping_emit(x) x

#else
#  define LOG_LIFECYCLE_cppu_helper_purpenv_Mapping_emit(x)

#endif


using namespace com::sun::star;


class Mapping : public uno_Mapping
{
    uno::Environment   m_from;
    uno::Environment   m_to;

    oslInterlockedCount m_nCount;

    cppu::helper::purpenv::ProbeFun * m_probeFun;
    void                            * m_pContext;

public:
    explicit  Mapping(uno_Environment                 * pFrom,
                      uno_Environment                 * pTo,
                      cppu::helper::purpenv::ProbeFun * probeFun,
                      void                            * pProbeContext);
    virtual  ~Mapping(void);

    void mapInterface(
        uno_Interface                    ** ppOut,
        uno_Interface                     * pUnoI,
        typelib_InterfaceTypeDescription  * pTypeDescr);

    void acquire(void);
    void release(void);
};

static void SAL_CALL s_mapInterface(
    uno_Mapping                       * puno_Mapping,
    uno_Interface                    ** ppOut,
    uno_Interface                     * pUnoI,
    typelib_InterfaceTypeDescription  * pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping *>(puno_Mapping);
    pMapping->mapInterface(ppOut, pUnoI, pTypeDescr);
}

extern "C" {
static void SAL_CALL s_acquire(uno_Mapping * puno_Mapping)
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping *>(puno_Mapping);
    pMapping->acquire();
}

static void SAL_CALL s_release(uno_Mapping * puno_Mapping)
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping * >(puno_Mapping);
    pMapping->release();
}


static void s_getIdentifier_v(va_list param)
{
    uno_ExtEnvironment *  pEnv  = va_arg(param, uno_ExtEnvironment *);
    rtl_uString        ** ppOid = va_arg(param, rtl_uString **);
    uno_Interface      *  pUnoI = va_arg(param, uno_Interface *);

    pEnv->getObjectIdentifier(pEnv, ppOid, pUnoI);
}

static void SAL_CALL s_free(uno_Mapping * puno_Mapping)
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping *>(puno_Mapping);
    delete pMapping;
}
}

Mapping::Mapping(uno_Environment                 * pFrom,
                 uno_Environment                 * pTo,
                 cppu::helper::purpenv::ProbeFun * probeFun,
                 void                            * pProbeContext
) SAL_THROW( () )
    : m_from    (pFrom),
      m_to      (pTo),
      m_nCount  (1),
      m_probeFun(probeFun),
      m_pContext(pProbeContext)
{
    LOG_LIFECYCLE_cppu_helper_purpenv_Mapping_emit(fprintf(stderr, "LIFE: %s -> %p\n", "Mapping::Mapping(uno_Environment * pFrom, uno_Environment * pTo) SAL_THROW( () )", this));

    uno_Mapping::acquire      = s_acquire;
    uno_Mapping::release      = s_release;
    uno_Mapping::mapInterface = (uno_MapInterfaceFunc)s_mapInterface;
}

Mapping::~Mapping(void)
{
    LOG_LIFECYCLE_cppu_helper_purpenv_Mapping_emit(fprintf(stderr, "LIFE: %s -> %p\n", "Mapping::~Mapping(void)", this));
}


void Mapping::mapInterface(
    uno_Interface                    ** ppOut,
    uno_Interface                     * pUnoI,
    typelib_InterfaceTypeDescription  * pTypeDescr)
{
    OSL_ASSERT(ppOut && pTypeDescr);
    if (*ppOut)
    {
        (*ppOut)->release(*ppOut);
        *ppOut = 0;
    }

    if (!pUnoI)
        return;

    // get object id of uno interface to be wrapped
    // need to enter environment because of potential "queryInterface" call
    rtl_uString * pOId = 0;
    uno_Environment_invoke(m_from.get(), s_getIdentifier_v, m_from.get(), &pOId, pUnoI);
    OSL_ASSERT(pOId);

     // try to get any known interface from target environment
    m_to.get()->pExtEnv->getRegisteredInterface(m_to.get()->pExtEnv, (void **)ppOut, pOId, pTypeDescr);

    if (!*ppOut) // not yet there, register new proxy interface
    {
        // try to publish a new proxy (ref count initially 1)
        uno_Interface * pProxy = new Proxy(this,
                                           m_from.get(),
                                           m_to.get(),
                                           pUnoI,
                                           pTypeDescr,
                                           pOId,
                                           m_probeFun,
                                           m_pContext);

        // proxy may be exchanged during registration
        m_to.get()->pExtEnv->registerProxyInterface(m_to.get()->pExtEnv,
                                                    (void **)&pProxy,
                                                    Proxy_free,
                                                    pOId,
                                                    pTypeDescr);

        *ppOut = pProxy;
    }

    rtl_uString_release(pOId);
}


void Mapping::acquire() SAL_THROW(())
{
    if (osl_incrementInterlockedCount(&m_nCount) == 1)
    {
        uno_Mapping * pMapping = this;

        ::uno_registerMapping(&pMapping, s_free, m_from.get(), m_to.get(), NULL);
    }
}

void Mapping::release() SAL_THROW(())
{
    if (osl_decrementInterlockedCount(&m_nCount) == 0)
        ::uno_revokeMapping(this);
}


namespace cppu { namespace helper { namespace purpenv {

void createMapping(uno_Mapping     ** ppMapping,
                   uno_Environment  * pFrom,
                   uno_Environment  * pTo,
                   ProbeFun         * probeFun,
                   void             * pContext
 )
{
    *ppMapping = new Mapping(pFrom, pTo, probeFun, pContext);

    ::uno_registerMapping(ppMapping, s_free, pFrom, pTo, NULL);
}

}}}
