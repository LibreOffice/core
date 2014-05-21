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
    get( pButton, "search_personas" );
    pButton->SetClickHdl( LINK( this, SelectPersonaDialog, VisitPersonas ) );

    get( m_pEdit, "search_term" );
    m_pEdit->SetPlaceholderText( "Search term..." );
}

OUString SelectPersonaDialog::GetPersonaURL() const
{
    OUString aText( m_pEdit->GetText() );

    if ( aText.startsWith( "https://addons.mozilla.org/" ) )
        return aText;

    return OUString();
}

IMPL_LINK( SelectPersonaDialog, VisitPersonas, PushButton*, /*pButton*/ )
{
    OUString searchTerm = m_pEdit->GetText();
    OUString rURL = "https://addons.allizom.org/en-US/firefox/api/1.5/search/" + searchTerm + "/9/";
    m_aSearchThread = new SearchAndParseThread( this, rURL );
    m_aSearchThread->launch();
    return 0;
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
        OUString aURL( aDialog.GetPersonaURL() );
        if ( !aURL.isEmpty() )
        {
            if ( CopyPersonaToGallery( aURL ) )
                m_pOwnPersona->Check();
            break;
        }
        // else TODO msgbox that the URL did not match
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
static bool parsePersonaInfo( const OString &rBuffer, OUString *pHeaderURL, OUString *pFooterURL, OUString *pTextColor, OUString *pAccentColor )
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

    return true;
}

bool SvxPersonalizationTabPage::CopyPersonaToGallery( const OUString &rURL )
{
    // init the input stream
    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
    if ( !xFileAccess.is() )
        return false;

    uno::Reference< io::XInputStream > xStream;
    try {
        xStream = xFileAccess->openFileRead( rURL );
    }
    catch (...)
    {
        return false;
    }
    if ( !xStream.is() )
        return false;

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
    OUString aHeaderURL, aFooterURL, aTextColor, aAccentColor;

    if ( !parsePersonaInfo( aBuffer.makeStringAndClear(), &aHeaderURL, &aFooterURL, &aTextColor, &aAccentColor ) )
        return false;

    // copy the images to the user's gallery
    OUString gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
    rtl::Bootstrap::expandMacros( gallery );
    gallery += "/user/gallery/personas/";
    osl::Directory::createPath( gallery );

    OUString aHeaderFile( INetURLObject( aHeaderURL ).getName() );
    OUString aFooterFile( INetURLObject( aFooterURL ).getName() );

    try {
        xFileAccess->copy( aHeaderURL, gallery + aHeaderFile );
        xFileAccess->copy( aFooterURL, gallery + aFooterFile );
    }
    catch ( const uno::Exception & )
    {
        return false;
    }

    m_aPersonaSettings = aHeaderFile + ";" + aFooterFile + ";" + aTextColor + ";" + aAccentColor;

    return true;
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
    Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
    Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);
    PersonasDocHandler* pHandler = new PersonasDocHandler();
    Reference< xml::sax::XDocumentHandler > xDocHandler = pHandler;
    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
    uno::Reference< io::XInputStream > xStream;
    xParser->setDocumentHandler( xDocHandler );

    // if ( !xFileAccess.is() )
    //     return false;

    try {
        xStream = xFileAccess->openFileRead( m_aURL );
    }
    catch (...)
    {
        // return false;
    }
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xStream;
    xParser->parseStream( aParserInput );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
