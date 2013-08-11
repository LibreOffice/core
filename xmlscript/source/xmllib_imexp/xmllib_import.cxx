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

Reference< xml::input::XElement > LibElementBase::getParent()
    throw (RuntimeException)
{
    return static_cast< xml::input::XElement * >( _pParent );
}

OUString LibElementBase::getLocalName()
    throw (RuntimeException)
{
    return _aLocalName;
}

sal_Int32 LibElementBase::getUid()
    throw (RuntimeException)
{
    return _pImport->XMLNS_LIBRARY_UID;
}

Reference< xml::input::XAttributes > LibElementBase::getAttributes()
    throw (RuntimeException)
{
    return _xAttributes;
}

void LibElementBase::ignorableWhitespace(
    OUString const & /*rWhitespaces*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

void LibElementBase::characters( OUString const & /*rChars*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // not used, all characters ignored
}

void LibElementBase::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

void LibElementBase::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
}
Reference< xml::input::XElement > LibElementBase::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException("unexpected element!", Reference< XInterface >(), Any() );
}

LibElementBase::LibElementBase(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    LibElementBase * pParent, LibraryImport * pImport )
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

LibElementBase::~LibElementBase()
    SAL_THROW(())
{
    _pImport->release();

    if (_pParent)
    {
        _pParent->release();
    }

#if OSL_DEBUG_LEVEL > 1
    OString aStr( OUStringToOString( _aLocalName, RTL_TEXTENCODING_ASCII_US ) );
    SAL_INFO("xmlscript.xmllib", "LibElementBase::~LibElementBase(): " << aStr.getStr() );
#endif
}

// XRoot

void LibraryImport::startDocument(
    Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, RuntimeException)
{
    XMLNS_LIBRARY_UID = xNamespaceMapping->getUidByUri( XMLNS_LIBRARY_URI );
    XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri( XMLNS_XLINK_URI );
}

void LibraryImport::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
}

void LibraryImport::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

void LibraryImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & /*xLocator*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

Reference< xml::input::XElement > LibraryImport::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    else if ( mpLibArray && rLocalName == "libraries" )
    {
        return new LibrariesElement( rLocalName, xAttributes, 0, this );
    }
    else if ( mpLibDesc && rLocalName == "library" )
    {
        LibDescriptor& aDesc = *mpLibDesc;
        aDesc.bLink = aDesc.bReadOnly = aDesc.bPasswordProtected = aDesc.bPreload = sal_False;

        aDesc.aName = xAttributes->getValueByUidName(XMLNS_LIBRARY_UID, "name" );
        getBoolAttr( &aDesc.bReadOnly, "readonly", xAttributes, XMLNS_LIBRARY_UID );
        getBoolAttr( &aDesc.bPasswordProtected, "passwordprotected", xAttributes, XMLNS_LIBRARY_UID );
        getBoolAttr( &aDesc.bPreload, "preload", xAttributes, XMLNS_LIBRARY_UID );

        return new LibraryElement( rLocalName, xAttributes, 0, this );
    }
    else
    {
        throw xml::sax::SAXException( "illegal root element (expected libraries) given: " + rLocalName, Reference< XInterface >(), Any() );
    }
}

LibraryImport::~LibraryImport()
    SAL_THROW(())
{
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("xmlscript.xmllib", "LibraryImport::~LibraryImport()." );
#endif
}

// libraries
Reference< xml::input::XElement > LibrariesElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // library
    else if ( rLocalName == "library" )
    {
        LibDescriptor aDesc;
        aDesc.bLink = aDesc.bReadOnly = aDesc.bPasswordProtected = aDesc.bPreload = sal_False;

        aDesc.aName = xAttributes->getValueByUidName(_pImport->XMLNS_LIBRARY_UID, "name" );
        aDesc.aStorageURL = xAttributes->getValueByUidName( _pImport->XMLNS_XLINK_UID, "href" );
        getBoolAttr(&aDesc.bLink, "link", xAttributes, _pImport->XMLNS_LIBRARY_UID );
        getBoolAttr(&aDesc.bReadOnly, "readonly", xAttributes, _pImport->XMLNS_LIBRARY_UID );
        getBoolAttr(&aDesc.bPasswordProtected, "passwordprotected", xAttributes, _pImport->XMLNS_LIBRARY_UID );

        mLibDescriptors.push_back( aDesc );
        return new LibraryElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected styles ot bulletinboard element!", Reference< XInterface >(), Any() );
    }
}

void LibrariesElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    sal_Int32 nLibCount = _pImport->mpLibArray->mnLibCount = (sal_Int32)mLibDescriptors.size();
    _pImport->mpLibArray->mpLibs = new LibDescriptor[ nLibCount ];

    for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        const LibDescriptor& rLib = mLibDescriptors[i];
        _pImport->mpLibArray->mpLibs[i] = rLib;
    }
}

// library
Reference< xml::input::XElement > LibraryElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // library
    else if ( rLocalName == "element" )
    {
        OUString aValue( xAttributes->getValueByUidName(_pImport->XMLNS_LIBRARY_UID, "name" ) );
        if (!aValue.isEmpty())
            mElements.push_back( aValue );

        return new LibElementBase( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected styles ot bulletinboard element!", Reference< XInterface >(), Any() );
    }
}

void LibraryElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    sal_Int32 nElementCount = mElements.size();
    Sequence< OUString > aElementNames( nElementCount );
    OUString* pElementNames = aElementNames.getArray();
    for( sal_Int32 i = 0 ; i < nElementCount ; i++ )
        pElementNames[i] = mElements[i];

    LibDescriptor* pLib = _pImport->mpLibDesc;
    if( !pLib )
        pLib = &static_cast< LibrariesElement* >( _pParent )->mLibDescriptors.back();
    pLib->aElementNames = aElementNames;
}

Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer( LibDescriptorArray* pLibArray )
        SAL_THROW( (Exception) )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new LibraryImport( pLibArray ) ) );
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibrary( LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new LibraryImport( &rLib ) ) );
}

LibDescriptorArray::LibDescriptorArray( sal_Int32 nLibCount )
{
    mnLibCount = nLibCount;
    mpLibs = new LibDescriptor[ mnLibCount ];
}

LibDescriptorArray::~LibDescriptorArray()
{
    delete[] mpLibs;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
