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

#ifndef INCLUDED_XMLOFF_TABLE_XMLTABLEIMPORT_HXX
#define INCLUDED_XMLOFF_TABLE_XMLTABLEIMPORT_HXX

#include <com/sun/star/table/XColumnRowRange.hpp>

#include <xmloff/dllapi.h>
#include <xmloff/xmlictxt.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/prhdlfac.hxx>

#include <rtl/ref.hxx>

#include <map>
#include <memory>

class SvXMLStyleContext;

typedef std::map< OUString, OUString > XMLTableTemplate;
typedef std::map < OUString, std::shared_ptr< XMLTableTemplate > > XMLTableTemplateMap;

class XMLTableImport : public salhelper::SimpleReferenceObject
{
    friend class XMLTableImportContext;

public:
    XMLTableImport( SvXMLImport& rImport, const rtl::Reference< XMLPropertySetMapper >& xCellPropertySetMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef );
    virtual ~XMLTableImport();

    SvXMLImportContext* CreateTableContext( sal_uInt16 nPrfx, const OUString& rLName,
                                            css::uno::Reference< css::table::XColumnRowRange >& xColumnRowRange );

    SvXMLStyleContext* CreateTableTemplateContext( sal_uInt16 nPrfx, const OUString& rLName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

    rtl::Reference< SvXMLImportPropertyMapper > GetCellImportPropertySetMapper() const { return mxCellImportPropertySetMapper; }
    rtl::Reference< SvXMLImportPropertyMapper > GetRowImportPropertySetMapper() const { return mxRowImportPropertySetMapper; }
    rtl::Reference< SvXMLImportPropertyMapper > GetColumnImportPropertySetMapper() const { return mxColumnImportPropertySetMapper; }

    void addTableTemplate( const OUString& rsStyleName, XMLTableTemplate& xTableTemplate );
    void finishStyles();

private:
    SvXMLImport&                                 mrImport;
    rtl::Reference< SvXMLImportPropertyMapper > mxCellImportPropertySetMapper;
    rtl::Reference< SvXMLImportPropertyMapper > mxRowImportPropertySetMapper;
    rtl::Reference< SvXMLImportPropertyMapper > mxColumnImportPropertySetMapper;

    XMLTableTemplateMap                         maTableTemplates;
};

#endif // INCLUDED_XMLOFF_TABLE_XMLTABLEIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
