#ifndef GCONFLAYER_HXX_
#define GCONFLAYER_HXX_

#ifndef GCONFBACKEND_HXX_
#include "gconfbackend.hxx"
#endif // GCONFBACKEND_HXX_


#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_

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
 class GconfLayer : public cppu::WeakImplHelper2<backend::XLayer,
                                                 util::XTimeStamped>
{
    public :
        /**
          Constructor given the configuration keys map and requested
          component name
          @param aKeyMap        Gconf->OO key map
          @param aCompoentName  Requested Component Name
          @param sTimestamp     timestamp indicating last modifictaion
         */
        GconfLayer(
            const rtl::OUString& sComponent,
            const KeyMappingTable& aKeyMap,
            const rtl::OUString& sTimestamp,
            const TSMappingTable& aTSMap,
            const uno::Reference<lang::XMultiServiceFactory>& xFactory);

        /** Destructor */
        ~GconfLayer(void) {}

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
    private :
        /** Convert Gconf Value to OO value */
        void convertGconfValue(
            const GConfValue* aValue,
            uno::Any* aOOValue,
            const rtl::OUString& OOType );

        /** Timestamp of Gconf layer */
        rtl::OUString mTimestamp ;
        rtl::OUString mComponent;
        const KeyMappingTable& mKeyMap;

        /** List of updated component TimeStamps */
        const TSMappingTable& mTSMap;

        uno::Reference<lang::XMultiServiceFactory> mFactory;
  } ;

#endif // GCONFLAYER
