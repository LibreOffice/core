/*************************************************************************
 *
 *  $RCSfile: SchXMLChartContext.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-05 10:55:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SCH_XMLCHARTCONTEXT_HXX_
#define _SCH_XMLCHARTCONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#include "transporttypes.hxx"

class SchXMLImport;
class SchXMLImportHelper;

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

// ----------------------------------------

class SchXMLChartContext : public SvXMLImportContext
{
private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    ::rtl::OUString maMainTitle, maSubTitle;
    com::sun::star::awt::Point maMainTitlePos, maSubTitlePos, maLegendPos;
    // #i51307# only set actual positions if the svg:x/y attributes have been read
    bool mbSetMainTitlePos;
    bool mbSetSubTitlePos;
    bool mbSetLegendPos;
    sal_Bool mbHasOwnTable;
    sal_Bool mbHasLegend;

    com::sun::star::uno::Sequence< com::sun::star::chart::ChartSeriesAddress > maSeriesAddresses;
    ::rtl::OUString msCategoriesAddress;
    ::rtl::OUString msChartAddress;
    ::rtl::OUString msTableNumberList;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxDrawPage;
    ::rtl::OUString msColTrans;
    ::rtl::OUString msRowTrans;

    ::com::sun::star::uno::Sequence< sal_Int32 > GetNumberSequenceFromString( const ::rtl::OUString& rStr );

public:
    SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const rtl::OUString& rLocalName );
    virtual ~SchXMLChartContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                                     com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    /** @descr  This method bundles some settings to the chart model and executes them with
            a locked controller.  This includes setting the draw page size and setting
            the chart type.
        @param  aChartSize  The size the draw page will be set to.
        @param  bDomainForDefaultDataNeeded This flag indicates wether the chart's data set
            has to contain a domain value.
        @param  aServiceName The name of the service the diagram is initialized with.
        @param  bSetWitchData   Indicates wether the data set takes it's data series from
            rows or from columns.
    */
    void    InitChart   (com::sun::star::awt::Size aChartSize,
                        sal_Bool bDomainForDefaultDataNeeded,
                        ::rtl::OUString aServiceName,
                        sal_Bool bSetSwitchData);
};

// ----------------------------------------

class SchXMLTitleContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    rtl::OUString& mrTitle;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxTitleShape;
    rtl::OUString msAutoStyleName;
    com::sun::star::awt::Point& mrPosition;
    bool & mrSetPosition;

public:
    SchXMLTitleContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const rtl::OUString& rLocalName,
                        rtl::OUString& rTitle,
                        com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xTitleShape,
                        com::sun::star::awt::Point& rPosition,
                        bool & rSetPosition );
    virtual ~SchXMLTitleContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                               com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLLegendContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    com::sun::star::awt::Point& mrPosition;
    bool & mrSetPosition;

public:
    SchXMLLegendContext( SchXMLImportHelper& rImpHelper,
                         SvXMLImport& rImport, const rtl::OUString& rLocalName,
                         com::sun::star::awt::Point& rPosition,
                        bool & rSetPosition );
    virtual ~SchXMLLegendContext();

    virtual void StartElement( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif  // _SCH_XMLCHARTCONTEXT_HXX_
