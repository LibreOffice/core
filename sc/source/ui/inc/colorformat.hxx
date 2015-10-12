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
    VclPtr<OKButton> mpBtnOk;
    VclPtr<CancelButton> mpBtnCancel;

    VclPtr<ColorListBox> mpLbPos;
    VclPtr<ColorListBox> mpLbNeg;
    VclPtr<ColorListBox> mpLbAxisCol;

    VclPtr<ListBox> mpLbFillType;
    VclPtr<ListBox> mpLbTypeMin;
    VclPtr<ListBox> mpLbTypeMax;
    VclPtr<ListBox> mpLbAxisPos;

    VclPtr<Edit> mpEdMin;
    VclPtr<Edit> mpEdMax;
    VclPtr<Edit> mpLenMin;
    VclPtr<Edit> mpLenMax;

    VclPtr<CheckBox> mpCbOnlyBar;

    OUString maStrWarnSameValue;
    OUString maCustomColor;
    SvNumberFormatter* mpNumberFormatter;

    ScDocument* mpDoc;
    ScAddress   maPos;

    DECL_LINK_TYPED(OkBtnHdl, Button*, void);
    DECL_LINK_TYPED(TypeSelectHdl, ListBox&, void);
    DECL_LINK_TYPED(PosSelectHdl, ListBox&, void);

    void Init();

public:
    ScDataBarSettingsDlg(vcl::Window* pParent, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos);
    virtual ~ScDataBarSettingsDlg();
    virtual void dispose() override;

    ScDataBarFormatData* GetData();
};

#endif // INCLUDED_SC_SOURCE_UI_INC_COLORFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

