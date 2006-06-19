#ifndef _WinInetLayer_HXX_
#define _WinInetLayer_HXX_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_PROPERTYINFO_HPP_
#include <com/sun/star/configuration/backend/PropertyInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <wininet.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

/**
  Implementation of the XLayer interfaces for fixed values
 */

class WinInetLayer : public cppu::WeakImplHelper2<backend::XLayer, util::XTimeStamped>
{
    public :
        typedef BOOL ( WINAPI *InternetQueryOption_Proc_T )( HINTERNET, DWORD, LPVOID, LPDWORD );

        /**
          Constructor given the requested component name

          @param lpfnInternetQueryOption   function pointer into wininet.dll
          @param aTimestamp     timestamp indicating last modifictaion
         */
        WinInetLayer(InternetQueryOption_Proc_T lpfnInternetQueryOption,
            const uno::Reference<uno::XComponentContext>& xContext);

        /** Destructor */
        ~WinInetLayer(void) {}

        // XLayer
        virtual void SAL_CALL readData(const uno::Reference<backend::XLayerHandler>& xHandler)
            throw ( backend::MalformedDataException,
                    lang::NullPointerException,
                    lang::WrappedTargetException,
                    uno::RuntimeException) ;

        // XTimeStamped
        virtual rtl::OUString SAL_CALL getTimestamp(void)
            throw (uno::RuntimeException);

    private :

        rtl::OUString m_aComponent ;

        uno::Reference<backend::XLayerContentDescriber> m_xLayerContentDescriber ;

        // The InternetQueryOption function pointer
        InternetQueryOption_Proc_T m_lpfnInternetQueryOption;
} ;

#endif // _WinInetLayer_HXX_
