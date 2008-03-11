/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharedocdlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-11 07:47:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

//-----------------------------------------------------------------------------

#include <osl/security.hxx>
#include <svtools/sharecontrolfile.hxx>
#include <svtools/useroptions.hxx>

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
    ,maLbUsers            ( this, ScResId( LB_USERS ) )
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
    DBG_ASSERT( mpViewData, "ScShareDocumentDlg CTOR: mpViewData is null!" );
    mpDocShell = ( mpViewData ? mpViewData->GetDocShell() : NULL );
    DBG_ASSERT( mpDocShell, "ScShareDocumentDlg CTOR: mpDocShell is null!" );

    FreeResource();

    bool bIsDocShared = ( mpDocShell ? mpDocShell->IsDocShared() : false );
    maCbShare.Check( bIsDocShared );
    maCbShare.SetToggleHdl( LINK( this, ScShareDocumentDlg, ToggleHandle ) );
    maFtWarning.Enable( bIsDocShared );

    long nTabs[] = { 2, 10, 122 };
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

IMPL_LINK( ScShareDocumentDlg, ToggleHandle, void*, EMPTYARG )
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
            ::svt::ShareControlFile aControlFile( mpDocShell->GetSharedFileUrl() );
            uno::Sequence< uno::Sequence< ::rtl::OUString > > aUsersData = aControlFile.GetUsersData();
            const uno::Sequence< ::rtl::OUString >* pUsersData = aUsersData.getConstArray();
            sal_Int32 nLength = aUsersData.getLength();

            if ( nLength > 0 )
            {
                sal_Int32 nUnknownUser = 1;

                for ( sal_Int32 i = 0; i < nLength; ++i )
                {
                    if ( pUsersData[i].getLength() != SHARED_ENTRYSIZE )
                    {
                        continue;
                    }

                    String aUser = pUsersData[i][SHARED_OOOUSERNAME_ID];
                    aUser.EraseLeadingAndTrailingChars();
                    if ( aUser.Len() == 0 )
                    {
                        aUser = pUsersData[i][SHARED_SYSUSERNAME_ID];
                        aUser.EraseLeadingAndTrailingChars();
                    }
                    if ( aUser.Len() == 0 )
                    {
                        aUser = maStrUnkownUser;
                        aUser += ' ';
                        aUser += String::CreateFromInt32( nUnknownUser++ );
                    }

                    String aDateTime = pUsersData[i][SHARED_EDITTIME_ID];

                    String aString( aUser );
                    aString += '\t';
                    aString += aDateTime;
                    maLbUsers.InsertEntry( aString, NULL );
                }
            }
            else
            {
                maLbUsers.InsertEntry( maStrNoUserData, NULL );
            }
        }
        catch ( uno::Exception& )
        {
            DBG_ERROR( "ScShareDocumentDlg::UpdateView(): caught exception\n" );
            maLbUsers.Clear();
            maLbUsers.InsertEntry( maStrNoUserData, NULL );
        }
    }
    else
    {
        // get OOO user name
        SvtUserOptions aUserOpt;
        String aUser = aUserOpt.GetFirstName();
        aUser += ' ';
        aUser += aUserOpt.GetLastName();
        aUser.EraseLeadingAndTrailingChars();
        if ( aUser.Len() == 0 )
        {
            // get sys user name
            ::rtl::OUString aUserName;
            ::osl::Security aSecurity;
            aSecurity.getUserName( aUserName );
            aUser = aUserName;
            aUser.EraseLeadingAndTrailingChars();
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
        aString += ScGlobal::pLocaleData->getTime( aDateTime, FALSE );

        maLbUsers.InsertEntry( aString, NULL );
    }
}
