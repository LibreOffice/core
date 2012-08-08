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

#include "mcnttype.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace std;
using namespace osl;

using ::rtl::OUString;

//------------------------------------------------------------------------
// constants
//------------------------------------------------------------------------

const char TSPECIALS[] =  "()<>@,;:\\\"/[]?=";
const char TOKEN[] = "!#$%&'*+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^_`abcdefghijklmnopqrstuvwxyz{|}~.";
const char SPACE[] = " ";
const char SEMICOLON[] = ";";

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CMimeContentType::CMimeContentType( const OUString& aCntType )
{
    init( aCntType );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::getMediaType( ) throw(RuntimeException)
{
    return m_MediaType;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::getMediaSubtype( ) throw(RuntimeException)
{
    return m_MediaSubtype;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::getFullMediaType( ) throw(RuntimeException)
{
    return m_MediaType + OUString("/") + m_MediaSubtype;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

Sequence< OUString > SAL_CALL CMimeContentType::getParameters( ) throw(RuntimeException)
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

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CMimeContentType::hasParameter( const OUString& aName ) throw(RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return ( m_ParameterMap.end( ) != m_ParameterMap.find( aName ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::getParameterValue( const OUString& aName ) throw(NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if ( !hasParameter( aName ) )
        throw NoSuchElementException( );

    return m_ParameterMap.find( aName )->second;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::init( const OUString& aCntType ) throw( IllegalArgumentException )
{
    if ( aCntType.isEmpty( ) )
        throw IllegalArgumentException( );

    m_nPos = 0;
    m_ContentType = aCntType;
    getSym( );
    type();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::getSym( void )
{
    if ( m_nPos < m_ContentType.getLength( ) )
    {
        m_nxtSym = m_ContentType.copy(m_nPos, 1);
        ++m_nPos;
        return;
    }

    m_nxtSym = OUString( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::acceptSym( const OUString& pSymTlb )
{
    if ( pSymTlb.indexOf( m_nxtSym ) < 0 )
        throw IllegalArgumentException( );

    getSym();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::skipSpaces( void )
{
    while (m_nxtSym.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(SPACE)))
        getSym( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::type( void )
{
    skipSpaces( );

    rtl::OUString sToken(TOKEN);

    // check FIRST( type )
    if ( !isInRange( m_nxtSym, sToken ) )
        throw IllegalArgumentException( );

    // parse
    while(  !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            m_MediaType += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString("/ ") ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    // check FOLLOW( type )
    skipSpaces( );
    acceptSym( OUString("/") );

    subtype( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::subtype( void )
{
    skipSpaces( );

    rtl::OUString sToken(TOKEN);

    // check FIRST( subtype )
    if ( !isInRange( m_nxtSym, sToken ) )
        throw IllegalArgumentException( );

    while( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            m_MediaSubtype += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString("; ") ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    // parse the rest
    skipSpaces( );
    trailer();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::trailer( void )
{
    rtl::OUString sToken(TOKEN);
    while( !m_nxtSym.isEmpty( ) )
    {
        if ( m_nxtSym == OUString("(") )
        {
            getSym( );
            comment( );
            acceptSym( OUString(")") );
        }
        else if ( m_nxtSym == OUString(";") )
        {
            // get the parameter name
            getSym( );
            skipSpaces( );

            if ( !isInRange( m_nxtSym, sToken ) )
                throw IllegalArgumentException( );

            OUString pname = pName( );

            skipSpaces();
            acceptSym( OUString("=") );

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

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::pName( )
{
    OUString pname;

    rtl::OUString sToken(TOKEN);
    while( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            pname += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString("= ") ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    return pname;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::pValue( )
{
    OUString pvalue;

    rtl::OUString sToken(TOKEN);
    // quoted pvalue
    if ( m_nxtSym == OUString( "\"" ) )
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

//------------------------------------------------------------------------
// the following combinations within a quoted value are not allowed:
// '";' (quote sign followed by semicolon) and '" ' (quote sign followed
// by space)
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::quotedPValue( )
{
    OUString pvalue;
    sal_Bool bAfterQuoteSign = sal_False;

    while ( !m_nxtSym.isEmpty( ) )
    {
        if ( bAfterQuoteSign && (
            (m_nxtSym.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(SPACE))) ||
            (m_nxtSym.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(SEMICOLON))))
           )
        {
            break;
        }
        else if ( isInRange( m_nxtSym, rtl::OUString(TOKEN) + rtl::OUString(TSPECIALS) + rtl::OUString(SPACE) ) )
        {
            pvalue += m_nxtSym;
            if ( m_nxtSym == OUString( "\"" ) )
                bAfterQuoteSign = sal_True;
            else
                bAfterQuoteSign = sal_False;
        }
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    return pvalue;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CMimeContentType::nonquotedPValue( )
{
    OUString pvalue;

    rtl::OUString sToken(TOKEN);
    while ( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, sToken ) )
            pvalue += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString("; ") ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    return pvalue;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::comment( void )
{
    while ( !m_nxtSym.isEmpty( ) )
    {
        if ( isInRange( m_nxtSym, rtl::OUString(TOKEN) + rtl::OUString(SPACE) ) )
            getSym( );
        else if ( m_nxtSym == OUString(")") )
            break;
        else
            throw IllegalArgumentException( );
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CMimeContentType::isInRange( const rtl::OUString& aChr, const rtl::OUString& aRange )
{
    return ( aRange.indexOf( aChr ) > -1 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
