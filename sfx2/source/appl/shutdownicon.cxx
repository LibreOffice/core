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
#include "precompiled_sfx2.hxx"

#include <shutdownicon.hxx>
#include <app.hrc>
#include <sfx2/app.hxx>
#include <vos/mutex.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
// #include <cmdlineargs.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/fcontnr.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <cppuhelper/compbase1.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/extract.hxx>
#include <tools/urlobj.hxx>
#include <osl/security.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/link.hxx>
#ifdef UNX // need symlink
#include <unistd.h>
#include <errno.h>
#endif
#include <vcl/timer.hxx>

#include "sfx2/sfxresid.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::vos;
#ifdef WNT
using ::rtl::OUString;
#else
using namespace ::rtl;
#endif
using namespace ::sfx2;

#ifdef ENABLE_QUICKSTART_APPLET
# if !defined(WIN32) && !defined(QUARTZ)
extern "C" { static void SAL_CALL thisModule() {} }
# endif
#endif

#if defined(UNX) && defined(ENABLE_SYSTRAY_GTK)
#define PLUGIN_NAME "libqstart_gtkli.so"
#endif

class SfxNotificationListener_Impl : public cppu::WeakImplHelper1< XDispatchResultListener >
{
public:
    virtual void SAL_CALL dispatchFinished( const DispatchResultEvent& aEvent ) throw( RuntimeException );
    virtual void SAL_CALL disposing( const EventObject& aEvent ) throw( RuntimeException );
};

void SAL_CALL SfxNotificationListener_Impl::dispatchFinished( const DispatchResultEvent& ) throw( RuntimeException )
{
    ShutdownIcon::LeaveModalMode();
}

void SAL_CALL SfxNotificationListener_Impl::disposing( const EventObject& ) throw( RuntimeException )
{
}

SFX_IMPL_XSERVICEINFO( ShutdownIcon, "com.sun.star.office.Quickstart", "com.sun.star.comp.desktop.QuickstartWrapper" )  \
SFX_IMPL_ONEINSTANCEFACTORY( ShutdownIcon );

bool ShutdownIcon::bModalMode = false;
ShutdownIcon* ShutdownIcon::pShutdownIcon = NULL;

// To remove conditionals
extern "C" {
    static void disabled_initSystray() { }
    static void disabled_deInitSystray() { }
}
#define DOSTRING( x )                       #x
#define STRING( x )                         DOSTRING( x )

bool ShutdownIcon::LoadModule( osl::Module **pModule,
                               oslGenericFunction *pInit,
                               oslGenericFunction *pDeInit )
{
    if ( pModule )
    {
        OSL_ASSERT ( pInit && pDeInit );
        *pInit = *pDeInit = NULL;
        *pModule = NULL;
    }

#ifdef ENABLE_QUICKSTART_APPLET
#  ifdef WIN32
    if ( pModule )
    {
        *pInit = win32_init_sys_tray;
        *pDeInit = win32_shutdown_sys_tray;
    }
    return true;
#  elif defined QUARTZ
    *pInit = aqua_init_systray;
    *pDeInit = aqua_shutdown_systray;
    return true;
#  else // UNX
    osl::Module *pPlugin;
    pPlugin = new osl::Module();

    oslGenericFunction pTmpInit = NULL;
    oslGenericFunction pTmpDeInit = NULL;
    if ( pPlugin->loadRelative( &thisModule, OUString (RTL_CONSTASCII_USTRINGPARAM( STRING( PLUGIN_NAME ) ) ) ) )
    {
        pTmpInit = pPlugin->getFunctionSymbol(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "plugin_init_sys_tray" ) ) );
        pTmpDeInit = pPlugin->getFunctionSymbol(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "plugin_shutdown_sys_tray" ) ) );
    }
    if ( !pTmpInit || !pTmpDeInit )
    {
        delete pPlugin;
        pPlugin = NULL;
    }
    if ( pModule )
    {
        *pModule = pPlugin;
        *pInit = pTmpInit;
        *pDeInit = pTmpDeInit;
    }
    else
    {
        bool bRet = pPlugin != NULL;
        delete pPlugin;
        return bRet;
    }
#  endif // UNX
#endif // ENABLE_QUICKSTART_APPLET
    if ( pModule )
    {
        if ( !*pInit )
            *pInit = disabled_initSystray;
        if ( !*pDeInit )
            *pDeInit = disabled_deInitSystray;
    }

    return true;
}

class IdleUnloader : Timer
{
    ::osl::Module *m_pModule;
public:
    IdleUnloader (::osl::Module **pModule) :
        m_pModule (*pModule)
    {
        *pModule = NULL;
        Start();
    }
    virtual void Timeout()
    {
        delete m_pModule;
        delete this;
    }
};

void ShutdownIcon::initSystray()
{
    if (m_bInitialized)
        return;
    m_bInitialized = true;

    (void) LoadModule( &m_pPlugin, &m_pInitSystray, &m_pDeInitSystray );
    m_bVeto = true;
    m_pInitSystray();
}

void ShutdownIcon::deInitSystray()
{
    if (!m_bInitialized)
        return;

    if (m_pDeInitSystray)
        m_pDeInitSystray();

    m_bVeto = false;
    m_pInitSystray = 0;
    m_pDeInitSystray = 0;
    new IdleUnloader (&m_pPlugin);

    delete m_pFileDlg;
    m_pFileDlg = NULL;
    m_bInitialized = false;
}


ShutdownIcon::ShutdownIcon( Reference< XMultiServiceFactory > aSMgr ) :
    ShutdownIconServiceBase( m_aMutex ),
    m_bVeto ( false ),
    m_bListenForTermination ( false ),
    m_bSystemDialogs( false ),
    m_pResMgr( NULL ),
    m_pFileDlg( NULL ),
    m_xServiceManager( aSMgr ),
    m_pInitSystray( 0 ),
    m_pDeInitSystray( 0 ),
    m_pPlugin( 0 ),
    m_bInitialized( false )
{
    m_bSystemDialogs = SvtMiscOptions().UseSystemFileDialog();
}

ShutdownIcon::~ShutdownIcon()
{
    deInitSystray();
    new IdleUnloader (&m_pPlugin);
}

// ---------------------------------------------------------------------------

void ShutdownIcon::OpenURL( const ::rtl::OUString& aURL, const ::rtl::OUString& rTarget, const Sequence< PropertyValue >& aArgs )
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        Reference < XDispatchProvider > xDispatchProvider( getInstance()->m_xDesktop, UNO_QUERY );
        if ( xDispatchProvider.is() )
        {
            com::sun::star::util::URL aDispatchURL;
            aDispatchURL.Complete = aURL;

            Reference < com::sun::star::util::XURLTransformer > xURLTransformer(
                ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.util.URLTransformer") ),
                com::sun::star::uno::UNO_QUERY );
            if ( xURLTransformer.is() )
            {
                try
                {
                    Reference< com::sun::star::frame::XDispatch > xDispatch;

                    xURLTransformer->parseStrict( aDispatchURL );
                    xDispatch = xDispatchProvider->queryDispatch( aDispatchURL, rTarget, 0 );
                    if ( xDispatch.is() )
                        xDispatch->dispatch( aDispatchURL, aArgs );
                }
                catch ( com::sun::star::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( com::sun::star::uno::Exception& )
                {
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------

void ShutdownIcon::FileOpen()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        EnterModalMode();
        getInstance()->StartFileDialog();
    }
}

// ---------------------------------------------------------------------------

void ShutdownIcon::FromTemplate()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        Reference < ::com::sun::star::frame::XFramesSupplier > xDesktop ( getInstance()->m_xDesktop, UNO_QUERY);
        Reference < ::com::sun::star::frame::XFrame > xFrame( xDesktop->getActiveFrame() );
        if ( !xFrame.is() )
            xFrame = Reference < ::com::sun::star::frame::XFrame >( xDesktop, UNO_QUERY );

        URL aTargetURL;
        aTargetURL.Complete = OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:5500" ) );
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aTargetURL );

        Reference < ::com::sun::star::frame::XDispatchProvider > xProv( xFrame, UNO_QUERY );
        Reference < ::com::sun::star::frame::XDispatch > xDisp;
        if ( xProv.is() )
        {
            if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
                xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
            else
                xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_blank"), 0 );
        }
        if ( xDisp.is() )
        {
            Sequence<PropertyValue> aArgs(1);
            PropertyValue* pArg = aArgs.getArray();
            pArg[0].Name = rtl::OUString::createFromAscii("Referer");
            pArg[0].Value <<= ::rtl::OUString::createFromAscii("private:user");
            Reference< ::com::sun::star::frame::XNotifyingDispatch > xNotifyer( xDisp, UNO_QUERY );
            if ( xNotifyer.is() )
            {
                EnterModalMode();
                xNotifyer->dispatchWithNotification( aTargetURL, aArgs, new SfxNotificationListener_Impl() );
            }
            else
                xDisp->dispatch( aTargetURL, aArgs );
        }
    }
}

// ---------------------------------------------------------------------------
#include <tools/rcid.h>
OUString ShutdownIcon::GetResString( int id )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( ! m_pResMgr )
        m_pResMgr = SfxResId::GetResMgr();
    ResId aResId( id, *m_pResMgr );
    aResId.SetRT( RSC_STRING );
    if( !m_pResMgr || !m_pResMgr->IsAvailable( aResId ) )
        return OUString();

    UniString aRes( ResId(id, *m_pResMgr) );
    return OUString( aRes );
}

// ---------------------------------------------------------------------------

OUString ShutdownIcon::GetUrlDescription( const OUString& aUrl )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return OUString( SvFileInformationManager::GetDescription( INetURLObject( aUrl ) ) );
}

// ---------------------------------------------------------------------------

void ShutdownIcon::StartFileDialog()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    bool bDirty = ( m_bSystemDialogs != static_cast<bool>(SvtMiscOptions().UseSystemFileDialog()) );

    if ( m_pFileDlg && bDirty )
    {
        // Destroy instance as changing the system file dialog setting
        // forces us to create a new FileDialogHelper instance!
        delete m_pFileDlg;
        m_pFileDlg = NULL;
    }

    if ( !m_pFileDlg )
        m_pFileDlg = new FileDialogHelper( WB_OPEN | SFXWB_MULTISELECTION, String() );
    m_pFileDlg->StartExecuteModal( STATIC_LINK( this, ShutdownIcon, DialogClosedHdl_Impl ) );
}

// ---------------------------------------------------------------------------

IMPL_STATIC_LINK( ShutdownIcon, DialogClosedHdl_Impl, FileDialogHelper*, EMPTYARG )
{
    DBG_ASSERT( pThis->m_pFileDlg, "ShutdownIcon, DialogClosedHdl_Impl(): no file dialog" );

    // use ctor for filling up filters automatically! #89169#
    if ( ERRCODE_NONE == pThis->m_pFileDlg->GetError() )
    {
        Reference< XFilePicker >    xPicker = pThis->m_pFileDlg->GetFilePicker();

        try
        {

            if ( xPicker.is() )
            {

                Reference < XFilePickerControlAccess > xPickerControls ( xPicker, UNO_QUERY );
                Reference < XFilterManager > xFilterManager ( xPicker, UNO_QUERY );

                Sequence< OUString >        sFiles = xPicker->getFiles();
                int                         nFiles = sFiles.getLength();

                int                         nArgs=3;
                Sequence< PropertyValue >   aArgs(3);

                Reference < com::sun::star::task::XInteractionHandler > xInteraction(
                    ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.task.InteractionHandler") ),
                    com::sun::star::uno::UNO_QUERY );

                aArgs[0].Name = OUString::createFromAscii( "InteractionHandler" );
                aArgs[0].Value <<= xInteraction;

                sal_Int16 nMacroExecMode = ::com::sun::star::document::MacroExecMode::USE_CONFIG;
                aArgs[1].Name = OUString::createFromAscii( "MacroExecutionMode" );
                aArgs[1].Value <<= nMacroExecMode;

                sal_Int16 nUpdateDoc = ::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG;
                aArgs[2].Name = OUString::createFromAscii( "UpdateDocMode" );
                aArgs[2].Value <<= nUpdateDoc;

                // pb: #102643# use the filedlghelper to get the current filter name,
                // because it removes the extensions before you get the filter name.
                OUString aFilterName( pThis->m_pFileDlg->GetCurrentFilter() );

                if ( xPickerControls.is() )
                {

                    // Set readonly flag

                    sal_Bool    bReadOnly = sal_False;


                    xPickerControls->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 ) >>= bReadOnly;

                    // #95239#: Only set porperty if readonly is set to TRUE

                    if ( bReadOnly )
                    {
                        aArgs.realloc( ++nArgs );
                        aArgs[nArgs-1].Name  = OUString::createFromAscii( "ReadOnly" );
                        aArgs[nArgs-1].Value <<= bReadOnly;
                    }

                    // Get version string

                    sal_Int32   iVersion = -1;

                    xPickerControls->getValue( ExtendedFilePickerElementIds::LISTBOX_VERSION, ControlActions::GET_SELECTED_ITEM_INDEX ) >>= iVersion;

                    if ( iVersion >= 0 )
                    {
                        sal_Int16   uVersion = (sal_Int16)iVersion;

                        aArgs.realloc( ++nArgs );
                        aArgs[nArgs-1].Name  = OUString::createFromAscii( "Version" );
                        aArgs[nArgs-1].Value <<= uVersion;
                    }

                    // Retrieve the current filter

                    if ( !aFilterName.getLength() )
                        xPickerControls->getValue( CommonFilePickerElementIds::LISTBOX_FILTER, ControlActions::GET_SELECTED_ITEM ) >>= aFilterName;

                }


                // Convert UI filter name to internal filter name

                if ( aFilterName.getLength() )
                {
                    const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4UIName( aFilterName, 0, SFX_FILTER_NOTINFILEDLG );

                    if ( pFilter )
                    {
                        aFilterName = pFilter->GetFilterName();

                        if ( aFilterName.getLength() )
                        {
                            aArgs.realloc( ++nArgs );
                            aArgs[nArgs-1].Name  = OUString::createFromAscii( "FilterName" );
                            aArgs[nArgs-1].Value <<= aFilterName;
                        }
                    }
                }

                if ( 1 == nFiles )
                    OpenURL( sFiles[0], OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ), aArgs );
                else
                {
                    OUString    aBaseDirURL = sFiles[0];
                    if ( aBaseDirURL.getLength() > 0 && aBaseDirURL[aBaseDirURL.getLength()-1] != '/' )
                        aBaseDirURL += OUString::createFromAscii("/");

                    int iFiles;
                    for ( iFiles = 1; iFiles < nFiles; iFiles++ )
                    {
                        OUString    aURL = aBaseDirURL;
                        aURL += sFiles[iFiles];
                        OpenURL( aURL, OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ), aArgs );
                    }
                }
            }
        }
        catch ( ... )
        {
        }
    }

#ifdef WNT
    // #103346 Destroy dialog to prevent problems with custom controls
    // This fix is dependent on the dialog settings. Destroying the dialog here will
    // crash the non-native dialog implementation! Therefore make this dependent on
    // the settings.
    if ( SvtMiscOptions().UseSystemFileDialog() )
    {
        delete pThis->m_pFileDlg;
        pThis->m_pFileDlg = NULL;
    }
#endif

    LeaveModalMode();
    return 0;
}

// ---------------------------------------------------------------------------

void ShutdownIcon::addTerminateListener()
{
    ShutdownIcon* pInst = getInstance();
    if ( ! pInst)
        return;

    if (pInst->m_bListenForTermination)
        return;

    Reference< XDesktop > xDesktop = pInst->m_xDesktop;
    if ( ! xDesktop.is())
        return;

    xDesktop->addTerminateListener( pInst );
    pInst->m_bListenForTermination = true;
}

// ---------------------------------------------------------------------------

void ShutdownIcon::terminateDesktop()
{
    ShutdownIcon* pInst = getInstance();
    if ( ! pInst)
        return;

    Reference< XDesktop > xDesktop = pInst->m_xDesktop;
    if ( ! xDesktop.is())
        return;

    // always remove ourselves as listener
    xDesktop->removeTerminateListener( pInst );
    pInst->m_bListenForTermination = true;

    // terminate desktop only if no tasks exist
    Reference< XFramesSupplier > xSupplier( xDesktop, UNO_QUERY );
    if ( xSupplier.is() )
    {
        Reference< XIndexAccess > xTasks ( xSupplier->getFrames(), UNO_QUERY );
        if( xTasks.is() )
        {
            if( xTasks->getCount() < 1 )
                xDesktop->terminate();
        }
    }

    // remove the instance pointer
    ShutdownIcon::pShutdownIcon = 0;
}

// ---------------------------------------------------------------------------

ShutdownIcon* ShutdownIcon::getInstance()
{
    OSL_ASSERT( pShutdownIcon );
    return pShutdownIcon;
}

// ---------------------------------------------------------------------------

ShutdownIcon* ShutdownIcon::createInstance()
{
    if (pShutdownIcon)
        return pShutdownIcon;

    ShutdownIcon *pIcon = NULL;
    try {
        Reference< XMultiServiceFactory > xSMgr( comphelper::getProcessServiceFactory() );
        pIcon = new ShutdownIcon( xSMgr );
        pIcon->init ();
        pShutdownIcon = pIcon;
    } catch (...) {
        delete pIcon;
    }

    return pShutdownIcon;
}

void ShutdownIcon::init() throw( ::com::sun::star::uno::Exception )
{
    // access resource system and sfx only protected by solarmutex
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ResMgr *pResMgr = SfxResId::GetResMgr();

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    m_pResMgr = pResMgr;
    aGuard.clear();
    Reference < XDesktop > xDesktop( m_xServiceManager->createInstance(
                                             DEFINE_CONST_UNICODE( "com.sun.star.frame.Desktop" )),
                                     UNO_QUERY );
    aGuard.reset();
    m_xDesktop = xDesktop;
}

// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::disposing()
{
    m_xServiceManager = Reference< XMultiServiceFactory >();
    m_xDesktop = Reference< XDesktop >();
}

// ---------------------------------------------------------------------------

// XEventListener
void SAL_CALL ShutdownIcon::disposing( const ::com::sun::star::lang::EventObject& )
    throw(::com::sun::star::uno::RuntimeException)
{
}

// ---------------------------------------------------------------------------

// XTerminateListener
void SAL_CALL ShutdownIcon::queryTermination( const ::com::sun::star::lang::EventObject& )
throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard  aGuard( m_aMutex );

    if ( m_bVeto )
        throw ::com::sun::star::frame::TerminationVetoException();
}


// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::notifyTermination( const ::com::sun::star::lang::EventObject& )
throw(::com::sun::star::uno::RuntimeException)
{
}


// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments )
    throw( ::com::sun::star::uno::Exception )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // third argument only sets veto, everything else will be ignored!
    if (aArguments.getLength() > 2)
    {
        sal_Bool bVeto = sal_True;
        bVeto = ::cppu::any2bool(aArguments[2]);
        m_bVeto = bVeto;
        return;
    }

    if ( aArguments.getLength() > 0 )
    {
        if ( !ShutdownIcon::pShutdownIcon )
        {
            try
            {
                sal_Bool bQuickstart = sal_False;
                bQuickstart = ::cppu::any2bool( aArguments[0] );
                if( !bQuickstart && !GetAutostart() )
                    return;
                aGuard.clear();
                init ();
                aGuard.reset();
                if ( !m_xDesktop.is() )
                    return;

                /* Create a sub-classed instance - foo */
                ShutdownIcon::pShutdownIcon = this;
                initSystray();
#ifdef OS2
                // above win32 starts the quickstart thread, but we have
                // quickstart running only when -quickstart is specified
                // on command line (next boot).
                // so if -quickstart was not specified, we cannot issue
                // quickstart veto on shutdown.
                if (bQuickstart)
                {
                    // disable shutdown
                    ShutdownIcon::getInstance()->SetVeto( true );
                    ShutdownIcon::getInstance()->addTerminateListener();
                }
#endif
            }
            catch(const ::com::sun::star::lang::IllegalArgumentException&)
            {
            }
        }
    }
    if ( aArguments.getLength() > 1 )
    {
            sal_Bool bAutostart = sal_False;
            bAutostart = ::cppu::any2bool( aArguments[1] );
            if (bAutostart && !GetAutostart())
                SetAutostart( sal_True );
            if (!bAutostart && GetAutostart())
                SetAutostart( sal_False );
    }

}

// -------------------------------

void ShutdownIcon::EnterModalMode()
{
    bModalMode = sal_True;
}

// -------------------------------

void ShutdownIcon::LeaveModalMode()
{
    bModalMode = sal_False;
}

#ifdef WNT
// defined in shutdowniconw32.cxx
#elif defined(OS2)
// defined in shutdowniconOs2.cxx
#elif defined QUARTZ
// defined in shutdowniconaqua.cxx
#else
bool ShutdownIcon::IsQuickstarterInstalled()
{
#ifndef ENABLE_QUICKSTART_APPLET
    return false;
#else // !ENABLE_QUICKSTART_APPLET
#ifdef UNX
    return LoadModule( NULL, NULL, NULL);
#endif // UNX
#endif // !ENABLE_QUICKSTART_APPLET
}
#endif // !WNT

// ---------------------------------------------------------------------------

#if defined (ENABLE_QUICKSTART_APPLET) && defined (UNX)
static OUString getDotAutostart( bool bCreate = false )
{
    OUString aShortcut;
    const char *pConfigHome;
    if( (pConfigHome = getenv("XDG_CONFIG_HOME") ) )
        aShortcut = OStringToOUString( OString( pConfigHome ), RTL_TEXTENCODING_UTF8 );
    else
    {
        OUString aHomeURL;
        osl::Security().getHomeDir( aHomeURL );
        ::osl::File::getSystemPathFromFileURL( aHomeURL, aShortcut );
        aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "/.config" ) );
    }
    aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "/autostart" ) );
    if (bCreate)
    {
        OUString aShortcutUrl;
        osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
        osl::Directory::createPath( aShortcutUrl );
    }
    return aShortcut;
}
#endif

rtl::OUString ShutdownIcon::getShortcutName()
{
#ifndef ENABLE_QUICKSTART_APPLET
    return OUString();
#else

    OUString aShortcutName( RTL_CONSTASCII_USTRINGPARAM( "StarOffice 6.0" ) );
    ResMgr* pMgr = SfxResId::GetResMgr();
    if( pMgr )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        UniString aRes( SfxResId( STR_QUICKSTART_LNKNAME ) );
        aShortcutName = OUString( aRes );
    }
#ifdef WNT
    aShortcutName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".lnk" ) );

    OUString aShortcut(GetAutostartFolderNameW32());
    aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "\\" ) );
    aShortcut += aShortcutName;
#else // UNX
    OUString aShortcut = getDotAutostart();
    aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "/qstart.desktop" ) );
#endif // UNX
    return aShortcut;
#endif // ENABLE_QUICKSTART_APPLET
}

bool ShutdownIcon::GetAutostart( )
{
#if defined(OS2)
    return GetAutostartOs2( );
#elif defined QUARTZ
    return true;
#else
    bool bRet = false;
#ifdef ENABLE_QUICKSTART_APPLET
    OUString aShortcut( getShortcutName() );
    OUString aShortcutUrl;
    osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
    osl::File f( aShortcutUrl );
    osl::File::RC error = f.open( OpenFlag_Read );
    if( error == osl::File::E_None )
    {
        f.close();
        bRet = true;
    }
#endif // ENABLE_QUICKSTART_APPLET
    return bRet;
#endif
}

void ShutdownIcon::SetAutostart( bool bActivate )
{
#ifdef ENABLE_QUICKSTART_APPLET
    OUString aShortcut( getShortcutName() );

    if( bActivate && IsQuickstarterInstalled() )
    {
#ifdef WNT
        EnableAutostartW32( aShortcut );
#else // UNX
        getDotAutostart( true );

        OUString aPath( RTL_CONSTASCII_USTRINGPARAM("${BRAND_BASE_DIR}/share/xdg/qstart.desktop" ) );
        Bootstrap::expandMacros( aPath );

        OUString aDesktopFile;
        ::osl::File::getSystemPathFromFileURL( aPath, aDesktopFile );

        OString aDesktopFileUnx = OUStringToOString( aDesktopFile,
                                                     osl_getThreadTextEncoding() );
        OString aShortcutUnx = OUStringToOString( aShortcut,
                                                  osl_getThreadTextEncoding() );
        if ((0 != symlink(aDesktopFileUnx, aShortcutUnx)) && (errno == EEXIST))
        {
        unlink(aShortcutUnx);
        symlink(aDesktopFileUnx, aShortcutUnx);
        }

        ShutdownIcon *pIcon = ShutdownIcon::createInstance();
        if( pIcon )
            pIcon->initSystray();
#endif // UNX
    }
    else
    {
        OUString aShortcutUrl;
        ::osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
        ::osl::File::remove( aShortcutUrl );
#ifdef UNX
        if (pShutdownIcon)
        {
            ShutdownIcon *pIcon = getInstance();
            pIcon->deInitSystray();
        }
#endif
    }
#elif defined OS2
    SetAutostartOs2( bActivate );
#else
    (void)bActivate; // unused variable
#endif // ENABLE_QUICKSTART_APPLET
}

static const ::sal_Int32 PROPHANDLE_TERMINATEVETOSTATE = 0;

// XFastPropertySet
void SAL_CALL ShutdownIcon::setFastPropertyValue(       ::sal_Int32                  nHandle,
                                                  const ::com::sun::star::uno::Any& aValue )
    throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::beans::PropertyVetoException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException)
{
    switch(nHandle)
    {
        case PROPHANDLE_TERMINATEVETOSTATE :
             {
                // use new value in case it's a valid information only
                ::sal_Bool bState( sal_False );
                if (! (aValue >>= bState))
                    return;

                m_bVeto = bState;
                if (m_bVeto && ! m_bListenForTermination)
                    addTerminateListener();
             }
             break;

        default :
            throw ::com::sun::star::beans::UnknownPropertyException();
    }
}

// XFastPropertySet
::com::sun::star::uno::Any SAL_CALL ShutdownIcon::getFastPropertyValue( ::sal_Int32 nHandle )
    throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aValue;
    switch(nHandle)
    {
        case PROPHANDLE_TERMINATEVETOSTATE :
             {
                bool bState   = (m_bListenForTermination && m_bVeto);
                     aValue <<= bState;
             }
             break;

        default :
            throw ::com::sun::star::beans::UnknownPropertyException();
    }

    return aValue;
}
