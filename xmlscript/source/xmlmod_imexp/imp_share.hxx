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

#ifndef INCLUDED_XMLSCRIPT_SOURCE_XMLMOD_IMEXP_IMP_SHARE_HXX
#define INCLUDED_XMLSCRIPT_SOURCE_XMLMOD_IMEXP_IMP_SHARE_HXX

#include <xmlscript/xmlmod_imexp.hxx>

#include <cppuhelper/implbase.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <com/sun/star/xml/input/XRoot.hpp>

#include <vector>

namespace xmlscript
{

// Script module import

struct ModuleImport
    : public ::cppu::WeakImplHelper< css::xml::input::XRoot >
{
    friend class ModuleElement;

    ModuleDescriptor& mrModuleDesc;

    sal_Int32 XMLNS_SCRIPT_UID;
    sal_Int32 XMLNS_LIBRARY_UID;
    sal_Int32 XMLNS_XLINK_UID;

public:
    explicit ModuleImport(ModuleDescriptor& rModuleDesc)
        : mrModuleDesc(rModuleDesc)
        , XMLNS_SCRIPT_UID(0)
        , XMLNS_LIBRARY_UID(0)
        , XMLNS_XLINK_UID(0)
    {
    }

    virtual ~ModuleImport();

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference< css::xml::input::XNamespaceMapping > const & xNamespaceMapping )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference< css::xml::sax::XLocator > const & xLocator )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startRootElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
};

class ModuleElement
    : public ::cppu::WeakImplHelper< css::xml::input::XElement >
{
protected:
    rtl::Reference<ModuleImport> mxImport;
    rtl::Reference<ModuleElement> mxParent;

    OUString _aLocalName;
    css::uno::Reference< css::xml::input::XAttributes > _xAttributes;
    OUStringBuffer _strBuffer;

public:
    ModuleElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ModuleElement * pParent, ModuleImport * pImport );
    virtual ~ModuleElement();

    // XElement
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL getParent()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLocalName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getUid()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::input::XAttributes > SAL_CALL getAttributes()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( OUString const & rChars )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
};

}

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLMOD_IMEXP_IMP_SHARE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
