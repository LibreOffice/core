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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CRNRDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CRNRDLG_HXX

#include "anyrefdg.hxx"
#include <rangelst.hxx>

#include <unordered_map>

class ScViewData;
class ScDocument;

class ScColRowNameRangesDlg : public ScAnyRefDlgController
{
public:
    ScColRowNameRangesDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                          ScViewData* ptrViewData);
    virtual ~ScColRowNameRangesDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual void    Close() override;

private:
    ScRange         theCurArea;
    ScRange         theCurData;

    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    typedef std::unordered_map< OUString, ScRange > NameRangeMap;
    NameRangeMap    aRangeMap;
    ScViewData*     pViewData;
    ScDocument&     rDoc;
    bool            bDlgLostFocus;

    formula::RefEdit* m_pEdActive;
    std::unique_ptr<weld::TreeView> m_xLbRange;

    std::unique_ptr<formula::RefEdit> m_xEdAssign;
    std::unique_ptr<formula::RefButton> m_xRbAssign;
    std::unique_ptr<weld::RadioButton> m_xBtnColHead;
    std::unique_ptr<weld::RadioButton> m_xBtnRowHead;
    std::unique_ptr<formula::RefEdit> m_xEdAssign2;
    std::unique_ptr<formula::RefButton> m_xRbAssign2;

    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnRemove;

    std::unique_ptr<weld::Frame> m_xRangeFrame;
    std::unique_ptr<weld::Label> m_xRangeFT;
    std::unique_ptr<weld::Label> m_xDataFT;

    void Init               ();
    void UpdateNames        ();
    void UpdateRangeData    ( const ScRange& rRange, bool bColName );
    void SetColRowData( const ScRange& rLabelRange, bool bRef=false);
    void AdjustColRowData( const ScRange& rDataRange, bool bRef=false);
    DECL_LINK( CancelBtnHdl, weld::Button&, void );
    DECL_LINK( OkBtnHdl, weld::Button&, void );
    DECL_LINK( AddBtnHdl, weld::Button&, void );
    DECL_LINK( RemoveBtnHdl, weld::Button&, void );
    DECL_LINK( Range1SelectHdl, weld::TreeView&, void );
    DECL_LINK( Range1DataModifyHdl, formula::RefEdit&, void );
    DECL_LINK( ColClickHdl, weld::Button&, void );
    DECL_LINK( RowClickHdl, weld::Button&, void );
    DECL_LINK( Range2DataModifyHdl, formula::RefEdit&, void );
    DECL_LINK( GetEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( LoseEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( GetButtonFocusHdl, formula::RefButton&, void );
    DECL_LINK( LoseButtonFocusHdl, formula::RefButton&, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_CRNRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
