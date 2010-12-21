/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_
#define __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_

#include <framework/fwedllapi.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <svtools/statusbarcontroller.hxx>

typedef svt::ToolboxController* ( *pfunc_setToolBoxControllerCreator)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL );
typedef svt::StatusbarController* ( *pfunc_setStatusBarControllerCreator)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, StatusBar* pStatusBar, unsigned short nID, const ::rtl::OUString& aCommandURL );
typedef void ( *pfunc_getRefreshToolbars)( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
typedef void ( *pfunc_createDockingWindow)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rResourceURL );
typedef bool ( *pfunc_isDockingWindowVisible)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rResourceURL );
typedef void ( *pfunc_activateToolPanel)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame, const ::rtl::OUString& i_rPanelURL );

namespace framework
{

FWE_DLLPUBLIC pfunc_setToolBoxControllerCreator SAL_CALL SetToolBoxControllerCreator( pfunc_setToolBoxControllerCreator pSetToolBoxControllerCreator );

FWE_DLLPUBLIC svt::ToolboxController* SAL_CALL CreateToolBoxController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL );

FWE_DLLPUBLIC pfunc_setStatusBarControllerCreator SAL_CALL SetStatusBarControllerCreator( pfunc_setStatusBarControllerCreator pSetStatusBarControllerCreator );

FWE_DLLPUBLIC svt::StatusbarController* SAL_CALL CreateStatusBarController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, StatusBar* pStatusBar, unsigned short nID, const ::rtl::OUString& aCommandURL );

FWE_DLLPUBLIC pfunc_getRefreshToolbars SAL_CALL SetRefreshToolbars( pfunc_getRefreshToolbars pRefreshToolbarsFunc );

FWE_DLLPUBLIC void SAL_CALL RefreshToolbars( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

FWE_DLLPUBLIC pfunc_createDockingWindow SAL_CALL SetDockingWindowCreator( pfunc_createDockingWindow pCreateDockingWindow );

FWE_DLLPUBLIC void SAL_CALL CreateDockingWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rResourceURL );

FWE_DLLPUBLIC pfunc_isDockingWindowVisible SAL_CALL SetIsDockingWindowVisible( pfunc_isDockingWindowVisible pIsDockingWindowVisible );

FWE_DLLPUBLIC bool SAL_CALL IsDockingWindowVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rResourceURL );

FWE_DLLPUBLIC pfunc_activateToolPanel SAL_CALL SetActivateToolPanel( pfunc_activateToolPanel i_pActivator );

FWE_DLLPUBLIC void SAL_CALL ActivateToolPanel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame, const ::rtl::OUString& i_rPanelURL );
}

#endif // __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_
