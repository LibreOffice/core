#ifndef _TREEVIEW_TVFACTORY_HXX_
#define _TREEVIEW_TVFACTORY_HXX_

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif



namespace treeview {

    class TVFactory:
        public cppu::OWeakObject,
        public com::sun::star::lang::XServiceInfo,
        public com::sun::star::lang::XTypeProvider,
        public com::sun::star::lang::XMultiServiceFactory
    {
    public:

        TVFactory( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

        ~TVFactory();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        release(
            void )
            throw( com::sun::star::uno::RuntimeException);


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        // XServiceInfo
        virtual rtl::OUString SAL_CALL
        getImplementationName(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        supportsService(
            const rtl::OUString& ServiceName )
            throw(com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames(
            void )
            throw( com::sun::star::uno::RuntimeException );

        // XMultiServiceFactory

        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        createInstance(
            const rtl::OUString& aServiceSpecifier )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments(
            const rtl::OUString& ServiceSpecifier,
            const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& Arguments )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getAvailableServiceNames( )
            throw( com::sun::star::uno::RuntimeException );

        // Other

        static rtl::OUString SAL_CALL getImplementationName_static();

        static com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames_static();

        static com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > SAL_CALL
        createServiceFactory(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr );

        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        CreateInstance(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory );


    private:

        // Members
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >  m_xMSF;
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface >             m_xHDS;
    };

}


#endif
