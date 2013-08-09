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
#ifndef _SCH_XMLPLOTAREACONTEXT_HXX_
#define _SCH_XMLPLOTAREACONTEXT_HXX_

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
    SchXML3DSceneAttributesHelper( SvXMLImport& rImporter );
    virtual ~SchXML3DSceneAttributesHelper();

    void getCameraDefaultFromDiagram( const ::com::sun::star::uno::Reference< com::sun::star::chart::XDiagram >& xDiagram );

private:
    SchXML3DSceneAttributesHelper();
};

class SchXMLPositonAttributesHelper
{
public:
    SchXMLPositonAttributesHelper( SvXMLImport& rImporter );
    ~SchXMLPositonAttributesHelper();

    bool readPositioningAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue );
    void readAutomaticPositioningProperties( XMLPropStyleContext* pPropStyleContext, const SvXMLStylesContext* pStylesCtxt );

    bool hasPosSize() const;
    bool isAutomatic() const;
    ::com::sun::star::awt::Rectangle getRectangle() const;

private:
    bool hasSize() const;
    bool hasPosition() const;

    SvXMLImport& m_rImport;

    ::com::sun::star::awt::Point m_aPosition;
    ::com::sun::star::awt::Size m_aSize;

    bool m_bHasSizeWidth;
    bool m_bHasSizeHeight;
    bool m_bHasPositionX;
    bool m_bHasPositionY;
    sal_Bool m_bAutoSize;
    sal_Bool m_bAutoPosition;
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
                           sal_Bool & rAllRangeAddressesAvailable,
                           sal_Bool & rColHasLabels,
                           sal_Bool & rRowHasLabels,
                           ::com::sun::star::chart::ChartDataRowSource & rDataRowSource,
                           SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
                           const OUString& aChartTypeServiceName,
                           tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                           const ::com::sun::star::awt::Size & rChartSize );
    virtual ~SchXMLPlotAreaContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > mxDiagram;
    ::com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > mxNewDoc;
    ::std::vector< SchXMLAxis > maAxes;
    OUString& mrCategoriesAddress;
    SeriesDefaultsAndStyles& mrSeriesDefaultsAndStyles;
    sal_Int32 mnNumOfLinesProp;
    sal_Bool  mbStockHasVolume;
    sal_Int32 mnSeries;
    GlobalSeriesImportInfo m_aGlobalSeriesImportInfo;

    SchXML3DSceneAttributesHelper maSceneImportHelper;
    SchXMLPositonAttributesHelper m_aOuterPositioning;//including axes and axes titles
    SchXMLPositonAttributesHelper m_aInnerPositioning;//excluding axes and axes titles
    bool mbPercentStacked;
    bool m_bAxisPositionAttributeImported;
    OUString msAutoStyleName;
    const OUString& m_rXLinkHRefAttributeToIndicateDataProvider;
    OUString& mrChartAddress;
    bool& m_rbHasRangeAtPlotArea;
    sal_Bool & mrColHasLabels;
    sal_Bool & mrRowHasLabels;
    ::com::sun::star::chart::ChartDataRowSource & mrDataRowSource;
    OUString maChartTypeServiceName;

    tSchXMLLSequencesPerIndex & mrLSequencesPerIndex;

    bool mbGlobalChartTypeUsedBySeries;
    ::com::sun::star::awt::Size maChartSize;
};

class SchXMLDataPointContext : public SvXMLImportContext
{
private:
    ::std::list< DataRowPointStyle >& mrStyleList;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    sal_Int32& mrIndex;
    bool mbSymbolSizeForSeriesIsMissingInFile;

public:
    SchXMLDataPointContext(  SvXMLImport& rImport, const OUString& rLocalName,
                             ::std::list< DataRowPointStyle >& rStyleList,
                             const ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XDataSeries >& xSeries, sal_Int32& rIndex,
                                bool bSymbolSizeForSeriesIsMissingInFile );
    virtual ~SchXMLDataPointContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

class SchXMLCoordinateRegionContext : public SvXMLImportContext
{
public:
    SchXMLCoordinateRegionContext(
            SvXMLImport& rImport
            , sal_uInt16 nPrefix
            , const OUString& rLocalName
            , SchXMLPositonAttributesHelper& rPositioning );
    virtual ~SchXMLCoordinateRegionContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLPositonAttributesHelper& m_rPositioning;
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
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::X3DDisplay > mxWallFloorSupplier;
    ContextType meContextType;

public:
    SchXMLWallFloorContext( SchXMLImportHelper& rImportHelper,
                            SvXMLImport& rImport,
                            sal_uInt16 nPrefix,
                            const OUString& rLocalName,
                            ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >& xDiagram,
                            ContextType eContextType );
    virtual ~SchXMLWallFloorContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
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
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XStatisticDisplay > mxStockPropProvider;
    ContextType meContextType;

public:
    SchXMLStockContext( SchXMLImportHelper& rImportHelper,
                        SvXMLImport& rImport,
                        sal_uInt16 nPrefix,
                        const OUString& rLocalName,
                        ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >& xDiagram,
                        ContextType eContextType );
    virtual ~SchXMLStockContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
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
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries,
        ContextType eContextType,
        const ::com::sun::star::awt::Size & rChartSize,
        tSchXMLLSequencesPerIndex & rLSequencesPerIndex );

    virtual ~SchXMLStatisticsObjectContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLImportHelper &                           mrImportHelper;
    ::std::list< DataRowPointStyle > &             mrStyleList;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    ContextType                                    meContextType;
    ::com::sun::star::awt::Size                    maChartSize;
    OUString maSeriesStyleName;
    tSchXMLLSequencesPerIndex& mrLSequencesPerIndex;
};

#endif  // _SCH_XMLPLOTAREACONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
