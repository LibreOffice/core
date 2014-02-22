/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include "conditionalexpression.hxx"


namespace rptui
{
    
    
    
    
    ConditionalExpression::ConditionalExpression( const sal_Char* _pAsciiPattern )
        :m_sPattern( OUString::createFromAscii( _pAsciiPattern ) )
    {
    }

    
    OUString ConditionalExpression::assembleExpression( const OUString& _rFieldDataSource, const OUString& _rLHS, const OUString& _rRHS ) const
    {
        OUString sExpression( m_sPattern );

        sal_Int32 nPatternIndex = sExpression.indexOf( '$' );
        while ( nPatternIndex > -1 )
        {
            const OUString* pReplace = NULL;
            switch ( sExpression[ nPatternIndex + 1 ] )
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

    
    bool ConditionalExpression::matchExpression( const OUString& _rExpression, const OUString& _rFieldDataSource, OUString& _out_rLHS, OUString& _out_rRHS ) const
    {
        (void)_rExpression;
        (void)_rFieldDataSource;
        (void)_out_rLHS;
        (void)_out_rRHS;

        
        
        

        
        OUString sMatchExpression( m_sPattern );
        const OUString sFieldDataPattern( "$$" );
        sal_Int32 nIndex( sMatchExpression.indexOf( sFieldDataPattern ) );
        while ( nIndex != -1 )
        {
            sMatchExpression = sMatchExpression.replaceAt( nIndex, sFieldDataPattern.getLength(), _rFieldDataSource );
            nIndex = sMatchExpression.indexOf( sFieldDataPattern, nIndex + _rFieldDataSource.getLength() );
        }

        const OUString sLHSPattern( "$1" );
        const OUString sRHSPattern( "$2" );
        sal_Int32 nLHSIndex( sMatchExpression.indexOf( sLHSPattern ) );
        sal_Int32 nRHSIndex( sMatchExpression.indexOf( sRHSPattern ) );

        
        OSL_ENSURE( sMatchExpression.indexOf( sLHSPattern, nLHSIndex + 1 ) == -1,
            "ConditionalExpression::matchExpression: unsupported pattern (more than one LHS occurrence)!" );
        OSL_ENSURE( sMatchExpression.indexOf( sRHSPattern, nRHSIndex + 1 ) == -1,
            "ConditionalExpression::matchExpression: unsupported pattern (more than one RHS occurrence)!" );
        
        OSL_ENSURE( ( nLHSIndex != -1 ) && ( ( nLHSIndex < nRHSIndex ) || ( nRHSIndex == -1 ) ),
            "ConditionalExpression::matchExpression: no LHS, or an RHS preceeding the LHS - this is not supported!" );

        
        
        if ( _rExpression.getLength() < nLHSIndex )
            return false;
        const OUString sExprPart1( _rExpression.copy( 0, nLHSIndex ) );
        const OUString sMatchExprPart1( sMatchExpression.copy( 0, nLHSIndex ) );
        if ( sExprPart1 != sMatchExprPart1 )
            
            return false;

        
        
        bool bHaveRHS( nRHSIndex != -1 );
        sal_Int32 nRightMostIndex( bHaveRHS ? nRHSIndex : nLHSIndex );
        const OUString sMatchExprPart3( sMatchExpression.copy( nRightMostIndex + 2 ) );
        if ( _rExpression.getLength() < sMatchExprPart3.getLength() )
            
            return false;
        const OUString sExprPart3( _rExpression.copy( _rExpression.getLength() - sMatchExprPart3.getLength() ) );
        if ( sExprPart3 != sMatchExprPart3 )
            
            return false;

        
        if ( !bHaveRHS )
        {
            _out_rLHS = _rExpression.copy( sExprPart1.getLength(), _rExpression.getLength() - sExprPart1.getLength() - sExprPart3.getLength() );
            return true;
        }

        
        sal_Int32 nMatchExprPart2Start( nLHSIndex + sLHSPattern.getLength() );
        OUString sMatchExprPart2 = sMatchExpression.copy(
            nMatchExprPart2Start,
            sMatchExpression.getLength() - nMatchExprPart2Start - sMatchExprPart3.getLength() - 2
        );
        
        const OUString sExpression( _rExpression.copy(
            sExprPart1.getLength(),
            _rExpression.getLength() - sExprPart1.getLength() - sExprPart3.getLength()
        ) );

        sal_Int32 nPart2Index = sExpression.indexOf( sMatchExprPart2 );
        if ( nPart2Index == -1 )
            
            return false;

        OSL_ENSURE( sExpression.indexOf( sMatchExprPart2, nPart2Index + 1 ) == -1,
            "ConditionalExpression::matchExpression: ambiguous matching!" );
            
            
            
            
            
            
            
            
            
            
            

        _out_rLHS = sExpression.copy( 0, nPart2Index );
        _out_rRHS = sExpression.copy( nPart2Index + sMatchExprPart2.getLength() );

        return true;
    }

    
    
    
    
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

} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
