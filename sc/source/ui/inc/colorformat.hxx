/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_COLORFORMAT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_COLORFORMAT_HXX

#include <vcl/weld.hxx>
#include <svx/colorbox.hxx>
#include <address.hxx>
#include <vcl/edit.hxx>

struct ScDataBarFormatData;
class ScDocument;
class SvNumberFormatter;

class ScDataBarSettingsDlg : public weld::GenericDialogController
{
private:
    OUString maStrWarnSameValue;
    SvNumberFormatter* mpNumberFormatter;

    ScDocument* const mpDoc;
    ScAddress const   maPos;

    std::unique_ptr<weld::Button> mxBtnOk;
    std::unique_ptr<weld::Button> mxBtnCancel;

    std::unique_ptr<ColorListBox> mxLbPos;
    std::unique_ptr<ColorListBox> mxLbNeg;
    std::unique_ptr<ColorListBox> mxLbAxisCol;

    std::unique_ptr<weld::ComboBox> mxLbFillType;
    std::unique_ptr<weld::ComboBox> mxLbTypeMin;
    std::unique_ptr<weld::ComboBox> mxLbTypeMax;
    std::unique_ptr<weld::ComboBox> mxLbAxisPos;

    std::unique_ptr<weld::Entry> mxEdMin;
    std::unique_ptr<weld::Entry> mxEdMax;
    std::unique_ptr<weld::Entry> mxLenMin;
    std::unique_ptr<weld::Entry> mxLenMax;

    std::unique_ptr<weld::CheckButton> mxCbOnlyBar;

    std::unique_ptr<weld::Label> mxStrSameValueFT;

    DECL_LINK(OkBtnHdl, weld::Button&, void);
    DECL_LINK(TypeSelectHdl, weld::ComboBox&, void);
    DECL_LINK(PosSelectHdl, weld::ComboBox&, void);

    void Init();

public:
    ScDataBarSettingsDlg(weld::Window* pParent, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos);
    virtual ~ScDataBarSettingsDlg() override;

    ScDataBarFormatData* GetData();
};

#endif // INCLUDED_SC_SOURCE_UI_INC_COLORFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

