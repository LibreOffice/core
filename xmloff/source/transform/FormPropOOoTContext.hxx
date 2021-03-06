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

#include <rtl/ref.hxx>
#include "TransformerContext.hxx"

class XMLFormPropValueTContext_Impl;

class XMLFormPropOOoTransformerContext : public XMLTransformerContext
{
    css::uno::Reference< css::xml::sax::XAttributeList > m_xAttrList;

    ::rtl::Reference< XMLFormPropValueTContext_Impl > m_xValueContext;

    OUString m_aElemQName;

    sal_Int16 m_nValueTypeAttr;

    ::xmloff::token::XMLTokenEnum m_eValueToken;
    ::xmloff::token::XMLTokenEnum m_eValueTypeToken;

    bool m_bIsList;

public:
    XMLFormPropOOoTransformerContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName );

    virtual ~XMLFormPropOOoTransformerContext() override;

    virtual rtl::Reference<XMLTransformerContext> CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
