/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <stdio.h>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#ifndef _PAD_RTSETUP_HRC_
#include <rtsetup.hrc>
#endif
#include <cmddlg.hxx>
#include <padialog.hxx>
#include <helper.hxx>
#include <prtsetup.hxx>

using namespace psp;
using namespace rtl;
using namespace padmin;

#define PRINTER_PERSISTENCE_GROUP "KnownPrinterCommands"
#define FAX_PERSISTENCE_GROUP "KnownFaxCommands"
#define PDF_PERSISTENCE_GROUP "KnowPdfCommands"
#define MAX_COMMANDS 50

void CommandStore::getSystemPrintCommands( ::std::list< String >& rCommands )
{
    static ::std::list< OUString > aSysCommands;
    static bool bOnce = false;
    if( ! bOnce )
    {
        bOnce = true;
        PrinterInfoManager::get().getSystemPrintCommands( aSysCommands );
    }

    ::std::list< OUString >::const_iterator it;
    for( it = aSysCommands.begin(); it != aSysCommands.end(); ++it )
        rCommands.push_back( *it );
}

void CommandStore::getSystemPdfCommands( ::std::list< String >& rCommands )
{
    static bool bOnce = false;
    static ::std::list< String > aSysCommands;

    if( ! bOnce )
    {
        bOnce = true;
        char pBuffer[1024];
        FILE* pPipe;
        String aCommand;
        rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

        pPipe = popen( "which gs 2>/dev/null", "r" );
        if( pPipe )
        {
            if (fgets( pBuffer, sizeof( pBuffer ), pPipe ) != NULL)
            {
                int nLen = strlen( pBuffer );
                if( pBuffer[nLen-1] == '\n' ) // strip newline
                    pBuffer[--nLen] = 0;
                aCommand = String( ByteString( pBuffer ), aEncoding );
                if( ( ( aCommand.GetChar( 0 ) == '/' )
                      || ( aCommand.GetChar( 0 ) == '.' && aCommand.GetChar( 1 ) == '/' )
                      || ( aCommand.GetChar( 0 ) == '.' && aCommand.GetChar( 1 ) == '.' && aCommand.GetChar( 2 ) == '/' ) )
                    && nLen > 2
                    && aCommand.GetChar( nLen-2 ) == 'g'
                    && aCommand.GetChar( nLen-1 ) == 's' )
                {
                    aCommand.AppendAscii( " -q -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=\"(OUTFILE)\" -" );
                    aSysCommands.push_back( aCommand );
                }
            }
            pclose( pPipe );
        }

        pPipe = popen( "which distill 2>/dev/null", "r" );
        if( pPipe )
        {
            if (fgets( pBuffer, sizeof( pBuffer ), pPipe ) != NULL)
            {
                int nLen = strlen( pBuffer );
                if( pBuffer[nLen-1] == '\n' ) // strip newline
                    pBuffer[--nLen] = 0;
                aCommand = String( ByteString( pBuffer ), aEncoding );
                if( ( ( aCommand.GetChar( 0 ) == '/' )
                      || ( aCommand.GetChar( 0 ) == '.' && aCommand.GetChar( 1 ) == '/' )
                      || ( aCommand.GetChar( 0 ) == '.' && aCommand.GetChar( 1 ) == '.' && aCommand.GetChar( 2 ) == '/' ) )
                    && nLen > 7
                    && aCommand.Copy( nLen - 8 ).EqualsAscii( "/distill" ) )
                {
                    aCommand.AppendAscii( " (TMP) ; mv `echo (TMP) | sed s/\\.ps\\$/.pdf/` \"(OUTFILE)\"" );
                    aSysCommands.push_back( aCommand );
                }
            }
            pclose( pPipe );
        }
    }
    ::std::list< String >::const_iterator it;
    for( it = aSysCommands.begin(); it != aSysCommands.end(); ++it )
        rCommands.push_back( *it );
}



void CommandStore::getStoredCommands( const char* pGroup, ::std::list< String >& rCommands )
{
    Config& rConfig( getPadminRC() );
    rConfig.SetGroup( pGroup );
    int nKeys = rConfig.GetKeyCount();
    ::std::list< String >::const_iterator it;
    while( nKeys-- )
    {
        String aCommand( rConfig.ReadKey( ByteString::CreateFromInt32( nKeys ), RTL_TEXTENCODING_UTF8 ) );
        if( aCommand.Len() )
        {
            for( it = rCommands.begin(); it != rCommands.end() && *it != aCommand; ++it )
                ;
            if( it == rCommands.end() )
                rCommands.push_back( aCommand );
        }
    }
}

void CommandStore::setCommands(
                               const char* pGroup,
                               const ::std::list< String >& rCommands,
                               const ::std::list< String >& rSysCommands
                               )
{
    Config& rConfig( getPadminRC() );
    rConfig.DeleteGroup( pGroup );
    rConfig.SetGroup( pGroup );
    ::std::list< String >::const_iterator it, loop;
    ::std::list< String > aWriteList;

    int nWritten = 0;
    for( it = rCommands.begin(); it != rCommands.end(); ++it )
    {
        if( it->Len() )
        {
            for( loop = rSysCommands.begin(); loop != rSysCommands.end() && *loop != *it; ++loop )
                ;
            if( loop == rSysCommands.end() )
            {
                aWriteList.push_back( *it );
                nWritten++;
            }
        }
    }
    while( nWritten > MAX_COMMANDS )
    {
        aWriteList.pop_front();
        nWritten--;
    }
    for( nWritten = 0, it = aWriteList.begin(); it != aWriteList.end(); ++it, ++nWritten )
        rConfig.WriteKey( ByteString::CreateFromInt32( nWritten ), ByteString( *it, RTL_TEXTENCODING_UTF8 ) );
}


void CommandStore::getPrintCommands( ::std::list< String >& rCommands )
{
    rCommands.clear();
    getSystemPrintCommands( rCommands );
    getStoredCommands( PRINTER_PERSISTENCE_GROUP, rCommands );
}

void CommandStore::getPdfCommands( ::std::list< String >& rCommands )
{
    rCommands.clear();
    getSystemPdfCommands( rCommands );
    getStoredCommands( PDF_PERSISTENCE_GROUP, rCommands );
}

void CommandStore::getFaxCommands( ::std::list< String >& rCommands )
{
    rCommands.clear();
    getStoredCommands( FAX_PERSISTENCE_GROUP, rCommands );
}

void CommandStore::setPrintCommands( const ::std::list< String >& rCommands )
{
    ::std::list< String > aSysCmds;
    getSystemPrintCommands( aSysCmds );
    setCommands( PRINTER_PERSISTENCE_GROUP, rCommands, aSysCmds );
}

void CommandStore::setPdfCommands( const ::std::list< String >& rCommands )
{
    ::std::list< String > aSysCmds;
    getSystemPdfCommands( aSysCmds );
    setCommands( PDF_PERSISTENCE_GROUP, rCommands, aSysCmds );
}

void CommandStore::setFaxCommands( const ::std::list< String >& rCommands )
{
    ::std::list< String > aSysCmds;
    setCommands( FAX_PERSISTENCE_GROUP, rCommands, aSysCmds );
}


RTSCommandPage::RTSCommandPage( RTSDialog* pParent ) :
        TabPage( &pParent->m_aTabControl, PaResId( RID_RTS_COMMANDPAGE ) ),
        m_pParent( pParent ),
        m_aCommandsCB( this, PaResId( RID_RTS_CMD_CB_COMMANDS ) ),
        m_aExternalCB( this, PaResId( RID_RTS_CMD_CB_EXTERNAL ) ),
        m_aQuickFT( this, PaResId( RID_RTS_CMD_FT_QUICKCMD ) ),
        m_aQuickCB( this, PaResId( RIT_RTS_CMD_CB_QUICKCMD ) ),
        m_aCommandTitle( this, PaResId( RID_RTS_CMD_FL_INSTALL ) ),
        m_aPrinterName( this, PaResId( RID_RTS_CMD_TXT_PRTNAME ) ),
        m_aConnectedTo( this, PaResId( RID_RTS_CMD_TXT_CONNECT ) ),
        m_aPrinterFL( this, PaResId( RID_RTS_CMD_FL_DEFAULT ) ),
        m_aConfigureText( this, PaResId( RID_RTS_CMD_TXT_CONFIGURE ) ),
        m_aConfigureBox( this, PaResId( RID_RTS_CMD_LB_CONFIGURE ) ),
        m_aPdfDirectoryText( this, PaResId( RID_RTS_CMD_TXT_PDFDIR ) ),
        m_aPdfDirectoryButton( this, PaResId( RID_RTS_CMD_BTN_PDFDIR ) ),
        m_aPdfDirectoryEdit( this, PaResId( RID_RTS_CMD_EDT_PDFDIR ) ),
        m_aFaxSwallowBox( this, PaResId( RID_RTS_CMD_BOX_SWALLOWFAXNO ) ),
        m_aHelpButton( this, PaResId( RID_RTS_CMD_BTN_HELP ) ),
        m_aRemovePB( this, PaResId( RID_RTS_CMD_BTN_REMOVE ) ),
        m_aFaxHelp( PaResId( RID_RTS_CMD_STR_FAXHELP ) ),
        m_aPrinterHelp( PaResId( RID_RTS_CMD_STR_PRINTERHELP ) ),
        m_aPdfHelp( PaResId( RID_RTS_CMD_STR_PDFHELP ) )
{
    // configuring as printer is only sensible in default print system
    PrinterInfoManager& rMgr( PrinterInfoManager::get() );
    if( rMgr.getType() == PrinterInfoManager::Default || rMgr.isCUPSDisabled() )
        m_nPrinterEntry = m_aConfigureBox.InsertEntry( String( PaResId( RID_RTS_CMD_STR_CONFIGURE_PRINTER ) ) );
    else
        m_nPrinterEntry = ~0;
    m_nFaxEntry = m_aConfigureBox.InsertEntry( String( PaResId( RID_RTS_CMD_STR_CONFIGURE_FAX ) ) );
    m_nPdfEntry = m_aConfigureBox.InsertEntry( String( PaResId( RID_RTS_CMD_STR_CONFIGURE_PDF ) ) );

    FreeResource();

    CommandStore::getPrintCommands( m_aPrinterCommands );
    CommandStore::getFaxCommands( m_aFaxCommands );
    CommandStore::getPdfCommands( m_aPdfCommands );

    m_aPrinterName.SetText( m_pParent->m_aPrinter );

    m_aCommandsCB.SetDoubleClickHdl( LINK( this, RTSCommandPage, DoubleClickHdl ) );
    m_aCommandsCB.SetSelectHdl( LINK( this, RTSCommandPage, SelectHdl ) );
    m_aCommandsCB.SetModifyHdl( LINK( this, RTSCommandPage, ModifyHdl ) );
    m_aConfigureBox.SetSelectHdl( LINK( this, RTSCommandPage, SelectHdl ) );
    m_aHelpButton.SetClickHdl( LINK( this, RTSCommandPage, ClickBtnHdl ) );
    m_aRemovePB.SetClickHdl( LINK( this, RTSCommandPage, ClickBtnHdl ) );
    m_aPdfDirectoryButton.SetClickHdl( LINK( this, RTSCommandPage, ClickBtnHdl ) );
    m_aExternalCB.SetToggleHdl( LINK( this, RTSCommandPage, ClickBtnHdl ) );

    m_aPdfDirectoryButton.Show( sal_False );
    m_aPdfDirectoryEdit.Show( sal_False );
    m_aPdfDirectoryText.Show( sal_False );
    m_aFaxSwallowBox.Show( sal_False );
    m_aCommandsCB.SetText( m_pParent->m_aJobData.m_aCommand );
    m_aQuickCB.SetText( m_pParent->m_aJobData.m_aQuickCommand );

    m_bWasFax = false;
    m_bWasPdf = false;
    m_aConfigureBox.SelectEntryPos( m_nPrinterEntry );
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aToken( m_pParent->m_aJobData.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( ! aToken.compareToAscii( "fax", 3 ) )
        {
            m_bWasFax = true;
            m_aFaxSwallowBox.Show( sal_True );
            sal_Int32 nPos = 0;
            m_aFaxSwallowBox.Check( ! aToken.getToken( 1, '=', nPos ).compareToAscii( "swallow", 7 ) ? sal_True : sal_False );
            m_aConfigureBox.SelectEntryPos( m_nFaxEntry );
        }
        else if( ! aToken.compareToAscii( "pdf=", 4 ) )
        {
            m_bWasPdf = true;
            sal_Int32 nPos = 0;
            m_aPdfDirectoryEdit.SetText( aToken.getToken( 1, '=', nPos ) );
            m_aPdfDirectoryEdit.Show( sal_True );
            m_aPdfDirectoryButton.Show( sal_True );
            m_aPdfDirectoryText.Show( sal_True );
            m_aConfigureBox.SelectEntryPos( m_nPdfEntry );
        }
        else if( ! aToken.compareToAscii( "external_dialog" ) )
        {
            m_aExternalCB.Check();
            m_bWasExternalDialog = true;
        }
    }

    m_aQuickCB.Enable( m_aExternalCB.IsChecked() );

    String aString( m_aConnectedTo.GetText() );
    aString += String( m_pParent->m_aJobData.m_aCommand );
    m_aConnectedTo.SetText( aString );

    UpdateCommands();
}

RTSCommandPage::~RTSCommandPage()
{
}

void RTSCommandPage::save()
{
    String aCommand,aQuickCommand;
    bool bHaveFax = m_aConfigureBox.GetSelectEntryPos() == m_nFaxEntry ? true : false;
    bool bHavePdf = m_aConfigureBox.GetSelectEntryPos() == m_nPdfEntry ? true : false;
    ::std::list< String >::iterator it;

    String aFeatures;
    sal_Int32 nIndex = 0;
    String aOldPdfPath;
    bool bOldFaxSwallow = false;
    bool bFaxSwallow = m_aFaxSwallowBox.IsChecked() ? true : false;
    bool bOldExternalDialog = false, bExternalDialog = m_aExternalCB.IsChecked() ? true : false;

    while( nIndex != -1 )
    {
        OUString aToken( m_pParent->m_aJobData.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( aToken.compareToAscii( "fax", 3 ) &&
            aToken.compareToAscii( "pdf", 3 ) &&
            aToken.compareToAscii( "external_dialog" )
          )
        {
            if( aToken.getLength() )
            {
                if( aFeatures.Len() )
                    aFeatures += ',';
                aFeatures += String( aToken );
            }
        }
        else if( ! aToken.compareToAscii( "pdf=", 4 ) )
        {
            sal_Int32 nPos = 0;
            aOldPdfPath = aToken.getToken( 1, '=', nPos );
        }
        else if( ! aToken.compareToAscii( "fax=", 4 ) )
        {
            sal_Int32 nPos = 0;
            bOldFaxSwallow = aToken.getToken( 1, '=', nPos ).compareToAscii( "swallow", 7 ) ? false : true;
        }
        else if( ! aToken.compareToAscii( "external_dialog" ) )
        {
            bOldExternalDialog = true;
        }
    }
    ::std::list< String >* pList = &m_aPrinterCommands;
    if( bExternalDialog )
    {
        if( aFeatures.Len() )
            aFeatures += ',';
        aFeatures.AppendAscii( "external_dialog" );
    }
    if( bHaveFax )
    {
        if( aFeatures.Len() )
            aFeatures += ',';
        aFeatures.AppendAscii( "fax=" );
        if( bFaxSwallow )
            aFeatures.AppendAscii( "swallow" );
        pList = &m_aFaxCommands;
    }
    if( bHavePdf )
    {
        if( aFeatures.Len() )
            aFeatures += ',';
        aFeatures.AppendAscii( "pdf=" );
        aFeatures.Append( m_aPdfDirectoryEdit.GetText() );
        pList = &m_aPdfCommands;
    }
    aCommand = m_aCommandsCB.GetText();
    aQuickCommand = m_aQuickCB.GetText();
    for( it = pList->begin(); it != pList->end() && *it != aCommand; ++it )
        ;
    if( it == pList->end() )
        pList->push_back( aCommand );

    if( aCommand != String( m_pParent->m_aJobData.m_aCommand )              ||
        aQuickCommand != String( m_pParent->m_aJobData.m_aQuickCommand )    ||
        ( m_bWasFax && ! bHaveFax )                                         ||
        ( ! m_bWasFax && bHaveFax )                                         ||
        ( m_bWasPdf && ! bHavePdf )                                         ||
        ( ! m_bWasPdf && bHavePdf )                                         ||
        ( bHavePdf && aOldPdfPath != m_aPdfDirectoryEdit.GetText() )        ||
        ( bHaveFax && bFaxSwallow != bOldFaxSwallow )                       ||
        ( m_bWasExternalDialog && ! bExternalDialog )                       ||
        ( ! m_bWasExternalDialog && bExternalDialog )
        )
    {
        m_pParent->m_aJobData.m_aCommand        = aCommand;
        m_pParent->m_aJobData.m_aQuickCommand   = aQuickCommand;
        m_pParent->m_aJobData.m_aFeatures       = aFeatures;

        PrinterInfoManager::get().changePrinterInfo( m_pParent->m_aPrinter, m_pParent->m_aJobData );
    }
    CommandStore::setPrintCommands( m_aPrinterCommands );
    CommandStore::setFaxCommands( m_aFaxCommands );
    CommandStore::setPdfCommands( m_aPdfCommands );
}


IMPL_LINK( RTSCommandPage, SelectHdl, Control*, pBox )
{
    if( pBox == &m_aConfigureBox )
    {
        sal_Bool bEnable = m_aConfigureBox.GetSelectEntryPos() == m_nPdfEntry ? sal_True : sal_False;
        m_aPdfDirectoryButton.Show( bEnable );
        m_aPdfDirectoryEdit.Show( bEnable );
        m_aPdfDirectoryText.Show( bEnable );
        bEnable = m_aConfigureBox.GetSelectEntryPos() == m_nFaxEntry ? sal_True : sal_False;
        m_aFaxSwallowBox.Show( bEnable );
        UpdateCommands();
    }
    else if( pBox == &m_aCommandsCB )
    {
        m_aRemovePB.Enable( sal_True );
    }

    return 0;
}

IMPL_LINK( RTSCommandPage, ClickBtnHdl, Button*, pButton )
{
    if( pButton == & m_aPdfDirectoryButton )
    {
        String aPath( m_aPdfDirectoryEdit.GetText() );
        if( chooseDirectory( aPath ) )
            m_aPdfDirectoryEdit.SetText( aPath );
    }
    else if( pButton == &m_aRemovePB )
    {
        String aEntry( m_aCommandsCB.GetText() );
        ::std::list< String >* pList;
        if( m_aConfigureBox.GetSelectEntryPos() == m_nPrinterEntry )
            pList = &m_aPrinterCommands;
        else if( m_aConfigureBox.GetSelectEntryPos() == m_nFaxEntry )
            pList = &m_aFaxCommands;
        else
            pList = &m_aPdfCommands;

        pList->remove( aEntry );
        m_aCommandsCB.RemoveEntry( aEntry );
        m_aQuickCB.RemoveEntry( aEntry );
    }
    else if( pButton == &m_aHelpButton )
    {
        String aHelpText;
        if( m_aConfigureBox.GetSelectEntryPos() == m_nPrinterEntry )
            aHelpText = m_aPrinterHelp;
        else if( m_aConfigureBox.GetSelectEntryPos() == m_nFaxEntry )
            aHelpText = m_aFaxHelp;
        else if( m_aConfigureBox.GetSelectEntryPos() == m_nPdfEntry )
            aHelpText = m_aPdfHelp;

        InfoBox aBox( this, aHelpText );
        aBox.Execute();
    }
    else if( pButton == &m_aExternalCB )
    {
        m_aQuickCB.Enable( m_aExternalCB.IsChecked() );
    }
    return 0;
}

IMPL_LINK( RTSCommandPage, DoubleClickHdl, ComboBox*, pComboBox )
{
    if( pComboBox == &m_aCommandsCB )
        ConnectCommand();
    return 0;
}

IMPL_LINK( RTSCommandPage, ModifyHdl, Edit*, pEdit )
{
    if( pEdit == &m_aCommandsCB )
        m_aRemovePB.Enable( m_aCommandsCB.GetEntryPos( m_aCommandsCB.GetText() ) != LISTBOX_ENTRY_NOTFOUND );

    return 0;
}

void RTSCommandPage::UpdateCommands()
{
    m_aCommandsCB.Clear();
    ::std::list< String >::iterator it;
    if( m_aConfigureBox.GetSelectEntryPos() == m_nPrinterEntry )
    {
        for( it = m_aPrinterCommands.begin(); it != m_aPrinterCommands.end(); ++it )
        {
            m_aCommandsCB.InsertEntry( *it );
            m_aQuickCB.InsertEntry( *it );
        }
        if( ! m_bWasFax )
            m_aCommandsCB.SetText( m_pParent->m_aJobData.m_aCommand );
        else
            m_aCommandsCB.SetText( String() );
    }
    else if( m_aConfigureBox.GetSelectEntryPos() == m_nFaxEntry )
    {
        for( it = m_aFaxCommands.begin(); it != m_aFaxCommands.end(); ++it )
        {
            m_aCommandsCB.InsertEntry( *it );
            m_aQuickCB.InsertEntry( *it );
        }
        if( m_bWasFax )
            m_aCommandsCB.SetText( m_pParent->m_aJobData.m_aCommand );
        else
            m_aCommandsCB.SetText( String() );
    }
    else if( m_aConfigureBox.GetSelectEntryPos() == m_nPdfEntry )
    {
        for( it = m_aPdfCommands.begin(); it != m_aPdfCommands.end(); ++it )
        {
            m_aCommandsCB.InsertEntry( *it );
            m_aQuickCB.InsertEntry( *it );
        }
        if( m_bWasPdf )
            m_aCommandsCB.SetText( m_pParent->m_aJobData.m_aCommand );
        else
            m_aCommandsCB.SetText( String() );
    }
}

void RTSCommandPage::ConnectCommand()
{
    String aString( m_aConnectedTo.GetText().GetToken( 0, ':' ) );
    aString.AppendAscii( ": " );
    aString += m_aCommandsCB.GetText();

    m_aConnectedTo.SetText( aString );
}
