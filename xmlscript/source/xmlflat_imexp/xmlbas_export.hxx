/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef XMLSCRIPT_XMLBAS_EXPORT_HXX
#define XMLSCRIPT_XMLBAS_EXPORT_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase4.hxx>
#include <osl/mutex.hxx>


//.........................................................................
namespace xmlscript
{
//.........................................................................

    // =============================================================================
    // class XMLBasicExporterBase
    // =============================================================================

    typedef ::cppu::WeakImplHelper4<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::document::XExporter,
        ::com::sun::star::document::XFilter > XMLBasicExporterBase_BASE;

    class XMLBasicExporterBase : public XMLBasicExporterBase_BASE
    {
    private:
        ::osl::Mutex                                                                        m_aMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
        sal_Bool                                                                            m_bOasis;

    public:
        XMLBasicExporterBase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, sal_Bool bOasis );
        virtual ~XMLBasicExporterBase();

        // XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XExporter
        virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxDoc )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XFilter
        virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
            throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancel()
            throw (::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class XMLBasicExporter
    // =============================================================================

    class XMLBasicExporter : public XMLBasicExporterBase
    {
    public:
        XMLBasicExporter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~XMLBasicExporter();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class XMLOasisBasicExporter
    // =============================================================================

    class XMLOasisBasicExporter : public XMLBasicExporterBase
    {
    public:
        XMLOasisBasicExporter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~XMLOasisBasicExporter();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace xmlscript
//.........................................................................

#endif // XMLSCRIPT_XMLBAS_EXPORT_HXX
