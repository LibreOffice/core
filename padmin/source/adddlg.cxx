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

#include "adddlg.hxx"
#include "newppdlg.hxx"
#include "cmddlg.hxx"
#include "padialog.hrc"

#include "vcl/msgbox.hxx"
#include "vcl/strhelper.hxx"

#include <tools/config.hxx>
#include <osl/thread.h>
#include <rtl/strbuf.hxx>
#include <boost/unordered_set.hpp>

using namespace psp;
using namespace padmin;
using namespace std;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringHash;
using ::rtl::OUStringToOString;


APTabPage::APTabPage( AddPrinterDialog* pParent, const ResId& rResId )
            : TabPage( pParent, rResId ),
              m_aTitle( PaResId( RID_ADDP_STR_TITLE ) ),
              m_pParent( pParent )
{
}

APChooseDevicePage::APChooseDevicePage( AddPrinterDialog* pParent ) :
        APTabPage( pParent, PaResId( RID_ADDP_PAGE_CHOOSEDEV ) ),
        m_aPrinterBtn( this, PaResId( RID_ADDP_CHDEV_BTN_PRINTER ) ),
        m_aFaxBtn( this, PaResId( RID_ADDP_CHDEV_BTN_FAX ) ),
        m_aPDFBtn( this, PaResId( RID_ADDP_CHDEV_BTN_PDF ) ),
        m_aOldBtn( this, PaResId( RID_ADDP_CHDEV_BTN_OLD ) ),
        m_aOverTxt( this, PaResId( RID_ADDP_CHDEV_TXT_OVER ) )
{
    FreeResource();
    m_aPrinterBtn.Check( sal_True );
    m_aFaxBtn.Check( sal_False );
    m_aPDFBtn.Check( sal_False );
    m_aOldBtn.Check( sal_False );
    if( ! AddPrinterDialog::getOldPrinterLocation().Len() )
        m_aOldBtn.Enable( sal_False );
    if( ! PrinterInfoManager::get().addOrRemovePossible() )
    {
        m_aPrinterBtn.Check( sal_False );
        m_aFaxBtn.Check( sal_True );
        m_aPrinterBtn.Enable( sal_False );
        m_aOldBtn.Enable( sal_False );
    }
}

APChooseDevicePage::~APChooseDevicePage()
{
}

bool APChooseDevicePage::check()
{
    return true;
}

void APChooseDevicePage::fill( PrinterInfo& rInfo )
{
    if( m_aPDFBtn.IsChecked() )
    {
        rInfo.m_aFeatures = OUString("pdf=");
    }
    else if( m_aFaxBtn.IsChecked() )
    {
        rInfo.m_aFeatures = OUString("fax");
    }
    else
        rInfo.m_aFeatures = OUString();
}

//--------------------------------------------------------------------

APChooseDriverPage::APChooseDriverPage( AddPrinterDialog* pParent )
        : APTabPage( pParent, PaResId( RID_ADDP_PAGE_CHOOSEDRIVER ) ),
          m_aDriverTxt( this, PaResId( RID_ADDP_CHDRV_TXT_DRIVER ) ),
          m_aDriverBox( this, PaResId( RID_ADDP_CHDRV_BOX_DRIVER ) ),
          m_aAddBtn( this, PaResId( RID_ADDP_CHDRV_BTN_ADD ) ),
          m_aRemBtn( this, PaResId( RID_ADDP_CHDRV_BTN_REMOVE ) ),
          m_aRemStr( PaResId( RID_ADDP_CHDRV_STR_REMOVE ) )
{
    FreeResource();
    m_aAddBtn.SetClickHdl( LINK( this, APChooseDriverPage, ClickBtnHdl ) );
    m_aRemBtn.SetClickHdl( LINK( this, APChooseDriverPage, ClickBtnHdl ) );
    m_aDriverBox.setDelPressedLink( LINK( this, APChooseDriverPage, DelPressedHdl ) );
    updateDrivers();
}

APChooseDriverPage::~APChooseDriverPage()
{
    for( int i = 0; i < m_aDriverBox.GetEntryCount(); i++ )
        delete (String*)m_aDriverBox.GetEntryData( i );
}

bool APChooseDriverPage::check()
{
    return m_aDriverBox.GetSelectEntryCount() > 0;
}

void APChooseDriverPage::fill( PrinterInfo& rInfo )
{
    sal_uInt16 nPos = m_aDriverBox.GetSelectEntryPos();
    String* pDriver = (String*)m_aDriverBox.GetEntryData( nPos );
    rInfo.m_aDriverName = *pDriver;
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "m_aLastPrinterName = \"%s\", rInfo.m_aPrinterName = \"%s\"\n",
             OUStringToOString( m_aLastPrinterName, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
             OUStringToOString( rInfo.m_aPrinterName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    if( rInfo.m_aPrinterName.equals( m_aLastPrinterName ) )
    {
        String aPrinter( AddPrinterDialog::uniquePrinterName( m_aDriverBox.GetEntry( nPos ) ) );
        rInfo.m_aPrinterName = m_aLastPrinterName = aPrinter;
    }
}

void APChooseDriverPage::updateDrivers( bool bRefresh, const rtl::OUString& rSelectDriver )
{
    for( int k = 0; k < m_aDriverBox.GetEntryCount(); k++ )
        delete (String*)m_aDriverBox.GetEntryData( k );
    m_aDriverBox.Clear();

    std::list< rtl::OUString > aDrivers;
    psp::PPDParser::getKnownPPDDrivers( aDrivers, bRefresh );

    rtl::OUString aSelectDriver( psp::PPDParser::getPPDPrinterName( rSelectDriver ) );

    rtl::OUString aSelectedEntry;
    for( std::list< rtl::OUString >::const_iterator it = aDrivers.begin(); it != aDrivers.end(); ++it )
    {
        rtl::OUString aDriver( psp::PPDParser::getPPDPrinterName( *it ) );
        if( !aDriver.isEmpty() )
        {
            int nPos = m_aDriverBox.InsertEntry( aDriver );
            m_aDriverBox.SetEntryData( nPos, new String( *it ) );
            if( aDriver == aSelectDriver )
                aSelectedEntry = aDriver;
        }
    }

    m_aDriverBox.SelectEntry( aSelectedEntry );
    m_aRemBtn.Enable( m_aDriverBox.GetEntryCount() > 0 );
}

IMPL_LINK( APChooseDriverPage, DelPressedHdl, ListBox*, pListBox )
{
    if( pListBox == &m_aDriverBox )
        ClickBtnHdl( &m_aRemBtn );

    return 0;
}

IMPL_LINK( APChooseDriverPage, ClickBtnHdl, PushButton*, pButton )
{
    if( pButton == &m_aAddBtn )
    {
        PPDImportDialog aDlg( this );
        if( aDlg.Execute() )
        {
            const std::list< rtl::OUString >& rImported( aDlg.getImportedFiles() );
            if( rImported.empty() )
                updateDrivers( true );
            else
                updateDrivers( true, rImported.front() );
        }
    }
    else if( pButton == &m_aRemBtn )
    {
        rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
        PrinterInfoManager& rPIManager( PrinterInfoManager::get() );

        for( int i = 0; i < m_aDriverBox.GetSelectEntryCount(); i++ )
        {
            int nSelect = m_aDriverBox.GetSelectEntryPos(i);
            String aDriver( *(String*)m_aDriverBox.GetEntryData( nSelect ) );
            if( aDriver.Len() )
            {
                // never delete the default driver
                if( aDriver.EqualsIgnoreCaseAscii( "SGENPRT" ) )
                {
                    String aText( PaResId( RID_ERR_REMOVESGENPRT ) );
                    aText.SearchAndReplace( rtl::OUString( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
                    ErrorBox aErrorBox( this, WB_OK | WB_DEF_OK, aText );
                    aErrorBox.SetText( m_aRemStr );
                    aErrorBox.Execute();
                    continue;
                }

                PrinterInfo aDefInfo( rPIManager.getPrinterInfo( rPIManager.getDefaultPrinter() ) );
                // for comparisons convert to a OUString
                OUString aPPD( aDriver );
                if( aDefInfo.m_aDriverName == aPPD )
                {
                    String aText( PaResId( RID_ERR_REMOVEDEFAULTDRIVER ) );
                    aText.SearchAndReplace( rtl::OUString( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
                    ErrorBox aErrorBox( this, WB_OK | WB_DEF_OK, aText );
                    aErrorBox.SetText( m_aRemStr );
                    aErrorBox.Execute();
                    continue;
                }

                ::std::list< OUString > aPrinters;
                ::std::list< OUString >::iterator it;
                rPIManager.listPrinters( aPrinters );
                for( it = aPrinters.begin(); it != aPrinters.end(); ++it )
                {
                    PrinterInfo aInfo( rPIManager.getPrinterInfo( *it ) );
                    if( aInfo.m_aDriverName == aPPD )
                        break;
                }

                if( it != aPrinters.end() )
                {
                    String aText( PaResId( RID_QUERY_DRIVERUSED ) );
                    aText.SearchAndReplace( rtl::OUString( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
                    QueryBox aBox( this, WB_YES_NO | WB_DEF_NO, aText );
                    aBox.SetText( m_aRemStr );
                    if( aBox.Execute() == RET_NO )
                        continue;
                }
                else
                {
                    String aText( PaResId( RID_QUERY_REMOVEDRIVER ) );
                    aText.SearchAndReplace( rtl::OUString( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
                    QueryBox aBox( this, WB_YES_NO | WB_DEF_NO, aText );
                    aBox.SetText( m_aRemStr );
                    if( aBox.Execute() == RET_NO )
                        continue;
                }

                // remove the printers using this driver
                for( it = aPrinters.begin(); it != aPrinters.end(); ++it )
                {
                    PrinterInfo aInfo( rPIManager.getPrinterInfo( *it ) );
                    if( aInfo.m_aDriverName == aPPD )
                        rPIManager.removePrinter( *it );
                }

                std::list< rtl::OUString > aDirs;
                // get only psprint's directories, not eventual system dirs
                psp::getPrinterPathList( aDirs, NULL );
                std::list< rtl::OUString >::iterator dir;
                for( dir = aDirs.begin(); dir != aDirs.end(); ++dir )
                {
                    ::std::list< String > aFiles;
                    ::std::list< String >::iterator file;
                    OUStringBuffer aDir( *dir );
                    aDir.append( sal_Unicode( '/' ) );
                    aDir.appendAscii( PRINTER_PPDDIR );
                    rtl::OUString aPPDDir( aDir.makeStringAndClear() );
                    FindFiles( aPPDDir, aFiles, String(  "PS;PPD;PS.GZ;PPD.GZ"  ), true );
                    for( file = aFiles.begin(); file != aFiles.end(); ++file )
                    {
                        String aFile( aPPDDir );
                        if( aFile.GetChar( aFile.Len() ) != '/' )
                            aFile.AppendAscii( "/" );
                        aFile.Append( *file );

                        int nPos = file->SearchBackward( '.' );
                        if( file->Copy( 0, nPos ) == String( aPPD ) )
                        {
                            rtl::OString aSysPath(rtl::OUStringToOString(aFile, aEncoding));
                            if (unlink(aSysPath.getStr()))
                            {
                                String aText( PaResId( RID_ERR_REMOVEDRIVERFAILED ) );
                                aText.SearchAndReplace( rtl::OUString( "%s1" ), m_aDriverBox.GetSelectEntry( i ) );
                                aText.SearchAndReplace( rtl::OUString( "%s2" ), aFile );
                                ErrorBox aErrorBox( this, WB_OK | WB_DEF_OK, aText );
                                aErrorBox.SetText( m_aRemStr );
                                aErrorBox.Execute();
                            }
                        }
                    }
                }
            }
        }
        updateDrivers();
    }
    return 0;
}

//--------------------------------------------------------------------

APNamePage::APNamePage( AddPrinterDialog* pParent, const String& rInitName, DeviceKind::type eKind )
        : APTabPage( pParent, PaResId( RID_ADDP_PAGE_NAME ) ),
          m_aNameTxt(
                     this,
                     PaResId(
                             eKind == DeviceKind::Printer ? RID_ADDP_NAME_TXT_NAME :
                             eKind == DeviceKind::Fax ? RID_ADDP_NAME_TXT_FAXNAME : RID_ADDP_NAME_TXT_PDFNAME
                             )
                     ),
          m_aNameEdt(
                     this,
                     PaResId(
                             eKind == DeviceKind::Printer ? RID_ADDP_NAME_EDT_NAME :
                             eKind == DeviceKind::Fax ? RID_ADDP_NAME_EDT_FAXNAME : RID_ADDP_NAME_EDT_PDFNAME
                             )
                     ),
          m_aDefaultBox( this, PaResId( RID_ADDP_NAME_BOX_DEFAULT ) ),
          m_aFaxSwallowBox( this, PaResId( RID_ADDP_NAME_BOX_FAXSWALLOW ) )
{
    FreeResource();
    if( eKind != DeviceKind::Printer )
        m_aDefaultBox.Show( sal_False );
    else
        m_aNameEdt.SetText( rInitName );
    if( eKind != DeviceKind::Fax )
        m_aFaxSwallowBox.Show( sal_False );

    m_aNameEdt.SetText( AddPrinterDialog::uniquePrinterName( m_aNameEdt.GetText() ) );
    m_aDefaultBox.Check( sal_False );
    m_aFaxSwallowBox.Check( sal_False );
}

APNamePage::~APNamePage()
{
}

bool APNamePage::check()
{
    return !m_aNameEdt.GetText().isEmpty();
}

void APNamePage::fill( PrinterInfo& rInfo )
{
    rInfo.m_aPrinterName = m_aNameEdt.GetText();
}

//--------------------------------------------------------------------

APCommandPage::APCommandPage( AddPrinterDialog* pParent, DeviceKind::type eKind )
        : APTabPage( pParent, PaResId( RID_ADDP_PAGE_COMMAND ) ),
          m_aCommandTxt( this, PaResId( RID_ADDP_CMD_TXT_COMMAND ) ),
          m_aCommandBox( this, PaResId( eKind == DeviceKind::Pdf ? RID_ADDP_CMD_BOX_PDFCOMMAND : RID_ADDP_CMD_BOX_COMMAND ) ),
          m_aHelpBtn( this, PaResId( RID_ADDP_CMD_BTN_HELP ) ),
          m_aHelpTxt( PaResId( eKind == DeviceKind::Fax ? RID_ADDP_CMD_STR_FAXHELP : RID_ADDP_CMD_STR_PDFHELP ) ),
          m_aPdfDirTxt( this, PaResId( RID_ADDP_CMD_TXT_PDFDIR ) ),
          m_aPdfDirEdt( this, PaResId( RID_ADDP_CMD_EDT_PDFDIR ) ),
          m_aPdfDirBtn( this, PaResId( RID_ADDP_CMD_BTN_PDFDIR ) ),
          m_eKind( eKind )
{
    FreeResource();
    ::std::list< String > aCommands;
    if( m_eKind == DeviceKind::Printer )
    {
        m_aHelpBtn.Show( sal_False );
        Size aSize = m_aCommandTxt.GetSizePixel();
        aSize.Width() = m_aCommandBox.GetSizePixel().Width();
        m_aCommandTxt.SetSizePixel( aSize );
    }
    if( m_eKind != DeviceKind::Pdf )
    {
        m_aPdfDirBtn.Show( sal_False );
        m_aPdfDirEdt.Show( sal_False );
        m_aPdfDirTxt.Show( sal_False );
    }
    switch( m_eKind )
    {
        case DeviceKind::Printer:   CommandStore::getPrintCommands( aCommands );break;
        case DeviceKind::Fax:       CommandStore::getFaxCommands( aCommands );break;
        case DeviceKind::Pdf:       CommandStore::getPdfCommands( aCommands );break;
    }
    // adjust height of command text and help button
    Rectangle aPosSize( m_aCommandTxt.GetPosPixel(), m_aCommandTxt.GetSizePixel() );
    Rectangle aTextSize = m_aCommandTxt.GetTextRect( Rectangle( Point(), aPosSize.GetSize() ), m_aCommandTxt.GetText() );
    if( aTextSize.GetWidth() <= 2*(aPosSize.GetWidth()+1) )
    {
        Size aNewSize( aPosSize.GetWidth(), aPosSize.GetHeight()*2/3 );
        if( aNewSize.Height() < m_aHelpBtn.GetSizePixel().Height()+2 )
            aNewSize.Height() = m_aHelpBtn.GetSizePixel().Height()+2;
        Point aNewPos( aPosSize.Left(), aPosSize.Top() + aPosSize.GetHeight() - aNewSize.Height() );
        m_aCommandTxt.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.X() = m_aHelpBtn.GetPosPixel().X();
        m_aHelpBtn.SetPosPixel( aNewPos );
    }

    // fill in commands
    ::std::list< String >::iterator it;
    for( it = aCommands.begin(); it != aCommands.end(); ++it )
        m_aCommandBox.InsertEntry( *it );

    m_aHelpBtn.SetClickHdl( LINK( this, APCommandPage, ClickBtnHdl ) );
    m_aPdfDirBtn.SetClickHdl( LINK( this, APCommandPage, ClickBtnHdl ) );
    if( m_eKind != DeviceKind::Printer )
    {
        m_aCommandBox.SetModifyHdl( LINK( this, APCommandPage, ModifyHdl ) );
        m_pParent->enableNext( false );
    }
}

APCommandPage::~APCommandPage()
{
    ::std::list< String > aCommands;
    String aLastCommand( m_aCommandBox.GetText() );
    for( int i = 0; i < m_aCommandBox.GetEntryCount(); i++ )
    {
        String aCommand( m_aCommandBox.GetEntry( i ) );
        if( aCommand != aLastCommand )
            aCommands.push_back( aCommand );
    }
    aCommands.push_back( aLastCommand );
    switch( m_eKind )
    {
        case DeviceKind::Printer:   CommandStore::setPrintCommands( aCommands );break;
        case DeviceKind::Fax:       CommandStore::setFaxCommands( aCommands );break;
        case DeviceKind::Pdf:       CommandStore::setPdfCommands( aCommands );break;
    }
}

IMPL_LINK( APCommandPage, ClickBtnHdl, PushButton*, pButton )
{
    if( pButton == &m_aHelpBtn )
    {
        InfoBox aBox( this, m_aHelpTxt );
        aBox.Execute();
    }
    else if( pButton == &m_aPdfDirBtn )
    {
        String aPath( m_aPdfDirEdt.GetText() );
        if( chooseDirectory( aPath ) )
            m_aPdfDirEdt.SetText( aPath );
    }
    return 0;
}

IMPL_LINK( APCommandPage, ModifyHdl, ComboBox*, pBox )
{
    if( pBox == &m_aCommandBox )
    {
        m_pParent->enableNext( !m_aCommandBox.GetText().isEmpty() );
    }
    return 0;
}

bool APCommandPage::check()
{
    return true;
}

void APCommandPage::fill( PrinterInfo& rInfo )
{
    rInfo.m_aCommand = m_aCommandBox.GetText();
}

//--------------------------------------------------------------------

APOldPrinterPage::APOldPrinterPage( AddPrinterDialog* pParent )
        : APTabPage( pParent, PaResId( RID_ADDP_PAGE_OLDPRINTERS ) ),
          m_aOldPrinterTxt( this, PaResId( RID_ADDP_OLD_TXT_PRINTERS ) ),
          m_aOldPrinterBox( this, PaResId( RID_ADDP_OLD_BOX_PRINTERS ) ),
          m_aSelectAllBtn( this, PaResId( RID_ADDP_OLD_BTN_SELECTALL ) )
{
    FreeResource();

    m_aSelectAllBtn.SetClickHdl( LINK( this, APOldPrinterPage, ClickBtnHdl ) );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    String aFileName( AddPrinterDialog::getOldPrinterLocation() );
    Config aConfig( aFileName );

    // read defaults
    aConfig.SetGroup( "Xprinter,PostScript" );
    rtl::OString aDefPageSize( aConfig.ReadKey( "PageSize" ) );
    rtl::OString aDefOrientation( aConfig.ReadKey( "Orientation" ) );
    rtl::OString aDefMarginLeft( aConfig.ReadKey( "MarginLeft" ) );
    rtl::OString aDefMarginRight( aConfig.ReadKey( "MarginRight" ) );
    rtl::OString aDefMarginTop( aConfig.ReadKey( "MarginTop" ) );
    rtl::OString aDefMarginBottom( aConfig.ReadKey( "MarginBottom" ) );
    rtl::OString aDefScale( aConfig.ReadKey( "Scale" ) );

    aConfig.SetGroup( "devices" );
    int nDevices = aConfig.GetKeyCount();
    for( int nKey = 0; nKey < nDevices; nKey++ )
    {
        aConfig.SetGroup( "devices" );
        rtl::OString aPrinter(aConfig.GetKeyName(nKey));
        rtl::OString aValue(aConfig.ReadKey(aPrinter));
        rtl::OString aPort(aValue.getToken(1, ','));
        rtl::OString aDriver(aValue.getToken(0, ' '));
        rtl::OString aPS( aValue.getToken(0, ',').getToken(1, ' ') );
        rtl::OString aNewDriver(aDriver);
        if( aDriver.equalsL(RTL_CONSTASCII_STRINGPARAM("GENERIC")))
            aNewDriver = rtl::OString(RTL_CONSTASCII_STRINGPARAM("SGENPRT"));

        if( aPS != "PostScript" )
            continue;

        const PPDParser* pParser = PPDParser::getParser(rtl::OStringToOUString(aNewDriver, aEncoding));
        if( pParser == NULL )
        {
            String aText( PaResId( RID_TXT_DRIVERDOESNOTEXIST ) );
            aText.SearchAndReplace( String(  "%s1"  ), rtl::OStringToOUString(aPrinter, aEncoding) );
            aText.SearchAndReplace( String(  "%s2"  ), rtl::OStringToOUString(aDriver, aEncoding) );
            InfoBox aBox( this, aText );
            aBox.Execute();
            continue;
        }

        // read the command
        aConfig.SetGroup( "ports" );
        rtl::OString aCommand( aConfig.ReadKey( aPort ) );
        if (aCommand.isEmpty())
        {
            String aText( PaResId( RID_TXT_PRINTERWITHOUTCOMMAND ) );
            aText.SearchAndReplace( String(  "%s"  ), rtl::OStringToOUString(aPrinter, aEncoding) );
            InfoBox aBox( this, aText );
            aBox.Execute();
            continue;
        }


        String aUPrinter( AddPrinterDialog::uniquePrinterName(rtl::OStringToOUString(aPrinter, aEncoding)) );

        PrinterInfo aInfo;
        aInfo.m_aDriverName = rtl::OStringToOUString(aNewDriver, aEncoding);
        aInfo.m_pParser         = pParser;
        aInfo.m_aContext.setParser( pParser );
        aInfo.m_aPrinterName    = aUPrinter;
        aInfo.m_aCommand = rtl::OStringToOUString(aCommand, aEncoding);

        // read the printer settings
        rtl::OStringBuffer aGroup(aDriver);
        aGroup.append(RTL_CONSTASCII_STRINGPARAM(",PostScript,"));
        aGroup.append(aPort);
        aConfig.SetGroup(aGroup.makeStringAndClear());

        aValue = aConfig.ReadKey( "PageSize", aDefPageSize );
        int nLeft, nRight, nTop, nBottom;
        if( !aValue.isEmpty() &&
            aInfo.m_pParser->getMargins( rtl::OStringToOUString(aValue, aEncoding),
                                         nLeft, nRight, nTop, nBottom ) )
        {
            const PPDKey* pKey = aInfo.m_pParser->getKey( String(  "PageSize"  ) );
            const PPDValue* pValue = pKey ? pKey->getValue( rtl::OStringToOUString(aValue, aEncoding) ) : NULL;
            if( pKey && pValue )
                aInfo.m_aContext.setValue( pKey, pValue );
            aValue = aConfig.ReadKey( "MarginLeft", aDefMarginLeft );
            if (!aValue.isEmpty())
                aInfo.m_nLeftMarginAdjust = aValue.toInt32() - (int)((double)nLeft * 35.27777778 );
            aValue = aConfig.ReadKey( "MarginRight", aDefMarginRight );
            if (!aValue.isEmpty())
                aInfo.m_nRightMarginAdjust = aValue.toInt32() - (int)((double)nRight * 35.27777778 );
            aValue = aConfig.ReadKey( "MarginTop", aDefMarginTop );
            if (!aValue.isEmpty())
                aInfo.m_nTopMarginAdjust = aValue.toInt32() - (int)((double)nTop * 35.27777778 );
            aValue = aConfig.ReadKey( "MarginBottom", aDefMarginBottom );
            if (!aValue.isEmpty())
                aInfo.m_nBottomMarginAdjust = aValue.toInt32() - (int)((double)nBottom * 35.27777778 );
        }

        aValue = aConfig.ReadKey( "Copies", aDefScale );
        if (!aValue.isEmpty())
            aInfo.m_nCopies = aValue.toInt32();

        aValue = aConfig.ReadKey( "Comment" );
        aInfo.m_aComment = rtl::OStringToOUString(aValue, aEncoding);

        aValue = aConfig.ReadKey( "Level" );
        if (!aValue.isEmpty())
            aInfo.m_nPSLevel = aValue.toInt32();

        aValue = aConfig.ReadKey( "Orientation", aDefOrientation );
        if (!aValue.isEmpty())
            aInfo.m_eOrientation = aValue.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("landscape")) ? orientation::Landscape : orientation::Portrait;
        int nGroupKeys = aConfig.GetKeyCount();
        for( int nPPDKey = 0; nPPDKey < nGroupKeys; nPPDKey++ )
        {
            rtl::OString aPPDKey( aConfig.GetKeyName( nPPDKey ) );
            // ignore page region
            // there are some ppd keys in old Xpdefaults that
            // should never have been writte because they are defaults
            // PageRegion leads to problems in conjunction
            // with a not matching PageSize
            if (aPPDKey.matchL(RTL_CONSTASCII_STRINGPARAM("PPD_")) &&
                !aPPDKey.equalsL(RTL_CONSTASCII_STRINGPARAM("PPD_PageRegion")))
            {
                aValue = aConfig.ReadKey( nPPDKey );
                aPPDKey = aPPDKey.copy(4);
                const PPDKey* pKey = aInfo.m_pParser->getKey( rtl::OStringToOUString(aPPDKey, RTL_TEXTENCODING_ISO_8859_1) );
                const PPDValue* pValue = pKey ? ( aValue.equalsL(RTL_CONSTASCII_STRINGPARAM("*nil")) ? NULL : pKey->getValue(rtl::OStringToOUString(aValue, RTL_TEXTENCODING_ISO_8859_1)) ) : NULL;
                if( pKey )
                    aInfo.m_aContext.setValue( pKey, pValue, true );
            }
        }

        m_aOldPrinters.push_back( aInfo );
        int nPos = m_aOldPrinterBox.InsertEntry( aInfo.m_aPrinterName );
        m_aOldPrinterBox.SetEntryData( nPos, & m_aOldPrinters.back() );
    }
}

APOldPrinterPage::~APOldPrinterPage()
{
}

IMPL_LINK( APOldPrinterPage, ClickBtnHdl, PushButton*, pButton )
{
    if( pButton == &m_aSelectAllBtn )
    {
        for( int i = 0; i < m_aOldPrinterBox.GetEntryCount(); i++ )
            m_aOldPrinterBox.SelectEntryPos( i );
    }
    return 0;
}

void APOldPrinterPage::addOldPrinters()
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    for( int i = 0; i < m_aOldPrinterBox.GetSelectEntryCount(); i++ )
    {
        PrinterInfo* pInfo = (PrinterInfo*)m_aOldPrinterBox.GetEntryData( m_aOldPrinterBox.GetSelectEntryPos( i ) );
        pInfo->m_aPrinterName = AddPrinterDialog::uniquePrinterName( pInfo->m_aPrinterName );
        if( ! rManager.addPrinter( pInfo->m_aPrinterName, pInfo->m_aDriverName ) )
        {
            String aText( PaResId( RID_TXT_PRINTERADDFAILED ) );
            aText.SearchAndReplace( String(  "%s"  ), pInfo->m_aPrinterName );
                ErrorBox aBox( this, WB_OK | WB_DEF_OK, aText );
                aBox.Execute();
                continue;
        }
        rManager.changePrinterInfo( pInfo->m_aPrinterName, *pInfo );
    }
}

bool APOldPrinterPage::check()
{
    return m_aOldPrinterBox.GetEntryCount() > 0;
}

void APOldPrinterPage::fill( PrinterInfo& )
{
}

//--------------------------------------------------------------------

APFaxDriverPage::APFaxDriverPage( AddPrinterDialog* pParent )
        : APTabPage( pParent, PaResId( RID_ADDP_PAGE_FAXDRIVER ) ),
          m_aFaxTxt( this, PaResId( RID_ADDP_FAXDRV_TXT_DRIVER ) ),
          m_aDefBtn( this, PaResId( RID_ADDP_FAXDRV_BTN_DEFAULT ) ),
          m_aSelectBtn( this, PaResId( RID_ADDP_FAXDRV_BTN_SELECT ) )
{
    FreeResource();

    m_aDefBtn.Check( sal_True );
    m_aSelectBtn.Check( sal_False );
    m_aSelectBtn.SetStyle( m_aSelectBtn.GetStyle() | WB_WORDBREAK );
}

APFaxDriverPage::~APFaxDriverPage()
{
}

bool APFaxDriverPage::check()
{
    return true;
}

void APFaxDriverPage::fill( PrinterInfo& rInfo )
{
    if( isDefault() )
    {
        rInfo.m_aDriverName = OUString("SGENPRT");
    }
}

//--------------------------------------------------------------------

APPdfDriverPage::APPdfDriverPage( AddPrinterDialog* pParent )
        : APTabPage( pParent, PaResId( RID_ADDP_PAGE_PDFDRIVER ) ),
          m_aPdfTxt( this, PaResId( RID_ADDP_PDFDRV_TXT_DRIVER ) ),
          m_aDefBtn( this, PaResId( RID_ADDP_PDFDRV_BTN_DEFAULT ) ),
          m_aDistBtn( this, PaResId( RID_ADDP_PDFDRV_BTN_DIST ) ),
          m_aSelectBtn( this, PaResId( RID_ADDP_PDFDRV_BTN_SELECT ) )
{
    FreeResource();

    m_aDefBtn.Check( sal_True );
    m_aDistBtn.Check( sal_False );
    m_aSelectBtn.Check( sal_False );
    m_aSelectBtn.SetStyle( m_aSelectBtn.GetStyle() | WB_WORDBREAK );
}

APPdfDriverPage::~APPdfDriverPage()
{
}

bool APPdfDriverPage::check()
{
    return true;
}

void APPdfDriverPage::fill( PrinterInfo& rInfo )
{
    if( isDefault() )
        rInfo.m_aDriverName = OUString("SGENPRT");
    else if( isDist() )
        rInfo.m_aDriverName = OUString("ADISTILL");
}

//--------------------------------------------------------------------

AddPrinterDialog::AddPrinterDialog( Window* pParent )
        : ModalDialog( pParent, PaResId( RID_ADD_PRINTER_DIALOG ) ),
          m_aCancelPB( this, PaResId( RID_ADDP_BTN_CANCEL ) ),
          m_aPrevPB( this, PaResId( RID_ADDP_BTN_PREV ) ),
          m_aNextPB( this, PaResId( RID_ADDP_BTN_NEXT ) ),
          m_aFinishPB( this, PaResId( RID_ADDP_BTN_FINISH ) ),
          m_aLine( this, PaResId( RID_ADDP_LINE ) ),
          m_aTitleImage( this, PaResId( RID_ADDP_CTRL_TITLE ) ),
          m_pCurrentPage( NULL ),
          m_pChooseDevicePage( NULL ),
          m_pCommandPage( NULL ),
          m_pChooseDriverPage( NULL ),
          m_pNamePage( NULL ),
          m_pOldPrinterPage( NULL ),
          m_pFaxDriverPage( NULL ),
          m_pFaxSelectDriverPage( NULL ),
          m_pFaxNamePage( NULL ),
          m_pFaxCommandPage( NULL ),
          m_pPdfDriverPage( NULL ),
          m_pPdfSelectDriverPage( NULL ),
          m_pPdfNamePage( NULL ),
          m_pPdfCommandPage( NULL )
{
    FreeResource();
    m_pCurrentPage = m_pChooseDevicePage = new APChooseDevicePage( this );
    m_pCurrentPage->Show( sal_True );
    m_aFinishPB.Enable( sal_False );
    m_aPrevPB.Enable( sal_False );

    m_aNextPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );
    m_aPrevPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );
    m_aFinishPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );
    m_aCancelPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );

    m_aTitleImage.SetBackgroundColor( Color( 0xff, 0xff, 0xff ) );
    m_aTitleImage.SetText( m_pCurrentPage->getTitle() );
    updateSettings();
}

AddPrinterDialog::~AddPrinterDialog()
{
    if( m_pChooseDevicePage )
        delete m_pChooseDevicePage;
    if( m_pChooseDriverPage )
        delete m_pChooseDriverPage;
    if( m_pNamePage )
        delete m_pNamePage;
    if( m_pCommandPage )
        delete m_pCommandPage;
    if( m_pOldPrinterPage )
        delete m_pOldPrinterPage;
    if( m_pFaxDriverPage )
        delete m_pFaxDriverPage;
    if( m_pFaxSelectDriverPage )
        delete m_pFaxSelectDriverPage;
    if( m_pFaxCommandPage )
        delete m_pFaxCommandPage;
    if( m_pFaxNamePage )
        delete m_pFaxNamePage;
    if( m_pPdfDriverPage )
        delete m_pPdfDriverPage;
    if( m_pPdfSelectDriverPage )
        delete m_pPdfSelectDriverPage;
    if( m_pPdfNamePage )
        delete m_pPdfNamePage;
    if( m_pPdfCommandPage )
        delete m_pPdfCommandPage;
}

void AddPrinterDialog::updateSettings()
{
    m_aTitleImage.SetImage( Image( BitmapEx( PaResId( RID_BMP_PRINTER ) ) ) );
}

void AddPrinterDialog::DataChanged( const DataChangedEvent& rEv )
{
    ModalDialog::DataChanged( rEv );
    if( (rEv.GetType() == DATACHANGED_SETTINGS) &&
        (rEv.GetFlags() & SETTINGS_STYLE) )
    {
        updateSettings();
    }
}

void AddPrinterDialog::advance()
{
    m_pCurrentPage->Show( sal_False );
    if( m_pCurrentPage == m_pChooseDevicePage )
    {
        if( m_pChooseDevicePage->isPrinter() )
        {
            if( ! m_pChooseDriverPage )
                m_pChooseDriverPage = new APChooseDriverPage( this );
            m_pCurrentPage = m_pChooseDriverPage;
            m_aPrevPB.Enable( sal_True );
        }
        else if( m_pChooseDevicePage->isOld() )
        {
            if( ! m_pOldPrinterPage )
                m_pOldPrinterPage = new APOldPrinterPage( this );
            m_pCurrentPage = m_pOldPrinterPage;
            m_aPrevPB.Enable( sal_True );
            m_aFinishPB.Enable( sal_True );
            m_aNextPB.Enable( sal_False );
        }
        else if( m_pChooseDevicePage->isFax() )
        {
            if( ! m_pFaxDriverPage )
                m_pFaxDriverPage = new APFaxDriverPage( this );
            m_pCurrentPage = m_pFaxDriverPage;
            m_aPrevPB.Enable( sal_True );
        }
        else if( m_pChooseDevicePage->isPDF() )
        {
            if( ! m_pPdfDriverPage )
                m_pPdfDriverPage = new APPdfDriverPage( this );
            m_pCurrentPage = m_pPdfDriverPage;
            m_aPrevPB.Enable( sal_True );
        }
    }
    else if( m_pCurrentPage == m_pChooseDriverPage )
    {
        if( ! m_pCommandPage )
            m_pCommandPage = new APCommandPage( this, DeviceKind::Printer );
        m_pCurrentPage = m_pCommandPage;
    }
    else if( m_pCurrentPage == m_pCommandPage )
    {
        if( ! m_pNamePage )
            m_pNamePage = new APNamePage( this, m_aPrinter.m_aPrinterName, DeviceKind::Printer );
        else
            m_pNamePage->setText( m_aPrinter.m_aPrinterName );
        m_pCurrentPage = m_pNamePage;
        m_aFinishPB.Enable( sal_True );
        m_aNextPB.Enable( sal_False );
    }
    else if( m_pCurrentPage == m_pFaxDriverPage )
    {
        if( ! m_pFaxDriverPage->isDefault() )
        {
            if( ! m_pFaxSelectDriverPage )
                m_pFaxSelectDriverPage = new APChooseDriverPage( this );
            m_pCurrentPage = m_pFaxSelectDriverPage;
        }
        else
        {
            if( ! m_pFaxCommandPage )
                m_pFaxCommandPage = new APCommandPage( this, DeviceKind::Fax );
            m_pCurrentPage = m_pFaxCommandPage;
        }
    }
    else if( m_pCurrentPage == m_pFaxSelectDriverPage )
    {
        if( ! m_pFaxCommandPage )
            m_pFaxCommandPage = new APCommandPage( this, DeviceKind::Fax );
        m_pCurrentPage = m_pFaxCommandPage;
    }
    else if( m_pCurrentPage == m_pFaxCommandPage )
    {
        if( ! m_pFaxNamePage )
            m_pFaxNamePage = new APNamePage( this, String(), DeviceKind::Fax );
        m_pCurrentPage = m_pFaxNamePage;
        m_aNextPB.Enable( sal_False );
        m_aFinishPB.Enable( sal_True );
    }
    else if( m_pCurrentPage == m_pPdfDriverPage )
    {
        if( ! m_pPdfDriverPage->isDefault() && ! m_pPdfDriverPage->isDist() )
        {
            if( ! m_pPdfSelectDriverPage )
                m_pPdfSelectDriverPage = new APChooseDriverPage( this );
            m_pCurrentPage = m_pPdfSelectDriverPage;
        }
        else
        {
            if( ! m_pPdfCommandPage )
                m_pPdfCommandPage = new APCommandPage( this, DeviceKind::Pdf );
            m_pCurrentPage = m_pPdfCommandPage;
        }
    }
    else if( m_pCurrentPage == m_pPdfSelectDriverPage )
    {
        if( ! m_pPdfCommandPage )
            m_pPdfCommandPage = new APCommandPage( this, DeviceKind::Pdf );
        m_pCurrentPage = m_pPdfCommandPage;
    }
    else if( m_pCurrentPage == m_pPdfCommandPage )
    {
        if( ! m_pPdfNamePage )
            m_pPdfNamePage = new APNamePage( this, String(), DeviceKind::Pdf );
        m_pCurrentPage = m_pPdfNamePage;
        m_aNextPB.Enable( sal_False );
        m_aFinishPB.Enable( sal_True );
    }

    m_pCurrentPage->Show( sal_True );
    m_aTitleImage.SetText( m_pCurrentPage->getTitle() );
}

void AddPrinterDialog::back()
{
    m_pCurrentPage->Show( sal_False );
    if( m_pCurrentPage == m_pChooseDriverPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( sal_False );
    }
    else if( m_pCurrentPage == m_pNamePage )
    {
        m_pCurrentPage = m_pCommandPage;
        m_aNextPB.Enable( sal_True );
    }
    else if( m_pCurrentPage == m_pCommandPage )
    {
        m_pCurrentPage = m_pChooseDriverPage;
    }
    else if( m_pCurrentPage == m_pOldPrinterPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( sal_False );
        m_aNextPB.Enable( sal_True );
    }
    else if( m_pCurrentPage == m_pFaxDriverPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( sal_False );
    }
    else if( m_pCurrentPage == m_pFaxSelectDriverPage )
    {
        m_pCurrentPage = m_pFaxDriverPage;
    }
    else if( m_pCurrentPage == m_pFaxNamePage )
    {
        m_pCurrentPage = m_pFaxCommandPage;
        m_aNextPB.Enable( sal_True );
    }
    else if( m_pCurrentPage == m_pFaxCommandPage )
    {
        m_pCurrentPage = m_pFaxDriverPage->isDefault() ? (APTabPage*)m_pFaxDriverPage : (APTabPage*)m_pFaxSelectDriverPage;
        m_aNextPB.Enable( sal_True );
    }
    else if( m_pCurrentPage == m_pPdfDriverPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( sal_False );
    }
    else if( m_pCurrentPage == m_pPdfSelectDriverPage )
    {
        m_pCurrentPage = m_pPdfDriverPage;
    }
    else if( m_pCurrentPage == m_pPdfNamePage )
    {
        m_pCurrentPage = m_pPdfCommandPage;
        m_aNextPB.Enable( sal_True );
    }
    else if( m_pCurrentPage == m_pPdfCommandPage )
    {
        m_pCurrentPage = m_pPdfDriverPage->isDefault() || m_pPdfDriverPage->isDist() ? (APTabPage*)m_pPdfDriverPage : (APTabPage*)m_pPdfSelectDriverPage;
        m_aNextPB.Enable( sal_True );
    }
    m_pCurrentPage->Show( sal_True );
    m_aTitleImage.SetText( m_pCurrentPage->getTitle() );
}

void AddPrinterDialog::addPrinter()
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    if( ! m_pChooseDevicePage->isOld() )
    {
        m_aPrinter.m_aPrinterName = uniquePrinterName( m_aPrinter.m_aPrinterName );
        if( rManager.addPrinter( m_aPrinter.m_aPrinterName, m_aPrinter.m_aDriverName ) )
        {
            PrinterInfo aInfo( rManager.getPrinterInfo( m_aPrinter.m_aPrinterName ) );
            aInfo.m_aCommand = m_aPrinter.m_aCommand;
            if( m_pChooseDevicePage->isPrinter() )
            {
                if( m_pNamePage->isDefault() )
                    rManager.setDefaultPrinter( m_aPrinter.m_aPrinterName );
            }
            else if( m_pChooseDevicePage->isFax() )
            {
                aInfo.m_aFeatures = OUString("fax=");
                if( m_pFaxNamePage->isFaxSwallow() )
                    aInfo.m_aFeatures += OUString("swallow");
            }
            else if( m_pChooseDevicePage->isPDF() )
            {
                OUString aPdf( "pdf=" );
                aPdf += m_pPdfCommandPage->getPdfDir();
                aInfo.m_aFeatures = aPdf;
            }
            rManager.changePrinterInfo( m_aPrinter.m_aPrinterName, aInfo );
        }
    }
    else if( m_pOldPrinterPage )
        m_pOldPrinterPage->addOldPrinters();
}

IMPL_LINK( AddPrinterDialog, ClickBtnHdl, PushButton*, pButton )
{
    if( pButton == &m_aNextPB )
    {
        if( m_pCurrentPage->check() )
        {
            m_pCurrentPage->fill( m_aPrinter );
            advance();
        }
    }
    else if( pButton == &m_aPrevPB )
    {
        if( m_pCurrentPage->check() )
            m_pCurrentPage->fill( m_aPrinter );
        back();
    }
    else if( pButton == &m_aFinishPB )
    {
        if( m_pCurrentPage->check() )
        {
            m_pCurrentPage->fill( m_aPrinter );
            addPrinter();
            PrinterInfoManager::get().writePrinterConfig();
            EndDialog( 1 );
        }
    }
    else if( pButton == &m_aCancelPB )
        EndDialog( 0 );

    return 0;
}

String AddPrinterDialog::uniquePrinterName( const String& rBase )
{
    String aResult( rBase );

    PrinterInfoManager& rManager( PrinterInfoManager::get() );

    sal_Int32 nVersion = 1;
    list< OUString > aPrinterList;
    rManager.listPrinters( aPrinterList );
    boost::unordered_set< OUString, OUStringHash > aPrinters;
    for( list< OUString >::const_iterator it = aPrinterList.begin(); it != aPrinterList.end(); ++it )
        aPrinters.insert( *it );
    while( aPrinters.find( aResult ) != aPrinters.end() )
    {
        aResult = rBase;
        aResult.AppendAscii( "_" );
        aResult += rtl::OUString::valueOf(nVersion++);
    }

    return aResult;
}

String AddPrinterDialog::getOldPrinterLocation()
{
    static const char* pHome = getenv( "HOME" );
    rtl::OString aFileName;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    if( pHome )
    {
        aFileName = rtl::OStringBuffer().append(pHome).
            append(RTL_CONSTASCII_STRINGPARAM("/.Xpdefaults")).
            makeStringAndClear();
        if (access(aFileName.getStr(), F_OK))
        {
            aFileName = rtl::OStringBuffer().append(pHome).
                append(RTL_CONSTASCII_STRINGPARAM("/.sversionrc")).
                makeStringAndClear();
            Config aSVer(rtl::OStringToOUString(aFileName, aEncoding));
            aSVer.SetGroup( "Versions" );
            aFileName = aSVer.ReadKey( "StarOffice 5.2" );
            if (!aFileName.isEmpty())
                aFileName = aFileName + rtl::OString(RTL_CONSTASCII_STRINGPARAM("/share/xp3/Xpdefaults"));
            else if(
                    (aFileName = aSVer.ReadKey( "StarOffice 5.1" ) ).getLength()
                    ||
                    (aFileName = aSVer.ReadKey( "StarOffice 5.0" ) ).getLength()
                    ||
                    (aFileName = aSVer.ReadKey( "StarOffice 4.0" ) ).getLength()
                    )
            {
                aFileName = aFileName + rtl::OString(RTL_CONSTASCII_STRINGPARAM("/xp3/Xpdefaults"));
            }
            if (!aFileName.isEmpty() && access(aFileName.getStr(), F_OK))
                aFileName = rtl::OString();
        }
    }

    return !aFileName.isEmpty() ? rtl::OStringToOUString(aFileName, aEncoding) : rtl::OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
