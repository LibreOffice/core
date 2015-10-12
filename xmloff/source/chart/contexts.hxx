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
    virtual ~SchXMLDocContext();

    TYPEINFO_OVERRIDE();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList ) override;
};

// context for flat file xml format
class SchXMLFlatDocContext_Impl
    : public SchXMLDocContext, public SvXMLMetaDocumentContext
{
public:
    SchXMLFlatDocContext_Impl(
        SchXMLImportHelper& i_rImpHelper,
        SchXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const OUString & i_rLName,
        const com::sun::star::uno::Reference<com::sun::star::document::XDocumentProperties>& i_xDocProps);

    virtual ~SchXMLFlatDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 i_nPrefix, const OUString& i_rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& i_xAttrList) override;
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
    virtual ~SchXMLBodyContext();

    virtual void EndElement() override;
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_CONTEXTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
