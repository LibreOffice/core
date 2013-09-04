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
#ifndef _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_
#define _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_

#include <xmloff/uniref.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlimp.hxx>

extern const XMLPropertyMapEntry aXMLChartPropMap[];

class SvXMLExport;

class XMLChartPropHdlFactory : public XMLPropertyHandlerFactory
{
private:
    const XMLPropertyHandler* GetShapePropertyHandler( sal_Int32 nType ) const;

public:
    virtual ~XMLChartPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;
};

class XMLChartPropertySetMapper : public XMLPropertySetMapper
{
public:
    XMLChartPropertySetMapper();
    ~XMLChartPropertySetMapper();
};

class XMLChartExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    const OUString msTrue;
    const OUString msFalse;
    bool mbAdaptPercentage;

    SvXMLExport& mrExport;
    com::sun::star::uno::Reference< com::sun::star::chart2::XChartDocument > mxChartDoc;

protected:
    virtual void ContextFilter(
        bool bEnableFoFontFamily,
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

    void setAdaptPercentage( bool bNewValue );

    void setChartDoc( com::sun::star::uno::Reference<
            com::sun::star::chart2::XChartDocument > xChartDoc );
};

class XMLChartImportPropertyMapper : public SvXMLImportPropertyMapper
{
private:
    SvXMLImport& mrImport;

public:
    XMLChartImportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper,
                                  const SvXMLImport& rImport );
    virtual ~XMLChartImportPropertyMapper();

    virtual bool handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const;

    virtual void finished(
        ::std::vector< XMLPropertyState >& rProperties,
        sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;
};

#endif  // _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
