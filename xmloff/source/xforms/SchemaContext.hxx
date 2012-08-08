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

#ifndef _XMLOFF_SCHEMACONTEXT_HXX
#define _XMLOFF_SCHEMACONTEXT_HXX

#include "TokenContext.hxx"
#include <com/sun/star/uno/Reference.hxx>


//
// forward declarations
//

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
    namespace xforms { class XDataTypeRepository; }
} } }
namespace rtl { class OUString; }
class SvXMLImport;
class SvXMLImportContext;

/** import the data type declarations from an xsd:schema element */
class SchemaContext : public TokenContext
{
    com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository> mxRepository;

public:

    SchemaContext( SvXMLImport& rImport,
                   sal_uInt16 nPrfx,
                   const ::rtl::OUString& rLName,
                   const com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository>& rRepository );
    virtual ~SchemaContext();


    //
    // implement TokenContext methods:
    //

protected:

    virtual void HandleAttribute(
        sal_uInt16 nToken,
        const rtl::OUString& rValue );

    virtual SvXMLImportContext* HandleChild(
        sal_uInt16 nToken,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
