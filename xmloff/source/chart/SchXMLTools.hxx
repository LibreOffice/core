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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLTOOLS_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLTOOLS_HXX

#include <rtl/ustring.hxx>
#include <xmloff/xmltoken.hxx>
#include "transporttypes.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence2.hpp>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
        class XRegressionCurve;
        namespace data {
            class XDataProvider;
        }
    }
}}}

class XMLPropStyleContext;
class SvXMLStylesContext;
class SvXMLExport;

namespace SchXMLTools
{
    bool isDocumentGeneratedWithOpenOfficeOlderThan2_0( const css::uno::Reference< css::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan2_3( const css::uno::Reference< css::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan2_4( const css::uno::Reference< css::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan3_0( const css::uno::Reference< css::frame::XModel >& xChartModel);
    bool isDocumentGeneratedWithOpenOfficeOlderThan3_3( const css::uno::Reference< css::frame::XModel >& xChartModel);

    void setBuildIDAtImportInfo( css::uno::Reference< css::frame::XModel > xModel
        , css::uno::Reference< css::beans::XPropertySet > xImportInfo );

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
        XML_CHART_CLASS_GL3DBAR,
        XML_CHART_CLASS_ADDIN,
        XML_CHART_CLASS_UNKNOWN
    };

    SchXMLChartTypeEnum GetChartTypeEnum( const OUString& rClassName );

    OUString GetChartTypeByClassName(
        const OUString & rClassName, bool bUseOldNames );

    ::xmloff::token::XMLTokenEnum getTokenByChartType(
        const OUString & rChartTypeService, bool bUseOldNames );

    OUString GetNewChartTypeName( const OUString & rOldChartTypeName );

    css::uno::Reference<
        css::chart2::data::XLabeledDataSequence2 > GetNewLabeledDataSequence();

    css::uno::Reference< css::chart2::data::XDataSequence > CreateDataSequence(
        const OUString& rRange,
        const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc );

    css::uno::Reference< css::chart2::data::XDataSequence > CreateDataSequenceWithoutConvert(
        const OUString& rRange,
        const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc );

    void CreateCategories(
        const css::uno::Reference< css::chart2::data::XDataProvider > & xDataProvider,
        const css::uno::Reference< css::chart2::XChartDocument > & xNewDoc,
        const OUString & rRangeAddress,
        sal_Int32 nCooSysIndex,
        sal_Int32 nDimensionIndex,
        tSchXMLLSequencesPerIndex * pLSequencesPerIndex = nullptr );

    css::uno::Any getPropertyFromContext( const OUString& rPropertyName, const XMLPropStyleContext * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt );

    void exportText( SvXMLExport& rExport, const OUString& rText, bool bConvertTabsLFs );

    void exportRangeToSomewhere( SvXMLExport& rExport, const OUString& rValue );

    /** checks if the data sequence has the property "CachedXMLRange" (true for
        internal data sequences), and if so sets this property to the range
        given in rXMLRange
     */
    void setXMLRangePropertyAtDataSequence(
        const css::uno::Reference< css::chart2::data::XDataSequence > & xDataSequence,
        const OUString & rXMLRange );

    /** checks if the data sequence has the property "CachedXMLRange" (true for
        internal data sequences), and if so retrieves this property and applies
        it to the range given in rOutXMLRange.

        @param bClearProp If true, the property is reset to its default after it
                          was assigned to rOutXMLRange

        @return true, if the property was found, assigned and is non-empty
     */
    bool getXMLRangePropertyFromDataSequence(
        const css::uno::Reference< css::chart2::data::XDataSequence > & xDataSequence,
        OUString & rOutXMLRange,
        bool bClearProp = false );

    css::uno::Reference< css::chart2::data::XDataProvider > getDataProviderFromParent( const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc );

    bool switchBackToDataProviderFromParent( const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc
        , const tSchXMLLSequencesPerIndex & rLSequencesPerIndex );

    void copyProperties(
        const css::uno::Reference< css::beans::XPropertySet > & xSource,
        const css::uno::Reference< css::beans::XPropertySet > & xDestination );
}

#endif // INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
