#ifndef _TEST_MULTISERVICEFAC_HXX_
#define _TEST_MULTISERVICEFAC_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif


namespace test_ftp {

    class Test_MultiServiceFactory
        : public cppu::OWeakObject,
          public com::sun::star::lang::XMultiServiceFactory
    {
    public:

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );


        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();

        // XMultiServiceFactory

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL
        createInstance(
            const ::rtl::OUString& aServiceSpecifier
        )
            throw (
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException
            );

        virtual
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments(
            const ::rtl::OUString& ServiceSpecifier,
            const ::com::sun::star::uno::Sequence
            < ::com::sun::star::uno::Any >& Arguments
        )
            throw (
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException
            );

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getAvailableServiceNames(
        )
            throw (
                ::com::sun::star::uno::RuntimeException
            );
    };

}

#endif
