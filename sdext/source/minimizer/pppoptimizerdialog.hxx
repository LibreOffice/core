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



#ifndef PPPOPTIMIZERDIALOG_HXX
#define PPPOPTIMIZERDIALOGOG_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/basemutex.hxx>

// ----------------------
// - PPPOptimizerDialog -
// ----------------------

class PPPOptimizerDialog :  protected ::cppu::BaseMutex,
                            public  ::cppu::WeakImplHelper3<
                                    com::sun::star::lang::XInitialization,
                                    com::sun::star::lang::XServiceInfo,
                                    com::sun::star::ui::dialogs::XExecutableDialog >
{
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;
    bool mbInitialized;
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > mxFrame;
    com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer > mxParentWindow;
    rtl::OUString msTitle;

public:

    PPPOptimizerDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~PPPOptimizerDialog();

    // XInitialization
    void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );
};

rtl::OUString PPPOptimizerDialog_getImplementationName();
com::sun::star::uno::Sequence< rtl::OUString > PPPOptimizerDialog_getSupportedServiceNames();
com::sun::star::uno::Reference< com::sun::star::uno::XInterface > PPPOptimizerDialog_createInstance( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & rSMgr )
    throw( com::sun::star::uno::Exception );

// -----------------------------------------------------------------------------

#endif // PPPOPTIMIZERDIALOG_HXX
