#ifndef GCONFLAYER_HXX_
#define GCONFLAYER_HXX_

#ifndef GCONFBACKEND_HXX_
#include "gconfbackend.hxx"
#endif // GCONFBACKEND_HXX_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_

#ifndef _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#endif // _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif // _CPPUHELPER_IMPLBASE2_HXX_


#include <gconf/gconf-client.h>

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

/**
  Implementation of the XLayer interfaces  for Gconf.
  The class performs the read of the contents of a component
  for a given entity.
  The read data is then available through the read data of base class
  BasicPlatformLayer
  The timestamp indicates the last modification time
  */
class GconfLayer : public cppu::WeakImplHelper2<backend::XLayer, util::XTimeStamped>
{
    public :
        /**
          Constructor given the configuration keys map and requested
          component name.

          @param aTimestamp     timestamp indicating last modifictaion
          @param aComponentName Requested Component Name
          @param aKeyMap        Gconf->OO key map
         */

        GconfLayer( const rtl::OUString& aComponent,
                    const rtl::OUString& aTimestamp,
                    const uno::Reference<uno::XComponentContext>& xContext);

        // XLayer
        virtual void SAL_CALL readData(
                const uno::Reference<backend::XLayerHandler>& xHandler)
                throw ( backend::MalformedDataException,
                        lang::NullPointerException,
                        lang::WrappedTargetException,
                        uno::RuntimeException) ;

        // XTimeStamped
        virtual rtl::OUString SAL_CALL getTimestamp(void)
            throw (uno::RuntimeException);

    protected:

    /** Destructor */
        ~GconfLayer(void) {}

    private :
        /** Convert Gconf Value to OO value */
/*        void convertGconfValue(
            const GConfValue* aValue,
            uno::Any* aOOValue,
            const rtl::OUString& OOType );
*/
        /** Timestamp of Gconf layer */
        rtl::OUString m_aTimestamp ;
        rtl::OUString m_aComponent ;

        uno::Reference<backend::XLayerContentDescriber> m_xLayerContentDescriber ;
  } ;

#endif // GCONFLAYER
