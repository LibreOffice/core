#ifndef _TEST_ACTIVEDATASINK_HXX_
#define _TEST_ACTIVEDATASINK_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif

namespace test_ftp {


    class Test_ActiveDataSink
        : public cppu::OWeakObject,
          public com::sun::star::io::XActiveDataSink
    {
    public:

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );


        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();


        // XActiveDataSink

        virtual void SAL_CALL
        setInputStream(const ::com::sun::star::uno::Reference<::com::sun::star::io::XInputStream>& aStream )
            throw(::com::sun::star::uno::RuntimeException)
        {
            m_xInputStream = aStream;
        }


        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL
        getInputStream(  )
            throw(::com::sun::star::uno::RuntimeException)
        {
            return m_xInputStream;
        }


    private:

        com::sun::star::uno::Reference<com::sun::star::io::XInputStream> m_xInputStream;

    };

}


#endif
