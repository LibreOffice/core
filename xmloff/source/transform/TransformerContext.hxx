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

#ifndef _XMLOFF_TRANSFORMERCONTEXT_HXX
#define _XMLOFF_TRANSFORMERCONTEXT_HXX

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <tools/solar.h>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>
#include <tools/rtti.hxx>
#include <xmloff/xmltoken.hxx>

class SvXMLNamespaceMap;
class XMLTransformerBase;

class XMLTransformerContext : public ::salhelper::SimpleReferenceObject
{
    friend class XMLTransformerBase;

    XMLTransformerBase& m_rTransformer;

    ::rtl::OUString m_aQName;

    SvXMLNamespaceMap   *m_pRewindMap;

    SvXMLNamespaceMap  *GetRewindMap() const { return m_pRewindMap; }
    void SetRewindMap( SvXMLNamespaceMap *p ) { m_pRewindMap = p; }

protected:

    XMLTransformerBase& GetTransformer() { return m_rTransformer; }
    const XMLTransformerBase& GetTransformer() const { return m_rTransformer; }

    void SetQName( const ::rtl::OUString& rQName ) { m_aQName = rQName; }

public:
    TYPEINFO();

    const ::rtl::OUString& GetQName() const { return m_aQName; }
    sal_Bool HasQName( sal_uInt16 nPrefix,
                       ::xmloff::token::XMLTokenEnum eToken ) const;
    sal_Bool HasNamespace( sal_uInt16 nPrefix ) const;

    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLTransformerContext( XMLTransformerBase& rTransformer,
                        const ::rtl::OUString& rQName );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLTransformerContext();

    // Create a childs element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // StartElement is called after a context has been constructed and
    // before a elements context is parsed. It may be used for actions that
    // require virtual methods. The default is to do nothing.
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // EndElement is called before a context will be destructed, but
    // after a elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement();

    // This method is called for all characters that are contained in the
    // current element. The default is to ignore them.
    virtual void Characters( const ::rtl::OUString& rChars );

    // Is the current context a persistent one (i.e. one that saves is content
    // rather than exporting it directly?
    virtual sal_Bool IsPersistent() const;

    // Export the whole element. By default, nothing is done here
    virtual void Export();

    // Export the element content. By default, nothing is done here
    virtual void ExportContent();
};


#endif  //  _XMLOFF_TRANSFORMERCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
