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

#ifndef INCLUDED_SC_SOURCE_UI_INC_AREASDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_AREASDLG_HXX

#include "address.hxx"

#include <svl/stritem.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include "anyrefdg.hxx"

class ScDocument;
class ScViewData;

class ScPrintAreasDlg : public ScAnyRefDlg
{
public:
                    ScPrintAreasDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent );
                    virtual ~ScPrintAreasDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual void    AddRefEntry() SAL_OVERRIDE;

    virtual bool    IsTableLocked() const SAL_OVERRIDE;

    virtual void    SetActive() SAL_OVERRIDE;
    virtual void    Deactivate() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

private:
    ListBox*                 pLbPrintArea;
    formula::RefEdit*        pEdPrintArea;
    formula::RefButton*      pRbPrintArea;

    ListBox*                 pLbRepeatRow;
    formula::RefEdit*        pEdRepeatRow;
    formula::RefButton*      pRbRepeatRow;

    ListBox*                 pLbRepeatCol;
    formula::RefEdit*        pEdRepeatCol;
    formula::RefButton*      pRbRepeatCol;

    OKButton*        pBtnOk;
    CancelButton*    pBtnCancel;

    bool            bDlgLostFocus;
    formula::RefEdit*       pRefInputEdit;
    ScDocument*     pDoc;
    ScViewData*     pViewData;
    SCTAB           nCurTab;

    void Impl_Reset();
    bool Impl_CheckRefStrings();
    void Impl_FillLists();
    bool Impl_GetItem( Edit* pEd, SfxStringItem& rItem );

    // Handler:
    DECL_LINK( Impl_SelectHdl,      ListBox*    );
    DECL_LINK( Impl_ModifyHdl,      formula::RefEdit*  );
    DECL_LINK( Impl_BtnHdl,         PushButton* );
    DECL_LINK( Impl_GetFocusHdl,    Control*    );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
