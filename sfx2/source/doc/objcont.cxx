/*************************************************************************
 *
 *  $RCSfile: objcont.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:01:09 $
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

#include <com/sun/star/uno/Reference.hxx>

#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif

#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen wg. QueryBox
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#pragma hdrstop

#include <svtools/stritem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/rectitem.hxx>
#include <svtools/urihelper.hxx>
#include <comphelper/processfactory.hxx>

#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#include <math.h>

#include <svtools/saveopt.hxx>
#include <svtools/useroptions.hxx>
#include <unotools/localfilehelper.hxx>

#include "sfxresid.hxx"
#include "stbmgr.hxx"
#include "dinfdlg.hxx"
#include "fltfnc.hxx"
#include "docfac.hxx"
#include "cfgmgr.hxx"
#include "viewsh.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "cfgitem.hxx"
#include "evntconf.hxx"
#include "interno.hxx"
#include "sfxhelp.hxx"
#include "dispatch.hxx"
#include "urlframe.hxx"
#include "printer.hxx"
#include "topfrm.hxx"
#include "basmgr.hxx"
#include "doctempl.hxx"
#include "doc.hrc"
#include "appdata.hxx"
#include "sfxbasemodel.hxx"
#include "accmgr.hxx"
#include "mnumgr.hxx"
#include "imgmgr.hxx"
#include "tbxconf.hxx"
#include "docfile.hxx"
#include "objuno.hxx"
#include "request.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//====================================================================

#define SFX_WINDOWS_STREAM "SfxWindows"
#define SFX_PREVIEW_STREAM "SfxPreview"

//====================================================================

GDIMetaFile* SfxObjectShell::GetPreviewMetaFile( sal_Bool bFullContent ) const
{
    // Nur wenn gerade nicht gedruckt wird, darf DoDraw aufgerufen
    // werden, sonst wird u.U. der Printer abgeschossen !
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
    if ( pFrame && pFrame->GetViewShell() &&
         pFrame->GetViewShell()->GetPrinter() &&
         pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
         return 0;

    GDIMetaFile* pFile = new GDIMetaFile;

    VirtualDevice aDevice;
    aDevice.EnableOutput( FALSE );

    SfxInPlaceObject* pInPlaceObj = GetInPlaceObject();
    DBG_ASSERT( pInPlaceObj, "Ohne Inplace Objekt keine Grafik" );
    if (pInPlaceObj)
    {
        MapMode aMode( pInPlaceObj->GetMapUnit() );
        aDevice.SetMapMode( aMode );
        pFile->SetPrefMapMode( aMode );

        Size aTmpSize;
        sal_Int8 nAspect;
        if ( bFullContent )
        {
            nAspect = ASPECT_CONTENT;
            aTmpSize = pInPlaceObj->GetVisArea( nAspect ).GetSize();
        }
        else
        {
            nAspect = ASPECT_THUMBNAIL;
            aTmpSize = ((SfxObjectShell*)this)->GetFirstPageSize();
        }

        pFile->SetPrefSize( aTmpSize );
        DBG_ASSERT( aTmpSize.Height()*aTmpSize.Width(),
                    "size of first page is 0, overload GetFirstPageSize or set vis-area!" );

        pFile->Record( &aDevice );
        pInPlaceObj->DoDraw(
                &aDevice, Point(0,0), aTmpSize,
                JobSetup(), nAspect );
        pFile->Stop();
    }

    return pFile;
}

FASTBOOL SfxObjectShell::SaveWindows_Impl( SvStorage &rStor ) const
{
    SvStorageStreamRef xStream = rStor.OpenStream( DEFINE_CONST_UNICODE( SFX_WINDOWS_STREAM ),
                                    STREAM_TRUNC | STREAM_STD_READWRITE);
    if ( !xStream )
        return FALSE;

    xStream->SetBufferSize(1024);
    xStream->SetVersion( rStor.GetVersion() );

    // "uber alle Fenster iterieren (aber aktives Window zuletzt)
    SfxViewFrame *pActFrame = SfxViewFrame::Current();
    if ( !pActFrame || pActFrame->GetObjectShell() != this )
        pActFrame = SfxViewFrame::GetFirst(this);

    String aActWinData;
    for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this, TYPE(SfxTopViewFrame) ); pFrame;
            pFrame = SfxViewFrame::GetNext(*pFrame, this, TYPE(SfxTopViewFrame) ) )
    {
        // Bei Dokumenten, die Outplace aktiv sind, kann beim Speichern auch schon die View weg sein!
        if ( pFrame->GetViewShell() )
        {
            SfxTopFrame* pTop = (SfxTopFrame*) pFrame->GetFrame();
            Window* pWin = pTop->GetTopWindow_Impl();

#if SUPD<613//MUSTINI
            char cToken = SfxIniManager::GetToken();
#else
            char cToken = ',';
#endif
            const BOOL bActWin = pActFrame == pFrame;
            String aUserData;
            pFrame->GetViewShell()->WriteUserData(aUserData);

            // assemble ini-data
            String aWinData;
            aWinData += String::CreateFromInt32( pFrame->GetCurViewId() );
            aWinData += cToken;
/*
            if ( !pWin || pWin->IsMaximized() )
                aWinData += SFX_WINSIZE_MAX;
            else if ( pWin->IsMinimized() )
                aWinData += SFX_WINSIZE_MIN;
            else
*/
#if SUPD<613//MUSTINI
            aWinData += SfxIniManager::GetString( pWin->GetPosPixel(), pWin->GetSizePixel() );
#endif
            aWinData += cToken;
            aWinData += aUserData;

            // aktives kennzeichnen
            aWinData += cToken;
            aWinData += bActWin ? '1' : '0';

            // je nachdem merken oder abspeichern
            if ( bActWin  )
                aActWinData = aWinData;
            else
                xStream->WriteByteString( aWinData );
        }
    }

    // aktives Window hinterher
    xStream->WriteByteString( aActWinData );
    return !xStream->GetError();
}

//====================================================================

SfxViewFrame* SfxObjectShell::LoadWindows_Impl( SfxTopFrame *pPreferedFrame )
{
    DBG_ASSERT( pPreferedFrame, "Can't be implemented in StarPortal!" );
    if ( pImp->bLoadingWindows || !pPreferedFrame )
        return NULL;

    DBG_ASSERT( GetMedium(), "A Medium should exist here!");
    if( !GetMedium() )
        return 0;

    // get correct mode
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrame *pPrefered = pPreferedFrame ? pPreferedFrame->GetCurrentViewFrame() : 0;
    SvtSaveOptions aOpt;
    BOOL bLoadDocWins = aOpt.IsSaveDocWins() && !pPrefered;
    BOOL bLoadDocView = aOpt.IsSaveDocView();

    // In a StarPortal not possible at the moment
    if ( Application::IsRemoteServer() )
        bLoadDocWins = FALSE;

    if ( !bLoadDocView )
        return 0;

    // try to get viewdata information for XML format
    REFERENCE < XVIEWDATASUPPLIER > xViewDataSupplier( GetModel(), ::com::sun::star::uno::UNO_QUERY );
    REFERENCE < XINDEXACCESS > xViewData;
    SvStorageStreamRef xStream;

    // get viewdata information for binary format
    SvStorage *pStor = HasName() ? GetStorage() : NULL;
    xStream = pStor ? pStor->OpenStream( DEFINE_CONST_UNICODE( SFX_WINDOWS_STREAM ), STREAM_STD_READ ) : 0;
    if ( xStream.Is() && xStream->GetError() == ERRCODE_NONE )
    {
        xStream->SetVersion( pStor->GetVersion() );
        xStream->SetBufferSize(1024);
    }
    else if ( xViewDataSupplier.is() )
    {
        xViewData = xViewDataSupplier->getViewData();
        if ( !xViewData.is() )
            return NULL;
    }
    else
        return NULL;

    BOOL bOldFormat = TRUE;             // old format : not in StarDesktop 5.x
    SfxViewFrame *pActiveFrame = 0;
    String aWinData;
    char cToken =',';
    SfxItemSet *pSet = GetMedium()->GetItemSet();

    pImp->bLoadingWindows = TRUE;
    BOOL bLoaded = FALSE;
    sal_Int32 nView = 0;

    // get saved information for all views
    while ( TRUE )
    {
        USHORT nViewId = 0;
        FASTBOOL bActive=FALSE, bMaximized=FALSE;
        String aPosSize;
        String aUserData;                   // used in the binary format
        SEQUENCE < PROPERTYVALUE > aSeq;    // used in the XML format
        if ( xViewData.is() )
        {
            // XML format
            // active view is the first view in the container
            bActive = ( nView == 0 );

            if ( nView == xViewData->getCount() )
                // finished
                break;

            // get viewdata and look for the stored ViewId
            ::com::sun::star::uno::Any aAny = xViewData->getByIndex( nView++ );
            if ( aAny >>= aSeq )
            {
                for ( sal_Int32 n=0; n<aSeq.getLength(); n++ )
                {
                    const PROPERTYVALUE& rProp = aSeq[n];
                    if ( rProp.Name.compareToAscii("ViewId") == COMPARE_EQUAL )
                    {
                        ::rtl::OUString aId;
                        rProp.Value >>= aId;
                        String aTmp( aId );
                        aTmp.Erase( 0, 4 );  // format is like in "view3"
                        nViewId = (USHORT) aTmp.ToInt32();
                        break;
                    }
                }
            }
        }
        else
        {
            // binary format
            xStream->ReadByteString( aWinData );
            if ( !aWinData.Len() )
                // reading finished
                break;

            if ( aWinData.GetToken( 0, cToken ).EqualsAscii( "TASK" ) )
            {
                // doesn't make any sense with the new task handling using system tasks or browser windows
                bOldFormat = FALSE;
                continue;
            }

            nViewId = (USHORT) aWinData.GetToken( 0, cToken ).ToInt32();
            if ( bOldFormat )
            {
                // Old format
                aPosSize = aWinData.GetToken( 1, cToken );
                aPosSize.ToLowerAscii();
                aUserData = aWinData.GetToken( 2, cToken );
                bActive = aWinData.GetToken( 3, cToken ).ToInt32();
            }
            else
            {
                // 5.0-Format, get activity state and UserData
                USHORT nPos=0;
                bActive = aWinData.GetToken( 3, cToken, nPos ).ToInt32();
                aUserData = aWinData.Copy( nPos );
            }
        }

        // load only active view, but current item is not the active one ?
        if ( !bLoadDocWins && !bActive )
        {
            if ( xViewData.is() )
                // in XML format the active view is the first one
                break;
            else
                continue;
        }

        // check for minimized/maximized/size
        if ( aPosSize.EqualsAscii( "min" ) )
            bMaximized = TRUE;
        else if ( aPosSize.EqualsAscii( "min" ) )
        {
            bMaximized = TRUE;
            bActive = FALSE;
        }
        else
            bMaximized = FALSE;

        Point aPt;
        Size aSz;
#if SUPD<613//MUSTINI
        if ( !bMaximized )
            SfxIniManager::GetPosSize( aPosSize, aPt, aSz );
#endif

        pSet->ClearItem( SID_USER_DATA );
        SfxViewFrame *pFrame = 0;
        if ( pPrefered )
        {
            // use the frame from the arguments, but don't set a window size
            pFrame = pPrefered;
            if ( pFrame->GetViewShell() || !pFrame->GetObjectShell() )
            {
                pSet->ClearItem( SID_VIEW_POS_SIZE );
                pSet->ClearItem( SID_WIN_POSSIZE );
                pSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );

                // avoid flickering controllers
                SfxBindings &rBind = pFrame->GetBindings();
                rBind.ENTERREGISTRATIONS();

                // set document into frame
                pPreferedFrame->InsertDocument( this );

                // restart controller updating
                rBind.LEAVEREGISTRATIONS();
            }
            else
            {
                // create new view
                pFrame->CreateView_Impl( nViewId );
            }
        }
        else
        {
            if ( bLoadDocWins )
            {
                // open in the background
                pSet->Put( SfxUInt16Item( SID_VIEW_ZOOM_MODE, 0 ) );
                if ( !bMaximized )
                    pSet->Put( SfxRectangleItem( SID_VIEW_POS_SIZE, Rectangle( aPt, aSz ) ) );
            }

            pSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );

            if ( pPreferedFrame )
            {
                // Frame "ubergeben, allerdings ist der noch leer
                pPreferedFrame->InsertDocument( this );
                pFrame = pPreferedFrame->GetCurrentViewFrame();
            }
            else
            {
                pFrame = SfxTopFrame::Create( this, nViewId, FALSE, pSet )->GetCurrentViewFrame();
            }

            // only temporary data, don't hold it in the itemset
            pSet->ClearItem( SID_VIEW_POS_SIZE );
            pSet->ClearItem( SID_WIN_POSSIZE );
            pSet->ClearItem( SID_VIEW_ZOOM_MODE );
        }

        bLoaded = TRUE;

        // UserData hier einlesen, da es ansonsten immer mit bBrowse=TRUE
        // aufgerufen wird, beim Abspeichern wurde aber bBrowse=FALSE verwendet
        if ( pFrame && pFrame->GetViewShell() )
        {
            if ( aUserData.Len() )
                pFrame->GetViewShell()->ReadUserData( aUserData, !bLoadDocWins );
            else if ( aSeq.getLength() )
                pFrame->GetViewShell()->ReadUserDataSequence( aSeq, !bLoadDocWins );
        }

        // perhaps there are more windows to load
        pPreferedFrame = NULL;

        if ( bActive )
            pActiveFrame = pFrame;

        if( pPrefered || !bLoadDocWins )
            // load only active window
            break;
    }

    if ( pActiveFrame )
    {
        if ( !pPrefered )
            // activate frame
            pActiveFrame->MakeActive_Impl( TRUE );
    }

    pImp->bLoadingWindows = FALSE;
    return pPrefered && bLoaded ? pPrefered : pActiveFrame;
}

//====================================================================

void SfxObjectShell::UpdateDocInfoForSave()
{
    if( !pImp->bDoNotTouchDocInfo )
    {
        SfxDocumentInfo &rDocInfo = GetDocInfo();
        rDocInfo.SetTemplateConfig( HasTemplateConfig() );

        if ( IsModified() )
        {
            // Keine Unterschiede mehr zwischen Save, SaveAs
            String aUserName = SvtUserOptions().GetFullName();
            if ( !rDocInfo.IsUseUserData() )
            {
                SfxStamp aCreated = rDocInfo.GetCreated();
                if ( aUserName == aCreated.GetName() )
                {
                    aCreated.SetName( String() );
                    rDocInfo.SetCreated( aCreated );
                }

                SfxStamp aPrinted = rDocInfo.GetPrinted();
                if ( aUserName == aPrinted.GetName() )
                {
                    aPrinted.SetName( String() );
                    rDocInfo.SetPrinted( aPrinted );
                }

                aUserName.Erase();
            }

            rDocInfo.SetChanged( aUserName );
            if ( !HasName() || pImp->bIsSaving )
                UpdateTime_Impl( rDocInfo );
        }

        if ( !pImp->bIsSaving )
            rDocInfo.SetPasswd( pImp->bPasswd );

        Broadcast( SfxDocumentInfoHint( &rDocInfo ) );
    }
}

// -----------------------------------------------------------------------

BOOL SfxObjectShell::SaveInfoAndConfig_Impl( SvStorageRef pNewStg )
{
    //Demnaechst mal gemeinsame Teile zusammenfassen
    UpdateDocInfoForSave();

#if !defined( SFX_KEY_MAXPREVIEWSIZE ) && defined( TFPLUGCOMM )
#define SFX_KEY_MAXPREVIEWSIZE SFX_KEY_ISFREE
#endif

#ifdef MI_doch_wieder_die_alte_preview
    String aMaxSize = SFX_INIMANAGER()->Get( SFX_KEY_MAXPREVIEWSIZE );
    ULONG nMaxSize = aMaxSize.Len() ? ULONG( aMaxSize ) : 50000;
#else
    ULONG nMaxSize = 0L;
#endif
    if( nMaxSize && !GetDocInfo().IsPasswd() &&
        SFX_CREATE_MODE_STANDARD == eCreateMode )
    {
        GDIMetaFile* pFile = GetPreviewMetaFile();
        if ( pFile )
        {
            SvCacheStream aStream;
            long nVer = pNewStg->GetVersion();
            aStream.SetVersion( nVer );
            aStream << *pFile;
            if( aStream.Tell() < nMaxSize )
            {
                SvStorageStreamRef xStream = pNewStg->OpenStream(
                    DEFINE_CONST_UNICODE( SFX_PREVIEW_STREAM ),
                    STREAM_TRUNC | STREAM_STD_READWRITE);
                if( xStream.Is() && !xStream->GetError() )
                {
                    long nVer = pNewStg->GetVersion();
                    xStream->SetVersion( nVer );
                    aStream.Seek( 0L );
                    *xStream << aStream;
                }
            }
            delete pFile;
        }
    }

    if( pImp->bIsSaving )
    {
        //!! kein Aufruf der Basisklasse wegen doppeltem Aufruf in Persist
        //if(!SfxObjectShell::Save())
        //  return FALSE;
        SvStorageRef aRef = GetMedium()->GetStorage();
        if ( aRef.Is() )
        {
            SfxDocumentInfo& rDocInfo = GetDocInfo();
            rDocInfo.Save(pNewStg);

            // wenn es sich um ein Dokument lokales Basic handelt, dieses
            // schreiben
            if ( pImp->pBasicMgr )
                pImp->pBasicMgr->Store( *pNewStg );
            else
            {
                String aURL;
                if( HasName() )
                    aURL = GetMedium()->GetName();
                else
                {
                    aURL = GetDocInfo().GetTemplateFileName();
                    // Bei Templates keine URL...
                    aURL = URIHelper::SmartRelToAbs( aURL );
                }
#ifndef TFPLUGCOMM
                BasicManager::CopyBasicData( GetStorage(), aURL, pNewStg );
#endif
            }

            // Windows-merken
            if ( TRUE ) HACK(aus config)
                SaveWindows_Impl( *pNewStg );

            // Konfiguration schreiben
            if ( GetConfigManager() )
            {
/* //!MBA
                if ( rDocInfo.HasTemplateConfig() )
                {
                    const String aTemplFileName( rDocInfo.GetTemplateFileName() );
                    if ( aTemplFileName.Len() )
                    {
                        INetURLObject aURL( aTemplFileName );
                        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL !" );

                        SvStorageRef aStor = new SvStorage( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                        if ( SVSTREAM_OK == aStor->GetError() )
                        {
                            GetConfigManager()->StoreConfiguration(aStor);
                            if (aRef->IsStream(SfxConfigManager::GetStreamName()))
                                aRef->Remove(SfxConfigManager::GetStreamName());
                        }
                    }
                }
                else
 */
                {
//! MBA                    GetConfigManager()->SetModified( TRUE );
                    GetConfigManager()->StoreConfiguration( pNewStg );
                }
            }
        }
        return TRUE;
    }
    else
    {
        //!! kein Aufruf der Basisklasse wegen doppeltem Aufruf in Persist
        //if(!SfxObjectShell::SaveAs(pNewStg))
        //  return FALSE;
        SfxApplication *pSfxApp = SFX_APP();
        SfxMedium *pActMed = GetMedium();

        // alte DocInfo laden
        SfxDocumentInfo &rDocInfo = GetDocInfo();

        // DocInfo speichern
        rDocInfo.Save( pNewStg );

        // wenn es sich um ein Dokument lokales Basic handelt, dieses schreiben
        if ( pImp->pBasicMgr )
            pImp->pBasicMgr->Store( *pNewStg );
#ifndef MI_NONOS
        else
        {
            String aURL;
            if( HasName() )
                aURL = GetMedium()->GetName();
            else
            {
                aURL = GetDocInfo().GetTemplateFileName();
                // Bei Templates keine URL...
                aURL = URIHelper::SmartRelToAbs( aURL );
            }
#ifndef TFPLUGCOMM
            BasicManager::CopyBasicData( GetStorage(), aURL, pNewStg );
#endif
        }
#endif
        // Windows-merken
        if ( TRUE ) HACK(aus config)
            SaveWindows_Impl( *pNewStg );

        // Konfiguration schreiben
        if (GetConfigManager())
        {
/* //!MBA
            if ( rDocInfo.HasTemplateConfig() )
            {
                const String aTemplFileName( rDocInfo.GetTemplateFileName() );
                if ( aTemplFileName.Len() )
                {
                    INetURLObject aURL( aTemplFileName );
                    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL !" );

                    SvStorageRef aStor = new SvStorage( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                    if ( SVSTREAM_OK == aStor->GetError() )
                    {
                        GetConfigManager()->StoreConfiguration(aStor);
                        if (pNewStg->IsStream(SfxConfigManager::GetStreamName()))
                            pNewStg->Remove(SfxConfigManager::GetStreamName());
                    }
                }
            }
            else
 */
            {
//!MBA                GetConfigManager()->SetModified( TRUE );
                GetConfigManager()->StoreConfiguration(pNewStg);
            }
        }

        return TRUE;
    }
}

//--------------------------------------------------------------------

// Bearbeitungszeit aktualisieren
SfxDocumentInfo& SfxObjectShell::UpdateTime_Impl(SfxDocumentInfo &rInfo)
{
    // Get old time from documentinfo
    Time aOldTime(rInfo.GetTime());

    // Initialize some local member! Its neccessary for wollow operations!
    DateTime    aNow                    ;   // Date and time at current moment
    Time        n24Time     (24,0,0,0)  ;   // Time-value for 24 hours - see follow calculation
    ULONG       nDays       = 0         ;   // Count of days between now and last editing
    Time        nAddTime    (0)         ;   // Value to add on aOldTime

    // Safe impossible cases!
    // User has changed time to the past between last editing and now ... its not possible!!!
    DBG_ASSERT( !(aNow.GetDate()<pImp->nTime.GetDate()), "Timestamp of last change is in the past ?!..." );

    // Do the follow only, if user has NOT changed time to the past.
    // Else add a time of 0 to aOldTime ... !!!
    if (aNow.GetDate()>=pImp->nTime.GetDate())
    {
        // Get count of days last editing.
        nDays = aNow.GetSecFromDateTime(pImp->nTime.GetDate())/86400 ;

        if (nDays==0)
        {
            // If no day between now and last editing - calculate time directly.
            nAddTime    =   (const Time&)aNow - (const Time&)pImp->nTime ;
        }
        else
        // If time of working without save greater then 1 month (!) ....
        // we add 0 to aOldTime!
        if (nDays<=31)
        {
            // If 1 or up to 31 days between now and last editing - calculate time indirectly.
            // nAddTime = (24h - nTime) + (nDays * 24h) + aNow
            --nDays;
             nAddTime    =  nDays*n24Time.GetTime() ;
            nAddTime    +=  n24Time-(const Time&)pImp->nTime        ;
            nAddTime    +=  aNow                    ;
        }

        aOldTime += nAddTime;
    }

    rInfo.SetTime(aOldTime.GetTime());
    pImp->nTime = aNow;
    rInfo.IncDocumentNumber();
    //! DocumentNummer
#if 0
    const String aDocNo(rInfo.GetUserKey(0).GetWord());
    const String aTitle(rInfo.GetUserKey(0).GetTitle());
    USHORT nNo = 1;
    if ( aDocNo.Len() )
    {
        nNo = (USHORT)aDocNo;
        if(nNo)
            ++nNo;
        else
            nNo = 1;
    }
    rInfo.SetUserKey(SfxDocUserKey(aTitle, nNo), 0);
#endif
    return rInfo;
}

//--------------------------------------------------------------------

void SfxObjectShell::DocInfoDlg_Impl( SfxDocumentInfo &rDocInfo )
{
    // anzuzeigenden Dokumentnamen ermitteln
    String aURL, aTitle;
    if ( HasName() && !pImp->aNewName.Len() )
    {
        aURL = GetMedium()->GetName();
        aTitle = GetTitle();
    }
    else
    {
        if ( !pImp->aNewName.Len() )
        {
            aURL = DEFINE_CONST_UNICODE( "private:factory/" );
            aURL += String::CreateFromAscii( GetFactory().GetShortName() );
            // aTitle = String( SfxResId( STR_NONAME ) );
        }
        else
        {
            aURL = DEFINE_CONST_UNICODE( "[private:factory/" );
            aURL += String::CreateFromAscii( GetFactory().GetShortName() );
            aURL += DEFINE_CONST_UNICODE( "]" );
            INetURLObject aURLObj( pImp->aNewName );
            aURL += aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
            // aTitle = aURLObj.GetBase();
        }
        aTitle = GetTitle();
    }

    // Itemset f"ur Dialog aufbereiten
    SfxDocumentInfoItem aDocInfoItem( aURL, rDocInfo );
    if ( !GetSlotState( SID_DOCTEMPLATE ) )
        aDocInfoItem.SetTemplate(FALSE);
    SfxItemSet aSet(GetPool(), SID_DOCINFO, SID_DOCINFO,
                    SID_EXPLORER_PROPS_START, SID_EXPLORER_PROPS_START,
                    0L );
    aSet.Put( aDocInfoItem );
    aSet.Put( SfxStringItem( SID_EXPLORER_PROPS_START, aTitle ) );

    // Dialog via Factory erzeugen und ausf"uhren
    SfxDocumentInfoDialog *pDlg = CreateDocumentInfoDialog(0, aSet);
    if ( RET_OK == pDlg->Execute() )
    {
        // neue DocInfo aus Dialog holen
        const SfxPoolItem *pItem = 0;
        if ( SFX_ITEM_SET ==
             pDlg->GetOutputItemSet()->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        {
            rDocInfo = (*(const SfxDocumentInfoItem *)pItem)();

            // ggf. den Titel des Dokuments neu setzen
            String aNewTitle = rDocInfo.GetTitle();
            aNewTitle.EraseLeadingChars();
            aNewTitle.EraseTrailingChars();
            if ( aTitle != aNewTitle && aNewTitle.Len() )
                SetTitle( aNewTitle );
        }
    }
    delete pDlg;
}

//------------------------------------------------------------------------

SfxDocumentInfoDialog* SfxObjectShell::CreateDocumentInfoDialog
(
    Window*             pParent,
    const SfxItemSet&   rSet
)
{
    return new SfxDocumentInfoDialog(pParent, rSet);
}

//--------------------------------------------------------------------

SvEmbeddedInfoObject* SfxObjectShell::InsertObject
(
    SvEmbeddedObject*   pObj,
    const String&       rName
)

{
    // Objekt erzeugen ist fehlgeschlagen?
    if ( !pObj )
        HACK(Fehlermeldung fehlt)
        return 0;

    String aName( rName );
    if( !aName.Len() )
    {
        aName = DEFINE_CONST_UNICODE("Object ");
        String aStr;
        USHORT i = 1;
        HACK(Wegen Storage Bug 46033)
        // for-Schleife wegen Storage Bug 46033
        for( USHORT n = 0; n < 100; n++ )
        {
            do
            {
                aStr = aName;
                aStr += String::CreateFromInt32( i );
                i++;
            } while ( Find( aStr ) );

            SvInfoObjectRef xSub = new SvEmbeddedInfoObject( pObj, aStr );
            if ( Move( xSub, aStr ) ) // Eigentuemer Uebergang
                return (SvEmbeddedInfoObject*) &xSub;
        }
    }
    else
    {
        SvInfoObjectRef xSub = new SvEmbeddedInfoObject( pObj, aName );
        if ( Move( xSub, aName ) ) // Eigentuemer Uebergang
            return (SvEmbeddedInfoObject*) &xSub;
    }
    return 0;
}

//-------------------------------------------------------------------------

SfxConfigManager* SfxObjectShell::GetConfigManager( BOOL bForceCreation )
{
    if ( !pImp->pCfgMgr )
    {
        if ( bForceCreation || HasStorage() && SfxConfigManager::HasConfiguration( *GetStorage() ) )
        {
            pImp->pCfgMgr = new SfxConfigManager( *this );
            SfxConfigItem* pItem = GetEventConfig_Impl( FALSE );
            if ( pItem && !pItem->GetConfigManager() )
                // imported binary format
                pItem->Connect( pImp->pCfgMgr );

        }
    }

    return pImp->pCfgMgr;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetConfigManager(SfxConfigManager *pMgr)
{
//    if ( pImp->pCfgMgr == SFX_CFGMANAGER() && pMgr)
//        pMgr->Activate(pImp->pCfgMgr);

    if ( pImp->pCfgMgr && pImp->pCfgMgr != pMgr )
        delete pImp->pCfgMgr;

    pImp->pCfgMgr = pMgr;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetTemplateConfig(BOOL bTplConf)
{
//    pImp->bTemplateConfig = bTplConf;
//    DBG_ASSERT(pImp->pCfgMgr || !bTplConf,"Keine Konfiguration in der Vorlage!");
}

//-------------------------------------------------------------------------

BOOL SfxObjectShell::HasTemplateConfig() const
{
//!MBA    return pImp->bTemplateConfig;
    return FALSE;
}

//-------------------------------------------------------------------------
/*
void SfxObjectShell::TransferConfig(SfxObjectShell& rObjSh)
{
    SfxConfigManager *pNewCfgMgr=0, *pOldCfgMgr=0;
    pOldCfgMgr = pImp->pCfgMgr;
    pImp->pCfgMgr = 0;

    pNewCfgMgr = rObjSh.pImp->pCfgMgr;
    rObjSh.pImp->pCfgMgr=0;

    SetConfigManager(pNewCfgMgr);
    rObjSh.SetConfigManager(pOldCfgMgr);
}
*/

//--------------------------------------------------------------------

SfxStyleSheetBasePool* SfxObjectShell::GetStyleSheetPool()
{
    return 0;
}

void SfxObjectShell::SetOrganizerSearchMask(
    SfxStyleSheetBasePool* pPool) const
{
    pPool->SetSearchMask(SFX_STYLE_FAMILY_ALL,
                         SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
}

//--------------------------------------------------------------------

USHORT SfxObjectShell::GetContentCount(USHORT nIdx1,
                                         USHORT nIdx2)
{
    switch(nIdx1)
    {
        case INDEX_IGNORE:
            return DEF_CONTENT_COUNT;
        case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pPool = GetStyleSheetPool();
            if(!pPool)
                return 0;
            SetOrganizerSearchMask(pPool);
            return pPool->Count();
        }
        case CONTENT_MACRO:
            break;
/*
        case CONTENT_CONFIG:
            return (GetConfigManager() && !HasTemplateConfig()) ?
                        GetConfigManager()->GetItemCount() : 0;
            break;
 */
    }
    return 0;
}


//--------------------------------------------------------------------

void  SfxObjectShell::TriggerHelpPI(USHORT nIdx1, USHORT nIdx2, USHORT nIdx3)
{
    if(nIdx1==CONTENT_STYLE && nIdx2 != INDEX_IGNORE) //StyleSheets
    {
        SfxStyleSheetBasePool *pPool = GetStyleSheetPool();
        SetOrganizerSearchMask(pPool);
        SfxStyleSheetBase *pStyle = (*pPool)[nIdx2];
#ifdef WIR_KOENNEN_WIEDER_HILFE_FUER_STYLESHEETS
        if(pStyle)
        {
            String aHelpFile;
            ULONG nHelpId=pStyle->GetHelpId(aHelpFile);
            SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
            if ( pHelpPI && nHelpId )
                pHelpPI->LoadTopic( nHelpId );
        }
#endif
    }
}

BOOL   SfxObjectShell::CanHaveChilds(USHORT nIdx1,
                                       USHORT nIdx2)
{
    switch(nIdx1) {
    case INDEX_IGNORE:
        return TRUE;
    case CONTENT_STYLE:
        return INDEX_IGNORE == nIdx2 || !GetStyleSheetPool()? FALSE: TRUE;
    case CONTENT_MACRO:
//!!    return INDEX_IGNORE == nIdx2? FALSE: TRUE;
        return FALSE;
/*
    case CONTENT_CONFIG:
        return INDEX_IGNORE == nIdx2 ? FALSE : TRUE;
 */
    }
    return FALSE;
}

//--------------------------------------------------------------------

void SfxObjectShell::GetContent(String &rText,
                                Bitmap &rClosedBitmap,
                                Bitmap &rOpenedBitmap,
                                BOOL &bCanDel,
                                USHORT i,
                                USHORT nIdx1,
                                USHORT nIdx2 )
{
    DBG_ERRORFILE( "Non high contrast method called. Please update calling code!" );
    SfxObjectShell::GetContent( rText, rClosedBitmap, rOpenedBitmap, BMP_COLOR_NORMAL, bCanDel, i, nIdx1, nIdx2 );
}

//--------------------------------------------------------------------

void   SfxObjectShell::GetContent(String &rText,
                                  Bitmap &rClosedBitmap,
                                  Bitmap &rOpenedBitmap,
                                  BmpColorMode eColorMode,
                                  BOOL &bCanDel,
                                  USHORT i,
                                  USHORT nIdx1,
                                  USHORT nIdx2 )
{
    bCanDel=TRUE;

    switch(nIdx1)
    {
        case INDEX_IGNORE:
        {
            USHORT nTextResId = 0;
            USHORT nClosedBitmapResId; // evtl. sp"ater mal unterschiedliche
            USHORT nOpenedBitmapResId; // "     "       "   "
            switch(i)
            {
                case CONTENT_STYLE:
                    nTextResId = STR_STYLES;
                    if ( eColorMode == BMP_COLOR_NORMAL )
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED;
                        nOpenedBitmapResId= BMP_STYLES_OPENED;
                    }
                    else
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED_HC;
                        nOpenedBitmapResId= BMP_STYLES_OPENED_HC;
                    }
                    break;
                case CONTENT_MACRO:
                    nTextResId = STR_MACROS;
                    if ( eColorMode == BMP_COLOR_NORMAL )
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED;
                        nOpenedBitmapResId= BMP_STYLES_OPENED;
                    }
                    else
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED_HC;
                        nOpenedBitmapResId= BMP_STYLES_OPENED_HC;
                    }
                    break;
/*
                case CONTENT_CONFIG:
                    nTextResId = STR_CONFIG;
                    nClosedBitmapResId= BMP_STYLES_CLOSED;
                    nOpenedBitmapResId= BMP_STYLES_OPENED;
                    break;
 */
            }

            if ( nTextResId )
            {
                rText  = String(SfxResId(nTextResId));
                rClosedBitmap = Bitmap(SfxResId(nClosedBitmapResId));
                rOpenedBitmap = Bitmap(SfxResId(nOpenedBitmapResId));
            }
            break;
        }

        case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pPool = GetStyleSheetPool();
            SetOrganizerSearchMask(pPool);
            SfxStyleSheetBase *pStyle = (*pPool)[i];
            rText = pStyle->GetName();
            bCanDel=((pStyle->GetMask() & SFXSTYLEBIT_USERDEF)
                     == SFXSTYLEBIT_USERDEF);
            rClosedBitmap = rOpenedBitmap =
                GetStyleFamilyBitmap(pStyle->GetFamily(), eColorMode );
        }
            break;
        case CONTENT_MACRO:
            break;
/*
        case CONTENT_CONFIG:
            if ( GetConfigManager() && !HasTemplateConfig())
            {
                rText = GetConfigManager()->GetItem(i);
                bCanDel = GetConfigManager()->CanDelete(i);
            }
            else
                rText = String();
            rClosedBitmap = Bitmap(SfxResId(BMP_STYLES_CLOSED));
            rOpenedBitmap = Bitmap(SfxResId(BMP_STYLES_OPENED));
            break;
*/
    }
}

//--------------------------------------------------------------------
Bitmap SfxObjectShell::GetStyleFamilyBitmap( SfxStyleFamily eFamily )
{
    DBG_ERRORFILE( "Non high contrast method called. Please update calling code!" );
    return SfxObjectShell::GetStyleFamilyBitmap( eFamily, BMP_COLOR_NORMAL );
}

//--------------------------------------------------------------------

Bitmap SfxObjectShell::GetStyleFamilyBitmap(SfxStyleFamily eFamily, BmpColorMode eColorMode )
{
    USHORT nResId = 0;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY1 : BMP_STYLES_FAMILY1_HC;
            break;
        case SFX_STYLE_FAMILY_PARA:
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY2 : BMP_STYLES_FAMILY2_HC;
            break;
        case SFX_STYLE_FAMILY_FRAME:
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY3 : BMP_STYLES_FAMILY3_HC;
            break;
        case SFX_STYLE_FAMILY_PAGE :
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY4 : BMP_STYLES_FAMILY4_HC;
            break;
    }

    if ( nResId )
        return Bitmap(SfxResId(nResId));
    else
        return Bitmap();
}


//--------------------------------------------------------------------

BOOL SfxObjectShell::Insert(SfxObjectShell &rSource,
                              USHORT nSourceIdx1,
                              USHORT nSourceIdx2,
                              USHORT nSourceIdx3,
                              USHORT &nIdx1,
                              USHORT &nIdx2,
                              USHORT &nIdx3,
                              USHORT &nDeleted)
{
    BOOL bRet = FALSE;

    if (INDEX_IGNORE == nIdx1 && CONTENT_STYLE == nSourceIdx1)
        nIdx1 = CONTENT_STYLE;

    if (CONTENT_STYLE == nSourceIdx1 && CONTENT_STYLE == nIdx1)
    {
        SfxStyleSheetBasePool* pHisPool  = rSource.GetStyleSheetPool();
        SfxStyleSheetBasePool* pMyPool   = GetStyleSheetPool();
        SetOrganizerSearchMask(pHisPool);
        SetOrganizerSearchMask(pMyPool);

        SfxStyleSheetBase* pHisSheet = (*pHisPool)[nSourceIdx2];

        // Einfuegen ist nur dann noetig, wenn ein StyleSheet
        // zwischen unterschiedlichen(!) Pools bewegt wird

        if (pMyPool != pHisPool)
        {
            if (INDEX_IGNORE == nIdx2)
            {
                nIdx2 = pMyPool->Count();
            }

            // wenn so eine Vorlage schon existiert: loeschen!
            String aOldName(pHisSheet->GetName());
            SfxStyleFamily eOldFamily = pHisSheet->GetFamily();

            SfxStyleSheetBase* pExist = pMyPool->Find(aOldName, eOldFamily);
            // USHORT nOldHelpId = pExist->GetHelpId(??? VB ueberlegt sich was);
            BOOL bUsedOrUserDefined;
            if( pExist )
            {
                bUsedOrUserDefined =
                    pExist->IsUsed() || pExist->IsUserDefined();
                if( ErrorHandler::HandleError(
                    *new MessageInfo( ERRCODE_SFXMSG_STYLEREPLACE, aOldName ) )
                    != ERRCODE_BUTTON_OK )
                    return FALSE;
                else
                {
                    pMyPool->Replace( *pHisSheet, *pExist );
                    SetModified( TRUE );
                    nIdx2 = nIdx1 = INDEX_IGNORE;
                    return TRUE;
                }
            }

            SfxStyleSheetBase& rNewSheet = pMyPool->Make(
                aOldName, eOldFamily,
                pHisSheet->GetMask(), nIdx2);

            // ItemSet der neuen Vorlage fuellen
            rNewSheet.GetItemSet().Set(pHisSheet->GetItemSet());

            // wer bekommt den Neuen als Parent? wer benutzt den Neuen als Follow?
            SfxStyleSheetBase* pTestSheet = pMyPool->First();
            while (pTestSheet)
            {
                if (pTestSheet->GetFamily() == eOldFamily &&
                    pTestSheet->HasParentSupport() &&
                    pTestSheet->GetParent() == aOldName)
                {
                    pTestSheet->SetParent(aOldName);
                    // Verknuepfung neu aufbauen
                }

                if (pTestSheet->GetFamily() == eOldFamily &&
                    pTestSheet->HasFollowSupport() &&
                    pTestSheet->GetFollow() == aOldName)
                {
                    pTestSheet->SetFollow(aOldName);
                    // Verknuepfung neu aufbauen
                }

                pTestSheet = pMyPool->Next();
            }
            bUsedOrUserDefined =
                rNewSheet.IsUsed() || rNewSheet.IsUserDefined();


            // hat der Neue einen Parent? wenn ja, mit gleichem Namen bei uns suchen
            if (pHisSheet->HasParentSupport())
            {
                const String& rParentName = pHisSheet->GetParent();
                if (0 != rParentName.Len())
                {
                    SfxStyleSheetBase* pParentOfNew =
                        pMyPool->Find(rParentName, eOldFamily);
                    if (pParentOfNew)
                        rNewSheet.SetParent(rParentName);
                }
            }

            // hat der Neue einen Follow? wenn ja, mit gleichem
            // Namen bei uns suchen
            if (pHisSheet->HasFollowSupport())
            {
                const String& rFollowName = pHisSheet->GetFollow();
                if (0 != rFollowName.Len())
                {
                    SfxStyleSheetBase* pFollowOfNew =
                        pMyPool->Find(rFollowName, eOldFamily);
                    if (pFollowOfNew)
                        rNewSheet.SetFollow(rFollowName);
                }
            }

            SetModified( TRUE );
            if( !bUsedOrUserDefined ) nIdx2 = nIdx1 = INDEX_IGNORE;

            bRet = TRUE;
        }
        else
            bRet = FALSE;
    }
/*
    else if (nSourceIdx1 == CONTENT_CONFIG)
    {
        nIdx1 = CONTENT_CONFIG;

        SfxConfigManager *pCfgMgr = SFX_CFGMANAGER();
        if (!GetConfigManager() || HasTemplateConfig())
        {
            SetConfigManager(new SfxConfigManager(0, pCfgMgr));
            SetTemplateConfig(FALSE);
            if (this == Current())
                GetConfigManager()->Activate(pCfgMgr);
        }

        if (GetConfigManager()->CopyItem(
            nSourceIdx2, nIdx2, rSource.GetConfigManager()))
        {
            SetModified(TRUE);
            bRet = TRUE;
            SFX_APP()->GetDispatcher_Impl()->Update_Impl(TRUE);
        }
    }
*/
    return bRet;
}

//--------------------------------------------------------------------

BOOL SfxObjectShell::Remove
(
    USHORT nIdx1,
    USHORT nIdx2,
    USHORT nIdx3
)
{
    BOOL bRet = FALSE;

    if (CONTENT_STYLE == nIdx1)
    {
        SfxStyleSheetBasePool* pMyPool  = GetStyleSheetPool();

        SetOrganizerSearchMask(pMyPool);

        SfxStyleSheetBase* pMySheet =  (*pMyPool)[nIdx2];
        String aName(pMySheet->GetName());
        String aEmpty;
        SfxStyleFamily  eFamily = pMySheet->GetFamily();
        if (pMySheet)
        {
            pMyPool->Erase(pMySheet);
            bRet = TRUE;
        }

        SfxStyleSheetBase* pTestSheet = pMyPool->First();
        while (pTestSheet)
        {
            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasParentSupport() &&
                pTestSheet->GetParent() == aName)
            {
                pTestSheet->SetParent(aEmpty); // Verknuepfung aufloesen
            }

            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasFollowSupport() &&
                pTestSheet->GetFollow() == aName)
            {
                pTestSheet->SetFollow(aEmpty); // Verknuepfung aufloesen
            }

            pTestSheet = pMyPool->Next();
        }
        if(bRet)
            SetModified( TRUE );
    }
/*
    else if (nIdx1 == CONTENT_CONFIG)
    {
        if (GetConfigManager()->RemoveItem(nIdx2))
        {
            SetModified(TRUE);
            bRet = TRUE;
            SFX_APP()->GetDispatcher_Impl()->Update_Impl(TRUE);
        }
    }
*/
    return bRet;
}

//--------------------------------------------------------------------

BOOL SfxObjectShell::Print
(
    Printer&        rPrt,
    USHORT          nIdx1,
    USHORT          nIdx2,
    USHORT          nIdx3,
    const String*   pObjectName
)

/*  [Beschreibung]
*/

{
    switch(nIdx1)
    {
      case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pPool = GetStyleSheetPool();
            SetOrganizerSearchMask(pPool);
            SfxStyleSheetIterator* pIter = pPool->CreateIterator(
                pPool->GetSearchFamily(), pPool->GetSearchMask() );
            USHORT nStyles = pIter->Count();
            SfxStyleSheetBase *pStyle = pIter->First();
            if ( !pStyle )
                return TRUE;

            if ( !rPrt.StartJob(String(SfxResId(STR_STYLES))) )
            {
                delete pIter;
                return FALSE;
            }
            if ( !rPrt.StartPage() )
            {
                delete pIter;
                return FALSE;
            }
            SfxStatusBarManager* pStbMgr = SFX_APP()->GetStatusBarManager();
            if ( pStbMgr )
                pStbMgr->StartProgressMode(String(SfxResId(STR_PRINT_STYLES)), nStyles);
            rPrt.SetMapMode(MapMode(MAP_10TH_MM));
            Font aFont( DEFINE_CONST_UNICODE( "Arial" ), Size(0, 64));   // 18pt
            aFont.SetWeight(WEIGHT_BOLD);
            rPrt.SetFont(aFont);
            const Size aPageSize(rPrt.GetOutputSize());
            const USHORT nXIndent = 200;
            USHORT nYIndent = 200;
            Point aOutPos(nXIndent, nYIndent);
            String aHeader(SfxResId(STR_PRINT_STYLES_HEADER));
            if ( pObjectName )
                aHeader += *pObjectName;
            else
                aHeader += GetTitle();
            long nTextHeight( rPrt.GetTextHeight() );
            rPrt.DrawText(aOutPos, aHeader);
            aOutPos.Y() += nTextHeight;
            aOutPos.Y() += nTextHeight/2;
            aFont.SetSize(Size(0, 35)); // 10pt
            nStyles = 1;
            while(pStyle)
            {
                if(pStbMgr)
                    pStbMgr->SetProgressState(nStyles++);
                // Ausgabe des Vorlagennamens
                String aStr(pStyle->GetName());
                aFont.SetWeight(WEIGHT_BOLD);
                rPrt.SetFont(aFont);
                nTextHeight = rPrt.GetTextHeight();
                // Seitenwechsel
                if ( aOutPos.Y() + nTextHeight*2 >
                    aPageSize.Height() - (long) nYIndent )
                {
                    rPrt.EndPage();
                    rPrt.StartPage();
                    aOutPos.Y() = nYIndent;
                }
                rPrt.DrawText(aOutPos, aStr);
                aOutPos.Y() += nTextHeight;

                // Ausgabe der Vorlagenbeschreibung
                aFont.SetWeight(WEIGHT_NORMAL);
                rPrt.SetFont(aFont);
                aStr = pStyle->GetDescription();
                const char cDelim = ' ';
                USHORT nStart = 0, nIdx = 0;

                nTextHeight = rPrt.GetTextHeight();
                // wie viele Worte passen auf eine Zeile
                while(nIdx < aStr.Len())
                {
                    USHORT  nOld = nIdx;
                    long nTextWidth;
                    nIdx = aStr.Search(cDelim, nStart);
                    nTextWidth = rPrt.GetTextWidth(aStr, nStart, nIdx-nStart);
                    while(nIdx != STRING_NOTFOUND &&
                          aOutPos.X() + nTextWidth <
                          aPageSize.Width() - (long) nXIndent)
                    {
                        nOld = nIdx;
                        nIdx = aStr.Search(cDelim, nIdx+1);
                        nTextWidth = rPrt.GetTextWidth(aStr, nStart, nIdx-nStart);
                    }
                    String aTmp(aStr, nStart, nIdx == STRING_NOTFOUND?
                                STRING_LEN :
                                nOld-nStart);
                    if ( aTmp.Len() )
                    {
                        nStart = nOld+1;    // wegen trailing space
                    }
                    else
                    {
                        USHORT nChar = 1;
                        while(
                            nStart + nChar < aStr.Len() &&
                            aOutPos.X() + rPrt.GetTextWidth(
                                aStr, nStart, nChar) <
                            aPageSize.Width() - nXIndent)
                            ++nChar;
                        aTmp = String(aStr, nStart, nChar-1);
                        nIdx = nStart + nChar;
                        nStart = nIdx;
                    }
                    if ( aOutPos.Y() + nTextHeight*2 >
                        aPageSize.Height() - nYIndent )
                    {
                        rPrt.EndPage();
                        rPrt.StartPage();
                        aOutPos.Y() = nYIndent;
                    }
                    rPrt.DrawText(aOutPos, aTmp);
                    aOutPos.Y() += rPrt.GetTextHeight();
                }
                pStyle = pIter->Next();
            }
            rPrt.EndPage();
            rPrt.EndJob();
            if ( pStbMgr )
                pStbMgr->EndProgressMode();
            delete pIter;
            break;
        }
      default:
          return FALSE;
    }
    return TRUE;
}

//--------------------------------------------------------------------

void SfxObjectShell::LoadStyles
(
    SfxObjectShell &rSource         /*  die Dokument-Vorlage, aus der
                                            die Styles geladen werden sollen */
)

/*  [Beschreibung]

    Diese Methode wird vom SFx gerufen, wenn aus einer Dokument-Vorlage
    Styles nachgeladen werden sollen. Bestehende Styles soll dabei
    "uberschrieben werden. Das Dokument mu"s daher neu formatiert werden.
    Daher werden die Applikationen in der Regel diese Methode "uberladen
    und in ihrer Implementierung die Implementierung der Basisklasse
    rufen.
*/

{
    struct Styles_Impl
    {
        SfxStyleSheetBase *pSource;
        SfxStyleSheetBase *pDest;
//      Styles_Impl () : pSource(0), pDest(0) {}
    };

    SfxStyleSheetBasePool *pSourcePool = rSource.GetStyleSheetPool();
    DBG_ASSERT(pSourcePool, "Source-DocumentShell ohne StyleSheetPool");
    SfxStyleSheetBasePool *pMyPool = GetStyleSheetPool();
    DBG_ASSERT(pMyPool, "Dest-DocumentShell ohne StyleSheetPool");
    pSourcePool->SetSearchMask(SFX_STYLE_FAMILY_ALL, 0xffff);
    Styles_Impl *pFound = new Styles_Impl[pSourcePool->Count()];
    USHORT nFound = 0;

    SfxStyleSheetBase *pSource = pSourcePool->First();
    while ( pSource )
    {
        SfxStyleSheetBase *pDest =
            pMyPool->Find( pSource->GetName(), pSource->GetFamily() );
        if ( !pDest )
        {
            pDest = &pMyPool->Make( pSource->GetName(),
                    pSource->GetFamily(), pSource->GetMask());
            // Setzen des Parents, der Folgevorlage
        }
        pFound[nFound].pSource = pSource;
        pFound[nFound].pDest = pDest;
        ++nFound;
        pSource = pSourcePool->Next();
    }

    for ( USHORT i = 0; i < nFound; ++i )
    {
        pFound[i].pDest->GetItemSet().PutExtended(pFound[i].pSource->GetItemSet(), SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
//      pFound[i].pDest->SetHelpId(pFound[i].pSource->GetHelpId());
        if(pFound[i].pSource->HasParentSupport())
            pFound[i].pDest->SetParent(pFound[i].pSource->GetParent());
        if(pFound[i].pSource->HasFollowSupport())
            pFound[i].pDest->SetFollow(pFound[i].pSource->GetParent());
    }
    delete pFound;
}

//--------------------------------------------------------------------

void SfxObjectShell::UpdateFromTemplate_Impl(  )

/*  [Beschreibung]

    Diese interne Methode pr"uft, ob das Dokument aus einem Template
    erzeugt wurde, und ob dieses neuer ist als das Dokument. Ist dies
    der Fall, wird der Benutzer gefragt, ob die Vorlagen (StyleSheets)
    updated werden sollen. Wird dies positiv beantwortet, werden die
    StyleSheets updated.
*/

{
    // Storage-medium?
    SfxMedium *pFile = GetMedium();
    DBG_ASSERT( pFile, "cannot UpdateFromTemplate without medium" );

    // only for own storage formats
    SvStorageRef xDocStor = pFile ? pFile->GetStorage() : 0;
    if ( !xDocStor.Is() || !pFile->GetFilter() || !pFile->GetFilter()->IsOwnFormat() )
        return;

    SFX_ITEMSET_ARG( pFile->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
    sal_Int16 bCanUpdateFromTemplate = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    // created from template?
    SfxDocumentInfo *pInfo = &GetDocInfo();
    String aTemplName( pInfo->GetTemplateName() );
    String aTemplFileName( pInfo->GetTemplateFileName() );
    String aFoundName;
    SvStorageRef aTemplStor;
    if ( aTemplName.Len() || aTemplFileName.Len() && !IsReadOnly() )
    {
        // try to locate template, first using filename
        // this must be done because writer global document uses this "great" idea to manage the templates of all parts
        // in the master document
        // but it is NOT an error if the template filename points not to a valid file
        SfxDocumentTemplates aTempl;
        aTempl.Construct();
        if ( aTemplFileName.Len() )
        {
            String aURL;
            if( ::utl::LocalFileHelper::ConvertSystemPathToURL( aTemplFileName, GetMedium()->GetName(), aURL ) )
            {
                aTemplStor = new SvStorage( aURL, STREAM_READ|STREAM_NOCREATE|STREAM_SHARE_DENYWRITE, STORAGE_TRANSACTED );
                if ( aTemplStor->GetError() )
                    aTemplStor.Clear();
                else
                    aFoundName = aURL;
            }
        }

        if( !aFoundName.Len() && aTemplName.Len() )
            // if the template filename did not lead to success, try to get a file name for the logical template name
            aTempl.GetFull( String(), aTemplName, aFoundName );
    }

    if ( aFoundName.Len() )
    {
        // check existence of template storage
        aTemplFileName = aFoundName;
        BOOL bLoad = FALSE;
        if ( !aTemplStor.Is() )
            aTemplStor = new SvStorage( aTemplFileName,
                            STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE, STORAGE_TRANSACTED );

        // should the document checked against changes in the template ?
        if ( !aTemplStor->GetError() && pInfo->IsQueryLoadTemplate() )
        {
            // load document info of template
            BOOL bOK = FALSE;
            DateTime aTemplDate;
            Reference < document::XStandaloneDocumentInfo > xDocInfo (
                    ::comphelper::getProcessServiceFactory()->createInstance(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.StandaloneDocumentInfo") ) ), UNO_QUERY );
            Reference < beans::XFastPropertySet > xSet( xDocInfo, UNO_QUERY );
            if ( xDocInfo.is() && xSet.is() )
            {
                try
                {
                    xDocInfo->loadFromURL( aTemplFileName );
                    Any aAny = xSet->getFastPropertyValue( WID_DATE_MODIFIED );
                    ::com::sun::star::util::DateTime aTmp;
                    if ( aAny >>= aTmp )
                    {
                        // get modify date from document info
                        aTemplDate = SfxDocumentInfoObject::impl_DateTime_Struct2Object( aTmp );
                        bOK = TRUE;
                    }
                }
                catch ( Exception& )
                {
                }
            }

            // if modify date was read successfully
            if ( bOK )
            {
                // compare modify data of template with the last check date of the document
                const DateTime aInfoDate( pInfo->GetTemplateDate(), pInfo->GetTemplateDate() );
                if ( aTemplDate > aInfoDate )
                {
                    // ask user
                    if( bCanUpdateFromTemplate == document::UpdateDocMode::QUIET_UPDATE
                     || bCanUpdateFromTemplate == document::UpdateDocMode::FULL_UPDATE )
                        bLoad = TRUE;
                    else if ( bCanUpdateFromTemplate == document::UpdateDocMode::ACCORDING_TO_CONFIG )
                    {
                        QueryBox aBox( GetDialogParent(), SfxResId(MSG_QUERY_LOAD_TEMPLATE) );
                        if ( RET_YES == aBox.Execute() )
                            bLoad = TRUE;
                    }

                    if( !bLoad )
                    {
                        // user refuses, so don't ask again for this document
                        pInfo->SetQueryLoadTemplate(FALSE);

                        if ( xDocStor->IsOLEStorage() )
                            pInfo->Save(xDocStor);
                        else
                            SetModified( TRUE );
                    }
                }
            }

            if ( bLoad )
            {
                // styles should be updated, create document in organizer mode to read in the styles
                SfxObjectShellLock xTemplDoc = GetFactory().CreateObject( SFX_CREATE_MODE_ORGANIZER );
                xTemplDoc->DoInitNew(0);
                String aOldBaseURL = INetURLObject::GetBaseURL();
                INetURLObject::SetBaseURL( INetURLObject( aTemplFileName ).GetMainURL( INetURLObject::NO_DECODE ) );
                if ( xTemplDoc->LoadFrom(aTemplStor) )
                {
                    // transfer styles from xTemplDoc to this document
                    LoadStyles(*xTemplDoc);

                    // remember date/time of check
                    pInfo->SetTemplateDate(aTemplDate);
                    pInfo->Save(xDocStor);
                }

                INetURLObject::SetBaseURL( aOldBaseURL );
            }
/*
            SfxConfigManager *pCfgMgr = SFX_CFGMANAGER();
            BOOL bConfig = pInfo->HasTemplateConfig();
            {
                SfxConfigManager *pTemplCfg = new SfxConfigManager(aTemplStor, pCfgMgr);
                SetConfigManager(pTemplCfg);
                SetTemplateConfig(TRUE);

                // Falls der gerade zerst"orte CfgMgr des Dokuments der
                // aktive war, pCfgMgr lieber neu holen
                pCfgMgr = SFX_CFGMANAGER();

                // ggf. den neuen ConfigManager aktivieren
                if ( this == SfxObjectShell::Current() )
                    pTemplCfg->Activate(pCfgMgr);
            }
*/
            // Template und Template-DocInfo werden nicht mehr gebraucht
//            delete pTemplInfo;
        }
    }
}

SfxEventConfigItem_Impl* SfxObjectShell::GetEventConfig_Impl( BOOL bForce )
{
    if ( bForce && !pImp->pEventConfig )
    {
        pImp->pEventConfig = new SfxEventConfigItem_Impl( SFX_ITEMTYPE_DOCEVENTCONFIG,
                    SFX_APP()->GetEventConfig(), this );
        if (pImp->pCfgMgr)
            pImp->pEventConfig->Connect( pImp->pCfgMgr );
        pImp->pEventConfig->Initialize();
    }

    return pImp->pEventConfig;
}

SvStorageRef SfxObjectShell::GetConfigurationStorage( SotStorage* pStor )
{
    // configuration storage shall be opened in own storage or a new storage, if the
    // document is getting stored into this storage
    if ( !pStor )
        pStor = GetStorage();

    if ( pStor->IsOLEStorage() )
                return (SvStorageRef) SotStorageRef();

    // storage is always opened in transacted mode, so changes must be commited
    SotStorageRef xStorage = pStor->OpenSotStorage( DEFINE_CONST_UNICODE("Configurations"),
                IsReadOnly() ? STREAM_STD_READ : STREAM_STD_READWRITE );
    if ( xStorage.Is() && xStorage->GetError() )
        xStorage.Clear();
        return (SvStorageRef) xStorage;
}

SotStorageStreamRef SfxObjectShell::GetConfigurationStream( const String& rName, BOOL bCreate )
{
    SotStorageStreamRef xStream;
    SvStorageRef xStorage = GetConfigurationStorage();
    if ( xStorage.Is() )
    {
        xStream = xStorage->OpenSotStream( rName,
            bCreate ? STREAM_STD_READWRITE|STREAM_TRUNC : STREAM_STD_READ );
        if ( xStream.Is() && xStream->GetError() )
            xStream.Clear();
    }

    return xStream;
}

SfxAcceleratorManager* SfxObjectShell::GetAccMgr_Impl()
{
    // already constructed ?!
    if ( pImp->pAccMgr )
        return pImp->pAccMgr;

    // get the typId ( = ResourceId )
    const ResId* pResId = GetFactory().GetAccelId();
    if ( !pResId )
        return NULL;

    if ( GetConfigManager() && pImp->pCfgMgr->HasConfigItem( pResId->GetId() ) )
    {
        // document has configuration
        pImp->pAccMgr = new SfxAcceleratorManager( *pResId, pImp->pCfgMgr );
        return pImp->pAccMgr;
    }
    else
        return GetFactory().GetAccMgr_Impl();
}

SfxMenuBarManager* SfxObjectShell::CreateMenuBarManager_Impl( SfxViewFrame* pViewFrame )
{
    SfxBindings& rBindings = pViewFrame->GetBindings();
    sal_Bool bCheckPlugin = SfxApplication::IsPlugin();
    const ResId* pId = bCheckPlugin ? GetFactory().GetPluginMenuBarId() : GetFactory().GetMenuBarId();
    DBG_ASSERT( pId && pId->GetId(), "Component must have own window!" );
    if ( !pId )
        return NULL;

    SfxConfigManager *pCfgMgr = SFX_APP()->GetConfigManager_Impl();
    if ( GetConfigManager() && pImp->pCfgMgr->HasConfigItem( pId->GetId() ) )
        pCfgMgr = pImp->pCfgMgr;

    SfxMenuBarManager* pMgr = new SfxMenuBarManager( *pId, rBindings, pCfgMgr, pViewFrame->ISA( SfxInPlaceFrame ) );
    return pMgr;
}

SfxImageManager* SfxObjectShell::GetImageManager_Impl()
{
    if ( pImp->pImageManager )
        return pImp->pImageManager;

    // every document has its own ImageManager, but they may use the global configuration!
    pImp->pImageManager = new SfxImageManager( this );
    return pImp->pImageManager;
}

SfxObjectShellRef MakeObjectShellForOrganizer_Impl( const String& aTargetURL, BOOL bForWriting )
{
    // check for own format
    SfxObjectShellRef xDoc;
    SfxMedium *pMed = new SfxMedium( aTargetURL, STREAM_STD_READ, FALSE, 0 );
    const SfxFilter* pFilter = NULL;
    if( SFX_APP()->GetFilterMatcher().GuessFilter( *pMed, &pFilter ) == ERRCODE_NONE && pFilter && pFilter->IsOwnFormat() )
    {
        delete pMed;
        StreamMode nMode = bForWriting ? STREAM_STD_READWRITE : STREAM_STD_READ;
        SvStorageRef xStor = new SvStorage( aTargetURL, nMode, STORAGE_TRANSACTED );
        xStor->SetVersion( pFilter->GetVersion() );
        if ( SVSTREAM_OK == xStor->GetError() )
        {
            // create document
            xDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName(), SFX_CREATE_MODE_ORGANIZER );
            if ( xDoc.Is() )
            {
                // partially load, so don't use DoLoad!
                xDoc->DoInitNew(0);
                if( !xDoc->LoadFrom( xStor ) )
                    xDoc.Clear();
                else
                {
                    // connect to storage, abandon temp. storage
                    xDoc->DoHandsOff();
                    xDoc->DoSaveCompleted( xStor );
                }
            }
        }
    }
    else
        delete pMed;

    return xDoc;
}

SfxToolBoxConfig* SfxObjectShell::GetToolBoxConfig_Impl()
{
    if ( !pImp->pTbxConfig )
    {
        pImp->pTbxConfig = new SfxToolBoxConfig(
            GetConfigManager() ? pImp->pCfgMgr : SFX_APP()->GetConfigManager_Impl() );
    }

    return pImp->pTbxConfig;
}


sal_Bool SfxObjectShell::IsHelpDocument() const
{
    const SfxFilter* pFilter = GetMedium()->GetFilter();
    return ( pFilter && pFilter->GetFilterName().CompareToAscii("writer_web_HTML_help") == COMPARE_EQUAL );
}
