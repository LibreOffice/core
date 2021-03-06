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

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>
#include <memory>

class SvXMLNamespaceMap;
class XMLTransformerBase;

class XMLTransformerContext : public ::salhelper::SimpleReferenceObject
{
    friend class XMLTransformerBase;

    XMLTransformerBase& m_rTransformer;

    OUString m_aQName;

    std::unique_ptr<SvXMLNamespaceMap>   m_xRewindMap;

    std::unique_ptr<SvXMLNamespaceMap>  TakeRewindMap() { return std::move(m_xRewindMap); }
    void PutRewindMap( std::unique_ptr<SvXMLNamespaceMap> p ) { m_xRewindMap = std::move(p); }

protected:

    XMLTransformerBase& GetTransformer() { return m_rTransformer; }
    const XMLTransformerBase& GetTransformer() const { return m_rTransformer; }

    void SetQName( const OUString& rQName ) { m_aQName = rQName; }

public:
    const OUString& GetQName() const { return m_aQName; }
    bool HasQName( sal_uInt16 nPrefix,
                       ::xmloff::token::XMLTokenEnum eToken ) const;
    bool HasNamespace( sal_uInt16 nPrefix ) const;

    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLTransformerContext( XMLTransformerBase& rTransformer,
                        const OUString& rQName );

    // Create a children element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual rtl::Reference<XMLTransformerContext> CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

    // StartElement is called after a context has been constructed and
    // before an elements context is parsed. It may be used for actions that
    // require virtual methods. The default is to do nothing.
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

    // EndElement is called before a context will be destructed, but
    // after an elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement();

    // This method is called for all characters that are contained in the
    // current element. The default is to ignore them.
    virtual void Characters( const OUString& rChars );

    // Is the current context a persistent one (i.e. one that saves is content
    // rather than exporting it directly?
    virtual bool IsPersistent() const;

    // Export the whole element. By default, nothing is done here
    virtual void Export();

    // Export the element content. By default, nothing is done here
    virtual void ExportContent();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
