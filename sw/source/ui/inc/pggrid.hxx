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


#ifndef _PGGRID_HXX
#define _PGGRID_HXX

#include <sfx2/tabdlg.hxx>
#include <colex.hxx>
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#include <svtools/ctrlbox.hxx>
/*--------------------------------------------------------------------
    Description:   TabPage Format/(Styles/)Page/Text grid
 --------------------------------------------------------------------*/

class SwTextGridPage: public SfxTabPage
{
    FixedLine       aGridTypeFL;
    RadioButton     aNoGridRB;
    RadioButton     aLinesGridRB;
    RadioButton     aCharsGridRB;
    CheckBox        aSnapToCharsCB;

    SwPageGridExample   aExampleWN;

    FixedLine       aLayoutFL;

    FixedText       aLinesPerPageFT;
    NumericField    aLinesPerPageNF;

    FixedText       aTextSizeFT;
    MetricField     aTextSizeMF;

    FixedText       aCharsPerLineFT;
    NumericField    aCharsPerLineNF;

    FixedText       aCharWidthFT;
    MetricField     aCharWidthMF;

    FixedText       aRubySizeFT;
    MetricField     aRubySizeMF;

    CheckBox        aRubyBelowCB;

    FixedLine       aDisplayFL;

    CheckBox        aDisplayCB;
    CheckBox        aPrintCB;
    FixedText       aColorFT;
    ColorListBox    aColorLB;

    Window*         aControls[18];

    sal_Int32       m_nRubyUserValue;
    sal_Bool        m_bRubyUserValue;
    Size            m_aPageSize;
    sal_Bool        m_bVertical;
    sal_Bool        m_bSquaredMode;

    SwTextGridPage(Window *pParent, const SfxItemSet &rSet);
    ~SwTextGridPage();

    void UpdatePageSize(const SfxItemSet& rSet);
    void PutGridItem(SfxItemSet& rSet);

    DECL_LINK(GridTypeHdl, RadioButton*);
    DECL_LINK(CharorLineChangedHdl, SpinField*);
    DECL_LINK(TextSizeChangedHdl, SpinField*);
    DECL_LINK(GridModifyHdl, void*);
    DECL_LINK(DisplayGridHdl, CheckBox*);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);

    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );
};

#endif
