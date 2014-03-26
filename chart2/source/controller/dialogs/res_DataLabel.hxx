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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_DATALABEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_DATALABEL_HXX

#define NUMBER_SEPARATORS 4

// header for class CheckBox
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
#include "TextDirectionListBox.hxx"

#include <svx/dialcontrol.hxx>
#include <svx/wrapfield.hxx>
#include <sfx2/tabdlg.hxx>

class SvNumberFormatter;

namespace chart
{

class DataLabelResources
{
public:
    DataLabelResources( VclBuilderContainer* pWindow, Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~DataLabelResources();

    bool FillItemSet(SfxItemSet& rOutAttrs) const;
    void Reset(const SfxItemSet& rInAttrs);

    void SetNumberFormatter( SvNumberFormatter* pFormatter );

private:
    CheckBox*            m_pCBNumber;
    PushButton*          m_pPB_NumberFormatForValue;
    CheckBox*            m_pCBPercent;
    PushButton*          m_pPB_NumberFormatForPercent;
    FixedText*           m_pFT_NumberFormatForPercent;
    CheckBox*            m_pCBCategory;
    CheckBox*            m_pCBSymbol;

    VclHBox*             m_pSeparatorResources;
    ListBox*             m_pLB_Separator;
    OUString             m_aEntryMap[NUMBER_SEPARATORS];

    VclHBox*             m_pBxLabelPlacement;
    ListBox*             m_pLB_LabelPlacement;

    VclHBox*             m_pBxOrientation;
    svx::DialControl*    m_pDC_Dial;
    FixedText*           m_pFT_Dial;
    svx::WrapField*      m_pNF_Degrees;

    VclHBox*             m_pBxTextDirection;
    TextDirectionListBox*    m_pLB_TextDirection;

    ::std::map< sal_Int32, sal_uInt16 > m_aPlacementToListBoxMap;
    ::std::map< sal_uInt16, sal_Int32 > m_aListBoxToPlacementMap;

    SvNumberFormatter*  m_pNumberFormatter;
    bool                m_bNumberFormatMixedState;
    bool                m_bPercentFormatMixedState;
    sal_uLong           m_nNumberFormatForValue;
    sal_uLong           m_nNumberFormatForPercent;

    bool                m_bSourceFormatMixedState;
    bool                m_bPercentSourceMixedState;
    bool                m_bSourceFormatForValue;
    bool                m_bSourceFormatForPercent;

    Window*             m_pWindow;
    SfxItemPool*        m_pPool;

    DECL_LINK(NumberFormatDialogHdl, PushButton * );
    DECL_LINK(CheckHdl, CheckBox* );
    void EnableControls();
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
