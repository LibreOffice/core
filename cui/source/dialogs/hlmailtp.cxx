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

#include <com/sun/star/util/XURLTransformer.hpp>
#include <sfx2/request.hxx>

#include <sfx2/viewfrm.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>

#include <hlmailtp.hxx>
#include <bitmaps.hlst>

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkMailTp::SvxHyperlinkMailTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet* pItemSet)
:   SvxHyperlinkTabPageBase ( pParent, pDlg, "HyperlinkMailPage", "cui/ui/hyperlinkmailpage.ui",
                              pItemSet )
{
    get(m_pCbbReceiver, "receiver");
    m_pCbbReceiver->SetSmartProtocol(INetProtocol::Mailto);
    get(m_pBtAdrBook, "adressbook");
    m_pBtAdrBook->SetModeImage(Image(StockImage::Yes, RID_SVXBMP_ADRESSBOOK));
    get(m_pFtSubject, "subject_label");
    get(m_pEdSubject, "subject");

    // Disable display of bitmap names.
    m_pBtAdrBook->EnableTextDisplay (false);

    InitStdControls();

    m_pCbbReceiver->Show();

    SetExchangeSupport ();

    // set handlers
    m_pBtAdrBook->SetClickHdl      ( LINK ( this, SvxHyperlinkMailTp, ClickAdrBookHdl_Impl ) );
    m_pCbbReceiver->SetModifyHdl   ( LINK ( this, SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl) );

    if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
        m_pBtAdrBook->Hide();
}

SvxHyperlinkMailTp::~SvxHyperlinkMailTp()
{
    disposeOnce();
}

void SvxHyperlinkMailTp::dispose()
{
    m_pCbbReceiver.clear();
    m_pBtAdrBook.clear();
    m_pFtSubject.clear();
    m_pEdSubject.clear();
    SvxHyperlinkTabPageBase::dispose();
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/

void SvxHyperlinkMailTp::FillDlgFields(const OUString& rStrURL)
{
    OUString aStrScheme = GetSchemeFromURL(rStrURL);

    // set URL-field and additional controls
    OUString aStrURLc (rStrURL);
    // set additional controls for EMail:
    if ( aStrScheme.startsWith( INET_MAILTO_SCHEME ) )
    {
        // Find mail-subject
        OUString aStrSubject, aStrTmp( aStrURLc );

        sal_Int32 nPos = aStrTmp.toAsciiLowerCase().indexOf( "subject" );

        if ( nPos != -1 )
            nPos = aStrTmp.indexOf( '=', nPos );

        if ( nPos != -1 )
            aStrSubject = aStrURLc.copy( nPos+1 );

        nPos = aStrURLc.indexOf( '?' );

        if ( nPos != -1 )
            aStrURLc = aStrURLc.copy( 0, nPos );

        m_pEdSubject->SetText ( aStrSubject );
    }
    else
    {
        m_pEdSubject->SetText ("");
    }

    m_pCbbReceiver->SetText ( aStrURLc );

    SetScheme( aStrScheme );
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkMailTp::GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                             OUString& aStrIntName, OUString& aStrFrame,
                                             SvxLinkInsertMode& eMode )
{
    rStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

OUString SvxHyperlinkMailTp::CreateAbsoluteURL() const
{
    OUString aStrURL = m_pCbbReceiver->GetText();
    INetURLObject aURL(aStrURL);

    if( aURL.GetProtocol() == INetProtocol::NotValid )
    {
        aURL.SetSmartProtocol( INetProtocol::Mailto );
        aURL.SetSmartURL(aStrURL);
    }

    // subject for EMail-url
    if( aURL.GetProtocol() == INetProtocol::Mailto )
    {
        if ( !m_pEdSubject->GetText().isEmpty() )
        {
            OUString aQuery = "subject=" + m_pEdSubject->GetText();
            aURL.SetParam(aQuery);
        }
    }

    if ( aURL.GetProtocol() != INetProtocol::NotValid )
        return aURL.GetMainURL( INetURLObject::DecodeMechanism::WithCharset );
    else //#105788# always create a URL even if it is not valid
        return aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

VclPtr<IconChoicePage> SvxHyperlinkMailTp::Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet* pItemSet )
{
    return VclPtr<SvxHyperlinkMailTp>::Create( pWindow, pDlg, pItemSet );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkMailTp::SetInitFocus()
{
    m_pCbbReceiver->GrabFocus();
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkMailTp::SetScheme(const OUString& rScheme)
{
    //update target:
    RemoveImproperProtocol(rScheme);
    m_pCbbReceiver->SetSmartProtocol( INetProtocol::Mailto );

    //show/hide  special fields for MAIL:
    m_pBtAdrBook->Enable();
    m_pEdSubject->Enable();
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkMailTp::RemoveImproperProtocol(const OUString& aProperScheme)
{
    OUString aStrURL ( m_pCbbReceiver->GetText() );
    if ( !aStrURL.isEmpty() )
    {
        OUString aStrScheme = GetSchemeFromURL( aStrURL );
        if ( !aStrScheme.isEmpty() && aStrScheme != aProperScheme )
        {
            aStrURL = aStrURL.copy( aStrScheme.getLength() );
            m_pCbbReceiver->SetText ( aStrURL );
        }
    }
}

/*************************************************************************
|*
|* Contents of editfield "receiver" modified
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl, Edit&, void)
{
    OUString aScheme = GetSchemeFromURL( m_pCbbReceiver->GetText() );
    if(!aScheme.isEmpty())
        SetScheme( aScheme );
}

/*************************************************************************
|*
|* Click on imagebutton : addressbook
|*
|************************************************************************/

IMPL_STATIC_LINK_NOARG(SvxHyperlinkMailTp, ClickAdrBookHdl_Impl, Button*, void)
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if( pViewFrame )
    {
        SfxItemPool &rPool = pViewFrame->GetPool();
        SfxRequest aReq(SID_VIEW_DATA_SOURCE_BROWSER, SfxCallMode::SLOT, rPool);
        pViewFrame->ExecuteSlot( aReq, true );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
