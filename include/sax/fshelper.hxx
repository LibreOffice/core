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

#include <com/sun/star/io/XOutputStream.hpp>
#include <sax/fastattribs.hxx>
#include <cstddef>
#include <initializer_list>
#include <memory>

#define FSNS(namespc, element) ((namespc << 16) | element)

namespace sax_fastparser {

enum MergeMarksEnum { MERGE_MARKS_APPEND = 0, MERGE_MARKS_PREPEND = 1, MERGE_MARKS_POSTPONE = 2, MERGE_MARKS_IGNORE = 3 };

typedef ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList > XFastAttributeListRef;

class FastSaxSerializer;

class AttrValue {
public:
    AttrValue(std::nullptr_t = nullptr): string_(nullptr), length_() {}

    AttrValue(OString const & value):
        reference_(value), string_(reference_.getStr()),
        length_(reference_.getLength())
    {}

    template<typename T> AttrValue(
        T & value,
        typename rtl::libreoffice_internal::ConstCharArrayDetector<
                T, rtl::libreoffice_internal::Dummy>::Type
            = rtl::libreoffice_internal::Dummy()):
        string_(value),
        length_(rtl::libreoffice_internal::ConstCharArrayDetector<T>::length)
    {
        assert(
            rtl::libreoffice_internal::ConstCharArrayDetector<T>::isValid(
                value));
    }

    template<typename T> AttrValue(
        T const &,
        typename rtl::libreoffice_internal::CharPtrDetector<
                T, rtl::libreoffice_internal::Dummy>::Type
            = rtl::libreoffice_internal::Dummy())
        = delete;

    template<typename T> AttrValue(
        T &,
        typename rtl::libreoffice_internal::NonConstCharArrayDetector<
                T, rtl::libreoffice_internal::Dummy>::Type
            = rtl::libreoffice_internal::Dummy())
        = delete;

    explicit AttrValue(char const * string, sal_Int32 length):
        string_(string), length_(length) {}

    explicit operator bool() const { return string_ != nullptr; }

    char const * getString() const { assert(bool(*this)); return string_; }

    sal_Int32 getLength() const { assert(bool(*this)); return length_; }

private:
    OString reference_;
    char const * string_;
    sal_Int32 length_;
};

struct Attr {
    sal_Int32 id;
    AttrValue value;
};

class SAX_DLLPUBLIC FastSerializerHelper
{
public:

    FastSerializerHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream, bool bWriteHeader = true );

    ~FastSerializerHelper();

    /// Start an element. After the first argument there can be a number of (attribute, value) pairs.
    void startElement(
        sal_Int32 elementTokenId,
        std::initializer_list<Attr> attrs = std::initializer_list<Attr>{});
    /// Create a single element. After the first argument there can be a number of (attribute, value) pairs.
    void singleElement(
        sal_Int32 elementTokenId,
        std::initializer_list<Attr> attrs = std::initializer_list<Attr>{});
    /// Start an element. After the first two arguments there can be a number of (attribute, value) pairs.
    void startElementNS(
        sal_Int32 namespaceTokenId, sal_Int32 elementTokenId,
        std::initializer_list<Attr> attrs = std::initializer_list<Attr>{})
    { startElement(FSNS(namespaceTokenId, elementTokenId), attrs); }
    /// Create a single element. After the first two arguments there can be a number of (attribute, value) pairs.
    void singleElementNS(
        sal_Int32 namespaceTokenId, sal_Int32 elementTokenId,
        std::initializer_list<Attr> attrs = std::initializer_list<Attr>{})
    { singleElement(FSNS(namespaceTokenId, elementTokenId), attrs); }
    void endElement(sal_Int32 elementTokenId);
    inline void endElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId)
        { endElement( FSNS( namespaceTokenId, elementTokenId ) ); }

    void singleElement(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList);
    inline void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
        { singleElement(FSNS( namespaceTokenId, elementTokenId), xAttrList); }

    void startElement(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList);
    inline void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
        { startElement( FSNS( namespaceTokenId, elementTokenId ), xAttrList ); }

    FastSerializerHelper* write(const char* value);
    FastSerializerHelper* write(const OUString& value);
    FastSerializerHelper* write(sal_Int32 value);
    FastSerializerHelper* write(sal_Int64 value);
    FastSerializerHelper* write(double value);

    FastSerializerHelper* writeEscaped(const char* value);
    FastSerializerHelper* writeEscaped(const OUString& value);

    FastSerializerHelper* writeId(sal_Int32 tokenId);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > getOutputStream();

    static FastAttributeList *createAttrList();

    void mark( const ::com::sun::star::uno::Sequence< sal_Int32 >& aOrder =
            ::com::sun::star::uno::Sequence< sal_Int32 >() );
    void mergeTopMarks( MergeMarksEnum eMergeType = MERGE_MARKS_APPEND );

private:
    FastSaxSerializer* mpSerializer;
};

typedef std::shared_ptr< FastSerializerHelper > FSHelperPtr;

}

#endif // INCLUDED_SAX_FSHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
