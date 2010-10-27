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

#ifndef CONDITIONALEXPRESSION_HXX
#define CONDITIONALEXPRESSION_HXX

#include "dllapi.h"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <rtl/ustring.hxx>

#include <boost/shared_ptr.hpp>

#include <map>

//........................................................................
namespace rptui
{
//........................................................................

    // =============================================================================
    // = ConditionalExpression
    // =============================================================================
    class REPORTDESIGN_DLLPUBLIC ConditionalExpression
    {
    private:
        const ::rtl::OUString       m_sPattern;

    public:
        ConditionalExpression( const sal_Char* _pAsciiPattern );

        /** assembles an expression string from a field data source, and one or two operands
        */
        ::rtl::OUString     assembleExpression( const ::rtl::OUString& _rFieldDataSource, const ::rtl::OUString& _rLHS, const ::rtl::OUString& _rRHS ) const;

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
        bool                matchExpression( const ::rtl::OUString& _rExpression, const ::rtl::OUString& _rFieldDataSource, ::rtl::OUString& _out_rLHS, ::rtl::OUString& _out_rRHS ) const;
    };

    //========================================================================
    //= ConditionType
    //========================================================================
    enum ConditionType
    {
        eFieldValueComparison   = 0,
        eExpression             = 1
    };

    //========================================================================
    //= ComparisonOperation
    //========================================================================
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

    typedef ::boost::shared_ptr< ConditionalExpression >                PConditionalExpression;
    typedef ::std::map< ComparisonOperation, PConditionalExpression >   ConditionalExpressions;

    // =============================================================================
    // = ConditionalExpressionFactory
    // =============================================================================
    class REPORTDESIGN_DLLPUBLIC ConditionalExpressionFactory
    {
    public:
        /// fills the given map with all ConditionalExpressions which we know
        static size_t  getKnownConditionalExpressions( ConditionalExpressions& _out_rCondExp );

    private:
        ConditionalExpressionFactory();                                                 // never implemented
        ConditionalExpressionFactory( const ConditionalExpressionFactory& );            // never implemented
        ConditionalExpressionFactory& operator=( const ConditionalExpressionFactory& ); // never implemented
    };
//........................................................................
} // namespace rptui
//........................................................................

#endif // CONDITIONALEXPRESSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
