#ifndef __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_
#define __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_

#include <svtools/svarray.hxx>
#include <vcl/bitmap.hxx>
#include <tools/string.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#ifndef _COM_SUN_STAR_IO_XOUPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

// #110897#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace framework
{

class ToolBoxConfiguration
{
    public:
        // #110897#
        static sal_Bool LoadToolBox(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rToolbarConfiguration );

        // #110897#
        static sal_Bool StoreToolBox(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rToolbarConfiguration );
};

} // namespace framework

#endif // __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_
