/*************************************************************************
 *
 *  $RCSfile: svdata.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:32:09 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#include <string.h>

#define _SV_SVDATA_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
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
#include <fontcfg.hxx>
#endif
#define private public
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif

#ifndef _VCL_UNOWRAP_HXX
#include <unowrap.hxx>
#endif
#ifndef _VCL_UNOHELP_HXX
#include <unohelp.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <button.hxx> // for Button::GetStandardText
#endif

#ifndef _SV_SALIMESTATUS_HXX
#include <salimestatus.hxx>
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
#include <salsys.hxx>
#include <svids.hrc>

#pragma hdrstop

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;
using namespace rtl;

// =======================================================================

// static Empty-SV-String
static XubString aImplSVEmptyStr;
XubString& rImplSVEmptyStr = aImplSVEmptyStr;
static ByteString aImplSVEmptyByteStr;
ByteString& rImplSVEmptyByteStr = aImplSVEmptyByteStr;

ImplSVData private_aImplSVData;
// static SV-Data
ImplSVData* pImplSVData = &private_aImplSVData;

// static SharedLib SV-Data
ImplSVShlData aImplSVShlData;

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
    ImplSVData* pSVData = pImplSVData;
    ImplSVData** ppSVData = (ImplSVData**)GetAppData( SHL_SV );
    *ppSVData = &private_aImplSVData;

    // init global sharedlib data
    // ...

    // init global instance data
    memset( pSVData, 0, sizeof( ImplSVData ) );
#ifdef REMOTE_APPSERVER
    pSVData->mpPrinterEnvironment = new NMSP_CLIENT::PrinterEnvironment();
#endif
}

// -----------------------------------------------------------------------

void ImplDeInitSVData()
{
    ImplSVData* pSVData = ImplGetSVData();

    // delete global instance data
    if( pSVData->mpSettingsConfigItem )
        delete pSVData->mpSettingsConfigItem;

    if( pSVData->maGDIData.mpDefFontConfig )
        delete pSVData->maGDIData.mpDefFontConfig;
    if( pSVData->maGDIData.mpFontSubstConfig )
        delete pSVData->maGDIData.mpFontSubstConfig;
    if( pSVData->mpImeStatus )
        delete pSVData->mpImeStatus;
    if( pSVData->mpSalSystem )
        delete pSVData->mpSalSystem;

    if ( pSVData->mpUnoWrapper )
    {
        pSVData->mpUnoWrapper->Destroy();
        pSVData->mpUnoWrapper = NULL;
    }

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
#ifdef REMOTE_APPSERVER
    delete pSVData->mpPrinterEnvironment;
#endif
}

// -----------------------------------------------------------------------

void ImplDestroySVData()
{
    ImplSVData** ppSVData = (ImplSVData**)GetAppData( SHL_SV );
    ImplSVData*  pSVData = *ppSVData;

    // delete global sharedlib data
    // ...

    *ppSVData = NULL;
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

        if ( !pSVData->mpDefaultWin )
        {
            DBG_WARNING( "ImplGetDefaultWindow(): No AppWindow" );
            pSVData->mpDefaultWin = new WorkWindow( 0, 0 );
        }
        Application::GetSolarMutex().release();
    }

    return pSVData->mpDefaultWin;
}

// -----------------------------------------------------------------------

#define VCL_CREATERESMGR_NAME( Name )   #Name MAKE_NUMSTR( SUPD )

ResMgr* ImplGetResMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpResMgr )
    {
        pSVData->mpResMgr = ResMgr::CreateResMgr( VCL_CREATERESMGR_NAME( vcl ) );
        if ( !pSVData->mpResMgr )
        {
            LanguageType eLang = Application::GetSettings().GetUILanguage();
            pSVData->mpResMgr = ResMgr::SearchCreateResMgr( VCL_CREATERESMGR_NAME( vcl ), eLang );
        }
    }
    return pSVData->mpResMgr;
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

    // no error messages during installation (i.e., when there is no configuration)
    bool bErrorMessage = (FALSE != vcl::SettingsConfigItem::get()->IsValidConfigMgr());

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
        if( bErrorMessage && bAllowCancel )
        {
            ResMgr *pResMgr = ImplGetResMgr();

            String aTitle(ResId(SV_ACCESSERROR_JAVA_NOT_CONFIGURED, pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, pResMgr));

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
        if( bErrorMessage && bAllowCancel )
        {
            ResMgr *pResMgr = ImplGetResMgr();

            String aTitle(ResId(SV_ACCESSERROR_FAULTY_JAVA, pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, pResMgr));

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
        if( bErrorMessage && bAllowCancel )
        {
            ResMgr *pResMgr = ImplGetResMgr();

            String aTitle(ResId(SV_ACCESSERROR_MISSING_JAVA, pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, pResMgr));

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
        if( bErrorMessage && bAllowCancel )
        {
            ResMgr *pResMgr = ImplGetResMgr();

            String aTitle(ResId(SV_ACCESSERROR_JAVA_DISABLED, pResMgr));
            String aMessage(ResId(SV_ACCESSERROR_JAVA_MSG, pResMgr));

            aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
            aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, pResMgr));

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
        if( bErrorMessage )
        {
            ResMgr *pResMgr = ImplGetResMgr();

            String aTitle;
            String aMessage(ResId(SV_ACCESSERROR_BRIDGE_MSG, pResMgr));

            if( 0 == e.Message.compareTo(::rtl::OUString::createFromAscii("ClassNotFound"), 13) )
            {
                aTitle = String(ResId(SV_ACCESSERROR_MISSING_BRIDGE, pResMgr));
            }
            else if( 0 == e.Message.compareTo(::rtl::OUString::createFromAscii("NoSuchMethod"), 12) )
            {
                aTitle = String(ResId(SV_ACCESSERROR_WRONG_VERSION, pResMgr));
            }

            if( aTitle.Len() != 0 )
            {
                if( bAllowCancel )
                {
                    // Something went wrong initializing the Java AccessBridge (on Windows) during the
                    // startup. Since the office will be probably unusable for a disabled user, we offer
                    // to terminate directly.
                    aMessage += String(" ", 1, RTL_TEXTENCODING_ASCII_US);
                    aMessage += String(ResId(SV_ACCESSERROR_OK_CANCEL_MSG, pResMgr));

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

#ifndef REMOTE_APPSERVER

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
        pFrameWindow = pFrameWindow->mpFrameData->mpNextFrame;
    }

    return NULL;
}

#endif

// -----------------------------------------------------------------------

#ifdef REMOTE_APPSERVER

void rvpExceptionHandler()
{
#ifdef DBG_UTIL
    fprintf( stderr, "RVP exception caught!\n" );
#endif
}

#endif
