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

#ifndef INCLUDED_SAX_FASTATTRIBS_HXX
#define INCLUDED_SAX_FASTATTRIBS_HXX

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/Attribute.hpp>
#include <com/sun/star/xml/FastAttribute.hpp>

#include <cppuhelper/implbase1.hxx>
#include <sax/saxdllapi.h>

#include <map>
#include <vector>

namespace sax_fastparser
{

struct UnknownAttribute
{
    OUString maNamespaceURL;
    OString maName;
    OString maValue;

    UnknownAttribute( const OUString& rNamespaceURL, const OString& rName, const sal_Char* pValue );

    UnknownAttribute( const OString& rName, const sal_Char* pValue );

    void FillAttribute( ::com::sun::star::xml::Attribute* pAttrib ) const;
};

typedef std::vector< UnknownAttribute > UnknownAttributeList;

/// A native C++ interface to tokenisation
class SAX_DLLPUBLIC FastTokenHandlerBase
{
    public:
        virtual ~FastTokenHandlerBase();
        virtual sal_Int32 getTokenDirect( const char *pToken, sal_Int32 nLength ) const = 0;
};

/// avoid constantly allocating and freeing sequences.
class SAX_DLLPUBLIC FastTokenLookup
{
    static const int mnUtf8BufferSize = 128;
    ::css::uno::Sequence< sal_Int8 > maUtf8Buffer;
public:
    FastTokenLookup();
    sal_Int32 getTokenFromChars(
        const ::css::uno::Reference< ::css::xml::sax::XFastTokenHandler > &mxTokenHandler,
        FastTokenHandlerBase *pTokenHandler,
        const char *pStr, size_t nLength = 0 );
};

class SAX_DLLPUBLIC FastAttributeList : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastAttributeList >
{
public:
    FastAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& xTokenHandler,
                       FastTokenHandlerBase *pOptHandlerBase = NULL );
    virtual ~FastAttributeList();

    void clear();
    void add( sal_Int32 nToken, const sal_Char* pValue, size_t nValueLength = 0 );
    void add( sal_Int32 nToken, const OString& rValue );
    void addNS( sal_Int32 nNamespaceToken, sal_Int32 nToken, const OString& rValue );
    void addUnknown( const OUString& rNamespaceURL, const OString& rName, const sal_Char* pValue );
    void addUnknown( const OString& rName, const sal_Char* pValue );

    // performance sensitive shortcuts to avoid allocation ...
    bool getAsInteger( sal_Int32 nToken, sal_Int32 &rInt);
    bool getAsDouble( sal_Int32 nToken, double &rDouble);
    bool getAsChar( sal_Int32 nToken, const char*& rPos ) const;

    // XFastAttributeList
    virtual ::sal_Bool SAL_CALL hasAttribute( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL getValueToken( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getValue( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getOptionalValue( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::xml::Attribute > SAL_CALL getUnknownAttributes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::xml::FastAttribute > SAL_CALL getFastAttributes() throw (::com::sun::star::uno::RuntimeException, std::exception);

private:
    inline sal_Int32 AttributeValueLength(sal_Int32 i);

private:
    sal_Char *mpChunk; ///< buffer to store all attribute values - null terminated strings
    sal_Int32 mnChunkLength; ///< size of allocated memory for mpChunk
    // maAttributeValues stores pointers, relative to mpChunk, for each attribute value string
    // length of the string is maAttributeValues[n+1] - maAttributeValues[n] - 1
    // maAttributeValues[0] == 0
    std::vector< sal_Int32 > maAttributeValues;
    std::vector< sal_Int32 > maAttributeTokens;
    UnknownAttributeList maUnknownAttributes;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler > mxTokenHandler;
    FastTokenHandlerBase *mpTokenHandler;

    FastTokenLookup maTokenLookup;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
