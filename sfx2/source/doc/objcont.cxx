/*************************************************************************
 *
 *  $RCSfile: objcont.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:32 $
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

#include "docfile.hxx"

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

#include "sfxresid.hxx"
#include "saveopt.hxx"
#include "stbmgr.hxx"
#include "dinfdlg.hxx"
#include "fltfnc.hxx"
#include "docfac.hxx"
#include "saveopt.hxx"
#include "cfgmgr.hxx"
#include "inimgr.hxx"
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

//====================================================================

#define SFX_WINDOWS_STREAM "SfxWindows"
#define SFX_PREVIEW_STREAM "SfxPreview"

//====================================================================

GDIMetaFile* SfxObjectShell::GetPreviewMetaFile(  ) const
{
    // Nur wenn gerade nicht gedruckt wird, darf DoDraw aufgerufen
    // werden, sonst wird u.U. der Printer abgeschossen !
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
    if ( pFrame && pFrame->GetViewShell() &&
         pFrame->GetViewShell()->GetPrinter() &&
         pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
         return 0;

    Size aTmpSize = ((SfxObjectShell*)this)->GetFirstPageSize();
    GDIMetaFile* pFile = new GDIMetaFile;
    pFile->SetPrefSize( aTmpSize );
    DBG_ASSERT( aTmpSize.Height()*aTmpSize.Width(),
                "size of first page is 0, overload GetFirstPageSize or set vis-area!" );
#define FRAME 4

    VirtualDevice aDevice;
    aDevice.EnableOutput( FALSE );
    SfxInPlaceObject* pObj = GetInPlaceObject();
    if( pObj )
    {
        MapMode aMode( pObj->GetMapUnit() );
        aDevice.SetMapMode( aMode );
    }
    pFile->Record( &aDevice );
    SfxInPlaceObject* pInPlaceObj = GetInPlaceObject();
    DBG_ASSERT( pInPlaceObj, "Ohne Inplace Objekt keine Grafik" );
    if (pInPlaceObj)
        pInPlaceObj->DoDraw(
            &aDevice, Point(0,0), aTmpSize,
            JobSetup(), ASPECT_THUMBNAIL );
    pFile->Stop();
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
    if ( pActFrame->GetObjectShell() != this )
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

            char cToken = SfxIniManager::GetToken();
            const BOOL bActWin = pActFrame == pFrame;
            String aUserData;
            pFrame->GetViewShell()->WriteUserData(aUserData);

            // assemble ini-data
            String aWinData;
            aWinData += pFrame->GetCurViewId();
            aWinData += cToken;
/*
            if ( !pWin || pWin->IsMaximized() )
                aWinData += SFX_WINSIZE_MAX;
            else if ( pWin->IsMinimized() )
                aWinData += SFX_WINSIZE_MIN;
            else
*/
            aWinData += SfxIniManager::GetString( pWin->GetPosPixel(), pWin->GetSizePixel() );
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

    const SfxFilter* pFilter;
    if( GetMedium() && ( pFilter = GetMedium()->GetFilter() ) && !pFilter->UsesStorage() )
        return 0;

    // Modus bestimmen
    SfxApplication *pSfxApp = SFX_APP();
    SfxViewFrame *pPrefered = pPreferedFrame ? pPreferedFrame->GetCurrentViewFrame() : 0;
    SfxOptions &rOpt = pSfxApp->GetOptions();
    BOOL bLoadDocWins = rOpt.IsSaveDocWins() && !pPrefered;
    BOOL bLoadDocView = rOpt.IsSaveDocView();

    // In a StarPortal not possible at the moment
    bLoadDocWins = FALSE;

    // gar nichts laden?
    if ( !bLoadDocView )
        return 0;

    // Sub-Stream "offnen
    SvStorage *pStor = GetStorage();
    SvStorageStreamRef xStream = pStor ? pStor->OpenStream( DEFINE_CONST_UNICODE( SFX_WINDOWS_STREAM ), STREAM_STD_READ ) : 0;
    if ( !xStream )
        return 0;

    BOOL bOldFormat = TRUE;
    xStream->SetVersion( pStor->GetVersion() );
    xStream->SetBufferSize(1024);

    // alle gespeicherten Fenster "offnen
    SfxViewFrame *pActiveFrame = 0;
    String aWinData;
    char cToken = SfxIniManager::GetToken();
    SfxItemSet *pSet = GetMedium()->GetItemSet();

    pImp->bLoadingWindows = TRUE;
    BOOL bLoaded = FALSE;
    while ( xStream->ReadByteString( aWinData ), aWinData.Len() )
    {
        if ( aWinData.GetToken( 0, cToken ).EqualsAscii( "TASK" ) )
        {
            // doesn't make any sense with the new task handling using system tasks or browser windows
            bOldFormat = FALSE;
        }
        else
        {
            USHORT nViewId = (USHORT) aWinData.GetToken( 0, cToken ).ToInt32();
            FASTBOOL bActive=FALSE, bMaximized=FALSE;
            String aUserData, aPosSize;
            if ( bOldFormat )
            {
                // Old format
                aPosSize = aWinData.GetToken( 1, cToken );
                aPosSize.ToLowerAscii();
                aUserData = aWinData.GetToken( 2, cToken );
                bActive = aWinData.GetToken( 3, cToken ).ToInt32();

                if ( aPosSize.EqualsAscii( SFX_WINSIZE_MAX ) )
                    bMaximized = TRUE;
                else if ( aPosSize.EqualsAscii( SFX_WINSIZE_MIN ) )
                {
                    bMaximized = TRUE;
                    bActive = FALSE;
                }
                else
                    bMaximized = FALSE;
            }
            else
            {
                // 5.0-Format, get activity state and UserData
                USHORT nPos=0;
                bActive = aWinData.GetToken( 3, cToken, nPos ).ToInt32();
                aUserData = aWinData.Copy( nPos );
            }

            Point aPt;
            Size aSz;
            if ( !bMaximized )
                SfxIniManager::GetPosSize( aPosSize, aPt, aSz );

            // nur aktives soll geladen werden, es ist aber nicht das aktive?
            if ( !bLoadDocWins && !bActive )
                continue;

            pSet->ClearItem( SID_USER_DATA );

            SfxViewFrame *pFrame = 0;
            if ( pPrefered )
            {
                // dann den mitgegebenen Frame verwenden, aber keine Gr"o\se
                // am Window setzen
                pFrame = pPrefered;
                if ( pFrame->GetViewShell() || !pFrame->GetObjectShell() )
                {
                    pSet->ClearItem( SID_VIEW_POS_SIZE );
                    pSet->ClearItem( SID_WIN_POSSIZE );
                    pSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );

                    // Flackern vermeiden
                    SfxBindings &rBind = pFrame->GetBindings();
                    rBind.ENTERREGISTRATIONS();

                    // dann ausr"aumen und das eigene Doc reinsetzen
                    pPreferedFrame->InsertDocument( this );

                    // Updating reaktivieren
                    rBind.LEAVEREGISTRATIONS();
                }
                else
                {
                    // sonst neue View erzeugen
                    pFrame->CreateView_Impl( nViewId );
                }
            }
            else
            {
                if ( bLoadDocWins )
                {
                    // Im Hintergrund "offnen
                    pSet->Put( SfxUInt16Item( SID_VIEW_ZOOM_MODE, 0 ) );
                    if ( !bMaximized )
                        pSet->Put( SfxRectangleItem( SID_VIEW_POS_SIZE, Rectangle( aPt, aSz ) ) );
                }

                pSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );
                pPreferedFrame->InsertDocument( this );
                pFrame = pPreferedFrame->GetCurrentViewFrame();

                // Wird nicht mehr gebraucht
                pSet->ClearItem( SID_VIEW_POS_SIZE );
                pSet->ClearItem( SID_WIN_POSSIZE );
                pSet->ClearItem( SID_VIEW_ZOOM_MODE );
            }

            bLoaded = TRUE;

            // UserData hier einlesen, da es ansonsten immer mit bBrowse=TRUE
            // aufgerufen wird, beim Abspeichern wurde aber bBrowse=FALSE verwendet
            if ( pFrame && pFrame->GetViewShell() )
                pFrame->GetViewShell()->ReadUserData( aUserData, !bLoadDocWins );

            // Evtl. sollen noch weitere Fenster geladen werden
            pPreferedFrame = NULL;

            if ( bActive )
                pActiveFrame = pFrame;

            if( pPrefered || !bLoadDocWins )
                // Es sollte nur das aktive Window geladen werden
                break;
        }
    }

    if ( pActiveFrame )
    {
        if ( !pPrefered )
            // Den zu aktivierenden Frame aktivieren
            pActiveFrame->MakeActive_Impl( TRUE );
    }

    pImp->bLoadingWindows = FALSE;
    return pPrefered && bLoaded ? pPrefered : pActiveFrame;
}

//====================================================================

void SfxObjectShell::UpdateDocInfoForSave()
{
    SfxDocumentInfo &rDocInfo = GetDocInfo();
    rDocInfo.SetTemplateConfig( HasTemplateConfig() );

    if ( IsModified() )
    {
        // Keine Unterschiede mehr zwischen Save, SaveAs
        String aUserName = SFX_INIMANAGER()->GetUserFullName();
        if ( !rDocInfo.IsUseUserData() )
            aUserName.Erase();

        rDocInfo.SetChanged( aUserName );
        if ( !HasName() || pImp->bIsSaving )
            UpdateTime_Impl( rDocInfo );
    }

    if ( !pImp->bIsSaving )
    {
        // Flag fuer Passwort merken
        rDocInfo.SetPasswd( pImp->bPasswd );

        // ggf. DocInfo Dialog
        if (  !pImp->bSilent && eCreateMode == SFX_CREATE_MODE_STANDARD &&
            0 == ( pImp->eFlags & SFXOBJECTSHELL_NODOCINFO ) && SFX_APP()->GetOptions().IsDocInfoSave() )
        {
//(mba)/task
/*
            Window* pWindow = Application::GetAppWindow();
            if ( pWindow )
                pWindow->EnterWait();
 */
            DocInfoDlg_Impl( rDocInfo );
/*
            if ( pWindow )
                pWindow->LeaveWait();
 */
        }
    }

    Broadcast( SfxDocumentInfoHint( &rDocInfo ) );
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
                SfxBasicManager::CopyBasicData( GetStorage(), aURL, pNewStg );
#endif
            }

            // Windows-merken
            if ( TRUE ) HACK(aus config)
                SaveWindows_Impl( *pNewStg );

            // Konfiguration schreiben
            if ( GetConfigManager() )
            {
                if ( rDocInfo.HasTemplateConfig() )
                {
                    const String aTemplFileName( rDocInfo.GetTemplateFileName() );
                    if ( aTemplFileName.Len() )
                    {
                        INetURLObject aURL( aTemplFileName, INET_PROT_FILE );
                        SvStorageRef aStor = new SvStorage( aURL.GetMainURL() );
                        if ( SVSTREAM_OK == aStor->GetError() )
                        {
                            GetConfigManager()->SaveConfig(aStor);
                            if (aRef->IsStream(SfxConfigManager::GetStreamName()))
                                aRef->Remove(SfxConfigManager::GetStreamName());
                        }
                    }
                }
                else
                {
                    GetConfigManager()->SetModified( TRUE );
                    GetConfigManager()->SaveConfig(pNewStg);
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
            SfxBasicManager::CopyBasicData( GetStorage(), aURL, pNewStg );
#endif
        }
#endif
        // Windows-merken
        if ( TRUE ) HACK(aus config)
            SaveWindows_Impl( *pNewStg );

        // Konfiguration schreiben
        if (GetConfigManager())
        {
            if ( rDocInfo.HasTemplateConfig() )
            {
                const String aTemplFileName( rDocInfo.GetTemplateFileName() );
                if ( aTemplFileName.Len() )
                {
                    INetURLObject aURL( aTemplFileName, INET_PROT_FILE );
                    SvStorageRef aStor = new SvStorage( aURL.GetMainURL() );
                    if ( SVSTREAM_OK == aStor->GetError() )
                    {
                        GetConfigManager()->SaveConfig(aStor);
                        if (pNewStg->IsStream(SfxConfigManager::GetStreamName()))
                            pNewStg->Remove(SfxConfigManager::GetStreamName());
                    }
                }
            }
            else
            {
                GetConfigManager()->SetModified( TRUE );
                GetConfigManager()->SaveConfig(pNewStg);
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
            nAddTime    =   aNow-pImp->nTime ;
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
            nAddTime    +=  n24Time-pImp->nTime     ;
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
            aURL += aURLObj.GetMainURL();
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
        aName = DEFINE_CONST_UNICODE( "Object " );
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

SfxConfigManager* SfxObjectShell::GetConfigManager() const
{
    return pImp->pCfgMgr;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetConfigManager(SfxConfigManager *pMgr)
{
    if (pImp->pCfgMgr == SFX_CFGMANAGER() && pMgr)
        pMgr->Activate(pImp->pCfgMgr);

    if (pImp->pCfgMgr)
        delete pImp->pCfgMgr;

    pImp->pCfgMgr = pMgr;
    pMgr->SetObjectShell(this);
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
//    return pImp->bTemplateConfig;
    return FALSE;
}

//-------------------------------------------------------------------------

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

        case CONTENT_CONFIG:
            return (GetConfigManager() && !HasTemplateConfig()) ?
                        GetConfigManager()->GetItemCount() : 0;
            break;
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
    case CONTENT_CONFIG:
        return INDEX_IGNORE == nIdx2 ? FALSE : TRUE;
    }
    return FALSE;
}

//--------------------------------------------------------------------

void   SfxObjectShell::GetContent(String &rText,
                                        Bitmap &rClosedBitmap,
                                        Bitmap &rOpenedBitmap,
                                        BOOL &bCanDel,
                                        USHORT i,
                                        USHORT nIdx1,
                                        USHORT nIdx2)
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
                    nClosedBitmapResId= BMP_STYLES_CLOSED;
                    nOpenedBitmapResId= BMP_STYLES_OPENED;
                    break;
                case CONTENT_MACRO:
                    nTextResId = STR_MACROS;
                    nClosedBitmapResId= BMP_STYLES_CLOSED;
                    nOpenedBitmapResId= BMP_STYLES_OPENED;
                    break;
                case CONTENT_CONFIG:
                    nTextResId = STR_CONFIG;
                    nClosedBitmapResId= BMP_STYLES_CLOSED;
                    nOpenedBitmapResId= BMP_STYLES_OPENED;
                    break;
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
                GetStyleFamilyBitmap(pStyle->GetFamily());
        }
            break;
        case CONTENT_MACRO:
            break;
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
    }
}

//--------------------------------------------------------------------

Bitmap SfxObjectShell::GetStyleFamilyBitmap(SfxStyleFamily eFamily)
{
    USHORT nResId = 0;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            nResId = BMP_STYLES_FAMILY1;
            break;
        case SFX_STYLE_FAMILY_PARA:
            nResId = BMP_STYLES_FAMILY2;
            break;
        case SFX_STYLE_FAMILY_FRAME:
            nResId = BMP_STYLES_FAMILY3;
            break;
        case SFX_STYLE_FAMILY_PAGE :
            nResId = BMP_STYLES_FAMILY4;
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
    else if (nIdx1 == CONTENT_CONFIG)
    {
        if (GetConfigManager()->RemoveItem(nIdx2))
        {
            SetModified(TRUE);
            bRet = TRUE;
            SFX_APP()->GetDispatcher_Impl()->Update_Impl(TRUE);
        }
    }
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
            ULONG nTextHeight( rPrt.GetTextHeight() );
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
                    aPageSize.Height() - nYIndent )
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
                    ULONG   nTextWidth;
                    nIdx = aStr.Search(cDelim, nStart);
                    nTextWidth = rPrt.GetTextWidth(aStr, nStart, nIdx-nStart);
                    while(nIdx != STRING_NOTFOUND &&
                          aOutPos.X() + nTextWidth <
                          aPageSize.Width() - nXIndent)
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
    SvStorageRef xDocStor = pFile && pFile->IsStorage() ? pFile->GetStorage() : 0;
    if ( !xDocStor.Is() || !pFile->GetFilter() || !pFile->GetFilter()->IsOwnFormat() )
        return;

    // created from template?
    SfxDocumentInfo *pInfo = &GetDocInfo();
    String aTemplName( pInfo->GetTemplateName() );
    String aTemplFileName( pInfo->GetTemplateFileName() );
    String aFoundName;
    SvStorageRef aTemplStor;
    if ( aTemplName.Len() || aTemplFileName.Len() )
    {
        // try to locate template, first using filename
        // this must be done because writer global document uses this "great" idea to manage the templates of all parts
        // in the master document
        // but it is NOT an error if the template filename points not to a valid file
        SfxDocumentTemplates aTempl;
        aTempl.Construct();
        if ( aTemplFileName.Len() && !IsReadOnly() )
        {
            INetURLObject aURL( aTemplFileName, INET_PROT_FILE );
            aTemplStor = new SvStorage( aURL.GetMainURL(),
                                    STREAM_READ | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYWRITE, STORAGE_TRANSACTED );
            if ( aTemplStor->GetError() )
                aTemplStor.Clear();
            else
                aFoundName = aTemplFileName;
        }

        // if the template filename did not lead to success, try the template logical name
        if( !aFoundName.Len() && aTempl.GetFull( String(), aTemplName, aFoundName ) && !IsReadOnly() )
        {
            // template found, check if comparing filenames is enabled
            if( (USHORT)SFX_INIMANAGER()->Get( SFX_KEY_SEARCHTEMPLATE ).ToInt32() && aTemplFileName != aFoundName )
            {
                // template with given template name was found but with a different filename
                SfxMedium aSfxMedium( aFoundName, STREAM_READ | STREAM_SHARE_DENYNONE, FALSE );
                const SfxFilter* pFilter = NULL;
                SFX_APP()->GetFilterMatcher().GuessFilter( aSfxMedium, &pFilter, SFX_FILTER_IMPORT | SFX_FILTER_TEMPLATE );
                if ( pFilter && pFilter->GetFilterContainer() == pFile->GetFilter()->GetFilterContainer() )
                {
                    String aMsg( SfxResId( STR_TEMPL_MOVED ) );
                    aMsg.SearchAndReplace( DEFINE_CONST_UNICODE( "$(TEMPLATE)" ), aTemplName );
                    aMsg.SearchAndReplace( DEFINE_CONST_UNICODE( "$(FOUND)" ), aFoundName );
                    if( QueryBox( 0, WB_YES_NO, aMsg ).Execute() == RET_YES )
                    {
                        pInfo->SetTemplateFileName( aFoundName );
                        FlushDocInfo();
                        SetModified( TRUE );
                    }
                    else
                        aFoundName.Erase();
                }
                else
                    aFoundName.Erase();
            }
        }
        else if ( !IsReadOnly() )
        {
            // template not found, ask user for removing template link
            String aMsg( SfxResId( STR_TEMPL_RESET ) );
            aMsg.SearchAndReplace( DEFINE_CONST_UNICODE( "$(TEMPLATE)" ), aTemplName );
            if( QueryBox( 0, WB_YES_NO, aMsg ).Execute() == RET_NO )
            {
                String aStr;
                pInfo->SetTemplateFileName( aStr );
                pInfo->SetTemplateName( aStr );
                FlushDocInfo();
                SetModified( TRUE );
            }
        }
    }

    if ( aFoundName.Len() )
    {
        aTemplFileName = aFoundName;
        BOOL bLoad = FALSE;
        INetURLObject aURL( aTemplFileName, INET_PROT_FILE );
        if ( !aTemplStor.Is() )
            aTemplStor = new SvStorage( aURL.GetMainURL(),
                                    STREAM_READ | STREAM_NOCREATE |
                                    STREAM_SHARE_DENYWRITE, STORAGE_TRANSACTED );

        if ( !aTemplStor->GetError() )
        {
            // Template-DocInfo laden
            SfxDocumentInfo *pTemplInfo = new SfxDocumentInfo;
            DateTime aTemplDate;
            if ( pTemplInfo->Load(aTemplStor) )
                aTemplDate = pTemplInfo->GetChanged().GetTime();

            // soll der Benutzer gefragt werden?
            if ( pInfo->IsQueryLoadTemplate() )
            {
                // ist das Template neuer?
                const DateTime aInfoDate( pInfo->GetTemplateDate(),
                                          pInfo->GetTemplateDate() );
                if ( aTemplDate > aInfoDate )
                {
                    // Benutzer fragen, ob update
                    QueryBox aBox( 0, SfxResId(MSG_QUERY_LOAD_TEMPLATE) );
                    if ( RET_YES == aBox.Execute() )
                        bLoad = TRUE;
                    else
                    {
                        // nein => am Doc merken, da\s nie wieder gefragt wird
                        pInfo->SetQueryLoadTemplate(FALSE);
                        pInfo->Save(xDocStor);
                    }
                }
            }

            // StyleSheets aus Template updaten?
            SfxConfigManager *pCfgMgr = SFX_CFGMANAGER();
            if ( bLoad )
            {
                // Document-Instanz f"ur das Template erzeugen und laden
                SfxObjectShellLock xTemplDoc =
                            GetFactory().CreateObject(SFX_CREATE_MODE_ORGANIZER);
                xTemplDoc->DoInitNew(0);
                String aOldBaseURL = INetURLObject::GetBaseURL();
                INetURLObject::SetBaseURL( INetURLObject( aTemplFileName, INET_PROT_FILE ).GetMainURL() );
                if ( xTemplDoc->LoadFrom(aTemplStor) )
                {
                    // StyleSheets ins eigene Doc laden
                    LoadStyles(*xTemplDoc);

                    // Update im Doc merken
                    pInfo->SetTemplateDate(aTemplDate);
                    pInfo->Save(xDocStor);
                }

                INetURLObject::SetBaseURL( aOldBaseURL );
            }
/*
            // Config aus Template laden?
            BOOL bConfig = pInfo->HasTemplateConfig();
            {
                // Config-Manager aus Template-Storage erzeugen
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
            delete pTemplInfo;
        }
    }
}

SfxEventConfigItem_Impl* SfxObjectShell::GetEventConfig_Impl( BOOL bForce )
{
    SfxApplication *pApp = SFX_APP();

    if ( bForce || pImp->pCfgMgr )
    {
        // Wenn kein bForce, aber es eine DocConfig gibt, mu\s auch eine
        // EventConfig angelegt werden
        if ( !pImp->pCfgMgr )
        {
            // Es soll eine EventConfig konfiguriert werden, dazu mu\s sie am
            // ConfigManager des Dokuments h"angen
            SfxConfigManager *pMgr = pApp->GetConfigManager();
            SetConfigManager( new SfxConfigManager( 0, pMgr ) );
            if ( this == SfxObjectShell::Current() )
                pImp->pCfgMgr->Activate( pMgr );
        }

        // Gegebenenfalls EventConfig erzeugen und ans Dokument konfigurieren
        if ( !pImp->pEventConfig )
        {
            pImp->pEventConfig =
                new SfxEventConfigItem_Impl( SFX_ITEMTYPE_DOCEVENTCONFIG,
                    pApp->GetEventConfig() );
            pImp->pEventConfig->Init( pImp->pCfgMgr );
        }
    }

    return pImp->pEventConfig;
}


