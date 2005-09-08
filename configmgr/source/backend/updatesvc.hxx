/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatesvc.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:36:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_UPDATESVC_HXX
#define CONFIGMGR_BACKEND_UPDATESVC_HXX

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XUPDATEHANDLER_HPP_
#include <com/sun/star/configuration/backend/XUpdateHandler.hpp>
#endif

// -----------------------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    class XLayerHandler;
    class XLayer;
} } } } }

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        using rtl::OUString;
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

        class UpdateService : public ::cppu::WeakImplHelper3<
                                            lang::XInitialization,
                                            lang::XServiceInfo,
                                            backenduno::XUpdateHandler
                                        >
        {
        public:
            typedef uno::Reference< uno::XComponentContext > const & CreationArg;

            explicit
            UpdateService(CreationArg _xContext);

            // XInitialization
            virtual void SAL_CALL
                initialize( const uno::Sequence< uno::Any >& aArguments )
                    throw (uno::Exception, uno::RuntimeException);

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL
                getImplementationName(  )
                    throw (uno::RuntimeException);

            virtual sal_Bool SAL_CALL
                supportsService( const ::rtl::OUString& ServiceName )
                    throw (uno::RuntimeException);

            virtual uno::Sequence< ::rtl::OUString > SAL_CALL
                getSupportedServiceNames(  )
                    throw (uno::RuntimeException);

        protected:
            typedef uno::Reference< lang::XMultiServiceFactory >    ServiceFactory;
            typedef uno::Reference< backenduno::XLayer >            Layer;

            ServiceFactory getServiceFactory() const
            { return m_xServiceFactory; }

            void  checkSourceLayer() SAL_THROW( (lang::IllegalAccessException) )
            { validateSourceLayerAndCheckNotEmpty(); }

            Layer getSourceLayer()   SAL_THROW( (lang::IllegalAccessException) );

            void writeUpdatedLayer(Layer const & _xLayer);

            virtual sal_Bool setImplementationProperty(OUString const & aName, uno::Any const & aValue);

            void raiseIllegalAccessException(sal_Char const * pMsg)
                SAL_THROW( (lang::IllegalAccessException) );

        private:
            bool validateSourceLayerAndCheckNotEmpty() SAL_THROW( (lang::IllegalAccessException) );

        private:
            typedef uno::Reference< backenduno::XLayerHandler >     LayerWriter;

            ServiceFactory  m_xServiceFactory;
            Layer           m_xSourceLayer;
            LayerWriter     m_xLayerWriter;
            enum { merge, truncate, protect } m_aSourceMode;

            static ServiceInfoHelper getServiceInfo();
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




