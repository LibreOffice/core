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

#ifndef INCLUDED_SAX_FSHELPER_HXX
#define INCLUDED_SAX_FSHELPER_HXX

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <sax/saxdllapi.h>
#include <optional>
#include <memory>
#include <string_view>
#include <utility>

namespace com::sun::star::io { class XOutputStream; }
namespace sax_fastparser { class FastAttributeList; }

constexpr sal_Int32 FSNS(sal_Int32 namespc, sal_Int32 element) { return (namespc << 16) | element; }

namespace sax_fastparser {

enum class MergeMarks { APPEND = 0, PREPEND = 1, POSTPONE = 2};

class FastSaxSerializer;

class SAX_DLLPUBLIC FastSerializerHelper
{
public:

    FastSerializerHelper( const css::uno::Reference< css::io::XOutputStream >& xOutputStream, bool bWriteHeader );

    ~FastSerializerHelper();

    /// Start an element. After the first argument there can be a number of (attribute, value) pairs.
    template<typename... Args>
    void startElement(sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, Args &&... args)
    {
        if (value)
            pushAttributeValue(attribute, value);
        startElement(elementTokenId, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void startElement(sal_Int32 elementTokenId, sal_Int32 attribute,
                      const std::optional<OString>& value, Args&&... args)
    {
        if (value)
            pushAttributeValue(attribute, *value);
        startElement(elementTokenId, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void startElement(sal_Int32 elementTokenId, sal_Int32 attribute,
                      const std::optional<OUString>& value, Args&&... args)
    {
        std::optional<OString> opt;
        if (value)
            opt = value->toUtf8();
        startElement(elementTokenId, attribute, opt, std::forward<Args>(args)...);
    }
    void startElement(sal_Int32 elementTokenId);

    /// Start an element. After the first two arguments there can be a number of (attribute, value) pairs.
    template<typename... Args>
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, Args &&... args)
    {
        startElement(FSNS(namespaceTokenId, elementTokenId), std::forward<Args>(args)...);
    }

    /// Create a single element. After the first argument there can be a number of (attribute, value) pairs.
    template<typename... Args>
    void singleElement(sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, Args &&... args)
    {
        if (value)
            pushAttributeValue(attribute, value);
        singleElement(elementTokenId, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void singleElement(sal_Int32 elementTokenId, sal_Int32 attribute,
                       const std::optional<OString>& value, Args&&... args)
    {
        if (value)
            pushAttributeValue(attribute, *value);
        singleElement(elementTokenId, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void singleElement(sal_Int32 elementTokenId, sal_Int32 attribute,
                       const std::optional<OUString>& value, Args&&... args)
    {
        std::optional<OString> opt;
        if (value)
            opt = value->toUtf8();
        singleElement(elementTokenId, attribute, opt, std::forward<Args>(args)...);
    }
    void singleElement(sal_Int32 elementTokenId);

    /// Create a single element. After the first two arguments there can be a number of (attribute, value) pairs.
    template<typename... Args>
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, Args &&... args)
    {
        singleElement(FSNS(namespaceTokenId, elementTokenId), std::forward<Args>(args)...);
    }

    void endElement(sal_Int32 elementTokenId);
    void endElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId)
        { endElement( FSNS( namespaceTokenId, elementTokenId ) ); }

    void singleElement(sal_Int32 elementTokenId, const rtl::Reference<FastAttributeList>& xAttrList);
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, rtl::Reference<FastAttributeList> const & xAttrList)
        { singleElement(FSNS( namespaceTokenId, elementTokenId), xAttrList); }

    void startElement(sal_Int32 elementTokenId, const rtl::Reference<FastAttributeList>& xAttrList);
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, rtl::Reference<FastAttributeList> const & xAttrList)
        { startElement( FSNS( namespaceTokenId, elementTokenId ), xAttrList ); }

    FastSerializerHelper* write(const char* value);
    FastSerializerHelper* write(const OString& value);
    FastSerializerHelper* write(std::u16string_view value);
    FastSerializerHelper* write(sal_Int32 value);
    FastSerializerHelper* write(sal_Int64 value);
    FastSerializerHelper* write(double value);

    FastSerializerHelper* writeEscaped(const char* value);
    FastSerializerHelper* writeEscaped(std::u16string_view value);

    FastSerializerHelper* writeId(sal_Int32 tokenId);

    css::uno::Reference< css::io::XOutputStream > const & getOutputStream() const;

    static rtl::Reference<FastAttributeList> createAttrList();

    void mark(sal_Int32 nTag,
            const css::uno::Sequence< sal_Int32 >& rOrder =
            css::uno::Sequence< sal_Int32 >() );
    void mergeTopMarks(sal_Int32 nTag,
            MergeMarks eMergeType = MergeMarks::APPEND );

private:
    void pushAttributeValue( sal_Int32 attribute, const char* value );
    void pushAttributeValue( sal_Int32 attribute, const OString& value );

    FastSaxSerializer* mpSerializer;
};

typedef std::shared_ptr< FastSerializerHelper > FSHelperPtr;

// Helpers to make intention to pass optional attributes to *Element functions explicit, instead of
// using `(condition) ? value.toUtf8().getStr() : nullptr` syntax.
inline const char* UseIf(const char* s, bool bUse) { return bUse ? s : nullptr; }
// OString, OUString
template<class TString>
std::optional<TString> UseIf(const TString& s, bool bUse)
{
    return bUse ? std::optional<TString>(s) : std::optional<TString>();
}

}

#endif // INCLUDED_SAX_FSHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
