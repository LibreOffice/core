/*************************************************************************
 *
 *  $RCSfile: XMLChartPropertySetMapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-19 14:24:56 $
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
#ifndef _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_
#define _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_

#ifndef _UNIVERSALL_REFERENCE_HXX
#include "uniref.hxx"
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include "xmlexppr.hxx"
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include "xmlimppr.hxx"
#endif

namespace rtl { class OUString; }

extern const XMLPropertyMapEntry aXMLChartPropMap[];


// ----------------------------------------

class XMLChartPropHdlFactory : public XMLPropertyHandlerFactory
{
private:
    const XMLPropertyHandler* GetShapePropertyHandler( sal_Int32 nType ) const;

public:
    virtual ~XMLChartPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;
};

// ----------------------------------------

class XMLChartPropertySetMapper : public XMLPropertySetMapper
{
protected:
    virtual void ContextFilter(
        ::std::vector< XMLPropertyState >& rProperties,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > rPropSet ) const;

public:
    XMLChartPropertySetMapper();
    ~XMLChartPropertySetMapper();
};

// ----------------------------------------

class XMLChartExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    const rtl::OUString msCDATA;
    const rtl::OUString msTrue;
    const rtl::OUString msFalse;

    /// this method is called for every item that has the MID_FLAG_ELEMENT_EXPORT flag set
    virtual void handleElementItem(
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const XMLPropertyState& rProperty, const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap, sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties = 0,
        sal_uInt32 nIdx = 0  ) const;

    /// this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set
    virtual void handleSpecialItem(
        SvXMLAttributeList& rAttrList, const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter, const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties = 0,
        sal_uInt32 nIdx = 0  ) const;

public:
    XMLChartExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~XMLChartExportPropertyMapper();
};

// ----------------------------------------

class XMLChartImportPropertyMapper : public SvXMLImportPropertyMapper
{
private:
public:
    XMLChartImportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~XMLChartImportPropertyMapper();

    virtual sal_Bool handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const;

    virtual void finished(
        ::std::vector< XMLPropertyState >& rProperties ) const;
};

#endif  // _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_
