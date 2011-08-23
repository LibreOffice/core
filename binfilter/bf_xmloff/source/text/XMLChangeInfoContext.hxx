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


#ifndef _XMLOFF_XMLCHANGEINFOCONTEXT_HXX
#define _XMLOFF_XMLCHANGEINFOCONTEXT_HXX

#include "xmlictxt.hxx"

#include <com/sun/star/uno/Reference.h>

#include <rtl/ustring.hxx>

#include <rtl/ustrbuf.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace binfilter {
class XMLChangedRegionImportContext;



/**
 * Import <office:change-info> elements as children of <text:changed-region>
 * elements. The attribute values will be passed to the enclosing
 * XMLChangedRegionImportContext (which has to be passed down in the
 * constructor).
 */
class XMLChangeInfoContext : public SvXMLImportContext
{
    const ::rtl::OUString& rType;

    ::rtl::OUString sAuthor;
    ::rtl::OUString sDateTime;
    ::rtl::OUStringBuffer sCommentBuffer;

    XMLChangedRegionImportContext& rChangedRegion;

public:

    TYPEINFO();

    XMLChangeInfoContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        XMLChangedRegionImportContext& rChangedRegion,
        const ::rtl::OUString& rChangeType);

    ~XMLChangeInfoContext();

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
