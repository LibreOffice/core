/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sharecontrolfile.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:09:24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <stdio.h>

#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>

#include <osl/time.h>
#include <osl/security.hxx>
#include <osl/socket.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/processfactory.hxx>

#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>

#include <svtools/useroptions.hxx>

#include <svtools/sharecontrolfile.hxx>

using namespace ::com::sun::star;

namespace svt {

// ----------------------------------------------------------------------
ShareControlFile::ShareControlFile( const ::rtl::OUString& aOrigURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_xFactory( xFactory )
{
    if ( !m_xFactory.is() )
        m_xFactory = ::comphelper::getProcessServiceFactory();

    if ( !aOrigURL.getLength() )
        throw lang::IllegalArgumentException();

    INetURLObject aDocURL( aOrigURL );
    ::rtl::OUString aShareURLString = aDocURL.GetPartBeforeLastName();
    aShareURLString += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".~sharing." ) );
    aShareURLString += aDocURL.GetName();
    m_aURL = INetURLObject( aShareURLString ).GetMainURL( INetURLObject::NO_DECODE );

    OpenStream();

    if ( !IsValid() )
        throw io::NotConnectedException();
}

// ----------------------------------------------------------------------
ShareControlFile::~ShareControlFile()
{
    try
    {
        Close();
    }
    catch( uno::Exception& )
    {}
}

// ----------------------------------------------------------------------
void ShareControlFile::OpenStream()
{
    // if it is called outside of constructor the mutex must be locked

    if ( !m_xStream.is() && m_aURL.getLength() )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
            uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess(
                xFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess") ),
                uno::UNO_QUERY_THROW );

        uno::Reference< io::XStream > xStream = xSimpleFileAccess->openFileReadWrite( m_aURL );
        m_xSeekable.set( xStream, uno::UNO_QUERY_THROW );
        m_xInputStream.set( xStream->getInputStream(), uno::UNO_QUERY_THROW );
        m_xOutputStream.set( xStream->getOutputStream(), uno::UNO_QUERY_THROW );
        m_xTruncate.set( m_xOutputStream, uno::UNO_QUERY_THROW );
        m_xStream = xStream;
    }
}

// ----------------------------------------------------------------------
void ShareControlFile::Close()
{
    // if it is called outside of destructor the mutex must be locked

    if ( m_xStream.is() )
    {
        try
        {
            if ( m_xInputStream.is() )
                m_xInputStream->closeInput();
            if ( m_xOutputStream.is() )
                m_xOutputStream->closeOutput();
        }
        catch( uno::Exception& )
        {}

        m_xStream = uno::Reference< io::XStream >();
        m_xInputStream = uno::Reference< io::XInputStream >();
        m_xOutputStream = uno::Reference< io::XOutputStream >();
        m_xSeekable = uno::Reference< io::XSeekable >();
        m_xTruncate = uno::Reference< io::XTruncate >();
        m_aUsersData.realloc( 0 );
    }
}

// ----------------------------------------------------------------------
uno::Sequence< uno::Sequence< ::rtl::OUString > > ShareControlFile::ParseList( const uno::Sequence< sal_Int8 >& aBuffer )
{
    sal_Int32 nCurPos = 0;
    sal_Int32 nCurEntry = 0;
    uno::Sequence< uno::Sequence< ::rtl::OUString > > aResult( 10 );

    while ( nCurPos < aBuffer.getLength() )
    {
        if ( nCurEntry >= aResult.getLength() )
            aResult.realloc( nCurEntry + 10 );
        aResult[nCurEntry] = ParseEntry( aBuffer, nCurPos );
        nCurEntry++;
    }

    aResult.realloc( nCurEntry );
    return aResult;
}

// ----------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ShareControlFile::ParseEntry( const uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos )
{
    uno::Sequence< ::rtl::OUString > aResult( SHARED_ENTRYSIZE );

    for ( int nInd = 0; nInd < SHARED_ENTRYSIZE; nInd++ )
    {
        aResult[nInd] = ParseName( aBuffer, o_nCurPos );
        if ( o_nCurPos >= aBuffer.getLength()
          || ( nInd < SHARED_ENTRYSIZE - 1 && aBuffer[o_nCurPos++] != ',' )
          || ( nInd == SHARED_ENTRYSIZE - 1 && aBuffer[o_nCurPos++] != ';' ) )
            throw io::WrongFormatException();
    }

    return aResult;
}

// ----------------------------------------------------------------------
::rtl::OUString ShareControlFile::ParseName( const uno::Sequence< sal_Int8 >& aBuffer, sal_Int32& o_nCurPos )
{
    ::rtl::OStringBuffer aResult;
    sal_Bool bHaveName = sal_False;
    sal_Bool bEscape = sal_False;

    while( !bHaveName )
    {
        if ( o_nCurPos >= aBuffer.getLength() )
            throw io::WrongFormatException();

        if ( aBuffer[o_nCurPos] == ',' || aBuffer[o_nCurPos] == ';' )
            bHaveName = sal_True;
        else
        {
            if ( bEscape )
            {
                if ( aBuffer[o_nCurPos] == ',' || aBuffer[o_nCurPos] == ';' || aBuffer[o_nCurPos] == '\\' )
                    aResult.append( (sal_Char)aBuffer[o_nCurPos] );
                else
                    throw io::WrongFormatException();

                bEscape = sal_False;
            }
            else
            {
                if ( aBuffer[o_nCurPos] == '\\' )
                    bEscape = sal_True;
                else
                    aResult.append( (sal_Char)aBuffer[o_nCurPos] );
            }

            o_nCurPos++;
        }
    }

    return ::rtl::OStringToOUString( aResult.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
}

// ----------------------------------------------------------------------
::rtl::OUString ShareControlFile::EscapeCharacters( const ::rtl::OUString& aSource )
{
    ::rtl::OUStringBuffer aBuffer;
    const sal_Unicode* pStr = aSource.getStr();
    for ( sal_Int32 nInd = 0; nInd < aSource.getLength() && pStr[nInd] != 0; nInd++ )
    {
        if ( pStr[nInd] == '\\' || pStr[nInd] == ';' || pStr[nInd] == ',' )
            aBuffer.append( (sal_Unicode)'\\' );
        aBuffer.append( pStr[nInd] );
    }

    return aBuffer.makeStringAndClear();
}

// ----------------------------------------------------------------------
::rtl::OUString ShareControlFile::GetOOOUserName()
{
    SvtUserOptions aUserOpt;
    ::rtl::OUString aName = aUserOpt.GetFirstName();
    aName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) );
    aName += aUserOpt.GetLastName();

    return aName;
}

// ----------------------------------------------------------------------
::rtl::OUString ShareControlFile::GetCurrentLocalTime()
{
    ::rtl::OUString aTime;

    TimeValue aSysTime;
    if ( osl_getSystemTime( &aSysTime ) )
    {
        TimeValue aLocTime;
        if ( osl_getLocalTimeFromSystemTime( &aSysTime, &aLocTime ) )
        {
            oslDateTime aDateTime;
            if ( osl_getDateTimeFromTimeValue( &aLocTime, &aDateTime ) )
            {
                char pDateTime[20];
                sprintf( pDateTime, "%02d.%02d.%4d %02d:%02d", aDateTime.Day, aDateTime.Month, aDateTime.Year, aDateTime.Hours, aDateTime.Minutes );
                aTime = ::rtl::OUString::createFromAscii( pDateTime );
            }
        }
    }

    return aTime;
}

// ----------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ShareControlFile::GenerateOwnEntry()
{
    uno::Sequence< ::rtl::OUString > aResult( SHARED_ENTRYSIZE );

    aResult[SHARED_OOOUSERNAME_ID] = GetOOOUserName();

    ::osl::Security aSecurity;
    aSecurity.getUserName( aResult[SHARED_SYSUSERNAME_ID] );

    aResult[SHARED_LOCALHOST_ID] = ::osl::SocketAddr::getLocalHostname();

    aResult[SHARED_EDITTIME_ID] = GetCurrentLocalTime();

    ::utl::Bootstrap::locateUserInstallation( aResult[SHARED_USERURL_ID] );


    return aResult;
}


// ----------------------------------------------------------------------
uno::Sequence< uno::Sequence< ::rtl::OUString > > ShareControlFile::GetUsersData()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    if ( !m_aUsersData.getLength() )
    {
        sal_Int64 nLength = m_xSeekable->getLength();
        if ( nLength > SAL_MAX_INT32 )
            throw uno::RuntimeException();

        uno::Sequence< sal_Int8 > aBuffer( (sal_Int32)nLength );
        m_xSeekable->seek( 0 );

        sal_Int32 nRead = m_xInputStream->readBytes( aBuffer, (sal_Int32)nLength );
        nLength -= nRead;
        while ( nLength > 0 )
        {
            uno::Sequence< sal_Int8 > aTmpBuf( (sal_Int32)nLength );
            nRead = m_xInputStream->readBytes( aTmpBuf, (sal_Int32)nLength );
            if ( nRead > nLength )
                throw uno::RuntimeException();

            for ( sal_Int32 nInd = 0; nInd < nRead; nInd++ )
                aBuffer[aBuffer.getLength() - (sal_Int32)nLength + nInd] = aTmpBuf[nInd];
            nLength -= nRead;
        }

        m_aUsersData = ParseList( aBuffer );
    }

    return m_aUsersData;
}

// ----------------------------------------------------------------------
void ShareControlFile::SetUsersDataAndStore( const uno::Sequence< uno::Sequence< ::rtl::OUString > >& aUsersData )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    if ( !m_xTruncate.is() || !m_xOutputStream.is() || !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xTruncate->truncate();
    m_xSeekable->seek( 0 );

    ::rtl::OUStringBuffer aBuffer;
    for ( sal_Int32 nInd = 0; nInd < aUsersData.getLength(); nInd++ )
    {
        if ( aUsersData[nInd].getLength() != SHARED_ENTRYSIZE )
            throw lang::IllegalArgumentException();

        for ( sal_Int32 nEntryInd = 0; nEntryInd < SHARED_ENTRYSIZE; nEntryInd++ )
        {
            aBuffer.append( EscapeCharacters( aUsersData[nInd][nEntryInd] ) );
            if ( nEntryInd < SHARED_ENTRYSIZE - 1 )
                aBuffer.append( (sal_Unicode)',' );
            else
                aBuffer.append( (sal_Unicode)';' );
        }
    }

    ::rtl::OString aStringData( ::rtl::OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ) );
    uno::Sequence< sal_Int8 > aData( (sal_Int8*)aStringData.getStr(), aStringData.getLength() );
    m_xOutputStream->writeBytes( aData );
    m_aUsersData = aUsersData;
}

// ----------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ShareControlFile::InsertOwnEntry()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    GetUsersData();
    uno::Sequence< ::uno::Sequence< ::rtl::OUString > > aNewData( m_aUsersData.getLength() + 1 );
    uno::Sequence< ::rtl::OUString > aNewEntry = GenerateOwnEntry();

    sal_Bool bExists = sal_False;
    sal_Int32 nNewInd = 0;
    for ( sal_Int32 nInd = 0; nInd < m_aUsersData.getLength(); nInd++ )
    {
        if ( m_aUsersData[nInd].getLength() == SHARED_ENTRYSIZE )
        {
            if ( m_aUsersData[nInd][SHARED_LOCALHOST_ID] == aNewEntry[SHARED_LOCALHOST_ID]
              && m_aUsersData[nInd][SHARED_SYSUSERNAME_ID] == aNewEntry[SHARED_SYSUSERNAME_ID]
              && m_aUsersData[nInd][SHARED_USERURL_ID] == aNewEntry[SHARED_USERURL_ID] )
            {
                if ( !bExists )
                {
                    aNewData[nNewInd] = aNewEntry;
                    bExists = sal_True;
                }
            }
            else
            {
                aNewData[nNewInd] = m_aUsersData[nInd];
            }

            nNewInd++;
        }
    }

    if ( !bExists )
        aNewData[nNewInd++] = aNewEntry;

    aNewData.realloc( nNewInd );
    SetUsersDataAndStore( aNewData );

    return aNewEntry;
}

// ----------------------------------------------------------------------
void ShareControlFile::RemoveEntry( const uno::Sequence< ::rtl::OUString >& aArgEntry )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    GetUsersData();

    uno::Sequence< ::rtl::OUString > aEntry = aArgEntry;
    if ( aEntry.getLength() != SHARED_ENTRYSIZE )
        aEntry = GenerateOwnEntry();

    uno::Sequence< ::uno::Sequence< ::rtl::OUString > > aNewData( m_aUsersData.getLength() + 1 );

    sal_Int32 nNewInd = 0;
    for ( sal_Int32 nInd = 0; nInd < m_aUsersData.getLength(); nInd++ )
    {
        if ( m_aUsersData[nInd].getLength() == SHARED_ENTRYSIZE )
        {
            if ( m_aUsersData[nInd][SHARED_LOCALHOST_ID] != aEntry[SHARED_LOCALHOST_ID]
              || m_aUsersData[nInd][SHARED_SYSUSERNAME_ID] != aEntry[SHARED_SYSUSERNAME_ID]
              || m_aUsersData[nInd][SHARED_USERURL_ID] != aEntry[SHARED_USERURL_ID] )
            {
                aNewData[nNewInd] = m_aUsersData[nInd];
                nNewInd++;
            }
        }
    }

    aNewData.realloc( nNewInd );
    SetUsersDataAndStore( aNewData );

    if ( !nNewInd )
    {
        // try to remove the file if it is empty
        RemoveFile();
    }
}

// ----------------------------------------------------------------------
void ShareControlFile::RemoveFile()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !IsValid() )
        throw io::NotConnectedException();

    Close();

    uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess(
        xFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess") ),
        uno::UNO_QUERY_THROW );
    xSimpleFileAccess->kill( m_aURL );
}

} // namespace svt

