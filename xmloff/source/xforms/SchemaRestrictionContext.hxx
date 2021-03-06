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

#pragma once

#include "TokenContext.hxx"
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
    namespace beans { class XPropertySet; }
    namespace xforms { class XDataTypeRepository; }
}

class SvXMLImport;
class SvXMLImportContext;

/** import the xsd:restriction element */
class SchemaRestrictionContext : public TokenContext
{
    css::uno::Reference<css::xforms::XDataTypeRepository> mxRepository;
    css::uno::Reference<css::beans::XPropertySet> mxDataType;
    OUString const msTypeName;
    OUString msBaseName;

public:
    SchemaRestrictionContext( SvXMLImport& rImport,
                              css::uno::Reference<css::xforms::XDataTypeRepository> const & rRepository,
                              const OUString& sTypeName );

private:
    // create mxDataType (if not already present)
    void CreateDataType();

    // implement TokenContext methods:

    virtual void HandleAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter ) override;

    virtual SvXMLImportContext* HandleChild(
        sal_Int32 nElementToken,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
