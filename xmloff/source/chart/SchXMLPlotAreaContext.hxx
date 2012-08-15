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

// ----------------------------------------

class SchXMLPositonAttributesHelper
{
public:
    SchXMLPositonAttributesHelper( SvXMLImport& rImporter );
    ~SchXMLPositonAttributesHelper();

    bool readPositioningAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue );
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

// ----------------------------------------

class SchXMLPlotAreaContext : public SvXMLImportContext
{
public:
    SchXMLPlotAreaContext( SchXMLImportHelper& rImpHelper,
                           SvXMLImport& rImport, const rtl::OUString& rLocalName,
                           const rtl::OUString& rXLinkHRefAttributeToIndicateDataProvider,
                           ::rtl::OUString& rCategoriesAddress,
                           ::rtl::OUString& rChartAddress,
                           bool& bHasRangeAtPlotArea,
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
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > mxDiagram;
    ::com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > mxNewDoc;
    ::std::vector< SchXMLAxis > maAxes;
    rtl::OUString& mrCategoriesAddress;
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
    ::rtl::OUString msAutoStyleName;
    const ::rtl::OUString& m_rXLinkHRefAttributeToIndicateDataProvider;
    ::rtl::OUString& mrChartAddress;
    bool& m_rbHasRangeAtPlotArea;
    sal_Bool & mrColHasLabels;
    sal_Bool & mrRowHasLabels;
    ::com::sun::star::chart::ChartDataRowSource & mrDataRowSource;
    ::rtl::OUString maChartTypeServiceName;

    tSchXMLLSequencesPerIndex & mrLSequencesPerIndex;

    bool mbGlobalChartTypeUsedBySeries;
    ::com::sun::star::awt::Size maChartSize;
};

//----------------------------------------

class SchXMLDataPointContext : public SvXMLImportContext
{
private:
    ::std::list< DataRowPointStyle >& mrStyleList;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    sal_Int32& mrIndex;
    bool mbSymbolSizeForSeriesIsMissingInFile;

public:
    SchXMLDataPointContext(  SvXMLImport& rImport, const rtl::OUString& rLocalName,
                             ::std::list< DataRowPointStyle >& rStyleList,
                             const ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XDataSeries >& xSeries, sal_Int32& rIndex,
                                bool bSymbolSizeForSeriesIsMissingInFile );
    virtual ~SchXMLDataPointContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLCoordinateRegionContext : public SvXMLImportContext
{
public:
    SchXMLCoordinateRegionContext(
            SvXMLImport& rImport
            , sal_uInt16 nPrefix
            , const rtl::OUString& rLocalName
            , SchXMLPositonAttributesHelper& rPositioning );
    virtual ~SchXMLCoordinateRegionContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLPositonAttributesHelper& m_rPositioning;
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
        const rtl::OUString &rSeriesStyleName,
        ::std::list< DataRowPointStyle >& rStyleList,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries,
        ContextType eContextType,
        const ::com::sun::star::awt::Size & rChartSize );

    virtual ~SchXMLStatisticsObjectContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLImportHelper &                           mrImportHelper;
    ::std::list< DataRowPointStyle > &             mrStyleList;
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    ContextType                                    meContextType;
    ::com::sun::star::awt::Size                    maChartSize;
    rtl::OUString maSeriesStyleName;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
