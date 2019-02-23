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

#ifndef INCLUDED_CONNECTIVITY_SQLERROR_HXX
#define INCLUDED_CONNECTIVITY_SQLERROR_HXX

#include <com/sun/star/sdbc/SQLException.hpp>
#include <connectivity/dbtoolsdllapi.hxx>
#include <boost/optional.hpp>
#include <memory>

namespace connectivity
{


    //= ErrorCondition

    /** the type of error codes to be used in SQLExceptions

        @see css::sdbc::SQLException::ErrorCode
    */
    typedef ::sal_Int32 ErrorCode;

    /** error condition values as defined in css::sdb::ErrorCondition
    */
    typedef ::sal_Int32 ErrorCondition;


    //= SQLError

    class SQLError_Impl;

    /** a class which provides helpers for working with SQLErrors

        In particular, this class provides vendor-specific error codes (where
        the vendor is OpenOffice.org Base), which can be used in OOo's various
        database drivers, and checked in application-level code, to properly
        recognize highly specific error conditions.

        @see css::sdb::ErrorCondition
    */
    class OOO_DLLPUBLIC_DBTOOLS SQLError
    {
    public:

        // - optional

        /** convenience wrapper around boost::optional, allowing implicit construction
        */
        class ParamValue : public ::boost::optional< OUString >
        {
            typedef ::boost::optional< OUString >  base_type;

        public:
            ParamValue( ) : base_type( ) { }
            ParamValue( OUString const& val ) : base_type( val ) { }
            ParamValue( ParamValue const& rhs ) : base_type( static_cast<base_type const&>( rhs ) ) { }

            bool    is() const { return !base_type::operator!(); }
        };


    public:
                        SQLError();
                        ~SQLError();

        /** returns the message associated with a given error condition, after (optionally) replacing
            a placeholder with a given string

            Some error messages need to contain references to runtime-dependent data (say, the
            name of a concrete table in the database), which in the resource file's strings are
            represented by a placeholder, namely $1$, $2, and so on. This method allows to
            retrieve such an error message, and replace up to 3 placeholders with their concrete
            values.

            In a non-product build, assertions will fire if the number of placeholders in the
            message's resource string does not match the number of passed parameter values.

            As specified in the css::sdb::ErrorCondition type,
            error messages thrown by core components of OpenOffice.org Base will contain
            a standardized prefix &quot;[OOoBase]&quot; in every message.

            @see css::sdb::ErrorCondition
        */
        OUString getErrorMessage(
                            const ErrorCondition _eCondition
                        ) const;

        /** returns the error code associated with a given error condition

            @see getErrorMessage
            @see css::sdb::ErrorCondition
            @see css::sdbc::SQLException::ErrorCode
        */
        static ErrorCode
                        getErrorCode( const ErrorCondition _eCondition );

        /** returns the prefix which is used for OpenOffice.org Base's error messages

            As specified in the css::sdb::ErrorCondition type,
            error messages thrown by core components of OpenOffice.org Base will
            contain a standardized prefix in every message. <code>getBaseErrorMessagePrefix</code>
            returns this prefix, so clients of such error messages might decide to strip this
            prefix before presenting the message to the user, or use it to determine
            whether a concrete error has been raised by a OpenOffice.org core component.
        */
        static const OUString&
                        getMessagePrefix();


        /** throws an SQLException describing the given error condition

            The thrown SQLException will contain the OOo-specific error code which derives
            from the given error condition, and the error message associated with that condition.

            @param  _eCondition
                the ErrorCondition which hit you

            @param  _rxContext
                the context in which the error occurred. This will be filled in as
                <member scope="css::uno">Exception::Context</member> member.

            @param _rParamValue1
                a runtime-dependent value which should be filled into the error message
                which is associated with <arg>_eCondition</arg>, replacing the first placeholder
                in this message.

            @param _rParamValue2
                a runtime-dependent value which should be filled into the error message
                which is associated with <arg>_eCondition</arg>, replacing the second placeholder
                in this message.

            @param _rParamValue3
                a runtime-dependent value which should be filled into the error message
                which is associated with <arg>_eCondition</arg>, replacing the third placeholder
                in this message.

            @see getErrorMessage
            @see getErrorCode
        */
        void            raiseException(
                            const ErrorCondition _eCondition,
                            const css::uno::Reference< css::uno::XInterface >& _rxContext,
                            const ParamValue& _rParamValue1 = ParamValue(),
                            const ParamValue& _rParamValue2 = ParamValue(),
                            const ParamValue& _rParamValue3 = ParamValue()
                        ) const;

        /** throws an SQLException describing the given error condition

            The thrown SQLException will contain the OOo-specific error code which derives
            from the given error condition, and the error message associated with that condition.

            Note: You should prefer the version of raiseException which takes
            an additional Context parameter, since this allows clients of your
            exception to examine where the error occurred.

            @param  _eCondition
                the ErrorCondition which hit you

            @see getErrorMessage
            @see getErrorCode
        */
        void            raiseException(
                            const ErrorCondition _eCondition
                        ) const;

        /** raises a typed exception, that is, a UNO exception which is derived from
            css::sdbc::SQLException

            @param  _eCondition
                the ErrorCondition which hit you

            @param  _rxContext
                the context in which the error occurred. This will be filled in as
                <member scope="css::uno">Exception::Context</member> member.

            @param _rExceptionType
                the type of the exception to throw. This type <em>must</em> specify
                an exception class derived from css::sdbc::SQLException.

            @throws ::std::bad_cast
                if <arg>_rExceptionType</arg> does not specify an exception class derived from
                css::sdbc::SQLException.

            @see getErrorMessage
            @see getErrorCode
        */
        void            raiseTypedException(
                            const ErrorCondition _eCondition,
                            const css::uno::Reference< css::uno::XInterface >& _rxContext,
                            const css::uno::Type& _rExceptionType
                        ) const;

        /** retrieves an <code>SQLException</code> object which contains information about
            the given error condition

            @param  _eCondition
                the ErrorCondition which hit you

            @param  _rxContext
                the context in which the error occurred. This will be filled in as
                <member scope="css::uno">Exception::Context</member> member.

            @param _rParamValue1
                a runtime-dependent value which should be filled into the error message
                which is associated with <arg>_eCondition</arg>, replacing the first placeholder
                in this message.

            @param _rParamValue2
                a runtime-dependent value which should be filled into the error message
                which is associated with <arg>_eCondition</arg>, replacing the second placeholder
                in this message.

            @param _rParamValue3
                a runtime-dependent value which should be filled into the error message
                which is associated with <arg>_eCondition</arg>, replacing the third placeholder
                in this message.

            @see getErrorMessage
            @see getErrorCode
        */
        css::sdbc::SQLException
                        getSQLException(
                            const ErrorCondition _eCondition,
                            const css::uno::Reference< css::uno::XInterface >& _rxContext,
                            const ParamValue& _rParamValue1 = ParamValue(),
                            const ParamValue& _rParamValue2 = ParamValue(),
                            const ParamValue& _rParamValue3 = ParamValue()
                        ) const;

    private:
        std::shared_ptr< SQLError_Impl > m_pImpl;
    };


} // namespace connectivity


#endif // INCLUDED_CONNECTIVITY_SQLERROR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
