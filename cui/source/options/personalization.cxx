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

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/weld.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/mnemonic.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <ucbhelper/content.hxx>
#include <comphelper/simplefileaccessinteraction.hxx>

#include <curl/curl.h>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

#include <vector>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;

#ifdef UNX
static const char kUserAgent[] = "LibreOffice PersonaDownloader/1.0 (Linux)";
#else
static const char kUserAgent[] = "LibreOffice PersonaDownloader/1.0 (unknown platform)";
#endif

struct PersonaInfo
{
    OUString sSlug;
    OUString sName;
    OUString sPreviewURL;
    OUString sHeaderURL;
    OUString sFooterURL;
    OUString sTextColor;
};

namespace {

// Callback to get the response data from server.
size_t WriteCallback(void *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  std::string* response = static_cast<std::string *>(userp);
  size_t real_size = size * nmemb;
  response->append(static_cast<char *>(ptr), real_size);
  return real_size;
}

// Callback to get the response data from server to a file.
size_t WriteCallbackFile(void *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  SvStream* response = static_cast<SvStream *>(userp);
  size_t real_size = size * nmemb;
  response->WriteBytes(ptr, real_size);
  return real_size;
}

// Gets the content of the given URL and returns as a standard string
std::string curlGet(const OString& rURL)
{
    CURL* curl = curl_easy_init();

    if (!curl)
            return std::string();

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);

    std::string response_body;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                    static_cast<void *>(&response_body));

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        SAL_WARN("cui.options", "Download failed. Error code: " << http_code);
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("cui.options", "curl error: " << cc);
    }

    return response_body;
}

// Downloads and saves the file at the given rURL to a local path (sFileURL)
void curlDownload(const OString& rURL, const OUString& sFileURL)
{
    CURL* curl = curl_easy_init();
    SvFileStream aFile( sFileURL, StreamMode::WRITE );

    if (!curl)
            return;

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                    static_cast<void *>(&aFile));

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        SAL_WARN("cui.options", "Download failed. Error code: " << http_code);
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("cui.options", "curl error: " << cc);
    }
}

} //End of anonymous namespace

SelectPersonaDialog::SelectPersonaDialog( vcl::Window *pParent )
    : ModalDialog( pParent, "SelectPersonaDialog", "cui/ui/select_persona_dialog.ui" )
{
    get( m_pSearchButton, "search_personas" );
    m_pSearchButton->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_pEdit, "search_term" );

    get( m_pCategories, "categoriesCB" );
    m_pCategories->SetSelectHdl( LINK( this, SelectPersonaDialog, SelectCategory ) );

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

    m_pCategories->SelectEntry("Featured");
    m_pCategories->GetSelectHdl().Call(*m_pCategories);
}

SelectPersonaDialog::~SelectPersonaDialog()
{
    disposeOnce();
}

void SelectPersonaDialog::dispose()
{
    if (m_pSearchThread.is())
    {
        // Release the solar mutex, so the thread is not affected by the race
        // when it's after the m_bExecute check but before taking the solar
        // mutex.
        SolarMutexReleaser aReleaser;
        m_pSearchThread->join();
    }

    m_pCategories.clear();
    m_pEdit.clear();
    m_pSearchButton.clear();
    m_pProgressLabel.clear();
    for (VclPtr<PushButton>& vp : m_vResultList)
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

IMPL_LINK_NOARG( SelectPersonaDialog, SearchPersonas, Button*, void )
{
    OUString searchTerm = m_pEdit->GetText();

    if( searchTerm.isEmpty( ) )
        return;

    if( m_pSearchThread.is() )
        m_pSearchThread->StopExecution();

    // Direct url of a persona given
    if ( searchTerm.startsWith( "https://addons.mozilla.org/" ) )
    {
        OUString sSlug = searchTerm.getToken( 6, '/' );

        // Check if we got the slug
        if ( sSlug.isEmpty() )
        {
            SolarMutexGuard aGuard;
            OUString sError = CuiResId( RID_SVXSTR_INVALIDPERSONAURL );
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                                                                       VclMessageType::Error, VclButtonsType::Ok,
                                                                                       sError));
            xBox->run();
            return;
        }

        // Remove invalid characters
        searchTerm = searchTerm.replaceAll("?", "");

        m_pSearchThread = new SearchAndParseThread( this, sSlug, true );
    }
    // Search term given
    else
    {
        // Remove invalid characters
        searchTerm = searchTerm.replaceAll("/", "");
        searchTerm = searchTerm.replaceAll("?", "");

        // 15 results so that invalid and duplicate search results whose names, textcolors etc. are null can be skipped
        OUString rSearchURL = "https://addons.mozilla.org/api/v3/addons/search/?q=" + searchTerm + "&type=persona&page_size=15";

        m_pSearchThread = new SearchAndParseThread( this, rSearchURL, false );
    }

    m_pSearchThread->launch();
}

IMPL_LINK_NOARG( SelectPersonaDialog, ActionOK, Button*, void )
{
    OUString aSelectedPersona = GetSelectedPersona();

    if( !aSelectedPersona.isEmpty() )
    {
        m_pGetPersonaThread = new GetPersonaThread( this, aSelectedPersona );
        m_pGetPersonaThread->launch();
    }

    else
    {
        if ( m_pSearchThread.is() )
            m_pSearchThread->StopExecution();

        EndDialog( RET_OK );
    }
}

IMPL_LINK_NOARG( SelectPersonaDialog, ActionCancel, Button*, void )
{
    if( m_pSearchThread.is() )
        m_pSearchThread->StopExecution();
    if( m_pGetPersonaThread.is() )
        m_pGetPersonaThread->StopExecution();

    EndDialog();
}

IMPL_LINK_NOARG( SelectPersonaDialog, SelectCategory, ListBox&, void )
{
    OUString searchTerm = *static_cast<OUString*>(m_pCategories->GetSelectedEntryData());
    OUString rSearchURL;

    if (searchTerm.isEmpty())
        return;

    if( m_pSearchThread.is() )
        m_pSearchThread->StopExecution();

    // 15 results so that invalid and duplicate search results whose names, textcolors etc. are null can be skipped
    if (searchTerm == "featured")
        rSearchURL = "https://addons.mozilla.org/api/v3/addons/search/?type=persona&app=firefox&status=public&sort=users&featured=true&page_size=15";
    else
        rSearchURL = "https://addons.mozilla.org/api/v3/addons/search/?type=persona&app=firefox&category=" + searchTerm + "&status=public&sort=downloads&page_size=15";

    m_pSearchThread = new SearchAndParseThread( this, rSearchURL, false );

    m_pSearchThread->launch();
}

IMPL_LINK( SelectPersonaDialog, SelectPersona, Button*, pButton, void )
{
    if( m_pSearchThread.is() )
        m_pSearchThread->StopExecution();
    if ( m_pGetPersonaThread.is() )
        return;

    for( sal_Int32 index = 0; index < MAX_RESULTS; index++ )
    {
        if( pButton == m_vResultList[index] )
        {
            if( !m_vPersonaSettings[index].isEmpty() )
            {
                m_aSelectedPersona = m_vPersonaSettings[index];

                // get the persona name from the setting variable to show in the progress.
                OUString aName( m_aSelectedPersona.getToken( 1, ';' ) );
                OUString aProgress( CuiResId(RID_SVXSTR_SELECTEDPERSONA) + aName );

                SetProgress( aProgress );
            }
            break;
        }
    }
}

void SelectPersonaDialog::SetAppliedPersonaSetting( OUString const & rPersonaSetting )
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

void SelectPersonaDialog::SetImages( const Image& aImage, const OUString& sName, const sal_Int32& nIndex )
{
    m_vResultList[nIndex]->Enable();
    m_vResultList[nIndex]->SetModeImage( aImage );
    m_vResultList[nIndex]->SetQuickHelpText( sName );
}

void SelectPersonaDialog::AddPersonaSetting( OUString const & rPersonaSetting )
{
    m_vPersonaSettings.push_back( rPersonaSetting );
}

void SelectPersonaDialog::ClearSearchResults()
{
    // for VCL to be able to destroy bitmaps
    SolarMutexGuard aGuard;
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
    get( m_pAppliedThemeLabel, "applied_theme_link" );

    get( m_pOwnPersona, "own_persona" );
    m_pOwnPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, ForceSelect ) );

    get( m_pSelectPersona, "select_persona" );
    m_pSelectPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, SelectPersona ) );

    for (sal_uInt32 i = 0; i < MAX_DEFAULT_PERSONAS; ++i)
    {
        OUString sDefaultId("default" + OUString::number(i));
        get( m_vDefaultPersonaImages[i], OUStringToOString(sDefaultId, RTL_TEXTENCODING_UTF8) );
        m_vDefaultPersonaImages[i]->SetClickHdl( LINK( this, SvxPersonalizationTabPage, DefaultPersona ) );
    }

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


VclPtr<SfxTabPage> SvxPersonalizationTabPage::Create( TabPageParent pParent, const SfxItemSet *rSet )
{
    return VclPtr<SvxPersonalizationTabPage>::Create( pParent.pParent, *rSet );
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
    OUString aSlug, aName;
    sal_Int32 nIndex = 0;

    aSlug = aPersonaSetting.getToken( 0, ';', nIndex );

    if ( nIndex > 0 )
        aName = "(" + aPersonaSetting.getToken( 0, ';', nIndex ) + ")";

    if ( !aName.isEmpty() )
    {
        m_pAppliedThemeLabel->SetText( aName );
        m_pAppliedThemeLabel->SetURL( "https://addons.mozilla.org/en-US/firefox/addon/" + aSlug + "/" );
        m_pAppliedThemeLabel->Show();
    }
    else
    {
        SAL_WARN("cui.options", "Applied persona doesn't have a name!");
    }
}

void SvxPersonalizationTabPage::LoadDefaultImages()
{
    // Load the pre saved personas

    OUString gallery
        = "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/gallery/personas/";
    rtl::Bootstrap::expandMacros( gallery );
    OUString aPersonasList = gallery + "personas_list.txt";
    SvFileStream aStream( aPersonasList, StreamMode::READ );
    GraphicFilter aFilter;
    Graphic aGraphic;
    sal_Int32 nIndex = 0;
    bool foundOne = false;

    while( aStream.IsOpen() && !aStream.eof() && nIndex < MAX_DEFAULT_PERSONAS )
    {
        OString aLine;
        OUString aPersonaSetting, aPreviewFile, aName;
        sal_Int32 nParseIndex = 0;

        aStream.ReadLine( aLine );
        aPersonaSetting = OStringToOUString( aLine, RTL_TEXTENCODING_UTF8 );
        aName = aPersonaSetting.getToken( 1, ';', nParseIndex );
        aPreviewFile = aPersonaSetting.getToken( 0, ';', nParseIndex );

        if (aPreviewFile.isEmpty())
            break;

        m_vDefaultPersonaSettings.push_back( aPersonaSetting );

        INetURLObject aURLObj( gallery + aPreviewFile );
        aFilter.ImportGraphic( aGraphic, aURLObj );
        BitmapEx aBmp = aGraphic.GetBitmapEx();
        m_vDefaultPersonaImages[nIndex]->SetModeImage( Image( aBmp ) );
        m_vDefaultPersonaImages[nIndex]->SetQuickHelpText(aName);
        m_vDefaultPersonaImages[nIndex++]->Show();
        foundOne = true;
    }

    m_pDefaultPersona->Enable(foundOne);
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
        OUString aPersonaSlug, aPersonaName, aPreviewFile, aHeaderFile, aFooterFile, aTextColor, aPersonaSettings;

        Any aValue = xPropertySet->getPropertyValue( "Slug" );
        aValue >>= aPersonaSlug;

        aValue = xPropertySet->getPropertyValue( "Name" );
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

        aPersonaSettings = aPersonaSlug + ";" + aPersonaName + ";" + aPreviewFile
                + ";" + aHeaderFile + ";" + aFooterFile + ";" + aTextColor;
        rtl::Bootstrap::expandMacros( aPersonaSettings );
        m_vExtensionPersonaSettings.push_back( aPersonaSettings );
    }
}

IMPL_LINK_NOARG( SvxPersonalizationTabPage, SelectPersona, Button*, void )
{
    m_pOwnPersona->Check();
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

IMPL_LINK( SvxPersonalizationTabPage, ForceSelect, Button*, pButton, void )
{
    if ( pButton == m_pOwnPersona && m_aPersonaSettings.isEmpty() )
        SelectPersona( m_pSelectPersona );
}

IMPL_LINK( SvxPersonalizationTabPage, DefaultPersona, Button*, pButton, void )
{
    m_pDefaultPersona->Check();
    for( sal_Int32 nIndex = 0; nIndex < MAX_DEFAULT_PERSONAS; ++nIndex )
    {
        if( pButton == m_vDefaultPersonaImages[nIndex] )
            m_aPersonaSettings = m_vDefaultPersonaSettings[nIndex];
    }
}

IMPL_LINK_NOARG( SvxPersonalizationTabPage, SelectInstalledPersona, ListBox&, void)
{
    m_pOwnPersona->Check();

    // Get the details of the selected theme.
    m_pExtensionPersonaPreview->Show();
    sal_Int32 nSelectedPos = m_pPersonaList->GetSelectedEntryPos();
    OUString aSettings = m_vExtensionPersonaSettings[nSelectedPos];
    sal_Int32 nIndex = aSettings.indexOf( ';' );
    OUString aPreviewFile = aSettings.copy( 0, nIndex );
    m_aPersonaSettings = aSettings.copy( nIndex + 1 );

    // Show the preview file in the button.
    GraphicFilter aFilter;
    Graphic aGraphic;
    INetURLObject aURLObj( aPreviewFile );
    aFilter.ImportGraphic( aGraphic, aURLObj );
    BitmapEx aBmp = aGraphic.GetBitmapEx();
    m_pExtensionPersonaPreview->SetModeImage( Image( aBmp ) );
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

bool getPreviewFile( const PersonaInfo& aPersonaInfo, OUString& pPreviewFile )
{
    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
    Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

    // copy the images to the user's gallery
    OUString gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( gallery );
    gallery += "/user/gallery/personas/" + aPersonaInfo.sSlug + "/";

    OUString aPreviewFile( INetURLObject( aPersonaInfo.sPreviewURL ).getName() );
    OString aPreviewURL = OUStringToOString( aPersonaInfo.sPreviewURL, RTL_TEXTENCODING_UTF8 );

    try {
        osl::Directory::createPath( gallery );

        if ( !xFileAccess->exists( gallery + aPreviewFile ) )
            curlDownload(aPreviewURL, gallery + aPreviewFile);
    }
    catch ( const uno::Exception & )
    {
        return false;
    }
    pPreviewFile = gallery + aPreviewFile;
    return true;
}

void parseResponse(const std::string& rResponse, std::vector<PersonaInfo> & aPersonas)
{
    orcus::json::document_tree aJsonDoc;
    orcus::json_config aConfig;

    if (rResponse.empty())
        return;

    aJsonDoc.load(rResponse, aConfig);

    auto aDocumentRoot = aJsonDoc.get_document_root();
    if (aDocumentRoot.type() != orcus::json::node_t::object)
    {
        SAL_WARN("cui.options", "invalid root entries: " << rResponse);
        return;
    }

    auto resultsArray = aDocumentRoot.child("results");

    for (size_t i = 0; i < resultsArray.child_count(); ++i)
    {
        auto arrayElement = resultsArray.child(i);

        try
        {
            PersonaInfo aNewPersona = {
                OStringToOUString( OString(arrayElement.child("slug").string_value().get()),
                    RTL_TEXTENCODING_UTF8 ),
                OStringToOUString( OString(arrayElement.child("name").child("en-US").string_value().get()),
                    RTL_TEXTENCODING_UTF8 ),
                OStringToOUString( OString(arrayElement.child("theme_data").child("previewURL").string_value().get()),
                    RTL_TEXTENCODING_UTF8 ),
                OStringToOUString( OString(arrayElement.child("theme_data").child("headerURL").string_value().get()),
                    RTL_TEXTENCODING_UTF8 ),
                OStringToOUString( OString(arrayElement.child("theme_data").child("footerURL").string_value().get()),
                    RTL_TEXTENCODING_UTF8 ),
                OStringToOUString( OString(arrayElement.child("theme_data").child("textcolor").string_value().get()),
                    RTL_TEXTENCODING_UTF8 )
            };

            aPersonas.push_back(aNewPersona);
        }
        catch(orcus::json::document_error& e)
        {
            // This usually happens when one of the values is null (type() == orcus::json::node_t::null)
            // TODO: Allow null values in personas.
            SAL_WARN("cui.options", "Persona JSON parse error: " << e.what());
        }
    }
}

PersonaInfo parseSingleResponse(const std::string& rResponse)
{
    orcus::json::document_tree aJsonDoc;
    orcus::json_config aConfig;

    if (rResponse.empty())
        return PersonaInfo();

    aJsonDoc.load(rResponse, aConfig);

    auto aDocumentRoot = aJsonDoc.get_document_root();
    if (aDocumentRoot.type() != orcus::json::node_t::object)
    {
        SAL_WARN("cui.options", "invalid root entries: " << rResponse);
        return PersonaInfo();
    }

    try
    {
        auto theme_data = aDocumentRoot.child("theme_data");

        PersonaInfo aNewPersona = {
            OUString(),
            OStringToOUString( OString(theme_data.child("name").string_value().get()),
            RTL_TEXTENCODING_UTF8 ),
            OStringToOUString( OString(theme_data.child("previewURL").string_value().get()),
            RTL_TEXTENCODING_UTF8 ),
            OStringToOUString( OString(theme_data.child("headerURL").string_value().get()),
            RTL_TEXTENCODING_UTF8 ),
            OStringToOUString( OString(theme_data.child("footerURL").string_value().get()),
            RTL_TEXTENCODING_UTF8 ),
            OStringToOUString( OString(theme_data.child("textcolor").string_value().get()),
            RTL_TEXTENCODING_UTF8 )
        };

        return aNewPersona;
    }
    catch(orcus::json::document_error& e)
    {
        // This usually happens when one of the values is null (type() == orcus::json::node_t::null)
        // TODO: Allow null values in personas.
        // TODO: Give a message to the user
        SAL_WARN("cui.options", "Persona JSON parse error: " << e.what());
        return PersonaInfo();
    }
}

} //End of anonymous namespace

void SearchAndParseThread::execute()
{
    m_pPersonaDialog->ClearSearchResults();
    OUString sProgress( CuiResId( RID_SVXSTR_SEARCHING ) );
    m_pPersonaDialog->SetProgress( sProgress );

    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );

    if (!m_bDirectURL)
    {
        OString rURL = OUStringToOString( m_aURL, RTL_TEXTENCODING_UTF8 );
        std::string sResponse = curlGet(rURL);

        std::vector<PersonaInfo> personaInfos;

        parseResponse(sResponse, personaInfos);

        if ( personaInfos.empty() )
        {
            sProgress = CuiResId( RID_SVXSTR_NORESULTS );
            m_pPersonaDialog->SetProgress( sProgress );
            return;
        }
        else
        {
            //Get Preview Files
            sal_Int32 nIndex = 0;
            for (const auto & personaInfo : personaInfos)
            {
                if( !m_bExecute )
                    return;

                OUString aPreviewFile;
                bool bResult = getPreviewFile(personaInfo, aPreviewFile);

                if (!bResult)
                {
                    SAL_INFO("cui.options", "Couldn't get the preview file. Skipping: " << aPreviewFile);
                    continue;
                }


                GraphicFilter aFilter;
                Graphic aGraphic;

                INetURLObject aURLObj( aPreviewFile );

                OUString aPersonaSetting = personaInfo.sSlug
                        + ";" + personaInfo.sName
                        + ";" + personaInfo.sPreviewURL
                        + ";" + personaInfo.sHeaderURL
                        + ";" + personaInfo.sFooterURL
                        + ";" + personaInfo.sTextColor;

                m_pPersonaDialog->AddPersonaSetting( aPersonaSetting );

                // for VCL to be able to create bitmaps / do visual changes in the thread
                SolarMutexGuard aGuard;
                aFilter.ImportGraphic( aGraphic, aURLObj );
                BitmapEx aBmp = aGraphic.GetBitmapEx();

                m_pPersonaDialog->SetImages( Image( aBmp ), personaInfo.sName, nIndex );
                m_pPersonaDialog->setOptimalLayoutSize();

                if (++nIndex >= MAX_RESULTS)
                    break;
            }

            //TODO: Give a message to the user if nIndex == 0
        }
    }
    else
    {
        //Now we have a slug instead a search term in m_aURL
        OString rURL = "https://addons.mozilla.org/api/v3/addons/addon/"
                     + OUStringToOString( m_aURL, RTL_TEXTENCODING_UTF8 )
                     + "/";
        std::string sResponse = curlGet(rURL);

        PersonaInfo aPersonaInfo = parseSingleResponse(sResponse);
        aPersonaInfo.sSlug = m_aURL;

        if ( aPersonaInfo.sName.isEmpty() )
        {
            //TODO: Give error message to user
            sProgress = CuiResId( RID_SVXSTR_NORESULTS );
            m_pPersonaDialog->SetProgress( sProgress );
            return;
        }
        else
        {
            //Get the preview file
            if( !m_bExecute )
                return;

            OUString aPreviewFile;
            bool bResult = getPreviewFile(aPersonaInfo, aPreviewFile);

            if (!bResult)
            {
                //TODO: Inform the user
                SAL_WARN("cui.options", "Couldn't get the preview file: " << aPreviewFile);
                return;
            }

            GraphicFilter aFilter;
            Graphic aGraphic;

            INetURLObject aURLObj( aPreviewFile );

            OUString aPersonaSetting = aPersonaInfo.sSlug
                    + ";" + aPersonaInfo.sName
                    + ";" + aPersonaInfo.sPreviewURL
                    + ";" + aPersonaInfo.sHeaderURL
                    + ";" + aPersonaInfo.sFooterURL
                    + ";" + aPersonaInfo.sTextColor;

            m_pPersonaDialog->AddPersonaSetting( aPersonaSetting );

            // for VCL to be able to create bitmaps / do visual changes in the thread
            SolarMutexGuard aGuard;
            aFilter.ImportGraphic( aGraphic, aURLObj );
            BitmapEx aBmp = aGraphic.GetBitmapEx();

            m_pPersonaDialog->SetImages( Image( aBmp ), aPersonaInfo.sName, 0 );
            m_pPersonaDialog->setOptimalLayoutSize();
        }

    }

    if( !m_bExecute )
        return;

    SolarMutexGuard aGuard;
    sProgress.clear();
    m_pPersonaDialog->SetProgress( sProgress );
    m_pPersonaDialog->setOptimalLayoutSize();
}

GetPersonaThread::GetPersonaThread( SelectPersonaDialog* pDialog,
                          const OUString& rSelectedPersona ) :
            Thread( "cuiPersonasGetPersonaThread" ),
            m_pPersonaDialog( pDialog ),
            m_aSelectedPersona( rSelectedPersona ),
            m_bExecute( true )
{
}

GetPersonaThread::~GetPersonaThread()
{
    //TODO: Clean-up
}

void GetPersonaThread::execute()
{
    OUString sProgress( CuiResId( RID_SVXSTR_APPLYPERSONA ) ), sError;
    m_pPersonaDialog->SetProgress( sProgress );

    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
    if ( !xFileAccess.is() )
        return;

    OUString aSlug, aName, aPreviewURL, aHeaderURL, aFooterURL, aTextColor;
    OUString aPersonaSetting;

    // get the required fields from m_aSelectedPersona
    sal_Int32 nIndex = 0;

    aSlug = m_aSelectedPersona.getToken(0, ';', nIndex);
    aName = m_aSelectedPersona.getToken(0, ';', nIndex);
    aPreviewURL = m_aSelectedPersona.getToken(0, ';', nIndex);
    aHeaderURL = m_aSelectedPersona.getToken(0, ';', nIndex);
    aFooterURL = m_aSelectedPersona.getToken(0, ';', nIndex);
    aTextColor = m_aSelectedPersona.getToken(0, ';', nIndex);

    // copy the images to the user's gallery
    OUString gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( gallery );
    gallery += "/user/gallery/personas/";

    OUString aPreviewFile( aSlug + "/" + INetURLObject( aPreviewURL ).getName() );
    OUString aHeaderFile( aSlug + "/" + INetURLObject( aHeaderURL ).getName() );
    OUString aFooterFile( aSlug + "/" + INetURLObject( aFooterURL ).getName() );

    try {
        osl::Directory::createPath( gallery );

        if ( !xFileAccess->exists(gallery + aHeaderFile) )
            curlDownload( OUStringToOString(aHeaderURL, RTL_TEXTENCODING_UTF8), gallery + aHeaderFile );

        if ( !xFileAccess->exists(gallery + aFooterFile) )
            curlDownload( OUStringToOString(aFooterURL, RTL_TEXTENCODING_UTF8), gallery + aFooterFile );
    }
    catch ( const uno::Exception & )
    {
        SolarMutexGuard aGuard;
        sError = CuiResId( RID_SVXSTR_SEARCHERROR );
        sError = sError.replaceAll("%1", m_aSelectedPersona);
        m_pPersonaDialog->SetProgress( OUString() );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                                                                   VclMessageType::Error, VclButtonsType::Ok,
                                                                                   sError));
        xBox->run();
        return;
    }

    if( !m_bExecute )
        return;

    SolarMutexGuard aGuard;

    aPersonaSetting = aSlug + ";" + aName + ";" + aPreviewFile + ";" + aHeaderFile + ";" + aFooterFile
            + ";" + aTextColor;

    m_pPersonaDialog->SetAppliedPersonaSetting( aPersonaSetting );
    m_pPersonaDialog->EndDialog( RET_OK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
