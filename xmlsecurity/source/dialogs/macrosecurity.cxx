/*************************************************************************
 *
 *  $RCSfile: macrosecurity.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: gt $ $Date: 2004-07-16 10:43:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <xmlsecurity/macrosecurity.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>

#ifndef _COM_SUN_STAR_XML_CRYPTO_XSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

// MM : added for password exception
#include <vcl/msgbox.hxx>
#include <com/sun/star/security/NoPasswordException.hpp>
using namespace ::com::sun::star::security;

// Only for bigIntegerToNumericString
#include <xmlsecurity/xmlsignaturehelper.hxx>

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _PICKERHELPER_HXX
#include <svtools/pickerhelper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#include <tools/urlobj.hxx>

#include "dialogs.hrc"
#include "resourcemanager.hxx"

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star;


MacroSecurity::MacroSecurity( Window* _pParent, cssu::Reference< lang::XMultiServiceFactory >& rxMSF, cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment )
    :TabDialog          ( _pParent, XMLSEC_RES( RID_XMLSECTP_MACROSEC ) )
    ,maSignatureHelper  ( rxMSF )
    ,maTabCtrl          ( this, ResId( 1 ) )
    ,maOkBtn            ( this, ResId( BTN_OK ) )
    ,maCancelBtn        ( this, ResId( BTN_CANCEL ) )
    ,maHelpBtn          ( this, ResId( BTN_HELP ) )
    ,maResetBtn         ( this, ResId( BTN_RESET ) )
{
    FreeResource();

    mxSecurityEnvironment = _rxSecurityEnvironment;

    maTabCtrl.SetTabPage( RID_XMLSECTP_SECLEVEL, new MacroSecurityLevelTP( &maTabCtrl, this ) );
    maTabCtrl.SetTabPage( RID_XMLSECTP_TRUSTSOURCES, new MacroSecurityTrustedSourcesTP( &maTabCtrl, this ) );
    maTabCtrl.SetCurPageId( RID_XMLSECTP_SECLEVEL );
}

MacroSecurity::~MacroSecurity()
{
}


MacroSecurityTP::MacroSecurityTP( Window* _pParent, const ResId& _rResId, MacroSecurity* _pDlg )
    :TabPage        ( _pParent, _rResId )
    ,mpDlg          ( _pDlg )
{
}


MacroSecurityLevelTP::MacroSecurityLevelTP( Window* _pParent, MacroSecurity* _pDlg )
    :MacroSecurityTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_SECLEVEL ), _pDlg )
    ,maSecLevelFL       ( this, ResId( FL_SECLEVEL ) )
    ,maVeryHighRB       ( this, ResId( RB_VERYHIGH ) )
    ,maHighRB           ( this, ResId( RB_HIGH ) )
    ,maMediumRB         ( this, ResId( RB_MEDIUM ) )
    ,maLowRB            ( this, ResId( RB_LOW ) )
{
    FreeResource();
}

void MacroSecurityLevelTP::ActivatePage()
{
    mpDlg->EnableReset();
}


IMPL_LINK( MacroSecurityTrustedSourcesTP, AddCertPBHdl, void*, EMTYARG )
{
    CertificateChooser  aChooser( this, mpDlg->mxSecurityEnvironment, mpDlg->maCurrentSignatureInformations );
    if( aChooser.Execute() )
    {
        uno::Reference< css::security::XCertificate > xCert = aChooser.GetSelectedCertificate();

        if( xCert.is() )
        {
//          mpDlg->maCurrentSignatureInformations.push_back( ;

            FillCertLB();
        }
    }

    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, ViewCertPBHdl, void*, EMTYARG )
{
    if( maTrustCertLB.FirstSelected() )
    {
        USHORT nSelected = USHORT( sal_Int32( maTrustCertLB.FirstSelected()->GetUserData() ) );
        const SignatureInformation& rInfo = mpDlg->maCurrentSignatureInformations[ nSelected ];
        uno::Reference< dcss::security::XCertificate > xCert = mpDlg->maSignatureHelper.GetSecurityEnvironment()->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );

        // If we don't get it, create it from signature data:
        if ( !xCert.is() )
            xCert = mpDlg->maSignatureHelper.GetSecurityEnvironment()->createCertificateFromAscii( rInfo.ouX509Certificate ) ;

        DBG_ASSERT( xCert.is(), "*MacroSecurityTrustedSourcesTP::ViewCertPBHdl(): Certificate not found and can't be created!" );

        uno::Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv = mpDlg->maSignatureHelper.GetSecurityEnvironment();
        CertificateViewer aViewer( this, xSecEnv, xCert );
        aViewer.Execute();
    }
    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, RemoveCertPBHdl, void*, EMTYARG )
{
    if( maTrustCertLB.FirstSelected() )
    {
        USHORT nSelected = USHORT( sal_Int32( maTrustCertLB.FirstSelected()->GetUserData() ) );
        mpDlg->maCurrentSignatureInformations.erase( mpDlg->maCurrentSignatureInformations.begin()+nSelected );

        FillCertLB();
    }

    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, AddLocPBHdl, void*, EMTYARG )
{
        try
        {
            rtl::OUString aService( RTL_CONSTASCII_USTRINGPARAM( FOLDER_PICKER_SERVICE_NAME ) );
            uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            uno::Reference < ui::dialogs::XFolderPicker > xFolderPicker( xFactory->createInstance( aService ), uno::UNO_QUERY );

            short nRet = xFolderPicker->execute();

            if( ui::dialogs::ExecutableDialogResults::OK != nRet )
                return 0;

            String aPathStr = xFolderPicker->getDirectory();
            INetURLObject aNewObj( aPathStr );
            aNewObj.removeFinalSlash();

            // then the new path also an URL else system path
            String aNewPathStr = ( aNewObj.GetProtocol() != INET_PROT_NOT_VALID )? aPathStr : aNewObj.getFSysPath( INetURLObject::FSYS_DETECT );

            if( maTrustFileLocLB.GetEntryPos( aNewPathStr ) == LISTBOX_ENTRY_NOTFOUND )
            {
                maTrustFileLocLB.InsertEntry( aNewPathStr );
            }
        }
        catch( uno::Exception& )
        {
            DBG_ERRORFILE( "MacroSecurityTrustedSourcesTP::AddLocPBHdl(): exception from folder picker" )
        }

    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, RemoveLocPBHdl, void*, EMTYARG )
{
    USHORT  nSel = maTrustFileLocLB.GetSelectEntryPos();
    if( nSel != LISTBOX_ENTRY_NOTFOUND )
    {
        maTrustFileLocLB.RemoveEntry( nSel );
    }

    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, TrustCertLBSelectHdl, void*, EMTYARG )
{
    bool    bSel = maTrustCertLB.FirstSelected() != NULL;
    maViewCertPB.Enable( bSel );
    maRemoveCertPB.Enable( bSel );

    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, TrustFileLocLBSelectHdl, void*, EMTYARG )
{
    maRemoveLocPB.Enable( maTrustFileLocLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );

    return 0;
}

/*void MacroSecurityTrustedSourcesTP::InsertCert( uno::Reference< css::security::XCertificate >& _rxCert, USHORT _nInd )
{
        String  aCN_Id( String::CreateFromAscii( "CN" ) );

        SvLBoxEntry*    pEntry = maTrustCertLB.InsertEntry( XmlSec::GetContentPart( _rxCert->getIssuerName(), aCN_Id ) );
        maTrustCertLB.SetEntryText( XmlSec::GetContentPart( _rxCert->getIssuerName(), aCN_Id ), pEntry, 1 );
        maTrustCertLB.SetEntryText( XmlSec::GetDateString( _rxCert->getNotAfter() ), pEntry, 2 );
        pEntry->SetUserData( ( void* ) _nInd );
}*/

void MacroSecurityTrustedSourcesTP::FillCertLB( void )
{
    maTrustCertLB.Clear();

    uno::Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv = mpDlg->maSignatureHelper.GetSecurityEnvironment();
    uno::Reference< css::security::XCertificate > xCert;

    String  aCN_Id( String::CreateFromAscii( "CN" ) );
    int     nInfos = mpDlg->maCurrentSignatureInformations.size();
    for( int n = 0; n < nInfos; ++n )
    {
        const SignatureInformation& rInfo = mpDlg->maCurrentSignatureInformations[n];
        xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );

        if( xCert.is() )
        {
            SvLBoxEntry* pEntry = maTrustCertLB.InsertEntry( XmlSec::GetContentPart( xCert->getSubjectName(), aCN_Id ) );
            maTrustCertLB.SetEntryText( XmlSec::GetContentPart( rInfo.ouX509IssuerName, aCN_Id ), pEntry, 1 );
            maTrustCertLB.SetEntryText( XmlSec::GetDateTimeString( rInfo.ouDate, rInfo.ouTime ), pEntry, 2 );
            pEntry->SetUserData( ( void* ) sal_Int32( n ) );        // missuse user data as index
        }
    }

    TrustCertLBSelectHdl( NULL );
}

MacroSecurityTrustedSourcesTP::MacroSecurityTrustedSourcesTP( Window* _pParent, MacroSecurity* _pDlg )
    :MacroSecurityTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_TRUSTSOURCES ), _pDlg )
    ,maTrustCertFL      ( this, ResId( FL_TRUSTCERT ) )
    ,maTrustCertLB      ( this, ResId( LB_TRUSTCERT ) )
    ,maAddCertPB        ( this, ResId( PB_ADD_TRUSTCERT ) )
    ,maViewCertPB       ( this, ResId( PB_VIEW_TRUSTCERT ) )
    ,maRemoveCertPB     ( this, ResId( PB_REMOVE_TRUSTCERT ) )
    ,maTrustFileLocFL   ( this, ResId( FL_TRUSTFILELOC ) )
    ,maTrustFileLocFI   ( this, ResId( FI_TRUSTFILELOC ) )
    ,maTrustFileLocLB   ( this, ResId( LB_TRUSTFILELOC ) )
    ,maAddLocPB         ( this, ResId( FL_ADD_TRUSTFILELOC ) )
    ,maRemoveLocPB      ( this, ResId( FL_REMOVE_TRUSTFILELOC ) )
{
    static long nTabs[] = { 3, 0, 35*CS_LB_WIDTH/100, 70*CS_LB_WIDTH/100 };
    maTrustCertLB.SetTabs( &nTabs[ 0 ] );
    maTrustCertLB.InsertHeaderEntry( String( ResId( STR_HEADERBAR ) ) );

    FreeResource();

    maAddCertPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, AddCertPBHdl ) );
    maViewCertPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, ViewCertPBHdl ) );
    maViewCertPB.Disable();
    maRemoveCertPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, RemoveCertPBHdl ) );
    maRemoveCertPB.Disable();
    maAddLocPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, AddLocPBHdl ) );
    maRemoveLocPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, RemoveLocPBHdl ) );
    maRemoveLocPB.Disable();

    FillCertLB();
}

void MacroSecurityTrustedSourcesTP::ActivatePage()
{
    mpDlg->EnableReset( false );
}
