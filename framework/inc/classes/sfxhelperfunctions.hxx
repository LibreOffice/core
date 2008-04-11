/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sfxhelperfunctions.hxx,v $
 * $Revision: 1.7 $
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

#include <com/sun/star/frame/XFrame.hpp>
#include <rtl/ustring.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <svtools/statusbarcontroller.hxx>

typedef svt::ToolboxController* ( *pfunc_setToolBoxControllerCreator)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL );
typedef svt::StatusbarController* ( *pfunc_setStatusBarControllerCreator)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, StatusBar* pStatusBar, unsigned short nID, const ::rtl::OUString& aCommandURL );
typedef void ( *pfunc_getRefreshToolbars)( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

namespace framework
{

pfunc_setToolBoxControllerCreator SAL_CALL SetToolBoxControllerCreator( pfunc_setToolBoxControllerCreator pSetToolBoxControllerCreator );

svt::ToolboxController* SAL_CALL CreateToolBoxController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL );

pfunc_setStatusBarControllerCreator SAL_CALL SetStatusBarControllerCreator( pfunc_setStatusBarControllerCreator pSetStatusBarControllerCreator );

svt::StatusbarController* SAL_CALL CreateStatusBarController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, StatusBar* pStatusBar, unsigned short nID, const ::rtl::OUString& aCommandURL );

pfunc_getRefreshToolbars SAL_CALL SetRefreshToolbars( pfunc_getRefreshToolbars pRefreshToolbarsFunc );

void SAL_CALL RefreshToolbars( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

}

#endif // __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_
