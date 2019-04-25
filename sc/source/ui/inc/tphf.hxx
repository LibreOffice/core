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

#include <svx/hdft.hxx>

class ScStyleDlg;

class ScHFPage : public SvxHFPage
{
public:
    virtual         ~ScHFPage() override;
    virtual void    dispose() override;

    virtual void    Reset( const SfxItemSet* rSet ) override;
    virtual bool    FillItemSet( SfxItemSet* rOutSet ) override;

    void            SetPageStyle( const OUString& rName )    { aStrPageStyle = rName; }
    void            SetStyleDlg ( ScStyleDlg* pDlg ) { pStyleDlg = pDlg; }

protected:
    ScHFPage(TabPageParent pParent, const SfxItemSet& rSet, sal_uInt16 nSetId);

    virtual void    ActivatePage() override;
    virtual void    DeactivatePage() override;
    virtual void    ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

private:
    SfxItemSet           aDataSet;
    OUString             aStrPageStyle;
    SvxPageUsage         nPageUsage;
    ScStyleDlg*          pStyleDlg;
    std::unique_ptr<weld::Button> m_xBtnEdit;

    DECL_LINK(BtnHdl, weld::Button&, void);
    DECL_LINK( HFEditHdl, void*, void );
    DECL_LINK(TurnOnHdl, weld::ToggleButton&, void);
};

class ScHeaderPage : public ScHFPage
{
    friend class VclPtr<ScHeaderPage>;
public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16*      GetRanges();

private:
    ScHeaderPage(TabPageParent pParent, const SfxItemSet& rSet);
};

class ScFooterPage : public ScHFPage
{
    friend class VclPtr<ScFooterPage>;
public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16*      GetRanges();

private:
    ScFooterPage(TabPageParent pParent, const SfxItemSet& rSet);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
