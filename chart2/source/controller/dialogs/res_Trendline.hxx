/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_Trendline.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:48:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_RES_TRENDLINE_HXX
#define CHART2_RES_TRENDLINE_HXX

#include <vcl/window.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
#include <svtools/itemset.hxx>
#include <svx/chrtitem.hxx>

namespace chart
{

class TrendlineResources
{
public:
    TrendlineResources( Window* pParent, const SfxItemSet& rInAttrs, bool bNoneAvailable );
    virtual ~TrendlineResources();

    void Reset(const SfxItemSet& rInAttrs);
    BOOL FillItemSet(SfxItemSet& rOutAttrs) const;

    void FillValueSets();

    long adjustControlSizes();

private:
    FixedLine m_aFLType;
    RadioButton m_aRBNone;
    RadioButton m_aRBLinear;
    RadioButton m_aRBLogarithmic;
    RadioButton m_aRBExponential;
    RadioButton m_aRBPower;

    FixedBitmap m_aFBNone;
    FixedBitmap m_aFBLinear;
    FixedBitmap m_aFBLogarithmic;
    FixedBitmap m_aFBExponential;
    FixedBitmap m_aFBPower;

    FixedLine m_aFLEquation;
    CheckBox m_aCBShowEquation;
    CheckBox m_aCBShowCorrelationCoeff;

    SvxChartRegress     m_eTrendLineType;

    bool                m_bNoneAvailable;
    bool                m_bTrendLineUnique;

    void UpdateControlStates();
    DECL_LINK( SelectTrendLine, RadioButton * );
};

} //  namespace chart

// CHART2_RES_TRENDLINE_HXX
#endif
