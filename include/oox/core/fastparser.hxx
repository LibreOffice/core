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

#ifndef INCLUDED_OOX_CORE_FASTPARSER_HXX
#define INCLUDED_OOX_CORE_FASTPARSER_HXX

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
    namespace uno { class XComponentContext; }
    namespace xml { namespace sax { class XFastDocumentHandler; } }
    namespace xml { namespace sax { class XFastParser; } }
    namespace xml { namespace sax { class XFastTokenHandler; } }
    namespace xml { namespace sax { struct InputSource; } }
} } }

namespace oox {
    struct NamespaceMap;
    class StorageBase;
}

namespace sax_fastparser {
    class FastSaxParser;
}

namespace oox {
namespace core {


/** Wrapper for a fast SAX parser that works on automatically generated OOXML
    token and namespace identifiers.
 */
class FastParser
{
public:
    explicit            FastParser(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext )
                            throw( css::uno::RuntimeException );

    virtual             ~FastParser();

    /** Registers an OOXML namespace at the parser. */
    void                registerNamespace( sal_Int32 nNamespaceId )
                            throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    /** Sets the passed document handler that will receive the SAX parser events. */
    void                setDocumentHandler(
                            const css::uno::Reference< css::xml::sax::XFastDocumentHandler >& rxDocHandler )
                            throw( css::uno::RuntimeException );

    /** Parses the passed SAX input source.
        @param bCloseStream  True = closes the stream in the input source after parsing. */
    void                parseStream( const css::xml::sax::InputSource& rInputSource, bool bCloseStream = false )
                            throw( css::xml::sax::SAXException, css::io::IOException, css::uno::RuntimeException );

    /** Parses the passed input stream.
        @param bCloseStream  True = closes the passed stream after parsing. */
    void                parseStream(
                            const css::uno::Reference< css::io::XInputStream >& rxInStream,
                            const OUString& rStreamName )
                            throw( css::xml::sax::SAXException, css::io::IOException, css::uno::RuntimeException );

    /** Parses a stream from the passed storage with the specified name.
        @param bCloseStream  True = closes the stream after parsing. */
    void                parseStream( StorageBase& rStorage, const OUString& rStreamName )
                            throw( css::xml::sax::SAXException, css::io::IOException, css::uno::RuntimeException );

    const css::uno::Reference< css::xml::sax::XFastTokenHandler >&
               getTokenHandler() const { return mxTokenHandler; }

private:
    css::uno::Reference< css::xml::sax::XFastParser >
                        mxParser;
    css::uno::Reference< css::xml::sax::XFastTokenHandler >
                        mxTokenHandler;
    const NamespaceMap& mrNamespaceMap;

    sax_fastparser::FastSaxParser* mpParser;
};


} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
