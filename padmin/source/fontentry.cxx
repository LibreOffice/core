/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "fontentry.hxx"
#include "helper.hxx"
#include "padialog.hrc"
#include "vcl/strhelper.hxx"

#include "vcl/msgbox.hxx"
#include "vcl/fontmanager.hxx"

#include "osl/thread.h"

#include "tools/config.hxx"

using namespace padmin;
using namespace osl;
using namespace psp;

using ::rtl::OUString;
using ::rtl::OUStringHash;
using ::rtl::OUStringToOString;
using ::rtl::OString;
using ::rtl::OStringHash;

FontNameDlg::FontNameDlg( Window *pParent ) :
        ModalDialog( pParent, PaResId( RID_FONTNAMEDIALOG ) ),
        m_aOKButton( this, PaResId( RID_FNTNM_BTN_OK ) ),
        m_aRenameButton( this, PaResId( RID_FNTNM_BTN_RENAME ) ),
        m_aRemoveButton( this, PaResId( RID_FNTNM_BTN_REMOVE ) ),
        m_aImportButton( this, PaResId( RID_FNTNM_BTN_IMPORT ) ),
        m_aFontBox( this, PaResId( RID_FNTNM_LB_FONTS ) ),
        m_aFixedText( this, PaResId( RID_FNTNM_FIXED ) ),
        m_aRenameString( PaResId( RID_FNTNM_STR_RENAME ) ),
        m_aRenameTTCString( PaResId( RID_FNTNM_STR_TTCRENAME ) ),
        m_aNoRenameString( PaResId( RID_FNTNM_STR_NOTRENAMABLE ) ),
        m_rFontManager( PrintFontManager::get() )
{
    FreeResource();

    m_aFontBox.EnableMultiSelection( sal_True );

    m_aOKButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aRenameButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aRemoveButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aImportButton.SetClickHdl( LINK( this, FontNameDlg, ClickBtnHdl ) );
    m_aFontBox.setDelPressedLink( LINK( this, FontNameDlg, DelPressedHdl ) );
    m_aFontBox.SetSelectHdl( LINK( this, FontNameDlg, SelectHdl ) );

    init();
}

FontNameDlg::~FontNameDlg()
{
}

String FontNameDlg::fillFontEntry( FastPrintFontInfo& rInfo, const String& rFile, bool bAddRegular )
{
    static String aThinTxt( PaResId( RID_TXT_FONT_THIN ) );
    static String aUltraLightTxt( PaResId( RID_TXT_FONT_ULTRALIGHT ) );
    static String aLightTxt( PaResId( RID_TXT_FONT_LIGHT ) );
    static String aSemiLightTxt( PaResId( RID_TXT_FONT_SEMILIGHT ) );
    static String aSemiBoldTxt( PaResId( RID_TXT_FONT_SEMIBOLD ) );
    static String aBoldTxt( PaResId( RID_TXT_FONT_BOLD ) );
    static String aUltraBoldTxt( PaResId( RID_TXT_FONT_ULTRABOLD ) );

    static String aItalicTxt( PaResId( RID_TXT_FONT_ITALIC ) );
    static String aObliqueTxt( PaResId( RID_TXT_FONT_OBLIQUE ) );

    static String aUltraCondensedTxt( PaResId( RID_TXT_FONT_ULTRACONDENSED ) );
    static String aExtraCondensedTxt( PaResId( RID_TXT_FONT_EXTRACONDENSED ) );
    static String aCondensedTxt( PaResId( RID_TXT_FONT_CONDENSED ) );
    static String aSemiCondensedTxt( PaResId( RID_TXT_FONT_SEMICONDENSED ) );
    static String aSemiExpandedTxt( PaResId( RID_TXT_FONT_SEMIEXPANDED ) );
    static String aExpandedTxt( PaResId( RID_TXT_FONT_EXPANDED ) );
    static String aExtraExpandedTxt( PaResId( RID_TXT_FONT_EXTRAEXPANDED ) );
    static String aUltraExpandedTxt( PaResId( RID_TXT_FONT_ULTRAEXPANDED ) );

    static String aRegularTxt( PaResId( RID_TXT_FONT_REGULAR ) );

    String aEntry( rInfo.m_aFamilyName );
    bool bWeight = true, bItalic = true, bWidth = true;
    switch( rInfo.m_eWeight )
    {
        case weight::Thin:          aEntry.AppendAscii( ", " ); aEntry.Append( aThinTxt ); break;
        case weight::UltraLight:    aEntry.AppendAscii( ", " ); aEntry.Append( aUltraLightTxt ); break;
        case weight::Light:         aEntry.AppendAscii( ", " ); aEntry.Append( aLightTxt ); break;
        case weight::SemiLight:     aEntry.AppendAscii( ", " ); aEntry.Append( aSemiLightTxt ); break;
        case weight::SemiBold:      aEntry.AppendAscii( ", " ); aEntry.Append( aSemiBoldTxt ); break;
        case weight::Bold:          aEntry.AppendAscii( ", " ); aEntry.Append( aBoldTxt ); break;
        case weight::UltraBold:     aEntry.AppendAscii( ", " ); aEntry.Append( aUltraBoldTxt ); break;
        default:
            bWeight = false;
            break;
    }
    switch( rInfo.m_eItalic )
    {
        case italic::Oblique:       aEntry.AppendAscii( ", " ); aEntry.Append( aObliqueTxt ); break;
        case italic::Italic:        aEntry.AppendAscii( ", " ); aEntry.Append( aItalicTxt ); break;
        default:
            bItalic = false;
            break;
    }
    switch( rInfo.m_eWidth )
    {
        case width::UltraCondensed: aEntry.AppendAscii( ", " ); aEntry.Append( aUltraCondensedTxt ); break;
        case width::ExtraCondensed: aEntry.AppendAscii( ", " ); aEntry.Append( aExtraCondensedTxt ); break;
        case width::Condensed:      aEntry.AppendAscii( ", " ); aEntry.Append( aCondensedTxt ); break;
        case width::SemiCondensed:  aEntry.AppendAscii( ", " ); aEntry.Append( aSemiCondensedTxt ); break;
        case width::SemiExpanded:   aEntry.AppendAscii( ", " ); aEntry.Append( aSemiExpandedTxt ); break;
        case width::Expanded:       aEntry.AppendAscii( ", " ); aEntry.Append( aExpandedTxt ); break;
        case width::ExtraExpanded:  aEntry.AppendAscii( ", " ); aEntry.Append( aExtraExpandedTxt ); break;
        case width::UltraExpanded:  aEntry.AppendAscii( ", " ); aEntry.Append( aUltraExpandedTxt ); break;
        default:
            bWidth = false;
            break;
    }

    if( bAddRegular && ! bItalic && ! bWidth && ! bWeight )
    {
        aEntry.AppendAscii( ", " );
        aEntry.Append( aRegularTxt );
    }

    aEntry.AppendAscii( " (" );
    aEntry.Append( rFile );
    aEntry.AppendAscii( ")" );
    return aEntry;
}

String FontNameDlg::fillFontEntry( const ::std::list< FastPrintFontInfo >& rInfos, const String& rFile )
{
    String aEntry;
    bool bFirst = true;
    for( ::std::list< FastPrintFontInfo >::const_iterator it = rInfos.begin(); it != rInfos.end(); ++it )
    {
        if( ! bFirst )
            aEntry.AppendAscii( " & " );
        aEntry.Append( String( it->m_aFamilyName ) );
        bFirst = false;
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
    m_aRemoveButton.Enable( sal_False );
    m_aRenameButton.Enable( sal_False );

    ::boost::unordered_map< OUString, int, OUStringHash > aFamilies;
    ::std::list< fontID >::iterator font_it;
    for( font_it = aFonts.begin(); font_it != aFonts.end(); ++font_it )
    {
        if( m_rFontManager.isPrivateFontFile( *font_it ) )
        {
            const OUString& rFamily( m_rFontManager.getFontFamily( *font_it ) );
            if( aFamilies.find( rFamily ) == aFamilies.end() )
                aFamilies[rFamily] = 0;
            aFamilies[rFamily]++;
        }
    }

    for( font_it = aFonts.begin(); font_it != aFonts.end(); ++font_it )
    {
        if( m_rFontManager.isPrivateFontFile( *font_it ) )
        {
            OString aFile( m_rFontManager.getFontFileSysPath( *font_it ) );
            int nLast = aFile.lastIndexOf( '/' );
            if( nLast != -1 )
                aFile = aFile.copy( nLast+1 );

            FastPrintFontInfo aInfo;
            m_rFontManager.getFontFastInfo( *font_it, aInfo );
            std::list< fontID > aDups;
            String aEntry;
            if( m_rFontManager.getFileDuplicates( *font_it, aDups ) )
            {
                FastPrintFontInfo aDupInfo;
                std::list< FastPrintFontInfo > aInfos;
                aInfos.push_back( aInfo );
                for( std::list< fontID >::iterator dup = aDups.begin(); dup != aDups.end(); ++dup )
                {
                    m_rFontManager.getFontFastInfo( *dup, aDupInfo );
                    aInfos.push_back( aDupInfo );
                    aFonts.remove( *dup );
                }
                aEntry = fillFontEntry( aInfos, String( ByteString( aFile ), osl_getThreadTextEncoding() ) );
            }
            else
                aEntry = fillFontEntry( aInfo, String( ByteString( aFile ), osl_getThreadTextEncoding() ), aFamilies[ aInfo.m_aFamilyName ] > 1  );
            sal_uInt16 nEntry = m_aFontBox.InsertEntry( aEntry );
            m_aFontBox.SetEntryData( nEntry, (void*)(sal_IntPtr)(*font_it) );
        }
    }
}

IMPL_LINK( FontNameDlg, SelectHdl, ListBox*, pBox )
{
    if( pBox == &m_aFontBox )
    {
        sal_Bool bEnable = m_aFontBox.GetSelectEntryCount() ? sal_True : sal_False;
        m_aRemoveButton.Enable( bEnable );
        m_aRenameButton.Enable( bEnable );
    }
    return 0;
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
    else if( pButton == &m_aRemoveButton && AreYouSure( this, RID_QUERY_REMOVEFONTFROMLIST ) && m_aFontBox.GetSelectEntryCount() )
    {
        ::std::list< fontID > aRemoveIDs;
        for( i = 0; i < m_aFontBox.GetSelectEntryCount(); i++ )
        {
            int nSelect = m_aFontBox.GetSelectEntryPos( i );
            aRemoveIDs.push_back( (fontID)(sal_IntPtr)m_aFontBox.GetEntryData( nSelect ) );
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
    else if( pButton == &m_aRenameButton && m_aFontBox.GetSelectEntryCount() )
    {
        for( i = 0; i < m_aFontBox.GetSelectEntryCount(); i++ )
        {
            fontID aFont = (fontID)(sal_IntPtr)m_aFontBox.GetEntryData( m_aFontBox.GetSelectEntryPos( i ) );
            if( ! m_rFontManager.checkChangeFontPropertiesPossible( aFont ) )
            {
                String aErrorText( m_aNoRenameString );
                aErrorText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), m_aFontBox.GetSelectEntry( i ) );
                ErrorBox aBox( this, WB_OK | WB_DEF_OK, aErrorText );
                aBox.Execute();
                continue;
            }
            ::std::list< fontID > aDuplicates;
            m_rFontManager.getFileDuplicates( aFont, aDuplicates );
            aDuplicates.push_front( aFont );
            int nFonts = aDuplicates.size();
            for( int n = 0; n < nFonts; n++ )
            {
                aFont = aDuplicates.front();
                aDuplicates.pop_front();
                String aFamily( m_rFontManager.getFontFamily( aFont ) );
                ::std::list< OUString > aAlternatives;
                m_rFontManager.getAlternativeFamilyNames( aFont, aAlternatives );
                ::std::list< String > aChoices;
                while( aAlternatives.size() )
                {
                    aChoices.push_back( aAlternatives.front() );
                    aAlternatives.pop_front();
                }
                String aQueryTxt( m_aRenameString );
                if( nFonts > 1 )
                {
                    aQueryTxt = m_aRenameTTCString;
                    aQueryTxt.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%d1" ) ), String::CreateFromInt32( n+1 ) );
                    aQueryTxt.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%d2" ) ), String::CreateFromInt32( nFonts ) );
                }
                aQueryTxt.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), aFamily );
                QueryString aQuery( this, aQueryTxt, aFamily, aChoices );
                if( aQuery.Execute() )
                {
                    aFamily.SearchAndReplaceAll( '-', ' ' );
                    aFamily.SearchAndReplaceAll( '?', ' ' );
                    aFamily.SearchAndReplaceAll( '*', ' ' );
                    aFamily = WhitespaceToSpace( aFamily );
                    if( aFamily.Len() )
                    {
                        String aXLFD = m_rFontManager.getFontXLFD( aFont );
                        aXLFD.SetToken( 2, '-', aFamily );
                        m_rFontManager.changeFontProperties( aFont, aXLFD );
                    }
                }
            }
        }
        init();
    }
    return 0;
}

//--------------------------------------------------

FontImportDialog::FontImportDialog( Window* pParent ) :
        ModalDialog( pParent, PaResId( RID_FONTIMPORT_DIALOG ) ),
        m_aOKBtn( this, PaResId( RID_FIMP_BTN_OK ) ),
        m_aCancelBtn( this, PaResId( RID_FIMP_BTN_CANCEL ) ),
        m_aSelectAllBtn( this, PaResId( RID_FIMP_BTN_SELECTALL ) ),
        m_aNewFontsBox( this, PaResId( RID_FIMP_BOX_NEWFONTS ) ),
        m_aFromFL( this, PaResId( RID_FIMP_FL_FROM ) ),
        m_aFromDirEdt( this, PaResId( RID_FIMP_EDT_FROM ) ),
        m_aFromBtn( this, PaResId( RID_FIMP_BTN_FROM ) ),
        m_aSubDirsBox( this, PaResId( RID_FIMP_BOX_SUBDIRS ) ),
        m_aTargetOptFL( this, PaResId( RID_FIMP_FL_TARGETOPTS ) ),
        m_aLinkOnlyBox( this, PaResId( RID_FIMP_BOX_LINKONLY ) ),
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

    m_aNewFontsBox.EnableMultiSelection( sal_True );

    m_aOKBtn.SetClickHdl( LINK( this, FontImportDialog, ClickBtnHdl ) );
    m_aSelectAllBtn.SetClickHdl( LINK( this, FontImportDialog, ClickBtnHdl ) );
    m_aFromBtn.SetClickHdl( LINK( this, FontImportDialog, ClickBtnHdl ) );
    m_aFromDirEdt.SetModifyHdl( LINK( this, FontImportDialog, ModifyHdl ) );
    m_aRefreshTimer.SetTimeoutHdl( LINK( this, FontImportDialog, RefreshTimeoutHdl ) );
    m_aRefreshTimer.SetTimeout( 2000 );
    m_aLinkOnlyBox.Check( sal_False );
    m_aSubDirsBox.Check( sal_True );
    m_aSubDirsBox.SetToggleHdl( LINK( this, FontImportDialog, ToggleHdl ) );

    Config& rPadminRC( getPadminRC() );
    rPadminRC.SetGroup( "FontImport" );
    m_aFromDirEdt.SetText( String( rPadminRC.ReadKey( "FromPath" ), RTL_TEXTENCODING_UTF8 ) );
    RefreshTimeoutHdl( NULL );
}

FontImportDialog::~FontImportDialog()
{
    Config& rPadminRC( getPadminRC() );
    rPadminRC.SetGroup( "FontImport" );
    rPadminRC.WriteKey( "FromPath", ByteString( m_aFromDirEdt.GetText(), RTL_TEXTENCODING_UTF8 ) );
}

void FontImportDialog::importFontsFailed( ::psp::PrintFontManager::ImportFontCallback::FailCondition eReason )
{
    String aText;
    switch( eReason )
    {
        case psp::PrintFontManager::ImportFontCallback::NoWritableDirectory:
            aText = m_aNoWritableFontsDirText;
            break;
        default:
            break;
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
        case psp::PrintFontManager::ImportFontCallback::NoAfmMetric:
            aText = m_aNoAfmText;
            break;
        case psp::PrintFontManager::ImportFontCallback::AfmCopyFailed:
            aText = m_aAfmCopyFailedText;
            break;
        case psp::PrintFontManager::ImportFontCallback::FontCopyFailed:
            aText = m_aFontCopyFailedText;
            break;
        default:
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

IMPL_LINK( FontImportDialog, ModifyHdl, Edit*, EMPTYARG )
{
    m_aRefreshTimer.Start();
    return 0;
}

IMPL_LINK( FontImportDialog, RefreshTimeoutHdl, void*, EMPTYARG )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    std::list< String > aFiles;
    m_aNewFonts.clear();
    OUString aDirectory( m_aFromDirEdt.GetText() );
    FindFiles( aDirectory, aFiles, String( RTL_CONSTASCII_USTRINGPARAM( "PFA;PFB;TTF;TTC;OTF" ) ), m_aSubDirsBox.IsChecked() );
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

    ::boost::unordered_map< OUString, int, OUStringHash > aFamilies;
    ::boost::unordered_map< OString, ::std::list< FastPrintFontInfo >, OStringHash >::iterator it;
    for( it = m_aNewFonts.begin(); it != m_aNewFonts.end(); ++it )
    {
        const OUString& rFamily( it->second.front().m_aFamilyName );
        if( aFamilies.find( rFamily ) == aFamilies.end() )
                aFamilies[rFamily] = 0;
            aFamilies[rFamily]++;
    }

    for( it = m_aNewFonts.begin(); it != m_aNewFonts.end(); ++it )
    {
        OString aFileName( it->first.copy( it->first.lastIndexOf( '/' )+1 ) );
        OUString aFile( OStringToOUString( aFileName, aEncoding ) );
        String aEntry;
        if( it->second.size() == 1 )
            aEntry = FontNameDlg::fillFontEntry( it->second.front(), aFile, aFamilies[ it->second.front().m_aFamilyName ] > 1 );
        else
            aEntry = FontNameDlg::fillFontEntry( it->second, aFile );
        sal_uInt16 nPos = m_aNewFontsBox.InsertEntry( aEntry );
        m_aNewFontsBox.SetEntryData( nPos, (void*)&(it->first) );
    }
}

void FontImportDialog::copyFonts()
{
    ::std::list< OString > aFiles;
    for( int i = 0; i < m_aNewFontsBox.GetSelectEntryCount(); i++ )
    {
        OString* pFile = (OString*)m_aNewFontsBox.GetEntryData( m_aNewFontsBox.GetSelectEntryPos( i ) );
        aFiles.push_back( *pFile );
    }

    int nSuccess = 0;
    if( aFiles.size() )
    {
        m_nFont = 0;
        m_pProgress = new ProgressDialog( this );
        m_pProgress->setRange( 0, aFiles.size() );
        m_pProgress->startOperation( m_aImportOperation );
        m_pProgress->Show( sal_True );
        m_pProgress->setValue( 0 );
        m_pProgress->Invalidate();
        m_pProgress->Sync();
        nSuccess = m_rFontManager.importFonts( aFiles, m_aLinkOnlyBox.IsChecked() ? true : false, this );
        m_pProgress->Show( sal_False );
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
    if( pButton == &m_aFromBtn )
    {
        String aPath( m_aFromDirEdt.GetText() );
        if( chooseDirectory( aPath ) )
        {
            m_aFromDirEdt.SetText( aPath );
            RefreshTimeoutHdl( NULL );
        }
    }
    else if( pButton == &m_aOKBtn )
    {
        copyFonts();
        EndDialog( 0 );
    }
    else if( pButton == &m_aSelectAllBtn )
    {
        m_aNewFontsBox.SetUpdateMode( sal_False );
        for( int i = 0; i < m_aNewFontsBox.GetEntryCount(); i++ )
            m_aNewFontsBox.SelectEntryPos( i, sal_True );
        m_aNewFontsBox.SetUpdateMode( sal_True );
    }
    return 0;
}

IMPL_LINK( FontImportDialog, ToggleHdl, CheckBox*, pBox )
{
    if( pBox == &m_aSubDirsBox )
    {
        RefreshTimeoutHdl( NULL );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
