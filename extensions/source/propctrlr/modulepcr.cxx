/*************************************************************************
 *
 *  $RCSfile: modulepcr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 11:54:38 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_MODULEPCR_HXX_
#include "modulepcr.hxx"
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ensureImpl()

//.........................................................................
namespace pcr
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
        ResMgr* m_pRessources;

    public:
        /// ctor
        OModuleImpl();
        ~OModuleImpl();

        /// get the manager for the ressources of the module
        ResMgr* getResManager();
    };

    //-------------------------------------------------------------------------
    OModuleImpl::OModuleImpl()
        :m_pRessources(NULL)
    {
    }

    //-------------------------------------------------------------------------
    OModuleImpl::~OModuleImpl()
    {
        if (m_pRessources)
            delete m_pRessources;
    }

    //-------------------------------------------------------------------------
    ResMgr* OModuleImpl::getResManager()
    {
        // note that this method is not threadsafe, which counts for the whole class !

        if (!m_pRessources)
        {
            // create a manager with a fixed prefix
            ByteString aMgrName = ByteString( "pcr" );
            aMgrName += ByteString::CreateFromInt32(SOLARUPD); // current build number
            m_pRessources = ResMgr::CreateResMgr(aMgrName.GetBuffer());
        }
        return m_pRessources;
    }

    //=========================================================================
    //= OModule
    //=========================================================================
    ::osl::Mutex    OModule::s_aMutex;
    sal_Int32       OModule::s_nClients = 0;
    OModuleImpl*    OModule::s_pImpl = NULL;
    //-------------------------------------------------------------------------
    ResMgr* OModule::getResManager()
    {
        ENTER_MOD_METHOD();
        return s_pImpl->getResManager();
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
            OSL_ASSERT("OModule::revokeComponent : have no class infos ! Are you sure called this method at the right time ?");
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
    sal_Bool OModule::writeComponentInfos(
            const Reference< XMultiServiceFactory >& /*_rxServiceManager*/,
            const Reference< XRegistryKey >& _rxRootKey)
    {
        OSL_ENSURE(_rxRootKey.is(), "OModule::writeComponentInfos : invalid argument !");

        if (!s_pImplementationNames)
        {
            OSL_ASSERT("OModule::writeComponentInfos : have no class infos ! Are you sure called this method at the right time ?");
            return sal_True;
        }
        OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
            "OModule::writeComponentInfos : inconsistent state (the pointers) !");
        OSL_ENSURE( (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                    &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
            "OModule::writeComponentInfos : inconsistent state !");

        sal_Int32 nLen = s_pImplementationNames->getLength();
        const ::rtl::OUString* pImplName = s_pImplementationNames->getConstArray();
        const Sequence< ::rtl::OUString >* pServices = s_pSupportedServices->getConstArray();

        ::rtl::OUString sRootKey("/", 1, RTL_TEXTENCODING_ASCII_US);
        for (sal_Int32 i=0; i<nLen; ++i, ++pImplName, ++pServices)
        {
            ::rtl::OUString aMainKeyName(sRootKey);
            aMainKeyName += *pImplName;
            aMainKeyName += ::rtl::OUString::createFromAscii("/UNO/SERVICES");

            try
            {
                Reference< XRegistryKey >  xNewKey( _rxRootKey->createKey(aMainKeyName) );

                const ::rtl::OUString* pService = pServices->getConstArray();
                for (sal_Int32 j=0; j<pServices->getLength(); ++j, ++pService)
                    xNewKey->createKey(*pService);
            }
            catch(Exception&)
            {
                OSL_ASSERT("OModule::writeComponentInfos : something went wrong while creating the keys !");
                return sal_False;
            }
        }

        return sal_True;
    }

    //--------------------------------------------------------------------------
    Reference< XInterface > OModule::getComponentFactory(
        const ::rtl::OUString& _rImplementationName,
        const Reference< XMultiServiceFactory >& _rxServiceManager)
    {
        OSL_ENSURE(_rxServiceManager.is(), "OModule::getComponentFactory : invalid argument (service manager) !");
        OSL_ENSURE(_rImplementationName.getLength(), "OModule::getComponentFactory : invalid argument (implementation name) !");

        if (!s_pImplementationNames)
        {
            OSL_ASSERT("OModule::getComponentFactory : have no class infos ! Are you sure called this method at the right time ?");
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

                xReturn = FactoryInstantiationFunction( _rxServiceManager, *pImplName, ComponentInstantiationFunction, *pServices);
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
}   // namespace pcr
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/01/12 11:29:40  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 08.01.01 12:54:13  fs
 ************************************************************************/

