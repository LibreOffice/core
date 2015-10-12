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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_AXISLABEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_AXISLABEL_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/orienthelper.hxx>
#include "TextDirectionListBox.hxx"

namespace chart
{

/**
*/

class SchAxisLabelTabPage : public SfxTabPage
{
private:
    VclPtr<CheckBox>            m_pCbShowDescription;

    VclPtr<FixedText>           m_pFlOrder;
    VclPtr<RadioButton>         m_pRbSideBySide;
    VclPtr<RadioButton>         m_pRbUpDown;
    VclPtr<RadioButton>         m_pRbDownUp;
    VclPtr<RadioButton>         m_pRbAuto;

    VclPtr<FixedText>           m_pFlTextFlow;
    VclPtr<CheckBox>            m_pCbTextOverlap;
    VclPtr<CheckBox>            m_pCbTextBreak;
    VclPtr<FixedText>           m_pFtABCD;
    VclPtr<FixedText>           m_pFlOrient;
    VclPtr<svx::DialControl>    m_pCtrlDial;
    VclPtr<FixedText>           m_pFtRotate;
    VclPtr<NumericField>        m_pNfRotate;
    VclPtr<TriStateBox>         m_pCbStacked;
    svx::OrientationHelper* m_pOrientHlp;

    VclPtr<FixedText>               m_pFtTextDirection;
    VclPtr<TextDirectionListBox>    m_pLbTextDirection;

    bool                m_bShowStaggeringControls;

    sal_Int32           m_nInitialDegrees;
    bool                m_bHasInitialDegrees;       /// false = DialControl in tristate
    bool                m_bInitialStacking;
    bool                m_bHasInitialStacking;      /// false = checkbox in tristate
    bool                m_bComplexCategories;

    DECL_LINK_TYPED ( ToggleShowLabel, Button*, void );

public:
    SchAxisLabelTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SchAxisLabelTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* rInAttrs );
    virtual bool FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void Reset( const SfxItemSet* rInAttrs ) override;

    void ShowStaggeringControls( bool bShowStaggeringControls );
    void SetComplexCategories( bool bComplexCategories );
};
} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
