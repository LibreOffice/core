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

#ifndef _XMLOFF_EVENTOOOTCONTEXT_HXX
#define _XMLOFF_EVENTOOOTCONTEXT_HXX

#include "DeepTContext.hxx"

class XMLTransformerOOoEventMap_Impl;

class XMLEventOOoTransformerContext : public XMLPersElemContentTContext
{
    sal_Bool m_bPersistent;

public:
    TYPEINFO();

    XMLEventOOoTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_Bool bPersistent = sal_False  );
    virtual ~XMLEventOOoTransformerContext();

    static XMLTransformerOOoEventMap_Impl *CreateEventMap();
    static void FlushEventMap( XMLTransformerOOoEventMap_Impl *p );
    static sal_uInt16 GetEventName( const ::rtl::OUString& rName,
                                    ::rtl::OUString& rNewName,
                                       XMLTransformerOOoEventMap_Impl& rMap );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual sal_Bool IsPersistent() const;
};

#endif  //  _XMLOFF_EVENTOOOTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
