/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "componentmodule.hxx"
#include <tools/resmgr.hxx>
#include <svl/solar.hrc>
#include <comphelper/sequence.hxx>
#include <tools/debug.hxx>
#include <rtl/strbuf.hxx>

#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ensureImpl()

//.........................................................................
namespace COMPMOD_NAMESPACE
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::registry;
    using namespace ::comphelper;
    using namespace ::cppu;

    //=========================================================================
    //= OModuleImpl
    //=========================================================================
    /** implementation for <type>OModule</type>. not threadsafe, has to be guarded by it's owner
    */
    class OModuleImpl
    {
        ResMgr*     m_pResources;
        sal_Bool    m_bInitialized;
        rtl::OString m_sFilePrefix;

    public:
        /// ctor
        OModuleImpl();
        ~OModuleImpl();

        /// get the manager for the resources of the module
        ResMgr* getResManager();
        void    setResourceFilePrefix(const ::rtl::OString& _rPrefix) { m_sFilePrefix = _rPrefix; }
    };

    //-------------------------------------------------------------------------
    OModuleImpl::OModuleImpl()
        :m_pResources(NULL)
        ,m_bInitialized(sal_False)
    {
    }

    //-------------------------------------------------------------------------
    OModuleImpl::~OModuleImpl()
    {
        if (m_pResources)
            delete m_pResources;
    }

    //-------------------------------------------------------------------------
    ResMgr* OModuleImpl::getResManager()
    {
        // note that this method is not threadsafe, which counts for the whole class !
        if (!m_pResources && !m_bInitialized)
        {
            DBG_ASSERT(!m_sFilePrefix.isEmpty(), "OModuleImpl::getResManager: no resource file prefix!");
            // create a manager with a fixed prefix
            m_pResources = ResMgr::CreateResMgr(m_sFilePrefix.getStr());
            DBG_ASSERT(m_pResources,
                    rtl::OStringBuffer("OModuleImpl::getResManager: could not create the resource manager (file name: ")
                .append(m_sFilePrefix)
                .append(")!").getStr());

            m_bInitialized = sal_True;
        }
        return m_pResources;
    }

    //=========================================================================
    //= OModule
    //=========================================================================
    ::osl::Mutex    OModule::s_aMutex;
    sal_Int32       OModule::s_nClients = 0;
    OModuleImpl*    OModule::s_pImpl = NULL;
    ::rtl::OString  OModule::s_sResPrefix;
    //-------------------------------------------------------------------------
    ResMgr* OModule::getResManager()
    {
        ENTER_MOD_METHOD();
        return s_pImpl->getResManager();
    }

    //-------------------------------------------------------------------------
    void OModule::setResourceFilePrefix(const ::rtl::OString& _rPrefix)
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        s_sResPrefix = _rPrefix;
        if (s_pImpl)
            s_pImpl->setResourceFilePrefix(_rPrefix);
    }

    //-------------------------------------------------------------------------
    void OModule::registerClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        ++s_nClients;
    }

    //-------------------------------------------------------------------------
    void OModule::revokeClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (!--s_nClients && s_pImpl)
        {
            delete s_pImpl;
            s_pImpl = NULL;
        }
    }

    //-------------------------------------------------------------------------
    void OModule::ensureImpl()
    {
        if (s_pImpl)
            return;
        s_pImpl = new OModuleImpl();
        s_pImpl->setResourceFilePrefix(s_sResPrefix);
    }

    //--------------------------------------------------------------------------
    //- registration helper
    //--------------------------------------------------------------------------

    Sequence< ::rtl::OUString >*                OModule::s_pImplementationNames = NULL;
    Sequence< Sequence< ::rtl::OUString > >*    OModule::s_pSupportedServices = NULL;
    Sequence< sal_Int64 >*                      OModule::s_pCreationFunctionPointers = NULL;
    Sequence< sal_Int64 >*                      OModule::s_pFactoryFunctionPointers = NULL;

    //--------------------------------------------------------------------------
    void OModule::registerComponent(
        const ::rtl::OUString& _rImplementationName,
        const Sequence< ::rtl::OUString >& _rServiceNames,
        ComponentInstantiation _pCreateFunction,
        FactoryInstantiation _pFactoryFunction)
    {
        if (!s_pImplementationNames)
        {
            OSL_ENSURE(!s_pSupportedServices && !s_pCreationFunctionPointers && !s_pFactoryFunctionPointers,
                "OModule::registerComponent : inconsistent state (the pointers (1)) !");
            s_pImplementationNames = new Sequence< ::rtl::OUString >;
            s_pSupportedServices = new Sequence< Sequence< ::rtl::OUString > >;
            s_pCreationFunctionPointers = new Sequence< sal_Int64 >;
            s_pFactoryFunctionPointers = new Sequence< sal_Int64 >;
        }
        OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
            "OModule::registerComponent : inconsistent state (the pointers (2)) !");

        OSL_ENSURE( (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
            "OModule::registerComponent : inconsistent state !");

        sal_Int32 nOldLen = s_pImplementationNames->getLength();
        s_pImplementationNames->realloc(nOldLen + 1);
        s_pSupportedServices->realloc(nOldLen + 1);
        s_pCreationFunctionPointers->realloc(nOldLen + 1);
        s_pFactoryFunctionPointers->realloc(nOldLen + 1);

        s_pImplementationNames->getArray()[nOldLen] = _rImplementationName;
        s_pSupportedServices->getArray()[nOldLen] = _rServiceNames;
        s_pCreationFunctionPointers->getArray()[nOldLen] = reinterpret_cast<sal_Int64>(_pCreateFunction);
        s_pFactoryFunctionPointers->getArray()[nOldLen] = reinterpret_cast<sal_Int64>(_pFactoryFunction);
    }

    //--------------------------------------------------------------------------
    void OModule::revokeComponent(const ::rtl::OUString& _rImplementationName)
    {
        if (!s_pImplementationNames)
        {
            OSL_FAIL("OModule::revokeComponent : have no class infos ! Are you sure called this method at the right time ?");
            return;
        }
        OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
            "OModule::revokeComponent : inconsistent state (the pointers) !");
        OSL_ENSURE( (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
            "OModule::revokeComponent : inconsistent state !");

        sal_Int32 nLen = s_pImplementationNames->getLength();
        const ::rtl::OUString* pImplNames = s_pImplementationNames->getConstArray();
        for (sal_Int32 i=0; i<nLen; ++i, ++pImplNames)
        {
            if (pImplNames->equals(_rImplementationName))
            {
                removeElementAt(*s_pImplementationNames, i);
                removeElementAt(*s_pSupportedServices, i);
                removeElementAt(*s_pCreationFunctionPointers, i);
                removeElementAt(*s_pFactoryFunctionPointers, i);
                break;
            }
        }

        if (s_pImplementationNames->getLength() == 0)
        {
            delete s_pImplementationNames; s_pImplementationNames = NULL;
            delete s_pSupportedServices; s_pSupportedServices = NULL;
            delete s_pCreationFunctionPointers; s_pCreationFunctionPointers = NULL;
            delete s_pFactoryFunctionPointers; s_pFactoryFunctionPointers = NULL;
        }
    }

    //--------------------------------------------------------------------------
    Reference< XInterface > OModule::getComponentFactory(
        const ::rtl::OUString& _rImplementationName,
        const Reference< XMultiServiceFactory >& _rxServiceManager)
    {
        OSL_ENSURE(_rxServiceManager.is(), "OModule::getComponentFactory : invalid argument (service manager) !");
        OSL_ENSURE(!_rImplementationName.isEmpty(), "OModule::getComponentFactory : invalid argument (implementation name) !");

        if (!s_pImplementationNames)
        {
            OSL_FAIL("OModule::getComponentFactory : have no class infos ! Are you sure called this method at the right time ?");
            return NULL;
        }
        OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
            "OModule::getComponentFactory : inconsistent state (the pointers) !");
        OSL_ENSURE( (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
            "OModule::getComponentFactory : inconsistent state !");


        Reference< XInterface > xReturn;


        sal_Int32 nLen = s_pImplementationNames->getLength();
        const ::rtl::OUString* pImplName = s_pImplementationNames->getConstArray();
        const Sequence< ::rtl::OUString >* pServices = s_pSupportedServices->getConstArray();
        const sal_Int64* pComponentFunction = s_pCreationFunctionPointers->getConstArray();
        const sal_Int64* pFactoryFunction = s_pFactoryFunctionPointers->getConstArray();

        for (sal_Int32 i=0; i<nLen; ++i, ++pImplName, ++pServices, ++pComponentFunction, ++pFactoryFunction)
        {
            if (pImplName->equals(_rImplementationName))
            {
                const FactoryInstantiation FactoryInstantiationFunction = reinterpret_cast<const FactoryInstantiation>(*pFactoryFunction);
                const ComponentInstantiation ComponentInstantiationFunction = reinterpret_cast<const ComponentInstantiation>(*pComponentFunction);

                xReturn = FactoryInstantiationFunction( _rxServiceManager, *pImplName, ComponentInstantiationFunction, *pServices, NULL);
                if (xReturn.is())
                {
                    xReturn->acquire();
                    return xReturn.get();
                }
            }
        }

        return NULL;
    }


//.........................................................................
}   // namespace COMPMOD_NAMESPACE
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
