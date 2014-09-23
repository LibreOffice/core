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
    CheckBox*       m_pAccessibilityTool;
    CheckBox*       m_pTextSelectionInReadonly;
    CheckBox*       m_pAnimatedGraphics;
    CheckBox*       m_pAnimatedTexts;
    CheckBox*       m_pTipHelpCB;
    NumericField*   m_pTipHelpNF;
    CheckBox*       m_pAutoDetectHC;
    CheckBox*       m_pAutomaticFontColor;
    CheckBox*       m_pPagePreviews;

    DECL_LINK(TipHelpHdl, CheckBox*);
    void EnableTipHelp(bool bCheck);

    SvxAccessibilityOptionsTabPage_Impl* m_pImpl;

    SvxAccessibilityOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
public:

    virtual ~SvxAccessibilityOptionsTabPage();

    static SfxTabPage*  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
