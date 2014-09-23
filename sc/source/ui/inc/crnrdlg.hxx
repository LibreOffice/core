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
#include "rangelst.hxx"
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#include <boost/unordered_map.hpp>

class ScViewData;
class ScDocument;

class ScColRowNameRangesDlg : public ScAnyRefDlg
{
public:
                    ScColRowNameRangesDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                 ScViewData*    ptrViewData );
                    virtual ~ScColRowNameRangesDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;

    virtual bool    IsRefInputMode() const SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

private:
    ListBox*         pLbRange;

    formula::RefEdit*        pEdAssign;
    formula::RefButton*      pRbAssign;
    RadioButton*     pBtnColHead;
    RadioButton*     pBtnRowHead;
    formula::RefEdit*        pEdAssign2;
    formula::RefButton*      pRbAssign2;

    OKButton*        pBtnOk;
    CancelButton*    pBtnCancel;
    PushButton*      pBtnAdd;
    PushButton*      pBtnRemove;

    ScRange         theCurArea;
    ScRange         theCurData;

    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    typedef ::boost::unordered_map< OUString, ScRange, OUStringHash > NameRangeMap;
    NameRangeMap    aRangeMap;
    ScViewData*     pViewData;
    ScDocument*     pDoc;
    formula::RefEdit*       pEdActive;
    bool            bDlgLostFocus;

    void Init               ();
    void UpdateNames        ();
    void UpdateRangeData    ( const ScRange& rRange, bool bColName );
    void SetColRowData( const ScRange& rLabelRange, bool bRef=false);
    void AdjustColRowData( const ScRange& rDataRange, bool bRef=false);
    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( Range1SelectHdl, void * );
    DECL_LINK( Range1DataModifyHdl, void * );
    DECL_LINK( ColClickHdl, void * );
    DECL_LINK( RowClickHdl, void * );
    DECL_LINK( Range2DataModifyHdl, void * );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, void* );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_CRNRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
