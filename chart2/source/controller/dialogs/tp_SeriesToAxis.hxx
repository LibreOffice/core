/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_SeriesToAxis.hxx,v $
 * $Revision: 1.5 $
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
private:
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

    DECL_LINK(EnableHdl, RadioButton * );

    sal_Int32   m_nAllSeriesAxisIndex;

public:
    SchOptionTabPage(Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchOptionTabPage();

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rInAttrs);
    virtual BOOL FillItemSet(SfxItemSet& rOutAttrs);
    virtual void Reset(const SfxItemSet& rInAttrs);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
