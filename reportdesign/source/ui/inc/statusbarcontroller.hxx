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


#ifndef RPTUI_STATUSBARCONTROLLER_HXX
#define RPTUI_STATUSBARCONTROLLER_HXX

#include <svtools/statusbarcontroller.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>

class SfxStatusBarControl;
namespace rptui
{
    typedef com::sun::star::uno::Reference< com::sun::star::frame::XStatusbarController > TStatusbarRef;

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::lang::XServiceInfo> OStatusbarController_BASE;
    class OStatusbarController : public ::svt::StatusbarController,
                                 public OStatusbarController_BASE
    {
        TStatusbarRef   m_rController;
        sal_uInt16      m_nSlotId;
        sal_uInt16      m_nId;
    public:
        OStatusbarController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    private:
        void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
        // XInterface
        DECLARE_XINTERFACE( )
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration

        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUpdatable
        virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusbarController
        virtual ::sal_Bool SAL_CALL mouseButtonDown( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseMove( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseButtonUp( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL command( const ::com::sun::star::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       ::sal_Bool bMouseEvent,
                                       const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                     const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click( const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick( const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException);
    };
}
#endif // DBAUI_STATUSBARCONTROLLER_HXX

