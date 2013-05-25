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

#ifndef __FRAMEWORK_UIELEMENT_POPUPMENU_TOOLBARCONTROLLER_HXX__
#define __FRAMEWORK_UIELEMENT_POPUPMENU_TOOLBARCONTROLLER_HXX__

#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <macros/xserviceinfo.hxx>
#include <svtools/toolboxcontroller.hxx>

namespace framework
{
    class PopupMenuToolbarController : public svt::ToolboxController
    {
    public:
        virtual ~PopupMenuToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );
        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        // XToolbarController
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);
        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent ) throw ( ::com::sun::star::uno::RuntimeException );

    protected:
        PopupMenuToolbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                                    const ::rtl::OUString &rPopupCommand = rtl::OUString() );
        virtual void functionExecuted( const rtl::OUString &rCommand );
        virtual sal_uInt16 getDropDownStyle() const;
        void createPopupMenuController();

        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >      m_xContext;
        sal_Bool                                                                      m_bHasController;
        com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >             m_xPopupMenu;

    private:
        rtl::OUString                                                                 m_aPopupCommand;
        com::sun::star::uno::Reference< com::sun::star::frame::XUIControllerFactory > m_xPopupMenuFactory;
        com::sun::star::uno::Reference< com::sun::star::frame::XPopupMenuController > m_xPopupMenuController;
    };

    class OpenToolbarController : public PopupMenuToolbarController
    {
    public:
        OpenToolbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        // XServiceInfo
        DECLARE_XSERVICEINFO
    };

    class NewToolbarController : public PopupMenuToolbarController
    {
    public:
        NewToolbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        // XServiceInfo
        DECLARE_XSERVICEINFO
    private:
        void functionExecuted( const rtl::OUString &rCommand );
        void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent ) throw ( ::com::sun::star::uno::RuntimeException );
        void setItemImage( const rtl::OUString &rCommand );
    };

    class WizardsToolbarController : public PopupMenuToolbarController
    {
    public:
        WizardsToolbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        // XServiceInfo
        DECLARE_XSERVICEINFO
    private:
        sal_uInt16 getDropDownStyle() const;
    };
}

#endif
