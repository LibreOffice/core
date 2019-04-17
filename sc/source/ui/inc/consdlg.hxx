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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONSDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONSDLG_HXX

#include <vcl/lstbox.hxx>
#include <vcl/layout.hxx>
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

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;

    virtual bool    IsRefInputMode() const override { return true; }
    virtual void    SetActive() override;

    virtual void    Close() override;
    virtual void    Deactivate() override;

private:
    OUString const         aStrUndefined;

    ScConsolidateParam const  theConsData;
    ScViewData&         rViewData;
    ScDocument* const         pDoc;
    std::unique_ptr<ScRangeUtil>  pRangeUtil;
    std::unique_ptr<ScAreaData[]> pAreaData;
    size_t              nAreaDataCount;
    sal_uInt16 const          nWhichCons;
    bool                bDlgLostFocus;

    formula::WeldRefEdit*   m_pRefInputEdit;

    std::unique_ptr<weld::ComboBox> m_xLbFunc;
    std::unique_ptr<weld::TreeView> m_xLbConsAreas;

    std::unique_ptr<weld::ComboBox> m_xLbDataArea;
    std::unique_ptr<formula::WeldRefEdit> m_xEdDataArea;
    std::unique_ptr<formula::WeldRefButton> m_xRbDataArea;

    std::unique_ptr<weld::ComboBox> m_xLbDestArea;
    std::unique_ptr<formula::WeldRefEdit> m_xEdDestArea;
    std::unique_ptr<formula::WeldRefButton> m_xRbDestArea;

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
    bool VerifyEdit(formula::WeldRefEdit* pEd);

    DECL_LINK( OkHdl,    weld::Button&, void );
    DECL_LINK( ClickHdl, weld::Button&, void );
    DECL_LINK( GetFocusHdl, weld::Widget&, void );
    DECL_LINK( GetEditFocusHdl, formula::WeldRefEdit&, void );
    DECL_LINK( ModifyHdl, formula::WeldRefEdit&, void );
    DECL_LINK( SelectTVHdl, weld::TreeView&, void );
    DECL_LINK( SelectCBHdl, weld::ComboBox&, void );

    static ScSubTotalFunc  LbPosToFunc( sal_Int32 nPos );
    static sal_Int32      FuncToLbPos( ScSubTotalFunc eFunc );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_CONSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
