/*************************************************************************
 *
 *  $RCSfile: fontentry.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pl $ $Date: 2001-06-05 17:33:50 $
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

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif

#ifndef _PAD_PADIALOG_HRC_
#include <padialog.hrc>
#endif
#ifndef _PAD_FONTENTRY_HXX_
#include <fontentry.hxx>
#endif
#ifndef _SVT_FILEDLG_HXX
#include <filedlg.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif

#if 0
#include <convafm.hxx>
#include <progress.hxx>
#include <osl/file.hxx>
#endif

using namespace padmin;
using namespace osl;
using namespace rtl;
using namespace psp;

#if 0
static void CreateAfmFile( const INetURLObject& rFontFile )
{
    rtl_TextEncoding aEncoding = gsl_getSystemTextEncoding();

    INetURLObject aFontMap( rFontFile.GetPath(), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
    aFontMap.Append( String( RTL_CONSTASCII_USTRINGPARAM( "Fontmap" ) ) );
    INetURLObject aAfmFile( rFontFile );
    aAfmFile.setExtension( String::CreateFromAscii( "afm", 3 ) );


    SvFileStream aMap( aFontMap.PathToFileName(), STREAM_WRITE | STREAM_TRUNC );
    if( aMap.IsOpen() )
    {
        SvFileStream aRead( rFontFile.GetFull(), STREAM_READ );
        ByteString aLine;
        ByteString aFullName;
        int nPos;
        while( ! aRead.IsEof() &&
               ( nPos = aLine.Search( "/FullName" ) ) == STRING_NOTFOUND )
            aRead.ReadLine( aLine );
        aRead.Close();
        if( nPos != STRING_NOTFOUND )
        {
            aLine.Erase( 0, nPos );
            aFullName = aLine.GetToken( 1, '(' ).GetToken( 0, ')' );

            aLine = '/';
            aLine += aFullName;
            aLine += ' ';
            aLine += '(';
            aLine += ByteString( rFontFile.GetName(), aEncoding );
            aLine += ')';
            aLine += ';';

            aMap.WriteLine( aLine );
        }
        aMap.Close();
        pid_t nPID = fork();
        if( ! nPID )
        {
            INetURLObject aTmp( rFontFile );
            aTmp.CutName();
            ByteString aEnv( "GS_LIB=" );
            aEnv += ByteString( aTmp.PathToFileName(), aEncoding );
            putenv( const_cast<char*>(aEnv.GetBuffer()) );

            int nDescr = open( ByteString( aAfmFile.PathToFileName(), aEncoding ).GetBuffer(),
                               O_CREAT | O_TRUNC| O_WRONLY,
                               00755 );
#ifdef DEBUG
            if( nDescr < 0 )
                fprintf( stderr, "open( %s ) failed because of %d\n", ByteString( aAfmFile.GetFull(), aEncoding ).GetBuffer(), errno );
#endif
            if( dup2( nDescr, STDOUT_FILENO ) > 0 )
            {
                execlp( "gs",
                        "-q",
                        "-dNODISPLAY",
                        "--", "printafm.ps",
                        aFullName.GetBuffer(),
                        NULL );
            }
#ifdef DEBUG
            else
                fprintf( stderr, "dup2( %d, %d ) failed because of %d\n", nDescr, STDOUT_FILENO, errno );
#endif
            _exit( 0 );
        }
        if( nPID > 0 )
            waitpid( nPID, NULL, 0 );
        MetricConverter::ConvertAFM( aAfmFile );
    }

    struct stat aStat;
    ByteString aSysFile( aAfmFile.PathToFileName(), aEncoding );
    if( stat( aSysFile.GetBuffer(), &aStat ) || ! aStat.st_size )
        unlink( aSysFile.GetBuffer() );
#ifdef DEBUG
    fprintf( stderr, " %s\n", ! access( aSysFile.GetBuffer(), F_OK ) ? "success" : "failed" );
#endif
    unlink( ByteString( aFontMap.PathToFileName(), aEncoding ).GetBuffer() );
}

#endif

FontNameDlg::FontNameDlg( Window *pParent ) :
        ModalDialog( pParent, PaResId( RID_FONTNAMEDIALOG ) ),
        m_aFixedText( this, PaResId( RID_FNTNM_FIXED ) ),
        m_aOKButton( this, PaResId( RID_FNTNM_BTN_OK ) ),
        m_aRemoveButton( this, PaResId( RID_FNTNM_BTN_REMOVE ) ),
        m_aFontBox( this, PaResId( RID_FNTNM_LB_FONTS ) ),
        m_aImportButton( this, PaResId( RID_FNTNM_BTN_IMPORT ) ),
        m_rFontManager( PrintFontManager::get() )
{
    FreeResource();

    m_aFontBox.EnableMultiSelection( TRUE );

    m_aOKButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aRemoveButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aImportButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aFontBox.setDelPressedLink( LINK( this, FontNameDlg, DelPressedHdl ) );

    init();
}

FontNameDlg::~FontNameDlg()
{
}

String FontNameDlg::fillFontEntry( FastPrintFontInfo& rInfo, const String& rFile )
{
    String aEntry( rInfo.m_aFamilyName );
    switch( rInfo.m_eWeight )
    {
        case weight::Thin:          aEntry.AppendAscii( ", Thin" );break;
        case weight::UltraLight:    aEntry.AppendAscii( ", Ultralight" );break;
        case weight::Light:         aEntry.AppendAscii( ", Light" );break;
        case weight::SemiLight:     aEntry.AppendAscii( ", Semilight" );break;
        case weight::SemiBold:      aEntry.AppendAscii( ", Semibold" );break;
        case weight::Bold:          aEntry.AppendAscii( ", Bold" );break;
        case weight::UltraBold:     aEntry.AppendAscii( ", Ultrabold" );break;
    }
    switch( rInfo.m_eItalic )
    {
        case italic::Oblique:       aEntry.AppendAscii( ", Oblique" );break;
        case italic::Italic:        aEntry.AppendAscii( ", Italic" );break;
    }
    switch( rInfo.m_eWidth )
    {
        case width::UltraCondensed: aEntry.AppendAscii( ", Ultracondensed" );break;
        case width::ExtraCondensed: aEntry.AppendAscii( ", Extracondensed" );break;
        case width::Condensed:      aEntry.AppendAscii( ", Condensed" );break;
        case width::SemiCondensed:  aEntry.AppendAscii( ", Semicondensed" );break;
        case width::SemiExpanded:   aEntry.AppendAscii( ", Semiexpanded" );break;
        case width::Expanded:       aEntry.AppendAscii( ", Expanded" );break;
        case width::ExtraExpanded:  aEntry.AppendAscii( ", Extraexpanded" );break;
        case width::UltraExpanded:  aEntry.AppendAscii( ", Ultraexpanded" );break;
    }

    aEntry.AppendAscii( " (" );
    aEntry.Append( rFile );
    aEntry.AppendAscii( ")" );
    return aEntry;
}

void FontNameDlg::init()
{
    ::std::list< fontID > aFonts;
    m_rFontManager.getFontList( aFonts );
    m_aFontBox.Clear();
    for( ::std::list< fontID >::iterator font_it = aFonts.begin(); font_it != aFonts.end(); ++font_it )
    {
        if( m_rFontManager.isPrivateFontFile( *font_it ) )
        {
            OString aFile( m_rFontManager.getFontFileSysPath( *font_it ) );
            int nLast = aFile.lastIndexOf( '/' );
            if( nLast != -1 )
                aFile = aFile.copy( nLast+1 );

            FastPrintFontInfo aInfo;
            m_rFontManager.getFontFastInfo( *font_it, aInfo );
            String aEntry( fillFontEntry( aInfo, String( ByteString( aFile ), gsl_getSystemTextEncoding() ) ) );
            USHORT nEntry = m_aFontBox.InsertEntry( aEntry );
            m_aFontBox.SetEntryData( nEntry, (void*)(*font_it) );
        }
    }
}

IMPL_LINK( FontNameDlg, DelPressedHdl, ListBox*, pBox )
{
    if( pBox == &m_aFontBox && m_aRemoveButton.IsEnabled() )
    {
        ClickBtnHdl( &m_aRemoveButton );
    }
    return 0;
}

struct RemoveEntry
{
    int         nCount;
    String      aSelectEntry;
    fontID      nSelectFont;
};

IMPL_LINK( FontNameDlg, ClickBtnHdl, Button*, pButton )
{

    int i;

    if( pButton == &m_aOKButton )
    {
        EndDialog();
    }
    else if( pButton == &m_aRemoveButton && AreYouSure( this, RID_QUERY_REMOVEFONTFROMLIST ) )
    {
        ::std::hash_map< OString, struct RemoveEntry, OStringHash > aRemoveList;
        for( i = 0; i < m_aFontBox.GetSelectEntryCount(); i++ )
        {
            int nSelect = m_aFontBox.GetSelectEntryPos( i );
            fontID nFont = (fontID)m_aFontBox.GetEntryData( nSelect );
            OString aFile( m_rFontManager.getFontFileSysPath( nFont ) );
            if( aRemoveList.find( aFile ) == aRemoveList.end() )
            {
                struct RemoveEntry aEntry;
                aEntry.nCount       = 0;
                aEntry.aSelectEntry = m_aFontBox.GetEntry( nSelect );
                aEntry.nSelectFont  = (fontID)m_aFontBox.GetEntryData( nSelect );
                aRemoveList[ aFile ] = aEntry;
            }
            aRemoveList[ aFile ].nCount++;
        }
        ::std::list< fontID > aRemoveIDs;
        for( ::std::hash_map< OString, struct RemoveEntry, OStringHash >::iterator it = aRemoveList.begin(); it != aRemoveList.end(); ++it )
        {
            ::std::list< fontID > aDups;
            bool bDuplicates = m_rFontManager.getFileDuplicates( it->second.nSelectFont, aDups );
            if( bDuplicates && aDups.size()+1 != it->second.nCount )
            {
                String aMessage( PaResId( RID_STR_REMOVE_MULTIPLE_FONTS ) );
                aMessage.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s1" ) ), it->second.aSelectEntry );
                String aFonts;
                String aFileName( OStringToOUString( it->first.copy( it->first.lastIndexOf( '/' )+1 ), osl_getThreadTextEncoding() ) );
                for( ::std::list< fontID >::iterator dup = aDups.begin(); dup != aDups.end(); ++dup )
                {
                    FastPrintFontInfo aInfo;
                    m_rFontManager.getFontFastInfo( *dup, aInfo );

                    aFonts.Append( fillFontEntry( aInfo, aFileName ) );
                    aFonts.Append( '\n' );
                }
                aMessage.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s2" ) ), aFonts );
                QueryBox aQueryBox( this, WB_YES_NO | WB_DEF_NO, aMessage );
                if( aQueryBox.Execute() == BUTTONID_NO )
                    continue;
            }
            aRemoveIDs.push_back( it->second.nSelectFont );
        }
        m_rFontManager.removeFonts( aRemoveIDs );
        init();
    }
    else if( pButton == &m_aImportButton )
    {
        FontImportDialog aDialog( this );
        aDialog.Execute();
        init();
    }
    return 0;
}

//--------------------------------------------------

FontImportDialog::FontImportDialog( Window* pParent ) :
        ModalDialog( pParent, PaResId( RID_FONTIMPORT_DIALOG ) ),
        m_aOKBtn( this, PaResId( RID_FIMP_BTN_OK ) ),
        m_aCancelBtn( this, PaResId( RID_FIMP_BTN_CANCEL ) ),
        m_aFromBox( this, PaResId( RID_FIMP_GRP_FROM ) ),
        m_aFromDirEdt( this, PaResId( RID_FIMP_EDT_FROM ) ),
        m_aFromBtn( this, PaResId( RID_FIMP_BTN_FROM ) ),
        m_aLinkOnlyBox( this, PaResId( RID_FIMP_BOX_LINKONLY ) ),
        m_aRemoveBtn( this, PaResId( RID_FIMP_BTN_REMOVE ) ),
        m_aNewFontsBox( this, PaResId( RID_FIMP_BOX_NEWFONTS ) ),
        m_aFixedText( this, PaResId( RID_FIMP_TXT_HELP ) ),
        m_bOverwriteAll( false ),
        m_bOverwriteNone( false ),
        m_pProgress( NULL ),
        m_aImportOperation( PaResId( RID_FIMP_STR_IMPORTOP ) ),
        m_aOverwriteQueryText( PaResId( RID_FIMP_STR_QUERYOVERWRITE ) ),
        m_aOverwriteAllText( PaResId( RID_FIMP_STR_OVERWRITEALL ) ),
        m_aOverwriteNoneText( PaResId( RID_FIMP_STR_OVERWRITENONE ) ),
        m_aNoAfmText( PaResId( RID_FIMP_STR_NOAFM ) ),
        m_aAfmCopyFailedText( PaResId( RID_FIMP_STR_AFMCOPYFAILED ) ),
        m_aFontCopyFailedText( PaResId( RID_FIMP_STR_FONTCOPYFAILED ) ),
        m_aNoWritableFontsDirText( PaResId( RID_FIMP_STR_NOWRITEABLEFONTSDIR ) ),
        m_aFontsImportedText( PaResId( RID_FIMP_STR_NUMBEROFFONTSIMPORTED ) ),
        m_rFontManager( ::psp::PrintFontManager::get() )
{
    FreeResource();

    m_aOKBtn.SetClickHdl( LINK( this, FontImportDialog, ClickBtnHdl ) );
    m_aRemoveBtn.SetClickHdl( LINK( this, FontImportDialog, ClickBtnHdl ) );
    m_aFromBtn.SetClickHdl( LINK( this, FontImportDialog, ClickBtnHdl ) );
    m_aFromDirEdt.SetModifyHdl( LINK( this, FontImportDialog, ModifyHdl ) );
    m_aRefreshTimer.SetTimeoutHdl( LINK( this, FontImportDialog, RefreshTimeoutHdl ) );
    m_aRefreshTimer.SetTimeout( 2000 );
    m_aNewFontsBox.setDelPressedLink( LINK( this, FontImportDialog, DelPressedHdl ) );
    m_aLinkOnlyBox.Check( FALSE );
}

FontImportDialog::~FontImportDialog()
{
}

void FontImportDialog::importFontsFailed( ::psp::PrintFontManager::ImportFontCallback::FailCondition eReason )
{
    String aText;
    switch( eReason )
    {
        case ::psp::PrintFontManager::ImportFontCallback::NoWritableDirectory:
            aText = m_aNoWritableFontsDirText;
    }
    ErrorBox aBox( m_pProgress ? (Window*)m_pProgress : (Window*)this, WB_OK | WB_DEF_OK, aText );
    aBox.Execute();
}

void FontImportDialog::progress( const ::rtl::OUString& rFile )
{
    m_pProgress->setValue( ++m_nFont );
    m_pProgress->setFilename( rFile );
}

bool FontImportDialog::queryOverwriteFile( const ::rtl::OUString& rFile )
{
    bool bRet = false;

    if( m_bOverwriteNone )
        return false;
    if( m_bOverwriteAll )
        return true;

    String aText( m_aOverwriteQueryText );
    aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), rFile );

    QueryBox aQueryBox( m_pProgress ? (Window*)m_pProgress : (Window*)this, WB_YES_NO | WB_DEF_NO, aText );
    aQueryBox.AddButton( m_aOverwriteAllText, 20, 0 );
    aQueryBox.AddButton( m_aOverwriteNoneText, 21, 0 );

    int nResult = aQueryBox.Execute();
    switch( nResult )
    {
        case BUTTONID_YES:      bRet = true; break;
        case BUTTONID_NO:       bRet = false; break;
        case 20:                bRet = m_bOverwriteAll = true; break;
        case 21:                bRet = false; m_bOverwriteNone = true; break;
    }

    return bRet;
}

void FontImportDialog::importFontFailed( const ::rtl::OUString& rFile, ::psp::PrintFontManager::ImportFontCallback::FailCondition eReason )
{
    String aText;
    switch( eReason )
    {
        case ::psp::PrintFontManager::ImportFontCallback::NoAfmMetric:
            aText = m_aNoAfmText;
            break;
        case ::psp::PrintFontManager::ImportFontCallback::AfmCopyFailed:
            aText = m_aAfmCopyFailedText;
            break;
        case ::psp::PrintFontManager::ImportFontCallback::FontCopyFailed:
            aText = m_aFontCopyFailedText;
            break;
    }
    aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), rFile );
    ErrorBox aBox( m_pProgress ? (Window*)m_pProgress : (Window*)this, WB_OK | WB_DEF_OK, aText );
    aBox.Execute();
}

bool FontImportDialog::isCanceled()
{
    return m_pProgress->isCanceled();
}

IMPL_LINK( FontImportDialog, ModifyHdl, Edit*,pEdit )
{
    m_aRefreshTimer.Start();
    return 0;
}

IMPL_LINK( FontImportDialog, RefreshTimeoutHdl, void*, pDummy )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    ::std::list< String > aFiles;
    m_aNewFonts.clear();
    OUString aDirectory( m_aFromDirEdt.GetText() );
    FindFiles( aDirectory, aFiles, String( RTL_CONSTASCII_USTRINGPARAM( "PFA;PFB;TTF;TTC" ) ) );
    OString aDir( OUStringToOString( aDirectory, aEncoding ) );
    aDir += "/";
    while( aFiles.begin() != aFiles.end() )
    {
        OString aFont( aDir );
        aFont += OUStringToOString( aFiles.front(), aEncoding );
        aFiles.pop_front();
        ::std::list< FastPrintFontInfo > aInfos;
        if( m_rFontManager.getImportableFontProperties( aFont, aInfos ) )
            m_aNewFonts[ aFont ] = aInfos;
    }
    fillFontBox();
    return 0;
}

void FontImportDialog::fillFontBox()
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    m_aNewFontsBox.Clear();
    for( ::std::hash_map< OString, ::std::list< FastPrintFontInfo >, OStringHash >::iterator it = m_aNewFonts.begin(); it != m_aNewFonts.end(); ++it )
    {
        OString aFileName( it->first.copy( it->first.lastIndexOf( '/' )+1 ) );
        OUString aFile( OStringToOUString( aFileName, aEncoding ) );
        for( ::std::list< FastPrintFontInfo >::iterator font_it = it->second.begin(); font_it != it->second.end(); ++font_it )
        {
            String aEntry( FontNameDlg::fillFontEntry( *font_it, aFile ) );
            USHORT nPos = m_aNewFontsBox.InsertEntry( aEntry );
            m_aNewFontsBox.SetEntryData( nPos, (void*)&(it->first) );
        }
    }
}

void FontImportDialog::copyFonts()
{
    ::std::list< OString > aFiles;
    for( ::std::hash_map< OString, ::std::list< FastPrintFontInfo >, OStringHash >::iterator it = m_aNewFonts.begin(); it != m_aNewFonts.end(); ++it )
        aFiles.push_back( it->first );

    int nSuccess = 0;
    if( aFiles.size() )
    {
        m_nFont = 0;
        m_pProgress = new ProgressDialog( this );
        m_pProgress->setRange( 0, aFiles.size() );
        m_pProgress->startOperation( m_aImportOperation );
        m_pProgress->Show( TRUE );
        m_pProgress->setValue( 0 );
        m_pProgress->Invalidate();
        m_pProgress->Sync();
        nSuccess = m_rFontManager.importFonts( aFiles, m_aLinkOnlyBox.IsChecked() ? true : false, this );
        m_pProgress->Show( FALSE );
        delete m_pProgress;
        m_pProgress = NULL;
    }
    String aText( m_aFontsImportedText );
    aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%d" ) ), String::CreateFromInt32( nSuccess ) );
    InfoBox aBox( this, aText );
    aBox.Execute();
}

IMPL_LINK( FontImportDialog, DelPressedHdl, ListBox*, pBox )
{
    ::std::hash_map< OString, ::std::pair< OUString, int >, OStringHash > aRemoveFonts;
    ::std::hash_map< OString, ::std::pair< OUString, int >, OStringHash >::iterator it;
    for( int i = 0; i < m_aNewFontsBox.GetSelectEntryCount(); i++ )
    {
        int nSelect = m_aNewFontsBox.GetSelectEntryPos( i );
        OString* pFile = (OString*)m_aNewFontsBox.GetEntryData( nSelect );
        it = aRemoveFonts.find( *pFile );
        if( it == aRemoveFonts.end() )
        {
            ::std::pair< OUString, int > aPair( m_aNewFontsBox.GetEntry( nSelect ), 0 );
            aRemoveFonts[ *pFile ] = aPair;
        }
        aRemoveFonts[ *pFile ].second++;
    }
    for( it = aRemoveFonts.begin(); it != aRemoveFonts.end(); ++it )
    {
        if( it->second.second != m_aNewFonts[ it->first ].size() )
        {
            String aMessage( PaResId( RID_STR_REMOVE_MULTIPLE_FONTS ) );
            aMessage.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s1" ) ), it->second.first );
            OString* pFile = (OString*)m_aNewFontsBox.GetEntryData( m_aNewFontsBox.GetEntryPos( it->second.first ) );
            String aFonts;
            for( int n = 0; n != m_aNewFontsBox.GetEntryCount(); n++ )
            {
                if( pFile == m_aNewFontsBox.GetEntryData( n ) &&
                    m_aNewFontsBox.GetEntry( n ) != String( it->second.first ) )
                {
                    aFonts.Append( m_aNewFontsBox.GetEntry( n ) );
                    aFonts.Append( '\n' );
                }
            }
            aMessage.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s2" ) ), aFonts );
            QueryBox aQueryBox( this, WB_YES_NO | WB_DEF_NO, aMessage );
            if( aQueryBox.Execute() == BUTTONID_NO )
                it->second.second = 0;
        }
    }
    for( it = aRemoveFonts.begin(); it != aRemoveFonts.end(); ++it )
    {
        if( it->second.second > 0 )
            m_aNewFonts.erase( it->first );
    }
    fillFontBox();
    return 0;
}

IMPL_LINK( FontImportDialog, ClickBtnHdl, Button*, pButton )
{
    if( pButton == &m_aFromBtn )
    {
        PathDialog aDlg( this );
        if( aDlg.Execute() )
        {
            m_aFromDirEdt.SetText( aDlg.GetPath() );
            RefreshTimeoutHdl( NULL );
        }
    }
    else if( pButton == &m_aOKBtn )
    {
        copyFonts();
        EndDialog( 0 );
    }
    else if( pButton == &m_aRemoveBtn )
    {
        DelPressedHdl( &m_aNewFontsBox );
    }
    return 0;
}
