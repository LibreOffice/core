/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "personalization.hxx"

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
    : SfxTabPage( pParent, "PersonalizationTabPage", "cui/ui/personalization_tab.ui", rSet )
{
    get( m_pNoBackground, "no_background" );
    get( m_pDefaultBackground, "default_background" );
    get( m_pOwnBackground, "own_background" );
    m_pNoBackground->SetClickHdl( LINK( this, SvxPersonalizationTabPage, EnableDisableSelectionButtons ) );
    m_pDefaultBackground->SetClickHdl( LINK( this, SvxPersonalizationTabPage, EnableDisableSelectionButtons ) );
    m_pOwnBackground->SetClickHdl( LINK( this, SvxPersonalizationTabPage, EnableDisableSelectionButtons ) );

    get( m_pSelectBackground, "select_background" );
    m_pSelectBackground->SetClickHdl( LINK( this, SvxPersonalizationTabPage, SelectBackground ) );

    get( m_pNoPersona, "no_persona" );
    get( m_pDefaultPersona, "default_persona" );
    get( m_pOwnPersona, "own_persona" );
    m_pNoPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, EnableDisableSelectionButtons ) );
    m_pDefaultPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, EnableDisableSelectionButtons ) );
    m_pOwnPersona->SetClickHdl( LINK( this, SvxPersonalizationTabPage, EnableDisableSelectionButtons ) );

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

IMPL_LINK( SvxPersonalizationTabPage, EnableDisableSelectionButtons, RadioButton*, pButton )
{
    PushButton *pPushButton = NULL;
    RadioButton *pRadioButton = NULL;

    if ( pButton == m_pNoBackground || pButton == m_pDefaultBackground || pButton == m_pOwnBackground )
    {
        pPushButton = m_pSelectBackground;
        pRadioButton = m_pOwnBackground;
    }
    else if ( pButton == m_pNoPersona || pButton == m_pDefaultPersona || pButton == m_pOwnPersona )
    {
        pPushButton = m_pSelectPersona;
        pRadioButton = m_pOwnPersona;
    }
    else
        return 0;

    if ( pRadioButton->IsChecked() && !pPushButton->IsEnabled() )
    {
        pPushButton->Enable();
        pPushButton->Invalidate();
    }
    else if ( !pRadioButton->IsChecked() && pPushButton->IsEnabled() )
    {
        pPushButton->Disable();
        pPushButton->Invalidate();
    }

    return 0;
}

IMPL_LINK( SvxPersonalizationTabPage, SelectBackground, PushButton*, /*pButton*/ )
{
    return 0;
}

IMPL_LINK( SvxPersonalizationTabPage, SelectPersona, PushButton*, /*pButton*/ )
{
    SelectPersonaDialog aDialog( NULL );
    /* TODO handle the ret val sal_Int16 nReturn =*/ aDialog.Execute();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
