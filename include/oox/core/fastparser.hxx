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

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <sal/types.h>
#include <oox/dllapi.h>

namespace com::sun::star {
    namespace io { class XInputStream; }
    namespace xml::sax { class XFastDocumentHandler; }
    namespace xml::sax { class XFastTokenHandler; }
    namespace xml::sax { struct InputSource; }
}

namespace oox {
    struct NamespaceMap;
    class StorageBase;
}

namespace sax_fastparser {
    class FastSaxParser;
}

namespace oox::core {


/** Wrapper for a fast SAX parser that works on automatically generated OOXML
    token and namespace identifiers.
 */
class OOX_DLLPUBLIC FastParser
{
public:
    /// @throws css::uno::RuntimeException
    explicit            FastParser();

                        FastParser(const FastParser&) = delete;
                        FastParser& operator=(const FastParser&) = delete;

                        ~FastParser();

    /** Registers an OOXML namespace at the parser.
        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    void                registerNamespace( sal_Int32 nNamespaceId );

    /** Sets the passed document handler that will receive the SAX parser events.
        @throws css::uno::RuntimeException
    */
    void                setDocumentHandler(
                            const css::uno::Reference< css::xml::sax::XFastDocumentHandler >& rxDocHandler );

    void                clearDocumentHandler();

    /** Parses the passed SAX input source.
        @param bCloseStream  True = closes the stream in the input source after parsing.
        @throws css::xml::sax::SAXException
        @throws css::io::IOException
        @throws css::uno::RuntimeException
    */
    void                parseStream( const css::xml::sax::InputSource& rInputSource, bool bCloseStream = false );

    /** Parses the passed input stream.
        @param bCloseStream  True = closes the passed stream after parsing.
        @throws css::xml::sax::SAXException
        @throws css::io::IOException
        @throws css::uno::RuntimeException
    */
    void                parseStream(
                            const css::uno::Reference< css::io::XInputStream >& rxInStream,
                            const OUString& rStreamName );

    /** Parses a stream from the passed storage with the specified name.
        @param bCloseStream  True = closes the stream after parsing.
        @throws css::xml::sax::SAXException
        @throws css::io::IOException
        @throws css::uno::RuntimeException
    */
    void                parseStream( StorageBase& rStorage, const OUString& rStreamName );

    const css::uno::Reference< css::xml::sax::XFastTokenHandler >&
               getTokenHandler() const { return mxTokenHandler; }

private:
    css::uno::Reference<css::xml::sax::XFastTokenHandler>   mxTokenHandler;
    const NamespaceMap&                                     mrNamespaceMap;
    rtl::Reference<sax_fastparser::FastSaxParser>           mxParser;
};


} // namespace oox::core

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
