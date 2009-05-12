#ifndef _LOCALELAYER_HXX_
#define _LOCALELAYER_HXX_

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/PropertyInfo.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif
#include <com/sun/star/util/XTimeStamped.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase2.hxx>


namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

/**
  Implementation of the XLayer interfaces for fixed values
 */

class LocaleLayer : public cppu::WeakImplHelper2<backend::XLayer, util::XTimeStamped>
{
    public :
        /**
          Constructor given the requested component name

          @param aPropInfoListyMap        Gconf->OO key map
          @param aCompoentName  Requested Component Name
          @param sTimestamp     timestamp indicating last modifictaion
         */
        LocaleLayer(const uno::Sequence<backend::PropertyInfo>& aPropInfoList,
            const rtl::OUString& aTimestamp,
            const uno::Reference<uno::XComponentContext>& xContext);

        /** Destructor */
        ~LocaleLayer(void) {}

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

        rtl::OUString m_aTimestamp ;
        rtl::OUString m_aComponent ;

        uno::Reference<backend::XLayerContentDescriber> m_xLayerContentDescriber ;
        uno::Sequence<backend::PropertyInfo> m_aPropInfoList ;
} ;

#endif // _LOCALELAYER_HXX_
