/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

    rtl::OUString maStrWarnSameValue;
    SvNumberFormatter* mpNumberFormatter;

    ScDocument* mpDoc;
    ScAddress   maPos;

    DECL_LINK(OkBtnHdl, void*);
    DECL_LINK(TypeSelectHdl, void*);

    void Init();

public:
    ScDataBarSettingsDlg(Window* pParent, ScDocument* pDoc, const ScAddress& rPos);
    ScDataBarSettingsDlg(Window* pParent, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos);

    ScDataBarFormatData* GetData();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
