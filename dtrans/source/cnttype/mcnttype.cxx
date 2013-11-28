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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include "mcnttype.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace rtl;
using namespace std;
using namespace osl;

//------------------------------------------------------------------------
// constants
//------------------------------------------------------------------------

const OUString TSPECIALS = OUString::createFromAscii( "()<>@,;:\\\"/[]?=" );
const OUString TOKEN     = OUString::createFromAscii( "!#$%&'*+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^_`abcdefghijklmnopqrstuvwxyz{|}~." );
const OUString SPACE     = OUString::createFromAscii( " " );
const OUString SEMICOLON = OUString::createFromAscii( ";" );

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
    return m_MediaType + OUString::createFromAscii( "/" ) + m_MediaSubtype;
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
    if ( !aCntType.getLength( ) )
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
        m_nxtSym = OUString( &m_ContentType[m_nPos], 1 );
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
    while ( SPACE == m_nxtSym )
        getSym( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::type( void )
{
    skipSpaces( );

    // check FIRST( type )
    if ( !isInRange( m_nxtSym, TOKEN ) )
        throw IllegalArgumentException( );

    // parse
    while(  m_nxtSym.getLength( ) )
    {
        if ( isInRange( m_nxtSym, TOKEN ) )
            m_MediaType += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString::createFromAscii( "/ " ) ) )
            break;
        else
            throw IllegalArgumentException( );
        getSym( );
    }

    // check FOLLOW( type )
    skipSpaces( );
    acceptSym( OUString::createFromAscii( "/" ) );

    subtype( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CMimeContentType::subtype( void )
{
    skipSpaces( );

    // check FIRST( subtype )
    if ( !isInRange( m_nxtSym, TOKEN ) )
        throw IllegalArgumentException( );

    while( m_nxtSym.getLength( ) )
    {
        if ( isInRange( m_nxtSym, TOKEN ) )
            m_MediaSubtype += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString::createFromAscii( "; " ) ) )
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
    while( m_nxtSym.getLength( ) )
    {
        if ( m_nxtSym == OUString::createFromAscii( "(" ) )
        {
            getSym( );
            comment( );
            acceptSym( OUString::createFromAscii( ")" ) );
        }
        else if ( m_nxtSym == OUString::createFromAscii( ";" ) )
        {
            // get the parameter name
            getSym( );
            skipSpaces( );

            if ( !isInRange( m_nxtSym, TOKEN ) )
                throw IllegalArgumentException( );

            OUString pname = pName( );

            skipSpaces();
            acceptSym( OUString::createFromAscii( "=" ) );

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

    while( m_nxtSym.getLength( ) )
    {
        if ( isInRange( m_nxtSym, TOKEN ) )
            pname += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString::createFromAscii( "= " ) ) )
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

    // quoted pvalue
    if ( m_nxtSym == OUString::createFromAscii( "\"" ) )
    {
        getSym( );
        pvalue = quotedPValue( );

        if (  OUString( &pvalue[pvalue.getLength() - 1], 1 ) != OUString::createFromAscii( "\"" ) )
            throw IllegalArgumentException( );

        // remove the last quote-sign
        const OUString qpvalue( pvalue.getStr(), pvalue.getLength( ) - 1 );
        pvalue = qpvalue;

        if ( !pvalue.getLength( ) )
            throw IllegalArgumentException( );
    }
    else if ( isInRange( m_nxtSym, TOKEN ) ) // unquoted pvalue
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

    while ( m_nxtSym.getLength( ) )
    {
        if ( bAfterQuoteSign && ((m_nxtSym == SPACE)||(m_nxtSym == SEMICOLON) ) )
            break;
        else if ( isInRange( m_nxtSym, TOKEN + TSPECIALS + SPACE ) )
        {
            pvalue += m_nxtSym;
            if ( m_nxtSym == OUString::createFromAscii( "\"" ) )
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

    while ( m_nxtSym.getLength( ) )
    {
        if ( isInRange( m_nxtSym, TOKEN ) )
            pvalue += m_nxtSym;
        else if ( isInRange( m_nxtSym, OUString::createFromAscii( "; " ) ) )
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
    while ( m_nxtSym.getLength( ) )
    {
        if ( isInRange( m_nxtSym, TOKEN + SPACE ) )
            getSym( );
        else if ( m_nxtSym == OUString::createFromAscii( ")" ) )
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
