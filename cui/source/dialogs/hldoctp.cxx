/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "cuihyperdlg.hxx"
#include <unotools/localfilehelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include "hldoctp.hxx"
#include "hyperdlg.hrc"
#include "hlmarkwn_def.hxx"

sal_Char const sHash[]          = "#";
sal_Char const sFileScheme[]    = INET_FILE_SCHEME;
sal_Char const sHTTPScheme[]    = INET_HTTP_SCHEME;

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperlinkDocTp::SvxHyperlinkDocTp ( Window *pParent, const SfxItemSet& rItemSet)
    : SvxHyperlinkTabPageBase ( pParent, CUI_RES( RID_SVXPAGE_HYPERLINK_DOCUMENT ), rItemSet ),
    maGrpDocument   ( this, CUI_RES (GRP_DOCUMENT) ),
    maFtPath        ( this, CUI_RES (FT_PATH_DOC) ),
    maCbbPath       ( this, INET_PROT_FILE ),
    maBtFileopen    ( this, CUI_RES (BTN_FILEOPEN) ),
    maGrpTarget     ( this, CUI_RES (GRP_TARGET) ),
    maFtTarget      ( this, CUI_RES (FT_TARGET_DOC) ),
    maEdTarget      ( this, CUI_RES (ED_TARGET_DOC) ),
    maFtURL         ( this, CUI_RES (FT_URL) ),
    maFtFullURL     ( this, CUI_RES (FT_FULL_URL) ),
    maBtBrowse      ( this, CUI_RES (BTN_BROWSE) ),
    mbMarkWndOpen   ( sal_False )
{
    // Disable display of bitmap names.
    maBtBrowse.EnableTextDisplay (sal_False);
    maBtFileopen.EnableTextDisplay (sal_False);

    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbPath.SetPosSizePixel ( LogicToPixel( Point( COL_2, 15 ), MAP_APPFONT ),
                                LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );
    maCbbPath.Show();
    String aFileScheme( INET_FILE_SCHEME, RTL_TEXTENCODING_ASCII_US );
    maCbbPath.SetBaseURL(aFileScheme);
    maCbbPath.SetHelpId( HID_HYPERDLG_DOC_PATH );

    SetExchangeSupport ();

    // overload handlers
    maBtFileopen.SetClickHdl ( LINK ( this, SvxHyperlinkDocTp, ClickFileopenHdl_Impl ) );
    maBtBrowse.SetClickHdl   ( LINK ( this, SvxHyperlinkDocTp, ClickTargetHdl_Impl ) );
    maCbbPath.SetModifyHdl   ( LINK ( this, SvxHyperlinkDocTp, ModifiedPathHdl_Impl ) );
    maEdTarget.SetModifyHdl  ( LINK ( this, SvxHyperlinkDocTp, ModifiedTargetHdl_Impl ) );

    maCbbPath.SetLoseFocusHdl( LINK ( this, SvxHyperlinkDocTp, LostFocusPathHdl_Impl ) );

    maBtBrowse.SetAccessibleRelationMemberOf( &maGrpTarget );
    maBtBrowse.SetAccessibleRelationLabeledBy( &maFtTarget );
    maBtFileopen.SetAccessibleRelationMemberOf( &maGrpDocument );
    maBtFileopen.SetAccessibleRelationLabeledBy( &maFtPath );
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

    String aStrMark;
    xub_StrLen nPos = aStrURL.SearchAscii( sHash );
    // path
    maCbbPath.SetText ( aStrURL.Copy( 0, ( nPos == STRING_NOTFOUND ? aStrURL.Len() : nPos ) ) );

    // set target in document at editfield
    if ( nPos != STRING_NOTFOUND && nPos<aStrURL.Len()-1 )
        aStrMark = aStrURL.Copy( nPos+1, aStrURL.Len() );
     maEdTarget.SetText ( aStrMark );

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
    OUString aStrURL;
    String aStrPath ( maCbbPath.GetText() );
    const String aBaseURL ( maCbbPath.GetBaseURL() );
    String aStrMark( maEdTarget.GetText() );

    if ( aStrPath != aEmptyStr )
    {
        INetURLObject aURL( aStrPath );
        if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )    // maybe the path is already a valid
            aStrURL = aStrPath;                             // hyperlink, then we can use this path directly
        else
            utl::LocalFileHelper::ConvertSystemPathToURL( aStrPath, aBaseURL, aStrURL );

        //#105788# always create a URL even if it is not valid
        if( aStrURL == aEmptyStr )
            aStrURL = aStrPath;
    }

    if( aStrMark != aEmptyStr )
    {
        aStrURL += OUString( sHash );
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

    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
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

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ClickFileopenHdl_Impl)
{
    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0,
        GetParent() );
    String aOldURL( GetCurrentURL() );
    if( aOldURL.EqualsIgnoreCaseAscii( sFileScheme, 0, sizeof( sFileScheme ) - 1 ) )
    {
        aDlg.SetDisplayDirectory( aOldURL );
    }

    DisableClose( sal_True );
    ErrCode nError = aDlg.Execute();
    DisableClose( sal_False );

    if ( ERRCODE_NONE == nError )
    {
        String aURL( aDlg.GetPath() );
        OUString aPath;

        utl::LocalFileHelper::ConvertURLToSystemPath( aURL, aPath );

        maCbbPath.SetBaseURL( aURL );
        maCbbPath.SetText( aPath );

        if ( aOldURL != GetCurrentURL() )
            ModifiedPathHdl_Impl (NULL);
    }

    return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : target
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ClickTargetHdl_Impl)
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

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ModifiedPathHdl_Impl)
{
    maStrURL = GetCurrentURL();

    maTimer.SetTimeout( 2500 );
    maTimer.Start();

    maFtFullURL.SetText( maStrURL );

    return( 0L );
}

/*************************************************************************
|*
|* If path-field was modify, to browse the new doc after timeout
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, TimeoutHdl_Impl)
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

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ModifiedTargetHdl_Impl)
{
    maStrURL = GetCurrentURL();

    if ( IsMarkWndVisible() )
        mpMarkWnd->SelectEntry ( maEdTarget.GetText() );

    maFtFullURL.SetText( maStrURL );

    return( 0L );
}

/*************************************************************************
|*
|* editfield "Target" lost focus
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, LostFocusPathHdl_Impl)
{
    maStrURL = GetCurrentURL();

    maFtFullURL.SetText( maStrURL );

    return (0L);
}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/

void SvxHyperlinkDocTp::SetMarkStr ( const String& aStrMark )
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
    INetURLObject aURL( aStrPath, INET_PROT_FILE );

    if( aURL.HasError() )
        return Type_Invalid;
    else
        return Type_ExistsFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
