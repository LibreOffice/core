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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_FLATTCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_FLATTCONTEXT_HXX

#include "PersAttrListTContext.hxx"


class XMLPersTextContentTContext : public XMLPersAttrListTContext
{
    OUString m_aCharacters;

public:
    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName );

    // attr list/text content persistence + renaming
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLPersTextContentTContext();

    // This method is called for all characters that are contained in the
    // current element.
    virtual void Characters( const OUString& rChars ) override;

    virtual void ExportContent() override;

    const OUString& GetTextContent() const { return m_aCharacters; }
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_FLATTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
