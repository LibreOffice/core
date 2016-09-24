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

#include "TransformerBase.hxx"
#include "PersMixedContentTContext.hxx"

#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

class XMLPersTextTContext_Impl : public XMLTransformerContext
{
    OUString m_aCharacters;

public:
    XMLPersTextTContext_Impl( XMLTransformerBase& rTransformer,
                           const OUString& rChars );
    virtual ~XMLPersTextTContext_Impl() override;

    virtual rtl::Reference<XMLTransformerContext> CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;
    virtual void Characters( const OUString& rChars ) override;

    virtual bool IsPersistent() const override;
    virtual void Export() override;
};

XMLPersTextTContext_Impl::XMLPersTextTContext_Impl(
        XMLTransformerBase& rImp,
        const OUString& rChars ) :
    XMLTransformerContext( rImp, OUString() ),
    m_aCharacters( rChars )
{
}

XMLPersTextTContext_Impl::~XMLPersTextTContext_Impl()
{
}

rtl::Reference<XMLTransformerContext> XMLPersTextTContext_Impl::CreateChildContext(
        sal_uInt16,
        const OUString&,
        const OUString&,
        const Reference< XAttributeList >& )
{
    OSL_ENSURE( false, "illegal call to CreateChildContext" );
    return {};
}

void XMLPersTextTContext_Impl::StartElement(
    const Reference< XAttributeList >& )
{
    OSL_ENSURE( false, "illegal call to StartElement" );
}

void XMLPersTextTContext_Impl::EndElement()
{
    OSL_ENSURE( false, "illegal call to EndElement" );
}

bool XMLPersTextTContext_Impl::IsPersistent() const
{
    return true;
}

void XMLPersTextTContext_Impl::Characters( const OUString& rChars )
{
    m_aCharacters += rChars;
}

void XMLPersTextTContext_Impl::Export()
{
    GetTransformer().GetDocHandler()->characters( m_aCharacters );
}

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLPersElemContentTContext( rImp, rQName )
{
}

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLPersElemContentTContext( rImp, rQName, nActionMap )
{
}

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLPersElemContentTContext( rImp, rQName, nPrefix, eToken )
{
}

XMLPersMixedContentTContext::XMLPersMixedContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLPersElemContentTContext( rImp, rQName, nPrefix, eToken, nActionMap )
{
}

XMLPersMixedContentTContext::~XMLPersMixedContentTContext()
{
}

void XMLPersMixedContentTContext::Characters( const OUString& rChars )
{
    AddContent( new XMLPersTextTContext_Impl( GetTransformer(), rChars ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
