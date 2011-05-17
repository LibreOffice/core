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

#include "conditionalexpression.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace rptui
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    // =============================================================================
    // = ConditionalExpression
    // =============================================================================
    // -----------------------------------------------------------------------------
    ConditionalExpression::ConditionalExpression( const sal_Char* _pAsciiPattern )
        :m_sPattern( ::rtl::OUString::createFromAscii( _pAsciiPattern ) )
    {
    }

    // -----------------------------------------------------------------------------
    ::rtl::OUString ConditionalExpression::assembleExpression( const ::rtl::OUString& _rFieldDataSource, const ::rtl::OUString& _rLHS, const ::rtl::OUString& _rRHS ) const
    {
        ::rtl::OUString sExpression( m_sPattern );

        sal_Int32 nPatternIndex = sExpression.indexOf( '$' );
        while ( nPatternIndex > -1 )
        {
            const ::rtl::OUString* pReplace = NULL;
            switch ( sExpression.getStr()[ nPatternIndex + 1 ] )
            {
            case '$': pReplace = &_rFieldDataSource; break;
            case '1': pReplace = &_rLHS; break;
            case '2': pReplace = &_rRHS; break;
            default: break;
            }

            if ( pReplace == NULL )
            {
                OSL_FAIL( "ConditionalExpression::assembleExpression: illegal pattern!" );
                break;
            }

            sExpression = sExpression.replaceAt( nPatternIndex, 2, *pReplace );
            nPatternIndex = sExpression.indexOf( '$', nPatternIndex + pReplace->getLength() + 1 );
        }
        return sExpression;
    }

    // -----------------------------------------------------------------------------
    bool ConditionalExpression::matchExpression( const ::rtl::OUString& _rExpression, const ::rtl::OUString& _rFieldDataSource, ::rtl::OUString& _out_rLHS, ::rtl::OUString& _out_rRHS ) const
    {
        (void)_rExpression;
        (void)_rFieldDataSource;
        (void)_out_rLHS;
        (void)_out_rRHS;

        // if we had regular expression, the matching would be pretty easy ...
        // just replace $1 and $2 in the pattern with (.*), and then get them with \1 resp. \2.
        // Unfortunately, we don't have such a regexp engine ...

        // Okay, let's start with replacing all $$ in our pattern with the actual field data source
        ::rtl::OUString sMatchExpression( m_sPattern );
        const ::rtl::OUString sFieldDataPattern( RTL_CONSTASCII_USTRINGPARAM( "$$" ) );
        sal_Int32 nIndex( sMatchExpression.indexOf( sFieldDataPattern ) );
        while ( nIndex != -1 )
        {
            sMatchExpression = sMatchExpression.replaceAt( nIndex, sFieldDataPattern.getLength(), _rFieldDataSource );
            nIndex = sMatchExpression.indexOf( sFieldDataPattern, nIndex + _rFieldDataSource.getLength() );
        }

        const ::rtl::OUString sLHSPattern( RTL_CONSTASCII_USTRINGPARAM( "$1" ) );
        const ::rtl::OUString sRHSPattern( RTL_CONSTASCII_USTRINGPARAM( "$2" ) );
        sal_Int32 nLHSIndex( sMatchExpression.indexOf( sLHSPattern ) );
        sal_Int32 nRHSIndex( sMatchExpression.indexOf( sRHSPattern ) );

        // now we should have at most one occurrence of $1 and $2, resp.
        OSL_ENSURE( sMatchExpression.indexOf( sLHSPattern, nLHSIndex + 1 ) == -1,
            "ConditionalExpression::matchExpression: unsupported pattern (more than one LHS occurrence)!" );
        OSL_ENSURE( sMatchExpression.indexOf( sRHSPattern, nRHSIndex + 1 ) == -1,
            "ConditionalExpression::matchExpression: unsupported pattern (more than one RHS occurrence)!" );
        // Also, an LHS must be present, and precede the RHS (if present)
        OSL_ENSURE( ( nLHSIndex != -1 ) && ( ( nLHSIndex < nRHSIndex ) || ( nRHSIndex == -1 ) ),
            "ConditionalExpression::matchExpression: no LHS, or an RHS preceeding the LHS - this is not supported!" );

        // up to the occurrence of the LHS (which must exist, see above), the two expressions
        // must be identical
        if ( _rExpression.getLength() < nLHSIndex )
            return false;
        const ::rtl::OUString sExprPart1( _rExpression.copy( 0, nLHSIndex ) );
        const ::rtl::OUString sMatchExprPart1( sMatchExpression.copy( 0, nLHSIndex ) );
        if ( sExprPart1 != sMatchExprPart1 )
            // the left-most expression parts do not match
            return false;

        // after the occurrence of the RHS (or the LHS, if there is no RHS), the two expressions
        // must be identical, too
        bool bHaveRHS( nRHSIndex != -1 );
        sal_Int32 nRightMostIndex( bHaveRHS ? nRHSIndex : nLHSIndex );
        const ::rtl::OUString sMatchExprPart3( sMatchExpression.copy( nRightMostIndex + 2 ) );
        if ( _rExpression.getLength() < sMatchExprPart3.getLength() )
            // the expression is not even long enough to hold the right-most part of the match expression
            return false;
        const ::rtl::OUString sExprPart3( _rExpression.copy( _rExpression.getLength() - sMatchExprPart3.getLength() ) );
        if ( sExprPart3 != sMatchExprPart3 )
            // the right-most expression parts do not match
            return false;

        // if we don't have an RHS, we're done
        if ( !bHaveRHS )
        {
            _out_rLHS = _rExpression.copy( sExprPart1.getLength(), _rExpression.getLength() - sExprPart1.getLength() - sExprPart3.getLength() );
            return true;
        }

        // strip the match expression by its right-most and left-most part, and by the placeholders $1 and $2
        sal_Int32 nMatchExprPart2Start( nLHSIndex + sLHSPattern.getLength() );
        ::rtl::OUString sMatchExprPart2 = sMatchExpression.copy(
            nMatchExprPart2Start,
            sMatchExpression.getLength() - nMatchExprPart2Start - sMatchExprPart3.getLength() - 2
        );
        // strip the expression by its left-most and right-most part
        const ::rtl::OUString sExpression( _rExpression.copy(
            sExprPart1.getLength(),
            _rExpression.getLength() - sExprPart1.getLength() - sExprPart3.getLength()
        ) );

        sal_Int32 nPart2Index = sExpression.indexOf( sMatchExprPart2 );
        if ( nPart2Index == -1 )
            // the "middle" part of the match expression does not exist in the expression at all
            return false;

        OSL_ENSURE( sExpression.indexOf( sMatchExprPart2, nPart2Index + 1 ) == -1,
            "ConditionalExpression::matchExpression: ambiguous matching!" );
            // if this fires, then we're lost: The middle part exists two times in the expression,
            // so we cannot reliably determine what's the LHS and what's the RHS.
            // Well, the whole mechanism is flawed, anyway:
            // Encoding the field content in the conditional expression will break as soon
            // as somebody
            // - assigns a Data Field to a control
            // - creates a conditional format expression for the control
            // - assigns another Data Field to the control
            // - opens the Conditional Format Dialog, again
            // Here, at the latest, you can see that we need another mechanism, anyway, which does not
            // rely on those strange expression building/matching

        _out_rLHS = sExpression.copy( 0, nPart2Index );
        _out_rRHS = sExpression.copy( nPart2Index + sMatchExprPart2.getLength() );

        return true;
    }

    // =============================================================================
    // = ConditionalExpressionFactory
    // =============================================================================
    // -----------------------------------------------------------------------------
    size_t ConditionalExpressionFactory::getKnownConditionalExpressions( ConditionalExpressions& _out_rCondExp )
    {
        ConditionalExpressions aEmpty;
        _out_rCondExp.swap( aEmpty );

        _out_rCondExp[ eBetween ]        = PConditionalExpression( new ConditionalExpression( "AND( ( $$ ) >= ( $1 ); ( $$ ) <= ( $2 ) )" ) );
        _out_rCondExp[ eNotBetween ]     = PConditionalExpression( new ConditionalExpression( "NOT( AND( ( $$ ) >= ( $1 ); ( $$ ) <= ( $2 ) ) )" ) );
        _out_rCondExp[ eEqualTo ]        = PConditionalExpression( new ConditionalExpression( "( $$ ) = ( $1 )" ) );
        _out_rCondExp[ eNotEqualTo ]     = PConditionalExpression( new ConditionalExpression( "( $$ ) <> ( $1 )" ) );
        _out_rCondExp[ eGreaterThan ]    = PConditionalExpression( new ConditionalExpression( "( $$ ) > ( $1 )" ) );
        _out_rCondExp[ eLessThan ]       = PConditionalExpression( new ConditionalExpression( "( $$ ) < ( $1 )" ) );
        _out_rCondExp[ eGreaterOrEqual ] = PConditionalExpression( new ConditionalExpression( "( $$ ) >= ( $1 )" ) );
        _out_rCondExp[ eLessOrEqual ]    = PConditionalExpression( new ConditionalExpression( "( $$ ) <= ( $1 )" ) );

        return _out_rCondExp.size();
    }
//........................................................................
} // namespace rptui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
