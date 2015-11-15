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

#include <sal/config.h>

#include <cassert>

#include <boost/logic/tribool.hpp>
#include <config_folders.h>
#include <shutdownicon.hxx>
#include <app.hrc>
#include <sfx2/app.hxx>
#include <osl/mutex.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/fcontnr.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/extract.hxx>
#include <tools/urlobj.hxx>
#include <osl/security.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#ifdef UNX // need symlink
#include <unistd.h>
#include <errno.h>
#endif

#include <sfx2/sfxresid.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;

#ifdef ENABLE_QUICKSTART_APPLET
# if !defined(WIN32) && !defined(MACOSX)
extern "C" { static void SAL_CALL thisModule() {} }
# endif
#endif

class SfxNotificationListener_Impl : public cppu::WeakImplHelper< XDispatchResultListener >
{
public:
    virtual void SAL_CALL dispatchFinished( const DispatchResultEvent& aEvent ) throw( RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const EventObject& aEvent ) throw( RuntimeException, std::exception ) override;
};

void SAL_CALL SfxNotificationListener_Impl::dispatchFinished( const DispatchResultEvent& ) throw( RuntimeException, std::exception )
{
    ShutdownIcon::LeaveModalMode();
}

void SAL_CALL SfxNotificationListener_Impl::disposing( const EventObject& ) throw( RuntimeException, std::exception )
{
}

OUString SAL_CALL ShutdownIcon::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.desktop.QuickstartWrapper");
}

sal_Bool SAL_CALL ShutdownIcon::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ShutdownIcon::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence< OUString > aSeq { "com.sun.star.office.Quickstart" };
    return aSeq;
}

bool ShutdownIcon::bModalMode = false;
ShutdownIcon* ShutdownIcon::pShutdownIcon = nullptr;

#if !defined( ENABLE_QUICKSTART_APPLET )
// To remove conditionals
extern "C" {
    static void disabled_initSystray() { }
    static void disabled_deInitSystray() { }
}
#endif

namespace {

boost::logic::tribool loaded(boost::logic::indeterminate);
oslGenericFunction pInitSystray(nullptr);
oslGenericFunction pDeInitSystray(nullptr);

bool LoadModule()
{
    if (boost::logic::indeterminate(loaded))
    {
#ifdef ENABLE_QUICKSTART_APPLET
#  ifdef WIN32
        pInitSystray = win32_init_sys_tray;
        pDeInitSystray = win32_shutdown_sys_tray;
        loaded = true;
#  elif defined MACOSX
        pInitSystray = aqua_init_systray;
        pDeInitSystray = aqua_shutdown_systray;
        loaded = true;
#  else // UNX
        osl::Module plugin;
        oslGenericFunction pTmpInit = nullptr;
        oslGenericFunction pTmpDeInit = nullptr;
        if ( plugin.loadRelative( &thisModule, "libqstart_gtklo.so" ) )
        {
            pTmpInit = plugin.getFunctionSymbol( "plugin_init_sys_tray" );
            pTmpDeInit = plugin.getFunctionSymbol( "plugin_shutdown_sys_tray" );
        }
        if ( !pTmpInit || !pTmpDeInit )
        {
            loaded = false;
        }
        else
        {
            plugin.release();
            pInitSystray = pTmpInit;
            pDeInitSystray = pTmpDeInit;
            loaded = true;
        }
#  endif // UNX
#else
        pInitSystray = disabled_initSystray;
        pDeInitSystray = disabled_deInitSystray;
        loaded = false;
#endif // ENABLE_QUICKSTART_APPLET
    }
    assert(!boost::logic::indeterminate(loaded));
    return loaded;
}

}

void ShutdownIcon::initSystray()
{
    if (m_bInitialized)
        return;
    m_bInitialized = true;

    (void) LoadModule();
    m_bVeto = true;
    pInitSystray();
}

void ShutdownIcon::deInitSystray()
{
    if (!m_bInitialized)
        return;

    if (pDeInitSystray)
        pDeInitSystray();

    m_bVeto = false;
    pInitSystray = nullptr;
    pDeInitSystray = nullptr;

    delete m_pFileDlg;
    m_pFileDlg = nullptr;
    m_bInitialized = false;
}


ShutdownIcon::ShutdownIcon( const css::uno::Reference< XComponentContext > & rxContext ) :
    ShutdownIconServiceBase( m_aMutex ),
    m_bVeto ( false ),
    m_bListenForTermination ( false ),
    m_bSystemDialogs( false ),
    m_pResMgr( nullptr ),
    m_pFileDlg( nullptr ),
    m_xContext( rxContext ),
    m_bInitialized( false )
{
    m_bSystemDialogs = SvtMiscOptions().UseSystemFileDialog();
}

ShutdownIcon::~ShutdownIcon()
{
    deInitSystray();
}



void ShutdownIcon::OpenURL( const OUString& aURL, const OUString& rTarget, const Sequence< PropertyValue >& aArgs )
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        css::uno::Reference < XDispatchProvider > xDispatchProvider( getInstance()->m_xDesktop, UNO_QUERY );
        if ( xDispatchProvider.is() )
        {
            css::util::URL aDispatchURL;
            aDispatchURL.Complete = aURL;

            css::uno::Reference< util::XURLTransformer > xURLTransformer( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
            try
            {
                css::uno::Reference< css::frame::XDispatch > xDispatch;

                xURLTransformer->parseStrict( aDispatchURL );
                xDispatch = xDispatchProvider->queryDispatch( aDispatchURL, rTarget, 0 );
                if ( xDispatch.is() )
                    xDispatch->dispatch( aDispatchURL, aArgs );
            }
            catch ( css::uno::RuntimeException& )
            {
                throw;
            }
            catch ( css::uno::Exception& )
            {
            }
        }
    }
}



void ShutdownIcon::FileOpen()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        ::SolarMutexGuard aGuard;
        EnterModalMode();
        getInstance()->StartFileDialog();
    }
}



void ShutdownIcon::FromTemplate()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        css::uno::Reference < css::frame::XFramesSupplier > xDesktop ( getInstance()->m_xDesktop, UNO_QUERY);
        css::uno::Reference < css::frame::XFrame > xFrame( xDesktop->getActiveFrame() );
        if ( !xFrame.is() )
            xFrame.set( xDesktop, UNO_QUERY );

        URL aTargetURL;
        aTargetURL.Complete = ".uno:NewDoc";
        css::uno::Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aTargetURL );

        css::uno::Reference < css::frame::XDispatchProvider > xProv( xFrame, UNO_QUERY );
        css::uno::Reference < css::frame::XDispatch > xDisp;
        if ( xProv.is() )
        {
            xDisp = xProv->queryDispatch( aTargetURL, "_self", 0 );
        }
        if ( xDisp.is() )
        {
            Sequence<PropertyValue> aArgs(1);
            PropertyValue* pArg = aArgs.getArray();
            pArg[0].Name = "Referer";
            pArg[0].Value <<= OUString("private:user");
            css::uno::Reference< css::frame::XNotifyingDispatch > xNotifier(xDisp, UNO_QUERY);
            if (xNotifier.is())
            {
                EnterModalMode();
                xNotifier->dispatchWithNotification(aTargetURL, aArgs, new SfxNotificationListener_Impl());
            }
            else
                xDisp->dispatch( aTargetURL, aArgs );
        }
    }
}


#include <tools/rcid.h>
OUString ShutdownIcon::GetResString( int id )
{
    ::SolarMutexGuard aGuard;

    if( ! m_pResMgr )
        m_pResMgr = SfxResId::GetResMgr();
    ResId aResId( id, *m_pResMgr );
    aResId.SetRT( RSC_STRING );
    if (!m_pResMgr->IsAvailable(aResId))
        return OUString();

    return ResId(id, *m_pResMgr).toString();
}



OUString ShutdownIcon::GetUrlDescription( const OUString& aUrl )
{
    ::SolarMutexGuard aGuard;

    return OUString( SvFileInformationManager::GetDescription( INetURLObject( aUrl ) ) );
}



void ShutdownIcon::StartFileDialog()
{
    ::SolarMutexGuard aGuard;

    bool bDirty = ( m_bSystemDialogs != static_cast<bool>(SvtMiscOptions().UseSystemFileDialog()) );

    if ( m_pFileDlg && bDirty )
    {
        // Destroy instance as changing the system file dialog setting
        // forces us to create a new FileDialogHelper instance!
        delete m_pFileDlg;
        m_pFileDlg = nullptr;
    }

    if ( !m_pFileDlg )
        m_pFileDlg = new FileDialogHelper(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                SFXWB_MULTISELECTION, OUString() );
    m_pFileDlg->StartExecuteModal( LINK( this, ShutdownIcon, DialogClosedHdl_Impl ) );
}



IMPL_LINK_TYPED( ShutdownIcon, DialogClosedHdl_Impl, FileDialogHelper*, /*unused*/, void )
{
    DBG_ASSERT( m_pFileDlg, "ShutdownIcon, DialogClosedHdl_Impl(): no file dialog" );

    // use constructor for filling up filters automatically!
    if ( ERRCODE_NONE == m_pFileDlg->GetError() )
    {
        css::uno::Reference< XFilePicker2 >    xPicker = m_pFileDlg->GetFilePicker();

        try
        {

            if ( xPicker.is() )
            {

                css::uno::Reference < XFilePickerControlAccess > xPickerControls ( xPicker, UNO_QUERY );
                css::uno::Reference < XFilterManager > xFilterManager ( xPicker, UNO_QUERY );

                Sequence< OUString >        sFiles = xPicker->getSelectedFiles();
                int                         nFiles = sFiles.getLength();

                int                         nArgs=3;
                Sequence< PropertyValue >   aArgs(3);

                css::uno::Reference < css::task::XInteractionHandler2 > xInteraction(
                    task::InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), nullptr) );

                aArgs[0].Name = "InteractionHandler";
                aArgs[0].Value <<= xInteraction;

                sal_Int16 nMacroExecMode = css::document::MacroExecMode::USE_CONFIG;
                aArgs[1].Name = "MacroExecutionMode";
                aArgs[1].Value <<= nMacroExecMode;

                sal_Int16 nUpdateDoc = css::document::UpdateDocMode::ACCORDING_TO_CONFIG;
                aArgs[2].Name = "UpdateDocMode";
                aArgs[2].Value <<= nUpdateDoc;

                // use the filedlghelper to get the current filter name,
                // because it removes the extensions before you get the filter name.
                OUString aFilterName( m_pFileDlg->GetCurrentFilter() );

                if ( xPickerControls.is() )
                {

                    // Set readonly flag

                    bool    bReadOnly = false;


                    xPickerControls->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 ) >>= bReadOnly;

                    // Only set property if readonly is set to TRUE

                    if ( bReadOnly )
                    {
                        aArgs.realloc( ++nArgs );
                        aArgs[nArgs-1].Name  = "ReadOnly";
                        aArgs[nArgs-1].Value <<= bReadOnly;
                    }

                    // Get version string

                    sal_Int32   iVersion = -1;

                    xPickerControls->getValue( ExtendedFilePickerElementIds::LISTBOX_VERSION, ControlActions::GET_SELECTED_ITEM_INDEX ) >>= iVersion;

                    if ( iVersion >= 0 )
                    {
                        sal_Int16   uVersion = (sal_Int16)iVersion;

                        aArgs.realloc( ++nArgs );
                        aArgs[nArgs-1].Name  = "Version";
                        aArgs[nArgs-1].Value <<= uVersion;
                    }

                    // Retrieve the current filter

                    if ( aFilterName.isEmpty() )
                        xPickerControls->getValue( CommonFilePickerElementIds::LISTBOX_FILTER, ControlActions::GET_SELECTED_ITEM ) >>= aFilterName;

                }


                // Convert UI filter name to internal filter name

                if ( !aFilterName.isEmpty() )
                {
                    const SfxFilter* pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4UIName( aFilterName, SfxFilterFlags::NONE, SfxFilterFlags::NOTINFILEDLG );

                    if ( pFilter )
                    {
                        aFilterName = pFilter->GetFilterName();

                        if ( !aFilterName.isEmpty() )
                        {
                            aArgs.realloc( ++nArgs );
                            aArgs[nArgs-1].Name  = "FilterName";
                            aArgs[nArgs-1].Value <<= aFilterName;
                        }
                    }
                }

                if ( 1 == nFiles )
                    OpenURL( sFiles[0], "_default", aArgs );
                else
                {
                    OUString    aBaseDirURL = sFiles[0];
                    if ( !aBaseDirURL.isEmpty() && !aBaseDirURL.endsWith("/") )
                        aBaseDirURL += "/";

                    int iFiles;
                    for ( iFiles = 1; iFiles < nFiles; iFiles++ )
                    {
                        OUString    aURL = aBaseDirURL;
                        aURL += sFiles[iFiles];
                        OpenURL( aURL, "_default", aArgs );
                    }
                }
            }
        }
        catch ( ... )
        {
        }
    }

#ifdef WNT
    // Destroy dialog to prevent problems with custom controls
    // This fix is dependent on the dialog settings. Destroying the dialog here will
    // crash the non-native dialog implementation! Therefore make this dependent on
    // the settings.
    if ( SvtMiscOptions().UseSystemFileDialog() )
    {
        delete m_pFileDlg;
        m_pFileDlg = NULL;
    }
#endif

    LeaveModalMode();
}



void ShutdownIcon::addTerminateListener()
{
    ShutdownIcon* pInst = getInstance();
    if ( ! pInst)
        return;

    if (pInst->m_bListenForTermination)
        return;

    css::uno::Reference< XDesktop2 > xDesktop = pInst->m_xDesktop;
    if ( ! xDesktop.is())
        return;

    xDesktop->addTerminateListener( pInst );
    pInst->m_bListenForTermination = true;
}



void ShutdownIcon::terminateDesktop()
{
    ShutdownIcon* pInst = getInstance();
    if ( ! pInst)
        return;

    css::uno::Reference< XDesktop2 > xDesktop = pInst->m_xDesktop;
    if ( ! xDesktop.is())
        return;

    // always remove ourselves as listener
    pInst->m_bListenForTermination = true;
    xDesktop->removeTerminateListener( pInst );

    // terminate desktop only if no tasks exist
    css::uno::Reference< XIndexAccess > xTasks ( xDesktop->getFrames(), UNO_QUERY );
    if( xTasks.is() && xTasks->getCount() < 1 )
        Application::Quit();

    // remove the instance pointer
    ShutdownIcon::pShutdownIcon = nullptr;
}



ShutdownIcon* ShutdownIcon::getInstance()
{
    OSL_ASSERT( pShutdownIcon );
    return pShutdownIcon;
}



ShutdownIcon* ShutdownIcon::createInstance()
{
    if (pShutdownIcon)
        return pShutdownIcon;

    ShutdownIcon *pIcon = nullptr;
    try {
        pIcon = new ShutdownIcon( comphelper::getProcessComponentContext() );
        pIcon->init ();
        pShutdownIcon = pIcon;
    } catch (...) {
        delete pIcon;
    }

    return pShutdownIcon;
}

void ShutdownIcon::init() throw( css::uno::Exception, std::exception )
{
    // access resource system and sfx only protected by solarmutex
    ::SolarMutexGuard aSolarGuard;
    ResMgr *pResMgr = SfxResId::GetResMgr();

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    m_pResMgr = pResMgr;
    aGuard.clear();
    css::uno::Reference < XDesktop2 > xDesktop = Desktop::create( m_xContext );
    aGuard.reset();
    m_xDesktop = xDesktop;
}



void SAL_CALL ShutdownIcon::disposing()
{
    m_xContext.clear();
    m_xDesktop.clear();

    deInitSystray();
}



// XEventListener
void SAL_CALL ShutdownIcon::disposing( const css::lang::EventObject& )
    throw(css::uno::RuntimeException, std::exception)
{
}



// XTerminateListener
void SAL_CALL ShutdownIcon::queryTermination( const css::lang::EventObject& )
throw(css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception)
{
    SAL_INFO("sfx.appl", "ShutdownIcon::queryTermination: veto is " << m_bVeto);
    ::osl::ClearableMutexGuard  aGuard( m_aMutex );

    if ( m_bVeto )
        throw css::frame::TerminationVetoException();
}




void SAL_CALL ShutdownIcon::notifyTermination( const css::lang::EventObject& )
throw(css::uno::RuntimeException, std::exception)
{
}




void SAL_CALL ShutdownIcon::initialize( const css::uno::Sequence< css::uno::Any>& aArguments )
    throw( css::uno::Exception, std::exception )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // third argument only sets veto, everything else will be ignored!
    if (aArguments.getLength() > 2)
    {
        bool bVeto = ::cppu::any2bool(aArguments[2]);
        m_bVeto = bVeto;
        return;
    }

    if ( aArguments.getLength() > 0 )
    {
        if ( !ShutdownIcon::pShutdownIcon )
        {
            try
            {
                bool bQuickstart = ::cppu::any2bool( aArguments[0] );
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
            }
            catch(const css::lang::IllegalArgumentException&)
            {
            }
        }
    }
    if ( aArguments.getLength() > 1 )
    {
            bool bAutostart = ::cppu::any2bool( aArguments[1] );
            if (bAutostart && !GetAutostart())
                SetAutostart( true );
            if (!bAutostart && GetAutostart())
                SetAutostart( false );
    }

}



void ShutdownIcon::EnterModalMode()
{
    bModalMode = true;
}



void ShutdownIcon::LeaveModalMode()
{
    bModalMode = false;
}

#ifdef WNT
// defined in shutdowniconw32.cxx
#elif defined MACOSX
// defined in shutdowniconaqua.cxx
#else
bool ShutdownIcon::IsQuickstarterInstalled()
{
#ifndef ENABLE_QUICKSTART_APPLET
    return false;
#else // !ENABLE_QUICKSTART_APPLET
#ifdef UNX
    return LoadModule();
#endif // UNX
#endif // !ENABLE_QUICKSTART_APPLET
}
#endif // !WNT



#if defined (ENABLE_QUICKSTART_APPLET) && defined (UNX)
/**
* Return the XDG autostart directory.
* http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html
* Available in Unix and with Quickstart enabled.
* @param bCreate Create the directory if it does not exist yet.
* @return OUString containing the autostart directory path.
*/
static OUString getAutostartDir( bool bCreate = false )
{
    OUString aShortcut;
    const char *pConfigHome;
    if( (pConfigHome = getenv("XDG_CONFIG_HOME") ) )
        aShortcut = OStringToOUString( OString( pConfigHome ),
                                       RTL_TEXTENCODING_UTF8 );
    else
    {
        OUString aHomeURL;
        osl::Security().getHomeDir( aHomeURL );
        ::osl::File::getSystemPathFromFileURL( aHomeURL, aShortcut );
        aShortcut += "/.config";
    }
    aShortcut += "/autostart";
    if (bCreate)
    {
        OUString aShortcutUrl;
        osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
        osl::Directory::createPath( aShortcutUrl );
    }
    return aShortcut;
}
#endif

OUString ShutdownIcon::getShortcutName()
{
#ifndef ENABLE_QUICKSTART_APPLET
    return OUString();
#else

    OUString aShortcutName( "StarOffice 6.0"  );
    ResMgr* pMgr = SfxResId::GetResMgr();
    if( pMgr )
    {
        ::SolarMutexGuard aGuard;
        aShortcutName = SFX2_RESSTR(STR_QUICKSTART_LNKNAME);
    }
#ifdef WNT
    aShortcutName += ".lnk";

    OUString aShortcut(GetAutostartFolderNameW32());
    aShortcut += "\\";
    aShortcut += aShortcutName;
#else // UNX
    OUString aShortcut = getAutostartDir();
    aShortcut += "/qstart.desktop";
#endif // UNX
    return aShortcut;
#endif // ENABLE_QUICKSTART_APPLET
}

bool ShutdownIcon::GetAutostart( )
{
#if defined MACOSX
    return true;
#else
    bool bRet = false;
#ifdef ENABLE_QUICKSTART_APPLET
    OUString aShortcut( getShortcutName() );
    OUString aShortcutUrl;
    osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
    osl::File f( aShortcutUrl );
    osl::File::RC error = f.open( osl_File_OpenFlag_Read );
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
        getAutostartDir( true );

        OUString aPath( "${BRAND_BASE_DIR}/" LIBO_SHARE_FOLDER "/xdg/qstart.desktop"  );
        rtl::Bootstrap::expandMacros( aPath );

        OUString aDesktopFile;
        ::osl::File::getSystemPathFromFileURL( aPath, aDesktopFile );

        OString aDesktopFileUnx = OUStringToOString( aDesktopFile,
                                                     osl_getThreadTextEncoding() );
        OString aShortcutUnx = OUStringToOString( aShortcut,
                                                  osl_getThreadTextEncoding() );
        if ((0 != symlink(aDesktopFileUnx.getStr(), aShortcutUnx.getStr())) && (errno == EEXIST))
        {
            unlink(aShortcutUnx.getStr());
            int ret = symlink(aDesktopFileUnx.getStr(), aShortcutUnx.getStr());
            (void)ret; //deliberately ignore return value, it's non-critical if it fails
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
#else
    (void)bActivate; // unused variable
#endif // ENABLE_QUICKSTART_APPLET
}

static const ::sal_Int32 PROPHANDLE_TERMINATEVETOSTATE = 0;

// XFastPropertySet
void SAL_CALL ShutdownIcon::setFastPropertyValue(       ::sal_Int32                  nHandle,
                                                  const css::uno::Any& aValue )
    throw (css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception)
{
    switch(nHandle)
    {
        case PROPHANDLE_TERMINATEVETOSTATE :
             {
                // use new value in case it's a valid information only
                bool bState( false );
                if (! (aValue >>= bState))
                    return;

                m_bVeto = bState;
                if (m_bVeto && ! m_bListenForTermination)
                    addTerminateListener();
             }
             break;

        default :
            throw css::beans::UnknownPropertyException();
    }
}

// XFastPropertySet
css::uno::Any SAL_CALL ShutdownIcon::getFastPropertyValue( ::sal_Int32 nHandle )
    throw (css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception)
{
    css::uno::Any aValue;
    switch(nHandle)
    {
        case PROPHANDLE_TERMINATEVETOSTATE :
             {
                bool bState   = (m_bListenForTermination && m_bVeto);
                     aValue <<= bState;
             }
             break;

        default :
            throw css::beans::UnknownPropertyException();
    }

    return aValue;
}

namespace {

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(new ShutdownIcon(context)))
    {}

    rtl::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_desktop_QuickstartWrapper_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
