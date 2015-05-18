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

#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <svl/itemset.hxx>
#include "TextDirectionListBox.hxx"
#include <svx/dialcontrol.hxx>
#include <sfx2/tabdlg.hxx>

class SvNumberFormatter;

namespace chart
{

class DataLabelResources
{
public:
    DataLabelResources( VclBuilderContainer* pWindow, vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~DataLabelResources();

    bool FillItemSet(SfxItemSet* rOutAttrs) const;
    void Reset(const SfxItemSet& rInAttrs);

    void SetNumberFormatter( SvNumberFormatter* pFormatter );

private:
    VclPtr<CheckBox>            m_pCBNumber;
    VclPtr<PushButton>          m_pPB_NumberFormatForValue;
    VclPtr<CheckBox>            m_pCBPercent;
    VclPtr<PushButton>          m_pPB_NumberFormatForPercent;
    VclPtr<FixedText>           m_pFT_NumberFormatForPercent;
    VclPtr<CheckBox>            m_pCBCategory;
    VclPtr<CheckBox>            m_pCBSymbol;

    VclPtr<VclHBox>             m_pSeparatorResources;
    VclPtr<ListBox>             m_pLB_Separator;
    OUString             m_aEntryMap[NUMBER_SEPARATORS];

    VclPtr<VclHBox>             m_pBxLabelPlacement;
    VclPtr<ListBox>             m_pLB_LabelPlacement;

    VclPtr<VclHBox>             m_pBxOrientation;
    VclPtr<svx::DialControl>    m_pDC_Dial;
    VclPtr<FixedText>           m_pFT_Dial;
    VclPtr<NumericField>        m_pNF_Degrees;

    VclPtr<VclHBox>             m_pBxTextDirection;
    VclPtr<TextDirectionListBox>    m_pLB_TextDirection;

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

    VclPtr<vcl::Window>             m_pWindow;
    SfxItemPool*        m_pPool;

    DECL_LINK(NumberFormatDialogHdl, PushButton * );
    DECL_LINK(CheckHdl, CheckBox* );
    void EnableControls();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
