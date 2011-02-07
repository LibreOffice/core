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
#ifndef _XMLOFF_SCH_XMLIMPORTHELPER_HXX_
#define _XMLOFF_SCH_XMLIMPORTHELPER_HXX_

#include <xmloff/uniref.hxx>
#include <xmloff/families.hxx>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>

namespace com { namespace sun { namespace star {
    namespace frame {
        class XModel;
    }
    namespace task {
        class XStatusIndicator;
    }
    namespace xml {
        namespace sax {
            class XAttributeList;
        }
    }
    namespace chart2 {
        namespace data {
            class XDataProvider;
            class XLabeledDataSequence;
        }
        class XChartDocument;
        class XDataSeries;
    }
}}}

class SvXMLUnitConverter;
class SvXMLStylesContext;
class XMLChartImportPropertyMapper;
class SvXMLTokenMap;
class SvXMLImportContext;
class SvXMLImport;

// ========================================

/** With this class you can import a <chart:chart> element containing
    its data as <table:table> element or without internal table. In
    the latter case you have to provide a table address mapper that
    converts table addresses in XML format to the appropriate application
    format.
 */
class SchXMLImportHelper : public UniRefBase
{
private:
    com::sun::star::uno::Reference< com::sun::star::chart::XChartDocument > mxChartDoc;
    SvXMLStylesContext* mpAutoStyles;

    SvXMLTokenMap* mpChartDocElemTokenMap;
    SvXMLTokenMap* mpTableElemTokenMap;
    SvXMLTokenMap* mpChartElemTokenMap;
    SvXMLTokenMap* mpPlotAreaElemTokenMap;
    SvXMLTokenMap* mpSeriesElemTokenMap;

    SvXMLTokenMap* mpChartAttrTokenMap;
    SvXMLTokenMap* mpPlotAreaAttrTokenMap;
    SvXMLTokenMap* mpLegendAttrTokenMap;
    SvXMLTokenMap* mpAutoStyleAttrTokenMap;
    SvXMLTokenMap* mpCellAttrTokenMap;
    SvXMLTokenMap* mpSeriesAttrTokenMap;
    SvXMLTokenMap* mpRegEquationAttrTokenMap;

public:

    SchXMLImportHelper();
    ~SchXMLImportHelper();

    /** get the context for reading the <chart:chart> element with subelements.
        The result is stored in the XModel given if it also implements
        XChartDocument
     */
    SvXMLImportContext* CreateChartContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::frame::XModel > xChartModel,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList >& xAttrList );

    /** set the auto-style context that will be used to retrieve auto-styles
        used inside the following <chart:chart> element to parse
     */
    void SetAutoStylesContext( SvXMLStylesContext* pAutoStyles ) { mpAutoStyles = pAutoStyles; }
    SvXMLStylesContext* GetAutoStylesContext() const { return mpAutoStyles; }

    const com::sun::star::uno::Reference<
        com::sun::star::chart::XChartDocument >& GetChartDocument()
        { return mxChartDoc; }

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();
    const SvXMLTokenMap& GetChartElemTokenMap();
    const SvXMLTokenMap& GetPlotAreaElemTokenMap();
    const SvXMLTokenMap& GetSeriesElemTokenMap();

    const SvXMLTokenMap& GetChartAttrTokenMap();
    const SvXMLTokenMap& GetPlotAreaAttrTokenMap();
    const SvXMLTokenMap& GetLegendAttrTokenMap();
    const SvXMLTokenMap& GetAutoStyleAttrTokenMap();
    const SvXMLTokenMap& GetCellAttrTokenMap();
    const SvXMLTokenMap& GetSeriesAttrTokenMap();
    const SvXMLTokenMap& GetRegEquationAttrTokenMap();

    static sal_uInt16 GetChartFamilyID() { return XML_STYLE_FAMILY_SCH_CHART_ID; }

    /** @param bPushLastChartType If </sal_False>, in case a new chart type has to
               be added (because it does not exist yet), it is appended at the
               end of the chart-type container.  When </sal_True>, a new chart type
               is added at one position before the last one, i.e. the formerly
               last chart type is pushed back, so that it remains the last one.

               This is needed when the global chart type is set to type A, but
               the first series has type B. Then B should appear before A (done
               by passing true).  Once a series of type A has been read,
               following new chart types are again be added at the end (by
               passing false).
     */
    static ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > GetNewDataSeries(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XChartDocument > & xDoc,
                    sal_Int32 nCoordinateSystemIndex,
                    const ::rtl::OUString & rChartTypeName,
                    bool bPushLastChartType = false );

    static void DeleteDataSeries(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries >& xSeries,
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XChartDocument > & xDoc );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > GetNewLabeledDataSequence();
};

#endif  // _XMLOFF_SCH_XMLIMPORTHELPER_HXX_
