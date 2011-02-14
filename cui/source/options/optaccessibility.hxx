/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

