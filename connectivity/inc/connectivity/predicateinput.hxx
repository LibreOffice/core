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

#ifndef CONNECTIVITY_PREDICATEINPUT_HXX
#define CONNECTIVITY_PREDICATEINPUT_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/i18n/XLocaleData4.hpp>
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
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData4 >
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

        ::rtl::OUString getPredicateValue(
            const ::rtl::OUString& _sField
            , const ::rtl::OUString& _rPredicateValue
            , sal_Bool _bForStatementUse
            , ::rtl::OUString* _pErrorMessage = NULL) const;

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

        ::rtl::OUString implParseNode(::connectivity::OSQLParseNode* pParseNode,sal_Bool _bForStatementUse) const;
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // CONNECTIVITY_PREDICATEINPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
