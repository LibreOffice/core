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

#include "ChartTypeDialogController.hxx"
#include "ResId.hxx"
#include "HelpIds.hrc"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "AxisHelper.hxx"
#include <unonames.hxx>

#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/PieChartOffsetMode.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

#include <svtools/controldims.hrc>
#include <svtools/valueset.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/builder.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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
                    , eThreeDLookScheme(ThreeDLookScheme_Realistic)
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
                    , eThreeDLookScheme(ThreeDLookScheme_Realistic)
                    , bSortByXValues(false)
                    , mbRoundedEdge(false)
{
}

bool ChartTypeParameter::mapsToSameService( const ChartTypeParameter& rParameter ) const
{
    return this->mapsToSimilarService( rParameter, 0 );
}
bool ChartTypeParameter::mapsToSimilarService( const ChartTypeParameter& rParameter, sal_Int32 nTheHigherTheLess ) const
{
    sal_Int32 nMax=7;
    if(nTheHigherTheLess>nMax)
        return true;
    if( this->bXAxisWithValues!=rParameter.bXAxisWithValues )
        return nTheHigherTheLess>nMax-1;
    if( this->b3DLook!=rParameter.b3DLook )
        return nTheHigherTheLess>nMax-2;
    if( this->eStackMode!=rParameter.eStackMode )
        return nTheHigherTheLess>nMax-3;
    if( this->nSubTypeIndex!=rParameter.nSubTypeIndex )
        return nTheHigherTheLess>nMax-4;
    if( this->bSymbols!=rParameter.bSymbols )
        return nTheHigherTheLess>nMax-5;
    if( this->bLines!=rParameter.bLines )
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
Image ChartTypeDialogController::getImage()
{
    return Image();
}
bool ChartTypeDialogController::isSubType( const OUString& rServiceName )
{
    const tTemplateServiceChartTypeParameterMap& rTemplateMap = this->getTemplateMap();
    tTemplateServiceChartTypeParameterMap::const_iterator aIt( rTemplateMap.find( rServiceName ));
    if( aIt != rTemplateMap.end())
        return true;
    return false;
}
ChartTypeParameter ChartTypeDialogController::getChartTypeParameterForService(
                                                    const OUString& rServiceName
                                                    , const uno::Reference< beans::XPropertySet >& xTemplateProps )
{
    ChartTypeParameter aRet;
    const tTemplateServiceChartTypeParameterMap& rTemplateMap = this->getTemplateMap();
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
            xTemplateProps->getPropertyValue( "Geometry3D" ) >>= aRet.nGeometry3D;
        }
        catch( uno::Exception& ex )
        {
            //not all templates need to support Geometry3D
            ex.Context.is();//to have debug information without compilation warnings
        }

        try
        {
            xTemplateProps->getPropertyValue(CHART_UNONAME_ROUNDED_EDGE) >>= aRet.mbRoundedEdge;
        }
        catch ( const uno::Exception& ) {}
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
    tTemplateServiceChartTypeParameterMap::const_iterator       aIter = rMap.begin();
    const tTemplateServiceChartTypeParameterMap::const_iterator aEnd  = rMap.end();
    for( sal_Int32 nMatchPrecision=0; nMatchPrecision<7 && !bFoundSomeMatch; nMatchPrecision++ )
    {
        for( aIter = rMap.begin(); aIter != aEnd; ++aIter )
        {
            if( rParameter.mapsToSimilarService( (*aIter).second, nMatchPrecision ) )
            {
                //remind some values
                ThreeDLookScheme aScheme = rParameter.eThreeDLookScheme;
                sal_Int32        nCurveResolution = rParameter.nCurveResolution;
                sal_Int32        nSplineOrder = rParameter.nSplineOrder;
                CurveStyle       eCurveStyle = rParameter.eCurveStyle;
                sal_Int32        nGeometry3D = rParameter.nGeometry3D;
                bool             bSortByXValues = rParameter.bSortByXValues;
                bool bRoundedEdge = rParameter.mbRoundedEdge;

                rParameter = (*aIter).second;

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
    tTemplateServiceChartTypeParameterMap::const_iterator       aIter = rMap.begin();
    const tTemplateServiceChartTypeParameterMap::const_iterator aEnd  = rMap.end();
    for( ; aIter != aEnd; ++aIter )
    {
        if( aParameter.mapsToSameService( (*aIter).second ) )
            return (*aIter).first;
    }

    OSL_FAIL( "ChartType not implemented yet - use fallback to similar type" );
    for( sal_Int32 nMatchPrecision=1; nMatchPrecision<8; nMatchPrecision++ )
    {
        for( aIter = rMap.begin(); aIter != aEnd; ++aIter )
        {
            if( aParameter.mapsToSimilarService( (*aIter).second, nMatchPrecision ) )
                return (*aIter).first;
        }
    }
    return OUString();
}
uno::Reference< XChartTypeTemplate > ChartTypeDialogController::getCurrentTemplate(
    const ChartTypeParameter& rParameter
    , const uno::Reference< lang::XMultiServiceFactory >& xTemplateManager ) const
{
    uno::Reference< XChartTypeTemplate > xTemplate(nullptr);

    OUString aServiceName( this->getServiceNameForParameter( rParameter ) );
    if(!aServiceName.isEmpty())
    {
        xTemplate.set( xTemplateManager->createInstance( aServiceName ), uno::UNO_QUERY );
        if(xTemplate.is())
        {
            uno::Reference< beans::XPropertySet > xTemplateProps( xTemplate, uno::UNO_QUERY );
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
                    xTemplateProps->setPropertyValue( "Geometry3D" , uno::Any(rParameter.nGeometry3D) );
                }
                catch( uno::Exception & ex )
                {
                    //not all templates need to support Geometry3D
                    ex.Context.is();//to have debug information without compilation warnings
                }

                try
                {
                    xTemplateProps->setPropertyValue(CHART_UNONAME_ROUNDED_EDGE, uno::Any(rParameter.mbRoundedEdge));
                }
                catch ( const uno::Exception& )
                {
                }

                try
                {
                    this->setTemplateProperties( xTemplateProps );
                }
                catch( const uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
        }
    }
    return xTemplate;
}

void ChartTypeDialogController::commitToModel( const ChartTypeParameter& rParameter
                , const uno::Reference< XChartDocument >& xChartModel )
{
    uno::Reference< lang::XMultiServiceFactory > xTemplateManager( xChartModel->getChartTypeManager(), uno::UNO_QUERY );
    uno::Reference< XChartTypeTemplate > xTemplate( this->getCurrentTemplate( rParameter, xTemplateManager ) );
    if(xTemplate.is())
    {
        uno::Reference< frame::XModel > xModel( xChartModel, uno::UNO_QUERY);

        // locked controllers
        ControllerLockGuardUNO aCtrlLockGuard( xModel );
        uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xModel );
        DiagramHelper::tTemplateWithServiceName aTemplateWithService(
            DiagramHelper::getTemplateForDiagram( xDiagram, xTemplateManager ));
        if( aTemplateWithService.first.is())
            aTemplateWithService.first->resetStyles( xDiagram );
        xTemplate->changeDiagram( xDiagram );
        if( AllSettings::GetMathLayoutRTL() )
            AxisHelper::setRTLAxisLayout( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 ) );
        if( rParameter.b3DLook )
            ThreeDHelper::setScheme( xDiagram, rParameter.eThreeDLookScheme );

        uno::Reference<beans::XPropertySet> xDiaProp(xDiagram, uno::UNO_QUERY);
        if (xDiaProp.is())
        {
            xDiaProp->setPropertyValue(CHART_UNONAME_SORT_BY_XVALUES, uno::Any(rParameter.bSortByXValues));
        }
    }
}
void ChartTypeDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.Clear();
}
bool ChartTypeDialogController::shouldShow_3DLookControl() const
{
    return false;
}
bool ChartTypeDialogController::shouldShow_StackingControl() const
{
    return false;
}
bool ChartTypeDialogController::shouldShow_DeepStackingControl() const
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

bool ChartTypeDialogController::shouldShow_GL3DResourceGroup() const
{
    return false;
}

void ChartTypeDialogController::showExtraControls( VclBuilderContainer* /*pParent*/ )
{
}
void ChartTypeDialogController::hideExtraControls() const
{
}
void ChartTypeDialogController::fillExtraControls( const ChartTypeParameter& /*rParameter*/
                                                  , const uno::Reference< XChartDocument >& /*xChartModel*/
                                                  , const uno::Reference< beans::XPropertySet >& /*xTemplateProps*/ ) const
{
}
void ChartTypeDialogController::setTemplateProperties( const uno::Reference< beans::XPropertySet >& /*xTemplateProps*/ ) const
{
}

ColumnOrBarChartDialogController_Base::ColumnOrBarChartDialogController_Base()
                                        : ChartTypeDialogController()
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
    return SCH_RESSTR( STR_TYPE_COLUMN );
}

Image ColumnChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_COLUMN)));
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
void ColumnChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.Clear();

    if( rParameter.b3DLook )
    {
        switch(rParameter.nGeometry3D)
        {
            case DataPointGeometry3D::CYLINDER:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_SAEULE_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_SAEULE_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_SAEULE_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_SAEULE_3D_4))));
            break;
            case DataPointGeometry3D::CONE:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_KEGEL_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_KEGEL_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_KEGEL_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_KEGEL_3D_4))));
            break;
            case DataPointGeometry3D::PYRAMID:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_PYRAMID_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_PYRAMID_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_PYRAMID_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_PYRAMID_3D_4))));
            break;
            default: //DataPointGeometry3D::CUBOID:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_COLUMNS_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_COLUMNS_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_COLUMNS_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_COLUMNS_3D))));
            break;
        }
    }
    else
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_COLUMNS_2D_1))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_COLUMNS_2D_2))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_COLUMNS_2D_3))));
    }

    rSubTypeList.SetItemText( 1, SCH_RESSTR( STR_NORMAL ) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR( STR_STACKED ) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR( STR_PERCENT ) );
    rSubTypeList.SetItemText( 4, SCH_RESSTR( STR_DEEP ) );
}

BarChartDialogController::BarChartDialogController()
{
}

BarChartDialogController::~BarChartDialogController()
{
}

OUString BarChartDialogController::getName()
{
    return SCH_RESSTR( STR_TYPE_BAR );
}

Image BarChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_BAR)));
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
void BarChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.Clear();

    if( rParameter.b3DLook )
    {
        switch(rParameter.nGeometry3D)
        {
            case DataPointGeometry3D::CYLINDER:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_ROEHRE_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_ROEHRE_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_ROEHRE_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_ROEHRE_3D_4))));
            break;
            case DataPointGeometry3D::CONE:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_KEGELQ_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_KEGELQ_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_KEGELQ_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_KEGELQ_3D_4))));
            break;
            case DataPointGeometry3D::PYRAMID:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_PYRAMIDQ_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_PYRAMIDQ_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_PYRAMIDQ_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_PYRAMIDQ_3D_4))));
            break;
            default: //DataPointGeometry3D::CUBOID:
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_BARS_3D_1))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_BARS_3D_2))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_BARS_3D_3))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_BARS_3D))));
            break;
        }
    }
    else
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_BARS_2D_1))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_BARS_2D_2))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_BARS_2D_3))));
    }
    rSubTypeList.SetItemText( 1, SCH_RESSTR( STR_NORMAL ) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR( STR_STACKED ) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR( STR_PERCENT ) );
    rSubTypeList.SetItemText( 4, SCH_RESSTR( STR_DEEP ) );
}

PieChartDialogController::PieChartDialogController()
{
}
PieChartDialogController::~PieChartDialogController()
{
}
OUString PieChartDialogController::getName()
{
    return SCH_RESSTR( STR_TYPE_PIE );
}
Image PieChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_PIE)));
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
void PieChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.Clear();

    if( rParameter.b3DLook )
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_CIRCLES_3D))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_CIRCLES_3D_EXPLODED))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_DONUT_3D))));
        rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_DONUT_3D_EXPLODED))));
    }
    else
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_CIRCLES_2D))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_CIRCLES_2D_EXPLODED))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_DONUT_2D))));
        rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_DONUT_2D_EXPLODED))));
    }
    rSubTypeList.SetItemText( 1, SCH_RESSTR( STR_NORMAL         ) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR( STR_PIE_EXPLODED   ) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR( STR_DONUT          ) );
    rSubTypeList.SetItemText( 4, SCH_RESSTR( STR_DONUT_EXPLODED ) );
}

bool PieChartDialogController::shouldShow_3DLookControl() const
{
    return true;
}
void PieChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    if(rParameter.eStackMode==GlobalStackMode_STACK_Z)
        rParameter.eStackMode = GlobalStackMode_NONE;
}
LineChartDialogController::LineChartDialogController()
{
}
LineChartDialogController::~LineChartDialogController()
{
}
OUString LineChartDialogController::getName()
{
    return SCH_RESSTR( STR_TYPE_LINE );
}

Image LineChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_LINE)));
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
void LineChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.Clear();

    switch( rParameter.eCurveStyle )
    {
        case CurveStyle_CUBIC_SPLINES:
        case CurveStyle_B_SPLINES:
            if( GlobalStackMode_NONE == rParameter.eStackMode || GlobalStackMode_STACK_Z == rParameter.eStackMode )
            {
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_XCATEGORY))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_XCATEGORY_SMOOTH))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_XCATEGORY_SMOOTH))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_XCATEGORY_SMOOTH))));
            }
            else
            {
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_STACKED))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_STACKED_SMOOTH))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_STACKED_SMOOTH))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_STACKED_SMOOTH))));
            }
            break;
        case CurveStyle_STEP_START:
        case CurveStyle_STEP_END:
        case CurveStyle_STEP_CENTER_X:
        case CurveStyle_STEP_CENTER_Y:
            if( GlobalStackMode_NONE == rParameter.eStackMode || GlobalStackMode_STACK_Z == rParameter.eStackMode )
            {
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_XCATEGORY))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_XCATEGORY_STEPPED))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_XCATEGORY_STEPPED))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_XCATEGORY_STEPPED))));
            }
            else
            {
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_STACKED))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_STACKED_STEPPED))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_STACKED_STEPPED))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_STACKED_STEPPED))));
            }
            break;
        default: // includes CurveStyle_LINES
            //direct lines
            if( GlobalStackMode_NONE == rParameter.eStackMode || GlobalStackMode_STACK_Z == rParameter.eStackMode )
            {
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_XCATEGORY))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_XCATEGORY))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_XCATEGORY))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_XCATEGORY))));
            }
            else
            {
                rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_STACKED))));
                rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_STACKED))));
                rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_STACKED))));
                rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_STACKED))));
            }
    }

    rSubTypeList.SetItemText( 1, SCH_RESSTR( STR_POINTS_ONLY ) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR( STR_POINTS_AND_LINES ) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR( STR_LINES_ONLY ) );
    rSubTypeList.SetItemText( 4, SCH_RESSTR( STR_LINES_3D ) );
}
bool LineChartDialogController::shouldShow_StackingControl() const
{
    return true;
}
bool LineChartDialogController::shouldShow_DeepStackingControl() const
{
    return false;
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
    return SCH_RESSTR( STR_TYPE_XY );
}

Image XYChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_XY)));
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

void XYChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.Clear();

    switch (rParameter.eCurveStyle)
    {
        case CurveStyle_CUBIC_SPLINES:
        case CurveStyle_B_SPLINES:
        {
            rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_XVALUES))));
            rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_XVALUES_SMOOTH))));
            rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_XVALUES_SMOOTH))));
            rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_XVALUES_SMOOTH))));
            break;
        }
        case CurveStyle_STEP_START:
        case CurveStyle_STEP_END:
        case CurveStyle_STEP_CENTER_X:
        case CurveStyle_STEP_CENTER_Y:
        {
            rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_XVALUES))));
            rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_XVALUES_STEPPED))));
            rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_XVALUES_STEPPED))));
            rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_XVALUES_STEPPED))));
            break;
        }
        default: // includes CurveStyle_LINES
            rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_POINTS_XVALUES))));
            rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_LINE_P_XVALUES))));
            rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_LINE_O_XVALUES))));
            rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_LINE3D_XVALUES))));
    }

    rSubTypeList.SetItemText( 1, SCH_RESSTR( STR_POINTS_ONLY ) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR( STR_POINTS_AND_LINES ) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR( STR_LINES_ONLY ) );
    rSubTypeList.SetItemText( 4, SCH_RESSTR( STR_LINES_3D ) );
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
    return SCH_RESSTR(STR_TYPE_AREA);
}

Image AreaChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_AREA)));
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

void AreaChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.Clear();

    if( rParameter.b3DLook )
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_AREAS_3D))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_AREAS_3D_1))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_AREAS_3D_2))));
    }
    else
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_AREAS_2D_1))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_AREAS_2D))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_AREAS_2D_3))));
    }

    rSubTypeList.SetItemText( 1, SCH_RESSTR( rParameter.b3DLook ? STR_DEEP : STR_NORMAL ) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR( STR_STACKED ) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR( STR_PERCENT ) );
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
    return SCH_RESSTR(STR_TYPE_NET);
}

Image NetChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_NET)));
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
void NetChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& rParameter )
{
    rSubTypeList.Clear();

    if( GlobalStackMode_NONE == rParameter.eStackMode )
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_NET_SYMB))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_NET_LINESYMB))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_NET))));
        rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_NET_FILL))));
    }
    else
    {
        rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_NET_SYMB_STACK))));
        rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_NET_LINESYMB_STACK))));
        rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_NET_STACK))));
        rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_NET_FILL_STACK))));
    }

    rSubTypeList.SetItemText( 1, SCH_RESSTR( STR_POINTS_ONLY ) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR( STR_POINTS_AND_LINES ) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR( STR_LINES_ONLY ) );
    rSubTypeList.SetItemText( 4, SCH_RESSTR( STR_FILLED ) );
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
    return SCH_RESSTR(STR_TYPE_STOCK);
}

Image StockChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_STOCK)));
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

void StockChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.Clear();
    rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_STOCK_1))));
    rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_STOCK_2))));
    rSubTypeList.InsertItem(3, Image(Bitmap(SchResId(BMP_STOCK_3))));
    rSubTypeList.InsertItem(4, Image(Bitmap(SchResId(BMP_STOCK_4))));

    rSubTypeList.SetItemText( 1, SCH_RESSTR(STR_STOCK_1) );
    rSubTypeList.SetItemText( 2, SCH_RESSTR(STR_STOCK_2) );
    rSubTypeList.SetItemText( 3, SCH_RESSTR(STR_STOCK_3) );
    rSubTypeList.SetItemText( 4, SCH_RESSTR(STR_STOCK_4) );
}

void StockChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = false;
    rParameter.eStackMode = GlobalStackMode_NONE;
}

CombiColumnLineChartDialogController::CombiColumnLineChartDialogController()
    : m_pFT_NumberOfLines(nullptr)
    , m_pMF_NumberOfLines(nullptr)
{
    bSupports3D = false;
}

OUString CombiColumnLineChartDialogController::getName()
{
    return SCH_RESSTR(STR_TYPE_COMBI_COLUMN_LINE);
}

Image CombiColumnLineChartDialogController::getImage( )
{
    return Image(BitmapEx(SchResId(BMP_TYPE_COLUMN_LINE)));
}

const tTemplateServiceChartTypeParameterMap& CombiColumnLineChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
    {"com.sun.star.chart2.template.ColumnWithLine" ,             ChartTypeParameter(1)},
    {"com.sun.star.chart2.template.StackedColumnWithLine" ,      ChartTypeParameter(2,false,false,GlobalStackMode_STACK_Y)}};
    return s_aTemplateMap;
}

void CombiColumnLineChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.Clear();
    rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_COLUMN_LINE))));
    rSubTypeList.InsertItem(2, Image(Bitmap(SchResId(BMP_COLUMN_LINE_STACKED))));

    rSubTypeList.SetItemText(1, SCH_RESSTR(STR_LINE_COLUMN));
    rSubTypeList.SetItemText(2, SCH_RESSTR(STR_LINE_STACKEDCOLUMN));
}

void CombiColumnLineChartDialogController::showExtraControls( VclBuilderContainer* pParent )
{
    if (!m_pFT_NumberOfLines)
    {
        pParent->get(m_pFT_NumberOfLines, "nolinesft");
    }
    if (!m_pMF_NumberOfLines)
    {
        pParent->get(m_pMF_NumberOfLines, "nolines");

        m_pMF_NumberOfLines->SetSpinSize(1);
        m_pMF_NumberOfLines->SetFirst( 1 );
        m_pMF_NumberOfLines->SetLast( 100 );
        m_pMF_NumberOfLines->SetMin( 1 );
        m_pMF_NumberOfLines->SetMax( 100 );

        m_pMF_NumberOfLines->SetModifyHdl( LINK( this, CombiColumnLineChartDialogController, ChangeLineCountHdl ) );
    }

    m_pFT_NumberOfLines->Show();
    m_pMF_NumberOfLines->Show();
}

void CombiColumnLineChartDialogController::hideExtraControls() const
{
    if(m_pFT_NumberOfLines)
        m_pFT_NumberOfLines->Hide();
    if(m_pMF_NumberOfLines)
        m_pMF_NumberOfLines->Hide();
}

void CombiColumnLineChartDialogController::fillExtraControls( const ChartTypeParameter& /*rParameter*/
                , const uno::Reference< XChartDocument >& xChartModel
                , const uno::Reference< beans::XPropertySet >& xTemplateProps ) const
{
    if(!m_pMF_NumberOfLines)
        return;

    uno::Reference< frame::XModel > xModel( xChartModel, uno::UNO_QUERY );

    uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xModel );
    if(!xDiagram.is())
        return;

    sal_Int32 nNumLines = 0;

    if(xTemplateProps.is())
    {
        try
        {
            xTemplateProps->getPropertyValue( "NumberOfLines" ) >>= nNumLines;
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    if( nNumLines < 0 )
        nNumLines = 0;
    m_pMF_NumberOfLines->SetValue( nNumLines );

    sal_Int32 nMaxLines = ChartModelHelper::getDataSeries( xModel ).size() - 1;
    if( nMaxLines < 0 )
        nMaxLines = 0;
    m_pMF_NumberOfLines->SetLast( nMaxLines );
    m_pMF_NumberOfLines->SetMax( nMaxLines );
}
void CombiColumnLineChartDialogController::setTemplateProperties( const uno::Reference< beans::XPropertySet >& xTemplateProps ) const
{
    if( xTemplateProps.is() )
    {
        sal_Int32 nNumLines = static_cast< sal_Int32 >( m_pMF_NumberOfLines->GetValue());
        xTemplateProps->setPropertyValue( "NumberOfLines" , uno::Any(nNumLines) );
    }
}

IMPL_LINK_NOARG(CombiColumnLineChartDialogController, ChangeLineCountHdl, Edit&, void)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged(this);
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
    return SCH_RESSTR(STR_TYPE_BUBBLE);
}

Image BubbleChartDialogController::getImage()
{
    return Image(BitmapEx(SchResId(BMP_TYPE_BUBBLE)));
}

const tTemplateServiceChartTypeParameterMap& BubbleChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap s_aTemplateMap{
        {"com.sun.star.chart2.template.Bubble" ,          ChartTypeParameter(1,true)}};
    return s_aTemplateMap;
}
void BubbleChartDialogController::fillSubTypeList( ValueSet& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.Clear();
    rSubTypeList.InsertItem(1, Image(Bitmap(SchResId(BMP_BUBBLE_1))));

    rSubTypeList.SetItemText( 1, SCH_RESSTR(STR_BUBBLE_1) );
}
void BubbleChartDialogController::adjustParameterToSubType( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = false;
    rParameter.eStackMode = GlobalStackMode_NONE;
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
