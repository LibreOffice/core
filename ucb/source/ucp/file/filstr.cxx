#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include "com/sun/star/io/IOException.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include "com/sun/star/uno/RuntimeException.hpp"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif

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
                                          SAL_STATIC_CAST( lang::XTypeProvider*,this ),
                                          SAL_STATIC_CAST( lang::XServiceInfo*,this ),
                                          SAL_STATIC_CAST( io::XStream*,this ),
                                          SAL_STATIC_CAST( io::XInputStream*,this ),
                                          SAL_STATIC_CAST( io::XOutputStream*,this ),
                                          SAL_STATIC_CAST( io::XSeekable*,this ),
                                          SAL_STATIC_CAST( io::XTruncate*,this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL
XStream_impl::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XStream_impl::release(
    void )
    throw()
{
    OWeakObject::release();
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XServiceInfo
//////////////////////////////////////////////////////////////////////////////////////////

rtl::OUString SAL_CALL
XStream_impl::getImplementationName()
    throw( uno::RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.io.comp.XStream");
}



sal_Bool SAL_CALL
XStream_impl::supportsService( const rtl::OUString& ServiceName )
    throw( uno::RuntimeException)
{
    return false;
}



uno::Sequence< rtl::OUString > SAL_CALL
XStream_impl::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > ret( 0 );
    return ret;
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////


XTYPEPROVIDER_IMPL_7( XStream_impl,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      io::XStream,
                      io::XSeekable,
                      io::XInputStream,
                      io::XOutputStream,
                      io::XTruncate )



XStream_impl::XStream_impl( shell* pMyShell,const rtl::OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_aFile( aUncPath ),
      m_xProvider( m_pMyShell->m_pProvider ),
      m_nErrorCode( TASKHANDLER_NO_ERROR ),
      m_nMinorErrorCode( TASKHANDLER_NO_ERROR ),
      m_bInputStreamCalled( false ),
      m_bOutputStreamCalled( false )
{
    osl::FileBase::RC err = m_aFile.open( OpenFlag_Read | OpenFlag_Write );
    if(  err != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFile.close();

        m_nErrorCode = TASKHANDLING_OPEN_FOR_STREAM;
        m_nMinorErrorCode = err;
    }
    else
        m_nIsOpen = true;
}


XStream_impl::~XStream_impl()
{
    try
    {
        closeStream();
    }
    catch (io::IOException const &)
    {
        OSL_ENSURE(false, "unexpected situation");
    }
    catch (uno::RuntimeException const &)
    {
        OSL_ENSURE(false, "unexpected situation");
    }
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
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bInputStreamCalled = true;
    }
    return uno::Reference< io::XInputStream >( this );
}


uno::Reference< io::XOutputStream > SAL_CALL
XStream_impl::getOutputStream(  )
    throw( uno::RuntimeException )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bOutputStreamCalled = true;
    }
    return uno::Reference< io::XOutputStream >( this );
}


void SAL_CALL XStream_impl::truncate(void)
    throw( io::IOException, uno::RuntimeException )
{
    if (osl::FileBase::E_None != m_aFile.setSize(0))
        throw io::IOException();

    if (osl::FileBase::E_None != m_aFile.setPos(Pos_Absolut,sal_uInt64(0)))
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

    sal_uInt64 nrc(0);
    if(m_aFile.read( (void* )buffer,sal_uInt64(nBytesToRead),nrc )
       != osl::FileBase::E_None)
    {
        delete[] buffer;
        throw io::IOException();
    }
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
    if(length)
    {
        sal_uInt64 nWrittenBytes(0);
        const sal_Int8* p = aData.getConstArray();
        if(osl::FileBase::E_None != m_aFile.write(((void*)(p)),sal_uInt64(length),nWrittenBytes) ||
           nWrittenBytes != length )
            throw io::IOException();
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
        osl::FileBase::RC err = m_aFile.sync();
        if( err != osl::FileBase::E_None ) {
            io::IOException ex;
            ex.Message = rtl::OUString::createFromAscii(
                "could not synchronize file to disc");
            throw ex;
        }

        err = m_aFile.close();

        if( err != osl::FileBase::E_None ) {
            io::IOException ex;
            ex.Message = rtl::OUString::createFromAscii(
                "could not close file");
            throw ex;
        }

        m_nIsOpen = false;
    }
}

void SAL_CALL
XStream_impl::closeInput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bInputStreamCalled = false;

    if( ! m_bOutputStreamCalled )
        closeStream();
}


void SAL_CALL
XStream_impl::closeOutput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bOutputStreamCalled = false;

    if( ! m_bInputStreamCalled )
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
