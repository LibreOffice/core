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
#ifndef _CHART2_TP_AXISLABEL_HXX
#define _CHART2_TP_AXISLABEL_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/wrapfield.hxx>
#include <svx/orienthelper.hxx>
#include "TextDirectionListBox.hxx"

namespace chart
{

/**
*/

class SchAxisLabelTabPage : public SfxTabPage
{
private:
    CheckBox*            m_pCbShowDescription;

    FixedText*           m_pFlOrder;
    RadioButton*         m_pRbSideBySide;
    RadioButton*         m_pRbUpDown;
    RadioButton*         m_pRbDownUp;
    RadioButton*         m_pRbAuto;

    FixedText*          m_pFlTextFlow;
    CheckBox*            m_pCbTextOverlap;
    CheckBox*            m_pCbTextBreak;
    FixedText*               m_pFtABCD;
    FixedText*           m_pFlOrient;
    svx::DialControl*    m_pCtrlDial;
    FixedText*           m_pFtRotate;
    svx::WrapField*      m_pNfRotate;
    TriStateBox*         m_pCbStacked;
    svx::OrientationHelper* m_pOrientHlp;

    FixedText*               m_pFtTextDirection;
    TextDirectionListBox*    m_pLbTextDirection;

    sal_Bool                m_bShowStaggeringControls;

    sal_Int32           m_nInitialDegrees;
    bool                m_bHasInitialDegrees;       /// false = DialControl in tristate
    bool                m_bInitialStacking;
    bool                m_bHasInitialStacking;      /// false = checkbox in tristate
    bool                m_bComplexCategories;

    DECL_LINK ( ToggleShowLabel, void* );

public:
    SchAxisLabelTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SchAxisLabelTabPage();

    void    Construct();

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );

    void ShowStaggeringControls( sal_Bool bShowStaggeringControls );
    void SetComplexCategories( bool bComplexCategories );
};
} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
