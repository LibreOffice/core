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
#ifndef SCH_XML_TOOLS_HXX_
#define SCH_XML_TOOLS_HXX_

#include <rtl/ustring.hxx>
#include <xmloff/xmltoken.hxx>
#include "transporttypes.hxx"

#include <com/sun/star/frame/XModel.hpp>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
        class XRegressionCurve;
        namespace data {
            class XDataProvider;
            class XLabeledDataSequence;
        }
    }
}}}

class XMLPropStyleContext;
class SvXMLStylesContext;
class SvXMLExport;

namespace SchXMLTools
{
    bool isDocumentGeneratedWithOpenOfficeOlderThan2_0( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan2_3( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan2_4( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan3_0( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan3_3( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel);

    void setBuildIDAtImportInfo( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel
        , ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xImportInfo );

    enum SchXMLChartTypeEnum
    {
        XML_CHART_CLASS_LINE,
        XML_CHART_CLASS_AREA,
        XML_CHART_CLASS_CIRCLE,
        XML_CHART_CLASS_RING,
        XML_CHART_CLASS_SCATTER,
        XML_CHART_CLASS_RADAR,
        XML_CHART_CLASS_FILLED_RADAR,
        XML_CHART_CLASS_BAR,
        XML_CHART_CLASS_STOCK,
        XML_CHART_CLASS_BUBBLE,
        XML_CHART_CLASS_ADDIN,
        XML_CHART_CLASS_UNKNOWN
    };

    SchXMLChartTypeEnum GetChartTypeEnum( const ::rtl::OUString& rClassName );

    ::rtl::OUString GetChartTypeByClassName(
        const ::rtl::OUString & rClassName, bool bUseOldNames );

    ::xmloff::token::XMLTokenEnum getTokenByChartType(
        const ::rtl::OUString & rChartTypeService, bool bUseOldNames );

    ::rtl::OUString GetNewChartTypeName( const ::rtl::OUString & rOldChartTypeName );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence > GetNewLabeledDataSequence();

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > CreateDataSequence(
        const ::rtl::OUString& rRange,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >& xChartDoc );

    void CreateCategories(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > & xDataProvider,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xNewDoc,
        const ::rtl::OUString & rRangeAddress,
        sal_Int32 nCooSysIndex,
        sal_Int32 nDimensionIndex,
        tSchXMLLSequencesPerIndex * pLSequencesPerIndex = 0 );

    ::com::sun::star::uno::Any getPropertyFromContext( const ::rtl::OUString& rPropertyName, const XMLPropStyleContext * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt );

    void exportText( SvXMLExport& rExport, const ::rtl::OUString& rText, bool bConvertTabsLFs );

    void exportRangeToSomewhere( SvXMLExport& rExport, const ::rtl::OUString& rValue );

    /** returns the properties of the equation of the first regression curve
        that is no mean-value line
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XRegressionCurve > getRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xDataSeries );

    /** checks if the data sequence has the property "CachedXMLRange" (true for
        internal data sequences), and if so sets this property to the range
        given in rXMLRange
     */
    void setXMLRangePropertyAtDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & xDataSequence,
        const ::rtl::OUString & rXMLRange );

    /** checks if the data sequence has the property "CachedXMLRange" (true for
        internal data sequences), and if so retrieves this property and applies
        it to the range given in rOutXMLRange.

        @param bClearProp If true, the property is reset to its default after it
                          was assigned to rOutXMLRange

        @return true, if the property was found, assigned and is non-empty
     */
    bool getXMLRangePropertyFromDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & xDataSequence,
        ::rtl::OUString & rOutXMLRange,
        bool bClearProp = false );

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > getDataProviderFromParent( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDoc );

    bool switchBackToDataProviderFromParent( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument >& xChartDoc
        , const tSchXMLLSequencesPerIndex & rLSequencesPerIndex );

    void copyProperties(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xSource,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xDestination );
}

#endif  // SCH_XML_TOOLS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
