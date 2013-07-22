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

#ifndef SC_CONSDLG_HXX
#define SC_CONSDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <vcl/morebtn.hxx>
#include "global.hxx"
#include "anyrefdg.hxx"

class ScViewData;
class ScDocument;
class ScRangeUtil;
class ScAreaData;


//============================================================================

class ScConsolidateDlg : public ScAnyRefDlg
{
public:
                    ScConsolidateDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                      const SfxItemSet& rArgSet );
                    ~ScConsolidateDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const { return sal_True; }
    virtual void    SetActive();

    virtual sal_Bool    Close();

protected:
    virtual void    Deactivate();

private:
    ListBox*         pLbFunc;
    ListBox*         pLbConsAreas;

    ListBox*         pLbDataArea;
    formula::RefEdit*        pEdDataArea;
    formula::RefButton*      pRbDataArea;

    ListBox*         pLbDestArea;
    formula::RefEdit*        pEdDestArea;
    formula::RefButton*      pRbDestArea;

    VclExpander*     pExpander;
    CheckBox*        pBtnByRow;
    CheckBox*        pBtnByCol;

    CheckBox*        pBtnRefs;

    OKButton*        pBtnOk;
    CancelButton*    pBtnCancel;
    HelpButton*      pBtnHelp;
    PushButton*      pBtnAdd;
    PushButton*      pBtnRemove;

    String          aStrUndefined;

    ScConsolidateParam  theConsData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    ScRangeUtil*        pRangeUtil;
    ScAreaData*         pAreaData;
    size_t              nAreaDataCount;
    sal_uInt16              nWhichCons;

    formula::RefEdit*           pRefInputEdit;
    sal_Bool                bDlgLostFocus;

#ifdef _CONSDLG_CXX
private:
    void Init               ();
    void FillAreaLists      ();
    sal_Bool VerifyEdit         ( formula::RefEdit* pEd );

    DECL_LINK( OkHdl,        void* );
    DECL_LINK( ClickHdl,     PushButton* );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( ModifyHdl,    formula::RefEdit* );
    DECL_LINK( SelectHdl,    ListBox* );

    ScSubTotalFunc  LbPosToFunc( sal_uInt16 nPos );
    sal_uInt16          FuncToLbPos( ScSubTotalFunc eFunc );
#endif // _CONSDLG_CXX
};



#endif // SC_CONSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
