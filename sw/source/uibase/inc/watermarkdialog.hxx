/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_WATERMARKDIALOG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_WATERMARKDIALOG_HXX

#include <sfx2/bindings.hxx>
#include <vcl/weld.hxx>
#include <svx/colorbox.hxx>
#include <sfx2/basedlgs.hxx>

class SwWatermarkDialog : public SfxDialogController
{
public:
    SwWatermarkDialog(weld::Window* pParent, SfxBindings& rBindings);
    virtual ~SwWatermarkDialog() override;

    void InitFields();

private:
    DECL_LINK(OKButtonHdl, weld::Button&, void);

    SfxBindings& m_rBindings;

    std::unique_ptr<weld::Entry> m_xTextInput;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::ComboBox> m_xFont;
    std::unique_ptr<weld::MetricSpinButton> m_xAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xTransparency;
    std::unique_ptr<ColorListBox> m_xColor;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
