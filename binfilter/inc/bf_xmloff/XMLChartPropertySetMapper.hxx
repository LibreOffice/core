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
#ifndef _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_
#define _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_

#include "uniref.hxx"
#include "xmlprmap.hxx"
#include "xmlexppr.hxx"
#include "xmlimppr.hxx"
#include "xmlimp.hxx"

namespace rtl { class OUString; }
namespace binfilter {

extern const XMLPropertyMapEntry aXMLChartPropMap[];

class SvXMLExport;

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
public:
    XMLChartPropertySetMapper();
    ~XMLChartPropertySetMapper();
};

// ----------------------------------------

class XMLChartExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    const ::rtl::OUString msTrue;
    const ::rtl::OUString msFalse;

    SvXMLExport& mrExport;

protected:
    virtual void ContextFilter(
        ::std::vector< XMLPropertyState >& rProperties,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > rPropSet ) const;

private:
    /// this method is called for every item that has the MID_FLAG_ELEMENT_EXPORT flag set
    virtual void handleElementItem(
        SvXMLExport& rExport,
        const XMLPropertyState& rProperty, sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties = 0,
        sal_uInt32 nIdx = 0  ) const;

    /// this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set
    virtual void handleSpecialItem(
        SvXMLAttributeList& rAttrList, const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter, const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties = 0,
        sal_uInt32 nIdx = 0  ) const;

public:
    XMLChartExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper,
                                  SvXMLExport& rExport );
    virtual ~XMLChartExportPropertyMapper();
};

// ----------------------------------------

class XMLChartImportPropertyMapper : public SvXMLImportPropertyMapper
{
private:
    SvXMLImport& mrImport;

public:
    XMLChartImportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper,
                                  const SvXMLImport& rImport );
    virtual ~XMLChartImportPropertyMapper();

    virtual sal_Bool handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const;

    virtual void finished(
        ::std::vector< XMLPropertyState >& rProperties,
        sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;
};

}//end of namespace binfilter
#endif	// _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
