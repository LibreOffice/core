/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLChartContext.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:45:09 $
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
#ifndef _SCH_XMLCHARTCONTEXT_HXX_
#define _SCH_XMLCHARTCONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#include "transporttypes.hxx"

#include <list>
#include <map>

class SchXMLImport;
class SchXMLImportHelper;

namespace com { namespace sun { namespace star {
    namespace chart {
        class XChartDocument;
        class XDiagram;
        struct ChartSeriesAddress;
    }
    namespace util {
        class XStringMapping;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    }}
    namespace drawing {
        class XShapes;
    }
}}}

// ----------------------------------------

struct SeriesDefaultsAndStyles
{
    //default values for series:
    ::com::sun::star::uno::Any    maSymbolTypeDefault;
    ::com::sun::star::uno::Any    maDataCaptionDefault;

    ::com::sun::star::uno::Any    maErrorIndicatorDefault;
    ::com::sun::star::uno::Any    maErrorCategoryDefault;
    ::com::sun::star::uno::Any    maConstantErrorLowDefault;
    ::com::sun::star::uno::Any    maConstantErrorHighDefault;
    ::com::sun::star::uno::Any    maPercentageErrorDefault;
    ::com::sun::star::uno::Any    maErrorMarginDefault;

    ::com::sun::star::uno::Any    maMeanValueDefault;
    ::com::sun::star::uno::Any    maRegressionCurvesDefault;

    ::com::sun::star::uno::Any    maStackedDefault;
    ::com::sun::star::uno::Any    maPercentDefault;
    ::com::sun::star::uno::Any    maDeepDefault;
    ::com::sun::star::uno::Any    maStackedBarsConnectedDefault;

    //additional information
    ::com::sun::star::uno::Any    maLinesOnProperty;

    //styles for series and datapoints
    ::std::list< DataRowPointStyle > maSeriesStyleList;
};

class SchXMLChartContext : public SvXMLImportContext
{
public:
    SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const rtl::OUString& rLocalName );
    virtual ~SchXMLChartContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                                     com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    ::rtl::OUString maMainTitle, maSubTitle;
    com::sun::star::awt::Point maMainTitlePos, maSubTitlePos, maLegendPos;
    sal_Bool mbHasOwnTable;
    sal_Bool mbAllRangeAddressesAvailable;
    sal_Bool mbColHasLabels;
    sal_Bool mbRowHasLabels;
    ::com::sun::star::chart::ChartDataRowSource meDataRowSource;
    bool mbIsStockChart;

    com::sun::star::uno::Sequence< com::sun::star::chart::ChartSeriesAddress > maSeriesAddresses;
    ::rtl::OUString msCategoriesAddress;
    ::rtl::OUString msChartAddress;

    SeriesDefaultsAndStyles maSeriesDefaultsAndStyles;
    tSchXMLLSequencesPerIndex maLSequencesPerIndex;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxDrawPage;
    ::rtl::OUString msColTrans;
    ::rtl::OUString msRowTrans;
    ::rtl::OUString maChartTypeServiceName;

    ::com::sun::star::awt::Size maChartSize;

    /** @descr  This method bundles some settings to the chart model and executes them with
            a locked controller.  This includes setting the draw page size and setting
            the chart type.
        @param  aChartSize  The size the draw page will be set to.
        @param  bDomainForDefaultDataNeeded This flag indicates wether the chart's data set
            has to contain a domain value.
        @param  aServiceName The name of the service the diagram is initialized with.
        @param  bSetWitchData   Indicates wether the data set takes it's data series from
            rows or from columns.
    */
    void    InitChart   (com::sun::star::awt::Size aChartSize,
                        sal_Bool bDomainForDefaultDataNeeded,
                        const ::rtl::OUString & rChartTypeServiceName,
                        sal_Bool bSetSwitchData);

    void ChangeDiagramAccordingToTemplate(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xNewDoc );
    ::com::sun::star::uno::Sequence< sal_Int32 > GetNumberSequenceFromString( const ::rtl::OUString& rStr, bool bAddOneToEachOldIndex );
    void MergeSeriesForStockChart();
};

// ----------------------------------------

class SchXMLTitleContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    rtl::OUString& mrTitle;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxTitleShape;
    rtl::OUString msAutoStyleName;

public:
    SchXMLTitleContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const rtl::OUString& rLocalName,
                        rtl::OUString& rTitle,
                        com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xTitleShape );
    virtual ~SchXMLTitleContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                               com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLLegendContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLLegendContext( SchXMLImportHelper& rImpHelper,
                         SvXMLImport& rImport, const rtl::OUString& rLocalName );
    virtual ~SchXMLLegendContext();

    virtual void StartElement( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif  // _SCH_XMLCHARTCONTEXT_HXX_
