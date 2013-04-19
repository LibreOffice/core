/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <svtools/ctrlbox.hxx>
#include <svl/zforlist.hxx>
#include "anyrefdg.hxx"

struct ScDataBarFormatData;
class ScDocument;

class ScDataBarSettingsDlg : public ModalDialog
{
private:
    OKButton maBtnOk;
    CancelButton maBtnCancel;
    FixedLine maFlBarColors;
    FixedLine maFlAxes;
    FixedLine maFlValues;

    FixedText maFtMin;
    FixedText maFtMax;
    FixedText maFtPositive;
    FixedText maFtNegative;
    FixedText maFtPosition;
    FixedText maFtAxisColor;

    ColorListBox maLbPos;
    ColorListBox maLbNeg;
    ColorListBox maLbAxisCol;

    ListBox maLbTypeMin;
    ListBox maLbTypeMax;
    ListBox maLbAxisPos;

    Edit maEdMin;
    Edit maEdMax;

    OUString maStrWarnSameValue;
    SvNumberFormatter* mpNumberFormatter;

    ScDocument* mpDoc;
    ScAddress   maPos;

    DECL_LINK(OkBtnHdl, void*);
    DECL_LINK(TypeSelectHdl, void*);

    void Init();

public:
    ScDataBarSettingsDlg(Window* pParent, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos);

    ScDataBarFormatData* GetData();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
