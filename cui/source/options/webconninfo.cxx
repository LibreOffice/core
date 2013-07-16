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

#include "webconninfo.hxx"
#include <dialmgr.hxx>
#include <cuires.hrc>
#include <sal/macros.h>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/UrlRecord.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/docpasswordrequest.hxx>
#include "svtools/treelistentry.hxx"
#include <vcl/layout.hxx>

using namespace ::com::sun::star;

//........................................................................
namespace svx
{
//........................................................................

// class PasswordTable ---------------------------------------------------

PasswordTable::PasswordTable(SvSimpleTableContainer& rParent, WinBits nBits)
    : SvSimpleTable(rParent, nBits | WB_NOINITIALSELECTION)
{
}

void PasswordTable::InsertHeaderItem(sal_uInt16 nColumn, const OUString& rText, HeaderBarItemBits nBits)
{
    GetTheHeaderBar().InsertItem( nColumn, rText, 0, nBits );
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

void PasswordTable::Resize()
{
    SvSimpleTable::Resize();
    if (isInitialLayout(this))
        setColWidths();
}

void PasswordTable::setColWidths()
{
    HeaderBar &rBar = GetTheHeaderBar();
    if (rBar.GetItemCount() < 2)
        return;
    long nUserNameWidth = 12 +
        std::max(rBar.GetTextWidth(rBar.GetItemText(2)),
        GetTextWidth(OUString("XXXXXXXXXXXX")));
    long nWebSiteWidth = std::max(
        12 + rBar.GetTextWidth(rBar.GetItemText(1)),
        GetSizePixel().Width() - nUserNameWidth);
    long aStaticTabs[]= { 2, 0, 0 };
    aStaticTabs[2] = nWebSiteWidth;
    SvSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
}

// class WebConnectionInfoDialog -----------------------------------------

// -----------------------------------------------------------------------
WebConnectionInfoDialog::WebConnectionInfoDialog(Window* pParent)
    : ModalDialog(pParent, "StoredWebConnectionDialog", "cui/ui/storedwebconnectiondialog.ui")
    , m_nPos( -1 )
{
    get(m_pRemoveBtn, "remove");
    get(m_pRemoveAllBtn, "removeall");
    get(m_pChangeBtn, "change");

    SvSimpleTableContainer *pPasswordsLBContainer = get<SvSimpleTableContainer>("logins");
    m_pPasswordsLB = new PasswordTable(*pPasswordsLBContainer, 0);

    long aStaticTabs[]= { 2, 0, 0 };
    m_pPasswordsLB->SetTabs( aStaticTabs );
    m_pPasswordsLB->InsertHeaderItem( 1, get<FixedText>("website")->GetText(),
        HIB_LEFT | HIB_VCENTER | HIB_FIXEDPOS | HIB_CLICKABLE | HIB_UPARROW );
    m_pPasswordsLB->InsertHeaderItem( 2, get<FixedText>("username")->GetText(),
        HIB_LEFT | HIB_VCENTER | HIB_FIXEDPOS );
    pPasswordsLBContainer->set_height_request(m_pPasswordsLB->GetTextHeight()*8);

    m_pPasswordsLB->SetHeaderBarClickHdl( LINK( this, WebConnectionInfoDialog, HeaderBarClickedHdl ) );
    m_pRemoveBtn->SetClickHdl( LINK( this, WebConnectionInfoDialog, RemovePasswordHdl ) );
    m_pRemoveAllBtn->SetClickHdl( LINK( this, WebConnectionInfoDialog, RemoveAllPasswordsHdl ) );
    m_pChangeBtn->SetClickHdl( LINK( this, WebConnectionInfoDialog, ChangePasswordHdl ) );


    FillPasswordList();

    m_pRemoveBtn->SetClickHdl( LINK( this, WebConnectionInfoDialog, RemovePasswordHdl ) );
    m_pRemoveAllBtn->SetClickHdl( LINK( this, WebConnectionInfoDialog, RemoveAllPasswordsHdl ) );
    m_pChangeBtn->SetClickHdl( LINK( this, WebConnectionInfoDialog, ChangePasswordHdl ) );
    m_pPasswordsLB->SetSelectHdl( LINK( this, WebConnectionInfoDialog, EntrySelectedHdl ) );

    m_pRemoveBtn->Enable( sal_False );
    m_pChangeBtn->Enable( sal_False );

    HeaderBarClickedHdl( NULL );
}

WebConnectionInfoDialog::~WebConnectionInfoDialog()
{
    delete m_pPasswordsLB;
}

// -----------------------------------------------------------------------
IMPL_LINK( WebConnectionInfoDialog, HeaderBarClickedHdl, SvSimpleTable*, pTable )
{
    m_pPasswordsLB->Resort( NULL == pTable );
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
                    OUString aUIEntry( aURLEntries[nURLInd].Url );
                    aUIEntry += OUString::valueOf( (sal_Unicode)'\t' );
                    aUIEntry += aURLEntries[nURLInd].UserList[nUserInd].UserName;
                    SvTreeListEntry* pEntry = m_pPasswordsLB->InsertEntry( aUIEntry );
                    pEntry->SetUserData( (void*)(sal_IntPtr)(nCount++) );
                }
            }

            // remember pos of first url container entry.
            m_nPos = nCount;

            uno::Sequence< OUString > aUrls
                = xMasterPasswd->getUrls( sal_True /* OnlyPersistent */ );

            for ( sal_Int32 nURLIdx = 0; nURLIdx < aUrls.getLength(); nURLIdx++ )
            {
                OUString aUIEntry( aUrls[ nURLIdx ] );
                aUIEntry += OUString::valueOf( (sal_Unicode)'\t' );
                aUIEntry += OUString( "*" );
                SvTreeListEntry* pEntry = m_pPasswordsLB->InsertEntry( aUIEntry );
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
        SvTreeListEntry* pEntry = m_pPasswordsLB->GetCurEntry();
        if ( pEntry )
        {
            OUString aURL = m_pPasswordsLB->GetEntryText( pEntry, 0 );
            OUString aUserName = m_pPasswordsLB->GetEntryText( pEntry, 1 );

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
            m_pPasswordsLB->RemoveEntry( pEntry );
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

        uno::Sequence< OUString > aUrls
            = xPasswdContainer->getUrls( sal_True /* OnlyPersistent */ );
        for ( sal_Int32 nURLIdx = 0; nURLIdx < aUrls.getLength(); nURLIdx++ )
            xPasswdContainer->removeUrl( aUrls[ nURLIdx ] );

        m_pPasswordsLB->Clear();
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
        SvTreeListEntry* pEntry = m_pPasswordsLB->GetCurEntry();
        if ( pEntry )
        {
            OUString aURL = m_pPasswordsLB->GetEntryText( pEntry, 0 );
            OUString aUserName = m_pPasswordsLB->GetEntryText( pEntry, 1 );

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
                uno::Sequence< OUString > aPasswd( 1 );
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
    SvTreeListEntry* pEntry = m_pPasswordsLB->GetCurEntry();
    if ( !pEntry )
    {
        m_pRemoveBtn->Enable( sal_False );
        m_pChangeBtn->Enable( sal_False );
    }
    else
    {
        m_pRemoveBtn->Enable( sal_True );

        // url container entries (-> use system credentials) have
        // no password
        sal_Int32 nPos = (sal_Int32)(sal_IntPtr)pEntry->GetUserData();
        m_pChangeBtn->Enable( nPos < m_nPos );
    }

    return 0;
}

//........................................................................
}   // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
