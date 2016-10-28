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
class SvxColorListBox;

class ScDataBarSettingsDlg : public ModalDialog
{
private:
    VclPtr<OKButton> mpBtnOk;
    VclPtr<CancelButton> mpBtnCancel;

    VclPtr<SvxColorListBox> mpLbPos;
    VclPtr<SvxColorListBox> mpLbNeg;
    VclPtr<SvxColorListBox> mpLbAxisCol;

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
    SvNumberFormatter* mpNumberFormatter;

    ScDocument* mpDoc;
    ScAddress   maPos;

    DECL_LINK(OkBtnHdl, Button*, void);
    DECL_LINK(TypeSelectHdl, ListBox&, void);
    DECL_LINK(PosSelectHdl, ListBox&, void);

    void Init();

public:
    ScDataBarSettingsDlg(vcl::Window* pParent, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos);
    virtual ~ScDataBarSettingsDlg() override;
    virtual void dispose() override;

    ScDataBarFormatData* GetData();
};

#endif // INCLUDED_SC_SOURCE_UI_INC_COLORFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

