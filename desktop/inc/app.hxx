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

#ifndef _DESKTOP_APP_HXX_
#define _DESKTOP_APP_HXX_

// stl includes first
#include <map>
#include <memory>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <tools/resmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Reference.h>
#include <osl/mutex.hxx>

using namespace com::sun::star::task;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace desktop
{

/*--------------------------------------------------------------------
    Description:    Application-class
 --------------------------------------------------------------------*/
class CommandLineArgs;
class Lockfile;
class AcceptorMap : public std::map< rtl::OUString, Reference<XInitialization> > {};
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
        void                    SetSplashScreenText( const ::rtl::OUString& rText );
        void                    SetSplashScreenProgress( sal_Int32 );

        // Bootstrap methods
        static void             InitApplicationServiceManager();
            // throws an exception upon failure

    private:
        void                    RegisterServices();
        void                    DeregisterServices();

        void                    DestroyApplicationServiceManager( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr );

        void                    CreateTemporaryDirectory();
        void                    RemoveTemporaryDirectory();

        sal_Bool                InitializeInstallation( const rtl::OUString& rAppFilename );
        bool                    InitializeConfiguration();
        void                    FlushConfiguration();
        static sal_Bool         shouldLaunchQuickstart();
        sal_Bool                InitializeQuickstartMode( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr );

        void                    HandleBootstrapPathErrors( ::utl::Bootstrap::Status, const ::rtl::OUString& aMsg );
        void                    StartSetup( const ::rtl::OUString& aParameters );

        // Create a error message depending on bootstrap failure code and an optional file url
        ::rtl::OUString         CreateErrorMsgString( utl::Bootstrap::FailureCode nFailureCode,
                                                      const ::rtl::OUString& aFileURL );

        static void             PreloadModuleData( const CommandLineArgs& );
        static void             PreloadConfigurationData();

        Reference<XStatusIndicator> m_rSplashScreen;
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
        static void                         createAcceptor(const rtl::OUString& aDescription);
        static void                         enableAcceptors();
        static void                         destroyAcceptor(const rtl::OUString& aDescription);

        bool                            m_bCleanedExtensionCache;
        bool                            m_bServicesRegistered;
        BootstrapError                  m_aBootstrapError;
        OUString                        m_aBootstrapErrorMessage;
        BootstrapStatus                 m_aBootstrapStatus;

        std::auto_ptr< Lockfile > m_pLockfile;
        Timer    m_firstRunTimer;

        static ResMgr*                  pResMgr;
};

}

#endif // _DESKTOP_APP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
