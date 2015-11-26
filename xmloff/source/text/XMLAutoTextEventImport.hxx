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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLAUTOTEXTEVENTIMPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLAUTOTEXTEVENTIMPORT_HXX

#include <xmloff/xmlimp.hxx>
#include <com/sun/star/uno/Reference.hxx>


namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace lang { class XMultiServiceFactory; }
    namespace text { class XAutoTextContainer; }
    namespace text { class XAutoTextGroup; }
    namespace text { class XAutoTextEntry; }
    namespace uno { template<class X> class Reference; }
    namespace uno { template<class X> class Sequence; }
    namespace uno { class XInterface; }
    namespace uno { class Exception; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }


class XMLAutoTextEventImport : public SvXMLImport
{
    css::uno::Reference<css::container::XNameReplace> xEvents;

public:
    explicit XMLAutoTextEventImport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext
        ) throw();

    virtual ~XMLAutoTextEventImport() throw();

    // XInitialization
    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any> & rArguments )
        throw(
            css::uno::Exception,
            css::uno::RuntimeException, std::exception) override;

protected:

    virtual SvXMLImportContext* CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

};


// global functions to support the component

css::uno::Sequence< OUString > SAL_CALL
    XMLAutoTextEventImport_getSupportedServiceNames()
    throw();

OUString SAL_CALL XMLAutoTextEventImport_getImplementationName()
    throw();

css::uno::Reference< css::uno::XInterface > SAL_CALL
    XMLAutoTextEventImport_createInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory > & )
    throw( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
