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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_CONTEXTS_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_CONTEXTS_HXX

#include "SchXMLImport.hxx"
#include "SchXMLTableContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmltkmap.hxx>

#include <xmloff/xmlmetai.hxx>

namespace com { namespace sun { namespace star { namespace xml { namespace sax {
        class XAttributeList;
}}}}}

/*
   These contexts are only needed by
   SchXMLImport not by the SchXMLImportHelper
   that is also used by other applications
*/

class SchXMLDocContext : public virtual SvXMLImportContext
{
protected:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLDocContext(
        SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLName );

    SchXMLDocContext(
        SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport,
        sal_Int32 nElement );

    virtual ~SchXMLDocContext() override;

    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

// context for flat file xml format
class SchXMLFlatDocContext_Impl
    : public SchXMLDocContext, public SvXMLMetaDocumentContext
{
public:
    SchXMLFlatDocContext_Impl(
        SchXMLImportHelper& i_rImpHelper,
        SchXMLImport& i_rImport,
        sal_Int32 i_nElement,
        const css::uno::Reference<css::document::XDocumentProperties>& i_xDocProps);

    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 i_nPrefix, const OUString& i_rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& i_xAttrList) override;

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class SchXMLBodyContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLBodyContext(
        SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLName );
    virtual ~SchXMLBodyContext() override;

    virtual void EndElement() override;
    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_CONTEXTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
