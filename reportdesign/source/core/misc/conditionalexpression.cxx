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

#include <conditionalexpression.hxx>

#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>


namespace rptui
{

    // = ConditionalExpression


    ConditionalExpression::ConditionalExpression(const OUString& sPattern)
        : m_sPattern(sPattern)
    {
    }


    OUString ConditionalExpression::assembleExpression( const OUString& _rFieldDataSource, const OUString& _rLHS, const OUString& _rRHS ) const
    {
        OUString sExpression( m_sPattern );

        sal_Int32 nPatternIndex = sExpression.indexOf( '$' );
        while ( nPatternIndex > -1 )
        {
            const OUString* pReplace = nullptr;
            switch ( sExpression[ nPatternIndex + 1 ] )
            {
            case '$': pReplace = &_rFieldDataSource; break;
            case '1': pReplace = &_rLHS; break;
            case '2': pReplace = &_rRHS; break;
            default: break;
            }

            if ( pReplace == nullptr )
            {
                OSL_FAIL( "ConditionalExpression::assembleExpression: illegal pattern!" );
                break;
            }

            sExpression = sExpression.replaceAt( nPatternIndex, 2, *pReplace );
            nPatternIndex = sExpression.indexOf( '$', nPatternIndex + pReplace->getLength() + 1 );
        }
        return sExpression;
    }


    bool ConditionalExpression::matchExpression( std::u16string_view _rExpression, const std::u16string_view _rFieldDataSource, OUString& _out_rLHS, OUString& _out_rRHS ) const
    {
        // if we had regular expression, the matching would be pretty easy ...
        // just replace $1 and $2 in the pattern with (.*), and then get them with \1 resp. \2.
        // Unfortunately, we don't have such a regexp engine ...

        // Okay, let's start with replacing all $$ in our pattern with the actual field data source
        OUString sMatchExpression( m_sPattern );
        sMatchExpression = sMatchExpression.replaceAll(u"$$", _rFieldDataSource);

        static constexpr OUString sLHSPattern( u"$1"_ustr );
        static constexpr OUString sRHSPattern( u"$2"_ustr );
        sal_Int32 nLHSIndex( sMatchExpression.indexOf( sLHSPattern ) );
        sal_Int32 nRHSIndex( sMatchExpression.indexOf( sRHSPattern ) );

        // now we should have at most one occurrence of $1 and $2, resp.
        OSL_ENSURE( sMatchExpression.indexOf( sLHSPattern, nLHSIndex + 1 ) == -1,
            "ConditionalExpression::matchExpression: unsupported pattern (more than one LHS occurrence)!" );
        OSL_ENSURE( sMatchExpression.indexOf( sRHSPattern, nRHSIndex + 1 ) == -1,
            "ConditionalExpression::matchExpression: unsupported pattern (more than one RHS occurrence)!" );
        // Also, an LHS must be present, and precede the RHS (if present)
        OSL_ENSURE( ( nLHSIndex != -1 ) && ( ( nLHSIndex < nRHSIndex ) || ( nRHSIndex == -1 ) ),
            "ConditionalExpression::matchExpression: no LHS, or an RHS preceding the LHS - this is not supported!" );

        // up to the occurrence of the LHS (which must exist, see above), the two expressions
        // must be identical
        if ( sal_Int32(_rExpression.size()) < nLHSIndex )
            return false;
        const std::u16string_view sExprPart1( _rExpression.substr( 0, nLHSIndex ) );
        const std::u16string_view sMatchExprPart1( sMatchExpression.subView( 0, nLHSIndex ) );
        if ( sExprPart1 != sMatchExprPart1 )
            // the left-most expression parts do not match
            return false;

        // after the occurrence of the RHS (or the LHS, if there is no RHS), the two expressions
        // must be identical, too
        bool bHaveRHS( nRHSIndex != -1 );
        sal_Int32 nRightMostIndex( bHaveRHS ? nRHSIndex : nLHSIndex );
        const std::u16string_view sMatchExprPart3( sMatchExpression.subView( nRightMostIndex + 2 ) );
        if ( _rExpression.size() < sMatchExprPart3.size() )
            // the expression is not even long enough to hold the right-most part of the match expression
            return false;
        const std::u16string_view sExprPart3( _rExpression.substr( _rExpression.size() - sMatchExprPart3.size() ) );
        if ( sExprPart3 != sMatchExprPart3 )
            // the right-most expression parts do not match
            return false;

        // if we don't have an RHS, we're done
        if ( !bHaveRHS )
        {
            _out_rLHS = _rExpression.substr( sExprPart1.size(), _rExpression.size() - sExprPart1.size() - sExprPart3.size() );
            return true;
        }

        // strip the match expression by its right-most and left-most part, and by the placeholders $1 and $2
        sal_Int32 nMatchExprPart2Start( nLHSIndex + sLHSPattern.getLength() );
        std::u16string_view sMatchExprPart2 = sMatchExpression.subView(
            nMatchExprPart2Start,
            sMatchExpression.getLength() - nMatchExprPart2Start - sMatchExprPart3.size() - 2
        );
        // strip the expression by its left-most and right-most part
        const std::u16string_view sExpression( _rExpression.substr(
            sExprPart1.size(),
            _rExpression.size() - sExprPart1.size() - sExprPart3.size()
        ) );

        size_t nPart2Index = sExpression.find( sMatchExprPart2 );
        if ( nPart2Index == std::u16string_view::npos )
            // the "middle" part of the match expression does not exist in the expression at all
            return false;

        OSL_ENSURE( sExpression.find( sMatchExprPart2, nPart2Index + 1 ) == std::u16string_view::npos,
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

        _out_rLHS = sExpression.substr( 0, nPart2Index );
        _out_rRHS = sExpression.substr( nPart2Index + sMatchExprPart2.size() );

        return true;
    }


    // = ConditionalExpressionFactory


    size_t ConditionalExpressionFactory::getKnownConditionalExpressions( ConditionalExpressions& _out_rCondExp )
    {
        ConditionalExpressions().swap(_out_rCondExp);

        _out_rCondExp[ eBetween ]        = std::make_shared<ConditionalExpression>(u"AND( ( $$ ) >= ( $1 ); ( $$ ) <= ( $2 ) )"_ustr);
        _out_rCondExp[ eNotBetween ]     = std::make_shared<ConditionalExpression>(u"NOT( AND( ( $$ ) >= ( $1 ); ( $$ ) <= ( $2 ) ) )"_ustr);
        _out_rCondExp[ eEqualTo ]        = std::make_shared<ConditionalExpression>(u"( $$ ) = ( $1 )"_ustr);
        _out_rCondExp[ eNotEqualTo ]     = std::make_shared<ConditionalExpression>(u"( $$ ) <> ( $1 )"_ustr);
        _out_rCondExp[ eGreaterThan ]    = std::make_shared<ConditionalExpression>(u"( $$ ) > ( $1 )"_ustr);
        _out_rCondExp[ eLessThan ]       = std::make_shared<ConditionalExpression>(u"( $$ ) < ( $1 )"_ustr);
        _out_rCondExp[ eGreaterOrEqual ] = std::make_shared<ConditionalExpression>(u"( $$ ) >= ( $1 )"_ustr);
        _out_rCondExp[ eLessOrEqual ]    = std::make_shared<ConditionalExpression>(u"( $$ ) <= ( $1 )"_ustr);

        return _out_rCondExp.size();
    }

} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
