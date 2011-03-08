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

#ifndef _XMLOFF_STYLEOOOTCONTEXT_HXX
#define _XMLOFF_STYLEOOOTCONTEXT_HXX

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
