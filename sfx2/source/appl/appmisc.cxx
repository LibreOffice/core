/*************************************************************************
 *
 *  $RCSfile: appmisc.cxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:03:16 $
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

#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _VOS_PROCESS_HXX //autogen
#include <vos/process.hxx>
#endif
#ifndef _XCEPTION_HXX_
#include <vos/xception.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_INVALIDREGISTRYEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyValue_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#include <tools/rcid.h>
#include <vos/mutex.hxx>
#include <svtools/pver.hxx>
#pragma hdrstop
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#include <unotools/ucbstreamhelper.hxx>
#include <framework/menuconfiguration.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/bootstrap.hxx>
#include <svtools/moduleoptions.hxx>
#include <osl/file.hxx>

#include "sfxresid.hxx"
#include "app.hxx"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "cfgmgr.hxx"
#include "cfgitem.hxx"
#include "tbxctrl.hxx"
#include "tbxconf.hxx"
#include "stbitem.hxx"
#include "accitem.hxx"
#include "mnuitem.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "request.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "workwin.hxx"
//#include "iodlg.hxx"
#include "intro.hxx"
#include "about.hxx"
#include "fcontnr.hxx"
#include "sfxlocal.hrc"
#include "sfx.hrc"
#include "app.hrc"
#include "templdlg.hxx"
#include "module.hxx"
#include "msgpool.hxx"
#include "topfrm.hxx"
#include "openflag.hxx"
#include "viewsh.hxx"
#include "appimp.hxx"
//#include "bmkmenu.hxx"
#include "objface.hxx"
#include "helper.hxx"   // SfxContentHelper::Kill()

using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//===================================================================

SV_IMPL_PTRARR( SfxTbxCtrlFactArr_Impl, SfxTbxCtrlFactory* );
SV_IMPL_PTRARR( SfxStbCtrlFactArr_Impl, SfxStbCtrlFactory* );
SV_IMPL_PTRARR( SfxMenuCtrlFactArr_Impl, SfxMenuCtrlFactory* );
SV_IMPL_PTRARR( SfxChildWinFactArr_Impl, SfxChildWinFactory* );
SV_IMPL_PTRARR( SfxModuleArr_Impl, SfxModule* );

//===================================================================

#define SfxApplication
#include "sfxslots.hxx"

class SfxSpecialConfigError_Impl
{
    String aError;

public:

    SfxSpecialConfigError_Impl( const String& rStr );
    DECL_LINK( TimerHdl, Timer*);
};


SfxSpecialConfigError_Impl::SfxSpecialConfigError_Impl( const String& rStr ) :
    aError( rStr )
{
    Timer *pTimer = new Timer;
    pTimer->SetTimeoutHdl( LINK(this, SfxSpecialConfigError_Impl, TimerHdl) );
    pTimer->SetTimeout( 0 );
    pTimer->Start();
}

IMPL_LINK( SfxSpecialConfigError_Impl, TimerHdl, Timer*, pTimer )
{
    delete pTimer;
    ErrorBox( 0, WinBits( WB_OK ) , aError ).Execute();
    delete this;
    SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_QUITAPP );
    return 0L;
}

//====================================================================


SFX_IMPL_INTERFACE(SfxApplication,SfxShell,SfxResId(RID_DESKTOP))
{
    SFX_STATUSBAR_REGISTRATION(SfxResId(SFX_ITEMTYPE_STATBAR));
}

//--------------------------------------------------------------------

SfxViewFrame* SfxApplication::CreateViewFrame(
    SfxObjectShell& rDoc, sal_uInt16 nViewId, sal_Bool bHidden )
{
    SfxItemSet *pSet = rDoc.GetMedium()->GetItemSet();
    if ( nViewId )
        pSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );
    if ( bHidden )
        pSet->Put( SfxBoolItem( SID_HIDDEN, sal_True ) );

    SfxFrame *pFrame = SfxTopFrame::Create( &rDoc, 0, bHidden );
    return pFrame->GetCurrentViewFrame();
}

void SfxApplication::InitializeDisplayName_Impl()
{
    SfxAppData_Impl* pAppData_Impl = Get_Impl();
    if ( !pAppData_Impl->pLabelResMgr )
        return;

    String aTitle = Application::GetDisplayName();
    if ( !aTitle.Len() )
    {
        OClearableGuard aGuard( OMutex::getGlobalMutex() );

        // create version string
/*!!! (pb) don't show a version number at the moment
        USHORT nProductVersion = ProductVersion::GetVersion().ToInt32();
        String aVersion( String::CreateFromInt32( nProductVersion / 10 ) );
        aVersion += 0x002E ; // 2Eh ^= '.'
        aVersion += ( String::CreateFromInt32( nProductVersion % 10 ) );
*/
        // load application title
        aTitle = String( ResId( RID_APPTITLE, pAppData_Impl->pLabelResMgr ) );
        // merge version into title
        aTitle.SearchAndReplaceAscii( "$(VER)", String() /*aVersion*/ );

        aGuard.clear();

#ifndef PRODUCT
        ::rtl::OUString aDefault;
        aTitle += DEFINE_CONST_UNICODE(" [");

        String aVerId( utl::Bootstrap::getBuildIdData( aDefault ));
        aTitle += aVerId;
        aTitle += ']';
#endif
        if ( GetDemoKind() == SFX_DEMOKIND_DEMO )
            aTitle += DEFINE_CONST_UNICODE(" (Demo Version)");

        Application::SetDisplayName( aTitle );
    }
}

//--------------------------------------------------------------------

void SfxApplication::OpenClients()
{
}

//--------------------------------------------------------------------

void SfxApplication::FillStatusBar( StatusBar& rBar )

/*  [Beschreibung]

    Diese virtuelle Factory-Methode wird vom SFx gerufen, um die
    StatusBar f"ur das Applikationsfenster zu erzeugen. Im Ggs. zu
    den anderen Standard-Controllern Menu, Accelerator und ToolBoxen
    kann die Status-Zeile nur mit dieser Factory erzeugt werden,
    da in der SV-Resource keine StatusBar-Items angegeben werden k"oennen.
*/

{
    rBar.InsertItem( SID_EXPLORER_SELECTIONSIZE, 200, SIB_IN|SIB_LEFT|SIB_AUTOSIZE );
}

//--------------------------------------------------------------------

void SfxApplication::IntroSlide()

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx gerufen, um die bei einem
    Mehrbild-Intro das n"achte Bild anzuzeigen. Sie sollte w"ahrend
    des Startups mehrmals gerufen werden, wenn l"angere Initialisierungen
    ausgef"uhrt werden.
*/

{
    if ( pImp->pIntro )
        pImp->pIntro->Slide();
}

//--------------------------------------------------------------------

PrinterDialog* SfxApplication::CreatePrinterDialog()

/*  [Beschreibung]

    Diese virtuelle Factory-Methode wird vom SFx gerufen, um einen
    Applikations-spezifischen PrinterDialog zu erzeugen.

    Die Default-Implementierung liefert einen 0-Pointer zur"uck.
*/

{
    return 0;
}
//--------------------------------------------------------------------

ModalDialog* SfxApplication::CreateAboutDialog()

/*  [Beschreibung]

    Diese virtuelle Factory-Methode wird vom SFx gerufen, um einen
    Modul-spezifischen AboutDialog zu erzeugen.

    Die Default-Implementierung erzeugt einen AboutDialog aus
    der Resource 'ModalDialog RID_DEFAULTABOUT' und verwendet
    zus"atzlich einen String RID_BUILDVERSION, f"ur die Update-Version
    der Applikation. Letztere wird angezeigt, sobald der Anwender
    eine spezielle, im Dialog definierte, Sequenz von Zeichen
    eingibt (z.B. 'Ctrl' 's' 'f' 'x').
*/

{
    // Buildversion suchen
    ::rtl::OUString aDefault;
    String          aVerId( utl::Bootstrap::getBuildIdData( aDefault ));

    if ( aVerId.Len() == 0 )
        DBG_ERROR( "No BUILDID in bootstrap file" );

    String aVersion( '[' );
    ( aVersion += aVerId ) += ']';

    // About-Dialog suchen
    ResId aDialogResId( RID_DEFAULTABOUT, pAppData_Impl->pLabelResMgr );
    ResMgr* pResMgr = pAppData_Impl->pLabelResMgr->IsAvailable(
                        aDialogResId.SetRT( RSC_MODALDIALOG ) )
                    ? pAppData_Impl->pLabelResMgr
                    : 0;
    aDialogResId.SetResMgr( pResMgr );
    if ( !Resource::GetResManager()->IsAvailable( aDialogResId ) )
        DBG_ERROR( "No RID_DEFAULTABOUT in label-resource-dll" );

    // About-Dialog anzeigen
    AboutDialog* pDlg = new AboutDialog( 0, aDialogResId, aVersion );
    return pDlg;
}

//--------------------------------------------------------------------

void SfxApplication::HandleConfigError_Impl( sal_uInt16 nErrorCode ) const
{
    sal_uInt16 nResId = 0;
    switch(nErrorCode)
    {
        case SfxConfigManager::ERR_READ:
            nResId = MSG_ERR_READ_CFG;
            break;
        case SfxConfigManager::ERR_WRITE:
            nResId = MSG_ERR_WRITE_CFG;
            break;
        case SfxConfigManager::ERR_OPEN:
            nResId = MSG_ERR_OPEN_CFG;
            break;
        case SfxConfigManager::ERR_FILETYPE:
            nResId = MSG_ERR_FILETYPE_CFG;
            break;
        case SfxConfigManager::ERR_VERSION:
            nResId = MSG_ERR_VERSION_CFG;
            break;
    }
    DBG_ASSERT(nResId != 0, "unbekannte Fehlerkonstante aus Konfiguration");
    if(nResId)
    {
        ErrorBox aErrorBox(NULL, SfxResId(nResId));
        aErrorBox.Execute();
    }
}

//--------------------------------------------------------------------
#ifdef WNT
extern String GetUserID();
#endif

//------------------------------------------------------------------------

SfxProgress* SfxApplication::GetProgress() const

/*  [Beschreibung]

    Liefert den f"ur die gesamte Applikation laufenden SfxProgress
    oder 0, falls keiner f"ur die gesamte Applikation l"auft.


    [Querverweise]

    <SfxProgress::GetActiveProgress(SfxViewFrame*)>
    <SfxViewFrame::GetProgress()const>
*/

{
    return pAppData_Impl->pProgress;
}

//------------------------------------------------------------------------

void SfxApplication::ToolboxExec_Impl( SfxRequest &rReq )
{
}

//------------------------------------------------------------------------


void SfxApplication::ToolboxState_Impl( SfxItemSet &rSet )
{
}

//------------------------------------------------------------------------

SvUShorts* SfxApplication::GetDisabledSlotList_Impl()
{
    sal_Bool bError = sal_False;
    SvUShorts* pList = pAppData_Impl->pDisabledSlotList;
    if ( !pList )
    {
        // Gibt es eine Slotdatei ?
        INetURLObject aUserObj( SvtPathOptions().GetUserConfigPath() );
        aUserObj.insertName( DEFINE_CONST_UNICODE( "slots.cfg" ) );
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aUserObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
        if ( !pStream || pStream->GetError() == ERRCODE_IO_NOTEXISTS )
        {
            delete pStream;
            INetURLObject aObj( SvtPathOptions().GetConfigPath() );
            aObj.insertName( DEFINE_CONST_UNICODE( "slots.cfg" ) );
            pStream = ::utl::UcbStreamHelper::CreateStream( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
        }

        BOOL bSlotsEnabled = SvtInternalOptions().SlotCFGEnabled();
        BOOL bSlots = ( pStream && !pStream->GetError() );
        if( bSlots && bSlotsEnabled )
        {
            // SlotDatei einlesen
            String aTitle;
            pStream->ReadByteString(aTitle);
            if ( aTitle.CompareToAscii("SfxSlotFile" ) == COMPARE_EQUAL )
            {
                sal_uInt16 nCount;
                (*pStream) >> nCount;
                pList = pAppData_Impl->pDisabledSlotList =
                        new SvUShorts( nCount < 255 ? (sal_Int8) nCount : 255, 255 );

                sal_uInt16 nSlot;
                for ( sal_uInt16 n=0; n<nCount; n++ )
                {
                    (*pStream) >> nSlot;
                    pList->Insert( nSlot, n );
                }

                pStream->ReadByteString(aTitle);
                if ( aTitle.CompareToAscii("END" ) != COMPARE_EQUAL || pStream->GetError() )
                {
                    // Lesen schief gegangen
                    DELETEZ( pList );
                    bError = sal_True;
                }
            }
            else
            {
                // Streamerkennung  fehlgeschlagen
                bError = sal_True;
            }
        }
        else if ( bSlots != bSlotsEnabled )
        {
            // Wenn kein Slotlist-Eintrag, dann darf auch keine SlotDatei
            // vorhanden sein
            bError = sal_True;
        }

        delete pStream;
    }
    else if ( pList == (SvUShorts*) -1L )
    {
        return NULL;
    }

    if ( !pList )
        pAppData_Impl->pDisabledSlotList = (SvUShorts*) -1L;

    if ( bError )
    {
        // Wenn ein Sloteintrag vorhanden ist, aber keine oder eine fehlerhafte
        // SlotDatei, oder aber eine Slotdatei, aber kein Sloteintrag, dann
        // gilt dies als fehlerhafte Konfiguration
        new SfxSpecialConfigError_Impl( String( SfxResId( RID_SPECIALCONFIG_ERROR ) ) );
    }

       return pList;
}


Config* SfxApplication::GetFilterIni()
{
    if ( !pAppData_Impl->pFilterIni )
    {
        OStartupInfo aInfo;
        ::rtl::OUString aApplicationName;
        // get the path of the executable
        if ( aInfo.getExecutableFile( aApplicationName ) == OStartupInfo::E_None )
        {
            // cut the name of the executable
            ::rtl::OUString aIniFile = aApplicationName.copy( 0, aApplicationName.lastIndexOf( '/' ) );
            // append the name of the filter ini
            aIniFile += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/install.ini" ) );
            // and create the Config instance
            pAppData_Impl->pFilterIni = new Config( aIniFile );
        }
    }

    return pAppData_Impl->pFilterIni;
}


SfxModule* SfxApplication::GetActiveModule( SfxViewFrame *pFrame ) const
{
    if ( !pFrame )
        pFrame = SfxViewFrame::Current();
    SfxObjectShell* pSh = 0;
    if( pFrame ) pSh = pFrame->GetObjectShell();
    return pSh ? pSh->GetModule() : 0;
}

SfxModule* SfxApplication::GetModule_Impl()
{
    SfxModule* pModule = GetActiveModule();
    if ( !pModule )
        pModule = GetActiveModule( SfxViewFrame::GetFirst( FALSE ) );
    if( pModule )
        return pModule;
    else
    {
        DBG_ERROR( "No module!" );
        return NULL;
    }
}


SfxSlotPool& SfxApplication::GetSlotPool( SfxViewFrame *pFrame ) const
{
    SfxModule *pMod = GetActiveModule( pFrame );
    if ( pMod && pMod->GetSlotPool() )
        return *pMod->GetSlotPool();
    else
        return *pSlotPool;
}


SfxAcceleratorManager* SfxApplication::GetAcceleratorManager() const
{
    // Accelerator immer mit ContainerBindings
    SfxViewFrame *pFrame = pViewFrame;
    if ( !pFrame )
        return pAcceleratorMgr;

    while ( pFrame->GetParentViewFrame_Impl() )
        pFrame = pFrame->GetParentViewFrame_Impl();

    SfxViewShell* pSh = 0;
    if( pFrame )
        pSh = pFrame->GetViewShell();
    if ( pSh )
    {
        SfxAcceleratorManager *pMgr = pSh->GetAccMgr_Impl();
        if ( pMgr )
            return pMgr;
    }

    return pAcceleratorMgr;
}

SfxAcceleratorManager* SfxApplication::GetGlobalAcceleratorManager() const
{
    return pAcceleratorMgr;
}

ISfxTemplateCommon* SfxApplication::GetCurrentTemplateCommon()
{
    if( pAppData_Impl->pTemplateCommon )
        return pAppData_Impl->pTemplateCommon;
    return NULL;
}


ISfxTemplateCommon* SfxApplication::GetCurrentTemplateCommon( SfxBindings& rBindings )
{
    if( pAppData_Impl->pTemplateCommon )
        return pAppData_Impl->pTemplateCommon;
    SfxChildWindow *pChild = rBindings.GetWorkWindow_Impl()->GetChildWindow_Impl(
        SfxTemplateDialogWrapper::GetChildWindowId() );
    if ( pChild )
        return ((SfxTemplateDialog*) pChild->GetWindow())->GetISfxTemplateCommon();
    return 0;
}
/*
long Select_Impl( void* pHdl, void* pVoid )
{
    Menu* pMenu = (Menu*) pVoid;
    String aURL( pMenu->GetItemCommand( pMenu->GetCurItemId() ) );
    if( !aURL.Len() )
        return 0;

    SfxDispatcher* pDispatcher = ((SfxBindings*)pHdl)->GetDispatcher_Impl();
    if ( ! pDispatcher )
        return 0;
    Reference<com::sun::star::frame::XFrame> xFrame( pDispatcher->GetFrame()->GetFrame()->GetFrameInterface() );
    if (! xFrame.is())
    {
        Reference < ::com::sun::star::frame::XFramesSupplier > xDesktop =
                Reference < ::com::sun::star::frame::XFramesSupplier >( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
        xFrame = Reference < ::com::sun::star::frame::XFrame > ( xDesktop->getActiveFrame() );
        if ( !xFrame.is() )
            xFrame = Reference < ::com::sun::star::frame::XFrame >( xDesktop, UNO_QUERY );
    }

    URL aTargetURL;
    aTargetURL.Complete = aURL;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    Reference < ::com::sun::star::frame::XDispatchProvider > xProv( xFrame, UNO_QUERY );
    Reference < ::com::sun::star::frame::XDispatch > xDisp;
    if ( xProv.is() )
    {
        if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
            xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        else
        {
            ::rtl::OUString aTargetFrame( ::rtl::OUString::createFromAscii("_blank") );
            ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( pMenu->GetCurItemId() );

            if ( pMenuAttributes )
                aTargetFrame = pMenuAttributes->aTargetFrame;

            xDisp = xProv->queryDispatch( aTargetURL, aTargetFrame , 0 );
        }
    }
    if ( xDisp.is() )
    {
        xDisp->dispatch( aTargetURL, Sequence<PropertyValue>() );
    }

    return TRUE;
}
*/
SfxMenuBarManager* SfxApplication::GetMenuBarManager() const
{
/*
    SfxViewFrame *pFrame = SfxViewFrame::Current();
    if ( pFrame )
        return pFrame->GetViewShell()->GetMenuBar_Impl();
    else
        return 0;
*/
    return 0;
}

SfxCancelManager *SfxApplication::GetCancelManager() const
{
    if ( !pAppData_Impl->pCancelMgr )
    {
        pAppData_Impl->pCancelMgr = new SfxCancelManager;
        pAppData_Impl->StartListening( *pAppData_Impl->pCancelMgr );
    }
    return pAppData_Impl->pCancelMgr;
}


