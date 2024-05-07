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

char const sHash[]          = "#";

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkDocTp::SvxHyperlinkDocTp(weld::Container* pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet)
    : SvxHyperlinkTabPageBase(pParent, pDlg, u"cui/ui/hyperlinkdocpage.ui"_ustr, u"HyperlinkDocPage"_ustr, pItemSet)
    , m_xCbbPath(new SvxHyperURLBox(xBuilder->weld_combo_box(u"path"_ustr)))
    , m_xBtFileopen(xBuilder->weld_button(u"fileopen"_ustr))
    , m_xEdTarget(xBuilder->weld_entry(u"target"_ustr))
    , m_xFtFullURL(xBuilder->weld_label(u"url"_ustr))
    , m_xBtBrowse(xBuilder->weld_button(u"browse"_ustr))
    , m_bMarkWndOpen(false)
{
    m_xCbbPath->SetSmartProtocol(INetProtocol::File);

    InitStdControls();

    m_xCbbPath->show();
    m_xCbbPath->SetBaseURL(INET_FILE_SCHEME);

    SetExchangeSupport();

    // set handlers
    m_xBtFileopen->connect_clicked( LINK ( this, SvxHyperlinkDocTp, ClickFileopenHdl_Impl ) );
    m_xBtBrowse->connect_clicked( LINK ( this, SvxHyperlinkDocTp, ClickTargetHdl_Impl ) );
    m_xCbbPath->connect_changed( LINK ( this, SvxHyperlinkDocTp, ModifiedPathHdl_Impl ) );
    m_xEdTarget->connect_changed( LINK ( this, SvxHyperlinkDocTp, ModifiedTargetHdl_Impl ) );

    m_xCbbPath->connect_focus_out( LINK ( this, SvxHyperlinkDocTp, LostFocusPathHdl_Impl ) );

    maTimer.SetInvokeHandler ( LINK ( this, SvxHyperlinkDocTp, TimeoutHdl_Impl ) );
}

SvxHyperlinkDocTp::~SvxHyperlinkDocTp()
{
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
    m_xCbbPath->set_entry_text( rStrURL.copy( 0, ( nPos == -1 ? rStrURL.getLength() : nPos ) ) );

    // set target in document at editfield
    OUString aStrMark;
    if ( nPos != -1 && nPos < rStrURL.getLength()-1 )
        aStrMark = rStrURL.copy( nPos+1 );
    m_xEdTarget->set_text( aStrMark );

    ModifiedPathHdl_Impl(*m_xCbbPath->getWidget());
}

/*************************************************************************
|*
|* retrieve current url-string
|*
|************************************************************************/
OUString SvxHyperlinkDocTp::GetCurrentURL () const
{
    // get data from dialog-controls
    OUString aStrURL;
    OUString aStrPath( m_xCbbPath->get_active_text() );
    OUString aStrMark( m_xEdTarget->get_text() );

    if ( !aStrPath.isEmpty() )
    {
        INetURLObject aURL( aStrPath );
        if ( aURL.GetProtocol() != INetProtocol::NotValid )    // maybe the path is already a valid
            aStrURL = aStrPath;                             // hyperlink, then we can use this path directly
        else
        {
            osl::FileBase::getFileURLFromSystemPath( aStrPath, aStrURL );
            aStrURL = INetURLObject::decode(aStrURL, INetURLObject::DecodeMechanism::ToIUri, RTL_TEXTENCODING_UTF8);
        }

        //#105788# always create a URL even if it is not valid
        if( aStrURL.isEmpty() )
            aStrURL = aStrPath;
    }

    if( !aStrMark.isEmpty() )
    {
        aStrURL += sHash + aStrMark;
    }

    return aStrURL;
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/
void SvxHyperlinkDocTp::GetCurrentItemData ( OUString& rStrURL, OUString& aStrName,
                                            OUString& aStrIntName, OUString& aStrFrame,
                                            SvxLinkInsertMode& eMode )
{
    // get data from standard-fields
    rStrURL = GetCurrentURL();

    if( rStrURL.equalsIgnoreAsciiCase( INET_FILE_SCHEME ) )
         rStrURL.clear();

    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/
std::unique_ptr<IconChoicePage> SvxHyperlinkDocTp::Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet)
{
    return std::make_unique<SvxHyperlinkDocTp>(pWindow, pDlg, pItemSet);
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/
void SvxHyperlinkDocTp::SetInitFocus()
{
    m_xCbbPath->grab_focus();
}

/*************************************************************************
|*
|* Click on imagebutton : fileopen
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkDocTp, ClickFileopenHdl_Impl, weld::Button&, void)
{
    DisableClose( true );
    // Open Fileopen-Dialog
    sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE,
        mpDialog->getDialog() );
    OUString aOldURL( GetCurrentURL() );
    if( aOldURL.startsWithIgnoreAsciiCase( INET_FILE_SCHEME ) )
    {
        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aOldURL, aPath);
        aDlg.SetDisplayFolder( aPath );
    }

    ErrCode nError = aDlg.Execute();
    DisableClose( false );

    if ( ERRCODE_NONE != nError )
        return;

    OUString aURL( aDlg.GetPath() );
    OUString aPath;

    osl::FileBase::getSystemPathFromFileURL(aURL, aPath);

    m_xCbbPath->SetBaseURL( aURL );
    m_xCbbPath->set_entry_text(aPath);

    if ( aOldURL != GetCurrentURL() )
        ModifiedPathHdl_Impl(*m_xCbbPath->getWidget());
}

/*************************************************************************
|*
|* Click on imagebutton : target
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkDocTp, ClickTargetHdl_Impl, weld::Button&, void)
{
    ShowMarkWnd();

    if ( GetPathType ( maStrURL ) == EPathType::ExistsFile  ||
         maStrURL.isEmpty() ||
         maStrURL.equalsIgnoreAsciiCase( INET_FILE_SCHEME ) ||
         maStrURL.startsWith( sHash ) )
    {
        mxMarkWnd->SetError( LERR_NOERROR );

        weld::WaitObject aWait(mpDialog->getDialog());

        if ( maStrURL.equalsIgnoreAsciiCase( INET_FILE_SCHEME ) )
            mxMarkWnd->RefreshTree ( u""_ustr );
        else
            mxMarkWnd->RefreshTree ( maStrURL );
    }
    else
        mxMarkWnd->SetError( LERR_DOCNOTOPEN );
}

/*************************************************************************
|*
|* Contents of combobox "Path" modified
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkDocTp, ModifiedPathHdl_Impl, weld::ComboBox&, void)
{
    maStrURL = GetCurrentURL();

    maTimer.SetTimeout( 2500 );
    maTimer.Start();

    m_xFtFullURL->set_label( maStrURL );
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
                                  maStrURL.equalsIgnoreAsciiCase( INET_FILE_SCHEME ) ) )
    {
        weld::WaitObject aWait(mpDialog->getDialog());

        if ( maStrURL.equalsIgnoreAsciiCase( INET_FILE_SCHEME ) )
            mxMarkWnd->RefreshTree ( u""_ustr );
        else
            mxMarkWnd->RefreshTree ( maStrURL );
    }
}

/*************************************************************************
|*
|* Contents of editfield "Target" modified
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkDocTp, ModifiedTargetHdl_Impl, weld::Entry&, void)
{
    maStrURL = GetCurrentURL();

    if (IsMarkWndVisible())
        mxMarkWnd->SelectEntry(m_xEdTarget->get_text());

    m_xFtFullURL->set_label( maStrURL );
}

/*************************************************************************
|*
|* editfield "Target" lost focus
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkDocTp, LostFocusPathHdl_Impl, weld::Widget&, void)
{
    maStrURL = GetCurrentURL();

    m_xFtFullURL->set_label( maStrURL );
}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/
void SvxHyperlinkDocTp::SetMarkStr ( const OUString& aStrMark )
{
    m_xEdTarget->set_text(aStrMark);

    ModifiedTargetHdl_Impl ( *m_xEdTarget );
}

/*************************************************************************
|*
|* retrieve kind of pathstr
|*
|************************************************************************/
SvxHyperlinkDocTp::EPathType SvxHyperlinkDocTp::GetPathType ( std::u16string_view rStrPath )
{
    INetURLObject aURL( rStrPath, INetProtocol::File );

    if( aURL.HasError() )
        return EPathType::Invalid;
    else
        return EPathType::ExistsFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
