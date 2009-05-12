/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: writersvc.hxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_XML_WRITERSVC_HXX
#define CONFIGMGR_XML_WRITERSVC_HXX

#include "serviceinfohelper.hxx"
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

// -----------------------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    class XLayerHandler;
} } } } }

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace io    = ::com::sun::star::io;
        namespace sax   = ::com::sun::star::xml::sax;
// -----------------------------------------------------------------------------

        template <class BackendInterface>
        class WriterService : public ::cppu::WeakImplHelper4<
                                            lang::XInitialization,
                                            lang::XServiceInfo,
                                            io::XActiveDataSource,
                                            BackendInterface
                                        >
        {
        public:
            explicit
            WriterService(uno::Reference< uno::XComponentContext > const & _xContext);

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

            // XActiveDataSink
            virtual void SAL_CALL
                setOutputStream( const uno::Reference< io::XOutputStream >& aStream )
                    throw (uno::RuntimeException);

            virtual uno::Reference< io::XOutputStream > SAL_CALL
                getOutputStream(  )
                    throw (uno::RuntimeException);

        protected:
            uno::Reference< lang::XMultiServiceFactory > getServiceFactory() const
            { return m_xServiceFactory; }

            uno::Reference< sax::XDocumentHandler > getWriteHandler() throw (uno::RuntimeException);
        private:
            uno::Reference< lang::XMultiServiceFactory >      m_xServiceFactory;
            uno::Reference< sax::XDocumentHandler >          m_xWriter;

            uno::Reference< sax::XDocumentHandler > createHandler() const throw (uno::RuntimeException);

            static ServiceInfoHelper getServiceInfo();
        };

// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




