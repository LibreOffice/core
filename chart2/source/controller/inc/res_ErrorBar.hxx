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
#pragma once

#include <memory>
#include <tools/link.hxx>
#include <svl/itemset.hxx>
#include <svx/chrtitem.hxx>
#include "RangeSelectionListener.hxx"

namespace com::sun::star::chart2 { class XChartDocument; }
namespace weld { class Builder; }
namespace weld { class Button; }
namespace weld { class CheckButton; }
namespace weld { class ComboBox; }
namespace weld { class DialogController; }
namespace weld { class Entry; }
namespace weld { class Frame; }
namespace weld { class Image; }
namespace weld { class Label; }
namespace weld { class MetricSpinButton; }
namespace weld { class RadioButton; }
namespace weld { class ToggleButton; }
namespace weld { class Widget; }

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
        weld::Builder* pParent, weld::DialogController* pControllerDialog, const SfxItemSet& rInAttrs, bool bNoneAvailable, chart::ErrorBarResources::tErrorBarType eType = ERROR_BAR_Y);
    virtual ~ErrorBarResources();

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );
    void SetErrorBarType( tErrorBarType eNewType );
    void SetChartDocumentForRangeChoosing(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDocument );
    void Reset(const SfxItemSet& rInAttrs);
    void FillItemSet(SfxItemSet& rOutAttrs) const;

    void FillValueSets();

    // ____ RangeSelectionListenerParent ____
    virtual void listeningFinished( const OUString & rNewRange ) override;
    virtual void disposingRangeSelection() override;

private:
    SvxChartKindError    m_eErrorKind;
    SvxChartIndicate     m_eIndicate;

    bool                 m_bErrorKindUnique;
    bool                 m_bIndicatorUnique;
    bool                 m_bRangePosUnique;
    bool                 m_bRangeNegUnique;

    tErrorBarType        m_eErrorBarType;
    sal_uInt16           m_nConstDecimalDigits;
    sal_Int64            m_nConstSpinSize;
    double               m_fPlusValue;
    double               m_fMinusValue;

    weld::DialogController* m_pController;
    std::unique_ptr< RangeSelectionHelper >  m_apRangeSelectionHelper;
    weld::Entry*         m_pCurrentRangeChoosingField;
    bool                 m_bHasInternalDataProvider;
    bool                 m_bEnableDataTableDialog;


    // category
    std::unique_ptr<weld::RadioButton> m_xRbNone;
    std::unique_ptr<weld::RadioButton> m_xRbConst;
    std::unique_ptr<weld::RadioButton> m_xRbPercent;
    std::unique_ptr<weld::RadioButton> m_xRbFunction;
    std::unique_ptr<weld::RadioButton> m_xRbRange;
    std::unique_ptr<weld::ComboBox> m_xLbFunction;

    // parameters
    std::unique_ptr<weld::Frame> m_xFlParameters;
    std::unique_ptr<weld::Widget> m_xBxPositive;
    std::unique_ptr<weld::MetricSpinButton> m_xMfPositive;
    std::unique_ptr<weld::Entry> m_xEdRangePositive;
    std::unique_ptr<weld::Button> m_xIbRangePositive;
    std::unique_ptr<weld::Widget> m_xBxNegative;
    std::unique_ptr<weld::MetricSpinButton> m_xMfNegative;
    std::unique_ptr<weld::Entry> m_xEdRangeNegative;
    std::unique_ptr<weld::Button> m_xIbRangeNegative;
    std::unique_ptr<weld::CheckButton> m_xCbSyncPosNeg;

    // indicator
    std::unique_ptr<weld::RadioButton> m_xRbBoth;
    std::unique_ptr<weld::RadioButton> m_xRbPositive;
    std::unique_ptr<weld::RadioButton> m_xRbNegative;
    std::unique_ptr<weld::Image> m_xFiBoth;
    std::unique_ptr<weld::Image> m_xFiPositive;
    std::unique_ptr<weld::Image> m_xFiNegative;

    std::unique_ptr<weld::Label> m_xUIStringPos;
    std::unique_ptr<weld::Label> m_xUIStringNeg;
    std::unique_ptr<weld::Label> m_xUIStringRbRange;

    DECL_LINK( CategoryChosen, weld::ToggleButton&, void );
    DECL_LINK( CategoryChosen2, weld::ComboBox&, void );
    DECL_LINK( SynchronizePosAndNeg, weld::ToggleButton&, void );
    DECL_LINK( PosValueChanged, weld::MetricSpinButton&, void );
    DECL_LINK( IndicatorChanged, weld::ToggleButton&, void );
    DECL_LINK( ChooseRange, weld::Button&, void );
    DECL_LINK( RangeChanged, weld::Entry&, void );

    void UpdateControlStates();
    void isRangeFieldContentValid(weld::Entry& rEdit);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
