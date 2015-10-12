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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTACCESSIBILITY_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTACCESSIBILITY_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
struct SvxAccessibilityOptionsTabPage_Impl;
class SvxAccessibilityOptionsTabPage : public SfxTabPage
{
    VclPtr<CheckBox>       m_pAccessibilityTool;
    VclPtr<CheckBox>       m_pTextSelectionInReadonly;
    VclPtr<CheckBox>       m_pAnimatedGraphics;
    VclPtr<CheckBox>       m_pAnimatedTexts;
    VclPtr<CheckBox>       m_pAutoDetectHC;
    VclPtr<CheckBox>       m_pAutomaticFontColor;
    VclPtr<CheckBox>       m_pPagePreviews;

    SvxAccessibilityOptionsTabPage_Impl* m_pImpl;

public:
    SvxAccessibilityOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxAccessibilityOptionsTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
