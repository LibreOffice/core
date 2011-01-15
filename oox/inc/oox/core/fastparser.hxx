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

#ifndef OOX_CORE_FASTPARSER_HXX
#define OOX_CORE_FASTPARSER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>

namespace oox {
    struct NamespaceMap;
    class StorageBase;
}

namespace oox {
namespace core {

// ============================================================================

/** Wrapper for a fast SAX parser that works on automatically generated OOXML
    token and namespace identifiers.
 */
class FastParser
{
public:
    explicit            FastParser(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual             ~FastParser();

    /** Registers an OOXML namespace at the parser. */
    void                registerNamespace( sal_Int32 nNamespaceId )
                            throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    /** Sets the passed document handler that will receive the SAX parser events. */
    void                setDocumentHandler(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler >& rxDocHandler )
                            throw( ::com::sun::star::uno::RuntimeException );

    /** Parses the passed SAX input source.
        @param bCloseStream  True = closes the stream in the input source after parsing. */
    void                parseStream( const ::com::sun::star::xml::sax::InputSource& rInputSource, bool bCloseStream = false )
                            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

    /** Parses the passed input stream.
        @param bCloseStream  True = closes the passed stream after parsing. */
    void                parseStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
                            const ::rtl::OUString& rStreamName, bool bCloseStream = false )
                            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

    /** Parses a stream from the passed storage with the specified name.
        @param bCloseStream  True = closes the stream after parsing. */
    void                parseStream( StorageBase& rStorage, const ::rtl::OUString& rStreamName, bool bCloseStream = false )
                            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastParser >
                        mxParser;
    const NamespaceMap& mrNamespaceMap;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif
