#ifndef _FILINPSTR_HXX_
#include "filinpstr.hxx"
#endif
#ifndef _FILERROR_HXX_
#include "filerror.hxx"
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



XInputStream_impl::XInputStream_impl( shell* pMyShell,const rtl::OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_aFile( aUncPath ),
      m_xProvider( pMyShell->m_pProvider ),
      m_nErrorCode( TASKHANDLER_NO_ERROR ),
      m_nMinorErrorCode( TASKHANDLER_NO_ERROR )
{
    osl::FileBase::RC err = m_aFile.open( OpenFlag_Read );
    if( err != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFile.close();

        m_nErrorCode = TASKHANDLING_OPEN_FOR_INPUTSTREAM;
        m_nMinorErrorCode = err;
    }
    else
        m_nIsOpen = true;
}


XInputStream_impl::~XInputStream_impl()
{
    try
    {
        closeInput();
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


sal_Int32 SAL_CALL XInputStream_impl::CtorSuccess()
{
    return m_nErrorCode;
};



sal_Int32 SAL_CALL XInputStream_impl::getMinorError()
{
    return m_nMinorErrorCode;
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XServiceInfo
//////////////////////////////////////////////////////////////////////////////////////////

rtl::OUString SAL_CALL
XInputStream_impl::getImplementationName()
    throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.io.comp.XInputStream");
}



sal_Bool SAL_CALL
XInputStream_impl::supportsService( const rtl::OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return false;
}



uno::Sequence< rtl::OUString > SAL_CALL
XInputStream_impl::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > ret( 0 );
    return ret;
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////


XTYPEPROVIDER_IMPL_4( XInputStream_impl,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      io::XSeekable,
                      io::XInputStream )



uno::Any SAL_CALL
XInputStream_impl::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( io::XInputStream*,this ),
                                          SAL_STATIC_CAST( lang::XTypeProvider*,this ),
                                          SAL_STATIC_CAST( lang::XServiceInfo*,this ),
                                          SAL_STATIC_CAST( io::XSeekable*,this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL
XInputStream_impl::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XInputStream_impl::release(
    void )
    throw()
{
    OWeakObject::release();
}



sal_Int32 SAL_CALL
XInputStream_impl::readBytes(
                 uno::Sequence< sal_Int8 >& aData,
                 sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    if( ! m_nIsOpen ) throw io::IOException();

    aData.realloc(nBytesToRead);
        //TODO! translate memory exhaustion (if it were detectable...) into
        // io::BufferSizeExceededException

    sal_uInt64 nrc;
    m_aFile.read( aData.getArray(),sal_uInt64(nBytesToRead),nrc );

    // Shrink aData in case we read less than nBytesToRead (XInputStream
    // documentation does not tell whether this is required, and I do not know
    // if any code relies on this, so be conservative---SB):
    if (nrc != nBytesToRead)
        aData.realloc(sal_Int32(nrc));
    return ( sal_Int32 ) nrc;
}

sal_Int32 SAL_CALL
XInputStream_impl::readSomeBytes(
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
XInputStream_impl::skipBytes(
    sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    m_aFile.setPos( osl_Pos_Current, sal_uInt64( nBytesToSkip ) );
}


sal_Int32 SAL_CALL
XInputStream_impl::available(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    return 0;
}


void SAL_CALL
XInputStream_impl::closeInput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    if( m_nIsOpen )
    {
        osl::FileBase::RC err = m_aFile.close();
        if( err != osl::FileBase::E_None )
            throw io::IOException();
        m_nIsOpen = false;
    }
}


void SAL_CALL
XInputStream_impl::seek(
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
XInputStream_impl::getPosition(
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
XInputStream_impl::getLength(
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
