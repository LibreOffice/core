/*************************************************************************
 *
 *  $RCSfile: SchXMLChartContext.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:02 $
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
}}}}}

// ----------------------------------------

class SchXMLChartContext : public SvXMLImportContext
{
private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    sal_Int32 mnWidth, mnHeight;
    rtl::OUString maMainTitle, maSubTitle;
    sal_Bool mbHasOwnTable;
    rtl::OUString msAutoStyleName;

    com::sun::star::uno::Sequence< com::sun::star::chart::ChartSeriesAddress > maSeriesAddresses;
    rtl::OUString msCategoriesAddress;

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
};

// ----------------------------------------

class SchXMLTitleContext : public SvXMLImportContext
{
private:
    rtl::OUString& maTitle;

public:
    SchXMLTitleContext( SvXMLImport& rImport, const rtl::OUString& rLocalName,
                        rtl::OUString& aTitle );
    virtual ~SchXMLTitleContext();

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

public:
    SchXMLLegendContext( SchXMLImportHelper& rImpHelper,
                         SvXMLImport& rImport, const rtl::OUString& rLocalName );
    virtual ~SchXMLLegendContext();

    virtual void StartElement( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif  // _SCH_XMLCHARTCONTEXT_HXX_
