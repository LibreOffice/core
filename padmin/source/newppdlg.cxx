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

#include <stdio.h>
#include <unistd.h>

#include "helper.hxx"
#include "padialog.hrc"
#include "newppdlg.hxx"
#include "padialog.hxx"
#include "progress.hxx"

#include "vcl/ppdparser.hxx"
#include "vcl/helper.hxx"
#include "vcl/svapp.hxx"
#include "vcl/mnemonic.hxx"

#include "tools/config.hxx"
#include "tools/urlobj.hxx"

#include "osl/file.hxx"

#include <list>

#define PPDIMPORT_GROUP "PPDImport"

using namespace padmin;
using namespace psp;
using namespace osl;


PPDImportDialog::PPDImportDialog( Window* pParent ) :
        ModalDialog( pParent, PaResId( RID_PPDIMPORT_DLG ) ),
        m_aOKBtn( this, PaResId( RID_PPDIMP_BTN_OK ) ),
        m_aCancelBtn( this, PaResId( RID_PPDIMP_BTN_CANCEL ) ),
        m_aPathTxt( this, PaResId( RID_PPDIMP_TXT_PATH ) ),
        m_aPathBox( this, PaResId( RID_PPDIMP_LB_PATH ) ),
        m_aSearchBtn( this, PaResId( RID_PPDIMP_BTN_SEARCH ) ),
        m_aDriverTxt( this, PaResId( RID_PPDIMP_TXT_DRIVER ) ),
        m_aDriverLB( this, PaResId( RID_PPDIMP_LB_DRIVER ) ),
        m_aPathGroup( this, PaResId( RID_PPDIMP_GROUP_PATH ) ),
        m_aDriverGroup( this, PaResId( RID_PPDIMP_GROUP_DRIVER ) ),
        m_aLoadingPPD( PaResId( RID_PPDIMP_STR_LOADINGPPD ) )
{
    FreeResource();

    String aText( m_aDriverTxt.GetText() );
    aText.SearchAndReplaceAscii( "%s", Button::GetStandardText( BUTTON_OK ) );
    m_aDriverTxt.SetText( MnemonicGenerator::EraseAllMnemonicChars( aText ) );

    Config& rConfig = getPadminRC();
    rConfig.SetGroup( PPDIMPORT_GROUP );
    m_aPathBox.SetText( OStringToOUString(rConfig.ReadKey("LastDir"), RTL_TEXTENCODING_UTF8) );
    for (sal_Int32 i = 0; i < 11; ++i)
    {
        OString aEntry(rConfig.ReadKey(OString::number(i)));
        if (!aEntry.isEmpty())
            m_aPathBox.InsertEntry(OStringToOUString(aEntry, RTL_TEXTENCODING_UTF8));
    }

    m_aOKBtn.SetClickHdl( LINK( this, PPDImportDialog, ClickBtnHdl ) );
    m_aCancelBtn.SetClickHdl( LINK( this, PPDImportDialog, ClickBtnHdl ) );
    m_aSearchBtn.SetClickHdl( LINK( this, PPDImportDialog, ClickBtnHdl ) );
    m_aPathBox.SetSelectHdl( LINK( this, PPDImportDialog, SelectHdl ) );
    m_aPathBox.SetModifyHdl( LINK( this, PPDImportDialog, ModifyHdl ) );

    if( !m_aPathBox.GetText().isEmpty() )
        Import();
}

PPDImportDialog::~PPDImportDialog()
{
    while( m_aDriverLB.GetEntryCount() )
    {
        delete (String*)m_aDriverLB.GetEntryData( 0 );
        m_aDriverLB.RemoveEntry( 0 );
    }
}

void PPDImportDialog::Import()
{
    OUString aImportPath( m_aPathBox.GetText() );

    Config& rConfig = getPadminRC();
    rConfig.SetGroup( PPDIMPORT_GROUP );
    rConfig.WriteKey( "LastDir", OUStringToOString(aImportPath, RTL_TEXTENCODING_UTF8) );

    int nEntries = m_aPathBox.GetEntryCount();
    while( nEntries-- )
        if( aImportPath == m_aPathBox.GetEntry( nEntries ) )
            break;
    if( nEntries < 0 )
    {
        sal_Int32 nNextEntry = rConfig.ReadKey("NextEntry").toInt32();
        rConfig.WriteKey( OString::number(nNextEntry), OUStringToOString(aImportPath, RTL_TEXTENCODING_UTF8) );
        nNextEntry = nNextEntry < 10 ? nNextEntry+1 : 0;
        rConfig.WriteKey( "NextEntry", OString::number(nNextEntry) );
        m_aPathBox.InsertEntry( aImportPath );
    }
    while( m_aDriverLB.GetEntryCount() )
    {
        delete (String*)m_aDriverLB.GetEntryData( 0 );
        m_aDriverLB.RemoveEntry( 0 );
    }

    ProgressDialog aProgress( Application::GetFocusWindow() );
    aProgress.startOperation( m_aLoadingPPD );

    ::std::list< String > aFiles;
    FindFiles( aImportPath, aFiles, OUString( "PS;PPD;PS.GZ;PPD.GZ" ), true );

    int i = 0;
    aProgress.setRange( 0, aFiles.size() );
    while( !aFiles.empty() )
    {
        aProgress.setValue( ++i );
        aProgress.setFilename( aFiles.front() );
        INetURLObject aPath( aImportPath, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        aPath.Append( aFiles.front() );
        String aPrinterName = PPDParser::getPPDPrinterName( aPath.PathToFileName() );
        aFiles.pop_front();

        if( ! aPrinterName.Len() )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Warning: File %s has empty printer name.\n",
                     OUStringToOString( aPath.PathToFileName(), osl_getThreadTextEncoding() ).getStr() );
#endif
            continue;
        }

        sal_uInt16 nPos = m_aDriverLB.InsertEntry( aPrinterName );
        m_aDriverLB.SetEntryData( nPos, new String( aPath.PathToFileName() ) );
    }
}

IMPL_LINK( PPDImportDialog, ClickBtnHdl, PushButton*, pButton )
{
    if( pButton == &m_aCancelBtn )
    {
        EndDialog( 0 );
    }
    else if( pButton == &m_aOKBtn )
    {
        // copy the files
        ::std::list< OUString > aToDirs;
        psp::getPrinterPathList( aToDirs, PRINTER_PPDDIR );
        ::std::list< OUString >::iterator writeDir = aToDirs.begin();
        m_aImportedFiles.clear();

        for( int i = 0; i < m_aDriverLB.GetSelectEntryCount(); i++ )
        {
            INetURLObject aFile( *(String*)m_aDriverLB.GetEntryData(
                m_aDriverLB.GetSelectEntryPos( i )
                ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
            OUString aFromUni( aFile.GetMainURL(INetURLObject::DECODE_TO_IURI) );

            do
            {
                INetURLObject aToFile( *writeDir, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
                aToFile.Append( aFile.GetName() );
                OUString aToUni( aToFile.GetMainURL(INetURLObject::DECODE_TO_IURI) );
                if( ! File::copy( aFromUni, aToUni ) )
                {
                    m_aImportedFiles.push_back( aToUni );
                    break;
                }
                ++writeDir;
            } while( writeDir != aToDirs.end() );
        }
        EndDialog( 1 );
    }
    else if( pButton == &m_aSearchBtn )
    {
        OUString aPath( m_aPathBox.GetText() );
        if( chooseDirectory( aPath ) )
        {
            m_aPathBox.SetText( aPath );
            Import();
        }
    }
    return 0;
}

IMPL_LINK( PPDImportDialog, SelectHdl, ComboBox*, pListBox )
{
    if( pListBox == &m_aPathBox )
    {
        Import();
    }
    return 0;
}

IMPL_LINK( PPDImportDialog, ModifyHdl, ComboBox*, pListBox )
{
    if( pListBox == &m_aPathBox )
    {
        OString aDir(OUStringToOString(m_aPathBox.GetText(), osl_getThreadTextEncoding()));
        if (!access( aDir.getStr(), F_OK))
            Import();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
