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

#ifndef INCLUDED_EXTENSIONS_SOURCE_LOGGING_LOGHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_LOGGING_LOGHANDLER_HXX

#include <com/sun/star/logging/XLogFormatter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/logging/LogRecord.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/string.hxx>


namespace logging
{

    class LogHandlerHelper
    {
    private:
        // <attributes>
        rtl_TextEncoding            m_eEncoding;
        sal_Int32                   m_nLevel;
        css::uno::Reference< css::logging::XLogFormatter >
                                    m_xFormatter;
        // <//attributes>

        css::uno::Reference< css::uno::XComponentContext >
                                    m_xContext;
        ::osl::Mutex&               m_rMutex;
        ::cppu::OBroadcastHelper&   m_rBHelper;
        bool                        m_bInitialized;

    public:
        LogHandlerHelper(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            ::osl::Mutex& _rMutex,
            ::cppu::OBroadcastHelper& _rBHelper
        );

    public:
        bool    getIsInitialized() const { return m_bInitialized; }
        void    setIsInitialized() { m_bInitialized = true; }

        bool    getEncoding(        OUString& _out_rEncoding ) const;
        bool    setEncoding( const  OUString& _rEncoding     );

        inline  rtl_TextEncoding
                getTextEncoding() const { return m_eEncoding; }

        inline  css::uno::Reference< css::logging::XLogFormatter >
                getFormatter() const { return m_xFormatter; }
        inline  void
                setFormatter( const css::uno::Reference< css::logging::XLogFormatter >& _rxFormatter )
                {
                    m_xFormatter = _rxFormatter;
                }

        inline  sal_Int32
                getLevel() const { return m_nLevel; }
        inline  void
                setLevel( const sal_Int32 _nLevel )
                {
                    m_nLevel = _nLevel;
                }

        /** prepares implementation of an public accessible method of a log handler

            <code>enterMethod</code> does the following things:
            <ul><li>It acquires the mutex given in the constructor.</li>
                <li>It checks whether the component is already initialized, and throws an exception if not os.</li>
                <li>It checks whether the component is already disposed, and throws an exception if not os.</li>
                <li>It creates a default formatter (PlainTextFormatter), if no formatter exists at this time.</li>
            </ul>
        */
        void enterMethod();

        /** formats a record for publishing it

            The method first checks whether the records log level is greater or equal our own
            log level. If not, <FALSE/> is returned.

            Second, our formatter is used to create a unicode string from the log record. If an error occurs
            during this, e.g. if the formatter is <NULL/> or throws an exception during formatting,
            <FALSE/> is returned.

            Finally, the unicode string is encoded into a byte string, using our encoding setting. Then,
            <TRUE/> is returned.
        */
        bool    formatForPublishing( const css::logging::LogRecord& _rRecord, OString& _out_rEntry ) const;

        /** retrieves our formatter's heading, encoded with our encoding

            @return <TRUE/> in case of success, <FALSE/> if any error occurred
        */
        bool    getEncodedHead( OString& _out_rHead ) const;

        /** retrieves our formatter's tail, encoded with our encoding

            @return <TRUE/> in case of success, <FALSE/> if any error occurred
        */
        bool    getEncodedTail( OString& _out_rTail ) const;

        /** initializes the instance from a collection of named settings

            The recognized named settings are <code>Encoding</code>, <code>Formatter</code>, and <code>Level</code>,
            which initialize the respective attributes.

            Settings which are recognized are remove from the given collection. This allows
            the caller to determine whether or not the collection contained any unsupported
            items, and react appropriately.

            @throws IllegalArgumentException
                if one of the values in the collection is of wrong type.
        */
        void    initFromSettings( const ::comphelper::NamedValueCollection& _rSettings );
    };


} // namespace logging


#endif // INCLUDED_EXTENSIONS_SOURCE_LOGGING_LOGHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
