/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <bf_xmloff/uniref.hxx>
#include <bf_xmloff/families.hxx>
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
}}}}}
namespace binfilter {

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
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > mxChartDoc;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringMapping > mxTableAddressMapper;
    SvXMLStylesContext* mpAutoStyles;

    SvXMLTokenMap* mpDocElemTokenMap;
    SvXMLTokenMap* mpTableElemTokenMap;
    SvXMLTokenMap* mpChartElemTokenMap;
    SvXMLTokenMap* mpPlotAreaElemTokenMap;
    SvXMLTokenMap* mpSeriesElemTokenMap;

    SvXMLTokenMap* mpChartAttrTokenMap;
    SvXMLTokenMap* mpPlotAreaAttrTokenMap;
    SvXMLTokenMap* mpAxisAttrTokenMap;
    SvXMLTokenMap* mpLegendAttrTokenMap;
    SvXMLTokenMap* mpAutoStyleAttrTokenMap;
    SvXMLTokenMap* mpCellAttrTokenMap;
    SvXMLTokenMap* mpSeriesAttrTokenMap;

public:

    SchXMLImportHelper();
    ~SchXMLImportHelper();

    /** get the context for reading the <chart:chart> element with subelements.
        The result is stored in the XModel given if it also implements
        XChartDocument
     */
    SvXMLImportContext* CreateChartContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > xChartModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    /** set the auto-style context that will be used to retrieve auto-styles
        used inside the following <chart:chart> element to parse
     */
    void SetAutoStylesContext( SvXMLStylesContext* pAutoStyles ) { mpAutoStyles = pAutoStyles; }
    SvXMLStylesContext* GetAutoStylesContext() const { return mpAutoStyles; }

    /** set the string mapper that is used to convert cell addresses in
        XML format into the application format

        If the mapper is set it is automatically used for conversion
      */
    void SetTableAddressMapper( ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XStringMapping >& xMapper )
        { mxTableAddressMapper = xMapper; }

    const ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XStringMapping >& GetTableAddressMapper()
        { return mxTableAddressMapper; }

    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XChartDocument >& GetChartDocument()
        { return mxChartDoc; }

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();
    const SvXMLTokenMap& GetChartElemTokenMap();
    const SvXMLTokenMap& GetPlotAreaElemTokenMap();
    const SvXMLTokenMap& GetSeriesElemTokenMap();

    const SvXMLTokenMap& GetChartAttrTokenMap();
    const SvXMLTokenMap& GetPlotAreaAttrTokenMap();
    const SvXMLTokenMap& GetAxisAttrTokenMap();
    const SvXMLTokenMap& GetLegendAttrTokenMap();
    const SvXMLTokenMap& GetCellAttrTokenMap();
    const SvXMLTokenMap& GetSeriesAttrTokenMap();

    static sal_uInt16 GetChartFamilyID() { return XML_STYLE_FAMILY_SCH_CHART_ID; }

    void		ResizeChartData( sal_Int32 nSeries, sal_Int32 nDataPoints = -1 );
};

}//end of namespace binfilter
#endif	// _XMLOFF_SCH_XMLIMPORTHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
