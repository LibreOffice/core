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

#ifndef _SAX_FS_HELPER_HXX_
#define _SAX_FS_HELPER_HXX_

#include <com/sun/star/uno/XReference.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <stdarg.h>
#include <boost/shared_ptr.hpp>
#include <sax/fastattribs.hxx>
#include "sax/dllapi.h"

#define FSNS(namespc, element) ((namespc << 16) | element)
const sal_Int32 FSEND = -1; // same as XML_TOKEN_INVALID

namespace sax_fastparser {

enum MergeMarksEnum { MERGE_MARKS_APPEND = 0, MERGE_MARKS_PREPEND = 1, MERGE_MARKS_POSTPONE = 2 };

typedef ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList > XFastAttributeListRef;

class FastSaxSerializer;

class SAX_DLLPUBLIC FastSerializerHelper
{
public:

    FastSerializerHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream );

    ~FastSerializerHelper();

    void startElement(const char* elementName, ...);
    void singleElement(const char* elementName, ...);
    void endElement(const char* elementName);

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

    void singleElement(const char* elementName, XFastAttributeListRef xAttrList);
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
    FastSerializerHelper* write(float value);
    FastSerializerHelper* write(double value);

    FastSerializerHelper* writeEscaped(const char* value);
    FastSerializerHelper* writeEscaped(const rtl::OUString& value);

    FastSerializerHelper* writeId(sal_Int32 tokenId);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > getOutputStream();

    FastAttributeList *createAttrList();

    void mark();
    void mergeTopMarks( MergeMarksEnum eMergeType = MERGE_MARKS_APPEND );

private:

    FastSaxSerializer* mpSerializer;
    com::sun::star::uno::Reference<com::sun::star::xml::sax::XFastTokenHandler> mxTokenHandler;

};

typedef boost::shared_ptr< FastSerializerHelper > FSHelperPtr;

}

#endif // _SAX_FS_HELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
