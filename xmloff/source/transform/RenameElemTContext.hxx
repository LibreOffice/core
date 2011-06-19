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

#ifndef _XMLOFF_RENAMEELEMTCONTEXT_HXX
#define _XMLOFF_RENAMEELEMTCONTEXT_HXX

#include <xmloff/xmltoken.hxx>
#include "TransformerContext.hxx"


class XMLRenameElemTransformerContext : public XMLTransformerContext
{
    ::rtl::OUString m_aElemQName;
    ::rtl::OUString m_aAttrQName;
    ::rtl::OUString m_aAttrValue;

public:
    TYPEINFO();

    // The following consutructor renames the element names "rQName"
    // to bPrefix/eToken
    XMLRenameElemTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken  );

    // The following consutructor renames the element names "rQName"
    // to bPrefix/eToken and adds an attribute nAPrefix/eAToken that has
    // the value eVToken.
    XMLRenameElemTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nAPrefix,
                              ::xmloff::token::XMLTokenEnum eAToken,
                              ::xmloff::token::XMLTokenEnum eVToken );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLRenameElemTransformerContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

#endif  //  _XMLOFF_RENAMEELEMCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
