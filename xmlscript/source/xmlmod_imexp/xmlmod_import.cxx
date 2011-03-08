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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"
#include "imp_share.hxx"

#include <osl/diagnose.h>

#include <rtl/ustrbuf.hxx>

#include <xmlscript/xml_import.hxx>
#include <comphelper/processfactory.hxx>


namespace xmlscript
{

//##################################################################################################

//__________________________________________________________________________________________________
Reference< xml::input::XElement > ModuleElement::getParent()
    throw (RuntimeException)
{
    return static_cast< xml::input::XElement * >( _pParent );
}
//__________________________________________________________________________________________________
OUString ModuleElement::getLocalName()
    throw (RuntimeException)
{
    return _aLocalName;
}
//__________________________________________________________________________________________________
sal_Int32 ModuleElement::getUid()
    throw (RuntimeException)
{
    return _pImport->XMLNS_SCRIPT_UID;
}
//__________________________________________________________________________________________________
Reference< xml::input::XAttributes > ModuleElement::getAttributes()
    throw (RuntimeException)
{
    return _xAttributes;
}
//__________________________________________________________________________________________________
void ModuleElement::ignorableWhitespace(
    OUString const & /*rWhitespaces*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // not used
}
//__________________________________________________________________________________________________
void ModuleElement::characters( OUString const & rChars )
    throw (xml::sax::SAXException, RuntimeException)
{
    _StrBuffer.append( rChars );
}
//__________________________________________________________________________________________________
void ModuleElement::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
void ModuleElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    _pImport->mrModuleDesc.aCode = _StrBuffer.makeStringAndClear();
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > ModuleElement::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("unexpected element!") ),
        Reference< XInterface >(), Any() );
}

//__________________________________________________________________________________________________
ModuleElement::ModuleElement(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ModuleElement * pParent, ModuleImport * pImport )
    SAL_THROW( () )
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
//__________________________________________________________________________________________________
ModuleElement::~ModuleElement()
    SAL_THROW( () )
{
    _pImport->release();

    if (_pParent)
    {
        _pParent->release();
    }

#if OSL_DEBUG_LEVEL > 1
    OString aStr( OUStringToOString( _aLocalName, RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "ModuleElement::~ModuleElement(): %s\n", aStr.getStr() );
#endif
}

//##################################################################################################

// XRoot

//______________________________________________________________________________
void ModuleImport::startDocument(
    Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, RuntimeException)
{
    XMLNS_SCRIPT_UID = xNamespaceMapping->getUidByUri(
        OUSTR(XMLNS_SCRIPT_URI) );
    XMLNS_LIBRARY_UID = xNamespaceMapping->getUidByUri(
        OUSTR(XMLNS_LIBRARY_URI) );
    XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri(
        OUSTR(XMLNS_XLINK_URI) );
}

//__________________________________________________________________________________________________
void ModuleImport::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // ignored
}
//__________________________________________________________________________________________________
void ModuleImport::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
void ModuleImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & /*xLocator*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > ModuleImport::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_SCRIPT_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // window
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("module") ))
    {
        mrModuleDesc.aName = xAttributes->getValueByUidName(
            XMLNS_SCRIPT_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("name") ) );
        mrModuleDesc.aLanguage = xAttributes->getValueByUidName(
            XMLNS_SCRIPT_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("language") ) );
        mrModuleDesc.aModuleType = xAttributes->getValueByUidName(
            XMLNS_SCRIPT_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("moduleType") ) );

        return new ModuleElement( rLocalName, xAttributes, 0, this );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "illegal root element (expected module) given: ") ) +
            rLocalName, Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
ModuleImport::~ModuleImport()
    SAL_THROW( () )
{
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "ModuleImport::~ModuleImport().\n" );
#endif
}

//##################################################################################################

Reference< xml::sax::XDocumentHandler >
SAL_CALL importScriptModule( ModuleDescriptor& rMod )
    SAL_THROW( (Exception) )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new ModuleImport( rMod ) ) );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
