/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_CUI_SOURCE_INC_FONTFEATURESDIALOG_HXX
#define INCLUDED_CUI_SOURCE_INC_FONTFEATURESDIALOG_HXX

#include <vcl/layout.hxx>
#include <vcl/font/Feature.hxx>
#include <svx/fntctrl.hxx>
#include "chardlg.hxx"
#include <memory>

namespace cui
{
struct FontFeatureItem
{
    FontFeatureItem()
        : m_aFeatureCode(0)
    {
    }

    sal_uInt32 m_aFeatureCode;
    VclPtr<Control> m_pText;
    VclPtr<ComboBox> m_pCombo;
    VclPtr<CheckBox> m_pCheck;
};

class FontFeaturesDialog : public ModalDialog
{
private:
    VclPtr<VclGrid> m_pContentGrid;
    VclPtr<SvxFontPrevWindow> m_pPreviewWindow;

    std::vector<FontFeatureItem> m_aFeatureItems;
    OUString m_sFontName;
    OUString m_sResultFontName;

    void initialize();
    OUString createFontNameWithFeatures();

    DECL_LINK(ComboBoxSelectedHdl, ComboBox&, void);
    DECL_LINK(CheckBoxToggledHdl, CheckBox&, void);

public:
    FontFeaturesDialog(vcl::Window* pParent, OUString const& rFontName);
    ~FontFeaturesDialog() override;
    void dispose() override;
    short Execute() override;

    OUString const& getResultFontName() { return m_sResultFontName; }

    void updateFontPreview();
};

} // end svx namespaces

#endif // INCLUDED_CUI_SOURCE_INC_FONTFEATURESDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
