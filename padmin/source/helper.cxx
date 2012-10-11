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

#include <unistd.h>
#include <helper.hxx>
#include <padialog.hrc>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <tools/config.hxx>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <unotools/confignode.hxx>
#include <vcl/unohelp.hxx>
#include <i18npool/mslangid.hxx>
#include <rtl/ustrbuf.hxx>


using namespace osl;
using namespace padmin;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ui::dialogs;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

#define MAX_PATH 1024

/*
 *  PaResId
 */

ResId padmin::PaResId( sal_uInt32 nId )
{
    static ResMgr* pPaResMgr = NULL;
    if( ! pPaResMgr )
    {
        ::com::sun::star::lang::Locale aLocale;

        utl::OConfigurationNode aNode =
            utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
                    vcl::unohelper::GetMultiServiceFactory(),
                    OUString("org.openoffice.Setup/L10N") );
        if ( aNode.isValid() )
        {
            rtl::OUString aLoc;
            Any aValue = aNode.getNodeValue( OUString("ooLocale") );
            if( aValue >>= aLoc )
            {
                sal_Int32 nIndex = 0;
                aLocale.Language = aLoc.getToken( 0, '-', nIndex );
                aLocale.Country = aLoc.getToken( 0, '-', nIndex );
                aLocale.Variant = aLoc.getToken( 0, '-', nIndex );
            }
        }
        pPaResMgr = ResMgr::SearchCreateResMgr( "spa", aLocale );
        AllSettings aSettings = Application::GetSettings();
        aSettings.SetUILocale( aLocale );
        Application::SetSettings( aSettings );
    }
    return ResId( nId, *pPaResMgr );
}

/*
 *  FindFiles
 */

void padmin::FindFiles( const String& rDirectory, ::std::list< String >& rResult, const String& rSuffixes, bool bRecursive )
{
    rResult.clear();

    OUString aDirPath;
    ::osl::FileBase::getFileURLFromSystemPath( rDirectory, aDirPath );
    Directory aDir( aDirPath );
    if( aDir.open() != FileBase::E_None )
        return;
    DirectoryItem aItem;
    while( aDir.getNextItem( aItem ) == FileBase::E_None )
    {
        FileStatus aStatus( osl_FileStatus_Mask_FileName            |
                            osl_FileStatus_Mask_Type
                            );
        if( aItem.getFileStatus( aStatus ) == FileBase::E_None )
        {
            if( aStatus.getFileType() == FileStatus::Regular ||
                aStatus.getFileType() == FileStatus::Link )
            {
                String aFileName = aStatus.getFileName();
                int nToken = comphelper::string::getTokenCount(rSuffixes, ';');
                while( nToken-- )
                {
                    String aSuffix = rSuffixes.GetToken( nToken, ';' );
                    if( aFileName.Len() > aSuffix.Len()+1 )
                    {
                        String aExtension = aFileName.Copy( aFileName.Len()-aSuffix.Len() );
                        if( aFileName.GetChar( aFileName.Len()-aSuffix.Len()-1 ) == '.' &&
                            aExtension.EqualsIgnoreCaseAscii( aSuffix ) )
                        {
                            rResult.push_back( aFileName );
                            break;
                        }
                    }
                }
            }
            else if( bRecursive && aStatus.getFileType() == FileStatus::Directory )
            {
                OUStringBuffer aSubDir( rDirectory );
                aSubDir.appendAscii( "/", 1 );
                aSubDir.append( aStatus.getFileName() );
                std::list< String > subfiles;
                FindFiles( aSubDir.makeStringAndClear(), subfiles, rSuffixes, bRecursive );
                for( std::list< String >::const_iterator it = subfiles.begin(); it != subfiles.end(); ++it )
                {
                    OUStringBuffer aSubFile( aStatus.getFileName() );
                    aSubFile.appendAscii( "/", 1 );
                    aSubFile.append( *it );
                    rResult.push_back( aSubFile.makeStringAndClear() );
                }
            }
        }
    }
    aDir.close();
}

/*
 *  DelMultiListBox
 */

long DelMultiListBox::Notify( NotifyEvent& rEvent )
{
    long nRet = 0;

    if( rEvent.GetType() == EVENT_KEYINPUT &&
        rEvent.GetKeyEvent()->GetKeyCode().GetCode() == KEY_DELETE )
    {
        m_aDelPressedLink.Call( this );
        nRet = 1;
    }
    else
        nRet = MultiListBox::Notify( rEvent );

    return nRet;
}

/*
 *  DelListBox
 */

long DelListBox::Notify( NotifyEvent& rEvent )
{
    long nRet = 0;

    if( rEvent.GetType() == EVENT_KEYINPUT &&
        rEvent.GetKeyEvent()->GetKeyCode().GetCode() == KEY_DELETE )
    {
        m_aDelPressedLink.Call( this );
        nRet = 1;
    }
    else
        nRet = ListBox::Notify( rEvent );

    return nRet;
}

/*
 *  QueryString
 */

QueryString::QueryString( Window* pParent, String& rQuery, String& rRet, const ::std::list< String >& rChoices ) :
        ModalDialog( pParent, PaResId( RID_STRINGQUERYDLG ) ),
        m_aOKButton( this, PaResId( RID_STRQRY_BTN_OK ) ),
        m_aCancelButton( this, PaResId( RID_STRQRY_BTN_CANCEL ) ),
        m_aFixedText( this, PaResId( RID_STRQRY_TXT_RENAME ) ),
        m_aEdit( this, PaResId( RID_STRQRY_EDT_NEWNAME ) ),
        m_aComboBox( this, PaResId( RID_STRQRY_BOX_NEWNAME ) ),
        m_rReturnValue( rRet )
{
    FreeResource();
    m_aOKButton.SetClickHdl( LINK( this, QueryString, ClickBtnHdl ) );
    m_aFixedText.SetText( rQuery );
    if( rChoices.begin() != rChoices.end() )
    {
        m_aComboBox.SetText( m_rReturnValue );
        m_aComboBox.InsertEntry( m_rReturnValue );
        for( ::std::list<String>::const_iterator it = rChoices.begin(); it != rChoices.end(); ++it )
            m_aComboBox.InsertEntry( *it );
        m_aEdit.Show( sal_False );
        m_bUseEdit = false;
    }
    else
    {
        m_aEdit.SetText( m_rReturnValue );
        m_aComboBox.Show( sal_False );
        m_bUseEdit = true;
    }
    SetText( Application::GetDisplayName() );
}

QueryString::~QueryString()
{
}

IMPL_LINK( QueryString, ClickBtnHdl, Button*, pButton )
{
    if( pButton == &m_aOKButton )
    {
        m_rReturnValue = m_bUseEdit ? m_aEdit.GetText() : m_aComboBox.GetText();
        EndDialog( 1 );
    }
    else
        EndDialog(0);
    return 0;
}

/*
 *  AreYouSure
 */

sal_Bool padmin::AreYouSure( Window* pParent, int nRid )
{
    if( nRid == -1 )
        nRid = RID_YOU_SURE;
    QueryBox aQueryBox( pParent, WB_YES_NO | WB_DEF_NO,
                        String( PaResId( nRid ) ) );
    return aQueryBox.Execute() == RET_NO ? sal_False : sal_True;
}

/*
 *  getPadminRC
 */

static Config* pRC = NULL;

Config& padmin::getPadminRC()
{
    if( ! pRC )
    {
        static const char* pEnv = getenv( "HOME" );
        String aFileName( pEnv ? pEnv : "", osl_getThreadTextEncoding() );
        aFileName.AppendAscii( "/.padminrc" );
        pRC = new Config( aFileName );
    }
    return *pRC;
}

void padmin::freePadminRC()
{
    if( pRC )
        delete pRC, pRC = NULL;
}

bool padmin::chooseDirectory( String& rInOutPath )
{
    bool bRet = false;
    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference< XFolderPicker2 > xFolderPicker = FolderPicker::create(xContext);;
    Reference< XControlAccess > xCA( xFolderPicker, UNO_QUERY );
    if( xCA.is() )
    {
        try
        {
            Any aState;
            aState <<= sal_False;
            xCA->setControlProperty( OUString(  "HelpButton"  ),
                                     OUString(  "Visible"  ),
                                     aState );

        }
        catch( ... )
        {
        }
    }
    INetURLObject aObj( rInOutPath, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
    xFolderPicker->setDisplayDirectory( aObj.GetMainURL(INetURLObject::DECODE_TO_IURI) );
    if( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        aObj = INetURLObject( xFolderPicker->getDirectory() );
        rInOutPath = aObj.PathToFileName();
        bRet = true;
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "could not get FolderPicker service\n" );
#endif
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
