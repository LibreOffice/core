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

#ifndef _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX
#define _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX

#include <rtl/ref.hxx>

#include <vector>
#include "DeepTContext.hxx"

class XMLPersMixedContentTContext : public XMLPersElemContentTContext
{
public:
    TYPEINFO();

    // mixed content persistence only
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // mixed content persistence + attribute processing
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nActionMap );

    // mixed content persistence + renaming
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // mixed content persistence + renaming + attribute processing
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual ~XMLPersMixedContentTContext();
};

#endif  //  _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
