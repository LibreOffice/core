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
#pragma once

#include "SchXMLChartContext.hxx"
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/shapeimport.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include "transporttypes.hxx"

class SvXMLImport;

namespace com::sun::star {
    namespace chart {
        class XDiagram;
        class X3DDisplay;
        class XStatisticDisplay;
    }
    namespace chart2 {
        class XChartDocument;
    }
    namespace xml::sax {
        class XAttributeList;
    }
}

class SchXML3DSceneAttributesHelper : public SdXML3DSceneAttributesHelper
{
public:
    explicit SchXML3DSceneAttributesHelper( SvXMLImport& rImporter );
    virtual ~SchXML3DSceneAttributesHelper();

    void getCameraDefaultFromDiagram( const css::uno::Reference< css::chart::XDiagram >& xDiagram );
};

class SchXMLPositionAttributesHelper
{
public:
    explicit SchXMLPositionAttributesHelper( SvXMLImport& rImporter );
    ~SchXMLPositionAttributesHelper();

    void readPositioningAttribute( sal_Int32 nAttributeToken, std::string_view rValue );
    void readAutomaticPositioningProperties( XMLPropStyleContext const * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt );

    bool hasPosSize() const;
    bool isAutomatic() const;
    css::awt::Rectangle getRectangle() const { return css::awt::Rectangle( m_aPosition.X, m_aPosition.Y, m_aSize.Width, m_aSize.Height );}

private:
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
                           SvXMLImport& rImport,
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
    virtual ~SchXMLPlotAreaContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

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

class SchXMLDataLabelSpanContext: public SvXMLImportContext
{
private:
    ::std::vector<OUString>& mrLabels;
    OUStringBuffer maCharBuffer;
public:
    SchXMLDataLabelSpanContext( SvXMLImport& rImport, ::std::vector<OUString>& rLabels);
    virtual void SAL_CALL characters( const OUString& rChars ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SchXMLDataLabelParaContext: public SvXMLImportContext
{
private:
    ::std::vector<OUString>& mrLabels;
public:
    SchXMLDataLabelParaContext( SvXMLImport& rImport, ::std::vector<OUString>& rLabels);
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

class SchXMLDataLabelContext: public SvXMLImportContext
{
private:
    ::std::vector<OUString>& mrLabels;
    DataRowPointStyle& mrDataLabelStyle;
public:
    SchXMLDataLabelContext(SvXMLImport& rImport,
                            ::std::vector<OUString>& rLabels, DataRowPointStyle& rDataLabel);

    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

class SchXMLDataPointContext : public SvXMLImportContext
{
private:
    ::std::vector<DataRowPointStyle>& mrStyleVector;
    bool mbHasLabelParagraph = false;
    sal_Int32& mrIndex;
    DataRowPointStyle mDataPoint;
    // We let the data point manage the DataRowPointStyle-struct of its data label
    DataRowPointStyle mDataLabel;

public:
    SchXMLDataPointContext(  SvXMLImport& rImport,
                             ::std::vector< DataRowPointStyle >& rStyleVector,
                             const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
                             sal_Int32& rIndex,
                             bool bSymbolSizeForSeriesIsMissingInFile );
    virtual ~SchXMLDataPointContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SchXMLCoordinateRegionContext : public SvXMLImportContext
{
public:
    SchXMLCoordinateRegionContext(
            SvXMLImport& rImport
            , SchXMLPositionAttributesHelper& rPositioning );
    virtual ~SchXMLCoordinateRegionContext() override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

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
                            css::uno::Reference< css::chart::XDiagram > const & xDiagram,
                            ContextType eContextType );
    virtual ~SchXMLWallFloorContext() override;
    virtual void SAL_CALL startFastElement (sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs) override;
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
                        css::uno::Reference< css::chart::XDiagram > const & xDiagram,
                        ContextType eContextType );
    virtual ~SchXMLStockContext() override;
    virtual void SAL_CALL startFastElement (sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs) override;
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
        const OUString &rSeriesStyleName,
        ::std::vector< DataRowPointStyle >& rStyleVector,
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        ContextType eContextType,
        tSchXMLLSequencesPerIndex & rLSequencesPerIndex );

    virtual ~SchXMLStatisticsObjectContext() override;

    virtual void SAL_CALL startFastElement (sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs) override;

private:
    SchXMLImportHelper &                           mrImportHelper;
    ::std::vector< DataRowPointStyle > &           mrStyleVector;
    css::uno::Reference< css::chart2::XDataSeries > m_xSeries;
    ContextType                                    meContextType;
    OUString maSeriesStyleName;
    tSchXMLLSequencesPerIndex& mrLSequencesPerIndex;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
