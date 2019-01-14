/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_INSTABLE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_INSTABLE_HXX

#include <sfx2/basedlgs.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <vcl/vclreferencebase.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>

#include "wrtsh.hxx"
#include "autoformatpreview.hxx"
#include <view.hxx>
#include <tblafmt.hxx>
#include <itabenum.hxx>

class SwInsTableDlg : public SfxDialogController
{
    TextFilter      m_aTextFilter;

    SwWrtShell*     pShell;
    SwTableAutoFormatTable* pTableTable;
    SwTableAutoFormat*      pTAutoFormat;

    sal_uInt8       lbIndex;
    sal_uInt8       tbIndex;
    sal_uInt8       minTableIndexInLb;
    sal_uInt8       maxTableIndexInLb;
    sal_Int64       nEnteredValRepeatHeaderNF;

    AutoFormatPreview m_aWndPreview;

    std::unique_ptr<weld::Entry> m_xNameEdit;
    std::unique_ptr<weld::SpinButton> m_xColNF;
    std::unique_ptr<weld::SpinButton> m_xRowNF;
    std::unique_ptr<weld::CheckButton> m_xHeaderCB;
    std::unique_ptr<weld::CheckButton> m_xRepeatHeaderCB;
    std::unique_ptr<weld::SpinButton> m_xRepeatHeaderNF;
    std::unique_ptr<weld::Widget> m_xRepeatGroup;
    std::unique_ptr<weld::CheckButton> m_xDontSplitCB;
    std::unique_ptr<weld::Button> m_xInsertBtn;
    std::unique_ptr<weld::TreeView> m_xLbFormat;
    std::unique_ptr<weld::CustomWeld> m_xWndPreview;

    // Returns 255 if mapping is not possible.
    // This means there cannot be more than 255 autotable style.
    sal_uInt8 lbIndexToTableIndex( const sal_uInt8 listboxIndex );
    void InitAutoTableFormat();

    DECL_LINK(TextFilterHdl, OUString&, bool);
    DECL_LINK(SelFormatHdl, weld::TreeView&, void);
    DECL_LINK(ModifyName, weld::Entry&, void);
    DECL_LINK(ModifyRowCol, weld::SpinButton&, void);
    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void);
    DECL_LINK(RepeatHeaderCheckBoxHdl, weld::ToggleButton&, void);
    DECL_LINK(ModifyRepeatHeaderNF_Hdl, weld::SpinButton&, void);

public:
    SwInsTableDlg(SwView& rView);

    void GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                    SwInsertTableOptions& rInsTableOpts, OUString& rTableAutoFormatName,
                    std::unique_ptr<SwTableAutoFormat>& prTAFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
