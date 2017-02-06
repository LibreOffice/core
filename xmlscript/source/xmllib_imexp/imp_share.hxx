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

#ifndef INCLUDED_XMLSCRIPT_SOURCE_XMLLIB_IMEXP_IMP_SHARE_HXX
#define INCLUDED_XMLSCRIPT_SOURCE_XMLLIB_IMEXP_IMP_SHARE_HXX

#include <xmlscript/xmllib_imexp.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <com/sun/star/xml/input/XRoot.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <rtl/ref.hxx>

#include <vector>

namespace xmlscript
{
inline sal_Int32 toInt32( OUString const & rStr )
{
    sal_Int32 nVal;
    if (rStr.getLength() > 2 && rStr[ 0 ] == '0' && rStr[ 1 ] == 'x')
    {
        nVal = rStr.copy( 2 ).toUInt32( 16 );
    }
    else
    {
        nVal = rStr.toInt32();
    }
    return nVal;
}
inline bool getBoolAttr(
    bool * pRet, OUString const & rAttrName,
    css::uno::Reference< css::xml::input::XAttributes > const & xAttributes, sal_Int32 uid )
{
    OUString aValue(
        xAttributes->getValueByUidName( uid, rAttrName ) );
    if (!aValue.isEmpty())
    {
        if ( aValue == "true" )
        {
            *pRet = true;
            return true;
        }
        else if ( aValue == "false" )
        {
            *pRet = false;
            return true;
        }
        else
        {
            throw css::xml::sax::SAXException(rAttrName + ": no boolean value (true|false)!", css::uno::Reference< css::uno::XInterface >(), css::uno::Any() );
        }
    }
    return false;
}

inline bool getStringAttr(
    OUString * pRet, OUString const & rAttrName,
    css::uno::Reference< css::xml::input::XAttributes > const & xAttributes, sal_Int32 uid )
{
    *pRet = xAttributes->getValueByUidName( uid, rAttrName );
    return (!pRet->isEmpty());
}

inline bool getLongAttr(
    sal_Int32 * pRet, OUString const & rAttrName,
    css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
    sal_Int32 uid )
{
    OUString aValue(
        xAttributes->getValueByUidName( uid, rAttrName ) );
    if (!aValue.isEmpty())
    {
        *pRet = toInt32( aValue );
        return true;
    }
    return false;
}

// Library import

struct LibraryImport
    : public ::cppu::WeakImplHelper< css::xml::input::XRoot >
{
    friend class LibrariesElement;
    friend class LibraryElement;

    LibDescriptorArray* mpLibArray;
    LibDescriptor*      mpLibDesc;      // Single library mode

    sal_Int32 XMLNS_LIBRARY_UID;
    sal_Int32 XMLNS_XLINK_UID;

public:
    explicit LibraryImport( LibDescriptorArray* pLibArray )
        : mpLibArray(pLibArray)
        , mpLibDesc(nullptr)
        , XMLNS_LIBRARY_UID(0)
        , XMLNS_XLINK_UID(0)
    {
    }

    // Single library mode
    explicit LibraryImport(LibDescriptor* pLibDesc)
        : mpLibArray(nullptr)
        , mpLibDesc(pLibDesc)
        , XMLNS_LIBRARY_UID(0)
        , XMLNS_XLINK_UID(0)
    {
    }

    virtual ~LibraryImport() override;

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference< css::xml::input::XNamespaceMapping > const & xNamespaceMapping ) override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData ) override;
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference< css::xml::sax::XLocator > const & xLocator ) override;
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startRootElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes ) override;
};

class LibElementBase
    : public ::cppu::WeakImplHelper< css::xml::input::XElement >
{
protected:
    rtl::Reference<LibraryImport>  mxImport;
    rtl::Reference<LibElementBase> mxParent;
private:
    OUString _aLocalName;
    css::uno::Reference< css::xml::input::XAttributes > _xAttributes;

public:
    LibElementBase(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport );
    virtual ~LibElementBase() override;

    // XElement
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL getParent() override;
    virtual OUString SAL_CALL getLocalName() override;
    virtual sal_Int32 SAL_CALL getUid() override;
    virtual css::uno::Reference< css::xml::input::XAttributes > SAL_CALL getAttributes() override;
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces ) override;
    virtual void SAL_CALL characters( OUString const & rChars ) override;
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData ) override;
    virtual void SAL_CALL endElement() override;
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes ) override;
};

class LibrariesElement : public LibElementBase
{
    friend class LibraryElement;

protected:
    std::vector< LibDescriptor > mLibDescriptors;

public:
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    LibrariesElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        LibraryImport * pImport )
        : LibElementBase( rLocalName, xAttributes, nullptr, pImport )
        {}
};

class LibraryElement : public LibElementBase
{
protected:
    std::vector< OUString > mElements;

public:

    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    LibraryElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport )
        : LibElementBase( rLocalName, xAttributes, pParent, pImport )
    {}
};

}

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLLIB_IMEXP_IMP_SHARE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
