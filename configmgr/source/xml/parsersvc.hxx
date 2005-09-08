/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parsersvc.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:41:31 $
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

#ifndef CONFIGMGR_XML_PARSERSVC_HXX
#define CONFIGMGR_XML_PARSERSVC_HXX

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
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
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        using rtl::OUString;
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace io    = ::com::sun::star::io;
        namespace sax   = ::com::sun::star::xml::sax;
// -----------------------------------------------------------------------------

        template <class BackendInterface>
        class ParserService : public ::cppu::WeakImplHelper4<
                                            lang::XInitialization,
                                            lang::XServiceInfo,
                                            io::XActiveDataSink,
                                            BackendInterface
                                        >
        {
        public:
            typedef uno::Reference< uno::XComponentContext > const & CreationArg;

            explicit
            ParserService(CreationArg _xContext);

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
                setInputStream( const uno::Reference< io::XInputStream >& aStream )
                    throw (uno::RuntimeException);

            virtual uno::Reference< io::XInputStream > SAL_CALL
                getInputStream(  )
                    throw (uno::RuntimeException);

        protected:
            typedef uno::Reference< sax::XDocumentHandler >         SaxHandler;
            typedef uno::Reference< uno::XComponentContext >        Context;

            Context getContext() const
            { return m_xContext; }

            void parse(SaxHandler const & _xHandler);
            // throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            Context   m_xContext;
            sax::InputSource m_aInputSource;

            static ServiceInfoHelper getServiceInfo();
        };

// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




