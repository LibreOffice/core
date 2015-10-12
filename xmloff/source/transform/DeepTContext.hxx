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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_DEEPTCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_DEEPTCONTEXT_HXX

#include <rtl/ref.hxx>

#include <vector>
#include "PersAttrListTContext.hxx"
#include "TContextVector.hxx"

class XMLPersElemContentTContext : public XMLPersAttrListTContext
{
    XMLTransformerContextVector m_aChildContexts;

public:
    // element content persistence only
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName );

    // element content persistence + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName,
                              sal_uInt16 nActionMap );

    // element content persistence + renaming
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // element content persistence + renaming + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    virtual ~XMLPersElemContentTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) override;

    void AddContent( XMLTransformerContext *pContent );

    bool HasElementContent() const
    {
        return !m_aChildContexts.empty();
    }

    virtual void ExportContent() override;
};

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_DEEPTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
