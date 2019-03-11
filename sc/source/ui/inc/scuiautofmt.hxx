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
#ifndef INCLUDED_SC_SOURCE_UI_INC_SCUIAUTOFMT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SCUIAUTOFMT_HXX

#include <vcl/customweld.hxx>
#include "autofmt.hxx"

class ScAutoFormatDlg : public weld::GenericDialogController
{
public:
    ScAutoFormatDlg(weld::Window* pParent,
                    ScAutoFormat* pAutoFormat,
                    const ScAutoFormatData* pSelFormatData,
                    const ScViewData *pViewData);
    virtual ~ScAutoFormatDlg() override;

    sal_uInt16 GetIndex() const { return nIndex; }
    OUString GetCurrFormatName();

private:
    OUString const        aStrTitle;
    OUString const        aStrLabel;
    OUString const        aStrClose;
    OUString const        aStrDelMsg;
    OUString const        aStrRename;

    ScAutoFormat*           pFormat;
    const ScAutoFormatData* pSelFmtData;
    sal_uInt16                  nIndex;
    bool                    bCoreDataChanged;
    bool                    bFmtInserted;

    ScAutoFmtPreview m_aWndPreview;
    std::unique_ptr<weld::TreeView> m_xLbFormat;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnRemove;
    std::unique_ptr<weld::Button> m_xBtnRename;
    std::unique_ptr<weld::CheckButton> m_xBtnNumFormat;
    std::unique_ptr<weld::CheckButton> m_xBtnBorder;
    std::unique_ptr<weld::CheckButton> m_xBtnFont;
    std::unique_ptr<weld::CheckButton> m_xBtnPattern;
    std::unique_ptr<weld::CheckButton> m_xBtnAlignment;
    std::unique_ptr<weld::CheckButton> m_xBtnAdjust;
    std::unique_ptr<weld::CustomWeld> m_xWndPreview;

    void Init           ();
    void UpdateChecks   ();

    DECL_LINK( CheckHdl, weld::ToggleButton&, void );
    DECL_LINK( AddHdl, weld::Button&, void );
    DECL_LINK( RemoveHdl, weld::Button&, void );
    DECL_LINK( SelFmtHdl, weld::TreeView&, void );
    DECL_LINK( CloseHdl, weld::Button&, void );
    DECL_LINK( DblClkHdl, weld::TreeView&, void );
    DECL_LINK( RenameHdl, weld::Button&, void );

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
