/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART2_RES_ERRORBAR_HXX
#define CHART2_RES_ERRORBAR_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/valueset.hxx>
#include <svl/itemset.hxx>
#include <svx/chrtitem.hxx>
#include "chartview/ChartSfxItemIds.hxx"
#include "RangeSelectionButton.hxx"
#include "RangeSelectionListener.hxx"
#include "RangeEdit.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>

#include <memory>

class Dialog;

//.............................................................................
namespace chart
{
//.............................................................................

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
        Window* pParent, Dialog * pParentDialog, const SfxItemSet& rInAttrst,
        bool bNoneAvailable,
        tErrorBarType eType = ERROR_BAR_Y );
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
    virtual void listeningFinished( const ::rtl::OUString & rNewRange );
    virtual void disposingRangeSelection();

private:
    // category
    FixedLine            m_aFlErrorCategory;
    RadioButton          m_aRbNone;
    RadioButton          m_aRbConst;
    RadioButton          m_aRbPercent;
    RadioButton          m_aRbFunction;
    RadioButton          m_aRbRange;
    ListBox              m_aLbFunction;

    // parameters
    FixedLine            m_aFlParameters;
    FixedText            m_aFtPositive;
    MetricField          m_aMfPositive;
    RangeEdit            m_aEdRangePositive;
    RangeSelectionButton m_aIbRangePositive;
    FixedText            m_aFtNegative;
    MetricField          m_aMfNegative;
    RangeEdit            m_aEdRangeNegative;
    RangeSelectionButton m_aIbRangeNegative;
    CheckBox             m_aCbSyncPosNeg;

    // indicator
    FixedLine            m_aFlIndicate;
    RadioButton          m_aRbBoth;
    RadioButton          m_aRbPositive;
    RadioButton          m_aRbNegative;
    FixedImage           m_aFiBoth;
    FixedImage           m_aFiPositive;
    FixedImage           m_aFiNegative;

    SvxChartKindError    m_eErrorKind;
    SvxChartIndicate     m_eIndicate;
    SvxChartRegress      m_eTrendLineType;

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

    Window *             m_pParentWindow;
    Dialog *             m_pParentDialog;
    ::std::auto_ptr< RangeSelectionHelper >
                         m_apRangeSelectionHelper;
    Edit *               m_pCurrentRangeChoosingField;
    bool                 m_bHasInternalDataProvider;
    bool                 m_bDisableDataTableDialog;

    DECL_LINK( CategoryChosen, void * );
    DECL_LINK( SynchronizePosAndNeg, void * );
    DECL_LINK( PosValueChanged, void * );
    DECL_LINK( IndicatorChanged, void * );
    DECL_LINK( ChooseRange, RangeSelectionButton * );
    DECL_LINK( RangeChanged, Edit * );

    void UpdateControlStates();
    bool isRangeFieldContentValid( Edit & rEdit );
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
