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

#include <o3tl/string_view.hxx>
#include <unotools/useroptions.hxx>
#include <svl/adrparse.hxx>

#include <hlinettp.hxx>
#include <hlmarkwn_def.hxx>


/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/
SvxHyperlinkInternetTp::SvxHyperlinkInternetTp(weld::Container* pParent,
                                               SvxHpLinkDlg* pDlg,
                                               const SfxItemSet* pItemSet)
    : SvxHyperlinkTabPageBase(pParent, pDlg, u"cui/ui/hyperlinkinternetpage.ui"_ustr, u"HyperlinkInternetPage"_ustr,
                              pItemSet)
    , m_bMarkWndOpen(false)
    , m_xCbbTarget(new SvxHyperURLBox(xBuilder->weld_combo_box(u"target"_ustr)))
    , m_xFtTarget(xBuilder->weld_label(u"target_label"_ustr))
{
    // gtk_size_group_set_ignore_hidden, "Measuring the size of hidden widgets
    // ...  they will report a size of 0 nowadays, and thus, their size will
    // not affect the other size group members", which is unfortunate. So here
    // before we hide the labels, take the size group width and set it as
    // explicit preferred size on a label that won't be hidden
    auto nLabelWidth = m_xFtTarget->get_preferred_size().Width();
    m_xFtTarget->set_size_request(nLabelWidth, -1);

    m_xCbbTarget->SetSmartProtocol(INetProtocol::Http);

    InitStdControls();

    m_xCbbTarget->show();

    SetExchangeSupport ();

    // set handlers
    m_xCbbTarget->connect_focus_out( LINK ( this, SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl ) );
    m_xCbbTarget->connect_changed( LINK ( this, SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl ) );
    maTimer.SetInvokeHandler ( LINK ( this, SvxHyperlinkInternetTp, TimeoutHdl_Impl ) );
}

SvxHyperlinkInternetTp::~SvxHyperlinkInternetTp()
{
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/
void SvxHyperlinkInternetTp::FillDlgFields(const OUString& rStrURL)
{
    INetURLObject aURL(rStrURL);
    OUString aStrScheme(GetSchemeFromURL(rStrURL));

    // set URL-field
    // Show the scheme, #72740
    if ( aURL.GetProtocol() != INetProtocol::NotValid )
        m_xCbbTarget->set_entry_text( aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ) );
    else
        m_xCbbTarget->set_entry_text(rStrURL);

    SetScheme(aStrScheme);
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkInternetTp::GetCurrentItemData ( OUString& rStrURL, OUString& aStrName,
                                                 OUString& aStrIntName, OUString& aStrFrame,
                                                 SvxLinkInsertMode& eMode )
{
    rStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

OUString SvxHyperlinkInternetTp::CreateAbsoluteURL() const
{
    // erase leading and trailing whitespaces
    OUString aStrURL(m_xCbbTarget->get_active_text().trim());

    INetURLObject aURL(aStrURL, GetSmartProtocolFromButtons());

    if ( aURL.GetProtocol() != INetProtocol::NotValid )
        return aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
    else //#105788# always create a URL even if it is not valid
        return aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

std::unique_ptr<IconChoicePage> SvxHyperlinkInternetTp::Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet)
{
    return std::make_unique<SvxHyperlinkInternetTp>(pWindow, pDlg, pItemSet);
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/
void SvxHyperlinkInternetTp::SetInitFocus()
{
    m_xCbbTarget->grab_focus();
}

/*************************************************************************
|*
|* Contents of editfield "Target" modified
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl, weld::ComboBox&, void)
{
    OUString aScheme = GetSchemeFromURL( m_xCbbTarget->get_active_text() );
    if( !aScheme.isEmpty() )
        SetScheme( aScheme );

    // start timer
    maTimer.SetTimeout( 2500 );
    maTimer.Start();
}

/*************************************************************************
|*
|* If target-field was modify, to browse the new doc after timeout
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, TimeoutHdl_Impl, Timer *, void)
{
    RefreshMarkWindow();
}

void SvxHyperlinkInternetTp::SetScheme(std::u16string_view rScheme)
{
    //update target:
    RemoveImproperProtocol(rScheme);
    m_xCbbTarget->SetSmartProtocol( GetSmartProtocolFromButtons() );

    //update 'link target in document'-window and opening-button
    if (o3tl::starts_with(rScheme, INET_HTTP_SCHEME) || rScheme.empty())
    {
        if ( m_bMarkWndOpen )
            ShowMarkWnd ();
    }
    else
    {
        //disable for https and ftp
        if ( m_bMarkWndOpen )
            HideMarkWnd ();
    }
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkInternetTp::RemoveImproperProtocol(std::u16string_view aProperScheme)
{
    OUString aStrURL ( m_xCbbTarget->get_active_text() );
    if ( !aStrURL.isEmpty() )
    {
        OUString aStrScheme(GetSchemeFromURL(aStrURL));
        if ( !aStrScheme.isEmpty() && aStrScheme != aProperScheme )
        {
            aStrURL = aStrURL.copy( aStrScheme.getLength() );
            m_xCbbTarget->set_entry_text( aStrURL );
        }
    }
}

INetProtocol SvxHyperlinkInternetTp::GetSmartProtocolFromButtons()
{
    return INetProtocol::Http;
}

/*************************************************************************
|*
|* Combobox Target lost the focus
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl, weld::Widget&, void)
{
    RefreshMarkWindow();
}

void SvxHyperlinkInternetTp::RefreshMarkWindow()
{
    if (IsMarkWndVisible())
    {
        weld::WaitObject aWait(mpDialog->getDialog());
        OUString aStrURL( CreateAbsoluteURL() );
        if ( !aStrURL.isEmpty() )
            mxMarkWnd->RefreshTree ( aStrURL );
        else
            mxMarkWnd->SetError( LERR_DOCNOTOPEN );
    }
}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/
void SvxHyperlinkInternetTp::SetMarkStr ( const OUString& aStrMark )
{
    OUString aStrURL(m_xCbbTarget->get_active_text());

    const sal_Unicode sUHash = '#';
    sal_Int32 nPos = aStrURL.lastIndexOf( sUHash );

    if( nPos != -1 )
        aStrURL = aStrURL.copy(0, nPos);

    aStrURL += OUStringChar(sUHash) + aStrMark;

    m_xCbbTarget->set_entry_text(aStrURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
