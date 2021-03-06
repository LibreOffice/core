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

#pragma once

#include <address.hxx>

#include "anyrefdg.hxx"

class ScDocument;
class ScViewData;
class SfxStringItem;

class ScPrintAreasDlg : public ScAnyRefDlgController
{
public:
    ScPrintAreasDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent);
    virtual ~ScPrintAreasDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual void    AddRefEntry() override;

    virtual bool    IsTableLocked() const override;

    virtual void    SetActive() override;
    virtual void    Deactivate() override;
    virtual void    Close() override;

private:
    bool            bDlgLostFocus;
    ScDocument*     pDoc;
    ScViewData*     pViewData;
    SCTAB           nCurTab;

    formula::RefEdit* m_pRefInputEdit;

    std::unique_ptr<weld::ComboBox> m_xLbPrintArea;
    std::unique_ptr<formula::RefEdit> m_xEdPrintArea;
    std::unique_ptr<formula::RefButton> m_xRbPrintArea;

    std::unique_ptr<weld::ComboBox> m_xLbRepeatRow;
    std::unique_ptr<formula::RefEdit> m_xEdRepeatRow;
    std::unique_ptr<formula::RefButton> m_xRbRepeatRow;

    std::unique_ptr<weld::ComboBox> m_xLbRepeatCol;
    std::unique_ptr<formula::RefEdit> m_xEdRepeatCol;
    std::unique_ptr<formula::RefButton> m_xRbRepeatCol;

    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    std::unique_ptr<weld::Frame> m_xPrintFrame;
    std::unique_ptr<weld::Frame> m_xRowFrame;
    std::unique_ptr<weld::Frame> m_xColFrame;

    std::unique_ptr<weld::Label> m_xPrintFrameFT;
    std::unique_ptr<weld::Label> m_xRowFrameFT;
    std::unique_ptr<weld::Label> m_xColFrameFT;

    void Impl_Reset();
    bool Impl_CheckRefStrings();
    void Impl_FillLists();
    bool Impl_GetItem( const formula::RefEdit* pEd, SfxStringItem& rItem );

    // Handler:
    DECL_LINK( Impl_SelectHdl, weld::ComboBox&, void );
    DECL_LINK( Impl_ModifyHdl, formula::RefEdit&, void  );
    DECL_LINK( Impl_BtnHdl,    weld::Button&, void );
    DECL_LINK( Impl_GetEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( Impl_GetFocusHdl, weld::Widget&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
