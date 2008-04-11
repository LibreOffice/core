/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: acceleratorinfo.hxx,v $
 * $Revision: 1.6 $
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

#ifndef __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_
#define __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_

#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/keycod.hxx>
#include <rtl/ustring.hxx>

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
