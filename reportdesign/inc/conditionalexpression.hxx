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

#ifndef INCLUDED_REPORTDESIGN_INC_CONDITIONALEXPRESSION_HXX
#define INCLUDED_REPORTDESIGN_INC_CONDITIONALEXPRESSION_HXX

#include "dllapi.h"

#include <rtl/ustring.hxx>
#include <map>
#include <memory>

namespace rptui
{



    // = ConditionalExpression

    class REPORTDESIGN_DLLPUBLIC ConditionalExpression
    {
    private:
        const OUString       m_sPattern;

    public:
        ConditionalExpression( const sal_Char* _pAsciiPattern );

        /** assembles an expression string from a field data source, and one or two operands
        */
        OUString     assembleExpression( const OUString& _rFieldDataSource, const OUString& _rLHS, const OUString& _rRHS ) const;

        /** matches the given expression string to the expression pattern represented by the object
            @param  _rExpression
                the expression to match
            @param  _rFieldDataSource
                the field data source
            @param  _out_rLHS
                output parameter taking the left hand side operand, if successful
            @param  _out_rRHS
                output parameter taking the right hand side operand, if successful
            @return
                <TRUE/> if and only if the expression string could be successfully matched to
                the pattern.
        */
        bool                matchExpression( const OUString& _rExpression, const OUString& _rFieldDataSource, OUString& _out_rLHS, OUString& _out_rRHS ) const;
    };


    //= ConditionType

    enum ConditionType
    {
        eFieldValueComparison   = 0,
        eExpression             = 1
    };


    //= ComparisonOperation

    enum ComparisonOperation
    {
        eBetween        = 0,
        eNotBetween     = 1,
        eEqualTo        = 2,
        eNotEqualTo     = 3,
        eGreaterThan    = 4,
        eLessThan       = 5,
        eGreaterOrEqual = 6,
        eLessOrEqual    = 7
    };

    typedef std::shared_ptr< ConditionalExpression >                PConditionalExpression;
    typedef ::std::map< ComparisonOperation, PConditionalExpression >   ConditionalExpressions;


    // = ConditionalExpressionFactory

    class REPORTDESIGN_DLLPUBLIC ConditionalExpressionFactory
    {
    public:
        /// fills the given map with all ConditionalExpressions which we know
        static size_t  getKnownConditionalExpressions( ConditionalExpressions& _out_rCondExp );

    private:
        ConditionalExpressionFactory( const ConditionalExpressionFactory& ) = delete;
        ConditionalExpressionFactory& operator=( const ConditionalExpressionFactory& ) = delete;
    };

} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_INC_CONDITIONALEXPRESSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
