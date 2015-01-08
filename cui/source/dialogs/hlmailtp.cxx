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

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <sfx2/request.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>

#include "hlmailtp.hxx"

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperlinkMailTp::SvxHyperlinkMailTp ( vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet)
:   SvxHyperlinkTabPageBase ( pParent, pDlg, "HyperlinkMailPage", "cui/ui/hyperlinkmailpage.ui",
                              rItemSet )
{
    get(m_pRbtMail, "linktyp_mail");
    get(m_pRbtNews, "linktyp_news");
    get(m_pCbbReceiver, "receiver");
    m_pCbbReceiver->SetSmartProtocol(INET_PROT_MAILTO);
    get(m_pBtAdrBook, "adressbook");
    m_pBtAdrBook->SetModeImage(Image(CUI_RES(RID_SVXBMP_ADRESSBOOK)));
    get(m_pFtSubject, "subject_label");
    get(m_pEdSubject, "subject");

    // Disable display of bitmap names.
    m_pBtAdrBook->EnableTextDisplay (false);

    InitStdControls();

    m_pCbbReceiver->Show();
    m_pCbbReceiver->SetHelpId( HID_HYPERDLG_MAIL_PATH );

    SetExchangeSupport ();

    // set defaults
    m_pRbtMail->Check ();

    // set handlers
    m_pRbtMail->SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
    m_pRbtNews->SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
    m_pBtAdrBook->SetClickHdl      ( LINK ( this, SvxHyperlinkMailTp, ClickAdrBookHdl_Impl ) );
    m_pCbbReceiver->SetModifyHdl   ( LINK ( this, SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl) );

    if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
        m_pBtAdrBook->Hide();
}

SvxHyperlinkMailTp::~SvxHyperlinkMailTp ()
{
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
        m_pEdSubject->SetText (aEmptyStr);
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

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

    // subject for EMail-url
    if( aURL.GetProtocol() == INET_PROT_MAILTO )
    {
        if ( m_pEdSubject->GetText() != OUString(aEmptyStr) )
        {
            OUString aQuery("subject=");
            aQuery += m_pEdSubject->GetText();
            aURL.SetParam(aQuery);
        }
    }

    if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        return aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
    else //#105788# always create a URL even if it is not valid
        return aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

IconChoicePage* SvxHyperlinkMailTp::Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet )
{
    return( new SvxHyperlinkMailTp( pWindow, pDlg, rItemSet ) );
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
    //if rScheme is empty or unknown the default beaviour is like it where MAIL
    const sal_Char sNewsScheme[]   = INET_NEWS_SCHEME;

    bool bMail = !rScheme.startsWith(sNewsScheme);

    //update protocol button selection:
    m_pRbtMail->Check(bMail);
    m_pRbtNews->Check(!bMail);

    //update target:
    RemoveImproperProtocol(rScheme);
    m_pCbbReceiver->SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for MAIL:
    m_pBtAdrBook->Enable(bMail);
    m_pEdSubject->Enable(bMail);
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkMailTp::RemoveImproperProtocol(const OUString& aProperScheme)
{
    OUString aStrURL ( m_pCbbReceiver->GetText() );
    if ( aStrURL != aEmptyStr )
    {
        OUString aStrScheme = GetSchemeFromURL( aStrURL );
        if ( aStrScheme != aEmptyStr && aStrScheme != aProperScheme )
        {
            aStrURL = aStrURL.copy( aStrScheme.getLength() );
            m_pCbbReceiver->SetText ( aStrURL );
        }
    }
}

OUString SvxHyperlinkMailTp::GetSchemeFromButtons() const
{
    if( m_pRbtNews->IsChecked() )
        return OUString(INET_NEWS_SCHEME);
    return OUString(INET_MAILTO_SCHEME);
}

INetProtocol SvxHyperlinkMailTp::GetSmartProtocolFromButtons() const
{
    if( m_pRbtNews->IsChecked() )
    {
        return INET_PROT_NEWS;
    }
    return INET_PROT_MAILTO;
}

/*************************************************************************
|*
|* Click on radiobutton : Type EMail
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkMailTp, Click_SmartProtocol_Impl)
{
    OUString aScheme = GetSchemeFromButtons();
    SetScheme( aScheme );
    return( 0L );
}

/*************************************************************************
|*
|* Contens of editfield "receiver" modified
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl)
{
    OUString aScheme = GetSchemeFromURL( m_pCbbReceiver->GetText() );
    if(!aScheme.isEmpty())
        SetScheme( aScheme );

    return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : addressbook
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkMailTp, ClickAdrBookHdl_Impl)
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if( pViewFrame )
    {
        SfxItemPool &rPool = pViewFrame->GetPool();
        SfxRequest aReq(SID_VIEW_DATA_SOURCE_BROWSER, SfxCallMode::SLOT, rPool);
        pViewFrame->ExecuteSlot( aReq, true );
    }


    return( 0L );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
