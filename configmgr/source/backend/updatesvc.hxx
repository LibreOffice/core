/*************************************************************************
 *
 *  $RCSfile: updatesvc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#include <drafts/com/sun/star/configuration/backend/XUpdateHandler.hpp>

// -----------------------------------------------------------------------------
namespace drafts {
namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    class XLayerHandler;
    class XLayer;
} } } } }
}
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
        namespace backenduno = drafts::com::sun::star::configuration::backend;
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




