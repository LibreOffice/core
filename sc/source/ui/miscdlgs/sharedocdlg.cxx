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

#include <scresid.hxx>
#include <sharedocdlg.hxx>
#include <strings.hrc>
#include <viewdata.hxx>

using namespace ::com::sun::star;

IMPL_LINK(ScShareDocumentDlg, SizeAllocated, const Size&, rSize, void)
{
    OUString sWidestAccessString = getWidestTime(*ScGlobal::pLocaleData);
    std::vector<int> aWidths;
    const int nAccessWidth = m_xLbUsers->get_pixel_size(sWidestAccessString).Width() * 2;
    aWidths.push_back(rSize.Width() - nAccessWidth);
    m_xLbUsers->set_column_fixed_widths(aWidths);
}

// class ScShareDocumentDlg

ScShareDocumentDlg::ScShareDocumentDlg(weld::Window* pParent, const ScViewData* pViewData)
    : GenericDialogController(pParent, "modules/scalc/ui/sharedocumentdlg.ui",
                              "ShareDocumentDialog")
    , m_aStrNoUserData(ScResId(STR_NO_USER_DATA_AVAILABLE))
    , m_aStrUnknownUser(ScResId(STR_UNKNOWN_USER_CONFLICT))
    , m_aStrExclusiveAccess(ScResId(STR_EXCLUSIVE_ACCESS))
    , mpDocShell(nullptr)
    , m_xCbShare(m_xBuilder->weld_check_button("share"))
    , m_xFtWarning(m_xBuilder->weld_label("warning"))
    , m_xLbUsers(m_xBuilder->weld_tree_view("users"))
{

    OSL_ENSURE( pViewData, "ScShareDocumentDlg CTOR: mpViewData is null!" );
    mpDocShell = ( pViewData ? pViewData->GetDocShell() : nullptr );
    OSL_ENSURE( mpDocShell, "ScShareDocumentDlg CTOR: mpDocShell is null!" );

    std::vector<int> aWidths;
    aWidths.push_back(m_xLbUsers->get_approximate_digit_width() * 25);
    m_xLbUsers->set_column_fixed_widths(aWidths);

    m_xLbUsers->set_size_request(-1, m_xLbUsers->get_height_rows(9));
    m_xLbUsers->connect_size_allocate(LINK(this, ScShareDocumentDlg, SizeAllocated));

    bool bIsDocShared = mpDocShell && mpDocShell->IsDocShared();
    m_xCbShare->set_active(bIsDocShared);
    m_xCbShare->connect_toggled( LINK( this, ScShareDocumentDlg, ToggleHandle ) );
    m_xFtWarning->set_sensitive(bIsDocShared);

    m_xLbUsers->set_selection_mode(SelectionMode::NONE);

    UpdateView();
}

ScShareDocumentDlg::~ScShareDocumentDlg()
{
}

IMPL_LINK_NOARG(ScShareDocumentDlg, ToggleHandle, weld::ToggleButton&, void)
{
    m_xFtWarning->set_sensitive(m_xCbShare->get_active());
}

bool ScShareDocumentDlg::IsShareDocumentChecked() const
{
    return m_xCbShare->get_active();
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

                        OUString aString = formatTime(aDateTime, *ScGlobal::pLocaleData);

                        m_xLbUsers->append_text(aUser);
                        m_xLbUsers->set_text(m_xLbUsers->n_children() - 1, aString, 1);
                    }
                }
            }
            else
            {
                m_xLbUsers->append_text(m_aStrNoUserData);
            }
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "ScShareDocumentDlg::UpdateView(): caught exception" );
            m_xLbUsers->clear();
            m_xLbUsers->append_text(m_aStrNoUserData);
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

        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();

        util::DateTime uDT(xDocProps->getModificationDate());
        DateTime aDateTime(uDT);

        OUString aString = formatTime(aDateTime, *ScGlobal::pLocaleData);
        aString += " ";
        aString += ScGlobal::pLocaleData->getTime( aDateTime, false );

        m_xLbUsers->append_text(aUser);
        m_xLbUsers->set_text(m_xLbUsers->n_children() - 1, aString, 1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
