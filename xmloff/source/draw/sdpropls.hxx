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

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_SDPROPLS_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_SDPROPLS_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <xmloff/xmlnume.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmlsdtypes.hxx>

// entry list for graphic properties

extern const XMLPropertyMapEntry aXMLSDProperties[];

// entry list for presentation page properties

extern const XMLPropertyMapEntry aXMLSDPresPageProps[];
extern const XMLPropertyMapEntry aXMLSDPresPageProps_onlyHeadersFooter[];

// enum maps for attributes

extern SvXMLEnumMapEntry const aXML_ConnectionKind_EnumMap[];
extern SvXMLEnumMapEntry const aXML_CircleKind_EnumMap[];

/** contains the attribute to property mapping for a drawing layer table */
extern const XMLPropertyMapEntry aXMLTableShapeAttributes[];

// factory for own graphic properties

class SvXMLExport;
class SvXMLImport;

class XMLSdPropHdlFactory : public XMLPropertyHandlerFactory
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    SvXMLExport* mpExport;
    SvXMLImport* mpImport;

public:
    XMLSdPropHdlFactory( css::uno::Reference< css::frame::XModel >, SvXMLExport& rExport );
    XMLSdPropHdlFactory( css::uno::Reference< css::frame::XModel >, SvXMLImport& rImport );
    virtual ~XMLSdPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const override;
};

class XMLShapePropertySetMapper : public XMLPropertySetMapper
{
public:
    XMLShapePropertySetMapper(const rtl::Reference< XMLPropertyHandlerFactory >& rFactoryRef, bool bForExport);
    virtual ~XMLShapePropertySetMapper();
};

class XMLShapeExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    SvxXMLNumRuleExport maNumRuleExp;
    bool mbIsInAutoStyles;

protected:
    virtual void ContextFilter(
        bool bEnableFoFontFamily,
        ::std::vector< XMLPropertyState >& rProperties,
        css::uno::Reference< css::beans::XPropertySet > rPropSet ) const override;
public:
    XMLShapeExportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport );
    virtual ~XMLShapeExportPropertyMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            SvXmlExportFlags nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = nullptr,
                            sal_uInt32 nIdx = 0
                            ) const override;

    void SetAutoStyles( bool bIsInAutoStyles ) { mbIsInAutoStyles = bIsInAutoStyles; }

    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = nullptr,
            sal_uInt32 nIdx = 0 ) const override;
};

class XMLPageExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    SvXMLExport& mrExport;

protected:
    virtual void ContextFilter(
        bool bEnableFoFontFamily,
        ::std::vector< XMLPropertyState >& rProperties,
        css::uno::Reference< css::beans::XPropertySet > rPropSet ) const override;
public:
    XMLPageExportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport );
    virtual ~XMLPageExportPropertyMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            SvXmlExportFlags nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = nullptr,
                            sal_uInt32 nIdx = 0
                            ) const override;
};

#endif // INCLUDED_XMLOFF_SOURCE_DRAW_SDPROPLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
