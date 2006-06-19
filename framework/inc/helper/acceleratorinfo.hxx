/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceleratorinfo.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:52:19 $
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

#ifndef __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_
#define __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _SV_KEYCODE_HXX
#include <vcl/keycod.hxx>
#endif

#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace framework
{

typedef ::rtl::OUString ( *pfunc_getCommandURLFromKeyCode)( const KeyCode& );
typedef KeyCode ( *pfunc_getKeyCodeFromCommandURL )( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aURL );

pfunc_getCommandURLFromKeyCode SAL_CALL SetCommandURLFromKeyCode( pfunc_getCommandURLFromKeyCode );
pfunc_getKeyCodeFromCommandURL SAL_CALL SetKeyCodeFromCommandURL( pfunc_getKeyCodeFromCommandURL );

::rtl::OUString SAL_CALL GetCommandURLFromKeyCode( const KeyCode& aKeyCode );
KeyCode GetKeyCodeFromCommandURL( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const rtl::OUString& aCommandURL );

}

#endif // __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_
