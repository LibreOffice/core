/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "personalization.hxx"

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/urlobj.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>

using namespace com::sun::star;

/** Dialog that will allow the user to choose a Persona to use.

So far there is no better possibility than just to paste the URL from
http://www.getpersona.com ...
*/
class SelectPersonaDialog : public ModalDialog
{
private:
    Edit *m_pEdit;                          ///< The input line for the Persona URL

public:
    SelectPersonaDialog( Window *pParent );

    /// Get the URL from the Edit field.
    OUString GetPersonaURL() const;

private:
    /// Handle the [Visit Firefox Personas] button
    DECL_LINK( VisitPersonas, PushButton* );
};

SelectPersonaDialog::SelectPersonaDialog( Window *pParent )
    : ModalDialog( pParent, "SelectPersonaDialog", "cui/ui/select_persona_dialog.ui" )
{
    PushButton *pButton;
    get( pButton, "visit_personas" );
    pButton->SetClickHdl( LINK( this, SelectPersonaDialog, VisitPersonas ) );

    get( m_pEdit, "persona_url" );
    m_pEdit->SetPlaceholderText( "http://www.getpersonas.com/persona/" );
}

OUString SelectPersonaDialog::GetPersonaURL() const
{
    OUString aText( m_pEdit->GetText() );

    if ( !aText.startsWith( "http://www.getpersonas.com/" ) &&
         !aText.startsWith( "https://www.getpersonas.com/" ) )
    {
        return OUString();
    }

    // canonicalize the URL
    OUString aPersona( "persona/" );
    sal_Int32 nPersona = aText.lastIndexOf( aPersona );

    if ( nPersona < 0 )
        return OUString();

    return "http://www.getpersonas.com/persona/" + aText.copy( nPersona + aPersona.getLength() );
}

IMPL_LINK( SelectPersonaDialog, VisitPersonas, PushButton*, /*pButton*/ )
{
    uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShell( com::sun::star::system::SystemShellExecute::create( ::comphelper::getProcessComponentContext() ) );

    xSystemShell->execute( "http://www.getpersonas.com", OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );

    return 0;
}

SvxPersonalizationTabPage::SvxPersonalizationTabPage( Window *pParent, const SfxItemSet &rSet )
    : SfxTabPage( pParent, "PersonalizationTabPage", "cui/ui/personalization_tab.ui", rSet ),
      m_aBackgroundURL()
{
    // background image
    get( m_pNoBackground, "no_background" );
    get( m_pDefaultBackground, "default_background" );

    get( m_pOwnBackground, "own_background" );
    m_pOwnBackground->SetClickHdl( LINK( this, SvxPersonalizationTabPage, ForceSelect ) );

    get( m_pSelectBackground, "select_background" );
    m_pSelectBackground->SetClickHdl( LINK( this, SvxPersonalizationTabPage, SelectBackground ) );

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

SfxTabPage* SvxPersonalizationTabPage::Create( Window *pParent, const SfxItemSet &rSet )
{
    return new SvxPersonalizationTabPage( pParent, rSet );
}

sal_Bool SvxPersonalizationTabPage::FillItemSet( SfxItemSet & )
{
    // background image
    OUString aBackground( "default" );
    if ( m_pNoBackground->IsChecked() )
        aBackground = "no";
    else if ( m_pOwnBackground->IsChecked() )
        aBackground = "own";

    // persona
    OUString aPersona( "default" );
    if ( m_pNoPersona->IsChecked() )
        aPersona = "no";
    else if ( m_pOwnPersona->IsChecked() )
        aPersona = "own";

    bool bModified = false;
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    if ( xContext.is() &&
            ( aBackground != officecfg::Office::Common::Misc::BackgroundImage::get( xContext ) ||
              m_aBackgroundURL != officecfg::Office::Common::Misc::BackgroundImageURL::get( xContext ) ||
              aPersona != officecfg::Office::Common::Misc::Persona::get( xContext ) ||
              m_aPersonaSettings != officecfg::Office::Common::Misc::PersonaSettings::get( xContext ) ) )
    {
        bModified = true;
    }

    // write
    boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );

    officecfg::Office::Common::Misc::BackgroundImage::set( aBackground, batch );
    officecfg::Office::Common::Misc::BackgroundImageURL::set( m_aBackgroundURL, batch );
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

void SvxPersonalizationTabPage::Reset( const SfxItemSet & )
{
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // background image
    OUString aBackground( "default" );
    if ( xContext.is() )
    {
        aBackground = officecfg::Office::Common::Misc::BackgroundImage::get( xContext );
        m_aBackgroundURL = officecfg::Office::Common::Misc::BackgroundImageURL::get( xContext );
    }

    if ( aBackground == "no" )
        m_pNoBackground->Check();
    else if ( aBackground == "own" )
        m_pOwnBackground->Check();
    else
        m_pDefaultBackground->Check();

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

IMPL_LINK( SvxPersonalizationTabPage, SelectBackground, PushButton*, /*pButton*/ )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    uno::Reference< ui::dialogs::XFilePicker3 > xFilePicker = ui::dialogs::FilePicker::createWithMode(xContext, ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE);

    xFilePicker->setMultiSelectionMode( false );

    uno::Reference< ui::dialogs::XFilePickerControlAccess > xController( xFilePicker, uno::UNO_QUERY );
    if ( xController.is() )
        xController->setValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, uno::makeAny( sal_True ) );

    xFilePicker->appendFilter( "Background images (*.jpg;*.png)", "*.jpg;*.png" ); // TODO localize

    while ( xFilePicker->execute() == ui::dialogs::ExecutableDialogResults::OK )
    {
        OUString aFile( xFilePicker->getFiles()[0] );

        if ( aFile.startsWith( "file:///" ) && ( aFile.endsWith( ".png" ) || aFile.endsWith( ".jpg" ) ) )
        {
            m_aBackgroundURL = aFile;
            m_pOwnBackground->Check();
            break;
        }

        // TODO display what is wrong (not a file:// or not .png / .jpg)
    }

    return 0;
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
    if ( pButton == m_pOwnBackground && m_aBackgroundURL.isEmpty() )
        SelectBackground( m_pSelectBackground );
    else if ( pButton == m_pOwnPersona && m_aPersonaSettings.isEmpty() )
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

    sal_Int32 end = rBuffer.indexOf( "&quot;", where );
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
    sal_Int32 persona = rBuffer.indexOf( "persona=\"{" );
    if ( persona < 0 )
        return false;

    // now search inside
    *pHeaderURL = searchValue( rBuffer, persona, "&quot;headerURL&quot;:&quot;" );
    if ( pHeaderURL->isEmpty() )
        return false;

    *pFooterURL = searchValue( rBuffer, persona, "&quot;footerURL&quot;:&quot;" );
    if ( pFooterURL->isEmpty() )
        return false;

    *pTextColor = searchValue( rBuffer, persona, "&quot;textcolor&quot;:&quot;" );
    if ( pTextColor->isEmpty() )
        return false;

    *pAccentColor = searchValue( rBuffer, persona, "&quot;accentcolor&quot;:&quot;" );
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
    OUString gallery = "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
