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

#include <dialmgr.hxx>
#include <cuires.hrc>
#include <sal/macros.h>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/UrlRecord.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/docpasswordrequest.hxx>
#include "webconninfo.hxx"
#include "webconninfo.hrc"

using namespace ::com::sun::star;

//........................................................................
namespace svx
{
//........................................................................

// class PasswordTable ---------------------------------------------------

PasswordTable::PasswordTable(SvxSimpleTableContainer& rParent, WinBits nBits)
    : SvxSimpleTable(rParent, nBits | WB_NOINITIALSELECTION)
{
}

void PasswordTable::InsertHeaderItem( sal_uInt16 nColumn, const String& rText, HeaderBarItemBits nBits )
{
    GetTheHeaderBar().InsertItem( nColumn, rText, 0, nBits );
}

void PasswordTable::ResetTabs()
{
    SetTabs();
}

void PasswordTable::Resort( bool bForced )
{
    sal_uInt16 nColumn = GetSelectedCol();
    if ( 0 == nColumn || bForced ) // only the first column is sorted
    {
        HeaderBarItemBits nBits = GetTheHeaderBar().GetItemBits(1);
        sal_Bool bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
        SvSortMode eMode = SortAscending;

        if ( bUp )
        {
            nBits &= ~HIB_UPARROW;
            nBits |= HIB_DOWNARROW;
            eMode = SortDescending;
        }
        else
        {
            nBits &= ~HIB_DOWNARROW;
            nBits |= HIB_UPARROW;
        }
        GetTheHeaderBar().SetItemBits( 1, nBits );
        SvTreeList* pListModel = GetModel();
        pListModel->SetSortMode( eMode );
        pListModel->Resort();
    }
}

// class WebConnectionInfoDialog -----------------------------------------

// -----------------------------------------------------------------------
WebConnectionInfoDialog::WebConnectionInfoDialog( Window* pParent ) :
     ModalDialog( pParent, CUI_RES( RID_SVXDLG_WEBCONNECTION_INFO ) )
    ,m_aNeverShownFI    ( this, CUI_RES( FI_NEVERSHOWN ) )
    ,m_aPasswordsLBContainer(this, CUI_RES( LB_PASSWORDS))
    ,m_aPasswordsLB(m_aPasswordsLBContainer)
    ,m_aRemoveBtn       ( this, CUI_RES( PB_REMOVE ) )
    ,m_aRemoveAllBtn    ( this, CUI_RES( PB_REMOVEALL ) )
    ,m_aChangeBtn       ( this, CUI_RES( PB_CHANGE ) )
    ,m_aButtonsFL       ( this, CUI_RES( FL_BUTTONS ) )
    ,m_aCloseBtn        ( this, CUI_RES( PB_CLOSE ) )
    ,m_aHelpBtn         ( this, CUI_RES( PB_HELP ) )
    ,m_nPos             ( -1 )
{
    static long aStaticTabs[]= { 3, 0, 150, 250 };
    m_aPasswordsLB.SetTabs( aStaticTabs );
    m_aPasswordsLB.InsertHeaderItem( 1, CUI_RESSTR( STR_WEBSITE ),
        HIB_LEFT | HIB_VCENTER | HIB_FIXEDPOS | HIB_CLICKABLE | HIB_UPARROW );
    m_aPasswordsLB.InsertHeaderItem( 2, CUI_RESSTR( STR_USERNAME ),
        HIB_LEFT | HIB_VCENTER | HIB_FIXEDPOS );
    m_aPasswordsLB.ResetTabs();

    FreeResource();

    m_aPasswordsLB.SetHeaderBarClickHdl( LINK( this, WebConnectionInfoDialog, HeaderBarClickedHdl ) );
    m_aRemoveBtn.SetClickHdl( LINK( this, WebConnectionInfoDialog, RemovePasswordHdl ) );
    m_aRemoveAllBtn.SetClickHdl( LINK( this, WebConnectionInfoDialog, RemoveAllPasswordsHdl ) );
    m_aChangeBtn.SetClickHdl( LINK( this, WebConnectionInfoDialog, ChangePasswordHdl ) );

    // one button too small for its text?
    sal_Int32 i = 0;
    long nBtnTextWidth = 0;
    Window* pButtons[] = { &m_aRemoveBtn, &m_aRemoveAllBtn, &m_aChangeBtn };
    Window** pButton = pButtons;
    const sal_Int32 nBCount = SAL_N_ELEMENTS( pButtons );
    for ( ; i < nBCount; ++i, ++pButton )
    {
        long nTemp = (*pButton)->GetCtrlTextWidth( (*pButton)->GetText() );
        if ( nTemp > nBtnTextWidth )
            nBtnTextWidth = nTemp;
    }
    nBtnTextWidth = nBtnTextWidth * 115 / 100; // a little offset
    long nButtonWidth = m_aRemoveBtn.GetSizePixel().Width();
    if ( nBtnTextWidth > nButtonWidth )
    {
        // so make the buttons broader and its control in front of it smaller
        long nDelta = nBtnTextWidth - nButtonWidth;
        pButton = pButtons;
        for ( i = 0; i < nBCount; ++i, ++pButton )
        {
            Point aNewPos = (*pButton)->GetPosPixel();
            if ( &m_aRemoveAllBtn == (*pButton) )
                aNewPos.X() += nDelta;
            else if ( &m_aChangeBtn == (*pButton) )
                aNewPos.X() -= nDelta;
            Size aNewSize = (*pButton)->GetSizePixel();
            aNewSize.Width() += nDelta;
            (*pButton)->SetPosSizePixel( aNewPos, aNewSize );
        }
    }

    FillPasswordList();

    m_aRemoveBtn.SetClickHdl( LINK( this, WebConnectionInfoDialog, RemovePasswordHdl ) );
    m_aRemoveAllBtn.SetClickHdl( LINK( this, WebConnectionInfoDialog, RemoveAllPasswordsHdl ) );
    m_aChangeBtn.SetClickHdl( LINK( this, WebConnectionInfoDialog, ChangePasswordHdl ) );
    m_aPasswordsLB.SetSelectHdl( LINK( this, WebConnectionInfoDialog, EntrySelectedHdl ) );

    m_aRemoveBtn.Enable( sal_False );
    m_aChangeBtn.Enable( sal_False );

    HeaderBarClickedHdl( NULL );
}

// -----------------------------------------------------------------------
WebConnectionInfoDialog::~WebConnectionInfoDialog()
{
}

// -----------------------------------------------------------------------
IMPL_LINK( WebConnectionInfoDialog, HeaderBarClickedHdl, SvxSimpleTable*, pTable )
{
    m_aPasswordsLB.Resort( NULL == pTable );
    return 0;
}

// -----------------------------------------------------------------------
void WebConnectionInfoDialog::FillPasswordList()
{
    try
    {
        uno::Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            uno::Reference< task::XInteractionHandler > xInteractionHandler(
                task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), 0),
                uno::UNO_QUERY);

            uno::Sequence< task::UrlRecord > aURLEntries = xMasterPasswd->getAllPersistent( xInteractionHandler );
            sal_Int32 nCount = 0;
            for ( sal_Int32 nURLInd = 0; nURLInd < aURLEntries.getLength(); nURLInd++ )
            {
                for ( sal_Int32 nUserInd = 0; nUserInd < aURLEntries[nURLInd].UserList.getLength(); nUserInd++ )
                {
                    ::rtl::OUString aUIEntry( aURLEntries[nURLInd].Url );
                    aUIEntry += ::rtl::OUString::valueOf( (sal_Unicode)'\t' );
                    aUIEntry += aURLEntries[nURLInd].UserList[nUserInd].UserName;
                    SvTreeListEntry* pEntry = m_aPasswordsLB.InsertEntry( aUIEntry );
                    pEntry->SetUserData( (void*)(sal_IntPtr)(nCount++) );
                }
            }

            // remember pos of first url container entry.
            m_nPos = nCount;

            uno::Sequence< rtl::OUString > aUrls
                = xMasterPasswd->getUrls( sal_True /* OnlyPersistent */ );

            for ( sal_Int32 nURLIdx = 0; nURLIdx < aUrls.getLength(); nURLIdx++ )
            {
                ::rtl::OUString aUIEntry( aUrls[ nURLIdx ] );
                aUIEntry += ::rtl::OUString::valueOf( (sal_Unicode)'\t' );
                aUIEntry += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*" ) );
                SvTreeListEntry* pEntry = m_aPasswordsLB.InsertEntry( aUIEntry );
                pEntry->SetUserData( (void*)(sal_IntPtr)(nCount++) );
            }
        }
    }
    catch( uno::Exception& )
    {}
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(WebConnectionInfoDialog, RemovePasswordHdl)
{
    try
    {
        SvTreeListEntry* pEntry = m_aPasswordsLB.GetCurEntry();
        if ( pEntry )
        {
            ::rtl::OUString aURL = m_aPasswordsLB.GetEntryText( pEntry, 0 );
            ::rtl::OUString aUserName = m_aPasswordsLB.GetEntryText( pEntry, 1 );

            uno::Reference< task::XPasswordContainer2 > xPasswdContainer(
                task::PasswordContainer::create(comphelper::getProcessComponentContext()));

            sal_Int32 nPos = (sal_Int32)(sal_IntPtr)pEntry->GetUserData();
            if ( nPos < m_nPos )
            {
                xPasswdContainer->removePersistent( aURL, aUserName );
            }
            else
            {
                xPasswdContainer->removeUrl( aURL );
            }
            m_aPasswordsLB.RemoveEntry( pEntry );
        }
    }
    catch( uno::Exception& )
    {}

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(WebConnectionInfoDialog, RemoveAllPasswordsHdl)
{
    try
    {
        uno::Reference< task::XPasswordContainer2 > xPasswdContainer(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        // should the master password be requested before?
        xPasswdContainer->removeAllPersistent();

        uno::Sequence< rtl::OUString > aUrls
            = xPasswdContainer->getUrls( sal_True /* OnlyPersistent */ );
        for ( sal_Int32 nURLIdx = 0; nURLIdx < aUrls.getLength(); nURLIdx++ )
            xPasswdContainer->removeUrl( aUrls[ nURLIdx ] );

        m_aPasswordsLB.Clear();
    }
    catch( uno::Exception& )
    {}

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(WebConnectionInfoDialog, ChangePasswordHdl)
{
    try
    {
        SvTreeListEntry* pEntry = m_aPasswordsLB.GetCurEntry();
        if ( pEntry )
        {
            ::rtl::OUString aURL = m_aPasswordsLB.GetEntryText( pEntry, 0 );
            ::rtl::OUString aUserName = m_aPasswordsLB.GetEntryText( pEntry, 1 );

            ::comphelper::SimplePasswordRequest* pPasswordRequest
                  = new ::comphelper::SimplePasswordRequest( task::PasswordRequestMode_PASSWORD_CREATE );
            uno::Reference< task::XInteractionRequest > rRequest( pPasswordRequest );

            uno::Reference< task::XInteractionHandler > xInteractionHandler(
                task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), 0),
                uno::UNO_QUERY );
            xInteractionHandler->handle( rRequest );

            if ( pPasswordRequest->isPassword() )
            {
                String aNewPass = pPasswordRequest->getPassword();
                uno::Sequence< ::rtl::OUString > aPasswd( 1 );
                aPasswd[0] = aNewPass;

                uno::Reference< task::XPasswordContainer2 > xPasswdContainer(
                    task::PasswordContainer::create(comphelper::getProcessComponentContext()));
                xPasswdContainer->addPersistent(
                    aURL, aUserName, aPasswd, xInteractionHandler );
            }
        }
    }
    catch( uno::Exception& )
    {}

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(WebConnectionInfoDialog, EntrySelectedHdl)
{
    SvTreeListEntry* pEntry = m_aPasswordsLB.GetCurEntry();
    if ( !pEntry )
    {
        m_aRemoveBtn.Enable( sal_False );
        m_aChangeBtn.Enable( sal_False );
    }
    else
    {
        m_aRemoveBtn.Enable( sal_True );

        // url container entries (-> use system credentials) have
        // no password
        sal_Int32 nPos = (sal_Int32)(sal_IntPtr)pEntry->GetUserData();
        m_aChangeBtn.Enable( nPos < m_nPos );
    }

    return 0;
}

//........................................................................
}   // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
