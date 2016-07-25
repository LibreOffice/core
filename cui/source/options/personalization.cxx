/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

#include "personalization.hxx"
#include "personasdochandler.hxx"

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <tools/urlobj.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/mnemonic.hxx>
#include <dialmgr.hxx>
#include "cuires.hrc"

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include "ucbhelper/content.hxx"
#include <comphelper/simplefileaccessinteraction.hxx>

#define MAX_RESULTS 9

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;

SelectPersonaDialog::SelectPersonaDialog( vcl::Window *pParent )
    : ModalDialog( pParent, "SelectPersonaDialog", "cui/ui/select_persona_dialog.ui" )
{
    get( m_pSearchButton, "search_personas" );
    m_pSearchButton->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_vSearchSuggestions[0], "suggestion1" );
    m_vSearchSuggestions[0]->SetText( "LibreOffice" );
    m_vSearchSuggestions[0]->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_vSearchSuggestions[1], "suggestion2" );
    m_vSearchSuggestions[1]->SetText( CUI_RES ( RID_SVXSTR_PERSONA_ABSTRACT ) );
    m_vSearchSuggestions[1]->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_vSearchSuggestions[2], "suggestion3" );
    m_vSearchSuggestions[2]->SetText( CUI_RES ( RID_SVXSTR_PERSONA_COLOR ) );
    m_vSearchSuggestions[2]->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_vSearchSuggestions[3], "suggestion4" );
    m_vSearchSuggestions[3]->SetText( CUI_RES( RID_SVXSTR_PERSONA_MUSIC ) );
    m_vSearchSuggestions[3]->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_vSearchSuggestions[4], "suggestion5" );
    m_vSearchSuggestions[4]->SetText( CUI_RES( RID_SVXSTR_PERSONA_NATURE ) );
    m_vSearchSuggestions[4]->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_vSearchSuggestions[5], "suggestion6" );
    m_vSearchSuggestions[5]->SetText( CUI_RES( RID_SVXSTR_PERSONA_SOLID ) );
    m_vSearchSuggestions[5]->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_pEdit, "search_term" );

    get( m_pProgressLabel, "progress_label" );

    get( m_pOkButton, "ok" );
    m_pOkButton->SetClickHdl( LINK( this, SelectPersonaDialog, ActionOK ) );

    get( m_pCancelButton, "cancel" );
    m_pCancelButton->SetClickHdl( LINK( this, SelectPersonaDialog, ActionCancel ) );
    get( m_vResultList[0], "result1" );
    get( m_vResultList[1], "result2" );
    get( m_vResultList[2], "result3" );
    get( m_vResultList[3], "result4" );
    get( m_vResultList[4], "result5" );
    get( m_vResultList[5], "result6" );
    get( m_vResultList[6], "result7" );
    get( m_vResultList[7], "result8" );
    get( m_vResultList[8], "result9" );

    for (VclPtr<PushButton> & nIndex : m_vResultList)
    {
        nIndex->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );
        nIndex->Disable();
    }
}

SelectPersonaDialog::~SelectPersonaDialog()
{
    disposeOnce();
}

void SelectPersonaDialog::dispose()
{
    if (m_pSearchThread.is())
        m_pSearchThread->join();

    m_pEdit.clear();
    m_pSearchButton.clear();
    m_pProgressLabel.clear();
    for (VclPtr<PushButton>& vp : m_vResultList)
        vp.clear();
    for (VclPtr<PushButton>& vp : m_vSearchSuggestions)
        vp.clear();
    m_pOkButton.clear();
    m_pCancelButton.clear();
    ModalDialog::dispose();
}

OUString SelectPersonaDialog::GetSelectedPersona() const
{
    if( !m_aSelectedPersona.isEmpty( ) )
        return m_aSelectedPersona;

    return OUString();
}

IMPL_LINK_TYPED( SelectPersonaDialog, SearchPersonas, Button*, pButton, void )
{
    OUString searchTerm;
    if( m_pSearchThread.is() )
        m_pSearchThread->StopExecution();

    if( pButton ==  m_pSearchButton )
        searchTerm = m_pEdit->GetText();
    else
    {
        for( VclPtr<PushButton> & i : m_vSearchSuggestions )
        {
            if( pButton == i )
            {
                // GetDisplayText() is returning a blank string, thus removing mnemonics explicitly.
                searchTerm = MnemonicGenerator::EraseAllMnemonicChars(i->GetText());
                break;
            }
        }
    }

    if( searchTerm.isEmpty( ) )
        return;

    // 15 results so that invalid and duplicate search results whose names can't be retrieved can be skipped
    OUString rSearchURL = "https://services.addons.allizom.org/en-US/firefox/api/1.5/search/" + searchTerm + "/9/15";

    if ( searchTerm.startsWith( "https://addons.mozilla.org/" ) )
    {
        searchTerm = "https://addons.mozilla.org/en-US/" + searchTerm.copy( searchTerm.indexOf( "firefox" ) );
        m_pSearchThread = new SearchAndParseThread( this, searchTerm, true );
    }
    else
        m_pSearchThread = new SearchAndParseThread( this, rSearchURL, false );

    m_pSearchThread->launch();
}

IMPL_LINK_NOARG_TYPED( SelectPersonaDialog, ActionOK, Button*, void )
{
    OUString aSelectedPersona = GetSelectedPersona();

    if( !aSelectedPersona.isEmpty() )
    {
        m_pSearchThread = new SearchAndParseThread( this, aSelectedPersona, false );
        m_pSearchThread->launch();
    }

    else
    {
        if( m_pSearchThread.is() )
            m_pSearchThread->StopExecution();
        EndDialog( RET_OK );
    }
}

IMPL_LINK_NOARG_TYPED( SelectPersonaDialog, ActionCancel, Button*, void )
{
    if( m_pSearchThread.is() )
        m_pSearchThread->StopExecution();

    EndDialog();
}

IMPL_LINK_TYPED( SelectPersonaDialog, SelectPersona, Button*, pButton, void )
{
    if( m_pSearchThread.is() )
        m_pSearchThread->StopExecution();

    for( sal_Int32 index = 0; index < 9; index++ )
    {
        if( pButton == m_vResultList[index] )
        {
            if( !m_vPersonaSettings[index].isEmpty() )
            {
                m_aSelectedPersona = m_vPersonaSettings[index];
                // get the persona name from the setting variable to show in the progress.
                sal_Int32 nNameIndex = m_aSelectedPersona.indexOf( ';' );
                OUString aName = m_aSelectedPersona.copy( 0, nNameIndex );
                OUString aProgress( CUI_RES( RID_SVXSTR_SELECTEDPERSONA ) );
                aProgress += aName;
                SetProgress( aProgress );
            }
            break;
        }
    }
}

void SelectPersonaDialog::SetAppliedPersonaSetting( OUString& rPersonaSetting )
{
    m_aAppliedPersona = rPersonaSetting;
}

const OUString& SelectPersonaDialog::GetAppliedPersonaSetting() const
{
    return m_aAppliedPersona;
}

void SelectPersonaDialog::SetProgress( const OUString& rProgress )
{
    if(rProgress.isEmpty())
        m_pProgressLabel->Hide();
    else
    {
        SolarMutexGuard aGuard;
        m_pProgressLabel->Show();
        m_pProgressLabel->SetText( rProgress );
        setOptimalLayoutSize();
    }
}

void SelectPersonaDialog::SetImages( const Image& aImage, sal_Int32 nIndex )
{
    m_vResultList[nIndex]->Enable();
    m_vResultList[nIndex]->SetModeImage( aImage );
}

void SelectPersonaDialog::AddPersonaSetting( OUString& rPersonaSetting )
{
    m_vPersonaSettings.push_back( rPersonaSetting );
}

void SelectPersonaDialog::ClearSearchResults()
{
    m_vPersonaSettings.clear();
    m_aSelectedPersona.clear();
    for(VclPtr<PushButton> & nIndex : m_vResultList)
    {
        nIndex->Disable();
        nIndex->SetModeImage(Image());
    }
}

SvxPersonalizationTabPage::SvxPersonalizationTabPage( vcl::Window *pParent, const SfxItemSet &rSet )
    : SfxTabPage( pParent, "PersonalizationTabPage", "cui/ui/personalization_tab.ui", &rSet )
{
    // persona
    get( m_pNoPersona, "no_persona" );
    get( m_pDefaultPersona, "default_persona" );
    get( m_pAppliedThemeLabel, "applied_theme" );

    get( m_pOwnPersona, "own_persona" );
    m_pOwnPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, ForceSelect ) );

    get( m_pSelectPersona, "select_persona" );
    m_pSelectPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, SelectPersona ) );

    get( m_vDefaultPersonaImages[0], "default1" );
    m_vDefaultPersonaImages[0]->SetClickHdl( LINK( this, SvxPersonalizationTabPage, DefaultPersona ) );

    get( m_vDefaultPersonaImages[1], "default2" );
    m_vDefaultPersonaImages[1]->SetClickHdl( LINK( this, SvxPersonalizationTabPage, DefaultPersona ) );

    get( m_vDefaultPersonaImages[2], "default3" );
    m_vDefaultPersonaImages[2]->SetClickHdl( LINK( this, SvxPersonalizationTabPage, DefaultPersona ) );

    get( m_pPersonaList, "installed_personas" );
    m_pPersonaList->SetSelectHdl( LINK( this, SvxPersonalizationTabPage, SelectInstalledPersona ) );

    get( m_pExtensionPersonaPreview, "persona_preview" );

    get ( m_pExtensionLabel, "extensions_label" );

    CheckAppliedTheme();
    LoadDefaultImages();
    LoadExtensionThemes();
}

SvxPersonalizationTabPage::~SvxPersonalizationTabPage()
{
    disposeOnce();
}

void SvxPersonalizationTabPage::dispose()
{
    m_pNoPersona.clear();
    m_pDefaultPersona.clear();
    m_pOwnPersona.clear();
    m_pSelectPersona.clear();
    for (VclPtr<PushButton> & i : m_vDefaultPersonaImages)
        i.clear();
    m_pExtensionPersonaPreview.clear();
    m_pPersonaList.clear();
    m_pExtensionLabel.clear();
    m_pAppliedThemeLabel.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SvxPersonalizationTabPage::Create( vcl::Window *pParent, const SfxItemSet *rSet )
{
    return VclPtr<SvxPersonalizationTabPage>::Create( pParent, *rSet );
}

bool SvxPersonalizationTabPage::FillItemSet( SfxItemSet * )
{
    // persona
    OUString aPersona( "default" );
    if ( m_pNoPersona->IsChecked() )
        aPersona = "no";
    else if ( m_pOwnPersona->IsChecked() )
        aPersona = "own";

    bool bModified = false;
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    if ( xContext.is() &&
            ( aPersona != officecfg::Office::Common::Misc::Persona::get( xContext ) ||
              m_aPersonaSettings != officecfg::Office::Common::Misc::PersonaSettings::get( xContext ) ) )
    {
        bModified = true;
    }

    // write
    std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
    if( aPersona == "no" )
        m_aPersonaSettings.clear();
    officecfg::Office::Common::Misc::Persona::set( aPersona, batch );
    officecfg::Office::Common::Misc::PersonaSettings::set( m_aPersonaSettings, batch );
    batch->commit();

    if ( bModified )
    {
        // broadcast the change
        DataChangedEvent aDataChanged( DataChangedEventType::SETTINGS, nullptr, AllSettingsFlags::STYLE );
        Application::NotifyAllWindows( aDataChanged );
    }

    return bModified;
}

void SvxPersonalizationTabPage::Reset( const SfxItemSet * )
{
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // persona
    OUString aPersona( "default" );
    if ( xContext.is() )
    {
        aPersona = officecfg::Office::Common::Misc::Persona::get( xContext );
        m_aPersonaSettings = officecfg::Office::Common::Misc::PersonaSettings::get( xContext );
    }

    if ( aPersona == "no" )
        m_pNoPersona->Check();
    else if ( aPersona == "own" )
        m_pOwnPersona->Check();
    else
        m_pDefaultPersona->Check();
}

void SvxPersonalizationTabPage::SetPersonaSettings( const OUString& aPersonaSettings )
{
    m_aPersonaSettings = aPersonaSettings;
    ShowAppliedThemeLabel( m_aPersonaSettings );
    m_pOwnPersona->Check();
}

void SvxPersonalizationTabPage::CheckAppliedTheme()
{
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    OUString aPersona( "default" ), aPersonaSetting;
    if ( xContext.is())
    {
        aPersona = officecfg::Office::Common::Misc::Persona::get( xContext );
        aPersonaSetting = officecfg::Office::Common::Misc::PersonaSettings::get( xContext );
    }
    if(aPersona == "own")
        ShowAppliedThemeLabel(aPersonaSetting);
}

void SvxPersonalizationTabPage::ShowAppliedThemeLabel(const OUString& aPersonaSetting)
{
    sal_Int32 nNameIndex = aPersonaSetting.indexOf( '/' );
    OUString aName = "(" + aPersonaSetting.copy( 0, nNameIndex ) +")";
    m_pAppliedThemeLabel->Show();
    m_pAppliedThemeLabel->SetText( aName );
}

void SvxPersonalizationTabPage::LoadDefaultImages()
{
    // Load the pre saved personas

    OUString gallery( "" );
    gallery = "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER;
    gallery += "/gallery/personas/";
    rtl::Bootstrap::expandMacros( gallery );
    OUString aPersonasList = gallery + "personas_list.txt";
    SvFileStream aStream( aPersonasList, StreamMode::READ );
    GraphicFilter aFilter;
    Graphic aGraphic;
    sal_Int32 nIndex = 0;

    while( aStream.IsOpen() && !aStream.IsEof() )
    {
        OString aLine;
        aStream.ReadLine( aLine );
        OUString aPersonaSetting( OStringToOUString( aLine, RTL_TEXTENCODING_UTF8 ) );
        OUString aPreviewFile;
        sal_Int32 nNewIndex = aPersonaSetting.indexOf( ';' );
        if( nNewIndex < 0 )
            break;
        aPreviewFile = aPersonaSetting.copy( 0, nNewIndex );
        aPersonaSetting = aPersonaSetting.copy( nNewIndex + 1 );
        m_vDefaultPersonaSettings.push_back( aPersonaSetting );

        INetURLObject aURLObj( gallery + aPreviewFile );
        aFilter.ImportGraphic( aGraphic, aURLObj );
        Bitmap aBmp = aGraphic.GetBitmap();
        m_vDefaultPersonaImages[nIndex]->Show();
        m_vDefaultPersonaImages[nIndex++]->SetModeImage( Image( aBmp ) );
    }
}

void SvxPersonalizationTabPage::LoadExtensionThemes()
{
    // See if any extensions are used to install personas. If yes, load them.

    css::uno::Sequence<OUString> installedPersonas( officecfg::Office::Common::Misc::PersonasList::get()->getElementNames() );
    sal_Int32 nLength = installedPersonas.getLength();

    if( nLength == 0 )
        return;

    m_pPersonaList->Show();
    m_pExtensionLabel->Show();

    for( sal_Int32 nIndex = 0; nIndex < nLength; nIndex++ )
    {
        Reference< XPropertySet > xPropertySet( officecfg::Office::Common::Misc::PersonasList::get()->getByName( installedPersonas[nIndex] ), UNO_QUERY_THROW );
        OUString aPersonaName, aPreviewFile, aHeaderFile, aFooterFile, aTextColor, aAccentColor, aPersonaSettings;
        Any aValue = xPropertySet->getPropertyValue( "Name" );
        aValue >>= aPersonaName;
        m_pPersonaList->InsertEntry( aPersonaName );

        aValue = xPropertySet->getPropertyValue( "Preview" );
        aValue >>= aPreviewFile;

        aValue = xPropertySet->getPropertyValue( "Header" );
        aValue >>= aHeaderFile;

        aValue = xPropertySet->getPropertyValue( "Footer" );
        aValue >>= aFooterFile;

        aValue = xPropertySet->getPropertyValue( "TextColor" );
        aValue >>= aTextColor;

        aValue = xPropertySet->getPropertyValue( "AccentColor" );
        aValue >>= aAccentColor;

        aPersonaSettings = aPreviewFile + ";" + aHeaderFile + ";" + aFooterFile + ";" + aTextColor + ";" + aAccentColor;
        rtl::Bootstrap::expandMacros( aPersonaSettings );
        m_vExtensionPersonaSettings.push_back( aPersonaSettings );
    }
}

IMPL_LINK_NOARG_TYPED( SvxPersonalizationTabPage, SelectPersona, Button*, void )
{
    ScopedVclPtrInstance< SelectPersonaDialog > aDialog(nullptr);

    if ( aDialog->Execute() == RET_OK )
    {
        OUString aPersonaSetting( aDialog->GetAppliedPersonaSetting() );
        if ( !aPersonaSetting.isEmpty() )
        {
            SetPersonaSettings( aPersonaSetting );
        }
    }
}

IMPL_LINK_TYPED( SvxPersonalizationTabPage, ForceSelect, Button*, pButton, void )
{
    if ( pButton == m_pOwnPersona && m_aPersonaSettings.isEmpty() )
        SelectPersona( m_pSelectPersona );
}

IMPL_LINK_TYPED( SvxPersonalizationTabPage, DefaultPersona, Button*, pButton, void )
{
    m_pDefaultPersona->Check();
    for( sal_Int32 nIndex = 0; nIndex < 3; nIndex++ )
    {
        if( pButton == m_vDefaultPersonaImages[nIndex] )
            m_aPersonaSettings = m_vDefaultPersonaSettings[nIndex];
    }
}

IMPL_LINK_NOARG_TYPED( SvxPersonalizationTabPage, SelectInstalledPersona, ListBox&, void)
{
    m_pOwnPersona->Check();

    // Get the details of the selected theme.
    m_pExtensionPersonaPreview->Show();
    sal_Int32 nSelectedPos = m_pPersonaList->GetSelectEntryPos();
    OUString aSettings = m_vExtensionPersonaSettings[nSelectedPos];
    sal_Int32 nIndex = aSettings.indexOf( ';' );
    OUString aPreviewFile = aSettings.copy( 0, nIndex );
    m_aPersonaSettings = aSettings.copy( nIndex + 1 );

    // Show the preview file in the button.
    GraphicFilter aFilter;
    Graphic aGraphic;
    INetURLObject aURLObj( aPreviewFile );
    aFilter.ImportGraphic( aGraphic, aURLObj );
    Bitmap aBmp = aGraphic.GetBitmap();
    m_pExtensionPersonaPreview->SetModeImage( Image( aBmp ) );
}

/// Find the value on the Persona page, and convert it to a usable form.
static OUString searchValue( const OString &rBuffer, sal_Int32 from, const OString &rIdentifier )
{
    sal_Int32 where = rBuffer.indexOf( rIdentifier, from );
    if ( where < 0 )
        return OUString();

    where += rIdentifier.getLength();

    sal_Int32 end = rBuffer.indexOf( "&#34;", where );
    if ( end < 0 )
        return OUString();

    OString aOString( rBuffer.copy( where, end - where ) );
    OUString aString( aOString.getStr(),  aOString.getLength(), RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS );

    return aString.replaceAll( "\\/", "/" );
}

/// Parse the Persona web page, and find where to get the bitmaps + the color values.
static bool parsePersonaInfo( const OString &rBuffer, OUString *pHeaderURL, OUString *pFooterURL,
                              OUString *pTextColor, OUString *pAccentColor, OUString *pPreviewURL,
                              OUString *pName )
{
    // it is the first attribute that contains "persona="
    sal_Int32 persona = rBuffer.indexOf( "data-browsertheme=\"{" );
    if ( persona < 0 )
        return false;

    // now search inside
    *pHeaderURL = searchValue( rBuffer, persona, "&#34;headerURL&#34;:&#34;" );
    if ( pHeaderURL->isEmpty() )
        return false;

    *pFooterURL = searchValue( rBuffer, persona, "&#34;footerURL&#34;:&#34;" );
    if ( pFooterURL->isEmpty() )
        return false;

    *pTextColor = searchValue( rBuffer, persona, "&#34;textcolor&#34;:&#34;" );
    if ( pTextColor->isEmpty() )
        return false;

    *pAccentColor = searchValue( rBuffer, persona, "&#34;accentcolor&#34;:&#34;" );
    if ( pAccentColor->isEmpty() )
        return false;

    *pPreviewURL = searchValue( rBuffer, persona, "&#34;previewURL&#34;:&#34;" );
    if ( pPreviewURL->isEmpty() )
        return false;

    *pName = searchValue( rBuffer, persona, "&#34;name&#34;:&#34;" );
    if ( pName->isEmpty() )
        return false;
    return true;
}

SearchAndParseThread::SearchAndParseThread( SelectPersonaDialog* pDialog,
                          const OUString& rURL, bool bDirectURL ) :
            Thread( "cuiPersonasSearchThread" ),
            m_pPersonaDialog( pDialog ),
            m_aURL( rURL ),
            m_bExecute( true ),
            m_bDirectURL( bDirectURL )
{
}

SearchAndParseThread::~SearchAndParseThread()
{
}

namespace {

bool getPreviewFile( const OUString& rURL, OUString *pPreviewFile, OUString *pPersonaSetting )
{
    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
    if ( !xFileAccess.is() )
        return false;

    Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference< io::XInputStream > xStream;
    try {
        css:: uno::Reference< task::XInteractionHandler > xIH(
            css::task::InteractionHandler::createWithParent( xContext, nullptr ) );

        xFileAccess->setInteractionHandler( new comphelper::SimpleFileAccessInteraction( xIH ) );
        xStream = xFileAccess->openFileRead( rURL );

        if( !xStream.is() )
            return false;
    }
    catch (...)
    {
        return false;
    }

    // read the persona specification
    // NOTE: Parsing for real is an overkill here; and worse - I tried, and
    // the HTML the site provides is not 100% valid ;-)
    const sal_Int32 BUF_LEN = 8000;
    uno::Sequence< sal_Int8 > buffer( BUF_LEN );
    OStringBuffer aBuffer( 64000 );

    sal_Int32 nRead = 0;
    while ( ( nRead = xStream->readBytes( buffer, BUF_LEN ) ) == BUF_LEN )
        aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );

    if ( nRead > 0 )
        aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );

    xStream->closeInput();

    // get the important bits of info
    OUString aHeaderURL, aFooterURL, aTextColor, aAccentColor, aPreviewURL, aName;

    if ( !parsePersonaInfo( aBuffer.makeStringAndClear(), &aHeaderURL, &aFooterURL, &aTextColor, &aAccentColor, &aPreviewURL, &aName ) )
        return false;

    // copy the images to the user's gallery
    OUString gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( gallery );
    gallery += "/user/gallery/personas/" + aName + "/";
    osl::Directory::createPath( gallery );

    OUString aPreviewFile( INetURLObject( aPreviewURL ).getName() );

    try {
        xFileAccess->copy( aPreviewURL, gallery + aPreviewFile );
    }
    catch ( const uno::Exception & )
    {
        return false;
    }
    *pPreviewFile = gallery + aPreviewFile;
    *pPersonaSetting = aName + ";" + aHeaderURL + ";" + aFooterURL + ";" + aTextColor + ";" + aAccentColor;
    return true;
}

}

void SearchAndParseThread::execute()
{
    if( m_aURL.startsWith( "https://" ) )
    {
        m_pPersonaDialog->ClearSearchResults();
        OUString sProgress( CUI_RES( RID_SVXSTR_SEARCHING ) ), sError;
        m_pPersonaDialog->SetProgress( sProgress );

        PersonasDocHandler* pHandler = new PersonasDocHandler();
        Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
        Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);
        Reference< xml::sax::XDocumentHandler > xDocHandler = pHandler;
        uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
        uno::Reference< io::XInputStream > xStream;
        xParser->setDocumentHandler( xDocHandler );

        if( !m_bDirectURL )
        {
            if ( !xFileAccess.is() )
                return;

            try {
                css:: uno::Reference< task::XInteractionHandler > xIH(
                    css::task::InteractionHandler::createWithParent( xContext, nullptr ) );

                xFileAccess->setInteractionHandler( new comphelper::SimpleFileAccessInteraction( xIH ) );

                xStream = xFileAccess->openFileRead( m_aURL );
                if( !xStream.is() )
                {
                    // in case of a returned CommandFailedException
                    // SimpleFileAccess serves it, returning an empty stream
                    sError = CUI_RES(RID_SVXSTR_SEARCHERROR);
                    sError = sError.replaceAll("%1", m_aURL);
                    m_pPersonaDialog->SetProgress( OUString() );
                    ScopedVclPtrInstance< ErrorBox > aBox( nullptr, WB_OK, sError);
                    aBox->Execute();
                    return;
                }
            }
            catch (...)
            {
                // a catch all clause, in case the exception is not
                // served elsewhere
                sError = CUI_RES(RID_SVXSTR_SEARCHERROR);
                sError = sError.replaceAll("%1", m_aURL);
                m_pPersonaDialog->SetProgress( OUString() );
                ScopedVclPtrInstance< ErrorBox > aBox( nullptr, WB_OK, sError );
                aBox->Execute();
                return;
            }

            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream = xStream;
            xParser->parseStream( aParserInput );

            if( !pHandler->hasResults() )
            {
                sProgress = CUI_RES( RID_SVXSTR_NORESULTS );
                m_pPersonaDialog->SetProgress( sProgress );
                return;
            }
        }

        std::vector<OUString> vLearnmoreURLs;
        std::vector<OUString>::iterator it;
        sal_Int32 nIndex = 0;
        GraphicFilter aFilter;
        Graphic aGraphic;

        if( !m_bDirectURL )
            vLearnmoreURLs = pHandler->getLearnmoreURLs();
        else
            vLearnmoreURLs.push_back( m_aURL );

        for( it = vLearnmoreURLs.begin(); it!=vLearnmoreURLs.end() && nIndex < MAX_RESULTS; ++it )
        {
            OUString sPreviewFile, aPersonaSetting;
            bool bResult = getPreviewFile( *it, &sPreviewFile, &aPersonaSetting );
            // parsing is buggy at times, as HTML is not proper. Skip it.
            if(aPersonaSetting.isEmpty() || !bResult)
            {
                if( m_bDirectURL )
                {
                    sError = CUI_RES(RID_SVXSTR_SEARCHERROR);
                    sError = sError.replaceAll("%1", m_aURL);
                    m_pPersonaDialog->SetProgress( OUString() );
                    ScopedVclPtrInstance< ErrorBox > aBox( nullptr, WB_OK, sError);
                    aBox->Execute();
                    return;
                }
                continue;
            }
            INetURLObject aURLObj( sPreviewFile );
            aFilter.ImportGraphic( aGraphic, aURLObj );
            Bitmap aBmp = aGraphic.GetBitmap();

            if( !m_bExecute )
                return;

            // for VCL to be able to do visual changes in the thread
            SolarMutexGuard aGuard;
            m_pPersonaDialog->SetImages( Image( aBmp ), nIndex++ );
            m_pPersonaDialog->setOptimalLayoutSize();
            m_pPersonaDialog->AddPersonaSetting( aPersonaSetting );
        }

        if( !m_bExecute )
            return;

        SolarMutexGuard aGuard;
        sProgress.clear();
        m_pPersonaDialog->SetProgress( sProgress );
        m_pPersonaDialog->setOptimalLayoutSize();
    }

    else
    {
        OUString sProgress( CUI_RES( RID_SVXSTR_APPLYPERSONA ) ), sError;
        m_pPersonaDialog->SetProgress( sProgress );

        uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
        if ( !xFileAccess.is() )
            return;

        OUString aName, aHeaderURL, aFooterURL, aTextColor, aAccentColor;
        OUString aPersonaSetting;

        // get the required fields from m_aURL
        sal_Int32 nOldIndex = 0;
        sal_Int32 nNewIndex = m_aURL.indexOf( ';', nOldIndex );
        aName = m_aURL.copy( nOldIndex, ( nNewIndex - nOldIndex ) );

        nOldIndex = nNewIndex + 1;
        nNewIndex = m_aURL.indexOf( ';', nOldIndex );
        aHeaderURL = m_aURL.copy(nOldIndex , ( nNewIndex - nOldIndex ) );

        nOldIndex = nNewIndex + 1;
        nNewIndex = m_aURL.indexOf( ';', nOldIndex );
        aFooterURL = m_aURL.copy( nOldIndex,  ( nNewIndex - nOldIndex ) );

        nOldIndex = nNewIndex + 1;
        nNewIndex = m_aURL.indexOf( ';', nOldIndex );
        aTextColor = m_aURL.copy( nOldIndex, ( nNewIndex - nOldIndex ) );

        nOldIndex = nNewIndex + 1;
        nNewIndex = m_aURL.getLength();
        aAccentColor = m_aURL.copy( nOldIndex, ( nNewIndex - nOldIndex ) );

        // copy the images to the user's gallery
        OUString gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
        rtl::Bootstrap::expandMacros( gallery );
        gallery += "/user/gallery/personas/";
        osl::Directory::createPath( gallery );

        OUString aHeaderFile( INetURLObject( aHeaderURL ).getName() );
        OUString aFooterFile( INetURLObject( aFooterURL ).getName() );

        aHeaderFile = aName + "/" + aHeaderFile;
        aFooterFile = aName + "/" + aFooterFile;

        try {
            xFileAccess->copy( aHeaderURL, gallery + aHeaderFile );
            xFileAccess->copy( aFooterURL, gallery + aFooterFile );
        }
        catch ( const uno::Exception & )
        {
            sError = CUI_RES( RID_SVXSTR_SEARCHERROR );
            sError = sError.replaceAll("%1", m_aURL);
            m_pPersonaDialog->SetProgress( OUString() );
            ScopedVclPtrInstance< ErrorBox > aBox( nullptr, WB_OK, sError);
            aBox->Execute();
            return;
        }

        if( !m_bExecute )
            return;

        SolarMutexGuard aGuard;

        aPersonaSetting = aHeaderFile + ";" + aFooterFile + ";" + aTextColor + ";" + aAccentColor;
        m_pPersonaDialog->SetAppliedPersonaSetting( aPersonaSetting );
        m_pPersonaDialog->EndDialog( RET_OK );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
