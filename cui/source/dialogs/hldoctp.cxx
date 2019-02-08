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

#include <cuihyperdlg.hxx>
#include <osl/file.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <hldoctp.hxx>
#include <hlmarkwn_def.hxx>
#include <bitmaps.hlst>

sal_Char const sHash[]          = "#";
sal_Char const sFileScheme[]    = INET_FILE_SCHEME;

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkDocTp::SvxHyperlinkDocTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet* pItemSet)
    : SvxHyperlinkTabPageBase ( pParent, pDlg, "HyperlinkDocPage", "cui/ui/hyperlinkdocpage.ui", pItemSet ),
    mbMarkWndOpen   ( false )
{
    get(m_pCbbPath, "path");
    m_pCbbPath->SetSmartProtocol(INetProtocol::File);
    get(m_pBtFileopen, "fileopen");
    m_pBtFileopen->SetModeImage(Image(StockImage::Yes, RID_SVXBMP_FILEOPEN));
    get(m_pEdTarget, "target");
    get(m_pFtFullURL, "url");
    get(m_pBtBrowse, "browse");
    m_pBtBrowse->SetModeImage(Image(StockImage::Yes, RID_SVXBMP_TARGET));

    // Disable display of bitmap names.
    m_pBtBrowse->EnableTextDisplay (false);
    m_pBtFileopen->EnableTextDisplay (false);

    InitStdControls();

    m_pCbbPath->Show();
    m_pCbbPath->SetBaseURL(INET_FILE_SCHEME);

    SetExchangeSupport ();

    // set handlers
    m_pBtFileopen->SetClickHdl ( LINK ( this, SvxHyperlinkDocTp, ClickFileopenHdl_Impl ) );
    m_pBtBrowse->SetClickHdl   ( LINK ( this, SvxHyperlinkDocTp, ClickTargetHdl_Impl ) );
    m_pCbbPath->SetModifyHdl   ( LINK ( this, SvxHyperlinkDocTp, ModifiedPathHdl_Impl ) );
    m_pEdTarget->SetModifyHdl  ( LINK ( this, SvxHyperlinkDocTp, ModifiedTargetHdl_Impl ) );

    m_pCbbPath->SetLoseFocusHdl( LINK ( this, SvxHyperlinkDocTp, LostFocusPathHdl_Impl ) );

    maTimer.SetInvokeHandler ( LINK ( this, SvxHyperlinkDocTp, TimeoutHdl_Impl ) );
}

SvxHyperlinkDocTp::~SvxHyperlinkDocTp()
{
    disposeOnce();
}

void SvxHyperlinkDocTp::dispose()
{
    m_pCbbPath.clear();
    m_pBtFileopen.clear();
    m_pEdTarget.clear();
    m_pFtFullURL.clear();
    m_pBtBrowse.clear();
    SvxHyperlinkTabPageBase::dispose();
}

/*************************************************************************
|*
|* Fill all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/

void SvxHyperlinkDocTp::FillDlgFields(const OUString& rStrURL)
{
    sal_Int32 nPos = rStrURL.indexOf(sHash);
    // path
    m_pCbbPath->SetText ( rStrURL.copy( 0, ( nPos == -1 ? rStrURL.getLength() : nPos ) ) );

    // set target in document at editfield
    OUString aStrMark;
    if ( nPos != -1 && nPos < rStrURL.getLength()-1 )
        aStrMark = rStrURL.copy( nPos+1 );
    m_pEdTarget->SetText ( aStrMark );

    ModifiedPathHdl_Impl ( *m_pCbbPath );
}

/*************************************************************************
|*
|* retrieve current url-string
|*
|************************************************************************/

OUString SvxHyperlinkDocTp::GetCurrentURL ()
{
    // get data from dialog-controls
    OUString aStrURL;
    OUString aStrPath ( m_pCbbPath->GetText() );
    OUString aStrMark( m_pEdTarget->GetText() );

    if ( !aStrPath.isEmpty() )
    {
        INetURLObject aURL( aStrPath );
        if ( aURL.GetProtocol() != INetProtocol::NotValid )    // maybe the path is already a valid
            aStrURL = aStrPath;                             // hyperlink, then we can use this path directly
        else
            osl::FileBase::getFileURLFromSystemPath( aStrPath, aStrURL );

        //#105788# always create a URL even if it is not valid
        if( aStrURL.isEmpty() )
            aStrURL = aStrPath;
    }

    if( !aStrMark.isEmpty() )
    {
        aStrURL += sHash;
        aStrURL += aStrMark;
    }

    return aStrURL;
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkDocTp::GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                            OUString& aStrIntName, OUString& aStrFrame,
                                            SvxLinkInsertMode& eMode )
{
    // get data from standard-fields
    rStrURL = GetCurrentURL();

    if( rStrURL.equalsIgnoreAsciiCase( sFileScheme ) )
         rStrURL.clear();

    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

VclPtr<IconChoicePage> SvxHyperlinkDocTp::Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet* pItemSet )
{
    return VclPtr<SvxHyperlinkDocTp>::Create( pWindow, pDlg, pItemSet );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkDocTp::SetInitFocus()
{
    m_pCbbPath->GrabFocus();
}

/*************************************************************************
|*
|* Click on imagebutton : fileopen
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ClickFileopenHdl_Impl, Button*, void)
{
    // Open Fileopen-Dialog
    sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE,
        GetFrameWeld() );
    OUString aOldURL( GetCurrentURL() );
    if( aOldURL.startsWithIgnoreAsciiCase( sFileScheme ) )
    {
        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aOldURL, aPath);
        aDlg.SetDisplayFolder( aPath );
    }

    DisableClose( true );
    ErrCode nError = aDlg.Execute();
    DisableClose( false );

    if ( ERRCODE_NONE == nError )
    {
        OUString aURL( aDlg.GetPath() );
        OUString aPath;

        osl::FileBase::getSystemPathFromFileURL(aURL, aPath);

        m_pCbbPath->SetBaseURL( aURL );
        m_pCbbPath->SetText( aPath );

        if ( aOldURL != GetCurrentURL() )
            ModifiedPathHdl_Impl(*m_pCbbPath);
    }
}

/*************************************************************************
|*
|* Click on imagebutton : target
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ClickTargetHdl_Impl, Button*, void)
{
    if ( GetPathType ( maStrURL ) == EPathType::ExistsFile  ||
         maStrURL.isEmpty() ||
         maStrURL.equalsIgnoreAsciiCase( sFileScheme ) ||
         maStrURL.startsWith( sHash ) )
    {
        mpMarkWnd->SetError( LERR_NOERROR );

        EnterWait();

        if ( maStrURL.equalsIgnoreAsciiCase( sFileScheme ) )
            mpMarkWnd->RefreshTree ( "" );
        else
            mpMarkWnd->RefreshTree ( maStrURL );

        LeaveWait();
    }
    else
        mpMarkWnd->SetError( LERR_DOCNOTOPEN );

    ShowMarkWnd ();
}

/*************************************************************************
|*
|* Contents of combobox "Path" modified
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ModifiedPathHdl_Impl, Edit&, void)
{
    maStrURL = GetCurrentURL();

    maTimer.SetTimeout( 2500 );
    maTimer.Start();

    m_pFtFullURL->SetText( maStrURL );
}

/*************************************************************************
|*
|* If path-field was modify, to browse the new doc after timeout
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, TimeoutHdl_Impl, Timer *, void)
{
    if ( IsMarkWndVisible() && ( GetPathType( maStrURL )== EPathType::ExistsFile ||
                                  maStrURL.isEmpty() ||
                                  maStrURL.equalsIgnoreAsciiCase( sFileScheme ) ) )
    {
        EnterWait();

        if ( maStrURL.equalsIgnoreAsciiCase( sFileScheme ) )
            mpMarkWnd->RefreshTree ( "" );
        else
            mpMarkWnd->RefreshTree ( maStrURL );

        LeaveWait();
    }
}

/*************************************************************************
|*
|* Contents of editfield "Target" modified
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, ModifiedTargetHdl_Impl, Edit&, void)
{
    maStrURL = GetCurrentURL();

    if ( IsMarkWndVisible() )
        mpMarkWnd->SelectEntry ( m_pEdTarget->GetText() );

    m_pFtFullURL->SetText( maStrURL );
}

/*************************************************************************
|*
|* editfield "Target" lost focus
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkDocTp, LostFocusPathHdl_Impl, Control&, void)
{
    maStrURL = GetCurrentURL();

    m_pFtFullURL->SetText( maStrURL );
}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/

void SvxHyperlinkDocTp::SetMarkStr ( const OUString& aStrMark )
{
    m_pEdTarget->SetText ( aStrMark );

    ModifiedTargetHdl_Impl ( *m_pEdTarget );
}

/*************************************************************************
|*
|* retrieve kind of pathstr
|*
|************************************************************************/

SvxHyperlinkDocTp::EPathType SvxHyperlinkDocTp::GetPathType ( const OUString& rStrPath )
{
    INetURLObject aURL( rStrPath, INetProtocol::File );

    if( aURL.HasError() )
        return EPathType::Invalid;
    else
        return EPathType::ExistsFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
