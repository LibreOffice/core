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
#include <vcl/msgbox.hxx>

using namespace com::sun::star;

/** Dialog that will allow the user to choose a Persona to use.

So far there is no better possibility than just to paste the URL from
http://www.getpersona.com ...
*/
class SelectPersonaDialog : public ModalDialog
{
public:
    SelectPersonaDialog( Window *pParent );
};

SelectPersonaDialog::SelectPersonaDialog( Window *pParent )
    : ModalDialog( pParent, "SelectPersonaDialog", "cui/ui/select_persona_dialog.ui" )
{
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
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
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
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

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
    // TODO m_pOwnBackground->Check(); if something selected
    // TODO parse the results

    return 0;
}

IMPL_LINK( SvxPersonalizationTabPage, SelectPersona, PushButton*, /*pButton*/ )
{
    SelectPersonaDialog aDialog( NULL );

    if ( aDialog.Execute() == RET_OK )
    {
        m_pOwnPersona->Check();
        // TODO parse the results
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
