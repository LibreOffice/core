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

#include "xmlictxt.hxx"
#include "shapeimport.hxx"

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/chart/ChartSeriesAddress.hpp>

#include <list>

#include "transporttypes.hxx"
namespace com { namespace sun { namespace star {
    namespace chart {
        class XDiagram;
        class X3DDisplay;
        class XStatisticDisplay;
    }
    namespace xml { namespace sax {
        class XAttributeList;
}}}}}
namespace binfilter {

class SvXMLImport;



// ----------------------------------------

namespace chartxml 
{
    
struct DataRowPointStyle
{
    enum StyleType
    {
        DATA_POINT,
        DATA_SERIES,
        MEAN_VALUE,
        REGRESSION,
        ERROR_INDICATOR
    };

    StyleType meType;
    sal_Int32 mnSeries;
    sal_Int32 mnIndex;
    sal_Int32 mnRepeat;
    ::rtl::OUString msStyleName;
    sal_Int32 mnAttachedAxis;

    DataRowPointStyle( StyleType eType,
                       sal_Int32 nSeries, sal_Int32 nIndex, sal_Int32 nRepeat, ::rtl::OUString sStyleName,
                       sal_Int32 nAttachedAxis = 0 ) :
            meType( eType ),
            mnSeries( nSeries ),
            mnIndex( nIndex ),
            mnRepeat( nRepeat ),
            msStyleName( sStyleName ),
            mnAttachedAxis( nAttachedAxis )
        {}
};

}	// namespace


// ----------------------------------------

class SchXMLPlotAreaContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > mxDiagram;
    std::vector< SchXMLAxis > maAxes;
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart::ChartSeriesAddress >& mrSeriesAddresses;
    ::rtl::OUString& mrCategoriesAddress;
    ::std::list< chartxml::DataRowPointStyle > maSeriesStyleList;
    sal_Int32 mnDomainOffset;
    sal_Int32 mnNumOfLines;
    sal_Bool  mbStockHasVolume;
    sal_Int32 mnSeries;
    sal_Int32 mnMaxSeriesLength;
    SdXML3DSceneAttributesHelper maSceneImportHelper;
    ::com::sun::star::awt::Size maSize;
    ::com::sun::star::awt::Point maPosition;
    ::rtl::OUString msAutoStyleName;
    ::rtl::OUString& mrChartAddress;
    ::rtl::OUString& mrTableNumberList;

public:
    SchXMLPlotAreaContext( SchXMLImportHelper& rImpHelper,
                           SvXMLImport& rImport, const ::rtl::OUString& rLocalName,
                           ::com::sun::star::uno::Sequence<
                               ::com::sun::star::chart::ChartSeriesAddress >& rSeriesAddresses,
                           ::rtl::OUString& rCategoriesAddress,
                           ::rtl::OUString& rChartAddress,
                           ::rtl::OUString& rTableNumberList );
    virtual ~SchXMLPlotAreaContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

// ----------------------------------------

class SchXMLAxisContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > mxDiagram;
    SchXMLAxis maCurrentAxis;
    std::vector< SchXMLAxis >& maAxes;
    ::rtl::OUString msAutoStyleName;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTitleShape();
    void CreateGrid( ::rtl::OUString sAutoStyleName, sal_Bool bIsMajor );

public:
    SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                       SvXMLImport& rImport, const ::rtl::OUString& rLocalName,
                       ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > xDiagram,
                       ::std::vector< SchXMLAxis >& aAxes );
    virtual ~SchXMLAxisContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLSeriesContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > mxDiagram;
    std::vector< SchXMLAxis >& mrAxes;
    ::com::sun::star::chart::ChartSeriesAddress& mrSeriesAddress;
    ::std::list< chartxml::DataRowPointStyle >& mrStyleList;
    sal_Int32 mnSeriesIndex;
    sal_Int32 mnDataPointIndex;
    sal_Int32& mrMaxSeriesLength;
    sal_Int32& mrDomainOffset;
    sal_Int32& mrNumOfLines;
    sal_Bool& mrStockHasVolume;
    SchXMLAxis* mpAttachedAxis;
    sal_Int32 mnAttachedAxis;
    ::rtl::OUString msAutoStyleName;

public:
    SchXMLSeriesContext( SchXMLImportHelper& rImpHelper,
                         SvXMLImport& rImport, const ::rtl::OUString& rLocalName,
                         ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >& xDiagram,
                         ::std::vector< SchXMLAxis >& rAxes,
                         ::com::sun::star::chart::ChartSeriesAddress& rSeriesAddress,
                         ::std::list< chartxml::DataRowPointStyle >& rStyleList,
                         sal_Int32 nSeriesIndex,
                         sal_Int32& rMaxSeriesLength,
                         sal_Int32& rDomainOffset,
                         sal_Int32& rNumOfLines,
                         sal_Bool&  rStockHasVolume );
    virtual ~SchXMLSeriesContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

//----------------------------------------

class SchXMLDataPointContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > mxDiagram;
    ::std::list< chartxml::DataRowPointStyle >& mrStyleList;
    sal_Int32 mnSeries;
    sal_Int32& mrIndex;

public:
    SchXMLDataPointContext(  SchXMLImportHelper& rImpHelper,
                             SvXMLImport& rImport, const ::rtl::OUString& rLocalName,
                             ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >& xDiagram,
                             ::std::list< chartxml::DataRowPointStyle >& rStyleList,
                             sal_Int32 nSeries, sal_Int32& rIndex );
    virtual ~SchXMLDataPointContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLCategoriesDomainContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::rtl::OUString& mrAddress;

public:
    SchXMLCategoriesDomainContext( SchXMLImportHelper& rImpHelper,
                                   SvXMLImport& rImport,
                                   sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   ::rtl::OUString& rAddress );
    virtual ~SchXMLCategoriesDomainContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
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
                            const ::rtl::OUString& rLocalName,
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
                        const ::rtl::OUString& rLocalName,
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
        const ::rtl::OUString& rLocalName,
        ::std::list< chartxml::DataRowPointStyle >& rStyleList,
        sal_Int32 nSeries,
        ContextType eContextType );

    virtual ~SchXMLStatisticsObjectContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLImportHelper &                           mrImportHelper;
    ::std::list< chartxml::DataRowPointStyle > & mrStyleList;
    sal_Int32                                      mnSeriesIndex;
    ContextType                                    meContextType;
};

}//end of namespace binfilter
#endif	// _SCH_XMLPLOTAREACONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
