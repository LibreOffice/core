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
    OKButton* mpBtnOk;
    CancelButton* mpBtnCancel;

    ColorListBox* mpLbPos;
    ColorListBox* mpLbNeg;
    ColorListBox* mpLbAxisCol;

    ListBox* mpLbTypeMin;
    ListBox* mpLbTypeMax;
    ListBox* mpLbAxisPos;

    Edit* mpEdMin;
    Edit* mpEdMax;

    OUString maStrWarnSameValue;
    SvNumberFormatter* mpNumberFormatter;

    ScDocument* mpDoc;
    ScAddress   maPos;

    DECL_LINK(OkBtnHdl, void*);
    DECL_LINK(TypeSelectHdl, void*);

    void Init();

public:
    ScDataBarSettingsDlg(vcl::Window* pParent, const ScDataBarFormatData& rData, ScDocument* pDoc, const ScAddress& rPos);

    ScDataBarFormatData* GetData();
};

#endif // INCLUDED_SC_SOURCE_UI_INC_COLORFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
