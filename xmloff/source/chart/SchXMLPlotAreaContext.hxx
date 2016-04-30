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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPLOTAREACONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPLOTAREACONTEXT_HXX

#include "SchXMLImport.hxx"
#include "SchXMLChartContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/shapeimport.hxx>
#include <com/sun/star/chart/ChartDataRowSource.hpp>

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

class SchXML3DSceneAttributesHelper : public SdXML3DSceneAttributesHelper
{
public:
    explicit SchXML3DSceneAttributesHelper( SvXMLImport& rImporter );
    virtual ~SchXML3DSceneAttributesHelper();

    void getCameraDefaultFromDiagram( const css::uno::Reference< css::chart::XDiagram >& xDiagram );

private:
    SchXML3DSceneAttributesHelper();
};

class SchXMLPositionAttributesHelper
{
public:
    explicit SchXMLPositionAttributesHelper( SvXMLImport& rImporter );
    ~SchXMLPositionAttributesHelper();

    void readPositioningAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue );
    void readAutomaticPositioningProperties( XMLPropStyleContext* pPropStyleContext, const SvXMLStylesContext* pStylesCtxt );

    bool hasPosSize() const;
    bool isAutomatic() const;
    css::awt::Rectangle getRectangle() const { return css::awt::Rectangle( m_aPosition.X, m_aPosition.Y, m_aSize.Width, m_aSize.Height );}

private:
    bool hasSize() const;
    bool hasPosition() const;

    SvXMLImport& m_rImport;

    css::awt::Point m_aPosition;
    css::awt::Size m_aSize;

    bool m_bHasSizeWidth;
    bool m_bHasSizeHeight;
    bool m_bHasPositionX;
    bool m_bHasPositionY;
    bool m_bAutoSize;
    bool m_bAutoPosition;
};

class SchXMLPlotAreaContext : public SvXMLImportContext
{
public:
    SchXMLPlotAreaContext( SchXMLImportHelper& rImpHelper,
                           SvXMLImport& rImport, const OUString& rLocalName,
                           const OUString& rXLinkHRefAttributeToIndicateDataProvider,
                           OUString& rCategoriesAddress,
                           OUString& rChartAddress,
                           bool& bHasRangeAtPlotArea,
                           bool & rAllRangeAddressesAvailable,
                           bool & rColHasLabels,
                           bool & rRowHasLabels,
                           css::chart::ChartDataRowSource & rDataRowSource,
                           SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
                           const OUString& aChartTypeServiceName,
                           tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                           const css::awt::Size & rChartSize );
    virtual ~SchXMLPlotAreaContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

private:
    SchXMLImportHelper& mrImportHelper;
    css::uno::Reference< css::chart::XDiagram > mxDiagram;
    css::uno::Reference< css::chart2::XChartDocument > mxNewDoc;
    ::std::vector< SchXMLAxis > maAxes;
    OUString& mrCategoriesAddress;
    SeriesDefaultsAndStyles& mrSeriesDefaultsAndStyles;
    sal_Int32 mnNumOfLinesProp;
    bool  mbStockHasVolume;
    sal_Int32 mnSeries;
    GlobalSeriesImportInfo m_aGlobalSeriesImportInfo;

    SchXML3DSceneAttributesHelper maSceneImportHelper;
    SchXMLPositionAttributesHelper m_aOuterPositioning;//including axes and axes titles
    SchXMLPositionAttributesHelper m_aInnerPositioning;//excluding axes and axes titles
    bool mbPercentStacked;
    bool m_bAxisPositionAttributeImported;
    OUString msAutoStyleName;
    const OUString& m_rXLinkHRefAttributeToIndicateDataProvider;
    OUString& mrChartAddress;
    bool& m_rbHasRangeAtPlotArea;
    bool & mrColHasLabels;
    bool & mrRowHasLabels;
    css::chart::ChartDataRowSource & mrDataRowSource;
    OUString maChartTypeServiceName;

    tSchXMLLSequencesPerIndex & mrLSequencesPerIndex;

    bool mbGlobalChartTypeUsedBySeries;
    css::awt::Size maChartSize;
};

class SchXMLDataPointContext : public SvXMLImportContext
{
private:
    ::std::list< DataRowPointStyle >& mrStyleList;
    css::uno::Reference< css::chart2::XDataSeries > m_xSeries;
    sal_Int32& mrIndex;
    bool mbSymbolSizeForSeriesIsMissingInFile;

public:
    SchXMLDataPointContext(  SvXMLImport& rImport, const OUString& rLocalName,
                             ::std::list< DataRowPointStyle >& rStyleList,
                             const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
                             sal_Int32& rIndex,
                             bool bSymbolSizeForSeriesIsMissingInFile );
    virtual ~SchXMLDataPointContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
};

class SchXMLCoordinateRegionContext : public SvXMLImportContext
{
public:
    SchXMLCoordinateRegionContext(
            SvXMLImport& rImport
            , sal_uInt16 nPrefix
            , const OUString& rLocalName
            , SchXMLPositionAttributesHelper& rPositioning );
    virtual ~SchXMLCoordinateRegionContext();
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

private:
    SchXMLPositionAttributesHelper& m_rPositioning;
};

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
    css::uno::Reference< css::chart::X3DDisplay > mxWallFloorSupplier;
    ContextType meContextType;

public:
    SchXMLWallFloorContext( SchXMLImportHelper& rImportHelper,
                            SvXMLImport& rImport,
                            sal_uInt16 nPrefix,
                            const OUString& rLocalName,
                            css::uno::Reference< css::chart::XDiagram >& xDiagram,
                            ContextType eContextType );
    virtual ~SchXMLWallFloorContext();
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
};

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
    css::uno::Reference< css::chart::XStatisticDisplay > mxStockPropProvider;
    ContextType meContextType;

public:
    SchXMLStockContext( SchXMLImportHelper& rImportHelper,
                        SvXMLImport& rImport,
                        sal_uInt16 nPrefix,
                        const OUString& rLocalName,
                        css::uno::Reference< css::chart::XDiagram >& xDiagram,
                        ContextType eContextType );
    virtual ~SchXMLStockContext();
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
};

class SchXMLStatisticsObjectContext : public SvXMLImportContext
{
public:
    enum ContextType
    {
        CONTEXT_TYPE_MEAN_VALUE_LINE,
        CONTEXT_TYPE_ERROR_INDICATOR
    };

    SchXMLStatisticsObjectContext(
        SchXMLImportHelper& rImportHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString &rSeriesStyleName,
        ::std::list< DataRowPointStyle >& rStyleList,
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        ContextType eContextType,
        tSchXMLLSequencesPerIndex & rLSequencesPerIndex );

    virtual ~SchXMLStatisticsObjectContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

private:
    SchXMLImportHelper &                           mrImportHelper;
    ::std::list< DataRowPointStyle > &             mrStyleList;
    css::uno::Reference< css::chart2::XDataSeries > m_xSeries;
    ContextType                                    meContextType;
    OUString maSeriesStyleName;
    tSchXMLLSequencesPerIndex& mrLSequencesPerIndex;
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPLOTAREACONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
