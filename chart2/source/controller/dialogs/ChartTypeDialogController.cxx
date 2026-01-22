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

#include <ChartTypeDialogController.hxx>
#include <ChartTypeManager.hxx>
#include <ChartTypeTemplate.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <ChartModelHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <Diagram.hxx>
#include <ControllerLockGuard.hxx>
#include <AxisHelper.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>
#include <ChartModel.hxx>

#include <com/sun/star/chart2/DataPointGeometry3D.hpp>

#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

ChartTypeParameter::ChartTypeParameter()
                    : nSubTypeIndex( 1 )
                    , bXAxisWithValues( false )
                    , b3DLook( false )
                    , bSymbols( true )
                    , bLines( true )
                    , eStackMode( GlobalStackMode_NONE )
                    , eCurveStyle( CurveStyle_LINES )
                    , nCurveResolution(20)
                    , nSplineOrder(3)
                    , nGeometry3D(DataPointGeometry3D::CUBOID)
                    , eThreeDLookScheme(ThreeDLookScheme::ThreeDLookScheme_Realistic)
                    , bSortByXValues(false)
                    , mbRoundedEdge(false)
{
}

ChartTypeParameter::ChartTypeParameter( sal_Int32 SubTypeIndex, bool HasXAxisWithValues
                    ,  bool Is3DLook,  GlobalStackMode nStackMode
                    , bool HasSymbols, bool HasLines
                    , CurveStyle nCurveStyle )
                    : nSubTypeIndex( SubTypeIndex )
                    , bXAxisWithValues( HasXAxisWithValues )
                    , b3DLook( Is3DLook )
                    , bSymbols( HasSymbols )
                    , bLines( HasLines )
                    , eStackMode( nStackMode )
                    , eCurveStyle( nCurveStyle )
                    , nCurveResolution(20)
                    , nSplineOrder(3)
                    , nGeometry3D(DataPointGeometry3D::CUBOID)
                    , eThreeDLookScheme(ThreeDLookScheme::ThreeDLookScheme_Realistic)
                    , bSortByXValues(false)
                    , mbRoundedEdge(false)
{
}

bool ChartTypeParameter::mapsToSameService( const ChartTypeParameter& rParameter ) const
{
    return mapsToSimilarService( rParameter, 0 );
}
bool ChartTypeParameter::mapsToSimilarService( const ChartTypeParameter& rParameter, sal_Int32 nTheHigherTheLess ) const
{
    sal_Int32 nMax=7;
    if(nTheHigherTheLess>nMax)
        return true;
    if( bXAxisWithValues!=rParameter.bXAxisWithValues )
        return nTheHigherTheLess>nMax-1;
    if( b3DLook!=rParameter.b3DLook )
        return nTheHigherTheLess>nMax-2;
    if( eStackMode!=rParameter.eStackMode )
        return nTheHigherTheLess>nMax-3;
    if( nSubTypeIndex!=rParameter.nSubTypeIndex )
        return nTheHigherTheLess>nMax-4;
    if( bSymbols!=rParameter.bSymbols )
        return nTheHigherTheLess>nMax-5;
    if( bLines!=rParameter.bLines )
        return nTheHigherTheLess>nMax-6;
    return true;
}

ChartTypeDialogController::ChartTypeDialogController()
    : bSupportsXAxisWithValues(false)
    , bSupports3D(true)
{
}

ChartTypeDialogController::~ChartTypeDialogController()
{
}

bool ChartTypeDialogController::isSubType( const OUString& rServiceName )
{
    const tTemplateServiceChartTypeParameterMap& rTemplateMap = getTemplateMap();
    tTemplateServiceChartTypeParameterMap::const_iterator aIt( rTemplateMap.find( rServiceName ));
    return aIt != rTemplateMap.end();
}
ChartTypeParameter ChartTypeDialogController::getChartTypeParameterForService(
                                                    const OUString& rServiceName
                                                    , const uno::Reference< beans::XPropertySet >& xTemplateProps )
{
    ChartTypeParameter aRet;
    const tTemplateServiceChartTypeParameterMap& rTemplateMap = getTemplateMap();
    tTemplateServiceChartTypeParameterMap::const_iterator aIt( rTemplateMap.find( rServiceName ));
    if( aIt != rTemplateMap.end())
        aRet = (*aIt).second;
    if( xTemplateProps.is() )
    {
        try
        {
            xTemplateProps->getPropertyValue( CHART_UNONAME_CURVE_STYLE ) >>= aRet.eCurveStyle;
            xTemplateProps->getPropertyValue( CHART_UNONAME_CURVE_RESOLUTION ) >>= aRet.nCurveResolution;
            xTemplateProps->getPropertyValue( CHART_UNONAME_SPLINE_ORDER ) >>= aRet.nSplineOrder;
        }
        catch( uno::Exception & ex )
        {
            //not all templates need to support CurveStyle, CurveResolution or SplineOrder
            ex.Context.is();//to have debug information without compilation warnings
        }

        try
        {
            xTemplateProps->getPropertyValue( u"Geometry3D"_ustr ) >>= aRet.nGeometry3D;
        }
        catch( uno::Exception& ex )
        {
            //not all templates need to support Geometry3D
            ex.Context.is();//to have debug information without compilation warnings
        }
    }
    return aRet;
}
void ChartTypeDialogController::adjustSubTypeAndEnableControls( ChartTypeParameter& /*rParameter*/ )
{
}
void ChartTypeDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    switch( rParameter.nSubTypeIndex )
    {
        case 2:
            rParameter.eStackMode=GlobalStackMode_STACK_Y;
            break;
        case 3:
            rParameter.eStackMode=GlobalStackMode_STACK_Y_PERCENT;
            break;
        case 4:
            rParameter.eStackMode=GlobalStackMode_STACK_Z;
            break;
        default:
            rParameter.eStackMode=GlobalStackMode_NONE;
            break;
    }
}
void ChartTypeDialogController::adjustParameterToMainType( ChartTypeParameter& rParameter )
{
    bool bFoundSomeMatch=false;

    rParameter.bXAxisWithValues = bSupportsXAxisWithValues;
    if( rParameter.b3DLook && !bSupports3D )
        rParameter.b3DLook = false;
    if(!rParameter.b3DLook && rParameter.eStackMode==GlobalStackMode_STACK_Z)
        rParameter.eStackMode = GlobalStackMode_NONE;

    const tTemplateServiceChartTypeParameterMap& rMap = getTemplateMap();
    for( sal_Int32 nMatchPrecision=0; nMatchPrecision<7 && !bFoundSomeMatch; nMatchPrecision++ )
    {
        for (auto const& elem : rMap)
        {
            if( rParameter.mapsToSimilarService( elem.second, nMatchPrecision ) )
            {
                //remind some values
                ThreeDLookScheme aScheme = rParameter.eThreeDLookScheme;
                sal_Int32        nCurveResolution = rParameter.nCurveResolution;
                sal_Int32        nSplineOrder = rParameter.nSplineOrder;
                CurveStyle       eCurveStyle = rParameter.eCurveStyle;
                sal_Int32        nGeometry3D = rParameter.nGeometry3D;
                bool             bSortByXValues = rParameter.bSortByXValues;
                bool bRoundedEdge = rParameter.mbRoundedEdge;

                rParameter = elem.second;

                //some values should not be changed with charttype
                rParameter.eThreeDLookScheme = aScheme;
                rParameter.nCurveResolution = nCurveResolution;
                rParameter.nSplineOrder =nSplineOrder;
                rParameter.eCurveStyle = eCurveStyle;
                rParameter.nGeometry3D = nGeometry3D;
                rParameter.bSortByXValues = bSortByXValues;
                rParameter.mbRoundedEdge = bRoundedEdge;

                bFoundSomeMatch = true;
                break;
            }
        }
    }
    if(!bFoundSomeMatch)
    {
        if(!rMap.empty())
            rParameter = (*rMap.begin()).second;
        else
            rParameter = ChartTypeParameter();
    }
}
OUString ChartTypeDialogController::getServiceNameForParameter( const ChartTypeParameter& rParameter ) const
{
    ChartTypeParameter aParameter(rParameter);
    if( aParameter.bXAxisWithValues )
        aParameter.eStackMode = GlobalStackMode_NONE;
    if(!aParameter.b3DLook && aParameter.eStackMode==GlobalStackMode_STACK_Z)
        aParameter.eStackMode = GlobalStackMode_NONE;
    const tTemplateServiceChartTypeParameterMap& rMap = getTemplateMap();
    for (auto const& elem : rMap)
    {
        if( aParameter.mapsToSameService(elem.second) )
            return elem.first;
    }

    OSL_FAIL( "ChartType not implemented yet - use fallback to similar type" );
    for( sal_Int32 nMatchPrecision=1; nMatchPrecision<8; nMatchPrecision++ )
    {
        for (auto const& elem : rMap)
        {
            if( aParameter.mapsToSimilarService(elem.second, nMatchPrecision) )
                return elem.first;
        }
    }
    return OUString();
}
rtl::Reference< ChartTypeTemplate > ChartTypeDialogController::getCurrentTemplate(
    const ChartTypeParameter& rParameter
    , const rtl::Reference< ChartTypeManager >& xTemplateManager ) const
{
    rtl::Reference< ChartTypeTemplate > xTemplate;

    OUString aServiceName( getServiceNameForParameter( rParameter ) );
    if(!aServiceName.isEmpty())
    {
        xTemplate = xTemplateManager->createTemplate( aServiceName );
        if(xTemplate.is())
        {
            uno::Reference< beans::XPropertySet > xTemplateProps( static_cast<cppu::OWeakObject*>(xTemplate.get()), uno::UNO_QUERY );
            if(xTemplateProps.is())
            {
                try
                {
                    xTemplateProps->setPropertyValue( CHART_UNONAME_CURVE_STYLE , uno::Any(rParameter.eCurveStyle) );
                    xTemplateProps->setPropertyValue( CHART_UNONAME_CURVE_RESOLUTION , uno::Any(rParameter.nCurveResolution) );
                    xTemplateProps->setPropertyValue( CHART_UNONAME_SPLINE_ORDER , uno::Any(rParameter.nSplineOrder) );
                }
                catch( uno::Exception & ex )
                {
                    //not all templates need to support CurveStyle, CurveResolution or SplineOrder
                    ex.Context.is();//to have debug information without compilation warnings
                }
                try
                {
                    xTemplateProps->setPropertyValue( u"Geometry3D"_ustr , uno::Any(rParameter.nGeometry3D) );
                }
                catch( uno::Exception & ex )
                {
                    //not all templates need to support Geometry3D
                    ex.Context.is();//to have debug information without compilation warnings
                }

                try
                {
                    setTemplateProperties( xTemplateProps );
                }
                catch( const uno::Exception & )
                {
                    DBG_UNHANDLED_EXCEPTION("chart2");
                }
            }
        }
    }
    return xTemplate;
}

void ChartTypeDialogController::commitToModel( const ChartTypeParameter& rParameter
                , const rtl::Reference<::chart::ChartModel>& xChartModel
                , const uno::Reference<beans::XPropertySet>& xTemplateProps)
{
    rtl::Reference< ::chart::ChartTypeManager > xTemplateManager = xChartModel->getTypeManager();
    rtl::Reference< ::chart::ChartTypeTemplate > xTemplate( getCurrentTemplate( rParameter, xTemplateManager ) );
    if(!xTemplate.is())
        return;

    // locked controllers
    ControllerLockGuardUNO aCtrlLockGuard( xChartModel );
    rtl::Reference< Diagram > xDiagram = xChartModel->getFirstChartDiagram();
    Diagram::tTemplateWithServiceName aTemplateWithService;
    if (xDiagram)
        aTemplateWithService = xDiagram->getTemplate( xTemplateManager );
    if( aTemplateWithService.xChartTypeTemplate.is())
        aTemplateWithService.xChartTypeTemplate->resetStyles2( xDiagram );
    xTemplate->changeDiagram( xDiagram );
    if( AllSettings::GetMathLayoutRTL() )
        AxisHelper::setRTLAxisLayout( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 ) );
    if( rParameter.b3DLook )
        xDiagram->setScheme( rParameter.eThreeDLookScheme );

    if (xDiagram.is())
    {
        xDiagram->setPropertyValue(CHART_UNONAME_SORT_BY_XVALUES, uno::Any(rParameter.bSortByXValues));

        sal_Int32 nSplitPos;
        try {
            if (xTemplateProps.is()) {
                xTemplateProps->getPropertyValue(u"SplitPos"_ustr) >>= nSplitPos;
                xDiagram->setPropertyValue(u"SplitPos"_ustr, uno::Any(nSplitPos));
            }
        }
        catch( uno::Exception & ex )
        {
            //not all templates need to support SplitPos
            ex.Context.is();//to have debug information without compilation warnings
        }

    }
}
void ChartTypeDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.clear();
}
OUString ChartTypeDialogController::getChartName( sal_Int32 /*nId*/ ) const
{
    return OUString();
}
bool ChartTypeDialogController::shouldShow_3DLookControl() const
{
    return false;
}
bool ChartTypeDialogController::shouldShow_StackingControl() const
{
    return false;
}
bool ChartTypeDialogController::shouldShow_SplineControl() const
{
    return false;
}
bool ChartTypeDialogController::shouldShow_GeometryControl() const
{
    return false;
}
bool ChartTypeDialogController::shouldShow_SortByXValuesResourceGroup() const
{
    return false;
}

void ChartTypeDialogController::showExtraControls(weld::Builder* /*pBuilder*/)
{
}
void ChartTypeDialogController::hideExtraControls() const
{
}
void ChartTypeDialogController::fillExtraControls(  const rtl::Reference<::chart::ChartModel>& /*xChartModel*/
                                                  , const uno::Reference< beans::XPropertySet >& /*xTemplateProps*/ ) const
{
}
void ChartTypeDialogController::setTemplateProperties( const uno::Reference< beans::XPropertySet >& /*xTemplateProps*/ ) const
{
}

VclPtr<VirtualDevice> ChartTypeDialogController::getVDev(Image pImage)
{
    BitmapEx aPreviewBitmap = pImage.GetBitmapEx();
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    const Point aNull(0, 0);
    if (pVDev->GetDPIScaleFactor() > 1)
        aPreviewBitmap.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());
    const Size aSize(aPreviewBitmap.GetSizePixel());
    pVDev->SetOutputSizePixel(aSize);
    pVDev->DrawBitmapEx(aNull, aPreviewBitmap);

    return pVDev;
}

ColumnOrBarChartDialogController_Base::ColumnOrBarChartDialogController_Base()
{
}
ColumnOrBarChartDialogController_Base::~ColumnOrBarChartDialogController_Base()
{
}
bool ColumnOrBarChartDialogController_Base::shouldShow_3DLookControl() const
{
    return true;
}
bool ColumnOrBarChartDialogController_Base::shouldShow_GeometryControl() const
{
    return true;
}
void ColumnOrBarChartDialogController_Base::adjustSubTypeAndEnableControls( ChartTypeParameter& rParameter )
{
    if(rParameter.nSubTypeIndex>3 && !rParameter.b3DLook)
    {
        rParameter.nSubTypeIndex=1;
    }
}
ColumnChartDialogController::ColumnChartDialogController()
{
}
ColumnChartDialogController::~ColumnChartDialogController()
{
}
OUString ColumnChartDialogController::getName()
{
    return SchResId(STR_TYPE_COLUMN);
}

OUString ColumnChartDialogController::getImage()
{
    return BMP_TYPE_COLUMN;
}

const tTemplateServiceChartTypeParameterMap& ColumnChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
        {"com.sun.star.chart2.template.Column" ,                         ChartTypeParameter(1,false,false,GlobalStackMode_NONE)},
        {"com.sun.star.chart2.template.StackedColumn" ,                  ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y)},
        {"com.sun.star.chart2.template.PercentStackedColumn" ,           ChartTypeParameter(3,false,false,GlobalStackMode_STACK_Y_PERCENT)},
        {"com.sun.star.chart2.template.ThreeDColumnFlat" ,               ChartTypeParameter(1,false,true,GlobalStackMode_NONE)},
        {"com.sun.star.chart2.template.StackedThreeDColumnFlat" ,        ChartTypeParameter(2,false,true,GlobalStackMode_STACK_Y)},
        {"com.sun.star.chart2.template.PercentStackedThreeDColumnFlat" , ChartTypeParameter(3,false,true,GlobalStackMode_STACK_Y_PERCENT)},
        {"com.sun.star.chart2.template.ThreeDColumnDeep" ,               ChartTypeParameter(4,false,true,GlobalStackMode_STACK_Z)}};
    return s_aTemplateMap;
}
void ColumnChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.clear();

    OUString normalText = SchResId( STR_NORMAL );
    OUString stackedText = SchResId( STR_STACKED );
    OUString percentText = SchResId( STR_PERCENT );
    OUString deepText = SchResId( STR_DEEP );

    if( rParameter.b3DLook )
    {
        switch(rParameter.nGeometry3D)
        {
            case DataPointGeometry3D::CYLINDER:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_SAEULE_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_SAEULE_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_SAEULE_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_SAEULE_3D_4)));
            break;
            case DataPointGeometry3D::CONE:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_KEGEL_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_KEGEL_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_KEGEL_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_KEGEL_3D_4)));
            break;
            case DataPointGeometry3D::PYRAMID:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_PYRAMID_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_PYRAMID_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_PYRAMID_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_PYRAMID_3D_4)));
            break;
            default: //DataPointGeometry3D::CUBOID:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_COLUMNS_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_COLUMNS_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_COLUMNS_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_COLUMNS_3D)));
            break;
        }
    }
    else
    {
        rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_COLUMNS_2D_1)));
        rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_COLUMNS_2D_2)));
        rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_COLUMNS_2D_3)));
    }
}

OUString ColumnChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_NORMAL );
        case 2:
            return SchResId( STR_STACKED );
        case 3:
            return SchResId( STR_PERCENT );
        case 4:
            return SchResId( STR_DEEP );
        default:
            break;
    }
    return OUString();
}

BarChartDialogController::BarChartDialogController()
{
}

BarChartDialogController::~BarChartDialogController()
{
}

OUString BarChartDialogController::getName()
{
    return SchResId(STR_TYPE_BAR);
}

OUString BarChartDialogController::getImage()
{
    return BMP_TYPE_BAR;
}

const tTemplateServiceChartTypeParameterMap& BarChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
        {"com.sun.star.chart2.template.Bar" ,                         ChartTypeParameter(1,false,false,GlobalStackMode_NONE)},
        {"com.sun.star.chart2.template.StackedBar" ,                  ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y)},
        {"com.sun.star.chart2.template.PercentStackedBar" ,           ChartTypeParameter(3,false,false,GlobalStackMode_STACK_Y_PERCENT)},
        {"com.sun.star.chart2.template.ThreeDBarFlat" ,               ChartTypeParameter(1,false,true,GlobalStackMode_NONE)},
        {"com.sun.star.chart2.template.StackedThreeDBarFlat" ,        ChartTypeParameter(2,false,true,GlobalStackMode_STACK_Y)},
        {"com.sun.star.chart2.template.PercentStackedThreeDBarFlat" , ChartTypeParameter(3,false,true,GlobalStackMode_STACK_Y_PERCENT)},
        {"com.sun.star.chart2.template.ThreeDBarDeep" ,               ChartTypeParameter(4,false,true,GlobalStackMode_STACK_Z)}};
    return s_aTemplateMap;
}
void BarChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.clear();

    OUString normalText = SchResId( STR_NORMAL );
    OUString stackedText = SchResId( STR_STACKED );
    OUString percentText = SchResId( STR_PERCENT );
    OUString deepText = SchResId( STR_DEEP );

    if( rParameter.b3DLook )
    {
        switch(rParameter.nGeometry3D)
        {
            case DataPointGeometry3D::CYLINDER:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_ROEHRE_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_ROEHRE_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_ROEHRE_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_ROEHRE_3D_4)));
            break;
            case DataPointGeometry3D::CONE:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_KEGELQ_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_KEGELQ_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_KEGELQ_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_KEGELQ_3D_4)));
            break;
            case DataPointGeometry3D::PYRAMID:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_PYRAMIDQ_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_PYRAMIDQ_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_PYRAMIDQ_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_PYRAMIDQ_3D_4)));
            break;
            default: //DataPointGeometry3D::CUBOID:
                rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_BARS_3D_1)));
                rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_BARS_3D_2)));
                rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_BARS_3D_3)));
                rSubTypeList.append("4", deepText, getVDev(Image(StockImage::Yes, BMP_BARS_3D)));
            break;
        }
    }
    else
    {
        rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_BARS_2D_1)));
        rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_BARS_2D_2)));
        rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_BARS_2D_3)));
    }
}

OUString BarChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_NORMAL );
        case 2:
            return SchResId( STR_STACKED );
        case 3:
            return SchResId( STR_PERCENT );
        case 4:
            return SchResId( STR_DEEP );
        default:
            break;
    }
    return OUString();
}

//=========
// PieChartDialogController
//=========

PieChartDialogController::PieChartDialogController()
{
}

PieChartDialogController::~PieChartDialogController()
{
}

OUString PieChartDialogController::getName()
{
    return SchResId(STR_TYPE_PIE);
}

OUString PieChartDialogController::getImage()
{
    return BMP_TYPE_PIE;
}

const tTemplateServiceChartTypeParameterMap& PieChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.Pie" ,                    ChartTypeParameter(1,false,false)},
    {"com.sun.star.chart2.template.PieAllExploded" ,         ChartTypeParameter(2,false,false)},
    {"com.sun.star.chart2.template.Donut" ,                  ChartTypeParameter(3,false,false)},
    {"com.sun.star.chart2.template.DonutAllExploded" ,       ChartTypeParameter(4,false,false)},
    {"com.sun.star.chart2.template.ThreeDPie" ,              ChartTypeParameter(1,false,true)},
    {"com.sun.star.chart2.template.ThreeDPieAllExploded" ,   ChartTypeParameter(2,false,true)},
    {"com.sun.star.chart2.template.ThreeDDonut" ,            ChartTypeParameter(3,false,true)},
    {"com.sun.star.chart2.template.ThreeDDonutAllExploded" , ChartTypeParameter(4,false,true)}};
    return s_aTemplateMap;
}
void PieChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.clear();

    OUString normalText = SchResId( STR_NORMAL );
    OUString explodedText = SchResId( STR_PIE_EXPLODED );
    OUString donutText = SchResId( STR_DONUT );
    OUString donutExplodedText = SchResId( STR_DONUT_EXPLODED );

    if( rParameter.b3DLook )
    {
        rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_CIRCLES_3D)));
        rSubTypeList.append("2", explodedText, getVDev(Image(StockImage::Yes, BMP_CIRCLES_3D_EXPLODED)));
        rSubTypeList.append("3", donutText, getVDev(Image(StockImage::Yes, BMP_DONUT_3D)));
        rSubTypeList.append("4", donutExplodedText, getVDev(Image(StockImage::Yes, BMP_DONUT_3D_EXPLODED)));
    }
    else
    {
        rSubTypeList.append("1", normalText, getVDev(Image(StockImage::Yes, BMP_CIRCLES_2D)));
        rSubTypeList.append("2", explodedText, getVDev(Image(StockImage::Yes, BMP_CIRCLES_2D_EXPLODED)));
        rSubTypeList.append("3", donutText, getVDev(Image(StockImage::Yes, BMP_DONUT_2D)));
        rSubTypeList.append("4", donutExplodedText, getVDev(Image(StockImage::Yes, BMP_DONUT_2D_EXPLODED)));
    }
}

bool PieChartDialogController::shouldShow_3DLookControl() const
{
    return true;
}

OUString PieChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_NORMAL );
        case 2:
            return SchResId( STR_PIE_EXPLODED );
        case 3:
            return SchResId( STR_DONUT );
        case 4:
            return SchResId( STR_DONUT_EXPLODED );
        default:
            break;
    }
    return OUString();
}

void PieChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    if(rParameter.eStackMode==GlobalStackMode_STACK_Z)
        rParameter.eStackMode = GlobalStackMode_NONE;
}

//=========
// OfPieChartDialogController
//=========

OfPieChartDialogController::OfPieChartDialogController()
{
}

OfPieChartDialogController::~OfPieChartDialogController()
{
}

OUString OfPieChartDialogController::getName()
{
    return SchResId(STR_TYPE_OFPIE);
}

OUString OfPieChartDialogController::getImage()
{
    return BMP_TYPE_OFPIE;
}

const tTemplateServiceChartTypeParameterMap& OfPieChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.BarOfPie" ,               ChartTypeParameter(1,false,false)},
    {"com.sun.star.chart2.template.PieOfPie" ,               ChartTypeParameter(2,false,false)}};
    return s_aTemplateMap;
}
void OfPieChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.clear();

    OUString barOfPieText = SchResId( STR_BAR_OF_PIE );
    OUString pieOfPieText = SchResId( STR_PIE_OF_PIE );

    rSubTypeList.append("1", barOfPieText, getVDev(Image(StockImage::Yes, BMP_BAR_OF_PIE)));
    rSubTypeList.append("2", pieOfPieText, getVDev(Image(StockImage::Yes, BMP_PIE_OF_PIE)));
}

OUString OfPieChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_BAR_OF_PIE );
        case 2:
            return SchResId( STR_PIE_OF_PIE );
        default:
            break;
    }
    return OUString();
}

bool OfPieChartDialogController::shouldShow_3DLookControl() const
{
    // Maybe a TODO?
    return false;
}

void OfPieChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    if(rParameter.eStackMode==GlobalStackMode_STACK_Z)
        rParameter.eStackMode = GlobalStackMode_NONE;
}

void OfPieChartDialogController::showExtraControls(weld::Builder* pBuilder)
{
    if (!m_xFT_CompositeSize)
    {
        m_xFT_CompositeSize = pBuilder->weld_label(u"compositesizeft"_ustr);
        assert(m_xFT_CompositeSize);
    }
    if (!m_xMF_CompositeSize)
    {
        m_xMF_CompositeSize = pBuilder->weld_spin_button(u"compositesize"_ustr);
        assert(m_xMF_CompositeSize);

        m_xMF_CompositeSize->set_increments(1, 10);
        m_xMF_CompositeSize->set_range(2, 100);

        m_xMF_CompositeSize->connect_value_changed( LINK( this,
                    OfPieChartDialogController, ChangeCompositeSizeHdl ) );
        m_xMF_CompositeSize->set_sensitive(true);
    }

    m_xFT_CompositeSize->show();
    m_xMF_CompositeSize->show();
}

void OfPieChartDialogController::hideExtraControls() const
{
    if (m_xFT_CompositeSize)
        m_xFT_CompositeSize->hide();
    if (m_xMF_CompositeSize)
        m_xMF_CompositeSize->hide();
}

void OfPieChartDialogController::fillExtraControls(
                  const rtl::Reference<::chart::ChartModel>& xChartModel
                , const uno::Reference< beans::XPropertySet >& xTemplateProps ) const
{
    if (!m_xMF_CompositeSize)
        return;

    rtl::Reference< Diagram > xDiagram = xChartModel->getFirstChartDiagram();
    if(!xDiagram.is())
        return;

    sal_Int32 nCompositeSize = 2;

    if(xTemplateProps.is())
    {
        try
        {
            xTemplateProps->getPropertyValue( u"SplitPos"_ustr ) >>= nCompositeSize;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
    if( nCompositeSize < 2 )
        nCompositeSize = 2;
    m_xMF_CompositeSize->set_value(nCompositeSize);

    // Limit based on number of entries in the series
    const std::vector< rtl::Reference< DataSeries > > dataSeriesVec =
        ChartModelHelper::getDataSeries( xChartModel);
    if (!dataSeriesVec.empty()) {
        const rtl::Reference<DataSeries>& ds = dataSeriesVec[0];
        const DataSeries::tDataSequenceContainer data = ds->getDataSequences2();
        const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aValuesSeries(
            DataSeriesHelper::getAllDataSequencesByRole( data , u"values"_ustr ) );

        assert(!aValuesSeries.empty());

        if (!aValuesSeries.empty())
        {
            const uno::Reference< chart2::data::XDataSequence > xSeq( aValuesSeries.front()->getValues() );

            // Allow all but one entry to be aggregated in the composite wedge
            sal_Int32 nMaxCompositeSize = xSeq->getData().getLength() - 1;

            if( nMaxCompositeSize < 2 )
                nMaxCompositeSize = 2;
            m_xMF_CompositeSize->set_max(nMaxCompositeSize);
        }
    }
}

void OfPieChartDialogController::setTemplateProperties( const uno::Reference< beans::XPropertySet >& xTemplateProps ) const
{
    if( xTemplateProps.is())
    {
        sal_Int32 nCompositeSize = m_xMF_CompositeSize->get_value();
        xTemplateProps->setPropertyValue( u"SplitPos"_ustr , uno::Any(nCompositeSize) );
    }
}

IMPL_LINK_NOARG(OfPieChartDialogController, ChangeCompositeSizeHdl, weld::SpinButton&, void)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged();
}

//=========
// LineChartDialogController
//=========

LineChartDialogController::LineChartDialogController()
{
}

LineChartDialogController::~LineChartDialogController()
{
}

OUString LineChartDialogController::getName()
{
    return SchResId(STR_TYPE_LINE);
}

OUString LineChartDialogController::getImage()
{
    return BMP_TYPE_LINE;
}

const tTemplateServiceChartTypeParameterMap& LineChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.Symbol" ,                     ChartTypeParameter(1,false,false,GlobalStackMode_NONE,true,false)},
    {"com.sun.star.chart2.template.StackedSymbol" ,              ChartTypeParameter(1,false,false,GlobalStackMode_STACK_Y,true,false)},
    {"com.sun.star.chart2.template.PercentStackedSymbol" ,       ChartTypeParameter(1,false,false,GlobalStackMode_STACK_Y_PERCENT,true,false)},
    {"com.sun.star.chart2.template.LineSymbol" ,                 ChartTypeParameter(2,false,false,GlobalStackMode_NONE,true,true)},
    {"com.sun.star.chart2.template.StackedLineSymbol" ,          ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y,true,true)},
    {"com.sun.star.chart2.template.PercentStackedLineSymbol" ,   ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y_PERCENT,true,true)},
    {"com.sun.star.chart2.template.Line" ,                       ChartTypeParameter(3,false,false,GlobalStackMode_NONE,false,true)},
    {"com.sun.star.chart2.template.StackedLine" ,                ChartTypeParameter(3,false,false,GlobalStackMode_STACK_Y,false,true)},
    {"com.sun.star.chart2.template.PercentStackedLine" ,         ChartTypeParameter(3,false,false,GlobalStackMode_STACK_Y_PERCENT,false,true)},
    {"com.sun.star.chart2.template.StackedThreeDLine" ,          ChartTypeParameter(4,false,true,GlobalStackMode_STACK_Y,false,true)},
    {"com.sun.star.chart2.template.PercentStackedThreeDLine" ,   ChartTypeParameter(4,false,true,GlobalStackMode_STACK_Y_PERCENT,false,true)},
    {"com.sun.star.chart2.template.ThreeDLineDeep" ,             ChartTypeParameter(4,false,true,GlobalStackMode_STACK_Z,false,true)}};
    return s_aTemplateMap;
}
void LineChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.clear();

    OUString pointsOnlyText = SchResId( STR_POINTS_ONLY );
    OUString pointsAndLinesText = SchResId( STR_POINTS_AND_LINES );
    OUString linesOnlyText = SchResId( STR_LINES_ONLY );
    OUString lines3DText = SchResId( STR_LINES_3D );

    switch( rParameter.eCurveStyle )
    {
        case CurveStyle_CUBIC_SPLINES:
        case CurveStyle_B_SPLINES:
            if( rParameter.eStackMode == GlobalStackMode_NONE || rParameter.eStackMode == GlobalStackMode_STACK_Z )
            {
                rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_XCATEGORY)));
                rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_XCATEGORY_SMOOTH)));
                rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_XCATEGORY_SMOOTH)));
                rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_XCATEGORY_SMOOTH)));
            }
            else
            {
                rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_STACKED)));
                rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_STACKED_SMOOTH)));
                rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_STACKED_SMOOTH)));
                rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_STACKED_SMOOTH)));
            }
            break;
        case CurveStyle_STEP_START:
        case CurveStyle_STEP_END:
        case CurveStyle_STEP_CENTER_X:
        case CurveStyle_STEP_CENTER_Y:
            if( rParameter.eStackMode == GlobalStackMode_NONE || rParameter.eStackMode == GlobalStackMode_STACK_Z )
            {
                rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_XCATEGORY)));
                rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_XCATEGORY_STEPPED)));
                rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_XCATEGORY_STEPPED)));
                rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_XCATEGORY_STEPPED)));
            }
            else
            {
                rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_STACKED)));
                rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_STACKED_STEPPED)));
                rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_STACKED_STEPPED)));
                rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_STACKED_STEPPED)));
            }
            break;
        default: // includes CurveStyle_LINES
            //direct lines
            if( rParameter.eStackMode == GlobalStackMode_NONE || rParameter.eStackMode == GlobalStackMode_STACK_Z )
            {
                rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_XCATEGORY)));
                rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_XCATEGORY)));
                rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_XCATEGORY)));
                rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_XCATEGORY)));
            }
            else
            {
                rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_STACKED)));
                rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_STACKED)));
                rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_STACKED)));
                rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_STACKED)));
            }
    }
}
OUString LineChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_POINTS_ONLY );
        case 2:
            return SchResId( STR_POINTS_AND_LINES );
        case 3:
            return SchResId( STR_LINES_ONLY );
        case 4:
            return SchResId( STR_LINES_3D );
        default:
            break;
    }
    return OUString();
}
bool LineChartDialogController::shouldShow_StackingControl() const
{
    return true;
}
bool LineChartDialogController::shouldShow_SplineControl() const
{
    return true;
}
void LineChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = false;

    switch( rParameter.nSubTypeIndex )
    {
        case 2:
            rParameter.bSymbols = true;
            rParameter.bLines = true;
            break;
        case 3:
            rParameter.bSymbols = false;
            rParameter.bLines = true;
            break;
        case 4:
            rParameter.bSymbols = false;
            rParameter.bLines = true;
            rParameter.b3DLook = true;
            if( rParameter.eStackMode == GlobalStackMode_NONE )
                rParameter.eStackMode = GlobalStackMode_STACK_Z;
            break;
        default:
            rParameter.bSymbols = true;
            rParameter.bLines = false;
            break;
    }

    if(!rParameter.b3DLook && rParameter.eStackMode == GlobalStackMode_STACK_Z )
        rParameter.eStackMode = GlobalStackMode_NONE;
}
void LineChartDialogController::adjustParameterToMainType( ChartTypeParameter& rParameter )
{
    if( rParameter.b3DLook && rParameter.eStackMode == GlobalStackMode_NONE )
        rParameter.eStackMode = GlobalStackMode_STACK_Z;

    ChartTypeDialogController::adjustParameterToMainType( rParameter );
}

XYChartDialogController::XYChartDialogController()
{
    bSupportsXAxisWithValues = true;
}

XYChartDialogController::~XYChartDialogController()
{
}

OUString XYChartDialogController::getName()
{
    return SchResId(STR_TYPE_XY);
}

OUString XYChartDialogController::getImage()
{
    return BMP_TYPE_XY;
}

const tTemplateServiceChartTypeParameterMap& XYChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.ScatterSymbol" ,              ChartTypeParameter(1,true,false,GlobalStackMode_NONE,true,false)},
    {"com.sun.star.chart2.template.ScatterLineSymbol" ,          ChartTypeParameter(2,true,false,GlobalStackMode_NONE,true,true)},
    {"com.sun.star.chart2.template.ScatterLine" ,                ChartTypeParameter(3,true,false,GlobalStackMode_NONE,false,true)},
    {"com.sun.star.chart2.template.ThreeDScatter" ,              ChartTypeParameter(4,true,true,GlobalStackMode_NONE,false,true)}};
    return s_aTemplateMap;
}

void XYChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.clear();

    OUString pointsOnlyText = SchResId( STR_POINTS_ONLY );
    OUString pointsAndLinesText = SchResId( STR_POINTS_AND_LINES );
    OUString linesOnlyText = SchResId( STR_LINES_ONLY );
    OUString lines3DText = SchResId( STR_LINES_3D );

    switch (rParameter.eCurveStyle)
    {
        case CurveStyle_CUBIC_SPLINES:
        case CurveStyle_B_SPLINES:
        {
            rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_XVALUES)));
            rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_XVALUES_SMOOTH)));
            rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_XVALUES_SMOOTH)));
            rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_XVALUES_SMOOTH)));
            break;
        }
        case CurveStyle_STEP_START:
        case CurveStyle_STEP_END:
        case CurveStyle_STEP_CENTER_X:
        case CurveStyle_STEP_CENTER_Y:
        {
            rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_XVALUES)));
            rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_XVALUES_STEPPED)));
            rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_XVALUES_STEPPED)));
            rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_XVALUES_STEPPED)));
            break;
        }
        default: // includes CurveStyle_LINES
            rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_POINTS_XVALUES)));
            rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_LINE_P_XVALUES)));
            rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_LINE_O_XVALUES)));
            rSubTypeList.append("4", lines3DText, getVDev(Image(StockImage::Yes, BMP_LINE3D_XVALUES)));
    }
}
OUString XYChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_POINTS_ONLY );
        case 2:
            return SchResId( STR_POINTS_AND_LINES );
        case 3:
            return SchResId( STR_LINES_ONLY );
        case 4:
            return SchResId( STR_LINES_3D );
        default:
            break;
    }
    return OUString();
}
bool XYChartDialogController::shouldShow_SplineControl() const
{
    return true;
}
bool XYChartDialogController::shouldShow_SortByXValuesResourceGroup() const
{
    return true;
}
void XYChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.eStackMode=GlobalStackMode_NONE;
    rParameter.b3DLook = false;
    rParameter.bXAxisWithValues = true;

    switch( rParameter.nSubTypeIndex )
    {
        case 2:
            rParameter.bSymbols = true;
            rParameter.bLines = true;
            break;
        case 3:
            rParameter.bSymbols = false;
            rParameter.bLines = true;
            break;
        case 4:
            rParameter.bSymbols = false;
            rParameter.bLines = true;
            rParameter.b3DLook = true;
            rParameter.eStackMode=GlobalStackMode_STACK_Z;
            break;
        default:
            rParameter.bSymbols = true;
            rParameter.bLines = false;
            break;
    }
}

AreaChartDialogController::AreaChartDialogController()
{
}

AreaChartDialogController::~AreaChartDialogController()
{
}

OUString AreaChartDialogController::getName()
{
    return SchResId(STR_TYPE_AREA);
}

OUString AreaChartDialogController::getImage()
{
    return BMP_TYPE_AREA;
}

bool AreaChartDialogController::shouldShow_3DLookControl() const
{
    return true;
}

const tTemplateServiceChartTypeParameterMap& AreaChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.Area" ,                     ChartTypeParameter(1,false,false,GlobalStackMode_NONE)},
    {"com.sun.star.chart2.template.ThreeDArea" ,               ChartTypeParameter(1,false,true,GlobalStackMode_STACK_Z)},
    {"com.sun.star.chart2.template.StackedArea" ,              ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y)},
    {"com.sun.star.chart2.template.StackedThreeDArea" ,        ChartTypeParameter(2,false,true,GlobalStackMode_STACK_Y)},
    {"com.sun.star.chart2.template.PercentStackedArea" ,       ChartTypeParameter(3,false,false,GlobalStackMode_STACK_Y_PERCENT)},
    {"com.sun.star.chart2.template.PercentStackedThreeDArea" , ChartTypeParameter(3,false,true,GlobalStackMode_STACK_Y_PERCENT)}};
    return s_aTemplateMap;
}

void AreaChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.clear();
    m_b3DLook = rParameter.b3DLook;

    OUString firstText = SchResId( m_b3DLook ? STR_DEEP : STR_NORMAL );
    OUString stackedText = SchResId( STR_STACKED );
    OUString percentText = SchResId( STR_PERCENT );

    if( rParameter.b3DLook )
    {
        rSubTypeList.append("1", firstText, getVDev(Image(StockImage::Yes, BMP_AREAS_3D)));
        rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_AREAS_3D_1)));
        rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_AREAS_3D_2)));
    }
    else
    {
        rSubTypeList.append("1", firstText, getVDev(Image(StockImage::Yes, BMP_AREAS_2D_1)));
        rSubTypeList.append("2", stackedText, getVDev(Image(StockImage::Yes, BMP_AREAS_2D)));
        rSubTypeList.append("3", percentText, getVDev(Image(StockImage::Yes, BMP_AREAS_2D_3)));
    }
}
OUString AreaChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( m_b3DLook ? STR_DEEP : STR_NORMAL );
        case 2:
            return SchResId( STR_STACKED );
        case 3:
            return SchResId( STR_PERCENT );
        default:
            break;
    }
    return OUString();
}
void AreaChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.eCurveStyle = CurveStyle_LINES;

    if( rParameter.nSubTypeIndex>3 )
        rParameter.nSubTypeIndex = 1;
    switch( rParameter.nSubTypeIndex )
    {
        case 2:
            rParameter.eStackMode=GlobalStackMode_STACK_Y;
            break;
        case 3:
            rParameter.eStackMode=GlobalStackMode_STACK_Y_PERCENT;
            break;
        default:
            if( rParameter.b3DLook )
                rParameter.eStackMode=GlobalStackMode_STACK_Z;
            else
                rParameter.eStackMode=GlobalStackMode_NONE;
            break;
    }
}
void AreaChartDialogController::adjustParameterToMainType( ChartTypeParameter& rParameter )
{
    if( rParameter.b3DLook && rParameter.eStackMode == GlobalStackMode_NONE )
        rParameter.eStackMode = GlobalStackMode_STACK_Z;

    ChartTypeDialogController::adjustParameterToMainType( rParameter );
}

NetChartDialogController::NetChartDialogController()
{
    bSupports3D = false;
}

NetChartDialogController::~NetChartDialogController()
{
}

OUString NetChartDialogController::getName()
{
    return SchResId(STR_TYPE_NET);
}

OUString NetChartDialogController::getImage()
{
    return BMP_TYPE_NET;
}

bool NetChartDialogController::shouldShow_StackingControl() const
{
    return true;
}

const tTemplateServiceChartTypeParameterMap& NetChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    //@todo need templates with symbols only
    {"com.sun.star.chart2.template.NetSymbol" ,              ChartTypeParameter(1,false,false,GlobalStackMode_NONE,true,false)},
    {"com.sun.star.chart2.template.StackedNetSymbol" ,       ChartTypeParameter(1,false,false,GlobalStackMode_STACK_Y,true,false)},
    {"com.sun.star.chart2.template.PercentStackedNetSymbol" ,ChartTypeParameter(1,false,false,GlobalStackMode_STACK_Y_PERCENT,true,false)},

    {"com.sun.star.chart2.template.Net" ,                    ChartTypeParameter(2,false,false,GlobalStackMode_NONE,true,true)},
    {"com.sun.star.chart2.template.StackedNet" ,             ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y,true,true)},
    {"com.sun.star.chart2.template.PercentStackedNet" ,      ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y_PERCENT,true,true)},

    {"com.sun.star.chart2.template.NetLine" ,                ChartTypeParameter(3,false,false,GlobalStackMode_NONE,false,true)},
    {"com.sun.star.chart2.template.StackedNetLine" ,         ChartTypeParameter(3,false,false,GlobalStackMode_STACK_Y,false,true)},
    {"com.sun.star.chart2.template.PercentStackedNetLine" ,  ChartTypeParameter(3,false,false,GlobalStackMode_STACK_Y_PERCENT,false,true)},

    {"com.sun.star.chart2.template.FilledNet" ,              ChartTypeParameter(4,false,false,GlobalStackMode_NONE,false,false)},
    {"com.sun.star.chart2.template.StackedFilledNet" ,       ChartTypeParameter(4,false,false,GlobalStackMode_STACK_Y,false,false)},
    {"com.sun.star.chart2.template.PercentStackedFilledNet" ,ChartTypeParameter(4,false,false,GlobalStackMode_STACK_Y_PERCENT,false,false)}};
    return s_aTemplateMap;
}
void NetChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.clear();

    OUString pointsOnlyText = SchResId( STR_POINTS_ONLY );
    OUString pointsAndLinesText = SchResId( STR_POINTS_AND_LINES );
    OUString linesOnlyText = SchResId( STR_LINES_ONLY );
    OUString filledText = SchResId( STR_FILLED );

    if( rParameter.eStackMode == GlobalStackMode_NONE )
    {
        rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_NET_SYMB)));
        rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_NET_LINESYMB)));
        rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_NET)));
        rSubTypeList.append("4", filledText, getVDev(Image(StockImage::Yes, BMP_NET_FILL)));
    }
    else
    {
        rSubTypeList.append("1", pointsOnlyText, getVDev(Image(StockImage::Yes, BMP_NET_SYMB_STACK)));
        rSubTypeList.append("2", pointsAndLinesText, getVDev(Image(StockImage::Yes, BMP_NET_LINESYMB_STACK)));
        rSubTypeList.append("3", linesOnlyText, getVDev(Image(StockImage::Yes, BMP_NET_STACK)));
        rSubTypeList.append("4", filledText, getVDev(Image(StockImage::Yes, BMP_NET_FILL_STACK)));
    }
}
OUString NetChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_POINTS_ONLY );
        case 2:
            return SchResId( STR_POINTS_AND_LINES );
        case 3:
            return SchResId( STR_LINES_ONLY );
        case 4:
            return SchResId( STR_FILLED );
        default:
            break;
    }
    return OUString();
}
void NetChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = false;
    if(rParameter.eStackMode==GlobalStackMode_STACK_Z)
        rParameter.eStackMode = GlobalStackMode_NONE;

    switch( rParameter.nSubTypeIndex )
    {
        case 2:
            rParameter.bSymbols = true;
            rParameter.bLines = true;
            break;
        case 3:
            rParameter.bSymbols = false;
            rParameter.bLines = true;
            break;
        case 4:
            rParameter.bSymbols = false;
            rParameter.bLines = false;
            break;
        default:
            rParameter.bSymbols = true;
            rParameter.bLines = false;
            break;
    }
}
StockChartDialogController::StockChartDialogController()
{
    bSupports3D = false;
}

StockChartDialogController::~StockChartDialogController()
{
}

OUString StockChartDialogController::getName()
{
    return SchResId(STR_TYPE_STOCK);
}

OUString StockChartDialogController::getImage()
{
    return BMP_TYPE_STOCK;
}

const tTemplateServiceChartTypeParameterMap& StockChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.StockLowHighClose" ,          ChartTypeParameter(1)},
    {"com.sun.star.chart2.template.StockOpenLowHighClose" ,      ChartTypeParameter(2)},
    {"com.sun.star.chart2.template.StockVolumeLowHighClose" ,    ChartTypeParameter(3)},
    {"com.sun.star.chart2.template.StockVolumeOpenLowHighClose" ,ChartTypeParameter(4)}};
    return s_aTemplateMap;
}

void StockChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.clear();

    OUString stock1Text = SchResId(STR_STOCK_1);
    OUString stock2Text = SchResId(STR_STOCK_2);
    OUString stock3Text = SchResId(STR_STOCK_3);
    OUString stock4Text = SchResId(STR_STOCK_4);

    rSubTypeList.append("1", stock1Text, getVDev(Image(StockImage::Yes, BMP_STOCK_1)));
    rSubTypeList.append("2", stock2Text, getVDev(Image(StockImage::Yes, BMP_STOCK_2)));
    rSubTypeList.append("3", stock3Text, getVDev(Image(StockImage::Yes, BMP_STOCK_3)));
    rSubTypeList.append("4", stock4Text, getVDev(Image(StockImage::Yes, BMP_STOCK_4)));
}

OUString StockChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_STOCK_1 );
        case 2:
            return SchResId( STR_STOCK_2 );
        case 3:
            return SchResId( STR_STOCK_3 );
        case 4:
            return SchResId( STR_STOCK_4 );
        default:
            break;
    }
    return OUString();
}

void StockChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = false;
    rParameter.eStackMode = GlobalStackMode_NONE;
}

CombiColumnLineChartDialogController::CombiColumnLineChartDialogController()
{
    bSupports3D = false;
}

OUString CombiColumnLineChartDialogController::getName()
{
    return SchResId(STR_TYPE_COMBI_COLUMN_LINE);
}

OUString CombiColumnLineChartDialogController::getImage()
{
    return BMP_TYPE_COLUMN_LINE;
}

const tTemplateServiceChartTypeParameterMap& CombiColumnLineChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.ColumnWithLine" ,             ChartTypeParameter(1)},
    {"com.sun.star.chart2.template.StackedColumnWithLine" ,      ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y)}};
    return s_aTemplateMap;
}

void CombiColumnLineChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.clear();

    OUString lineColumnText = SchResId(STR_LINE_COLUMN);
    OUString lineStackedColumnText = SchResId(STR_LINE_STACKEDCOLUMN);

    rSubTypeList.append("1", lineColumnText, getVDev(Image(StockImage::Yes, BMP_COLUMN_LINE)));
    rSubTypeList.append("2", lineStackedColumnText, getVDev(Image(StockImage::Yes, BMP_COLUMN_LINE_STACKED)));
}

OUString CombiColumnLineChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_LINE_COLUMN );
        case 2:
            return SchResId( STR_LINE_STACKEDCOLUMN );
        default:
            break;
    }
    return OUString();
}

void CombiColumnLineChartDialogController::showExtraControls(weld::Builder* pBuilder)
{
    if (!m_xFT_NumberOfLines)
    {
        m_xFT_NumberOfLines = pBuilder->weld_label(u"nolinesft"_ustr);
    }
    if (!m_xMF_NumberOfLines)
    {
        m_xMF_NumberOfLines = pBuilder->weld_spin_button(u"nolines"_ustr);

        m_xMF_NumberOfLines->set_increments(1, 10);
        m_xMF_NumberOfLines->set_range(1, 100);

        m_xMF_NumberOfLines->connect_value_changed( LINK( this, CombiColumnLineChartDialogController, ChangeLineCountHdl ) );
    }

    m_xFT_NumberOfLines->show();
    m_xMF_NumberOfLines->show();
}

void CombiColumnLineChartDialogController::hideExtraControls() const
{
    if (m_xFT_NumberOfLines)
        m_xFT_NumberOfLines->hide();
    if (m_xMF_NumberOfLines)
        m_xMF_NumberOfLines->hide();
}

void CombiColumnLineChartDialogController::fillExtraControls(
                  const rtl::Reference<::chart::ChartModel>& xChartModel
                , const uno::Reference< beans::XPropertySet >& xTemplateProps ) const
{
    if (!m_xMF_NumberOfLines)
        return;

    rtl::Reference< Diagram > xDiagram = xChartModel->getFirstChartDiagram();
    if(!xDiagram.is())
        return;

    sal_Int32 nNumLines = 0;

    if(xTemplateProps.is())
    {
        try
        {
            xTemplateProps->getPropertyValue( u"NumberOfLines"_ustr ) >>= nNumLines;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
    if( nNumLines < 0 )
        nNumLines = 0;
    m_xMF_NumberOfLines->set_value(nNumLines);

    sal_Int32 nMaxLines = ChartModelHelper::getDataSeries( xChartModel ).size() - 1;
    if( nMaxLines < 0 )
        nMaxLines = 0;
    m_xMF_NumberOfLines->set_max(nMaxLines);
}
void CombiColumnLineChartDialogController::setTemplateProperties( const uno::Reference< beans::XPropertySet >& xTemplateProps ) const
{
    if( xTemplateProps.is() )
    {
        sal_Int32 nNumLines = m_xMF_NumberOfLines->get_value();
        xTemplateProps->setPropertyValue( u"NumberOfLines"_ustr , uno::Any(nNumLines) );
    }
}

IMPL_LINK_NOARG(CombiColumnLineChartDialogController, ChangeLineCountHdl, weld::SpinButton&, void)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged();
}
void CombiColumnLineChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = false;
    if(rParameter.eStackMode==GlobalStackMode_STACK_Z)
        rParameter.eStackMode = GlobalStackMode_NONE;

    switch( rParameter.nSubTypeIndex )
    {
        case 2:
            rParameter.eStackMode=GlobalStackMode_STACK_Y;
            break;
        default:
            rParameter.eStackMode=GlobalStackMode_NONE;
            break;
    }
}

BubbleChartDialogController::BubbleChartDialogController()
{
}

BubbleChartDialogController::~BubbleChartDialogController()
{
}

OUString BubbleChartDialogController::getName()
{
    return SchResId(STR_TYPE_BUBBLE);
}

OUString BubbleChartDialogController::getImage()
{
    return BMP_TYPE_BUBBLE;
}

const tTemplateServiceChartTypeParameterMap& BubbleChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
        {"com.sun.star.chart2.template.Bubble" ,          ChartTypeParameter(1,true)}};
    return s_aTemplateMap;
}
void BubbleChartDialogController::fillSubTypeList( weld::IconView& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.clear();

    OUString bubble1Text = SchResId(STR_BUBBLE_1);

    rSubTypeList.append("1", bubble1Text, getVDev(Image(StockImage::Yes, BMP_BUBBLE_1)));
}
OUString BubbleChartDialogController::getChartName( sal_Int32 nId ) const
{
    switch (nId)
    {
        case 1:
            return SchResId( STR_BUBBLE_1 );
        default:
            break;
    }
    return OUString();
}
void BubbleChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = false;
    rParameter.eStackMode = GlobalStackMode_NONE;
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
