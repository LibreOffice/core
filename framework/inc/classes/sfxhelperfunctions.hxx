/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxhelperfunctions.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 10:17:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
