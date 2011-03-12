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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "impviewframe.hxx"
#include "statcach.hxx"
#include "viewfac.hxx"
#include "workwin.hxx"

#include "sfx2/app.hxx"
#include "sfx2/bindings.hxx"
#include "sfx2/ctrlitem.hxx"
#include "sfx2/dispatch.hxx"
#include "sfx2/docfac.hxx"
#include "sfx2/docfile.hxx"
#include "sfx2/objitem.hxx"
#include "sfx2/objsh.hxx"
#include "sfx2/request.hxx"
#include "sfx2/viewfrm.hxx"
#include "sfx2/viewsh.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/asynclink.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/rectitem.hxx>
#include <svl/stritem.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XComponent;

//------------------------------------------------------------------------

static ::rtl::OUString GetModuleName_Impl( const ::rtl::OUString& sDocService )
{
    uno::Reference< container::XNameAccess > xMM( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.ModuleManager"))), uno::UNO_QUERY );
    ::rtl::OUString sVar;
    if ( !xMM.is() )
        return sVar;

    try
    {
        ::comphelper::NamedValueCollection aAnalyzer( xMM->getByName( sDocService ) );
        sVar = aAnalyzer.getOrDefault( "ooSetupFactoryUIName", ::rtl::OUString() );
    }
    catch( uno::Exception& )
    {
        sVar = ::rtl::OUString();
    }

    return sVar;
}

//--------------------------------------------------------------------
void SfxFrameViewWindow_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        SfxObjectShell* pDoc = pFrame->GetObjectShell();
        if ( pDoc && !pFrame->IsVisible() )
            pFrame->Show();

        pFrame->Resize();
    }
    else
        Window::StateChanged( nStateChange );
}

void SfxFrameViewWindow_Impl::Resize()
{
    if ( IsReallyVisible() || IsReallyShown() || GetOutputSizePixel().Width() )
        pFrame->Resize();
}

static String _getTabString()
{
    String result;

    Reference < XMaterialHolder > xHolder(
        ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.tab.tabreg") ), UNO_QUERY );
    if (xHolder.is())
    {
        rtl::OUString aTabString;
        Sequence< NamedValue > sMaterial;
        if (xHolder->getMaterial() >>= sMaterial) {
            for (int i=0; i < sMaterial.getLength(); i++) {
                if ((sMaterial[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("title"))) &&
                    (sMaterial[i].Value >>= aTabString))
                {
                    result += ' ';
                    result += String(aTabString);
                }
            }
        }
    }
    return result;
}

//========================================================================

//--------------------------------------------------------------------
String SfxViewFrame::UpdateTitle()

/*  [Description]

    With this method, can the SfxViewFrame be forced to immediately provide
    the new title from the <SfxObjectShell>.

    [Note]

    This is for example necessary if one listens to the SfxObjectShell as
    SfxListener and then react on the <SfxSimpleHint> SFX_HINT_TITLECHANGED,
    then query the title of his views. However these views (SfxTopViewFrames)
    are  also SfxListener and because the order of notifications might not be
    fixed, the title update will be enforced in advance.

    [Example]

    void SwDocShell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
    {
        if ( rHint.IsA(TYPE(SfxSimpleHint)) )
        {
            switch( ( (SfxSimpleHint&) rHint ).GetId() )
            {
                case SFX_HINT_TITLECHANGED:
                    for ( SfxViewFrame *pTop = SfxViewFrame::GetFirst( this );
                          pTop;
                          pTop = SfxViewFrame::GetNext( this );
                    {
                        pTop->UpdateTitle();
                        ... pTop->GetName() ...
                    }
                    break;
                ...
            }
        }
    }
*/

{
    DBG_CHKTHIS(SfxViewFrame, 0);

    const SfxObjectFactory &rFact = GetObjectShell()->GetFactory();
    pImp->aFactoryName = String::CreateFromAscii( rFact.GetShortName() );

    SfxObjectShell *pObjSh = GetObjectShell();
    if ( !pObjSh )
        return String();

//    if  ( pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
//        // No UpdateTitle with Embedded-ObjectShell
//        return String();

    const SfxMedium *pMedium = pObjSh->GetMedium();
    String aURL;
    GetFrame();  // -Wall required??
    if ( pObjSh->HasName() )
    {
        INetURLObject aTmp( pMedium->GetName() );
        aURL = aTmp.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    }

    if ( aURL != pImp->aActualURL )
        // URL has changed
        pImp->aActualURL = aURL;

    // Is there another view?
    sal_uInt16 nViews=0;
    for ( SfxViewFrame *pView= GetFirst(pObjSh);
          pView && nViews<2;
          pView = GetNext(*pView,pObjSh) )
        if ( ( pView->GetFrameType() & SFXFRAME_HASTITLE ) &&
             !IsDowning_Impl())
            nViews++;

    // Window Title
    String aTitle;
    if ( nViews == 2 || pImp->nDocViewNo > 1 )
        // Then attach the number
        aTitle = pObjSh->UpdateTitle( NULL, pImp->nDocViewNo );
    else
        aTitle = pObjSh->UpdateTitle();

    // SbxObjects name
    String aSbxName = pObjSh->SfxShell::GetName();
    if ( IsVisible() )
    {
        aSbxName += ':';
        aSbxName += String::CreateFromInt32(pImp->nDocViewNo);
    }

    SetName( aSbxName );
    pImp->aFrameTitle = aTitle;
    GetBindings().Invalidate( SID_FRAMETITLE );
    GetBindings().Invalidate( SID_CURRENT_URL );

    ::rtl::OUString aProductName;
    ::utl::ConfigManager::GetDirectConfigProperty(::utl::ConfigManager::PRODUCTNAME) >>= aProductName;

    aTitle += String::CreateFromAscii( " - " );
    aTitle += String(aProductName);
    aTitle += ' ';
    ::rtl::OUString aDocServiceName( GetObjectShell()->GetFactory().GetDocumentServiceName() );
    aTitle += String( GetModuleName_Impl( aDocServiceName ) );
#ifdef DBG_UTIL
    ::rtl::OUString aDefault;
    aTitle += DEFINE_CONST_UNICODE(" [");
    String aVerId( utl::Bootstrap::getBuildIdData( aDefault ));
    aTitle += aVerId;
    aTitle += ']';
#endif

    // append TAB string if available
    aTitle += _getTabString();

    GetBindings().Invalidate( SID_NEWDOCDIRECT );

    /* AS_TITLE
    Window* pWindow = GetFrame()->GetTopWindow_Impl();
    if ( pWindow && pWindow->GetText() != aTitle )
        pWindow->SetText( aTitle );
    */
    return aTitle;
}

void SfxViewFrame::Exec_Impl(SfxRequest &rReq )
{
    // If presently the shells are replaced...
    if ( !GetObjectShell() || !GetViewShell() )
        return;

    switch ( rReq.GetSlot() )
    {
        case SID_SHOWPOPUPS :
        {
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, SID_SHOWPOPUPS, FALSE);
            BOOL bShow = pShowItem ? pShowItem->GetValue() : TRUE;
            SFX_REQUEST_ARG(rReq, pIdItem, SfxUInt16Item, SID_CONFIGITEMID, FALSE);
            USHORT nId = pIdItem ? pIdItem->GetValue() : 0;

            SfxWorkWindow *pWorkWin = GetFrame().GetWorkWindow_Impl();
            if ( bShow )
            {
                // First, make the floats viewable
                pWorkWin->MakeChildsVisible_Impl( bShow );
                GetDispatcher()->Update_Impl( TRUE );

                // Then view it
                GetBindings().HidePopups( !bShow );
            }
            else
            {
                // Hide all
                SfxBindings *pBind = &GetBindings();
                while ( pBind )
                {
                    pBind->HidePopupCtrls_Impl( !bShow );
                    pBind = pBind->GetSubBindings_Impl();
                }

                pWorkWin->HidePopups_Impl( !bShow, TRUE, nId );
                pWorkWin->MakeChildsVisible_Impl( bShow );
            }

            Invalidate( rReq.GetSlot() );
            rReq.Done();
            break;
        }

        case SID_ACTIVATE:
        {
            MakeActive_Impl( TRUE );
            rReq.SetReturnValue( SfxObjectItem( 0, this ) );
            break;
        }

        case SID_NEWDOCDIRECT :
        {
            SFX_REQUEST_ARG( rReq, pFactoryItem, SfxStringItem, SID_NEWDOCDIRECT, FALSE);
            String aFactName;
            if ( pFactoryItem )
                aFactName = pFactoryItem->GetValue();
            else if ( pImp->aFactoryName.Len() )
                aFactName = pImp->aFactoryName;
            else
            {
                OSL_FAIL("Missing argument!");
                break;
            }

            SfxRequest aReq( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, GetPool() );
            String aFact = String::CreateFromAscii("private:factory/");
            aFact += aFactName;
            aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aFact ) );
            aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, &GetFrame() ) );
            aReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii( "_blank" ) ) );
            SFX_APP()->ExecuteSlot( aReq );
            const SfxViewFrameItem* pItem = PTR_CAST( SfxViewFrameItem, aReq.GetReturnValue() );
            if ( pItem )
                rReq.SetReturnValue( SfxFrameItem( 0, pItem->GetFrame() ) );
            break;
        }

        case SID_CLOSEWIN:
        {
            // disable CloseWin, if frame is not a task
            Reference < XCloseable > xTask( GetFrame().GetFrameInterface(),  UNO_QUERY );
            if ( !xTask.is() )
                break;

            if ( GetViewShell()->PrepareClose() )
            {
                // More Views on the same Document?
                SfxObjectShell *pDocSh = GetObjectShell();
                int bOther = sal_False;
                for ( const SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocSh );
                      !bOther && pFrame;
                      pFrame = SfxViewFrame::GetNext( *pFrame, pDocSh ) )
                    bOther = (pFrame != this);

                // Document only needs to be queried, if no other View present.
                sal_Bool bClosed = sal_False;
                sal_Bool bUI = TRUE;
                if ( ( bOther || pDocSh->PrepareClose( bUI ) ) )
                {
                    if ( !bOther )
                        pDocSh->SetModified( FALSE );
                    rReq.Done(); // Must call this before Close()!
                    bClosed = sal_False;
                    try
                    {
                        xTask->close(sal_True);
                        bClosed = sal_True;
                    }
                    catch( CloseVetoException& )
                    {
                        bClosed = sal_False;
                    }
                }

                rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bClosed ));
            }
            return;
        }
    }

    rReq.Done();
}

void SfxViewFrame::GetState_Impl( SfxItemSet &rSet )
{
    SfxObjectShell *pDocSh = GetObjectShell();

    if ( !pDocSh )
        return;

    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges, "Set without Range");
    while ( *pRanges )
    {
        for ( sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich )
        {
            switch(nWhich)
            {
            case SID_NEWDOCDIRECT :
            {
                if ( pImp->aFactoryName.Len() )
                {
                    String aFact = String::CreateFromAscii("private:factory/");
                    aFact += pImp->aFactoryName;
                    rSet.Put( SfxStringItem( nWhich, aFact ) );
                }
                break;
            }

            case SID_NEWWINDOW:
                rSet.DisableItem(nWhich);
                break;

            case SID_CLOSEWIN:
            {
                // disable CloseWin, if frame is not a task
                Reference < XCloseable > xTask( GetFrame().GetFrameInterface(),  UNO_QUERY );
                if ( !xTask.is() )
                    rSet.DisableItem(nWhich);
                break;
            }

            case SID_SHOWPOPUPS :
                break;

            case SID_OBJECT:
                if ( GetViewShell() && GetViewShell()->GetVerbs().getLength() && !GetObjectShell()->IsInPlaceActive() )
                {
                    uno::Any aAny;
                    aAny <<= GetViewShell()->GetVerbs();
                    rSet.Put( SfxUnoAnyItem( USHORT( SID_OBJECT ), aAny ) );
                }
                else
                    rSet.DisableItem( SID_OBJECT );
                break;

            default:
                OSL_FAIL( "invalid message-id" );
            }
        }
        ++pRanges;
    }
}

void SfxViewFrame::INetExecute_Impl( SfxRequest &rRequest )
{
    sal_uInt16 nSlotId = rRequest.GetSlot();
    switch( nSlotId )
    {
        case SID_BROWSE_FORWARD:
        case SID_BROWSE_BACKWARD:
            OSL_FAIL( "SfxViewFrame::INetExecute_Impl: SID_BROWSE_FORWARD/BACKWARD are dead!" );
            break;
        case SID_CREATELINK:
        {
/*! (pb) we need new implementation to create a link
*/
            break;
        }
        case SID_FOCUSURLBOX:
        {
            SfxStateCache *pCache = GetBindings().GetAnyStateCache_Impl( SID_OPENURL );
            if( pCache )
            {
                SfxControllerItem* pCtrl = pCache->GetItemLink();
                while( pCtrl )
                {
                    pCtrl->StateChanged( SID_FOCUSURLBOX, SFX_ITEM_UNKNOWN, 0 );
                    pCtrl = pCtrl->GetItemLink();
                }
            }
        }
    }

    // Recording
    rRequest.Done();
}

void SfxViewFrame::INetState_Impl( SfxItemSet &rItemSet )
{
    rItemSet.DisableItem( SID_BROWSE_FORWARD );
    rItemSet.DisableItem( SID_BROWSE_BACKWARD );

    // Add/SaveToBookmark at BASIC-IDE, QUERY-EDITOR etc. disable
    SfxObjectShell *pDocSh = GetObjectShell();
    sal_Bool bPseudo = pDocSh && !( pDocSh->GetFactory().GetFlags() & SFXOBJECTSHELL_HASOPENDOC );
    sal_Bool bEmbedded = pDocSh && pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED;
    if ( !pDocSh || bPseudo || bEmbedded || !pDocSh->HasName() )
        rItemSet.DisableItem( SID_CREATELINK );
}

void SfxViewFrame::SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY )
{
    GetViewShell()->SetZoomFactor( rZoomX, rZoomY );
}

void SfxViewFrame::Activate( sal_Bool bMDI )
{
    DBG_ASSERT(GetViewShell(), "No Shell");
    if ( bMDI )
        pImp->bActive = sal_True;
//(mba): here maybe as in Beanframe NotifyEvent ?!
}

void SfxViewFrame::Deactivate( sal_Bool bMDI )
{
    DBG_ASSERT(GetViewShell(), "No Shell");
    if ( bMDI )
        pImp->bActive = sal_False;
//(mba): here maybe as in Beanframe NotifyEvent ?!
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
