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
#ifndef _CHART2_TP_SERIESTOAXIS_HXX
#define _CHART2_TP_SERIESTOAXIS_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
// header for FixedText
#include <vcl/fixed.hxx>
// header for CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for MetricField
#include <vcl/field.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

class SchOptionTabPage : public SfxTabPage
{
public:
    SchOptionTabPage(Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchOptionTabPage();

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rInAttrs);
    virtual sal_Bool FillItemSet(SfxItemSet& rOutAttrs);
    virtual void Reset(const SfxItemSet& rInAttrs);

    void Init( bool bProvidesSecondaryYAxis, bool bProvidesOverlapAndGapWidth, bool bProvidesBarConnectors );

private: //methods
    void AdaptControlPositionsAndVisibility();

private: //member
    FixedLine   aGrpAxis;
    RadioButton aRbtAxis1;
    RadioButton aRbtAxis2;

    FixedLine   aGrpBar;
    FixedText   aFTGap;
    MetricField aMTGap;
    FixedText   aFTOverlap;
    MetricField aMTOverlap;
    CheckBox    aCBConnect;
    CheckBox    aCBAxisSideBySide;

    FixedLine   m_aFL_PlotOptions;
    FixedText   m_aFT_MissingValues;
    RadioButton m_aRB_DontPaint;
    RadioButton m_aRB_AssumeZero;
    RadioButton m_aRB_ContinueLine;

    CheckBox    m_aCBIncludeHiddenCells;

    DECL_LINK(EnableHdl, RadioButton * );

    sal_Int32   m_nAllSeriesAxisIndex;

    bool m_bProvidesSecondaryYAxis;
    bool m_bProvidesOverlapAndGapWidth;
    bool m_bProvidesBarConnectors;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
