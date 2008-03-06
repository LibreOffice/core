/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLPlotAreaContext.hxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:51:28 $
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
#ifndef _SCH_XMLPLOTAREACONTEXT_HXX_
#define _SCH_XMLPLOTAREACONTEXT_HXX_

#include "SchXMLImport.hxx"
#include "SchXMLChartContext.hxx"

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include <xmloff/shapeimport.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSERIESADDRESS_HPP_
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif

#include "transporttypes.hxx"

class SvXMLImport;

namespace com { namespace sun { namespace star {
    namespace chart {
        class XDiagram;
        class X3DDisplay;
        class XStatisticDisplay;
    }
    namespace chart2 {
        class XChartDocument;
    }
    namespace xml { namespace sax {
        class XAttributeList;
}}}}}

// ----------------------------------------

class SchXML3DSceneAttributesHelper : public SdXML3DSceneAttributesHelper
{
public:
    SchXML3DSceneAttributesHelper( SvXMLImport& rImporter );
    virtual ~SchXML3DSceneAttributesHelper();

    void getCameraDefaultFromDiagram( const ::com::sun::star::uno::Reference< com::sun::star::chart::XDiagram >& xDiagram );

private:
    SchXML3DSceneAttributesHelper();
};

class SchXMLPlotAreaContext : public SvXMLImportContext
{
public:
    SchXMLPlotAreaContext( SchXMLImportHelper& rImpHelper,
                           SvXMLImport& rImport, const rtl::OUString& rLocalName,
                           ::com::sun::star::uno::Sequence<
                               ::com::sun::star::chart::ChartSeriesAddress >& rSeriesAddresses,
                           ::rtl::OUString& rCategoriesAddress,
                           ::rtl::OUString& rChartAddress,
                           sal_Bool & rHasOwnTable,
                           sal_Bool & rAllRangeAddressesAvailable,
                           sal_Bool & rColHasLabels,
                           sal_Bool & rRowHasLabels,
                           ::com::sun::star::chart::ChartDataRowSource & rDataRowSource,
                           SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
                           const ::rtl::OUString& aChartTypeServiceName,
                           tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                           const ::com::sun::star::awt::Size & rChartSize );
    virtual ~SchXMLPlotAreaContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > mxDiagram;
    ::com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > mxNewDoc;
    ::std::vector< SchXMLAxis > maAxes;
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart::ChartSeriesAddress >& mrSeriesAddresses;
    rtl::OUString& mrCategoriesAddress;
    SeriesDefaultsAndStyles& mrSeriesDefaultsAndStyles;
    sal_Int32 mnNumOfLinesProp;
    sal_Int32 mnNumOfLinesReadBySeries;
    sal_Bool  mbStockHasVolume;
    sal_Int32 mnSeries;
    sal_Int32 mnMaxSeriesLength;
    SchXML3DSceneAttributesHelper maSceneImportHelper;
    ::com::sun::star::awt::Size maSize;
    ::com::sun::star::awt::Point maPosition;
    bool mbHasSize;
    bool mbHasPosition;
    bool mbPercentStacked;
    ::rtl::OUString msAutoStyleName;
    ::rtl::OUString& mrChartAddress;
    sal_Bool & mrHasOwnTable;
    sal_Bool & mrAllRangeAddressesAvailable;
    sal_Bool & mrColHasLabels;
    sal_Bool & mrRowHasLabels;
    ::com::sun::star::chart::ChartDataRowSource & mrDataRowSource;
    ::rtl::OUString maFirstFirstDomainAddress;
    sal_Int32 mnFirstFirstDomainIndex;

    ::rtl::OUString maChartTypeServiceName;

    tSchXMLLSequencesPerIndex & mrLSequencesPerIndex;
    sal_Int32 mnCurrentDataIndex;
    bool mbGlobalChartTypeUsedBySeries;
    ::com::sun::star::awt::Size maChartSize;
};

// ----------------------------------------

class SchXMLAxisContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > mxDiagram;
    SchXMLAxis maCurrentAxis;
    std::vector< SchXMLAxis >& maAxes;
    rtl::OUString msAutoStyleName;
    rtl::OUString& mrCategoriesAddress;
    bool mbAddMissingXAxisForNetCharts; //to correct errors from older versions
    bool mbAdaptWrongPercentScaleValues; //to correct errors from older versions

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTitleShape();
    void CreateGrid( ::rtl::OUString sAutoStyleName, sal_Bool bIsMajor );
    void CreateAxis();
    void SetAxisTitle();

public:
    SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                       SvXMLImport& rImport, const rtl::OUString& rLocalName,
                       ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > xDiagram,
                       std::vector< SchXMLAxis >& aAxes,
                       ::rtl::OUString& rCategoriesAddress,
                       bool bAddMissingXAxisForNetCharts,
                       bool bAdaptWrongPercentScaleValues );
    virtual ~SchXMLAxisContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

//----------------------------------------

class SchXMLDataPointContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::std::list< DataRowPointStyle >& mrStyleList;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    sal_Int32& mrIndex;
    bool mbSymbolSizeForSeriesIsMissingInFile;

public:
    SchXMLDataPointContext(  SchXMLImportHelper& rImpHelper,
                             SvXMLImport& rImport, const rtl::OUString& rLocalName,
                             ::std::list< DataRowPointStyle >& rStyleList,
                             const ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XDataSeries >& xSeries, sal_Int32& rIndex,
                                bool bSymbolSizeForSeriesIsMissingInFile );
    virtual ~SchXMLDataPointContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLCategoriesDomainContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    rtl::OUString& mrAddress;

public:
    SchXMLCategoriesDomainContext( SchXMLImportHelper& rImpHelper,
                                   SvXMLImport& rImport,
                                   sal_uInt16 nPrefix,
                                   const rtl::OUString& rLocalName,
                                   rtl::OUString& rAddress );
    virtual ~SchXMLCategoriesDomainContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLWallFloorContext : public SvXMLImportContext
{
public:
    enum ContextType
    {
        CONTEXT_TYPE_WALL,
        CONTEXT_TYPE_FLOOR
    };

private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::X3DDisplay > mxWallFloorSupplier;
    ContextType meContextType;

public:
    SchXMLWallFloorContext( SchXMLImportHelper& rImportHelper,
                            SvXMLImport& rImport,
                            sal_uInt16 nPrefix,
                            const rtl::OUString& rLocalName,
                            ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >& xDiagram,
                            ContextType eContextType );
    virtual ~SchXMLWallFloorContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLStockContext : public SvXMLImportContext
{
public:
    enum ContextType
    {
        CONTEXT_TYPE_GAIN,
        CONTEXT_TYPE_LOSS,
        CONTEXT_TYPE_RANGE
    };

private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XStatisticDisplay > mxStockPropProvider;
    ContextType meContextType;

public:
    SchXMLStockContext( SchXMLImportHelper& rImportHelper,
                        SvXMLImport& rImport,
                        sal_uInt16 nPrefix,
                        const rtl::OUString& rLocalName,
                        ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >& xDiagram,
                        ContextType eContextType );
    virtual ~SchXMLStockContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLStatisticsObjectContext : public SvXMLImportContext
{
public:
    enum ContextType
    {
        CONTEXT_TYPE_MEAN_VALUE_LINE,
        CONTEXT_TYPE_REGRESSION_CURVE,
        CONTEXT_TYPE_ERROR_INDICATOR
    };

    SchXMLStatisticsObjectContext(
        SchXMLImportHelper& rImportHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        ::std::list< DataRowPointStyle >& rStyleList,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries,
        ContextType eContextType,
        const ::com::sun::star::awt::Size & rChartSize );

    virtual ~SchXMLStatisticsObjectContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLImportHelper &                           mrImportHelper;
    ::std::list< DataRowPointStyle > &             mrStyleList;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    ContextType                                    meContextType;
    ::com::sun::star::awt::Size                    maChartSize;
};

// ----------------------------------------

class SchXMLEquationContext : public SvXMLImportContext
{
public:
    SchXMLEquationContext(
        SchXMLImportHelper& rImportHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries,
        const ::com::sun::star::awt::Size & rChartSize,
        DataRowPointStyle & rRegressionStyle );

    virtual ~SchXMLEquationContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLImportHelper &                           mrImportHelper;
    DataRowPointStyle &                            mrRegressionStyle;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    ::com::sun::star::awt::Size maChartSize;
};

#endif  // _SCH_XMLPLOTAREACONTEXT_HXX_
