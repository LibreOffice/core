#ifndef KDEINETLAYER_HXX_
#define KDEINETLAYER_HXX_

#ifndef KDEBACKEND_HXX_
#include "kdebackend.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
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

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef INCLUDED_VCL_KDE_HEADERS_H
#include <vcl/kde_headers.h>
#endif

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

/**
  Implementation of the XLayer interface for the KDE values mapped into
  the org.openoffice.Inet configuration component.
  */
class KDEInetLayer : public cppu::WeakImplHelper2<backend::XLayer, util::XTimeStamped>
{
public :
    /**
      Constructor given the component context

      @param xContext       The component context
    */

    KDEInetLayer(const uno::Reference<uno::XComponentContext>& xContext);

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
    ~KDEInetLayer(void) {}

private :
    uno::Reference<backend::XLayerContentDescriber> m_xLayerContentDescriber ;

    void SAL_CALL setProxy
        (uno::Sequence<backend::PropertyInfo> &aPropInfoList, sal_Int32 &nProperties,
         int nProxyType, const QString &aNoProxyfor = QString(),
         const QString &aHTTPProxy = QString(), const QString &aFTPProxy = QString(), const QString &aHTTPSProxy = QString()) const;
  } ;

#endif // KDEINETLAYER
