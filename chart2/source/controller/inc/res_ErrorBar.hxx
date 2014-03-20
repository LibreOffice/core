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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RES_ERRORBAR_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RES_ERRORBAR_HXX

#include <vcl/button.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svl/itemset.hxx>
#include <svx/chrtitem.hxx>
#include "chartview/ChartSfxItemIds.hxx"
#include "RangeSelectionListener.hxx"


#include <com/sun/star/chart2/XChartDocument.hpp>

class Dialog;

namespace chart
{

class RangeSelectionHelper;

class ErrorBarResources : public RangeSelectionListenerParent
{
public:
    enum tErrorBarType
    {
        ERROR_BAR_X,
        ERROR_BAR_Y
    };

    ErrorBarResources(
        VclBuilderContainer* pParent, Dialog* pParentDialog, const SfxItemSet& rInAttrs, bool bNoneAvailable, chart::ErrorBarResources::tErrorBarType eType = ERROR_BAR_Y );
    virtual ~ErrorBarResources();

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );
    void SetErrorBarType( tErrorBarType eNewType );
    void SetChartDocumentForRangeChoosing(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument );
    void Reset(const SfxItemSet& rInAttrs);
    sal_Bool FillItemSet(SfxItemSet& rOutAttrs) const;

    void FillValueSets();

    // ____ RangeSelectionListenerParent ____
    virtual void listeningFinished( const OUString & rNewRange );
    virtual void disposingRangeSelection();

private:
    // category
    RadioButton*          m_pRbNone;
    RadioButton*          m_pRbConst;
    RadioButton*          m_pRbPercent;
    RadioButton*          m_pRbFunction;
    RadioButton*          m_pRbRange;
    ListBox*              m_pLbFunction;

    // parameters
    VclFrame*             m_pFlParameters;
    VclBox*               m_pBxPositive;
    MetricField*          m_pMfPositive;
    Edit*                 m_pEdRangePositive;
    PushButton*           m_pIbRangePositive;
    VclBox*               m_pBxNegative;
    MetricField*          m_pMfNegative;
    Edit*                 m_pEdRangeNegative;
    PushButton*           m_pIbRangeNegative;
    CheckBox*             m_pCbSyncPosNeg;

    // indicator
    RadioButton*          m_pRbBoth;
    RadioButton*          m_pRbPositive;
    RadioButton*          m_pRbNegative;
    FixedImage*           m_pFiBoth;
    FixedImage*           m_pFiPositive;
    FixedImage*           m_pFiNegative;

    FixedText*           m_pUIStringPos;
    FixedText*           m_pUIStringNeg;
    FixedText*           m_pUIStringRbRange;

    SvxChartKindError    m_eErrorKind;
    SvxChartIndicate     m_eIndicate;

    bool                 m_bErrorKindUnique;
    bool                 m_bIndicatorUnique;
    bool                 m_bPlusUnique;
    bool                 m_bMinusUnique;
    bool                 m_bRangePosUnique;
    bool                 m_bRangeNegUnique;

    bool                 m_bNoneAvailable;

    tErrorBarType        m_eErrorBarType;
    sal_uInt16           m_nConstDecimalDigits;
    sal_Int64            m_nConstSpinSize;

    Dialog *             m_pParentDialog;
    boost::scoped_ptr< RangeSelectionHelper >  m_apRangeSelectionHelper;
    Edit *               m_pCurrentRangeChoosingField;
    bool                 m_bHasInternalDataProvider;
    bool                 m_bEnableDataTableDialog;

    DECL_LINK( CategoryChosen, void * );
    DECL_LINK( SynchronizePosAndNeg, void * );
    DECL_LINK( PosValueChanged, void * );
    DECL_LINK( IndicatorChanged, void * );
    DECL_LINK( ChooseRange, PushButton * );
    DECL_LINK( RangeChanged, Edit * );

    void UpdateControlStates();
    bool isRangeFieldContentValid( Edit & rEdit );
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
