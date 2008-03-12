/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objcont.cxx,v $
 *
 *  $Revision: 1.74 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:23:03 $
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
#include "precompiled_sfx2.hxx"

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTANDALONEDOCUMENTINFO_HPP_
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
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

#include <svtools/stritem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/rectitem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/urihelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>

#ifndef INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX
#include <svtools/securityoptions.hxx>
#endif
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
#include <vcl/virdev.hxx>

#include <sfx2/app.hxx>
#include "sfxresid.hxx"
#include "appdata.hxx"
#include <sfx2/dinfdlg.hxx>
#include "fltfnc.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/evntconf.hxx>
#include "sfxhelp.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/topfrm.hxx>
#include "basmgr.hxx"
#include <sfx2/doctempl.hxx>
#include "doc.hrc"
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include "openflag.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//====================================================================

#define SFX_WINDOWS_STREAM "SfxWindows"
#define SFX_PREVIEW_STREAM "SfxPreview"

//====================================================================

/*
static
bool operator< (const util::DateTime& i_rLeft, const util::DateTime& i_rRight)
{
    return i_rLeft.Year             < i_rRight.Year
        || i_rLeft.Month            < i_rRight.Month
        || i_rLeft.Day              < i_rRight.Day
        || i_rLeft.Hours            < i_rRight.Hours
        || i_rLeft.Minutes          < i_rRight.Minutes
        || i_rLeft.Seconds          < i_rRight.Seconds
        || i_rLeft.HundredthSeconds < i_rRight.HundredthSeconds;
}
*/

static
bool operator> (const util::DateTime& i_rLeft, const util::DateTime& i_rRight)
{
    return i_rLeft.Year             > i_rRight.Year
        || i_rLeft.Month            > i_rRight.Month
        || i_rLeft.Day              > i_rRight.Day
        || i_rLeft.Hours            > i_rRight.Hours
        || i_rLeft.Minutes          > i_rRight.Minutes
        || i_rLeft.Seconds          > i_rRight.Seconds
        || i_rLeft.HundredthSeconds > i_rRight.HundredthSeconds;
}


::boost::shared_ptr<GDIMetaFile>
SfxObjectShell::GetPreviewMetaFile( sal_Bool bFullContent ) const
{
    return CreatePreviewMetaFile_Impl( bFullContent, sal_False );
}


::boost::shared_ptr<GDIMetaFile>
SfxObjectShell::CreatePreviewMetaFile_Impl( sal_Bool bFullContent, sal_Bool bHighContrast ) const
{
    // Nur wenn gerade nicht gedruckt wird, darf DoDraw aufgerufen
    // werden, sonst wird u.U. der Printer abgeschossen !
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
    if ( pFrame && pFrame->GetViewShell() &&
         pFrame->GetViewShell()->GetPrinter() &&
         pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
         return ::boost::shared_ptr<GDIMetaFile>();

    ::boost::shared_ptr<GDIMetaFile> pFile(new GDIMetaFile);

    VirtualDevice aDevice;
    aDevice.EnableOutput( FALSE );

    // adjust the output device if HC-metafile is requested
    if ( bHighContrast )
        aDevice.SetDrawMode( aDevice.GetDrawMode() | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );

    MapMode aMode( ((SfxObjectShell*)this)->GetMapUnit() );
    aDevice.SetMapMode( aMode );
    pFile->SetPrefMapMode( aMode );

    Size aTmpSize;
    sal_Int8 nAspect;
    if ( bFullContent )
    {
        nAspect = ASPECT_CONTENT;
        aTmpSize = GetVisArea( nAspect ).GetSize();
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
    ((SfxObjectShell*)this)->DoDraw( &aDevice, Point(0,0), aTmpSize, JobSetup(), nAspect );
    pFile->Stop();

    return pFile;
}

//REMOVE    FASTBOOL SfxObjectShell::SaveWindows_Impl( SvStorage &rStor ) const
//REMOVE    {
//REMOVE        SvStorageStreamRef xStream = rStor.OpenStream( DEFINE_CONST_UNICODE( SFX_WINDOWS_STREAM ),
//REMOVE                                        STREAM_TRUNC | STREAM_STD_READWRITE);
//REMOVE        if ( !xStream )
//REMOVE            return FALSE;
//REMOVE
//REMOVE        xStream->SetBufferSize(1024);
//REMOVE        xStream->SetVersion( rStor.GetVersion() );
//REMOVE
//REMOVE        // "uber alle Fenster iterieren (aber aktives Window zuletzt)
//REMOVE        SfxViewFrame *pActFrame = SfxViewFrame::Current();
//REMOVE        if ( !pActFrame || pActFrame->GetObjectShell() != this )
//REMOVE            pActFrame = SfxViewFrame::GetFirst(this);
//REMOVE
//REMOVE        String aActWinData;
//REMOVE        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this, TYPE(SfxTopViewFrame) ); pFrame;
//REMOVE                pFrame = SfxViewFrame::GetNext(*pFrame, this, TYPE(SfxTopViewFrame) ) )
//REMOVE        {
//REMOVE            // Bei Dokumenten, die Outplace aktiv sind, kann beim Speichern auch schon die View weg sein!
//REMOVE            if ( pFrame->GetViewShell() )
//REMOVE            {
//REMOVE                SfxTopFrame* pTop = (SfxTopFrame*) pFrame->GetFrame();
//REMOVE                pTop->GetTopWindow_Impl();
//REMOVE
//REMOVE                char cToken = ',';
//REMOVE                const BOOL bActWin = pActFrame == pFrame;
//REMOVE                String aUserData;
//REMOVE                pFrame->GetViewShell()->WriteUserData(aUserData);
//REMOVE
//REMOVE                // assemble ini-data
//REMOVE                String aWinData;
//REMOVE                aWinData += String::CreateFromInt32( pFrame->GetCurViewId() );
//REMOVE                aWinData += cToken;
//REMOVE    /*
//REMOVE                if ( !pWin || pWin->IsMaximized() )
//REMOVE                    aWinData += SFX_WINSIZE_MAX;
//REMOVE                else if ( pWin->IsMinimized() )
//REMOVE                    aWinData += SFX_WINSIZE_MIN;
//REMOVE                else
//REMOVE    */
//REMOVE                aWinData += cToken;
//REMOVE                aWinData += aUserData;
//REMOVE
//REMOVE                // aktives kennzeichnen
//REMOVE                aWinData += cToken;
//REMOVE                aWinData += bActWin ? '1' : '0';
//REMOVE
//REMOVE                // je nachdem merken oder abspeichern
//REMOVE                if ( bActWin  )
//REMOVE                    aActWinData = aWinData;
//REMOVE                else
//REMOVE                    xStream->WriteByteString( aWinData );
//REMOVE            }
//REMOVE        }
//REMOVE
//REMOVE        // aktives Window hinterher
//REMOVE        xStream->WriteByteString( aActWinData );
//REMOVE        return !xStream->GetError();
//REMOVE    }

//====================================================================

SfxViewFrame* SfxObjectShell::LoadWindows_Impl( SfxTopFrame *pPreferedFrame )
{
    DBG_ASSERT( pPreferedFrame, "Call without preferred Frame is not supported anymore!" );
    if ( pImp->bLoadingWindows || !pPreferedFrame )
        return NULL;

    DBG_ASSERT( GetMedium(), "A Medium should exist here!");
    if( !GetMedium() )
        return 0;

    // get correct mode
    SFX_APP();
    SfxViewFrame *pPrefered = pPreferedFrame ? pPreferedFrame->GetCurrentViewFrame() : 0;
    SvtSaveOptions aOpt;
    BOOL bLoadDocWins = aOpt.IsSaveDocWins() && !pPrefered;

    // try to get viewdata information for XML format
    REFERENCE < XVIEWDATASUPPLIER > xViewDataSupplier( GetModel(), ::com::sun::star::uno::UNO_QUERY );
    REFERENCE < XINDEXACCESS > xViewData;

    if ( xViewDataSupplier.is() )
    {
        xViewData = xViewDataSupplier->getViewData();
        if ( !xViewData.is() )
            return NULL;
    }
    else
        return NULL;

    SfxViewFrame *pActiveFrame = 0;
    String aWinData;
    SfxItemSet *pSet = GetMedium()->GetItemSet();

    pImp->bLoadingWindows = TRUE;
    BOOL bLoaded = FALSE;
    sal_Int32 nView = 0;

    // get saved information for all views
    while ( TRUE )
    {
        USHORT nViewId = 0;
        FASTBOOL bMaximized=FALSE;
        String aPosSize;
        String aUserData;                   // used in the binary format
        SEQUENCE < PROPERTYVALUE > aSeq;    // used in the XML format

        // XML format
        // active view is the first view in the container
        FASTBOOL bActive = ( nView == 0 );

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

        // load only active view, but current item is not the active one ?
        // in XML format the active view is the first one
        if ( !bLoadDocWins && !bActive )
            break;

        // check for minimized/maximized/size
        if ( aPosSize.EqualsAscii( "max" ) )
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
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());

    // clear user data if recommend (see 'Tools - Options - Open/StarOffice - Security')
    if ( SvtSecurityOptions().IsOptionSet(
            SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO ) )
    {
        xDocProps->resetUserData( ::rtl::OUString() );
    }
    else if ( IsModified() )
    {
        String aUserName = SvtUserOptions().GetFullName();
        if ( !IsUseUserData() )
        {
            // remove all data pointing to the current user
            if (xDocProps->getAuthor().equals(aUserName)) {
                xDocProps->setAuthor( ::rtl::OUString() );
            }
            xDocProps->setModifiedBy( ::rtl::OUString() );
            if (xDocProps->getPrintedBy().equals(aUserName)) {
                xDocProps->setPrintedBy( ::rtl::OUString() );
            }
        }
        else
        {
            // update ModificationAuthor, revision and editing time
            ::DateTime now;
            xDocProps->setModificationDate( util::DateTime(
                now.Get100Sec(), now.GetSec(), now.GetMin(),
                now.GetHour(), now.GetDay(), now.GetMonth(),
                now.GetYear() ) );
            xDocProps->setModifiedBy( aUserName );
            if ( !HasName() || pImp->bIsSaving )
                // QUESTION: not in case of "real" SaveAs as this is meant to create a new document
                UpdateTime_Impl( xDocProps );
        }
    }
}

//--------------------------------------------------------------------

// Bearbeitungszeit aktualisieren
void SfxObjectShell::UpdateTime_Impl(
    const uno::Reference<document::XDocumentProperties> & i_xDocProps)
{
    // Get old time from documentinfo
    sal_Int32 secs = i_xDocProps->getEditingDuration();
    Time aOldTime(secs/3600, (secs%3600)/60, secs%60);

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

    pImp->nTime = aNow;
    i_xDocProps->setEditingDuration(
        aOldTime.GetHour()*3600+aOldTime.GetMin()*60+aOldTime.GetSec());
    i_xDocProps->setEditingCycles(i_xDocProps->getEditingCycles() + 1);
}

//--------------------------------------------------------------------

SfxDocumentInfoDialog* SfxObjectShell::CreateDocumentInfoDialog
(
    Window*             pParent,
    const SfxItemSet&   rSet
)
{
    return new SfxDocumentInfoDialog(pParent, rSet);
}

//--------------------------------------------------------------------

SfxStyleSheetBasePool* SfxObjectShell::GetStyleSheetPool()
{
    return 0;
}

void SfxObjectShell::SetOrganizerSearchMask(
    SfxStyleSheetBasePool* pStylePool) const
{
    pStylePool->SetSearchMask(
        SFX_STYLE_FAMILY_ALL,
        SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
}

//--------------------------------------------------------------------

USHORT SfxObjectShell::GetContentCount(
    USHORT nIdx1,
    USHORT /*nIdx2*/)
{
    switch(nIdx1)
    {
        case INDEX_IGNORE:
            return DEF_CONTENT_COUNT;
        case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
            if(!pStylePool)
                return 0;
            SetOrganizerSearchMask(pStylePool);
            return pStylePool->Count();
        }
        case CONTENT_MACRO:
            break;
/*
        case CONTENT_CONFIG:
            return ( GetConfigManager() ) ?
                        GetConfigManager()->GetItemCount() : 0;
            break;
 */
    }
    return 0;
}


//--------------------------------------------------------------------
//TODO/CLEANUP: remove this method (it's virtual)
void  SfxObjectShell::TriggerHelpPI(USHORT nIdx1, USHORT nIdx2, USHORT)
{
    if(nIdx1==CONTENT_STYLE && nIdx2 != INDEX_IGNORE) //StyleSheets
    {
        SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
        SetOrganizerSearchMask(pStylePool);
#ifdef WIR_KOENNEN_WIEDER_HILFE_FUER_STYLESHEETS
        SfxStyleSheetBase *pStyle = (*pStylePool)[nIdx2];
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
                                  USHORT /*nIdx2*/ )
{
    bCanDel=TRUE;

    switch(nIdx1)
    {
        case INDEX_IGNORE:
        {
            USHORT nTextResId = 0;
            USHORT nClosedBitmapResId = 0; // evtl. sp"ater mal unterschiedliche
            USHORT nOpenedBitmapResId = 0; // "     "       "   "
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
            SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
            SetOrganizerSearchMask(pStylePool);
            SfxStyleSheetBase *pStyle = (*pStylePool)[i];
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
            if ( GetConfigManager() )
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
        case SFX_STYLE_FAMILY_PSEUDO:
        case SFX_STYLE_FAMILY_ALL:
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
                              USHORT /*nSourceIdx3*/,
                              USHORT &nIdx1,
                              USHORT &nIdx2,
                              USHORT &/*nIdx3*/,
                              USHORT &/*nDeleted*/)
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
        SfxStyleSheetBase* pHisSheet = NULL;

        if ( pHisPool && pHisPool->Count() > nSourceIdx2 )
            pHisSheet = (*pHisPool)[nSourceIdx2];

        // Einfuegen ist nur dann noetig, wenn ein StyleSheet
        // zwischen unterschiedlichen(!) Pools bewegt wird

        if ( pHisSheet && pMyPool != pHisPool )
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
        if ( !GetConfigManager() )
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
    USHORT /*nIdx3*/
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
            pMyPool->Remove(pMySheet);
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
    USHORT          /*nIdx2*/,
    USHORT          /*nIdx3*/,
    const String*   pObjectName
)

/*  [Beschreibung]
*/

{
    switch(nIdx1)
    {
      case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
            SetOrganizerSearchMask(pStylePool);
            SfxStyleSheetIterator* pIter = pStylePool->CreateIterator(
                pStylePool->GetSearchFamily(), pStylePool->GetSearchMask() );
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
            Reference< task::XStatusIndicator > xStatusIndicator;
            xStatusIndicator = SFX_APP()->GetStatusIndicator();
            if ( xStatusIndicator.is() )
                xStatusIndicator->start( String(SfxResId(STR_PRINT_STYLES)), nStyles );

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
                if ( xStatusIndicator.is() )
                    xStatusIndicator->setValue( nStyles++ );
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
            if ( xStatusIndicator.is() )
                xStatusIndicator->end();
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
    delete [] pFound;
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
    if ( !pFile )
        return;

    if ( !::utl::LocalFileHelper::IsLocalFile( pFile->GetName() ) )
        // update only for documents loaded from the local file system
        return;

    // only for own storage formats
    uno::Reference< embed::XStorage > xDocStor = pFile->GetStorage();
    if ( !pFile->GetFilter() || !pFile->GetFilter()->IsOwnFormat() )
        return;

    SFX_ITEMSET_ARG( pFile->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
    sal_Int16 bCanUpdateFromTemplate = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    // created from template?
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    ::rtl::OUString aTemplName( xDocProps->getTemplateName() );
    ::rtl::OUString aTemplURL( xDocProps->getTemplateURL() );
    String aFoundName;

    if ( aTemplName.getLength() || aTemplURL.getLength() && !IsReadOnly() )
    {
        // try to locate template, first using filename
        // this must be done because writer global document uses this "great" idea to manage the templates of all parts
        // in the master document
        // but it is NOT an error if the template filename points not to a valid file
        SfxDocumentTemplates aTempl;
        aTempl.Construct();
        if ( aTemplURL.getLength() )
        {
            String aURL;
            if( ::utl::LocalFileHelper::ConvertSystemPathToURL( aTemplURL, GetMedium()->GetName(), aURL ) )
                aFoundName = aURL;
        }

        if( !aFoundName.Len() && aTemplName.getLength() )
            // if the template filename did not lead to success, try to get a file name for the logical template name
            aTempl.GetFull( String(), aTemplName, aFoundName );
    }

    if ( aFoundName.Len() )
    {
        // check existence of template storage
        aTemplURL = aFoundName;
        BOOL bLoad = FALSE;

        // should the document checked against changes in the template ?
        if ( IsQueryLoadTemplate() )
        {
            // load document info of template
            BOOL bOK = FALSE;
            util::DateTime aTemplDate;
            Reference < document::XStandaloneDocumentInfo > xDocInfo (
                    ::comphelper::getProcessServiceFactory()->createInstance(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.StandaloneDocumentInfo") ) ), UNO_QUERY );
            Reference < beans::XFastPropertySet > xSet( xDocInfo, UNO_QUERY );
            if ( xDocInfo.is() && xSet.is() )
            {
                try
                {
                    xDocInfo->loadFromURL( aTemplURL );
                    Any aAny = xSet->getFastPropertyValue( WID_DATE_MODIFIED );
                    ::com::sun::star::util::DateTime aTmp;
                    if ( aAny >>= aTemplDate )
                    {
                        // get modify date from document info
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
                const util::DateTime aInfoDate( xDocProps->getTemplateDate() );
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
                        SetQueryLoadTemplate(FALSE);
                        SetModified( TRUE );
                    }
                }
            }

            if ( bLoad )
            {
                // styles should be updated, create document in organizer mode to read in the styles
                //TODO: testen!
                SfxObjectShellLock xTemplDoc = CreateObjectByFactoryName( GetFactory().GetFactoryName(), SFX_CREATE_MODE_ORGANIZER );
                xTemplDoc->DoInitNew(0);

                // TODO/MBA: do we need a BaseURL? Then LoadFrom must be extended!
                //xTemplDoc->SetBaseURL( aFoundName );

                // TODO/LATER: make sure that we don't use binary templates!
                SfxMedium aMedium( aFoundName, STREAM_STD_READ );
                if ( xTemplDoc->LoadFrom( aMedium ) )
                {
                    // transfer styles from xTemplDoc to this document
                    // TODO/MBA: make sure that no BaseURL is needed in *this* document
                    LoadStyles(*xTemplDoc);

                    // remember date/time of check
                    xDocProps->setTemplateDate(aTemplDate);
                    // TODO/LATER: new functionality to store document info is required ( didn't work for SO7 XML format )
//REPLACE                   pInfo->Save(xDocStor);
                }
            }
/*
        SfxConfigManager *pCfgMgr = SFX_CFGMANAGER();
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

/*
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
} */

//REMOVE    SvStorageRef SfxObjectShell::GetConfigurationStorage( SotStorage* pStor )
//REMOVE    {
//REMOVE        // configuration storage shall be opened in own storage or a new storage, if the
//REMOVE        // document is getting stored into this storage
//REMOVE        if ( !pStor )
//REMOVE            pStor = GetStorage();
//REMOVE
//REMOVE        if ( pStor->IsOLEStorage() )
//REMOVE                    return (SvStorageRef) SotStorageRef();
//REMOVE
//REMOVE        // storage is always opened in transacted mode, so changes must be commited
//REMOVE        SotStorageRef xStorage = pStor->OpenSotStorage( DEFINE_CONST_UNICODE("Configurations"),
//REMOVE                    IsReadOnly() ? STREAM_STD_READ : STREAM_STD_READWRITE );
//REMOVE        if ( xStorage.Is() && xStorage->GetError() )
//REMOVE            xStorage.Clear();
//REMOVE            return (SvStorageRef) xStorage;
//REMOVE    }

//REMOVE    SotStorageStreamRef SfxObjectShell::GetConfigurationStream( const String& rName, BOOL bCreate )
//REMOVE    {
//REMOVE        SotStorageStreamRef xStream;
//REMOVE        SvStorageRef xStorage = GetConfigurationStorage();
//REMOVE        if ( xStorage.Is() )
//REMOVE        {
//REMOVE            xStream = xStorage->OpenSotStream( rName,
//REMOVE                bCreate ? STREAM_STD_READWRITE|STREAM_TRUNC : STREAM_STD_READ );
//REMOVE            if ( xStream.Is() && xStream->GetError() )
//REMOVE                xStream.Clear();
//REMOVE        }
//REMOVE
//REMOVE        return xStream;
//REMOVE    }

SfxObjectShellRef MakeObjectShellForOrganizer_Impl( const String& aTargetURL, BOOL bForWriting )
{
    // check for own format
    SfxObjectShellRef xDoc;
    StreamMode nMode = bForWriting ? SFX_STREAM_READWRITE : SFX_STREAM_READONLY;
    SfxMedium *pMed = new SfxMedium( aTargetURL, nMode, FALSE, 0 );
    const SfxFilter* pFilter = NULL;
    pMed->UseInteractionHandler(TRUE);
    if( SFX_APP()->GetFilterMatcher().GuessFilter( *pMed, &pFilter ) == ERRCODE_NONE && pFilter && pFilter->IsOwnFormat() )
    {
        // create document
        xDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName(), SFX_CREATE_MODE_ORGANIZER );
        if ( xDoc.Is() )
        {
            // partially load, so don't use DoLoad!
            xDoc->DoInitNew(0);
            // TODO/LATER: make sure that we don't use binary templates!
            if( xDoc->LoadFrom( *pMed ) )
            {
                // connect to storage, abandon temp. storage
                xDoc->DoSaveCompleted( pMed );
            }
            else
                xDoc.Clear();
        }
    }
    else
        delete pMed;

    return xDoc;
}

sal_Bool SfxObjectShell::IsHelpDocument() const
{
    const SfxFilter* pFilter = GetMedium()->GetFilter();
    return ( pFilter && pFilter->GetFilterName().CompareToAscii("writer_web_HTML_help") == COMPARE_EQUAL );
}

void SfxObjectShell::ResetFromTemplate( const String& rTemplateName, const String& rFileName )
{
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    xDocProps->setTemplateURL( ::rtl::OUString() );
    xDocProps->setTemplateName( ::rtl::OUString() );
    xDocProps->setTemplateDate( util::DateTime() );
    xDocProps->resetUserData( ::rtl::OUString() );

    // TODO/REFACTOR:
    // Title?

    if( ::utl::LocalFileHelper::IsLocalFile( rFileName ) )
    {
        String aFoundName;
        if( SFX_APP()->Get_Impl()->GetDocumentTemplates()->GetFull( String(), rTemplateName, aFoundName ) )
        {
            INetURLObject aObj( rFileName );
            xDocProps->setTemplateURL( aObj.GetMainURL(INetURLObject::DECODE_TO_IURI) );
            xDocProps->setTemplateName( rTemplateName );
            SetQueryLoadTemplate( sal_True );
        }
    }
}

sal_Bool SfxObjectShell::IsQueryLoadTemplate() const
{
    return pImp->bQueryLoadTemplate;
}

sal_Bool SfxObjectShell::IsUseUserData() const
{
    return pImp->bUseUserData;
}

void SfxObjectShell::SetQueryLoadTemplate( sal_Bool bNew )
{
    if ( pImp->bQueryLoadTemplate != bNew )
        SetModified( TRUE );
    pImp->bQueryLoadTemplate = bNew;
}

void SfxObjectShell::SetUseUserData( sal_Bool bNew )
{
    if ( pImp->bUseUserData != bNew )
        SetModified( TRUE );
    pImp->bUseUserData = bNew;
}

sal_Bool SfxObjectShell::IsLoadReadonly() const
{
    return pImp->bLoadReadonly;
}

sal_Bool SfxObjectShell::IsSaveVersionOnClose() const
{
    return pImp->bSaveVersionOnClose;
}

void SfxObjectShell::SetLoadReadonly( sal_Bool bNew )
{
    if ( pImp->bLoadReadonly != bNew )
        SetModified( TRUE );
    pImp->bLoadReadonly = bNew;
}

void SfxObjectShell::SetSaveVersionOnClose( sal_Bool bNew )
{
    if ( pImp->bSaveVersionOnClose != bNew )
        SetModified( TRUE );
    pImp->bSaveVersionOnClose = bNew;
}

