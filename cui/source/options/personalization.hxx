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
    RadioButton *m_pNoPersona; ///< Just the default look, without any bitmap

public:
    SvxPersonalizationTabPage( Window *pParent, const SfxItemSet &rSet );
    ~SvxPersonalizationTabPage();

    static SfxTabPage* Create( Window *pParent, const SfxItemSet &rSet );
};

#endif // SVX_PERSONALIZATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
