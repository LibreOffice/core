/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLExportHelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:39:53 $
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
#ifndef _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
#define _XMLOFF_SCH_XMLEXPORTHELPER_HXX_

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGMAPPING_HPP_
#include <com/sun/star/util/XStringMapping.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif

#include <queue>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace chart {
        class XDiagram;
        class XChartDocument;
        class XChartDataArray;
        struct ChartSeriesAddress;
    }
    namespace chart2 {
        class XDiagram;
        class XChartDocument;
        class XDataSeries;
        namespace data
        {
            class XDataProvider;
            class XDataSequence;
        }
    }
    namespace drawing {
        class XShape;
        class XShapes;
    }
    namespace task {
        class XStatusIndicator;
    }
    namespace frame {
        class XModel;
    }
    namespace xml {
        namespace sax {
            class XAttributeList;
}}}}}

class SvXMLAutoStylePoolP;
class XMLChartExportPropertyMapper;
class SvXMLExport;
class XMLPropertyHandlerFactory;

/** With this class you can export a <chart:chart> element containing
    its data as <table:table> element or without internal table. In
    the latter case you have to provide a table address mapper if the
    cell addressing set at the document is not in XML format.
 */
class XMLOFF_DLLPUBLIC SchXMLExportHelper : public UniRefBase
{
public:
    // first: data sequence for label, second: data sequence for values.
    typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >,
            ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > > tLabelValuesDataPair;
    typedef ::std::vector< tLabelValuesDataPair > tDataSequenceCont;

private:
    SvXMLExport& mrExport;
    SvXMLAutoStylePoolP& mrAutoStylePool;
    UniReference< XMLPropertyHandlerFactory > mxPropertyHandlerFactory;
    UniReference< XMLPropertySetMapper > mxPropertySetMapper;
    UniReference< XMLChartExportPropertyMapper > mxExpPropMapper;

    rtl::OUString msTableName;
    rtl::OUStringBuffer msStringBuffer;
    rtl::OUString msString;

    // members filled by InitRangeSegmentationProperties (retrieved from DataProvider)
    sal_Bool mbHasSeriesLabels;
    sal_Bool mbHasCategoryLabels; //if the categories are only automatically generated this will be false
    sal_Bool mbRowSourceColumns;
    rtl::OUString msChartAddress;
    rtl::OUString msTableNumberList;
    ::com::sun::star::uno::Sequence< sal_Int32 > maSequenceMapping;

    rtl::OUString msCLSID;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxAdditionalShapes;

    tDataSequenceCont m_aDataSequencesToExport;

    /** first parseDocument: collect autostyles and store names in this queue
        second parseDocument: export content and use names from this queue
     */
    ::std::queue< ::rtl::OUString > maAutoStyleNameQueue;
    SAL_DLLPRIVATE void CollectAutoStyle(
        const std::vector< XMLPropertyState >& aStates );
    SAL_DLLPRIVATE void AddAutoStyleAttribute(
        const std::vector< XMLPropertyState >& aStates );

    SAL_DLLPRIVATE SvXMLAutoStylePoolP& GetAutoStylePoolP()
    { return mrAutoStylePool; }

    /// if bExportContent is false the auto-styles are collected
    SAL_DLLPRIVATE void parseDocument( com::sun::star::uno::Reference<
                            com::sun::star::chart::XChartDocument >& rChartDoc,
                        sal_Bool bExportContent,
                        sal_Bool bIncludeTable = sal_False );
    SAL_DLLPRIVATE void exportTable();
    SAL_DLLPRIVATE void exportPlotArea(
        com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > xDiagram,
        com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > xNewDiagram,
        const ::com::sun::star::awt::Size & rPageSize,
        sal_Bool bExportContent,
        sal_Bool bIncludeTable );
    SAL_DLLPRIVATE void exportAxes( const com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > & xDiagram,
                                    const com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > & xNewDiagram,
                                    sal_Bool bExportContent );

    SAL_DLLPRIVATE void exportSeries(
        const com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > & xNewDiagram,
        const ::com::sun::star::awt::Size & rPageSize,
        sal_Bool bExportContent,
        sal_Bool bHasTwoYAxes );
    SAL_DLLPRIVATE void exportCandleStickSeries(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > > & aSeriesSeq,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        sal_Bool bJapaneseCandleSticks,
        sal_Bool bExportContent );
    SAL_DLLPRIVATE void exportDataPoints(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xSeriesProperties,
        sal_Int32 nSeriesLength,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        sal_Bool bExportContent );
    SAL_DLLPRIVATE void exportRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xSeriesProp,
        const ::com::sun::star::awt::Size & rPageSize,
        sal_Bool bExportContent );

    /// add svg position as attribute for current element
    SAL_DLLPRIVATE void addPosition( const ::com::sun::star::awt::Point & rPosition );
    SAL_DLLPRIVATE void addPosition( com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape );
    /// add svg size as attribute for current element
    SAL_DLLPRIVATE void addSize( const ::com::sun::star::awt::Size & rSize );
    SAL_DLLPRIVATE void addSize( com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape );
    /// fills the member msString with the appropriate String (i.e. "A3")
    SAL_DLLPRIVATE void getCellAddress( sal_Int32 nCol, sal_Int32 nRow );
    /// interchanges rows and columns of the sequence given
    SAL_DLLPRIVATE void swapDataArray( com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< double > >& rSequence );
    /// exports a string as a paragraph element
    SAL_DLLPRIVATE void exportText( const ::rtl::OUString& rText, bool bConvertTabsLFs = false );
    SAL_DLLPRIVATE void exportErrorBarRanges();

    SAL_DLLPRIVATE SchXMLExportHelper(SchXMLExportHelper &); // not defined
    SAL_DLLPRIVATE void operator =(SchXMLExportHelper &); // not defined

public:
    SchXMLExportHelper( SvXMLExport& rExport,
                        SvXMLAutoStylePoolP& rASPool );

    virtual ~SchXMLExportHelper();

    // auto-styles
    /// parse chart and collect all auto-styles used in current pool
    void collectAutoStyles( com::sun::star::uno::Reference<
                            com::sun::star::chart::XChartDocument > rChartDoc );
    /// write the styles collected into the current pool as <style:style> elements
    void exportAutoStyles();

    /** export the <chart:chart> element corresponding to rChartDoc
        if bIncludeTable is true, the chart data is exported as <table:table>
        element (inside the chart element).

        Otherwise the external references stored in the chart document are used
        for writing the corresponding attributes at series

        All attributes contained in xAttrList are written at the chart element,
        which ist the outer element of a chart. So these attributes can easily
        be parsed again by the container
     */
    void exportChart( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc,
                      sal_Bool bIncludeTable );

    /// returns the string corresponding to the current FileFormat CLSID for Chart
    const rtl::OUString& getChartCLSID();

    UniReference< XMLPropertySetMapper > GetPropertySetMapper() const { return mxPropertySetMapper; }

    void SetChartRangeAddress( const ::rtl::OUString& rAddress )
        { msChartAddress = rAddress; }
    void SetTableNumberList( const ::rtl::OUString& rList )
        { msTableNumberList = rList; }

    void InitRangeSegmentationProperties(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    // if no data provider exists by, now the model (as XChild) is asked for its
    // parent which creates the data provider that is finally set at the chart
    // document
    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >
        GetDataProvider( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    ::com::sun::star::awt::Size getPageSize(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc ) const;
};

#endif  // _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
