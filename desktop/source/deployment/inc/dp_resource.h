/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_resource.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-11 17:16:27 $
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

#if ! defined INCLUDED_DP_RESOURCE_H
#define INCLUDED_DP_RESOURCE_H

#include "tools/resmgr.hxx"
#include "tools/string.hxx"
#include "tools/resid.hxx"
#include "com/sun/star/lang/Locale.hpp"
#include "dp_misc.h"
#include <memory>


namespace dp_misc {

//==============================================================================
ResId getResId( USHORT id );

//==============================================================================
String getResourceString( USHORT id );

template <typename Unique, USHORT id>
struct StaticResourceString :
        public ::rtl::StaticWithInit<const ::rtl::OUString, Unique> {
    const ::rtl::OUString operator () () { return getResourceString(id); }
};

//==============================================================================
::com::sun::star::lang::Locale toLocale( ::rtl::OUString const & slang );

//==============================================================================
::com::sun::star::lang::Locale const & getOfficeLocale();

}

#endif
