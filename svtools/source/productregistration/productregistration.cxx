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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "productregistration.hxx"
#include "unotools/regoptions.hxx"
#include "registrationdlg.hxx"
#include <svtools/svtools.hrc>
#include "cppuhelper/factory.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <functional>
#include <memory>

#define PRODREG_IMPLNAME "com.sun.star.comp.setup.ProductRegistration"
#define PRODREG_SERVNAME "com.sun.star.setup.ProductRegistration"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;

using rtl::OUString;

//........................................................................
namespace svt
{
//........................................................................

    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::system;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::awt;

    //-------------------------------------------------------------------

    struct EqualsOUString : public ::std::unary_function< OUString, sal_Bool >
    {
        const OUString& m_rCompare;
        EqualsOUString( const OUString& _rCompare ) : m_rCompare( _rCompare ) { }

        sal_Bool operator() ( const OUString& _rCompare )
        {
            return m_rCompare.equals( _rCompare );
        }
    };

    //====================================================================
    //= OProductRegistration
    //====================================================================

    //--------------------------------------------------------------------
    OProductRegistration::OProductRegistration( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
    {
    }

    //--------------------------------------------------------------------
    Reference< XInterface > OProductRegistration::Create( const Reference< XMultiServiceFactory >& _rxORB )
    {
        return static_cast< ::cppu::OWeakObject* >( new OProductRegistration( _rxORB ) );
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL OProductRegistration::getImplementationName_Static( )
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM( PRODREG_IMPLNAME ));
    }

    //--------------------------------------------------------------------
    Sequence< OUString > SAL_CALL OProductRegistration::getSupportedServiceNames_Static(  ) throw (RuntimeException)
    {
        Sequence< OUString > aServiceNames( 1 );
        aServiceNames[ 0 ] = OUString(RTL_CONSTASCII_USTRINGPARAM( PRODREG_SERVNAME ));
        return aServiceNames;
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL OProductRegistration::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_Static( );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OProductRegistration::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
    {
        Sequence< OUString > aServiceNames( getSupportedServiceNames( ) );
        const OUString* pNames = aServiceNames.getConstArray( );
        const OUString* pNamesEnd = aServiceNames.getConstArray( ) + aServiceNames.getLength();

        const OUString* pFound = ::std::find_if(
            pNames,
            pNamesEnd,
            EqualsOUString( _rServiceName )
        );
        return pFound != pNamesEnd;
    }

    //--------------------------------------------------------------------
    Sequence< OUString > SAL_CALL OProductRegistration::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_Static( );
    }

    //--------------------------------------------------------------------
    static  Reference< XFrame > lcl_getActiveFrame( const Reference< XMultiServiceFactory >& xFactory )
    {
        try
        {
            Reference< XDesktop > xDesktop(
                xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop") ) ),
                UNO_QUERY_THROW );

            Reference< XFrame > xFrame(xDesktop->getCurrentFrame());
            if( ! xFrame.is() )
            {
                // Perhaps the frames collection of the desktop knows about an "active frame"?
                Reference< XFramesSupplier > xFrames( xDesktop, UNO_QUERY_THROW );
                xFrame =  xFrames->getActiveFrame();
            }

            return xFrame;
        }
        catch(const Exception& )
        {
            OSL_ENSURE( sal_False, "lcl_getActiveFrame: caught an exception!" );
            return Reference< XFrame >();
        }
    }

    //--------------------------------------------------------------------
    static Window* lcl_getPreferredDialogParent( const Reference< XFrame >& xFrame )
    {
        Window* pReturn = Application::GetDefDialogParent();

        try
        {
            if ( xFrame.is() )
            {
                Reference< XWindow > xWindow = xFrame->getContainerWindow();
                if ( xWindow.is() )
                    pReturn = VCLUnoHelper::GetWindow( xWindow );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "lcl_getPreferredDialogParent: caught an exception!" );
        }

        return pReturn;
    }

    //--------------------------------------------------------------------
    static bool lcl_isEvalVersion( const Reference< XMultiServiceFactory >& _rxORB )
    {
        bool bIsEvaluationVersion = false;

        try
        {
            Reference < XMaterialHolder > xHolder(
                _rxORB->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.tab.tabreg" ) ) ),
                UNO_QUERY
            );

            if ( xHolder.is() )
            {
                Any aData = xHolder->getMaterial();
                Sequence < NamedValue > aSeq;

                if ( aData >>= aSeq )
                {
                    // it's an evaluation version - a non-eval version wouldn't provide this "material"
                    bIsEvaluationVersion = true;
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( false, "lcl_isEvalVersion: caught an exception!" );
        }

        return bIsEvaluationVersion;
    }

    //--------------------------------------------------------------------
    static bool lcl_doNewStyleRegistration( const Reference< XMultiServiceFactory >& xFactory, bool online )
    {
        try
        {
            Reference< XMultiServiceFactory > xConfigProvider(
                xFactory->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) )
                ),
                UNO_QUERY_THROW
            );

            PropertyValue aNodePath;
            aNodePath.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) );
            aNodePath.Value = makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Jobs/Events" ) ) );

            Sequence< Any > lArguments(1);
            lArguments[0] = makeAny( aNodePath );

            Reference< XHierarchicalNameAccess > xNameAccess(
                xConfigProvider->createInstanceWithArguments(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" ) ),
                    lArguments
                ),
                UNO_QUERY_THROW
            );

            if( ! xNameAccess->hasByHierarchicalName( OUString( RTL_CONSTASCII_USTRINGPARAM( "onRegisterNow/JobList" ) ) ) )
                return false;

            Reference< XJobExecutor > xJobExecutor(
                xFactory->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.task.JobExecutor"  ) )
                ),
                UNO_QUERY_THROW
            );

            xJobExecutor->trigger( online ? OUString( RTL_CONSTASCII_USTRINGPARAM( "onRegisterNow" ) ) :
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "onRegisterLater" ) ) );

            return true;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( false, "lcl_getOnlineRegistrationDispatch: caught an exception!" );
            return false;
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OProductRegistration::trigger( const OUString& _rEvent ) throw (RuntimeException)
    {
        bool registerOnline = false;

        switch ( classify( _rEvent ) )
        {
            case etRegistrationRequired:
                registerOnline = true;
                break;

            default:
                break;
        }

        // prefer new style registration
        if( ! lcl_doNewStyleRegistration(m_xORB, registerOnline ) && registerOnline )
            doOnlineRegistration();
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OProductRegistration::execute( const Sequence< NamedValue >& ) throw (IllegalArgumentException, Exception, RuntimeException)
    {
        Any aReturn;

        static sal_Bool bFirstEncounter( sal_True );
        if ( bFirstEncounter )
        {   // during this session, this event was never triggered before ....
            bFirstEncounter = sal_False;

            sal_Bool bDeactivateJob = sal_True;

            // our config options
            utl::RegOptions aRegOptions;
            // check them for the permissions for the dialog
            utl::RegOptions::DialogPermission ePermission( aRegOptions.getDialogPermission() );

            if ( utl::RegOptions::dpDisabled != ePermission )
            {   // the dialog is _not_ disabled

                // for this session, I'm no interested in the dialog registration anymore
                aRegOptions.markSessionDone( );

                if  (   ( utl::RegOptions::dpNotThisSession == ePermission )     // first trigger session not reached
                    ||  ( utl::RegOptions::dpRemindLater == ePermission )        // or at a later reminder date
                    )
                {   // the dialog should be executed during one of the next sessions
                    bDeactivateJob = sal_False;
                }
                else
                {
                    // if we're here, the dialog should be executed during this session
                    OSL_ENSURE( utl::RegOptions::dpThisSession == ePermission, "OProductRegistration::execute: invalid permissions!" );

                    {
                        // this is some kind of HACK.
                        // This registration dialog is intended to appear very very early during the
                        // first office start after installation. Unfortunately, this is so early
                        // that even SFX is not yet loaded, thus the SfxHelp class is not yet available,
                        // thus, there is no help during the lifetime of the dialog.
                        // To fake this, we explicitly load the necessary services when the user
                        // really requests help herein.
                        Reference < XInitialization > xOfficeWrapper(
                                m_xORB->createInstance(
                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.OfficeWrapper" ) )
                            ),
                            UNO_QUERY
                        );
                        if ( xOfficeWrapper.is() )
                            xOfficeWrapper->initialize( Sequence < Any >() );
                    }

                    std::auto_ptr<ResMgr> pResMgr (ResMgr::CreateResMgr (
                        CREATEVERSIONRESMGR_NAME(productregistration)));

                    Reference< XFrame > xFrame = lcl_getActiveFrame( m_xORB );
                    // execute it
                    RegistrationDialog aDialog (
                        lcl_getPreferredDialogParent( xFrame ),
                        ResId( DLG_REGISTRATION_REQUEST, *pResMgr.get() ),
                        lcl_isEvalVersion( m_xORB ) );
                    aDialog.Execute();

                    bool registerOnline = false;

                    switch ( aDialog.getResponse() )
                    {
                        case RegistrationDialog::urRegisterNow:
                            registerOnline = true;
                            break;

                        case RegistrationDialog::urRegisterLater:
                            bDeactivateJob = sal_False;
                            // remind again in seven days from now on ...
                            aRegOptions.activateReminder( 7 );
                            break;

                        case RegistrationDialog::urRegisterNever:
                        case RegistrationDialog::urAlreadyRegistered:
                            // never register or already registered
                            // -> deactivate the job, and nothing else
                            break;

                        default:
                            OSL_ENSURE( sal_False, "OProductRegistration::execute: invalid response from the dialog!" );
                    }

                    // prefer new style registration
                    if( ! lcl_doNewStyleRegistration(m_xORB, registerOnline) && registerOnline )
                        doOnlineRegistration();
                }
            }

            Sequence< NamedValue > aJobResponse( 1 );
            aJobResponse[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Deactivate" ));
            aJobResponse[0].Value <<= bDeactivateJob;
            aReturn <<= aJobResponse;
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void OProductRegistration::doOnlineRegistration( )
    {
        sal_Bool bSuccess = sal_False;
        try
        {
            // create the Desktop component which can load components
            Reference< XSystemShellExecute > xSystemShell(
                m_xORB->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SystemShellExecute" )) ),
                UNO_QUERY
            );
            OSL_ENSURE( xSystemShell.is(), "OProductRegistration::doOnlineRegistration: invalid SystemExecute component!" );

            // access the configuration to retrieve the URL we shall use for registration
            utl::RegOptions aOptions;
            OUString sRegistrationURL( aOptions.getRegistrationURL( ) );
            OSL_ENSURE( sRegistrationURL.getLength(), "OProductRegistration::doOnlineRegistration: invalid URL found!" );

            if ( xSystemShell.is() && sRegistrationURL.getLength() )
            {
                xSystemShell->execute( sRegistrationURL, OUString(), SystemShellExecuteFlags::DEFAULTS );
                bSuccess = sal_True;
            }
        }
        catch( const Exception& )
        {
        }
        if ( !bSuccess )
        {
            std::auto_ptr<ResMgr> pResMgr (ResMgr::CreateResMgr (
                CREATEVERSIONRESMGR_NAME(productregistration)));

            ErrorBox aRegistrationError(
                Application::GetDefDialogParent(),
                ResId( ERRBOX_REG_NOSYSBROWSER, *pResMgr.get() ));
            aRegistrationError.Execute();

            // try again later
            utl::RegOptions aRegOptions;
            aRegOptions.activateReminder( 7 );
        }
    }

    //--------------------------------------------------------------------
    OProductRegistration::EventType OProductRegistration::classify( const OUString& _rEventDesc )
    {
        EventType eReturn = etUnknown;
        if ( _rEventDesc.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("RegistrationRequired")) )
        {
            eReturn = etRegistrationRequired;
        }
        return eReturn;
    }

//........................................................................
}   // namespace svt
//........................................................................

extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * /* pServiceManager */, void * pRegistryKey)
{
    if (pRegistryKey)
    {
        Reference< XRegistryKey > xRegistryKey (
            reinterpret_cast< XRegistryKey* >( pRegistryKey ));
        Reference< XRegistryKey > xNewKey;

        xNewKey = xRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "/" PRODREG_IMPLNAME "/UNO/SERVICES" )));
        xNewKey->createKey(
            OUString(RTL_CONSTASCII_USTRINGPARAM( PRODREG_SERVNAME )));

        return sal_True;
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * /* pRegistryKey */)
{
    void * pResult = 0;
    if (pServiceManager)
    {
        Reference< XSingleServiceFactory > xFactory;
        if (svt::OProductRegistration::getImplementationName_Static().compareToAscii (pImplementationName) == 0)
        {
            xFactory = cppu::createSingleFactory (
                reinterpret_cast< XMultiServiceFactory* >(pServiceManager),
                svt::OProductRegistration::getImplementationName_Static(),
                svt::OProductRegistration::Create,
                svt::OProductRegistration::getSupportedServiceNames_Static());
        }
        if (xFactory.is())
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
