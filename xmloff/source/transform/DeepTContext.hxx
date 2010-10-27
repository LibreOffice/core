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

#ifndef _XMLOFF_DEEPTCONTEXT_HXX
#define _XMLOFF_DEEPTCONTEXT_HXX

#include <rtl/ref.hxx>

#include <vector>
#include "PersAttrListTContext.hxx"
#include "TContextVector.hxx"

class XMLPersElemContentTContext : public XMLPersAttrListTContext
{
    XMLTransformerContextVector m_aChildContexts;

public:
    TYPEINFO();

    // element content persistence only
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // element content persistence + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nActionMap );

    // element content persistence + renaming
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // element content persistence + renaming + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    virtual ~XMLPersElemContentTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    void AddContent( XMLTransformerContext *pContent );

    sal_Bool HasElementContent() const
    {
        return static_cast<sal_Bool>( !m_aChildContexts.empty() );
    }

    virtual void ExportContent();
};

#endif  //  _XMLOFF_DEEPTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
