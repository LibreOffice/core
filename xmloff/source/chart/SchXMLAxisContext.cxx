/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "SchXMLAxisContext.hxx"
#include "SchXMLChartContext.hxx"
#include "SchXMLTools.hxx"
#include "xmlnmspe.hxx"
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>

#include <tools/debug.hxx>

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>

#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <com/sun/star/drawing/LineStyle.hpp>

using namespace ::xmloff::token;
using namespace com::sun::star;

using rtl::OUString;
using com::sun::star::uno::Reference;

//----------------------------------------
//----------------------------------------

static __FAR_DATA SvXMLEnumMapEntry aXMLAxisClassMap[] =
{
    { XML_X,  SCH_XML_AXIS_X  },
    { XML_Y,  SCH_XML_AXIS_Y  },
    { XML_Z,  SCH_XML_AXIS_Z  },
    { XML_TOKEN_INVALID, 0 }
};

//----------------------------------------
//----------------------------------------

class SchXMLCategoriesContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    OUString& mrAddress;

public:
    SchXMLCategoriesContext( SchXMLImportHelper& rImpHelper,
                                   SvXMLImport& rImport,
                                   sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   OUString& rAddress );
    virtual ~SchXMLCategoriesContext();
    virtual void StartElement( const Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

//----------------------------------------
//----------------------------------------

SchXMLAxisContext::SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport, const OUString& rLocalName,
                                      Reference< chart::XDiagram > xDiagram,
                                      std::vector< SchXMLAxis >& aAxes,
                                      OUString & rCategoriesAddress,
                                      bool bAddMissingXAxisForNetCharts,
                                      bool bAdaptWrongPercentScaleValues,
                                      bool bAdaptXAxisOrientationForOld2DBarCharts,
                                      bool& rbAxisPositionAttributeImported ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxDiagram( xDiagram ),
        maAxes( aAxes ),
        mrCategoriesAddress( rCategoriesAddress ),
        mbAddMissingXAxisForNetCharts( bAddMissingXAxisForNetCharts ),
        mbAdaptWrongPercentScaleValues( bAdaptWrongPercentScaleValues ),
        mbAdaptXAxisOrientationForOld2DBarCharts( bAdaptXAxisOrientationForOld2DBarCharts ),
        m_rbAxisPositionAttributeImported( rbAxisPositionAttributeImported )
{
}

SchXMLAxisContext::~SchXMLAxisContext()
{}

/* returns a shape for the current axis's title. The property
   "Has...AxisTitle" is set to "True" to get the shape
 */
Reference< drawing::XShape > SchXMLAxisContext::getTitleShape()
{
    Reference< drawing::XShape > xResult;
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);
    Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( xDiaProp.is())
                        xDiaProp->setPropertyValue( OUString::createFromAscii( "HasXAxisTitle" ), aTrueBool );
                    xResult = Reference< drawing::XShape >( xSuppl->getXAxisTitle(), uno::UNO_QUERY );
                }
            }
            else
            {
                Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() )
                {
                    if( xDiaProp.is() )
                        xDiaProp->setPropertyValue( OUString::createFromAscii( "HasSecondaryXAxisTitle" ), aTrueBool );
                    xResult = Reference< drawing::XShape >( xSuppl->getSecondXAxisTitle(), uno::UNO_QUERY );
                }
            }
            break;
        case SCH_XML_AXIS_Y:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( xDiaProp.is())
                        xDiaProp->setPropertyValue( OUString::createFromAscii( "HasYAxisTitle" ), aTrueBool );
                    xResult = Reference< drawing::XShape >( xSuppl->getYAxisTitle(), uno::UNO_QUERY );
                }
            }
            else
            {
                Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() )
                {
                    if( xDiaProp.is() )
                        xDiaProp->setPropertyValue( OUString::createFromAscii( "HasSecondaryYAxisTitle" ), aTrueBool );
                    xResult = Reference< drawing::XShape >( xSuppl->getSecondYAxisTitle(), uno::UNO_QUERY );
                }
            }
            break;
        case SCH_XML_AXIS_Z:
        {
            Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
            if( xSuppl.is())
            {
                if( xDiaProp.is())
                    xDiaProp->setPropertyValue( OUString::createFromAscii( "HasZAxisTitle" ), aTrueBool );
                xResult = Reference< drawing::XShape >( xSuppl->getZAxisTitle(), uno::UNO_QUERY );
            }
            break;
        }
        case SCH_XML_AXIS_UNDEF:
            DBG_ERROR( "Invalid axis" );
            break;
    }

    return xResult;
}

void SchXMLAxisContext::CreateGrid( OUString sAutoStyleName,
                                    sal_Bool bIsMajor )
{
    Reference< chart::XDiagram > xDia = mrImportHelper.GetChartDocument()->getDiagram();
    Reference< beans::XPropertySet > xGridProp;
    OUString sPropertyName;
    DBG_ASSERT( xDia.is(), "diagram object is invalid!" );

    Reference< beans::XPropertySet > xDiaProp( xDia, uno::UNO_QUERY );
    uno::Any aTrueBool( uno::makeAny( true ));

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            {
                Reference< chart::XAxisXSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( OUString::createFromAscii("HasXAxisGrid"), aTrueBool );
                        xGridProp = xSuppl->getXMainGrid();
                    }
                    else
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( OUString::createFromAscii("HasXAxisHelpGrid"), aTrueBool );
                        xGridProp = xSuppl->getXHelpGrid();
                    }
                }
            }
            break;
        case SCH_XML_AXIS_Y:
            {
                Reference< chart::XAxisYSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( OUString::createFromAscii("HasYAxisGrid"), aTrueBool );
                        xGridProp = xSuppl->getYMainGrid();
                    }
                    else
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( OUString::createFromAscii("HasYAxisHelpGrid"), aTrueBool );
                        xGridProp = xSuppl->getYHelpGrid();
                    }
                }
            }
            break;
        case SCH_XML_AXIS_Z:
            {
                Reference< chart::XAxisZSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( OUString::createFromAscii("HasZAxisGrid"), aTrueBool );
                        xGridProp = xSuppl->getZMainGrid();
                    }
                    else
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( OUString::createFromAscii("HasZAxisHelpGrid"), aTrueBool );
                        xGridProp = xSuppl->getZHelpGrid();
                    }
                }
            }
            break;
        case SCH_XML_AXIS_UNDEF:
            DBG_ERROR( "Invalid axis" );
            break;
    }

    // set properties
    if( xGridProp.is())
    {
        // the line color is black as default, in the model it is a light gray
        xGridProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LineColor" )),
                                     uno::makeAny( COL_BLACK ));
        if( sAutoStyleName.getLength())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), sAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                    (( XMLPropStyleContext* )pStyle )->FillPropertySet( xGridProp );
            }
        }
    }
}

void SchXMLAxisContext::StartElement( const Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetAxisAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_AXIS_DIMENSION:
                {
                    USHORT nEnumVal;
                    if( rImport.GetMM100UnitConverter().convertEnum( nEnumVal, aValue, aXMLAxisClassMap ))
                        maCurrentAxis.eClass = ( SchXMLAxisClass )nEnumVal;
                }
                break;
            case XML_TOK_AXIS_NAME:
                maCurrentAxis.aName = aValue;
                break;
            case XML_TOK_AXIS_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
        }
    }

    // check for number of axes with same category
    maCurrentAxis.nIndexInCategory = 0;
    sal_Int32 nNumOfAxes = maAxes.size();
    for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
    {
        if( maAxes[ nCurrent ].eClass == maCurrentAxis.eClass )
            maCurrentAxis.nIndexInCategory++;
    }
    CreateAxis();
}
namespace
{

Reference< chart2::XAxis > lcl_getAxis( const Reference< frame::XModel >& xChartModel,
                                            sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    Reference< chart2::XAxis > xAxis;

    try
    {
        Reference< chart2::XChartDocument > xChart2Document( xChartModel, uno::UNO_QUERY );
        if( xChart2Document.is() )
        {
            Reference< chart2::XDiagram > xDiagram( xChart2Document->getFirstDiagram());
            Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
            uno::Sequence< Reference< chart2::XCoordinateSystem > >
                aCooSysSeq( xCooSysCnt->getCoordinateSystems());
            sal_Int32 nCooSysIndex = 0;
            if( nCooSysIndex < aCooSysSeq.getLength() )
            {
                Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[nCooSysIndex] );
                if( xCooSys.is() && nDimensionIndex < xCooSys->getDimension() )
                {
                    const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                    if( nAxisIndex <= nMaxAxisIndex )
                        xAxis = xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex );
                }
            }
        }
    }
    catch( uno::Exception & )
    {
        DBG_ERROR( "Couldn't get axis" );
    }

    return xAxis;
}

bool lcl_divideBy100( uno::Any& rDoubleAny )
{
    bool bChanged = false;
    double fValue=0.0;
    if( (rDoubleAny>>=fValue) && (fValue!=0.0) )
    {
        fValue/=100.0;
        rDoubleAny = uno::makeAny(fValue);
        bChanged = true;
    }
    return bChanged;
}

bool lcl_AdaptWrongPercentScaleValues(chart2::ScaleData& rScaleData)
{
    bool bChanged = lcl_divideBy100( rScaleData.Minimum );
    bChanged = lcl_divideBy100( rScaleData.Maximum ) || bChanged;
    bChanged = lcl_divideBy100( rScaleData.Origin ) || bChanged;
    bChanged = lcl_divideBy100( rScaleData.IncrementData.Distance ) || bChanged;
    return bChanged;
}

}//end anonymous namespace

void SchXMLAxisContext::CreateAxis()
{
    // add new Axis to list
    maAxes.push_back( maCurrentAxis );

    // set axis at chart
    Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );
    Reference< beans::XPropertySet > xProp;
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);
    uno::Any aFalseBool;
    aFalseBool <<= (sal_Bool)(sal_False);
    Reference< frame::XModel > xDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        OUString::createFromAscii( "HasXAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on x axis" );
                }
                Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getXAxis();
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        OUString::createFromAscii( "HasSecondaryXAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on second x axis" );
                }
                Reference< chart::XTwoAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryXAxis();
            }
            break;

        case SCH_XML_AXIS_Y:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        OUString::createFromAscii( "HasYAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on y axis" );
                }
                Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getYAxis();


                if( mbAddMissingXAxisForNetCharts )
                {
                    if( xDiaProp.is() )
                    {
                        try
                        {
                            xDiaProp->setPropertyValue(
                                OUString::createFromAscii( "HasXAxis" ), uno::makeAny(sal_True) );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Couldn't turn on x axis" );
                        }
                    }
                }
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        OUString::createFromAscii( "HasSecondaryYAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on second y axis" );
                }
                Reference< chart::XTwoAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryYAxis();
            }
            break;

        case SCH_XML_AXIS_Z:
            {
                bool bSettingZAxisSuccedded = false;
                try
                {
                    OUString sHasZAxis( OUString::createFromAscii( "HasZAxis" ) );
                    xDiaProp->setPropertyValue( sHasZAxis, aTrueBool );
                    xDiaProp->getPropertyValue( sHasZAxis ) >>= bSettingZAxisSuccedded;
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on z axis" );
                }
                if( bSettingZAxisSuccedded )
                {
                    Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                    if( xSuppl.is())
                        xProp = xSuppl->getZAxis();
                }
            }
            break;
        case SCH_XML_AXIS_UNDEF:
            // nothing
            break;
    }

    // set properties
    if( xProp.is())
    {
        // #i109879# the line color is black as default, in the model it is a light gray
        xProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "LineColor" )),
                                     uno::makeAny( COL_BLACK ));

        xProp->setPropertyValue( OUString::createFromAscii( "DisplayLabels" ), aFalseBool );

        // #88077# AutoOrigin 'on' is default
        xProp->setPropertyValue( OUString::createFromAscii( "AutoOrigin" ), aTrueBool );

        if( msAutoStyleName.getLength())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), msAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                {
                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext = const_cast< XMLPropStyleContext * >( dynamic_cast< const XMLPropStyleContext * >( pStyle ));
                    if( pPropStyleContext )
                        pPropStyleContext->FillPropertySet( xProp );

                    if( mbAdaptWrongPercentScaleValues && maCurrentAxis.eClass==SCH_XML_AXIS_Y )
                    {
                        //set scale data of added x axis back to default
                        Reference< chart2::XAxis > xAxis( lcl_getAxis( GetImport().GetModel(),
                                            1 /*nDimensionIndex*/, maCurrentAxis.nIndexInCategory /*nAxisIndex*/ ) );
                        if( xAxis.is() )
                        {
                            chart2::ScaleData aScaleData( xAxis->getScaleData());
                            if( lcl_AdaptWrongPercentScaleValues(aScaleData) )
                                xAxis->setScaleData( aScaleData );
                        }
                    }

                    if( mbAddMissingXAxisForNetCharts )
                    {
                        //copy style from y axis to added x axis:

                        Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                        if( xSuppl.is() )
                        {
                            Reference< beans::XPropertySet > xXAxisProp( xSuppl->getXAxis() );
                            (( XMLPropStyleContext* )pStyle )->FillPropertySet( xXAxisProp );
                        }

                        //set scale data of added x axis back to default
                        Reference< chart2::XAxis > xAxis( lcl_getAxis( GetImport().GetModel(),
                                            0 /*nDimensionIndex*/, 0 /*nAxisIndex*/ ) );
                        if( xAxis.is() )
                        {
                            chart2::ScaleData aScaleData;
                            aScaleData.AxisType = chart2::AxisType::CATEGORY;
                            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                            xAxis->setScaleData( aScaleData );
                        }

                        //set line style of added x axis to invisible
                        Reference< beans::XPropertySet > xNewAxisProp( xAxis, uno::UNO_QUERY );
                        if( xNewAxisProp.is() )
                        {
                            xNewAxisProp->setPropertyValue( OUString::createFromAscii("LineStyle")
                                , uno::makeAny(drawing::LineStyle_NONE));
                        }
                    }

                    if( mbAdaptXAxisOrientationForOld2DBarCharts && maCurrentAxis.eClass == SCH_XML_AXIS_X )
                    {
                        bool bIs3DChart = false;
                        if( xDiaProp.is() && ( xDiaProp->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Dim3D"))) >>= bIs3DChart )
                            && !bIs3DChart )
                        {
                            Reference< chart2::XChartDocument > xChart2Document( GetImport().GetModel(), uno::UNO_QUERY );
                            if( xChart2Document.is() )
                            {
                                Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xChart2Document->getFirstDiagram(), uno::UNO_QUERY );
                                if( xCooSysCnt.is() )
                                {
                                    uno::Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems() );
                                    if( aCooSysSeq.getLength() )
                                    {
                                        bool bSwapXandYAxis = false;
                                        Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[0] );
                                        Reference< beans::XPropertySet > xCooSysProp( xCooSys, uno::UNO_QUERY );
                                        if( xCooSysProp.is() && ( xCooSysProp->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("SwapXAndYAxis"))) >>= bSwapXandYAxis )
                                            && bSwapXandYAxis )
                                        {
                                            Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension( 0, maCurrentAxis.nIndexInCategory );
                                            if( xAxis.is() )
                                            {
                                                chart2::ScaleData aScaleData = xAxis->getScaleData();
                                                aScaleData.Orientation = chart2::AxisOrientation_REVERSE;
                                                xAxis->setScaleData( aScaleData );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    m_rbAxisPositionAttributeImported = m_rbAxisPositionAttributeImported || SchXMLTools::getPropertyFromContext(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("CrossoverPosition")), pPropStyleContext, pStylesCtxt ).hasValue();
                }
            }
        }
    }
}

void SchXMLAxisContext::SetAxisTitle()
{
    // add new Axis to list
    maAxes.push_back( maCurrentAxis );

    // set axis at chart
    sal_Bool bHasTitle = ( maCurrentAxis.aTitle.getLength() > 0 );
    Reference< frame::XModel > xDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    Reference< beans::XPropertySet > xTitleProp( xSuppl->getXAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            else
            {
                Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    Reference< beans::XPropertySet > xTitleProp( xSuppl->getSecondXAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            break;

        case SCH_XML_AXIS_Y:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    Reference< beans::XPropertySet > xTitleProp( xSuppl->getYAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                                DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            else
            {
                Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    Reference< beans::XPropertySet > xTitleProp( xSuppl->getSecondYAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            break;

        case SCH_XML_AXIS_Z:
            {
                Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    Reference< beans::XPropertySet > xTitleProp( xSuppl->getZAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            break;
        case SCH_XML_AXIS_UNDEF:
            // nothing
            break;
    }
}

SvXMLImportContext* SchXMLAxisContext::CreateChildContext(
    USHORT p_nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetAxisElemTokenMap();

    switch( rTokenMap.Get( p_nPrefix, rLocalName ))
    {
        case XML_TOK_AXIS_TITLE:
        {
            Reference< drawing::XShape > xTitleShape = getTitleShape();
            pContext = new SchXMLTitleContext( mrImportHelper, GetImport(), rLocalName,
                                               maCurrentAxis.aTitle,
                                               xTitleShape );
        }
        break;

        case XML_TOK_AXIS_CATEGORIES:
            pContext = new SchXMLCategoriesContext( mrImportHelper, GetImport(),
                                                          p_nPrefix, rLocalName,
                                                          mrCategoriesAddress );
            maCurrentAxis.bHasCategories = true;
            break;

        case XML_TOK_AXIS_GRID:
        {
            sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
            sal_Bool bIsMajor = sal_True;       // default value for class is "major"
            OUString sAutoStyleName;

            for( sal_Int16 i = 0; i < nAttrCount; i++ )
            {
                OUString sAttrName = xAttrList->getNameByIndex( i );
                OUString aLocalName;
                USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

                if( nPrefix == XML_NAMESPACE_CHART )
                {
                    if( IsXMLToken( aLocalName, XML_CLASS ) )
                    {
                        if( IsXMLToken( xAttrList->getValueByIndex( i ), XML_MINOR ) )
                            bIsMajor = sal_False;
                    }
                    else if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                        sAutoStyleName = xAttrList->getValueByIndex( i );
                }
            }

            CreateGrid( sAutoStyleName, bIsMajor );

            // don't create a context => use default context. grid elements are empty
            pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
        }
        break;

        default:
            pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
            break;
    }

    return pContext;
}

void SchXMLAxisContext::EndElement()
{
    SetAxisTitle();
}

// ========================================

namespace
{

Reference< chart2::XAxis > lcl_getAxis( const Reference< chart2::XCoordinateSystem > xCooSys, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    Reference< chart2::XAxis > xAxis;
    try
    {
        xAxis = xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex );
    }
    catch( uno::Exception & )
    {
    }
    return xAxis;
}

} // anonymous namespace

void SchXMLAxisContext::CorrectAxisPositions( const Reference< chart2::XChartDocument >& xNewDoc,
                          const OUString& rChartTypeServiceName,
                          const OUString& rODFVersionOfFile,
                          bool bAxisPositionAttributeImported )
{
    if( ( !rODFVersionOfFile.getLength() || rODFVersionOfFile.equalsAscii("1.0")
        || rODFVersionOfFile.equalsAscii("1.1")
        || ( rODFVersionOfFile.equalsAscii("1.2") && !bAxisPositionAttributeImported ) ) )
    {
        try
        {
            Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xNewDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
            uno::Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
            if( aCooSysSeq.getLength() )
            {
                Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[0] );
                if( xCooSys.is() )
                {
                    Reference< chart2::XAxis > xMainXAxis = lcl_getAxis( xCooSys, 0, 0 );
                    Reference< chart2::XAxis > xMainYAxis = lcl_getAxis( xCooSys, 1, 0 );
                    //Reference< chart2::XAxis > xMajorZAxis = lcl_getAxis( xCooSys, 2, 0 );
                    Reference< chart2::XAxis > xSecondaryXAxis = lcl_getAxis( xCooSys, 0, 1 );
                    Reference< chart2::XAxis > xSecondaryYAxis = lcl_getAxis( xCooSys, 1, 1 );

                    Reference< beans::XPropertySet > xMainXAxisProp( xMainXAxis, uno::UNO_QUERY );
                    Reference< beans::XPropertySet > xMainYAxisProp( xMainYAxis, uno::UNO_QUERY );
                    Reference< beans::XPropertySet > xSecondaryXAxisProp( xSecondaryXAxis, uno::UNO_QUERY );
                    Reference< beans::XPropertySet > xSecondaryYAxisProp( xSecondaryYAxis, uno::UNO_QUERY );

                    if( xMainXAxisProp.is() && xMainYAxisProp.is() )
                    {
                        chart2::ScaleData aMainXScale = xMainXAxis->getScaleData();
                        if( 0 == rChartTypeServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.ScatterChartType" ) ) )
                        {
                            xMainYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_VALUE) );
                            double fCrossoverValue = 0.0;
                            aMainXScale.Origin >>= fCrossoverValue;
                            xMainYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverValue")
                                    , uno::makeAny( fCrossoverValue ) );

                            if( aMainXScale.Orientation == chart2::AxisOrientation_REVERSE )
                            {
                                xMainYAxisProp->setPropertyValue( OUString::createFromAscii("LabelPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END) );
                                xMainYAxisProp->setPropertyValue( OUString::createFromAscii("MarkPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                            }
                            else
                            {
                                xMainYAxisProp->setPropertyValue( OUString::createFromAscii("LabelPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_START) );
                                xMainYAxisProp->setPropertyValue( OUString::createFromAscii("MarkPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                            }
                        }
                        else
                        {
                            if( aMainXScale.Orientation == chart2::AxisOrientation_REVERSE )
                            {
                                xMainYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                        , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                            }
                            else
                            {
                                xMainYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                        , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                            }
                        }

                        chart2::ScaleData aMainYScale = xMainYAxis->getScaleData();
                        xMainXAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_VALUE) );
                        double fCrossoverValue = 0.0;
                        aMainYScale.Origin >>= fCrossoverValue;
                        xMainXAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverValue")
                                , uno::makeAny( fCrossoverValue ) );

                        if( aMainYScale.Orientation == chart2::AxisOrientation_REVERSE )
                        {
                            xMainXAxisProp->setPropertyValue( OUString::createFromAscii("LabelPosition")
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END) );
                            xMainXAxisProp->setPropertyValue( OUString::createFromAscii("MarkPosition")
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                            if( xSecondaryXAxisProp.is() )
                                xSecondaryXAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                        }
                        else
                        {
                            xMainXAxisProp->setPropertyValue( OUString::createFromAscii("LabelPosition")
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_START) );
                            xMainXAxisProp->setPropertyValue( OUString::createFromAscii("MarkPosition")
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                            if( xSecondaryXAxisProp.is() )
                                xSecondaryXAxisProp->setPropertyValue( OUString::createFromAscii("CrossoverPosition")
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                        }
                    }
                }
            }
        }
        catch( uno::Exception & )
        {
        }
    }
}

// ========================================

SchXMLCategoriesContext::SchXMLCategoriesContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    OUString& rAddress ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrAddress( rAddress )
{
}

SchXMLCategoriesContext::~SchXMLCategoriesContext()
{
}

void SchXMLCategoriesContext::StartElement( const Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( aLocalName, XML_CELL_RANGE_ADDRESS ) )
        {
            Reference< chart2::XChartDocument > xNewDoc( GetImport().GetModel(), uno::UNO_QUERY );
            mrAddress = xAttrList->getValueByIndex( i );
        }
    }
}

// ========================================
