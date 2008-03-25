#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif


namespace chelp {

    class BufferedInputStream
        : public cppu::OWeakObject,
          public com::sun::star::io::XInputStream,
          public com::sun::star::io::XSeekable
    {
    private:

        sal_Int32      m_nBufferLocation;
        sal_Int32      m_nBufferSize;
        sal_Int8      *m_pBuffer;
        osl::Mutex     m_aMutex;

    public:

        BufferedInputStream(
            const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream);

        ~BufferedInputStream();

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();


        virtual sal_Int32 SAL_CALL readBytes( com::sun::star::uno::Sequence< sal_Int8 >& aData,
                                              sal_Int32 nBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Int32 SAL_CALL readSomeBytes( com::sun::star::uno::Sequence< sal_Int8 >& aData,
                                                  sal_Int32 nMaxBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Int32 SAL_CALL available( void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL closeInput( void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL seek( sal_Int64 location )
            throw( com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Int64 SAL_CALL getPosition( void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Int64 SAL_CALL getLength( void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );
    };


    extern com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
    turnToSeekable(
        const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream);

}
