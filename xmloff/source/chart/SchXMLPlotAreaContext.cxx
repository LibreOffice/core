/*************************************************************************
 *
 *  $RCSfile: SchXMLPlotAreaContext.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: bm $ $Date: 2001-03-28 11:51:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "SchXMLChartContext.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLImport.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif
#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISXSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISYSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XAXISZSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGMAPPING_HPP_
#include <com/sun/star/util/XStringMapping.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISASSIGN_HPP_
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_X3DDISPLAY_HPP_
#include <com/sun/star/chart/X3DDisplay.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

using namespace com::sun::star;

static __FAR_DATA SvXMLEnumMapEntry aXMLAxisClassMap[] =
{
    { sXML_category,    SCH_XML_AXIS_CATEGORY   },
    { sXML_domain,      SCH_XML_AXIS_DOMAIN     },
    { sXML_value,       SCH_XML_AXIS_VALUE      },
    { sXML_series,      SCH_XML_AXIS_SERIES     },
    { 0, 0 }
};

SchXMLPlotAreaContext::SchXMLPlotAreaContext( SchXMLImportHelper& rImpHelper,
                                              SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                              uno::Sequence< chart::ChartSeriesAddress >& rSeriesAddresses,
                                              rtl::OUString& rCategoriesAddress ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrSeriesAddresses( rSeriesAddresses ),
        mrCategoriesAddress( rCategoriesAddress ),
        mnDomainOffset( 0 ),
        mnSeries( 0 ),
        mnMaxSeriesLength( 0 ),
        maSceneImportHelper( rImport )
{
    // get Diagram
    uno::Reference< chart::XChartDocument > xDoc( rImpHelper.GetChartDocument(), uno::UNO_QUERY );
    if( xDoc.is())
    {
        mxDiagram = xDoc->getDiagram();
    }
    DBG_ASSERT( mxDiagram.is(), "Couldn't get XDiagram" );

    // turn off all axes initially
    uno::Any aFalseBool;
    aFalseBool <<= (sal_Bool)(sal_False);

    uno::Reference< lang::XServiceInfo > xInfo( mxDiagram, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
    if( xInfo.is() &&
        xProp.is())
    {
        try
        {
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisXSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxisGrid" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxisDescription" ), aFalseBool );
            }
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisXSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryXAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryXAxisDescription" ), aFalseBool );
            }

            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisYSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxisGrid" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxisDescription" ), aFalseBool );
            }
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisYSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryYAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryYAxisDescription" ), aFalseBool );
            }

            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisZSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasZAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasZAxisDescription" ), aFalseBool );
            }
            // data from now on only comes out of columns
            uno::Any aAny;
            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            aAny <<= eSource;
            xProp->setPropertyValue( rtl::OUString::createFromAscii( "DataRowSource" ), aAny );
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_ERROR( "Property required by service not supported" );
        }
    }
}

SchXMLPlotAreaContext::~SchXMLPlotAreaContext()
{}

SvXMLImportContext* SchXMLPlotAreaContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetPlotAreaElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_PA_AXIS:
            pContext = new SchXMLAxisContext( mrImportHelper, GetImport(), rLocalName, mxDiagram, maAxes );
            break;

        case XML_TOK_PA_SERIES:
            {
                mrSeriesAddresses.realloc( mnSeries + 1 );
                pContext = new SchXMLSeriesContext( mrImportHelper, GetImport(), rLocalName,
                                                    mxDiagram, maAxes, mrSeriesAddresses[ mnSeries ],
                                                    maSeriesStyleList,
                                                    mnSeries, mnMaxSeriesLength, mnDomainOffset );
                mnSeries++;
            }
            break;

        case XML_TOK_PA_CATEGORIES:
            pContext = new SchXMLCategoriesDomainContext( mrImportHelper, GetImport(),
                                                          nPrefix, rLocalName,
                                                          mrCategoriesAddress );
            break;

        case XML_TOK_PA_WALL:
            pContext = new SchXMLWallFloorContext( mrImportHelper, GetImport(), nPrefix, rLocalName, mxDiagram,
                                                   SchXMLWallFloorContext::CONTEXT_TYPE_WALL );
            break;
        case XML_TOK_PA_FLOOR:
            pContext = new SchXMLWallFloorContext( mrImportHelper, GetImport(), nPrefix, rLocalName, mxDiagram,
                                                   SchXMLWallFloorContext::CONTEXT_TYPE_FLOOR );
            break;
        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SchXMLPlotAreaContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    awt::Size aSize;
    awt::Point aPosition;
    uno::Any aTransMatrixAny;

    // initialize size and position
    uno::Reference< drawing::XShape > xDiaShape( mxDiagram, uno::UNO_QUERY );
    if( xDiaShape.is())
    {
        aSize = xDiaShape->getSize();
        aPosition = xDiaShape->getPosition();
    }
    rtl::OUString sAutoStyleName;

    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetPlotAreaAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_PA_X:
                GetImport().GetMM100UnitConverter().convertMeasure( aPosition.X, aValue );
                break;
            case XML_TOK_PA_Y:
                GetImport().GetMM100UnitConverter().convertMeasure( aPosition.Y, aValue );
                break;
            case XML_TOK_PA_WIDTH:
                GetImport().GetMM100UnitConverter().convertMeasure( aSize.Width, aValue );
                break;
            case XML_TOK_PA_HEIGHT:
                GetImport().GetMM100UnitConverter().convertMeasure( aSize.Height, aValue );
                break;
            case XML_TOK_PA_STYLE_NAME:
                sAutoStyleName = aValue;
                break;
            case XML_TOK_PA_TRANSFORM:
                {
                    SdXMLImExTransform3D aTransform( aValue, GetImport().GetMM100UnitConverter());
                    if( aTransform.NeedsAction())
                    {
                        drawing::HomogenMatrix xHomMat;
                        if( aTransform.GetFullHomogenTransform( xHomMat ))
                            aTransMatrixAny <<= xHomMat;
                    }
                    break;
                }
        }
    }

    // set properties
    uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
    if( sAutoStyleName.getLength())
    {
        if( xProp.is())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), sAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                    (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
            }
        }
    }

    // set rotation - before size!
    if( xProp.is() &&
        aTransMatrixAny.hasValue())
    {
        // set 3d flag before, so that property is available
        // if chart is 2d with transform matrix the flag will be
        // reset in the FillPropertySet method
        try
        {
            uno::Any aTrueAny;
            aTrueAny <<= (sal_Bool)(sal_True);

            // perform build so that 3d scene is available
            sal_Bool bHasControllersLocked = sal_False;
            uno::Reference< chart::XChartDocument > xDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
            if( xDoc.is() &&
                (bHasControllersLocked = xDoc->hasControllersLocked()) == sal_True )
                xDoc->unlockControllers();

            xProp->setPropertyValue(
                ::rtl::OUString::createFromAscii( "Dim3D" ),
                aTrueAny );
            xProp->setPropertyValue(
                ::rtl::OUString::createFromAscii( "D3DTransformMatrix" ),
                aTransMatrixAny );

            if( bHasControllersLocked )
                xDoc->lockControllers();
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_ERROR( "Transform-Matrix cannot be set" );
        }
    }

    // set changed size and position
    if( xDiaShape.is())
    {
        xDiaShape->setSize( aSize );
        xDiaShape->setPosition( aPosition );
    }
}

void SchXMLPlotAreaContext::EndElement()
{
    mrImportHelper.ResizeChartData( mnSeries, mnMaxSeriesLength );

    uno::Reference< beans::XPropertySet > xProp;
    sal_Int32 i;

    // set autostyles for series and data points
    const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
    const SvXMLStyleContext* pStyle = NULL;
    ::rtl::OUString sCurrStyleName;

    if( pStylesCtxt )
    {
        for( ::std::list< ::chartxml::DataRowPointStyle >::iterator iStyle = maSeriesStyleList.begin();
             iStyle != maSeriesStyleList.end();
             iStyle++ )
        {
            if( iStyle->mnIndex == -1 )
            {
                // data row style
                for( i = 0; i < iStyle->mnRepeat; i++ )
                {
                    try
                    {
                        xProp = mxDiagram->getDataRowProperties( iStyle->mnSeries + i );

                        if( xProp.is())
                        {
                            if( (iStyle->msStyleName).getLength())
                            {
                                if( ! sCurrStyleName.equals( iStyle->msStyleName ))
                                {
                                    sCurrStyleName = iStyle->msStyleName;
                                    pStyle = pStylesCtxt->FindStyleChildContext(
                                        mrImportHelper.GetChartFamilyID(), sCurrStyleName );
                                }

                                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                                    (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
                            }

                            if( iStyle->mnAttachedAxis != 1 )
                            {
                                uno::Any aAny;
                                aAny <<= chart::ChartAxisAssign::SECONDARY_Y;

                                xProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Axis" )), aAny );
                            }
                        }
                    }
                    catch( uno::Exception aEx )
                    {
#ifdef DBG_UTIL
                        String aStr( aEx.Message );
                        ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
                        DBG_ERROR1( "PlotAreaContext:EndElement(): Exception caught: %s", aBStr.GetBuffer());
#endif
                    }
                }
            }
            else
            {
                // data point style
                for( i = 0; i < iStyle->mnRepeat; i++ )
                {
                    try
                    {
                        xProp = mxDiagram->getDataPointProperties( iStyle->mnIndex + i, iStyle->mnSeries );

                        if( xProp.is())
                        {
                            if( ! sCurrStyleName.equals( iStyle->msStyleName ))
                            {
                                sCurrStyleName = iStyle->msStyleName;
                                pStyle = pStylesCtxt->FindStyleChildContext(
                                    mrImportHelper.GetChartFamilyID(), sCurrStyleName );
                            }

                            if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                                (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
                        }
                    }
                    catch( uno::Exception aEx )
                    {
#ifdef DBG_UTIL
                        String aStr( aEx.Message );
                        ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
                        DBG_ERROR1( "PlotAreaContext:EndElement(): Exception caught: %s", aBStr.GetBuffer());
#endif
                    }
                }
            }
        }   // styles iterator
    }
}

// ========================================

SchXMLAxisContext::SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                      uno::Reference< chart::XDiagram > xDiagram,
                                      std::vector< SchXMLAxis >& aAxes ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxDiagram( xDiagram ),
        maAxes( aAxes )
{
}

SchXMLAxisContext::~SchXMLAxisContext()
{}

/* returns a shape for the current axis's title. The property
   "Has...AxisTitle" is set to "True" to get the shape
 */
uno::Reference< drawing::XShape > SchXMLAxisContext::getTitleShape()
{
    uno::Reference< drawing::XShape > xResult;
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);
    uno::Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );

    uno::Reference< chart::XChartDocument > xDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
    sal_Bool bHasControllersLocked = sal_False;
    if( xDoc.is() &&
        (bHasControllersLocked = xDoc->hasControllersLocked()) == sal_True )
        xDoc->unlockControllers();

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_CATEGORY:
        case SCH_XML_AXIS_DOMAIN:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                uno::Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( xDiaProp.is())
                        xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasXAxisTitle" ), aTrueBool );
                    xResult = uno::Reference< drawing::XShape >( xSuppl->getXAxisTitle(), uno::UNO_QUERY );
                }
            }
            break;
        case SCH_XML_AXIS_VALUE:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                uno::Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( xDiaProp.is())
                        xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasYAxisTitle" ), aTrueBool );
                    xResult = uno::Reference< drawing::XShape >( xSuppl->getYAxisTitle(), uno::UNO_QUERY );
                }
            }
            break;
        case SCH_XML_AXIS_SERIES:
            uno::Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
            if( xSuppl.is())
            {
                if( xDiaProp.is())
                    xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasZAxisTitle" ), aTrueBool );
                xResult = uno::Reference< drawing::XShape >( xSuppl->getZAxisTitle(), uno::UNO_QUERY );
            }
            break;
    }

    if( bHasControllersLocked && xDoc.is())
        xDoc->lockControllers();

    return xResult;
}

void SchXMLAxisContext::CreateGrid( ::rtl::OUString sAutoStyleName,
                                    sal_Bool bIsMajor )
{
    uno::Reference< chart::XDiagram > xDia = mrImportHelper.GetChartDocument()->getDiagram();
    uno::Reference< beans::XPropertySet > xGridProp;
    ::rtl::OUString sPropertyName;
    DBG_ASSERT( xDia.is(), "diagram object is invalid!" );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_CATEGORY:
        case SCH_XML_AXIS_DOMAIN:
            {
                uno::Reference< chart::XAxisXSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        xGridProp = xSuppl->getXMainGrid();
                        sPropertyName = ::rtl::OUString::createFromAscii( "HasXAxisGrid" );
                    }
                    else
                    {
                        xGridProp = xSuppl->getXHelpGrid();
                        sPropertyName = ::rtl::OUString::createFromAscii( "HasXAxisHelpGrid" );
                    }
                }
            }
            break;
        case SCH_XML_AXIS_VALUE:
            {
                uno::Reference< chart::XAxisYSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        xGridProp = xSuppl->getYMainGrid();
                        sPropertyName = ::rtl::OUString::createFromAscii( "HasYAxisGrid" );
                    }
                    else
                    {
                        xGridProp = xSuppl->getYHelpGrid();
                        sPropertyName = ::rtl::OUString::createFromAscii( "HasYAxisHelpGrid" );
                    }
                }
            }
            break;
        case SCH_XML_AXIS_SERIES:
            {
                uno::Reference< chart::XAxisZSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        xGridProp = xSuppl->getZMainGrid();
                        sPropertyName = ::rtl::OUString::createFromAscii( "HasZAxisGrid" );
                    }
                    else
                    {
                        xGridProp = xSuppl->getZHelpGrid();
                        sPropertyName = ::rtl::OUString::createFromAscii( "HasZAxisHelpGrid" );
                    }
                }
            }
            break;
    }

    // enable grid
    if( sPropertyName.getLength())
    {
        uno::Reference< beans::XPropertySet > xDiaProp( xDia, uno::UNO_QUERY );
        uno::Any aTrueBool;
        aTrueBool <<= (sal_Bool)(sal_True);
        if( xDiaProp.is())
        {
            try
            {
                xDiaProp->setPropertyValue( sPropertyName, aTrueBool );
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_ERROR( "Cannot enable grid due to missing property" );
            }
        }
    }

    // set properties
    if( sAutoStyleName.getLength() &&
        xGridProp.is())
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

void SchXMLAxisContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetAxisAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_AXIS_CLASS:
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
}

void SchXMLAxisContext::EndElement()
{
    // add new Axis to list
    maAxes.push_back( maCurrentAxis );

    // set axis at chart
    uno::Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProp;
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);
    sal_Bool bHasTitle = ( maCurrentAxis.aTitle.getLength() > 0 );
    uno::Reference< frame::XModel > xDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_CATEGORY:
        case SCH_XML_AXIS_DOMAIN:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasXAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on x axis" );
                }
                uno::Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    xProp = xSuppl->getXAxis();
                    if( bHasTitle )
                    {
                        uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getXAxisTitle(), uno::UNO_QUERY );
                        if( xTitleProp.is())
                        {
                            try
                            {
                                uno::Any aAny;
                                aAny <<= maCurrentAxis.aTitle;
                                xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                            }
                            catch( beans::UnknownPropertyException )
                            {
                                DBG_ERROR( "Property String for Title not available" );
                            }
                            uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                            if( xShape.is())
                            {
                                // perform build chart with new title string
                                // so that setting the position works correctly
                                if( xDoc.is())
                                {
                                    xDoc->unlockControllers();
                                    xDoc->lockControllers();
                                }
                                xShape->setPosition( maCurrentAxis.aPosition );
                            }
                        }
                    }
                }
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasSecondaryXAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on second x axis" );
                }
                uno::Reference< chart::XTwoAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryXAxis();
            }
            break;

        case SCH_XML_AXIS_VALUE:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasYAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on y axis" );
                }
                uno::Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    xProp = xSuppl->getYAxis();
                    if( bHasTitle )
                    {
                        uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getYAxisTitle(), uno::UNO_QUERY );
                        if( xTitleProp.is())
                        {
                            try
                            {
                                uno::Any aAny;
                                aAny <<= maCurrentAxis.aTitle;
                                xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                            }
                            catch( beans::UnknownPropertyException )
                            {
                                DBG_ERROR( "Property String for Title not available" );
                            }
                            uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                            if( xShape.is())
                            {
                                // perform build chart with new title string
                                // so that setting the position works correctly
                                if( xDoc.is())
                                {
                                    xDoc->unlockControllers();
                                    xDoc->lockControllers();
                                }
                                xShape->setPosition( maCurrentAxis.aPosition );
                            }
                        }
                    }
                }
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasSecondaryYAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on second y axis" );
                }
                uno::Reference< chart::XTwoAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryYAxis();
            }
            break;

        case SCH_XML_AXIS_SERIES:
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasZAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on z axis" );
                }
                uno::Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    xProp = xSuppl->getZAxis();
                    if( bHasTitle )
                    {
                        uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getZAxisTitle(), uno::UNO_QUERY );
                        if( xTitleProp.is())
                        {
                            try
                            {
                                uno::Any aAny;
                                aAny <<= maCurrentAxis.aTitle;
                                xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                            }
                            catch( beans::UnknownPropertyException )
                            {
                                DBG_ERROR( "Property String for Title not available" );
                            }
                            uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                            if( xShape.is())
                            {
                                // perform build chart with new title string
                                // so that setting the position works correctly
                                if( xDoc.is())
                                {
                                    xDoc->unlockControllers();
                                    xDoc->lockControllers();
                                }
                                xShape->setPosition( maCurrentAxis.aPosition );
                            }
                        }
                    }
                }
            }
            break;
    }

    // set properties
    if( msAutoStyleName.getLength() &&
        xProp.is())
    {
        const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
        if( pStylesCtxt )
        {
            const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                mrImportHelper.GetChartFamilyID(), msAutoStyleName );

            if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
        }
    }
}

SvXMLImportContext* SchXMLAxisContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();

    if( nPrefix == XML_NAMESPACE_CHART )
    {
        if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_title )))
        {
            uno::Reference< drawing::XShape > xTitleShape = getTitleShape();
            pContext = new SchXMLTitleContext( mrImportHelper, rImport, rLocalName,
                                               maCurrentAxis.aTitle,
                                               xTitleShape,
                                               maCurrentAxis.aPosition );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_grid )))
        {
            sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
            sal_Bool bIsMajor = sal_True;       // default value for class is "major"
            rtl::OUString sAutoStyleName;

            for( sal_Int16 i = 0; i < nAttrCount; i++ )
            {
                rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
                rtl::OUString aLocalName;
                USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

                if( nPrefix == XML_NAMESPACE_CHART )
                {
                    if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_class )))
                    {
                        if( xAttrList->getValueByIndex( i ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_minor )))
                            bIsMajor = sal_False;
                    }
                    else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_style_name )))
                        sAutoStyleName = xAttrList->getValueByIndex( i );
                }
            }

            CreateGrid( sAutoStyleName, bIsMajor );

            // don't create a context => use default context. grid elements are empty
        }
    }

    if( ! pContext )
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );

    return pContext;
}

// ========================================

SchXMLSeriesContext::SchXMLSeriesContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport, const rtl::OUString& rLocalName,
    uno::Reference< chart::XDiagram >& xDiagram,
    std::vector< SchXMLAxis >& rAxes,
    com::sun::star::chart::ChartSeriesAddress& rSeriesAddress,
    ::std::list< ::chartxml::DataRowPointStyle >& rStyleList,
    sal_Int32 nSeriesIndex,
    sal_Int32& rMaxSeriesLength,
    sal_Int32& rDomainOffset ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mxDiagram( xDiagram ),
        mrAxes( rAxes ),
        mrImportHelper( rImpHelper ),
        mrSeriesAddress( rSeriesAddress ),
        mrStyleList( rStyleList ),
        mnSeriesIndex( nSeriesIndex ),
        mnDataPointIndex( 0 ),
        mrDomainOffset( rDomainOffset ),
        mrMaxSeriesLength( rMaxSeriesLength ),
        mpAttachedAxis( NULL )
{
}

SchXMLSeriesContext::~SchXMLSeriesContext()
{
}

void SchXMLSeriesContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    ::rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetSeriesAttrTokenMap();
    ::rtl::OUString sAutoStyleName;
    sal_Int32 nAttachedAxis = 1;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_SERIES_CELL_RANGE:
                mrSeriesAddress.DataRangeAddress = aValue;
                break;
            case XML_TOK_SERIES_LABEL_ADDRESS:
                mrSeriesAddress.LabelAddress = aValue;
                break;
            case XML_TOK_SERIES_ATTACHED_AXIS:
                {
                    sal_Int32 nNumOfAxes = mrAxes.size();
                    for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
                    {
                        if( aValue.equals( mrAxes[ nCurrent ].aName ) &&
                            mrAxes[ nCurrent ].eClass == SCH_XML_AXIS_VALUE )
                        {
                            mpAttachedAxis = &( mrAxes[ nCurrent ] );
                        }
                    }
                }
                break;
            case XML_TOK_SERIES_STYLE_NAME:
                sAutoStyleName = aValue;
                break;
            case XML_TOK_SERIES_CHART_CLASS:
                // not supported yet
                break;
        }
    }

    if( mpAttachedAxis )
    {
        if( mpAttachedAxis->nIndexInCategory > 0 )
        {
            // secondary axis => property has to be set (primary is default)
            nAttachedAxis = 2;
        }
    }

    if( sAutoStyleName.getLength() ||
        nAttachedAxis != 1 )
    {
        ::chartxml::DataRowPointStyle aStyle( mnSeriesIndex, -1, 1, sAutoStyleName, nAttachedAxis );
        mrStyleList.push_back( aStyle );
    }
}

void SchXMLSeriesContext::EndElement()
{
    if( mrMaxSeriesLength < mnDataPointIndex )
        mrMaxSeriesLength = mnDataPointIndex;
}

SvXMLImportContext* SchXMLSeriesContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetSeriesElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_SERIES_DOMAIN:
            {
                sal_Int32 nIndex = mrSeriesAddress.DomainRangeAddresses.getLength();
                mrSeriesAddress.DomainRangeAddresses.realloc( nIndex + 1 );
                mrDomainOffset++;
                pContext = new SchXMLCategoriesDomainContext(
                    mrImportHelper, GetImport(),
                    nPrefix, rLocalName,
                    mrSeriesAddress.DomainRangeAddresses[ nIndex ] );
            }
            break;
        case XML_TOK_SERIES_DATA_POINT:
            pContext = new SchXMLDataPointContext( mrImportHelper, GetImport(), rLocalName, mxDiagram,
                                                   mrStyleList, mnSeriesIndex + mrDomainOffset, mnDataPointIndex );
            break;

        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

// ========================================

SchXMLDataPointContext::SchXMLDataPointContext(  SchXMLImportHelper& rImpHelper,
                                                 SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                                 uno::Reference< chart::XDiagram >& xDiagram,
                                                 ::std::list< ::chartxml::DataRowPointStyle >& rStyleList,
                                                 sal_Int32 nSeries, sal_Int32& rIndex ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxDiagram( xDiagram ),
        mrStyleList( rStyleList ),
        mnSeries( nSeries ),
        mrIndex( rIndex )
{
}

SchXMLDataPointContext::~SchXMLDataPointContext()
{
}

void SchXMLDataPointContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    ::rtl::OUString aValue;
    ::rtl::OUString sAutoStyleName;
    sal_Int32 nRepeat = 1;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_style_name )))
                sAutoStyleName = xAttrList->getValueByIndex( i );
            else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_repeated )))
                nRepeat = xAttrList->getValueByIndex( i ).toInt32();
        }
    }

    if( sAutoStyleName.getLength())
    {
        ::chartxml::DataRowPointStyle aStyle( mnSeries, mrIndex, nRepeat, sAutoStyleName );
        mrStyleList.push_back( aStyle );
    }
    mrIndex += nRepeat;
}

// ========================================

SchXMLCategoriesDomainContext::SchXMLCategoriesDomainContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    rtl::OUString& rAddress ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrAddress( rAddress )
{
}

SchXMLCategoriesDomainContext::~SchXMLCategoriesDomainContext()
{
}

void SchXMLCategoriesDomainContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_cell_range_address )))
        {
            mrAddress = xAttrList->getValueByIndex( i );
        }
    }
}

// ========================================

SchXMLWallFloorContext::SchXMLWallFloorContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    uno::Reference< chart::XDiagram >& xDiagram,
    ContextType eContextType ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxWallFloorSupplier( xDiagram, uno::UNO_QUERY ),
        meContextType( eContextType )
{
}

SchXMLWallFloorContext::~SchXMLWallFloorContext()
{
}

void SchXMLWallFloorContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    if( mxWallFloorSupplier.is())
    {
        sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
        rtl::OUString sAutoStyleName;

        for( sal_Int16 i = 0; i < nAttrCount; i++ )
        {
            rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            rtl::OUString aLocalName;
            USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

            if( nPrefix == XML_NAMESPACE_CHART &&
                aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_style_name )))
            {
                sAutoStyleName = xAttrList->getValueByIndex( i );
            }
        }

        if( sAutoStyleName.getLength())
        {
            // set properties
            uno::Reference< beans::XPropertySet > xProp( ( meContextType == CONTEXT_TYPE_WALL )
                                                         ? mxWallFloorSupplier->getWall()
                                                         : mxWallFloorSupplier->getFloor(),
                                                         uno::UNO_QUERY );
            if( xProp.is())
            {
                const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
                if( pStylesCtxt )
                {
                    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                        mrImportHelper.GetChartFamilyID(), sAutoStyleName );

                    if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                        (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
                }
            }
        }
    }
}
