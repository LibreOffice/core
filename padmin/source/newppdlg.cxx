/*************************************************************************
 *
 *  $RCSfile: newppdlg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:21:55 $
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

#include <stdio.h>
#include <unistd.h>

#ifndef _PSPRINT_PPDPARSER_HXX_
#include <psprint/ppdparser.hxx>
#endif
#ifndef _PSPRINT_HELPER_HXX_
#include <psprint/helper.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif
#ifndef _PAD_PADIALOG_HRC_
#include <padialog.hrc>
#endif
#ifndef _PAD_NEWPPDLG_HXX_
#include <newppdlg.hxx>
#endif
#ifndef _PAD_PADIALOG_HXX_
#include <padialog.hxx>
#endif
#ifndef _PAD_PROGRESS_HXX_
#include <progress.hxx>
#endif

#define PPDIMPORT_GROUP "PPDImport"

using namespace padmin;
using namespace psp;
using namespace osl;
using namespace rtl;

PPDImportDialog::PPDImportDialog( Window* pParent ) :
        ModalDialog( pParent, PaResId( RID_PPDIMPORT_DLG ) ),
        m_aOKBtn( this, PaResId( RID_PPDIMP_BTN_OK ) ),
        m_aCancelBtn( this, PaResId( RID_PPDIMP_BTN_CANCEL ) ),
        m_aSearchBtn( this, PaResId( RID_PPDIMP_BTN_SEARCH ) ),
        m_aPathBox( this, PaResId( RID_PPDIMP_LB_PATH ) ),
        m_aDriverTxt( this, PaResId( RID_PPDIMP_TXT_DRIVER ) ),
        m_aDriverLB( this, PaResId( RID_PPDIMP_LB_DRIVER ) ),
        m_aPathGroup( this, PaResId( RID_PPDIMP_GROUP_PATH ) ),
        m_aDriverGroup( this, PaResId( RID_PPDIMP_GROUP_DRIVER ) ),
        m_aPathTxt( this, PaResId( RID_PPDIMP_TXT_PATH ) ),
        m_aLoadingPPD( PaResId( RID_PPDIMP_STR_LOADINGPPD ) )
{
    FreeResource();

    String aText( m_aDriverTxt.GetText() );
    aText.SearchAndReplaceAscii( "%s", Button::GetStandardText( BUTTON_OK ) );
    aText.EraseAllChars( '~' );
    m_aDriverTxt.SetText( aText );

    Config& rConfig = getPadminRC();
    rConfig.SetGroup( PPDIMPORT_GROUP );
    m_aPathBox.SetText( String( rConfig.ReadKey( "LastDir" ), RTL_TEXTENCODING_UTF8 ) );
    for( int i = 0; i < 11; i++ )
    {
        ByteString aEntry( rConfig.ReadKey( ByteString::CreateFromInt32( i ) ) );
        if( aEntry.Len() )
            m_aPathBox.InsertEntry( String( aEntry, RTL_TEXTENCODING_UTF8 ) );
    }

    m_aOKBtn.SetClickHdl( LINK( this, PPDImportDialog, ClickBtnHdl ) );
    m_aCancelBtn.SetClickHdl( LINK( this, PPDImportDialog, ClickBtnHdl ) );
    m_aSearchBtn.SetClickHdl( LINK( this, PPDImportDialog, ClickBtnHdl ) );
    m_aPathBox.SetSelectHdl( LINK( this, PPDImportDialog, SelectHdl ) );
    m_aPathBox.SetModifyHdl( LINK( this, PPDImportDialog, ModifyHdl ) );

    if( m_aPathBox.GetText().Len() )
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
    String aImportPath( m_aPathBox.GetText() );

    Config& rConfig = getPadminRC();
    rConfig.SetGroup( PPDIMPORT_GROUP );
    rConfig.WriteKey( "LastDir", ByteString( aImportPath, RTL_TEXTENCODING_UTF8 ) );

    int nEntries = m_aPathBox.GetEntryCount();
    while( nEntries-- )
        if( aImportPath == m_aPathBox.GetEntry( nEntries ) )
            break;
    if( nEntries < 0 )
    {
        int nNextEntry = rConfig.ReadKey( "NextEntry" ).ToInt32();
        rConfig.WriteKey( ByteString::CreateFromInt32( nNextEntry ), ByteString( aImportPath, RTL_TEXTENCODING_UTF8 ) );
        nNextEntry = nNextEntry < 10 ? nNextEntry+1 : 0;
        rConfig.WriteKey( "NextEntry", ByteString::CreateFromInt32( nNextEntry ) );
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
    FindFiles( aImportPath, aFiles, String::CreateFromAscii( "PS;PPD" ) );

    int i = 0;
    aProgress.setRange( 0, aFiles.size() );
    while( aFiles.size() )
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
                     ByteString( ByteString( aPath.PathToFileName(), osl_getThreadTextEncoding() ).GetBuffer() ) );
#endif
            continue;
        }

        USHORT nPos = m_aDriverLB.InsertEntry( aPrinterName );
        m_aDriverLB.SetEntryData( nPos, new String( aPath.PathToFileName() ) );
    }
}

IMPL_LINK( PPDImportDialog, ClickBtnHdl, PushButton*, pButton )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    if( pButton == &m_aCancelBtn )
    {
        EndDialog( 0 );
    }
    else if( pButton == &m_aOKBtn )
    {
        // copy the files
        ::std::list< String > aToDirs;
        String aPathList( ::psp::getPrinterPath() );
        int nTokens = aPathList.GetTokenCount( ':' );
        for( int n = 0; n < nTokens; n++ )
        {
            String aPath = aPathList.GetToken( n, ':' );
            aPath.AppendAscii( "/"PSPRINT_PPDDIR );
            aToDirs.push_back( aPath );
        }
        ::std::list< String >::iterator writeDir = aToDirs.begin();

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
                aToFile.setExtension( String::CreateFromAscii( "PPD" ) );
                OUString aToUni( aToFile.GetMainURL(INetURLObject::DECODE_TO_IURI) );
                if( ! File::copy( aFromUni, aToUni ) )
                    break;
                ++writeDir;
            } while( writeDir != aToDirs.end() );
        }
        EndDialog( 1 );
    }
    else if( pButton == &m_aSearchBtn )
    {
        String aPath( m_aPathBox.GetText() );
        if( chooseDirectory( this, aPath ) )
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
        ByteString aDir( m_aPathBox.GetText(), osl_getThreadTextEncoding() );
        if( ! access( aDir.GetBuffer(), F_OK ) )
            Import();
    }
    return 0;
}
