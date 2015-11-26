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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLSERIES2CONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLSERIES2CONTEXT_HXX

#include "transporttypes.hxx"
#include "SchXMLChartContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/SchXMLImportHelper.hxx>
#include <xmloff/xmlstyle.hxx>

#include <vector>
#include <list>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
        class XDataSeries;
    }
    namespace awt {
        struct Size;
    }
}}}

// class for child contexts: series, data point and statistics objects
class SchXMLSeries2Context : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    css::uno::Reference< css::chart2::XChartDocument > mxNewDoc;
    ::std::vector< SchXMLAxis >& mrAxes;
    ::std::list< DataRowPointStyle >& mrStyleList;
    ::std::list< RegressionStyle >& mrRegressionStyleList;

    css::uno::Reference< css::chart2::XDataSeries > m_xSeries;
    sal_Int32 mnSeriesIndex;
    sal_Int32 mnDataPointIndex;
    bool m_bStockHasVolume;

    GlobalSeriesImportInfo& m_rGlobalSeriesImportInfo;

    SchXMLAxis* mpAttachedAxis;
    sal_Int32 mnAttachedAxis;
    OUString msAutoStyleName;
    ::std::vector< OUString > maDomainAddresses;
    OUString maGlobalChartTypeName;
    OUString maSeriesChartTypeName;
    OUString m_aSeriesRange;
    bool            m_bHasDomainContext;
    tSchXMLLSequencesPerIndex & mrLSequencesPerIndex;
    tSchXMLLSequencesPerIndex maPostponedSequences;
    bool& mrGlobalChartTypeUsedBySeries;
    bool mbSymbolSizeIsMissingInFile;
    css::awt::Size maChartSize;

public:
    SchXMLSeries2Context( SchXMLImportHelper& rImpHelper,
                          SvXMLImport& rImport, const OUString& rLocalName,
                          const css::uno::Reference< css::chart2::XChartDocument > & xNewDoc,
                          std::vector< SchXMLAxis >& rAxes,
                          ::std::list< DataRowPointStyle >& rStyleList,
                          ::std::list< RegressionStyle >& rRegressionStyleList,
                          sal_Int32 nSeriesIndex,
                          bool bStockHasVolume,
                          GlobalSeriesImportInfo& rGlobalSeriesImportInfo,
                          const OUString & aGlobalChartTypeName,
                          tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                          bool& rGlobalChartTypeUsedBySeries,
                          const css::awt::Size & rChartSize );
    virtual ~SchXMLSeries2Context();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    static void initSeriesPropertySets( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const css::uno::Reference< css::frame::XModel >& xChartModel );

    static void setDefaultsToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles );

    static void setStylesToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart
        , tSchXMLLSequencesPerIndex & rInOutLSequencesPerIndex );

    static void setStylesToStatisticsObjects( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString &rCurrStyleName );

    static void setStylesToRegressionCurves(
                    SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
                    const SvXMLStylesContext* pStylesCtxt,
                    const SvXMLStyleContext*& rpStyle,
                    OUString &rCurrStyleName );

    static void setStylesToDataPoints( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart, bool bIsDonutChart, bool bSwitchOffLinesForScatter );

    static void switchSeriesLinesOff( ::std::list< DataRowPointStyle >& rSeriesStyleList );
};

// INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLSERIES2CONTEXT_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
