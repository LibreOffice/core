/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <vcl/status.hxx>
#include <vcl/msgbox.hxx>
#include <vos/process.hxx>
#include <vos/xception.hxx>
#include <svl/whiter.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svtools/filter.hxx>
#include <unotools/internaloptions.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/Reference.h>
#include <tools/config.hxx>
#include <tools/rcid.h>
#include <vos/mutex.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <framework/menuconfiguration.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/moduleoptions.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>

#include "sfx2/sfxresid.hxx"
#include <sfx2/app.hxx>
#include "appdata.hxx"
#include "arrdecl.hxx"
#include <sfx2/tbxctrl.hxx>
#include "sfx2/stbitem.hxx"
#include <sfx2/mnuitem.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "workwin.hxx"
#include "intro.hxx"
#include <sfx2/fcontnr.hxx>
#include "sfxlocal.hrc"
#include <sfx2/sfx.hrc>
#include "app.hrc"
#include <sfx2/templdlg.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>
#include "openflag.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/objface.hxx>
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

#define SFX_ITEMTYPE_STATBAR             4

SFX_IMPL_INTERFACE(SfxApplication,SfxShell,SfxResId(RID_DESKTOP))
{
    SFX_STATUSBAR_REGISTRATION(SfxResId(SFX_ITEMTYPE_STATBAR));
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_0);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_1);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_2);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_3);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_4);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_5);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_6);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_7);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_8);
    SFX_CHILDWINDOW_REGISTRATION(SID_DOCKWIN_9);
}

//--------------------------------------------------------------------
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

        sal_Bool bSlotsEnabled = SvtInternalOptions().SlotCFGEnabled();
        sal_Bool bSlots = ( pStream && !pStream->GetError() );
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


SfxModule* SfxApplication::GetModule_Impl()
{
    SfxModule* pModule = SfxModule::GetActiveModule();
    if ( !pModule )
        pModule = SfxModule::GetActiveModule( SfxViewFrame::GetFirst( sal_False ) );
    if( pModule )
        return pModule;
    else
    {
        DBG_ERROR( "No module!" );
        return NULL;
    }
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

SfxResourceManager& SfxApplication::GetResourceManager() const { return *pAppData_Impl->pResMgr; }
sal_Bool  SfxApplication::IsDowning() const { return pAppData_Impl->bDowning; }
SfxDispatcher* SfxApplication::GetAppDispatcher_Impl() { return pAppData_Impl->pAppDispat; }
SfxSlotPool& SfxApplication::GetAppSlotPool_Impl() const { return *pAppData_Impl->pSlotPool; }
//SfxOptions&  SfxApplication::GetOptions() { return *pAppData_Impl->pOptions; }
//const SfxOptions& SfxApplication::GetOptions() const { return *pAppData_Impl->pOptions; }
