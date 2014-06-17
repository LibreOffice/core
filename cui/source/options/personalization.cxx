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
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/urlobj.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/graphicfilter.hxx>

#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include "ucbhelper/content.hxx"

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

SelectPersonaDialog::SelectPersonaDialog( Window *pParent )
    : ModalDialog( pParent, "SelectPersonaDialog", "cui/ui/select_persona_dialog.ui" )
{
    get( m_pSearchButton, "search_personas" );
    m_pSearchButton->SetClickHdl( LINK( this, SelectPersonaDialog, SearchPersonas ) );

    get( m_pEdit, "search_term" );
    m_pEdit->SetPlaceholderText( "Search term..." );

    get( m_pProgressLabel, "progress_label" );

    get( m_pOkButton, "ok" );
    m_pOkButton->SetClickHdl( LINK( this, SelectPersonaDialog, ActionOK ) );

    get( m_pCancelButton, "cancel" );
    m_pCancelButton->SetClickHdl( LINK( this, SelectPersonaDialog, ActionCancel ) );

    get(m_vResultList[0], "result1");
    m_vResultList[0]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[1], "result2");
    m_vResultList[1]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[2], "result3");
    m_vResultList[2]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[3], "result4");
    m_vResultList[3]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[4], "result5");
    m_vResultList[4]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[5], "result6");
    m_vResultList[5]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[6], "result7");
    m_vResultList[6]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[7], "result8");
    m_vResultList[7]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );

    get(m_vResultList[8], "result9");
    m_vResultList[8]->SetClickHdl( LINK( this, SelectPersonaDialog, SelectPersona ) );
}

OUString SelectPersonaDialog::GetSelectedPersona() const
{
    if( !m_aSelectedPersona.isEmpty( ) )
        return m_aSelectedPersona;

    return OUString();
}

IMPL_LINK( SelectPersonaDialog, SearchPersonas, PushButton*, /*pButton*/ )
{
    OUString searchTerm = m_pEdit->GetText();
    if( searchTerm.isEmpty( ) )
        return 0;

    OUString rSearchURL = "https://addons.allizom.org/en-US/firefox/api/1.5/search/" + searchTerm + "/9/9";
    m_rSearchThread = new SearchAndParseThread( this, rSearchURL );
    m_rSearchThread->launch();
    return 0;
}

IMPL_LINK( SelectPersonaDialog, ActionOK, PushButton*, /* pButton */ )
{
    OUString aSelectedPersona = GetSelectedPersona();

    if( !aSelectedPersona.isEmpty() )
    {
        m_rSearchThread = new SearchAndParseThread( this, aSelectedPersona );
        m_rSearchThread->launch();
    }

    else
        EndDialog( RET_OK );
    return 0;
}

IMPL_LINK( SelectPersonaDialog, ActionCancel, PushButton*, /* pButton */ )
{
    if( m_rSearchThread.is() )
        m_rSearchThread->terminate();

    EndDialog( RET_CANCEL );
    return 0;
}

IMPL_LINK( SelectPersonaDialog, SelectPersona, PushButton*, pButton )
{
    if( pButton == m_vResultList[0] )
    {
        if( !m_vPersonaSettings[0].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[0];
    }

    else if( pButton == m_vResultList[1] )
    {
        if( !m_vPersonaSettings[1].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[1];
    }

    else if( pButton == m_vResultList[2] )
    {
        if( !m_vPersonaSettings[2].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[2];
    }

    else if( pButton == m_vResultList[3] )
    {
        if( !m_vPersonaSettings[3].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[3];
    }

    else if( pButton == m_vResultList[4] )
    {
        if( !m_vPersonaSettings[4].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[4];
    }

    else if( pButton == m_vResultList[5] )
    {
        if( !m_vPersonaSettings[5].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[5];
    }

    else if( pButton == m_vResultList[6] )
    {
        if( !m_vPersonaSettings[6].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[6];
    }

    else if( pButton == m_vResultList[7] )
    {
        if( !m_vPersonaSettings[7].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[7];
    }

    else if( pButton == m_vResultList[8] )
    {
        if( !m_vPersonaSettings[8].isEmpty() )
            m_aSelectedPersona = m_vPersonaSettings[8];
    }

    return 0;
}

void SelectPersonaDialog::SetAppliedPersonaSetting( OUString& rPersonaSetting )
{
    m_aAppliedPersona = rPersonaSetting;
}

OUString SelectPersonaDialog::GetAppliedPersonaSetting() const
{
    return m_aAppliedPersona;
}

void SelectPersonaDialog::SetProgress( OUString& rProgress )
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

void SelectPersonaDialog::SetImages( std::vector<Image> &rImageList )
{
    sal_Int32 nCount = 0;
    for( std::vector<Image>::iterator it=rImageList.begin(); it!=rImageList.end(); ++it )
    {
        m_vResultList[nCount]->Show();
        m_vResultList[nCount]->SetModeImage( *it );
        nCount++;
    }
}

void SelectPersonaDialog::AddPersonaSetting( OUString& rPersonaSetting )
{
    m_vPersonaSettings.push_back( rPersonaSetting );
}

void SelectPersonaDialog::ClearSearchResults()
{
    m_vPersonaSettings.clear();
    m_aSelectedPersona = "";
    for( sal_Int32 nIndex = 0; nIndex < 9; nIndex++ )
        m_vResultList[nIndex]->Hide();
}

SvxPersonalizationTabPage::SvxPersonalizationTabPage( Window *pParent, const SfxItemSet &rSet )
    : SfxTabPage( pParent, "PersonalizationTabPage", "cui/ui/personalization_tab.ui", &rSet )
{
    // persona
    get( m_pNoPersona, "no_persona" );
    get( m_pDefaultPersona, "default_persona" );

    get( m_pOwnPersona, "own_persona" );
    m_pOwnPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, ForceSelect ) );

    get( m_pSelectPersona, "select_persona" );
    m_pSelectPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, SelectPersona ) );
}

SvxPersonalizationTabPage::~SvxPersonalizationTabPage()
{
}

SfxTabPage* SvxPersonalizationTabPage::Create( Window *pParent, const SfxItemSet *rSet )
{
    return new SvxPersonalizationTabPage( pParent, *rSet );
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
    boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );

    officecfg::Office::Common::Misc::Persona::set( aPersona, batch );
    officecfg::Office::Common::Misc::PersonaSettings::set( m_aPersonaSettings, batch );
    batch->commit();

    if ( bModified )
    {
        // broadcast the change
        DataChangedEvent aDataChanged( DATACHANGED_SETTINGS, NULL, SETTINGS_STYLE );
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

IMPL_LINK( SvxPersonalizationTabPage, SelectPersona, PushButton*, /*pButton*/ )
{
    SelectPersonaDialog aDialog( NULL );

    while ( aDialog.Execute() == RET_OK )
    {
        OUString aPersonaSetting( aDialog.GetAppliedPersonaSetting() );
        if ( !aPersonaSetting.isEmpty() )
        {
            setPersonaSettings( aPersonaSetting );
        }

        break;
    }

    return 0;
}

IMPL_LINK( SvxPersonalizationTabPage, ForceSelect, RadioButton*, pButton )
{
    if ( pButton == m_pOwnPersona && m_aPersonaSettings.isEmpty() )
        SelectPersona( m_pSelectPersona );

    return 0;
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

void SvxPersonalizationTabPage::setPersonaSettings( const OUString aPersonaSettings )
{
    m_aPersonaSettings = aPersonaSettings;
    m_pOwnPersona->Check();
}

SearchAndParseThread::SearchAndParseThread( SelectPersonaDialog* pDialog,
                          const OUString& rURL ) :
            Thread( "cuiPersonasSearchThread" ),
            m_pPersonaDialog( pDialog ),
            m_aURL( rURL )
{
}

SearchAndParseThread::~SearchAndParseThread()
{
}

void SearchAndParseThread::execute()
{
    if( m_aURL.startsWith( "https://" ) )
    {
        m_pPersonaDialog->ClearSearchResults();
        OUString sProgress( "Searching.. Please Wait.." );
        m_pPersonaDialog->SetProgress( sProgress );
        Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
        Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);
        PersonasDocHandler* pHandler = new PersonasDocHandler();
        Reference< xml::sax::XDocumentHandler > xDocHandler = pHandler;
        uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
        uno::Reference< io::XInputStream > xStream;
        xParser->setDocumentHandler( xDocHandler );

        if ( !xFileAccess.is() )
            return;

        try {
            xStream = xFileAccess->openFileRead( m_aURL );
        }
        catch (...)
        {
            sProgress = "Something went wrong. Please try again.";
            m_pPersonaDialog->SetProgress( sProgress );
            return;
        }

        xml::sax::InputSource aParserInput;
        aParserInput.aInputStream = xStream;
        xParser->parseStream( aParserInput );

        if( !pHandler->hasResults() )
        {
            sProgress = "No results found.";
            m_pPersonaDialog->SetProgress( sProgress );
            return;
        }

        std::vector<OUString> vLearnmoreURLs = pHandler->getLearnmoreURLs();
        std::vector<OUString>::iterator it;
        std::vector<Image> vResultList;
        GraphicFilter aFilter;
        Graphic aGraphic;

        for( it = vLearnmoreURLs.begin(); it!=vLearnmoreURLs.end(); ++it )
        {
            OUString sHeaderFile, aPersonaSetting;
            getPreviewFile( *it, &sHeaderFile, &aPersonaSetting );
            INetURLObject aURLObj( sHeaderFile );
            aFilter.ImportGraphic( aGraphic, aURLObj );
            Bitmap aBmp = aGraphic.GetBitmap();
            vResultList.push_back( Image( aBmp ) );
            m_pPersonaDialog->AddPersonaSetting(aPersonaSetting);
        }

        // for VCL to be able to do visual changes in the thread
        SolarMutexGuard aGuard;

        m_pPersonaDialog->SetImages( vResultList );
        sProgress = "";
        m_pPersonaDialog->SetProgress( sProgress );
        m_pPersonaDialog->setOptimalLayoutSize();
    }

    else
    {
        OUString sProgress( "Applying persona.." );
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
            sProgress = "Something went wrong. Please try again.";
            m_pPersonaDialog->SetProgress( sProgress );
            return;
        }

        SolarMutexGuard aGuard;

        aPersonaSetting = aHeaderFile + ";" + aFooterFile + ";" + aTextColor + ";" + aAccentColor;
        m_pPersonaDialog->SetAppliedPersonaSetting( aPersonaSetting );
        m_pPersonaDialog->EndDialog( RET_OK );
    }
}

void SearchAndParseThread::getPreviewFile( const OUString& rURL, OUString *pHeaderFile, OUString *pPersonaSetting )
{
    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
    if ( !xFileAccess.is() )
        return;

    uno::Reference< io::XInputStream > xStream;
    try {
        xStream = xFileAccess->openFileRead( rURL );
    }
    catch (...)
    {
        OUString sProgress = "Something went wrong. Please try again.";
        m_pPersonaDialog->SetProgress( sProgress );
        return;
    }
    if ( !xStream.is() )
        return;

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
        return;

    // copy the images to the user's gallery
    OUString gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( gallery );
    gallery += "/user/gallery/personas/";
    gallery += aName + "/";
    osl::Directory::createPath( gallery );

    OUString aPreviewFile( INetURLObject( aPreviewURL ).getName() );

    try {
        xFileAccess->copy( aPreviewURL, gallery + aPreviewFile );
    }
    catch ( const uno::Exception & )
    {
        return;
    }
    *pHeaderFile = gallery + aPreviewFile;
    *pPersonaSetting = aName + ";" + aHeaderURL + ";" + aFooterURL + ";" + aTextColor + ";" + aAccentColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
