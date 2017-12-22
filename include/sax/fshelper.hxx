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
#include <stdarg.h>
#include <memory>
#include <utility>

#define FSNS(namespc, element) ((namespc << 16) | element)
// Backwards compatibility for code that used FSEND to terminate the vararg.
// As soon as no supported LO version has the varargs code, this can be removed entirely
// (otherwise backports might break silently if people didn't add FSEND).
// Ctor is there to get an error when trying to pass it to a vararg by accident.
struct FSEND_t { FSEND_t() {}; };
static const FSEND_t FSEND = FSEND_t();
const sal_Int32 FSEND_internal = -1; // same as XML_TOKEN_INVALID

#define SAX_ARGS_ARG( arg1, arg2, convert, num ) arg1##num, arg2##num convert
#define SAX_ARGS_ARG1( arg1, arg2, convert ) SAX_ARGS_ARG( arg1, arg2, convert, 1 )
#define SAX_ARGS_ARG2( arg1, arg2, convert ) SAX_ARGS_ARG1( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 2 )
#define SAX_ARGS_ARG3( arg1, arg2, convert ) SAX_ARGS_ARG2( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 3 )
#define SAX_ARGS_ARG4( arg1, arg2, convert ) SAX_ARGS_ARG3( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 4 )
#define SAX_ARGS_ARG5( arg1, arg2, convert ) SAX_ARGS_ARG4( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 5 )
#define SAX_ARGS_ARG6( arg1, arg2, convert ) SAX_ARGS_ARG5( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 6 )
#define SAX_ARGS_ARG7( arg1, arg2, convert ) SAX_ARGS_ARG6( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 7 )
#define SAX_ARGS_ARG8( arg1, arg2, convert ) SAX_ARGS_ARG7( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 8 )
#define SAX_ARGS_ARG9( arg1, arg2, convert ) SAX_ARGS_ARG8( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 9 )
#define SAX_ARGS_ARG10( arg1, arg2, convert ) SAX_ARGS_ARG9( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 10 )
#define SAX_ARGS_ARG11( arg1, arg2, convert ) SAX_ARGS_ARG10( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 11 )
#define SAX_ARGS_ARG12( arg1, arg2, convert ) SAX_ARGS_ARG11( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 12 )
#define SAX_ARGS_ARG13( arg1, arg2, convert ) SAX_ARGS_ARG12( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 13 )
#define SAX_ARGS_ARG14( arg1, arg2, convert ) SAX_ARGS_ARG13( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 14 )
#define SAX_ARGS_ARG15( arg1, arg2, convert ) SAX_ARGS_ARG14( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 15 )
#define SAX_ARGS_ARG16( arg1, arg2, convert ) SAX_ARGS_ARG15( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 16 )
#define SAX_ARGS_ARG17( arg1, arg2, convert ) SAX_ARGS_ARG16( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 17 )
#define SAX_ARGS_ARG18( arg1, arg2, convert ) SAX_ARGS_ARG17( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 18 )
#define SAX_ARGS_ARG19( arg1, arg2, convert ) SAX_ARGS_ARG18( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 19 )
#define SAX_ARGS_ARG20( arg1, arg2, convert ) SAX_ARGS_ARG19( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 20 )
#define SAX_ARGS_ARG21( arg1, arg2, convert ) SAX_ARGS_ARG20( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 21 )
#define SAX_ARGS_ARG22( arg1, arg2, convert ) SAX_ARGS_ARG21( arg1, arg2, convert ), SAX_ARGS_ARG( arg1, arg2, convert, 22 )

namespace sax_fastparser {

enum class MergeMarks { APPEND = 0, PREPEND = 1, POSTPONE = 2};

typedef css::uno::Reference< css::xml::sax::XFastAttributeList > XFastAttributeListRef;

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
    void startElement(sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, FSEND_t)
    {
        if (value)
            pushAttributeValue(attribute, value);
        startElement(elementTokenId, FSEND);
    }
    template<typename... Args>
    void startElement(sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, Args &&... args)
    {
        pushAttributeValue(attribute, value);
        startElement(elementTokenId, std::forward<Args>(args)...);
    }
    void startElement(sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, FSEND_t)
    {
        pushAttributeValue(attribute, value);
        startElement(elementTokenId, FSEND);
    }
    void startElement(sal_Int32 elementTokenId, FSEND_t);

    /// Start an element. After the first two arguments there can be a number of (attribute, value) pairs.
    template<typename... Args>
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, Args &&... args)
    {
        if (value)
            pushAttributeValue(attribute, value);
        startElementNS(namespaceTokenId, elementTokenId, std::forward<Args>(args)...);
    }
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, FSEND_t)
    {
        if (value)
            pushAttributeValue(attribute, value);
        startElementNS(namespaceTokenId, elementTokenId, FSEND);
    }
    template<typename... Args>
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, Args &&... args)
    {
        pushAttributeValue(attribute, value);
        startElementNS(namespaceTokenId, elementTokenId, std::forward<Args>(args)...);
    }
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, FSEND_t)
    {
        pushAttributeValue(attribute, value);
        startElementNS(namespaceTokenId, elementTokenId, FSEND);
    }
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, FSEND_t)
    {
        startElement(FSNS(namespaceTokenId, elementTokenId), FSEND);
    }

    /// Create a single element. After the first argument there can be a number of (attribute, value) pairs.
    template<typename... Args>
    void singleElement(sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, Args &&... args)
    {
        if (value)
            pushAttributeValue(attribute, value);
        singleElement(elementTokenId, std::forward<Args>(args)...);
    }
    void singleElement(sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, FSEND_t)
    {
        if (value)
            pushAttributeValue(attribute, value);
        singleElement(elementTokenId, FSEND);
    }
    template<typename... Args>
    void singleElement(sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, Args &&... args)
    {
        pushAttributeValue(attribute, value);
        singleElement(elementTokenId, std::forward<Args>(args)...);
    }
    void singleElement(sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, FSEND_t)
    {
        pushAttributeValue(attribute, value);
        singleElement(elementTokenId, FSEND);
    }
    void singleElement(sal_Int32 elementTokenId, FSEND_t);

    /// Create a single element. After the first two arguments there can be a number of (attribute, value) pairs.
    template<typename... Args>
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, Args &&... args)
    {
        if (value)
            pushAttributeValue(attribute, value);
        singleElementNS(namespaceTokenId, elementTokenId, std::forward<Args>(args)...);
    }
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const char* value, FSEND_t)
    {
        if (value)
            pushAttributeValue(attribute, value);
        singleElementNS(namespaceTokenId, elementTokenId, FSEND);
    }
    template<typename... Args>
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, Args &&... args)
    {
        pushAttributeValue(attribute, value);
        singleElementNS(namespaceTokenId, elementTokenId, std::forward<Args>(args)...);
    }
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, sal_Int32 attribute, const OString& value, FSEND_t)
    {
        pushAttributeValue(attribute, value);
        singleElementNS(namespaceTokenId, elementTokenId, FSEND);
    }
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, FSEND_t)
    {
        singleElement(FSNS(namespaceTokenId, elementTokenId), FSEND);
    }

    void endElement(sal_Int32 elementTokenId);
    void endElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId)
        { endElement( FSNS( namespaceTokenId, elementTokenId ) ); }

    void singleElement(sal_Int32 elementTokenId, const XFastAttributeListRef& xAttrList);
    void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, XFastAttributeListRef const & xAttrList)
        { singleElement(FSNS( namespaceTokenId, elementTokenId), xAttrList); }

    void startElement(sal_Int32 elementTokenId, const XFastAttributeListRef& xAttrList);
    void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, XFastAttributeListRef const & xAttrList)
        { startElement( FSNS( namespaceTokenId, elementTokenId ), xAttrList ); }

    FastSerializerHelper* write(const char* value);
    FastSerializerHelper* write(const OUString& value);
    FastSerializerHelper* write(sal_Int32 value);
    FastSerializerHelper* write(sal_Int64 value);
    FastSerializerHelper* write(double value);

    FastSerializerHelper* writeEscaped(const char* value);
    FastSerializerHelper* writeEscaped(const OUString& value);

    FastSerializerHelper* writeId(sal_Int32 tokenId);

    css::uno::Reference< css::io::XOutputStream > const & getOutputStream() const;

    static FastAttributeList *createAttrList();

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

}

#endif // INCLUDED_SAX_FSHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
