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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_METATCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_METATCONTEXT_HXX

#include <rtl/ref.hxx>
#include <map>

#include "FlatTContext.hxx"

typedef ::std::multimap< OUString,
                    ::rtl::Reference< XMLPersTextContentTContext > > XMLMetaContexts_Impl;


class XMLMetaTransformerContext : public XMLTransformerContext
{
    XMLMetaContexts_Impl m_aContexts;

public:
    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLMetaTransformerContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLMetaTransformerContext() override;

    // Create a children element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual rtl::Reference<XMLTransformerContext> CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    // EndElement is called before a context will be destructed, but
    // after a elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement() override;

    // This method is called for all characters that are contained in the
    // current element. The default is to ignore them.
    virtual void Characters( const OUString& rChars ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_METATCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
