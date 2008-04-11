/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loghandler.hxx,v $
 * $Revision: 1.3 $
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

#ifndef LOGHANDLER_HXX
#define LOGHANDLER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/logging/XLogFormatter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/logging/LogRecord.hpp>
/** === end UNO includes === **/

#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/string.hxx>

//........................................................................
namespace logging
{
//........................................................................

    //====================================================================
    //=
    //====================================================================
    class LogHandlerHelper
    {
    private:
        // <attributes>
        rtl_TextEncoding    m_eEncoding;
        sal_Int32           m_nLevel;
        ::com::sun::star::uno::Reference< ::com::sun::star::logging::XLogFormatter >
                            m_xFormatter;
        // <//attributes>

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                    m_xContext;
        ::osl::Mutex&               m_rMutex;
        ::cppu::OBroadcastHelper&   m_rBHelper;
        bool                        m_bInitialized;

    public:
        LogHandlerHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            ::osl::Mutex& _rMutex,
            ::cppu::OBroadcastHelper& _rBHelper
        );

    public:
        bool    getIsInitialized() const { return m_bInitialized; }
        void    setIsInitialized() { m_bInitialized = true; }

        bool    getEncoding(        ::rtl::OUString& _out_rEncoding ) const;
        bool    setEncoding( const  ::rtl::OUString& _rEncoding     );

        inline  rtl_TextEncoding
                getTextEncoding() const { return m_eEncoding; }

        inline  ::com::sun::star::uno::Reference< ::com::sun::star::logging::XLogFormatter >
                getFormatter() const { return m_xFormatter; }
        inline  void
                setFormatter( const ::com::sun::star::uno::Reference< ::com::sun::star::logging::XLogFormatter >& _rxFormatter )
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
        bool    formatForPublishing( const ::com::sun::star::logging::LogRecord& _rRecord, ::rtl::OString& _out_rEntry ) const;

        /** retrieves our formatter's heading, encoded with our encoding

            @return <TRUE/> in case of success, <FALSE/> if any error occured
        */
        bool    getEncodedHead( ::rtl::OString& _out_rHead ) const;

        /** retrieves our formatter's tail, encoded with our encoding

            @return <TRUE/> in case of success, <FALSE/> if any error occured
        */
        bool    getEncodedTail( ::rtl::OString& _out_rTail ) const;

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

//........................................................................
} // namespace logging
//........................................................................

#endif // LOGHANDLER_HXX
