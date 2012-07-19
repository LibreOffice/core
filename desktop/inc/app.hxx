/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _DESKTOP_APP_HXX_
#define _DESKTOP_APP_HXX_

// stl includes first
#include <map>
#include <boost/scoped_ptr.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <tools/resmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Reference.h>
#include <osl/mutex.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace desktop
{

/*--------------------------------------------------------------------
    Description:    Application-class
 --------------------------------------------------------------------*/
class CommandLineArgs;
class Lockfile;
class AcceptorMap : public std::map< OUString, css::uno::Reference<css::lang::XInitialization> > {};
struct ConvertData;
class Desktop : public Application
{
    friend class UserInstall;

    int doShutdown();

    public:
        enum BootstrapError
        {
            BE_OK,
            BE_UNO_SERVICEMANAGER,
            BE_UNO_SERVICE_CONFIG_MISSING,
            BE_PATHINFO_MISSING,
            BE_USERINSTALL_FAILED,
            BE_LANGUAGE_MISSING,
            BE_USERINSTALL_NOTENOUGHDISKSPACE,
            BE_USERINSTALL_NOWRITEACCESS,
            BE_MULTISESSION_NOT_SUPPORTED,
            BE_OFFICECONFIG_BROKEN
        };
        enum BootstrapStatus
        {
            BS_OK,
            BS_TERMINATE
        };

                                Desktop();
                                ~Desktop();
        virtual int         Main( );
        virtual void            Init();
        virtual void            InitFinished();
        virtual void            DeInit();
        virtual sal_Bool            QueryExit();
        virtual sal_uInt16          Exception(sal_uInt16 nError);
        virtual void            SystemSettingsChanging( AllSettings& rSettings, Window* pFrame );
        virtual void            AppEvent( const ApplicationEvent& rAppEvent );

        DECL_LINK(          OpenClients_Impl, void* );

        static void             OpenClients();
        static void             OpenDefault();

        DECL_LINK( EnableAcceptors_Impl, void*);

        static void             HandleAppEvent( const ApplicationEvent& rAppEvent );
        static ResMgr*          GetDesktopResManager();
        static CommandLineArgs& GetCommandLineArgs();

        void                    HandleBootstrapErrors(
            BootstrapError nError, OUString const & aMessage );
        void                    SetBootstrapError(
            BootstrapError nError, OUString const & aMessage )
        {
            if ( m_aBootstrapError == BE_OK )
            {
                m_aBootstrapError = nError;
                m_aBootstrapErrorMessage = aMessage;
            }
        }

        void                    SetBootstrapStatus( BootstrapStatus nStatus )
        {
            m_aBootstrapStatus = nStatus;
        }
        BootstrapStatus          GetBootstrapStatus() const
        {
            return m_aBootstrapStatus;
        }

        static sal_Bool         isCrashReporterEnabled();

        // first-start (ever) related methods
        static sal_Bool         CheckExtensionDependencies();

        static void             DoRestartActionsIfNecessary( sal_Bool bQuickStart );
        static void             SetRestartState();

        void                    SynchronizeExtensionRepositories();
        void                    SetSplashScreenText( const OUString& rText );
        void                    SetSplashScreenProgress( sal_Int32 );

        // Bootstrap methods
        static void             InitApplicationServiceManager();
            // throws an exception upon failure

    private:
        void RegisterServices(
            css::uno::Reference<
                css::uno::XComponentContext > const & context);
        void                    DeregisterServices();

        void                    CreateTemporaryDirectory();
        void                    RemoveTemporaryDirectory();

        sal_Bool                InitializeInstallation( const OUString& rAppFilename );
        bool                    InitializeConfiguration();
        void                    FlushConfiguration();
        static sal_Bool         shouldLaunchQuickstart();
        sal_Bool                InitializeQuickstartMode( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        void                    HandleBootstrapPathErrors( ::utl::Bootstrap::Status, const OUString& aMsg );
        void                    StartSetup( const OUString& aParameters );

        // Create a error message depending on bootstrap failure code and an optional file url
        OUString         CreateErrorMsgString( utl::Bootstrap::FailureCode nFailureCode,
                                                      const OUString& aFileURL );

        static void             PreloadModuleData( const CommandLineArgs& );
        static void             PreloadConfigurationData();

        css::uno::Reference<css::task::XStatusIndicator> m_rSplashScreen;
        void                    OpenSplashScreen();
        void                    CloseSplashScreen();

        void                    EnableOleAutomation();
                                DECL_LINK( ImplInitFilterHdl, ConvertData* );
        DECL_LINK(          AsyncInitFirstRun, void* );
        /** checks if the office is run the first time
            <p>If so, <method>DoFirstRunInitializations</method> is called (asynchronously and delayed) and the
            respective flag in the configuration is reset.</p>
        */
        void                    CheckFirstRun( );

        /// does initializations which are necessary for the first run of the office
        void                    DoFirstRunInitializations();

        static void             ShowBackingComponent(Desktop * progress);

        static sal_Bool         SaveTasks();

        static void             retrieveCrashReporterState();
        static sal_Bool         isUIOnSessionShutdownAllowed();

        // on-demand acceptors
        static void                         createAcceptor(const OUString& aDescription);
        static void                         enableAcceptors();
        static void                         destroyAcceptor(const OUString& aDescription);

        bool                            m_bCleanedExtensionCache;
        bool                            m_bServicesRegistered;
        BootstrapError                  m_aBootstrapError;
        OUString                        m_aBootstrapErrorMessage;
        BootstrapStatus                 m_aBootstrapStatus;

        boost::scoped_ptr<Lockfile> m_xLockfile;
        Timer    m_firstRunTimer;

        static ResMgr*                  pResMgr;
};

}

#endif // _DESKTOP_APP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
