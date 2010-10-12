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
Reference< xml::input::XElement > LibElementBase::getParent()
    throw (RuntimeException)
{
    return static_cast< xml::input::XElement * >( _pParent );
}
//__________________________________________________________________________________________________
OUString LibElementBase::getLocalName()
    throw (RuntimeException)
{
    return _aLocalName;
}
//__________________________________________________________________________________________________
sal_Int32 LibElementBase::getUid()
    throw (RuntimeException)
{
    return _pImport->XMLNS_LIBRARY_UID;
}
//__________________________________________________________________________________________________
Reference< xml::input::XAttributes > LibElementBase::getAttributes()
    throw (RuntimeException)
{
    return _xAttributes;
}
//__________________________________________________________________________________________________
void LibElementBase::ignorableWhitespace(
    OUString const & /*rWhitespaces*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
void LibElementBase::characters( OUString const & /*rChars*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // not used, all characters ignored
}

//__________________________________________________________________________________________________
void LibElementBase::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}

//__________________________________________________________________________________________________
void LibElementBase::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > LibElementBase::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("unexpected element!") ),
        Reference< XInterface >(), Any() );
}

//__________________________________________________________________________________________________
LibElementBase::LibElementBase(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    LibElementBase * pParent, LibraryImport * pImport )
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
LibElementBase::~LibElementBase()
    SAL_THROW( () )
{
    _pImport->release();

    if (_pParent)
    {
        _pParent->release();
    }

#if OSL_DEBUG_LEVEL > 1
    OString aStr( OUStringToOString( _aLocalName, RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "LibElementBase::~LibElementBase(): %s\n", aStr.getStr() );
#endif
}

//##################################################################################################

// XRoot

//______________________________________________________________________________
void LibraryImport::startDocument(
    Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, RuntimeException)
{
    XMLNS_LIBRARY_UID = xNamespaceMapping->getUidByUri(
        OUSTR(XMLNS_LIBRARY_URI) );
    XMLNS_XLINK_UID = xNamespaceMapping->getUidByUri(
        OUSTR(XMLNS_XLINK_URI) );
}
//__________________________________________________________________________________________________
void LibraryImport::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
void LibraryImport::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
void LibraryImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & /*xLocator*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > LibraryImport::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    else if (mpLibArray && rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("libraries") ))
    {
        return new LibrariesElement( rLocalName, xAttributes, 0, this );
    }
    else if (mpLibDesc && rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("library") ))
    {
        LibDescriptor& aDesc = *mpLibDesc;
        aDesc.bLink = aDesc.bReadOnly = aDesc.bPasswordProtected = aDesc.bPreload = sal_False;

        aDesc.aName = xAttributes->getValueByUidName(
            XMLNS_LIBRARY_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("name") ) );
        getBoolAttr(
            &aDesc.bReadOnly,
            OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ), xAttributes,
            XMLNS_LIBRARY_UID );
        getBoolAttr(
            &aDesc.bPasswordProtected,
            OUString( RTL_CONSTASCII_USTRINGPARAM("passwordprotected") ),
            xAttributes, XMLNS_LIBRARY_UID );
        getBoolAttr(
            &aDesc.bPreload,
            OUString( RTL_CONSTASCII_USTRINGPARAM("preload") ),
            xAttributes, XMLNS_LIBRARY_UID );

        return new LibraryElement( rLocalName, xAttributes, 0, this );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal root element (expected libraries) given: ") ) +
            rLocalName, Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
LibraryImport::~LibraryImport()
    SAL_THROW( () )
{
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "LibraryImport::~LibraryImport().\n" );
#endif
}

//##################################################################################################


// libraries
//__________________________________________________________________________________________________
Reference< xml::input::XElement > LibrariesElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // library
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("library") ))
    {
        LibDescriptor aDesc;
        aDesc.bLink = aDesc.bReadOnly = aDesc.bPasswordProtected = aDesc.bPreload = sal_False;

        aDesc.aName = xAttributes->getValueByUidName(
            _pImport->XMLNS_LIBRARY_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("name") ) );
        aDesc.aStorageURL = xAttributes->getValueByUidName(
            _pImport->XMLNS_XLINK_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("href") ) );
        getBoolAttr(
            &aDesc.bLink,
            OUString( RTL_CONSTASCII_USTRINGPARAM("link") ),
            xAttributes, _pImport->XMLNS_LIBRARY_UID );
        getBoolAttr(
            &aDesc.bReadOnly,
            OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
            xAttributes, _pImport->XMLNS_LIBRARY_UID );
        getBoolAttr(
            &aDesc.bPasswordProtected,
            OUString( RTL_CONSTASCII_USTRINGPARAM("passwordprotected") ),
            xAttributes, _pImport->XMLNS_LIBRARY_UID );

        mLibDescriptors.push_back( aDesc );
        return new LibraryElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected styles ot bulletinboard element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
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
//__________________________________________________________________________________________________
Reference< xml::input::XElement > LibraryElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_LIBRARY_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // library
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("element") ))
    {
        OUString aValue( xAttributes->getValueByUidName(
            _pImport->XMLNS_LIBRARY_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("name") ) ) );
        if (aValue.getLength())
            mElements.push_back( aValue );

        return new LibElementBase( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected styles ot bulletinboard element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
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


//##################################################################################################

Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer( LibDescriptorArray* pLibArray )
        SAL_THROW( (Exception) )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new LibraryImport( pLibArray ) ) );
}

//##################################################################################################

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibrary( LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >( new LibraryImport( &rLib ) ) );
}


//##################################################################################################

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
