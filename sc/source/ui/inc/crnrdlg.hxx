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
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#include <unordered_map>

class ScViewData;
class ScDocument;

class ScColRowNameRangesDlg : public ScAnyRefDlg
{
public:
                    ScColRowNameRangesDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                 ScViewData*    ptrViewData );
                    virtual ~ScColRowNameRangesDlg() override;
    virtual void    dispose() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual bool    Close() override;

private:
    VclPtr<ListBox>         pLbRange;

    VclPtr<formula::RefEdit>        pEdAssign;
    VclPtr<formula::RefButton>      pRbAssign;
    VclPtr<RadioButton>     pBtnColHead;
    VclPtr<RadioButton>     pBtnRowHead;
    VclPtr<formula::RefEdit>        pEdAssign2;
    VclPtr<formula::RefButton>      pRbAssign2;

    VclPtr<OKButton>        pBtnOk;
    VclPtr<CancelButton>    pBtnCancel;
    VclPtr<PushButton>      pBtnAdd;
    VclPtr<PushButton>      pBtnRemove;

    ScRange         theCurArea;
    ScRange         theCurData;

    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    typedef std::unordered_map< OUString, ScRange > NameRangeMap;
    NameRangeMap    aRangeMap;
    ScViewData*     pViewData;
    ScDocument*     pDoc;
    VclPtr<formula::RefEdit>       pEdActive;
    bool            bDlgLostFocus;

    void Init               ();
    void UpdateNames        ();
    void UpdateRangeData    ( const ScRange& rRange, bool bColName );
    void SetColRowData( const ScRange& rLabelRange, bool bRef=false);
    void AdjustColRowData( const ScRange& rDataRange, bool bRef=false);
    DECL_LINK( CancelBtnHdl, Button*, void );
    DECL_LINK( OkBtnHdl, Button*, void );
    DECL_LINK( AddBtnHdl, Button*, void );
    DECL_LINK( RemoveBtnHdl, Button*, void );
    DECL_LINK( Range1SelectHdl, ListBox&, void );
    DECL_LINK( Range1DataModifyHdl, Edit&, void );
    DECL_LINK( ColClickHdl, Button*, void );
    DECL_LINK( RowClickHdl, Button*, void );
    DECL_LINK( Range2DataModifyHdl, Edit&, void );
    DECL_LINK( GetFocusHdl, Control&, void );
    DECL_LINK( LoseFocusHdl, Control&, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_CRNRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
