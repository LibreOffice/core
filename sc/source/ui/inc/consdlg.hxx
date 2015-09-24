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

class ScConsolidateDlg : public ScAnyRefDlg
{
public:
                    ScConsolidateDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                      const SfxItemSet& rArgSet );
                    virtual ~ScConsolidateDlg();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;

    virtual bool    IsRefInputMode() const SAL_OVERRIDE { return true; }
    virtual void    SetActive() SAL_OVERRIDE;

    virtual bool    Close() SAL_OVERRIDE;

protected:
    virtual void    Deactivate() SAL_OVERRIDE;

private:
    VclPtr<ListBox>         pLbFunc;
    VclPtr<ListBox>         pLbConsAreas;

    VclPtr<ListBox>         pLbDataArea;
    VclPtr<formula::RefEdit>        pEdDataArea;
    VclPtr<formula::RefButton>      pRbDataArea;

    VclPtr<ListBox>         pLbDestArea;
    VclPtr<formula::RefEdit>        pEdDestArea;
    VclPtr<formula::RefButton>      pRbDestArea;

    VclPtr<VclExpander>     pExpander;
    VclPtr<CheckBox>        pBtnByRow;
    VclPtr<CheckBox>        pBtnByCol;

    VclPtr<CheckBox>        pBtnRefs;

    VclPtr<OKButton>        pBtnOk;
    VclPtr<CancelButton>    pBtnCancel;
    VclPtr<PushButton>      pBtnAdd;
    VclPtr<PushButton>      pBtnRemove;

    OUString         aStrUndefined;

    ScConsolidateParam  theConsData;
    ScViewData&         rViewData;
    ScDocument*         pDoc;
    ScRangeUtil*        pRangeUtil;
    ScAreaData*         pAreaData;
    size_t              nAreaDataCount;
    sal_uInt16          nWhichCons;

    VclPtr<formula::RefEdit>   pRefInputEdit;
    bool                bDlgLostFocus;

    void Init               ();
    void FillAreaLists      ();
    bool VerifyEdit         ( formula::RefEdit* pEd );

    DECL_LINK_TYPED( OkHdl,    Button*, void );
    DECL_LINK_TYPED( ClickHdl, Button*, void );
    DECL_LINK_TYPED( GetFocusHdl, Control&, void );
    DECL_LINK( ModifyHdl,    formula::RefEdit* );
    DECL_LINK( SelectHdl,    ListBox* );

    static ScSubTotalFunc  LbPosToFunc( sal_Int32 nPos );
    static sal_Int32      FuncToLbPos( ScSubTotalFunc eFunc );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_CONSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
