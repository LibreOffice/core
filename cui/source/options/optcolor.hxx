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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTCOLOR_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTCOLOR_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

namespace svtools {class EditableColorConfig;class EditableExtendedColorConfig;}
class ColorConfigCtrl_Impl;
class AbstractSvxNameDialog;
class SvxColorOptionsTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

    ListBox* m_pColorSchemeLB;
    PushButton* m_pSaveSchemePB;
    PushButton* m_pDeleteSchemePB;
    ColorConfigCtrl_Impl* m_pColorConfigCT;

    bool bFillItemSetCalled;

    svtools::EditableColorConfig* pColorConfig;
    svtools::EditableExtendedColorConfig* pExtColorConfig;

    DECL_LINK(SchemeChangedHdl_Impl, ListBox*);
    DECL_LINK(SaveDeleteHdl_Impl, PushButton*);
    DECL_LINK(CheckNameHdl_Impl, AbstractSvxNameDialog*);
    void UpdateColorConfig();

public:
    SvxColorOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxColorOptionsTabPage(  );

    static SfxTabPage*  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    virtual int         DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;
    virtual void        FillUserData() SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
