/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macro_expander.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 11:53:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef CPPUHELPER_SOURCE_MACRO_EXPANDER_HXX
#define CPPUHELPER_SOURCE_MACRO_EXPANDER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

namespace rtl { class OUString; }

namespace cppuhelper {

namespace detail {

/**
 * Helper function to expand macros based on the unorc/uno.ini.
 *
 * @internal
 *
 * @param text
 * Some text.
 *
 * @return
 * The expanded text.
 *
 * @exception com::sun::star::lang::IllegalArgumentException
 * If uriReference is a vnd.sun.star.expand URL reference that contains unknown
 * macros.
 */
::rtl::OUString expandMacros(rtl::OUString const & text);

}

}

#endif
