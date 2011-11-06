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



#ifndef RPTXML_DBLOADER2_HXX
#define RPTXML_DBLOADER2_HXX

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <vcl/svapp.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/types.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/file.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <vcl/msgbox.hxx>

// -------------------------------------------------------------------------
namespace rptxml
{

    class ORptTypeDetection : public ::cppu::WeakImplHelper2< ::com::sun::star::document::XExtendedFilterDetection, ::com::sun::star::lang::XServiceInfo>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
public:
    ORptTypeDetection(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    // XServiceInfo
    ::rtl::OUString                                     SAL_CALL getImplementationName() throw(  );
    sal_Bool                                            SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(  );
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  SAL_CALL getSupportedServiceNames(void) throw(  );

    // static methods
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException )
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.report.ORptTypeDetection");
    }
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    virtual ::rtl::OUString SAL_CALL detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (::com::sun::star::uno::RuntimeException);
};
}
#endif
