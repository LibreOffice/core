/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
