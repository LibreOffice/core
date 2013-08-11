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

#include "imp_share.hxx"
#include "xml_import.hxx"

namespace xmlscript
{

Reference< xml::input::XElement > ModuleElement::getParent()
    throw (RuntimeException)
{
    return static_cast< xml::input::XElement * >( _pParent );
}
OUString ModuleElement::getLocalName()
    throw (RuntimeException)
{
    return _aLocalName;
}
sal_Int32 ModuleElement::getUid()
    throw (RuntimeException)
{
    return _pImport->XMLNS_SCRIPT_UID;
}
Reference< xml::input::XAttributes > ModuleElement::getAttributes()
    throw (RuntimeException)
{
    return _xAttributes;
}

void ModuleElement::ignorableWhitespace(
    OUString const & /*rWhitespaces*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // not used
}

void ModuleElement::characters( OUString const & rChars )
    throw (xml::sax::SAXException, RuntimeException)
{
    _StrBuffer.append( rChars );
}

void ModuleElement::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

void ModuleElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    _pImport->mrModuleDesc.aCode = _StrBuffer.makeStringAndClear();
}

Reference< xml::input::XElement > ModuleElement::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException("unexpected element!", Reference< XInterface >(), Any() );
}

ModuleElement::ModuleElement(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ModuleElement * pParent, ModuleImport * pImport )
    SAL_THROW(())
    : _pImport( pImport )
    , _pParent( pParent )
    , _aLocalName( rLocalName )
    , _xAttributes( xAttributes )
{
    _pImport->acquire();

    if (_pParent)
    {
        _pParent->acquire();
    }
}

ModuleElement::~ModuleElement()
    SAL_THROW(())
{
    _pImport->release();

    if (_pParent)
    {
        _pParent->release();
    }

#if OSL_DEBUG_LEVEL > 1
    OString aStr( OUStringToOString( _aLocalName, RTL_TEXTENCODING_ASCII_US ) );
    SAL_INFO("xmlscript.xmlmod", "ModuleElement::~ModuleElement(): " << aStr.getStr() );
#endif
}

// XRoot

void ModuleImport::startDocument(
    Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, RuntimeException)
{
    XMLNS_SCRIPT_UID = xNamespaceMapping->getUidByUri( XMLNS_SCRIPT_URI );
    XMLNS_LIBRARY_UID = xNamespaceMapping->getUidByUri( XMLNS_LIBRARY_URI );
    XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri( XMLNS_XLINK_URI );
}

void ModuleImport::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // ignored
}

void ModuleImport::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

void ModuleImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & /*xLocator*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

Reference< xml::input::XElement > ModuleImport::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
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

        return new ModuleElement( rLocalName, xAttributes, 0, this );
    }
    else
    {
        throw xml::sax::SAXException("illegal root element (expected module) given: " + rLocalName, Reference< XInterface >(), Any() );
    }
}
ModuleImport::~ModuleImport()
    SAL_THROW(())
{
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("xmlscript.xmlmod", "ModuleImport::~ModuleImport()." );
#endif
}

Reference< xml::sax::XDocumentHandler >
SAL_CALL importScriptModule( ModuleDescriptor& rMod )
    SAL_THROW( (Exception) )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new ModuleImport( rMod ) ) );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
