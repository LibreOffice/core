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
#include "hyperdlg.hrc"

using namespace ::rtl;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperlinkMailTp::SvxHyperlinkMailTp ( Window *pParent, const SfxItemSet& rItemSet)
:   SvxHyperlinkTabPageBase ( pParent, CUI_RES( RID_SVXPAGE_HYPERLINK_MAIL ),
                              rItemSet ),
    maGrpMailNews   ( this, CUI_RES (GRP_MAILNEWS) ),
    maRbtMail       ( this, CUI_RES (RB_LINKTYP_MAIL) ),
    maRbtNews       ( this, CUI_RES (RB_LINKTYP_NEWS) ),
    maFtReceiver    ( this, CUI_RES (FT_RECEIVER) ),
    maCbbReceiver   ( this, INET_PROT_MAILTO ),
    maBtAdrBook     ( this, CUI_RES (BTN_ADRESSBOOK) ),
    maFtSubject     ( this, CUI_RES (FT_SUBJECT) ),
    maEdSubject     ( this, CUI_RES (ED_SUBJECT) )
{
    // Disable display of bitmap names.
    maBtAdrBook.EnableTextDisplay (sal_False);

    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbReceiver.SetPosSizePixel ( LogicToPixel( Point( COL_2, 25 ), MAP_APPFONT ),
                                    LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );

    maCbbReceiver.Show();
    maCbbReceiver.SetHelpId( HID_HYPERDLG_MAIL_PATH );

    SetExchangeSupport ();

    // set defaults
    maRbtMail.Check ();

    // overload handlers
    maRbtMail.SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
    maRbtNews.SetClickHdl        ( LINK ( this, SvxHyperlinkMailTp, Click_SmartProtocol_Impl ) );
    maBtAdrBook.SetClickHdl      ( LINK ( this, SvxHyperlinkMailTp, ClickAdrBookHdl_Impl ) );
    maCbbReceiver.SetModifyHdl   ( LINK ( this, SvxHyperlinkMailTp, ModifiedReceiverHdl_Impl) );

    if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
        maBtAdrBook.Hide();

    maBtAdrBook.SetAccessibleRelationMemberOf( &maGrpMailNews );
    maBtAdrBook.SetAccessibleRelationLabeledBy( &maFtReceiver );
}

SvxHyperlinkMailTp::~SvxHyperlinkMailTp ()
{
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/

void SvxHyperlinkMailTp::FillDlgFields ( String& aStrURL )
{
    INetURLObject aURL( aStrURL );
    OUString aStrScheme = GetSchemeFromURL( aStrURL );

    // set URL-field and additional controls
    OUString aStrURLc ( aStrURL );
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

        maEdSubject.SetText ( aStrSubject );
    }
    else
    {
        maEdSubject.SetText (aEmptyStr);
    }

    maCbbReceiver.SetText ( aStrURLc );

    SetScheme( aStrScheme );
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkMailTp::GetCurentItemData ( OUString& rStrURL, String& aStrName,
                                             String& aStrIntName, String& aStrFrame,
                                             SvxLinkInsertMode& eMode )
{
    rStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

String SvxHyperlinkMailTp::CreateAbsoluteURL() const
{
    String aStrURL = maCbbReceiver.GetText();
    INetURLObject aURL(aStrURL);

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

    // subject for EMail-url
    if( aURL.GetProtocol() == INET_PROT_MAILTO )
    {
        if ( maEdSubject.GetText() != OUString(aEmptyStr) )
        {
            String aQuery = OUString("subject=");
            aQuery.Append( maEdSubject.GetText() );
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

IconChoicePage* SvxHyperlinkMailTp::Create( Window* pWindow, const SfxItemSet& rItemSet )
{
    return( new SvxHyperlinkMailTp( pWindow, rItemSet ) );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkMailTp::SetInitFocus()
{
    maCbbReceiver.GrabFocus();
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkMailTp::SetScheme( const String& aScheme )
{
    //if  aScheme is empty or unknown the default beaviour is like it where MAIL
    const sal_Char sNewsScheme[]   = INET_NEWS_SCHEME;

    sal_Bool bMail = aScheme.SearchAscii( sNewsScheme ) != 0;

    //update protocol button selection:
    maRbtMail.Check(bMail);
    maRbtNews.Check(!bMail);

    //update target:
    RemoveImproperProtocol(aScheme);
    maCbbReceiver.SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for MAIL:
    maFtSubject.Enable(bMail);
    maEdSubject.Enable(bMail);
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkMailTp::RemoveImproperProtocol(const String& aProperScheme)
{
    String aStrURL ( maCbbReceiver.GetText() );
    if ( aStrURL != aEmptyStr )
    {
        String aStrScheme = GetSchemeFromURL( aStrURL );
        if ( aStrScheme != aEmptyStr && aStrScheme != aProperScheme )
        {
            aStrURL.Erase ( 0, aStrScheme.Len() );
            maCbbReceiver.SetText ( aStrURL );
        }
    }
}

String SvxHyperlinkMailTp::GetSchemeFromButtons() const
{
    if( maRbtNews.IsChecked() )
        return OUString(INET_NEWS_SCHEME);
    return OUString(INET_MAILTO_SCHEME);
}

INetProtocol SvxHyperlinkMailTp::GetSmartProtocolFromButtons() const
{
    if( maRbtNews.IsChecked() )
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
    String aScheme = GetSchemeFromButtons();
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
    String aScheme = GetSchemeFromURL( maCbbReceiver.GetText() );
    if(aScheme.Len()!=0)
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
        SfxRequest aReq(SID_VIEW_DATA_SOURCE_BROWSER, 0, rPool);
        pViewFrame->ExecuteSlot( aReq, sal_True );
    }


    return( 0L );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
