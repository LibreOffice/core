/*************************************************************************
 *
 *  $RCSfile: providerfactory.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: lla $ $Date: 2001-08-01 12:16:00 $
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

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

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
                RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "Configmgr::ODisposingListener", "jb99855", "disposing()");
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
    void OProviderFactory::ensureBootstrapSettings()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::OProviderFactory", "jb99855", "ensureBootstrapSettings()");
        if (!m_pPureSettings)
            m_pPureSettings = new BootstrapSettings();
    }

    //---------------------------------------------------------------------------------------
    static bool isReusableConnection(const ConnectionSettings& _rSettings)
    {
        // #78409
        // if a provider is queried with a password, we always create a new instance for it,
        // as don't wan't to remember the passwords for the user.

        if ( _rSettings.hasPassword() && !_rSettings.isLocalSession())
            return false;

        if (_rSettings.hasReinitializeFlag() && _rSettings.getReinitializeFlag())
            return false;

        return  true;
    }

    //---------------------------------------------------------------------------------------
    extern OUString buildConnectString(const ConnectionSettings& _rSettings);

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::implGetProvider(const ConnectionSettings& _rSettings)
    {
        OUString const sConnectString = buildConnectString(_rSettings);

        Reference< XInterface > xReturn;

        ProviderCacheIterator aExistentProviderPos = m_aProviders.find(sConnectString);
        if (m_aProviders.end() != aExistentProviderPos)
        {
            xReturn = aExistentProviderPos->second;
        }

        if (!xReturn.is())
        {
            // create and connect the provider (may still throw exceptions)
            xReturn = (*m_pObjectCreator)(m_xORB, _rSettings);

            // check for success
            if (!xReturn.is())
            {
                OSL_ENSURE(false, "Object creator could not create provider, but returned NULL instead of throwing an exception");
                sal_Char const sCannotCreate[] = "Cannot create ConfigurationProvider. Unknown backend or factory error.";
                // should become CannotLoadConfigurationException
                throw uno::Exception( OUString::createFromAscii(sCannotCreate), *this );
            }

            // remember it for later usage
            if (isReusableConnection(_rSettings))
                m_aProviders[sConnectString] = xReturn;
        }

        return xReturn;
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::implCreateProviderWithSettings(const ConnectionSettings& _rSettings, bool _bRequiresBootstrap)
    {
        try
        {
            return implGetProvider(_rSettings);
        }
        catch(uno::Exception& e)
        {
            if (_bRequiresBootstrap)
            {
                OSL_ASSERT(m_pPureSettings);
                raiseBootstrapException(*m_pPureSettings, *this);

                OSL_ASSERT(m_pPureSettings->status == BOOTSTRAP_DATA_OK);
            }

            sal_Char const sConnectionFailure[] = "Cannot open Configuration: ";
            OUString const sFailure = OUString::createFromAscii(sConnectionFailure);
            e.Message = sFailure.concat(e.Message);
            throw;
        }
    }

    //---------------------------------------------------------------------------------------
    void OProviderFactory::ensureDefaultProvider()
    {
        if (m_xDefaultProvider.is())
            return;

        ensureBootstrapSettings();

        ConnectionSettings aThisRoundSettings(m_pPureSettings->settings);

        aThisRoundSettings.validate();
        OSL_ENSURE(aThisRoundSettings.isComplete(), "Incomplete Data for creating a ConfigurationProvider");

        m_xDefaultProvider = implCreateProviderWithSettings(aThisRoundSettings,true);

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
        RTL_LOGFILE_CONTEXT( aLog, "Configmgr::OProviderFactory::createProvider()" );

        ensureDefaultProvider();
        return m_xDefaultProvider;
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::createProviderWithArguments(const Sequence< Any >& _rArguments)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::OProviderFactory", "jb99855", "createProviderWithArguments()");
        ConnectionSettings aSettings(_rArguments);
        return createProviderWithSettings( aSettings );
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::createProviderWithSettings(const ConnectionSettings& _rSettings)
    {
        MutexGuard aGuard(m_aMutex);

        ConnectionSettings aThisRoundSettings(_rSettings);

        // use bootstrap data if necessary
        bool bUseBootstrapData = !aThisRoundSettings.isComplete();

        // detect a plugin session. Can be specified only as argument
        sal_Bool bIsPluginSession = aThisRoundSettings.isPlugin();

        if (bIsPluginSession)
        {
            OSL_ENSURE(!aThisRoundSettings.isSourcePathValid(),"Invalid Argument: No explicit source path should be specified for plugin session");
            bUseBootstrapData = true;
        }

        // use bootstrap data if necessary
        if (bUseBootstrapData)
        {
            ensureBootstrapSettings();

            ConnectionSettings aMergedSettings = m_pPureSettings->settings;
            aMergedSettings.mergeOverrides(aThisRoundSettings);
            aMergedSettings.swap(aThisRoundSettings);
        }

        // if we have a plugin session, translate the session type into the one appliable.
        if (bIsPluginSession)
        {
            // try to create (or share) a local-session provider

            // For a plugin-local session, we need a valid update directory.
            // (We can't just rely on the session to fail if it is created with a valid source directory and an
            // invalid update directory. In such a scenario it will succeed to open, but not to update.)
            if (!m_pPureSettings->settings.isLocalSession())
            {
                const OUString sLocalSessionIdentifier = OUString::createFromAscii(LOCAL_SESSION_IDENTIFIER);

                // (We can't just rely on the session to fail if it is created with a valid source directory and an
                // invalid update directory. In such a scenario it will succeed to open, but not to update.)
                if (aThisRoundSettings.isComplete(sLocalSessionIdentifier) &&
                    aThisRoundSettings.isUpdatePathValid())
                try
                {
                    aThisRoundSettings.setSessionType(sLocalSessionIdentifier, Settings::SO_ADJUSTMENT);

                    Reference< XInterface > xLocalProvider
                        = implGetProvider(aThisRoundSettings);

                    if (xLocalProvider.is()) return xLocalProvider;
                }
                catch(Exception&)
                {
                    // allowed. The creation of the local provider may fail.
                }
            }
            // did not create the local session

            // -> create the original one
            if (m_pPureSettings->settings.isSessionTypeKnown())
            {
                OUString sOriginalType = m_pPureSettings->settings.getSessionType();
                aThisRoundSettings.setSessionType(sOriginalType, Settings::SO_ADJUSTMENT);
            }
            else
            {
                OUString const sPortalSessionIdentifier = OUString::createFromAscii(PORTAL_SESSION_IDENTIFIER);
                aThisRoundSettings.setSessionType(sPortalSessionIdentifier, Settings::SO_ADJUSTMENT);
            }
        }

        aThisRoundSettings.validate();
        OSL_ENSURE(aThisRoundSettings.isComplete(), "Incomplete Data for creating a ConfigurationProvider");

        Reference< XInterface > xProvider =
            implCreateProviderWithSettings( aThisRoundSettings,bUseBootstrapData);

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
 *  Revision 1.11  2001/06/22 08:26:18  jb
 *  Correct argument-dependent caching of providers
 *
 *  Revision 1.10  2001/05/22 07:42:07  jb
 *  #81412# Erroneous handling of default provider
 *
 *  Revision 1.9  2001/05/18 16:16:52  jb
 *  #81412# Cleaned up bootstrap settings handling; Added recognition of bootstrap errors
 *
 *  Revision 1.8  2001/04/03 16:33:58  jb
 *  Local AdministrationProvider now mapped to Setup-session
 *
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

