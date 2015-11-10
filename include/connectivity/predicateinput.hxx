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

#ifndef INCLUDED_CONNECTIVITY_PREDICATEINPUT_HXX
#define INCLUDED_CONNECTIVITY_PREDICATEINPUT_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/i18n/XLocaleData4.hpp>
#include <connectivity/sqlparse.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <com/sun/star/uno/Any.hxx>


namespace dbtools
{



    //= OPredicateInputController

    /** A class which allows input of an SQL predicate for a row set column
        into a edit field.
    */
    class OOO_DLLPUBLIC_DBTOOLS OPredicateInputController
    {
    private:
        css::uno::Reference< css::sdbc::XConnection >
                m_xConnection;
        css::uno::Reference< css::util::XNumberFormatter >
                m_xFormatter;
        css::uno::Reference< css::i18n::XLocaleData4 >
                m_xLocaleData;

        ::connectivity::OSQLParser
                m_aParser;

    public:
        OPredicateInputController(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const ::connectivity::IParseContext* _pParseContext = nullptr
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
        bool        normalizePredicateString(
            OUString& _rPredicateValue,
            const css::uno::Reference< css::beans::XPropertySet >& _rxField,
            OUString* _pErrorMessage = nullptr
        ) const;

        /** get the value of the predicate, as a string to be used in a WHERE clause
        @param _rPredicateValue
            the value which has been normalized using normalizePredicateString
        @param _rxField
            is the field for which a predicate is to be entered
        @param _pErrorMessage
            If not <NULL/>, and a parsing error occurs, the error message will be copied to the string the argument
            points to.
        @see normalizePredicateString
        */
        OUString getPredicateValueStr(
            const OUString& _rPredicateValue,
            const css::uno::Reference< css::beans::XPropertySet > & _rxField,
            OUString* _pErrorMessage = nullptr
        ) const;

        OUString getPredicateValueStr(
            const OUString& _sField
            , const OUString& _rPredicateValue
            , OUString* _pErrorMessage = nullptr) const;

        /** get the value of the predicate, either as an empty or as a string
        @param _rPredicateValue
            the value which has been normalized using normalizePredicateString
        @param _rxField
            is the field for which a predicate is to be entered
        @param _pErrorMessage
            If not <NULL/>, and a parsing error occurs, the error message will be copied to the string the argument
            points to.
        @see normalizePredicateString
        */
        css::uno::Any getPredicateValue(
            const OUString& _rPredicateValue,
            const css::uno::Reference< css::beans::XPropertySet > & _rxField,
            OUString* _pErrorMessage = nullptr
        ) const;

    private:
        ::connectivity::OSQLParseNode* implPredicateTree(
            OUString& _rErrorMessage,
            const OUString& _rStatement,
            const css::uno::Reference< css::beans::XPropertySet > & _rxField
        ) const;

        bool getSeparatorChars(
            const css::lang::Locale& _rLocale,
            sal_Unicode& _rDecSep,
            sal_Unicode& _rThdSep
        ) const;

        css::uno::Any implParseNode(::connectivity::OSQLParseNode* pParseNode, bool _bForStatementUse) const;
    };


}   // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_PREDICATEINPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
