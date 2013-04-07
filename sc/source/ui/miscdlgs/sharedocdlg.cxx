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
#include <svl/sharecontrolfile.hxx>
#include <unotools/useroptions.hxx>

#include <docsh.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>


#include "sharedocdlg.hxx"
#include "sharedocdlg.hrc"
#include "scresid.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"


using namespace ::com::sun::star;


//=============================================================================
// class ScShareDocumentDlg
//=============================================================================

ScShareDocumentDlg::ScShareDocumentDlg( Window* pParent, ScViewData* pViewData )
    :ModalDialog( pParent, ScResId( RID_SCDLG_SHAREDOCUMENT ) )
    ,maCbShare            ( this, ScResId( CB_SHARE ) )
    ,maFtWarning          ( this, ScResId( FT_WARNING ) )
    ,maFlUsers            ( this, ScResId( FL_USERS ) )
    ,maFtUsers            ( this, ScResId( FT_USERS ) )
    ,m_aLbUsersContainer(this, ScResId(LB_USERS))
    ,maLbUsers(m_aLbUsersContainer)
    ,maFlEnd              ( this, ScResId( FL_END ) )
    ,maBtnHelp            ( this, ScResId( BTN_HELP ) )
    ,maBtnOK              ( this, ScResId( BTN_OK ) )
    ,maBtnCancel          ( this, ScResId( BTN_CANCEL ) )
    ,maStrTitleName       ( ScResId( STR_TITLE_NAME ) )
    ,maStrTitleAccessed   ( ScResId( STR_TITLE_ACCESSED ) )
    ,maStrNoUserData      ( ScResId( STR_NO_USER_DATA ) )
    ,maStrUnkownUser      ( ScResId( STR_UNKNOWN_USER ) )
    ,maStrExclusiveAccess ( ScResId( STR_EXCLUSIVE_ACCESS ) )
    ,mpViewData           ( pViewData )
    ,mpDocShell           ( NULL )
{
    OSL_ENSURE( mpViewData, "ScShareDocumentDlg CTOR: mpViewData is null!" );
    mpDocShell = ( mpViewData ? mpViewData->GetDocShell() : NULL );
    OSL_ENSURE( mpDocShell, "ScShareDocumentDlg CTOR: mpDocShell is null!" );

    FreeResource();

    bool bIsDocShared = ( mpDocShell ? mpDocShell->IsDocShared() : false );
    maCbShare.Check( bIsDocShared );
    maCbShare.SetToggleHdl( LINK( this, ScShareDocumentDlg, ToggleHandle ) );
    maFtWarning.Enable( bIsDocShared );

    long nTabs[] = { 2, 10, 128 };
    maLbUsers.SetTabs( nTabs );

    String aHeader( maStrTitleName );
    aHeader += '\t';
    aHeader += maStrTitleAccessed;
    maLbUsers.InsertHeaderEntry( aHeader, HEADERBAR_APPEND, HIB_LEFT | HIB_LEFTIMAGE | HIB_VCENTER );
    maLbUsers.SetSelectionMode( NO_SELECTION );

    UpdateView();
}

ScShareDocumentDlg::~ScShareDocumentDlg()
{
}

IMPL_LINK_NOARG(ScShareDocumentDlg, ToggleHandle)
{
    maFtWarning.Enable( maCbShare.IsChecked() );

    return 0;
}

bool ScShareDocumentDlg::IsShareDocumentChecked() const
{
    return maCbShare.IsChecked();
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
            uno::Sequence< uno::Sequence< OUString > > aUsersData = aControlFile.GetUsersData();
            const uno::Sequence< OUString >* pUsersData = aUsersData.getConstArray();
            sal_Int32 nLength = aUsersData.getLength();

            if ( nLength > 0 )
            {
                sal_Int32 nUnknownUser = 1;

                for ( sal_Int32 i = 0; i < nLength; ++i )
                {
                    if ( pUsersData[i].getLength() > SHARED_EDITTIME_ID )
                    {
                        OUString aUser;
                        if ( !pUsersData[i][SHARED_OOOUSERNAME_ID].isEmpty() )
                        {
                            aUser = pUsersData[i][SHARED_OOOUSERNAME_ID];
                        }
                        else if ( !pUsersData[i][SHARED_SYSUSERNAME_ID].isEmpty() )
                        {
                            aUser = pUsersData[i][SHARED_SYSUSERNAME_ID];
                        }
                        else
                        {
                            aUser = OUString(maStrUnkownUser) + " " + OUString::number( nUnknownUser++ );
                        }

                        // parse the edit time string of the format "DD.MM.YYYY hh:mm"
                        OUString aDateTimeStr = pUsersData[i][SHARED_EDITTIME_ID];
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
                        Time aTime( nHours, nMinutes );
                        DateTime aDateTime( aDate, aTime );

                        String aString( aUser );
                        aString += '\t';
                        aString += ScGlobal::pLocaleData->getDate( aDateTime );
                        aString += ' ';
                        aString += ScGlobal::pLocaleData->getTime( aDateTime, false );

                        maLbUsers.InsertEntry( aString, NULL );
                    }
                }
            }
            else
            {
                maLbUsers.InsertEntry( maStrNoUserData, NULL );
            }
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "ScShareDocumentDlg::UpdateView(): caught exception\n" );
            maLbUsers.Clear();
            maLbUsers.InsertEntry( maStrNoUserData, NULL );
        }
    }
    else
    {
        // get OOO user name
        SvtUserOptions aUserOpt;
        String aUser = aUserOpt.GetFirstName();
        if ( aUser.Len() > 0 )
        {
            aUser += ' ';
        }
        aUser += String(aUserOpt.GetLastName());
        if ( aUser.Len() == 0 )
        {
            // get sys user name
            OUString aUserName;
            ::osl::Security aSecurity;
            aSecurity.getUserName( aUserName );
            aUser = aUserName;
        }
        if ( aUser.Len() == 0 )
        {
            // unknown user name
            aUser = maStrUnkownUser;
        }
        aUser += ' ';
        aUser += maStrExclusiveAccess;
        String aString( aUser );
        aString += '\t';

        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();

        util::DateTime uDT(xDocProps->getModificationDate());
        Date d(uDT.Day, uDT.Month, uDT.Year);
        Time t(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        DateTime aDateTime(d,t);

        aString += ScGlobal::pLocaleData->getDate( aDateTime );
        aString += ' ';
        aString += ScGlobal::pLocaleData->getTime( aDateTime, false );

        maLbUsers.InsertEntry( aString, NULL );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
