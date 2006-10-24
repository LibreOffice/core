/**
  Copyright 2005 Sun Microsystems, Inc.
*/

#ifndef INCLUDED_RTFTOK_XMLSCANNER_HXX
#define INCLUDED_RTFTOK_XMLSCANNER_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMAIN_HPP_
#include <com/sun/star/lang/XMain.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

namespace writerfilter { namespace rtftok {

class XMLScanner : public cppu::WeakImplHelper1 < ::com::sun::star::lang::XMain >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext;

public:
    XMLScanner(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext);

    virtual ::sal_Int32 SAL_CALL run( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aArguments ) throw (::com::sun::star::uno::RuntimeException);

public:
    const static sal_Char SERVICE_NAME[40];
    const static sal_Char IMPLEMENTATION_NAME[40];

};

::rtl::OUString XMLScanner_getImplementationName ();
sal_Bool SAL_CALL XMLScanner_supportsService( const ::rtl::OUString& ServiceName );
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL XMLScanner_getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL XMLScanner_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext) throw( ::com::sun::star::uno::Exception );

} } /* end namespace writerfilter::rtftok */

#define RTFTOK_XMLSCANNER_COMPONENT_ENTRY \
{\
   writerfilter::rtftok::XMLScanner_createInstance,\
   writerfilter::rtftok::XMLScanner_getImplementationName,\
   writerfilter::rtftok::XMLScanner_getSupportedServiceNames,\
   ::cppu::createSingleComponentFactory,\
   0, 0\
}


#endif /* INCLUDED_RTFTOK_SCANNERTESTSERVICE_HXX */
