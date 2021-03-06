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

#pragma once

#include <global.hxx>
#include "anyrefdg.hxx"

class ScViewData;
class ScDocument;
class ScRangeUtil;
class ScAreaData;

class ScConsolidateDlg : public ScAnyRefDlgController
{
public:
    ScConsolidateDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                     const SfxItemSet& rArgSet);
    virtual ~ScConsolidateDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;

    virtual bool    IsRefInputMode() const override { return true; }
    virtual void    SetActive() override;

    virtual void    Close() override;
    virtual void    Deactivate() override;

private:
    OUString            aStrUndefined;

    ScConsolidateParam  theConsData;
    ScViewData&         rViewData;
    ScDocument&         rDoc;
    std::unique_ptr<ScAreaData[]> pAreaData;
    size_t              nAreaDataCount;
    sal_uInt16          nWhichCons;
    bool                bDlgLostFocus;

    formula::RefEdit*   m_pRefInputEdit;

    std::unique_ptr<weld::ComboBox> m_xLbFunc;
    std::unique_ptr<weld::TreeView> m_xLbConsAreas;

    std::unique_ptr<weld::ComboBox> m_xLbDataArea;
    std::unique_ptr<formula::RefEdit> m_xEdDataArea;
    std::unique_ptr<formula::RefButton> m_xRbDataArea;

    std::unique_ptr<weld::ComboBox> m_xLbDestArea;
    std::unique_ptr<formula::RefEdit> m_xEdDestArea;
    std::unique_ptr<formula::RefButton> m_xRbDestArea;

    std::unique_ptr<weld::Expander> m_xExpander;
    std::unique_ptr<weld::CheckButton> m_xBtnByRow;
    std::unique_ptr<weld::CheckButton> m_xBtnByCol;

    std::unique_ptr<weld::CheckButton> m_xBtnRefs;

    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnRemove;

    std::unique_ptr<weld::Label> m_xDataFT;
    std::unique_ptr<weld::Label> m_xDestFT;

    void Init               ();
    void FillAreaLists      ();
    bool VerifyEdit(formula::RefEdit* pEd);

    DECL_LINK( OkHdl,    weld::Button&, void );
    DECL_LINK( ClickHdl, weld::Button&, void );
    DECL_LINK( GetFocusHdl, weld::Widget&, void );
    DECL_LINK( GetEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( ModifyHdl, formula::RefEdit&, void );
    DECL_LINK( SelectTVHdl, weld::TreeView&, void );
    DECL_LINK( SelectCBHdl, weld::ComboBox&, void );

    static ScSubTotalFunc  LbPosToFunc( sal_Int32 nPos );
    static sal_Int32      FuncToLbPos( ScSubTotalFunc eFunc );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
