/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_ErrorBar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:55:20 $
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
#ifndef CHART2_RES_ERRORBAR_HXX
#define CHART2_RES_ERRORBAR_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/valueset.hxx>
#include <svtools/itemset.hxx>
#include <svx/chrtitem.hxx>
#include "chartview/ChartSfxItemIds.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

class ErrorBarResources
{
public:
    enum tErrorBarType
    {
        ERROR_BAR_X,
        ERROR_BAR_Y
    };

    ErrorBarResources( Window* pParent, const SfxItemSet& rInAttrst, tErrorBarType eType = ERROR_BAR_Y );
    virtual ~ErrorBarResources();

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );
    void SetErrorBarType( tErrorBarType eNewType );

    void Reset(const SfxItemSet& rInAttrs);
    BOOL FillItemSet(SfxItemSet& rOutAttrs) const;

    void FillValueSets();

private:
    FixedLine       m_aFlErrorCategory;
    RadioButton     m_aRbtNone;
    RadioButton     m_aRbtVariant;
    RadioButton     m_aRbtSigma;
    RadioButton     m_aRbtPercent;
    RadioButton     m_aRbtBigError;
    RadioButton     m_aRbtConst;
//     RadioButton     m_aRbtRange;
    MetricField     m_aMtrFldPercent;
    MetricField     m_aMtrFldBigError;
    FixedText       m_aFTConstPlus;
    MetricField     m_aMtrFldConstPlus;
    FixedText       m_aFTConstMinus;
    MetricField     m_aMtrFldConstMinus;
    FixedLine       m_aFLIndicate;
    ValueSet        m_aIndicatorSet;

    SvxChartKindError   m_eErrorKind;
    SvxChartIndicate    m_eIndicate;
    SvxChartRegress     m_eTrendLineType;

    bool                m_bErrorKindUnique;
    bool                m_bIndicatorUnique;
    bool                m_bPlusUnique;
    bool                m_bMinusUnique;

    tErrorBarType       m_eErrorBarType;

    DECL_LINK(RBtnClick, Button *);
    DECL_LINK(SelectIndicate, void *);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
