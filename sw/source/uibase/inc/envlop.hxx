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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_ENVLOP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_ENVLOP_HXX

#include <svtools/svmedit.hxx>
#include <sfx2/tabdlg.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>

#include "envimg.hxx"

#define GetFldVal(rField)         (rField).Denormalize((rField).GetValue(FUNIT_TWIP))
#define SetFldVal(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FUNIT_TWIP)

class SwEnvPage;
class SwEnvFmtPage;
class SwWrtShell;
class Printer;

class SwEnvPreview : public vcl::Window
{
    void Paint(const Rectangle&) SAL_OVERRIDE;

public:

    SwEnvPreview(vcl::Window * pParent, WinBits nStyle);

protected:
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual Size GetOptimalSize() const SAL_OVERRIDE;
};

class SwEnvDlg : public SfxTabDialog
{
friend class SwEnvPage;
friend class SwEnvFmtPage;
friend class SwEnvPrtPage;
friend class SwEnvPreview;

    SwEnvItem       aEnvItem;
    SwWrtShell      *pSh;
    Printer         *pPrinter;
    SfxItemSet      *pAddresseeSet;
    SfxItemSet      *pSenderSet;
    sal_uInt16      m_nEnvPrintId;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) SAL_OVERRIDE;
    virtual short   Ok() SAL_OVERRIDE;

public:
     SwEnvDlg(vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert);
    virtual ~SwEnvDlg();
};

class SwEnvPage : public SfxTabPage
{
    VclMultiLineEdit* m_pAddrEdit;
    ListBox*      m_pDatabaseLB;
    ListBox*      m_pTableLB;
    ListBox*      m_pDBFieldLB;
    PushButton*   m_pInsertBT;
    CheckBox*     m_pSenderBox;
    VclMultiLineEdit* m_pSenderEdit;
    SwEnvPreview* m_pPreview;

    SwWrtShell*   pSh;
    OUString      sActDBName;

     SwEnvPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwEnvPage();

    DECL_LINK( DatabaseHdl, ListBox * );
    DECL_LINK(FieldHdl, void *);
    DECL_LINK(SenderHdl, void *);

    void InitDatabaseBox();

    SwEnvDlg* GetParentSwEnvDlg() {return (SwEnvDlg*)GetParentDialog();}

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage* Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet* pSet = 0) SAL_OVERRIDE;
            void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rSet) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
