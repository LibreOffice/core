/*************************************************************************
 *
 *  $RCSfile: hldoctp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pw $ $Date: 2000-11-22 13:38:19 $
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

#include "hyperdlg.hxx"

#include "hldoctp.hxx"
#include "hyperdlg.hrc"

sal_Char __READONLY_DATA sHash[]         = "#";
sal_Char __READONLY_DATA sFileScheme[]    = INET_FILE_SCHEME;
sal_Char __READONLY_DATA sNewsSRVScheme[] = INET_NEWS_SRV_SCHEME;
sal_Char __READONLY_DATA sHTTPScheme[]    = INET_HTTP_SCHEME;

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperlinkDocTp::SvxHyperlinkDocTp ( Window *pParent, const SfxItemSet& rItemSet)
:   maGrpDocument   ( this, ResId (GRP_DOCUMENT) ),
    maFtPath        ( this, ResId (FT_PATH_DOC) ),
    maCbbPath       ( this, INET_PROT_FILE ),
    maBtFileopen    ( this, ResId (BTN_FILEOPEN) ),
    maGrpTarget     ( this, ResId (GRP_TARGET) ),
    maFtTarget      ( this, ResId (FT_TARGET_DOC) ),
    maEdTarget      ( this, ResId (ED_TARGET_DOC) ),
    maFtURL         ( this, ResId (FT_URL) ),
    maFtFullURL     ( this, ResId (FT_FULL_URL) ),
    maBtBrowse      ( this, ResId (BTN_BROWSE) ),
    mbMarkWndOpen   ( FALSE ),
    SvxHyperlinkTabPageBase ( pParent, SVX_RES( RID_SVXPAGE_HYPERLINK_DOCUMENT ), rItemSet )
{
    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbPath.SetPosSizePixel ( LogicToPixel( Point( 54, 15 ), MAP_APPFONT ),
                                LogicToPixel( Size ( 176, 60), MAP_APPFONT ) );
    maCbbPath.Show();
    maCbbPath.SetHelpId( HID_HYPERDLG_DOC_PATH );

    SetExchangeSupport ();

    // overload handlers
    maBtFileopen.SetClickHdl ( LINK ( this, SvxHyperlinkDocTp, ClickFileopenHdl_Impl ) );
    maBtBrowse.SetClickHdl   ( LINK ( this, SvxHyperlinkDocTp, ClickTargetHdl_Impl ) );
    maCbbPath.SetModifyHdl   ( LINK ( this, SvxHyperlinkDocTp, ModifiedPathHdl_Impl ) );
    maEdTarget.SetModifyHdl  ( LINK ( this, SvxHyperlinkDocTp, ModifiedTargetHdl_Impl ) );

    maCbbPath.SetLoseFocusHdl( LINK ( this, SvxHyperlinkDocTp, LostFocusPathHdl_Impl ) );

    maTimer.SetTimeoutHdl ( LINK ( this, SvxHyperlinkDocTp, TimeoutHdl_Impl ) );
}

SvxHyperlinkDocTp::~SvxHyperlinkDocTp ()
{
}

/*************************************************************************
|*
|* Fill all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/

void SvxHyperlinkDocTp::FillDlgFields ( String& aStrURL )
{
    INetURLObject aURL ( aStrURL );
    String aStrScheme;

    // set protocoll-radiobuttons
    INetProtocol aProtocol = aURL.GetProtocol ();
    switch ( aProtocol )
    {
        case INET_PROT_FILE :
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sFileScheme ) );
            break;
        case INET_PROT_POP3 :
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( INET_POP3_SCHEME ) );
            break;
        case INET_PROT_IMAP :
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( INET_IMAP_SCHEME ) );
            break;
        case INET_PROT_OUT :
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( INET_OUT_SCHEME ) );
            break;
        default :
            if ( aStrURL.SearchAscii( sNewsSRVScheme ) == 0 )
                aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sNewsSRVScheme ) );
            else if( aStrURL.SearchAscii( sHash ) == 0 )
                aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sFileScheme ) );
    }

    if ( aStrScheme != aEmptyStr )
    {
        String aStrURL ( aURL.GetMainURL() );
        String aStrMark;

        xub_StrLen nPos = aStrURL.SearchAscii( sHash );
        // path
        maCbbPath.SetText ( aStrURL.Copy( 0, ( nPos == STRING_NOTFOUND ? aStrURL.Len() : nPos ) ) );

        // set target in document at editfield
        if ( nPos != STRING_NOTFOUND && nPos<aStrURL.Len()-1 )
            aStrMark = aStrURL.Copy( nPos+1, aStrURL.Len() );
         maEdTarget.SetText ( aStrMark );
    }
    else
    {
        maCbbPath.SetText ( aEmptyStr );
        maEdTarget.SetText ( aEmptyStr );
    }

    ModifiedPathHdl_Impl ( NULL );
}

/*************************************************************************
|*
|* retrieve current url-string
|*
|************************************************************************/

String SvxHyperlinkDocTp::GetCurrentURL ()
{
    // get data from dialog-controls
    String aStrURL ( maCbbPath.GetText() );
    String aStrMark( maEdTarget.GetText() );

    if ( aStrURL != aEmptyStr )
    {
        if ( aStrMark != aEmptyStr )
        {
            aStrURL.AppendAscii( "#" );
            aStrURL += aStrMark;
        }

        // create a real URL-String
        INetURLObject aURL;
        if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
            aURL.SetSmartProtocol( INET_PROT_FILE );

        aURL.SetSmartURL( aStrURL );

        // if there is a empty string, the url will be the html-scheme
        // but its better to show only the file-scheme
        aStrURL = aURL.GetMainURL();

        if ( aStrURL.SearchAscii( sHTTPScheme ) == 0 )
        {
            aStrURL.Erase( 0, UniString::CreateFromAscii(
                           RTL_CONSTASCII_STRINGPARAM ( sHTTPScheme ) ).Len() );
            String aStrTmp( aStrURL );
            aStrURL.AssignAscii( sHTTPScheme );
            aStrURL += aStrTmp;
        }
    }
    else
        if( aStrMark != aEmptyStr )
        {
            aStrURL.AssignAscii( sHash );
            aStrURL += aStrMark;
        }

    return aStrURL;
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkDocTp::GetCurentItemData ( String& aStrURL, String& aStrName,
                                            String& aStrIntName, String& aStrFrame,
                                            SvxLinkInsertMode& eMode )
{
    // get data from standard-fields
    aStrURL = GetCurrentURL();

    if( aStrURL.EqualsIgnoreCaseAscii( sFileScheme ) )
         aStrURL=aEmptyStr;

    aStrIntName = mpEdText->GetText();
    aStrName    = mpEdIndication->GetText();
    aStrFrame   = mpCbbFrame->GetText();
    eMode       = (SvxLinkInsertMode) (mpLbForm->GetSelectEntryPos()+1);
    if( IsHTMLDoc() )
        eMode = (SvxLinkInsertMode) ( UINT16(eMode) | HLINK_HTMLMODE );

    if ( aStrName == aEmptyStr )
        aStrName = aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

IconChoicePage* SvxHyperlinkDocTp::Create( Window* pWindow, const SfxItemSet& rItemSet )
{
    return( new SvxHyperlinkDocTp( pWindow, rItemSet ) );
}

/*************************************************************************
|*
|* Activate / Deactivate Tabpage
|*
|************************************************************************/

void SvxHyperlinkDocTp::ActivatePage( const SfxItemSet& rItemSet )
{
    ///////////////////////////////////////
    // Set dialog-fields from input-itemset
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // standard-fields
        FillStandardDlgFields (pHyperlinkItem);

        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );
    }

    // show mark-window if it was open before
    if ( mbMarkWndOpen )
        ShowMarkWnd ();
}

int SvxHyperlinkDocTp::DeactivatePage( SfxItemSet* pSet )
{
    // hide mark-wnd
    mbMarkWndOpen = IsMarkWndVisible ();
    HideMarkWnd ();

    // retrieve data of dialog
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    // put item
    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_GETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    pSet->Put (aItem);

    return( LEAVE_PAGE );
}

/*************************************************************************
|*
|* Fill output-ItemSet
|*
|************************************************************************/

BOOL SvxHyperlinkDocTp::FillItemSet( SfxItemSet& rOut)
{
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    // put data into itemset
    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_SETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    rOut.Put (aItem);

    return TRUE;
}

/*************************************************************************
|*
|* reset dialog-fields
|*
|************************************************************************/

void SvxHyperlinkDocTp::Reset( const SfxItemSet& rItemSet)
{
    ///////////////////////////////////////
    // Set dialog-fields from create-itemset
    maStrInitURL = aEmptyStr;

    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );

        // set dialog-fields
        FillStandardDlgFields (pHyperlinkItem);

        // set all other fields
        FillDlgFields ( (String&)pHyperlinkItem->GetURL() );

        // Store initial URL
        maStrInitURL = pHyperlinkItem->GetURL();
    }
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkDocTp::SetInitFocus()
{
    maCbbPath.GrabFocus();
}

/*************************************************************************
|*
|* Click on imagebutton : fileopen
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkDocTp, ClickFileopenHdl_Impl, void *, EMPTYARG )
{
    // Open Fileopen-Dialog
    SfxFileDialog* pFileDlg = SFX_APP()->CreateDocFileDialog( ( WB_3DLOOK | WB_STDMODAL | WB_OPEN ),
                                                                *(SfxObjectFactory*)NULL );
    INetURLObject aOldURL( GetCurrentURL(), INET_PROT_FILE );
    if( !aOldURL.GetMainURL().EqualsIgnoreCaseAscii( sFileScheme ) )
    {
        pFileDlg->SetPathURL( aOldURL.GetMainURL() );
    }

    if( pFileDlg )
    {
        if ( pFileDlg->Execute() == RET_OK )
        {
            String aStrFilename( pFileDlg->GetPath() );
            aStrFilename = aStrFilename.EraseLeadingChars();

            sal_Char const sFile[] = "file";
            if ( aStrFilename.SearchAscii( sFile ) == 0)
            {
                INetURLObject aURL;
                aURL.SetSmartProtocol( INET_PROT_FILE );
                aURL.SetSmartURL( aStrFilename );

                maCbbPath.SetText( aURL.GetFull() );
            }

            if ( aOldURL.GetMainURL() != GetCurrentURL() )
                ModifiedPathHdl_Impl (NULL);
        }

        delete pFileDlg;
    }

    return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : target
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkDocTp, ClickTargetHdl_Impl, void *, EMPTYARG )
{
    if ( GetPathType ( maStrURL ) == Type_ExistsFile  ||
         maStrURL == aEmptyStr                        ||
         maStrURL.EqualsIgnoreCaseAscii( sFileScheme ) ||
         maStrURL.SearchAscii( sHash ) == 0 )
    {
        mpMarkWnd->SetError( LERR_NOERROR );

        EnterWait();

        if ( maStrURL.EqualsIgnoreCaseAscii( sFileScheme ) )
            mpMarkWnd->RefreshTree ( aEmptyStr );
        else
            mpMarkWnd->RefreshTree ( maStrURL );

        LeaveWait();
    }
    else
        mpMarkWnd->SetError( LERR_DOCNOTOPEN );

    ShowMarkWnd ();

    return( 0L );
}

/*************************************************************************
|*
|* Contens of combobox "Path" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkDocTp, ModifiedPathHdl_Impl, void *, EMPTYARG )
{
    maStrURL = GetCurrentURL();

    maTimer.SetTimeout( 2500 );
    maTimer.Start();

    maFtFullURL.SetText( maStrURL );

    if ( mbNewName && !maStrURL.EqualsIgnoreCaseAscii( sFileScheme ) )
        mpEdIndication->SetText( maFtFullURL.GetText() );

    return( 0L );
}

/*************************************************************************
|*
|* If path-field was modify, to browse the new doc after timeout
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkDocTp, TimeoutHdl_Impl, Timer *, EMPTYARG )
{
    if ( IsMarkWndVisible() && ( GetPathType( maStrURL )==Type_ExistsFile ||
                                  maStrURL == aEmptyStr                   ||
                                  maStrURL.EqualsIgnoreCaseAscii( sFileScheme ) ) )
    {
        EnterWait();

        if ( maStrURL.EqualsIgnoreCaseAscii( sFileScheme ) )
            mpMarkWnd->RefreshTree ( aEmptyStr );
        else
            mpMarkWnd->RefreshTree ( maStrURL );

        LeaveWait();
    }

    return( 0L );
}

/*************************************************************************
|*
|* Contens of editfield "Target" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkDocTp, ModifiedTargetHdl_Impl, void *, EMPTYARG )
{
    maStrURL = GetCurrentURL();

    if ( IsMarkWndVisible() )
        mpMarkWnd->SelectEntry ( maEdTarget.GetText() );

    maFtFullURL.SetText( maStrURL );

    if ( mbNewName && !maStrURL.EqualsIgnoreCaseAscii( sFileScheme ) )
        mpEdIndication->SetText ( maFtFullURL.GetText() );

    return( 0L );
}

/*************************************************************************
|*
|* editfield "Target" lost focus
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkDocTp, LostFocusPathHdl_Impl, void *, EMPTYARG )
{
    maStrURL = GetCurrentURL();

    maFtFullURL.SetText(maStrURL);

    if ( mbNewName && !maStrURL.EqualsIgnoreCaseAscii( sFileScheme ) )
        mpEdIndication->SetText ( maFtFullURL.GetText() );

    return (0L);
}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/

void SvxHyperlinkDocTp::SetMarkStr ( String& aStrMark )
{
    maEdTarget.SetText ( aStrMark );

    ModifiedTargetHdl_Impl ( NULL );
}

/*************************************************************************
|*
|* retrieve kind of pathstr
|*
|************************************************************************/

SvxHyperlinkDocTp::EPathType SvxHyperlinkDocTp::GetPathType ( String& aStrPath )
{
    BOOL bExists = FALSE;
    INetURLObject aURL( aStrPath, INET_PROT_FILE );

    if( aURL.HasError() )
        return Type_Invalid;
    else
        return Type_ExistsFile;

    return Type_Unknown;
}


