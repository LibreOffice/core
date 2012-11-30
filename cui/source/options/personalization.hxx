/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SVX_PERSONALIZATION_HXX
#define SVX_PERSONALIZATION_HXX

#include <sfx2/tabdlg.hxx>

class FixedText;

class SvxPersonalizationTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

private:
    RadioButton *m_pNoBackground;           ///< Just the default look, without any bitmap
    RadioButton *m_pDefaultBackground;      ///< Use the built-in bitmap for Writer background
    RadioButton *m_pOwnBackground;          ///< Use the user-defined bitmap
    PushButton *m_pSelectBackground;        ///< Let the user select in the 'own' case

    RadioButton *m_pNoPersona;              ///< Just the default look, without any bitmap
    RadioButton *m_pDefaultPersona;         ///< Use the built-in bitmap
    RadioButton *m_pOwnPersona;             ///< Use the user-defined bitmap
    PushButton *m_pSelectPersona;           ///< Let the user select in the 'own' case

public:
    SvxPersonalizationTabPage( Window *pParent, const SfxItemSet &rSet );
    ~SvxPersonalizationTabPage();

    static SfxTabPage* Create( Window *pParent, const SfxItemSet &rSet );

private:
    /// Maintain sane behavior of the m_pSelect(Background|Persona) buttons
    DECL_LINK( EnableDisableSelectionButtons, RadioButton* );

    /// Handle the bacground selection
    DECL_LINK( SelectBackground, PushButton* );

    /// Handle the Persona selection
    DECL_LINK( SelectPersona, PushButton* );
};

#endif // SVX_PERSONALIZATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
