/*************************************************************************
 *
 *  $RCSfile: opostponedtruncationstream.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 14:34:36 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <opostponedtruncationstream.hxx>

using namespace ::com::sun::star;

// ========================================================================
struct PTFStreamData_Impl
{
    uno::Reference< ucb::XSimpleFileAccess > m_xFileAccess;
    sal_Bool m_bDelete;
    ::rtl::OUString m_aURL;

    // the streams below are not visible from outside so there is no need to remember position

    // original stream related members
    uno::Reference< io::XStream > m_xOrigStream;
    uno::Reference< io::XTruncate > m_xOrigTruncate;
    uno::Reference< io::XSeekable > m_xOrigSeekable;
    uno::Reference< io::XInputStream > m_xOrigInStream;
    uno::Reference< io::XOutputStream > m_xOrigOutStream;

    sal_Bool m_bInOpen;
    sal_Bool m_bOutOpen;

    sal_Bool m_bPostponedTruncate;


    PTFStreamData_Impl(
            const uno::Reference< ucb::XSimpleFileAccess >& xFileAccess,
            sal_Bool bDelete,
            const ::rtl::OUString& aURL,
            const uno::Reference< io::XStream >& xOrigStream,
            const uno::Reference< io::XTruncate >& xOrigTruncate,
            const uno::Reference< io::XSeekable >& xOrigSeekable,
            const uno::Reference< io::XInputStream >& xOrigInStream,
            const uno::Reference< io::XOutputStream >& xOrigOutStream )
    : m_xFileAccess( xFileAccess )
    , m_bDelete( bDelete )
    , m_aURL( aURL )
    , m_xOrigStream( xOrigStream )
    , m_xOrigInStream( xOrigInStream )
    , m_xOrigOutStream( xOrigOutStream )
    , m_xOrigTruncate( xOrigTruncate )
    , m_xOrigSeekable( xOrigSeekable )
    , m_bInOpen( sal_False )
    , m_bOutOpen( sal_False )
    , m_bPostponedTruncate( sal_True )
    {}

    void NoPostponing()
    {
        m_bDelete = sal_False;
        m_bPostponedTruncate = sal_False;
    }
};

// ========================================================================
// ------------------------------------------------------------------------
OPostponedTruncationFileStream::OPostponedTruncationFileStream(
        const ::rtl::OUString& aURL,
        const uno::Reference< lang::XMultiServiceFactory >& xFactory,
        const uno::Reference< ucb::XSimpleFileAccess >& xFileAccess,
        sal_Bool bDeleteIfNotCommited )
: m_pStreamData( NULL )
{
    sal_Bool bDelete = sal_False;
    if ( bDeleteIfNotCommited )
        bDelete = !xFileAccess->exists( aURL );

    uno::Reference< io::XStream > xOrigStream = xFileAccess->openFileReadWrite( aURL );
    uno::Reference< io::XTruncate > xOrigTruncate( xOrigStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XSeekable > xOrigSeekable( xOrigStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XInputStream > xOrigInStream = xOrigStream->getInputStream();
    uno::Reference< io::XOutputStream > xOrigOutStream = xOrigStream->getOutputStream();
    if ( !xOrigInStream.is() || !xOrigOutStream.is() )
        throw uno::RuntimeException();

    m_pStreamData = new PTFStreamData_Impl( xFileAccess, bDelete, aURL,
                                            xOrigStream, xOrigTruncate, xOrigSeekable, xOrigInStream, xOrigOutStream );
}

// ------------------------------------------------------------------------
OPostponedTruncationFileStream::~OPostponedTruncationFileStream()
{
    CloseAll_Impl();
}

// ------------------------------------------------------------------------
void OPostponedTruncationFileStream::CloseAll_Impl()
{
    if ( m_pStreamData )
    {
        sal_Bool bDelete = m_pStreamData->m_bDelete;
        ::rtl::OUString aURL = m_pStreamData->m_aURL;
        uno::Reference< ucb::XSimpleFileAccess > xFileAccess = m_pStreamData->m_xFileAccess;

        delete m_pStreamData;
        m_pStreamData = NULL;

        if ( bDelete && xFileAccess.is() && aURL.getLength() )
        {
            // delete the file
            try
            {
                xFileAccess->kill( aURL );
            } catch( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Could not remove the file!" );
            }
        }
    }
}

// ------------------------------------------------------------------------
void OPostponedTruncationFileStream::CheckScheduledTruncation_Impl()
{
    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigTruncate.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigTruncate->truncate();
        m_pStreamData->NoPostponing();
    }
}

// com::sun::star::io::XStream
// ------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OPostponedTruncationFileStream::getInputStream(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pStreamData )
        m_pStreamData->m_bInOpen = sal_True;
    return static_cast< io::XInputStream* >( this );
}


// ------------------------------------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OPostponedTruncationFileStream::getOutputStream(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pStreamData )
        m_pStreamData->m_bOutOpen = sal_True;
    return static_cast< io::XOutputStream* >( this );
}



// com::sun::star::io::XInputStream
// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL OPostponedTruncationFileStream::readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        // the stream must behave as truncated one
        aData.realloc( 0 );
        return 0;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigInStream->readBytes( aData, nBytesToRead );
    }
}


// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL OPostponedTruncationFileStream::readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        // the stream must behave as truncated one
        aData.realloc( 0 );
        return 0;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigInStream->readBytes( aData, nMaxBytesToRead );
    }
}

// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::skipBytes( ::sal_Int32 nBytesToSkip )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        // the stream must behave as truncated one
        if ( nBytesToSkip > 0 )
            throw io::BufferSizeExceededException();

        return;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigInStream->skipBytes( nBytesToSkip );
    }
}


// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL OPostponedTruncationFileStream::available(  )
    throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        // the stream must behave as truncated one
            return 0;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigInStream->available();
    }
}


// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::closeInput()
    throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    m_pStreamData->m_bInOpen = sal_False;
    if ( !m_pStreamData->m_bOutOpen )
        CloseAll_Impl();
}



// com::sun::star::io::XOutputStream
// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::writeBytes( const uno::Sequence< ::sal_Int8 >& aData )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // writing method must check the truncation
    CheckScheduledTruncation_Impl();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigOutStream.is() )
        throw uno::RuntimeException();

    m_pStreamData->m_xOrigOutStream->writeBytes( aData );
}


// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::flush(  )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
    {
        OSL_ENSURE( sal_False, "flush() call on closed stream!\n" );
        return;
        // in future throw exception, for now some code might call flush() on closed stream
        // since file ucp implementation allows it
        // throw io::NotConnectedException();
    }

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        // it is no writing call, thus must be ignored
        return;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigOutStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigOutStream->flush();
    }
}


// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::closeOutput(  )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    m_pStreamData->m_bOutOpen = sal_False;
    if ( !m_pStreamData->m_bInOpen )
        CloseAll_Impl();
}



// com::sun::star::io::XTruncate
// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::truncate(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        // the truncation is already scheduled, ignore
        return;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigTruncate.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigTruncate->truncate();
    }
}



// com::sun::star::io::XSeekable
// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::seek( ::sal_Int64 location )
    throw (lang::IllegalArgumentException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        if ( location > 0 )
            throw lang::IllegalArgumentException();

        return;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigSeekable.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigSeekable->seek( location );
    }
}


// ------------------------------------------------------------------------
::sal_Int64 SAL_CALL OPostponedTruncationFileStream::getPosition(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        return 0;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigSeekable.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigSeekable->getPosition();
    }
}


// ------------------------------------------------------------------------
::sal_Int64 SAL_CALL OPostponedTruncationFileStream::getLength(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
    {
        return 0;
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigSeekable.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigSeekable->getLength();
    }
}

// ------------------------------------------------------------------------
void SAL_CALL OPostponedTruncationFileStream::waitForCompletion()
    throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bPostponedTruncate )
        return;

    uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor( m_pStreamData->m_xOrigOutStream, uno::UNO_QUERY );
    if ( asyncOutputMonitor.is() )
        asyncOutputMonitor->waitForCompletion();
}

