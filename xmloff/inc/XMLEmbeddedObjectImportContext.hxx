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

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlictxt.hxx>

namespace com { namespace sun { namespace star { namespace lang {
    class XComponent; } } } }

class XMLEmbeddedObjectImportContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler > xHandler;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XComponent > xComp;

    ::rtl::OUString sFilterService;
    // #100592#
    ::rtl::OUString sCLSID;

public:
    TYPEINFO();

    const ::rtl::OUString& GetFilterServiceName() const { return sFilterService; }
    // #100592#
    const ::rtl::OUString& GetFilterCLSID() const { return sCLSID; }

    XMLEmbeddedObjectImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual ~XMLEmbeddedObjectImportContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );

    sal_Bool SetComponent(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& rComp );

};

#endif  //  _XMLOFF_XMLEMBEDDEDOBJECTIMPORTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
