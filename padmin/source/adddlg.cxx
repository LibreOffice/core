/*************************************************************************
 *
 *  $RCSfile: adddlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:21:20 $
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

#include <unistd.h>

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _PAD_ADDDLG_HXX_
#include <adddlg.hxx>
#endif
#ifndef _PAD_NEWPPDLG_HXX_
#include <newppdlg.hxx>
#endif
#ifndef _PAD_COMMANDDLG_HXX_
#include <cmddlg.hxx>
#endif
#ifndef _PAD_PADIALOG_HRC_
#include <padialog.hrc>
#endif
#ifndef _PSPRINT_STRHELPER_HXX_
#include <psprint/strhelper.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#include <hash_set>

using namespace rtl;
using namespace psp;
using namespace padmin;
using namespace std;

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
    m_aPrinterBtn.Check( TRUE );
    m_aFaxBtn.Check( FALSE );
    m_aPDFBtn.Check( FALSE );
    m_aOldBtn.Check( FALSE );
    if( ! AddPrinterDialog::getOldPrinterLocation().Len() )
        m_aOldBtn.Enable( FALSE );
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
        rInfo.m_aFeatures = OUString::createFromAscii( "pdf=" );
    }
    else if( m_aFaxBtn.IsChecked() )
    {
        rInfo.m_aFeatures = OUString::createFromAscii( "fax" );
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
    USHORT nPos = m_aDriverBox.GetSelectEntryPos();
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

void APChooseDriverPage::updateDrivers()
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    for( int k = 0; k < m_aDriverBox.GetEntryCount(); k++ )
        delete (String*)m_aDriverBox.GetEntryData( k );
    m_aDriverBox.Clear();

    String aPathList = ::psp::getPrinterPath();
    int nTokens = aPathList.GetTokenCount( ':' );
    for( int i = 0; i < nTokens; i++ )
    {
        String aPath( aPathList.GetToken( i, ':' ) );
        if( aPath.GetChar( aPath.Len() ) != '/' )
            aPath.AppendAscii( "/" );
        aPath.Append( String( RTL_CONSTASCII_USTRINGPARAM( PSPRINT_PPDDIR ) ) );
        if( access( ByteString( aPath, aEncoding ).GetBuffer(), F_OK ) )
            continue;

        ::std::list< String > aFiles;
        FindFiles( aPath, aFiles, String( RTL_CONSTASCII_USTRINGPARAM( "PS;PPD" ) ) );

        for( ::std::list< String >::const_iterator it = aFiles.begin(); it != aFiles.end(); ++it )
        {
            String aPPD( *it );
            aPPD.Erase( aPPD.SearchBackward( '.' ) );
            String aDriver( ::psp::PPDParser::getPPDPrinterName( aPPD ) );
            if( aDriver.Len() )
            {
                int nPos = m_aDriverBox.InsertEntry( aDriver );
                m_aDriverBox.SetEntryData( nPos, new String( aPPD ) );
                if( aPPD.EqualsAscii( "SGENPRT" ) )
                    m_aDriverBox.SelectEntryPos( nPos );
            }
        }
    }
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
            updateDrivers();
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
                    aText.SearchAndReplace( String::CreateFromAscii( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
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
                    aText.SearchAndReplace( String::CreateFromAscii( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
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
                    aText.SearchAndReplace( String::CreateFromAscii( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
                    QueryBox aBox( this, WB_YES_NO | WB_DEF_NO, aText );
                    aBox.SetText( m_aRemStr );
                    if( aBox.Execute() == RET_NO )
                        continue;
                }
                else
                {
                    String aText( PaResId( RID_QUERY_REMOVEDRIVER ) );
                    aText.SearchAndReplace( String::CreateFromAscii( "%s" ), m_aDriverBox.GetSelectEntry( i ) );
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

                ::std::list< String > aDirs;
                String aPathList( ::psp::getPrinterPath() );
                int nTokens = aPathList.GetTokenCount( ':' );
                for( int n = 0; n < nTokens; n++ )
                {
                    String aPath = aPathList.GetToken( n, ':' );
                    aPath.AppendAscii( "/"PSPRINT_PPDDIR );
                    aDirs.push_back( aPath );
                }
                ::std::list< String >::iterator dir;

                for( dir = aDirs.begin(); dir != aDirs.end(); ++dir )
                {
                    ::std::list< String > aFiles;
                    ::std::list< String >::iterator file;
                    FindFiles( *dir, aFiles, String( RTL_CONSTASCII_USTRINGPARAM( "PS;PPD" ) ) );
                    for( file = aFiles.begin(); file != aFiles.end(); ++file )
                    {
                        String aFile( *dir );
                        if( aFile.GetChar( aFile.Len() ) != '/' )
                            aFile.AppendAscii( "/" );
                        aFile.Append( *file );

                        int nPos = file->SearchBackward( '.' );
                        if( file->Copy( 0, nPos ) == String( aPPD ) )
                        {
                            ByteString aSysPath( aFile, aEncoding );
                            if( unlink( aSysPath.GetBuffer() ) )
                            {
                                String aText( PaResId( RID_ERR_REMOVEDRIVERFAILED ) );
                                aText.SearchAndReplace( String::CreateFromAscii( "%s1" ), m_aDriverBox.GetSelectEntry( i ) );
                                aText.SearchAndReplace( String::CreateFromAscii( "%s2" ), aFile );
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
        m_aDefaultBox.Show( FALSE );
    else
        m_aNameEdt.SetText( rInitName );
    if( eKind != DeviceKind::Fax )
        m_aFaxSwallowBox.Show( FALSE );

    m_aNameEdt.SetText( AddPrinterDialog::uniquePrinterName( m_aNameEdt.GetText() ) );
    m_aDefaultBox.Check( FALSE );
    m_aFaxSwallowBox.Check( FALSE );
}

APNamePage::~APNamePage()
{
}

bool APNamePage::check()
{
    return m_aNameEdt.GetText().Len();
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
        m_aHelpBtn.Show( FALSE );
        Size aSize = m_aCommandTxt.GetSizePixel();
        aSize.Width() = m_aCommandBox.GetSizePixel().Width();
        m_aCommandTxt.SetSizePixel( aSize );
    }
    if( m_eKind != DeviceKind::Pdf )
    {
        m_aPdfDirBtn.Show( FALSE );
        m_aPdfDirEdt.Show( FALSE );
        m_aPdfDirTxt.Show( FALSE );
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
        if( chooseDirectory( this, aPath ) )
            m_aPdfDirEdt.SetText( aPath );
    }
    return 0;
}

IMPL_LINK( APCommandPage, ModifyHdl, ComboBox*, pBox )
{
    if( pBox == &m_aCommandBox )
    {
        m_pParent->enableNext( m_aCommandBox.GetText().Len() );
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
    ByteString aDefPageSize( aConfig.ReadKey( "PageSize" ) );
    ByteString aDefOrientation( aConfig.ReadKey( "Orientation" ) );
    ByteString aDefMarginLeft( aConfig.ReadKey( "MarginLeft" ) );
    ByteString aDefMarginRight( aConfig.ReadKey( "MarginRight" ) );
    ByteString aDefMarginTop( aConfig.ReadKey( "MarginTop" ) );
    ByteString aDefMarginBottom( aConfig.ReadKey( "MarginBottom" ) );
    ByteString aDefScale( aConfig.ReadKey( "Scale" ) );
    ByteString aDefCopies( aConfig.ReadKey( "Copies" ) );
    ByteString aDefDPI( aConfig.ReadKey( "DPI" ) );

    aConfig.SetGroup( "devices" );
    int nDevices = aConfig.GetKeyCount();
    for( int nKey = 0; nKey < nDevices; nKey++ )
    {
        aConfig.SetGroup( "devices" );
        ByteString aPrinter( aConfig.GetKeyName( nKey ) );
        ByteString aValue( aConfig.ReadKey( aPrinter ) );
        ByteString aPort( aValue.GetToken( 1, ',' ) );
        ByteString aDriver( aValue.GetToken( 0, ' ' ) );
        ByteString aPS( aValue.GetToken( 0, ',' ).GetToken( 1, ' ' ) );
        ByteString aNewDriver( aDriver );
        if( aDriver == "GENERIC" )
            aNewDriver = "SGENPRT";

        if( aPS != "PostScript" )
            continue;

        const PPDParser* pParser = PPDParser::getParser( String( aNewDriver, aEncoding ) );
        if( pParser == NULL )
        {
            String aText( PaResId( RID_TXT_DRIVERDOESNOTEXIST ) );
            aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s1" ) ), String( aPrinter, aEncoding ) );
            aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s2" ) ), String( aDriver, aEncoding ) );
            InfoBox aBox( this, aText );
            aBox.Execute();
            continue;
        }

        // read the command
        aConfig.SetGroup( "ports" );
        ByteString aCommand( aConfig.ReadKey( aPort ) );
        if( ! aCommand.Len() )
        {
            String aText( PaResId( RID_TXT_PRINTERWITHOUTCOMMAND ) );
            aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), String( aPrinter, aEncoding ) );
            InfoBox aBox( this, aText );
            aBox.Execute();
            continue;
        }


        String aUPrinter( AddPrinterDialog::uniquePrinterName( String( aPrinter, aEncoding ) ) );

        PrinterInfo aInfo;
        aInfo.m_aDriverName     = String( aNewDriver, aEncoding );
        aInfo.m_pParser         = pParser;
        aInfo.m_aContext.setParser( pParser );
        aInfo.m_aPrinterName    = aUPrinter;
        aInfo.m_aCommand        = String( aCommand, aEncoding );

        // read the printer settings
        ByteString aGroup( aDriver );
        aGroup += ",PostScript,";
        aGroup += aPort;
        aConfig.SetGroup( aGroup );

        aValue = aConfig.ReadKey( "PageSize", aDefPageSize );
        int nLeft, nRight, nTop, nBottom;
        if( aValue.Len() &&
            aInfo.m_pParser->getMargins( String( aValue, aEncoding ),
                                         nLeft, nRight, nTop, nBottom ) )
        {
            const PPDKey* pKey = aInfo.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
            const PPDValue* pValue = pKey ? pKey->getValue( String( aValue, aEncoding ) ) : NULL;
            if( pKey && pValue )
                aInfo.m_aContext.setValue( pKey, pValue );
            aValue = aConfig.ReadKey( "MarginLeft", aDefMarginLeft );
            if( aValue.Len() )
                aInfo.m_nLeftMarginAdjust = aValue.ToInt32() - ((double)nLeft * 35.27777778 );
            aValue = aConfig.ReadKey( "MarginRight", aDefMarginRight );
            if( aValue.Len() )
                aInfo.m_nRightMarginAdjust = aValue.ToInt32() - ((double)nRight * 35.27777778 );
            aValue = aConfig.ReadKey( "MarginTop", aDefMarginTop );
            if( aValue.Len() )
                aInfo.m_nTopMarginAdjust = aValue.ToInt32() - ((double)nTop * 35.27777778 );
            aValue = aConfig.ReadKey( "MarginBottom", aDefMarginBottom );
            if( aValue.Len() )
                aInfo.m_nBottomMarginAdjust = aValue.ToInt32() - ((double)nBottom * 35.27777778 );
        }

        aValue = aConfig.ReadKey( "Scale", aDefScale );
        if( aValue.Len() )
            aInfo.m_nScale = 100.0 * StringToDouble( aValue );

        aValue = aConfig.ReadKey( "Copies", aDefScale );
        if( aValue.Len() )
            aInfo.m_nCopies = aValue.ToInt32();

        aValue = aConfig.ReadKey( "Comment" );
        aInfo.m_aComment = String( aValue, aEncoding );

        aValue = aConfig.ReadKey( "Level" );
        if( aValue.Len() )
            aInfo.m_nPSLevel = aValue.ToInt32();

        aValue = aConfig.ReadKey( "Orientation", aDefOrientation );
        if( aValue.Len() )
            aInfo.m_eOrientation = aValue.CompareIgnoreCaseToAscii( "landscape" ) == COMPARE_EQUAL ? orientation::Landscape : orientation::Portrait;
        int nGroupKeys = aConfig.GetKeyCount();
        for( int nPPDKey = 0; nPPDKey < nGroupKeys; nPPDKey++ )
        {
            ByteString aPPDKey( aConfig.GetKeyName( nPPDKey ) );
            // ignore page region
            // there are some ppd keys in old Xpdefaults that
            // should never have been writte because they are defaults
            // PageRegion leads to problems in conjunction
            // with a not matching PageSize
            if( aPPDKey.CompareTo( "PPD_", 4 ) == COMPARE_EQUAL &&
                aPPDKey != "PPD_PageRegion"
                )
            {
                aValue = aConfig.ReadKey( nPPDKey );
                aPPDKey.Erase( 0, 4 );
                const PPDKey* pKey = aInfo.m_pParser->getKey( String( aPPDKey, RTL_TEXTENCODING_ISO_8859_1 ) );
                const PPDValue* pValue = pKey ? ( aValue.Equals( "*nil" ) ? NULL : pKey->getValue( String( aValue, RTL_TEXTENCODING_ISO_8859_1 ) ) ) : NULL;
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
            aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), pInfo->m_aPrinterName );
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

void APOldPrinterPage::fill( PrinterInfo& rInfo )
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

    m_aDefBtn.Check( TRUE );
    m_aSelectBtn.Check( FALSE );
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
        rInfo.m_aDriverName = OUString::createFromAscii( "SGENPRT" );
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

    m_aDefBtn.Check( TRUE );
    m_aDistBtn.Check( FALSE );
    m_aSelectBtn.Check( FALSE );
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
        rInfo.m_aDriverName = OUString::createFromAscii( "SGENPRT" );
    else if( isDist() )
        rInfo.m_aDriverName = OUString::createFromAscii( "ADISTILL" );
}

//--------------------------------------------------------------------

AddPrinterDialog::AddPrinterDialog( Window* pParent )
        : ModalDialog( pParent, PaResId( RID_ADD_PRINTER_DIALOG ) ),
          m_aNextPB( this, PaResId( RID_ADDP_BTN_NEXT ) ),
          m_aPrevPB( this, PaResId( RID_ADDP_BTN_PREV ) ),
          m_aFinishPB( this, PaResId( RID_ADDP_BTN_FINISH ) ),
          m_aCancelPB( this, PaResId( RID_ADDP_BTN_CANCEL ) ),
          m_aLine( this, PaResId( RID_ADDP_LINE ) ),
          m_aTitleImage( this, PaResId( RID_ADDP_CTRL_TITLE ) ),
          m_pCurrentPage( NULL ),
          m_pChooseDevicePage( NULL ),
          m_pChooseDriverPage( NULL ),
          m_pNamePage( NULL ),
          m_pCommandPage( NULL ),
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
    m_pCurrentPage->Show( TRUE );
    m_aFinishPB.Enable( FALSE );
    m_aPrevPB.Enable( FALSE );

    m_aNextPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );
    m_aPrevPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );
    m_aFinishPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );
    m_aCancelPB.SetClickHdl( LINK( this, AddPrinterDialog, ClickBtnHdl ) );

    m_aTitleImage.SetBackgroundColor( Color( 0xff, 0xff, 0xff ) );
    m_aTitleImage.SetText( m_pCurrentPage->getTitle() );
    m_aTitleImage.SetImage( Image( Bitmap( PaResId( RID_BMP_PRINTER ) ) ) );
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
    if( m_pPdfDriverPage )
        delete m_pPdfDriverPage;
    if( m_pPdfSelectDriverPage )
        delete m_pPdfSelectDriverPage;
    if( m_pPdfNamePage )
        delete m_pPdfNamePage;
    if( m_pPdfCommandPage )
        delete m_pPdfCommandPage;
}

void AddPrinterDialog::advance()
{
    m_pCurrentPage->Show( FALSE );
    if( m_pCurrentPage == m_pChooseDevicePage )
    {
        if( m_pChooseDevicePage->isPrinter() )
        {
            if( ! m_pChooseDriverPage )
                m_pChooseDriverPage = new APChooseDriverPage( this );
            m_pCurrentPage = m_pChooseDriverPage;
            m_aPrevPB.Enable( TRUE );
        }
        else if( m_pChooseDevicePage->isOld() )
        {
            if( ! m_pOldPrinterPage )
                m_pOldPrinterPage = new APOldPrinterPage( this );
            m_pCurrentPage = m_pOldPrinterPage;
            m_aPrevPB.Enable( TRUE );
            m_aFinishPB.Enable( TRUE );
            m_aNextPB.Enable( FALSE );
        }
        else if( m_pChooseDevicePage->isFax() )
        {
            if( ! m_pFaxDriverPage )
                m_pFaxDriverPage = new APFaxDriverPage( this );
            m_pCurrentPage = m_pFaxDriverPage;
            m_aPrevPB.Enable( TRUE );
        }
        else if( m_pChooseDevicePage->isPDF() )
        {
            if( ! m_pPdfDriverPage )
                m_pPdfDriverPage = new APPdfDriverPage( this );
            m_pCurrentPage = m_pPdfDriverPage;
            m_aPrevPB.Enable( TRUE );
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
        m_aFinishPB.Enable( TRUE );
        m_aNextPB.Enable( FALSE );
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
        m_aNextPB.Enable( FALSE );
        m_aFinishPB.Enable( TRUE );
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
        m_aNextPB.Enable( FALSE );
        m_aFinishPB.Enable( TRUE );
    }

    m_pCurrentPage->Show( TRUE );
    m_aTitleImage.SetText( m_pCurrentPage->getTitle() );
}

void AddPrinterDialog::back()
{
    m_pCurrentPage->Show( FALSE );
    if( m_pCurrentPage == m_pChooseDriverPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( FALSE );
    }
    else if( m_pCurrentPage == m_pNamePage )
    {
        m_pCurrentPage = m_pCommandPage;
        m_aNextPB.Enable( TRUE );
    }
    else if( m_pCurrentPage == m_pCommandPage )
    {
        m_pCurrentPage = m_pChooseDriverPage;
    }
    else if( m_pCurrentPage == m_pOldPrinterPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( FALSE );
        m_aNextPB.Enable( TRUE );
    }
    else if( m_pCurrentPage == m_pFaxDriverPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( FALSE );
    }
    else if( m_pCurrentPage == m_pFaxSelectDriverPage )
    {
        m_pCurrentPage = m_pFaxDriverPage;
    }
    else if( m_pCurrentPage == m_pFaxNamePage )
    {
        m_pCurrentPage = m_pFaxCommandPage;
        m_aNextPB.Enable( TRUE );
    }
    else if( m_pCurrentPage == m_pFaxCommandPage )
    {
        m_pCurrentPage = m_pFaxDriverPage->isDefault() ? (APTabPage*)m_pFaxDriverPage : (APTabPage*)m_pFaxSelectDriverPage;
        m_aNextPB.Enable( TRUE );
    }
    else if( m_pCurrentPage == m_pPdfDriverPage )
    {
        m_pCurrentPage = m_pChooseDevicePage;
        m_aPrevPB.Enable( FALSE );
    }
    else if( m_pCurrentPage == m_pPdfSelectDriverPage )
    {
        m_pCurrentPage = m_pPdfDriverPage;
    }
    else if( m_pCurrentPage == m_pPdfNamePage )
    {
        m_pCurrentPage = m_pPdfCommandPage;
        m_aNextPB.Enable( TRUE );
    }
    else if( m_pCurrentPage == m_pPdfCommandPage )
    {
        m_pCurrentPage = m_pPdfDriverPage->isDefault() || m_pPdfDriverPage->isDist() ? (APTabPage*)m_pPdfDriverPage : (APTabPage*)m_pPdfSelectDriverPage;
        m_aNextPB.Enable( TRUE );
    }
    m_pCurrentPage->Show( TRUE );
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
                aInfo.m_aFeatures = OUString::createFromAscii( "fax=" );
                if( m_pFaxNamePage->isFaxSwallow() )
                    aInfo.m_aFeatures += OUString::createFromAscii( "swallow" );
            }
            else if( m_pChooseDevicePage->isPDF() )
            {
                OUString aPdf( OUString::createFromAscii( "pdf=" ) );
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

    int nVersion = 1;
    bool bDoublet;
    list< OUString > aPrinterList;
    rManager.listPrinters( aPrinterList );
    hash_set< OUString, OUStringHash > aPrinters;
    for( list< OUString >::const_iterator it = aPrinterList.begin(); it != aPrinterList.end(); ++it )
        aPrinters.insert( *it );
    while( aPrinters.find( aResult ) != aPrinters.end() )
    {
        aResult = rBase;
        aResult.AppendAscii( "_" );
        aResult += String::CreateFromInt32( nVersion++ );
    }

    return aResult;
}

String AddPrinterDialog::getOldPrinterLocation()
{
    static const char* pHome = getenv( "HOME" );
    String aRet;
    ByteString aFileName;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    if( pHome )
    {
        aFileName = pHome;
        aFileName.Append( "/.Xpdefaults" );
        if( access( aFileName.GetBuffer(), F_OK ) )
        {
            aFileName = pHome;
            aFileName.Append( "/.sversionrc" );
            Config aSVer( String( aFileName, aEncoding ) );
            aSVer.SetGroup( "Versions" );
            aFileName = aSVer.ReadKey( "StarOffice 5.2" );
            if( aFileName.Len() )
                aFileName.Append( "/share/xp3/Xpdefaults" );
            else if(
                    (aFileName = aSVer.ReadKey( "StarOffice 5.1" ) ).Len()
                    ||
                    (aFileName = aSVer.ReadKey( "StarOffice 5.0" ) ).Len()
                    ||
                    (aFileName = aSVer.ReadKey( "StarOffice 4.0" ) ).Len()
                    )
            {
                aFileName.Append( "/xp3/Xpdefaults" );
            }
            if( aFileName.Len() && access( aFileName.GetBuffer(), F_OK ) )
                aFileName.Erase();
        }
    }
    if( aFileName.Len() )
        aRet = String( aFileName, aEncoding );
    return aRet;
}
