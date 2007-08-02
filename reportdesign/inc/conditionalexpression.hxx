/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conditionalexpression.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:27:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
