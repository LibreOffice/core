#ifndef _TREEVIEW_TVREAD_HXX_
#define _TREEVIEW_TVREAD_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif


namespace treeview {


    struct ConfigData
    {
        sal_uInt64 filelen;
        rtl::OUString fileurl,locale,system;
        rtl::OUString appendix;
    };


    class TVDom;
    class TVChildTarget;

    class TVBase
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::container::XNameAccess,
          public com::sun::star::container::XHierarchicalNameAccess,
          public com::sun::star::util::XChangesNotifier,
          public com::sun::star::lang::XComponent
    {
        friend class TVChildTarget;

    public:

        virtual ~TVBase() { }

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


        // XNameAccess

        virtual com::sun::star::uno::Type SAL_CALL
        getElementType(  )
            throw( com::sun::star::uno::RuntimeException )
        {
            return getCppuVoidType();
        }


        virtual sal_Bool SAL_CALL hasElements()
            throw( com::sun::star::uno::RuntimeException )
        {
            return true;
        }

        // XChangesNotifier

        virtual void SAL_CALL
        addChangesListener(
            const com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >& aListener )
            throw( com::sun::star::uno::RuntimeException )
        {
            // read only
        }

        virtual void SAL_CALL
        removeChangesListener(
            const com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >& aListener )
            throw( com::sun::star::uno::RuntimeException )
        {
            // read only
        }


        // XComponent

        virtual void SAL_CALL dispose( )
            throw( com::sun::star::uno::RuntimeException )
        {
        }

        virtual void SAL_CALL addEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException )
        {
        }

        virtual void SAL_CALL
        removeEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
            throw( com::sun::star::uno::RuntimeException )
        {
        }


        // Abstract functions
        // XNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException) = 0;

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException ) = 0;

        virtual sal_Bool SAL_CALL
        hasByName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException ) = 0;


        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException ) = 0;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException ) = 0;

    }; // end class TVBase





    class TVRead
        : public TVBase
    {
        friend class TVChildTarget;

    public:

        TVRead();

        TVRead( const ConfigData& configData,TVDom* tvDom = 0 );

        ~TVRead();

        // XNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


    private:

        rtl::OUString                    Title;
        rtl::OUString                    TargetURL;
        rtl::Reference< TVChildTarget >  Children;

    };  // end class TVRead






    class TVChildTarget
        : public TVBase
    {
    public:

        TVChildTarget( const ConfigData& configData,TVDom* tvDom );

        TVChildTarget( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

        ~TVChildTarget();

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


    private:
        std::vector< rtl::Reference< TVRead > >   Elements;

        ConfigData init(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

    };  // end class TVChildTarget

}


#endif
