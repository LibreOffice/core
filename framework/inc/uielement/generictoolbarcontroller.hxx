/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX_

#include <svtools/toolboxcontroller.hxx>
#include <vcl/toolbox.hxx>
#include <memory>

class PopupMenu;

namespace framework
{

struct ExecuteInfo;
class ToolBar;
class GenericToolbarController : public svt::ToolboxController
{
    public:
        GenericToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                  const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                  ToolBox* pToolBar,
                                  sal_uInt16   nID,
                                  const rtl::OUString& aCommand );
        virtual ~GenericToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

         DECL_STATIC_LINK( GenericToolbarController, ExecuteHdl_Impl, ExecuteInfo* );

    protected:
        ToolBox*        m_pToolbar;
        sal_uInt16      m_nID;
        sal_Bool        m_bEnumCommand : 1,
                        m_bMadeInvisible : 1;
        rtl::OUString   m_aEnumCommand;
};

class MenuToolbarController : public GenericToolbarController
{
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > m_xMenuDesc;
    PopupMenu* pMenu;
    com::sun::star::uno::Reference< com::sun::star::lang::XComponent > m_xMenuManager;
    rtl::OUString m_aModuleIdentifier;
    public:
        MenuToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                  const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                  ToolBox* pToolBar,
                                  sal_uInt16   nID,
                                  const rtl::OUString& aCommand,
                                  const rtl::OUString& aModuleIdentifier,
                                  const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& xMenuDesc );

    ~MenuToolbarController();
    // XToolbarController
    virtual void SAL_CALL click() throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);

};

}

#endif // __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
