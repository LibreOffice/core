/*************************************************************************
 *
 *  $RCSfile: appinit.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-23 12:23:17 $
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

#include "app.hxx"

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#include <svtools/svtools.hrc>
#include <svtools/saveopt.hxx>

#ifndef _SV_CONFIG_HXX
#include <vcl/config.hxx>
#endif
#ifndef _SOERR_HXX //autogen
#include <so3/soerr.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _INETBND_HXX //autogen
#include <so3/inetbnd.hxx>
#endif
#ifndef _INET_WRAPPER_HXX
#include <inet/wrapper.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#include <svtools/inethist.hxx>
#ifndef _COM_SUN_STAR_INSTALLATION_XPROTOCOLHANDLERCHECK_HPP_
#include <com/sun/star/installation/XProtocolHandlerCheck.hpp>
#endif
#ifndef _COM_SUN_STAR_INSTALLATION_PROTOCOLS_HPP_
#include <com/sun/star/installation/protocols.hpp>
#endif
#ifndef _COM_SUN_STAR_INSTALLATION_PROTDLGRES_HPP_
#include <com/sun/star/installation/ProtDlgRes.hpp>
#endif
#ifndef _COM_SUN_STAR_INSTALLATION_XINSTALLATIONCHECK_HPP_
#include <com/sun/star/installation/XInstallationCheck.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPARAMETERIZEDCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_INSTALLATION_PROTDLGRES_HPP_
#include <com/sun/star/installation/ProtDlgRes.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XUNOURLRESOLVER_HPP_
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#pragma hdrstop

#include "unoctitm.hxx"
#include "appimp.hxx"
#include "accmgr.hxx"
#include "app.hrc"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "cfgmgr.hxx"
#include "dispatch.hxx"
#include "docfac.hxx"
#include "evntconf.hxx"
#include "frameobj.hxx"
#include "fsetobsh.hxx"
#include "fsetview.hxx"
#include "imgmgr.hxx"
#include "interno.hxx"
#include "intro.hxx"
#include "macrconf.hxx"
#include "mnumgr.hxx"
#include "msgpool.hxx"
#include "newhdl.hxx"
#include "picklist.hxx"
#include "plugobj.hxx"
#include "progress.hxx"
#if SUPD<608
#include "saveopt.hxx"
#endif
#include "sfxhelp.hxx"
#include "sfxresid.hxx"
#include "sfxtypes.hxx"
#include "stbmgr.hxx"
#include "tbxconf.hxx"
#include "viewsh.hxx"
#include "eacopier.hxx"
#include "nochaos.hxx"
#include "ucbhelp.hxx"
#include "fcontnr.hxx"
#include "helper.hxx"   // SfxContentHelper::Kill()

#ifdef UNX
#define stricmp(a,b) strcmp(a,b)
#endif

#ifdef MAC
int svstricmp( const char* pStr1, const char* pStr2);
#define stricmp svstricmp
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

void doFirstTimeInit();

class SfxTerminateListener_Impl : public ::cppu::WeakImplHelper1< XTerminateListener  >
{
public:
    virtual void SAL_CALL queryTermination( const EventObject& aEvent ) throw( TerminationVetoException, RuntimeException );
    virtual void SAL_CALL notifyTermination( const EventObject& aEvent ) throw( RuntimeException );
    virtual void SAL_CALL disposing( const EventObject& Source ) throw( RuntimeException );
};

void SAL_CALL SfxTerminateListener_Impl::disposing( const EventObject& Source ) throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SFX_APP()->Deinitialize();
}

void SAL_CALL SfxTerminateListener_Impl::queryTermination( const EventObject& aEvent ) throw(TerminationVetoException, RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !SFX_APP()->QueryExit_Impl() )
        throw TerminationVetoException();
}

void SAL_CALL SfxTerminateListener_Impl::notifyTermination( const EventObject& aEvent ) throw(RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxApplication* pApp = SFX_APP();
    SfxPickList_Impl::Get()->ClearMemCache();
    pApp->Get_Impl()->aLateInitTimer.Stop();
    pApp->Broadcast( SfxSimpleHint( SFX_HINT_DEINITIALIZING ) );
    pApp->Get_Impl()->pAppDispatch->ReleaseAll();
    pApp->Get_Impl()->pAppDispatch->release();
    pApp->NotifyEvent(SfxEventHint( SFX_EVENT_CLOSEAPP) );
    Application::Quit();
}

//====================================================================

static bool configureUcb(String const & rUcbUrl)
{
    Reference< XMultiServiceFactory >
        xFactory(comphelper::getProcessServiceFactory());
    if (!xFactory.is())
    {
        DBG_ERROR("configureUcb: No XMultiServiceFactory");
        return false;
    }

    // Create (local) UCB configured as specified in the registry (it should
    // be configured to have no content providers, otherwise the rUcbUrl
    // (i.e., the '-ucb=...' command line argument) will be ignored:
    Reference< ucb::XContentProviderManager > xUcb;
    Sequence< Any > aArgs(1);
    aArgs[0] <<= sal_True;
    try
    {
        xUcb
            = Reference< ucb::XContentProviderManager >(
                  xFactory->
                      createInstanceWithArguments(
                          rtl::OUString(
                              RTL_CONSTASCII_USTRINGPARAM(
                                  "com.sun.star.ucb.UniversalContentBroker")),
                          aArgs),
                  UNO_QUERY);
    }
    catch (RuntimeException const &) { throw; }
    catch (Exception const &) {}
    if (!xUcb.is())
    {
        DBG_ERROR("configureUcb(): No UniversalContentBroker service");
        return false;
    }

    // If there's already any content providers registered, ignore rUcbUrl:
    if (xUcb->queryContentProviders().getLength() != 0)
    {
        DBG_WARNING(
            "configureUcb(): Configured UCB and -ucb command line arg");
        return true;
    }

    // Modify rUcbUrl with user supplied information until connecting to this
    // URL succeeds:
    Reference< bridge::XUnoUrlResolver > xResolver;
    try
    {
        xResolver
            = Reference< bridge::XUnoUrlResolver >(
                  xFactory->
                      createInstance(
                          rtl::OUString(
                              RTL_CONSTASCII_USTRINGPARAM(
                                  "com.sun.star.bridge.UnoUrlResolver"))),
                  UNO_QUERY);
    }
    catch (RuntimeException const &) { throw; }
    catch (Exception const &) {}
    if (!xResolver.is())
    {
        DBG_ERROR("configureUcb(): No UnoUrlResolver service");
        return false;
    }
    String aTheUcbUrl(rUcbUrl);
    Reference< awt::XDialog > xDialog;
    Reference< beans::XPropertySet > xPropertySet;
    for (;;)
    {
        bool bSuccess = false;
        try
        {
            bSuccess = xResolver->resolve(aTheUcbUrl).is() != false;
        }
        catch (connection::NoConnectException const &) {}
        catch (connection::ConnectionSetupException const &) {}
        catch (IllegalArgumentException const &) {}
        if (bSuccess)
            break;

        // Get the login dialog:
        if (!xDialog.is())
        {
            try
            {
                xDialog
                    = Reference< awt::XDialog >(
                          xFactory->
                              createInstance(
                                  rtl::OUString(
                                      RTL_CONSTASCII_USTRINGPARAM(
                                          "com.sun.star.framework."
                                              "LoginDialog"))),
                          UNO_QUERY);
            }
            catch (RuntimeException const &) { throw; }
            catch (Exception const &) {}
            xPropertySet
                = Reference< beans::XPropertySet >(xDialog, UNO_QUERY);
            if (!(xDialog.is() && xPropertySet.is()))
            {
                DBG_ERROR("configureUcb(): No LoginDialog service");
                return false;
            }
        }

        // Identify the authentication data within the URL:
        enum Parameter
        {
            PARAM_NONE = 0,
            PARAM_HOST = 2,
            PARAM_PORT = 4,
            PARAM_USER = 6,
            PARAM_PASSWORD = 8,
            PARAM_TICKET = 9
        };
        struct Occurence
        {
            xub_StrLen m_nStart;
            xub_StrLen m_nValue;
            xub_StrLen m_nEnd;
            Parameter m_eParam;
        };
        Occurence aParams[4];
        int nParamCount = 0;
        xub_StrLen nEnd = aTheUcbUrl.Len();
        xub_StrLen nPos = 0;
        while (nPos != nEnd && aTheUcbUrl.GetChar(nPos++) != ':');
            // skip <scheme> part...
        while (nPos != nEnd && aTheUcbUrl.GetChar(nPos) != ',')
            ++nPos; // skip 'name' of <connection> part...
        xub_StrLen nInsert = nPos;
        if (nPos != nEnd && aTheUcbUrl.GetChar(nPos) == ',')
            ++nPos;
        while (nPos != nEnd && aTheUcbUrl.GetChar(nPos) != ';')
        {
            xub_StrLen nStart = nPos;
            while (nPos != nEnd
                   && aTheUcbUrl.GetChar(nPos) != '='
                   && aTheUcbUrl.GetChar(nPos) != ','
                   && aTheUcbUrl.GetChar(nPos) != ';')
                ++nPos;
            if (nPos != nEnd && aTheUcbUrl.GetChar(nPos) == '=')
            {
                String aName(aTheUcbUrl, nStart, nPos - nStart);
                Parameter eParam
                    = aName.EqualsIgnoreCaseAscii("host") ? PARAM_HOST :
                      aName.EqualsIgnoreCaseAscii("port") ? PARAM_PORT :
                      aName.EqualsIgnoreCaseAscii("user") ? PARAM_USER :
                      aName.EqualsIgnoreCaseAscii("password") ?
                           PARAM_PASSWORD :
                      aName.EqualsIgnoreCaseAscii("ticket") ? PARAM_TICKET :
                           PARAM_NONE;
                if (int(eParam) != int(PARAM_NONE))
                    // work around compiler bug...
                {
                    bool bNew = true;
                    for (int i = 0; i != nParamCount; ++i)
                        if ((aParams[i].m_eParam & ~1) == (eParam & ~1))
                            // treat 'password' and 'ticket' the same...
                        {
                            bNew = false;
                            break;
                        }
                    if (bNew)
                    {
                        aParams[nParamCount].m_eParam = eParam;
                        aParams[nParamCount].m_nStart = nStart - 1;
                        aParams[nParamCount].m_nValue = ++nPos;
                        while (nPos != nEnd
                               && aTheUcbUrl.GetChar(nPos) != ','
                               && aTheUcbUrl.GetChar(nPos) != ';')
                            ++nPos;
                        aParams[nParamCount].m_nEnd = nPos;
                        ++nParamCount;
                    }
                }
            }
            while (nPos != nEnd
                   && aTheUcbUrl.GetChar(nPos) != ','
                   && aTheUcbUrl.GetChar(nPos) != ';')
                ++nPos;
            if (nPos != nEnd && aTheUcbUrl.GetChar(nPos) == ',')
                ++nPos;
        }

        // Copy the authentication data from the URL to the dialog:
        try
        {
            String aHostPort;
            for (int i = 0; i < nParamCount; ++i)
            {
                String aValue(INetURLObject::decode(
                                  aTheUcbUrl.Copy(aParams[i].m_nValue,
                                                  aParams[i].m_nEnd
                                                      - aParams[i].m_nValue),
                                  '%', INetURLObject::DECODE_WITH_CHARSET));
                if (aValue.Len() != 0)
                    switch (aParams[i].m_eParam)
                    {
                        case PARAM_HOST:
                            aHostPort.Insert(aValue, 0);
                            break;

                        case PARAM_PORT:
                            aHostPort.Append(':');
                            aHostPort.Append(aValue);
                            break;

                        case PARAM_USER:
                            xPropertySet->
                                setPropertyValue(
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                      "UserName")),
                                    makeAny(rtl::OUString(aValue)));
                            break;

                        case PARAM_PASSWORD:
                            xPropertySet->
                                setPropertyValue(
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                      "Password")),
                                    makeAny(rtl::OUString(aValue)));
                            break;
                    }
            }
            if (aHostPort.Len() != 0)
                xPropertySet->
                    setPropertyValue(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Server")),
                        makeAny(rtl::OUString(aHostPort)));
        }
        catch (beans::UnknownPropertyException const &) {}
        catch (beans::PropertyVetoException const &) {}
        catch (IllegalArgumentException const &) {}
        catch (WrappedTargetException const &) {}

        if (xDialog->execute() == 0)
            return false;

        // Copy the authentication data back from the dialog into the URL:
        rtl::OUString aHost;
        rtl::OUString aUser;
        rtl::OUString aPassword;
        bSuccess = false;
        try
        {
            bSuccess
                = (xPropertySet->
                           getPropertyValue(
                               rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                 "Server")))
                       >>= aHost)
                && (xPropertySet->
                            getPropertyValue(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                  "UserName")))
                        >>= aUser)
                && (xPropertySet->
                            getPropertyValue(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                  "Password")))
                        >>= aPassword);
        }
        catch (beans::UnknownPropertyException const &) {}
        catch (WrappedTargetException const &) {}
        if (!bSuccess)
        {
            DBG_ERROR("configureUcb(): Bad LoginDialog service");
            return false;
        }
        sal_Int32 nColon = aHost.indexOf(':');
        if (nColon == -1)
            nColon = aHost.getLength();
        while (nParamCount-- > 0)
            aTheUcbUrl.Erase(aParams[nParamCount].m_nStart,
                             aParams[nParamCount].m_nEnd
                                 - aParams[nParamCount].m_nStart);
        if (aPassword.getLength() != 0)
        {
            aTheUcbUrl.InsertAscii(",password=", nInsert);
            aTheUcbUrl.Insert(INetURLObject::encode(
                                  aPassword,
                                  INetURLObject::PART_UNO_PARAM_VALUE,
                                  '%',
                                  INetURLObject::ENCODE_ALL),
                              nInsert + RTL_CONSTASCII_LENGTH(",password="));
        }
        if (aUser.getLength() != 0)
        {
            aTheUcbUrl.InsertAscii(",user=", nInsert);
            aTheUcbUrl.Insert(INetURLObject::encode(
                                  aUser,
                                  INetURLObject::PART_UNO_PARAM_VALUE,
                                  '%',
                                  INetURLObject::ENCODE_ALL),
                              nInsert + RTL_CONSTASCII_LENGTH(",user="));
        }
        if (aHost.getLength() - nColon > 1)
        {
            aTheUcbUrl.InsertAscii(",port=", nInsert);
            aTheUcbUrl.Insert(INetURLObject::encode(
                                  aHost.copy(nColon + 1),
                                  INetURLObject::PART_UNO_PARAM_VALUE,
                                  '%',
                                  INetURLObject::ENCODE_ALL),
                              nInsert + RTL_CONSTASCII_LENGTH(",port="));
        }
        if (nColon > 0)
        {
            aTheUcbUrl.InsertAscii(",host=", nInsert);
            aTheUcbUrl.Insert(INetURLObject::encode(
                                  aHost.copy(0, nColon),
                                  INetURLObject::PART_UNO_PARAM_VALUE,
                                  '%',
                                  INetURLObject::ENCODE_ALL),
                              nInsert + RTL_CONSTASCII_LENGTH(",host="));
        }
    }

    // Get the (local) RemoteAccessUCP:
    Reference< ucb::XParameterizedContentProvider > xRemoteAccessUcp;
    try
    {
        xRemoteAccessUcp
            = Reference< ucb::XParameterizedContentProvider >(
                  xFactory->
                      createInstance(
                          rtl::OUString(
                              RTL_CONSTASCII_USTRINGPARAM(
                                  "com.sun.star.ucb."
                                      "RemoteAccessContentProvider"))),
                  UNO_QUERY);
    }
    catch (RuntimeException const &) { throw; }
    catch (Exception const &) {}
    if (!xRemoteAccessUcp.is())
    {
        DBG_ERROR("configureUcb(): No RemoteAccessContentProvider service");
        return false;
    }

    // Create an instance of the RemoteAccessUCP specialized on the
    // (template, url) pair:
    rtl::OUString aTemplate(RTL_CONSTASCII_USTRINGPARAM(".*"));
    Reference< ucb::XContentProvider > xInstance;
    try
    {
        xInstance
            = xRemoteAccessUcp->registerInstance(aTemplate, aTheUcbUrl, false);
    }
    catch (IllegalArgumentException const &)
    {
        DBG_ERROR("configureUcb(): Can't registerInstance()");
        return false;
    }
    if (!xInstance.is())
    {
        DBG_ERROR("configureUcb(): Bad registerInstance()");
        return false;
    }

    // Register the specialized RemoteAccessUCP instance at the (local) UCB:
    try
    {
        xUcb->registerContentProvider(xInstance, aTemplate, false);
    }
    catch (ucb::DuplicateProviderException const &)
    {
        DBG_ERROR("configureUcb(): Bad registerContentProvider()");
        try
        {
            xRemoteAccessUcp->deregisterInstance(aTemplate, aTheUcbUrl);
        }
        catch (lang::IllegalArgumentException const &) {}
        return false;
    }
    catch (uno::RuntimeException const &)
    {
        DBG_ERROR("configureUcb(): Bad registerContentProvider()");
        try
        {
            xRemoteAccessUcp->deregisterInstance(aTemplate, aTheUcbUrl);
        }
        catch (lang::IllegalArgumentException const &) {}
        throw;
    }

    return true;
}

//====================================================================

FASTBOOL SfxApplication::Initialize_Impl()
{
#ifdef TLX_VALIDATE
    StgIo::SetErrorLink( LINK( this, SfxStorageErrHdl, Error ) );
#endif

//! FSysEnableSysErrorBox( FALSE ); (pb) replaceable?

    Reference < XDesktop > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    xDesktop->addTerminateListener( new SfxTerminateListener_Impl() );

    if( !CheckTryBuy_Impl() )
    {
        exit(-1);
        return FALSE;;
    }

    Application::EnableAutoHelpId();

    pAppData_Impl->pAppDispatch = new SfxStatusDispatcher;
    pAppData_Impl->pAppDispatch->acquire();

    // SV-Look
    Help::EnableContextHelp();
    Help::EnableExtHelp();
    Help::EnableQuickHelp();

    // falls der IniManager nicht schon durch CreateResManager erzeugt wurde
    if ( !pAppIniMgr )
        pAppIniMgr = CreateIniManager();

    // StarObjects initialisieren
    if ( !SvFactory::Init() )
        ErrorBox( 0, SfxResId(MSG_ERR_SOINIT) ).Execute();

    // Factory f"ur das SfxFrameObject anlegen; da der Pointer in den AppDaten
    // liegt, dieser aber nicht exportierbar ist, mu\s ein exportierbarer
    // Wrapper angelegt werden
    pAppData_Impl->pSfxFrameObjectFactoryPtr = new SfxFrameObjectFactoryPtr;
    pAppData_Impl->pSfxFrameObjectFactoryPtr->pSfxFrameObjectFactory = SfxFrameObject::ClassFactory();
    SvBindStatusCallback::SetProgressCallback( STATIC_LINK( 0, SfxProgress, DefaultBindingProgress ) );
    INetURLHistory::GetOrCreate()->SetLocation( GetIniManager()->Get( SFX_KEY_USERCONFIG_PATH ) );

    // merken, falls Applikation normal gestartet wurde
    if ( pAppData_Impl->bDirectAliveCount )
        SvFactory::IncAliveCount();

    pImp->pEventHdl = new UniqueIndex( 1, 4, 4 );
    InitializeDisplayName_Impl();

#ifdef DBG_UTIL
    // Der SimplerErrorHandler dient Debugzwecken. In der Product werden
    // nichtgehandelte Fehler durch Errorcode 1 an SFX gegeben.
    new SimpleErrorHandler;
#endif
    new SfxErrorHandler(RID_ERRHDL, ERRCODE_AREA_TOOLS, ERRCODE_AREA_LIB1);
    new SfxErrorHandler(
        RID_SO_ERROR_HANDLER, ERRCODE_AREA_SO, ERRCODE_AREA_SO_END);
    new SfxErrorHandler(
        (RID_SJ_START +1), ERRCODE_AREA_JAVA, ERRCODE_AREA_JAVA_END);
    new SfxErrorHandler(
        RID_BASIC_START, ERRCODE_AREA_SBX, ERRCODE_AREA_SBX_END );

    // diverse Pointer
    pImp->pAutoSaveTimer = new Timer;
    String aPickSize = GetIniManager()->Get( SFX_KEY_PICKLIST );
    int nPickSize = !aPickSize.Len() ? 4 : (int) aPickSize.ToInt32();
    SfxPickList_Impl::GetOrCreate( Min(9, nPickSize) );

    /////////////////////////////////////////////////////////////////

    DBG_ASSERT( !pAppDispat, "AppDispatcher already exists" );
    pAppDispat = new SfxDispatcher((SfxDispatcher*)0);
    pSlotPool = new SfxSlotPool;
    pImp->pTbxCtrlFac = new SfxTbxCtrlFactArr_Impl;
    pImp->pStbCtrlFac = new SfxStbCtrlFactArr_Impl;
    pImp->pMenuCtrlFac = new SfxMenuCtrlFactArr_Impl;
    pImp->pViewFrames = new SfxViewFrameArr_Impl;
    pImp->pViewShells = new SfxViewShellArr_Impl;
    pImp->pObjShells = new SfxObjectShellArr_Impl;
    nInterfaces = SFX_INTERFACE_APP+8;
    pInterfaces = new SfxInterface*[nInterfaces];
    memset( pInterfaces, 0, sizeof(SfxInterface*) * nInterfaces );

    pAppData_Impl->pAppCfg = new SfxConfigManager;
    pAppData_Impl->pAppCfg->Activate( pCfgMgr );

    pAcceleratorMgr = new SfxAcceleratorManager;

    pImageMgr = new SfxImageManager;
#if SUPD<608
    pOptions = new SfxOptions;
#endif
    SfxNewHdl* pNewHdl = SfxNewHdl::GetOrCreate();

    // Die Strings muessen leider zur Laufzeit gehalten werden, da wir bei
    // einer ::com::sun::star::uno::Exception keine Resourcen mehr laden duerfen.
    pImp->aMemExceptionString = pNewHdl->GetMemExceptionString();
    pImp->aResWarningString       = String( SfxResId( STR_RESWARNING ) );
    pImp->aResExceptionString     = String( SfxResId( STR_RESEXCEPTION ) );
    pImp->aSysResExceptionString  = String( SfxResId( STR_SYSRESEXCEPTION ) );

    Registrations_Impl();

    RegisterEvent(SFX_EVENT_STARTAPP, String(SfxResId(STR_EVENT_STARTAPP)));
    RegisterEvent(SFX_EVENT_CLOSEAPP, String(SfxResId(STR_EVENT_CLOSEAPP)));
    RegisterEvent(SFX_EVENT_CREATEDOC,String(SfxResId(STR_EVENT_CREATEDOC)));
    RegisterEvent(SFX_EVENT_OPENDOC,  String(SfxResId(STR_EVENT_OPENDOC)));
    RegisterEvent(SFX_EVENT_SAVEASDOC,  String(SfxResId(STR_EVENT_SAVEASDOC)));
    RegisterEvent(SFX_EVENT_SAVEASDOCDONE,  String(SfxResId(STR_EVENT_SAVEASDOCDONE)));
    RegisterEvent(SFX_EVENT_SAVEDOC,  String(SfxResId(STR_EVENT_SAVEDOC)));
    RegisterEvent(SFX_EVENT_SAVEDOCDONE,  String(SfxResId(STR_EVENT_SAVEDOCDONE)));
    RegisterEvent(SFX_EVENT_PREPARECLOSEDOC,  String(SfxResId(STR_EVENT_PREPARECLOSEDOC)));
    RegisterEvent(SFX_EVENT_CLOSEDOC,  String(SfxResId(STR_EVENT_CLOSEDOC)));
    RegisterEvent(SFX_EVENT_ACTIVATEDOC,  String(SfxResId(STR_EVENT_ACTIVATEDOC)));
    RegisterEvent(SFX_EVENT_DEACTIVATEDOC,  String(SfxResId(STR_EVENT_DEACTIVATEDOC)));
    RegisterEvent(SFX_EVENT_PRINTDOC,   String(SfxResId(STR_EVENT_PRINTDOC)));
    RegisterEvent(SFX_EVENT_ONERROR,    String(SfxResId(STR_EVENT_ONERROR)));
    RegisterEvent(SFX_EVENT_NEWMESSAGE, String(SfxResId(STR_EVENT_NEWMESSAGE)));

    // Filtercontainer fuer ::com::sun::star::sdbcx::User Overrides registrieren
    SfxFilterContainer* pDefaults = new SfxFilterContainer( DEFINE_CONST_UNICODE(SFX_FCONTNR_REDIRECTS) );
    SfxFilterMatcher& rMatcher = GetFilterMatcher();
    rMatcher.AddContainer( pDefaults );
    pDefaults->LoadFilters(
        DEFINE_CONST_UNICODE(SFX_FCONTNR_REDIRECTS), sal_False,
        SFX_FILTER_REDIRECT | SFX_FILTER_IMPORT | SFX_FILTER_ALIEN |
        SFX_FILTER_NOTINFILEDLG | SFX_FILTER_NOTINCHOOSER );

    if( _nFeatures & SFX_FEATURE_SCALC &&
        _nFeatures & SFX_FEATURE_SIMPRESS &&
        _nFeatures & SFX_FEATURE_SDRAW &&
        _nFeatures & SFX_FEATURE_SIMAGE &&
        _nFeatures & SFX_FEATURE_SCHART &&
        _nFeatures & SFX_FEATURE_SMATH &&
        _nFeatures & SFX_FEATURE_SWRITER )
    {
        // Office Filter registrieren
        SfxFilterContainer* pContainer =
            new SfxFilterContainer( DEFINE_CONST_UNICODE("soffice4") );
        pContainer->AddFilter( new SfxOfficeFilter( pContainer ), 0 );
        rMatcher.AddContainer( pContainer );
    }

    // Subklasse initialisieren
    bDowning = sal_False;
    bInInit = sal_True;
    Init();

    // get CHAOS item pool...
    pAppData_Impl->pPool = NoChaos::GetItemPool();
    SetPool( pAppData_Impl->pPool );

    // If '-ucb=...' was specified on the command line, configure the UCB now:
    if (pAppData_Impl->aUcbUrl.Len() != 0
        && !configureUcb(pAppData_Impl->aUcbUrl))
    {
        exit(-1);
        return FALSE;
    }

    InsertLateInitHdl( LINK(this, SfxApplication,LateInitNewMenu_Impl) );
    InsertLateInitHdl( LINK(this, SfxApplication,LateInitWizMenu_Impl) );
    InsertLateInitHdl( LINK(pNewHdl, SfxNewHdl, InitMem_Impl) );
    InsertLateInitHdl( LINK(this, SfxApplication,SpecialService_Impl) );
    InsertLateInitHdl( STATIC_LINK( pAppData_Impl, SfxAppData_Impl, CreateDocumentTemplates ) );

    bInInit = sal_False;
    if ( bDowning )
        return sal_False;

    // App-Dispatcher aufbauen
    pAppDispat->Push(*this);
    pAppDispat->DoActivate_Impl( sal_True );

    //  if not done in Init(), load the configuration
    if ( !pImp->bConfigLoaded )
        LoadConfig();
    SvtSaveOptions aSaveOptions;
    pImp->pAutoSaveTimer->SetTimeout( aSaveOptions.GetAutoSaveTime() * 60000 );
    pImp->pAutoSaveTimer->SetTimeoutHdl( LINK( pApp, SfxApplication, AutoSaveHdl_Impl ) );

    // App-StartEvent
    NotifyEvent(SfxEventHint(SFX_EVENT_STARTAPP), sal_False);

//(dv)  if ( !pAppData_Impl->bBean )
//(mba)        doFirstTimeInit();

    Application::PostUserEvent( LINK( this, SfxApplication, OpenClients_Impl ) );

    DELETEZ(pImp->pIntro);

    // start LateInit
    SfxAppData_Impl *pAppData = Get_Impl();
    pAppData->aLateInitTimer.SetTimeout( 250 );
    pAppData->aLateInitTimer.SetTimeoutHdl( LINK( this, SfxApplication, LateInitTimerHdl_Impl ) );
    pAppData->aLateInitTimer.Start();

/*! (pb) no help at this time
    Application::PostUserEvent( LINK( this, SfxApplication, StartWelcomeScreenHdl_Impl ) );
*/
    return sal_True;
}

IMPL_LINK( SfxApplication, SpecialService_Impl, void*, pVoid )
{
    if ( pAppData_Impl->bBean )
            return 0;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMgr( ::comphelper::getProcessServiceFactory() );
    ::com::sun::star::uno::Reference< ::com::sun::star::installation::XInstallationCheck >  xInst( xMgr->createInstance( DEFINE_CONST_UNICODE("com.sun.star.installation.FontCheck") ), ::com::sun::star::uno::UNO_QUERY );
    if ( xInst.is() )
        xInst->checkWithDialog( sal_False );

    String aWizard = GetIniManager()->Get( DEFINE_CONST_UNICODE("Common"), 0, 0, DEFINE_CONST_UNICODE("RunWizard") );
    sal_Bool bRunWizard = (sal_Bool) (sal_uInt16) aWizard.ToInt32();
    if ( bRunWizard )
    {
        SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
        SfxStringItem aMacro( SID_FILE_NAME, DEFINE_CONST_UNICODE("macro://#InternetSetup.Run.Main()") );
//(mba)        pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aMacro, &aReferer, 0L );
        GetIniManager()->DeleteKey( DEFINE_CONST_UNICODE("Common"), DEFINE_CONST_UNICODE("RunWizard") );
        GetIniManager()->Flush();
    }
    else if ( !pAppData_Impl->bBean )
    {
        // StarOffice registration
        INetURLObject aORegObj( GetIniManager()->Get( SFX_KEY_USERCONFIG_PATH ), INET_PROT_FILE );
        aORegObj.insertName( DEFINE_CONST_UNICODE( "oreg.ini" ) );
        Config aCfg( aORegObj.PathToFileName() );
        aCfg.SetGroup( "reg" );
        sal_uInt16 nRegKey = (sal_uInt16) aCfg.ReadKey( "registration", "0" ).ToInt32();
        if( nRegKey == 0 )
            GetAppDispatcher_Impl()->Execute(SID_ONLINE_REGISTRATION_DLG, SFX_CALLMODE_ASYNCHRON);
    }

    return 0;
}

