#ifndef _MyJob_HXX

#define _MyJob_HXX



#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_

#include <com/sun/star/task/XJob.hpp>

#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#include <com/sun/star/lang/XServiceInfo.hpp>

#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_

#include <cppuhelper/implbase2.hxx>

#endif



namespace com

{

    namespace sun

    {

        namespace star

        {

            namespace frame

            {

                class XModel;

                class XFrame;

            }



            namespace beans

            {

                struct NamedValue;

            }

        }

    }

}



class MyJob : public cppu::WeakImplHelper2

<

    com::sun::star::task::XJob,

    com::sun::star::lang::XServiceInfo

>

{

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;



public:

    MyJob( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &rxMSF)

        : mxMSF( rxMSF ) {}

    virtual ~MyJob() {}



    // XJob

    virtual ::com::sun::star::uno::Any SAL_CALL execute(

        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments )

        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);



    // XServiceInfo

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )

        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )

        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )

        throw (::com::sun::star::uno::RuntimeException);

};



::rtl::OUString MyJob_getImplementationName()

    throw ( ::com::sun::star::uno::RuntimeException );



sal_Bool SAL_CALL MyJob_supportsService( const ::rtl::OUString& ServiceName )

    throw ( ::com::sun::star::uno::RuntimeException );



::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL MyJob_getSupportedServiceNames(  )

    throw ( ::com::sun::star::uno::RuntimeException );



::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >

SAL_CALL MyJob_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)

    throw ( ::com::sun::star::uno::Exception );



#endif

