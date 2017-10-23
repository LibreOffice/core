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
#include <xml_import.hxx>

using namespace css;
using namespace css::uno;

namespace xmlscript
{

Reference< xml::input::XElement > LibElementBase::getParent()
{
    return mxParent.get();
}

OUString LibElementBase::getLocalName()
{
    return _aLocalName;
}

sal_Int32 LibElementBase::getUid()
{
    return mxImport->XMLNS_LIBRARY_UID;
}

Reference< xml::input::XAttributes > LibElementBase::getAttributes()
{
    return _xAttributes;
}

void LibElementBase::ignorableWhitespace(
    OUString const & /*rWhitespaces*/ )
{
}

void LibElementBase::characters( OUString const & /*rChars*/ )
{
    // not used, all characters ignored
}

void LibElementBase::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
{
}

void LibElementBase::endElement()
{
}
Reference< xml::input::XElement > LibElementBase::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
{
    throw xml::sax::SAXException("unexpected element!", Reference< XInterface >(), Any() );
}

LibElementBase::LibElementBase(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    LibElementBase * pParent, LibraryImport * pImport )
    : mxImport( pImport )
    , mxParent( pParent )
    , _aLocalName( rLocalName )
    , _xAttributes( xAttributes )
{
}

LibElementBase::~LibElementBase()
{
    SAL_INFO("xmlscript.xmllib", "LibElementBase::~LibElementBase(): " << _aLocalName );
}

// XRoot

void LibraryImport::startDocument(
    Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
{
    XMLNS_LIBRARY_UID = xNamespaceMapping->getUidByUri( XMLNS_LIBRARY_URI );
    XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri( XMLNS_XLINK_URI );
}

void LibraryImport::endDocument()
{
}

void LibraryImport::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
{
}

void LibraryImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & /*xLocator*/ )
{
}

Reference< xml::input::XElement > LibraryImport::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if (XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    else if ( mpLibArray && rLocalName == "libraries" )
    {
        return new LibrariesElement( rLocalName, xAttributes, this );
    }
    else if ( mpLibDesc && rLocalName == "library" )
    {
        LibDescriptor& aDesc = *mpLibDesc;
        aDesc.bLink = aDesc.bReadOnly = aDesc.bPasswordProtected = aDesc.bPreload = false;

        aDesc.aName = xAttributes->getValueByUidName(XMLNS_LIBRARY_UID, "name" );
        getBoolAttr( &aDesc.bReadOnly, "readonly", xAttributes, XMLNS_LIBRARY_UID );
        getBoolAttr( &aDesc.bPasswordProtected, "passwordprotected", xAttributes, XMLNS_LIBRARY_UID );
        getBoolAttr( &aDesc.bPreload, "preload", xAttributes, XMLNS_LIBRARY_UID );

        return new LibraryElement( rLocalName, xAttributes, nullptr, this );
    }
    else
    {
        throw xml::sax::SAXException( "illegal root element (expected libraries) given: " + rLocalName, Reference< XInterface >(), Any() );
    }
}

LibraryImport::~LibraryImport()
{
    SAL_INFO("xmlscript.xmllib", "LibraryImport::~LibraryImport()." );
}

// libraries
Reference< xml::input::XElement > LibrariesElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if (mxImport->XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // library
    else if ( rLocalName == "library" )
    {
        LibDescriptor aDesc;
        aDesc.bLink = aDesc.bReadOnly = aDesc.bPasswordProtected = aDesc.bPreload = false;

        aDesc.aName = xAttributes->getValueByUidName(mxImport->XMLNS_LIBRARY_UID, "name" );
        aDesc.aStorageURL = xAttributes->getValueByUidName( mxImport->XMLNS_XLINK_UID, "href" );
        getBoolAttr(&aDesc.bLink, "link", xAttributes, mxImport->XMLNS_LIBRARY_UID );
        getBoolAttr(&aDesc.bReadOnly, "readonly", xAttributes, mxImport->XMLNS_LIBRARY_UID );
        getBoolAttr(&aDesc.bPasswordProtected, "passwordprotected", xAttributes, mxImport->XMLNS_LIBRARY_UID );

        mLibDescriptors.push_back( aDesc );
        return new LibraryElement( rLocalName, xAttributes, this, mxImport.get() );
    }
    else
    {
        throw xml::sax::SAXException( "expected styles ot bulletinboard element!", Reference< XInterface >(), Any() );
    }
}

void LibrariesElement::endElement()
{
    sal_Int32 nLibCount = mxImport->mpLibArray->mnLibCount = (sal_Int32)mLibDescriptors.size();
    mxImport->mpLibArray->mpLibs.reset( new LibDescriptor[ nLibCount ] );

    for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        const LibDescriptor& rLib = mLibDescriptors[i];
        mxImport->mpLibArray->mpLibs[i] = rLib;
    }
}

// library
Reference< xml::input::XElement > LibraryElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if (mxImport->XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // library
    else if ( rLocalName == "element" )
    {
        OUString aValue( xAttributes->getValueByUidName(mxImport->XMLNS_LIBRARY_UID, "name" ) );
        if (!aValue.isEmpty())
            mElements.push_back( aValue );

        return new LibElementBase( rLocalName, xAttributes, this, mxImport.get() );
    }
    else
    {
        throw xml::sax::SAXException( "expected styles ot bulletinboard element!", Reference< XInterface >(), Any() );
    }
}

void LibraryElement::endElement()
{
    sal_Int32 nElementCount = mElements.size();
    Sequence< OUString > aElementNames( nElementCount );
    OUString* pElementNames = aElementNames.getArray();
    for( sal_Int32 i = 0 ; i < nElementCount ; i++ )
        pElementNames[i] = mElements[i];

    LibDescriptor* pLib = mxImport->mpLibDesc;
    if( !pLib )
        pLib = &static_cast< LibrariesElement* >( mxParent.get() )->mLibDescriptors.back();
    pLib->aElementNames = aElementNames;
}

Reference< css::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer( LibDescriptorArray* pLibArray )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new LibraryImport( pLibArray ) ) );
}

css::uno::Reference< css::xml::sax::XDocumentHandler >
SAL_CALL importLibrary( LibDescriptor& rLib )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new LibraryImport( &rLib ) ) );
}

LibDescriptorArray::LibDescriptorArray( sal_Int32 nLibCount )
{
    mnLibCount = nLibCount;
    mpLibs.reset( new LibDescriptor[ mnLibCount ] );
}

LibDescriptorArray::~LibDescriptorArray()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
