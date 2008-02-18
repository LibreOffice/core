/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_ObjectProperties.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:44:58 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#ifndef _ZFORLIST_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <svtools/zforlist.hxx>
#endif

#include "dlg_ObjectProperties.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "TabPages.hrc"
#include "tp_AxisLabel.hxx"
#include "tp_DataLabel.hxx"
#include "tp_LegendPosition.hxx"
#include "tp_PointGeometry.hxx"
#include "tp_Scale.hxx"
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
#include "chartview/NumberFormatterWrapper.hxx"
#include "AxisIndexDefines.hxx"

#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif


#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif

#ifndef _OFF_OFAITEM_HXX
#include <svx/ofaitem.hxx>
#endif
#ifndef _SVX_GRAPHICITEM_HXX
#include <svx/svxgrahicitem.hxx>
#endif

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_TAB_LINE_HXX
#include <svx/tabline.hxx>
#endif

#include <svx/flagsdef.hxx>

#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ObjectPropertiesDialogParameter::ObjectPropertiesDialogParameter( const rtl::OUString& rObjectCID )
        : m_aObjectCID( rObjectCID )
        , m_eObjectType( ObjectIdentifier::getObjectType( m_aObjectCID ) )
        , m_bAffectsMultipleObjects(false)
        , m_aLocalizedName()
        , m_bHasGeometryProperties(false)
        , m_bHasStatisticProperties(false)
        , m_bHasRegressionProperties(false)
        , m_bProvidesSecondaryYAxis(false)
        , m_bProvidesOverlapAndGapWidth(false)
        , m_bHasAreaProperties(false)
        , m_bHasLineProperties(false)
        , m_bHasSymbolProperties(false)
        , m_bHasScaleProperties(false)
        , m_bCanAxisLabelsBeStaggered(false)
        , m_bHasNumberProperties(false)
        , m_bProvidesStartingAngle(false)
{
    rtl::OUString aParticleID = ObjectIdentifier::getParticleID( m_aObjectCID );
    m_bAffectsMultipleObjects = aParticleID.equals(C2U("ALLELEMENTS"));
}
ObjectPropertiesDialogParameter::~ObjectPropertiesDialogParameter()
{
}
ObjectType ObjectPropertiesDialogParameter::getObjectType() const
{
    return m_eObjectType;
}
rtl::OUString ObjectPropertiesDialogParameter::getLocalizedName() const
{
    return m_aLocalizedName;
}

void ObjectPropertiesDialogParameter::init( const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( m_aObjectCID, xChartModel );
    uno::Reference< XChartType > xChartType = ChartModelHelper::getChartTypeOfSeries( xChartModel, xSeries );
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );

    bool bHasSeriesProperties = (OBJECTTYPE_DATA_SERIES==m_eObjectType || OBJECTTYPE_DATA_LABELS==m_eObjectType);
    bool bHasDataPointproperties = (OBJECTTYPE_DATA_POINT==m_eObjectType);

    if( bHasSeriesProperties || bHasDataPointproperties )
    {
        m_bHasGeometryProperties = ChartTypeHelper::isSupportingGeometryProperties( xChartType, nDimensionCount );
        m_bHasAreaProperties     = ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount );
        m_bHasSymbolProperties   = ChartTypeHelper::isSupportingSymbolProperties( xChartType, nDimensionCount );

        if( bHasSeriesProperties )
        {
            m_bHasStatisticProperties =  ChartTypeHelper::isSupportingStatisticProperties( xChartType, nDimensionCount );
            m_bHasRegressionProperties = ChartTypeHelper::isSupportingRegressionProperties( xChartType, nDimensionCount );
            m_bProvidesSecondaryYAxis =  ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimensionCount, 1 );
            m_bProvidesOverlapAndGapWidth =  ChartTypeHelper::isSupportingOverlapAndGapWidthProperties( xChartType, nDimensionCount );
            m_bProvidesStartingAngle = ChartTypeHelper::isSupportingStartingAngle( xChartType );
        }
    }
    m_bHasLineProperties     = true; //@todo ask object

    if( OBJECTTYPE_AXIS == m_eObjectType )
    {
        //show scale properties only for a single axis not for multiselection
        m_bHasScaleProperties = !m_bAffectsMultipleObjects;

        //no scale page for series axis
        if( m_bHasScaleProperties )
        {
            uno::Reference< XAxis > xAxis( ObjectIdentifier::getAxisForCID( m_aObjectCID, xChartModel ) );
            if( xAxis.is() )
            {
                ScaleData aData( xAxis->getScaleData() );
                if( chart2::AxisType::SERIES == aData.AxisType )
                    m_bHasScaleProperties = false;
                if( chart2::AxisType::REALNUMBER == aData.AxisType || chart2::AxisType::PERCENT == aData.AxisType )
                    m_bHasNumberProperties = true;
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
        else
        {
            ObjectType eType = m_eObjectType;
            if( OBJECTTYPE_DATA_LABEL == eType )
                 eType = OBJECTTYPE_DATA_POINT;
            else if( OBJECTTYPE_DATA_LABELS == eType )
                eType = OBJECTTYPE_DATA_SERIES;
            m_aLocalizedName = ObjectNameProvider::getName(eType,m_bAffectsMultipleObjects);
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
bool ObjectPropertiesDialogParameter::HasRegressionProperties() const
{
    return m_bHasRegressionProperties;
}
bool ObjectPropertiesDialogParameter::ProvidesSecondaryYAxis() const
{
    return m_bProvidesSecondaryYAxis;
}
bool ObjectPropertiesDialogParameter::ProvidesOverlapAndGapWidth() const
{
    return m_bProvidesOverlapAndGapWidth;
}
bool ObjectPropertiesDialogParameter::HasAreaProperties() const
{
    return m_bHasAreaProperties;
}
bool ObjectPropertiesDialogParameter::HasLineProperties() const
{
    return m_bHasLineProperties;
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
bool ObjectPropertiesDialogParameter::HasNumberProperties() const
{
    return m_bHasNumberProperties;
}
bool ObjectPropertiesDialogParameter::ProvidesStartingAngle() const
{
    return m_bProvidesStartingAngle;
}

//const USHORT nNoArrowDlg          = 1100;
const USHORT nNoArrowNoShadowDlg    = 1101;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------

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

    switch (eObjectType)
    {
        case OBJECTTYPE_TITLE:
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId(STR_PAGE_BORDER)));
            AddTabPage(RID_SVXPAGE_AREA, String(SchResId(STR_PAGE_AREA)));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, String(SchResId(STR_PAGE_TRANSPARENCY)));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, String(SchResId(STR_PAGE_CHARACTERS)));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, String(SchResId(STR_PAGE_FONT_EFFECTS)));
            AddTabPage(TP_ALIGNMENT, String(SchResId(STR_PAGE_ALIGNMENT)), SchAlignmentTabPage::Create, NULL);
            break;

        case OBJECTTYPE_LEGEND:
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId(STR_PAGE_BORDER)));
            AddTabPage(RID_SVXPAGE_AREA, String(SchResId(STR_PAGE_AREA)));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, String(SchResId(STR_PAGE_TRANSPARENCY)));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, String(SchResId(STR_PAGE_CHARACTERS)));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, String(SchResId(STR_PAGE_FONT_EFFECTS)));
            AddTabPage(TP_LEGEND_POS, String(SchResId(STR_PAGE_POSITION)), SchLegendPosTabPage::Create, NULL);
            break;

        case OBJECTTYPE_DATA_SERIES:
        case OBJECTTYPE_DATA_POINT:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_LABELS:
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId( m_pParameter->HasAreaProperties() ? STR_PAGE_BORDER : STR_PAGE_LINE )));
            if(m_pParameter->HasAreaProperties())
            {
                AddTabPage(RID_SVXPAGE_AREA, String(SchResId(STR_PAGE_AREA)));
                AddTabPage(RID_SVXPAGE_TRANSPARENCE, String(SchResId(STR_PAGE_TRANSPARENCY)));
            }
            AddTabPage(RID_SVXPAGE_CHAR_NAME, String(SchResId(STR_PAGE_CHARACTERS)));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, String(SchResId(STR_PAGE_FONT_EFFECTS)));
            AddTabPage(TP_DATA_DESCR, String(SchResId(STR_OBJECT_DATALABELS)), DataLabelsTabPage::Create, NULL);
            if( m_pParameter->HasStatisticProperties() )
                AddTabPage(TP_YERRORBAR, String(SchResId(STR_PAGE_YERROR_BARS)), ErrorBarsTabPage::Create, NULL);
            if( m_pParameter->HasGeometryProperties() )
                AddTabPage(TP_LAYOUT, String(SchResId(STR_PAGE_LAYOUT)),SchLayoutTabPage::Create, NULL);
            if( m_pParameter->ProvidesSecondaryYAxis() || m_pParameter->ProvidesOverlapAndGapWidth() )
                AddTabPage(TP_OPTIONS, String(SchResId(STR_PAGE_OPTIONS)),SchOptionTabPage::Create, NULL);
            if( m_pParameter->ProvidesStartingAngle())
                AddTabPage(TP_POLAROPTIONS, String(SchResId(STR_PAGE_OPTIONS)),PolarOptionsTabPage::Create, NULL);
            break;

        case OBJECTTYPE_AXIS:
        {
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId(STR_PAGE_LINE)));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, String(SchResId(STR_PAGE_CHARACTERS)));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, String(SchResId(STR_PAGE_FONT_EFFECTS)));
            AddTabPage(TP_AXIS_LABEL, String(SchResId(STR_OBJECT_LABEL)), SchAxisLabelTabPage::Create, NULL);
            if( m_pParameter->HasScaleProperties() )
                AddTabPage(TP_SCALE_Y, String(SchResId(STR_PAGE_SCALE)), ScaleTabPage::Create, NULL);
            if( m_pParameter->HasNumberProperties() )
                AddTabPage(RID_SVXPAGE_NUMBERFORMAT, String(SchResId(STR_PAGE_NUMBERS)));
            break;
        }

        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
        case OBJECTTYPE_DATA_ERRORS:
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_STOCK_RANGE:
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId(STR_PAGE_LINE)));
            break;

        case OBJECTTYPE_DATA_CURVE:
            OSL_ASSERT( m_pParameter->HasRegressionProperties());
            AddTabPage(TP_TRENDLINE, String(SchResId(STR_PAGE_TRENDLINE_TYPE)), TrendlineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId(STR_PAGE_LINE)));
            break;

        case OBJECTTYPE_DATA_STOCK_LOSS:
        case OBJECTTYPE_DATA_STOCK_GAIN:
        case OBJECTTYPE_PAGE:
        case OBJECTTYPE_DIAGRAM_FLOOR:
        case OBJECTTYPE_DIAGRAM_WALL:
        case OBJECTTYPE_DIAGRAM:
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId(STR_PAGE_BORDER)));
            AddTabPage(RID_SVXPAGE_AREA, String(SchResId(STR_PAGE_AREA)));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, String(SchResId(STR_PAGE_TRANSPARENCY)));
            break;

        case OBJECTTYPE_LEGEND_ENTRY:
        case OBJECTTYPE_AXIS_UNITLABEL:
        case OBJECTTYPE_UNKNOWN:
            // nothing
            break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            AddTabPage(RID_SVXPAGE_LINE, String(SchResId(STR_PAGE_BORDER)));
            AddTabPage(RID_SVXPAGE_AREA, String(SchResId(STR_PAGE_AREA)));
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, String(SchResId(STR_PAGE_TRANSPARENCY)));
            AddTabPage(RID_SVXPAGE_CHAR_NAME, String(SchResId(STR_PAGE_CHARACTERS)));
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, String(SchResId(STR_PAGE_FONT_EFFECTS)));
            AddTabPage(RID_SVXPAGE_NUMBERFORMAT, String(SchResId(STR_PAGE_NUMBERS)));
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

void SchAttribTabDlg::PageCreated(USHORT nId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch (nId)
    {
        case TP_LAYOUT:
        break;
        case RID_SVXPAGE_LINE:
            aSet.Put (SvxColorTableItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
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
            //rPage.ActivatePage(*GetInputSetImpl()); //what for?
            break;

        case RID_SVXPAGE_AREA:
            aSet.Put(SvxColorTableItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
            aSet.Put(SvxGradientListItem(m_pViewElementListProvider->GetGradientList(),SID_GRADIENT_LIST));
            aSet.Put(SvxHatchListItem(m_pViewElementListProvider->GetHatchList(),SID_HATCH_LIST));
            aSet.Put(SvxBitmapListItem(m_pViewElementListProvider->GetBitmapList(),SID_BITMAP_LIST));
            aSet.Put(SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put(SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            //rPage.ActivatePage(*GetInputSetImpl()); //what for?
            break;

        case RID_SVXPAGE_TRANSPARENCE:
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_CHAR_NAME:

            //CHINA001 ((SvxCharNamePage&)rPage).
            //CHINA001     SetFontList(SvxFontListItem(m_pViewElementListProvider->getFontList()));
            aSet.Put (SvxFontListItem(m_pViewElementListProvider->getFontList(), SID_ATTR_CHAR_FONTLIST)); //CHINA001
            rPage.PageCreated(aSet); //CHINA001
            break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            //CHINA001 ((SvxCharEffectsPage&) rPage).DisableControls( DISABLE_CASEMAP );
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP)); //CHINA001
            rPage.PageCreated(aSet);
            break;

        case TP_AXIS_LABEL:
        {
            bool bShowStaggeringControls = m_pParameter->CanAxisLabelsBeStaggered();
            ((SchAxisLabelTabPage&)rPage).ShowStaggeringControls( bShowStaggeringControls );
            break;
        }

        case TP_ALIGNMENT:
            break;

        case TP_SCALE_Y:
            {
                ScaleTabPage & rAxisTabPage = static_cast< ScaleTabPage & >( rPage );

                // #i81259# fix for #101318# undone.  The numberformatter passed
                // here must contain the actually used number format.  Showing
                // numbers with more digits is now solved in FormattedField
                // (using the input format also for output).
                rAxisTabPage.SetNumFormatter( m_pNumberFormatter );
            }
            break;
//DLNF         case TP_DATA_DESCR:
//DLNF             {
//DLNF                 DataLabelsTabPage & rLabelPage = static_cast< DataLabelsTabPage & >( rPage );
//DLNF                 rLabelPage.SetNumberFormatter( m_pNumberFormatter );
//DLNF             }
//DLNF             break;

        case TP_DATA_DESCR:
            {
                DataLabelsTabPage & rLabelPage = static_cast< DataLabelsTabPage & >( rPage );
                rLabelPage.SetNumberFormatter( m_pNumberFormatter );
            }
            break;

        case RID_SVXPAGE_NUMBERFORMAT:
               aSet.Put (SvxNumberInfoItem( m_pNumberFormatter, (const USHORT)SID_ATTR_NUMBERFORMAT_INFO));
            rPage.PageCreated(aSet);
            break;

        case TP_YERRORBAR:
        {
            ErrorBarsTabPage * pTabPage = dynamic_cast< ErrorBarsTabPage * >( &rPage );
            OSL_ASSERT( pTabPage );
            if( pTabPage )
            {
                pTabPage->SetAxisMinorStepWidthForErrorBarDecimals( m_fAxisMinorStepWidthForErrorBarDecimals );
                pTabPage->SetErrorBarType( ErrorBarResources::ERROR_BAR_Y );
            }
            break;
        }
    }
}

IMPL_LINK( SchAttribTabDlg, OKPressed, void * , EMPTYARG )
{
    m_bOKPressed = true;
    return m_aOriginalOKClickHdl.Call( this );
}

bool SchAttribTabDlg::DialogWasClosedWithOK() const
{
    return m_bOKPressed;
}

//.............................................................................
} //namespace chart
//.............................................................................
