#ifndef _FILSTR_HXX_
#include "filstr.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


// Return class for XStream_impl::getInputStream

class fileaccess::XInputStreamForStream
    : public cppu::OWeakObject,
      public io::XInputStream,
      public io::XSeekable
{
public:

    XInputStreamForStream( XStream_impl* xPtr )
        : m_xPtr( xPtr )
    {
        m_xPtr->acquire();
    }

    ~XInputStreamForStream()
    {
        closeInput();
    }

    uno::Any SAL_CALL
    queryInterface(
        const uno::Type& rType )
        throw( uno::RuntimeException)
    {
        uno::Any aRet = cppu::queryInterface( rType,
                                              SAL_STATIC_CAST( io::XInputStream*,this ),
                                              SAL_STATIC_CAST( io::XSeekable*,this ) );
        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }


    void SAL_CALL
    acquire(
        void )
        throw( uno::RuntimeException)
    {
        OWeakObject::acquire();
    }

    void SAL_CALL
    release(
        void )
        throw( uno::RuntimeException )
    {
        OWeakObject::release();
    }

    sal_Int32 SAL_CALL
    readBytes(
        uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nBytesToRead )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException)
    {
        if( m_xPtr )
            return m_xPtr->readBytes( aData,nBytesToRead );
        else
            throw io::IOException();
    }

    sal_Int32 SAL_CALL
    readSomeBytes(
        uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nMaxBytesToRead )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException)
    {
        if( m_xPtr )
            return m_xPtr->readSomeBytes( aData,nMaxBytesToRead );
        else
            throw io::IOException();
    }

    void SAL_CALL
    skipBytes(
        sal_Int32 nBytesToSkip )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            m_xPtr->skipBytes( nBytesToSkip );
        else
            throw io::IOException();
    }

    sal_Int32 SAL_CALL
    available(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            return m_xPtr->available();
        else
            return 0;
    }

    void SAL_CALL
    closeInput(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
        {
            m_xPtr->closeInput();
            m_xPtr->release();
            m_xPtr = 0;
        }
    }

    void SAL_CALL
    seek(
        sal_Int64 location )
        throw( lang::IllegalArgumentException,
               io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            m_xPtr->seek( location );
        else
            throw io::IOException();
    }

    sal_Int64 SAL_CALL
    getPosition(
        void )
        throw( io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            return m_xPtr->getPosition();
        else
            throw io::IOException();
    }

    sal_Int64 SAL_CALL
    getLength(
        void )
        throw( io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            return m_xPtr->getLength();
        else
            throw io::IOException();
    }


private:

    XStream_impl* m_xPtr;
};




// Return class for XStream_impl::getOutputStream


class fileaccess::XOutputStreamForStream
    : public cppu::OWeakObject,
      public io::XOutputStream,
      public io::XSeekable,
      public io::XTruncate
{
public:

    XOutputStreamForStream( XStream_impl* xPtr )
        : m_xPtr( xPtr )
    {
        m_xPtr->acquire();
    }

    ~XOutputStreamForStream()
    {
        closeOutput();
    }

    // XInterface

    uno::Any SAL_CALL
    queryInterface(
        const uno::Type& rType )
        throw( uno::RuntimeException)
    {
        uno::Any aRet = cppu::queryInterface( rType,
                                              SAL_STATIC_CAST( io::XOutputStream*,this ),
                                              SAL_STATIC_CAST( io::XSeekable*,this ),
                                              SAL_STATIC_CAST( io::XTruncate*,this )
        );
        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }


    void SAL_CALL
    acquire(
        void )
        throw( uno::RuntimeException)
    {
        OWeakObject::acquire();
    }

    void SAL_CALL
    release(
        void )
        throw( uno::RuntimeException )
    {
        OWeakObject::release();
    }

    // XTruncate

    void SAL_CALL truncate( void )
        throw( io::IOException,
               uno::RuntimeException )
    {
        if ( m_xPtr )
            m_xPtr->truncate();
        else
            throw io::IOException();
    }

    // XSeekable

    void SAL_CALL
    seek(
        sal_Int64 location )
        throw( lang::IllegalArgumentException,
               io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            m_xPtr->seek( location );
        else
            throw io::IOException();
    }

    sal_Int64 SAL_CALL
    getPosition(
        void )
        throw( io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            return m_xPtr->getPosition();
        else
            throw io::IOException();
    }

    sal_Int64 SAL_CALL
    getLength(
        void )
        throw( io::IOException,
               uno::RuntimeException )
    {
        if( m_xPtr )
            return m_xPtr->getLength();
        else
            throw io::IOException();
    }


    // XOutputStream

    void SAL_CALL
    writeBytes( const uno::Sequence< sal_Int8 >& aData )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException)
    {
        if( m_xPtr )
            m_xPtr->writeBytes( aData );
        else
            throw io::IOException();
    }

    void SAL_CALL
    flush(  )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException)
    {
        if( m_xPtr )
            m_xPtr->flush();
        else
            throw io::IOException();
    }

    void SAL_CALL
    closeOutput(  )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException)
    {
        if( m_xPtr )
        {
            m_xPtr->closeOutput();
            m_xPtr->release();
            m_xPtr = 0;
        }
    }

private:

    XStream_impl* m_xPtr;
};



/******************************************************************************/
/*                                                                            */
/*               XStream_impl implementation                                  */
/*                                                                            */
/******************************************************************************/


uno::Any SAL_CALL
XStream_impl::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( io::XStream*,this ),
                                          SAL_STATIC_CAST( io::XSeekable*,this ),
                                          SAL_STATIC_CAST( io::XTruncate*,this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL
XStream_impl::acquire(
    void )
    throw( uno::RuntimeException )
{
    OWeakObject::acquire();
}


void SAL_CALL
XStream_impl::release(
    void )
    throw( uno::RuntimeException )
{
    OWeakObject::release();
}



XStream_impl::XStream_impl( shell* pMyShell,const rtl::OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_aFile( aUncPath ),
      m_xProvider( m_pMyShell->m_pProvider ),
      m_pInputStream( 0 ),
      m_pOutputStream( 0 ),
      m_bInputStreamClosed( false ),
      m_bOutputStreamClosed( false ),
      m_nErrorCode( TASKHANDLER_NO_ERROR ),
      m_nMinorErrorCode( TASKHANDLER_NO_ERROR )
{
    osl::FileBase::RC err = m_aFile.open( OpenFlag_Read | OpenFlag_Write );
    if(  err != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFile.close();
        m_bOutputStreamClosed = m_bInputStreamClosed = true;

        m_nErrorCode = TASKHANDLING_OPEN_FOR_STREAM;
        m_nMinorErrorCode = err;
    }
    else
        m_nIsOpen = true;
}


XStream_impl::~XStream_impl()
{
    closeStream();
}


sal_Int32 SAL_CALL XStream_impl::CtorSuccess()
{
    return m_nErrorCode;
}



sal_Int32 SAL_CALL XStream_impl::getMinorError()
{
    return m_nMinorErrorCode;
}



uno::Reference< io::XInputStream > SAL_CALL
XStream_impl::getInputStream(  )
    throw( uno::RuntimeException)
{
    if( ! m_bInputStreamClosed && ! m_pInputStream )
        m_pInputStream = new XInputStreamForStream( this );

    return uno::Reference< io::XInputStream >( m_pInputStream );
}


uno::Reference< io::XOutputStream > SAL_CALL
XStream_impl::getOutputStream(  )
    throw( uno::RuntimeException )
{
    if( ! m_bOutputStreamClosed && ! m_pOutputStream )
        m_pOutputStream = new XOutputStreamForStream( this );

    return uno::Reference< io::XOutputStream >( m_pOutputStream );
}


void SAL_CALL XStream_impl::truncate(void)
    throw( io::IOException, uno::RuntimeException )
{
    if( osl::FileBase::E_None != m_aFile.setSize(0) )
        throw io::IOException();
}



//===========================================================================
// XStream_impl private non interface methods
//===========================================================================

sal_Int32 SAL_CALL
XStream_impl::readBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    if( ! m_nIsOpen )
        throw io::IOException();

    sal_Int8 * buffer;
    try
    {
        buffer = new sal_Int8[nBytesToRead];
    }
    catch( std::bad_alloc )
    {
        if( m_nIsOpen ) m_aFile.close();
        throw io::BufferSizeExceededException();
    }

    sal_uInt64 nrc;
    m_aFile.read( (void* )buffer,sal_uInt64(nBytesToRead),nrc );

    aData = uno::Sequence< sal_Int8 > ( buffer, (sal_uInt32)nrc );
    delete[] buffer;
    return ( sal_Int32 ) nrc;
}


sal_Int32 SAL_CALL
XStream_impl::readSomeBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL
XStream_impl::skipBytes(
    sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    m_aFile.setPos( osl_Pos_Current, sal_uInt64( nBytesToSkip ) );
}


sal_Int32 SAL_CALL
XStream_impl::available(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    return 0;
}


void SAL_CALL
XStream_impl::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    sal_Int32 length = aData.getLength();
    sal_uInt64 nWrittenBytes;
    if( length )
    {
        const sal_Int8* p = aData.getConstArray();
        m_aFile.write( ((void*)(p)),
                       sal_uInt64( length ),
                       nWrittenBytes );
        if( nWrittenBytes != length )
        {
            // DBG_ASSERT( "Write Operation not successful" );
            throw io::IOException();
        }
    }
}


void SAL_CALL
XStream_impl::closeStream(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    if( m_nIsOpen )
    {
        if( osl::FileBase::E_None != m_aFile.close() )
            throw io::IOException();

        m_nIsOpen = false;
    }
    m_pInputStream = 0;
    m_pOutputStream = 0;
    m_bOutputStreamClosed = true;
    m_bInputStreamClosed = true;
}

void SAL_CALL
XStream_impl::closeInput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    closeStream();
}


void SAL_CALL
XStream_impl::closeOutput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    closeStream();
}


void SAL_CALL
XStream_impl::seek(
    sal_Int64 location )
    throw( lang::IllegalArgumentException,
           io::IOException,
           uno::RuntimeException )
{
    if( location < 0 )
        throw lang::IllegalArgumentException();
    if( osl::FileBase::E_None != m_aFile.setPos( Pos_Absolut, sal_uInt64( location ) ) )
        throw io::IOException();
}


sal_Int64 SAL_CALL
XStream_impl::getPosition(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    sal_uInt64 uPos;
    if( osl::FileBase::E_None != m_aFile.getPos( uPos ) )
        throw io::IOException();
    return sal_Int64( uPos );
}

sal_Int64 SAL_CALL
XStream_impl::getLength(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    osl::FileBase::RC   err;
    sal_uInt64          uCurrentPos, uEndPos;

    err = m_aFile.getPos( uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( Pos_End, 0 );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.getPos( uEndPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( Pos_Absolut, uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();
    else
        return sal_Int64( uEndPos );

}


void SAL_CALL
XStream_impl::flush()
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    return;
}
