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
#ifndef _SCH_XMLCHARTCONTEXT_HXX_
#define _SCH_XMLCHARTCONTEXT_HXX_

#include "xmlictxt.hxx"
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/drawing/XShape.hpp>

#include "transporttypes.hxx"
namespace com { namespace sun { namespace star {
    namespace chart {
        class XChartDocument;
        class XDiagram;
        struct ChartSeriesAddress;
    }
    namespace util {
        class XStringMapping;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    }}
    namespace drawing {
        class XShapes;
    }
}}}
namespace binfilter {

class SchXMLImport;
class SchXMLImportHelper;



// ----------------------------------------

class SchXMLChartContext : public SvXMLImportContext
{
private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    ::rtl::OUString maMainTitle, maSubTitle;
    ::com::sun::star::awt::Point maMainTitlePos, maSubTitlePos, maLegendPos;
    sal_Bool mbHasOwnTable;
    sal_Bool mbHasLegend;

    ::com::sun::star::uno::Sequence< ::com::sun::star::chart::ChartSeriesAddress > maSeriesAddresses;
    ::rtl::OUString msCategoriesAddress;
    ::rtl::OUString msChartAddress;
    ::rtl::OUString msTableNumberList;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxDrawPage;
    ::rtl::OUString msColTrans;
    ::rtl::OUString msRowTrans;

    ::com::sun::star::uno::Sequence< sal_Int32 > GetNumberSequenceFromString( const ::rtl::OUString& rStr );

public:
    SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const ::rtl::OUString& rLocalName );
    virtual ~SchXMLChartContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
            
private:
    /**	@descr	This method bundles some settings to the chart model and executes them with
            a locked controller.  This includes setting the draw page size and setting
            the chart type.
        @param	aChartSize	The size the draw page will be set to.
        @param	bDomainForDefaultDataNeeded	This flag indicates wether the chart's data set
            has to contain a domain value.
        @param	aServiceName The name of the service the diagram is initialized with.
        @param	bSetWitchData	Indicates wether the data set takes it's data series from
            rows or from columns.
    */
    void	InitChart	( ::com::sun::star::awt::Size aChartSize,
                        sal_Bool bDomainForDefaultDataNeeded,
                        ::rtl::OUString aServiceName,
                        sal_Bool bSetSwitchData);
};

// ----------------------------------------

class SchXMLTitleContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::rtl::OUString& mrTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxTitleShape;
    ::rtl::OUString msAutoStyleName;
    ::com::sun::star::awt::Point& mrPosition;

public:
    SchXMLTitleContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const ::rtl::OUString& rLocalName,
                        ::rtl::OUString& rTitle,
                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xTitleShape,
                        ::com::sun::star::awt::Point& rPosition );
    virtual ~SchXMLTitleContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLLegendContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::awt::Point& mrPosition;

public:
    SchXMLLegendContext( SchXMLImportHelper& rImpHelper,
                         SvXMLImport& rImport, const ::rtl::OUString& rLocalName,
                         ::com::sun::star::awt::Point& rPosition );
    virtual ~SchXMLLegendContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

}//end of namespace binfilter
#endif	// _SCH_XMLCHARTCONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
