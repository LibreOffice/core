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

#ifndef INCLUDED_XMLOFF_INC_METAIMPORTCOMPONENT_HXX
#define INCLUDED_XMLOFF_INC_METAIMPORTCOMPONENT_HXX

#include <xmloff/xmlimp.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>


class XMLMetaImportComponent : public SvXMLImport
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> mxDocProps;

public:
    // XMLMetaImportComponent() throw();
    XMLMetaImportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext
        ) throw();

    virtual ~XMLMetaImportComponent() throw();


protected:

    virtual SvXMLImportContext* CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


// global functions to support the component

::com::sun::star::uno::Sequence< OUString > SAL_CALL
    XMLMetaImportComponent_getSupportedServiceNames()
    throw();

OUString SAL_CALL XMLMetaImportComponent_getImplementationName()
    throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    XMLMetaImportComponent_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
