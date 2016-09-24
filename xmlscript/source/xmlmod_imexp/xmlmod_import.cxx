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

#include <sal/config.h>

#include <sal/log.hxx>

#include "imp_share.hxx"
#include "xml_import.hxx"

using namespace css;
using namespace css::uno;

namespace xmlscript
{

Reference< xml::input::XElement > ModuleElement::getParent()
    throw (RuntimeException, std::exception)
{
    return nullptr;
}
OUString ModuleElement::getLocalName()
    throw (RuntimeException, std::exception)
{
    return _aLocalName;
}
sal_Int32 ModuleElement::getUid()
    throw (RuntimeException, std::exception)
{
    return mxImport->XMLNS_SCRIPT_UID;
}
Reference< xml::input::XAttributes > ModuleElement::getAttributes()
    throw (RuntimeException, std::exception)
{
    return _xAttributes;
}

void ModuleElement::ignorableWhitespace(
    OUString const & /*rWhitespaces*/ )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
    // not used
}

void ModuleElement::characters( OUString const & rChars )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
    _strBuffer.append( rChars );
}

void ModuleElement::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
}

void ModuleElement::endElement()
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
    mxImport->mrModuleDesc.aCode = _strBuffer.makeStringAndClear();
}

Reference< xml::input::XElement > ModuleElement::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
    throw xml::sax::SAXException("unexpected element!", Reference< XInterface >(), Any() );
}

ModuleElement::ModuleElement(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ModuleImport * pImport )
    : mxImport( pImport )
    , _aLocalName( rLocalName )
    , _xAttributes( xAttributes )
{
}

ModuleElement::~ModuleElement()
{
    SAL_INFO("xmlscript.xmlmod", "ModuleElement::~ModuleElement(): " << _aLocalName );
}

// XRoot

void ModuleImport::startDocument(
    Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
    XMLNS_SCRIPT_UID = xNamespaceMapping->getUidByUri( XMLNS_SCRIPT_URI );
    XMLNS_LIBRARY_UID = xNamespaceMapping->getUidByUri( XMLNS_LIBRARY_URI );
    XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri( XMLNS_XLINK_URI );
}

void ModuleImport::endDocument()
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
    // ignored
}

void ModuleImport::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
}

void ModuleImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & /*xLocator*/ )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
}

Reference< xml::input::XElement > ModuleImport::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException, std::exception)
{
    if (XMLNS_SCRIPT_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // window
    else if ( rLocalName == "module" )
    {
        mrModuleDesc.aName = xAttributes->getValueByUidName( XMLNS_SCRIPT_UID, "name" );
        mrModuleDesc.aLanguage = xAttributes->getValueByUidName( XMLNS_SCRIPT_UID, "language" );
        mrModuleDesc.aModuleType = xAttributes->getValueByUidName( XMLNS_SCRIPT_UID, "moduleType" );

        return new ModuleElement( rLocalName, xAttributes, this );
    }
    else
    {
        throw xml::sax::SAXException("illegal root element (expected module) given: " + rLocalName, Reference< XInterface >(), Any() );
    }
}

ModuleImport::~ModuleImport()
{
    SAL_INFO("xmlscript.xmlmod", "ModuleImport::~ModuleImport()." );
}

Reference< xml::sax::XDocumentHandler >
SAL_CALL importScriptModule( ModuleDescriptor& rMod )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new ModuleImport( rMod ) ) );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
