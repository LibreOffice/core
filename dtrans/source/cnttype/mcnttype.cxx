/*************************************************************************
 *
 *  $RCSfile: mcnttype.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tra $ $Date: 2001-02-26 06:59:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _MCNTTYPE_HXX_
#include "mcnttype.hxx"
#endif

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
        OUString qpvalue( pvalue, pvalue.getLength( ) - 1 );
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