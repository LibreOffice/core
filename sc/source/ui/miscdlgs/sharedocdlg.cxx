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

#include <osl/security.hxx>
#include <sfx2/dialoghelper.hxx>
#include <svl/sharecontrolfile.hxx>
#include <unotools/useroptions.hxx>

#include <docsh.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "sharedocdlg.hxx"
#include "scresid.hxx"
#include "viewdata.hxx"

using namespace ::com::sun::star;

class ScShareTable : public SvSimpleTable
{
private:
    OUString m_sWidestAccessString;
public:
    explicit ScShareTable(SvSimpleTableContainer& rParent)
        : SvSimpleTable(rParent)
    {
        m_sWidestAccessString = getWidestTime(*ScGlobal::pLocaleData);
    }
    virtual void Resize() override
    {
        SvSimpleTable::Resize();
        if (isInitialLayout(this))
            setColWidths();
    }
    void setColWidths()
    {
        HeaderBar &rBar = GetTheHeaderBar();
        if (rBar.GetItemCount() < 2)
            return;

        long nAccessedWidth = 12 +
            std::max(rBar.GetTextWidth(rBar.GetItemText(2)),
            GetTextWidth(m_sWidestAccessString));
        long nWebSiteWidth = std::max(
            12 + rBar.GetTextWidth(rBar.GetItemText(1)),
            GetSizePixel().Width() - nAccessedWidth);
        long aStaticTabs[]= { 2, 0, 0 };
        aStaticTabs[2] = nWebSiteWidth;
        SvSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
    }
};

// class ScShareDocumentDlg

ScShareDocumentDlg::ScShareDocumentDlg( vcl::Window* pParent, ScViewData* pViewData )
    : ModalDialog(pParent, "ShareDocumentDialog", "modules/scalc/ui/sharedocumentdlg.ui")
    , mpViewData(pViewData)
    , mpDocShell(NULL)
{
    OSL_ENSURE( mpViewData, "ScShareDocumentDlg CTOR: mpViewData is null!" );
    mpDocShell = ( mpViewData ? mpViewData->GetDocShell() : NULL );
    OSL_ENSURE( mpDocShell, "ScShareDocumentDlg CTOR: mpDocShell is null!" );

    get(m_pCbShare, "share");
    get(m_pFtWarning, "warning");

    SvSimpleTableContainer *pCtrl = get<SvSimpleTableContainer>("users");
    pCtrl->set_height_request(pCtrl->GetTextHeight()*9);
    m_pLbUsers = VclPtr<ScShareTable>::Create(*pCtrl);

    m_aStrNoUserData = get<FixedText>("nouserdata")->GetText();
    m_aStrUnknownUser = get<FixedText>("unknownuser")->GetText();
    m_aStrExclusiveAccess = get<FixedText>("exclusive")->GetText();

    bool bIsDocShared = mpDocShell && mpDocShell->IsDocShared();
    m_pCbShare->Check( bIsDocShared );
    m_pCbShare->SetToggleHdl( LINK( this, ScShareDocumentDlg, ToggleHandle ) );
    m_pFtWarning->Enable( bIsDocShared );

    long nTabs[] = { 2, 0, 0 };
    m_pLbUsers->SetTabs( nTabs );

    OUString aHeader(get<FixedText>("name")->GetText());
    aHeader += "\t";
    aHeader += get<FixedText>("accessed")->GetText();
    m_pLbUsers->InsertHeaderEntry( aHeader, HEADERBAR_APPEND, HeaderBarItemBits::LEFT | HeaderBarItemBits::LEFTIMAGE | HeaderBarItemBits::VCENTER );
    m_pLbUsers->SetSelectionMode( NO_SELECTION );

    UpdateView();
}

ScShareDocumentDlg::~ScShareDocumentDlg()
{
    disposeOnce();
}

void ScShareDocumentDlg::dispose()
{
    m_pLbUsers.disposeAndClear();
    m_pCbShare.clear();
    m_pFtWarning.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(ScShareDocumentDlg, ToggleHandle, CheckBox&, void)
{
    m_pFtWarning->Enable( m_pCbShare->IsChecked() );
}

bool ScShareDocumentDlg::IsShareDocumentChecked() const
{
    return m_pCbShare->IsChecked();
}

void ScShareDocumentDlg::UpdateView()
{
    if ( !mpDocShell )
    {
        return;
    }

    if ( mpDocShell->IsDocShared() )
    {
        try
        {
            ::svt::ShareControlFile aControlFile( mpDocShell->GetSharedFileURL() );
            std::vector<LockFileEntry> aUsersData = aControlFile.GetUsersData();
            sal_Int32 nLength = aUsersData.size();

            if ( nLength > 0 )
            {
                sal_Int32 nUnknownUser = 1;

                for ( sal_Int32 i = 0; i < nLength; ++i )
                {
                    if ( !aUsersData[i][LockFileComponent::EDITTIME].isEmpty() )
                    {
                        OUString aUser;
                        if ( !aUsersData[i][LockFileComponent::OOOUSERNAME].isEmpty() )
                        {
                            aUser = aUsersData[i][LockFileComponent::OOOUSERNAME];
                        }
                        else if ( !aUsersData[i][LockFileComponent::SYSUSERNAME].isEmpty() )
                        {
                            aUser = aUsersData[i][LockFileComponent::SYSUSERNAME];
                        }
                        else
                        {
                            aUser = m_aStrUnknownUser + " " + OUString::number( nUnknownUser++ );
                        }

                        // parse the edit time string of the format "DD.MM.YYYY hh:mm"
                        OUString aDateTimeStr = aUsersData[i][LockFileComponent::EDITTIME];
                        sal_Int32 nIndex = 0;
                        OUString aDateStr = aDateTimeStr.getToken( 0, ' ', nIndex );
                        OUString aTimeStr = aDateTimeStr.getToken( 0, ' ', nIndex );
                        nIndex = 0;
                        sal_uInt16 nDay = sal::static_int_cast< sal_uInt16 >( aDateStr.getToken( 0, '.', nIndex ).toInt32() );
                        sal_uInt16 nMonth = sal::static_int_cast< sal_uInt16 >( aDateStr.getToken( 0, '.', nIndex ).toInt32() );
                        sal_uInt16 nYear = sal::static_int_cast< sal_uInt16 >( aDateStr.getToken( 0, '.', nIndex ).toInt32() );
                        nIndex = 0;
                        sal_uInt16 nHours = sal::static_int_cast< sal_uInt16 >( aTimeStr.getToken( 0, ':', nIndex ).toInt32() );
                        sal_uInt16 nMinutes = sal::static_int_cast< sal_uInt16 >( aTimeStr.getToken( 0, ':', nIndex ).toInt32() );
                        Date aDate( nDay, nMonth, nYear );
                        tools::Time aTime( nHours, nMinutes );
                        DateTime aDateTime( aDate, aTime );

                        OUString aString( aUser );
                        aString += "\t";
                        aString += formatTime(aDateTime, *ScGlobal::pLocaleData);

                        m_pLbUsers->InsertEntry( aString );
                    }
                }
            }
            else
            {
                m_pLbUsers->InsertEntry( m_aStrNoUserData );
            }
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "ScShareDocumentDlg::UpdateView(): caught exception\n" );
            m_pLbUsers->Clear();
            m_pLbUsers->InsertEntry( m_aStrNoUserData );
        }
    }
    else
    {
        // get OOO user name
        SvtUserOptions aUserOpt;
        OUString aUser = aUserOpt.GetFirstName();
        if ( !aUser.isEmpty() )
        {
            aUser += " ";
        }
        aUser += aUserOpt.GetLastName();
        if ( aUser.isEmpty() )
        {
            // get sys user name
            OUString aUserName;
            ::osl::Security aSecurity;
            aSecurity.getUserName( aUserName );
            aUser = aUserName;
        }
        if ( aUser.isEmpty() )
        {
            // unknown user name
            aUser = m_aStrUnknownUser;
        }
        aUser += " ";
        aUser += m_aStrExclusiveAccess;
        OUString aString = aUser + "\t";

        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();

        util::DateTime uDT(xDocProps->getModificationDate());
        DateTime aDateTime(uDT);

        aString += formatTime(aDateTime, *ScGlobal::pLocaleData);
        aString += " ";
        aString += ScGlobal::pLocaleData->getTime( aDateTime, false );

        m_pLbUsers->InsertEntry( aString );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
