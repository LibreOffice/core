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
#ifndef INCLUDED_SVX_HDFT_HXX
#define INCLUDED_SVX_HDFT_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>

#include <svx/pagectrl.hxx>
#include <svx/svxdllapi.h>

namespace svx
{
    SVX_DLLPUBLIC bool ShowBorderBackgroundDlg( Window* pParent, SfxItemSet* pBBSet,
            bool bEnableBackgroundSelector );
}

// class  SvxHFPage ------------------------------------------------------

class SVX_DLLPUBLIC SvxHFPage: public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    virtual bool    FillItemSet( SfxItemSet& rOutSet );
    virtual void    Reset( const SfxItemSet& rSet );

    virtual         ~SvxHFPage();

    void            DisableDeleteQueryBox() { bDisableQueryBox = true; }
    void            EnableBackgroundSelector( bool bNew ) { bEnableBackgroundSelector = bNew; }
    void            EnableDynamicSpacing();

protected:
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

    SvxHFPage( Window* pParent, const SfxItemSet& rSet, sal_uInt16 nSetId );

    FixedText*       m_pPageLbl;
    CheckBox*        m_pTurnOnBox;
    CheckBox*        m_pCntSharedBox;
    CheckBox*        m_pCntSharedFirstBox;
    FixedText*       m_pLMLbl;
    MetricField*     m_pLMEdit;
    FixedText*       m_pRMLbl;
    MetricField*     m_pRMEdit;
    FixedText*       m_pDistFT;
    MetricField*     m_pDistEdit;
    CheckBox*        m_pDynSpacingCB;
    FixedText*       m_pHeightFT;
    MetricField*     m_pHeightEdit;
    CheckBox*        m_pHeightDynBtn;
    SvxPageWindow*   m_pBspWin;
    PushButton*      m_pBackgroundBtn;

    sal_uInt16       nId;
    SfxItemSet*      pBBSet;
    bool             bDisableQueryBox;
    bool             bEnableBackgroundSelector;
    bool             bInReset;

    void            InitHandler();
    DECL_LINK( TurnOnHdl, CheckBox*);
    DECL_LINK(DistModify, void *);
    DECL_LINK(HeightModify, void *);
    DECL_LINK(BorderModify, void *);
    DECL_LINK(BackgroundHdl, void *);

    void            UpdateExample();
    DECL_LINK(RangeHdl, void *);

private:
    SVX_DLLPRIVATE void         ResetBackground_Impl( const SfxItemSet& rSet );
};

// class SvxHeaderPage ---------------------------------------------------

class SVX_DLLPUBLIC SvxHeaderPage : public SvxHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    SvxHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

// class SvxFooterPage ---------------------------------------------------

class SVX_DLLPUBLIC SvxFooterPage : public SvxHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    SVX_DLLPRIVATE SvxFooterPage(   Window* pParent, const SfxItemSet& rSet );
};

class SVX_DLLPUBLIC DeleteHeaderDialog : public MessageDialog
{
public:
    DeleteHeaderDialog(Window *pParent)
        : MessageDialog(pParent, "DeleteHeaderDialog",
            "svx/ui/deleteheaderdialog.ui")
    {
    }
};

class SVX_DLLPUBLIC DeleteFooterDialog : public MessageDialog
{
public:
    DeleteFooterDialog(Window *pParent)
        : MessageDialog(pParent, "DeleteFooterDialog",
            "svx/ui/deletefooterdialog.ui")
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
