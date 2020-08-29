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

#include <cppuhelper/implbase.hxx>
#include <sax/saxdllapi.h>

#include <vector>

namespace com::sun::star::xml::sax { class XFastTokenHandler; }
namespace com::sun::star::xml { struct Attribute; }
namespace com::sun::star::xml { struct FastAttribute; }

namespace sax_fastparser
{

struct UnknownAttribute
{
    OUString maNamespaceURL;
    OString maName;
    OString maValue;

    UnknownAttribute( const OUString& rNamespaceURL, const OString& rName, const OString& value );
    UnknownAttribute( const OString& rName, const OString& value );

    void FillAttribute( css::xml::Attribute* pAttrib ) const;
};

/// A native C++ interface to tokenisation
class SAX_DLLPUBLIC FastTokenHandlerBase :
        public cppu::WeakImplHelper< css::xml::sax::XFastTokenHandler >
{
 public:
    virtual ~FastTokenHandlerBase();
    virtual sal_Int32 getTokenDirect( const char *pToken, sal_Int32 nLength ) const = 0;

    /**
     * Client method to attempt the use of this interface if possible.
     * @xTokenHandler - the token lookup interface
     * @pStr - string buffer to lookup
     * @nLength - optional length of chars in that buffer
     *
     * @return Tokenized form of pStr
     */
    static sal_Int32 getTokenFromChars(
                         const FastTokenHandlerBase *pTokenHandler,
                         const char *pStr, size_t nLength );
};


class SAX_DLLPUBLIC FastAttributeList final : public cppu::WeakImplHelper< css::xml::sax::XFastAttributeList >
{
public:
    FastAttributeList( FastTokenHandlerBase *pTokenHandler );
    virtual ~FastAttributeList() override;

    void clear();
    void reserve( sal_Int32 nNumTokens )
    {
        maAttributeValues.reserve(nNumTokens+1);
        maAttributeTokens.reserve(nNumTokens);
    }
    void add( const FastAttributeList& );
    void add( const css::uno::Reference<css::xml::sax::XFastAttributeList>& );
    void add( sal_Int32 nToken, const char* pValue );
    void add( sal_Int32 nToken, const char* pValue, size_t nValueLength );
    void add( sal_Int32 nToken, const OString& rValue );
    void addNS( sal_Int32 nNamespaceToken, sal_Int32 nToken, const OString& rValue );
    // note: rQName is *namespace-prefixed*
    void addUnknown( const OUString& rNamespaceURL, const OString& rQName, const OString& value );
    void addUnknown( const OString& rName, const OString& value );
    const std::vector< sal_Int32 >&  getFastAttributeTokens() const { return maAttributeTokens; }
    const char* getFastAttributeValue(size_t nIndex) const { return mpChunk + maAttributeValues[nIndex]; }
    sal_Int32 AttributeValueLength(size_t i) const { return maAttributeValues[i + 1] - maAttributeValues[i] - 1; }

    // performance sensitive shortcuts to avoid allocation ...
    bool getAsInteger( sal_Int32 nToken, sal_Int32 &rInt) const;
    bool getAsDouble( sal_Int32 nToken, double &rDouble) const;
    bool getAsChar( sal_Int32 nToken, const char*& rPos ) const;
    sal_Int32 getAsIntegerByIndex( sal_Int32 nTokenIndex ) const;
    const char* getAsCharByIndex( sal_Int32 nTokenIndex ) const;
    OUString getValueByIndex( sal_Int32 nTokenIndex ) const;

    // XFastAttributeList
    virtual sal_Bool SAL_CALL hasAttribute( ::sal_Int32 Token ) override;
    virtual ::sal_Int32 SAL_CALL getValueToken( ::sal_Int32 Token ) override;
    virtual ::sal_Int32 SAL_CALL getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) override;
    virtual OUString SAL_CALL getValue( ::sal_Int32 Token ) override;
    virtual OUString SAL_CALL getOptionalValue( ::sal_Int32 Token ) override;
    virtual css::uno::Sequence< css::xml::Attribute > SAL_CALL getUnknownAttributes(  ) override;
    virtual css::uno::Sequence< css::xml::FastAttribute > SAL_CALL getFastAttributes() override;

    sal_Int32 getAttributeIndex( ::sal_Int32 Token )
    {
        for (size_t i=0; i<maAttributeTokens.size(); ++i)
            if (maAttributeTokens[i] == Token)
                return i;
        return -1;
    }

    /// Use for fast iteration and conversion of attributes
    class FastAttributeIter {
        const FastAttributeList &mrList;
        size_t mnIdx;

    public:
        FastAttributeIter(const FastAttributeList &rList, size_t nIdx)
            : mrList(rList), mnIdx(nIdx)
        {
        }

        FastAttributeIter& operator++ ()
        {
            mnIdx++;
            return *this;
        }
        bool operator!=( const FastAttributeIter& rhs ) const
        {
            return mnIdx != rhs.mnIdx;
        }

        const FastAttributeIter& operator*() const
        {
            return *this;
        }

        sal_Int32 getToken() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return mrList.maAttributeTokens[mnIdx];
        }
        bool isEmpty() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return mrList.AttributeValueLength(mnIdx) < 1;
        }
        sal_Int32 toInt32() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return rtl_str_toInt32(mrList.getFastAttributeValue(mnIdx), 10);
        }
        double toDouble() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return rtl_str_toDouble(mrList.getFastAttributeValue(mnIdx));
        }
        OUString toString() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return OUString(mrList.getFastAttributeValue(mnIdx),
                            mrList.AttributeValueLength(mnIdx),
                            RTL_TEXTENCODING_UTF8);
        }
        const char* toCString() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return mrList.getFastAttributeValue(mnIdx);
        }
        sal_Int32 getLength() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return mrList.AttributeValueLength(mnIdx);
        }
        bool isString(const char *str) const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return !strcmp(str, mrList.getFastAttributeValue(mnIdx));
        }
    };
    FastAttributeIter begin() const { return FastAttributeIter(*this, 0); }
    FastAttributeIter end() const { return FastAttributeIter(*this, maAttributeTokens.size()); }
    FastAttributeIter find( sal_Int32 nToken ) const;

private:
    char     *mpChunk; ///< buffer to store all attribute values - null terminated strings
    sal_Int32 mnChunkLength; ///< size of allocated memory for mpChunk
    // maAttributeValues stores pointers, relative to mpChunk, for each attribute value string
    // length of the string is maAttributeValues[n+1] - maAttributeValues[n] - 1
    // maAttributeValues[0] == 0
    std::vector< sal_Int32 > maAttributeValues;
    std::vector< sal_Int32 > maAttributeTokens;
    std::vector< UnknownAttribute > maUnknownAttributes;
    FastTokenHandlerBase * mpTokenHandler;
};

inline FastAttributeList& castToFastAttributeList(
                    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    assert( dynamic_cast <FastAttributeList *> ( xAttrList.get() ) != nullptr );
    return *static_cast <FastAttributeList *> ( xAttrList.get() );
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
