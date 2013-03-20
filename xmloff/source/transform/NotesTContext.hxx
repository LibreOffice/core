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

#include "DeepTContext.hxx"

class XMLNotesTransformerContext : public XMLPersElemContentTContext
{
    sal_Bool m_bEndNote;
    sal_Bool m_bPersistent;
    ::xmloff::token::XMLTokenEnum m_eTypeToken;

public:
    TYPEINFO();

    XMLNotesTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              ::xmloff::token::XMLTokenEnum m_eToken,
                              sal_Bool bPersistent  );

    virtual ~XMLNotesTransformerContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual sal_Bool IsPersistent() const;
};

#endif  //  _XMLOFF_NOTESTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
