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

#ifndef _SAX_FS_HELPER_HXX_
#define _SAX_FS_HELPER_HXX_

#include <com/sun/star/uno/XReference.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <stdarg.h>
#include <boost/shared_ptr.hpp>
#include <sax/fastattribs.hxx>

#define FSNS(namespc, element) ((namespc << 16) | element)
const sal_Int32 FSEND = -1; // same as XML_TOKEN_INVALID

namespace sax_fastparser {

enum MergeMarksEnum { MERGE_MARKS_APPEND = 0, MERGE_MARKS_PREPEND = 1, MERGE_MARKS_POSTPONE = 2 };

typedef ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList > XFastAttributeListRef;

class FastSaxSerializer;

class SAX_DLLPUBLIC FastSerializerHelper
{
public:

    FastSerializerHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream, bool bWriteHeader = true );

    ~FastSerializerHelper();

    void startElementV(sal_Int32 elementTokenId, va_list args);
    void singleElementV(sal_Int32 elementTokenId, va_list args);

    inline void startElement(sal_Int32 elementTokenId, ...)
        { va_list args; va_start( args, elementTokenId ); startElementV( elementTokenId, args ); va_end( args ); }
    inline void singleElement(sal_Int32 elementTokenId, ...)
        { va_list args; va_start( args, elementTokenId ); singleElementV( elementTokenId, args ); va_end( args ); }
    inline void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, ...)
        { va_list args; va_start( args, elementTokenId ); startElementV( FSNS( namespaceTokenId, elementTokenId), args ); va_end( args ); }
    inline void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, ...)
        { va_list args; va_start( args, elementTokenId ); singleElementV( FSNS( namespaceTokenId, elementTokenId), args ); va_end( args ); }
    void endElement(sal_Int32 elementTokenId);
    inline void endElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId)
        { endElement( FSNS( namespaceTokenId, elementTokenId ) ); }

    inline void singleElement(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
        { singleElementV(elementTokenId, xAttrList); }
    void singleElementV(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList);
    inline void singleElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
        { singleElementV(FSNS( namespaceTokenId, elementTokenId), xAttrList); }

    void startElementV(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList);
    inline void startElementNS(sal_Int32 namespaceTokenId, sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
        { startElementV( FSNS( namespaceTokenId, elementTokenId ), xAttrList ); }

    FastSerializerHelper* write(const char* value);
    FastSerializerHelper* write(const rtl::OUString& value);
    FastSerializerHelper* write(sal_Int32 value);
    FastSerializerHelper* write(sal_Int64 value);
    FastSerializerHelper* write(double value);

    FastSerializerHelper* writeEscaped(const char* value);
    FastSerializerHelper* writeEscaped(const rtl::OUString& value);

    FastSerializerHelper* writeId(sal_Int32 tokenId);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > getOutputStream();

    FastAttributeList *createAttrList();

    void mark( ::com::sun::star::uno::Sequence< sal_Int32 > aOrder =
            ::com::sun::star::uno::Sequence< sal_Int32 >() );
    void mergeTopMarks( MergeMarksEnum eMergeType = MERGE_MARKS_APPEND );

private:

    FastSaxSerializer* mpSerializer;
    com::sun::star::uno::Reference<com::sun::star::xml::sax::XFastTokenHandler> mxTokenHandler;

};

typedef boost::shared_ptr< FastSerializerHelper > FSHelperPtr;

}

#endif // _SAX_FS_HELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
