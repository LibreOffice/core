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

#include <dlg_ObjectProperties.hxx>
#include <strings.hrc>
#include "tp_AxisLabel.hxx"
#include "tp_DataLabel.hxx"
#include "tp_LegendPosition.hxx"
#include "tp_PointGeometry.hxx"
#include "tp_Scale.hxx"
#include "tp_AxisPositions.hxx"
#include "tp_ErrorBars.hxx"
#include "tp_Trendline.hxx"
#include "tp_SeriesToAxis.hxx"
#include "tp_TitleRotation.hxx"
#include "tp_PolarOptions.hxx"
#include "tp_DataPointOption.hxx"
#include <ResId.hxx>
#include <ViewElementListProvider.hxx>
#include <ChartModelHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <ObjectNameProvider.hxx>
#include <DiagramHelper.hxx>
#include <NumberFormatterWrapper.hxx>
#include <AxisHelper.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <ChartModel.hxx>
#include <CommonConverters.hxx>
#include <RegressionCalculationHelper.hxx>

#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <svl/intitem.hxx>
#include <svl/languageoptions.hxx>

#include <svx/svxids.hrc>

#include <svx/drawitem.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svxgrahicitem.hxx>

#include <svx/dialogs.hrc>
#include <editeng/flstitem.hxx>

#include <svx/flagsdef.hxx>
#include <svx/numinf.hxx>

#include <svl/cjkoptions.hxx>
#include <tools/diagnose_ex.h>

namespace com::sun::star::chart2 { class XChartType; }
namespace com::sun::star::chart2 { class XDataSeries; }

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::beans::XPropertySet;

ObjectPropertiesDialogParameter::ObjectPropertiesDialogParameter( const OUString& rObjectCID )
        : m_aObjectCID( rObjectCID )
        , m_eObjectType( ObjectIdentifier::getObjectType( m_aObjectCID ) )
        , m_bAffectsMultipleObjects(false)
        , m_aLocalizedName()
        , m_bHasGeometryProperties(false)
        , m_bHasStatisticProperties(false)
        , m_bProvidesSecondaryYAxis(false)
        , m_bProvidesOverlapAndGapWidth(false)
        , m_bProvidesBarConnectors(false)
        , m_bHasAreaProperties(false)
        , m_bHasSymbolProperties(false)
        , m_bHasNumberProperties(false)
        , m_bProvidesStartingAngle(false)
        , m_bProvidesMissingValueTreatments(false)
        , m_bIsPieChartDataPoint(false)
        , m_bHasScaleProperties(false)
        , m_bCanAxisLabelsBeStaggered(false)
        , m_bSupportingAxisPositioning(false)
        , m_bShowAxisOrigin(false)
        , m_bIsCrossingAxisIsCategoryAxis(false)
        , m_bSupportingCategoryPositioning(false)
        , m_aCategories()
        , m_bComplexCategoriesAxis( false )
        , m_nNbPoints( 0 )
{
    OUString aParticleID = ObjectIdentifier::getParticleID( m_aObjectCID );
    m_bAffectsMultipleObjects = (aParticleID == "ALLELEMENTS");
}
ObjectPropertiesDialogParameter::~ObjectPropertiesDialogParameter()
{
}

void ObjectPropertiesDialogParameter::init( const uno::Reference< frame::XModel >& xChartModel )
{
    m_xChartDocument.set( xChartModel, uno::UNO_QUERY );
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( m_aObjectCID, xChartModel );
    uno::Reference< XChartType > xChartType = ChartModelHelper::getChartTypeOfSeries( xChartModel, xSeries );
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );

    bool bHasSeriesProperties = (m_eObjectType==OBJECTTYPE_DATA_SERIES);
    bool bHasDataPointproperties = (m_eObjectType==OBJECTTYPE_DATA_POINT);

    if( bHasSeriesProperties || bHasDataPointproperties )
    {
        m_bHasGeometryProperties = ChartTypeHelper::isSupportingGeometryProperties( xChartType, nDimensionCount );
        m_bHasAreaProperties     = ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount );
        m_bHasSymbolProperties   = ChartTypeHelper::isSupportingSymbolProperties( xChartType, nDimensionCount );
        m_bIsPieChartDataPoint   = bHasDataPointproperties && ChartTypeHelper::isSupportingStartingAngle( xChartType );

        if( bHasSeriesProperties )
        {
            m_bHasStatisticProperties =  ChartTypeHelper::isSupportingStatisticProperties( xChartType, nDimensionCount );
            m_bProvidesSecondaryYAxis =  ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimensionCount );
            m_bProvidesOverlapAndGapWidth =  ChartTypeHelper::isSupportingOverlapAndGapWidthProperties( xChartType, nDimensionCount );
            m_bProvidesBarConnectors =  ChartTypeHelper::isSupportingBarConnectors( xChartType, nDimensionCount );
            m_bProvidesStartingAngle = ChartTypeHelper::isSupportingStartingAngle( xChartType );

            m_bProvidesMissingValueTreatments = ChartTypeHelper::getSupportedMissingValueTreatments( xChartType )
                                            .hasElements();
        }
    }

    if( m_eObjectType == OBJECTTYPE_DATA_ERRORS_X ||
        m_eObjectType == OBJECTTYPE_DATA_ERRORS_Y ||
        m_eObjectType == OBJECTTYPE_DATA_ERRORS_Z)
        m_bHasStatisticProperties = true;

    if( m_eObjectType == OBJECTTYPE_AXIS )
    {
        //show scale properties only for a single axis not for multiselection
        m_bHasScaleProperties = !m_bAffectsMultipleObjects;

        if( m_bHasScaleProperties )
        {
            uno::Reference< XAxis > xAxis( ObjectIdentifier::getAxisForCID( m_aObjectCID, xChartModel ) );
            if( xAxis.is() )
            {
                //no scale page for series axis
                ScaleData aData( xAxis->getScaleData() );
                if( aData.AxisType == chart2::AxisType::SERIES )
                    m_bHasScaleProperties = false;
                if( aData.AxisType != chart2::AxisType::SERIES )
                    m_bHasNumberProperties = true;

                //is the crossing main axis a category axes?:
                uno::Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis( xAxis, xDiagram ) );
                uno::Reference< XAxis > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( xAxis, xCooSys ) );
                if( xCrossingMainAxis.is() )
                {
                    ScaleData aScale( xCrossingMainAxis->getScaleData() );
                    m_bIsCrossingAxisIsCategoryAxis = ( aScale.AxisType == chart2::AxisType::CATEGORY  );
                    if( m_bIsCrossingAxisIsCategoryAxis )
                    {
                        ChartModel* pModel = dynamic_cast<ChartModel*>(xChartModel.get());
                        if (pModel)
                            m_aCategories = DiagramHelper::getExplicitSimpleCategories( *pModel );
                    }
                }

                sal_Int32 nCooSysIndex=0;
                sal_Int32 nDimensionIndex=0;
                sal_Int32 nAxisIndex=0;
                if( AxisHelper::getIndicesForAxis( xAxis, xDiagram, nCooSysIndex, nDimensionIndex, nAxisIndex ) )
                {
                    xChartType = AxisHelper::getFirstChartTypeWithSeriesAttachedToAxisIndex( xDiagram, nAxisIndex );
                    //show positioning controls only if they make sense
                    m_bSupportingAxisPositioning = ChartTypeHelper::isSupportingAxisPositioning( xChartType, nDimensionCount, nDimensionIndex );

                    //show axis origin only for secondary y axis
                    if( nDimensionIndex==1 && nAxisIndex==1 && ChartTypeHelper::isSupportingBaseValue( xChartType ) )
                        m_bShowAxisOrigin = true;

                    if ( nDimensionIndex == 0 && ( aData.AxisType == chart2::AxisType::CATEGORY || aData.AxisType == chart2::AxisType::DATE ) )
                    {
                        ChartModel* pModel = dynamic_cast<ChartModel*>(xChartModel.get());
                        if (pModel)
                        {
                            ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSys, *pModel );
                            m_bComplexCategoriesAxis = aExplicitCategoriesProvider.hasComplexCategories();
                        }

                        if (!m_bComplexCategoriesAxis)
                            m_bSupportingCategoryPositioning = ChartTypeHelper::isSupportingCategoryPositioning( xChartType, nDimensionCount );
                    }
                }
            }
        }

        //no staggering of labels for 3D axis
        m_bCanAxisLabelsBeStaggered = nDimensionCount==2;
    }

    if( m_eObjectType == OBJECTTYPE_DATA_CURVE )
    {
        uno::Reference< data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        Sequence< Reference< data::XLabeledDataSequence > > aDataSeqs( xSource->getDataSequences());
        Sequence< double > aXValues, aYValues;
        bool bXValuesFound = false, bYValuesFound = false;
        m_nNbPoints = 0;
        sal_Int32 i = 0;
        for( i=0;
             ! (bXValuesFound && bYValuesFound) && i<aDataSeqs.getLength();
             ++i )
        {
            try
            {
                Reference< data::XDataSequence > xSeq( aDataSeqs[i]->getValues());
                Reference< XPropertySet > xProp( xSeq, uno::UNO_QUERY_THROW );
                OUString aRole;
                if( xProp->getPropertyValue( "Role" ) >>= aRole )
                {
                    if( !bXValuesFound && aRole == "values-x" )
                    {
                        aXValues = DataSequenceToDoubleSequence( xSeq );
                        bXValuesFound = true;
                    }
                    else if( !bYValuesFound && aRole == "values-y" )
                    {
                        aYValues = DataSequenceToDoubleSequence( xSeq );
                        bYValuesFound = true;
                    }
                }
            }
            catch( const Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
        if( !bXValuesFound && bYValuesFound )
        {
            // initialize with 1, 2, ...
            //first category (index 0) matches with real number 1.0
            aXValues.realloc( aYValues.getLength() );
            for( i=0; i<aXValues.getLength(); ++i )
                aXValues[i] = i+1;
            bXValuesFound = true;
        }

        if( bXValuesFound && bYValuesFound &&
            aXValues.hasElements() &&
            aYValues.hasElements() )
        {
            RegressionCalculationHelper::tDoubleVectorPair aValues(
                RegressionCalculationHelper::cleanup( aXValues, aYValues, RegressionCalculationHelper::isValid()));
            m_nNbPoints = aValues.second.size();
        }
    }

     //create gui name for this object
    {
        if( !m_bAffectsMultipleObjects && m_eObjectType == OBJECTTYPE_AXIS )
        {
            m_aLocalizedName = ObjectNameProvider::getAxisName( m_aObjectCID, xChartModel );
        }
        else if( !m_bAffectsMultipleObjects && ( m_eObjectType == OBJECTTYPE_GRID || m_eObjectType == OBJECTTYPE_SUBGRID ) )
        {
            m_aLocalizedName = ObjectNameProvider::getGridName( m_aObjectCID, xChartModel );
        }
        else if( !m_bAffectsMultipleObjects && m_eObjectType == OBJECTTYPE_TITLE )
        {
            m_aLocalizedName = ObjectNameProvider::getTitleName( m_aObjectCID, xChartModel );
        }
        else
        {
            switch( m_eObjectType )
            {
                case OBJECTTYPE_DATA_POINT:
                case OBJECTTYPE_DATA_LABEL:
                case OBJECTTYPE_DATA_LABELS:
                case OBJECTTYPE_DATA_ERRORS_X:
                case OBJECTTYPE_DATA_ERRORS_Y:
                case OBJECTTYPE_DATA_ERRORS_Z:
                case OBJECTTYPE_DATA_AVERAGE_LINE:
                case OBJECTTYPE_DATA_CURVE:
                case OBJECTTYPE_DATA_CURVE_EQUATION:
                    if( m_bAffectsMultipleObjects )
                        m_aLocalizedName = ObjectNameProvider::getName_ObjectForAllSeries( m_eObjectType );
                    else
                        m_aLocalizedName = ObjectNameProvider::getName_ObjectForSeries( m_eObjectType, m_aObjectCID, m_xChartDocument );
                    break;
                default:
                    m_aLocalizedName = ObjectNameProvider::getName(m_eObjectType,m_bAffectsMultipleObjects);
                    break;
            }
        }
    }
}

const sal_uInt16 nNoArrowNoShadowDlg    = 1101;

void SchAttribTabDlg::setSymbolInformation( std::unique_ptr<SfxItemSet> pSymbolShapeProperties,
                std::unique_ptr<Graphic> pAutoSymbolGraphic )
{
    m_pSymbolShapeProperties = std::move(pSymbolShapeProperties);
    m_pAutoSymbolGraphic = std::move(pAutoSymbolGraphic);
}

void SchAttribTabDlg::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    m_fAxisMinorStepWidthForErrorBarDecimals = fMinorStepWidth;
}

SchAttribTabDlg::SchAttribTabDlg(weld::Window* pParent,
                                 const SfxItemSet* pAttr,
                                 const ObjectPropertiesDialogParameter* pDialogParameter,
                                 const ViewElementListProvider* pViewElementListProvider,
                                 const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier)
    : SfxTabDialogController(pParent, "modules/schart/ui/attributedialog.ui", "AttributeDialog", pAttr)
    , m_pParameter( pDialogParameter )
    , m_pViewElementListProvider( pViewElementListProvider )
    , m_pNumberFormatter(nullptr)
    , m_fAxisMinorStepWidthForErrorBarDecimals(0.1)
    , m_bOKPressed(false)
{
    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    m_pNumberFormatter = aNumberFormatterWrapper.getSvNumberFormatter();

    m_xDialog->set_title(pDialogParameter->getLocalizedName());

    SvtCJKOptions aCJKOptions;

    switch (pDialogParameter->getObjectType())
    {
        case OBJECTTYPE_TITLE:
            AddTabPage("border", SchResId(STR_PAGE_BORDER), RID_SVXPAGE_LINE);
            AddTabPage("area", SchResId(STR_PAGE_AREA), RID_SVXPAGE_AREA);
            AddTabPage("transparent", SchResId(STR_PAGE_TRANSPARENCY), RID_SVXPAGE_TRANSPARENCE);
            AddTabPage("fontname", SchResId(STR_PAGE_FONT), RID_SVXPAGE_CHAR_NAME);
            AddTabPage("effects", SchResId(STR_PAGE_FONT_EFFECTS), RID_SVXPAGE_CHAR_EFFECTS);
            AddTabPage("alignment", SchResId(STR_PAGE_ALIGNMENT), SchAlignmentTabPage::Create);
            if( aCJKOptions.IsAsianTypographyEnabled() )
                AddTabPage("asian", SchResId(STR_PAGE_ASIAN), RID_SVXPAGE_PARA_ASIAN);
            break;

        case OBJECTTYPE_LEGEND:
            AddTabPage("border", SchResId(STR_PAGE_BORDER), RID_SVXPAGE_LINE);
            AddTabPage("area", SchResId(STR_PAGE_AREA), RID_SVXPAGE_AREA);
            AddTabPage("transparent", SchResId(STR_PAGE_TRANSPARENCY), RID_SVXPAGE_TRANSPARENCE);
            AddTabPage("fontname", SchResId(STR_PAGE_FONT), RID_SVXPAGE_CHAR_NAME);
            AddTabPage("effects", SchResId(STR_PAGE_FONT_EFFECTS), RID_SVXPAGE_CHAR_EFFECTS);
            AddTabPage("legendpos", SchResId(STR_PAGE_POSITION), SchLegendPosTabPage::Create);
            if (aCJKOptions.IsAsianTypographyEnabled())
                AddTabPage("asian", SchResId(STR_PAGE_ASIAN), RID_SVXPAGE_PARA_ASIAN);
            break;

        case OBJECTTYPE_DATA_SERIES:
        case OBJECTTYPE_DATA_POINT:
            if( m_pParameter->ProvidesSecondaryYAxis() || m_pParameter->ProvidesOverlapAndGapWidth() || m_pParameter->ProvidesMissingValueTreatments() )
                AddTabPage("options", SchResId(STR_PAGE_OPTIONS),SchOptionTabPage::Create);
            if( m_pParameter->ProvidesStartingAngle())
                AddTabPage("polaroptions", SchResId(STR_PAGE_OPTIONS), PolarOptionsTabPage::Create);
            if (m_pParameter->IsPieChartDataPoint())
                AddTabPage("datapointoption", SchResId(STR_PAGE_OPTIONS), DataPointOptionTabPage::Create);

            if( m_pParameter->HasGeometryProperties() )
                AddTabPage("layout", SchResId(STR_PAGE_LAYOUT), SchLayoutTabPage::Create);

            if(m_pParameter->HasAreaProperties())
            {
                AddTabPage("area", SchResId(STR_PAGE_AREA), RID_SVXPAGE_AREA);
                AddTabPage("transparent", SchResId(STR_PAGE_TRANSPARENCY), RID_SVXPAGE_TRANSPARENCE);
            }
            AddTabPage("border", SchResId( m_pParameter->HasAreaProperties() ? STR_PAGE_BORDER : STR_PAGE_LINE ), RID_SVXPAGE_LINE);
            break;

        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_LABELS:
            AddTabPage("border", SchResId(STR_PAGE_BORDER), RID_SVXPAGE_LINE);
            AddTabPage("datalabels", SchResId(STR_OBJECT_DATALABELS), DataLabelsTabPage::Create);
            AddTabPage("fontname", SchResId(STR_PAGE_FONT), RID_SVXPAGE_CHAR_NAME);
            AddTabPage("effects", SchResId(STR_PAGE_FONT_EFFECTS), RID_SVXPAGE_CHAR_EFFECTS);
            if( aCJKOptions.IsAsianTypographyEnabled() )
                AddTabPage("asian", SchResId(STR_PAGE_ASIAN), RID_SVXPAGE_PARA_ASIAN);

            break;

        case OBJECTTYPE_AXIS:
        {
            if( m_pParameter->HasScaleProperties() )
            {
                AddTabPage("scale", SchResId(STR_PAGE_SCALE), ScaleTabPage::Create);
                //no positioning page for z axes so far as the tickmarks are not shown so far
                AddTabPage("axispos", SchResId(STR_PAGE_POSITIONING), AxisPositionsTabPage::Create);
            }
            AddTabPage("border", SchResId(STR_PAGE_LINE), RID_SVXPAGE_LINE);
            AddTabPage("axislabel", SchResId(STR_OBJECT_LABEL), SchAxisLabelTabPage::Create);
            if( m_pParameter->HasNumberProperties() )
                AddTabPage("numberformat", SchResId(STR_PAGE_NUMBERS), RID_SVXPAGE_NUMBERFORMAT);
            AddTabPage("fontname", SchResId(STR_PAGE_FONT), RID_SVXPAGE_CHAR_NAME);
            AddTabPage("effects", SchResId(STR_PAGE_FONT_EFFECTS), RID_SVXPAGE_CHAR_EFFECTS);
            if( aCJKOptions.IsAsianTypographyEnabled() )
                AddTabPage("asian", SchResId(STR_PAGE_ASIAN), RID_SVXPAGE_PARA_ASIAN);
            break;
        }

        case OBJECTTYPE_DATA_ERRORS_X:
            AddTabPage("xerrorbar", SchResId(STR_PAGE_XERROR_BARS), ErrorBarsTabPage::Create);
            AddTabPage("border", SchResId(STR_PAGE_LINE), RID_SVXPAGE_LINE);
            break;

        case OBJECTTYPE_DATA_ERRORS_Y:
            AddTabPage("yerrorbar", SchResId(STR_PAGE_YERROR_BARS), ErrorBarsTabPage::Create);
            AddTabPage("border", SchResId(STR_PAGE_LINE), RID_SVXPAGE_LINE);
            break;

        case OBJECTTYPE_DATA_ERRORS_Z:
            break;

        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_STOCK_RANGE:
            AddTabPage("border", SchResId(STR_PAGE_LINE), RID_SVXPAGE_LINE);
            break;

        case OBJECTTYPE_DATA_CURVE:
            AddTabPage("trendline", SchResId(STR_PAGE_TRENDLINE_TYPE), TrendlineTabPage::Create);
            AddTabPage("border", SchResId(STR_PAGE_LINE), RID_SVXPAGE_LINE);
            break;

        case OBJECTTYPE_DATA_STOCK_LOSS:
        case OBJECTTYPE_DATA_STOCK_GAIN:
        case OBJECTTYPE_PAGE:
        case OBJECTTYPE_DIAGRAM_FLOOR:
        case OBJECTTYPE_DIAGRAM_WALL:
        case OBJECTTYPE_DIAGRAM:
            AddTabPage("border", SchResId(STR_PAGE_BORDER), RID_SVXPAGE_LINE);
            AddTabPage("area", SchResId(STR_PAGE_AREA), RID_SVXPAGE_AREA);
            AddTabPage("transparent", SchResId(STR_PAGE_TRANSPARENCY), RID_SVXPAGE_TRANSPARENCE);
            break;

        case OBJECTTYPE_LEGEND_ENTRY:
        case OBJECTTYPE_AXIS_UNITLABEL:
        case OBJECTTYPE_UNKNOWN:
            // nothing
            break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            AddTabPage("border", SchResId(STR_PAGE_BORDER), RID_SVXPAGE_LINE);
            AddTabPage("area", SchResId(STR_PAGE_AREA), RID_SVXPAGE_AREA);
            AddTabPage("transparent", SchResId(STR_PAGE_TRANSPARENCY), RID_SVXPAGE_TRANSPARENCE);
            AddTabPage("fontname", SchResId(STR_PAGE_FONT), RID_SVXPAGE_CHAR_NAME);
            AddTabPage("effects", SchResId(STR_PAGE_FONT_EFFECTS), RID_SVXPAGE_CHAR_EFFECTS);
            AddTabPage("numberformat", SchResId(STR_PAGE_NUMBERS), RID_SVXPAGE_NUMBERFORMAT);
            if (SvtLanguageOptions().IsCTLFontEnabled())
            {
                /*  When rotation is supported for equation text boxes, use
                    SchAlignmentTabPage::Create here. The special
                    SchAlignmentTabPage::CreateWithoutRotation can be deleted. */
                AddTabPage("alignment", SchResId(STR_PAGE_ALIGNMENT), SchAlignmentTabPage::CreateWithoutRotation);
            }
            break;
        default:
            break;
    }

    // used to find out if user left the dialog with OK. When OK is pressed but
    // no changes were done, Cancel is returned by the SfxTabDialog. See method
    // DialogWasClosedWithOK.
    GetOKButton().connect_clicked(LINK(this, SchAttribTabDlg, OKPressed));
}

SchAttribTabDlg::~SchAttribTabDlg()
{
}

void SchAttribTabDlg::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "border")
    {
        aSet.Put (SvxColorListItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
        aSet.Put (SvxDashListItem(m_pViewElementListProvider->GetDashList(),SID_DASH_LIST));
        aSet.Put (SvxLineEndListItem(m_pViewElementListProvider->GetLineEndList(),SID_LINEEND_LIST));
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nNoArrowNoShadowDlg));

        if( m_pParameter->HasSymbolProperties() )
        {
            aSet.Put(OfaPtrItem(SID_OBJECT_LIST,m_pViewElementListProvider->GetSymbolList()));
            if( m_pSymbolShapeProperties )
                aSet.Put(SfxTabDialogItem(SID_ATTR_SET,*m_pSymbolShapeProperties));
            if( m_pAutoSymbolGraphic )
                aSet.Put(SvxGraphicItem(*m_pAutoSymbolGraphic));
        }
        rPage.PageCreated(aSet);
    }
    else if (rId == "area")
    {
        aSet.Put(SvxColorListItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
        aSet.Put(SvxGradientListItem(m_pViewElementListProvider->GetGradientList(),SID_GRADIENT_LIST));
        aSet.Put(SvxHatchListItem(m_pViewElementListProvider->GetHatchList(),SID_HATCH_LIST));
        aSet.Put(SvxBitmapListItem(m_pViewElementListProvider->GetBitmapList(),SID_BITMAP_LIST));
        aSet.Put(SvxPatternListItem(m_pViewElementListProvider->GetPatternList(),SID_PATTERN_LIST));
        aSet.Put(SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put(SfxUInt16Item(SID_DLG_TYPE,nNoArrowNoShadowDlg));
        rPage.PageCreated(aSet);
    }
    else if (rId == "transparent")
    {
        aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
        aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nNoArrowNoShadowDlg));
        rPage.PageCreated(aSet);
    }
    else if (rId == "fontname")
    {
        aSet.Put (SvxFontListItem(m_pViewElementListProvider->getFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (rId == "effects")
    {
        aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
        rPage.PageCreated(aSet);
    }
    else if (rId == "axislabel")
    {
        bool bShowStaggeringControls = m_pParameter->CanAxisLabelsBeStaggered();
        static_cast<SchAxisLabelTabPage&>(rPage).ShowStaggeringControls( bShowStaggeringControls );
        dynamic_cast< SchAxisLabelTabPage& >( rPage ).SetComplexCategories( m_pParameter->IsComplexCategoriesAxis() );
    }
    else if (rId == "axispos")
    {
        AxisPositionsTabPage* pPage = dynamic_cast< AxisPositionsTabPage* >( &rPage );
        if(pPage)
        {
            pPage->SetNumFormatter( m_pNumberFormatter );
            if( m_pParameter->IsCrossingAxisIsCategoryAxis() )
            {
                pPage->SetCrossingAxisIsCategoryAxis( m_pParameter->IsCrossingAxisIsCategoryAxis() );
                pPage->SetCategories( m_pParameter->GetCategories() );
            }
            pPage->SupportAxisPositioning( m_pParameter->IsSupportingAxisPositioning() );
            pPage->SupportCategoryPositioning( m_pParameter->IsSupportingCategoryPositioning() );
        }
    }
    else if (rId == "scale")
    {
        ScaleTabPage* pScaleTabPage = dynamic_cast< ScaleTabPage* >( &rPage );
        if(pScaleTabPage)
        {
            pScaleTabPage->SetNumFormatter( m_pNumberFormatter );
            pScaleTabPage->ShowAxisOrigin( m_pParameter->ShowAxisOrigin() );
        }
    }
    else if (rId == "datalabels")
    {
        DataLabelsTabPage* pLabelPage = dynamic_cast< DataLabelsTabPage* >( &rPage );
        if( pLabelPage )
            pLabelPage->SetNumberFormatter( m_pNumberFormatter );
    }
    else if (rId == "numberformat")
    {
        aSet.Put (SvxNumberInfoItem( m_pNumberFormatter, static_cast<sal_uInt16>(SID_ATTR_NUMBERFORMAT_INFO)));
        rPage.PageCreated(aSet);
    }
    else if (rId == "xerrorbar")
    {
        ErrorBarsTabPage * pTabPage = dynamic_cast< ErrorBarsTabPage * >( &rPage );
        OSL_ASSERT( pTabPage );
        if( pTabPage )
        {
            pTabPage->SetAxisMinorStepWidthForErrorBarDecimals( m_fAxisMinorStepWidthForErrorBarDecimals );
            pTabPage->SetErrorBarType( ErrorBarResources::ERROR_BAR_X );
            pTabPage->SetChartDocumentForRangeChoosing( m_pParameter->getDocument());
        }
    }
    else if (rId == "yerrorbar")
    {
        ErrorBarsTabPage * pTabPage = dynamic_cast< ErrorBarsTabPage * >( &rPage );
        OSL_ASSERT( pTabPage );
        if( pTabPage )
        {
            pTabPage->SetAxisMinorStepWidthForErrorBarDecimals( m_fAxisMinorStepWidthForErrorBarDecimals );
            pTabPage->SetErrorBarType( ErrorBarResources::ERROR_BAR_Y );
            pTabPage->SetChartDocumentForRangeChoosing( m_pParameter->getDocument());
        }
    }
    else if (rId == "options")
    {
        SchOptionTabPage* pTabPage = dynamic_cast< SchOptionTabPage* >( &rPage );
        if( pTabPage && m_pParameter )
            pTabPage->Init( m_pParameter->ProvidesSecondaryYAxis(), m_pParameter->ProvidesOverlapAndGapWidth(),
                m_pParameter->ProvidesBarConnectors() );
    }
    else if (rId == "trendline")
    {
        TrendlineTabPage* pTrendlineTabPage = dynamic_cast< TrendlineTabPage* >( &rPage );
        if(pTrendlineTabPage)
        {
            pTrendlineTabPage->SetNumFormatter( m_pNumberFormatter );
            pTrendlineTabPage->SetNbPoints( m_pParameter->getNbPoints() );
        }
    }
}

IMPL_LINK(SchAttribTabDlg, OKPressed, weld::Button&, rButton, void)
{
    m_bOKPressed = true;
    OkHdl(rButton);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
