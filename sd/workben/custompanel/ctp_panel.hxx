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



#ifndef SD_WORKBENCH_CTP_PANEL_HXX
#define SD_WORKBENCH_CTP_PANEL_HXX

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= class SingleColorPanel
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper3    <   ::com::sun::star::drawing::framework::XView
                                                ,   ::com::sun::star::ui::XToolPanel
                                                ,   ::com::sun::star::awt::XPaintListener
                                                >   SingleColorPanel_Base;
    class SingleColorPanel  :public ::cppu::BaseMutex
                            ,public SingleColorPanel_Base
    {
    public:
        SingleColorPanel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XConfigurationController >& i_rConfigController,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& i_rResourceId
        );

        // XToolPanel
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getWindow(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL createAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& ParentAccessible ) throw (::com::sun::star::uno::RuntimeException);

        // XView
        // (no methods)

        // XResource
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId > SAL_CALL getResourceId(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isAnchorOnly(  ) throw (::com::sun::star::uno::RuntimeException);

        // XPaintListener
        virtual void SAL_CALL windowPaint( const ::com::sun::star::awt::PaintEvent& e ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent equivalents
        virtual void SAL_CALL disposing();

    protected:
        ~SingleColorPanel();

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >   m_xResourceId;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                      m_xWindow;
    };

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................

#endif // SD_WORKBENCH_CTP_PANEL_HXX
