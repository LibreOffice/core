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
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
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

    if ( !aText.startsWith( "http://www.getpersonas.com/" ) )
        return OUString();

    return aText;
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

    get( m_pSelectBackground, "select_background" );
    m_pSelectBackground->SetClickHdl( LINK( this, SvxPersonalizationTabPage, SelectBackground ) );

    // persona
    get( m_pNoPersona, "no_persona" );
    get( m_pDefaultPersona, "default_persona" );
    get( m_pOwnPersona, "own_persona" );

    get( m_pSelectPersona, "select_persona" );
    LINK( this, SvxPersonalizationTabPage, SelectPersona );
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
              aPersona != officecfg::Office::Common::Misc::Persona::get( xContext ) ) )
    {
        bModified = true;
    }

    // write
    boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );

    officecfg::Office::Common::Misc::BackgroundImage::set( aBackground, batch );
    officecfg::Office::Common::Misc::BackgroundImageURL::set( m_aBackgroundURL, batch );
    officecfg::Office::Common::Misc::Persona::set( aPersona, batch );

    batch->commit();

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
        aPersona = officecfg::Office::Common::Misc::Persona::get( xContext );

    if ( aPersona == "no" )
        m_pNoPersona->Check();
    else if ( aPersona == "own" )
        m_pOwnPersona->Check();
    else
        m_pDefaultPersona->Check();
}

IMPL_LINK( SvxPersonalizationTabPage, SelectBackground, PushButton*, /*pButton*/ )
{
    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    if ( !xFactory.is() )
        return 0;

    uno::Reference< ui::dialogs::XFilePicker > xFilePicker( xFactory->createInstance( "com.sun.star.ui.dialogs.FilePicker" ), uno::UNO_QUERY );
    if ( !xFilePicker.is() )
        return 0;

    xFilePicker->setMultiSelectionMode( false );

    uno::Reference< ui::dialogs::XFilePickerControlAccess > xController( xFilePicker, uno::UNO_QUERY );
    if ( xController.is() )
        xController->setValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, uno::makeAny( sal_True ) );

    uno::Reference< ui::dialogs::XFilterManager > xFilterMgr( xFilePicker, uno::UNO_QUERY );
    if ( xFilterMgr.is() )
        xFilterMgr->appendFilter( "Background images (*.jpg;*.png)", "*.jpg;*.png" ); // TODO localize

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
        if ( aURL != "" )
        {
            // TODO parse the results
            m_pOwnPersona->Check();
            break;
        }
        // else TODO msgbox that the URL did not match
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
