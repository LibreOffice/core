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
#include <initializer_list>
#include <memory>

#define FSNS(namespc, element) ((namespc << 16) | element)

namespace sax_fastparser {

enum MergeMarksEnum { MERGE_MARKS_APPEND = 0, MERGE_MARKS_PREPEND = 1, MERGE_MARKS_POSTPONE = 2, MERGE_MARKS_IGNORE = 3 };

typedef ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList > XFastAttributeListRef;

class FastSaxSerializer;

namespace sax {

struct Attr {
    sal_Int32 id;
    char const * value;
};

}

class SAX_DLLPUBLIC FastSerializerHelper
{
public:

    FastSerializerHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream, bool bWriteHeader = true );

    ~FastSerializerHelper();

    /// Start an element. After the first argument there can be a number of (attribute, value) pairs.
    void startElement(
        sal_Int32 elementTokenId,
        std::initializer_list<sax::Attr> attrs
            = std::initializer_list<sax::Attr>{});
    /// Create a single element. After the first argument there can be a number of (attribute, value) pairs.
    void singleElement(
        sal_Int32 elementTokenId,
        std::initializer_list<sax::Attr> attrs
            = std::initializer_list<sax::Attr>{});
    /// Start an element. After the first two arguments there can be a number of (attribute, value) pairs.
    void startElementNS(
        sal_Int32 namespaceTokenId, sal_Int32 elementTokenId,
        std::initializer_list<sax::Attr> attrs
            = std::initializer_list<sax::Attr>{})
    { startElement(FSNS(namespaceTokenId, elementTokenId), attrs); }
    /// Create a single element. After the first two arguments there can be a number of (attribute, value) pairs.
    void singleElementNS(
        sal_Int32 namespaceTokenId, sal_Int32 elementTokenId,
        std::initializer_list<sax::Attr> attrs
            = std::initializer_list<sax::Attr>{})
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
