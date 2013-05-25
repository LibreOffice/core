/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
                                    const OUString &rPopupCommand = OUString() );
        virtual void functionExecuted( const OUString &rCommand );
        virtual sal_uInt16 getDropDownStyle() const;
        void createPopupMenuController();

        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >      m_xContext;
        sal_Bool                                                                      m_bHasController;
        com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >             m_xPopupMenu;

    private:
        OUString                                                                      m_aPopupCommand;
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
        void functionExecuted( const OUString &rCommand );
        void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent ) throw ( ::com::sun::star::uno::RuntimeException );
        void setItemImage( const OUString &rCommand );
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
