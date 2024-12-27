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
#include <com/sun/star/util/XCloneable.hpp>

#include <cppuhelper/implbase.hxx>
#include <o3tl/string_view.hxx>
#include <sax/saxdllapi.h>

#include <string_view>
#include <vector>

namespace com::sun::star::xml::sax { class XFastTokenHandler; }
namespace com::sun::star::xml { struct Attribute; }

namespace sax_fastparser
{

struct UnknownAttribute
{
    OUString maNamespaceURL;
    OString maName;
    OString maValue;

    UnknownAttribute( OUString sNamespaceURL, OString aName, OString value );
    UnknownAttribute( OString sName, OString value );

    void FillAttribute( css::xml::Attribute* pAttrib ) const;
};

/// A native C++ interface to tokenisation
class SAX_DLLPUBLIC FastTokenHandlerBase :
        public cppu::WeakImplHelper< css::xml::sax::XFastTokenHandler >
{
 public:
    virtual ~FastTokenHandlerBase();
    virtual sal_Int32 getTokenDirect(std::string_view token) const = 0;

    /**
     * Client method to attempt the use of this interface if possible.
     * @xTokenHandler - the token lookup interface
     * @str - string buffer to lookup
     *
     * @return Tokenized form of str
     */
    static sal_Int32 getTokenFromChars(
                         const FastTokenHandlerBase *pTokenHandler,
                         std::string_view str );
};


class SAX_DLLPUBLIC FastAttributeList final : public cppu::WeakImplHelper< css::xml::sax::XFastAttributeList, css::util::XCloneable >
{
public:
    FastAttributeList( FastTokenHandlerBase *pTokenHandler );
    FastAttributeList( const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );
    virtual ~FastAttributeList() override;

    void clear();
    void reserve( sal_Int32 nNumTokens )
    {
        maAttributeValues.reserve(nNumTokens+1);
        maAttributeTokens.reserve(nNumTokens);
    }
    void add( const FastAttributeList& );
    void add( const css::uno::Reference<css::xml::sax::XFastAttributeList>& );
    void add( sal_Int32 nToken, std::string_view value );
    void add( sal_Int32 nToken, std::u16string_view sValue ); // Converts to UTF-8
    template <typename C, typename T1, typename T2>
    void add( sal_Int32 nToken, rtl::StringConcat<C, T1, T2>&& value) { add(nToken, Concat2View(value)); }
    template <typename Val, typename... Rest, std::enable_if_t<(sizeof...(Rest) > 0), int> = 0>
    void add( sal_Int32 nToken, Val&& val, Rest&&... rest )
    {
        add(nToken, std::forward<Val>(val));
        add(std::forward<Rest>(rest)...);
    }
    void addNS( sal_Int32 nNamespaceToken, sal_Int32 nToken, std::string_view sValue );
    void addNS( sal_Int32 nNamespaceToken, sal_Int32 nToken, std::u16string_view sValue );
    // note: rQName is *namespace-prefixed*
    void addUnknown( const OUString& rNamespaceURL, const OString& rQName, const OString& value );
    void addUnknown( const OString& rName, const OString& value );
    const std::vector< sal_Int32 >&  getFastAttributeTokens() const { return maAttributeTokens; }
    const char* getFastAttributeValue(size_t nIndex) const { return mpChunk + maAttributeValues[nIndex]; }
    sal_Int32 AttributeValueLength(size_t i) const { return maAttributeValues[i + 1] - maAttributeValues[i] - 1; }
    size_t size() const { return maAttributeValues.size(); }

    // performance sensitive shortcuts to avoid allocation ...
    bool getAsInteger( sal_Int32 nToken, sal_Int32 &rInt) const;
    bool getAsDouble( sal_Int32 nToken, double &rDouble) const;
    bool getAsView( sal_Int32 nToken, std::string_view& rPos ) const;
    sal_Int32 getAsIntegerByIndex( sal_Int32 nTokenIndex ) const
    {
        return o3tl::toInt32(getAsViewByIndex(nTokenIndex));
    }
    std::string_view getAsViewByIndex( sal_Int32 nTokenIndex ) const
    {
        return std::string_view(getFastAttributeValue(nTokenIndex), AttributeValueLength(nTokenIndex));
    }
    OUString getValueByIndex( sal_Int32 nTokenIndex ) const
    {
        return OStringToOUString(getAsViewByIndex(nTokenIndex), RTL_TEXTENCODING_UTF8);
    }
    sal_Int32 getValueTokenByIndex(sal_Int32 nTokenIndex) const;

    // XFastAttributeList
    virtual sal_Bool SAL_CALL hasAttribute( ::sal_Int32 Token ) override;
    virtual ::sal_Int32 SAL_CALL getValueToken( ::sal_Int32 Token ) override;
    virtual ::sal_Int32 SAL_CALL getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) override;
    virtual OUString SAL_CALL getValue( ::sal_Int32 Token ) override;
    virtual OUString SAL_CALL getOptionalValue( ::sal_Int32 Token ) override;
    virtual css::uno::Sequence< css::xml::Attribute > SAL_CALL getUnknownAttributes(  ) override;
    virtual css::uno::Sequence< css::xml::FastAttribute > SAL_CALL getFastAttributes() override;

    // XCloneable
    virtual ::css::uno::Reference< ::css::util::XCloneable > SAL_CALL createClone() override;

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
            return mrList.getAsIntegerByIndex(mnIdx);
        }
        double toDouble() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return o3tl::toDouble(mrList.getAsViewByIndex(mnIdx));
        }
        bool toBoolean() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return rtl_str_toBoolean(mrList.getFastAttributeValue(mnIdx));
        }
        OUString toString() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return mrList.getValueByIndex(mnIdx);
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
        std::string_view toView() const
        {
            assert(mnIdx < mrList.maAttributeTokens.size());
            return mrList.getAsViewByIndex(mnIdx);
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
