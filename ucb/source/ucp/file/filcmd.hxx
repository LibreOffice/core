#ifndef _FILCMD_HXX_
#define _FILCMD_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif


namespace fileaccess {


    // forward
    class shell;


    class XCommandInfo_impl
        : public cppu::OWeakObject,
          public com::sun::star::ucb::XCommandInfo
    {
    public:

        XCommandInfo_impl( shell* pMyShell, const rtl::OUString& aUnqPath );

        virtual ~XCommandInfo_impl();

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

        // XCommandInfo

        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo > SAL_CALL
        getCommands(
            void )
            throw( com::sun::star::uno::RuntimeException);

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByName(
            const rtl::OUString& Name )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::ucb::CommandInfo SAL_CALL
        getCommandInfoByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::ucb::UnsupportedCommandException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasCommandByName(
            const rtl::OUString& Name )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasCommandByHandle(
            sal_Int32 Handle )
            throw( com::sun::star::uno::RuntimeException );


    private:

        shell*                                                                  m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider > m_xProvider;
    };

}

#endif
