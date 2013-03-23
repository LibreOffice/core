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
#if 1

#include <rtl/ref.hxx>
#include "FamilyType.hxx"
#include "DeepTContext.hxx"


class XMLTransformerActions;

class XMLStyleOOoTContext : public XMLPersElemContentTContext
{
       XMLFamilyType m_eFamily;

    sal_Bool m_bPersistent;

public:
    TYPEINFO();

    XMLStyleOOoTContext( XMLTransformerBase& rTransformer,
                        const ::rtl::OUString& rQName,
                           XMLFamilyType eType,
                           sal_Bool bPersistent );
    XMLStyleOOoTContext( XMLTransformerBase& rTransformer,
                        const ::rtl::OUString& rQName,
                           XMLFamilyType eType,
                           sal_uInt16 nPrefix,
                           ::xmloff::token::XMLTokenEnum eToken,
                           sal_Bool bPersistent );

    virtual ~XMLStyleOOoTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual void Characters( const ::rtl::OUString& rChars );
    virtual sal_Bool IsPersistent() const;

    static XMLTransformerActions *CreateTransformerActions( sal_uInt16 nType );

};

#endif  //  _XMLOFF_STYLEOOOTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
