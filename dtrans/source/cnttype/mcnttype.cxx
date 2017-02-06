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

#include <sal/config.h>

#include <com/sun/star/container/NoSuchElementException.hpp>

#include "mcnttype.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace std;
using namespace osl;

const char TSPECIALS[] =  "()<>@,;:\\\"/[]?=";
const char TOKEN[] = "!#$%&'*+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^_`abcdefghijklmnopqrstuvwxyz{|}~.";
const char SPACE[] = " ";
const char SEMICOLON[] = ";";

CMimeContentType::CMimeContentType( const OUString& aCntType )
{
    init( aCntType );
}

OUString SAL_CALL CMimeContentType::getMediaType( )
{
    return m_MediaType;
}

OUString SAL_CALL CMimeContentType::getMediaSubtype( )
{
    return m_MediaSubtype;
}

OUString SAL_CALL CMimeContentType::getFullMediaType( )
{
    return m_MediaType + "/" + m_MediaSubtype;
}

Sequence< OUString > SAL_CALL CMimeContentType::getParameters( )
{
    MutexGuard aGuard( m_aMutex );

    Sequence< OUString > seqParams;

    map< OUString, OUString >::iterator iter;
    map< OUString, OUString >::iterator iter_end = m_ParameterMap.end( );

    for ( iter = m_ParameterMap.begin( ); iter != iter_end; ++iter )
    {
        seqParams.realloc( seqParams.getLength( ) + 1 );
        seqParams[seqParams.getLength( ) - 1] = iter->first;
    }

    return seqParams;
}

sal_Bool SAL_CALL CMimeContentType::hasParameter( const OUString& aName )
{
    MutexGuard aGuard( m_aMutex );
    return ( m_ParameterMap.end( ) != m_ParameterMap.find( aName ) );
}

OUString SAL_CALL CMimeContentType::getParameterValue( const OUString& aName )
{
    MutexGuard aGuard( m_aMutex );

    if ( !hasParameter( aName ) )
        throw NoSuchElementException( );

    return m_ParameterMap.find( aName )->second;
}

void SAL_CALL CMimeContentType::init( const OUString& aCntType )
{
    if ( aCntType.isEmpty( ) )
        throw IllegalArgumentException( );

    m_nPos = 0;
    m_ContentType = aCntType;
    getSym( );
    type();
}

void SAL_CALL CMimeContentType::getSym()
{
    if ( m_nPos < m_ContentType.getLength( ) )
    {
        m_nxtSym = m_ContentType.copy(m_nPos, 1);
        ++m_nPos;
        return;
    }

    m_nxtSym = OUString( );
}

void SAL_CALL CMimeContentType::acceptSym( const OUString& pSymTlb )
{
    if ( pSymTlb.indexOf( m_nxtSym ) < 0 )
        throw IllegalArgumentException( );

    getSym();
}

void SAL_CALL CMimeContentType::skipSpaces()
{
    while (m_nxtSym == SPACE)
        getSym( );
}

void SAL_CALL CMimeContentType::type()
{
    skipSpaces( );

    OUString sToken(TOKEN);

    // check FIRST( type )
    if ( !isInRange( m_nxtSym, sToken ) )
        throw IllegalArgumentException( );

    // parse
    while(  !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            m_MediaType += m_nxtSym;
        else if ( isInRange( m_nxtSym, "/ " ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    // check FOLLOW( type )
    skipSpaces( );
    acceptSym( "/" );

    subtype( );
}

void SAL_CALL CMimeContentType::subtype()
{
    skipSpaces( );

    OUString sToken(TOKEN);

    // check FIRST( subtype )
    if ( !isInRange( m_nxtSym, sToken ) )
        throw IllegalArgumentException( );

    while( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            m_MediaSubtype += m_nxtSym;
        else if ( isInRange( m_nxtSym, "; " ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    // parse the rest
    skipSpaces( );
    trailer();
}

void SAL_CALL CMimeContentType::trailer()
{
    OUString sToken(TOKEN);
    while( !m_nxtSym.isEmpty( ) )
    {
        if ( m_nxtSym == "(" )
        {
            getSym( );
            comment( );
            acceptSym( ")" );
        }
        else if ( m_nxtSym == ";" )
        {
            // get the parameter name
            getSym( );
            skipSpaces( );

            if ( !isInRange( m_nxtSym, sToken ) )
                throw IllegalArgumentException( );

            OUString pname = pName( );

            skipSpaces();
            acceptSym( "=" );

            // get the parameter value
            skipSpaces( );

            OUString pvalue = pValue( );

            // insert into map
            if ( !m_ParameterMap.insert( pair < const OUString, OUString > ( pname, pvalue ) ).second )
                throw IllegalArgumentException( );
        }
        else
            throw IllegalArgumentException( );

        skipSpaces( );
    }
}

OUString SAL_CALL CMimeContentType::pName( )
{
    OUString pname;

    OUString sToken(TOKEN);
    while( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            pname += m_nxtSym;
        else if ( isInRange( m_nxtSym, "= " ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    return pname;
}

OUString SAL_CALL CMimeContentType::pValue( )
{
    OUString pvalue;

    OUString sToken(TOKEN);
    // quoted pvalue
    if ( m_nxtSym == "\"" )
    {
        getSym( );
        pvalue = quotedPValue( );

        if ( pvalue[pvalue.getLength() - 1] != '"' )
            throw IllegalArgumentException( );

        // remove the last quote-sign
        pvalue = pvalue.copy(0, pvalue.getLength() - 1);

        if ( pvalue.isEmpty( ) )
            throw IllegalArgumentException( );
    }
    else if ( isInRange( m_nxtSym, sToken ) ) // unquoted pvalue
    {
        pvalue = nonquotedPValue( );
    }
    else
        throw IllegalArgumentException( );

    return pvalue;
}

// the following combinations within a quoted value are not allowed:
// '";' (quote sign followed by semicolon) and '" ' (quote sign followed
// by space)

OUString SAL_CALL CMimeContentType::quotedPValue( )
{
    OUString pvalue;
    bool bAfterQuoteSign = false;

    while ( !m_nxtSym.isEmpty( ) )
    {
        if ( bAfterQuoteSign && (
            (m_nxtSym == SPACE) ||
            (m_nxtSym == SEMICOLON))
           )
        {
            break;
        }
        else if ( isInRange( m_nxtSym, OUStringLiteral(TOKEN) + TSPECIALS + SPACE ) )
        {
            pvalue += m_nxtSym;
            if ( m_nxtSym == "\"" )
                bAfterQuoteSign = true;
            else
                bAfterQuoteSign = false;
        }
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    return pvalue;
}

OUString SAL_CALL CMimeContentType::nonquotedPValue( )
{
    OUString pvalue;

    OUString sToken(TOKEN);
    while ( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            pvalue += m_nxtSym;
        else if ( isInRange( m_nxtSym, "; " ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    return pvalue;
}

void SAL_CALL CMimeContentType::comment()
{
    while ( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, OUStringLiteral(TOKEN) + SPACE ) )
            getSym( );
        else if ( m_nxtSym == ")" )
            break;
        else
            throw IllegalArgumentException( );
    }
}

bool SAL_CALL CMimeContentType::isInRange( const OUString& aChr, const OUString& aRange )
{
    return ( aRange.indexOf( aChr ) > -1 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
