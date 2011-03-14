/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _PGGRID_HXX
#define _PGGRID_HXX

#include <sfx2/tabdlg.hxx>
#include <colex.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
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
    FixedText       aLinesRangeFT;

    FixedText       aTextSizeFT;
    MetricField     aTextSizeMF;

    FixedText       aCharsPerLineFT;
    NumericField    aCharsPerLineNF;
    FixedText       aCharsRangeFT;

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

    Window*         aControls[20];

    sal_Int32       m_nRubyUserValue;
    sal_Bool        m_bRubyUserValue;
    Size            m_aPageSize;
    sal_Bool        m_bVertical;
    sal_Bool        m_bSquaredMode;
    sal_Bool        m_bHRulerChanged;
    sal_Bool        m_bVRulerChanged;

    SwTextGridPage(Window *pParent, const SfxItemSet &rSet);
    ~SwTextGridPage();

    void UpdatePageSize(const SfxItemSet& rSet);
    void PutGridItem(SfxItemSet& rSet);
    void SetLinesOrCharsRanges(FixedText & rField, const sal_Int32 nValue );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
