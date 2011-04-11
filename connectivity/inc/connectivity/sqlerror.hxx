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

#ifndef CONNECTIVITY_SQLERROR_HXX
#define CONNECTIVITY_SQLERROR_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/SQLException.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    //====================================================================
    //= ErrorCondition
    //====================================================================
    /** the type of error codes to be used in SQLExceptions

        @see com::sun::star::sdbc::SQLException::ErrorCode
    */
    typedef ::sal_Int32 ErrorCode;

    /** error condition values as defined in <type scope="com::sun::star::sdb">ErrorCondition</type>
    */
    typedef ::sal_Int32 ErrorCondition;

    //====================================================================
    //= SQLError
    //====================================================================
    class SQLError_Impl;

    /** a class which provides helpers for working with SQLErrors

        In particular, this class provides vendor-specific error codes (where
        the vendor is OpenOffice.org Base), which can be used in OOo's various
        database drivers, and checked in application-level code, to properly
        recognize highly specific error conditions.

        @see ::com::sun::star::sdb::ErrorCondition
    */
    class OOO_DLLPUBLIC_DBTOOLS SQLError
    {
    public:
        // --------------------------------------------------------------------
        // - optional
        // --------------------------------------------------------------------
        /** convenience wrapper around boost::optional, allowing implicit construction
        */
        class ParamValue : public ::boost::optional< ::rtl::OUString >
        {
            typedef ::boost::optional< ::rtl::OUString >  base_type;

        public:
            ParamValue( ) : base_type( ) { }
            ParamValue( ::rtl::OUString const& val ) : base_type( val ) { }
            ParamValue( ParamValue const& rhs ) : base_type( (base_type const&)rhs ) { }

            bool    is() const { return !base_type::operator!(); }
        };


    public:
        explicit        SQLError( const ::comphelper::ComponentContext& _rContext );
                        ~SQLError();

        /** returns the message associated with a given error condition, after (optionally) replacing
            a placeholder with a given string

            Some error messages need to contain references to runtime-dependent data (say, the
            name of a concrete table in the database), which in the resource file's strings are
            represented by a placeholder, namely $1$, $2, and so on. This method allows to
            retrieve such an error message, and replace upo to 3 placeholders with their concrete
            values.

            In a non-product build, assertions will fire if the number of placeholders in the
            message's resource string does not match the number of passed parameter values.

            As specified in the <type scope="com::sun::star::sdb">ErrorCondition</type> type,
            error messages thrown by core components of OpenOffice.org Base will contain
            a standardized prefix &quot;[OOoBase]&quot; in every message.

            @param _rParamValue1
                the value which the placeholder $1$ should be replaced with. If this value is
                not present (see <code>::boost::optional::operator !</code>), then no replacement
                will happen, and <code>_rParamValue2</code> and <code>_rParamValue3</code> will be
                ignored.

            @param _rParamValue2
                the value which the placeholder $2$ should be replaced with. If this value is
                not present (see <code>::boost::optional::operator !</code>), then no replacement
                will happen, and <code>_rParamValue3</code> will be ignored.

            @param _rParamValue1
                the value which the placeholder $1$ should be replaced with. If this value is
                not present (see <code>::boost::optional::operator !</code>), then no replacement
                will happen.

            @see ::com::sun::star::sdb::ErrorCondition
        */
        ::rtl::OUString getErrorMessage(
                            const ErrorCondition _eCondition,
                            const ParamValue& _rParamValue1 = ParamValue(),
                            const ParamValue& _rParamValue2 = ParamValue(),
                            const ParamValue& _rParamValue3 = ParamValue()
                        ) const;

        /** returns the SQLState associated with a given error condition

            @see getErrorMessage
            @see getErrorCode
            @see ::com::sun::star::sdb::ErrorCondition
            @see ::com::sun::star::sdbc::SQLException::SQLState
        */
        ::rtl::OUString getSQLState( const ErrorCondition _eCondition ) const;

        /** returns the error code associated with a given error condition

            @see getErrorMessage
            @see getSQLState
            @see ::com::sun::star::sdb::ErrorCondition
            @see ::com::sun::star::sdbc::SQLException::ErrorCode
        */
        static ErrorCode
                        getErrorCode( const ErrorCondition _eCondition );

        /** returns the prefix which is used for OpenOffice.org Base's error messages

            As specified in the <type scope="com::sun::star::sdb">ErrorCondition</type> type,
            error messages thrown by core components of OpenOffice.org Base will
            contain a standardized prefix in every message. <code>getBaseErrorMessagePrefix</code>
            returns this prefix, so clients of such error messages might decide to strip this
            prefix before presenting the message to the user, or use it to determine
            whether a concrete error has been raised by a OpenOffice.org core component.
        */
        static const ::rtl::OUString&
                        getMessagePrefix();


        /** throws an SQLException describing the given error condition

            The thrown SQLException will contain the OOo-specific error code which derives
            from the given error condition, and the error message associated with that condition.

            @param  _eCondition
                the ErrorCondition which hit you

            @param  _rxContext
                the context in which the error occurred. This will be filled in as
                <member scope="com::sun::star::uno">Exception::Context</member> member.

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
            @see getSQLState
        */
        void            raiseException(
                            const ErrorCondition _eCondition,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
                            const ParamValue& _rParamValue1 = ParamValue(),
                            const ParamValue& _rParamValue2 = ParamValue(),
                            const ParamValue& _rParamValue3 = ParamValue()
                        ) const;

        /** throws an SQLException describing the given error condition

            The thrown SQLException will contain the OOo-specific error code which derives
            from the given error condition, and the error message associated with that condition.

            Note: You should prefer the version of <type>raiseException</type> which takes
            an additional <type>Context</type> parameter, since this allows clients of your
            exception to examine where the error occurred.

            @param  _eCondition
                the ErrorCondition which hit you

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
            @see getSQLState
        */
        void            raiseException(
                            const ErrorCondition _eCondition,
                            const ParamValue& _rParamValue1 = ParamValue(),
                            const ParamValue& _rParamValue2 = ParamValue(),
                            const ParamValue& _rParamValue3 = ParamValue()
                        ) const;

        /** raises a typed exception, that is, a UNO exception which is derived from
            <type scope="com::sun::star::sdbc">SQLException</type>

            @param  _eCondition
                the ErrorCondition which hit you

            @param  _rxContext
                the context in which the error occurred. This will be filled in as
                <member scope="com::sun::star::uno">Exception::Context</member> member.

            @param _rExceptionType
                the type of the exception to throw. This type <em>must</em> specify
                an exception class derived from <type scope="com::sun::star::sdbc">SQLException</type>.

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

            @throws ::std::bad_cast
                if <arg>_rExceptionType</arg> does not specify an exception class derived from
                <type scope="com::sun::star::sdbc">SQLException</type>.

            @see getErrorMessage
            @see getErrorCode
            @see getSQLState
        */
        void            raiseTypedException(
                            const ErrorCondition _eCondition,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
                            const ::com::sun::star::uno::Type& _rExceptionType,
                            const ParamValue& _rParamValue1 = ParamValue(),
                            const ParamValue& _rParamValue2 = ParamValue(),
                            const ParamValue& _rParamValue3 = ParamValue()
                        ) const;

        /** retrieves an <code>SQLException</code> object which contains information about
            the given error condition

            @param  _eCondition
                the ErrorCondition which hit you

            @param  _rxContext
                the context in which the error occurred. This will be filled in as
                <member scope="com::sun::star::uno">Exception::Context</member> member.

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
            @see getSQLState
        */
        ::com::sun::star::sdbc::SQLException
                        getSQLException(
                            const ErrorCondition _eCondition,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
                            const ParamValue& _rParamValue1 = ParamValue(),
                            const ParamValue& _rParamValue2 = ParamValue(),
                            const ParamValue& _rParamValue3 = ParamValue()
                        ) const;

    private:
        ::boost::shared_ptr< SQLError_Impl > m_pImpl;
    };

//........................................................................
} // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_SQLERROR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
