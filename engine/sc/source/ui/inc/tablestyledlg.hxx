/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <svx/colorbox.hxx>
#include <tablestyle.hxx>

#include <map>
#include <memory>
#include <string_view>

class ScDocument;
class ScPatternAttr;

// Create a new Calc table style. The user names the style and, per table
// region, sets a background colour, font colour, bold and a simple border. On
// OK the dialog builds the style and hands it to the caller, which registers
// it with the document; the dialog itself does not change the document.
class ScTableStyleDlg final : public weld::GenericDialogController
{
public:
    ScTableStyleDlg(weld::Window* pParent, ScDocument& rDoc);
    virtual ~ScTableStyleDlg() override;

    // The style the user built, moved out to the caller; null when cancelled.
    std::unique_ptr<ScTableStyle> TakeStyle() { return std::move(mxResultStyle); }

private:
    ScDocument& mrDoc;

    // The edited formatting, one pattern per region the user has touched.
    // A region with no entry is left unstyled, matching GetSetPatterns.
    std::map<ScTableStyleElement, std::unique_ptr<ScPatternAttr>> maElementPatterns;
    // True while the pickers are being loaded for a newly selected region, so
    // their change handlers do not write the values straight back.
    bool mbLoading = false;
    std::unique_ptr<ScTableStyle> mxResultStyle;

    std::unique_ptr<weld::Entry> mxName;
    std::unique_ptr<weld::TreeView> mxRegions;
    std::unique_ptr<weld::CheckButton> mxBold;
    std::unique_ptr<weld::ComboBox> mxBorderStyle;
    std::unique_ptr<weld::Button> mxClear;
    std::unique_ptr<weld::Label> mxWarning;
    std::unique_ptr<weld::Image> mxPreview;
    std::unique_ptr<weld::Button> mxOk;
    std::unique_ptr<ColorListBox> mxBackColor;
    std::unique_ptr<ColorListBox> mxFontColor;
    std::unique_ptr<ColorListBox> mxBorderColor;

    ScTableStyleElement GetSelectedElement() const;
    bool HasSelectedElement() const;
    // The pattern for a region, created empty on first use.
    ScPatternAttr& EnsurePattern(ScTableStyleElement eElement);
    void LoadRegion();
    void ApplyPickersToRegion();
    void RefreshPreview();
    bool IsNameFree(std::u16string_view rUIName) const;
    void UpdateOkState();
    std::unique_ptr<ScTableStyle> BuildStyle(const OUString& rProgrammaticName) const;

    DECL_LINK(RegionSelectHdl, weld::TreeView&, void);
    DECL_LINK(ColorHdl, ColorListBox&, void);
    DECL_LINK(BoldHdl, weld::Toggleable&, void);
    DECL_LINK(BorderHdl, weld::ComboBox&, void);
    DECL_LINK(ClearHdl, weld::Button&, void);
    DECL_LINK(NameHdl, weld::Entry&, void);
    DECL_LINK(OkHdl, weld::Button&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
