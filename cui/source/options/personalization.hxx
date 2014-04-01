/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_PERSONALIZATION_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_PERSONALIZATION_HXX

#include <sfx2/tabdlg.hxx>

class FixedText;

class SvxPersonalizationTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

private:
    RadioButton *m_pNoPersona;              ///< Just the default look, without any bitmap
    RadioButton *m_pDefaultPersona;         ///< Use the built-in bitmap
    RadioButton *m_pOwnPersona;             ///< Use the user-defined bitmap
    PushButton *m_pSelectPersona;           ///< Let the user select in the 'own' case
    OUString m_aPersonaSettings;            ///< Header and footer images + color to be set in the settings.

public:
    SvxPersonalizationTabPage( Window *pParent, const SfxItemSet &rSet );
    virtual ~SvxPersonalizationTabPage();

    static SfxTabPage* Create( Window *pParent, const SfxItemSet &rSet );

    /// Apply the settings ([OK] button).
    virtual bool FillItemSet( SfxItemSet &rSet ) SAL_OVERRIDE;

    /// Reset to default settings ([Revert] button).
    virtual void Reset( const SfxItemSet &rSet ) SAL_OVERRIDE;

private:
    /// Handle the Persona selection
    DECL_LINK( SelectPersona, PushButton* );

    /// When 'own' is chosen, but the Persona is not chosen yet.
    DECL_LINK( ForceSelect, RadioButton* );

    /// Download the bitmaps + color settings, and copy them to user's profile.
    bool CopyPersonaToGallery( const OUString &rURL );
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_PERSONALIZATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
