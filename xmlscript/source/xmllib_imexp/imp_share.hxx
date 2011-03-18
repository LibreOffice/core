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

#include <xmlscript/xmllib_imexp.hxx>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <com/sun/star/xml/input/XRoot.hpp>

#include <vector>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{
inline sal_Int32 toInt32( OUString const & rStr ) SAL_THROW( () )
{
    sal_Int32 nVal;
    if (rStr.getLength() > 2 && rStr[ 0 ] == '0' && rStr[ 1 ] == 'x')
    {
        nVal = rStr.copy( 2 ).toInt32( 16 );
    }
    else
    {
        nVal = rStr.toInt32();
    }
    return nVal;
}
inline bool getBoolAttr(
    sal_Bool * pRet, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes, sal_Int32 uid )
{
    OUString aValue(
        xAttributes->getValueByUidName( uid, rAttrName ) );
    if (aValue.getLength())
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("true") ))
        {
            *pRet = sal_True;
            return true;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("false") ))
        {
            *pRet = sal_False;
            return true;
        }
        else
        {
            throw xml::sax::SAXException(
                rAttrName +
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              ": no boolean value (true|false)!") ),
                Reference< XInterface >(), Any() );
        }
    }
    return false;
}

inline bool getStringAttr(
    OUString * pRet, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes, sal_Int32 uid )
{
    *pRet = xAttributes->getValueByUidName( uid, rAttrName );
    return (pRet->getLength() > 0);
}

inline bool getLongAttr(
    sal_Int32 * pRet, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes,
    sal_Int32 uid )
{
    OUString aValue(
        xAttributes->getValueByUidName( uid, rAttrName ) );
    if (aValue.getLength())
    {
        *pRet = toInt32( aValue );
        return true;
    }
    return false;
}

//==================================================================================================
// Library import

//==================================================================================================
struct LibraryImport
    : public ::cppu::WeakImplHelper1< xml::input::XRoot >
{
    friend class LibrariesElement;
    friend class LibraryElement;

    LibDescriptorArray* mpLibArray;
    LibDescriptor*      mpLibDesc;      // Single library mode

    sal_Int32 XMLNS_LIBRARY_UID;
    sal_Int32 XMLNS_XLINK_UID;

public:
    inline LibraryImport( LibDescriptorArray* pLibArray )
        SAL_THROW( () )
        : mpLibArray( pLibArray )
        , mpLibDesc( NULL ) {}
    // Single library mode
    inline LibraryImport( LibDescriptor* pLibDesc )
        SAL_THROW( () )
        : mpLibArray( NULL )
        , mpLibDesc( pLibDesc ) {}
    virtual ~LibraryImport()
        SAL_THROW( () );

    // XRoot
    virtual void SAL_CALL startDocument(
        Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        Reference< xml::sax::XLocator > const & xLocator )
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::input::XElement > SAL_CALL startRootElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================
class LibElementBase
    : public ::cppu::WeakImplHelper1< xml::input::XElement >
{
protected:
    LibraryImport * _pImport;
    LibElementBase * _pParent;

    OUString _aLocalName;
    Reference< xml::input::XAttributes > _xAttributes;

public:
    LibElementBase(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport )
        SAL_THROW( () );
    virtual ~LibElementBase()
        SAL_THROW( () );

    // XElement
    virtual Reference< xml::input::XElement > SAL_CALL getParent()
        throw (RuntimeException);
    virtual OUString SAL_CALL getLocalName()
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getUid()
        throw (RuntimeException);
    virtual Reference< xml::input::XAttributes > SAL_CALL getAttributes()
        throw (RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL characters( OUString const & rChars )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================

class LibrariesElement : public LibElementBase
{
    friend class LibraryElement;

protected:
    vector< LibDescriptor > mLibDescriptors;

public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    LibrariesElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport )
        SAL_THROW( () )
        : LibElementBase( rLocalName, xAttributes, pParent, pImport )
        {}
};

//==================================================================================================

class LibraryElement : public LibElementBase
{
protected:
    vector< OUString > mElements;

public:

    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    LibraryElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport )
        SAL_THROW( () )
        : LibElementBase( rLocalName, xAttributes, pParent, pImport )
    {}
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
