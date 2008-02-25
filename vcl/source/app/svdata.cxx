/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdata.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:54:21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <string.h>

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <vcl/salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <vcl/salframe.hxx>
#endif

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#include <osl/process.h>
#include <osl/file.hxx>
#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _VCL_FONTCFG_HXX
#include <vcl/fontcfg.hxx>
#endif
#ifndef VCL_INC_CONFIGSETTINGS_HXX
#include <vcl/configsettings.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <vcl/svdata.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <vcl/window.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _VCL_UNOHELP_HXX
#include <vcl/unohelp.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx> // for Button::GetStandardText
#endif
#ifndef _SV_DOCKWIN_HXX
#include <vcl/dockwin.hxx>  // for DockingManager
#endif

#ifndef _SV_SALIMESTATUS_HXX
#include <vcl/salimestatus.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XEXTENDEDTOOLKIT_HPP_
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_JAVANOTCONFIGUREDEXCEPTION_HPP_
#include <com/sun/star/java/JavaNotConfiguredException.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_JAVAVMCREATIONFAILUREEXCEPTION_HPP_
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_MISSINGJAVARUNTIMEEXCEPTION_HPP_
#include <com/sun/star/java/MissingJavaRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_JAVADISABLEDEXCEPTION_HPP_
#include <com/sun/star/java/JavaDisabledException.hpp>
#endif

#include <com/sun/star/lang/XComponent.hpp>

#include <stdio.h>
#include <vcl/salsys.hxx>
#include <vcl/svids.hrc>
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;
using namespace rtl;

// =======================================================================

namespace
{
    struct private_aImplSVData :
        public rtl::Static<ImplSVData, private_aImplSVData> {};
}

// static SV-Data
ImplSVData* pImplSVData = NULL;

SalSystem* ImplGetSalSystem()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSalSystem )
        pSVData->mpSalSystem = pSVData->mpDefInst->CreateSalSystem();
    return pSVData->mpSalSystem;
}


static String& ReplaceJavaErrorMessages( String& rString )
{
    rString.SearchAndReplaceAllAscii( "%OK", Button::GetStandardText( BUTTON_OK ) );
    rString.SearchAndReplaceAllAscii( "%IGNORE", Button::GetStandardText( BUTTON_IGNORE ) );
    rString.SearchAndReplaceAllAscii( "%CANCEL", Button::GetStandardText( BUTTON_CANCEL ) );

    return rString;
}

// =======================================================================

void ImplInitSVData()
{
    pImplSVData = &private_aImplSVData::get();

    // init global instance data
    memset( pImplSVData, 0, sizeof( ImplSVData ) );
    pImplSVData->maHelpData.mbAutoHelpId = sal_True;
    pImplSVData->maHelpData.mbAutoHelpId = sal_True;
    pImplSVData->maNWFData.maMenuBarHighlightTextColor = Color( COL_TRANSPARENT );
}

// -----------------------------------------------------------------------

void ImplDeInitSVData()
{
    ImplSVData* pSVData = ImplGetSVData();

    // delete global instance data
    if( pSVData->mpSettingsConfigItem )
        delete pSVData->mpSettingsConfigItem;

    if( pSVData->mpDockingManager )
        delete pSVData->mpDockingManager;

    if( pSVData->maGDIData.mpDefaultFontConfiguration )
        delete pSVData->maGDIData.mpDefaultFontConfiguration;
    if( pSVData->maGDIData.mpFontSubstConfiguration )
        delete pSVData->maGDIData.mpFontSubstConfiguration;

    if ( pSVData->maAppData.mpMSFTempFileName )
    {
        if ( pSVData->maAppData.mxMSF.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( pSVData->maAppData.mxMSF, ::com::sun::star::uno::UNO_QUERY );
            xComp->dispose();
            pSVData->maAppData.mxMSF = NULL;
        }

        ::rtl::OUString aFileUrl;
        ::osl::File::getFileURLFromSystemPath( *pSVData->maAppData.mpMSFTempFileName, aFileUrl );
        osl::File::remove( aFileUrl );
        delete pSVData->maAppData.mpMSFTempFileName;
        pSVData->maAppData.mpMSFTempFileName = NULL;
    }
}

// -----------------------------------------------------------------------

void ImplDestroySVData()
{
    pImplSVData = NULL;
}

// -----------------------------------------------------------------------

Window* ImplGetDefaultWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin;

    // First test if we already have a default window.
    // Don't only place a single if..else inside solar mutex lockframe
    // because then we might have to wait for the solar mutex what is not neccessary
    // if we already have a default window.

    if ( !pSVData->mpDefaultWin )
    {
        Application::GetSolarMutex().acquire();

        // Test again because the thread who released the solar mutex could have called
        // the same method

        if ( !pSVData->mpDefaultWin && !pSVData->mbDeInit )
        {
            DBG_WARNING( "ImplGetDefaultWindow(): No AppWindow" );
            pSVData->mpDefaultWin = new WorkWindow( 0, WB_DEFAULTWIN );
            pSVData->mpDefaultWin->SetText( OUString( RTL_CONSTASCII_USTRINGPARAM( "VCL ImplGetDefaultWindow" ) ) );
        }
        Application::GetSolarMutex().release();
    }

    return pSVData->mpDefaultWin;
}

// -----------------------------------------------------------------------

#define VCL_CREATERESMGR_NAME( Name )   #Name

ResMgr* ImplGetResMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpResMgr )
    {
        ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        pSVData->mpResMgr = ResMgr::SearchCreateResMgr( VCL_CREATERESMGR_NAME( vcl ), aLocale );

        static bool bMessageOnce = false;
        if( !pSVData->mpResMgr && ! bMessageOnce )
        {
            bMessageOnce = true;
            const char* pMsg =
                "Missing vcl resource. This indicates that files vital to localization are missing. "
                "You might have a corrupt installation.";
            fprintf( stderr, "%s\n", pMsg );
            ErrorBox aBox( NULL, WB_OK | WB_DEF_OK, rtl::OUString( pMsg, strlen( pMsg ), RTL_TEXTENCODING_ASCII_US ) );
            aBox.Execute();
        }
    }
    return pSVData->mpResMgr;
}

DockingManager* ImplGetDockingManager()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpDockingManager )
        pSVData->mpDockingManager = new DockingManager();

    return pSVData->mpDockingManager;
}

class AccessBridgeCurrentContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
{
public:
    AccessBridgeCurrentContext(
        const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > &context ) :
        m_prevContext( context ) {}

    // XCurrentContext
    virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
        throw (com::sun::star::uno::RuntimeException);
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_prevContext;
};

com::sun::star::uno::Any AccessBridgeCurrentContext::getValueByName( const rtl::OUString & Name )
    throw (com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any ret;
    if( Name.equalsAscii( "java-vm.interaction-handler" ) )
    {
        // Currently, for accessbility no interaction handler shall be offered.
        // There may be introduced later on a handler using native toolkits
        // jbu->obr: Instantiate here your interaction handler
    }
    else if( m_prevContext.is() )
    {
        ret = m_prevContext->getValueByName( Name );
    }
    return ret;
}


bool ImplInitAccessBridge(BOOL bAllowCancel, BOOL &rCancelled)
{
    rCancelled = FALSE;

    bool bErrorMessage = true;

    // Note:
    // if bAllowCancel is TRUE we were called from application startup
    //  where we will disable any Java errorboxes and show our own accessibility dialog if Java throws an exception
    // if bAllowCancel is FALSE we were called from Tools->Options
    //  where we will see Java errorboxes, se we do not show our dialogs in addition to Java's

    try
    {
        bool bSuccess = true;

        // No error messages when env var is set ..
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( pEnv && *pEnv )
        {
            bErrorMessage = false;
        }

        ImplSVData* pSVData = ImplGetSVData();
        if( ! pSVData->mxAccessBridge.is() )
        {
            Reference< XMultiServiceFactory > xFactory(vcl::unohelper::GetMultiServiceFactory());

            if( xFactory.is() )
            {
                Reference< XExtendedToolkit > xToolkit =
                    Reference< XExtendedToolkit >(Application::GetVCLToolkit(), UNO_QUERY);

                Sequence< Any > arguments(1);
                arguments[0] = makeAny(xToolkit);

                // Disable default java error messages on startup, because they were probably unreadable
                // for a disabled user. Use native message boxes which are accessible without java support.
                // No need to do this when activated by Tools-Options dialog ..
                if( bAllowCancel )
                {
                    // customize the java-not-available-interaction-handler entry within the
                    // current context when called at startup.
                    com::sun::star::uno::ContextLayer layer(
                        new AccessBridgeCurrentContext( com::sun::star::uno::getCurrentContext() ) );

                    pSVData->mxAccessBridge = xFactory->createInstanceWithArguments(
                            OUString::createFromAscii( "com.sun.star.accessibility.AccessBridge" ),
                            arguments
                        );
                }
                else
                {
                    pSVData->mxAccessBridge = xFactory->createInstanceWithArguments(
                            OUString::createFromAscii( "com.sun.star.accessibility.AccessBridge" ),
                            arguments
                        );
                }

                if( !pSVData->mxAccessBridge.is() )
                    bSuccess = false;
            }
        }

        return bSuccess;
    }

    catch(::com::sun::star::java::JavaNotConfiguredException e)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            String aTitle(ResId(SV_ACCESSERROR_JAVA_NOT_CONFIGURED, *pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr));

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = TRUE;
        }

        return false;
    }

    catch(::com::sun::star::java::JavaVMCreationFailureException e)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            String aTitle(ResId(SV_ACCESSERROR_FAULTY_JAVA, *pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr));

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = TRUE;
        }

        return false;
    }

    catch(::com::sun::star::java::MissingJavaRuntimeException e)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            String aTitle(ResId(SV_ACCESSERROR_MISSING_JAVA, *pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr));

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = TRUE;
        }

        return false;
    }

    catch(::com::sun::star::java::JavaDisabledException e)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && bAllowCancel && pResMgr )
        {
            String aTitle(ResId(SV_ACCESSERROR_JAVA_DISABLED, *pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, *pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr));

            int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                aTitle,
                ReplaceJavaErrorMessages(aMessage),
                SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL);

            // Do not change the setting in case the user chooses to cancel
            if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                rCancelled = TRUE;
        }

        return false;
    }


    catch(::com::sun::star::uno::RuntimeException e)
    {
        ResMgr *pResMgr = ImplGetResMgr();
        if( bErrorMessage && pResMgr )
        {
            String aTitle;
            String aMessage(ResId(SV_ACCESSERROR_BRIDGE_MSG, *pResMgr));

            if( 0 == e.Message.compareTo(::rtl::OUString::createFromAscii("ClassNotFound"), 13) )
            {
                aTitle = String(ResId(SV_ACCESSERROR_MISSING_BRIDGE, *pResMgr));
            }
            else if( 0 == e.Message.compareTo(::rtl::OUString::createFromAscii("NoSuchMethod"), 12) )
            {
                aTitle = String(ResId(SV_ACCESSERROR_WRONG_VERSION, *pResMgr));
            }

            if( aTitle.Len() != 0 )
            {
                if( bAllowCancel )
                {
                    // Something went wrong initializing the Java AccessBridge (on Windows) during the
                    // startup. Since the office will be probably unusable for a disabled user, we offer
                    // to terminate directly.
                    aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
                    aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, *pResMgr));

                    int ret = ImplGetSalSystem()->ShowNativeMessageBox(
                        aTitle,
                        ReplaceJavaErrorMessages(aMessage),
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL,
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL);

                    // Do not change the setting in case the user chooses to cancel
                    if( SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL == ret )
                        rCancelled = TRUE;
                }
                else
                {
                    // The user tried to activate accessibility support using Tools-Options dialog,
                    // so we don't offer to terminate here !
                    ImplGetSalSystem()->ShowNativeMessageBox(
                        aTitle,
                        ReplaceJavaErrorMessages(aMessage),
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK,
                        SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK);
                }
            }
        }

        return false;
    }

    catch (...)
    {
        return false;
    }
}

// -----------------------------------------------------------------------

Window* ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFrameWindow = pSVData->maWinData.mpFirstFrame;
    while ( pFrameWindow )
    {
        if ( pFrameWindow->ImplGetFrame() == pFrame )
        {
            Window* pWindow = pFrameWindow->ImplFindWindow( rSalFramePos );
            if ( !pWindow )
                pWindow = pFrameWindow->ImplGetWindow();
            rSalFramePos = pWindow->ImplFrameToOutput( rSalFramePos );
            return pWindow;
        }
        pFrameWindow = pFrameWindow->ImplGetFrameData()->mpNextFrame;
    }

    return NULL;
}
