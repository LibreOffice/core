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
#ifndef CHART2_RES_DATALABEL_HXX
#define CHART2_RES_DATALABEL_HXX

#include "res_TextSeparator.hxx"
// header for class CheckBox
#include <vcl/button.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
#include "TextDirectionListBox.hxx"

#include <svx/dialcontrol.hxx>

class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class DataLabelResources
{
public:
    DataLabelResources( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~DataLabelResources();

    sal_Bool FillItemSet(SfxItemSet& rOutAttrs) const;
    void Reset(const SfxItemSet& rInAttrs);

    void SetNumberFormatter( SvNumberFormatter* pFormatter );

private:
    CheckBox            m_aCBNumber;
    PushButton          m_aPB_NumberFormatForValue;
    CheckBox            m_aCBPercent;
    PushButton          m_aPB_NumberFormatForPercent;
    CheckBox            m_aCBCategory;
    CheckBox            m_aCBSymbol;

    TextSeparatorResources  m_aSeparatorResources;

    FixedText           m_aFT_LabelPlacement;
    ListBox             m_aLB_LabelPlacement;

    FixedLine           m_aFL_Rotate;
    svx::DialControl    m_aDC_Dial;
    FixedText           m_aFT_Degrees;
    NumericField        m_aNF_Degrees;

    FixedText               m_aFT_TextDirection;
    TextDirectionListBox    m_aLB_TextDirection;

    ::std::map< sal_Int32, sal_uInt16 > m_aPlacementToListBoxMap;
    ::std::map< sal_uInt16, sal_Int32 > m_aListBoxToPlacementMap;

    SvNumberFormatter*  m_pNumberFormatter;
    bool                m_bNumberFormatMixedState;
    bool                m_bPercentFormatMixedState;
    sal_uLong               m_nNumberFormatForValue;
    sal_uLong               m_nNumberFormatForPercent;

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

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
