/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: app.hxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:29:39 $
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

#ifndef _DESKTOP_APP_HXX_
#define _DESKTOP_APP_HXX_

// stl includes first
#include <map>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_TIMER_HXX_
#include <vcl/timer.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

using namespace com::sun::star::task;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

#define DESKTOP_SAVETASKS_MOD 0x1
#define DESKTOP_SAVETASKS_UNMOD 0x2
#define DESKTOP_SAVETASKS_ALL 0x3

namespace desktop
{

/*--------------------------------------------------------------------
    Description:    Application-class
 --------------------------------------------------------------------*/
class IntroWindow_Impl;
class CommandLineArgs;
class Lockfile;
class AcceptorMap : public std::map< OUString, Reference<XInitialization> > {};
struct ConvertData;
class Desktop : public Application
{
    friend class UserInstall;

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
            BE_USERINSTALL_NOWRITEACCESS
        };
        enum BootstrapStatus
        {
            BS_OK,
            BS_TERMINATE
        };

                                Desktop();
                                ~Desktop();
        virtual void            Main( );
        virtual void            Init();
        virtual void            DeInit();
        virtual BOOL            QueryExit();
        virtual USHORT          Exception(USHORT nError);
        virtual void            SystemSettingsChanging( AllSettings& rSettings, Window* pFrame );
        virtual void            AppEvent( const ApplicationEvent& rAppEvent );

        DECL_LINK(          OpenClients_Impl, void* );

        static void             OpenClients();
        static void             OpenDefault();

        DECL_LINK( EnableAcceptors_Impl, void*);

        static void             HandleAppEvent( const ApplicationEvent& rAppEvent );
        static ResMgr*          GetDesktopResManager();
        static CommandLineArgs* GetCommandLineArgs();

        void                    HandleBootstrapErrors( BootstrapError );
        void                    SetBootstrapError( BootstrapError nError )
        {
            if ( m_aBootstrapError == BE_OK )
                m_aBootstrapError = nError;
        }
        BootstrapError          GetBootstrapError() const
        {
            return m_aBootstrapError;
        }

        void                    SetBootstrapStatus( BootstrapStatus nStatus )
        {
            m_aBootstrapStatus = nStatus;
        }
        BootstrapStatus          GetBootstrapStatus() const
        {
            return m_aBootstrapStatus;
        }

        DECL_STATIC_LINK( Desktop, AsyncTerminate, void*);
        static sal_Bool         CheckOEM();
        static sal_Bool         isCrashReporterEnabled();

        // first-start (ever) & license relate methods
        static rtl::OUString    GetLicensePath();
        static sal_Bool         LicenseNeedsAcceptance();
        static sal_Bool         IsFirstStartWizardNeeded();

    private:
        // Bootstrap methods
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > CreateApplicationServiceManager();

        void                    RegisterServices( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr );
        void                    DeregisterServices();

        void                    DestroyApplicationServiceManager( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr );

        void                    CreateTemporaryDirectory();
        void                    RemoveTemporaryDirectory();

        sal_Bool                InitializeInstallation( const rtl::OUString& rAppFilename );
        sal_Bool                InitializeConfiguration();
        sal_Bool                InitializeQuickstartMode( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr );

        void                    HandleBootstrapPathErrors( ::utl::Bootstrap::Status, const ::rtl::OUString& aMsg );
        void                    StartSetup( const ::rtl::OUString& aParameters );

        // Get a resource message string securely e.g. if resource cannot be retrieved return aFaultBackMsg
        ::rtl::OUString         GetMsgString( USHORT nId, const ::rtl::OUString& aFaultBackMsg );

        // Create a error message depending on bootstrap failure code and an optional file url
        ::rtl::OUString         CreateErrorMsgString( utl::Bootstrap::FailureCode nFailureCode,
                                                      const ::rtl::OUString& aFileURL );

        static void             PreloadModuleData( CommandLineArgs* );
        static void             PreloadConfigurationData();

        Reference<XStatusIndicator> m_rSplashScreen;
        void                    OpenSplashScreen();
        void                    SetSplashScreenProgress(sal_Int32);
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

        static sal_Bool         SaveTasks();
        static sal_Bool         _bTasksSaved;

        static void             retrieveCrashReporterState();

        // on-demand acceptors
        static void                         createAcceptor(const OUString& aDescription);
        static void                         enableAcceptors();
        static void                         destroyAcceptor(const OUString& aDescription);

        sal_Bool                        m_bMinimized;
        sal_Bool                        m_bInvisible;
        bool                            m_bServicesRegistered;
        USHORT                          m_nAppEvents;
        IntroWindow_Impl*               m_pIntro;
        BootstrapError                  m_aBootstrapError;
        BootstrapStatus                 m_aBootstrapStatus;

        Lockfile *m_pLockfile;
        Timer    m_firstRunTimer;

        static ResMgr*                  pResMgr;
        static sal_Bool                 bSuppressOpenDefault;
};

}

#endif // _DESKTOP_APP_HXX_
