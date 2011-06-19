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

#ifndef _XMLOFF_FLATTCONTEXT_HXX
#define _XMLOFF_FLATTCONTEXT_HXX

#include "PersAttrListTContext.hxx"


class XMLPersTextContentTContext : public XMLPersAttrListTContext
{
    ::rtl::OUString m_aCharacters;

public:
    TYPEINFO();

    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // attr list/text content persistence + renaming
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLPersTextContentTContext();

    // This method is called for all characters that are contained in the
    // current element.
    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void ExportContent();

    const ::rtl::OUString& GetTextContent() const { return m_aCharacters; }
};

#endif  //  _XMLOFF_FLATTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
