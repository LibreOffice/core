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

#ifndef __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_
#define __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_

#include <framework/fwedllapi.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <svtools/statusbarcontroller.hxx>

typedef svt::ToolboxController* ( *pfunc_setToolBoxControllerCreator)(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    ToolBox* pToolbox,
    unsigned short nID,
    const OUString& aCommandURL );

typedef svt::StatusbarController* ( *pfunc_setStatusBarControllerCreator)(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    StatusBar* pStatusBar,
    unsigned short nID,
    const OUString& aCommandURL );

typedef void ( *pfunc_getRefreshToolbars)(
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

typedef void ( *pfunc_createDockingWindow)(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    const OUString& rResourceURL );

typedef bool ( *pfunc_isDockingWindowVisible)(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    const OUString& rResourceURL );

typedef void ( *pfunc_activateToolPanel)(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame,
    const OUString& i_rPanelURL );


namespace framework
{
FWE_DLLPUBLIC pfunc_setToolBoxControllerCreator SAL_CALL SetToolBoxControllerCreator( pfunc_setToolBoxControllerCreator pSetToolBoxControllerCreator );
FWE_DLLPUBLIC svt::ToolboxController* SAL_CALL CreateToolBoxController(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    ToolBox* pToolbox,
    unsigned short nID,
    const OUString& aCommandURL );

FWE_DLLPUBLIC pfunc_setStatusBarControllerCreator SAL_CALL SetStatusBarControllerCreator( pfunc_setStatusBarControllerCreator pSetStatusBarControllerCreator );
FWE_DLLPUBLIC svt::StatusbarController* SAL_CALL CreateStatusBarController(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    StatusBar* pStatusBar,
    unsigned short nID,
     const OUString& aCommandURL );

FWE_DLLPUBLIC pfunc_getRefreshToolbars SAL_CALL SetRefreshToolbars( pfunc_getRefreshToolbars pRefreshToolbarsFunc );
FWE_DLLPUBLIC void SAL_CALL RefreshToolbars(
     ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

FWE_DLLPUBLIC pfunc_createDockingWindow SAL_CALL SetDockingWindowCreator( pfunc_createDockingWindow pCreateDockingWindow );
FWE_DLLPUBLIC void SAL_CALL CreateDockingWindow(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    const OUString& rResourceURL );

FWE_DLLPUBLIC pfunc_isDockingWindowVisible SAL_CALL SetIsDockingWindowVisible( pfunc_isDockingWindowVisible pIsDockingWindowVisible );
FWE_DLLPUBLIC bool SAL_CALL IsDockingWindowVisible(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    const OUString& rResourceURL );

FWE_DLLPUBLIC pfunc_activateToolPanel SAL_CALL SetActivateToolPanel( pfunc_activateToolPanel i_pActivator );
FWE_DLLPUBLIC void SAL_CALL ActivateToolPanel(
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame,
    const OUString& i_rPanelURL );
}

#endif // __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
