#ifndef _DESKTOP_CONFIGINIT_HXX_
#define _DESKTOP_CONFIGINIT_HXX_

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

/** creates a ConfigurationProvider instance
Important: exceptions thrown from that method will contain a readily
displayable message.

    @return
        The default configuration provider for the application or<br/>
        <NULL/>, if startup was canceled

    @throw com::sun::star::configuration::CannotLoadConfigurationException
        if the configuration provider can't be created

    @throw com::sun::star::lang::ServiceNotRegisteredException
        if the ConfigurationProvider service is unknwon

    @throw com::sun::star::lang::WrappedTargetException
        if the configuration backend could be created,
        but incurred a failure later

*/
extern
com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
    CreateApplicationConfigurationProvider( );

#endif
