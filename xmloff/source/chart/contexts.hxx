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
#ifndef _SCH_XML_CONTEXTS_HXX_
#define _SCH_XML_CONTEXTS_HXX_

#include "SchXMLImport.hxx"
#include "SchXMLTableContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmltkmap.hxx>

#include <xmloff/xmlmetai.hxx>

namespace com { namespace sun { namespace star { namespace xml { namespace sax {
        class XAttributeList;
}}}}}

/* ========================================

   These contexts are only nedded by
   SchXMLImport not by the SchXMLImportHelper
   that is also used by other applications

   ======================================== */

class SchXMLDocContext : public virtual SvXMLImportContext
{
protected:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLDocContext(
        SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLName );
    virtual ~SchXMLDocContext();

    TYPEINFO();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ========================================

// context for flat file xml format
class SchXMLFlatDocContext_Impl
    : public SchXMLDocContext, public SvXMLMetaDocumentContext
{
public:
    SchXMLFlatDocContext_Impl(
        SchXMLImportHelper& i_rImpHelper,
        SchXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const ::rtl::OUString & i_rLName,
        const com::sun::star::uno::Reference<com::sun::star::document::XDocumentProperties>& i_xDocProps,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XDocumentHandler>& i_xDocBuilder);

    virtual ~SchXMLFlatDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 i_nPrefix, const ::rtl::OUString& i_rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& i_xAttrList);
};

// ========================================

class SchXMLBodyContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLBodyContext(
        SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const rtl::OUString& rLName );
    virtual ~SchXMLBodyContext();

    virtual void EndElement();
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ========================================

#endif  // _SCH_XML_CONTEXTS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
