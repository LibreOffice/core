/*************************************************************************
 *
 *  $RCSfile: providerfactory.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-03 16:33:58 $
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
#include <stdio.h>
#ifndef _CONFIGMGR_PROVIDER_FACTORY_HXX_
#include "providerfactory.hxx"
#endif

#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#include "confsvccomponent.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif

// #include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#define THISREF() static_cast< ::cppu::OWeakObject* >(this)


//........................................................................
namespace configmgr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::cppu;
    using namespace ::osl;


    //=======================================================================================
    //= OProviderFactory
    //=======================================================================================
    typedef ::cppu::WeakImplHelper1<XEventListener> XEventListener_BASE;
    class ODisposingListener : public XEventListener_BASE
    {
        OProviderFactory& m_aFactory;
    public:
        ODisposingListener(OProviderFactory& _aFactory):
            m_aFactory(_aFactory){}

        virtual void SAL_CALL disposing(com::sun::star::lang::EventObject const& rEvt) throw()
            {
                m_aFactory.disposing(rEvt);
            }
        ~ODisposingListener()
            {
                volatile int dummy = 0;
            }

    };
    //---------------------------------------------------------------------------------------

    //=======================================================================================
    //= OProviderFactory
    //=======================================================================================
    //---------------------------------------------------------------------------------------
    OProviderFactory::OProviderFactory(const Reference< XMultiServiceFactory >& _rxORB, ProviderInstantiation _pObjectCreator)
        :m_pObjectCreator(_pObjectCreator)
        ,m_xORB(_rxORB)
        ,m_pPureSettings(NULL)
    {
        ODisposingListener *pListener = new ODisposingListener(*this);
        m_xEventListener = pListener;

        Reference<com::sun::star::lang::XComponent> xComponent(_rxORB, UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->addEventListener(m_xEventListener);
            // CFG_TRACE_INFO("insert disposeListener.");
        }
    }

//---------------------------------------------------------------------------------------
    OProviderFactory::~OProviderFactory()
    {
        delete m_pPureSettings;

        Reference<com::sun::star::lang::XComponent> xComponent(m_xORB, UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->removeEventListener(m_xEventListener);
        }
        xComponent = xComponent.query(m_xDefaultProvider);
        if (xComponent.is())
        {
            xComponent->removeEventListener(m_xEventListener);
        }
    }

    //---------------------------------------------------------------------------------------
    void OProviderFactory::ensureSettings()
    {
        if (!m_pPureSettings)
            m_pPureSettings = new ConnectionSettings;
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::implGetProvider(const ConnectionSettings& _rSettings, sal_Bool _bCreateWithPassword)
    {
        // the providers for the given session type
        UserSpecificProviders& rProviders = m_aProviders[_rSettings.getSessionType()];

        // the user to retrieve the provider for
        ::rtl::OUString sUser;
        if (_rSettings.hasUser())
            sUser = _rSettings.getUser();

        Reference< XInterface > xReturn;
        if (!_bCreateWithPassword)
        {
            UserSpecificProvidersIterator aExistentProvider = rProviders.find(sUser);
            if (rProviders.end() != aExistentProvider)
                xReturn = aExistentProvider->second;
        }

        // #78409
        // if a provider is queried with a password, we always create a new instance for it,
        // as don't wan't to remember the passwords for the user.

        if (!xReturn.is())
        {
            // create and connect the provider (may still throw exceptions)
            xReturn = (*m_pObjectCreator)(m_xORB, _rSettings);
            // remember it for later usage
            if (!_bCreateWithPassword)
                rProviders[sUser] = xReturn;
        }

        return xReturn;
    }

    //---------------------------------------------------------------------------------------
    void OProviderFactory::ensureDefaultProvider()
    {
        MutexGuard aGuard(m_aMutex);
        if (m_xDefaultProvider.is())
            return;
        ensureSettings();

        m_xDefaultProvider = createProviderWithArguments(Sequence< Any >());

        // register disposing listener
        Reference<com::sun::star::lang::XComponent> xComponent(m_xDefaultProvider, UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->addEventListener(m_xEventListener);
        }
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::createProvider()
    {
        MutexGuard aGuard(m_aMutex);
        ensureDefaultProvider();
        return m_xDefaultProvider;
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::createProviderWithArguments(const Sequence< Any >& _rArguments)
    {
        MutexGuard aGuard(m_aMutex);

        ensureSettings();
        ConnectionSettings aThisRoundSettings = m_pPureSettings->createMergedSettings(_rArguments);

        ::rtl::OUString sSessionType = aThisRoundSettings.getSessionType();
        sal_Bool bIsPluginSession = (0 == sSessionType.compareToAscii(PLUGIN_SESSION_IDENTIFIER));

        Reference< XInterface > xProvider;

        // if we have a plugin session, translate the session type into the one appliable.
        if (bIsPluginSession)
        {
            // try to create (or share) a local-session provider

            // For a plugin-local session, we need a valid update directory.
            // (We can't just rely on the session to fail if it is created with a valid source directory and an
            // invalid update directory. In such a scenario it will succeed to open, but not to update.)
            if (aThisRoundSettings.hasRegistry() && aThisRoundSettings.isValidUpdatePath())
            {
                const ::rtl::OUString sLocalSessionIdentifier = ::rtl::OUString::createFromAscii(LOCAL_SESSION_IDENTIFIER);
                aThisRoundSettings.setSessionType(sLocalSessionIdentifier);

                try
                {
                    xProvider = implGetProvider(
                        aThisRoundSettings,
                        sal_False       // no password for local sessions
                        );
                }
                catch(Exception&)
                {
                    // allowed. The creation of the local provider may fail.
                }
            }

            if (!xProvider.is())
            {   // failed to create the local session
                // -> create a portal one
                const ::rtl::OUString sPortalSessionIdentifier = ::rtl::OUString::createFromAscii(PORTAL_SESSION_IDENTIFIER);
                aThisRoundSettings.setSessionType(sPortalSessionIdentifier);
                // the real creation is below ...
                sSessionType = sPortalSessionIdentifier;
            }
        }

        if (!xProvider.is())
            xProvider = implGetProvider(aThisRoundSettings, aThisRoundSettings.hasPassword());
        return xProvider;
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OProviderFactory::createInstance(  ) throw(Exception, RuntimeException)
    {
        // default provider
        return createProvider();
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OProviderFactory::createInstanceWithArguments( const Sequence< Any >& _rArguments ) throw(Exception, RuntimeException)
    {
        return createProviderWithArguments(_rArguments);
    }

    //=======================================================================================
    Reference< XSingleServiceFactory > SAL_CALL createProviderFactory(
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ProviderInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames
        )
    {
        return new OProviderFactory(rServiceManager, pCreateFunction);
    }

    void OProviderFactory::disposing(com::sun::star::lang::EventObject const& _rEvt) throw()
    {
        MutexGuard aGuard(m_aMutex);
        if (_rEvt.Source == m_xORB)
        {
            Reference<com::sun::star::lang::XComponent> xComponent(m_xDefaultProvider, UNO_QUERY);
            if (xComponent.is())
            {
                xComponent->removeEventListener(m_xEventListener);
            }
            m_xORB = NULL;
            m_xDefaultProvider = NULL;
        }
        else if (_rEvt.Source == m_xDefaultProvider)
        {
            m_xDefaultProvider = NULL;
        }
        else
            OSL_ENSURE(sal_False, "unknown object disposed.");
    }

//........................................................................
}   // namespace configmgr
//........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.7  2001/03/21 12:15:40  jl
 *  OSL_ENSHURE replaced by OSL_ENSURE
 *
 *  Revision 1.6  2001/01/26 07:54:21  lla
 *  #82734# disposing with lasy writing necessary
 *
 *  Revision 1.5  2000/12/07 16:46:12  dg
 *  #81469# incomplete adjustments to portal environment fixed
 *
 *  Revision 1.4  2000/12/07 13:56:21  tlx
 *  #81469#
 *
 *  Revision 1.3  2000/12/03 17:12:52  dg
 *  #81164# stdio missing
 *
 *  Revision 1.2  2000/12/03 11:52:13  dg
 *  #81164# invalid provider instantiation
 *
 *  Revision 1.1  2000/12/01 13:53:17  fs
 *  initial checkin - afctory for configuration provider(s)
 *
 *
 *  Revision 1.0 30.11.00 19:05:35  fs
 ************************************************************************/

