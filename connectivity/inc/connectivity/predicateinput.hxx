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

#ifndef CONNECTIVITY_PREDICATEINPUT_HXX
#define CONNECTIVITY_PREDICATEINPUT_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <connectivity/sqlparse.hxx>
#include "connectivity/dbtoolsdllapi.hxx"

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=====================================================================
    //= OPredicateInputController
    //=====================================================================
    /** A class which allows input of an SQL predicate for a row set column
        into a edit field.
    */
    class OOO_DLLPUBLIC_DBTOOLS OPredicateInputController
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                m_xFormatter;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData >
                m_xLocaleData;

        ::connectivity::OSQLParser
                m_aParser;

    public:
        OPredicateInputController(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::connectivity::IParseContext* _pParseContext = NULL
        );

        /** transforms a "raw" predicate value (usually obtained from a user input) into a valid predicate for the given column
        @param _rPredicateValue
            The text to normalize.
        @param _rxField
            The field for which the text should be a predicate value.
        @param _pErrorMessage
            If not <NULL/>, and a parsing error occurs, the error message will be copied to the string the argument
            points to.
        */
        sal_Bool        normalizePredicateString(
            ::rtl::OUString& _rPredicateValue,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
            ::rtl::OUString* _pErrorMessage = NULL
        ) const;

        /** get's a value of the predicate which can be used in a WHERE clause.
        @param _rPredicateValue
            the value which has been normalized using normalizePredicateString
        @param _rxField
            is the field for which a predicate is to be entered
        @param _bForStatementUse
            If <TRUE/>, the returned value can be used in an SQL statement. If <FALSE/>, it can be used
            for instance for setting parameter values.
        @param _pErrorMessage
            If not <NULL/>, and a parsing error occurs, the error message will be copied to the string the argument
            points to.
        @see normalizePredicateString
        */
        ::rtl::OUString getPredicateValue(
            const ::rtl::OUString& _rPredicateValue,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField,
            sal_Bool _bForStatementUse,
            ::rtl::OUString* _pErrorMessage = NULL
        ) const;

    private:
        ::connectivity::OSQLParseNode* implPredicateTree(
            ::rtl::OUString& _rErrorMessage,
            const ::rtl::OUString& _rStatement,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField
        ) const;

        sal_Bool getSeparatorChars(
            const ::com::sun::star::lang::Locale& _rLocale,
            sal_Unicode& _rDecSep,
            sal_Unicode& _rThdSep
        ) const;
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // CONNECTIVITY_PREDICATEINPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
