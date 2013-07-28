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

#include <svl/zforlist.hxx>
#include "dlg_ObjectProperties.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "TabPages.hrc"
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
#include "ResId.hxx"
#include "ViewElementListProvider.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ObjectNameProvider.hxx"
#include "DiagramHelper.hxx"
#include "NumberFormatterWrapper.hxx"
#include "AxisIndexDefines.hxx"
#include "AxisHelper.hxx"
#include "ExplicitCategoriesProvider.hxx"

#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <svl/intitem.hxx>
#include <svl/languageoptions.hxx>

#include <svx/svxids.hrc>

#include <svx/drawitem.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svxgrahicitem.hxx>

#include <svx/dialogs.hrc>
#include <editeng/flstitem.hxx>
#include <svx/tabline.hxx>

#include <svx/flagsdef.hxx>
#include <svx/numinf.hxx>

#include <svl/cjkoptions.hxx>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

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
        , m_bHasScaleProperties(false)
        , m_bCanAxisLabelsBeStaggered(false)
        , m_bSupportingAxisPositioning(false)
        , m_bShowAxisOrigin(false)
        , m_bIsCrossingAxisIsCategoryAxis(false)
        , m_aCategories()
        , m_xChartDocument( 0 )
        , m_bComplexCategoriesAxis( false )
{
    OUString aParticleID = ObjectIdentifier::getParticleID( m_aObjectCID );
    m_bAffectsMultipleObjects = (aParticleID == "ALLELEMENTS");
}
ObjectPropertiesDialogParameter::~ObjectPropertiesDialogParameter()
{
}
ObjectType ObjectPropertiesDialogParameter::getObjectType() const
{
    return m_eObjectType;
}
OUString ObjectPropertiesDialogParameter::getLocalizedName() const
{
    return m_aLocalizedName;
}

void ObjectPropertiesDialogParameter::init( const uno::Reference< frame::XModel >& xChartModel )
{
    m_xChartDocument.set( xChartModel, uno::UNO_QUERY );
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( m_aObjectCID, xChartModel );
    uno::Reference< XChartType > xChartType = ChartModelHelper::getChartTypeOfSeries( xChartModel, xSeries );
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );

    bool bHasSeriesProperties = (OBJECTTYPE_DATA_SERIES==m_eObjectType);
    bool bHasDataPointproperties = (OBJECTTYPE_DATA_POINT==m_eObjectType);

    if( bHasSeriesProperties || bHasDataPointproperties )
    {
        m_bHasGeometryProperties = ChartTypeHelper::isSupportingGeometryProperties( xChartType, nDimensionCount );
        m_bHasAreaProperties     = ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount );
        m_bHasSymbolProperties   = ChartTypeHelper::isSupportingSymbolProperties( xChartType, nDimensionCount );

        if( bHasSeriesProperties )
        {
            m_bHasStatisticProperties =  ChartTypeHelper::isSupportingStatisticProperties( xChartType, nDimensionCount );
            m_bProvidesSecondaryYAxis =  ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimensionCount, 1 );
            m_bProvidesOverlapAndGapWidth =  ChartTypeHelper::isSupportingOverlapAndGapWidthProperties( xChartType, nDimensionCount );
            m_bProvidesBarConnectors =  ChartTypeHelper::isSupportingBarConnectors( xChartType, nDimensionCount );
            m_bProvidesStartingAngle = ChartTypeHelper::isSupportingStartingAngle( xChartType );

            m_bProvidesMissingValueTreatments = ChartTypeHelper::getSupportedMissingValueTreatments( xChartType )
                                            .getLength() ? true : false;
        }
    }

    if( m_eObjectType == OBJECTTYPE_DATA_ERRORS_X ||
        m_eObjectType == OBJECTTYPE_DATA_ERRORS_Y ||
        m_eObjectType == OBJECTTYPE_DATA_ERRORS_Z)
        m_bHasStatisticProperties = true;

    if( OBJECTTYPE_AXIS == m_eObjectType )
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
                if( chart2::AxisType::SERIES == aData.AxisType )
                    m_bHasScaleProperties = false;
                if( chart2::AxisType::SERIES != aData.AxisType )
                    m_bHasNumberProperties = true;

                sal_Int32 nCooSysIndex=0;
                sal_Int32 nDimensionIndex=0;
                sal_Int32 nAxisIndex=0;
                if( AxisHelper::getIndicesForAxis( xAxis, xDiagram, nCooSysIndex, nDimensionIndex, nAxisIndex ) )
                {
                    xChartType = AxisHelper::getFirstChartTypeWithSeriesAttachedToAxisIndex( xDiagram, nAxisIndex );
                    //show positioning controls only if they make sense
                    m_bSupportingAxisPositioning = ChartTypeHelper::isSupportingAxisPositioning( xChartType, nDimensionCount, nDimensionIndex );

                    //show axis origin only for secondary y axis
                    if( 1==nDimensionIndex && 1==nAxisIndex && ChartTypeHelper::isSupportingBaseValue( xChartType ) )
                        m_bShowAxisOrigin = true;
                }

                //is the crossin main axis a category axes?:
                uno::Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis( xAxis, xDiagram ) );
                uno::Reference< XAxis > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( xAxis, xCooSys ) );
                if( xCrossingMainAxis.is() )
                {
                    ScaleData aScale( xCrossingMainAxis->getScaleData() );
                    m_bIsCrossingAxisIsCategoryAxis = ( chart2::AxisType::CATEGORY == aScale.AxisType  );
                    if( m_bIsCrossingAxisIsCategoryAxis )
                        m_aCategories = DiagramHelper::getExplicitSimpleCategories( Reference< chart2::XChartDocument >( xChartModel, uno::UNO_QUERY) );
                }

                m_bComplexCategoriesAxis = false;
                if ( nDimensionIndex == 0 && aData.AxisType == chart2::AxisType::CATEGORY )
                {
                    ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSys, xChartModel );
                    m_bComplexCategoriesAxis = aExplicitCategoriesProvider.hasComplexCategories();
                }
            }
        }

        //no staggering of labels for 3D axis
        m_bCanAxisLabelsBeStaggered = nDimensionCount==2;
    }

    //create gui name for this object
    {
        if( !m_bAffectsMultipleObjects && OBJECTTYPE_AXIS == m_eObjectType )
        {
            m_aLocalizedName = ObjectNameProvider::getAxisName( m_aObjectCID, xChartModel );
        }
        else if( !m_bAffectsMultipleObjects && ( OBJECTTYPE_GRID == m_eObjectType || OBJECTTYPE_SUBGRID == m_eObjectType ) )
        {
            m_aLocalizedName = ObjectNameProvider::getGridName( m_aObjectCID, xChartModel );
        }
        else if( !m_bAffectsMultipleObjects && OBJECTTYPE_TITLE == m_eObjectType )
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

bool ObjectPropertiesDialogParameter::HasGeometryProperties() const
{
    return m_bHasGeometryProperties;
}
bool ObjectPropertiesDialogParameter::HasStatisticProperties() const
{
    return m_bHasStatisticProperties;
}
bool ObjectPropertiesDialogParameter::ProvidesSecondaryYAxis() const
{
    return m_bProvidesSecondaryYAxis;
}
bool ObjectPropertiesDialogParameter::ProvidesOverlapAndGapWidth() const
{
    return m_bProvidesOverlapAndGapWidth;
}
bool ObjectPropertiesDialogParameter::ProvidesBarConnectors() const
{
    return m_bProvidesBarConnectors;
}
bool ObjectPropertiesDialogParameter::HasAreaProperties() const
{
    return m_bHasAreaProperties;
}
bool ObjectPropertiesDialogParameter::HasSymbolProperties() const
{
    return m_bHasSymbolProperties;
}
bool ObjectPropertiesDialogParameter::HasScaleProperties() const
{
    return m_bHasScaleProperties;
}
bool ObjectPropertiesDialogParameter::CanAxisLabelsBeStaggered() const
{
    return m_bCanAxisLabelsBeStaggered;
}
bool ObjectPropertiesDialogParameter::ShowAxisOrigin() const
{
    return m_bShowAxisOrigin;
}
bool ObjectPropertiesDialogParameter::IsSupportingAxisPositioning() const
{
    return m_bSupportingAxisPositioning;
}
bool ObjectPropertiesDialogParameter::IsCrossingAxisIsCategoryAxis() const
{
    return m_bIsCrossingAxisIsCategoryAxis;
}
const uno::Sequence< OUString >& ObjectPropertiesDialogParameter::GetCategories() const
{
    return m_aCategories;
}
bool ObjectPropertiesDialogParameter::HasNumberProperties() const
{
    return m_bHasNumberProperties;
}
bool ObjectPropertiesDialogParameter::ProvidesStartingAngle() const
{
    return m_bProvidesStartingAngle;
}
bool ObjectPropertiesDialogParameter::ProvidesMissingValueTreatments() const
{
    return m_bProvidesMissingValueTreatments;
}
uno::Reference< chart2::XChartDocument > ObjectPropertiesDialogParameter::getDocument() const
{
    return m_xChartDocument;
}
bool ObjectPropertiesDialogParameter::IsComplexCategoriesAxis() const
{
    return m_bComplexCategoriesAxis;
}

const sal_uInt16 nNoArrowNoShadowDlg    = 1101;

void SchAttribTabDlg::setSymbolInformation( SfxItemSet* pSymbolShapeProperties,
                Graphic* pAutoSymbolGraphic )
{
    m_pSymbolShapeProperties = pSymbolShapeProperties;
    m_pAutoSymbolGraphic = pAutoSymbolGraphic;
}

void SchAttribTabDlg::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    m_fAxisMinorStepWidthForErrorBarDecimals = fMinorStepWidth;
}

SchAttribTabDlg::SchAttribTabDlg(Window* pParent,
                                 const SfxItemSet* pAttr,
                                 const ObjectPropertiesDialogParameter* pDialogParameter,
                                 const ViewElementListProvider* pViewElementListProvider,
                                 const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
                                 )
    : SfxTabDialog(pParent, SchResId(DLG_OBJECT_PROPERTIES), pAttr)
    , eObjectType(pDialogParameter->getObjectType())
    , nDlgType(nNoArrowNoShadowDlg)
    , nPageType(0)
    , m_pParameter( pDialogParameter )
    , m_pViewElementListProvider( pViewElementListProvider )
    , m_pNumberFormatter(0)
    , m_pSymbolShapeProperties(NULL)
    , m_pAutoSymbolGraphic(NULL)
    , m_fAxisMinorStepWidthForErrorBarDecimals(0.1)
    , m_bOKPressed(false)
{
    FreeResource();

    NumberFormatterWrapper aNumberFormatterWrapper( xNumberFormatsSupplier );
    m_pNumberFormatter = aNumberFormatterWrapper.getSvNumberFormatter();

    this->SetText( pDialogParameter->getLocalizedName() );

    SvtCJKOptions aCJKOptions;

    switch (eObjectType)
    {
        case OBJECTTYPE_TITLE:
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_BORDER));
            AddTabPage(RID_SVXPAGE_AREA, SCH_RESSTR(STR_PAGE_AREA));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SCH_RESSTR(STR_PAGE_TRANSPARENCY));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SCH_RESSTR(STR_PAGE_FONT));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SCH_RESSTR(STR_PAGE_FONT_EFFECTS));
            AddTabPage(TP_ALIGNMENT, SCH_RESSTR(STR_PAGE_ALIGNMENT), SchAlignmentTabPage::Create, NULL);
            if( aCJKOptions.IsAsianTypographyEnabled() )
                AddTabPage(RID_SVXPAGE_PARA_ASIAN, SCH_RESSTR(STR_PAGE_ASIAN));
            break;

        case OBJECTTYPE_LEGEND:
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_BORDER));
            AddTabPage(RID_SVXPAGE_AREA, SCH_RESSTR(STR_PAGE_AREA));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SCH_RESSTR(STR_PAGE_TRANSPARENCY));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SCH_RESSTR(STR_PAGE_FONT));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SCH_RESSTR(STR_PAGE_FONT_EFFECTS));
            AddTabPage(TP_LEGEND_POS, SCH_RESSTR(STR_PAGE_POSITION), SchLegendPosTabPage::Create, NULL);
            if( aCJKOptions.IsAsianTypographyEnabled() )
                AddTabPage(RID_SVXPAGE_PARA_ASIAN, SCH_RESSTR(STR_PAGE_ASIAN));
            break;

        case OBJECTTYPE_DATA_SERIES:
        case OBJECTTYPE_DATA_POINT:
            if( m_pParameter->ProvidesSecondaryYAxis() || m_pParameter->ProvidesOverlapAndGapWidth() || m_pParameter->ProvidesMissingValueTreatments() )
                AddTabPage(TP_OPTIONS, SCH_RESSTR(STR_PAGE_OPTIONS),SchOptionTabPage::Create, NULL);
            if( m_pParameter->ProvidesStartingAngle())
                AddTabPage(TP_POLAROPTIONS, SCH_RESSTR(STR_PAGE_OPTIONS),PolarOptionsTabPage::Create, NULL);

            if( m_pParameter->HasGeometryProperties() )
                AddTabPage(TP_LAYOUT, SCH_RESSTR(STR_PAGE_LAYOUT),SchLayoutTabPage::Create, NULL);

            if(m_pParameter->HasAreaProperties())
            {
                AddTabPage(RID_SVXPAGE_AREA, SCH_RESSTR(STR_PAGE_AREA));
                AddTabPage(RID_SVXPAGE_TRANSPARENCE, SCH_RESSTR(STR_PAGE_TRANSPARENCY));
            }
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR( m_pParameter->HasAreaProperties() ? STR_PAGE_BORDER : STR_PAGE_LINE ));
            break;

        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_LABELS:
            AddTabPage(TP_DATA_DESCR, SCH_RESSTR(STR_OBJECT_DATALABELS), DataLabelsTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SCH_RESSTR(STR_PAGE_FONT));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SCH_RESSTR(STR_PAGE_FONT_EFFECTS));
            if( aCJKOptions.IsAsianTypographyEnabled() )
                AddTabPage(RID_SVXPAGE_PARA_ASIAN, SCH_RESSTR(STR_PAGE_ASIAN));

            break;

        case OBJECTTYPE_AXIS:
        {
            if( m_pParameter->HasScaleProperties() )
                AddTabPage(TP_SCALE, SCH_RESSTR(STR_PAGE_SCALE), ScaleTabPage::Create, NULL);

            if( m_pParameter->HasScaleProperties() )//no positioning page for z axes so far as the tickmarks are not shown so far
                AddTabPage(TP_AXIS_POSITIONS, SCH_RESSTR(STR_PAGE_POSITIONING), AxisPositionsTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_LINE));
            AddTabPage(TP_AXIS_LABEL, SCH_RESSTR(STR_OBJECT_LABEL), SchAxisLabelTabPage::Create, NULL);
            if( m_pParameter->HasNumberProperties() )
                AddTabPage(RID_SVXPAGE_NUMBERFORMAT, SCH_RESSTR(STR_PAGE_NUMBERS));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SCH_RESSTR(STR_PAGE_FONT));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SCH_RESSTR(STR_PAGE_FONT_EFFECTS));
            if( aCJKOptions.IsAsianTypographyEnabled() )
                AddTabPage(RID_SVXPAGE_PARA_ASIAN, SCH_RESSTR(STR_PAGE_ASIAN));
            break;
        }

        case OBJECTTYPE_DATA_ERRORS_X:
            AddTabPage(TP_XERRORBAR, SCH_RESSTR(STR_PAGE_XERROR_BARS), ErrorBarsTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_LINE));
            break;

        case OBJECTTYPE_DATA_ERRORS_Y:
            AddTabPage(TP_YERRORBAR, SCH_RESSTR(STR_PAGE_YERROR_BARS), ErrorBarsTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_LINE));
            break;

        case OBJECTTYPE_DATA_ERRORS_Z:
            break;

        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_STOCK_RANGE:
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_LINE));
            break;

        case OBJECTTYPE_DATA_CURVE:
            AddTabPage(TP_TRENDLINE, SCH_RESSTR(STR_PAGE_TRENDLINE_TYPE), TrendlineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_LINE));
            break;

        case OBJECTTYPE_DATA_STOCK_LOSS:
        case OBJECTTYPE_DATA_STOCK_GAIN:
        case OBJECTTYPE_PAGE:
        case OBJECTTYPE_DIAGRAM_FLOOR:
        case OBJECTTYPE_DIAGRAM_WALL:
        case OBJECTTYPE_DIAGRAM:
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_BORDER));
            AddTabPage(RID_SVXPAGE_AREA, SCH_RESSTR(STR_PAGE_AREA));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SCH_RESSTR(STR_PAGE_TRANSPARENCY));
            break;

        case OBJECTTYPE_LEGEND_ENTRY:
        case OBJECTTYPE_AXIS_UNITLABEL:
        case OBJECTTYPE_UNKNOWN:
            // nothing
            break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            AddTabPage(RID_SVXPAGE_LINE, SCH_RESSTR(STR_PAGE_BORDER));
            AddTabPage(RID_SVXPAGE_AREA, SCH_RESSTR(STR_PAGE_AREA));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SCH_RESSTR(STR_PAGE_TRANSPARENCY));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SCH_RESSTR(STR_PAGE_FONT));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SCH_RESSTR(STR_PAGE_FONT_EFFECTS));
            AddTabPage(RID_SVXPAGE_NUMBERFORMAT, SCH_RESSTR(STR_PAGE_NUMBERS));
            if( SvtLanguageOptions().IsCTLFontEnabled() )
                /*  When rotation is supported for equation text boxes, use
                    SchAlignmentTabPage::Create here. The special
                    SchAlignmentTabPage::CreateWithoutRotation can be deleted. */
                AddTabPage(TP_ALIGNMENT, SCH_RESSTR(STR_PAGE_ALIGNMENT), SchAlignmentTabPage::CreateWithoutRotation, NULL);
            break;
        default:
            break;
    }

    // used to find out if user left the dialog with OK. When OK is pressed but
    // no changes were done, Cancel is returned by the SfxTabDialog. See method
    // DialogWasClosedWithOK.
    m_aOriginalOKClickHdl = GetOKButton().GetClickHdl();
    GetOKButton().SetClickHdl( LINK( this, SchAttribTabDlg, OKPressed ));
}

SchAttribTabDlg::~SchAttribTabDlg()
{
    delete m_pSymbolShapeProperties;
    delete m_pAutoSymbolGraphic;
}

void SchAttribTabDlg::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch (nId)
    {
        case TP_LAYOUT:
        break;
        case RID_SVXPAGE_LINE:
            aSet.Put (SvxColorListItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(m_pViewElementListProvider->GetDashList(),SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(m_pViewElementListProvider->GetLineEndList(),SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));

            if( m_pParameter->HasSymbolProperties() )
            {
                aSet.Put(OfaPtrItem(SID_OBJECT_LIST,m_pViewElementListProvider->GetSymbolList()));
                if( m_pSymbolShapeProperties )
                    aSet.Put(SfxTabDialogItem(SID_ATTR_SET,*m_pSymbolShapeProperties));
                if( m_pAutoSymbolGraphic )
                    aSet.Put(SvxGraphicItem(SID_GRAPHIC,*m_pAutoSymbolGraphic));
            }
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_AREA:
            aSet.Put(SvxColorListItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
            aSet.Put(SvxGradientListItem(m_pViewElementListProvider->GetGradientList(),SID_GRADIENT_LIST));
            aSet.Put(SvxHatchListItem(m_pViewElementListProvider->GetHatchList(),SID_HATCH_LIST));
            aSet.Put(SvxBitmapListItem(m_pViewElementListProvider->GetBitmapList(),SID_BITMAP_LIST));
            aSet.Put(SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put(SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_TRANSPARENCE:
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_CHAR_NAME:

            aSet.Put (SvxFontListItem(m_pViewElementListProvider->getFontList(), SID_ATTR_CHAR_FONTLIST));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
            rPage.PageCreated(aSet);
            break;

        case TP_AXIS_LABEL:
        {
            bool bShowStaggeringControls = m_pParameter->CanAxisLabelsBeStaggered();
            ((SchAxisLabelTabPage&)rPage).ShowStaggeringControls( bShowStaggeringControls );
            ( dynamic_cast< SchAxisLabelTabPage& >( rPage ) ).SetComplexCategories( m_pParameter->IsComplexCategoriesAxis() );
            break;
        }

        case TP_ALIGNMENT:
            break;

        case TP_AXIS_POSITIONS:
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
                }
            }
            break;

        case TP_SCALE:
            {
                ScaleTabPage* pScaleTabPage = dynamic_cast< ScaleTabPage* >( &rPage );
                if(pScaleTabPage)
                {
                    pScaleTabPage->SetNumFormatter( m_pNumberFormatter );
                    pScaleTabPage->ShowAxisOrigin( m_pParameter->ShowAxisOrigin() );
                }
            }
            break;

        case TP_DATA_DESCR:
            {
                DataLabelsTabPage* pLabelPage = dynamic_cast< DataLabelsTabPage* >( &rPage );
                if( pLabelPage )
                    pLabelPage->SetNumberFormatter( m_pNumberFormatter );
            }
            break;

        case RID_SVXPAGE_NUMBERFORMAT:
               aSet.Put (SvxNumberInfoItem( m_pNumberFormatter, (const sal_uInt16)SID_ATTR_NUMBERFORMAT_INFO));
            rPage.PageCreated(aSet);
            break;
        case TP_XERRORBAR:
        {
            ErrorBarsTabPage * pTabPage = dynamic_cast< ErrorBarsTabPage * >( &rPage );
            OSL_ASSERT( pTabPage );
            if( pTabPage )
            {
                pTabPage->SetAxisMinorStepWidthForErrorBarDecimals( m_fAxisMinorStepWidthForErrorBarDecimals );
                pTabPage->SetErrorBarType( ErrorBarResources::ERROR_BAR_X );
                pTabPage->SetChartDocumentForRangeChoosing( m_pParameter->getDocument());
            }
            break;
        }
        case TP_YERRORBAR:
        {
            ErrorBarsTabPage * pTabPage = dynamic_cast< ErrorBarsTabPage * >( &rPage );
            OSL_ASSERT( pTabPage );
            if( pTabPage )
            {
                pTabPage->SetAxisMinorStepWidthForErrorBarDecimals( m_fAxisMinorStepWidthForErrorBarDecimals );
                pTabPage->SetErrorBarType( ErrorBarResources::ERROR_BAR_Y );
                pTabPage->SetChartDocumentForRangeChoosing( m_pParameter->getDocument());
            }
            break;
        }
        case TP_OPTIONS:
        {
            SchOptionTabPage* pTabPage = dynamic_cast< SchOptionTabPage* >( &rPage );
            if( pTabPage && m_pParameter )
                pTabPage->Init( m_pParameter->ProvidesSecondaryYAxis(), m_pParameter->ProvidesOverlapAndGapWidth(),
                    m_pParameter->ProvidesBarConnectors() );
            break;
        }
    }
}

IMPL_LINK_NOARG(SchAttribTabDlg, OKPressed)
{
    m_bOKPressed = true;
    return m_aOriginalOKClickHdl.Call( this );
}

bool SchAttribTabDlg::DialogWasClosedWithOK() const
{
    return m_bOKPressed;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
