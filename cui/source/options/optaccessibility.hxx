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
#ifndef _SVX_OPTACCESSIBILITY_HXX
#define _SVX_OPTACCESSIBILITY_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
struct SvxAccessibilityOptionsTabPage_Impl;
class SvxAccessibilityOptionsTabPage : public SfxTabPage
{
    FixedLine       m_aMiscellaneousLabel;      // FL_MISCELLANEOUS
    CheckBox        m_aAccessibilityTool;       // CB_ACCESSIBILITY_TOOL
    CheckBox        m_aTextSelectionInReadonly; // CB_TEXTSELECTION
    CheckBox        m_aAnimatedGraphics;        // CB_ANIMATED_GRAPHICS
    CheckBox        m_aAnimatedTexts;           // CB_ANIMATED_TEXTS
    CheckBox        m_aTipHelpCB;               // CB_TIPHELP
    NumericField    m_aTipHelpNF;               // NF_TIPHELP
    FixedText       m_aTipHelpFT;               // FT_TIPHELP
    FixedLine       m_aHCOptionsLabel;          // FL_HC_OPTIONS
    CheckBox        m_aAutoDetectHC;            // CB_AUTO_DETECT_HC
    CheckBox        m_aAutomaticFontColor;      // CB_AUTOMATIC_FONT_COLOR
    CheckBox        m_aPagePreviews;            // CB_PAGEPREVIEWS

    DECL_LINK(TipHelpHdl, CheckBox*);
    void EnableTipHelp(sal_Bool bCheck);

    SvxAccessibilityOptionsTabPage_Impl* m_pImpl;

    SvxAccessibilityOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
public:

    virtual ~SvxAccessibilityOptionsTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
