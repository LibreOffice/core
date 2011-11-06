/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

