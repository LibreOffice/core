/*************************************************************************
 *
 *  $RCSfile: fontentry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2001-05-08 13:15:13 $
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
        m_aFoundryBox( this, PaResId( RID_FNTNM_FOUNDRY ) ),
        m_aFamilyEdit( this, PaResId( RID_FNTNM_FAMILY ) ),
        m_aWeightBox( this, PaResId( RID_FNTNM_WEIGHT ) ),
        m_aSlantBox( this, PaResId( RID_FNTNM_SLANT ) ),
        m_aStyleWidthBox( this, PaResId( RID_FNTNM_STYLEWIDTH ) ),
        m_aSpacingListBox( this, PaResId( RID_FNTNM_SPACING ) ),
        m_aAddStyleBox( this, PaResId( RID_FNTNM_ADDSTYLE ) ),
        m_aRegistryBox( this, PaResId( RID_FNTNM_REGISTRY ) ),
        m_aEncodingBox( this, PaResId( RID_FNTNM_ENCODING ) ),
        m_aTxtFoundry( this, PaResId( RID_FNTNM_TXT_FOUNDRY ) ),
        m_aTxtFamily( this, PaResId( RID_FNTNM_TXT_FAMILY ) ),
        m_aTxtWeight( this, PaResId( RID_FNTNM_TXT_WEIGHT ) ),
        m_aTxtSlant( this, PaResId( RID_FNTNM_TXT_SLANT ) ),
        m_aTxtStyleWidth( this, PaResId( RID_FNTNM_TXT_STYLEWIDTH ) ),
        m_aTxtSpacing( this, PaResId( RID_FNTNM_TXT_SPACING ) ),
        m_aTxtAddStyle( this, PaResId( RID_FNTNM_TXT_ADDSTYLE ) ),
        m_aTxtRegistry( this, PaResId( RID_FNTNM_TXT_REGISTRY ) ),
        m_aTxtEncoding( this, PaResId( RID_FNTNM_TXT_ENCODING ) ),
        m_aRemoveButton( this, PaResId( RID_FNTNM_BTN_REMOVE ) ),
        m_aModifyButton( this, PaResId( RID_FNTNM_BTN_MODIFY ) ),
        m_aFontBox( this, PaResId( RID_FNTNM_LB_FONTS ) ),
        m_aPropGroupBox( this, PaResId( RID_FNTNM_GRP_FONTPROP ) ),
        m_rFontManager( PrintFontManager::get() ),
        m_aFontsDirWriteFailed( PaResId( RID_FNTNM_STR_FONTSDIRWRITEFAILED ) )
{
    FreeResource();

    m_aFontBox.EnableMultiSelection( TRUE );

    ::std::list< fontID > aFonts;
    m_rFontManager.getFontList( aFonts );
    for( ::std::list< fontID >::iterator font_it = aFonts.begin(); font_it != aFonts.end(); ++font_it )
    {
        fonttype::type eType = m_rFontManager.getFontType( *font_it );
        if( eType == fonttype::Type1 || eType == fonttype::TrueType )
        {
            m_aFonts[ *font_it ] = m_rFontManager.getFontXLFD( *font_it );
            ByteString aFontFile( m_rFontManager.getFontFileSysPath( *font_it ) );
            int nTokens = aFontFile.GetTokenCount( '/' );
            String aEntry( aFontFile.GetToken( nTokens-1, '/' ), gsl_getSystemTextEncoding() );
            aEntry += '=';
            aEntry += String( m_rFontManager.getFontFamily( *font_it ) );
            USHORT nEntry = m_aFontBox.InsertEntry( aEntry );
            m_aFontBox.SetEntryData( nEntry, (void*)(*font_it) );
        }
    }

    // iterate over fonts and fill comoboxes
    ::std::hash_map< OUString, int, OUStringHash > aFoundries, aWeights, aWidths, aRegistries, aEncodings;
    for( ::std::hash_map< fontID, String >::iterator it = m_aFonts.begin(); it != m_aFonts.end(); ++it )
    {
        aFoundries[ it->second.GetToken( 1, '-' ).ToLowerAscii() ]      = 1;
        aWeights[ it->second.GetToken( 3, '-' ).ToLowerAscii() ]        = 1;
        aWidths[ it->second.GetToken( 5, '-' ).ToLowerAscii() ]         = 1;
        aRegistries[ it->second.GetToken( 13, '-' ).ToLowerAscii() ]    = 1;
        aEncodings[ it->second.GetToken( 14, '-' ).ToLowerAscii() ]     = 1;
    }
    ::std::hash_map< OUString, int, OUStringHash >::iterator m_it;
    for( m_it = aFoundries.begin(); m_it != aFoundries.end(); ++m_it )
        m_aFoundryBox.InsertEntry( m_it->first );
    for( m_it = aWeights.begin(); m_it != aWeights.end(); ++m_it )
        m_aWeightBox.InsertEntry( m_it->first );
    for( m_it = aWidths.begin(); m_it != aWidths.end(); ++m_it )
        m_aStyleWidthBox.InsertEntry( m_it->first );
    for( m_it = aRegistries.begin(); m_it != aRegistries.end(); ++m_it )
        m_aRegistryBox.InsertEntry( m_it->first );
    for( m_it = aEncodings.begin(); m_it != aEncodings.end(); ++m_it )
        m_aEncodingBox.InsertEntry( m_it->first );

    m_aFontBox.SelectEntryPos( 0, TRUE );
    SelectFont();

    m_aOKButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aRemoveButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aModifyButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aFontBox.SetSelectHdl( LINK( this, FontNameDlg, SelectHdl ) );
    m_aFontBox.setDelPressedLink( LINK( this, FontNameDlg, DelPressedHdl ) );
}

FontNameDlg::~FontNameDlg()
{
}

void FontNameDlg::changeSelected()
{
    ::std::list< fontID > aFonts;
    for( int i = 0; i < m_aFontBox.GetSelectEntryCount(); i++ )
        aFonts.push_back( (fontID)m_aFontBox.GetEntryData( m_aFontBox.GetSelectEntryPos( i ) ) );
    while( aFonts.begin() != aFonts.end() )
    {
        ChangeFontEntry( aFonts.front() );
        aFonts.pop_front();
    }
}

void FontNameDlg::ChangeFontEntry( fontID nFont )
{
    rtl_TextEncoding aEncoding = gsl_getSystemTextEncoding();

    String aFontFoundry, aFontFamily, aFontWeight, aFontSlant,
        aFontStyleWidth, aAddStyle, aFontSpacing, aFontRegistry, aFontEncoding,
        aLine, aFileName;

    m_aPropGroupBox.SetText( String( PaResId( RID_FNTNM_NOFONT_TXT ) ) );

    aFontFoundry    = m_aFoundryBox.GetText();
    if( aFontFoundry.GetChar(0) == '*' )
        aFontFoundry    = m_aFonts[ nFont ].GetToken( 1, '-' );

    aFontFamily     = m_aFamilyEdit.GetText();
    if( aFontFamily.GetChar(0) == '*' )
        aFontFamily     = m_aFonts[ nFont ].GetToken( 2, '-' );

    aFontWeight     = m_aWeightBox.GetText();
    if( aFontWeight.GetChar(0) == '*' )
        aFontWeight     = m_aFonts[ nFont ].GetToken( 3, '-' );

    aFontSlant      = m_aSlantBox.GetText().GetToken( 0, ' ' );
    if( aFontSlant.GetChar(0) == '*' )
        aFontSlant      = m_aFonts[ nFont ].GetToken( 4, '-' );

    aFontStyleWidth = m_aStyleWidthBox.GetText();
    if( aFontStyleWidth.GetChar(0) == '*' )
            aFontStyleWidth = m_aFonts[ nFont ].GetToken( 5, '-' );

    aAddStyle       = m_aAddStyleBox.GetText();
    if( aAddStyle.GetChar(0) == '*' )
        aAddStyle       = m_aFonts[ nFont ].GetToken( 6, '-' );

    aFontSpacing    = m_aSpacingListBox.GetSelectEntry().GetToken( 0, ' ' );
    if( aFontSpacing.GetChar(0) == '*' )
        aFontSpacing    = m_aFonts[ nFont ].GetToken( 11, '-' );

    aFontRegistry   = m_aRegistryBox.GetText();
    if( aFontRegistry.GetChar(0) == '*' )
        aFontRegistry   = m_aFonts[ nFont ].GetToken( 13, '-' );

    aFontEncoding   = m_aEncodingBox.GetText();
    if( aFontEncoding.GetChar(0) == '*' )
            aFontEncoding   = m_aFonts[ nFont ].GetToken( 14, '-' );

    // create new XLFD
    String aXLFD( RTL_CONSTASCII_USTRINGPARAM( "-" ) );
    aXLFD += aFontFoundry;
    aXLFD += '-';
    aFontFamily.SearchAndReplaceAll( '-', '_' );
    aXLFD += aFontFamily;
    aXLFD += '-';
    aXLFD += aFontWeight;
    aXLFD += '-';
    aXLFD += aFontSlant;
    aXLFD += '-';
    aXLFD += aFontStyleWidth;
    aXLFD += '-';
    aXLFD += aAddStyle;
    aXLFD.AppendAscii( "-0-0-0-0-" );
    aXLFD += aFontSpacing;
    aXLFD.AppendAscii( "-0-" );
    aXLFD += aFontRegistry;
    aXLFD += '-';
    aXLFD += aFontEncoding;

    if( m_rFontManager.changeFontProperties( nFont, aXLFD ) )
    {
        m_aFonts[ nFont ] = aXLFD;

        // update ListBox
        for( int i = 0; i < m_aFontBox.GetEntryCount(); i++ )
        {
            if( m_aFontBox.GetEntryData( i ) == (void*)nFont )
            {
                m_aFontBox.RemoveEntry( i );
                ByteString aFontFile( m_rFontManager.getFontFileSysPath( nFont ) );
                int nTokens = aFontFile.GetTokenCount( '/' );
                String aEntry( aFontFile.GetToken( nTokens-1, '/' ), aEncoding );
                aEntry += '=';
                aEntry += String( m_rFontManager.getFontFamily( nFont ) );
                USHORT nEntry = m_aFontBox.InsertEntry( aEntry );
                m_aFontBox.SetEntryData( nEntry, (void*)nFont );
            }
        }
    }
    else
    {
        INetURLObject aFont( OStringToOUString( m_rFontManager.getFontFileSysPath( nFont ), aEncoding ), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        String aString( m_aFontsDirWriteFailed );
        aString.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s1" ) ),
                                  aFont.GetName() );
        aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "fonts.dir" ) ) );
        aString.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s2" ) ),
                                  aFont.PathToFileName() );
        ErrorBox aBox( this, WB_OK | WB_DEF_OK, aString );
        aBox.Execute();
    }
}

String FontNameDlg::matchSelectionToken( int n )
{
    if( m_aFontBox.GetSelectEntryCount() < 1 )
        return '*';

    fontID nFont = (fontID)m_aFontBox.GetEntryData( m_aFontBox.GetSelectEntryPos( 0 ) );
    if( ! nFont )
        return '*';
    String aMatch = m_aFonts[ nFont ].GetToken( n, '-' );
    int i;
    for( i = 1; i < m_aFontBox.GetSelectEntryCount(); i++ )
    {
        nFont = (fontID)m_aFontBox.GetEntryData( m_aFontBox.GetSelectEntryPos( i ) );
        if( m_aFonts[ nFont ].GetToken( n, '-' ) != aMatch )
        {
            aMatch = '*';
            break;
        }
    }
    return aMatch;
}

void FontNameDlg::SelectFont()
{
    if( m_aFontBox.GetSelectEntryCount() < 1 )
        return;

    // initialize xfontnames

    String aMatch;
    int i;

    m_aFoundryBox.SetText( matchSelectionToken( 1 ) );
    m_aFamilyEdit.SetText( matchSelectionToken( 2 ) );
    m_aWeightBox.SetText( matchSelectionToken( 3 ) );
    m_aSlantBox.SetText( matchSelectionToken( 4 ) );
    m_aStyleWidthBox.SetText( matchSelectionToken( 5 ) );
    m_aAddStyleBox.SetText( matchSelectionToken( 6 ) );
    aMatch = matchSelectionToken( 11 );
    for( i = 0; i < m_aSpacingListBox.GetEntryCount(); i++ )
    {
        if( aMatch == m_aSpacingListBox.GetEntry( i ).GetToken( 0, ' ' ) )
        {
            m_aSpacingListBox.SelectEntryPos( i, TRUE );
            break;
        }
    }
    if( i >= m_aSpacingListBox.GetEntryCount() )
        m_aSpacingListBox.SelectEntry( '*' );
    m_aRegistryBox.SetText( matchSelectionToken( 13 ) );
    m_aEncodingBox.SetText( matchSelectionToken( 14 ) );
}

IMPL_LINK( FontNameDlg, DelPressedHdl, ListBox*, pBox )
{
    if( pBox == &m_aFontBox && m_aRemoveButton.IsEnabled() )
    {
        ClickBtnHdl( &m_aRemoveButton );
    }
    return 0;
}

IMPL_LINK( FontNameDlg, ClickBtnHdl, Button*, pButton )
{

    int i;

    if( pButton == &m_aOKButton )
    {
        EndDialog();
    }
    else if( pButton == &m_aRemoveButton && AreYouSure( this, RID_QUERY_REMOVEFONTFROMLIST ) )
    {
        ::std::list< fontID > aRemoveList;
        for( i = 0; i < m_aFontBox.GetSelectEntryCount(); i++ )
        {
            fontID nFont = (fontID)m_aFontBox.GetEntryData( m_aFontBox.GetSelectEntryPos( i ) );
            // if( m_rFontManager.removeFont( nFont ) )
            aRemoveList.push_back( nFont );
        }

        while( aRemoveList.begin() != aRemoveList.end() )
        {
            fontID nFont = aRemoveList.front();
            aRemoveList.pop_front();
            for( i = 0; i < m_aFontBox.GetEntryCount(); i++ )
            {
                if( m_aFontBox.GetEntryData( i ) == (void*)nFont )
                {
                    m_aFontBox.RemoveEntry( i );
                    break;
                }
            }
        }
        m_aFontBox.SelectEntryPos( 0, TRUE );
        SelectFont();
    }
    else if( pButton == &m_aModifyButton )
    {
        changeSelected();
    }
    return 0;
}

IMPL_LINK( FontNameDlg, SelectHdl, ListBox*, pBox )
{
    if( pBox == &m_aFontBox )
    {
        SelectFont();
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
    m_aFromBtn.SetClickHdl( LINK( this, FontImportDialog, ClickBtnHdl ) );
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

void FontImportDialog::copyFonts()
{
    ::std::list< String > aFiles;
    String aDirectory( m_aFromDirEdt.GetText() );
    FindFiles( aDirectory, aFiles, String( RTL_CONSTASCII_USTRINGPARAM( "PFA;PFB;TTF" ) ) );
    ::std::list< OUString > aFontFiles;
    while( aFiles.begin() != aFiles.end() )
    {
        String aFont( aDirectory );
        aFont += '/';
        aFont += aFiles.front();
        aFiles.pop_front();
        aFontFiles.push_back( aFont );
    }
    int nSuccess = 0;
    if( aFontFiles.size() )
    {
        m_nFont = 0;
        m_pProgress = new ProgressDialog( this );
        m_pProgress->setRange( 0, aFontFiles.size() );
        m_pProgress->startOperation( m_aImportOperation );
        m_pProgress->Show( TRUE );
        m_pProgress->setValue( 0 );
        m_pProgress->Invalidate();
        m_pProgress->Sync();
        nSuccess = m_rFontManager.importFonts( aFontFiles, this );
        m_pProgress->Show( FALSE );
        delete m_pProgress;
        m_pProgress = NULL;
    }
    String aText( m_aFontsImportedText );
    aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%d" ) ), String::CreateFromInt32( nSuccess ) );
    InfoBox aBox( this, aText );
    aBox.Execute();
}

IMPL_LINK( FontImportDialog, ClickBtnHdl, Button*, pButton )
{
    if( pButton == & m_aFromBtn )
    {
        PathDialog aDlg( this );
        if( aDlg.Execute() )
            m_aFromDirEdt.SetText( aDlg.GetPath() );
    }
    else if( pButton == & m_aOKBtn )
    {
        copyFonts();
        EndDialog( 0 );
    }
    return 0;
}
